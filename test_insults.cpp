#include <cppunit/extensions/HelperMacros.h>

#include "insults.h"


class Insults_test : public CppUnit::TestFixture {

private:
    static constexpr size_t NB_GENERATE = 10;
    static const char *expected_insults[NB_GENERATE];

public:
    void test_insults()
    {
        Generator gen(Insults::CHOICES);
        Insults insults;
        Key state = 0;
        for(size_t i = 0; i < NB_GENERATE; ++i)
        {
            state = gen.generate(state);
            std::string insult = insults.generate(state);
            CPPUNIT_ASSERT(insult == expected_insults[i]);
        }
    }

    CPPUNIT_TEST_SUITE(Insults_test);
    CPPUNIT_TEST(test_insults);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(Insults_test);

const char *Insults_test::expected_insults[NB_GENERATE] = {
    "click.the.link-asshole-or.spit.on.your.daugther."
        "in.a.cemetery.cunt.go.on.click-bitch.ovh",
    "click-the-link-scumbag-or.go.eat-your.grandmother."
        "in-a.bus-you.dipshit-come.on.click-bitch.ovh",
    "click-the.link-pig.or.spit-on-your-dog-"
        "under.a-car-idiot.go.on.click-bitch.ovh",
    "click.the-link.asshole-or-spit.on.your-dog-"
        "under.a.car-idiot-come-on.click-bitch.ovh",
    "click.the-link-scumbag.or.spit-on-your.dog-"
        "up.your-own-asshole.you.dipshit-go.on.click-bitch.ovh",
    "click.the.link.nerd-or-go.kill-your-daugther-"
        "in-a-doghouse.you-faggot-come-on.click-bitch.ovh",
    "click.the.link-bitch-or-go.fuck-your.wife."
        "under-a-car.you.slut-come.on.click-bitch.ovh",
    "click-the-link.smart-ass.or-spit-on-your-grandmother-"
        "in-a.church-stupid.go-on.click-bitch.ovh",
    "click-the-link-pig-or.go.eat-your-pet."
        "under-a-car.you-slut-come.on.click-bitch.ovh",
    "click-the.link.bitch.or.spit-on.your-pet-"
        "up-your.own-asshole-you.cunt.come.on.click-bitch.ovh",
};
