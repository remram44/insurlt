#ifndef INSULTS_H
#define INSULTS_H

#include <initializer_list>
#include <string>
#include <vector>

#include "permgen.h"


class Chooser {

private:
    std::vector<std::string> m_Choices;

private:
    void gen_choices(const std::string &orig, size_t index,
                     const std::string &prefix);

public:
    Chooser(std::initializer_list<const char*> choices);
    const std::string &operator()(Key &key) const;
    size_t size() const;

};


class CombinedChoosers {

private:
    std::vector<Chooser> m_Choosers;

public:
    CombinedChoosers(std::initializer_list<Chooser> choosers);
    std::string operator()(Key key) const;
    size_t size() const;

};


class Insults {

private:
    static constexpr Key CHOICES = 3609722880;

    Generator m_Generator;

    CombinedChoosers m_Choosers;

public:
    Insults();
    std::string generate(Key &state);

};

#endif
