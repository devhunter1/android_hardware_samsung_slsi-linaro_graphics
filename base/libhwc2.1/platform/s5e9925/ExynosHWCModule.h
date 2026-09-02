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

#ifndef ANDROID_EXYNOS_HWC_MODULE_H_
#define ANDROID_EXYNOS_HWC_MODULE_H_

#include "ExynosHWC.h"
#include "DeconHeader.h"
#include "ExynosHWCHelper.h"

#define VSYNC_DEV_PREFIX    "/sys/devices/platform/"
#define PSR_DEV_NAME        "19f00000.decon_0/psr_info"

#define HIBER_EXIT_NODE_NAME    "/sys/devices/platform/19f00000.drmdecon/hiber_exit"
#define BRIGHTNESS_NODE_BASE    "/sys/class/backlight/panel/brightness"
#define MAX_BRIGHTNESS_NODE_BASE    "/sys/class/backlight/panel/max_brightness"

#define DP_LINK_NAME        "10ab0000.displayport"
#define DP_UEVENT_NAME      "change@/devices/platform/%s/extcon/extcon0"
#define DP_CABLE_STATE_NAME "/sys/devices/platform/%s/extcon/extcon0/cable.%d/state"

#define IDMA(x) static_cast<decon_idma_type>(x)

#define DEFAULT_MPP_DST_YUV_FORMAT HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SPN_SBWC
#define MSC_CLOCK   800000

#define VPP_CLOCK       800000000
#define VPP_PIXEL_PER_CLOCK 2

#define VPP_MARGIN 1.1
#define VPP_DISP_FACTOR 1.0

#define G2D_CLOCK       712024

#define VOTF_BUF_INDEX_MAX 15

#define HAS_FINGERPRINT_MASK_LAYER

typedef enum assignOrderType {
    ORDER_SAJC,
    ORDER_WCG,
    ORDER_AXI,
} assignOrderType_t;

typedef enum DPUblockId {
    DPUF0,
    DPUF1,
    DPU_BLOCK_CNT,
} DPUblockId_t;

const std::unordered_map<DPUblockId_t, String8> DPUBlocks = {
    {DPUF0, String8("DPUF0")},
    {DPUF1, String8("DPUF1")},
};

typedef enum AXIPortId {
    AXI0,
    AXI1,
    AXI_PORT_CNT,
} AXIPortId_t;

enum {
    HWC_DISPLAY_NONE_BIT = 0
};

enum {
    L0 = 0, L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12, L13, L14, L15, WB
};
/*
 * pre_assign_info: all display_descriptors that want to reserve
 */
struct exynos_mpp_t {
    int physicalType;
    int logicalType;
    char name[16];
    uint32_t physical_index;
    uint32_t logical_index;
    uint32_t pre_assign_info;
    // For TDM
    uint32_t hw_block_index;
    uint32_t axi_port_index;
};

struct vOtfInfo_t {
    mpp_phycal_type_t type;
    uint32_t index;
    decon_idma_type channel;
    unsigned int dma_idx;
    unsigned int trs_idx;
};

const vOtfInfo_t VOTF_INFO_MAP[] = {
    // DPUF0 AXI0
    {MPP_DPP_GF,     0, IDMA(L0),  0, 0},
    {MPP_DPP_VGRFS,  0, IDMA(L1),  0, 1},
    {MPP_DPP_GF,     1, IDMA(L2),  0, 2},
    {MPP_DPP_VGRFS,  1, IDMA(L3),  0, 3},

    // DPUF0 AXI1
    {MPP_DPP_GF,     2, IDMA(L4),  0, 4},
    {MPP_DPP_VGFS,   0, IDMA(L5),  0, 5},
    {MPP_DPP_GF,     3, IDMA(L6),  0, 6},
    {MPP_DPP_VGFS,   1, IDMA(L7),  0, 7},

    // DPUF1 AXI0
    {MPP_DPP_GF,     4, IDMA(L8),  1, 0},
    {MPP_DPP_VGRFS,  2, IDMA(L9),  1, 1},
    {MPP_DPP_GF,     5, IDMA(L10), 1, 2},
    {MPP_DPP_VGRFS,  3, IDMA(L11), 1, 3},

    // DPUF1 AXI1
    {MPP_DPP_GF,     6, IDMA(L12), 1, 4},
    {MPP_DPP_VGFS,   2, IDMA(L13), 1, 5},
    {MPP_DPP_GF,     7, IDMA(L14), 1, 6},
    {MPP_DPP_VGFS,   3, IDMA(L15), 1, 7}
};

