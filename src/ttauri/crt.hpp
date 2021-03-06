// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file crt.hpp
 * CRT - Main entry point of a ttauri program.
 *
 * This header file will abstract the entry point for a program
 * for different operating systems, and call the `tt_main()` function
 * that should be defined as a portable entry point of the program.
 *
 * This header should be included only once by a only a single
 * translation-unit, as it defines `main()` or `WinMain()`.
 *
 * The work done by this abstraction is purposefully very limited,
 * its task it to make sure the command-line arguments are split into
 * tokens according to the rules of the operating system's shell. And
 * that the command line arguments are encoded as UTF-8.
 */

#pragma once

#include "crt_utils.hpp"

#if not defined(TT_CRT_NO_MAIN)

/** Main entry-point.
 *
 * @param argc Number of arguments
 * @param argv A nullptr terminated list of pointers to null terminated strings.
 * @return Exit code.
 */
int tt_main(int argc, char *argv[]);

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include <Windows.h>

/** Windows entry-point.
 * This function will call `tt_main()`.
 *
 * It will use GetCommandLineW() to retrieve the command line in Unicode.
 *
 * The nShowCmd is used to insert add a command line argument at index 1:
 *  - 1,4,5,8,9,10: (No command line argument added)
 *  - 3: --window-state=maximize
 *  - 0,2,6,7,11: --window-state=minimize
 */
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance,
    [[maybe_unused]] _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
    auto [argc, argv] = tt::crt_start(hInstance, nShowCmd);
    ttlet r = tt_main(argc, argv);
    return tt::crt_finish(argc, argv, r);
}

#else
#error "Need entry point for this architecture"
#endif
#endif
