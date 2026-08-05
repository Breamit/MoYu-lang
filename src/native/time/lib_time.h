#ifndef __LIB_TIME_H__
#define __LIB_TIME_H__
#include <src/type.h>
#include <src/object/object.hpp>
#include <src/marco.h>

SPtr<Object> timeNow(const Array<SPtr<Object>> &args);

SPtr<Object> timeNowf(const Array<SPtr<Object>> &args);



#endif