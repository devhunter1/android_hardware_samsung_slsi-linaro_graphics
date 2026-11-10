/*
 *  ion.cpp
 *
 *  Copyright 2018 Samsung Electronics Co., Ltd.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <hardware/exynos/ion.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <log/log.h>
#include <stdatomic.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ion.h"
#include "ion_uapi.h"

/*
 * If vendor system heap is registered, all request of system heap
 * are allocated from vendor system heap instead of built-in system heap.
 * The vendor system heap is the same purpose allocator as the system heap,
 * but it is optimized and supports debugging function for exynos.
 */
#define EXYNOS_ION_HEAP_VENDOR_SYSTEM_MASK  (1 << 14)

#define ION_MAX_HEAP_COUNT 15

/*
 * ION heap names
 * The array index is the legacy heap id
 */
static const struct {
    const char *name;
    const char *dmaheap_name;
    unsigned int namelen;
} ion_heap_name[ION_MAX_HEAP_COUNT] = {
    {"ion_system_heap",    "system",                   15},
    {"crypto_heap",        "crypto",                   11},
    {"vfw_heap",           "reserved",                 8 }, /* no use for dma-heap */
    {"vstream_heap",       "system-secure-vstream",    12},
    {"[reserved]",         "reserved",                 0 }, /* reserved heap id. never use */
    {"vframe_heap",        "vframe",                   11},
    {"vscaler_heap",       "vscaler",                  12},
    {"vnfw_heap",          "reserved",                 9 }, /* no use for dma-heap */
    {"gpu_crc",            "gpu_crc",                  7 },
    {"gpu_buffer",         "system-secure-gpu_buffer", 10},
    {"camera_heap",        "camera",                   11},
    {"secure_camera_heap", "secure_camera",            18},
    {"[reserved]",         "reserved",                 0 }, /* reserved heap id. never use */
    {"ext_ui",             "ext_ui",                   6 },
    {"vendor_system_heap", "system",                   22 },
};

#define ION_NUM_HEAP_NAMES (unsigned int)(sizeof(ion_heap_name)/sizeof(ion_heap_name[0]))

#define ION_HEAP_TYPE_NONE INT_MAX

const char *exynos_ion_get_heap_name(unsigned int legacy_heap_id) {
    if (legacy_heap_id >= ION_NUM_HEAP_NAMES)
        return NULL;

    return ion_heap_name[legacy_heap_id].name;
}

class DefaultSystemInterface : public SystemInterface {
public:
    ~DefaultSystemInterface() { }
    int Open(const char *path) {
        return open(path, O_RDONLY | O_CLOEXEC);
    }
    int Ioctl(int fd, unsigned int cmd, void *data) {
        return ioctl(fd, cmd, data);
    }
    int Ioctl(int fd, unsigned int cmd) {
        return ioctl(fd, cmd);
    }
    int Close(int fd) {
        return close(fd);
    }
};

int DmabufExporter::legacy_free_handle(int ion_fd, int handle) {
    struct ion_handle_data data = { .handle = handle, };

    return systemInterface.Ioctl(ion_fd, ION_IOC_FREE, &data);
}

int DmabufExporter::alloc_legacy(int ion_fd, size_t len, unsigned int heap_mask, unsigned int flags) {
    int ret;
    struct ion_fd_data fd_data;
    struct ion_allocation_data alloc_data = {
        .len = len,
        .align = 0,
        .heap_id_mask = heap_mask,
        .flags = flags,
    };

    ret = systemInterface.Ioctl(ion_fd, ION_IOC_ALLOC, &alloc_data);
    if (ret < 0) {
        ALOGE("%s(%d, %zu, %#x, %#x) ION_IOC_ALLOC failed: %s", __func__,
              ion_fd, len, heap_mask, flags, strerror(errno));
        return -1;
    }

    fd_data.handle = alloc_data.handle;

    ret = systemInterface.Ioctl(ion_fd, ION_IOC_SHARE, &fd_data);
    legacy_free_handle(ion_fd, alloc_data.handle);
    if (ret < 0) {
        ALOGE("%s(%d, %zu, %#x, %#x) ION_IOC_SHARE failed: %s", __func__,
              ion_fd, len, heap_mask, flags, strerror(errno));
        return -1;
    }

    return fd_data.fd;
}

