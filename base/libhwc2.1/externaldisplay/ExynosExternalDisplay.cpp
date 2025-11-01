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

#include <hardware/hwcomposer_defs.h>
#include "ExynosExternalDisplay.h"
#include <errno.h>
#include "ExynosLayer.h"
#include "ExynosHWCHelper.h"
#include "ExynosHWCDebug.h"
#include "ExynosDisplayFbInterfaceModule.h"
#include "ExynosExternalDisplayFbInterfaceModule.h"
#include "ExynosDisplayDrmInterface.h"
#include "ExynosFenceTracer.h"
#include <linux/fb.h>

#define SKIP_FRAME_COUNT 3
extern struct exynos_hwc_control exynosHWCControl;

ExynosExternalDisplay::ExynosExternalDisplay(DisplayIdentifier node)
    : ExynosDisplay(node) {
    DISPLAY_LOGD(eDebugExternalDisplay, "");

    mDisplayControl.cursorSupport = true;

    mEnabled = false;
    mBlanked = false;

    mXres = 0;
    mYres = 0;
    mXdpi = 0;
    mYdpi = 0;
    mVsyncPeriod = 0;
    mSkipStartFrame = 0;
    mSkipFrameCount = -1;
    mIsSkipFrame = false;
    mVirtualDisplayState = 0;

    //TODO : Hard coded currently
    mNumMaxPriorityAllowed = 1;
    mPowerModeState = (hwc2_power_mode_t)HWC_POWER_MODE_OFF;
    mBaseWindowIndex = node.index * 2;
    mMaxWindowNum = PRIMARY_MAIN_EXTERNAL_WINCNT;

    mSinkHdrSupported = false;
    mHpdStatus = false;

    mEventNodeName.appendFormat(DP_CABLE_STATE_NAME, DP_LINK_NAME, mIndex);
}

ExynosExternalDisplay::~ExynosExternalDisplay() {
}

int ExynosExternalDisplay::openExternalDisplay() {
    DISPLAY_LOGD(eDebugExternalDisplay, "");

    int ret = setVsyncEnabledInternal(HWC2_VSYNC_ENABLE);
    if (ret != HWC2_ERROR_NONE) {
        DISPLAY_LOGE("Fail to open ExternalDisplay(%d)", ret);
        return ret;
    }
    mVsyncCallback.enableVSync(true);

    mSkipFrameCount = SKIP_FRAME_COUNT;
    mSkipStartFrame = 0;
    mActiveConfig = 0;
    mPlugState = true;

    if (mLayers.size() != 0) {
        mLayers.clear();
    }

    DISPLAY_LOGD(eDebugExternalDisplay, "open fd for External Display(%d)", ret);

    return ret;
}

void ExynosExternalDisplay::closeExternalDisplay() {
    DISPLAY_LOGD(eDebugExternalDisplay, "");

    setVsyncEnabledInternal(HWC2_VSYNC_DISABLE);
    mVsyncCallback.enableVSync(false);

    if (mPowerModeState != (hwc2_power_mode_t)HWC_POWER_MODE_OFF) {
        if (mDisplayInterface->setPowerMode(HWC_POWER_MODE_OFF) < 0) {
            DISPLAY_LOGE("%s: set powermode ioctl failed errno : %d", __func__, errno);
            return;
        }
    }

    mPowerModeState = (hwc2_power_mode_t)HWC_POWER_MODE_OFF;

    DISPLAY_LOGD(eDebugExternalDisplay, "Close fd for External Display");

    mPlugState = false;
    mBlanked = false;
    mSkipFrameCount = SKIP_FRAME_COUNT;

    for (size_t i = 0; i < mLayers.size(); i++) {
        ExynosLayer *layer = mLayers[i];
        layer->mAcquireFence = mFenceTracer.fence_close(layer->mAcquireFence,
                                                        mDisplayInfo.displayIdentifier, FENCE_TYPE_SRC_ACQUIRE, FENCE_IP_LAYER,
                                                        "ext::closeExternalDisplay: layer acq_fence");
        layer->mReleaseFence = -1;
        layer->mLayerBuffer = NULL;
    }

    mDisplayConfigs.clear();
}

