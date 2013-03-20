
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
    // XXX Unimplemented
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

/* Executes the push whose context is in the git_push object. */
int FVMTransport::Push(git_push *push)
{
    // XXX Unimplemented
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
    // XXX Unimplemented
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

FVMTransport::FVMTransport()
{
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

