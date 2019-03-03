/**
 * @file	: tmng_resp.c
 * @brief	: vtcp terminal resp
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

int32_t tmng_commrsp_enc(uint8_t *puc_payload, uint16_t *pus_payload_len, vtcprsp_s *pst_resp)
{
	if ((NULL == puc_payload) || (NULL == pst_resp)) {
		loge("tmng resp invalid param");
		return -1;	
	}
	
	memcpy(puc_payload, pst_resp, sizeof(vtcprsp_s));
	*pus_payload_len = sizeof(vtcprsp_s);
	
	return 0;
}

#ifdef __cplusplus
}
#endif
