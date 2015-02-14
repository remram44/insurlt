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

std::string CombinedChoosers::operator()(Key key) const
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


Insults::Insults()
  : m_Choosers{
        Chooser{"click the link "},
        Chooser{"bitch",
                "fuckface",
                "bastard",
                "asshole",
                "dickhead",
                "nerd",
                "pig",
                "scumbag",
                "smart-ass",
                "motherfucker"},
        Chooser{" or "},
        Chooser{"go kill",
                "go fuck",
                "go eat",
                "spit on"},
        Chooser{" "},
        Chooser{"your sister",
                "your daugther",
                "your wife",
                "your grandmother",
                "your grandfather",
                "your dog",
                "your brother",
                "your hamster",
                "your pet",
                "your mom",
                "your dad"},
        Chooser{" "},
        Chooser{"in a cemetery",
                "in a church",
                "in a cave",
                "in your basement",
                "under a car",
                "in a field",
                "up your own asshole",
                "in a doghouse",
                "in a bus"},
        Chooser{" ", " you "},
        Chooser{"cunt",
                "fucker",
                "stupid",
                "dipshit",
                "idiot",
                "freak",
                "slut",
                "faggot"},
        Chooser{" go on", " come on"},
        Chooser{".click-bitch.ovh"}}
{
    assert(m_Choosers.size() == CHOICES);
}

std::string Insults::generate(Key state)
{
    return m_Choosers(state);
}