const dpp_channel_map_t IDMA_CHANNEL_MAP[] = {
    // DPUF0 AXI0
    {MPP_DPP_GF,     0, IDMA_GF0,    IDMA(L0)},
    {MPP_DPP_VGRFS,  0, IDMA_VGRFS0, IDMA(L1)},
    {MPP_DPP_GF,     1, IDMA_GF1,    IDMA(L2)},
    {MPP_DPP_VGRFS,  1, IDMA_VGRFS1, IDMA(L3)},

    // DPUF0 AXI1
    {MPP_DPP_GF,     2, IDMA_GF2,    IDMA(L4)},
    {MPP_DPP_VGFS,   0, IDMA_VGFS0,  IDMA(L5)},
    {MPP_DPP_GF,     3, IDMA_GF3,    IDMA(L6)},
    {MPP_DPP_VGFS,   1, IDMA_VGFS1,  IDMA(L7)},

    // DPUF1 AXI0
    {MPP_DPP_GF,     4, IDMA_GF4,    IDMA(L8)},
    {MPP_DPP_VGRFS,  2, IDMA_VGRFS2, IDMA(L9)},
    {MPP_DPP_GF,     5, IDMA_GF5,    IDMA(L10)},
    {MPP_DPP_VGRFS,  3, IDMA_VGRFS3, IDMA(L11)},

    // DPUF1 AXI1
    {MPP_DPP_GF,     6, IDMA_GF6,    IDMA(L12)},
    {MPP_DPP_VGFS,   2, IDMA_VGFS2,  IDMA(L13)},
    {MPP_DPP_GF,     7, IDMA_GF7,    IDMA(L14)},
    {MPP_DPP_VGFS,   3, IDMA_VGFS3,  IDMA(L15)},

    {MPP_P_TYPE_MAX, 0, ODMA_WB,     IDMA(WB)}, // not idma but..
    {static_cast<mpp_phycal_type_t>(MAX_DECON_DMA_TYPE), 0, MAX_DECON_DMA_TYPE, IDMA(WB+1)}
};

#define MAX_NAME_SIZE   32
struct exynos_display_t {
    uint32_t type;
    uint32_t index;
    char display_name[MAX_NAME_SIZE];
    char decon_node_name[MAX_NAME_SIZE];
    char vsync_node_name[MAX_NAME_SIZE];
};

/******** Description about display bit ********/
/*   DISPLAY BIT = 1 << (DISPLAY_MODE_MASK_LEN * display mode
 *                       + SECOND_DISPLAY_START_BIT * display index
 *                       + display type);
 *   ex) HWC_DISPLAY_EXTERNAL2_BIT = 1 << (DISPLAY_MODE_MASK_LEN * display mode(0)
 *                                         + SECOND_DISPLAY_START_BIT * display index(1)
 *                                         + displayy type(1))
 *                                 = 1 << 5
 *   PRIMARY MAIN MODE :
 *      0 bit : HWC_DISPLAY_PRIMARY_BIT,
 *      1 bit : HWC_DISPLAY_EXTERNAL_BIT,
 *      2 bit : HWC_DISPLAY_VIRTUAL_BIT,
 *      5 bit : HWC_DISPLAY_EXTERNAL2_BIT (unused),
 *   EXTERNAL MAIN MODE :
 *      8 bit : EXTERNAL_MAIN_DISPLAY_PRIMARY_BIT,
 *      9 bit : EXTERNAL_MAIN_DISPLAY_EXTERNAL_BIT,
 *      10 bit : EXTERNAL_MAIN_DISPLAY_VIRTUAL_BIT,
 ***************************************************/

