/*
 * Copyright (C) 2021 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ION_H
#define _ION_H

#include <errno.h>
#include <log/log.h>

class SystemInterface {
public:
    virtual ~SystemInterface() { }
    virtual int Open(const char *path);
    virtual int Ioctl(int fd, unsigned int cmd, void *data);
    virtual int Ioctl(int fd, unsigned int cmd);
    virtual int Close(int fd);
};

#define MAX_HEAP_PATH 64
static const char DmaHeapRoot[] = "/dev/dma_heap/";

enum exp_version {
    UNKNOWN_VERSION,
    ION_MODERN_VERSION,
    ION_LEGACY_VERSION,
    DMAHEAP_VERSION,
};

class DmabufExporter {
public:
    DmabufExporter(SystemInterface &_systemInterface) : systemInterface(_systemInterface), dma_buf_trace_supported(true) {
        char path[MAX_HEAP_PATH];

        strcpy(path, DmaHeapRoot);
        strcat(path, "system");

        int fd = systemInterface.Open(path);

        if (fd >= 0) {
            version = DMAHEAP_VERSION;
        } else {
            fd = systemInterface.Open("/dev/ion");
            if (fd < 0) {
                ALOGE("%s Failed to find BOTH DMA-HEAP and ION", __func__);
                version = UNKNOWN_VERSION;
                return;
            }
            legacy_free_handle(fd, 0);
            version = (errno == ENOTTY) ? ION_MODERN_VERSION : ION_LEGACY_VERSION;
        }
        systemInterface.Close(fd);
    }
    int open();
    int close(int fd);
    int alloc(int ion_fd, size_t len, unsigned int legacy_heap_mask, unsigned int flags);
    int import_handle(int ion_fd, int fd, int* handle);
    int legacy_free_handle(int ion_fd, int handle);
    int free_handle(int ion_fd, int handle);
    int sync_fd(int ion_fd, int fd);
    int sync_fd_partial(int ion_fd, int fd, off_t offset, size_t len);
    int sync(int ion_fd, int fd, int direction, int sync);
    unsigned int heap_mask(const char *heap_name);
    int trace_buffer(int fd);
    int untrace_buffer(int fd);

private:
    int alloc_legacy(int ion_fd, size_t len, unsigned int legacy_heap_mask, unsigned int flags);
    int alloc_modern(int ion_fd, size_t len, unsigned int legacy_heap_mask, unsigned int flags);
    int alloc_dma_heap(size_t len, unsigned int legacy_heap_mask, unsigned int flags);
    int query_heap_id(int ion_fd, unsigned int legacy_heap_mask);

    SystemInterface &systemInterface;
    bool dma_buf_trace_supported;
    enum exp_version version;
};
#endif
