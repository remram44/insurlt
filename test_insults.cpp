#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

#include "insults.h"


class Insults_test : public CppUnit::TestFixture {

private:
    static constexpr size_t NB_GENERATE = 10;
    static const char *expected_insults[NB_GENERATE];

public:
    void test_insults()
    {
        Insults insults(0);
        for(size_t i = 0; i < NB_GENERATE; ++i)
        {
            std::string insult = insults.generate();
            CPPUNIT_ASSERT(insult == expected_insults[i]);
        }
    }

    CPPUNIT_TEST_SUITE(Insults_test);
    CPPUNIT_TEST(test_insults);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(Insults_test);

const char *Insults_test::expected_insults[NB_GENERATE] = {
    "vas-y.clique-sale.fils-de-pute-ou.va.crever.ta.fille."
        "au.cimetiere.raclure.de.chiottes.allez.clique-salope.ovh",
    "vas-y-clique.fils.de-chienne.ou-va.defoncer.l-anus-de.ta-soeur-"
        "sous-ma.caisse.petite-catin-allez.clique-salope.ovh",
    "vas-y-clique-grosse.merde.ou.va.defoncer.l-anus-de.ton.grand.pere-"
        "sous-ma.caisse.pauvre-merde-allez.clique-salope.ovh",
    "vas-y.clique.sale-encule.ou-va-trouer-le-cul.de.ta-grand-mere."
        "dans-les-champs-petite-catin-allez.clique-salope.ovh",
    "vas-y.clique-gros.con.ou.va-trouer.le.cul-de.ta-grand-mere."
        "chez.le-coiffeur.raclure-de-chiottes.allez.clique-salope.ovh",
    "vas-y-clique.sale.encule.ou-va-trouer.le-cul.de-ton.grand.pere-"
        "dans.les-champs.grosse-pute-allez.clique-salope.ovh",
    "vas-y-clique-sale.fils-de-pute-ou.va.gifler.ta.soeur-"
        "a.carrefour-pauvre-merde.allez.clique-salope.ovh",
    "vas-y.clique.salope-ou.va.trouer-le.cul.de.ton.grand.pere-"
        "sous-ma.caisse-sale-enflure-allez.clique-salope.ovh",
    "vas-y.clique-pauvre-con.ou.va.defoncer.l-anus.de.ton-grand.pere-"
        "dans.ma-cave.fils.de-chien-allez.clique-salope.ovh",
    "vas-y-clique.sale-fils-de-pute-ou-va-trouer-le-cul.de.ta.grand-mere."
        "dans-les-champs.face-de-pet.allez.clique-salope.ovh"
};