#define PRIMARY_MAIN_EXTERNAL_WINCNT   3
#define EXTERNAL_MAIN_EXTERNAL_WINCNT  4
#define PRIMARY_MAIN_VIRTUAL_WINCNT   5

#define DISPLAY_MODE_MASK_LEN    8
#define DISPLAY_MODE_MASK_BIT    0xff
#define SECOND_DISPLAY_START_BIT   4

enum {
    DISPLAY_MODE_PRIMARY_MAIN = 0,  /* This is default mode */
    DISPLAY_MODE_EXTERNAL_MAIN,
    DISPLAY_MODE_NUM
};

/*
 * This is base window index of primary display for each display mode.
 * External display base window is always 0
 */
const uint32_t EXTERNAL_WINDOW_COUNT[] = {PRIMARY_MAIN_EXTERNAL_WINCNT, EXTERNAL_MAIN_EXTERNAL_WINCNT};

#define EXTERNAL_MAIN_DISPLAY_START_BIT (DISPLAY_MODE_MASK_LEN * DISPLAY_MODE_EXTERNAL_MAIN)
enum {
    EXTERNAL_MAIN_DISPLAY_PRIMARY_BIT = 1 << (EXTERNAL_MAIN_DISPLAY_START_BIT + HWC_DISPLAY_PRIMARY),
    EXTERNAL_MAIN_DISPLAY_EXTERNAL_BIT = 1 << (EXTERNAL_MAIN_DISPLAY_START_BIT + HWC_DISPLAY_EXTERNAL),
    EXTERNAL_MAIN_DISPLAY_VIRTUAL_BIT = 1 << (EXTERNAL_MAIN_DISPLAY_START_BIT + HWC_DISPLAY_VIRTUAL),
};

const exynos_mpp_t AVAILABLE_OTF_MPP_UNITS[] = {
    {MPP_DPP_GF, MPP_LOGICAL_DPP_GF,
     "DPP_GF0", 0, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF0), static_cast<uint32_t>(AXI0)},

    {MPP_DPP_VGRFS, MPP_LOGICAL_DPP_VGRFS,
     "DPP_VGRFS0", 0, 0,
     HWC_DISPLAY_EXTERNAL_BIT | HWC_DISPLAY_VIRTUAL_BIT,
     static_cast<uint32_t>(DPUF0), static_cast<uint32_t>(AXI0)},

    {MPP_DPP_GF, MPP_LOGICAL_DPP_GF,
     "DPP_GF1", 1, 0,
     HWC_DISPLAY_EXTERNAL_BIT | HWC_DISPLAY_VIRTUAL_BIT,
     static_cast<uint32_t>(DPUF0), static_cast<uint32_t>(AXI0)},

    {MPP_DPP_VGRFS, MPP_LOGICAL_DPP_VGRFS,
     "DPP_VGRFS1", 1, 0,
     HWC_DISPLAY_EXTERNAL_BIT | HWC_DISPLAY_VIRTUAL_BIT,
     static_cast<uint32_t>(DPUF0), static_cast<uint32_t>(AXI0)},

    {MPP_DPP_GF, MPP_LOGICAL_DPP_GF,
     "DPP_GF2", 2, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF0), static_cast<uint32_t>(AXI1)},

    {MPP_DPP_VGFS, MPP_LOGICAL_DPP_VGFS,
     "DPP_VGFS0", 0, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF0), static_cast<uint32_t>(AXI1)},

    {MPP_DPP_GF, MPP_LOGICAL_DPP_GF,
     "DPP_GF3", 3, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF0), static_cast<uint32_t>(AXI1)},

    {MPP_DPP_VGFS, MPP_LOGICAL_DPP_VGFS,
     "DPP_VGFS1", 1, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF0), static_cast<uint32_t>(AXI1)},

    {MPP_DPP_GF, MPP_LOGICAL_DPP_GF,
     "DPP_GF4", 4, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF1), static_cast<uint32_t>(AXI0)},

    {MPP_DPP_VGRFS, MPP_LOGICAL_DPP_VGRFS,
     "DPP_VGRFS2", 2, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF1), static_cast<uint32_t>(AXI0)},

    {MPP_DPP_GF, MPP_LOGICAL_DPP_GF,
     "DPP_GF5", 5, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF1), static_cast<uint32_t>(AXI0)},

    {MPP_DPP_VGRFS, MPP_LOGICAL_DPP_VGRFS,
     "DPP_VGRFS3", 3, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF1), static_cast<uint32_t>(AXI0)},

    {MPP_DPP_GF, MPP_LOGICAL_DPP_GF,
     "DPP_GF6", 6, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF1), static_cast<uint32_t>(AXI1)},

    {MPP_DPP_VGFS, MPP_LOGICAL_DPP_VGFS,
     "DPP_VGFS2", 2, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF1), static_cast<uint32_t>(AXI1)},

    {MPP_DPP_GF, MPP_LOGICAL_DPP_GF,
     "DPP_GF7", 7, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF1), static_cast<uint32_t>(AXI1)},

    {MPP_DPP_VGFS, MPP_LOGICAL_DPP_VGFS,
     "DPP_VGFS3", 3, 0,
     HWC_DISPLAY_PRIMARY_BIT,
     static_cast<uint32_t>(DPUF1), static_cast<uint32_t>(AXI1)}
};

