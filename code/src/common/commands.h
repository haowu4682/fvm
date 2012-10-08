// This file includes all the possible commands in FVM
//
#ifndef COMMON_COMMANDS_H_
#define COMMON_COMMANDS_H_

// Single User Mode Commands
// Start tracing
int cmd_repo_start(int argc, char **argv);
// Change trace level
int cmd_repo_tracelevel(int argc, char **argv);
// Change config
int cmd_repo_config(int argc, char **argv);
// Enter the backtrace mode
int cmd_repo_backtrace(int argc, char **argv);

// Multiple User Mode Commands

#endif // COMMON_COMMANDS_H_

