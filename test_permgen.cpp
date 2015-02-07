#include <cppunit/extensions/HelperMacros.h>

#include "permgen.h"


class PermGen_test : public CppUnit::TestFixture {

public:
    void test_generate()
    {
        std::vector<Key> keys;
        {
            Generator gen(3456789012);
            Key state = 0;
            for(size_t i = 0; i < 10; ++i)
            {
                state = gen.generate(state);
                CPPUNIT_ASSERT(state <= 3456789012);
                keys.push_back(state);
            }
        }
        {
            Generator gen2(3456789012);
            Key state = keys[4];
            for(size_t i = 0; i < 5; ++i)
            {
                state = gen2.generate(state);
                CPPUNIT_ASSERT(state == keys[5+i]);
            }
        }
    }

    CPPUNIT_TEST_SUITE(PermGen_test);
    CPPUNIT_TEST(test_generate);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(PermGen_test);