const exynos_mpp_t AVAILABLE_M2M_MPP_UNITS[] = {
    {MPP_MSC, MPP_LOGICAL_MSC, "MSC0_PRI", 0, 0, HWC_DISPLAY_PRIMARY_BIT|EXTERNAL_MAIN_DISPLAY_PRIMARY_BIT},
    {MPP_MSC, MPP_LOGICAL_MSC_YUV, "MSC0_VIR0", 0, 1, HWC_DISPLAY_VIRTUAL_BIT},
    {MPP_MSC, MPP_LOGICAL_MSC_YUV, "MSC0_VIR1", 0, 2, HWC_DISPLAY_VIRTUAL_BIT},
    {MPP_MSC, MPP_LOGICAL_MSC_YUV, "MSC0_EXT0", 0, 3, HWC_DISPLAY_EXTERNAL_BIT},
};

/* AVAILABLE_DISPLAY_UNITS's index is same with index of mDisplays. Many part of exynos HWC operates by order of
   mDisplays' index, so rules for deciding a AVAILABLE_DISPLAY_UNITS's index should be followed.
   - Rules
     1. Display's order is decided by descending order of display type defined in "hwcomposer_defs.h".
     2. If more than two displays have same display type, order is decided by descending order of
        exynos_display_t's index value.
   Scenario affcted by this order is as follows.
   - Scenario
     1. HWC's main API like as validateDisplay and presentDisplay should be called by inverse order of mDisplays' index.
     2. Process for preassigning M2MMPP resources should be called by inverse order of mDisplays' index.
     3. For the process about preassigning OTFMPP resources, display that do not use DPU like as virtual display
        should be alligned at the end. */
const exynos_display_t AVAILABLE_DISPLAY_UNITS[] = {
    {HWC_DISPLAY_PRIMARY, 0, "PrimaryDisplay",      "0", ""},
    {HWC_DISPLAY_EXTERNAL, 0, "ExternalDisplay",    "1", ""},
    {HWC_DISPLAY_VIRTUAL, 0, "VirtualDisplay", "2", ""},
};

#define DISPLAY_COUNT sizeof(AVAILABLE_DISPLAY_UNITS)/sizeof(exynos_display_t)

const uint32_t ATTRIBUTE_PRIORITY_LIST[] = {
    ATTRIBUTE_HDR10PLUS,
};

const size_t product_layerAttributePriority[] = {ATTRIBUTE_DRM, ATTRIBUTE_HDR10PLUS, ATTRIBUTE_HDR10};

struct display_resource_info_t {
    uint32_t displayId;
    /* If this variable has a non-zero value, the display uses resource sets by defined value */
    uint32_t fix_setcnt;
    /* Small number have higher priority than bigger one.
     * Range of priority is from 0 to (NUM_OF_DISPLAY_NEEDED_OTFMPP -1) */
    uint32_t dpp_priority;
    uint32_t attr_priority[MAX_ATTRIBUTE_NUM];
};