int ExynosExternalDisplay::getDisplayConfigs(uint32_t *outNumConfigs, hwc2_config_t *outConfigs) {
    DISPLAY_LOGD(eDebugExternalDisplay, "");

    int32_t ret = mDisplayInterface->getDisplayConfigs(outNumConfigs, outConfigs, mDisplayConfigs);
    if (ret)
        DISPLAY_LOGE("%s:: failed to getDisplayConfigs, ret(%d)", __func__, ret);

    if (outConfigs) {
        mActiveConfig = outConfigs[0];
        displayConfigs_t displayConfig = mDisplayConfigs[mActiveConfig];
        mXres = displayConfig.width;
        mYres = displayConfig.height;
        mVsyncPeriod = displayConfig.vsyncPeriod;
        if (mDisplayInterface->mType == INTERFACE_TYPE_DRM) {
            int32_t ret2 = mDisplayInterface->setActiveConfig(*this, mActiveConfig, displayConfig);
            if (ret2) {
                DISPLAY_LOGE("%s:: failed to setActiveConfigs, ret(%d)", __func__, ret2);
                return ret2;
            }
        }
    }
    return ret;
}

int32_t ExynosExternalDisplay::getActiveConfig(hwc2_config_t *outConfig) {
    DISPLAY_LOGD(eDebugExternalDisplay, "");
    *outConfig = mActiveConfig;
    return HWC2_ERROR_NONE;
}

bool ExynosExternalDisplay::handleRotate() {
    if (mSkipStartFrame < SKIP_EXTERNAL_FRAME) {
        mIsSkipFrame = false;
        return false;
    }

    mIsSkipFrame = false;
    return false;
}

bool ExynosExternalDisplay::checkRotate() {
    return false;
}

int32_t ExynosExternalDisplay::preProcessValidate(
    DeviceValidateInfo &validateInfo,
    uint64_t &geometryChanged) {
    Mutex::Autolock lock(mExternalMutex);
    DISPLAY_LOGD(eDebugExternalDisplay, "");

    mNeedSkipPresent = false;
    if (mSkipStartFrame < SKIP_EXTERNAL_FRAME) {
        ALOGI("[ExternalDisplay] %s : Skip %d th start frame", __func__, mSkipStartFrame);
        initDisplay();
        /*
         * geometry should be set before ExynosDisplay::validateDisplay is called
         * not to skip resource assignment
         */
        if (mPlugState)
            setGeometryChanged(GEOMETRY_DEVICE_DISPLAY_ADDED, geometryChanged);
        else
            setGeometryChanged(GEOMETRY_DEVICE_DISPLAY_REMOVED, geometryChanged);
    }

    if (handleRotate() || (mPlugState == false)) {
        setGeometryChanged(GEOMETRY_LAYER_UNKNOWN_CHANGED, geometryChanged);
        mClientCompositionInfo.initializeInfos();
        mExynosCompositionInfo.initializeInfos();
        mRenderingState = RENDERING_STATE_VALIDATED;
        mRenderingStateFlags.validateFlag = true;
        mIsSkipFrame = true;
        return HWC2_ERROR_NONE;
    }

    if (mSkipStartFrame < SKIP_EXTERNAL_FRAME) {
        /*
         * Set mIsSkipFrame before calling ExynosDisplay::validateDisplay()
         * startPostProcessing() that is called by ExynosDisplay::validateDisplay()
         * checks mIsSkipFrame.
         */
        mIsSkipFrame = true;
    }

    return ExynosDisplay::preProcessValidate(validateInfo, geometryChanged);
}

int32_t ExynosExternalDisplay::postProcessValidate() {
    Mutex::Autolock lock(mExternalMutex);
    DISPLAY_LOGD(eDebugExternalDisplay, "");

    int32_t ret = ExynosDisplay::postProcessValidate();

    if (mSkipStartFrame < SKIP_EXTERNAL_FRAME) {
        initDisplay();
        mRenderingState = RENDERING_STATE_VALIDATED;
        for (size_t i = 0; i < mLayers.size(); i++) {
            ExynosLayer *layer = mLayers[i];
            if (layer) {
                layer->mValidateCompositionType = HWC2_COMPOSITION_CLIENT;
                layer->mReleaseFence = layer->mAcquireFence;
            }
        }

        ALOGI("[ExternalDisplay] %s : Skip %d th start frame", __func__, mSkipStartFrame);

        mSkipStartFrame++;
    }

    return ret;
}

int32_t ExynosExternalDisplay::canSkipValidate() {
    /*
     * SurfaceFlinger may call vadlidate, present for a few frame
     * even though external display is disconnected.
     * Cammands for primary display can be discarded if validate is skipped
     * in this case. HWC should return error not to skip validate.
     */
    if ((mHpdStatus == false) || (mBlanked == true))
        return SKIP_ERR_DISP_NOT_CONNECTED;

    if ((mSkipStartFrame > (SKIP_EXTERNAL_FRAME - 1)) && (mEnabled == false) &&
        (mPowerModeState == (hwc2_power_mode_t)HWC_POWER_MODE_NORMAL))
        return SKIP_ERR_DISP_NOT_POWER_ON;

    if (checkRotate() || (mIsSkipFrame) ||
        (mSkipStartFrame < SKIP_EXTERNAL_FRAME))
        return SKIP_ERR_FORCE_VALIDATE;

    return ExynosDisplay::canSkipValidate();
}

