#ifndef __BUILTIN_TYPE_H__
#define __BUILTIN_TYPE_H__

#include <src/type.h>
#include <src/object/object.hpp>

// len(); required 1 argument and type must be List , Map , String
SPtr<Object> builtinLen(const Array<SPtr<Object>>& args);

// keys(); required 1 argument and type must be Map
SPtr<Object> builtinKeys(const Array<SPtr<Object>>& args);

// values(); required 1 argument and type must be Map
SPtr<Object> builtinValues(const Array<SPtr<Object>>& args);

// type(); required 1 argument and type must be any
SPtr<Object> builtinType(const Array<SPtr<Object>>& args);



#endif