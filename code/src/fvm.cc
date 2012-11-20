// The files contains the command line interface for FVM

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <fvmclient.h>
#include <partialtracer.h>

#include <common/common.h>
#include <common/commands.h>
#include <common/util.h>

FVMClient *client = NULL;
PartialTracer *tracer = NULL;

int cmd_help(const Vector<String>& args);

// Exit the fvm manager
int cmd_exit(const Vector<String> &)
{
    return 1;
}

// Connect to the server
int cmd_connect(const Vector<String> &args)
{
    int rc;

    // Usage: connect server_addr port
    if (args.size() < 3) {
        printf("Usage: connect server_addr port\n");
        return 0;
    }

    // Set up FVM client attributes
    client->hostname(args[1]);
    client->port(atoi(args[2].c_str()));

    // Connect
#if 0
    rc = client->Connect();
    if (rc < 0) {
        LOG("Connection failed for %s:%s", args[1].c_str(), args[2].c_str());
        return 0;
    }
#endif

    printf("Server set up finished.\n");
    return 0;
}

// Link
int cmd_link(const Vector<String> &args)
{
    int rc;

    if (args.size() < 4) {
        printf("Usage: link repository link_src link_dst\n");
        return 0;
    }

    if (tracer != NULL) {
        printf("A tracer already exists! stop the program and try again.");
        return 0;
    }

    // Start the tracer
    PartialTracer *tracer = new PartialTracer(args[1], args[2], args[3], client);
    tracer->Start();

    return 0;
}

int cmd_commit(const Vector<String>& args)
{
    int rc;

    if (tracer == NULL) {
        printf("Use the 'link' command to create a link first!\n");
        return 0;
    }

    tracer->Commit();

    return 0;
}

int cmd_repo_backtrace_start(const Vector<String> &args)
{
    if (args.size() < 3 || args[2] == "master") {
        printf("Usage: backtrace path_to_backtrace branch_name\n");
        printf("branch_name cannot be 'master'");
        return -1;
    }

    if (tracer == NULL) {
        printf("Use the 'link' command to create a link first!\n");
        return 0;
    }

    tracer->StartBacktrace(args[2], args[1]);

    return 0;
}

int cmd_repo_backtrace_stop(const Vector<String> &args)
{
    if (args.size() < 3 || args[2] == "master") {
        printf("Usage: backtrace branch_name merge_choice\n");
        printf("branch_name cannot be 'master'\n");
        return -1;
    }

    if (tracer == NULL) {
        printf("Use the 'link' command to create a link first!\n");
        return 0;
    }

    BacktraceMergeChoice choice;

    if (args[2] == "abondon") {
        choice = kAbandon;
    } else if (args[2] == "overwrite") {
        choice = kOverwrite;
    } else if (args[2] == "merge") {
        choice = kMerge;
    } else {
        printf("Unknown merge_choice, Aborted.");
    }

    tracer->StopBacktrace(args[1], choice);

    return 0;
}

struct cmd_t {
    const char *cmd;
    int (*fn) (const Vector<String>& /* args */);
    const char *help_msg;
};

struct cmd_t fvm_commands[] = {
    // Sharing mode command
    { "server", cmd_connect, "Set up the server information"},
    { "link", cmd_link, "Link a repository to a specified destination"},
    { "commit", cmd_commit, "Manually make a commitment"},
    { "backtrace-start", cmd_repo_backtrace_start, "enter backtrace mode for a specific path"},
    { "backtrace-stop", cmd_repo_backtrace_stop, "exit backtrace mode for a specific path"},

    // Universal command
    { "help", cmd_help, "List help information" },
    { "exit", cmd_exit, "Exit the FVM console"}
};

void usage() {
    printf("Usage: <command> [<arguments>...]\n");
    printf("Command List:\n");
    for (int i = 0; i < ARRAY_SIZE(fvm_commands); ++i) {
        printf("%s - %s\n", fvm_commands[i].cmd, fvm_commands[i].help_msg);
    }
}

// Display Help Information
int cmd_help(const Vector<String>& args)
{

    // Display the universal help command
    if (args.size() > 1) {
        for (int i = 0; i < ARRAY_SIZE(fvm_commands); ++i) {
            if (args[0] == fvm_commands[i].cmd) {
                // TODO: Display internal help information for every command
                printf("No help information yet.\n");
                return 0;
            }
        }
    }

    usage();

    return 0;
}

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

int main(int argc, char **argv)
{
    int rc = 0;
    Vector<String> command_args;

    if (argc < 3) {
        printf("Usage: fvm user_name user_email\n");
        return 0;
    }

    client = new FVMClient(argv[1], argv[2]);

    while (rc != 1) {
        std::cout << "fvm> ";
        command_args.clear();
        readline(std::cin, command_args);
        rc = handle_command(command_args);

        if (rc < 0) {
            DBG("Command execution failed. error code = %d", rc);
        }
    }

    delete client;

    return 0;
}

