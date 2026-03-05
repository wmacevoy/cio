#include <limits.h>
#include "facts.h"
#include "cio.h"
#include "utf8.h"

FACTS(CIOBase) {
  CIO io;
  CIOInit(&io);
  FACT(CIOGetReads(&io),==,0);
  FACT(CIOGetWrites(&io),==,0);  
  FACT(CIORead(&io),==,-1);
  FACT(CIOPeek(&io,0),==,-1);  
  FACT(CIOWrite(&io,0),==,0);
  FACT(CIOGetReads(&io),==,0);
  FACT(CIOGetWrites(&io),==,1);  
  FACT(CIOWrite(&io,-1),==,-1);
  FACT(CIOGetReads(&io),==,0);
  FACT(CIOGetWrites(&io),==,1);  
  CIOClose(&io);
}

FACTS(CIOArrayU8Fixed) {
  uint8_t a[5] = {99,0,0,0,99};

  CIOArray io;
  CIOArrayU8Init(&io,a+1,0,3,3,3);

  FACT(CIOWrite(&io,10),==,0);  FACT(a[1],==,10);
  FACT(CIOWrite(&io,20),==,0);  FACT(a[2],==,20);
  FACT(CIOWrite(&io,30),==,0);  FACT(a[3],==,30);
  FACT(CIOWrite(&io,40),==,-1);
  FACT(a[0],==,99); FACT(a[4],==,99);
  FACT(io.data,==,(void*)(a+1));

  CIOClose(&io);
  
}

FACTS(CIOArrayU16Dynamic) {
  uint16_t a[3];
  CIOArray io;
  int n = 1000;
  CIOArrayU16Init(&io,a,0,0,3,n);

  for (int i=0; i<=n; ++i) {
    FACT(CIOWrite(&io,10*i),==,(i < n) ? 0 : -1);
    if (i < n) {
      FACT(((uint16_t*)io.data)[i],==,10*i);
    }
  }
  
  CIOClose(&io);  
}

FACTS(CIOUTF8In) {
  char buf[1024];
  char *p=buf;
  for (int k=0; k<10; ++k) {
    for (int len=1; len<=4; ++len) {
      int c = (1<<(5*len))|(k+1);
      FACT(utf8enclen(c),==,len);
      utf8encval(p,c,len);
      p += len;
    }
  }
  *p = 0;
  CIOArray u8;
  CIOArrayU8Init(&u8,(uint8_t*)buf,0,p-buf,p-buf,p-buf);
  CIOUTF8 utf8;
  CIOUTF8Init(&utf8,&u8.base,0);
  for (int k=0; k<10; ++k) {
    for (int len=1; len<=4; ++len) {
      int c = (1<<(5*len))|(k+1);
      FACT(CIOPeek(&utf8,k*4+(len-1)),==,c);
    }
  }
  for (int k=0; k<10; ++k) {
    for (int len=1; len<=4; ++len) {
      int c = (1<<(5*len))|(k+1);
      FACT(CIORead(&utf8),==,c);
    }
  }

  CIOClose(&utf8);
  CIOClose(&u8);
  
}

FACTS(CIOUTF8Out) {
  char buf[1024];
  char *p=buf;
  for (int k=0; k<10; ++k) {
    for (int len=1; len<=4; ++len) {
      int c = (1<<(5*len))|(k+1);
      FACT(utf8enclen(c),==,len);
      utf8encval(p,c,len);
      p += len;
    }
  }
  *p = 0;

  CIOArray u8;
  CIOArrayU8Init(&u8,NULL,0,0,0,INT_MAX);
  CIOUTF8 utf8;
  CIOUTF8Init(&utf8,&u8.base,0);

  for (int k=0; k<10; ++k) {
    for (int len=1; len<=4; ++len) {
      int c = (1<<(5*len))|(k+1);
      FACT(CIOWrite(&utf8,c),==,0);
    }
  }
  FACT(CIOWrite(&utf8,0),==,0);

  FACT(strcmp(buf,(char*)u8.data),==,0);

  CIOClose(&utf8);
  CIOClose(&u8);
}

FACTS(CIOUTF8Close) {
  char buf[1024];
  char *p=buf;
  for (int k=0; k<10; ++k) {
    for (int len=1; len<=4; ++len) {
      int c = (1<<(5*len))|(k+1);
      utf8encval(p,c,len);
      p += len;
    }
  }
  CIOArray u8;
  CIOArrayU8Init(&u8,(uint8_t*)buf,0,p-buf,p-buf,p-buf);
  CIOUTF8 utf8;
  CIOUTF8Init(&utf8,&u8.base,1);
  for (int k=0; k<10; ++k) {
    for (int len=1; len<=4; ++len) {
      int c = (1<<(5*len))|(k+1);
      FACT(CIORead(&utf8),==,c);
    }
  }
  FACT(CIORead(&utf8),==,-1);
  CIOClose(&utf8);
  // u8 was closed by utf8 since close=1; verify position was reset
  FACT(u8.position,==,0);
  FACT(u8.size,==,0);
}

