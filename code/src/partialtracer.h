// The file describers Tracer, the utility to trace files

#ifndef PARTIALTRACER_H__
#define PARTIALTRACER_H__

#include <fvmclient.h>
#include <single/tracer.h>

class PartialTracer : public Tracer {
    public:
        // Constructors
        PartialTracer(const String& repo, const String& link_src,
                const String& link_dst, FVMClient *client);



    protected:
        virtual void Trace();
        void PartialCommit();

        String relative_path_;
        String link_dst_;

        FVMClient *client_;
};

#endif // PARTIALTRACER_H__

