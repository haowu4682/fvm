// This file includes all the possible commands in FVM
//
#ifndef COMMON_COMMANDS_H_
#define COMMON_COMMANDS_H_

#include <common/common.h>

// Universal Commands
// Display Help Information
int cmd_help(Vector<String>& args);

// Single User Mode Commands
// Start tracing
int cmd_repo_start(Vector<String>& args);
// Change trace level
int cmd_repo_tracelevel(Vector<String>& args);
// Change config
int cmd_repo_config(Vector<String>& args);
// Enter the backtrace mode
int cmd_repo_backtrace(Vector<String>& args);

// Multiple User Mode Commands

#endif // COMMON_COMMANDS_H_

