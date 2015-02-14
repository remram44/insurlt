#include <cstdio>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
extern char **environ;
#endif
#include <cstdlib>
#include <fcgio.h>

#include "database.h"
#include "insults.h"
#include "template.h"


static bool startswith(const std::string &str, const std::string &prefix)
{
    return str.compare(0, prefix.length(), prefix) == 0;
}


static int base16_decode(char e)
{
    if(e >= '0' && e <= '9')
        return (unsigned char)(e - '0');
    else if(e >= 'a' && e <= 'f')
        return (unsigned char)(e - 'a' + 10);
    else if(e >= 'A' && e <= 'F')
        return (unsigned char)(e - 'A' + 10);
    else
        return -1;
}


class URLValidator {

private:
    unsigned char m_Table[256];

public:
    static const char *const ALLOWED_CHARS;

    URLValidator();
    bool operator()(const std::string &url);

} valid_url;

const char *const URLValidator::ALLOWED_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "-._~:/?#[]@!$&'()*+,;="
    "%";

URLValidator::URLValidator()
{
    for(size_t i = 0; i <= 255; ++i)
        m_Table[i] = 0;
    for(const char *c = ALLOWED_CHARS; *c != 0; ++c)
        m_Table[(size_t)(unsigned char)*c] = 1;
}

bool URLValidator::operator()(const std::string &url)
{
    if(!startswith(url, "http://") && !startswith(url, "https://") &&
       !startswith(url, "ftp://"))
        return false;
    for(size_t i = 0; i < url.size(); ++i)
    {
        const unsigned char c = url[i];
        if(!m_Table[(size_t)c])
            return false;
    }
    return true;
}


static std::string get_var(const std::string &formdata,
                           const std::string &var)
{
    std::string buffer;
    enum EState { VARNAME, VALUE, WRONGVALUE };
    EState state = VARNAME;
    for(size_t i = 0; i < formdata.size(); ++i)
    {
        switch(state)
        {
        case VARNAME:
            if(formdata[i] == '=')
            {
                if(buffer == var)
                    state = VALUE;
                else
                    state = WRONGVALUE;
                buffer = "";
            }
            else
                buffer += formdata[i];
            break;
        case VALUE:
            if(formdata[i] == '&')
                return buffer;
            else if(formdata[i] == '+')
                buffer += ' ';
            else if(formdata[i] == '%')
            {
                if(i + 2 >= formdata.size())
                {
                    buffer += formdata.substr(i);
                    i = formdata.size();
                }
                else
                {
                    int char1 = base16_decode(formdata[i+1]);
                    int char2 = base16_decode(formdata[i+2]);
                    if(char1 < 0 || char2 < 0)
                        buffer += formdata.substr(i, 3);
                    else
                        buffer += (char)((char1 << 4) | char2);
                    i += 2;
                }
            }
            else
                buffer += formdata[i];
            break;
        case WRONGVALUE:
            if(formdata[i] == '&')
                state = VARNAME;
            break;
        }
    }
    if(state == VALUE)
        return buffer;
    return "";
}


// Maximum number of bytes allowed to be read from stdin
static const unsigned long REQ_IN_MAX = 10000;

static std::string get_req(FCGX_Request *request,
                    std::istream &req_in, std::ostream &req_err)
{
    char *clenstr = FCGX_GetParam("CONTENT_LENGTH", request->envp);
    unsigned long clen = REQ_IN_MAX;
    std::string content;

    if(clenstr)
    {
        clen = std::strtol(clenstr, &clenstr, 10);
        if(*clenstr)
        {
            req_err << "can't parse \"CONTENT_LENGTH="
                    << FCGX_GetParam("CONTENT_LENGTH", request->envp)
                    << "\"\n";
            clen = REQ_IN_MAX;
        }
        // *always* put a cap on the amount of data that will be read
        else if(clen > REQ_IN_MAX)
            clen = REQ_IN_MAX;

        content.resize(clen);

        req_in.read(&content[0], clen);
        clen = req_in.gcount();
    }
    else
    {
        // *never* read req_in when CONTENT_LENGTH is missing or unparsable
        clen = 0;
    }

    // Chew up any remaining req_in - this shouldn't be necessary
    // but is because mod_fastcgi doesn't handle it correctly.

    // ignore() doesn't set the eof bit in some versions of glibc++
    // so use gcount() instead of eof()...
    do
        req_in.ignore(1024);
    while(req_in.gcount() == 1024);

    return content;
}


