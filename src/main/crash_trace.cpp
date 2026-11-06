// On Windows, prints where the program crashed to stderr: the exception and each frame of the crashing thread's stack,
// named from the .pdb beside the exe. Recompiled functions carry the game's names (syms/), so a trace points at the
// game code that crashed; abort(), std::terminate and the CRT's checks print theirs too. --stacks <seconds> (development) prints every thread's stack that long after start, so a hang
// shows where each thread waits or spins. --watch <address>,... (development) stops on every write to those words of the
// game's memory with the CPU's debug registers and prints the value written and the stack that wrote it.
#include <cstdint>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>
#include <TlHelp32.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <exception>
#include <set>
#include <string>
#include <thread>

static void print_stack(HANDLE process, HANDLE thread, CONTEXT context) {
    STACKFRAME64 frame{};
    frame.AddrPC.Offset = context.Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
    alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + 256];
    for (int i = 0; i < 48; i++) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame, &context, nullptr,
                SymFunctionTableAccess64, SymGetModuleBase64, nullptr) || frame.AddrPC.Offset == 0) {
            break;
        }
        SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 255;
        DWORD64 displacement = 0;
        if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol)) {
            fprintf(stderr, "  %2d %s+0x%llX", i, symbol->Name, (unsigned long long)displacement);
            IMAGEHLP_LINE64 line{};
            line.SizeOfStruct = sizeof(line);
            DWORD line_displacement = 0;
            if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &line_displacement, &line)) {
                fprintf(stderr, " (%s:%lu)", line.FileName, line.LineNumber);
            }
            fprintf(stderr, "\n");
        } else {
            fprintf(stderr, "  %2d %p\n", i, (void*)frame.AddrPC.Offset);
        }
    }
}

static LONG WINAPI print_crash(EXCEPTION_POINTERS* info) {
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(process, nullptr, TRUE);
    fprintf(stderr, "Crash: exception 0x%08lX at %p", info->ExceptionRecord->ExceptionCode,
        info->ExceptionRecord->ExceptionAddress);
    if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        fprintf(stderr, " (%s 0x%llX)", info->ExceptionRecord->ExceptionInformation[0] ? "writing" : "reading",
            (unsigned long long)info->ExceptionRecord->ExceptionInformation[1]);
    }
    fprintf(stderr, "\n");

    print_stack(process, GetCurrentThread(), *info->ContextRecord);
    fflush(stderr);
    return EXCEPTION_CONTINUE_SEARCH;
}


// abort(), std::terminate and the CRT's invalid parameter check end the program by __fastfail, which no exception
// filter sees, so each prints the stack it was called on first.
static void print_here(const char* what) {
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(process, nullptr, TRUE);
    fprintf(stderr, "Crash: %s\n", what);
    CONTEXT context{};
    RtlCaptureContext(&context);
    print_stack(process, GetCurrentThread(), context);
    fflush(stderr);
}

void install_crash_trace() {
    SetUnhandledExceptionFilter(print_crash);
    signal(SIGABRT, [](int) { print_here("abort()"); });
    std::set_terminate([] { print_here("std::terminate"); std::abort(); });
    _set_invalid_parameter_handler([](const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t) {
        print_here("invalid parameter to a CRT function");
        std::abort();
    });
}

void print_stacks_after(double seconds) {
    std::thread([seconds] {
        std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
        HANDLE process = GetCurrentProcess();
        SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
        SymInitialize(process, nullptr, TRUE);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        THREADENTRY32 entry{};
        entry.dwSize = sizeof(entry);
        for (BOOL ok = Thread32First(snapshot, &entry); ok; ok = Thread32Next(snapshot, &entry)) {
            if (entry.th32OwnerProcessID != GetCurrentProcessId() || entry.th32ThreadID == GetCurrentThreadId()) {
                continue;
            }
            HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE,
                entry.th32ThreadID);
            if (thread == nullptr) {
                continue;
            }
            SuspendThread(thread);
            CONTEXT context{};
            context.ContextFlags = CONTEXT_FULL;
            if (GetThreadContext(thread, &context)) {
                fprintf(stderr, "Thread %lu:\n", entry.th32ThreadID);
                print_stack(process, thread, context);
            }
            ResumeThread(thread);
            CloseHandle(thread);
        }
        CloseHandle(snapshot);
        fflush(stderr);
    }).detach();
}

