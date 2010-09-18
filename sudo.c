/**
 * sudo.c: Run a program as elevated.
 * Copyright (c) 2009 Ji Han.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#pragma comment(lib,"shell32.lib") /* for ShellExecuteW */
#pragma comment(lib,"user32.lib") /* for MessageBoxW */

#include <stdlib.h>

#if !defined(NDEBUG)
#   include <assert.h>
#endif

#if !defined(__cplusplus)
    typedef enum {false, true} bool;
#endif

/* cmdlpbrk "command line pointer break"
 * returns a pointer to the first occurance in cmdl of any character in delims,
 * or a null pointer if there are no matches.
 * characters in a double quoted string are treated literally.
 * a double quotation mark begins/ends a quoted string,
 * unless it's following an odd number of backslashes and treated literally.
 * delims shall not contain the double quotation mark or the backslash.
 */
const wchar_t*
cmdlpbrk(const wchar_t* cmdl, const wchar_t* delims)
{
    const wchar_t* pwc = cmdl;
    bool quote = false; /* next char is not quoted */
    bool esc = false; /* next char is not escaped */

#if !defined(NDEBUG)
    /* '\"' and '\\' not allowed in delims */
    assert(wcscspn(delims, L"\"\\") == wcslen(delims));
#endif

    for ( ; *pwc ; ++pwc)
    {
        switch(*pwc)
          {
            case L'\\':
                esc = !esc;
                break;
            case L'"':
                if (!esc) { quote = !quote; }
                break;
            default:
                esc = false;
                if (!quote && wcschr(delims, *pwc)) { return pwc; }
          }
    }

    return NULL;
}

/* *** *** *** */
LPCWSTR szTitle = L"sudo";
LPCWSTR szWhiteSpaces = L" \t";
LPCWSTR szNullString = L"";
const wchar_t wcharNull = L'\0';
LPCWSTR szShowUsage = L"SUDO Copyright 2009 J.\n"
                      L"Run a program as elevated.\n"
                      L"usage: sudo <program> [arguments]\n";
/* *** *** *** */

int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow)
{
    LPWSTR szExec = NULL;
    LPCWSTR szArgs = NULL;
    int len;

    if (!lpCmdLine || wcscmp(lpCmdLine, szNullString) == 0) {
        MessageBoxW(NULL, szShowUsage, szTitle, MB_OK);
        return EXIT_FAILURE;
    }

    szArgs = cmdlpbrk(lpCmdLine, szWhiteSpaces);

    if (szArgs) {
        len = szArgs - lpCmdLine;
        szExec = malloc(sizeof(wchar_t)*(len+1));
        wcsncpy(szExec, lpCmdLine, len);
        szExec[len] = wcharNull;
    } else {
        szExec = lpCmdLine;
        szArgs = szNullString;
    }

    ShellExecuteW(NULL, L"runas", szExec, szArgs, NULL, SW_SHOW);
    return EXIT_SUCCESS;
}

