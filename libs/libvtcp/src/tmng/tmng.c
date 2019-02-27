/**
 * @file	: tmng.h
 * @brief	: terminal manage
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

int32_t tmng_regmsg_enc(uint8_t *puc_payload, uint16_t *pus_payload_len, vtcp_reg_msg_s *pst_msg)
{
	int32_t i = 0;
	uint8_t *puc_ptr = puc_payload;

	if ((NULL == puc_payload) || (NULL == pst_msg)) {
		loge("tmng register invalid param");
		return -1;	
	}
	
	PUTSHORT(pst_msg->us_provid, puc_payload);
	PUTSHORT(pst_msg->us_cityid, puc_payload);
	for (i = 0; i < sizeof(pst_msg->auc_manufacid); i ++) {
		PUTCHAR(pst_msg->auc_manufacid[i], puc_payload);
	}
	
	for (i = 0; i < sizeof(pst_msg->auc_ttype); i ++) {
		PUTCHAR(pst_msg->auc_ttype[i], puc_payload);
	}
	
	for (i = 0; i < sizeof(pst_msg->auc_tid); i ++) {
		PUTCHAR(pst_msg->auc_tid[i], puc_payload);
	}
	PUTCHAR(pst_msg->uc_pltcolor, puc_payload);
	for (i = 0; i < strlen(pst_msg->ac_pltnums) + 1; i ++) {
		PUTCHAR(pst_msg->ac_pltnums[i], puc_payload);	
	}
	*pus_payload_len = puc_payload - puc_ptr;	
	
	return 0;
}

int32_t tmng_regrsp_dec(uint8_t *puc_payload, uint16_t us_len, vtcp_reg_rsp_s *pst_resp)
{
	int32_t i = 0;

	if (NULL == pst_resp) {
		return -1;
	}

	GETSHORT(pst_resp->us_seqnum, puc_payload);
	us_len -= sizeof(uint16_t);
	GETCHAR(pst_resp->uc_retcode, puc_payload);
	us_len --;
	for (i = 0; i < us_len; i ++) {
		GETCHAR(pst_resp->ac_authcode[i], puc_payload);
	}
	pst_resp->ui_authcode_len = us_len;
//	logi("len: %d authcode: %s", pst_resp->ui_authcode_len, pst_resp->ac_authcode);

	return 0;
}

#ifdef __cplusplus
}
#endif

