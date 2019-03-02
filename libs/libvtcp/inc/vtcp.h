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

#define VTCP_DEFAULT_AUTHFILE	"auth.bin"

typedef enum {
	VTCP_REGISTER_SUCC 			= 0,
	VTCP_VIEHCLE_REGISTERED		= 1,
	VTCP_NO_VIEHCLE				= 2,
	VTCP_TERM_REGISTERED		= 3,
	VTCP_NO_TERM				= 4,
} vtcp_reg_code_e;

typedef enum {
	VTCP_RET_SUCC				= 0,
	VTCP_RET_FAIL				= 1,
	VTCP_RET_MSGERROR			= 2,
	VTCP_RET_NOTSUPPORT			= 3,
	VTCP_RET_ALERT				= 4,
} vtcp_retcode_e;

typedef struct {
	int32_t			ui_len;
	uint8_t			auc_code[32];
} vtcp_authcode_s;

typedef struct {
	const char 		*pc_addr;
	const char		*pc_authfile;
	uint16_t 		us_port;
	uint8_t			auc_telnum[6];
} vtcp_cfg_s;

typedef int32_t (*vtcp_cb)(uint16_t us_seqnum, uint16_t us_msgid, uint8_t *puc_payload, uint16_t us_len);
typedef struct {
	vtcp_cfg_s			st_cfg;
	pthread_t			tid;
	vtcp_cb				pf_cb;
	uint16_t			us_seqnum;
	pthread_mutex_t		st_lock;
	struct list_head 	vrb_list;
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

extern int32_t vtcp_authsave(vtcp_authcode_s *pst_auth);

extern int32_t vtcp_authload(vtcp_authcode_s *pst_auth);

extern void vtcp_authrm(void);

extern int32_t vtcp_isauth(void);

extern void vtcp_loop(vtcp_cb pf_cb);

extern int32_t vtcp_register(vtcp_reg_msg_s *pst_msg, vtcp_reg_rsp_s *pst_rsp);

extern int32_t vtcp_authorise(vtcp_auth_msg_s *pst_msg, vtcprsp_s *pst_rsp);

extern int32_t vtcp_hb(vtcprsp_s *pst_rsp);

extern int32_t vtcp_unregister(vtcprsp_s *pst_rsp);

extern void vtcp_setlog(uint32_t ui_level);

#ifdef __cplusplus
}
#endif
#endif

