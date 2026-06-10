#pragma once
#include <string>


namespace IP_GLOBAL{

    struct IpGlobal_t
    {
        IpGlobal_t()=default;
        ~IpGlobal_t()=default;
        int global();
        std::string obtenerIPGlobal() ;
    };

    
}