// Author: Hao Wu <haowu@cs.utexas.edu>

#ifndef COMMON_COMMON_H__
#define COMMON_COMMON_H__

#include <assert.h>
#include <bits/wordsize.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <sys/user.h>

// Constant definition
#define kNanosecondsToSeconds 1e-9

// Type definition
#define Vector std::vector
#define String std::string
#define Set std::set
#define Map std::map
#define Pair std::pair

// Common macros
#define EXEC_BLOCKED(path, args...) do {\
    pid_t pid; \
    int status; \
    pid = vfork(); \
    if (pid == 0) { \
        execl(path, ##args); \
    } else { \
        wait(&status); \
    } \
} while (0)

// Execute the command, and use pipes to get the result from stdout and stderr
#define EXEC_PIPED(stdout_fileno, stderr_fileno, path, args...) do {\
    pid_t pid; \
    int status; \
    int stdout_pipefd[2], stderr_pipefd[2]; \
\
    pipe(stdout_pipefd); \
    pipe(stderr_pipefd); \
\
    pid = vfork(); \
    if (pid == 0) { \
        close(stdout_pipefd[0]); \
        close(stderr_pipefd[0]); \
        dup2(stdout_pipefd[1], STDOUT_FILENO); \
        dup2(stderr_pipefd[1], STDERR_FILENO); \
        execl(path, ##args); \
    } else { \
        close(stdout_pipefd[1]); \
        close(stderr_pipefd[1]); \
        stdout_fileno = stdout_pipefd[0]; \
        stderr_fileno = stderr_pipefd[0]; \
        wait(&status); \
    } \
} while (0)

// Debugging tools
#define LOG(fmt,args...) do \
{ \
    struct timespec time; \
    double real_time; \
    clock_gettime(CLOCK_REALTIME, &time); \
    real_time = time.tv_sec + kNanosecondsToSeconds * time.tv_nsec; \
    fprintf(stderr, "%f:%s:%s:%d: " fmt "\n", real_time, __FILE__,__func__,__LINE__ ,##args); \
    fflush(stderr);\
    if (errno != 0 && errno != EAGAIN) \
    { \
        perror("errno"); \
        errno = 0; \
    } \
} while (0)

#ifdef DEBUG
#define DBG(fmt,args...) LOG("DEBUG " fmt,##args)
#else
#define DBG(fmt,args...)
#endif

#define LOG1(x) LOG("%s",x)
#define DBG1(x) DBG("%s",x)

#define ASSERT(x) assert(x)

#define TESTERROR(ret, msg, args...) do \
{ \
    if ((ret) < 0) \
    { \
        LOG(msg, ##args); \
        return (ret); \
    } \
} while (0)

// Macros
#define ARRAY_SIZE(a) sizeof(a)/sizeof((a)[0])

#endif //__COMMON_COMMON_H__

