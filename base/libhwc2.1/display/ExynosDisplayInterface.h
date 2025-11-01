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

#ifndef _EXYNOSDISPLAYINTERFACE_H
#define _EXYNOSDISPLAYINTERFACE_H

#include <sys/types.h>
#include <hardware/hwcomposer2.h>
#include <utils/Errors.h>
#include <vector>
#include "ExynosHWCHelper.h"
#include "ExynosMPP.h"
#include "DeconHeader.h"
#include "ExynosHWCTypes.h"
#include "ExynosHWCDebug.h"
#include "ExynosDpuData.h"

using namespace android;

struct DeviceToDisplayInterface {
    std::vector<ExynosMPP *> exynosMPPsForChannel;
};

class ExynosDisplayInterface {
  public:
    static void removeBuffer(const uint64_t &bufferId);
    virtual ~ExynosDisplayInterface();
    virtual void init(const DisplayIdentifier &display, void *__unused deviceData,
                      const size_t __unused deviceDataSize) { mDisplayIdentifier = display; };
    void updateDisplayInfo(DisplayInfo &dispInfo) { mDisplayInfo = dispInfo; };
    virtual int32_t setPowerMode(int32_t __unused mode) { return HWC2_ERROR_NONE; };
    virtual int32_t setVsyncEnabled(uint32_t __unused enabled) { return HWC2_ERROR_NONE; };
    virtual int32_t setLowPowerMode(bool __unused suspend) { return HWC2_ERROR_UNSUPPORTED; };
    virtual bool isDozeModeAvailable() const { return false; };
    virtual int32_t getDisplayConfigs(uint32_t *outNumConfigs, hwc2_config_t *outConfigs,
                                      std::map<uint32_t, displayConfigs_t> &displayConfigs);
    virtual void dumpDisplayConfigs(){};
    virtual int32_t getColorModes(
        uint32_t *outNumModes,
        int32_t *outModes);
    virtual int32_t setColorMode(int32_t __unused mode, int32_t __unused dqe_fd) { return HWC2_ERROR_NONE; };
    virtual int32_t setActiveConfig(ExynosDisplay __unused &exynosDisplay, hwc2_config_t __unused config,
                                    displayConfigs_t __unused &displayConfig) { return HWC2_ERROR_NONE; };
    virtual int32_t getDisplayVsyncPeriod(hwc2_vsync_period_t *__unused outVsyncPeriod) { return HWC2_ERROR_NONE; };
    virtual int32_t getDisplayVsyncTimestamp(uint64_t *__unused outVsyncTimestamp) { return HWC2_ERROR_NONE; };
    virtual int32_t getDPUConfig(hwc2_config_t *outConfig);
    virtual int32_t setCursorPositionAsync(uint32_t __unused x_pos,
                                           uint32_t __unused y_pos) { return HWC2_ERROR_NONE; };
    virtual int32_t updateHdrCapabilities(std::vector<int32_t> __unused &outTypes,
                                          float *__unused outMaxLuminance, float *__unused outMaxAverageLuminance,
                                          float *__unused outMinLuminance) { return HWC2_ERROR_NONE; };
    virtual int32_t deliverWinConfigData(exynos_dpu_data &__unused dpuData) { return NO_ERROR; };
    virtual int32_t clearDisplay() { return NO_ERROR; };
    virtual int32_t disableSelfRefresh(uint32_t __unused disable) { return NO_ERROR; };
    virtual int32_t setForcePanic();
    virtual int getDisplayFd() { return -1; };
    virtual uint32_t getMaxWindowNum() { return 0; };
    virtual int32_t setColorTransform(const float *__unused matrix,
                                      int32_t __unused hint, int32_t __unused dqe_fd) { return HWC2_ERROR_UNSUPPORTED; }
    virtual int32_t getRenderIntents(int32_t __unused mode, uint32_t *__unused outNumIntents,
                                     int32_t *__unused outIntents) { return HWC2_ERROR_NONE; }
    virtual int32_t setColorModeWithRenderIntent(int32_t __unused mode, int32_t __unused intent,
                                                 int32_t __unused dqe_fd) { return HWC2_ERROR_NONE; }
    virtual int32_t getReadbackBufferAttributes(int32_t * /*android_pixel_format_t*/ outFormat,
                                                int32_t * /*android_dataspace_t*/ outDataspace);
    virtual void setRepeaterBuffer(bool __unused val){};

    /* HWC 2.3 APIs */
    virtual int32_t getDisplayIdentificationData(uint8_t *__unused outPort,
                                                 uint32_t *__unused outDataSize, uint8_t *__unused outData) { return HWC2_ERROR_NONE; }
    virtual decon_idma_type getDeconDMAType(uint32_t __unused type, uint32_t __unused index) { return MAX_DECON_DMA_TYPE; }

    /* For HWC 2.4 APIs */
    virtual int32_t getVsyncAppliedTime(hwc2_config_t __unused configId,
                                        displayConfigs __unused &config, int64_t *__unused actualChangeTime) { return NO_ERROR; }
    virtual int32_t getConfigChangeDuration() { return 2; };
    virtual int32_t setActiveConfigWithConstraints(ExynosDisplay __unused &exynosDisplay, hwc2_config_t __unused config,
                                                   displayConfigs_t __unused &displayConfig, bool __unused test = false) { return NO_ERROR; };
    virtual void getDisplayHWInfo(uint32_t __unused &xres, uint32_t __unused &yres,
                                  int __unused &psrMode, std::vector<ResolutionInfo> __unused &resolutionInfo){};

    virtual void registerVsyncHandler(ExynosVsyncHandler *handle) { mVsyncHandler = handle; };
    virtual int getVsyncFd() const { return -1; };
    virtual void setDeviceToDisplayInterface(const struct DeviceToDisplayInterface __unused &initData){};
    virtual bool readHotplugStatus() { return true; };
    virtual void updateUeventNodeName(String8 __unused node){};
    virtual bool updateHdrSinkInfo() { return false; };

    virtual void onDisplayRemoved(){};
    virtual void onLayerDestroyed(hwc2_layer_t __unused layer){};
    virtual void onLayerCreated(hwc2_layer_t __unused layer){};
    virtual void onClientTargetDestroyed(void *__unused owner){};

    virtual void canDisableAllPlanes(__unused bool canDisable){};
    virtual uint64_t getWorkingVsyncPeriod() { return 0; };
    virtual hwc2_config_t getPreferredModeId() { return 0; };
    virtual void resetConfigRequestState(){};

  public:
    uint32_t mType = INTERFACE_TYPE_NONE;
    ExynosVsyncHandler *mVsyncHandler = NULL;
    DisplayIdentifier mDisplayIdentifier;
    DisplayInfo mDisplayInfo;
    bool mIsHdrSink = false;
};

#endif