int32_t ExynosExternalDisplay::presentDisplay(
    DevicePresentInfo &presentInfo, int32_t *outPresentFence) {
    Mutex::Autolock lock(mExternalMutex);
    DISPLAY_LOGD(eDebugExternalDisplay, "");
    int32_t ret;

    if ((mIsSkipFrame) || (mHpdStatus == false) || (mBlanked == true)) {
        *outPresentFence = -1;
        for (size_t i = 0; i < mLayers.size(); i++) {
            ExynosLayer *layer = mLayers[i];
            layer->mAcquireFence = mFenceTracer.fence_close(layer->mAcquireFence,
                                                            mDisplayInfo.displayIdentifier, FENCE_TYPE_SRC_ACQUIRE, FENCE_IP_LAYER,
                                                            "ext::presentDisplay: layer acq_fence, skip case");
            layer->mReleaseFence = -1;
        }
        mClientCompositionInfo.mAcquireFence =
            mFenceTracer.fence_close(mClientCompositionInfo.mAcquireFence,
                                     mDisplayInfo.displayIdentifier, FENCE_TYPE_SRC_ACQUIRE, FENCE_IP_FB,
                                     "ext::presentDisplay: client_comp acq_fence, skip case");
        mClientCompositionInfo.mReleaseFence = -1;

        /* this frame is not presented, but mRenderingState is updated to RENDERING_STATE_PRESENTED */
        initDisplay();

        invalidate();

        return HWC2_ERROR_NONE;
    }

    ret = ExynosDisplay::presentDisplay(presentInfo, outPresentFence);

    return ret;
}
int32_t ExynosExternalDisplay::setClientTarget(
    buffer_handle_t target,
    int32_t acquireFence, int32_t /*android_dataspace_t*/ dataspace,
    uint64_t &geometryFlag) {
    buffer_handle_t handle = NULL;
    if (target != NULL)
        handle = target;
    if ((mClientCompositionInfo.mHasCompositionLayer == true) &&
        (handle == NULL) &&
        (mClientCompositionInfo.mSkipFlag == false)) {
        /*
         * openExternalDisplay() can be called between validateDisplay and getChangedCompositionTypes.
         * Then getChangedCompositionTypes() returns no layer because openExternalDisplay() clears mLayers.
         * SurfaceFlinger might not change compositionType to HWC2_COMPOSITION_CLIENT.
         * Handle can be NULL in this case. It is not error case.
         */
        if (mSkipStartFrame == 0) {
            if (acquireFence >= 0)
                mFenceTracer.fence_close(acquireFence, mDisplayInfo.displayIdentifier,
                                         FENCE_TYPE_SRC_ACQUIRE, FENCE_IP_FB,
                                         "ext::setClientTarget: layer acq_fence");
            acquireFence = -1;
            if (mClientCompositionInfo.mDataSpace != static_cast<android_dataspace>(dataspace))
                setGeometryChanged(GEOMETRY_DISPLAY_DATASPACE_CHANGED, geometryFlag);
            mClientCompositionInfo.setTargetBuffer(handle, acquireFence, (android_dataspace)dataspace);
            return HWC2_ERROR_NONE;
        }
    }

    if (mPlugState == false) {
        if (acquireFence >= 0)
            mFenceTracer.fence_close(acquireFence, mDisplayInfo.displayIdentifier,
                                     FENCE_TYPE_SRC_ACQUIRE, FENCE_IP_FB,
                                     "ext::setClientTarget: layer acq_fence");
        DISPLAY_LOGI("%s : skip setClientTarget because external display is already disconnected",
                     __func__);
        return HWC2_ERROR_NONE;
    }

    return ExynosDisplay::setClientTarget(target, acquireFence, dataspace,
                                          geometryFlag);
}

