static const auto maxPowerOfTwoAllowed = 29;
static const auto minPowerOfTwoAllowed = 15;

template <class SourceHeap>
void * GCMalloc<SourceHeap>::malloc(size_t sz) {
  // If Size is lesser than zero return Null pointer
  if(sz <= 0)
    return nullptr;

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
  
  // Step 1 : Check if such a free list exists
  for(int index=sizeClass;index<maxAvailableIndices;index++){ // Runs from size Class to  sizeClass < 1039
    // When searches happens and blocks need  to be selected only one thread has to take that block. Two or more threads should not have an allocation to a single block
    // heapLock.lock();
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
      break;

    }
    // heapLock.unlock();

  }

  if(BlockAvailable){
    ptr = block;
    // YOU DONT NEED TO CHANGE BLOCKS ALLOCATED SIZE;
  }
  else{
    // We Need to carefully allocate memory, no two threads can have same memory address ptr
    //http://www.devx.com/tips/Tip/12582
    ptr = SourceHeap::malloc(maxRequiredSizeFromHeap);
    block = new (ptr) Header;
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

  heapLock.unlock();

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
  return h->allocatedSize;
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
int GCMalloc<SourceHeap>::getSizeClass(size_t sz) {
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
