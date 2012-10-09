// The files contains the command line interface for FVM

#include <cstring>

#include <common/common.h>
#include <common/commands.h>

void usage() {
    // TODO detail usage info
    printf("Usage: fvm <command> [<arguments>...]\n");
}

struct cmd_t {
    const char *cmd;
    int (*fn) (int /* argc */, char ** /* argv */);
};

struct cmd_t fvm_commands[] = {
    // Universal Commands
    {"help", cmd_help}
//    {"start", cmd_repo_start},
};

// Display Help Information
int cmd_help(int argc, char **argv)
{
    // TODO: Display internal help information for every command

    // Display the universal help command
    usage();
}

// Handle a pre-defined command in commands.h
int handle_command(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        return -1;
    }

    // Decide the command to be executed according to the first argument
    for (int i = 0; i < ARRAY_SIZE(fvm_commands); ++i) {
        if (strcmp(argv[1], fvm_commands[i].cmd) == 0) {
            fvm_commands[i].fn(argc, argv);
            return 0;
        }
    }

    // At this point, the command is not found in the command list.
    // So we indicate the usage and exits
    LOG("Error: command not found.");
    usage();

    return -1;
}

int main(int argc, char **argv)
{
    int rc;

    rc = handle_command(argc, argv);
    if (rc < 0) {
        DBG("Command execution failed. error code = %d", rc);
    }

    return 0;
}

