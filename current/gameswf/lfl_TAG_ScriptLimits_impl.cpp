#include "lfl_TAG_ScriptLimits_impl.h"
#include "gameswf/gameswf_stream.h"

gameswf::CTagScriptLimits::CTagScriptLimits():
m_RecursionLimit(256),
m_TimeOutLimit(17) //The default value for ScriptTimeoutSeconds varies by platform and is between 15 to 20 seconds.
{

}

gameswf::CTagScriptLimits::~CTagScriptLimits()
{

}

void gameswf::CTagScriptLimits::Read( lfl_stream* pStream )
{
     m_RecursionLimit = pStream->read_u16();
     m_TimeOutLimit = pStream->read_u16();
}

Uint16 gameswf::CTagScriptLimits::TimeOutLimit() const
{
    return m_TimeOutLimit;
}

Uint16 gameswf::CTagScriptLimits::RecursionLimit() const
{
    return m_RecursionLimit;
}
