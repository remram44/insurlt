#include <cppunit/extensions/HelperMacros.h>

#include "database.h"
#include "insults.h"


class Database_test : public CppUnit::TestFixture {

public:
    void test_state()
    {
        try {
            Generator gen(Insults::CHOICES);
            Database db(":memory:", gen);
            CPPUNIT_ASSERT(db.getState() == 0);
            db.setState(42);
            CPPUNIT_ASSERT(db.getState() == 42);
            db.setState(189);
            Key s = db.nextState();
            CPPUNIT_ASSERT(s == 2405963442);
            CPPUNIT_ASSERT(db.getState() == (sqlite3_int64)s);
        }
        catch(DatabaseError &e)
        {
            CPPUNIT_FAIL((std::string("Got exception: ") + e.what()).c_str());
        }
    }

    void test_urls()
    {
        try {
            Generator gen(Insults::CHOICES);
            Database db(":memory:", gen);
            CPPUNIT_ASSERT(
                    db.resolveURL("http://aaa.clique-salope.ovh/", true) ==
                    "");
            CPPUNIT_ASSERT(
                    db.resolveURL("http://bbb.clique-salope.ovh/", false) ==
                    "");
            db.storeURL("http://aaa.clique-salope.ovh/",
                        "http://aaa.net/");
            CPPUNIT_ASSERT(
                    db.resolveURL("http://aaa.clique-salope.ovh/", false) ==
                    "http://aaa.net/");
            CPPUNIT_ASSERT(
                    db.resolveURL("http://bbb.clique-salope.ovh/", false) ==
                    "");
            CPPUNIT_ASSERT(db.getViews("http://aaa.clique-salope.ovh/") == 0);
            CPPUNIT_ASSERT(
                    db.resolveURL("http://aaa.clique-salope.ovh/", true) ==
                    "http://aaa.net/");
            CPPUNIT_ASSERT(db.getViews("http://aaa.clique-salope.ovh/") == 1);
            CPPUNIT_ASSERT(
                    db.resolveURL("http://bbb.clique-salope.ovh/", true) ==
                    "");
            CPPUNIT_ASSERT(
                    db.resolveURL("http://aaa.clique-salope.ovh/", true) ==
                    "http://aaa.net/");
            CPPUNIT_ASSERT(db.getViews("http://aaa.clique-salope.ovh/") == 2);
        }
        catch(DatabaseError &e)
        {
            CPPUNIT_FAIL((std::string("Got exception: ") + e.what()).c_str());
        }
    }

    CPPUNIT_TEST_SUITE(Database_test);
    CPPUNIT_TEST(test_state);
    CPPUNIT_TEST(test_urls);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(Database_test);
