#include <cassert>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "template.h"


TemplateError::TemplateError(const std::string &msg)
  : m_Message(msg)
{
}

TemplateError::~TemplateError() throw()
{
}

const char *TemplateError::what() const throw()
{
    return m_Message.c_str();
}


void EmptyVariableProvider::handle_variable(std::ostream &out, const std::string &name)
{
    throw TemplateError("Missing value for variable " + name);
}


static void raise_error(const std::string &filename, int pos,
                        const std::string &error)
{
    std::ostringstream msg;
    msg << "Error in " << filename << " character " << pos
        << ": " << error;
    throw TemplateError(msg.str());
}


Template::Template(const std::string &filename)
{
    std::ifstream source(filename, std::ios::in | std::ios::binary);
    if(source.bad())
        throw TemplateError("Can't open file: " + filename);
    compile(source, filename);
}

Template::Template(std::istream &source, const std::string &filename)
{
    compile(source, filename);
}

void Template::compile(std::istream &source, const std::string &filename)
{
    std::string current;
    enum {
        NORMAL,
        OPEN_1,
        OPEN_2,
        VARNAME,
        CLOSE_1,
        CLOSE_2
    } state = NORMAL;
    std::istreambuf_iterator<char> i(source), e;
    size_t pos = 0;
    for(; i != e; ++i, ++pos)
    {
        char c = *i;
        switch(state)
        {
        case NORMAL:
            if(c == '{')
                state = OPEN_1;
            else
                current += c;
            break;
        case OPEN_1:
            if(c == '{')
                state = OPEN_2;
            else
                raise_error(filename, pos, "expecting {");
            break;
        case OPEN_2:
            if(!is_space(c))
            {
                m_Blocks.push_back(Block(DATA, current));
                current = c;
                state = VARNAME;
            }
            break;
        case VARNAME:
            if(is_space(c))
                state = CLOSE_1;
            else if(c == '}')
                state = CLOSE_2;
            else
            {
                current += c;
                break;
            }
            assert(state != VARNAME);
            m_Blocks.push_back(Block(VARIABLE, current));
            current.clear();
            break;
        case CLOSE_1:
            if(c == '}')
                state = CLOSE_2;
            else if(!is_space(c))
                raise_error(filename, pos, "expecting }");
            break;
        case CLOSE_2:
            if(c == '}')
                state = NORMAL;
            else
                raise_error(filename, pos, "expecting }");
            break;
        }
    }
    if(state != NORMAL)
        raise_error(filename, pos, "premature end of stream");
    if(!current.empty())
        m_Blocks.push_back(Block(DATA, current));
}

Template::~Template()
{
}

bool Template::is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

void Template::render(std::ostream &out, VariableProvider &vars)
{
    for(const Block &block : m_Blocks)
    {
        switch(block.first)
        {
        case DATA:
            out << block.second;
            break;
        case VARIABLE:
            vars.handle_variable(out, block.second);
            break;
        }
    }
}

void Template::render(std::ostream &out,
                      std::initializer_list<const char*> vars)
{
    std::unordered_map<std::string, std::string> map;
    assert(vars.size() % 2 == 0);
    auto iter = vars.begin();
    while(iter != vars.end())
    {
        const char *key = *iter++;
        map[key] = *iter++;
    }
    render(out, map);
}
