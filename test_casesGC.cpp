#include <iostream>
#include <cstring>
#include <assert.h>
using namespace std;


#include "gcmalloc.hh"
extern "C"
{
  //  void * xxmalloc(size_t);
  //  void xxfree(void *);
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



int testme2;
static const auto maxNextGC = 10000;
int * testme = (int *)malloc(maxNextGC);

void testGlobalsStillPresent(){
  cout << "--------------------GLOBALS TEST CASE ------------" << endl;
  int * t = testme;
  cout<< (size_t)testme <<endl;
  *t = 5000;
  assert(*testme == 5000);
  char * t4 = (char *) malloc(maxNextGC); //Trigger GC
  // char * t3 = (char *) malloc(maxNextGC+1); //Trigger GC
  // char * t4 = (char *) malloc(maxNextGC+1); //Trigger GC
  cout<< " Address of t4 " << t4 << endl;
  // strcpy(t2, "This should be intact.\n");
  // strcpy(t3, "This should be intact.\n");
  strcpy(t4, "This should be intact.\n");
  void * p;
  void * ptr;
  Header *h;
  ptr = ((char *)((void *)testme)) - sizeof(Header);
  cout<< (size_t)t <<endl;
  h = static_cast<Header*>(ptr);
  cout<<"COOKIE"<<endl;
  cout<< h->validateCookie()<<endl;  assert(h->isMarked()==false);
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

void testCaseOne(){
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
  // 

}


int main()
{

  cout << "-----------" << endl;
  cout << "----------- MAIN PROGAM -----------" << endl;
  testGlobalsStillPresent();
  Header *h = (Header *)malloc(sizeof(Header));

  if(true){
    // functionTwo(h);
    char * t2 = (char *) malloc(maxNextGC);
    // char * t3 = (char *) malloc(maxNextGC+1); //Trigger GC
    cout<< "Address of T2 should be equal to Address of T4  :  :-)" << (size_t)t2 << endl;
    // GC Would be triggered, now need to make sure the old one is getting added to freed objects;  
  }
  testCaseOne();

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
