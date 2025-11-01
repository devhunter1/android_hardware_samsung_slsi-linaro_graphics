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

#ifndef _EXYNOSDISPLAYDRMINTERFACE_H
#define _EXYNOSDISPLAYDRMINTERFACE_H

#include <samsung_drm.h>
#include <xf86drmMode.h>

#include <unordered_map>

#include "ExynosDisplay.h"
#include "ExynosDisplayInterface.h"
#include "ExynosHWC.h"
#include "ExynosMPP.h"
#include "ExynosHWCTypes.h"
#include "ExynosDrmFramebufferManager.h"
#include "drmconnector.h"
#include "drmcrtc.h"
#include "vsyncworker.h"

#ifndef HWC_FORCE_PANIC_PATH
#define HWC_FORCE_PANIC_PATH "/d/dri/1/crtc-0/panic"
#endif

using namespace android;
using DrmPropertyMap = std::unordered_map<uint32_t, uint64_t>;

enum drm_virtual8k_split {
    EXYNOS_SPLIT_NONE = 0,
    EXYNOS_SPLIT_LEFT,
    EXYNOS_SPLIT_RIGHT,
    EXYNOS_SPLIT_TOP,
    EXYNOS_SPLIT_BOTTOM
};

class ExynosDisplayDrmInterface : public ExynosDisplayInterface,
                                  public VsyncCallback {
  public:
    class DrmModeAtomicReq {
      public:
        DrmModeAtomicReq(){};
        DrmModeAtomicReq(ExynosDisplayDrmInterface *displayInterface);
        ~DrmModeAtomicReq();

        DrmModeAtomicReq(const DrmModeAtomicReq &) = delete;
        DrmModeAtomicReq &operator=(const DrmModeAtomicReq &) = delete;

        void init(ExynosDisplayDrmInterface *displayInterface);
        drmModeAtomicReqPtr pset() { return mPset; };
        void reset();
        void setError(int err) { mError = err; };
        int getError() { return mError; };
        int32_t atomicAddProperty(const uint32_t id,
                                  const DrmProperty &property,
                                  uint64_t value, bool optional = false);
        String8 &dumpAtomicCommitInfo(String8 &result, bool debugPrint = false);
        int commit(uint32_t flags, bool loggingForDebug = false);
        void addOldBlob(uint32_t blob_id) {
            mOldBlobs.push_back(blob_id);
        };
        int destroyOldBlobs() {
            for (auto &blob : mOldBlobs) {
                int ret = mDrmDisplayInterface->mDrmDevice->DestroyPropertyBlob(blob);
                if (ret) {
                    HWC_LOGE(mDrmDisplayInterface->mDisplayIdentifier,
                             "Failed to destroy old blob after commit %d", ret);
                    return ret;
                }
            }
            mOldBlobs.clear();
            return NO_ERROR;
        };

      private:
        drmModeAtomicReqPtr mPset = nullptr;
        int mError = 0;
        ExynosDisplayDrmInterface *mDrmDisplayInterface = NULL;
        /* Destroy old blobs after commit */
        std::vector<uint32_t> mOldBlobs;
        int drmFd() const { return mDrmDisplayInterface->mDrmDevice->fd(); }
    };
    void Callback(int display, int64_t timestamp) override;

    ExynosDisplayDrmInterface();
    ~ExynosDisplayDrmInterface();
    virtual void init(const DisplayIdentifier &display, void *__unused deviceData,
                      const size_t deviceDataSize) override;
    virtual int32_t setPowerMode(int32_t mode);
    virtual int32_t setLowPowerMode(bool suspend) override;
    virtual bool isDozeModeAvailable() const {
        return (mDozeDrmModes.size() != 0) ? true : false;
    };
    virtual int32_t setVsyncEnabled(uint32_t enabled);
    virtual int32_t getDisplayConfigs(uint32_t *outNumConfigs, hwc2_config_t *outConfigs,
                                      std::map<uint32_t, displayConfigs_t> &displayConfigs);
    virtual void dumpDisplayConfigs();
    virtual int32_t getColorModes(
        uint32_t *outNumModes,
        int32_t *outModes);
    virtual int32_t setColorTransform(const float *__unused matrix,
                                      int32_t __unused hint, int32_t __unused dqe_fd);
    virtual int32_t setColorMode(int32_t mode, int32_t __unused dqe_fd);
    virtual int32_t setColorModeWithRenderIntent(int32_t __unused mode, int32_t __unused intent,
                                                 int32_t __unused dqe_fd);
    virtual int32_t getDPUConfig(hwc2_config_t *config) override;
    virtual int32_t setActiveConfig(ExynosDisplay &exynosDisplay, hwc2_config_t config,
                                    displayConfigs_t __unused &displayConfig);
    virtual int32_t setCursorPositionAsync(uint32_t x_pos, uint32_t y_pos);
    virtual int32_t updateHdrCapabilities(std::vector<int32_t> &outTypes,
                                          float *outMaxLuminance, float *outMaxAverageLuminance, float *outMinLuminance);
    virtual int32_t deliverWinConfigData(exynos_dpu_data &dpuData);
    virtual int32_t clearDisplay();
    virtual int32_t disableSelfRefresh(uint32_t disable);
    virtual int32_t setForcePanic();
    virtual int getDisplayFd() { return mDrmDevice->fd(); };
    virtual void initDrmDevice(DrmDevice *drmDevice, int drmDisplayId);
    virtual uint32_t getMaxWindowNum();
    virtual int32_t getReadbackBufferAttributes(int32_t * /*android_pixel_format_t*/ outFormat,
                                                int32_t * /*android_dataspace_t*/ outDataspace);
    virtual int32_t getDisplayIdentificationData(uint8_t *outPort,
                                                 uint32_t *outDataSize, uint8_t *outData);

    /* For HWC 2.4 APIs */
    virtual int32_t getDisplayVsyncPeriod(hwc2_vsync_period_t *outVsyncPeriod);
    virtual int32_t getConfigChangeDuration();
    virtual int32_t getVsyncAppliedTime(hwc2_config_t configId, displayConfigs &config,
                                        int64_t *actualChangeTime);
    virtual int32_t setActiveConfigWithConstraints(ExynosDisplay __unused &exynosDisplay,
        hwc2_config_t config, displayConfigs_t &displayConfig, bool test = false);
    virtual void getDisplayHWInfo(uint32_t __unused &xres, uint32_t __unused &yres, int &psrMode,
                                  std::vector<ResolutionInfo> &resolutionInfo);

    virtual void setDeviceToDisplayInterface(const struct DeviceToDisplayInterface &initData);
    virtual void onDisplayRemoved() override;
    virtual void onLayerDestroyed(hwc2_layer_t layer) override {
        mFBManager.removeBuffersForOwner((void *)layer);
    };
    virtual void onLayerCreated(hwc2_layer_t __unused outLayer) override {
        mFBManager.cleanupSignal();
    };
    virtual void onClientTargetDestroyed(void *owner) override {
        mFBManager.removeBuffersForOwner(owner);
    };

    struct virtual8KOTFHalfInfo {
        int32_t channelId = -1;
        exynos_win_config_data config;
    };
    struct virtual8KOTFInfo {
        virtual8KOTFHalfInfo firstHalf;
        virtual8KOTFHalfInfo lastHalf;
    };
    bool getVirtual8KOtfInfo(exynos_win_config_data &config,
                             virtual8KOTFInfo &virtualOtfInfo);
    virtual bool readHotplugStatus();
    virtual bool updateHdrSinkInfo();
    virtual void canDisableAllPlanes(bool canDisable) {
        mCanDisableAllPlanes = canDisable;
    }
    virtual uint64_t getWorkingVsyncPeriod() override {
        if (mWorkingVsyncPeriod)
            return 1000000000 / mWorkingVsyncPeriod;
        return 0;
    };
    virtual void resetConfigRequestState() override {
        mDesiredModeState.needs_modeset = false;
    };

  protected:
    struct ModeState {
        bool needs_modeset = false;
        DrmMode mode;
        uint32_t blob_id = 0;
        uint32_t old_blob_id = 0;
        void setMode(const DrmMode newMode, const uint32_t modeBlob,
                     DrmModeAtomicReq &drmReq) {
            drmReq.addOldBlob(old_blob_id);
            mode = newMode;
            old_blob_id = blob_id;
            blob_id = modeBlob;
        };
        void reset() {
            *this = {};
        };
        void apply(ModeState &toModeState, DrmModeAtomicReq &drmReq) {
            toModeState.setMode(mode, blob_id, drmReq);
            drmReq.addOldBlob(old_blob_id);
            reset();
        };
    };
    struct ColorRequest {
        bool hasRequest = false;
        int32_t dqe_fd = -1;
        int32_t color_mode = -1;
        int32_t render_intent = -1;
        void requestTransform(int32_t fd) {
            hasRequest = true;
            dqe_fd = fd;
        }
        void requestColorMode(int32_t fd, int32_t mode, int32_t intent = -1) {
            hasRequest = true;
            dqe_fd = fd;
            color_mode = mode;
            render_intent = intent;
        }
        void apply(DrmModeAtomicReq &drmReq, DrmCrtc *drmCrtc) {
            if (!hasRequest)
                return;
            if (dqe_fd >= 0) {
                if (drmReq.atomicAddProperty(drmCrtc->id(),
                                             drmCrtc->dqe_fd_property(), dqe_fd) < 0) {
                    ALOGE("%s:: set dqe_fd property failed", __func__);
                }
            }
            if (color_mode >= 0) {
                if (drmReq.atomicAddProperty(drmCrtc->id(),
                                             drmCrtc->color_mode_property(), color_mode) < 0) {
                    ALOGE("%s:: set color mode property failed", __func__);
                }
            }
            if (render_intent >= 0) {
                if (drmReq.atomicAddProperty(drmCrtc->id(),
                                             drmCrtc->render_intent_property(), render_intent) < 0) {
                    ALOGE("%s:: set render_intent property failed", __func__);
                }
            }
            hasRequest = false;
            dqe_fd = -1;
            color_mode = -1;
            render_intent = -1;
        }
    };

    int32_t createModeBlob(const DrmMode &mode, uint32_t &modeBlob);
    int32_t setDisplayMode(DrmModeAtomicReq &drmReq, const uint32_t modeBlob);
    int32_t chosePreferredConfig();
    static std::tuple<uint64_t, int> halToDrmEnum(
        const int32_t halData, const DrmPropertyMap &drmEnums);
    /*
     * This function adds FB and gets new fb id if fbId is 0,
     * if fbId is not 0, this reuses fbId.
     */
    int32_t setupCommitFromDisplayConfig(DrmModeAtomicReq &drmReq,
                                         const exynos_win_config_data &config,
                                         const uint32_t configIndex,
                                         const std::unique_ptr<DrmPlane> &plane,
                                         uint32_t &fbId);

    int32_t setFrameStaticMeta(DrmModeAtomicReq &drmReq,
                               const exynos_win_config_data &config);

    int32_t setupPartialRegion(exynos_dpu_data &dpuData,
                               DrmModeAtomicReq &drmReq);

    static void parseEnums(const DrmProperty &property,
                           const std::vector<std::pair<uint32_t, const char *>> &enums,
                           DrmPropertyMap &out_enums);
    void parseBlendEnums(const DrmProperty &property);
    void parseStandardEnums(const DrmProperty &property);
    void parseTransferEnums(const DrmProperty &property);
    void parseRangeEnums(const DrmProperty &property);
    void parseColorModeEnums(const DrmProperty &property);
    void parsePanelTypeEnums(const DrmProperty &property);
    void parseVirtual8kEnums(const DrmProperty &property);

    void disablePlanes(DrmModeAtomicReq &drmReq,
                       uint32_t *planeEnableInfo = nullptr);

    int32_t setupConcurrentWritebackCommit(exynos_dpu_data &dpuData,
                                           DrmModeAtomicReq &drmReq);
    int32_t setupStandaloneWritebackCommit(exynos_dpu_data &dpuData,
                                           DrmModeAtomicReq &drmReq);
    void flipFBs(bool isActiveCommit);
    int32_t setWorkingVsyncPeriodProp(DrmModeAtomicReq &drmReq);
    hwc2_config_t getPreferredModeId() { return mPreferredModeId; };

  private:
    int32_t getLowPowerDrmModeModeInfo();
    int32_t setActiveDrmMode(DrmMode const &mode);
    int32_t getBufferId(const exynos_win_config_data &config, uint32_t &fbId,
                        const bool useCache);
    int32_t clearActiveDrmMode();

  protected:
    struct PartialRegionState {
        struct drm_clip_rect partial_rect = {0, 0, 0, 0};
        uint32_t blob_id = 0;
        bool isUpdated(drm_clip_rect rect) {
            return ((partial_rect.x1 != rect.x1) ||
                    (partial_rect.y1 != rect.y1) ||
                    (partial_rect.x2 != rect.x2) ||
                    (partial_rect.y2 != rect.y2));
        };
    };

  protected:
    class DrmWritebackInfo {
      public:
        void init(DrmDevice *drmDevice, uint32_t displayId);
        DrmConnector *getWritebackConnector() { return mWritebackConnector; };
        void pickFormatDataspace();
        static constexpr uint32_t PREFERRED_READBACK_FORMAT =
            HAL_PIXEL_FORMAT_RGBA_8888;
        uint32_t mWritebackFormat = HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;

      private:
        DrmDevice *mDrmDevice = NULL;
        DrmConnector *mWritebackConnector = NULL;
        std::vector<uint32_t> mSupportedFormats;
    };
    DrmDevice *mDrmDevice;
    DrmCrtc *mDrmCrtc;
    DrmConnector *mDrmConnector;
    VSyncWorker mDrmVSyncWorker;
    ModeState mActiveModeState;
    ModeState mDesiredModeState;
    PartialRegionState mPartialRegionState;
    /* Mapping plane id to ExynosMPP, key is plane id */
    std::unordered_map<uint32_t, ExynosMPP *> mExynosMPPsForPlane;

    DrmPropertyMap mBlendEnums;
    DrmPropertyMap mStandardEnums;
    DrmPropertyMap mTransferEnums;
    DrmPropertyMap mRangeEnums;
    DrmPropertyMap mPanelTypeEnums;
    DrmPropertyMap mColorModeEnums;
    DrmPropertyMap mVirtual8kEnums;

    DrmWritebackInfo mWritebackInfo;

    FramebufferManager &mFBManager = FramebufferManager::getInstance();

    DrmModeAtomicReq mDrmReq;
    ColorRequest mColorRequest;

  private:
    std::unordered_map</*mode id*/ uint32_t, DrmMode> mDozeDrmModes;
    ExynosFenceTracer &mFenceTracer = ExynosFenceTracer::getInstance();
    std::list<uint32_t> mOldFbIds;
    std::list<uint32_t> mFbIds;
    bool mCanDisableAllPlanes = false;
    uint64_t mWorkingVsyncPeriod = 0;
    hwc2_config_t mPreferredModeId = 0;
};

#endif
