#include <exception>
#include <iostream>

#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/benders/factories/BendersApp.h"

#ifdef _WIN32
// Temporary crash-diagnostics helper for investigating the Windows-only
// BendersByBatch MPI crash (see PR #1267): writes a minidump on the first
// genuinely-fatal exception (access violation, etc.) so the failure can be
// inspected in WinDbg without a live debugger attached, since attaching one
// changes the MPI timing enough to mask the race. Not for permanent use.
//
// Registered two ways for robustness: a vectored exception handler (runs
// before anything else in the dispatch chain, including any top-level
// filter MPI_Init may install of its own, which would otherwise silently
// replace a plain SetUnhandledExceptionFilter) and, as a fallback, the
// unhandled-exception filter itself. Both funnel into the same writer,
// guarded so only the first fatal exception produces a dump. A lighter
// (non-full-memory) dump type is used deliberately: MS-MPI appears to tear
// down the whole job very soon after a rank dies, and a full-memory dump
// risks not finishing in time.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <dbghelp.h>
#include <windows.h>

#include <atomic>
#include <cstdio>

#pragma comment(lib, "Dbghelp.lib")

std::atomic<bool> g_dump_written{false};

bool IsFatalExceptionCode(DWORD code)
{
    switch (code)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_STACK_OVERFLOW:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_PRIV_INSTRUCTION:
    case EXCEPTION_IN_PAGE_ERROR:
        return true;
    default:
        return false;
    }
}

void WriteCrashDump(EXCEPTION_POINTERS* exception_pointers)
{
    bool expected = false;
    if (!g_dump_written.compare_exchange_strong(expected, true))
    {
        return;
    }

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

        MINIDUMP_TYPE dump_type = static_cast<MINIDUMP_TYPE>(
          MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithThreadInfo
          | MiniDumpWithProcessThreadData | MiniDumpWithUnloadedModules
          | MiniDumpWithFullMemoryInfo);

        MiniDumpWriteDump(GetCurrentProcess(),
                          GetCurrentProcessId(),
                          file,
                          dump_type,
                          exception_pointers ? &mdei : nullptr,
                          nullptr,
                          nullptr);
        FlushFileBuffers(file);
        CloseHandle(file);
    }
}

LONG WINAPI VectoredCrashHandler(EXCEPTION_POINTERS* exception_pointers)
{
    if (IsFatalExceptionCode(exception_pointers->ExceptionRecord->ExceptionCode))
    {
        WriteCrashDump(exception_pointers);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI UnhandledCrashFilter(EXCEPTION_POINTERS* exception_pointers)
{
    WriteCrashDump(exception_pointers);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    AddVectoredExceptionHandler(1, VectoredCrashHandler);
    SetUnhandledExceptionFilter(UnhandledCrashFilter);
#endif
    try
    {
        mpi::environment env(argc, argv);
#ifdef _WIN32
        // Re-assert both hooks in case MPI_Init installed its own handlers
        // that would otherwise take precedence.
        AddVectoredExceptionHandler(1, VectoredCrashHandler);
        SetUnhandledExceptionFilter(UnhandledCrashFilter);
#endif
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
