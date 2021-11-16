#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>


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
    int allocated;
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
		    int allocated,
		    int size,
		    int capacity,
		    int maxCapacity);

  void CIOArrayU8Init(CIOArray *me,
		    uint8_t *data,
		    int allocated,		      
		    int size,
		    int capacity,
		    int maxCapacity);
  
  void CIOArrayU16Init(CIOArray *me,
		    uint16_t *data,
		    int allocated,		       
		    int size,
		    int capacity,
		    int maxCapacity);
  
  void CIOArrayU32Init(CIOArray *me,
		    uint32_t *data,
		    int allocated,		       
		    int size,
		    int capacity,
		    int maxCapacity);

  /* 
  void CIOArrayWideCharInit(CIOArray *me,
		    wchar_t *data,
		    int allocated,		       
		    int size,
		    int capacity,
		    int maxCapacity);
  */

  void CIOArrayConstInit(CIOArray *me,
		    CIOArrayElementReadPtr elementRead,
		    CIOArrayElementWritePtr elementWrite,		    
		    int elementSize,
		    void *data,
		    int allocated,
		    int size,
		    int capacity,
		    int maxCapacity);

  void CIOArrayConstU8Init(CIOArray *me,
			   const uint8_t *data,
			   int allocated,	      
			   int size);
  
  void CIOArrayConstU16Init(CIOArray *me,
			    const uint16_t *data,
			    int allocated,		       
			    int size);
  
  void CIOArrayConstU32Init(CIOArray *me,
			    const uint32_t *data,
			    int allocated,		       
			    int size);

  /*
  void CIOArrayConstWideCharInit(CIOArray *me,
				 const wchar_t *data,
				 int allocated,		       
				 int size);
  */

  struct CIOFILEStruct;
  typedef struct CIOFILEStruct CIOFILE;
  struct CIOFILEStruct {
    CIO base;
    CIOArray buffer;
    uint8_t buffer0[16];
    FILE *file;
    int eof;
    int close;
  };
  
  void CIOFILEInit(CIOFILE *me,FILE *file,int close);

  struct CIOUTF8Struct;
  typedef struct CIOUTF8Struct CIOUTF8;
  struct CIOUTF8Struct {
    CIO base;
    CIO *u8;
    CIOArray buffer;
    uint32_t buffer0[16];
  };
  void CIOUTF8Init(CIOUTF8 *me, CIO *u8);
  

#ifdef __cplusplus
} // extern "C"
#endif  
