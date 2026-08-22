#include <exception>
#include <iostream>

#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/benders/factories/BendersApp.h"

#ifdef _WIN32
// Temporary crash-diagnostics helper for investigating the Windows-only
// BendersByBatch MPI crash (see PR #1267): writes a full minidump on any
// unhandled SEH exception (access violation, etc.) so the failure can be
// inspected in WinDbg without a live debugger attached, since attaching one
// changes the MPI timing enough to mask the race. Not for permanent use.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <dbghelp.h>
#include <windows.h>

#include <cstdio>

#pragma comment(lib, "Dbghelp.lib")

LONG WINAPI WriteCrashDump(EXCEPTION_POINTERS* exception_pointers)
{
    char dump_path[MAX_PATH];
    std::snprintf(dump_path, sizeof(dump_path), "benders_crash_pid_%lu.dmp",
                 GetCurrentProcessId());

    HANDLE file = CreateFileA(dump_path,
                              GENERIC_WRITE,
                              0,
                              nullptr,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (file != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = exception_pointers;
        mdei.ClientPointers = FALSE;

        MiniDumpWriteDump(GetCurrentProcess(),
                          GetCurrentProcessId(),
                          file,
                          MiniDumpWithFullMemory,
                          exception_pointers ? &mdei : nullptr,
                          nullptr,
                          nullptr);
        CloseHandle(file);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetUnhandledExceptionFilter(WriteCrashDump);
#endif
    try
    {
        mpi::environment env(argc, argv);
        mpi::communicator world;
        // First check usage (options are given)
        if (world.rank() == 0)
        {
            usage(argc);
        }
        auto benders_factory = BendersApp(argv[1], world, SOLVER::BENDERS);
        return benders_factory.Run();
    }
    catch (std::exception& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!" << std::endl;
        return 1;
    }
}
