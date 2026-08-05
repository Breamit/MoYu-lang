#ifndef __LIB_STDIO_H__
#define __LIB_STDIO_H__
#include <src/type.h>
#include <src/marco.h>
#include <iostream>
#include <sstream>
#include <src/object/object.hpp>

// print 
SPtr<Object> ioPrint(const Array<SPtr<Object>> &args);

// println
SPtr<Object> ioPrintln(const Array<SPtr<Object>> &args);

// getStr
SPtr<Object> ioGetStr(const Array<SPtr<Object>> &args);

// getLine
SPtr<Object> ioGetLine(const Array<SPtr<Object>> &args);

// getIntFloat
SPtr<Object> ioGetFloat(const Array<SPtr<Object>> &args);

// getInt
SPtr<Object> ioGetInt(const Array<SPtr<Object>> &args);
#endif