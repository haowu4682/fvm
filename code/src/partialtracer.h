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

        virtual int Commit();

    protected:
        virtual int Commit(const String& branch_name);
        virtual int Checkout(const String& branch_name);

#if 0
        virtual void Abandon(const String& old_branch, const String& new_branch);
        virtual void Overwrite(const String& old_branch, const String& new_branch);
        virtual void Merge(const String& old_branch, const String& new_branch);
#endif

        String relative_path_;
        String link_dst_;

        FVMClient *client_;
};

#endif // PARTIALTRACER_H__

