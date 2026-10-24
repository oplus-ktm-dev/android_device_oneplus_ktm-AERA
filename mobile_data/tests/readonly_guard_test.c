/* SPDX-License-Identifier: Apache-2.0 */
#include "../readonly-guard.c"
#include <assert.h>

static char opened[256];
static unsigned opens, reads;
static bool forced_read_only;
struct rmtfd { unsigned id; };
static struct rmtfd client;

int __real_open(const char *path, int flags, ...)
{
    (void)flags;
    ++opens;
    snprintf(opened, sizeof(opened), "%s", path);
    errno = ENOENT;
    return -1;
}
int __real_openat(int dirfd, const char *path, int flags, ...)
{
    (void)dirfd;
    return __real_open(path, flags);
}
int __real_storage_init(const char *root, bool ro, bool partitions)
{
    (void)root; (void)partitions;
    forced_read_only = ro;
    return 0;
}
struct rmtfd *__real_storage_open(unsigned node, const char *path, const char *slot)
{
    (void)node; (void)path; (void)slot;
    return &client;
}
void __real_storage_close(struct rmtfd *fd) { (void)fd; }
ssize_t __real_storage_pread(struct rmtfd *fd, void *buf, size_t size, off_t offset)
{
    (void)fd; (void)offset;
    ++reads;
    memset(buf, 0, size);
    return size;
}
int main(void)
{
    const char *legacy = "/dev/block/bootdevice/by-name/oem_stanvbk";
    setenv("AERA_CELLULAR_SLOT", "_a", 1);
    __wrap_open(legacy, O_RDONLY);
    assert(opens == 1 && !strcmp(opened, "/dev/block/bootdevice/by-name/oplusstanvbk_a"));
    __wrap___open_2(legacy, O_RDONLY);
    assert(opens == 2 && !strcmp(opened, "/dev/block/bootdevice/by-name/oplusstanvbk_a"));
    unsigned before = opens;
    assert(__wrap_open(legacy, O_RDWR) == -1 && errno == EROFS && opens == before);
    assert(__wrap_open(legacy, O_RDONLY | O_TRUNC) == -1 && errno == EROFS);
    assert(__wrap___open_2(legacy, O_RDWR) == -1 && errno == EROFS && opens == before);
    assert(__wrap___openat_2(AT_FDCWD, legacy, O_RDWR) == -1 && errno == EROFS && opens == before);
    assert(__wrap___open_2(legacy, O_RDONLY | O_CREAT) == -1 && errno == EROFS);
    assert(__wrap_pwrite(-1, "x", 1, 0) == -1 && errno == EROFS);
    __wrap_storage_init("unused", false, true);
    assert(forced_read_only);
    setenv("AERA_CELLULAR_SLOT", "_b", 1);
    __wrap_open(legacy, O_RDONLY);
    assert(!strcmp(opened, "/dev/block/bootdevice/by-name/oplusstanvbk_b"));
    setenv("AERA_CELLULAR_SLOT", "_invalid", 1);
    before = opens;
    assert(__wrap_open(legacy, O_RDONLY) == -1 && errno == EINVAL && opens == before);
    __wrap_open("/dev/block/bootdevice/by-name/oem_dycnvbk", O_RDONLY);
    assert(!strcmp(opened, "/dev/block/bootdevice/by-name/oplusdycnvbk"));
    char page[512];
    __wrap_storage_open(0, "/oem/nvbk/static", "_a");
    assert(__wrap_storage_pread(&client, page, sizeof(page), 0) == 512 && reads == 1);
    assert(__wrap_storage_pread(&client, page, sizeof(page), 20 * 1024 * 1024) == -1);
    assert(reads == 1 && !strcmp(opened, "/tmp/aera-cellular/oem-miscinfo"));
    __wrap_storage_close(&client);
    assert(oem_static_client == NULL);
    puts("KTM NV mapping, blocked writes, RAM shadowing and fail-closed metadata passed");
}
