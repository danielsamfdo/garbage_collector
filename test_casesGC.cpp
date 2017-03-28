#include <iostream>
#include <cstring>
#include <assert.h>
using namespace std;
class Node {
private:
  size_t number;  // magic number at the start of every object
public:
  // prev and next objects, whether allocated or freed.
  Node * prevObject;
  Node * nextObject;
};
#include "tprintf.hh"

#include "gcmalloc.hh"
extern "C"
{
  void gc();
  //  void * xxmalloc(size_t);
  //  void xxfree(void *);
}

Node * addObject(){
  Node * h = (Node *) malloc(sizeof(Node));
  h->prevObject = nullptr;
  h->nextObject = nullptr;
  return h;
}
Node * t;

void testCaseGlobal(){
  t =(Node*) malloc(sizeof(Node));
  gc();
  Node * t1 =(Node*) malloc(sizeof(Node));
  t->nextObject = t1;
  Node * t2 =(Node*) malloc(sizeof(Node));
  t1->nextObject = t2;
  return;
}

void testCaseRecursion(){
  gc();
  Node * h = (Node *) malloc(sizeof(Node));
  h->nextObject = h;
  gc();
}

void testCaseGarbageFive(){
  gc();
  Node * h = (Node*)malloc(sizeof(Node));
  Node * it = h;
  int i=1;
  while(i<5){
    it->nextObject = addObject();
    tprintf("Address of It : @ \n", (size_t)(it->nextObject));
    it = it->nextObject;
    i++;
  }
  h->nextObject = nullptr;
  gc();
  Node * h1 = (Node*)malloc(sizeof(Node));
  tprintf("Address of h1 : @ \n", (size_t)(h1));
  h1 = (Node*)malloc(sizeof(Node));
  tprintf("Address of h1 : @ \n", (size_t)(h1));
  h1 = (Node*)malloc(sizeof(Node));
  tprintf("Address of h1 : @ \n", (size_t)(h1));
  h1 = (Node*)malloc(sizeof(Node));
  tprintf("Address of h1 : @ \n", (size_t)(h1));
}

void functionOne(){
  cout << "--------------------------------------------------" << endl;
  char * t = (char *) malloc(2000);
  void * p;
  void * ptr;
  Header *h;
  cout<< " New Variable with same requirements Address : " << (size_t) t <<endl;
  ptr = static_cast<char*>(t) - sizeof(Header);
  h = static_cast<Header*>(ptr);
  char * t2 = (char *) malloc(2000);
  cout<< "Header h for the pointer t " << (size_t) t << " is " <<h->isMarked()<< " and cookie is " << h->validateCookie() <<endl;
}


void funcCall(){
  cout << " -------------- LOCAL BLOCK START ---------------------- " << endl;
  char * inScp = (char *) malloc(256);
  cout << "Address of inScp is ::: " << (size_t) inScp << endl;
  // strcpy(inScp, "inScp : This should be intact.\n");
  // inScp=nullptr;
  cout << " -------------- LOCAL BLOCK END ---------------------- " << endl;
}


int testme2;
static const auto maxNextGC = 1073741824/64;
int * testme = (int *)malloc(maxNextGC);

void testGlobalsStillPresent(){
  cout << "--------------------GLOBALS TEST CASE ------------" << endl;
  int * t = testme;
  cout<< (size_t)testme <<endl;
  *t = 5000;
  assert(*testme == 5000);
  char * t3 = (char *) malloc(maxNextGC);
  char * t4 = (char *) malloc(maxNextGC); //Trigger GC
  cout<< " Address of t3 " << (size_t)t3 << endl;
  cout<< " Address of t4 " << (size_t)t4 << endl;
  strcpy(t4, "This should be intact.\n");
  void * p;
  void * ptr;
  Header *h;
  ptr = ((char *)((void *)testme)) - sizeof(Header);
  h = static_cast<Header*>(ptr);
  assert(h->isMarked()==false);
  assert(h->validateCookie()==true);
  assert(*testme == 5000);//Value is Unchanged and not Garbage Collected
  cout << "--------------------------------------------------" << endl;
}