const display_resource_info_t RESOURCE_INFO_TABLE[] = {
};

/*
 * Note :
 * When External or Virtual display is connected,
 * Primary amount = total - others
 */
class HWResourceIndexes {
    private:
        tdm_attr_t attr;
        DPUblockId_t DPUBlockNo;
        int displayId;

    public:
        HWResourceIndexes(tdm_attr_t _attr, DPUblockId_t _DPUBlockNo, int _displayId)
            : attr(_attr), DPUBlockNo(_DPUBlockNo), displayId(_displayId) {}
        bool operator<(const HWResourceIndexes& rhs) const {
            if (attr != rhs.attr) return attr < rhs.attr;

            if (DPUBlockNo != rhs.DPUBlockNo) return DPUBlockNo < rhs.DPUBlockNo;

            if (displayId != rhs.displayId) return displayId < rhs.displayId;

            return false;
        }
};

typedef struct HWResourceAmounts {
    int maxAssignedAmount;
    int totalAmount;
} HWResourceAmounts_t;

/* Note :
 * When External or Virtual display is connected,
 * Primary amount = total - others */

const std::map<HWResourceIndexes, HWResourceAmounts_t> HWResourceTables = {
    {HWResourceIndexes(TDM_ATTR_SRAM_AMOUNT, DPUF0, HWC_DISPLAY_PRIMARY),  {0, 40}},
    {HWResourceIndexes(TDM_ATTR_SRAM_AMOUNT, DPUF0, HWC_DISPLAY_EXTERNAL), {40, 80}},
    {HWResourceIndexes(TDM_ATTR_SRAM_AMOUNT, DPUF0, HWC_DISPLAY_VIRTUAL),  {40, 80}},
    {HWResourceIndexes(TDM_ATTR_SRAM_AMOUNT, DPUF1, HWC_DISPLAY_PRIMARY),  {40, 80}},
    {HWResourceIndexes(TDM_ATTR_SRAM_AMOUNT, DPUF1, HWC_DISPLAY_EXTERNAL), {0, 80}},
    {HWResourceIndexes(TDM_ATTR_SRAM_AMOUNT, DPUF1, HWC_DISPLAY_VIRTUAL),  {0, 80}},

    {HWResourceIndexes(TDM_ATTR_SCALE, DPUF0, HWC_DISPLAY_PRIMARY),  {0, 2}},
    {HWResourceIndexes(TDM_ATTR_SCALE, DPUF0, HWC_DISPLAY_EXTERNAL), {2, 2}},
    {HWResourceIndexes(TDM_ATTR_SCALE, DPUF0, HWC_DISPLAY_VIRTUAL),  {2, 2}},
    {HWResourceIndexes(TDM_ATTR_SCALE, DPUF1, HWC_DISPLAY_PRIMARY),  {2, 2}},
    {HWResourceIndexes(TDM_ATTR_SCALE, DPUF1, HWC_DISPLAY_EXTERNAL), {0, 2}},
    {HWResourceIndexes(TDM_ATTR_SCALE, DPUF1, HWC_DISPLAY_VIRTUAL),  {0, 2}},

    {HWResourceIndexes(TDM_ATTR_SBWC, DPUF0, HWC_DISPLAY_PRIMARY),  {0, 2}},
    {HWResourceIndexes(TDM_ATTR_SBWC, DPUF0, HWC_DISPLAY_EXTERNAL), {2, 2}},
    {HWResourceIndexes(TDM_ATTR_SBWC, DPUF0, HWC_DISPLAY_VIRTUAL),  {2, 2}},
    {HWResourceIndexes(TDM_ATTR_SBWC, DPUF1, HWC_DISPLAY_PRIMARY),  {2, 2}},
    {HWResourceIndexes(TDM_ATTR_SBWC, DPUF1, HWC_DISPLAY_EXTERNAL), {0, 2}},
    {HWResourceIndexes(TDM_ATTR_SBWC, DPUF1, HWC_DISPLAY_VIRTUAL),  {0, 2}},

    {HWResourceIndexes(TDM_ATTR_SAJC, DPUF0, HWC_DISPLAY_PRIMARY),  {0, 2}},
    {HWResourceIndexes(TDM_ATTR_SAJC, DPUF0, HWC_DISPLAY_EXTERNAL), {2, 2}},
    {HWResourceIndexes(TDM_ATTR_SAJC, DPUF0, HWC_DISPLAY_VIRTUAL),  {2, 2}},
    {HWResourceIndexes(TDM_ATTR_SAJC, DPUF1, HWC_DISPLAY_PRIMARY),  {2, 2}},
    {HWResourceIndexes(TDM_ATTR_SAJC, DPUF1, HWC_DISPLAY_EXTERNAL), {0, 2}},
    {HWResourceIndexes(TDM_ATTR_SAJC, DPUF1, HWC_DISPLAY_VIRTUAL),  {0, 2}},

    {HWResourceIndexes(TDM_ATTR_ITP, DPUF0, HWC_DISPLAY_PRIMARY),  {0, 3}},
    {HWResourceIndexes(TDM_ATTR_ITP, DPUF0, HWC_DISPLAY_EXTERNAL), {3, 3}},
    {HWResourceIndexes(TDM_ATTR_ITP, DPUF0, HWC_DISPLAY_VIRTUAL),  {3, 3}},
    {HWResourceIndexes(TDM_ATTR_ITP, DPUF1, HWC_DISPLAY_PRIMARY),  {3, 3}},
    {HWResourceIndexes(TDM_ATTR_ITP, DPUF1, HWC_DISPLAY_EXTERNAL), {0, 3}},
    {HWResourceIndexes(TDM_ATTR_ITP, DPUF1, HWC_DISPLAY_VIRTUAL),  {0, 3}},

    {HWResourceIndexes(TDM_ATTR_ROT_90, DPUF0, HWC_DISPLAY_PRIMARY),  {0, 4}},
    {HWResourceIndexes(TDM_ATTR_ROT_90, DPUF0, HWC_DISPLAY_EXTERNAL), {4, 4}},
    {HWResourceIndexes(TDM_ATTR_ROT_90, DPUF0, HWC_DISPLAY_VIRTUAL),  {4, 4}},
    {HWResourceIndexes(TDM_ATTR_ROT_90, DPUF1, HWC_DISPLAY_PRIMARY),  {4, 4}},
    {HWResourceIndexes(TDM_ATTR_ROT_90, DPUF1, HWC_DISPLAY_EXTERNAL), {0, 4}},
    {HWResourceIndexes(TDM_ATTR_ROT_90, DPUF1, HWC_DISPLAY_VIRTUAL),  {0, 4}},
};

