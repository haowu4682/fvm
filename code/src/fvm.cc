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

FVMClient *client;

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

    // Usage: link REPO LINK_SRC LINK_DST
    if (args.size() < 4) {
        printf("Usage: link repository link_src link_dst\n");
        return 0;
    }

#if 0
    if (!client->connected()) {
        printf("Please connect to the server first.\n");
        return 0;
    }
#endif

    String head_id;
    rc = client->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return 0;
    }

    rc = client->RetrieveHead(args[1], head_id);
    if (rc < 0) {
        LOG("Cannot retrieve HEAD status.");
        return 0;
    }

    rc = client->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return 0;
    }
    rc = client->Checkout(args[1], args[2].substr(1), args[3], head_id);
    if (rc < 0) {
        LOG("Cannot checkout the specified repository.");
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

    String head_id;
    rc = client->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return 0;
    }

    rc = client->RetrieveHead(args[1], head_id);
    if (rc < 0) {
        LOG("Cannot retrieve HEAD status.");
        return 0;
    }

    rc = client->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return 0;
    }
    rc = client->Commit(args[1], args[2].substr(1), args[3], head_id);
    if (rc < 0) {
        LOG("Cannot checkout the specified repository.");
        return 0;
    }

    return 0;
}

struct cmd_t {
    const char *cmd;
    int (*fn) (const Vector<String>& /* args */);
    const char *help_msg;
};

struct cmd_t fvm_commands[] = {
    // Single user mode command
    { "start", cmd_repo_start, "start automatical tracing"},
    { "backtrace", cmd_repo_backtrace, "enter backtrace mode"},

    // Sharing mode command
    { "server", cmd_connect, "Set up the server information"},
    { "link", cmd_link, "Link a repository to a specified destination"},
    { "commit", cmd_commit, "Manually make a commitment"},
    //{ "checkout", cmd_checkout},

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
        printf("Usage: fvm user_name user_email");
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
