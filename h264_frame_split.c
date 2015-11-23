
#include "h264_frame_split.h"
#include "util/log.h"

#include <stdlib.h>

void h264_frame_split(const unsigned char *frame, int len, nalu_sink_f nalu_sink, void* userdata)
{
	const unsigned char *byte;
	unsigned int next3byte;
	const unsigned char *nalu1_begin;
	const unsigned char *nalu2_begin;

	if (NULL == frame || 0 == len || NULL == nalu_sink)
	{
		log_error("h264_frame_split: bad frame(%p) or bad len(%u) or bad nalu_sink(%p)", frame, len, nalu_sink);
		return;
	}
	
	byte = frame;
	while (byte < (frame+len))
	{
		next3byte = byte[0] << 16 | byte[1] << 8 | byte[2];
		
		/* Ѱ��start_code_prefix_one_3bytes */
		if (next3byte != 0x000001)
		{
			byte++;
			continue;
		}
		
		nalu1_begin = byte + 3;
		
		nalu2_begin = nalu1_begin + 1;		/* ����nalu_type�ֽ� */
		while (nalu2_begin < (frame+len))
		{
			next3byte = nalu2_begin[0] << 16 | nalu2_begin[1] << 8 | nalu2_begin[2];
			if (next3byte == 0x000001)
			{
				break;
			}
			
			nalu2_begin++;
		}
		
		if (nalu2_begin == (frame+len))
		{
			/* nalu1_beginָ���NAL unit�Ѿ��ǵ�ǰ֡�е����һ��NAL unit */
			nalu_sink(nalu1_begin, (nalu2_begin - nalu1_begin), 1, userdata);
			break;
		}
		else
		{
			/* nalu2_begin��ָ����һ��nalu��start_code_prefix */
			byte = nalu2_begin;
	
			nalu_sink(nalu1_begin, (nalu2_begin - nalu1_begin), 0, userdata);
		}
	}

    return;
}
