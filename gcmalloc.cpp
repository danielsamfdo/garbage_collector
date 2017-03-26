static const auto maxPowerOfTwoAllowed = 29;
static const auto minPowerOfTwoAllowed = 15;

template <class SourceHeap>
GCMalloc<SourceHeap>::GCMalloc(){
    startHeap = SourceHeap::getStart();
    endHeap = SourceHeap::getStart();// static_cast<char *>(SourceHeap::getStart()) + SourceHeap::getRemaining();
    allocatedObjects = nullptr;
    for (auto& f : freedObjects) {
      f = nullptr;
    }
    initialized = false;
}


template <class SourceHeap>
void * GCMalloc<SourceHeap>::malloc(size_t sz) {
  // If Size is lesser than zero return Null pointer
  if(sz <= 0)
    return nullptr;

  if(initialized && triggerGC(sz)){
    gc();
  }
  unsigned int headerSize = sizeof(Header);
  
  int sizeClass = GCMalloc<SourceHeap>::getSizeClass(sz);
  if(sizeClass==-1){ // If Unsatisfiable Request Return NULL
    return NULL;
  }

  void *ptr;
  size_t allocatedForTheRequest = GCMalloc<SourceHeap>::getSizeFromClass(sizeClass);
  size_t maxRequiredSizeFromHeap = headerSize + allocatedForTheRequest;
  bool BlockAvailable = false;
  Header *block = nullptr;
  // Header **HeaderFreelistArray = freedObjects;
  int maxAvailableIndices = Threshold/Base + 15;
  heapLock.lock();

  int index = sizeClass;
  if(freedObjects[index]!=nullptr){
    block = freedObjects[index];
    if(freedObjects[index]->nextObject != nullptr){
      freedObjects[index] = freedObjects[index]->nextObject;
      freedObjects[index]->prevObject = nullptr;
    }
    else{
      freedObjects[index] = nullptr;
    }
    BlockAvailable = true;
  }

  if(BlockAvailable){
    ptr = block;
    // YOU DONT NEED TO CHANGE BLOCKS ALLOCATED SIZE;
  }
  else{
    // We Need to carefully allocate memory, no two threads can have same memory address ptr
    //http://www.devx.com/tips/Tip/12582
    objectsAllocated+=1;
    // if(objectsAllocated > 246)
    initialized = true;
    ptr = SourceHeap::malloc(maxRequiredSizeFromHeap);
    block = new (ptr) Header;
    // tprintf("Size of Header @\n",(size_t) static_cast<char*>(ptr));
    endHeap = (static_cast<char*>(ptr) + headerSize + allocatedForTheRequest);
    // void * endPointer = (static_cast<char*>(endHeap));
    // tprintf("START HEAP  @\n",(size_t)startHeap);
    // tprintf("END HEAP UPDATE @\n",(size_t)endHeap);
  }

  // When we are altering links, we need to make sure it is thread safe.
  if(allocatedObjects==nullptr){
    allocatedObjects = block;
    block->prevObject = nullptr;
    block->nextObject = nullptr;
  }
  else{
    block->prevObject = nullptr;
    block->nextObject = allocatedObjects;
    allocatedObjects->prevObject = block;
    allocatedObjects = block;
  }
  block->setCookie();
  heapLock.unlock();
  allocated += allocatedForTheRequest;
  block->setAllocatedSize(allocatedForTheRequest);
  // http://stackoverflow.com/questions/6449935/increment-void-pointer-by-one-byte-by-two
  ptr = static_cast<char*>(ptr) + headerSize;
  // http://stackoverflow.com/questions/1898153/how-to-determine-if-memory-is-aligned-testing-for-alignment-not-aligning
  if((((unsigned long)ptr % Alignment) != 0))
    return nullptr; // Memory Alignment Issue
  return ptr;
  
}

template <class SourceHeap>
size_t GCMalloc<SourceHeap>::getSize(void * p) {
  void *ptr = static_cast<char*>(p) - sizeof(Header);
  Header *h = static_cast<Header*>(ptr);
  return h->getAllocatedSize();
}


template <class SourceHeap>
void GCMalloc<SourceHeap>::walk(const std::function< void(Header *) >& f) {
  // FIX ME
  // f();
  Header *iterator = allocatedObjects;
  printf("WALKING THROUGH ALLOCATED OBJECTS\n");
  // size_t count = 0;
  while(iterator!=nullptr){
    f(iterator);
    iterator = iterator->nextObject;
    // count++;
  }
  // printf("Count : %d\n",count);
}

