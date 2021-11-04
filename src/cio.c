#include <stdlib.h>
#include <string.h>

#include "cio.h"

int CIORead(void *me) {
  int status = ((CIO*)me)->read((CIO*)me);
  if (status != -1) { ++(((CIO*)me)->reads); }
  return status;
}

int CIOWrite(void *me, int data) {
  int status = -1;
  if (data != -1) {
    status = ((CIO*)me)->write((CIO*)me,data);
    if (status != -1) { ++(((CIO*)me)->writes); }
  } else {
    ((CIO*)me)->close((CIO*)me);
  }
  return status;
}

int CIOPeek(void *me, int offset) {
  return ((CIO*)me)->peek((CIO*)me,offset);
}

void CIOClose(void *me) {
  ((CIO*)me)->close((CIO*)me);
}

int CIOGetReads(void *me) { ((CIO*)me)->reads; }

int CIOGetWrites(void *me) { ((CIO*)me)->writes; }  

static int CIOBaseRead(CIO *me) { return -1; }
static int CIOBaseWrite(CIO *me, int data) { return 0; }
static int CIOBasePeek(CIO *me, int offset) { return -1; }
static void CIOBaseClose(CIO *me) {}



void CIOInit(CIO *me) {
  me->read = &CIOBaseRead;
  me->write = &CIOBaseWrite;
  me->peek = &CIOBasePeek;
  me->close = &CIOBaseClose;
  me->reads = 0;
  me->writes = 0;
}

static int CIOArrayPeek(CIOArray *me, int offset) {
  int position = me->position+offset;
  if (0 <= position && position < me->size) {
    unsigned char *location = ((unsigned char*) me->data) + position*me->elementSize;
    int value = me->elementRead(location);
    return value;
  } else {
    return -1;
  }
}

static int CIOArrayRead(CIOArray *me) {
  int value = CIOArrayPeek(me,0);
  if (value != -1) {
    ++(me->position);
  }
  return value;
}

static int CIOArrayWrite(CIOArray *me, int value) {
  int position = me->position;
  if (position >= me->capacity) {
    if (position < me->maxCapacity) {
      int newCapacity = 3*position/2;
      if (newCapacity < 32) newCapacity = 32;
      if (newCapacity > me->maxCapacity) newCapacity=me->maxCapacity;
      void *newData = realloc(me->data,me->elementSize*newCapacity);
      if (newData != NULL) {
	memset(((unsigned char*)newData)+me->elementSize*me->capacity,
	       0,
	       me->elementSize*(newCapacity-me->capacity));
	me->data = newData;
	me->capacity = newCapacity;
      } else {
	return -1;
      }
    }
  }
  if (me->size <= me->position) {
    me->size = me->position+1;
  }
  void *location = ((unsigned char*) me->data)+me->position*me->elementSize;
  me->elementWrite(location,value);
  ++me->position;
  return 0;
}


static void CIOArrayClose(CIOArray *me) {
}

void CIOArrayInit(CIOArray *me,
		  CIOArrayElementReadPtr elementRead,
		  CIOArrayElementWritePtr elementWrite,
		  int elementSize,
		  void *data,
		  int size,
		  int capacity,
		  int maxCapacity) {
  CIOInit(&me->base);
  me->base.read=(CIOReadPtr)&CIOArrayRead;
  me->base.write=(CIOWritePtr)&CIOArrayWrite;
  me->base.peek=(CIOPeekPtr)&CIOArrayPeek;
  me->base.close=(CIOClosePtr)&CIOArrayClose;
  me->elementRead = elementRead;
  me->elementWrite = elementWrite;
  me->elementSize = elementSize;
  me->data=data;
  me->position=0;
  me->size=size;
  me->capacity=capacity;
  me->maxCapacity=me->maxCapacity;
}

int CIOArrayElementReadU8(void *location) {
  return *(uint8_t*)location;
}

void CIOArrayElementWriteU8(void *location, int value) {
  *(uint8_t*)location = value;
}

int CIOArrayElementReadU16(void *location) {
  return *(uint16_t*)location;
}

void CIOArrayElementWriteU16(void *location, int value) {
  *(uint16_t*)location = value;
}

int CIOArrayElementReadU32(void *location) {
  return *(uint32_t*)location;
}

void CIOArrayElementWriteU32(void *location, int value) {
  *(uint32_t*)location = value;
}

void CIOArrayU8Init(CIOArray *me,
		    uint8_t *data,
		    int size,
		    int capacity,
		    int maxCapacity) {
  CIOArrayInit(me,
	       CIOArrayElementReadU8,CIOArrayElementWriteU8,sizeof(uint8_t),
	       data,size,capacity,maxCapacity);
}
  
void CIOArrayU16Init(CIOArray *me,
		     uint16_t *data,
		     int size,
		     int capacity,
		     int maxCapacity) {
  CIOArrayInit(me,
	       CIOArrayElementReadU16,CIOArrayElementWriteU16,sizeof(uint16_t),
	       data,size,capacity,maxCapacity);
  
}

void CIOArrayU32Init(CIOArray *me,
		     uint32_t *data,
		     int size,
		     int capacity,
		     int maxCapacity) {
  CIOArrayInit(me,
	       CIOArrayElementReadU32,CIOArrayElementWriteU32,sizeof(uint32_t),
	       data,size,capacity,maxCapacity);
}


