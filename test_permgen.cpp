#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

#include "permgen.h"


class PermGen_test : public CppUnit::TestFixture {

public:
    void test_generate()
    {
        Generator gen(0, 3456789012);
        std::vector<Key> keys;
        for(size_t i = 0; i < 10; ++i)
        {
            Key g = gen.generate();
            std::cerr << g << std::endl;
            CPPUNIT_ASSERT(g <= 3456789012);
            keys.push_back(g);
        }
        gen.setState(keys[4]);
        for(size_t i = 0; i < 5; ++i)
            CPPUNIT_ASSERT(gen.generate() == keys[5+i]);
    }

    CPPUNIT_TEST_SUITE(PermGen_test);
    CPPUNIT_TEST(test_generate);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(PermGen_test);
