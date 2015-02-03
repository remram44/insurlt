#include <cstdio>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
extern char **environ;
#endif
#include <cstdlib>
#include <fcgio.h>


// Maximum number of bytes allowed to be read from stdin
static const unsigned long REQ_IN_MAX = 1000000;


static void penv(const char *const *envp, std::ostream &req_out)
{
    req_out << "<PRE>\n";
    for(; *envp; ++envp)
        req_out << *envp << "\n";
    req_out << "</PRE>\n";
}


static long get_req(FCGX_Request *request, char **content,
                    std::istream &req_in, std::ostream &req_err)
{
    char * clenstr = FCGX_GetParam("CONTENT_LENGTH", request->envp);
    unsigned long clen = REQ_IN_MAX;

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
        if(clen > REQ_IN_MAX)
            clen = REQ_IN_MAX;

        *content = new char[clen];

        req_in.read(*content, clen);
        clen = req_in.gcount();
    }
    else
    {
        // *never* read req_in when CONTENT_LENGTH is missing or unparsable
        *content = 0;
        clen = 0;
    }

    // Chew up any remaining req_in - this shouldn't be necessary
    // but is because mod_fastcgi doesn't handle it correctly.

    // ignore() doesn't set the eof bit in some versions of glibc++
    // so use gcount() instead of eof()...
    do
        req_in.ignore(1024);
    while(req_in.gcount() == 1024);

    return clen;
}


int main()
{
    int count = 0;
    long pid = getpid();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while(FCGX_Accept_r(&request) == 0)
    {
        // Note that the default bufsize (0) will cause the use of iostream
        // methods that require positioning (such as peek(), seek(),
        // unget() and putback()) to fail (in favour of more efficient IO).
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        std::istream req_in(&cin_fcgi_streambuf);
        std::ostream req_out(&cout_fcgi_streambuf);
        std::ostream req_err(&cerr_fcgi_streambuf);

        // Although FastCGI supports writing before reading,
        // many http clients (browsers) don't support it (so
        // the connection deadlocks until a timeout expires!).
        char *content;
        unsigned long clen = get_req(&request, &content,
                                     req_in, req_err);

        req_out << "Content-type: text/html\r\n"
                   "\r\n"
                   "<TITLE>echo-cpp</TITLE>\n"
                   "<H1>insurlt</H1>\n"
                   "<H4>PID: " << pid << "</H4>\n"
                   "<H4>Request Number: " << ++count << "</H4>\n";

        req_out << "<H4>Request Environment</H4>\n";
        penv(request.envp, req_out);

        req_out << "<H4>Process/Initial Environment</H4>\n";
        penv(environ, req_out);

        req_out << "<H4>Standard Input - " << clen;
        if(clen == REQ_IN_MAX)
            req_out << " (REQ_IN_MAX)";
        req_out << " bytes</H4>\n";
        if(clen)
            req_out.write(content, clen);

        if(content)
            delete[] content;

        // If the output streambufs had non-zero bufsizes and
        // were constructed outside of the accept loop (i.e.
        // their destructor won't be called here), they would
        // have to be flushed here.
    }

    return 0;
}
