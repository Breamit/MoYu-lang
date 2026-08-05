#ifndef __LIB_JSON_H__
#define __LIB_JSON_H__
#include <src/type.h>
#include <src/marco.h>
#include <src/object/object.hpp>

// parse
SPtr<Object> jsonParse(const Array<SPtr<Object>> &args);


// stringify
SPtr<Object> jsonStringify(const Array<SPtr<Object>> &args);

#endif