// The file describes Tracer, the utility to trace files

#include <string>
#include <vector>

#include <common/common.h>
#include <single/tracer.h>

Tracer* Tracer::current_tracer = NULL;

Tracer::Tracer()
{
    running_ = false;

    Init();
}

void Tracer::Start()
{
    running_ = true;
}

void Tracer::Stop()
{
    running_ = false;
}

// An auxilary function to call tracer->Trace() for pthread.
void * trace_pthread_function(void * tracer_void_ptr)
{
    // WARN: We do not do type check, so the user must make sure that tracer is
    // a pointer to Tracer class.
    Tracer * tracer = static_cast<Tracer *>(tracer_void_ptr);
    tracer->Trace();
}

void Tracer::Init()
{
    int rc;

    if ((rc = pthread_create(&trace_thread_, NULL, &trace_pthread_function, this))) {
        LOG("Thread creation failed: %d\n", rc);
        LOG("The tracer cannot be started!");
    }
}

void Tracer::Trace()
{
    // TODO Implement
}

int cmd_repo_start(Vector<String>& args)
{
    // Step 1: Read in config
    // TODO Read in config

    // Step 2: Initialize tracer
    if (Tracer::current_tracer == NULL) {
        // TODO Use config here
        Tracer::current_tracer = new Tracer();
    }

    Tracer* tracer = Tracer::current_tracer;

    // Step 3: Start the tracer
    tracer->Start();

    return 0;
}

