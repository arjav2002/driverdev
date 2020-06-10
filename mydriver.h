#ifndef _MYDRIVER_H_
#define _MYDRIVER_H_

#include <ntddk.h>

#ifdef __USE_DIRECT__
#define IO_TYPE DO_DIRECT_IO
#define USE_WRITE_FUNCTION Example_WriteDirectIO
#define USE_READ_FUNCTION Example_ReadDirectIO
#endif
 
#ifdef __USE_BUFFERED__
#define IO_TYPE DO_BUFFERED_IO
#define USE_WRITE_FUNCTION Example_WriteBufferedIO
#define USE_READ_FUNCTION Example_ReadBufferedIO
#endif

#ifndef IO_TYPE
#define IO_TYPE 0
#define USE_WRITE_FUNCTION Example_WriteNeither
#define USE_READ_FUNCTION Example_ReadNeither
#endif

#endif /* _MYDRIVER_H_ */	