void functionTwo(Header *h){
  cout << "--------------------------------------------------" << endl;
  Header *t = (Header *)malloc(sizeof(Header));
  cout<< " Local Variable Address Still to Be reached : " << (size_t) t <<endl;
  t->nextObject = nullptr;
  t->prevObject = h;
  h->nextObject = t;
}

void testCaseLivenessOne(){
  // check if Object is still live
  void * p;
  if(true){
    char * m = (char *)malloc(maxNextGC);
    p = m;
    strcpy(m, "m : This should be intact.\n");
  }
  char * m2 = (char *)malloc(maxNextGC); // GC is triggered
  strcpy(m2, "m2 : This should be intact.\n");
  char * m3 = (char *)malloc(maxNextGC); // GC is triggered
  strcpy(m3, "m3 : This should be intact.\n");
  char * m4 = (char *)malloc(maxNextGC); // GC is triggered
  strcpy(m4, "m4 : This should be intact.\n");
  cout << (char *)p << endl;
  assert(strcmp((char*)p, "m : This should be intact.\n") == 0);
  void * ptr;
  Header *h;
  ptr = ((char *)((void *)p)) - sizeof(Header);
  h = static_cast<Header*>(ptr);
  assert(h->validateCookie()==true);
  assert(h->getAllocatedSize()==maxNextGC);
  // 

}

void testCaseLivenessTwo(){
  cout << "----------- LIVENESS TEST CASE 2 -----------" << endl;
  void * p;
  int offset = 20;
  void * startStr;

  if(true){
    char * q = (char *) malloc(256);
    cout << "Address of Q is ::: " << (size_t) q << endl;
    strcpy(q, "q : This should be intact.\n");
    q = q + offset;  
    p = q;
    cout<< (char*)p<<endl;
  }
  size_t s = 0;
  while(s<=maxNextGC){
    // cout<< (char*)p<<endl;
    char * q2 = (char *)malloc(256); 
    strcpy(q2, "q2 : This should be intact.\n");
    s+=256;
  }

  void * ptr;
  Header *h;
  ptr = ((char *)(p)) - offset - sizeof(Header);
  p = (char *)(p) - offset;
  h = static_cast<Header*>(ptr);
  assert(h->validateCookie()==true);
  assert(h->getAllocatedSize()==256);
  cout << "Address of P is ::: " << (size_t) p << endl;
  cout << (char *)p << endl;
  assert(strcmp((char*)p, "q : This should be intact.\n") == 0);
  // 
  cout << "----------- LIVENESS TEST CASE END -----------" << endl;

}

int * ptrRet512(){
  int * t = (int  *) malloc(512);
  cout << "Address of ptr 512 "<<(size_t)t <<endl;
  return t;
}

void testLivenessThree(){
  int * p = ptrRet512();
  void *ptr = ((char*)((void *)p)) - sizeof(Header);
  Header *h = (Header *)(ptr);
  cout << "Address of ptr 512 "<<(size_t)p <<endl;
  assert(h->validateCookie());
  assert(h->isMarked()==false);

}

void testGarbageFour(){
  int * t = (int  *) malloc(512);
  tprintf( "Address of ptr t1 @ \n ",(size_t)t);
  t = (int  *) malloc(512);
  tprintf( "Address of ptr t2 @ \n ",(size_t)t);
  t = (int  *) malloc(512);
  tprintf( "Address of ptr t3 @ \n ",(size_t)t);
  t = (int  *) malloc(512);
  tprintf( "Address of ptr t @ \n ",(size_t)t);
  gc();
  int *t1 = (int  *) malloc(512);
  tprintf( "Address of ptr t1 @ \n ",(size_t)t1);
  int *t2 = (int  *) malloc(512);
  tprintf( "Address of ptr t2 @ \n ",(size_t)t2);
  int *t3 = (int  *) malloc(512);
  tprintf( "Address of ptr t3 @ \n ",(size_t)t3);
}

void testGarbageThree(){
  cout << "----------- Garbage TEST CASE 3 -----------" << endl;
  ptrRet512();
  size_t s = 0;
  gc();
  char * outScp = (char *) malloc(512);
  cout << "Address of outScp is ::: " << (size_t) outScp << endl;
  cout << "----------- Garbage TEST CASE 3 END -----------" << endl;
}

