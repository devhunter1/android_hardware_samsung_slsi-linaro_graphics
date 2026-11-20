/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <drm/drm_mode.h>
#include "ExynosDeviceDrmInterface.h"
#include "ExynosHWCDebug.h"
#include <hardware/hwcomposer_defs.h>
#include "DeconDrmHeader.h"
#include "DrmDataType.h"

#if __has_include(<drm/exynos_drm_modifier.h>)
#  include <drm/exynos_drm_modifier.h>
#else
#  include "exynos_drm_modifier.h"
#endif

ExynosDeviceDrmInterface::ExynosDeviceDrmInterface()
    : mDrmDevice(nullptr) {
    mType = INTERFACE_TYPE_DRM;
}

ExynosDeviceDrmInterface::~ExynosDeviceDrmInterface() {
    if (mDrmDevice != nullptr)
        mDrmDevice->event_listener()->UnRegisterHotplugHandler(static_cast<DrmEventHandler *>(this));
}

void ExynosDeviceDrmInterface::init(void *deviceData, size_t &deviceDataSize) {
    if (deviceData == nullptr) {
        mDrmResourceManager.Init();
        mDrmDevice = mDrmResourceManager.GetDrmDevice(HWC_DISPLAY_PRIMARY);
        assert(mDrmDevice != NULL);

        mDrmDevice->event_listener()->RegisterHotplugHandler(static_cast<DrmEventHandler *>(this));
        mDrmDevice->event_listener()->RegisterPanelResetHandler(static_cast<DrmEventHandler *>(this));
        deviceDataSize = sizeof(DeviceInterfaceData);
        mFBManager.init(mDrmDevice->fd());
    } else {
        DeviceInterfaceData data(mDrmDevice);
        *(DeviceInterfaceData *)deviceData = data;
    }
    return;
}

int32_t ExynosDeviceDrmInterface::getRestrictions(struct dpp_restrictions_info_v2 *&restrictions, uint32_t otfMPPSize) {
    int32_t ret = 0;
    mDPUInfo.dpuInfo.dpp_cnt = mDrmDevice->planes().size();
    uint32_t channelId = 0;

    for (auto &plane : mDrmDevice->planes()) {
        /* Set size restriction information */
        if (plane->hw_restrictions_property().id()) {
            uint64_t blobId;
            std::tie(ret, blobId) = plane->hw_restrictions_property().value();
            if (ret)
                break;
            struct drm_dpp_ch_restriction *res;
            drmModePropertyBlobPtr blob = drmModeGetPropertyBlob(mDrmDevice->fd(), blobId);
            if (!blob) {
                ALOGE("Fail to get blob for hw_restrictions(%" PRId64 ")", blobId);
                ret = HWC2_ERROR_UNSUPPORTED;
                break;
            }
            res = (struct drm_dpp_ch_restriction *)blob->data;
            setDppChannelRestriction(mDPUInfo.dpuInfo.dpp_ch[channelId], *res);
            drmModeFreePropertyBlob(blob);
        } else if(plane->restrictions_property().id()) {
            uint64_t blobId;
            std::tie(ret, blobId) = plane->restrictions_property().value();
            if (ret)
                break;
            drmModePropertyBlobPtr blob = drmModeGetPropertyBlob(mDrmDevice->fd(), blobId);
            if (!blob) {
                ALOGE("Fail to get blob for restrictions(%" PRId64 ")", blobId);
                ret = HWC2_ERROR_UNSUPPORTED;
                break;
            }
            setDppChannelRestriction(mDPUInfo.dpuInfo.dpp_ch[channelId], blob);
            drmModeFreePropertyBlob(blob);
        } else {
            ALOGI("plane[%d] There is no hw restriction information", channelId);
            ret = HWC2_ERROR_UNSUPPORTED;
            break;
        }

        if (plane->format_property().id()) {
            uint64_t blobId;
            std::tie(ret, blobId) = plane->format_property().value();
            if (ret)
                break;
            struct drm_format_modifier_blob *res;
            drmModePropertyBlobPtr blob = drmModeGetPropertyBlob(mDrmDevice->fd(), blobId);
            if (!blob) {
                ALOGE("Fail to get blob for format_property(%" PRId64 ")", blobId);
                ret = HWC2_ERROR_UNSUPPORTED;
                break;
            }
            res = (struct drm_format_modifier_blob *)blob->data;
            ALOGD("version %d, count_formats : %d, format_offset : %x, count_modifiers : %d, modifiers offset : %x",
                  res->version, res->count_formats, res->formats_offset, res->count_modifiers, res->modifiers_offset);

            /* find format ptr */
            char *formatsPtr = (char *)(res);
            formatsPtr = formatsPtr + res->formats_offset;
            uint32_t *formats = (uint32_t *)formatsPtr;

            /* find modifier ptr */
            char *modifiersPtr = (char *)(res);
            modifiersPtr = modifiersPtr + res->modifiers_offset;
            drm_format_modifier *modifiers = (drm_format_modifier *)modifiersPtr;

            /* find SBWC support from modifier information */
            bool isSupportSBWC = false;
            for (uint32_t i = 0; i < res->count_modifiers; i++) {
                for (uint32_t j = 0; j < res->count_formats; j++) {
                    if (modifiers[i].formats & (1 << j)) {
                        ALOGD("prop modifier : drm format[%d](%c%c%c%c)", j,
                              formats[j], formats[j] >> 8, formats[j] >> 16, formats[j] >> 24);
                    }
                }
                ALOGD("ch : %d, prop : modifiers[%d] : formats[%llu], offset[%d], pad[%d], modifier[%llx]", channelId,
                      i, modifiers[i].formats, modifiers[i].offset, modifiers[i].pad, modifiers[i].modifier);
                if (modifiers[i].modifier & SBWC_IDENTIFIER)
                    isSupportSBWC = true;
            }

            /* find HAL format support that regarding modifier (e.g. SBWC) */
            for (uint32_t j = 0; j < res->count_formats; j++) {
                uint32_t halFormatNum = 0;
                uint32_t halFormats[MAX_SAME_HAL_PIXEL_FORMAT];
                int &formatIndex = mDPUInfo.dpuInfo.dpp_ch[channelId].restriction.format_cnt;
                drmFormatToHalFormats(formats[j], &halFormatNum, halFormats);
                for (uint32_t i = 0; i < halFormatNum; i++) {
                    ALOGD("prop : drm format[%d](%c%c%c%c), halFormat(%s)", j,
                          formats[j], formats[j] >> 8, formats[j] >> 16, formats[j] >> 24,
                          getFormatStr(halFormats[i]).c_str());
                    if (!isSupportSBWC &&
                        (isFormatSBWC(halFormats[i]) ||
                         (halFormats[i] == HAL_PIXEL_FORMAT_EXYNOS_420_SPN_SBWC_DECOMP) ||
                         (halFormats[i] == HAL_PIXEL_FORMAT_EXYNOS_P010_N_SBWC_DECOMP))) {
                        ALOGE("Ch %d is not support SBWC", channelId);
                        continue;
                    }
                    mDPUInfo.dpuInfo.dpp_ch[channelId].restriction.format[formatIndex] =
                        halFormats[i];
                    formatIndex++;
                }
            }

            drmModeFreePropertyBlob(blob);
        }

        if (hwcCheckDebugMessages(eDebugAttrSetting))
            printDppRestriction(mDPUInfo.dpuInfo.dpp_ch[channelId]);

        channelId++;
    }
    if (ret != NO_ERROR)
        ALOGI("Fail to get restriction (ret: %d)", ret);

    restrictions = &mDPUInfo.dpuInfo;
    return ret;
}

