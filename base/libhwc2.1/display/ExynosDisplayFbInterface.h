/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef _EXYNOSDISPLAYFBINTERFACE_H
#define _EXYNOSDISPLAYFBINTERFACE_H

#include "ExynosDisplayInterface.h"
#include <sys/types.h>
#include <hardware/hwcomposer2.h>
#include <linux/videodev2.h>
#include "videodev2_exynos_displayport.h"
#include "DeconHeader.h"
#include "DeconFbHeader.h"
#include "ExynosHWCTypes.h"

/* dpu dataspace = trasnfer[13:9] | range[8:6] | standard[5:0] */
#define DPU_DATASPACE_STANDARD_SHIFT 0
#define DPU_DATASPACE_STANDARD_MASK 0x3f

#define DPU_DATASPACE_RANGE_SHIFT 6
#define DPU_DATASPACE_RANGE_MASK 0x1C0

#define DPU_DATASPACE_TRANSFER_SHIFT 9
#define DPU_DATASPACE_TRANSFER_MASK 0x3E00

#define HWC_CLEARDISPLAY_WITH_COLORMAP

template <typename T , typename = int>
struct hasFdLut : std::false_type {};

template <typename T>
struct hasFdLut <T , decltype((void)T::fd_lut , 0)> : std::true_type {};

template<typename T>
std::enable_if_t<hasFdLut<decon_win_config>::value, T>
setFdLut(T&& dConfig , int fd_lut) {
  dConfig->fd_lut = fd_lut;
  return 0;
}

template<typename T>
std::enable_if_t<!hasFdLut<decon_win_config>::value, T>
setFdLut(T&& __unused dConfig , int __unused fd_lut) {
  return 0;
}

template<typename T, typename U = int>
std::enable_if_t<hasFdLut<decon_win_config>::value, U>
getFdLut(T&& dConfig) {
  return dConfig->fd_lut;
}

template<typename T, typename U = int>
std::enable_if_t<!hasFdLut<decon_win_config>::value, U>
getFdLut(T&& __unused dConfig) {
  return -1;
}

class ExynosDisplayFbInterface : public ExynosDisplayInterface {
  public:
    ExynosDisplayFbInterface();
    ~ExynosDisplayFbInterface();
    virtual void init(const DisplayIdentifier &display, void *deviceData,
                      const size_t deviceDataSize) override;
    virtual int32_t setPowerMode(int32_t mode);
    virtual bool isDozeModeAvailable() const {
#ifdef USES_DOZEMODE
        return true;
#else
        return false;
#endif
    };
    virtual int32_t setVsyncEnabled(uint32_t enabled);
    virtual int32_t getDisplayConfigs(uint32_t *outNumConfigs, hwc2_config_t *outConfigs,
                                      std::map<uint32_t, displayConfigs_t> &displayConfigs);
    virtual void dumpDisplayConfigs();
    virtual int32_t getColorModes(
        uint32_t *outNumModes,
        int32_t *outModes);
    virtual int32_t setColorMode(int32_t mode, int32_t __unused dqe_fd);
    virtual int32_t getDPUConfig(hwc2_config_t *config);
    virtual int32_t setActiveConfig(ExynosDisplay &exynosDisplay, hwc2_config_t config, displayConfigs_t &displayConfig);
    virtual int32_t setActiveConfigWithConstraints(ExynosDisplay &exynosDisplay, hwc2_config_t desiredConfig,
                                                   displayConfigs_t &displayConfig, bool test = false);
    virtual int32_t getDisplayVsyncPeriod(hwc2_vsync_period_t *outVsyncPeriod);
    virtual int32_t getDisplayVsyncTimestamp(uint64_t *outVsyncTimestamp);
    virtual int32_t setCursorPositionAsync(uint32_t x_pos, uint32_t y_pos);
    virtual int32_t updateHdrCapabilities(std::vector<int32_t> &outTypes,
                                          float *outMaxLuminance, float *outMaxAverageLuminance, float *outMinLuminance);
    virtual int32_t deliverWinConfigData(exynos_dpu_data &dpuData);
    virtual int32_t clearDisplay();
    virtual int32_t disableSelfRefresh(uint32_t disable);
    virtual int32_t setForcePanic();
    virtual int getDisplayFd() { return mDisplayFd; };
    virtual uint32_t getMaxWindowNum();
    virtual decon_idma_type getDeconDMAType(uint32_t type, uint32_t index);

    virtual int32_t setColorTransform(const float *matrix, int32_t hint, int32_t __unused dqe_fd);
    virtual int32_t getRenderIntents(int32_t mode,
                                     uint32_t *outNumIntents, int32_t *outIntents);
    virtual int32_t setColorModeWithRenderIntent(int32_t mode, int32_t intent, int32_t __unused dqe_fd);
    virtual int32_t getReadbackBufferAttributes(int32_t * /*android_pixel_format_t*/ outFormat,
                                                int32_t * /*android_dataspace_t*/ outDataspace);
    virtual void setRepeaterBuffer(bool val);

    /* Implement this to module */
    virtual decon_idma_type getSubDeconDMAType(decon_idma_type channel);
    virtual int32_t preProcessForVirtual8K(struct decon_win_config *savedVirtualWinConfig);
    virtual int32_t postProcessForVirtual8K(struct decon_win_config savedVirtualWinConfig);
    int32_t mVirtual8KDPPIndex = -1;

    /* HWC 2.3 APIs */
    virtual int32_t getDisplayIdentificationData(uint8_t *outPort,
                                                 uint32_t *outDataSize, uint8_t *outData);
    int mDisplayFd = -1;
    virtual void setVsyncFd();
    virtual int getVsyncFd() const { return mVsyncFd; }
    int32_t getVsyncAndFps(uint64_t *timestamp, uint32_t *fps);
    virtual bool readHotplugStatus() { return false; };
    virtual void updateUeventNodeName(String8 __unused node){};
    virtual bool updateHdrSinkInfo() { return false; };

  protected:
    static void clearFbWinConfigData(decon_win_config_data &winConfigData);
    dpp_csc_eq halDataSpaceToDisplayParam(const exynos_win_config_data &config);
    virtual dpp_hdr_standard halTransferToDisplayParam(const exynos_win_config_data &config);
    String8 &dumpFbWinConfigInfo(String8 &result,
                                 decon_win_config_data &fbConfig, bool debugPrint = false);
    void setReadbackConfig(exynos_dpu_data &dpuData,
                           decon_win_config *config);
    void setStandAloneWritebackConfig(exynos_dpu_data &dpuData,
                                      decon_win_config *config);
    android_dataspace dpuDataspaceToHalDataspace(uint32_t dpu_dataspace);
    static android_dataspace dataspaceFromConfig(const exynos_win_config_data &config);
    virtual int32_t configFromDisplayConfig(decon_win_config &config,
                                            const exynos_win_config_data &display_config);
    virtual void alignDSCBlockSize(hwc_rect &merge_rect);
    virtual void updateDSCBlockSize(){};

  protected:
    /**
         * LCD device member variables
         */
    decon_win_config_data mFbConfigData;
    decon_edid_data mEdidData;
    int mVsyncFd = -1;
    ExynosFenceTracer &mFenceTracer = ExynosFenceTracer::getInstance();
    uint32_t mXres;
    uint32_t mYres;
    int32_t mDSCHSliceNum;
    int32_t mDSCYSliceSize;
};
#endif
