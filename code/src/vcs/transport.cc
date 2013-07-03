
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include <vcs/gitvcs.h>

/* Set progress and error callbacks */
int SetCallbacks(struct git_transport *transport,
        git_transport_message_cb progress_cb,
        git_transport_message_cb error_cb,
        void *payload)
{
    // XXX Unimplemented
    return 0;
}

/* Connect the transport to the remote repository, using the given
 * direction. */
int FVMTransport::Connect(const char *url,
        git_cred_acquire_cb cred_acquire_cb,
        void *cred_acquire_payload,
        int direction,
        int flags)
{
    int rc;

    // Initialize session
    session = ssh_new();
    if (session == NULL) {
        return -1;
    }

    // Set options
    // TODO extract host and port from url
    int verbosity = SSH_LOG_PROTOCOL;
    ssh_options_set(session, SSH_OPTIONS_HOST, "localhost");
    ssh_options_set(session, SSH_OPTIONS_PORT, &port_);
    ssh_options_set(session, SSH_OPTIONS_USER, username_.c_str());
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

    // Connect to the server
    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        LOG("Error connecting to localhost: %s\n",
                ssh_get_error(session));
        exit(-1);
    }

    // Authenticate
    // FVM requires the user to use PUBLIC_KEY authentication
    rc = ssh_userauth_publickey_auto(session, NULL/*username.c_str()*/, NULL);
    if (rc == SSH_AUTH_ERROR) {
        LOG("Authentication failed: %s\n",
                ssh_get_error(session));
        return rc;
    } else if (rc == SSH_AUTH_DENIED || rc == SSH_AUTH_PARTIAL) {
        LOG("Authentication denied: %s\n",
                ssh_get_error(session));
        return rc;
    }

    // Open channel
    channel = ssh_channel_new(session);
    if (channel == NULL) {
        return SSH_ERROR;
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        ssh_channel_free(channel);
        return rc;
    }

    // Finish up setting attributes
    is_connected_ = true;

    return 0;
}

/* This function may be called after a successful call to connect(). The
 * provided callback is invoked for each ref discovered on the remote
 * end. */
int Ls(struct git_transport *transport,
        git_headlist_cb list_cb,
        void *payload)
{
    // XXX Unimplemented
    return 0;
}

#define TestAndWrite(channel, buf, size, msg) do { \
        int rc = ssh_channel_write(channel, buf, size); \
        if (rc < 0) { \
            LOG(msg); \
            return -1; \
        } \
    } while (0)


String GetHeader(git_push *push)
{
    int rc;
    std::stringstream header_stream;
    size_t spec_count;
    git_push_spec* spec;
    char buf[4000];

    spec_count = git_push_refspec_count(push);
    //object_count = git_push_object_count(push);

    header_stream << "spec_count" << std::endl;

    for (size_t i = 0; i < spec_count; ++i) {
        spec = git_push_refspec_byindex(push, i);
        git_pushspec_tobuffer(spec, buf, 4000);
        header_stream << buf << std::endl;
    }

    return header_stream.str();
}

static int WriteObjectCallback(void *buf, size_t size, void *data)
{
    int rc;
    ssh_channel *channel = static_cast<ssh_channel *>(data);
    char size_str_buf[100];
    snprintf(size_str_buf, 100, "%ld\n", size);
    TestAndWrite(*channel, size_str_buf, strlen(size_str_buf), "Failed to write object!");
    TestAndWrite(*channel, buf, size, "Failed to write object!");
}

/* Executes the push whose context is in the git_push object. */
int FVMTransport::Push(git_push *push)
{
    int rc;
    // XXX Magic number
    char buf[10000];
    size_t spec_count, object_count;
    git_push_spec* spec;
    git_packbuilder* pb;

    // Retrieve information
    String header = GetHeader(push);

    // Write header
    // "nr_specs nr_objects\n"
    //"spec1\n"
    //"..."
    //"specn\n"
    TestAndWrite(channel, header.c_str(), header.size(), "Cannot write push package header!");

    // Write objects
    //"<size><buf_content>
    pb = git_push_packbuilder(push);
    git_packbuilder_foreach(pb, WriteObjectCallback, &channel);

    return 0;
}

/* This function may be called after a successful call to connect(), when
 * the direction is FETCH. The function performs a negotiation to calculate
 * the wants list for the fetch. */
int NegotiateFetch(struct git_transport *transport,
        git_repository *repo,
        const git_remote_head * const *refs,
        size_t count)
{
    // XXX Unimplemented
    return 0;
}

/* This function may be called after a successful call to negotiate_fetch(),
 * when the direction is FETCH. This function retrieves the pack file for
 * the fetch from the remote end. */
int DownloadPack(struct git_transport *transport,
        git_repository *repo,
        git_transfer_progress *stats,
        git_transfer_progress_callback progress_cb,
        void *progress_payload)
{
    // XXX Unimplemented
    return 0;
}

/* Checks to see if the transport is connected */
int FVMTransport::IsConnected()
{
    return is_connected_;
}

/* Reads the flags value previously passed into connect() */
int ReadFlags(struct git_transport *transport, int *flags)
{
    // XXX Unimplemented
    return 0;
}

/* Cancels any outstanding transport operation */
void Cancel(struct git_transport *transport)
{
    // XXX Unimplemented
}

/* This function is the reverse of connect() -- it terminates the
 * connection to the remote end. */
int FVMTransport::Close()
{
    if (is_connected_) {
        is_connected_ = false;
        ssh_channel_free(channel);
        ssh_free(session);
    }

    return 0;
}

/* Frees/destructs the git_transport object. */
void FVMTransport::Free(struct git_transport *transport)
{
    // It breaks if the "git_transport" is a static object.
    // However in the case, the logic should not call the "Free" function, so
    // it should not happen in reasonable execution.
    delete transport;
}

int FVMConnect(struct git_transport *transport,
        const char *url,
        git_cred_acquire_cb cred_acquire_cb,
        void *cred_acquire_payload,
        int direction,
        int flags)
{
    FVMTransport *fvm_transport = static_cast<FVMTransport *>(transport);
    assert(fvm_transport != NULL);

    return fvm_transport->Connect(url, cred_acquire_cb,
            cred_acquire_payload, direction, flags);
}

int FVMPush(struct git_transport *transport, git_push *push)
{
    FVMTransport *fvm_transport = static_cast<FVMTransport *>(transport);
    assert(fvm_transport != NULL);

    return fvm_transport->Push(push);
}

/* Checks to see if the transport is connected */
int FVMIsConnected(struct git_transport *transport)
{
    FVMTransport *fvm_transport = static_cast<FVMTransport *>(transport);
    assert(fvm_transport != NULL);

    return fvm_transport->IsConnected();
}

int FVMClose(struct git_transport *transport)
{
    FVMTransport *fvm_transport = static_cast<FVMTransport *>(transport);
    assert(fvm_transport != NULL);

    return fvm_transport->Close();
}

FVMTransport::FVMTransport(const String& username, int port)
{
    username_ = username;
    port_ = port;
    version = 1;
    is_connected_ = false;

    set_callbacks = NULL;
    connect = FVMConnect;
    ls = NULL;
    push = FVMPush;
    negotiate_fetch = NULL;
    download_pack = NULL;
    is_connected = FVMIsConnected;
    read_flags = NULL;
    cancel = NULL;
    close = FVMClose;
    free = Free;
}