int DmabufExporter::query_heap_id(int ion_fd, unsigned int legacy_heap_mask) {
    unsigned int i, legacy_heap_id;
    struct ion_heap_query query;
    struct ion_exynos_heap_data data[ION_NUM_HEAP_IDS];

    memset(&data, 0, sizeof(data));
    memset(&query, 0, sizeof(query));

    query.cnt = ION_NUM_HEAP_IDS;
    query.heaps = (__u64)data;

    if (systemInterface.Ioctl(ion_fd, ION_IOC_HEAP_QUERY, &query) < 0) {
        ALOGE("%s: failed query heaps with ion_fd %d: %s",
              __func__, ion_fd, strerror(errno));
        return 0;
    }

    if (query.cnt > ION_NUM_HEAP_IDS)
        query.cnt = ION_NUM_HEAP_IDS;

    for (legacy_heap_id = 0; legacy_heap_id < ION_NUM_HEAP_NAMES; legacy_heap_id++) {
        if ((1 << legacy_heap_id) & legacy_heap_mask) {
            for (i = 0; i < ION_NUM_HEAP_IDS; i++) {
                if (!strcmp(data[i].name, ion_heap_name[legacy_heap_id].name))
                    return 1 << data[i].heap_id;
            }
        }
    }
    return 0;
}

int DmabufExporter::alloc_modern(int ion_fd, size_t len, unsigned int legacy_heap_mask, unsigned int flags) {
    int ret;
    struct ion_new_allocation_data data = {
        .len = len,
        .flags = flags,
        .heap_id_mask = 0,
    };

    if (legacy_heap_mask == EXYNOS_ION_HEAP_SYSTEM_MASK)
        data.heap_id_mask = query_heap_id(ion_fd, EXYNOS_ION_HEAP_VENDOR_SYSTEM_MASK);

    if (!data.heap_id_mask)
        data.heap_id_mask = query_heap_id(ion_fd, legacy_heap_mask);

    if (!data.heap_id_mask) {
        ALOGE("%s: unable to find heaps of heap_mask %#x", __func__, legacy_heap_mask);
        return -1;
    }

    ret = systemInterface.Ioctl(ion_fd, ION_IOC_NEW_ALLOC, &data);
    if (ret < 0) {
        ALOGE("%s(%d, %zu, %#x(%#x), %#x) failed: %s", __func__,
              ion_fd, len, legacy_heap_mask, data.heap_id_mask, flags, strerror(errno));
        return -1;
    }

    return (int)data.fd;
}

int DmabufExporter::alloc_dma_heap(size_t len, unsigned int legacy_heap_mask, unsigned int flags) {
    char path[MAX_HEAP_PATH];
    int id;

    strcpy(path, DmaHeapRoot);
    for (id = 0; id < ION_NUM_HEAP_NAMES; id++) {
        if (legacy_heap_mask & (1 << id)) {
            strcat(path, ion_heap_name[id].dmaheap_name);
            break;
        }
    }
    if (id == ION_NUM_HEAP_NAMES) {
        ALOGE("%s invalid heapmask (%zu, %#x, %#x)", __func__, len, legacy_heap_mask, flags);
        return -EINVAL;
    }

    /* Append heap name for flags */
    if (flags & ION_FLAG_PROTECTED)
        strcat(path, "-secure");
    else if (!(flags & ION_FLAG_CACHED))
        strcat(path, "-uncached");

    int ret, fd = systemInterface.Open(path);
    if (fd < 0) {
        ALOGE("%s No device for %s (%zu, %#x, %#x) failed: %s", __func__,
              path, len, legacy_heap_mask, flags, strerror(errno));
        return fd;
    }
    struct dma_heap_allocation_data data;

    data.fd = 0;
    data.len = len;
    data.fd_flags = O_RDWR | O_CLOEXEC;
    data.heap_flags = 0;

    ret = systemInterface.Ioctl(fd, DMA_HEAP_IOCTL_ALLOC, &data);
    if (ret < 0)
        ALOGE("%s Allocation failure for %s (%zu, %#x, %#x) failed: %s", __func__,
              path, len, legacy_heap_mask, flags, strerror(errno));
    else
        ret = data.fd;

    systemInterface.Close(fd);

    return ret;
}

