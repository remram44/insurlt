#include "permgen.h"


constexpr Key M = 0xFFFFFFFF;
constexpr Key A = 1103515245;
constexpr Key C = 12345;

Generator::Generator(Key max)
  : m_Max(max)
{
}

Key Generator::generate(Key state)
{
    do
        state = (A * state + C) & M;
    while(state > m_Max);
    return state;
}
