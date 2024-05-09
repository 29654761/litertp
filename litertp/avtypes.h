/**
 * @file avtype.h
 * @brief Media struct defined.
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#ifndef __AV_TYPES_H__
#define __AV_TYPES_H__

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif




typedef enum media_type_t
{
	media_type_data = 0,
	media_type_audio = 1,
	media_type_video = 2,
}media_type_t;

typedef enum codec_type_t
{
	codec_type_unknown = 0,
	codec_type_pcma=1,
	codec_type_pcmu = 2,
	codec_type_g722 = 3,
	codec_type_opus = 4,
	codec_type_mpeg4_generic = 5,   // aac rfc-3640
	codec_type_mp4a_latm = 6,		// aac rfc-3016
	codec_type_telephone_event=7,
	codec_type_cn=8,
	codec_type_mp2p=9,
	codec_type_mp2s=10,

	codec_type_h264 = 101,
	codec_type_h265 = 102,
	codec_type_vp8 = 103,
	codec_type_vp9 = 104,
	codec_type_av1=105,

	codec_type_rtx = 200,
	codec_type_red = 201,
	codec_type_ulpfec = 202,
}codec_type_t;



typedef enum color_format_t
{
	color_format_unknown = 0,
	color_format_rgb24 = 1,
	color_format_rgb32 = 2,
	color_format_yuv444 = 3,
	color_format_yuv422 = 4,
	color_format_yuv420 = 5,
	color_format_mjpeg = 10
}color_format_t;

typedef struct av_rect_t
{
	int x;
	int y;
	int cx;
	int cy;
}av_rect_t;

typedef struct av_frame_t
{
	int64_t pts;
	int64_t dts;
	uint8_t* data;
	uint32_t data_size;
	media_type_t mt;
	codec_type_t ct;
	uint32_t duration;
}av_frame_t;


typedef enum transform_order_t {
	rtp_transform_rotate_scale = 1,		// rotate before scale
	rtp_transform_scale_rotate = 2,		// scale before rotate
}transform_order_t;

typedef enum transform_scale_t {
	rtp_transform_scale_none = 0,			// no scaling
	rtp_transform_scale_x = 1,			// reverse on horizontal
	rtp_transform_scale_y = 2,			// reverse on vertical
}transform_scale_t;

typedef enum transform_angle_t {
	rtp_transform_rotate_0 = 0,			// no rotating
	rtp_transform_rotate_90 = 1,		// rotate clockwise 90
	rtp_transform_rotate_180 = 2,		// rotate clockwise 180
	rtp_transform_rotate_270 = 3,		// rotate clockwise 270
}transform_angle_t;

typedef union transform_t
{
	uint32_t v;
	struct {
		transform_order_t order : 2;
		transform_scale_t scale : 3;
		transform_angle_t angle : 3;
	} s;
}transform_t;


typedef struct av_picture_t
{
	color_format_t fmt;
	int width;
	int height;
	unsigned char* buffer[4];
	int stride[4];
	uint32_t transform;
}av_picture_t;

typedef struct av_pcm_t
{
	int samples;
	int sample_rate;
	int channel;
	int bits_per_sample;
	uint8_t* buffer;
}av_pcm_t;


typedef void (*av_picture_event)(void* ctx,const av_picture_t& picture);
typedef void (*av_pcm_event)(void* ctx, const av_pcm_t& pcm);
typedef void (*av_frame_event)(void* ctx, const av_frame_t& frame);

#ifdef __cplusplus
}
#endif

#endif // !__AV_TYPES_H__