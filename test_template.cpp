#include <cppunit/extensions/HelperMacros.h>
#include <sstream>
#include <utility>
#include <unordered_map>

#define private public
#include "template.h"


class Template_test : public CppUnit::TestFixture {

public:
    void test_compile()
    {
        std::istringstream stream("Th{{end  }} {{ verb }}: {{\n what}}!");
        try {
            Template tpl(stream);
            std::vector<Template::Block> blocks{
                Template::Block(Template::DATA, "Th"),
                Template::Block(Template::VARIABLE, "end"),
                Template::Block(Template::DATA, " "),
                Template::Block(Template::VARIABLE, "verb"),
                Template::Block(Template::DATA, ": "),
                Template::Block(Template::VARIABLE, "what"),
                Template::Block(Template::DATA, "!"),
            };
            CPPUNIT_ASSERT(tpl.m_Blocks == blocks);
        }
        catch(TemplateError &e)
        {
            CPPUNIT_FAIL((std::string("Got exception: ") + e.what()).c_str());
        }
    }

    void test_render()
    {
        std::istringstream stream("Th{{end  }} {{ verb }}: {{  what}}!");
        try {
            Template tpl(stream);
            typedef std::pair<std::string, std::string> Item;

            std::ostringstream oss;
            tpl.render(
                    oss,
                    std::unordered_map<std::string, std::string>{
                        Item("end", "is"),
                        Item("verb", "is"),
                        Item("what", "a house")
                    });
            CPPUNIT_ASSERT(oss.str() == "This is: a house!");

            oss.str("");
            tpl.render(
                    oss,
                    {
                        "end", "ese",
                        "verb", "are",
                        "what", "trees"
                    });
            CPPUNIT_ASSERT(oss.str() == "These are: trees!");
        }
        catch(TemplateError &e)
        {
            CPPUNIT_FAIL((std::string("Got exception: ") + e.what()).c_str());
        }
    }

    void do_error_test(const std::string &expr, bool should_fail)
    {
        std::istringstream stream(expr);
        if(should_fail)
            CPPUNIT_ASSERT_THROW(Template a(stream), TemplateError);
        else
            CPPUNIT_ASSERT_NO_THROW(Template a(stream));
    }

    void test_errors()
    {
        do_error_test("This{", true);
        do_error_test("This{{", true);
        do_error_test("This{ {is}}", true);
        do_error_test("This{{is} }", true);
        do_error_test("This{{is}}", false);
        do_error_test("This{{is}", true);
        do_error_test("This{{is", true);
    }

    void test_missing_var()
    {
        try {
            typedef std::pair<std::string, std::string> Item;

            std::istringstream stream("Ceci {{is}} un {{test}}");
            Template tpl(stream);
            std::ostringstream oss;
            tpl.render(
                    oss,
                    std::unordered_map<std::string, std::string>{
                        Item("is", "est"),
                        Item("test", "essai")
                    });
            CPPUNIT_ASSERT_THROW(
                tpl.render(
                        oss,
                        std::unordered_map<std::string, std::string>{
                            Item("is", "est")
                        }),
                TemplateError);
        }
        catch(TemplateError &e)
        {
            CPPUNIT_FAIL((std::string("Got exception: ") + e.what()).c_str());
        }
    }

    CPPUNIT_TEST_SUITE(Template_test);
    CPPUNIT_TEST(test_compile);
    CPPUNIT_TEST(test_render);
    CPPUNIT_TEST(test_errors);
    CPPUNIT_TEST(test_missing_var);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(Template_test);