void ExynosDeviceDrmInterface::HandleEvent(uint64_t timestamp_us) {
    if (mHotplugHandler == NULL)
        return;
    mHotplugHandler->handleHotplug();
}

void ExynosDeviceDrmInterface::setDppChannelRestriction(struct dpp_ch_restriction &common_restriction,
                                                        struct drm_dpp_ch_restriction &drm_restriction) {
    common_restriction.id = drm_restriction.id;
    common_restriction.attr = drm_restriction.attr;
    common_restriction.restriction.src_f_w = drm_restriction.restriction.src_f_w;
    common_restriction.restriction.src_f_h = drm_restriction.restriction.src_f_h;
    common_restriction.restriction.src_w = drm_restriction.restriction.src_w;
    common_restriction.restriction.src_h = drm_restriction.restriction.src_h;
    common_restriction.restriction.src_x_align = drm_restriction.restriction.src_x_align;
    common_restriction.restriction.src_y_align = drm_restriction.restriction.src_y_align;
    common_restriction.restriction.dst_f_w = drm_restriction.restriction.dst_f_w;
    common_restriction.restriction.dst_f_h = drm_restriction.restriction.dst_f_h;
    common_restriction.restriction.dst_w = drm_restriction.restriction.dst_w;
    common_restriction.restriction.dst_h = drm_restriction.restriction.dst_h;
    common_restriction.restriction.dst_x_align = drm_restriction.restriction.dst_x_align;
    common_restriction.restriction.dst_y_align = drm_restriction.restriction.dst_y_align;
    common_restriction.restriction.blk_w = drm_restriction.restriction.blk_w;
    common_restriction.restriction.blk_h = drm_restriction.restriction.blk_h;
    common_restriction.restriction.blk_x_align = drm_restriction.restriction.blk_x_align;
    common_restriction.restriction.blk_y_align = drm_restriction.restriction.blk_y_align;
    common_restriction.restriction.src_h_rot_max = drm_restriction.restriction.src_h_rot_max;
    //common_restriction.restriction.src_w_rot_max = drm_restriction.restriction.src_w_rot_max;
    common_restriction.restriction.scale_down = drm_restriction.restriction.scale_down;
    common_restriction.restriction.scale_up = drm_restriction.restriction.scale_up;
    common_restriction.restriction.format_cnt = 0;

    /* scale ratio can't be 0 */
    if (common_restriction.restriction.scale_down == 0)
        common_restriction.restriction.scale_down = 1;
    if (common_restriction.restriction.scale_up == 0)
        common_restriction.restriction.scale_up = 1;
}
void ExynosDeviceDrmInterface::setDppChannelRestriction(struct dpp_ch_restriction &common_restriction,
                                                        drmModePropertyBlobPtr blob) {
    char *ptr = (char *)blob->data;
    int remaining = blob->length;
    std::map<dpu_res_key, uint32_t> u32_values;
    std::map<dpu_res_key, uint64_t> u64_values;
    std::map<dpu_res_key, __u32_range> u32_range_values;

    while (remaining > 0) {
        switch (*(int *)ptr) {
            case DPU_RES_TYPE_U32: {
                struct dpu_res_u32 *res = (struct dpu_res_u32 *)ptr;
                u32_values.emplace(res->key, res->val);
                ptr += sizeof(*res);
                remaining -= sizeof(*res);
                break;
            }
            case DPU_RES_TYPE_U64: {
                struct dpu_res_u64 *res = (struct dpu_res_u64 *)ptr;
                u64_values.emplace(res->key, res->val);
                ptr += sizeof(*res);
                remaining -= sizeof(*res);
                break;
            }
            case DPU_RES_TYPE_U32_RANGE: {
                struct dpu_res_u32_range *res = (struct dpu_res_u32_range *)ptr;
                u32_range_values.emplace(res->key, res->val);
                ptr += sizeof(*res);
                remaining -= sizeof(*res);
                break;
            }
            default:
                ALOGW("Unknown restriction type %d found with key %d", (dpu_res_type)*ptr,
                        (dpu_res_key)*(ptr+sizeof(dpu_res_type)));
                break;
        }
    }

    common_restriction.id = u32_values.at(DPU_RES_ID);
    common_restriction.port = u32_values.at(DPU_RES_PORT);
    common_restriction.dpuf_id = u32_values.at(DPU_RES_DPUF_ID);
    common_restriction.attr = u64_values.at(DPU_RES_ATTR);
    common_restriction.restriction.src_f_w = u32_range_values.at(DPU_RES_SRC_F_W);
    common_restriction.restriction.src_f_h = u32_range_values.at(DPU_RES_SRC_F_H);
    common_restriction.restriction.src_w = u32_range_values.at(DPU_RES_SRC_W);
    common_restriction.restriction.src_h = u32_range_values.at(DPU_RES_SRC_H);
    common_restriction.restriction.src_x_align = u32_values.at(DPU_RES_SRC_X_ALIGN);
    common_restriction.restriction.src_y_align = u32_values.at(DPU_RES_SRC_Y_ALIGN);
    common_restriction.restriction.dst_f_w = u32_range_values.at(DPU_RES_DST_F_W);
    common_restriction.restriction.dst_f_h = u32_range_values.at(DPU_RES_DST_F_H);
    common_restriction.restriction.dst_w = u32_range_values.at(DPU_RES_DST_W);
    common_restriction.restriction.dst_h = u32_range_values.at(DPU_RES_DST_H);
    common_restriction.restriction.dst_x_align = u32_values.at(DPU_RES_DST_X_ALIGN);
    common_restriction.restriction.dst_y_align = u32_values.at(DPU_RES_DST_Y_ALIGN);
    common_restriction.restriction.blk_w = u32_range_values.at(DPU_RES_BLK_W);
    common_restriction.restriction.blk_h = u32_range_values.at(DPU_RES_BLK_H);
    common_restriction.restriction.blk_x_align = u32_values.at(DPU_RES_BLK_X_ALIGN);
    common_restriction.restriction.blk_y_align = u32_values.at(DPU_RES_BLK_Y_ALIGN);
    common_restriction.restriction.src_h_rot_max = u32_values.at(DPU_RES_SRC_H_ROT_MAX);
    common_restriction.restriction.scale_down = u32_values.at(DPU_RES_SCALE_DOWN);
    common_restriction.restriction.scale_up = u32_values.at(DPU_RES_SCALE_UP);
    common_restriction.restriction.format_cnt = 0;

    /* scale ratio can't be 0 */
    if (common_restriction.restriction.scale_down == 0)
        common_restriction.restriction.scale_down = 1;
    if (common_restriction.restriction.scale_up == 0)
        common_restriction.restriction.scale_up = 1;
}

void ExynosDeviceDrmInterface::HandlePanelEvent(uint64_t timestamp_us) {
    if (mPanelResetHandler == NULL)
        return;
    mPanelResetHandler->handlePanelReset();
}
