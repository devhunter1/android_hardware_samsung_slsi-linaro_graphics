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

#include <hardware/hardware.h>
#include "ExynosPrimaryDisplayModule.h"
#include "ExynosHWCDebug.h"
#include "ExynosLayer.h"
#include "ExynosGraphicBuffer.h"

using namespace vendor::graphics;

ExynosPrimaryDisplayModule::ExynosPrimaryDisplayModule(DisplayIdentifier node)
    :    ExynosPrimaryDisplay(node)
{
    mPerfTuneState = PERF_TUNE_OFF;
}

ExynosPrimaryDisplayModule::~ExynosPrimaryDisplayModule () {
}

int32_t ExynosPrimaryDisplayModule::validateWinConfigData()
{
    if (mDisplayConfigPending) {
        setActiveConfigInternal(mActiveConfig);
    }

    return ExynosDisplay::validateWinConfigData();
}

void ExynosPrimaryDisplayModule::doPreProcessing(DeviceValidateInfo &validateInfo,
        uint64_t &geometryChanged)
{

    bool prevAdjustDisplayFrame = mDisplayControl.adjustDisplayFrame;
    if (validateInfo.nonPrimaryDisplays.size() > 0) {
        mDisplayControl.adjustDisplayFrame = true;
    } else {
        mDisplayControl.adjustDisplayFrame = false;
    }
    if (prevAdjustDisplayFrame != mDisplayControl.adjustDisplayFrame)
        setGeometryChanged(GEOMETRY_DISPLAY_ADJUST_SIZE_CHANGED, geometryChanged);

    ExynosDisplay::doPreProcessing(validateInfo, geometryChanged);
}

int32_t ExynosPrimaryDisplayModule::setPerformanceSetting()
{
    if (mLayers.size() == 0)
        return NO_ERROR;

    bool perfTuneMode = false;
    for (size_t i = 0; i < mLayers.size(); i++) {
        ExynosLayer* layer = mLayers[i];
        int32_t d_width = layer->mPreprocessedInfo.displayFrame.right - layer->mPreprocessedInfo.displayFrame.left;
        int32_t d_height = layer->mPreprocessedInfo.displayFrame.bottom - layer->mPreprocessedInfo.displayFrame.top;
        if ((layer->mTransform & HAL_TRANSFORM_ROT_90) &&
                (layer->mPreprocessedInfo.sourceCrop.left == 0) &&
                (layer->mPreprocessedInfo.sourceCrop.top == 0) &&
                (layer->mPreprocessedInfo.sourceCrop.right == 1280) &&
                (layer->mPreprocessedInfo.sourceCrop.bottom == 720) &&
                (d_width >= 1075) && (d_width <= 1085) &&
                (d_height >= 1915) && (d_height <= 1925) &&
                (layer->mDataSpace == 0x10c10000) &&
                (ExynosGraphicBufferMeta::get_format(layer->mLayerBuffer) == HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP_M) &&
                (mLayers.size() < 3)) {
            perfTuneMode = true;
            break;
        } else {
            perfTuneMode = false;
        }
    }

    return NO_ERROR;
}