int ExynosExternalDisplay::enable() {
    ALOGI("[ExternalDisplay] %s +", __func__);

    if (mHpdStatus == false) {
        ALOGI("HPD is not connected");
        return HWC2_ERROR_NONE;
    }

    if (mEnabled == true) {
        DISPLAY_LOGD(eDebugExternalDisplay, "Skip powermode on because it is already on state.");
        return HWC2_ERROR_NONE;
    }

    if (mDisplayInterface->setPowerMode(HWC_POWER_MODE_NORMAL) < 0) {
        DISPLAY_LOGE("set powermode ioctl failed errno : %d", errno);
        return HWC2_ERROR_UNSUPPORTED;
    }

    mEnabled = true;

    ALOGI("[ExternalDisplay] %s -", __func__);

    return HWC2_ERROR_NONE;
}

int ExynosExternalDisplay::disable() {
    ALOGI("[ExternalDisplay] %s +", __func__);

    if (mSkipStartFrame > (SKIP_EXTERNAL_FRAME - 1)) {
        clearDisplay();
    } else {
        ALOGI("Skip clearDisplay to avoid resource conflict");
    }

    if ((mDisplayInterface->mType == INTERFACE_TYPE_DRM) &&
        (mHpdStatus == true)) {
        return HWC2_ERROR_NONE;
    }

    if (mDisplayInterface->setPowerMode(HWC_POWER_MODE_OFF) < 0) {
        DISPLAY_LOGE("set powermode ioctl failed errno : %d", errno);
        return HWC2_ERROR_UNSUPPORTED;
    }

    mEnabled = false;
    mPowerModeState = (hwc2_power_mode_t)HWC_POWER_MODE_OFF;

    ALOGI("[ExternalDisplay] %s -", __func__);

    return HWC2_ERROR_NONE;
}

int32_t ExynosExternalDisplay::setPowerMode(int32_t /*hwc2_power_mode_t*/ mode,
                                            uint64_t &geometryFlag) {
    Mutex::Autolock lock(mExternalMutex);
    {
        /* TODO state check routine should be added */

        int fb_blank = 0;
        int err = 0;
        if (mode == HWC_POWER_MODE_OFF) {
            fb_blank = FB_BLANK_POWERDOWN;
            err = disable();
        } else if (mHpdStatus == false) {
            /* HPD is not connected */
            fb_blank = FB_BLANK_POWERDOWN;
        } else {
            fb_blank = FB_BLANK_UNBLANK;
            err = enable();
        }

        if (err != 0) {
            DISPLAY_LOGE("set powermode ioctl failed errno : %d", errno);
            return HWC2_ERROR_UNSUPPORTED;
        }

        if (mHpdStatus == false) {
            mPowerModeState = (hwc2_power_mode_t)HWC_POWER_MODE_OFF;
            DISPLAY_LOGI("%s:: mode(HWC_POWER_MODE_OFF) becasue HPD is not connected, requested mode(%d)",
                         __func__, mode);
        } else {
            mPowerModeState = (hwc2_power_mode_t)mode;
            DISPLAY_LOGD(eDebugExternalDisplay, "%s:: mode(%d), blank(%d)", __func__, mode, fb_blank);
        }

        if (mode == HWC_POWER_MODE_OFF) {
            /* It should be called from validate() when the screen is on */
            mNeedSkipPresent = true;
            setGeometryChanged(GEOMETRY_DISPLAY_POWER_OFF, geometryFlag);
            if ((mRenderingState >= RENDERING_STATE_VALIDATED) &&
                (mRenderingState < RENDERING_STATE_PRESENTED))
                closeFencesForSkipFrame(RENDERING_STATE_VALIDATED);
            mRenderingState = RENDERING_STATE_NONE;
        } else {
            setGeometryChanged(GEOMETRY_DISPLAY_POWER_ON, geometryFlag);
        }
    }
    return HWC2_ERROR_NONE;
}

int32_t ExynosExternalDisplay::startPostProcessing() {
    if ((mHpdStatus == false) || (mBlanked == true) || mIsSkipFrame) {
        ALOGI("%s:: skip startPostProcessing display(%d) mHpdStatus(%d)",
              __func__, mDisplayId, mHpdStatus);
        return NO_ERROR;
    }
    return ExynosDisplay::startPostProcessing();
}

bool ExynosExternalDisplay::getHDRException(ExynosLayer *__unused layer,
                                            DevicePresentInfo &__unused deviceInfo) {
    bool ret = false;

    if (mSinkHdrSupported) {
        ret = true;
    }
    return ret;
}

bool ExynosExternalDisplay::getHDRException() {
    bool ret = false;

    if (mSinkHdrSupported) {
        ret = true;
    }
    return ret;
}

