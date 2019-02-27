/**
 * @file	: tmng.h
 * @brief	: terminal manage
 * @version :
 * @author	: jyin
 * @date	: Jun 19, 2018
 */

#ifndef __TMNG_H__
#define __TMNG_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Reference JT-808 Terminal Register Message */
typedef struct tmng_reg_msg {
	uint16_t	us_provid;
	uint16_t	us_cityid;
	uint8_t		auc_manufacid[5];
	uint8_t		auc_ttype[8];
	uint8_t		auc_tid[7];
	uint8_t		uc_pltcolor;
	char		ac_pltnums[32];
} __attribute__((packed)) vtcp_reg_msg_s;

/** Reference JT-808 Terminal Register Reply */
typedef struct tmng_reg_reply {
	uint16_t	us_seqnum;
	uint8_t		uc_retcode;
	uint32_t	ui_authcode_len;
	char		ac_authcode[32];
} __attribute__((packed)) vtcp_reg_rsp_s;

extern int32_t tmng_regmsg_enc(uint8_t *puc_payload, uint16_t *pus_payload_len, vtcp_reg_msg_s *pst_msg);

extern int32_t tmng_regrsp_dec(uint8_t *puc_payload, uint16_t us_len, vtcp_reg_rsp_s *pst_resp);

#ifdef __cplusplus
}
#endif
#endif
