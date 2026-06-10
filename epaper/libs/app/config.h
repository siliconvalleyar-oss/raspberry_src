//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   config.h
//          License             :   GNU 
//          Author              :   Lio
//          Change History      :
//          Processor           :   ARM 64 or 32
//          Hardware            :		
//          Complier            :   ARM
//          Company             :
//          Dependencies        :
//          Description         :   Configuration all
//          brief               :	
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

    #if (defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 4))//es de 32 bits?
        #define  CPU_32_BITS
        #else
        #define  CPU_64_BITS
    #endif

namespace CONFIG{

    #ifdef CPU_64_BITS

#endif

    #ifdef CPU_32_BITS

#endif

}