template <class SourceHeap>
size_t GCMalloc<SourceHeap>::getSizeFromClass(int index) {
  int ans = 0;

  //If Index is not within bounds
  if(index <0 || index >= (Threshold/Base) + (maxPowerOfTwoAllowed-minPowerOfTwoAllowed+1))
      return -1;// To Check
      
  // If it lies within multiples of 16
  if(index<(Threshold/Base)){
    ans = index*Base + Base;
  }
  else if(index >=(Threshold/Base)){// If it is after the multiples of 16, in the powers of Two blocks
    int maxIndexAllowedForMultiplesOfThresholdByBase = (Threshold/Base)-1; // 1023 is the max allowed index accessible by FreedObjects 
    index -= maxIndexAllowedForMultiplesOfThresholdByBase;
    ans = 1 << (index+(maxPowerOfTwoAllowed-minPowerOfTwoAllowed));
  }
  return ans;
}


template <class SourceHeap>
int constexpr GCMalloc<SourceHeap>::getSizeClass(size_t sz) {
  int number = sz;
  int classInd = 0;
  int maxIndexAllowedForMultiplesOfThresholdByBase = (Threshold/Base)-1; // 1023 is the max allowed index accessible by FreedObjects 

  size_t maxSizeAllowed = 1<<maxPowerOfTwoAllowed;//2^29 is the max allowed
  int sizeIndexForMultiples = (Threshold/Base);
  // Return -1 if size is exceededing max Size
  if(sz > maxSizeAllowed )
    return -1;

  // For those which is less than 16*1024
  if((number/Base)<sizeIndexForMultiples){
    classInd = number/Base;
    if(number%Base==0){
      classInd-=1;
    }
    return classInd;
  }
  // Exactly 1024*16
  if((number/Base)==sizeIndexForMultiples && number%Base==0){
    return maxIndexAllowedForMultiplesOfThresholdByBase;
  }
  // For higher than 1024*16 till 2**29
  classInd = maxIndexAllowedForMultiplesOfThresholdByBase;
  number = 1;
  int position = 0;
  while(number<sz){
    number = number << 1;
    position += 1;    
  }
  classInd+=position-minPowerOfTwoAllowed+1;
  return classInd;
}


// Scan through this region of memory looking for pointers to mark (and mark them).
template <class SourceHeap>
void GCMalloc<SourceHeap>::scan(void * start, void * end){
  char * start_p = static_cast<char*>(start);
  char * end_p = static_cast<char*>(end);
  void ** ptr = (void **) start_p;
  for(char *i = start_p;i<end_p;i+=8){
    if(isPointer((void *) *ptr))
      markReachable((void *) *ptr);
  }
  // tprintf("SCAN ADDR : @ \n", (size_t)(p));
  // tprintf("SCAN ADDR : @ \n", (size_t)(p));
}
  
// Indicate whether it is time to trigger a garbage collection
// (call this inside your malloc).
// Clearly this must happen when the heap is entirely full,
// but for performance reasons, you should trigger GCs more frequently
// (though not too frequently) using the fields below.
template <class SourceHeap>
bool GCMalloc<SourceHeap>::triggerGC(size_t szRequested){
  // TO DO ADD CONDITIONS
  return true;
}

// Perform a garbage collection pass.
template <class SourceHeap>
void GCMalloc<SourceHeap>::gc(){
  if(!inGC){
    void * st = SourceHeap::getStart();
    inGC = true;
    //tprintf("Start @ End @ \n",(size_t)startHeap,(size_t)endHeap);
    tprintf("Objects Allocated before mark and sweep @ \n",objectsAllocated);
    mark();
    // sweep();
    tprintf("Objects Allocated after mark and sweep @ \n",objectsAllocated);
    inGC = false;
  }
}
  
  // Mark all reachable objects.
