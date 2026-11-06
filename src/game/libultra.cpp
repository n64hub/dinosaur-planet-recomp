// libultra functions the game links that N64Recomp leaves out (its ignored_funcs) and the runtime doesn't implement,
// so each call to one in RecompiledFuncs/ comes here.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "recomp.h"
#include "ultramodern/error_handling.hpp"

// The game's scheduler (libultra's own, func_8003B820) writes SP_CLR_HALT and the rest of 0xAAAA82 when it restarts
// the RSP after a task that ran past its time. The runtime runs each RSP task whole, so there is no RSP to restart.
extern "C" void __osSpSetStatus_recomp(uint8_t* rdram, recomp_context* ctx) {
}

// The rest are called only from code that never runs here: the debugger (rmon, which the game never starts), the VI
// manager's thread and its timer interrupt (the runtime has its own VI manager), the Controller Pak's file system and
// CRCs (under osMotorInit and osPfs*, which the runtime replaces or the game never calls, its saves being on Flash),
// and a raw SI read (func_800112F0) whose only caller nothing calls. One that runs anyway stops the game, saying which.
static void unreachable(const char* name) {
    std::string message = std::string{"The game called libultra's "} + name + ", which this build doesn't have.";
    fprintf(stderr, "%s\n", message.c_str());
    ultramodern::error_handling::message_box(message.c_str());
    std::quick_exit(EXIT_FAILURE);
}

#define UNREACHABLE(name) \
    extern "C" void name##_recomp(uint8_t* rdram, recomp_context* ctx) { unreachable(#name); }

UNREACHABLE(__osViSwapContext)
UNREACHABLE(__osSetCompare)
UNREACHABLE(__osSetTimerIntr)
UNREACHABLE(__osInsertTimer)
UNREACHABLE(__osGetCause)
UNREACHABLE(__osSpDeviceBusy)
UNREACHABLE(__osContAddressCrc)
UNREACHABLE(__osPfsSelectBank)
UNREACHABLE(__osContRamRead)
UNREACHABLE(__osCheckPackId)
UNREACHABLE(__osRepairPackId)
UNREACHABLE(__osSiGetAccess)
UNREACHABLE(__osSiRawStartDma)
UNREACHABLE(__osSiRelAccess)
UNREACHABLE(__rmonInit)
UNREACHABLE(__rmonHitBreak)
UNREACHABLE(__rmonHitSpBreak)
UNREACHABLE(__rmonWriteWordTo)
UNREACHABLE(__rmonReadWordAt)
UNREACHABLE(__rmonRCPrunning)
UNREACHABLE(__rmonStepRCP)
UNREACHABLE(__rmonGetThreadStatus)
UNREACHABLE(__rmonGetExceptionStatus)
UNREACHABLE(__rmonSendReply)
UNREACHABLE(__rmonMaskIdleThreadInts)
UNREACHABLE(__rmonStopUserThreads)
UNREACHABLE(__rmonSendFault)