int DmabufExporter::open(void) {
    if (version == DMAHEAP_VERSION)
        return 0;

    int fd = systemInterface.Open("/dev/ion");
    if (fd < 0)
        ALOGE("open /dev/ion failed: %s", strerror(errno));

    return fd;
}

int DmabufExporter::close(int fd) {
    if (version == DMAHEAP_VERSION)
        return 0;

    int ret = systemInterface.Close(fd);
    if (ret < 0)
        ALOGE("closing fd %d of /dev/ion failed: %s", fd, strerror(errno));
    return ret;
}

int DmabufExporter::alloc(int ion_fd, size_t len, unsigned int legacy_heap_mask, unsigned int flags) {
    int fd;

    if (version == DMAHEAP_VERSION)
        fd = alloc_dma_heap(len, legacy_heap_mask, flags);
    else if (version == ION_LEGACY_VERSION)
        fd = alloc_legacy(ion_fd, len, legacy_heap_mask, flags);
    else
        fd = alloc_modern(ion_fd, len, legacy_heap_mask, flags);

    return fd;
}

int DmabufExporter::trace_buffer(int fd)
{
    if (!dma_buf_trace_supported)
        return 0;

    if (systemInterface.Ioctl(fd, DMA_BUF_IOCTL_TRACK) < 0) {
        if (errno == ENOTTY) {
            dma_buf_trace_supported = false;
            return 0;
        }
        ALOGE("%s(%d) failed: %s", __func__, fd, strerror(errno));
        return -1;
    }

    return 0;
}

int DmabufExporter::untrace_buffer(int fd)
{
    if (!dma_buf_trace_supported)
        return 0;

    if (systemInterface.Ioctl(fd, DMA_BUF_IOCTL_UNTRACK) < 0) {
        ALOGE("%s(%d) failed: %s", __func__, fd, strerror(errno));
        return -1;
    }

    return 0;
}

int DmabufExporter::import_handle(int ion_fd, int fd, int* handle) {
    int ret;
    struct ion_fd_data data = {
        .fd = fd,
    };

    assert(handle == NULL);

    if (version != ION_LEGACY_VERSION) {
        if (trace_buffer(fd))
            return -1;
        /*
         * buffer fd is not a handle and they are maintained seperately.
         * But we provide buffer fd as the buffer handle to keep the libion
         * api compatible with the legacy users including gralloc.
         * We should gradually change all the legacy users in the near future.
         */
        *handle = fd;
        return 0;
    }

    ret = systemInterface.Ioctl(ion_fd, ION_IOC_IMPORT, &data);
    if (ret < 0) {
        ALOGE("%s(%d, %d) failed: %s", __func__, ion_fd, fd, strerror(errno));
        return -1;
    }
    *handle = data.handle;

    return 0;
}

int DmabufExporter::free_handle(int ion_fd, int handle) {
    int ret;

    if (version != ION_LEGACY_VERSION) {
        if (untrace_buffer(handle))
            return -1;
        return 0;
    }

    ret = legacy_free_handle(ion_fd, handle);
    if (ret < 0) {
        ALOGE("%s(%d, %d) failed: %s", __func__, ion_fd, handle, strerror(errno));
        return -1;
    }

    return 0;
}

int DmabufExporter::sync_fd(int ion_fd, int fd) {
    struct ion_fd_data data = {
        .fd = fd,
    };

    if (version != ION_LEGACY_VERSION)
        return 0;

    if (systemInterface.Ioctl(ion_fd, ION_IOC_SYNC, &data) < 0) {
        ALOGE("%s(%d, %d) failed: %s", __func__, ion_fd, fd, strerror(errno));
        return -1;
    }

    return 0;
}

int DmabufExporter::sync_fd_partial(int ion_fd, int fd, off_t offset, size_t len) {
    struct ion_fd_partial_data data = {
        .fd = fd,
        .offset = offset,
        .len = len
    };

    if (version != ION_LEGACY_VERSION)
        return 0;

    if (systemInterface.Ioctl(ion_fd, ION_IOC_SYNC_PARTIAL, &data) < 0) {
        ALOGE("%s(%d, %d, %lu, %zu) failed: %s", __func__, ion_fd, fd, offset, len, strerror(errno));
        return -1;
    }

    return 0;
}