typedef enum lbWidthIndex {
    LB_W_8_512,
    LB_W_513_1024,
    LB_W_1025_2048,
    LB_W_2049_INF,
} lbWidthIndex_t;

typedef struct lbWidthBoundary {
    uint32_t widthDownto;
    uint32_t widthUpto;
} lbWidthBoundary_t;

const std::map<lbWidthIndex_t, lbWidthBoundary_t> LB_WIDTH_INDEX_MAP = {
    {LB_W_8_512,     {8, 512}},
    {LB_W_513_1024,  {513, 1024}},
    {LB_W_1025_2048, {1025, 2048}},
    {LB_W_2049_INF,  {2049, 0xffff}},
};

class sramAmountParams {
private:
    tdm_attr_t attr;
    uint32_t formatProperty;
    lbWidthIndex_t widthIndex;

public:
    sramAmountParams(tdm_attr_t _attr, uint32_t _formatProperty, lbWidthIndex_t _widthIndex)
          : attr(_attr), formatProperty(_formatProperty), widthIndex(_widthIndex) {}
    bool operator<(const sramAmountParams& rhs) const {
        if (attr != rhs.attr) return attr < rhs.attr;

        if (formatProperty != rhs.formatProperty) return formatProperty < rhs.formatProperty;

        if (widthIndex != rhs.widthIndex) return widthIndex < rhs.widthIndex;

        return false;
    }
};

