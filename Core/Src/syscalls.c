/**
 *******************************************************************************
 * @file  syscalls.c
 * @brief Minimal newlib syscall stubs for bare-metal builds.
 *******************************************************************************
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

extern char __HeapBase;
extern char __HeapLimit;

int _close(int file)
{
    (void)file;
    errno = ENOSYS;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

int _write(int file, const char *ptr, int len)
{
    (void)file;
    (void)ptr;
    return len;
}

void *_sbrk(ptrdiff_t incr)
{
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == NULL) {
        heap_end = &__HeapBase;
    }

    prev_heap_end = heap_end;
    if ((heap_end + incr) > &__HeapLimit) {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end += incr;
    return prev_heap_end;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _getpid(void)
{
    return 1;
}
