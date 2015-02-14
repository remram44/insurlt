#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility> // std::pair
#include <vector>


class TemplateError : std::exception {

private:
    const std::string m_Message;

public:
    TemplateError(const std::string &msg);
    ~TemplateError() throw();
    const char *what() const throw();

};


class VariableProvider {

public:
    virtual void handle_variable(std::ostream &out,
                                 const std::string &name) = 0;

};


template<typename Map>
class MapVariableProvider : public VariableProvider {

private:
    const Map &m_Map;

public:
    MapVariableProvider(const Map &map)
      : m_Map(map)
    {
    }

    void handle_variable(std::ostream &out, const std::string &name)
    {
        try {
            out << m_Map.at(name);
        }
        catch(std::out_of_range &e)
        {
            throw TemplateError("Missing value for variable" + name);
        }
    }

};


class Template {

public:
    enum BlockType {
        DATA,
        VARIABLE
    };

    typedef std::pair<BlockType, std::string> Block;

private:
    std::vector<Block> m_Blocks;

protected:
    static bool is_space(char c);

public:
    Template(const std::string &filename);
    Template(std::istream &source, const std::string &filename = "<stream>");
    virtual ~Template();

    void compile(std::istream &source, const std::string &filename);

    template<typename Map>
    typename std::enable_if<
        !std::is_base_of<VariableProvider, Map>::value,
        void>::type
    render(std::ostream &out, const Map &map)
    {
        MapVariableProvider<Map> wrapper(map);
        render(out, wrapper);
    }
    void render(std::ostream &out, VariableProvider &vars);
    void render(std::ostream &out, std::initializer_list<const char*> vars);

};

#endif
