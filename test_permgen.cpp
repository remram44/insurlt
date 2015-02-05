#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

#include "permgen.h"


class PermGen_test : public CppUnit::TestFixture {

public:
    void test_generate()
    {
        std::vector<Key> keys;
        {
            Generator gen(0, 3456789012);
            for(size_t i = 0; i < 10; ++i)
            {
                Key g = gen.generate();
                CPPUNIT_ASSERT(g <= 3456789012);
                keys.push_back(g);
            }
        }
        {
            Generator gen2(keys[4], 3456789012);
            for(size_t i = 0; i < 5; ++i)
                CPPUNIT_ASSERT(gen2.generate() == keys[5+i]);
        }
    }

    CPPUNIT_TEST_SUITE(PermGen_test);
    CPPUNIT_TEST(test_generate);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(PermGen_test);
