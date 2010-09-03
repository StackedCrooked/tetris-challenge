#include "Console.h"
#include <ios>
#include <stdexcept>
#include <io.h>
#include <fcntl.h>
#include <windows.h>


void AttachToConsole()
{
    if (!::AttachConsole(ATTACH_PARENT_PROCESS))
    {
        if (::GetLastError() == ERROR_ACCESS_DENIED)
        {
            throw std::runtime_error("Failed to start a console for displaying logging output.");
        }
        AllocConsole();
    }
    
    int raw_out = _open_osfhandle(reinterpret_cast<intptr_t>(GetStdHandle(STD_OUTPUT_HANDLE)), _O_TEXT);
    *stdout = *_fdopen(raw_out, "w");
    setvbuf(stdout, NULL, _IONBF, 0);

    int raw_err = _open_osfhandle(reinterpret_cast<intptr_t>(GetStdHandle(STD_ERROR_HANDLE)), _O_TEXT);
    *stderr = *_fdopen(raw_err, "w");
    setvbuf(stderr, NULL, _IONBF, 0);

    int raw_in = _open_osfhandle(reinterpret_cast<intptr_t>(GetStdHandle(STD_INPUT_HANDLE)), _O_TEXT);
    *stdin = *_fdopen(raw_in, "r");
    setvbuf(stdin, NULL, _IONBF, 0);
    // Fix all cout, wcout, cin, wcin, cerr, wcerr, clog and wclog.
    std::ios::sync_with_stdio();
}