void ExynosExternalDisplay::handleHotplugEvent(bool hpdStatus) {
    Mutex::Autolock lock(mExternalMutex);
    {
        Mutex::Autolock lock(mDisplayMutex);
        mHpdStatus = hpdStatus;
        if (!mHpdStatus)
            mDisplayInterface->onDisplayRemoved();
        if (mHpdStatus) {
            if (openExternalDisplay() < 0) {
                DISPLAY_LOGE("Failed to openExternalDisplay");
                mHpdStatus = false;
                return;
            }
        } else {
            disable();
            closeExternalDisplay();
        }
    }

    ALOGI("HPD status changed to %s, mDisplayId %d, mDisplayFd %d", mHpdStatus ? "enabled" : "disabled", mDisplayId, mDisplayInterface->getDisplayFd());
}

void ExynosExternalDisplay::initDisplayInterface(uint32_t interfaceType,
                                                 void *deviceData, size_t &deviceDataSize) {
    if (interfaceType == INTERFACE_TYPE_DRM)
        mDisplayInterface = std::make_unique<ExynosDisplayDrmInterface>();
    else
        mDisplayInterface = std::make_unique<ExynosExternalDisplayFbInterfaceModule>();
    mDisplayInterface->init(mDisplayInfo.displayIdentifier,
                            deviceData, deviceDataSize);
    mDisplayInterface->updateUeventNodeName(mEventNodeName);
}

void ExynosExternalDisplay::init(uint32_t maxWindowNum, ExynosMPP *blendingMPP) {
    ExynosDisplay::init(maxWindowNum, blendingMPP);
    mHdrCoefInterface = createHdrInterfaceInstance();
    mHdr10PMetaInterface = createHdr10PMetaInterfaceInstance();
    if (mHdrCoefInterface) {
        mHdrCoefInterface->sethdr10pMetaInterface(mHdr10PMetaInterface);
        mHdrTargetInfo.bpc = HDR_BPC_8;
        mHdrTargetInfo.hdr_capa = HDR_CAPA_OUTER;
    }
    /* Init power mode */
    if (mDisplayInterface->mType == INTERFACE_TYPE_DRM)
        mDisplayInterface->setPowerMode(HWC_POWER_MODE_OFF);
}

int32_t ExynosExternalDisplay::getDisplayVsyncPeriod(hwc2_vsync_period_t *outVsyncPeriod) {
    return getDisplayVsyncPeriodInternal(outVsyncPeriod);
}

int32_t ExynosExternalDisplay::getDisplayVsyncPeriodInternal(hwc2_vsync_period_t *outVsyncPeriod) {
    DISPLAY_LOGD(eDebugExternalDisplay, "return vsyncPeriod as default : %d", mVsyncPeriod);
    *outVsyncPeriod = mVsyncPeriod;
    return HWC2_ERROR_NONE;
}

int32_t ExynosExternalDisplay::setActiveConfigWithConstraints(hwc2_config_t config,
                                                              hwc_vsync_period_change_constraints_t *vsyncPeriodChangeConstraints,
                                                              hwc_vsync_period_change_timeline_t *outTimeline,
                                                              bool needUpdateTimeline) {
    return HWC2_ERROR_NONE;
}

int32_t ExynosExternalDisplay::doDisplayConfigPostProcess() {
    return HWC2_ERROR_NONE;
}

int32_t ExynosExternalDisplay::getHdrCapabilities(uint32_t *outNumTypes,
                                                  int32_t *outTypes, float *outMaxLuminance,
                                                  float *outMaxAverageLuminance, float *outMinLuminance) {
    if (outTypes == NULL) {
#ifndef USES_HDR_GLES_CONVERSION
        mDisplayInterface->updateHdrSinkInfo();
        mSinkHdrSupported = false;
#else
        mSinkHdrSupported = mDisplayInterface->updateHdrSinkInfo();
#endif
    }

    int32_t ret = ExynosDisplay::getHdrCapabilities(outNumTypes, outTypes,
                                                    outMaxLuminance, outMaxAverageLuminance, outMinLuminance);

    if (outTypes == NULL) {
        *outNumTypes = 2;
    } else {
        outTypes[0] = HAL_HDR_HDR10;
        outTypes[1] = HAL_HDR_HLG;
        ALOGI("%s: outNumTypes(%u), maxLuminance(%f), maxAverageLuminance(%f), minLuminance(%f)",
              __func__, *outNumTypes, *outMaxLuminance,
              *outMaxAverageLuminance, *outMinLuminance);
    }
    return ret;
}

bool ExynosExternalDisplay::checkDisplayUnstarted() {
    return (mPlugState == true) && (mSkipStartFrame < SKIP_EXTERNAL_FRAME);
}
