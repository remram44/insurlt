#include <cassert>

#include "insults.h"


Chooser::Chooser(std::initializer_list<const char*> choices)
{
    for(const char *orig : choices)
        gen_choices(orig, 0, "");
}

void Chooser::gen_choices(const std::string &orig, size_t index,
                          const std::string &prefix)
{
    size_t pos = orig.find(' ', index);
    if(pos == std::string::npos)
        m_Choices.push_back(prefix + orig.substr(index));
    else
    {
        gen_choices(orig, pos + 1,
                    prefix + orig.substr(index, pos - index) + ".");
        gen_choices(orig, pos + 1,
                    prefix + orig.substr(index, pos - index) + "-");
    }
}

const std::string &Chooser::operator()(Key &key) const
{
    size_t p = key % m_Choices.size();
    key /= m_Choices.size();
    return m_Choices[p];
}

size_t Chooser::size() const
{
    return m_Choices.size();
}


CombinedChoosers::CombinedChoosers(std::initializer_list<Chooser> choosers)
  : m_Choosers(choosers)
{
}

std::string CombinedChoosers::operator()(Key &key) const
{
    std::string result;
    for(const Chooser &ch : m_Choosers)
        result += ch(key);
    return result;
}

size_t CombinedChoosers::size() const
{
    size_t choices = 1;
    for(const Chooser &ch : m_Choosers)
        choices *= ch.size();
    return choices;
}


Insults::Insults(Key state)
  : m_Generator(state, CHOICES),
    m_Choosers{
        Chooser{"vas-y clique "},
        Chooser{"salope",
                "gros con",
                "pauvre con",
                "sale fils de pute",
                "petite bite",
                "fils de chienne",
                "grosse merde",
                "vieux chacal",
                "sale encule",
                "gros batard"},
        Chooser{" ou "},
        Chooser{"va trouer le cul de",
                "va gifler",
                "va defoncer l-anus de",
                "va tuer",
                "va crever",
                "va cracher sur"},
        Chooser{" "},
        Chooser{"ta fille",
                "ta grand mere",
                "ton grand pere",
                "ton chien",
                "ta femme",
                "ta chienne",
                "ta soeur",
                "ton frere"},
        Chooser{" "},
        Chooser{"au cimetiere",
                "chez le coiffeur",
                "dans ma cave",
                "dans la cave",
                "sous ma caisse",
                "dans les champs",
                "dans ton cul",
                "dans la niche",
                "a carrefour"},
        Chooser{" "},
        Chooser{"raclure de chiottes",
                "face de pet",
                "sous merde",
                "sale batard",
                "pauvre merde",
                "fils de chien",
                "grosse pute",
                "sale enflure",
                "petite catin"},
        Chooser{" allez.clique-salope.ovh"}}
{
    assert(m_Choosers.size() == CHOICES);
}

std::string Insults::generate()
{
    Key key = m_Generator.generate();
    return m_Choosers(key);
}

Key Insults::state() const
{
    return m_Generator.state();
}
