
/* 实现从一个H264裸码流数据帧中分离出NALU */

#ifndef H264_FRAME_SPLIT_H
#define H264_FRAME_SPLIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* last参数标示当前nalu是否是图像帧的最后一个nalu */
typedef void (*nalu_sink_f)(const unsigned char *nalu, unsigned len, int is_last, void *uerdata);

/* 可调用此函数处理一帧H.264裸码流数据，分离出的NALU单元通过提供的回调返回给user
 * 分离出的NALU结构是H.264白皮书"B.1.1字节流NAL单元语法"中nal_unit部分,
 * 不包含leading_zero_8bits/zero_byte/start_code_prefix_one_3bytes
 */
void h264_frame_split(const unsigned char *frame, int len, nalu_sink_f nalu_sink, void* userdata);

#ifdef __cplusplus
}
#endif

#endif /* !H264_FRAME_SPLIT_H */
