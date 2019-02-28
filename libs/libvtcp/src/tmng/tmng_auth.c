/**
 * @file	: tmng_auth.c
 * @brief	: terminal manage authorised
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

int32_t tmng_authmsg_enc(uint8_t *puc_payload, uint16_t *pus_payload_len, vtcp_auth_msg_s *pst_msg)
{
	int32_t i = 0;
	uint8_t *puc_ptr = puc_payload;
	
	if ((NULL == puc_payload) || (NULL == pst_msg)) {
		loge("tmng register invalid param");
		return -1;	
	}
	
	for (i = 0; i < pst_msg->us_len; i ++) {
		PUTCHAR(pst_msg->ac_authcode[i], puc_payload);
	}
	*pus_payload_len = puc_payload - puc_ptr;	

	return 0;
}

int32_t tmng_authrsp_dec(uint8_t *puc_payload, uint16_t us_len, vtcprsp_s *pst_resp)
{
	return vtcprsp_dec(puc_payload, us_len, pst_resp);
}

#ifdef __cplusplus
}
#endif

