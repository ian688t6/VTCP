/**
 * @file	: vcp.c
 * @brief	: vcp (viehcle communication protocol)
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

static vtcp_s gst_vtcp;

static void make_vtcpmsg(uint16_t us_msgid, uint8_t *puc_payload, uint32_t ui_payload_len, vtcpmsg_s *pst_msg)
{
	vtcp_s *pst_vtcp = &gst_vtcp; 
	
	pst_msg->uc_id0 							= VTCP_ID_CODE;
	pst_msg->pauc_payload 						= puc_payload;
	pst_msg->st_msghdr.us_msgid 				= us_msgid;
	pst_msg->st_msghdr.un_msgprop.prop.len 		= ui_payload_len;
	pst_msg->st_msghdr.un_msgprop.prop.crypt 	= 0;
	pst_msg->st_msghdr.un_msgprop.prop.split 	= 0;
	memcpy(pst_msg->st_msghdr.auc_bcd, pst_vtcp->st_cfg.auc_telnum, sizeof(pst_msg->st_msghdr.auc_bcd));
	pst_msg->st_msghdr.us_seqnum 				= pst_vtcp->us_seqnum;
	pst_msg->uc_crc 							= vtcpmsg_calc_crc(&pst_msg->st_msghdr, pst_msg->pauc_payload);
	pst_msg->uc_id1 							= VTCP_ID_CODE;
	pst_vtcp->us_seqnum ++;
	
	return;
}

void *loopreqs(void *pv_arg)
{
	for (;;) {
	}	

	return NULL;
}

void looprecv(void)
{
	return;
}

void vtcp_setconf(vtcp_cfg_s *pst_val)
{
	vtcp_cfg_s *pst_cfg = &gst_vtcp.st_cfg;
	*pst_cfg = *pst_val;

	logi("vtcp server-%s:%d telnum-%02x%02x%02x%02x%02x%02x", 
	pst_cfg->pc_addr, pst_cfg->us_port,
	pst_cfg->auc_telnum[0], pst_cfg->auc_telnum[1], pst_cfg->auc_telnum[2],
	pst_cfg->auc_telnum[3], pst_cfg->auc_telnum[4], pst_cfg->auc_telnum[5]);

	return;
}

void vtcp_getconf(vtcp_cfg_s *pst_val)
{
	*pst_val = gst_vtcp.st_cfg;
	return;
}

int32_t vtcp_conn(void)
{
	vtcp_s *pst_vtcp = &gst_vtcp;
	pst_vtcp->us_seqnum = 2;
	pthread_mutex_init(&pst_vtcp->st_lock, NULL);
	return sock_conn(pst_vtcp->st_cfg.pc_addr, pst_vtcp->st_cfg.us_port);
}

void vtcp_disconn(void)
{
	vtcp_s *pst_vtcp = &gst_vtcp;
	
	sock_disconn();
	pthread_mutex_destroy(&pst_vtcp->st_lock);

	return;
}

int32_t vtcp_isauth(void)
{
	/* Todo: check vtcp is authorised */

	return 0;
}

int32_t vtcp_sendreq(uint16_t us_msgid, void *pv_buf, uint16_t us_len)
{
	int32_t			i_ret = 0;
	vtcpmsg_s 		st_msg;
	vtcpmsg_buf_s 	st_msgbuf;
	
	make_vtcpmsg(us_msgid, pv_buf, us_len, &st_msg);
	vtcpmsg_enc(&st_msg, &st_msgbuf);
	
	i_ret = sock_send(st_msgbuf.auc_buf, st_msgbuf.ui_len);
	if (0 > i_ret) {
		loge("vtcp register msg send failed!");
	}

	/* Todo: if we got a reply then return */

	return i_ret;
}

int32_t vtcp_gotresp(vtcpmsg_s *pst_msg, uint8_t *puc_payload)
{
	vtcpmsg_buf_s 	st_msgbuf;

	/* Todo: got request response */
	st_msgbuf.ui_len = sock_recv(st_msgbuf.auc_buf, sizeof(st_msgbuf.auc_buf), 5);
	if (0 > st_msgbuf.ui_len) {
		loge("got reply failed!");
		return -1;
	}
	vtcpmsg_dec(&st_msgbuf, pst_msg, puc_payload);
	
	return 0;
}

void vtcp_loop(vtcp_cb pf_cb)
{
    fd_set st_rfds;
    struct timeval st_timeval;
	vtcpmsg_buf_s  st_rcvbuf;
	int32_t i_ret 	= 0;
	int32_t i_maxfd = 0;
	int32_t i_fd 	= sock_getfd();
	
	if (i_maxfd <= i_fd) {
		i_maxfd = i_fd;
	}	
	
	/* looprecv */
	for (;;) {
		FD_ZERO(&st_rfds);
		FD_SET(i_fd, &st_rfds);
		st_timeval.tv_sec  = 3;
		st_timeval.tv_usec = 0;
		i_ret = select(i_maxfd + 1, &st_rfds, NULL, NULL, &st_timeval);
		if (i_ret < 0) {
			loge("select failed!");
			break;
		} else if (i_ret == 0) {
			continue;
		} else {
			if (FD_ISSET(i_fd, &st_rfds)) {
				st_rcvbuf.ui_len = sock_recv(st_rcvbuf.auc_buf, sizeof(st_rcvbuf.auc_buf), 0);
				if (0 > st_rcvbuf.ui_len) {
					loge("got reply failed!");
					continue;
				}
				logi("loop");
				vtcpmsg_buf_dump(&st_rcvbuf);	
			}
		}
	}
	return;
}

int32_t vtcp_register(vtcp_reg_msg_s *pst_msg, vtcp_reg_rsp_s *pst_rsp)
{
	int32_t			i_ret = 0;
	uint16_t		us_len = 0;
	uint8_t			auc_payload[VTCP_PAYLOAD_LEN] = {0};
	vtcpmsg_s 		st_msg;
	
	if ((NULL == pst_msg) || (NULL == pst_rsp)) {
		return -1;
	}
	
	/* Todo: encode term register msg payload */
	tmng_regmsg_enc(auc_payload, &us_len, pst_msg);
	
	/* Todo: send register request message */
	vtcp_sendreq(VTCP_MSG_REGISTER, auc_payload, us_len);
	
	/* Todo: wait and got response */
	vtcp_gotresp(&st_msg, auc_payload);
	
	/* Todo: decode term register response */
	tmng_regrsp_dec(st_msg.pauc_payload, st_msg.st_msghdr.un_msgprop.prop.len, pst_rsp);

	return i_ret;
}

#ifdef __cplusplus
}
#endif

