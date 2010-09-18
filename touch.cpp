/**
 * touch - change file access and modification times
 * C++ implementation for Microsoft Windows.
 *
 * free to distribute under the GPL license.
 * if you have not received a copy of the license along with the code,
 * confer to http://www.gnu.org/licenses/gpl.html
 *
 * (C) Copyright 2009, 2010, Ji Han (jihan917<at>yahoo<dot>com).
 */


#include <windows.h>
#include <atltime.h>
#include "getopt.h"


typedef struct
{
    bool a;
    bool c;
    bool m;
} flags_st;

typedef struct
{
    const char *ref_file;
    const char *time;
} optargs_st;

typedef struct
{
    int SS;
    int mm;
    int hh;
    int DD;
    int MM;
    int CCYY;
} datetime_st;


void usage()
{
    fprintf(stdout,
            "\n"
            "touch - change file access and modification times\n"
            "(C) Copyright 2009, 2010, Ji Han (jihan917<at>yahoo<dot>com).\n"
            "\n"
            "SYNOPSIS: touch [-acm] [-r ref_file | -t time] file...\n"
            "\n"
            "for further information, see\n"
            "http://www.opengroup.org/onlinepubs/009695399/utilities/touch.html\n");
}


/**
 * parse options in command-line arguments
 */
void optparse(int argc, char **argv, flags_st& flags, optargs_st& optargs, int& ind)
{
    opt_st *opt = NULL;
    opt_init(&opt, malloc);

    for (int c; (c = getopt_r(argc, argv, ":acmr:t:", opt)) != -1; )
    {
        switch (c)
        {
            case 'a': flags.a = true;
                      break;

            case 'c': flags.c = true;
                      break;

            case 'm': flags.m = true;
                      break;

            case 'r': optargs.ref_file = optarg(opt);
                      optargs.time = NULL;
                      break;

            case 't': optargs.time = optarg(opt);
                      optargs.ref_file = NULL;
                      break;

            case ':': fprintf(stderr,
                              "touch: option '-%c' requires an operand.\n",
                              optopt(opt));
                      usage();
                      free(opt);
                      exit(EXIT_FAILURE);

            case '?': fprintf(stderr,
                              "touch: invalid option: '-%c'.\n",
                              optopt(opt));
                      usage();
                      free(opt);
                      exit(EXIT_FAILURE);
        }
    }
    ind = optind(opt);
    free(opt);
}


/**
 * parse string "[[CC]YY]MMDDhhmm[.SS]" as date and time
 */
bool timeparse(char *time, datetime_st& datetime)
{
    /* get SS */
    int SS = 0;

    char *end = strchr(time, '\0');
    char *dot = strchr(time, '.');
    if (dot)
    {
        SS = strtol(dot + 1, NULL, 10);
        *(end = dot) = '\0';
    }

    ptrdiff_t len = end - time;
    if (SS < 0 || SS > 60 ||
        !(len == 8 || len == 10 || len == 12))
    {
        return false;
    }

    /* get mm, hh, DD, MM */
    int mm, hh, DD, MM;

    mm = strtol(end - 2, NULL, 10);
    *(end -= 2) = '\0';
    hh = strtol(end - 2, NULL, 10);
    *(end -= 2) = '\0';
    DD = strtol(end - 2, NULL, 10);
    *(end -= 2) = '\0';
    MM = strtol(end - 2, NULL, 10);
    *(end -= 2) = '\0';

    if (mm < 0 || mm > 59 ||
        hh < 0 || hh > 23 ||
        DD < 1 || DD > 31 ||
        MM < 1 || MM > 12)
    {
        return false;
    }

    /* get CCYY */
    int CCYY;

    if (len == 12)
    {
        CCYY = strtol(end - 4, NULL, 10);
        if (CCYY < 1601) return false;
    }
    else if (len == 10)
    {
        int YY = strtol(end - 2, NULL, 10);
        int CC = (YY >= 69) ? 19 : 20;
        CCYY = 100 * CC + YY;
    }
    else /* len == 8 */
    {
        SYSTEMTIME st;
        GetSystemTime(&st);
        CCYY = st.wYear;
    }

    datetime.SS = SS;
    datetime.mm = mm;
    datetime.hh = hh;
    datetime.DD = DD;
    datetime.MM = MM;
    datetime.CCYY = CCYY;
    return true;
}


int main (int argc, char** argv)
{
    HANDLE hFile;
    FILETIME atime;
    FILETIME mtime;
    SYSTEMTIME st;

    flags_st flags = { false, false, false };
    optargs_st optargs = { NULL, NULL };
    int ind;
    optparse(argc, argv, flags, optargs, ind);

    /**
     * if neither the -a nor -m options were specified,
     * touch shall behave as if both the -a and -m options were specified.
     */
    if (!flags.a && !flags.m)
    {
        flags.a = true;
        flags.m = true;
    }

    /**
     * get timestamps
     */
    if (optargs.ref_file)
    {
        hFile = CreateFile(optargs.ref_file,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_FLAG_BACKUP_SEMANTICS,
                           NULL);
        if (!GetFileTime(hFile, NULL, &atime, &mtime))
        {
            fprintf(stderr,
                    "touch: error opening '%s': no such file or directory.\n",
                    optargs.ref_file);
            usage();
            exit(EXIT_FAILURE);
        }
        CloseHandle(hFile);
    }
    else if (optargs.time)
    {
        char time[16];
        strncpy(time, optargs.time, 16);
        datetime_st datetime;
        if (!timeparse(time, datetime))
        {
            fprintf(stderr,
                    "touch: error parsing '%s': invalid date format.\n",
                    optargs.time);
            usage();
            exit(EXIT_FAILURE);
        }
        CTime ctime(datetime.CCYY,
                    datetime.MM,
                    datetime.DD,
                    datetime.hh,
                    datetime.mm,
                    datetime.SS);
        ctime.GetAsSystemTime(st);
        if (flags.a) SystemTimeToFileTime(&st, &atime);
        if (flags.m) SystemTimeToFileTime(&st, &mtime);
    }
    else
    {
        GetSystemTime(&st);
        if (flags.a) SystemTimeToFileTime(&st, &atime);
        if (flags.m) SystemTimeToFileTime(&st, &mtime);
    }

    /**
     * set file timestamps
     */
    for (int i = ind; i < argc; ++i)
    {
        hFile = CreateFile(argv[i],
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           flags.c ? OPEN_EXISTING : OPEN_ALWAYS,
                           FILE_FLAG_BACKUP_SEMANTICS,
                           NULL);
        SetFileTime(hFile,
                    NULL,
                    flags.a ? &atime : NULL,
                    flags.m ? &mtime : NULL);
        CloseHandle(hFile);
    }

    return 0;
}

