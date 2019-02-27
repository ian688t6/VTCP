/**
 * @file	: vcp.h
 * @brief	: vcp (viehcle communication protocol - GB/JT-808)
 * @version :
 * @author	: jyin
 * @date	: Jun 19, 2018
 */

#ifndef __VTCP_H__
#define __VTCP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include "sock.h"
#include "vtcpmsg.h"
#include "tmng.h"


typedef enum {
	VTCP_REGISTER_SUCC 			= 0,
	VTCP_VIEHCLE_REGISTERED		= 1,
	VTCP_NO_VIEHCLE				= 2,
	VTCP_TERM_REGISTERED		= 3,
	VTCP_NO_TERM				= 4,
} vtcp_reg_code_e;

typedef struct {
	uint32_t		ui_len;
	uint8_t			auc_code[32];
} vtcp_authcode_s;

typedef struct {
	const char 		*pc_addr;
	uint16_t 		us_port;
	uint8_t			auc_telnum[6];
	vtcp_authcode_s	st_authcode;
} vtcp_cfg_s;

typedef int32_t (*vtcp_cb)(uint32_t ui_state, void *pv_data, uint32_t ui_size);
typedef struct {
	vtcp_cfg_s		st_cfg;
	pthread_t		tid;
	vtcp_cb			pf_cb;
	uint16_t		us_seqnum;
	pthread_mutex_t	st_lock;
} vtcp_s;

static inline uint8_t bin2bcd(uint8_t uc_val)
{
	return ((uc_val / 10) << 4) + uc_val % 10;
}

static inline uint8_t bcd2bin(uint8_t uc_val)
{
	return (uc_val & 0x0f) + (uc_val >> 4);
}

extern void vtcp_setconf(vtcp_cfg_s *pst_cfg);

extern void vtcp_getconf(vtcp_cfg_s *pst_cfg);

extern int32_t vtcp_conn(void);

extern void vtcp_disconn(void);

extern int32_t vtcp_isauth(void);

extern void vtcp_loop(vtcp_cb pf_cb);

extern int32_t vtcp_sendreq(uint16_t us_msgid, void *pv_buf, uint16_t us_len);

extern int32_t vtcp_gotresp(vtcpmsg_s *pst_msg, uint8_t *puc_payload);

extern int32_t vtcp_register(vtcp_reg_msg_s *pst_msg, vtcp_reg_rsp_s *pst_rsp);

#ifdef __cplusplus
}
#endif
#endif

