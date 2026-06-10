#pragma once
#include <string>

namespace BTC{

    struct Btc_t{
         Btc_t()=default;
        ~Btc_t()=default;

        
        //size_t WriteCallback(void* , size_t, size_t, std::string*);
        void function(std::string& );
        private:
    };

}