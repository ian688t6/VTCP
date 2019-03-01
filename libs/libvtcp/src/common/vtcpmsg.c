/**
 * @file	: vtcpmsg.c
 * @brief	: vtcp msg encode and decode
 * @version :
 * @author	: jyin
 * @date	: Jun 19, 2018
 */
#include "os.h"
#include "log.h"
#include "vtcp.h"

#ifdef __cplusplus
extern "C" {
#endif

static int32_t check_escape(uint8_t *puc_seq, int32_t ui_len)
{
	int32_t i = 0;

	for (i = 1; i < ui_len - 1; i ++) {
		if ((0x7e == puc_seq[i]) || (0x7d == puc_seq[i]))
				return 1;
	}

	return 0;
}

static int32_t check_descape(uint8_t *puc_seq, int32_t ui_len)
{
	int32_t i = 0;

	for (i = 1; i < ui_len - 1; i ++) {
		if (0x7d == puc_seq[i])
				return 1;
	}

	return 0;
}

static void escape(uint8_t *puc_seq, uint32_t ui_srclen, int32_t *pui_dstlen)
{
	uint32_t ui_len = ui_srclen;
	uint8_t *puc_ch	= NULL;
	uint8_t *puc_nseq = NULL;

	logi("do escape: len = %d", ui_srclen);
	/* Todo: alloc new seq */
	puc_nseq = (uint8_t *)malloc(ui_srclen);
	if (!puc_nseq) {
		loge("escape alloc failed!");
		return;
	}
	memcpy(puc_nseq, puc_seq, ui_srclen);
	/* Todo: skip seq[0] */
	puc_seq ++;	
	for (puc_ch = puc_nseq + 1; puc_ch < &puc_nseq[ui_srclen - 1]; puc_ch ++) {
		switch (*puc_ch) {
		case 0x7e:
		*puc_seq++ = 0x7d;	
		*puc_seq++ = 0x02;
		ui_len ++;
		break;
		case 0x7d:
		*puc_seq++ = 0x7d;	
		*puc_seq++ = 0x01;	
		ui_len ++;
		break;
		default:
		*puc_seq++ = *puc_ch;	
		break;
		}
	}
	*puc_seq = puc_nseq[ui_srclen - 1];
	*pui_dstlen = ui_len;
	free(puc_nseq);
	
	return;
}

static void descape(uint8_t *puc_seq, uint32_t ui_len, int32_t *pui_dstlen)
{
	int32_t i = 0;
	uint8_t *puc_tmp = puc_seq;
	uint8_t *puc_copy_seq = NULL;

	puc_copy_seq = (uint8_t *)malloc(ui_len);
	if (NULL == puc_copy_seq) {
		return;
	}
	memcpy(puc_copy_seq, puc_seq, ui_len);
	memset(puc_seq, 0x0, ui_len);
	*puc_seq ++ = puc_copy_seq[0]; 
	for (i = 1; i < ui_len - 1; i ++) {
		if (0x7d == puc_copy_seq[i]) {
			switch (puc_copy_seq[i + 1]) {
			case 0x01:
			*puc_seq ++ = 0x7d;
			i ++;
			break;
			case 0x02:
			*puc_seq ++ = 0x7e;
			i ++;
			break;
			default: break;
			}
			continue;
		}
		*puc_seq ++ = puc_copy_seq[i];
	}
	*puc_seq ++ = puc_copy_seq[ui_len - 1]; 
	*pui_dstlen = puc_seq - puc_tmp;
	free(puc_copy_seq);
	
	return;
}

int32_t vtcpmsg_calc_crc(vtcpmsg_hdr_s *pst_hdr, uint8_t *puc_payload)
{
	uint32_t i = 0;
	uint8_t *puc_hdr = (uint8_t *)pst_hdr;
	uint32_t ui_hdr_len = sizeof(*pst_hdr) - sizeof(pst_hdr->ui_spack);
	uint8_t uc_crc = puc_hdr[0];

	for (i = 1; i < ui_hdr_len; i ++) {
		uc_crc ^= puc_hdr[i];
	}
	
	if (puc_payload) {
		for (i = 0; i < pst_hdr->un_msgprop.prop.len; i ++) {
			uc_crc ^= puc_payload[i];
		}
	}
	
	return uc_crc;
}

int32_t vtcpmsg_enc(vtcpmsg_s *pst_msg, vtcpmsg_buf_s *pst_buf)
{
	int32_t i = 0;
	uint8_t *puc_buf = NULL;
	
	if ((NULL == pst_msg) || (NULL == pst_buf)) {
		loge("vtcp_enc_msg invalid param!");
		return -1;
	}
	
	puc_buf = pst_buf->auc_buf;
	PUTCHAR(pst_msg->uc_id0, puc_buf);
	PUTSHORT(pst_msg->st_msghdr.us_msgid, puc_buf);
	PUTSHORT(pst_msg->st_msghdr.un_msgprop.val, puc_buf);
	for (i = 0; i < sizeof(pst_msg->st_msghdr.auc_bcd); i ++) {
		PUTCHAR(pst_msg->st_msghdr.auc_bcd[i], puc_buf);
	}
	PUTSHORT(pst_msg->st_msghdr.us_seqnum, puc_buf);
	if (pst_msg->pauc_payload && (0 < pst_msg->st_msghdr.un_msgprop.prop.len)) {
		memcpy(puc_buf, pst_msg->pauc_payload, pst_msg->st_msghdr.un_msgprop.prop.len);
		puc_buf += pst_msg->st_msghdr.un_msgprop.prop.len;
	}
	PUTCHAR(pst_msg->uc_crc, puc_buf);
	PUTCHAR(pst_msg->uc_id1, puc_buf);
	if (check_escape(pst_buf->auc_buf, (puc_buf - pst_buf->auc_buf))) {
		escape(pst_buf->auc_buf, (puc_buf - pst_buf->auc_buf), &pst_buf->ui_len);
	} else {
		pst_buf->ui_len = (puc_buf- pst_buf->auc_buf);
	}
//	vtcpmsg_buf_dump(pst_buf);
	
	return 0;
}

int32_t vtcpmsg_dec(vtcpmsg_buf_s *pst_buf, vtcpmsg_s *pst_msg, uint8_t *puc_payload)
{
	int32_t i = 0;
	uint8_t uc_crc = 0;
	uint8_t *puc_buf = pst_buf->auc_buf;

	if (check_descape(pst_buf->auc_buf, pst_buf->ui_len)) {
		descape(pst_buf->auc_buf, pst_buf->ui_len, &pst_buf->ui_len);
		vtcpmsg_buf_dump(pst_buf);
	}

	GETCHAR(pst_msg->uc_id0, puc_buf);
	GETSHORT(pst_msg->st_msghdr.us_msgid, puc_buf);
	GETSHORT(pst_msg->st_msghdr.un_msgprop.val, puc_buf);
	for (i = 0; i < sizeof(pst_msg->st_msghdr.auc_bcd); i ++) {
		GETCHAR(pst_msg->st_msghdr.auc_bcd[i], puc_buf);
	}
	GETSHORT(pst_msg->st_msghdr.us_seqnum, puc_buf);
	if ((0 < pst_msg->st_msghdr.un_msgprop.prop.len) && puc_payload) {
		memcpy(puc_payload, puc_buf, pst_msg->st_msghdr.un_msgprop.prop.len);
		pst_msg->pauc_payload = puc_payload;
		puc_buf += pst_msg->st_msghdr.un_msgprop.prop.len;
	}
	uc_crc = vtcpmsg_calc_crc(&pst_msg->st_msghdr, pst_msg->pauc_payload);
	GETCHAR(pst_msg->uc_crc, puc_buf);
	if (pst_msg->uc_crc != uc_crc) {
		loge("we got vtcp msg crc wrong calc crc%02x %02x!", uc_crc, pst_msg->uc_crc);
		return -1;
	}
	GETCHAR(pst_msg->uc_id1, puc_buf);
	vtcpmsg_dump(pst_msg);

	return 0;
}

int32_t vtcprsp_dec(uint8_t *puc_payload, uint16_t us_len, vtcprsp_s *pst_resp)
{
	if ((NULL == pst_resp) || (NULL == puc_payload) || 
			(us_len != sizeof(vtcprsp_s))) {
		return -1;
	}
	
	GETSHORT(pst_resp->us_seqnum, puc_payload);
	GETSHORT(pst_resp->us_msgid, puc_payload);
	GETCHAR(pst_resp->uc_retcode, puc_payload);
	
	return 0;
}

void vtcpmsg_dump(vtcpmsg_s *pst_msg)
{
	int32_t i = 0;

	logi("VTCPID: %02x", pst_msg->uc_id0);
	logi("vtcpmsg header: msgid-%04x, prop-%04x len-%04x crypt-%04x split-%04x resv-%04x",
		pst_msg->st_msghdr.us_msgid, 
		pst_msg->st_msghdr.un_msgprop.val,
		pst_msg->st_msghdr.un_msgprop.prop.len,
		pst_msg->st_msghdr.un_msgprop.prop.crypt,
		pst_msg->st_msghdr.un_msgprop.prop.split,
		pst_msg->st_msghdr.un_msgprop.prop.resv);
	logi("vtcpmsg header: bcd-%02x %02x %02x %02x %02x %02x",
		pst_msg->st_msghdr.auc_bcd[0], pst_msg->st_msghdr.auc_bcd[1], 
		pst_msg->st_msghdr.auc_bcd[2], pst_msg->st_msghdr.auc_bcd[3],
		pst_msg->st_msghdr.auc_bcd[4], pst_msg->st_msghdr.auc_bcd[5]);
	logi("vtcpmsg seqnum: %04x", pst_msg->st_msghdr.us_seqnum);
	logi("vtcpmsg payload:");
	for (i = 0; i < pst_msg->st_msghdr.un_msgprop.prop.len; i ++) {
		printf("%02x ", pst_msg->pauc_payload[i]);
	}
	printf("\n");
	logi("vtcpmsg crc: %02x", pst_msg->uc_crc);

	return;
}

void vtcpmsg_buf_dump(vtcpmsg_buf_s *pst_buf)
{
	int32_t i = 0;
	
	logi("VTCPMSG BUF:");
	for (i = 0; i < pst_buf->ui_len; i ++) {
		printf("%02x ", pst_buf->auc_buf[i]);	
	}
	printf("\n");
}

#ifdef __cplusplus
}
#endif

