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


void error404(std::ostream &req_out)
{
    req_out << "Status: 404 Not Found\r\n"
               "Server: insurlt\r\n"
               "Content-Type: text/html; charset=utf-8\r\n"
               "\r\n"
               "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
               "<html><head>\n"
               "<title>404 Not Found</title>\n"
               "</head><body>\n"
               "<h1>Erreur 404</h1>\n"
               "<p>Ya rien ici. <a href=\"http://clique-salope.ovh/\">"
               "Retourne \xC3\xA0 l'index, salope.</a></p>\n"
               "</body></html>\n";
}


int main()
{
    const char *db_path = getenv("DATABASE_PATH");
    if(!db_path || db_path[0] == 0)
        return 1;

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
                req_out << "Content-type: text/html\r\n"
                           "\r\n"
                           "<form method=\"post\" action=\"/\">\n"
                           "<input type=\"test\" name=\"url\"/>\n"
                           "<input type=\"submit\" value=\"Allez !\">\n"
                           "</form>\n";
            }
            else if(method == "POST" && uri == "/")
            {
                std::string their_url = get_var(content, "url");
                if(their_url.empty())
                    error404(req_out);
                else
                {
                    Key new_key = db.nextState();
                    std::string our_url = insults.generate(new_key);
                    db.storeURL(our_url, their_url);

                    req_out << "Status: 301 Moved Permanently\r\n"
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
                    error404(req_out);
                else
                {
                    std::string our_url = insults.generate(key);

                    req_out << "Content-type: text/html\r\n"
                               "\r\n"
                               "<p>URL: " << our_url << "</p>\n";
                }
            }
            else
                error404(req_out);
        }
        else
        {
            // TODO: Lookup host in database, redirect with 301
            // or 404
            error404(req_out);
        }

        // If the output streambufs had non-zero bufsizes and
        // were constructed outside of the accept loop (i.e.
        // their destructor won't be called here), they would
        // have to be flushed here.
    }

    return 0;
}
