/**
 * which.cpp - search the PATH environment variable for programs.
 * a portable C++ implementation for Microsoft Windows and GNU/Linux.
 *
 * free to distribute under the GPL license.
 * if you have not received a copy of the license along with the code,
 * confer to http://www.gnu.org/licenses/gpl.html
 *
 * (C) Copyright 2009, 2010, Ji Han (jihan917<at>yahoo<dot>com).
 */


#include <sys/stat.h>
#include <cstring>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include <vector>


#if defined(_MSC_VER)
#   define strtok_r strtok_s
#elif defined(__MINGW32__)

    /**
     * strtok_r - split string into tokens
     * (C) Copyright 2009, 2010, Ji Han.
     * see http://www.opengroup.org/onlinepubs/000095399/functions/strtok.html
     * for the specification.
     */
    char* strtok_r(char* s, const char* sep, char** lasts)
    {
        if (s == NULL) s = *lasts;  // on subsequent calls, recover context first.
        while (*s && strchr(sep, *s)) ++s;  // skip any leading separators.
        if (*s == '\0')  // there's no more token.
        {
            *lasts = s;  // save context for subsequent calls.
            return NULL;
        }

        char* ret = s;  // return the beginning of token.
        while (!strchr(sep, *s)) ++s; // locate the end of token.
        if (*s) *s++ = '\0';  // if this is not the last token, terminate with null byte.
        *lasts = s;  // save context for subsequent calls.
        return ret;
    }

#endif

#if defined(_WIN32)
    const static std::string Sep("\\");
    const static std::string PathSep(";");
#else
    const static std::string Sep("/");
    const static std::string PathSep(":");
#endif


static int id(int x)
{
    return x;
}

const static std::pointer_to_unary_function<int, int> Id(id);


struct FileExists
    : std::unary_function<const std::string&, bool>
{
    bool operator()(const std::string& filename) const
    {
        struct stat buf;
        return (!stat(filename.c_str(), &buf));
    }
};


/**
 * get content of environment variable named `var',
 * transform the string with `transformer' (optional),
 * and split to a list of strings by `delims'.
 */
std::list<std::string>
splitStringGetEnv(const std::string& var,
                  const std::string& delims,
                  std::pointer_to_unary_function<int, int> transformer = Id)
{
    std::list<std::string> ret;

    const char* lpVar = getenv(var.c_str());
    if (lpVar == NULL) return ret;

    std::string strVar(lpVar);
    if (strVar.empty()) return ret;

    transform(strVar.begin(), strVar.end(), strVar.begin(), transformer);

    std::vector<char> buffer;
    buffer.reserve(strVar.size());
    std::copy(strVar.begin(), strVar.end(), back_inserter(buffer));

    char* token = NULL;
    const char* IFS = delims.c_str();
    char* context = NULL;

    token = strtok_r(&buffer.front(), IFS, &context);
    while (token != NULL)
    {
        ret.push_back(std::string(token));
        token = strtok_r(NULL, IFS, &context);
    }

    return ret;
}


int main (int argc, char** argv)
{
    if (*(argv + 1) == NULL) return EXIT_FAILURE;

    bool toShowAllMatches = false;
    for (int i = 1; *(argv + i) && (**(argv + i) == '-'); ++i)
    {
        if (!strcmp(*(argv + i), "--")) break;

        if (!strcmp(*(argv + i), "-a"))
        {
            toShowAllMatches = true;
            continue;
        }

        // other options (if any) follow.

        std::cerr << "Error: invalid option '" << *(argv + i) << "'.\n"
                     "Usage: " << *argv << " [-a] args...\n";
        return EXIT_FAILURE;
    }

    std::list<std::string> path = splitStringGetEnv("PATH", PathSep);

#if defined(_WIN32)
    std::list<std::string> pathext =
        splitStringGetEnv("PATHEXT", PathSep, std::ptr_fun(::tolower));
#endif

    for (int i = 1; i < argc; ++i)
    {
        std::string filename(*(argv + i));

#if defined(_WIN32)
        // expand filename by PATHEXT (.EXE, .BAT, .VBS, .JS, etc.)
        std::list<std::string> filenamesWithExt(pathext);
        filenamesWithExt.insert(filenamesWithExt.begin(), std::string(""));

        for (std::list<std::string>::iterator iter = filenamesWithExt.begin();
             iter != filenamesWithExt.end();
             ++iter)
        {
            iter->insert(0, filename);
        }
#else
        std::list<std::string> filenamesWithExt(1, filename);
#endif

        for (std::list<std::string>::iterator iter = path.begin();
             iter != path.end();
             ++iter)
        {
            std::string dirname(*iter);
            dirname += Sep;
            std::list<std::string> filespecs(filenamesWithExt);

            for (std::list<std::string>::iterator iter = filespecs.begin();
                 iter != filespecs.end();
                 ++iter)
            {
                iter->insert(0, dirname);
            }

            filespecs.erase(remove_if(filespecs.begin(),
                                      filespecs.end(),
                                      std::not1(FileExists())),
                            filespecs.end());

            if (!filespecs.empty())
            {
                if (toShowAllMatches)
                {
                    std::copy(filespecs.begin(),
                              filespecs.end(),
                              std::ostream_iterator<std::string>(std::cout, "\n"));
                }
                else
                {
                    std::cout << *filespecs.begin() << std::endl;
                    break;
                }
            }
        }
    }

    return 0;
}

