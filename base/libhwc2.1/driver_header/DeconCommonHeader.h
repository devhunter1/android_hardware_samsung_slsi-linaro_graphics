/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef _DECON_COMMON_HELPER_H
#define _DECON_COMMON_HELPER_H
#define HDR_CAPABILITIES_NUM 4
#define MAX_FMT_CNT 64
#define MAX_DPP_CNT 7
#define MAX_DPP_CNT_V2 17
typedef unsigned int u32;
enum decon_psr_mode {
    DECON_VIDEO_MODE = 0,
    DECON_MIPI_COMMAND_MODE = 1,
    DECON_DP_PSR_MODE = 2,
};
enum decon_pixel_format {
    DECON_PIXEL_FORMAT_ARGB_8888 = 0,
    DECON_PIXEL_FORMAT_ABGR_8888,
    DECON_PIXEL_FORMAT_RGBA_8888,
    DECON_PIXEL_FORMAT_BGRA_8888,
    DECON_PIXEL_FORMAT_XRGB_8888,
    DECON_PIXEL_FORMAT_XBGR_8888,
    DECON_PIXEL_FORMAT_RGBX_8888,
    DECON_PIXEL_FORMAT_BGRX_8888,
    DECON_PIXEL_FORMAT_RGBA_5551,
    DECON_PIXEL_FORMAT_BGRA_5551,
    DECON_PIXEL_FORMAT_ABGR_4444,
    DECON_PIXEL_FORMAT_RGBA_4444,
    DECON_PIXEL_FORMAT_BGRA_4444,
    DECON_PIXEL_FORMAT_RGB_565,
    DECON_PIXEL_FORMAT_BGR_565,
    DECON_PIXEL_FORMAT_ARGB_2101010,
    DECON_PIXEL_FORMAT_ABGR_2101010,
    DECON_PIXEL_FORMAT_RGBA_1010102,
    DECON_PIXEL_FORMAT_BGRA_1010102,
    DECON_PIXEL_FORMAT_NV16,
    DECON_PIXEL_FORMAT_NV61,
    DECON_PIXEL_FORMAT_YVU422_3P,
    DECON_PIXEL_FORMAT_NV12,
    DECON_PIXEL_FORMAT_NV21,
    DECON_PIXEL_FORMAT_NV12M,
    DECON_PIXEL_FORMAT_NV21M,
    DECON_PIXEL_FORMAT_YUV420,
    DECON_PIXEL_FORMAT_YVU420,
    DECON_PIXEL_FORMAT_YUV420M,
    DECON_PIXEL_FORMAT_YVU420M,
    DECON_PIXEL_FORMAT_NV12N,
    DECON_PIXEL_FORMAT_NV12N_10B,
    DECON_PIXEL_FORMAT_NV12M_P010,
    DECON_PIXEL_FORMAT_NV21M_P010,
    DECON_PIXEL_FORMAT_NV12M_S10B,
    DECON_PIXEL_FORMAT_NV21M_S10B,
    DECON_PIXEL_FORMAT_NV16M_P210,
    DECON_PIXEL_FORMAT_NV61M_P210,
    DECON_PIXEL_FORMAT_NV16M_S10B,
    DECON_PIXEL_FORMAT_NV61M_S10B,
    DECON_PIXEL_FORMAT_NV12_P010,
    DECON_PIXEL_FORMAT_NV12M_SBWC_8B,
    DECON_PIXEL_FORMAT_NV12M_SBWC_10B,
    DECON_PIXEL_FORMAT_NV21M_SBWC_8B,
    DECON_PIXEL_FORMAT_NV21M_SBWC_10B,
    DECON_PIXEL_FORMAT_NV12N_SBWC_8B,
    DECON_PIXEL_FORMAT_NV12N_SBWC_10B,
    /* formats for lossy SBWC case  */
    DECON_PIXEL_FORMAT_NV12M_SBWC_8B_L50,
    DECON_PIXEL_FORMAT_NV12M_SBWC_8B_L75,
    DECON_PIXEL_FORMAT_NV12N_SBWC_8B_L50,
    DECON_PIXEL_FORMAT_NV12N_SBWC_8B_L75,
    DECON_PIXEL_FORMAT_NV12M_SBWC_10B_L40,
    DECON_PIXEL_FORMAT_NV12M_SBWC_10B_L60,
    DECON_PIXEL_FORMAT_NV12M_SBWC_10B_L80,
    DECON_PIXEL_FORMAT_NV12N_SBWC_10B_L40,
    DECON_PIXEL_FORMAT_NV12N_SBWC_10B_L60,
    DECON_PIXEL_FORMAT_NV12N_SBWC_10B_L80,
    DECON_PIXEL_FORMAT_MAX,
};
enum decon_blending {
    DECON_BLENDING_NONE = 0,
    DECON_BLENDING_PREMULT = 1,
    DECON_BLENDING_COVERAGE = 2,
    DECON_BLENDING_MAX = 3,
};
enum dpp_rotate {
    DPP_ROT_NORMAL = 0x0,
    DPP_ROT_XFLIP,
    DPP_ROT_YFLIP,
    DPP_ROT_180,
    DPP_ROT_90,
    DPP_ROT_90_XFLIP,
    DPP_ROT_90_YFLIP,
    DPP_ROT_270,
};
enum dpp_comp_src {
    DPP_COMP_SRC_NONE = 0,
    DPP_COMP_SRC_G2D,
    DPP_COMP_SRC_GPU
};
enum dpp_csc_eq {
    CSC_STANDARD_SHIFT = 0,
    CSC_BT_601 = 0,
    CSC_BT_709 = 1,
    CSC_BT_2020 = 2,
    CSC_DCI_P3 = 3,
    CSC_BT_601_625,
    CSC_BT_601_625_UNADJUSTED,
    CSC_BT_601_525,
    CSC_BT_601_525_UNADJUSTED,
    CSC_BT_2020_CONSTANT_LUMINANCE,
    CSC_BT_470M,
    CSC_FILM,
    CSC_ADOBE_RGB,
    CSC_UNSPECIFIED = 63,
    CSC_RANGE_SHIFT = 6,
    CSC_RANGE_LIMITED = 0x0,
    CSC_RANGE_FULL = 0x1,
    CSC_RANGE_EXTENDED,
    CSC_RANGE_UNSPECIFIED = 7
};
enum dpp_hdr_standard {
    DPP_HDR_OFF = 0,
    DPP_HDR_ST2084,
    DPP_HDR_HLG,
    DPP_TRANSFER_LINEAR,
    DPP_TRANSFER_SRGB,
    DPP_TRANSFER_SMPTE_170M,
    DPP_TRANSFER_GAMMA2_2,
    DPP_TRANSFER_GAMMA2_6,
    DPP_TRANSFER_GAMMA2_8
};
enum hwc_ver {
    HWC_INIT = 0,
    HWC_1_0 = 1,
    HWC_2_0 = 2,
};
enum disp_pwr_mode {
    DECON_POWER_MODE_OFF = 0,
    DECON_POWER_MODE_DOZE,
    DECON_POWER_MODE_NORMAL,
    DECON_POWER_MODE_DOZE_SUSPEND,
};

