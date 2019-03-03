/**
 * @file	: vtcpmsg.h
 * @brief	: vtcp msg encode and decode
 * @version :
 * @author	: jyin
 * @date	: Jun 19, 2018
 */

#ifndef __VTCPMSG_H__
#define __VTCPMSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VTCP_MSG_LEN				(0x500)
#define VTCP_PAYLOAD_LEN			(0x400)
#define VTCP_ID_CODE				(0x7e)

enum {
	VTCP_MSG_RESP 				= 0x0001,
	VTCP_MSG_REGISTER 			= 0x0100,
	VTCP_MSG_AUTHORISE 			= 0x0102,
	VTCP_MSG_HB 				= 0x0002,
	VTCP_MSG_UNREGISTER 		= 0x0003,
	VTCP_MSG_CTLVIECLE			= 0x8105,
};

#define PLATFORM_MSG(msgcode) (msgcode | (0x01 << 15))

#define GETCHAR(c, cp) { \
	unsigned char *t_cp = (unsigned char *)(cp); \
	(c) = (t_cp[0]); \
	(cp) ++; \
}

#define GETSHORT(s, cp) { \
	unsigned char *t_cp = (unsigned char *)(cp); \
	(s) = ((uint16_t)t_cp[0] << 8) \
	    | ((uint16_t)t_cp[1]) \
	    ; \
	(cp) += 2; \
}

#define GETLONG(l, cp) { \
	unsigned char *t_cp = (unsigned char *)(cp); \
	(l) = ((uint32_t)t_cp[0] << 24) \
	    | ((uint32_t)t_cp[1] << 16) \
	    | ((uint32_t)t_cp[2] << 8) \
	    | ((uint32_t)t_cp[3]) \
	    ; \
	(cp) += 4; \
}

#define PUTCHAR(c, cp) { \
	unsigned char t_c = (unsigned char)(c); \
	unsigned char *t_cp = (unsigned char *)(cp); \
	*t_cp++ = t_c; \
	(cp) ++;\
}

#define PUTSHORT(s, cp) { \
	uint16_t t_s = (uint16_t)(s); \
	unsigned char *t_cp = (unsigned char *)(cp); \
	*t_cp++ = t_s >> 8; \
	*t_cp   = t_s; \
	(cp) += 2; \
}

#define PUTLONG(l, cp) { \
	uint32_t t_l = (uint32_t)(l); \
	unsigned char *t_cp = (unsigned char *)(cp); \
	*t_cp++ = t_l >> 24; \
	*t_cp++ = t_l >> 16; \
	*t_cp++ = t_l >> 8; \
	*t_cp   = t_l; \
	(cp) += 4; \
}

typedef struct {
	uint16_t 	us_msgid;
	union {
		uint16_t val;
		struct {
			uint16_t len	:10;
			uint16_t crypt	:3;
			uint16_t split	:1;
			uint16_t resv	:2;
		} prop;
	} un_msgprop;
	uint8_t auc_bcd[6];
	uint16_t us_seqnum;
	uint32_t ui_spack;
} __attribute__((packed)) vtcpmsg_hdr_s;

typedef struct {
	uint8_t  		uc_id0;
	vtcpmsg_hdr_s 	st_msghdr;
	uint8_t 		*pauc_payload;
	uint8_t			uc_crc;
	uint8_t			uc_id1;
} __attribute__((packed)) vtcpmsg_s;

typedef struct {
	uint16_t		us_seqnum;
	uint16_t		us_msgid;
	uint8_t			uc_retcode;
} __attribute__((packed)) vtcprsp_s;

typedef struct {
	int32_t			ui_len;
	uint8_t			auc_buf[VTCP_MSG_LEN];
} vtcpmsg_buf_s;

extern int32_t vtcpmsg_enc(vtcpmsg_s *pst_msg, vtcpmsg_buf_s *pst_buf);

extern int32_t vtcpmsg_dec(vtcpmsg_buf_s *pst_buf, vtcpmsg_s *pst_msg, uint8_t *puc_payload);

extern int32_t vtcprsp_dec(uint8_t *puc_payload, uint16_t us_len, vtcprsp_s *pst_resp);

extern void vtcpmsg_make(uint16_t us_msgid, uint8_t *puc_payload, uint32_t ui_payload_len, vtcpmsg_s *pst_msg);

extern int32_t vtcpmsg_calc_crc(vtcpmsg_hdr_s *pst_hdr, uint8_t *puc_payload);

extern void vtcpmsg_dump(vtcpmsg_s *pst_msg);

extern void vtcpmsg_buf_dump(vtcpmsg_buf_s *pst_msgbuf);

#ifdef __cplusplus
}
#endif
#endif