FACTS(CIOFILEIn) {
  FILE *tmp = tmpfile();
  fprintf(tmp,"%s","abcdefghijklmnopqrstuvwxyz");
  fseek(tmp,0,SEEK_SET);
  CIOFILE ci;
  CIOFILEInit(&ci,tmp,0);
  for (int i='a'; i<='z'; ++i) {
    FACT(CIOPeek(&ci,i-'a'),==,i);
  }
  for (int i='a'; i<='z'; ++i) {
    FACT(CIORead(&ci),==,i);
  }
  CIOClose(&ci);
}

FACTS(CIOFILEOut) {
  FILE *tmp = tmpfile();
  CIOFILE co;
  CIOFILEInit(&co,tmp,0);

  for (int i='a'; i<='z'; ++i) {
    FACT(CIOWrite(&co,i),==,0);
  }
  fseek(tmp,0,SEEK_SET);
  for (int i='a'; i<='z'; ++i) {
    FACT(fgetc(tmp),==,i);
  }
  CIOClose(&co);
}

FACTS(CIOArrayU8Read) {
  uint8_t data[] = {10, 20, 30, 40, 50};
  CIOArray io;
  CIOArrayU8Init(&io, data, 0, 5, 5, 5);
  // Peek without consuming
  for (int i = 0; i < 5; ++i) {
    FACT(CIOPeek(&io, i), ==, (i+1)*10);
  }
  FACT(CIOPeek(&io, 5), ==, -1);
  // Read consumes
  for (int i = 0; i < 5; ++i) {
    FACT(CIORead(&io), ==, (i+1)*10);
  }
  FACT(CIORead(&io), ==, -1);
  FACT(CIOGetReads(&io), ==, 5);
  CIOClose(&io);
}

FACTS(CIOArrayU32RoundTrip) {
  CIOArray io;
  CIOArrayU32Init(&io, NULL, 0, 0, 0, 100);
  for (int i = 0; i < 50; ++i) {
    FACT(CIOWrite(&io, 1000 + i), ==, 0);
  }
  FACT(CIOGetWrites(&io), ==, 50);
  FACT(CIOArrayGetSize(&io), ==, 50);
  // Reset position for reading
  io.position = 0;
  for (int i = 0; i < 50; ++i) {
    FACT(CIORead(&io), ==, 1000 + i);
  }
  FACT(CIORead(&io), ==, -1);
  CIOClose(&io);
}

FACTS(CIOErrorPaths) {
  // Writing to a full fixed array
  uint8_t a[2] = {0, 0};
  CIOArray fixed;
  CIOArrayU8Init(&fixed, a, 0, 0, 2, 2);
  FACT(CIOWrite(&fixed, 1), ==, 0);
  FACT(CIOWrite(&fixed, 2), ==, 0);
  FACT(CIOWrite(&fixed, 3), ==, -1);  // position 2 >= capacity 2, can't grow
  CIOClose(&fixed);

  // Reading past EOF on CIOFILE
  FILE *tmp = tmpfile();
  fprintf(tmp, "AB");
  fseek(tmp, 0, SEEK_SET);
  CIOFILE ci;
  CIOFILEInit(&ci, tmp, 0);
  FACT(CIORead(&ci), ==, 'A');
  FACT(CIORead(&ci), ==, 'B');
  FACT(CIORead(&ci), ==, -1);
  FACT(CIOPeek(&ci, 0), ==, -1);
  CIOClose(&ci);
  fclose(tmp);

  // Double close safety
  CIOArray dc;
  CIOArrayU8Init(&dc, NULL, 0, 0, 0, 10);
  CIOWrite(&dc, 42);
  CIOClose(&dc);
  CIOClose(&dc);  // should not crash

  // CIOWrite with -1 returns error, does not close
  CIO io;
  CIOInit(&io);
  FACT(CIOWrite(&io, -1), ==, -1);
  FACT(CIOGetWrites(&io), ==, 0);
}

FACTS_REGISTER_ALL() {
    FACTS_REGISTER(CIOBase);
    FACTS_REGISTER(CIOArrayU8Fixed);
    FACTS_REGISTER(CIOArrayU16Dynamic);
    FACTS_REGISTER(CIOUTF8In);
    FACTS_REGISTER(CIOUTF8Out);
    FACTS_REGISTER(CIOUTF8Close);
    FACTS_REGISTER(CIOFILEIn);
    FACTS_REGISTER(CIOFILEOut);
    FACTS_REGISTER(CIOArrayU8Read);
    FACTS_REGISTER(CIOArrayU32RoundTrip);
    FACTS_REGISTER(CIOErrorPaths);
}

FACTS_MAIN
