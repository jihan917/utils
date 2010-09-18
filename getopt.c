/**
 * getopt_r - command option parsing
 * free to distribute under the GPL license.
 * (C) Copyright 2009, 2010, Ji Han (jihan917<at>yahoo<dot>com).
 * see http://www.opengroup.org/onlinepubs/000095399/functions/getopt.html
 * for the specification of getopt, the non-reentrant counterpart.
 */

#include "getopt.h"

struct opt_st
{
    char *optarg;
    int optind;
    int optopt;
    int __i;
};

const char *const optarg(const opt_st *const opt) { return opt->optarg; }
const int optind(const opt_st *const opt) { return opt->optind; }
const int optopt(const opt_st *const opt) { return opt->optopt; }

void opt_init(opt_st **popt, void* (*alloc)(size_t))
{
    *popt = (opt_st *)alloc(sizeof(opt_st));
    assert(popt);
    (*popt)->optind = 1;
    (*popt)->__i = 1;
}


int
getopt_r(int argc,
         char *const *argv,
         const char *optstring,
         opt_st *opt)
{
    const char *p = NULL;
    int i = opt->optind;
    if (!argv[i] || *argv[i] != '-' || !strcmp(argv[i], "-")) return (-1);
    if (!strcmp(argv[i], "--"))
    {
        ++opt->optind;
        return (-1);
    }

    p = strchr(optstring, argv[i][opt->__i]);
    if (p == NULL)
    {
        opt->optopt = argv[i][opt->__i];
        return '?';
    }

    if (*(p + 1) == ':')
    {
        if (argv[i][opt->__i + 1] != '\0')
        {
            opt->optarg = &argv[i][opt->__i + 1];
        }
        else
        {
            if (!argv[++opt->optind])
            {
                opt->optopt = *p;
                return (optstring[0] == ':' ? ':' : '?');
            }
            opt->optarg = argv[opt->optind];
        }
            ++opt->optind;
            opt->__i = 1;
    }
    else
    {
        if (argv[i][opt->__i + 1] != '\0')
        {
            ++opt->__i;
        }
        else
        {
            ++opt->optind;
            opt->__i = 1;
        }
    }
    return (*p);
}

