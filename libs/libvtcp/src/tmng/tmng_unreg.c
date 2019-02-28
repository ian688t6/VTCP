/**
 * @file	: tmng_unreg.c
 * @brief	: terminal manage unregister
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

int32_t tmng_unregrsp_dec(uint8_t *puc_payload, uint16_t us_len, vtcprsp_s *pst_resp)
{
	return vtcprsp_dec(puc_payload, us_len, pst_resp);
}

#ifdef __cplusplus
}
#endif