int DmabufExporter::sync(int ion_fd, int fd, int direction, int sync) {
    if (version == ION_LEGACY_VERSION)
        return sync_fd(ion_fd, fd);

    struct dma_buf_sync data;

    direction &= (ION_SYNC_READ | ION_SYNC_WRITE);
    data.flags = sync | direction;

    if (systemInterface.Ioctl(fd, DMA_BUF_IOCTL_SYNC, &data) < 0) {
        ALOGE("%s(%d, %llu) failed: %m", __func__, fd, data.flags);
        return -1;
    }

    return 0;
}

unsigned int DmabufExporter::heap_mask(const char *heap_name) {
    unsigned int id;

    if (heap_name == NULL)
        return 0;

    if (version == ION_MODERN_VERSION) {
        struct ion_heap_query query;
        struct ion_exynos_heap_data data[ION_NUM_HEAP_IDS];
        unsigned int i;
        int ion_fd = systemInterface.Open("/dev/ion");

        if (ion_fd < 0) {
            ALOGE("%s: open /dev/ion failed: %s", __func__, strerror(errno));
            return 0;
        }

        memset(&data, 0, sizeof(data));
        memset(&query, 0, sizeof(query));
        query.cnt = ION_NUM_HEAP_IDS;
        query.heaps = (__u64)data;

        if (systemInterface.Ioctl(ion_fd, ION_IOC_HEAP_QUERY, &query) < 0) {
            ALOGE("%s: failed query heaps: %s", __func__, strerror(errno));
            systemInterface.Close(ion_fd);
            return 0;
        }
        systemInterface.Close(ion_fd);

        if (query.cnt > ION_NUM_HEAP_IDS)
            query.cnt = ION_NUM_HEAP_IDS;

        for (i = 0; i < query.cnt; i++) {
            if (!strncmp(data[i].name, heap_name, strlen(heap_name)))
                return 1 << data[i].heap_id;
        }
        return 0;
    }

    /*
     * Legacy ion (and dma-heap): the heap id is fixed and given by the
     * index in ion_heap_name[]. Unknown names fall back to the system heap.
     */
    for (id = 0; id < ION_NUM_HEAP_NAMES; id++) {
        if (ion_heap_name[id].namelen &&
            !strncmp(ion_heap_name[id].name, heap_name, strlen(heap_name)))
            return 1 << id;
    }
    return EXYNOS_ION_HEAP_SYSTEM_MASK;
}

DmabufExporter& getDefaultExporter(void) {
    static DefaultSystemInterface systemInterface;
    static DmabufExporter exporter(systemInterface);

    return exporter;
}

unsigned int exynos_ion_get_heap_mask(const char *heap_name) {
    return getDefaultExporter().heap_mask(heap_name);
}
int exynos_ion_open() {
    return getDefaultExporter().open();
}
int exynos_ion_close(int fd) {
    return getDefaultExporter().close(fd);
}
int exynos_ion_alloc(int ion_fd, size_t len, unsigned int heap_mask, unsigned int flags) {
    return getDefaultExporter().alloc(ion_fd, len, heap_mask, flags);
}
int exynos_ion_import_handle(int ion_fd, int fd, int* handle) {
    return getDefaultExporter().import_handle(ion_fd, fd, handle);
}
int exynos_ion_free_handle(int ion_fd, int handle) {
    return getDefaultExporter().free_handle(ion_fd, handle);
}
int exynos_ion_sync_fd(int ion_fd, int fd) {
    return getDefaultExporter().sync_fd(ion_fd, fd);
}
int exynos_ion_sync_fd_partial(int ion_fd, int fd, off_t offset, size_t len) {
    return getDefaultExporter().sync_fd_partial(ion_fd, fd, offset, len);
}
int exynos_ion_sync_start(int ion_fd, int fd, int direction) {
    return getDefaultExporter().sync(ion_fd, fd, direction, DMA_BUF_SYNC_START);
}
int exynos_ion_sync_end(int ion_fd, int fd, int direction) {
    return getDefaultExporter().sync(ion_fd, fd, direction, DMA_BUF_SYNC_END);
}
