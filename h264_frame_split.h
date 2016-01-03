
/* ʵ�ִ�һ��H264����������֡�з����NALU */

#ifndef H264_FRAME_SPLIT_H
#define H264_FRAME_SPLIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* last������ʾ��ǰnalu�Ƿ���ͼ��֡�����һ��nalu */
typedef void (*nalu_sink_f)(const unsigned char *nalu, unsigned len, int is_last, void *uerdata);

/* �ɵ��ô˺�������һ֡H.264���������ݣ��������NALU��Ԫͨ���ṩ�Ļص����ظ�user
 * �������NALU�ṹ��H.264��Ƥ��"B.1.1�ֽ���NAL��Ԫ�﷨"��nal_unit����,
 * ������leading_zero_8bits/zero_byte/start_code_prefix_one_3bytes
 */
void h264_frame_split(const unsigned char *frame, int len, nalu_sink_f nalu_sink, void* userdata);

#ifdef __cplusplus
}
#endif

#endif /* !H264_FRAME_SPLIT_H */
