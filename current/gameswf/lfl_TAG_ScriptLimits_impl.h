#pragma once

#include "lfl_execute_tag.h"

namespace gameswf
{
    class CTagScriptLimits: public execute_tag
    {
    private:
        Uint16 m_RecursionLimit;
        Uint16 m_TimeOutLimit;
    public:
        CTagScriptLimits();
        ~CTagScriptLimits();

        void                Read(lfl_stream* pStream);
        Uint16              TimeOutLimit() const;
        Uint16              RecursionLimit() const;
    };
}