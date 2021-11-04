#include "facts.h"
#include "cio.h"

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
  FACT(CIOWrite(&io,-1),==,0);
  FACT(CIOGetReads(&io),==,0);
  FACT(CIOGetWrites(&io),==,1);  
  CIOClose(&io);
}

FACTS_FINISHED
FACTS_MAIN







