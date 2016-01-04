
#include "h264_frame_split.h"

void h264_frame_split(const unsigned char *frame, int len, nalu_sink_f nalu_sink, void* userdata)
{
	const unsigned char *byte;
	unsigned int next4bytes;
	const unsigned char *nalu1_begin;
	const unsigned char *nalu2_begin;
	
	/* H264��Ƥ�� B.1.2 �ڶ� NALU ����ṹ˵������
	 *
	 * leading_zero_8bits: 0x00 �� NALU Ϊ�ֽ����ĵ�һ�� NALU ʱ����
	 * zero_byte: 0x00 �� NALU Ϊ SPS/PPS, ��Ϊ Access Unit �ĵ�һ�� NALU ʱ����
	 * start_code_prefix_one_3bytes: 0x000001, NALU ��ʼ��ǰ׺
	 *  < ����� NALU ���� >
	 * trailing_zero_8bits: 0x00
	 *
	 * ���������������Կ�������� NALU �����Ǳ� 0x00000001 ���ָ�ģ��������� 0 �ֽ�
	 * �����ָ���̼��ǻ��������ṹ�����е�
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
		
		/* ��������� start_code_prefix_one_3bytes���Լ� leading_zero_8bits ��ǰһ�� NALU �� trailing_zero_8bits */
		nalu1_begin = byte + 4;
		
		nalu2_begin = nalu1_begin + 1;		/* ����nalu_type�ֽ� */
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
			/* nalu1_begin ָ��� NALU �Ѿ��ǵ�ǰ֡�е����һ�� NALU */
			
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
			/* nalu2_begin ��ָ����һ��nalu��start_code_prefix */
			byte = nalu2_begin;
	
			nalu_sink(nalu1_begin, (nalu2_begin - nalu1_begin), 0, userdata);
		}
	}

    return;
}
