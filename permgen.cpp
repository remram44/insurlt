#include "permgen.h"


//constexpr Key M = 1 << 32;
constexpr Key A = 1103515245;
constexpr Key C = 12345;

Generator::Generator(Key state, Key max)
  : m_State(state), m_Max(max)
{
}

Key Generator::generate()
{
    do
        m_State = (A * m_State + C) & 0xFFFFFFFF;
    while(m_State > m_Max);
    return m_State;
}

void Generator::setState(Key state)
{
    m_State = state;
}

Key Generator::state() const
{
    return m_State;
}
