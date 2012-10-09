// The files contains the command line interface for FVM

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <common/common.h>
#include <common/commands.h>

void usage() {
    // TODO detail usage info
    printf("Usage: fvm <command> [<arguments>...]\n");
}

// Display Help Information
int cmd_help(const Vector<String>& args)
{
    // TODO: Display internal help information for every command

    // Display the universal help command
    usage();

    return 0;
}

// Exit the fvm manager
int cmd_exit(const Vector<String> &)
{
    return 1;
}

struct cmd_t {
    const char *cmd;
    int (*fn) (const Vector<String>& /* args */);
};

struct cmd_t fvm_commands[] = {
    // Universal Commands
    { "help", cmd_help },
    { "exit", cmd_exit }
//    {"start", cmd_repo_start},
};

// Handle a pre-defined command in commands.h
// Return 0: Normal Execution
// Return -1: Command not Found
// Return 1: Exit loop
int handle_command(const Vector<String>& args)
{
    int rc;

    if (args.empty()) {
        usage();
        return -1;
    }

    // Decide the command to be executed according to the first argument
    for (int i = 0; i < ARRAY_SIZE(fvm_commands); ++i) {
        if (args[0] == fvm_commands[i].cmd) {
            rc = fvm_commands[i].fn(args);
            return rc;
        }
    }

    // At this point, the command is not found in the command list.
    // So we indicate the usage and exits
    LOG("Error: command not found.");
    usage();

    return -1;
}

// Read a line and split it into string arrays
std::istream& readline(std::istream& is, // the input stream for the command
        Vector<String>& str_array) // the string array to store the result
{
    String line;
    getline(is, line);

    std::istringstream line_stream(line);
    while (line_stream)
    {
        String str;
        line_stream >> str;
        str_array.push_back(str);
    }

    return is;
}

int main(int argc, char **argv)
{
    int rc = 0;
    Vector<String> command_args;

    while (rc != 1) {
        std::cout << "fvm> ";
        command_args.clear();
        readline(std::cin, command_args);
        rc = handle_command(command_args);

        if (rc < 0) {
            DBG("Command execution failed. error code = %d", rc);
        }
    }

    return 0;
}