enum dpp_attr {
    DPP_ATTR_AFBC = 0,
    DPP_ATTR_SAJC = 0,
    DPP_ATTR_BLOCK = 1,
    DPP_ATTR_FLIP = 2,
    DPP_ATTR_ROT = 3,
    DPP_ATTR_CSC = 4,
    DPP_ATTR_SCALE = 5,
    DPP_ATTR_HDR = 6,
    DPP_ATTR_C_HDR = 7,
    DPP_ATTR_C_HDR10_PLUS = 8,
    DPP_ATTR_WCG = 9,
    DPP_ATTR_SBWC = 10,
    DPP_ATTR_HDR10_PLUS = 11,
    DPP_ATTR_IDMA = 16,
    DPP_ATTR_ODMA = 17,
    DPP_ATTR_DPP = 18,
    DPP_ATTR_MAX,
};

struct dpp_size_range {
    u32 min;
    u32 max;
    u32 align;
};
struct dpp_restriction {
    struct dpp_size_range src_f_w;
    struct dpp_size_range src_f_h;
    struct dpp_size_range src_w;
    struct dpp_size_range src_h;
    u32 src_x_align;
    u32 src_y_align;
    struct dpp_size_range dst_f_w;
    struct dpp_size_range dst_f_h;
    struct dpp_size_range dst_w;
    struct dpp_size_range dst_h;
    u32 dst_x_align;
    u32 dst_y_align;
    struct dpp_size_range blk_w;
    struct dpp_size_range blk_h;
    u32 blk_x_align;
    u32 blk_y_align;
    u32 src_h_rot_max;
    //u32 src_w_rot_max;
    u32 format[MAX_FMT_CNT];
    int format_cnt;
    u32 scale_down;
    u32 scale_up;
    u32 reserved[6];
};
struct dpp_ch_restriction {
    int id;
    int port = -1;
    int dpuf_id = -1;
    unsigned long attr;
    struct dpp_restriction restriction;
    u32 reserved[4];
};
struct dpp_restrictions_info {
    u32 ver;
    struct dpp_ch_restriction dpp_ch[MAX_DPP_CNT];
    int dpp_cnt;
    u32 reserved[4];
};

struct dpp_restrictions_info_v2 {
    u32 ver;
    struct dpp_ch_restriction dpp_ch[MAX_DPP_CNT_V2];
    int dpp_cnt;
    u32 reserved[4];
};
#endif