enum {
    SBWC_Y = 0,
    SBWC_UV,
};

const std::map<sramAmountParams, uint32_t> sramAmountMap = {
    /** Non rotation **/
    /** BIT8 = 32bit format **/
    {sramAmountParams(TDM_ATTR_SAJC, RGB | BIT8, LB_W_8_512),     4},
    {sramAmountParams(TDM_ATTR_SAJC, RGB | BIT8, LB_W_513_1024),  4},
    {sramAmountParams(TDM_ATTR_SAJC, RGB | BIT8, LB_W_1025_2048), 8},
    {sramAmountParams(TDM_ATTR_SAJC, RGB | BIT8, LB_W_2049_INF), 9},

    {sramAmountParams(TDM_ATTR_SBWC, BIT8, LB_W_8_512),     1},
    {sramAmountParams(TDM_ATTR_SBWC, BIT8, LB_W_513_1024),  3},
    {sramAmountParams(TDM_ATTR_SBWC, BIT8, LB_W_1025_2048), 3},
    {sramAmountParams(TDM_ATTR_SBWC, BIT8, LB_W_2049_INF), 4},

    {sramAmountParams(TDM_ATTR_SBWC, BIT10, LB_W_8_512),     1},
    {sramAmountParams(TDM_ATTR_SBWC, BIT10, LB_W_513_1024),  3},
    {sramAmountParams(TDM_ATTR_SBWC, BIT10, LB_W_1025_2048), 3},
    {sramAmountParams(TDM_ATTR_SBWC, BIT10, LB_W_2049_INF), 4},

    {sramAmountParams(TDM_ATTR_ROT_90, BIT8, LB_W_8_512),     6},
    {sramAmountParams(TDM_ATTR_ROT_90, BIT8, LB_W_513_1024),  12},
    {sramAmountParams(TDM_ATTR_ROT_90, BIT8, LB_W_1025_2048), 24},
    {sramAmountParams(TDM_ATTR_ROT_90, BIT8, LB_W_2049_INF), 28},

    {sramAmountParams(TDM_ATTR_ROT_90, BIT10, LB_W_8_512),     4},
    {sramAmountParams(TDM_ATTR_ROT_90, BIT10, LB_W_513_1024),  6},
    {sramAmountParams(TDM_ATTR_ROT_90, BIT10, LB_W_1025_2048), 12},
    {sramAmountParams(TDM_ATTR_ROT_90, BIT10, LB_W_2049_INF), 15},

    {sramAmountParams(TDM_ATTR_ROT_90, SBWC_Y, LB_W_8_512),     2},
    {sramAmountParams(TDM_ATTR_ROT_90, SBWC_Y, LB_W_513_1024),  4},
    {sramAmountParams(TDM_ATTR_ROT_90, SBWC_Y, LB_W_1025_2048), 8},
    {sramAmountParams(TDM_ATTR_ROT_90, SBWC_Y, LB_W_2049_INF), 9},

    {sramAmountParams(TDM_ATTR_ROT_90, SBWC_UV, LB_W_8_512),     2},
    {sramAmountParams(TDM_ATTR_ROT_90, SBWC_UV, LB_W_513_1024),  2},
    {sramAmountParams(TDM_ATTR_ROT_90, SBWC_UV, LB_W_1025_2048), 4},
    {sramAmountParams(TDM_ATTR_ROT_90, SBWC_UV, LB_W_2049_INF), 6},

    {sramAmountParams(TDM_ATTR_ITP, BIT8, LB_W_2049_INF),  2},
    {sramAmountParams(TDM_ATTR_ITP, BIT10, LB_W_2049_INF), 2},

    {sramAmountParams(TDM_ATTR_SCALE, FORMAT_YUV_MASK, LB_W_2049_INF), 12},
    {sramAmountParams(TDM_ATTR_SCALE, FORMAT_RGB_MASK, LB_W_2049_INF), 16}
};
#endif
