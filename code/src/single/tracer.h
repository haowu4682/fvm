// The file describers Tracer, the utility to trace files

#ifndef SINGLE_TRACER_H__
#define SINGLE_TRACER_H__

#include <pthread.h>

#include <common/common.h>
#include <single/config.h>
#include <vcs/vcs.h>

class Tracer {
    public:
        // The current running tracer in the system
        static Tracer *current_tracer;

        // Constructor(s)
        Tracer();
        Tracer(const RepoConfig &config);
        Tracer(const String& config_str);

        // Initialize the tracer thread
        virtual void Init();

        // Start the tracer
        virtual void Start();
        // Stop the tracer
        virtual void Stop();

    protected:
        // Whether the tracing is running.
        bool running_;

        // The thread for tracing
        pthread_t trace_thread_;

        // The config of the tracer
        RepoConfig config_;

        // Trace selected files using the config in config_
        void Trace();
        // The friend function is used for pthread only
        friend void * trace_pthread_function(void * /* tracer_void_ptr */);

        // The version control system
        VersionControlSystem* vcs;
};

#endif // SINGLE_TRACER_H__

