// CURRENT THIS IS NOT INCLUDED IN THE PROJECT
#ifndef BACKEND_BACKEND_H__
#define BACKEND_BACKEND_H__

#include <git2.h>

#include <common/common.h>

class Backend {
    public:
        Backend();

    protected:
        git_odb_backend odb_backend_;
        git_odb* odb_;
};

#endif // BACKEND_BACKEND_H__

