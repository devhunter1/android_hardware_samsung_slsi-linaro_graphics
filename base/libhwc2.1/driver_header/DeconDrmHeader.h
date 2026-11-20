#ifndef _DECON_DRM_HELPER_H
#define _DECON_DRM_HELPER_H

/* TODO(b/149514037): This header file should be removed */

struct drm_dpp_restriction {
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

    u32 src_w_rot_max; /* limit of source img width in case of rotation */
    u32 src_h_rot_max; /* limit of source img height in case of rotation */

    u32 scale_down;
    u32 scale_up;
};

struct drm_dpp_ch_restriction {
    int id;
    unsigned long attr;
    struct drm_dpp_restriction restriction;
};

typedef struct dpu_str_u32 {
    __s8 str[32];
    __u32 val;
} __str_u32;

typedef struct dpp_size_range __u32_range;

enum dpu_res_key {
    DPU_RES_ID,
    DPU_RES_PORT,
    DPU_RES_DPUF_ID,
    DPU_RES_ATTR,

    DPU_RES_SRC_F_W,
    DPU_RES_SRC_F_H,
    DPU_RES_SRC_W,
    DPU_RES_SRC_H,
    DPU_RES_SRC_X_ALIGN,
    DPU_RES_SRC_Y_ALIGN,
    DPU_RES_SRC_W_ROT_MAX,
    DPU_RES_SRC_H_ROT_MAX,

    DPU_RES_SRC_RGB_COMP_F_W,
    DPU_RES_SRC_RGB_COMP_W,
    DPU_RES_SRC_RGB_COMP_F_H,
    DPU_RES_SRC_RGB_COMP_H,

    DPU_RES_SRC_YUV_COMP_F_W,
    DPU_RES_SRC_YUV_COMP_W,
    DPU_RES_SRC_YUV_COMP_F_H,
    DPU_RES_SRC_YUV_COMP_H,

    DPU_RES_DST_F_W,
    DPU_RES_DST_F_H,
    DPU_RES_DST_W,
    DPU_RES_DST_H,
    DPU_RES_DST_X_ALIGN,
    DPU_RES_DST_Y_ALIGN,

    DPU_RES_BLK_W,
    DPU_RES_BLK_H,
    DPU_RES_BLK_X_ALIGN,
    DPU_RES_BLK_Y_ALIGN,

    DPU_RES_SCALE_DOWN,
    DPU_RES_SCALE_UP,

    DPU_RES_DISP_CLK_MAX,
    DPU_RES_DISP_CLK_MARGIN_PCT,
    DPU_RES_DISP_CLK_FACTOR_PCT,
    DPU_RES_DISP_CLK_PPC,

    DPU_RES_MODE_PLANE_CNT,
};

enum dpu_res_type {
    DPU_RES_TYPE_S32,
    DPU_RES_TYPE_U32,
    DPU_RES_TYPE_S64,
    DPU_RES_TYPE_U64,
    DPU_RES_TYPE_U32_RANGE,
    DPU_RES_TYPE_U64_BITMASK,
    DPU_RES_TYPE_STR_U32,
};

#define DEFINE_DPU_RESTRICT(t, T)						\
struct dpu_res_##t {								\
	enum dpu_res_type	type;						\
	enum dpu_res_key	key;						\
	__##t			val;						\
};										\
static inline void dpu_res_add_##t(char **data, size_t *size, \
				     enum dpu_res_key key, const void *val)               \
{                                      \
	struct dpu_res_##t *ptr = (struct dpu_res_##t *)(*data);     \
                                       \
	ptr->type = DPU_RES_TYPE_##T;         \
	ptr->key = (key);                     \
	ptr->val = *(__##t *)(val);           \
	*data += sizeof(*ptr);                \
	*size -= sizeof(*ptr);                \
}
DEFINE_DPU_RESTRICT(u32, U32)
DEFINE_DPU_RESTRICT(u64, U64)
DEFINE_DPU_RESTRICT(u32_range, U32_RANGE)
DEFINE_DPU_RESTRICT(str_u32, STR_U32)

#endif
