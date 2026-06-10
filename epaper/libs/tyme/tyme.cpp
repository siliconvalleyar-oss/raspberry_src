//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   tyme.cpp
//          License             :   GNU 
//          Author              :   Lio
//          Change History      :
//          Processor           :   ARM
//          Hardware            :		
//          Complier            :   ARM
//          Company             :
//          Dependencies        :
//          Description         :
//          brief               :	
//
//////////////////////////////////////////////////////////////////////////////


#include <chrono>
#include <thread>
#include <tyme/tyme.h>


namespace TYME{
    void delay(int ms){
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));  
    }
    void delay_us(int us){
        std::this_thread::sleep_for(std::chrono::microseconds (us)); 
    }
    void delay_ms(int ms){
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));  
    }
    void delay_sec   ( int sec){
        std::this_thread::sleep_for(std::chrono::seconds(sec));  
    }
}