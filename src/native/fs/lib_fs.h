#ifndef __LIB_FS_H__
#define __LIB_FS_H__
#include <src/type.h>
#include <src/marco.h>
#include <src/object/object.hpp>

extern "C" {
    char* fs_call(char* request);
    void fs_free(char* ptr);
}

SPtr<Object> fsReadText(const Array<SPtr<Object>>& args);


#endif