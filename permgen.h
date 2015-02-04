#ifndef PERMGEN_H
#define PERMGEN_H

#include <cstdint>


typedef std::uint_fast32_t Key;

/**
 * Generates a random permutation on 32-bit integers.
 *
 * Warning: max should be set to a value close to 2^32. Generation will take
 * mean time proportional to 2^32/max
 */
class Generator {

private:
    Key m_State;
    const Key m_Max;

public:
    /**
     * Constructor.
     *
     * @param state Current state of the generator.
     * @param max Maximum number to return (inclusive).
     */
    Generator(Key state, Key max);
    Key generate();

    void setState(Key state);
    Key state() const;

};

#endif
