// This file includes all the possible commands in FVM
//
#ifndef COMMON_COMMANDS_H_
#define COMMON_COMMANDS_H_

#include <common/common.h>

// Universal Commands
// Display Help Information
int cmd_help(const Vector<String>& args);

// Tracer Commands
// Start tracing
int cmd_repo_start(const Vector<String>& args);
// Change trace level
int cmd_repo_tracelevel(const Vector<String>& args);
// Change config
int cmd_repo_config(const Vector<String>& args);
// Enter the backtrace mode
int cmd_repo_backtrace_start(const Vector<String>& args);
// Exit the backtrace mode
int cmd_repo_backtrace_stop(const Vector<String>& args);

#endif // COMMON_COMMANDS_H_

