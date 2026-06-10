//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   work.h
//          License             :   GNU 
//          Author              :   Lio
//          Change History      :
//          Processor           :   ARM
//          Hardware            :		
//          Complier            :   ARM
//          Company             :
//          Dependencies        :
//          Description         :   namespace Work
//          brief               :	
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


namespace  WORK{

    struct Work_t{
        //Work_t()=default;
        virtual ~Work_t() = default;
        virtual void init() = 0;
    };
    
}