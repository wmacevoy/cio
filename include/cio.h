#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  struct CIOStruct;
  typedef struct CIOStruct CIO;
  typedef int (*CIOReadPtr)(CIO *me);
  typedef int (*CIOWritePtr)(CIO *me,int value);
  typedef int (*CIOPeekPtr)(CIO *me,int offset);
  typedef void (*CIOClosePtr)(CIO *me);
  struct CIOStruct {
    CIOReadPtr read;
    CIOWritePtr write;
    CIOPeekPtr peek;
    CIOClosePtr close;
    int reads;
    int writes;
  };
  void CIOInit(CIO *me);

  int CIORead(void *me);

  int CIOWrite(void *me, int data);

  int CIOPeek(void *me, int offset);

  void CIOClose(void *me);

  int CIOGetReads(void *me);

  int CIOGetWrites(void *me);

  struct CIOArrayStruct;
  typedef struct CIOArrayStruct CIOArray;

  typedef int (*CIOArrayElementReadPtr)(void *location);
  typedef void (*CIOArrayElementWritePtr)(void *location, int value);

  struct CIOArrayStruct {
    CIO base;
    CIOArrayElementReadPtr elementRead;
    CIOArrayElementWritePtr elementWrite;
    int elementSize;
    void *data;
    int position;
    int size;
    int capacity;
    int maxCapacity;
  };

  void CIOArrayInit(CIOArray *me,
		    CIOArrayElementReadPtr elementRead,
		    CIOArrayElementWritePtr elementWrite,		    
		    int elementSize,
		    void *data,
		    int size,
		    int capacity,
		    int maxCapacity);

  void CIOArrayU8Init(CIOArray *me,
		    uint8_t *data,
		    int size,
		    int capacity,
		    int maxCapacity);
  
  void CIOArrayU16Init(CIOArray *me,
		    uint16_t *data,
		    int size,
		    int capacity,
		    int maxCapacity);
  
  void CIOArrayU32Init(CIOArray *me,
		    uint32_t *data,
		    int size,
		    int capacity,
		    int maxCapacity);
  

#ifdef __cplusplus
} // extern "C"
#endif  
