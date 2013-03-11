
// CURRENT THIS IS NOT INCLUDED IN THE PROJECT
#include <backend/backend.h>

Backend::Backend()
{
// TODO set up the following attributes
// git_odb
#if 0
// git_odb_backend
unsigned int version
git_odb *odb
int (* read)(void **, size_t *, git_otype *, struct git_odb_backend *, const git_oid *)
int (* read_prefix)(git_oid *, void **, size_t *, git_otype *, struct git_odb_backend *, const git_oid *, size_t)
int (* read_header)(size_t *, git_otype *, struct git_odb_backend *, const git_oid *)
int (* write)(git_oid *, struct git_odb_backend *, const void *, size_t, git_otype)
int (* writestream)(struct git_odb_stream **, struct git_odb_backend *, size_t, git_otype)
int (* readstream)(struct git_odb_stream **, struct git_odb_backend *, const git_oid *)
int (* exists)(struct git_odb_backend *, const git_oid *)
int (* refresh)(struct git_odb_backend *)
int (* foreach)(struct git_odb_backend *, git_odb_foreach_cb cb, void *payload)
int (* writepack)(struct git_odb_writepack **, struct git_odb_backend *, git_transfer_progress_callback progress_cb, void *progress_payload)
void (* free)(struct git_odb_backend *)
#endif
}

// Obsoleted
#if 0
// git_config_backend
unsigned int version
struct git_config *cfg
int (*open)(struct git_config_backend *, unsigned int level)
int (*get)(const struct git_config_backend *, const char *key, const git_config_entry **entry)
int (*get_multivar)(struct git_config_backend *, const char *key, const char *regexp, git_config_foreach_cb callback, void *payload)
int (*set)(struct git_config_backend *, const char *key, const char *value)
int (*set_multivar)(git_config_backend *cfg, const char *name, const char *regexp, const char *value)
int (*del)(struct git_config_backend *, const char *key)
int (*foreach)(struct git_config_backend *, const char *, git_config_foreach_cb callback, void *payload)
int (*refresh)(struct git_config_backend *)
void (*free)(struct git_config_backend *)
#endif

