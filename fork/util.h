#ifndef SISOP_2023A_UTIL_H
#define SISOP_2023A_UTIL_H

#include <unistd.h>

pid_t
fork_process(void);

int
create_pipe(int *fds);

ssize_t
readsome(int fd, void *buf, size_t count);

ssize_t
writesome(int fd, const void *buf, size_t count);

#endif //SISOP_2023A_UTIL_H
