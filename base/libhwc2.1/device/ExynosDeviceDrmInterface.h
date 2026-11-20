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

#ifndef _EXYNOSDEVICEDRMINTERFACE_H
#define _EXYNOSDEVICEDRMINTERFACE_H

#include "resourcemanager.h"
#include "ExynosDeviceInterface.h"
#include "ExynosDrmFramebufferManager.h"

using namespace android;

class ExynosDeviceDrmInterface : public ExynosDeviceInterface,
                                 public DrmEventHandler {
  public:
    ExynosDeviceDrmInterface();
    virtual ~ExynosDeviceDrmInterface();
    virtual void init(void *deviceData, size_t &deviceDataSize) override;
    virtual int32_t getRestrictions(struct dpp_restrictions_info_v2 *&restrictions, uint32_t otfMPPSize);
    void HandleEvent(uint64_t timestamp_us) override;
    void setDppChannelRestriction(struct dpp_ch_restriction &common_restriction,
                                struct drm_dpp_ch_restriction &drm_restriction);
    void setDppChannelRestriction(struct dpp_ch_restriction &common_restriction,
                                  drmModePropertyBlobPtr blob);
    void HandlePanelEvent(uint64_t timestamp_us) override;
  protected:
    ResourceManager mDrmResourceManager;
    DrmDevice *mDrmDevice;
    FramebufferManager &mFBManager = FramebufferManager::getInstance();
};

#endif  //_EXYNOSDEVICEDRMINTERFACE_H
