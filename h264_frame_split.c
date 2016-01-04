
#include "h264_frame_split.h"

void h264_frame_split(const unsigned char *frame, int len, nalu_sink_f nalu_sink, void* userdata)
{
	const unsigned char *byte;
	unsigned int next4bytes;
	const unsigned char *nalu1_begin;
	const unsigned char *nalu2_begin;
	
	/* H264白皮书 B.1.2 节对 NALU 语义结构说明如下
	 *
	 * leading_zero_8bits: 0x00 当 NALU 为字节流的第一个 NALU 时包含
	 * zero_byte: 0x00 当 NALU 为 SPS/PPS, 或为 Access Unit 的第一个 NALU 时包含
	 * start_code_prefix_one_3bytes: 0x000001, NALU 起始码前缀
	 *  < 具体的 NALU 数据 >
	 * trailing_zero_8bits: 0x00
	 *
	 * 综上述条件，可以看出具体的 NALU 数据是被 0x00000001 所分割的，或额外包含 0 字节
	 * 下述分割过程既是基于上述结构来进行的
	 */

	byte = frame;
	while ((byte+4) < (frame+len))
	{
		next4bytes = (byte[0] << 24) | (byte[1] << 16) | (byte[2] << 8) | byte[3];
		
		if (next4bytes != 0x00000001)
		{
			byte++;
			continue;
		}
		
		/* 跳过自身的 start_code_prefix_one_3bytes，以及 leading_zero_8bits 或前一个 NALU 的 trailing_zero_8bits */
		nalu1_begin = byte + 4;
		
		nalu2_begin = nalu1_begin + 1;		/* 跳过nalu_type字节 */
		while ((nalu2_begin+4) < (frame+len))
		{
			next4bytes = (nalu2_begin[0] << 24) | (nalu2_begin[1] << 16) | (nalu2_begin[2] << 8) | nalu2_begin[3];
			if (next4bytes == 0x00000001)
			{
				break;
			}
			
			nalu2_begin++;
		}
		
		if ((nalu2_begin+4) == (frame+len))
		{
			/* nalu1_begin 指向的 NALU 已经是当前帧中的最后一个 NALU */
			
			if (next4bytes == 0x00000001)
			{
				nalu_sink(nalu1_begin, (nalu2_begin - nalu1_begin), 1, userdata);
			}
			else
			{
				nalu_sink(nalu1_begin, (frame+len - nalu1_begin), 1, userdata);
			}

			break;
		}
		else
		{
			/* nalu2_begin 是指向下一个nalu的start_code_prefix */
			byte = nalu2_begin;
	
			nalu_sink(nalu1_begin, (nalu2_begin - nalu1_begin), 0, userdata);
		}
	}

    return;
}
