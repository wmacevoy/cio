#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "utf8.h"

#include "cio.h"

int CIORead(void *me) {
  assert(me != NULL);
  int status = ((CIO*)me)->read((CIO*)me);
  if (status != -1) { ++(((CIO*)me)->reads); }
  return status;
}

int CIOWrite(void *me, int data) {
  assert(me != NULL);
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
  assert(me != NULL);
  return ((CIO*)me)->peek((CIO*)me,offset);
}

void CIOClose(void *me) {
  assert(me != NULL);
  ((CIO*)me)->close((CIO*)me);
}

int CIOGetReads(void *me) { assert(me != NULL); return ((CIO*)me)->reads; }

int CIOGetWrites(void *me) { assert(me != NULL); return ((CIO*)me)->writes; }  

static int CIOBaseRead(CIO *me) { return -1; }
static int CIOBaseWrite(CIO *me, int data) { return 0; }
static int CIOBasePeek(CIO *me, int offset) { return -1; }
static void CIOBaseClose(CIO *me) {}

void CIOInit(CIO *me) {
  assert(me != NULL);
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
      int newCapacity = (position <= INT_MAX / 3) ? 3 * position / 2 : me->maxCapacity;
      if (newCapacity < 32) newCapacity = 32;
      if (newCapacity > me->maxCapacity) newCapacity = me->maxCapacity;
      void *newData = malloc(me->elementSize*newCapacity);
      if (newData != NULL) {
	if (me->data != NULL) {
	  memcpy(newData,me->data,me->elementSize*me->size);
	  memset(me->data,0,me->elementSize*me->size);
	  if (me->allocated) {
	    free(me->data);
	  }
	}
	memset(((unsigned char*)newData)+me->elementSize*me->capacity,
	       0,
	       me->elementSize*(newCapacity-me->capacity));
	me->data = newData;
	me->capacity = newCapacity;
	me->allocated = 1;
      } else {
	return -1;
      }
    } else {
      return -1;
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
  if (me->elementWrite != NULL) {
    memset(me->data,0,me->elementSize*me->capacity);
  }
  if (me->allocated) {
    free(me->data);
    me->data = NULL;
    me->capacity = 0;
    me->allocated = 0;
  }
  me->position = 0;
  me->size = 0;
}

void CIOArrayInit(CIOArray *me,
		  CIOArrayElementReadPtr elementRead,
		  CIOArrayElementWritePtr elementWrite,
		  int elementSize,
		  void *data,
		  int allocated,
		  int size,
		  int capacity,
		  int maxCapacity) {
  assert(me != NULL);
  CIOInit(&me->base);
  me->base.read=(CIOReadPtr)&CIOArrayRead;
  me->base.write=(CIOWritePtr)&CIOArrayWrite;
  me->base.peek=(CIOPeekPtr)&CIOArrayPeek;
  me->base.close=(CIOClosePtr)&CIOArrayClose;
  me->elementRead = elementRead;
  me->elementWrite = elementWrite;
  me->elementSize = elementSize;
  me->data=data;
  me->allocated = allocated;
  me->position=0;
  me->size=size;
  me->capacity=capacity;
  me->maxCapacity=maxCapacity;
  if (me->capacity > 0 && me->data == NULL) {
    me->data = calloc(me->elementSize,me->capacity);
    if (me->data == NULL) {
      me->size = 0;
      me->capacity = 0;
    } else {
      me->allocated = 1;
    }
  }
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
		    int allocated,
		    int size,
		    int capacity,
		    int maxCapacity) {
  CIOArrayInit(me,
	       CIOArrayElementReadU8,CIOArrayElementWriteU8,sizeof(uint8_t),
	       data,allocated,size,capacity,maxCapacity);
}

void CIOArrayConstU8Init(CIOArray *me,
			 const uint8_t *data,
			 int allocated,
			 int size) {
  CIOArrayInit(me,
	       CIOArrayElementReadU8,NULL,sizeof(uint8_t),
	       (uint8_t*)data,allocated,size,size,size);
}

  
void CIOArrayU16Init(CIOArray *me,
		     uint16_t *data,
		     int allocated,
		     int size,
		     int capacity,
		     int maxCapacity) {
  CIOArrayInit(me,
	       CIOArrayElementReadU16,CIOArrayElementWriteU16,sizeof(uint16_t),
	       data,allocated,size,capacity,maxCapacity);
  
}

void CIOArrayConstU16Init(CIOArray *me,
			  const uint16_t *data,
			  int allocated,
			  int size) {
  CIOArrayInit(me,
	       CIOArrayElementReadU16,NULL,sizeof(uint16_t),
	       (uint16_t*)data,allocated,size,size,size);
  
}


void CIOArrayU32Init(CIOArray *me,
		     uint32_t *data,
		     int allocated,
		     int size,
		     int capacity,
		     int maxCapacity) {
  CIOArrayInit(me,
	       CIOArrayElementReadU32,CIOArrayElementWriteU32,sizeof(uint32_t),
	       data,allocated,size,capacity,maxCapacity);
}

