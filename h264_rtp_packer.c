
#include "h264_rtp_packer.h"
#include "util/log.h"
#include "rtp/rtp_rtcp_packet.h"

#include <stdlib.h>
#include <string.h>

#ifdef WIN32
	#include <winsock2.h>			/* for htons()/htonl() */
#else
	#include <arpa/inet.h>			/* for htons()/htonl() */
#endif

struct h264_rtp_packer
{
	rtp_sink_f rtp_sink;
	void *userdata;

	unsigned short sn;
	unsigned timestamp;

	unsigned max_playload_len;
	struct
	{
		rtp_head_t *head;
		unsigned char *payload;
	}packet;
};

h264_rtp_packer_t* h264_rtp_packer_new(unsigned pt, unsigned init_sn, unsigned ssrc, unsigned max_playload_len, rtp_sink_f sink, void *userdata)
{
	h264_rtp_packer_t* packer;

	if (NULL == sink)
	{
		log_error("h264_rtp_packer_new: bad rtp sink");
		return NULL;
	}

	packer = (h264_rtp_packer_t*)malloc(sizeof(h264_rtp_packer_t)+sizeof(rtp_head_t)+max_playload_len);

	memset(packer, 0, sizeof(h264_rtp_packer_t)+sizeof(rtp_head_t)+max_playload_len);
	packer->rtp_sink = sink;
	packer->userdata = userdata;
	packer->max_playload_len = max_playload_len;
	packer->sn = init_sn;

	packer->packet.head = (rtp_head_t*)&packer[1];
	packer->packet.payload = (unsigned char*)&packer->packet.head[1];

	packer->packet.head->V = 2;
	packer->packet.head->P = 0;
	packer->packet.head->X = 0;
	packer->packet.head->CC = 0;	
	packer->packet.head->PT = pt & 0x7f;
	packer->packet.head->ssrc = htonl(ssrc);

	/* ���¼��������ڴ�������лᶯ̬�仯 */
	packer->packet.head->M = 0;
	packer->packet.head->SN = 0;
	packer->packet.head->timestamp = 0;

	return packer;
}

void h264_rtp_packer_destroy(h264_rtp_packer_t* packer)
{
	free(packer);

	return;
}

void h264_rtp_packer_pack(h264_rtp_packer_t* packer, const unsigned char *nalu, unsigned len, unsigned ts, int is_last)
{
	unsigned packet_size;
	unsigned char* payload;
	unsigned max_playload_len;
	unsigned char nalu_type;
	const unsigned char *nalu_ptr;

	if (NULL == packer || NULL == nalu || 0 == len)
	{
		log_error("h264_rtp_packer_pack: bad packer(%p) or bad nalu(%p) or bad nalu len(%u)", packer, nalu, len);
		return;
	}

	packer->packet.head->timestamp = htonl(ts);

	payload = packer->packet.payload;
	max_playload_len = packer->max_playload_len;
	
	if (len <= max_playload_len)
	{
		/* ��ǰNALU������һ��RTP�������ɣ������single NALU mode */
		packet_size = sizeof(rtp_head_t);
		memcpy(payload, nalu, len);
		packet_size += len;
		if (is_last)
		{
			packer->packet.head->M = 1;
		}
		else
		{
			packer->packet.head->M = 0;
		}
		packer->packet.head->SN = htons(packer->sn);
		packer->rtp_sink((unsigned char*)packer->packet.head, packet_size, packer->userdata);
		packer->sn++;
	}
	else
	{
		/* ��Ҫ���з�Ƭ���� */
		
		nalu_type = nalu[0];
		nalu_ptr = nalu;
		
		packer->packet.head->M = 0;
		
		/* ��ʼƬ */
		packet_size = sizeof(rtp_head_t);
		/* FU indicator */
		payload[0] = (nalu_type & 0xE0) | 28;
		/* FU header */
		payload[1] = (nalu_type & 0x1F) | 0x80;
		nalu_ptr++;
		memcpy(payload+2, nalu_ptr, max_playload_len-2);
		packet_size += max_playload_len;

		packer->packet.head->SN = htons(packer->sn);
		packer->sn++;
		packer->rtp_sink((unsigned char*)packer->packet.head, packet_size, packer->userdata);

		/* payload ���Ѿ������2���ֽ�
		 * ��ʼƬ���ĵ�NALU��(max_playload_len-1)��bytes
		 */
		nalu_ptr += max_playload_len-2;
		len -= max_playload_len - 1;
		
		while (len > (max_playload_len-2))
		{
			/* ������Ƭ */
			
			/* �м�Ƭ */
			packet_size = sizeof(rtp_head_t);
			/* FU indicator */
			payload[0] = (nalu_type & 0xE0) | 28;
			/* FU header */
			payload[1] = (nalu_type & 0x1F);

			memcpy(payload+2, nalu_ptr, max_playload_len-2);
			packet_size += max_playload_len;

			packer->packet.head->SN = htons(packer->sn);
			packer->rtp_sink((unsigned char*)packer->packet.head, packet_size, packer->userdata);
			packer->sn++;

			/* ���м�Ƭ���ĵ�(max_playload_len-2)��bytes */
			nalu_ptr += max_playload_len-2;
			len -= max_playload_len-2;
		}

		/* ����Ƭ */
		packet_size = sizeof(rtp_head_t);
		/* FU indicator */
		payload[0] = (nalu_type & 0xE0) | 28;
		/* FU header */
		payload[1] = (nalu_type & 0x1F) | 0x40;
		memcpy(payload+2, nalu_ptr, len);
		packet_size += len + 2;
		if (is_last)
		{
			packer->packet.head->M = 1;
		}
		
		packer->packet.head->SN = htons(packer->sn);
		packer->rtp_sink((unsigned char*)packer->packet.head, packet_size, packer->userdata);
		packer->sn++;
	}

	return;
}