int main()
{
    const char *db_path = getenv("DATABASE_PATH");
    if(!db_path || db_path[0] == 0)
        return 1;
    std::string tpl_path;
    {
        const char *tpl_path_ = getenv("TEMPLATES_PATH");
        if(!tpl_path_ || tpl_path_[0] == 0)
            return 1;
        tpl_path = tpl_path_;
    }

    Template index(tpl_path + "/index.html");
    Template error(tpl_path + "/error.html");
    Template created(tpl_path + "/created.html");

    Insults insults;
    Generator gen(insults.CHOICES);
    Database db(db_path, gen);


    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while(FCGX_Accept_r(&request) == 0)
    {
        // Note that the default bufsize (0) will cause the use of iostream
        // methods that require positioning (such as peek(), seek(),
        // unget() and putback()) to fail (in favour of more efficient IO).
        fcgi_streambuf in_fcgi_streambuf(request.in);
        fcgi_streambuf out_fcgi_streambuf(request.out);
        fcgi_streambuf err_fcgi_streambuf(request.err);

        std::istream req_in(&in_fcgi_streambuf);
        std::ostream req_out(&out_fcgi_streambuf);
        std::ostream req_err(&err_fcgi_streambuf);

        // Although FastCGI supports writing before reading,
        // many http clients (browsers) don't support it (so
        // the connection deadlocks until a timeout expires!).
        std::string content = get_req(&request, req_in, req_err);

        // Gets method and path from environment
        std::string method, uri, host;
        {
            const char *const *envp = request.envp;
            for(; *envp; ++envp)
            {
                if(startswith(*envp, "REQUEST_URI="))
                    uri = (*envp) + 12;
                else if(startswith(*envp, "REQUEST_METHOD="))
                    method = (*envp) + 15;
                else if(startswith(*envp, "HTTP_HOST="))
                    host = (*envp) + 10;
            }
        }

        if(startswith(host, "localhost") ||
           startswith(host, "127.") ||
           startswith(host, "www.clique-salope.ovh") ||
           startswith(host, "clique-salope.ovh"))
        {
            if(method == "GET" && uri == "/")
            {
                req_out << "Server: insurlt\r\n"
                           "Content-type: text/html; charset=utf-8\r\n"
                           "\r\n";
                index.render(req_out);
            }
            else if(method == "POST" && uri == "/")
            {
                std::string their_url = get_var(content, "url");
                if(their_url.empty() || !valid_url(their_url))
                {
                    req_out << "Status: 404 Not Found\r\n"
                               "Server: insurlt\r\n"
                               "Content-type: text/html; charset=utf-8\r\n"
                               "\r\n";
                    error.render(req_out, {
                            "message",
                            "T'as oubli\xC3\xA9 l'URL, face de pet ?"});
                }
                else
                {
                    Key new_key = db.nextState();
                    std::string our_url = insults.generate(new_key);
                    db.storeURL(our_url, their_url);

                    req_out << "Status: 303 See Other\r\n"
                               "Server: insurlt\r\n"
                               "Location: /created?" << new_key << "\r\n"
                               "Content-type: text/plain\r\n"
                               "\r\n"
                            << our_url << "\n";
                }
            }
            else if(startswith(uri, "/created?"))
            {
                char *endptr;
                Key key = std::strtol(uri.c_str() + 9, &endptr, 10);
                if(*endptr)
                {
                    req_out << "Status: 404 Not Found\r\n"
                               "Server: insurlt\r\n"
                               "Content-type: text/html; charset=utf-8\r\n"
                               "\r\n";
                    error.render(req_out, {
                            "message",
                            "Il manque un num\xC3\xA9ro, b\xC3\xA2tard"});
                }
                else
                {
                    std::string our_url = insults.generate(key);

                    req_out << "Server: insurlt\r\n"
                               "Content-type: text/html; charset=utf-8\r\n"
                               "\r\n";
                    created.render(req_out, {
                            "url", our_url.c_str()});
                }
            }
            else
            {
                req_out << "Status: 404 Not Found\r\n"
                           "Server: insurlt\r\n"
                           "Content-type: text/html; charset=utf-8\r\n"
                           "\r\n";
                error.render(req_out, {
                        "message",
                        "Il n'y a rien ici. T'es perdu, grosse merde ?"});
            }
        }
        else
        {
            std::string our_url = host;
            size_t end = our_url.find_first_of(":/");
            if(end != std::string::npos)
                our_url = our_url.substr(0, end);
            std::string their_url = db.resolveURL(our_url, true);
            if(their_url.empty())
            {
                req_out << "Status: 404 Not Found\r\n"
                           "Server: insurlt\r\n"
                           "Content-type: text/html; charset=utf-8\r\n"
                           "\r\n";
                error.render(req_out, {
                        "message",
                        "Il n'y a rien ici. T'es perdu, grosse merde ?"});
            }
            else
            {
                req_out << "Status: 301 Moved Permanently\r\n"
                           "Server: insurlt\r\n"
                           "Location: " << their_url << "\r\n"
                           "Content-type: text/plain\r\n"
                           "\r\n"
                        << their_url << "\n";
            }
        }

        // If the output streambufs had non-zero bufsizes and
        // were constructed outside of the accept loop (i.e.
        // their destructor won't be called here), they would
        // have to be flushed here.
    }

    return 0;
}
