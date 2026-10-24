/* SPDX-License-Identifier: Apache-2.0 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "oem_backing.h"

char aera_rmtfs_uio_device[64];
int __real_storage_init(const char *root, bool read_only, bool partitions);
int __wrap_storage_init(const char *root, bool read_only, bool partitions)
{
    (void)read_only;
    return __real_storage_init(root, true, partitions);
}

int __real_open(const char *path, int flags, ...);
int __real_openat(int dirfd, const char *path, int flags, ...);

__attribute__((constructor)) static void init_logging(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

// The only writable device is the explicitly checked volatile exchange RAM.
int __wrap_open(const char *path, int flags, ...)
{
    if ((flags & (O_CREAT | O_TRUNC | O_APPEND)) ||
        ((flags & O_ACCMODE) != O_RDONLY &&
         (!aera_rmtfs_uio_device[0] || strcmp(path, aera_rmtfs_uio_device)))) {
        fprintf(stderr, "WRITE GUARD rejected open: %s\n", path);
        errno = EROFS;
        return -1;
    }
    if (!strcmp(path, "/dev/block/bootdevice/by-name/oem_stanvbk")) {
        const char *backing = ktm_static_nv_backing(getenv("AERA_CELLULAR_SLOT"));
        if (!backing) {
            errno = EINVAL;
            return -1;
        }
        path = backing;
    } else if (!strcmp(path, "/dev/block/bootdevice/by-name/oem_dycnvbk")) {
        path = "/dev/block/bootdevice/by-name/oplusdycnvbk";
    }
    return __real_open(path, flags);
}

int __wrap_openat(int dirfd, const char *path, int flags, ...)
{
    if ((flags & (O_CREAT | O_TRUNC | O_APPEND)) || (flags & O_ACCMODE) != O_RDONLY) {
        errno = EROFS;
        return -1;
    }
    return __real_openat(dirfd, path, flags);
}

// Bionic FORTIFY routes two-argument open/openat calls through these entry
// points. Guard them too: wrapping only open/openat misses Android callers.
int __wrap___open_2(const char *path, int flags)
{
    return __wrap_open(path, flags);
}

int __wrap___openat_2(int dirfd, const char *path, int flags)
{
    return __wrap_openat(dirfd, path, flags);
}

ssize_t __wrap_pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    (void)fd; (void)buf; (void)count; (void)offset;
    errno = EROFS;
    return -1;
}

struct rmtfd;
static struct rmtfd *oem_static_client;
struct rmtfd *__real_storage_open(unsigned node, const char *path, const char *slot);
struct rmtfd *__wrap_storage_open(unsigned node, const char *path, const char *slot)
{
    struct rmtfd *client = __real_storage_open(node, path, slot);
    if (client && !strcmp(path, "/oem/nvbk/static"))
        oem_static_client = client;
    return client;
}

void __real_storage_close(struct rmtfd *client);
void __wrap_storage_close(struct rmtfd *client)
{
    if (client == oem_static_client)
        oem_static_client = NULL;
    __real_storage_close(client);
}

ssize_t __real_storage_pread(struct rmtfd *, void *, size_t, off_t);
ssize_t __wrap_storage_pread(struct rmtfd *client, void *buf, size_t count, off_t offset)
{
    // Stock KTM exposes OEM miscinfo at virtual sector 40960. Keep that
    // extension entirely in RAM; never read beyond or extend the block device.
    if (client == oem_static_client && client && offset == 20 * 1024 * 1024 && count == 512) {
        struct stat info;
        int fd = __real_open("/tmp/aera-cellular/oem-miscinfo", O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
        if (fd < 0)
            return -1;
        ssize_t n = -1;
        if (!fstat(fd, &info) && S_ISREG(info.st_mode) && info.st_size == 512 &&
            info.st_uid == 0 && !(info.st_mode & 0077))
            n = read(fd, buf, count);
        close(fd);
        if (n != (ssize_t)count) {
            errno = EIO;
            return -1;
        }
        return n;
    }
    return __real_storage_pread(client, buf, count, offset);
}