// --watch: up to four addresses, each [*...]<hex>[+-<hex>][:<length>], resolved when the game's main loop first runs
// (watch_writes_arm, from dino_story_pass): each * reads the word at the address so far, so **8008C994+4 is the second
// word of the table DLL 1's handle points at. The length is 1, 2, 4 (the default) or 8, and the address is rounded down
// to it. A thread polls the process's threads and sets the debug registers on each new one, so a write in a thread made
// later is caught from its first 10 ms on. Each watch prints its first 20 writes.
namespace {
    std::string watch_spec;
    uint8_t* watch_host[4];
    uint32_t watch_vram[4];
    int watch_length[4];
    int watch_count = 0;
    std::atomic<int> watch_hits[4];
    ULONGLONG watch_armed_at = 0;

    LONG WINAPI on_watch(EXCEPTION_POINTERS* info) {
        if (info->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP || (info->ContextRecord->Dr6 & 0xF) == 0) {
            return EXCEPTION_CONTINUE_SEARCH;
        }
        DWORD64 hit = info->ContextRecord->Dr6 & 0xF;
        info->ContextRecord->Dr6 = 0;
        for (int i = 0; i < watch_count; i++) {
            if (!(hit & (1ull << i)) || watch_hits[i]++ >= 20) {
                continue;
            }
            uint32_t value = *reinterpret_cast<uint32_t*>(watch_host[i]);
            fprintf(stderr, "--watch: 0x%08X now 0x%08X, %.3f s after arming, thread %lu:\n", watch_vram[i], value,
                (GetTickCount64() - watch_armed_at) / 1000.0, GetCurrentThreadId());
            print_stack(GetCurrentProcess(), GetCurrentThread(), *info->ContextRecord);
            fflush(stderr);
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    void set_debug_registers(DWORD thread_id) {
        HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, thread_id);
        if (thread == nullptr) {
            return;
        }
        SuspendThread(thread);
        CONTEXT context{};
        context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        if (GetThreadContext(thread, &context)) {
            DWORD64* address[4] = { &context.Dr0, &context.Dr1, &context.Dr2, &context.Dr3 };
            context.Dr7 = 0;
            for (int i = 0; i < watch_count; i++) {
                *address[i] = reinterpret_cast<DWORD64>(watch_host[i]);
                DWORD64 length = watch_length[i] == 1 ? 0 : watch_length[i] == 2 ? 1 : watch_length[i] == 8 ? 2 : 3;
                // Local enable, break on writes only (01), the length's code.
                context.Dr7 |= (1ull << (i * 2)) | (1ull << (16 + i * 4)) | (length << (18 + i * 4));
            }
            SetThreadContext(thread, &context);
        }
        ResumeThread(thread);
        CloseHandle(thread);
    }
}

void watch_writes(const char* spec) {
    watch_spec = spec;
}

void watch_writes_arm(uint8_t* rdram) {
    if (watch_spec.empty() || watch_count != 0) {
        return;
    }
    for (const char* p = watch_spec.c_str(); p != nullptr && *p != '\0' && watch_count < 4;
            p = strchr(p, ',') ? strchr(p, ',') + 1 : nullptr) {
        int derefs = 0;
        while (*p == '*') {
            derefs++;
            p++;
        }
        char* end = nullptr;
        uint32_t address = strtoul(p, &end, 16);
        for (int i = 0; i < derefs; i++) {
            address = *reinterpret_cast<uint32_t*>(rdram + (address - 0x80000000u));
        }
        if (*end == '+' || *end == '-') {
            bool minus = *end == '-';
            uint32_t offset = strtoul(end + 1, &end, 16);
            address = minus ? address - offset : address + offset;
        }
        int length = 4;
        if (*end == ':') {
            length = atoi(end + 1);
        }
        if (length != 1 && length != 2 && length != 8) {
            length = 4;
        }
        address &= ~(uint32_t)(length - 1);
        watch_vram[watch_count] = address;
        watch_length[watch_count] = length;
        watch_host[watch_count] = rdram + (address - 0x80000000u);
        fprintf(stderr, "--watch: %d bytes at 0x%08X\n", length, address);
        watch_count++;
    }
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(process, nullptr, TRUE);
    AddVectoredExceptionHandler(1, on_watch);
    watch_armed_at = GetTickCount64();
    std::thread([] {
        std::set<DWORD> armed;
        DWORD self = GetCurrentThreadId();
        while (true) {
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            THREADENTRY32 entry{};
            entry.dwSize = sizeof(entry);
            for (BOOL ok = Thread32First(snapshot, &entry); ok; ok = Thread32Next(snapshot, &entry)) {
                if (entry.th32OwnerProcessID == GetCurrentProcessId() && entry.th32ThreadID != self &&
                        armed.insert(entry.th32ThreadID).second) {
                    set_debug_registers(entry.th32ThreadID);
                }
            }
            CloseHandle(snapshot);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }).detach();
}
#else
void install_crash_trace() {}
void print_stacks_after(double) {}
void watch_writes(const char*) {}
void watch_writes_arm(uint8_t*) {}
#endif