template <class SourceHeap>
void GCMalloc<SourceHeap>::mark(){
  // tprintf("Doing Mark Phase\n");
  sp.walkGlobals([&](void * p){ if(isPointer(p)){markReachable(p); } });//void *ptr = static_cast<char*>(p) - sizeof(Header); Header *h = static_cast<Header*>(ptr); int v = h->validateCookie(); tprintf("It is @ pointer @\n",v,(size_t)ptr);});
  // tprintf("It is allocated : @ pointer \n",h->getAllocatedSize()); 
  sp.walkStack([&](void * p){ if(isPointer(p)){markReachable(p); }});// });//void *ptr = static_cast<char*>(p) - sizeof(Header); Header *h = static_cast<Header*>(ptr); int v = h->validateCookie(); tprintf("It is @ pointer @\n",v,(size_t)ptr);});

}

// Mark one object as reachable and recursively mark everything reachable from it.
template <class SourceHeap>
void GCMalloc<SourceHeap>::markReachable(void * ptr){
  Header *h;
  void *p;
  do{
    p = static_cast<char*>(ptr) - sizeof(Header);
    h = static_cast<Header*>(p);
    if(h->validateCookie())
      break;
    ptr = static_cast<char*>(ptr) - 1;
    if(ptr<static_cast<char*>(startHeap)+32 || ptr >endHeap)
      return;
  }while(true);
  
  if(h->validateCookie() && !h->isMarked()){
    tprintf("Marked Object, Requested Size @ , Address: @\n",(size_t)h->getAllocatedSize(),(size_t)ptr);
    h->mark();
    void* start = static_cast<char *>((void *)h) + sizeof(Header);
    void* end = static_cast<char *>((void *)h) + sizeof(Header) + h->getAllocatedSize();
    scan(start,end);
    //tprintf("It is allocated address : @ pointer and is marked : @ \n",(size_t)ptr, (size_t)h->isMarked());
  }
  // tprintf("It is allocated : @ pointer \n",(size_t)h->validateCookie());
}

// Reclaim all unreachable objects (add to free lists).
template <class SourceHeap>
void GCMalloc<SourceHeap>::sweep(){
  Header *iterator = allocatedObjects;
  // tprintf("Doing Sweep\n");
  size_t count = 0;
  while(iterator!=nullptr){
    void * ptr = static_cast<char *>((void *)iterator)+sizeof(Header);
    Header *h = iterator;
    if(!iterator->isMarked()){
      void * headerToBeFreedPtr = static_cast<char *>((void *)iterator);
      iterator = iterator->nextObject;
      tprintf("Freeing at @ \n", (size_t)ptr);
      objectsAllocated-=1;
      // privateFree(headerToBeFreedPtr);
    }else{
      tprintf("Not Freeing at @ \n", (size_t)ptr);
      iterator = iterator->nextObject;
      h->clear();
    }
  }
}

// Free one object.
template <class SourceHeap>
void GCMalloc<SourceHeap>::privateFree(void * p){
  // void * ptr = static_cast<char *>((void *)p)+sizeof(Header);
  Header *h = static_cast<Header*>(p);
  size_t sizeClass = GCMalloc<SourceHeap>::getSizeClass(h->getAllocatedSize());
  if(h == allocatedObjects){ // If the free block is the head
    allocatedObjects = allocatedObjects->nextObject;
    // iterator = allocatedObjects;
  }
  else{ // If the free block is not the head
      Header *prev = h->prevObject;
      Header *next = h->nextObject;
      if(prev)
        prev->nextObject = next;
      if(next)
        next->prevObject = prev;
    }
  h->prevObject = nullptr;
  h->nextObject = freedObjects[sizeClass];
  freedObjects[sizeClass] = h;
  tprintf("valid Header for freeing : @\n",(size_t)(h->validateCookie()));
  allocated -= h->getAllocatedSize();
  
}

  // Returns true if the argument looks like a pointer that we allocated.
  // This should be as precise as possible without ignoring real allocated objects.
  // Just returning true is *not* an option :)
template <class SourceHeap>
bool GCMalloc<SourceHeap>::isPointer(void * p){
  size_t value = (size_t)p;
  // void * st = static_cast<char *>(p) + sizeof(Header);
  if(startHeap<= p && p<=endHeap){
    // tprintf("checking @ is within @ and @",(size_t)&p, (size_t)&startHeap, (size_t)&endHeap);
    return true;
  }
  return false;
}


// number of bytes currently allocated  
template <class SourceHeap>
size_t GCMalloc<SourceHeap>::bytesAllocated() {
  return allocated;
}