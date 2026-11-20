/*
 *  ion_uapi.h
 *
 *   Copyright 2018 Samsung Electronics Co., Ltd.
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

#ifndef __EXYNOS_ION_H__
#define __EXYNOS_ION_H__

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/ion.h>
#include <linux/ion_4.19.h>

#define DMA_BUF_IOCTL_TRACK    _IO('b', 8)
#define DMA_BUF_IOCTL_UNTRACK  _IO('b', 9)

#define ION_HEAP_TYPE_HPA ION_HEAP_TYPE_CUSTOM

struct ion_fd_partial_data {
    int handle;
    int fd;
    off_t offset;
    size_t len;
};

struct ion_exynos_heap_data {
    char name[MAX_HEAP_NAME];
    __u32 type;
    __u32 heap_id;
    __u32 size;       /* reserved 0 */
    __u32 heap_flags; /* reserved 1 */
    __u32 reserved2;
};

#define ION_HEAPDATA_FLAGS_DEFER_FREE		1
#define ION_HEAPDATA_FLAGS_ALLOW_PROTECTION	2
#define ION_HEAPDATA_FLAGS_UNTOUCHABLE		4

#define ION_IOC_SYNC_PARTIAL _IOWR(ION_IOC_MAGIC, 9, struct ion_fd_partial_data)

#endif /* __EXYNOS_ION_H__ */
