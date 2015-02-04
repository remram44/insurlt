#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

#include "insults.h"


class Insults_test : public CppUnit::TestFixture {

public:
    void test_insults()
    {
        Insults insults(0);
        for(size_t i = 0; i < 10; ++i)
        {
            std::string insult = insults.generate();
            std::cerr << insult << std::endl;
            CPPUNIT_ASSERT(insult.size() > 60 &&
                           insult.size() < 200);
        }
    }

    CPPUNIT_TEST_SUITE(Insults_test);
    CPPUNIT_TEST(test_insults);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(Insults_test);
