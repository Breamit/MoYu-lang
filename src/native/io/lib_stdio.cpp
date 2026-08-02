#include "lib_stdio.h"

SPtr<Object> ioPrint(const Array<SPtr<Object>> &args) {
    u32 size = args.size();
    if(size == 0) return std::make_shared<NoneObject>();
    std::stringstream ss;
    for(const auto& arg : args) {
        ss << arg->inspect();
    }
    std::cout << ss.str() ;
    return std::make_shared<NoneObject>();
}

SPtr<Object> ioPrintln(const Array<SPtr<Object>> &args) {
    u32 size = args.size();
    if(size == 0) return std::make_shared<NoneObject>();
    std::stringstream ss;
    for(const auto& arg : args) {
        ss << arg->inspect();
    }
    std::cout << ss.str() << std::endl;
    return std::make_shared<NoneObject>();
}

SPtr<Object> ioGetStr(const Array<SPtr<Object>> &args) {
    String s;
    std::cin>>s;
    return std::make_shared<StringObject>(s);
}

SPtr<Object> ioGetLine(const Array<SPtr<Object>> &args) {
    String s;
    std::getline(std::cin, s);
    return std::make_shared<StringObject>(s);
}

SPtr<Object> ioGetFloat(const Array<SPtr<Object>> &args)  {
    f64 fl;
    std::cin>>fl;
    return std::make_shared<F64Object>(fl);
}

SPtr<Object> ioGetInt(const Array<SPtr<Object>> &args) {
    i64 i;
    std::cin>>i;
    return std::make_shared<I64Object>(i);
}