void CIOArrayConstU32Init(CIOArray *me,
			  const uint32_t *data,
			  int allocated,
			  int size) {
  CIOArrayInit(me,
	       CIOArrayElementReadU32,NULL,sizeof(uint32_t),
	       (uint32_t*)data,allocated,size,size,size);
}


static int CIOFILEPeek(CIOFILE *me, int offset) {
  int idx = me->head + offset;
  while (me->buffer.size <= idx && !me->eof) {
    int c = fgetc(me->file);
    if (c == -1) {
      me->eof = 1;
      break;
    }
    if (CIOWrite(&me->buffer, c) == -1) { me->eof = 1; return -1; }
  }
  return (idx < me->buffer.size ? ((uint8_t*)me->buffer.data)[idx] : -1);
}

int CIOFILERead(CIOFILE *me) {
  int ans = CIOFILEPeek(me, 0);
  if (ans != -1) {
    ++me->head;
    if (me->head > 0 && me->head >= me->buffer.size / 2) {
      int remaining = me->buffer.size - me->head;
      uint8_t *data = (uint8_t*)me->buffer.data;
      memmove(data, data + me->head, remaining);
      me->buffer.size = remaining;
      me->buffer.position = remaining;
      me->head = 0;
    }
  }
  return ans;
}

int CIOFILEWrite(CIOFILE *me,int value) {
  if (value != -1) {
    char c=value;
    if (fwrite(&c,1,1,me->file) != 1) { return -1; }
    return 0;
  }
  return -1;
}

void CIOFILEClose(CIOFILE *me) {
  CIOClose(&me->buffer);
  if (me->close) {
    fclose(me->file);
    me->file = NULL;
  }
}

void CIOFILEInit(CIOFILE *me,FILE *file,int close) {
  assert(me != NULL);
  CIOInit(&me->base);
  me->base.read = (CIOReadPtr) &CIOFILERead;
  me->base.peek = (CIOPeekPtr) &CIOFILEPeek;
  me->base.write = (CIOWritePtr) &CIOFILEWrite;
  me->base.close = (CIOClosePtr) &CIOFILEClose;

  CIOArrayU8Init(&me->buffer,me->buffer0,0,0,sizeof(me->buffer0)/sizeof(uint8_t),INT_MAX);

  me->file=file;
  me->head=0;
  me->eof=0;
  me->close=close;
}

static int CIOUTF8Peek(CIOUTF8 *me, int offset) {
  char buf[6];
  int idx = me->head + offset;
  while (me->buffer.size <= idx) {
    int decoded = 0;
    for (int len = 1; len<=4; ++len) {
      int c = CIOPeek(me->u8,len-1);
      if (c == -1) {
	break;
      }
      buf[len-1]=c;
      int declen = utf8declen(buf,len);
      if (declen > 0) {
	for (int i=0; i<len; ++i) CIORead(me->u8);
	if (CIOWrite(&me->buffer,utf8decval(buf,declen))==-1) { return -1; }
	decoded = 1;
	break;
      }
    }
    *(uint32_t*)buf=0;
    if (!decoded) {
      return -1;
    }
  }
  return ((uint32_t*)me->buffer.data)[idx];
}

int CIOUTF8Read(CIOUTF8 *me) {
  int ans = CIOUTF8Peek(me,0);
  if (ans != -1) {
    ++me->head;
    if (me->head > 0 && me->head >= me->buffer.size / 2) {
      int remaining = me->buffer.size - me->head;
      uint32_t *data = (uint32_t*)me->buffer.data;
      memmove(data, data + me->head, remaining * sizeof(uint32_t));
      me->buffer.size = remaining;
      me->buffer.position = remaining;
      me->head = 0;
    }
  }
  return ans;
}

int CIOUTF8Write(CIOUTF8 *me, int wc) {
  char buf[4];
  int enclen=utf8enclen(wc);
  utf8encval(buf,wc,enclen);
  for (int i=0; i<enclen; ++i) {
    if (CIOWrite(me->u8,buf[i]) == -1) { return -1; }
  }
  return 0;
}

void CIOUTF8Close(CIOUTF8 *me) {
  CIOClose(&me->buffer);
  if (me->close) {
    CIOClose(me->u8);
  }
}

void CIOUTF8Init(CIOUTF8 *me, CIO *u8, int close) {
  assert(me != NULL);
  CIOInit(&me->base);
  CIOArrayU32Init(&me->buffer,me->buffer0,0,0,sizeof(me->buffer0)/sizeof(uint32_t),INT_MAX);
  me->base.read = (CIOReadPtr)&CIOUTF8Read;
  me->base.peek = (CIOPeekPtr)&CIOUTF8Peek;
  me->base.write = (CIOWritePtr)&CIOUTF8Write;
  me->base.close = (CIOClosePtr)&CIOUTF8Close;
  me->u8=u8;
  me->head=0;
  me->close=close;
}