void testCaseGarbageOne(){
  cout << "----------- Garbage TEST CASE 1 -----------" << endl;

  if(true){
    funcCall();
  }
  gc();
  char * outScp = (char *) malloc(256);;//(char *) malloc(256);
  
  cout << "Address of outScp is ::: " << (size_t) outScp << endl;
  cout << "----------- Garbage TEST CASE 1 END -----------" << endl;

}

void testCaseGarbageTwo(){
  cout << "----------- Garbage TEST CASE 2 -----------" << endl;

  if(true){
    funcCall();
  }
  size_t s = 0;
  while(s<=maxNextGC){
    char * q2 = (char *)malloc(10000); 
    strcpy(q2, "GC : This should be intact.\n");
    // cout << "Address of inBetween Variables is ::: " << (size_t) q2 << endl;
    s+=10000;
  }
  char * outScp = (char *) malloc(256);
  cout << "Address of outScp is ::: " << (size_t) outScp << endl;
  cout << "----------- Garbage TEST CASE 2 END -----------" << endl;
}

int main()
{

  cout << "-----------" << endl;
  cout << "----------- MAIN PROGAM -----------" << endl;
  testCaseGarbageOne();
  testGarbageFour();
  testCaseGarbageFive();
  cout<< "----------- TEST GLOBALS CASE START -----------"<<endl;
  testGlobalsStillPresent();
  Header *h = (Header *)malloc(sizeof(Header));

  if(true){
    // functionTwo(h);
    char * t2 = (char *) malloc(maxNextGC);
    gc();
    // char * t3 = (char *) malloc(maxNextGC+1); //Trigger GC
    cout<< "Address of T2 should be equal to Address of T4  :  :-)" << (size_t)t2 << endl;
    // GC Would be triggered, now need to make sure the old one is getting added to freed objects;  
  }

  cout<< "----------- TEST GLOBALS CASE END -----------"<<endl;

  cout<< "----------- TEST CASE RECURSION -----------"<<endl;
  testCaseRecursion();  
  cout<< "----------- TEST CASE RECURSION -----------"<<endl;
  cout<< "----------- GLOBAL CASE 2 -----------"<<endl;
  testCaseGlobal();
  gc();
  cout<<"No object collected"<<endl;
  cout<< "----------- GLOBAL CASE 2 END -----------"<<endl;
  // testCaseLivenessOne();
  // testCaseLivenessTwo();
  // int ** p1 = (int **) malloc(8);
  // int * p2 = (int *) malloc(8);
  // Header *h1 = (Header *)malloc(sizeof(Header));

  // char * q = (char *) malloc(256);
  // strcpy(q, "This should be intact.\n");
  // if(true){
  //   void * p;
  //   void * ptr;
  //   Header *h;
  //   cout<< "p1 address = " << (size_t) &p1 << ", p2 address = " << (size_t) &p2 << endl;
  //   // TRIGGER GC CONDITION HAPPENS
  //   char * q2 = (char *) malloc(128);
  //   p = p1;
  //   ptr = static_cast<char*>(p) - sizeof(Header);
  //   h = static_cast<Header*>(ptr);
  //   cout<< "Header h for the pointer p1 " << (size_t) &p1 << " is " <<h->isMarked()<< " and cookie is " << h->validateCookie() <<endl;
    
  //   p = p2;
  //   ptr = static_cast<char*>(p) - sizeof(Header);
  //   h = static_cast<Header*>(ptr);
  //   cout<< "Header h for the pointer p2 " << (size_t) &p2 << " is " <<h->isMarked()<< " and cookie is " << h->validateCookie() <<endl;

  //   p = q; 
  //   ptr = static_cast<char*>(p) - sizeof(Header);
  //   h = static_cast<Header*>(ptr);
  //   cout<< "Header h for the pointer q " << (size_t) &p1 << " is " <<h->isMarked()<< " and cookie is " << h->validateCookie() <<endl;

  //   cout<< " Old Variable with requirements(16) Address : " << (size_t) q2 <<endl;

  // }
  // if(true){
  //   functionOne();
  //   functionTwo(h1);

  //   char * t2 = (char *) malloc(2000);
  //   cout << "Header for h is present in " << (size_t)h1 << endl;
  //   // GC Would be triggered, now need to make sure the old one is getting added to freed objects;  
  // }
  
  return 0;
}
