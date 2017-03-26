#include <iostream>
#include <cstring>
using namespace std;


#include "gcmalloc.hh"
extern "C"
{
  //  void * xxmalloc(size_t);
  //  void xxfree(void *);
}

int testme;


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

void functionTwo(Header *h){
  cout << "--------------------------------------------------" << endl;
  Header *t = (Header *)malloc(sizeof(Header));
  cout<< " Local Variable Address Still to Be reached : " << (size_t) t <<endl;
  t->nextObject = nullptr;
  t->prevObject = h;
  h->nextObject = t;
}

int main()
{
  cout << (size_t) &testme << endl;
  cout << "-----------" << endl;
  int ** p1 = (int **) malloc(8);
  int * p2 = (int *) malloc(8);
  Header *h = (Header *)malloc(sizeof(Header));

  char * q = (char *) malloc(256);
  strcpy(q, "This should be intact.\n");
  if(true){
    void * p;
    void * ptr;
    Header *h;
    cout<< "p1 address = " << (size_t) &p1 << ", p2 address = " << (size_t) &p2 << endl;
    // TRIGGER GC CONDITION HAPPENS
    char * q2 = (char *) malloc(128);
    p = p1;
    ptr = static_cast<char*>(p) - sizeof(Header);
    h = static_cast<Header*>(ptr);
    cout<< "Header h for the pointer p1 " << (size_t) &p1 << " is " <<h->isMarked()<< " and cookie is " << h->validateCookie() <<endl;
    
    p = p2;
    ptr = static_cast<char*>(p) - sizeof(Header);
    h = static_cast<Header*>(ptr);
    cout<< "Header h for the pointer p2 " << (size_t) &p2 << " is " <<h->isMarked()<< " and cookie is " << h->validateCookie() <<endl;

    p = q; 
    ptr = static_cast<char*>(p) - sizeof(Header);
    h = static_cast<Header*>(ptr);
    cout<< "Header h for the pointer q " << (size_t) &p1 << " is " <<h->isMarked()<< " and cookie is " << h->validateCookie() <<endl;

    cout<< " Old Variable with requirements(16) Address : " << (size_t) q2 <<endl;

  }
  if(true){
    functionOne();
    functionTwo(h);

    char * t2 = (char *) malloc(2000);
    cout << "Header for h is present in " << (size_t)h << endl;
    // GC Would be triggered, now need to make sure the old one is getting added to freed objects;  
  }

  q = q + 4;
  functionOne();
  char * p = nullptr;
  cout << "p1 address = " << (size_t) &p1 << ", p2 address = " << (size_t) &p2 << endl;
  cout << "p address = " << (size_t) &p << endl;
  cout << "p1 should be reachable: " << (size_t) p1 << endl;
  cout << "p2 should be reachable: " << (size_t) p2 << endl;
  *p1 = p2;
  *p2 = 12;
  for (int i = 0; i < 2; i++) {
    char * ptr = (char *) malloc(256);
    cout << "ptr should not be reachable: Address" << (size_t) ptr << endl;
  }

  
  for (int i = 0; i < 200000; i++) {
    char * ptr = (char *) malloc(256);
    ptr[0] = 'X';
    if (i == 10000) {
      p = ptr;
      ptr[0] = 'Y';
    }
    cout << i << " -- " << (size_t) ptr << endl;
  }
  cout << **p1 << endl; // should be 12
  cout << p[0] << endl; // should be Y
  cout << (char *) (q - 4) << endl;
  return 0;
}
