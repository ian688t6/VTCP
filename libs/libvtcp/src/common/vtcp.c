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

typedef struct {
	vtcp_cfg_s			st_cfg;
	pthread_t			tid;
	vtcp_cb				pf_cb;
	uint16_t			us_seqnum;
	pthread_mutex_t		st_lock;
	struct list_head 	vrb_list;
} vtcp_s;

typedef struct {
	struct list_head 	list;
	uint16_t			us_seqnum;
	uint16_t			us_msgid;
	uint32_t			ui_comp;
	uint8_t				auc_payload[VTCP_PAYLOAD_LEN];
	uint16_t			us_len;
} vrb_s;

static vtcp_s gst_vtcp;
uint32_t gui_log_level = 0;

static vrb_s *new_vrb(uint16_t us_msgid, uint8_t *puc_payload, uint16_t us_len)
{
	vrb_s *pst_vrb = NULL;	
	
	pst_vrb = (vrb_s *)malloc(sizeof(vrb_s));
	if (NULL == pst_vrb) {
		loge("vrb alloc failed!");
		return NULL;	
	}
	memset(pst_vrb, 0x0, sizeof(vrb_s));
	memcpy(pst_vrb->auc_payload, puc_payload, us_len);
	pst_vrb->us_len		= us_len;
	pst_vrb->us_msgid	= us_msgid;
	
	return pst_vrb;
}

static void del_vrb(vrb_s *pst_vrb)
{
	vtcp_s *pst_vtcp = &gst_vtcp; 
	
	pthread_mutex_lock(&pst_vtcp->st_lock);	
	list_del(&pst_vrb->list);
	free(pst_vrb);
	pthread_mutex_unlock(&pst_vtcp->st_lock);
	return;
}

static int32_t wait_complete(int32_t i_flag)
{
	static int32_t i_retries = 0;
	struct timespec st_waiter;

	if (1 == i_flag) {
		i_retries = 0;
		return 0;
	}

	st_waiter.tv_sec = 0;
	st_waiter.tv_nsec = 10000;
	nanosleep(&st_waiter, NULL);
	if (i_retries ++ < 50000)
		return 1;
	i_retries = 0;
	return 0;
}

static vrb_s *search_forvrb(uint16_t us_msgid, uint16_t us_seqnum)
{
	vrb_s 	*n;
	vrb_s 	*node;
	vtcp_s 	*pst_vtcp = &gst_vtcp; 
	
	pthread_mutex_lock(&pst_vtcp->st_lock);
	list_for_each_entry_safe(node, n, &pst_vtcp->vrb_list, list) {
		if ((node->us_seqnum == us_seqnum) && (node->us_msgid == us_msgid)) {
			pthread_mutex_unlock(&pst_vtcp->st_lock);
			return node;
		}	
	}
	pthread_mutex_unlock(&pst_vtcp->st_lock);

	return NULL;
}

static int32_t sendrsp(uint16_t us_msgid, uint8_t *puc_payload, uint16_t us_len)
{
	int32_t			i_ret = 0;
	vtcpmsg_s 		st_msg;
	vtcpmsg_buf_s 	st_msgbuf;
	vtcp_s 			*pst_vtcp = &gst_vtcp;

	st_msg.uc_id0 							= VTCP_ID_CODE;
	st_msg.pauc_payload 					= puc_payload;
	st_msg.st_msghdr.us_msgid 				= us_msgid;
	st_msg.st_msghdr.un_msgprop.prop.len 	= us_len;
	st_msg.st_msghdr.un_msgprop.prop.crypt 	= 0;
	st_msg.st_msghdr.un_msgprop.prop.split 	= 0;
	st_msg.st_msghdr.us_seqnum 				= pst_vtcp->us_seqnum;
	memcpy(st_msg.st_msghdr.auc_bcd, pst_vtcp->st_cfg.auc_telnum, sizeof(st_msg.st_msghdr.auc_bcd));
	vtcpmsg_enc(&st_msg, &st_msgbuf);
	i_ret = sock_send(st_msgbuf.auc_buf, st_msgbuf.ui_len);
	if (0 > i_ret) {
		loge("vtcp register msg send failed!");
	}
	return i_ret;
}

static int32_t sendreq(vrb_s *pst_vrb)
{
	int32_t			i_ret = 0;
	vtcpmsg_s 		st_msg;
	vtcpmsg_buf_s 	st_msgbuf;
	vtcp_s 			*pst_vtcp = &gst_vtcp;
	
	if (NULL == pst_vrb) {
		return -1;
	}
	
	st_msg.uc_id0 							= VTCP_ID_CODE;
	st_msg.pauc_payload 					= pst_vrb->auc_payload;
	st_msg.st_msghdr.us_msgid 				= pst_vrb->us_msgid;
	st_msg.st_msghdr.un_msgprop.prop.len 	= pst_vrb->us_len;
	st_msg.st_msghdr.un_msgprop.prop.crypt 	= 0;
	st_msg.st_msghdr.un_msgprop.prop.split 	= 0;
	st_msg.st_msghdr.us_seqnum 				= pst_vtcp->us_seqnum;
	memcpy(st_msg.st_msghdr.auc_bcd, pst_vtcp->st_cfg.auc_telnum, sizeof(st_msg.st_msghdr.auc_bcd));
	st_msg.uc_crc 							= vtcpmsg_calc_crc(&st_msg.st_msghdr, st_msg.pauc_payload);
	st_msg.uc_id1 							= VTCP_ID_CODE;
	pst_vtcp->us_seqnum ++;
	
	vtcpmsg_enc(&st_msg, &st_msgbuf);
	pst_vrb->us_seqnum 	= st_msg.st_msghdr.us_seqnum;
	pthread_mutex_lock(&pst_vtcp->st_lock);	
	list_add(&pst_vrb->list, &pst_vtcp->vrb_list);
	pthread_mutex_unlock(&pst_vtcp->st_lock);	
	logd("[SENDREQ]");
	vtcpmsg_buf_dump(&st_msgbuf);
	logd("------------------------------\r\n");
	i_ret = sock_send(st_msgbuf.auc_buf, st_msgbuf.ui_len);
	if (0 > i_ret) {
		loge("vtcp register msg send failed!");
	}

	return i_ret;
}

static int32_t gotresp(vtcpmsg_s *pst_msg, uint8_t *puc_payload)
{
	vtcpmsg_buf_s 	st_msgbuf;

	/* Todo: got request response */
	st_msgbuf.ui_len = sock_recv(st_msgbuf.auc_buf, sizeof(st_msgbuf.auc_buf), 0);
	if (0 >= st_msgbuf.ui_len) {
		loge("got reply failed!");
		return -1;
	}

	logd("[GOTRESP] len=%d", st_msgbuf.ui_len);
	vtcpmsg_buf_dump(&st_msgbuf);
	logd("------------------------------\r\n");
	return vtcpmsg_dec(&st_msgbuf, pst_msg, puc_payload);
}

static void do_common_resp(vtcpmsg_s *pst_msg)
{
	vrb_s 	*pst_vrb = NULL;
	vtcprsp_s st_resp;
	
	vtcprsp_dec(pst_msg->pauc_payload, pst_msg->st_msghdr.un_msgprop.prop.len, &st_resp);
	pst_vrb = search_forvrb(st_resp.us_msgid, st_resp.us_seqnum);
	if (NULL == pst_vrb) {
		return;
	}
	memcpy(pst_vrb->auc_payload, &st_resp, sizeof(vtcprsp_s));	
	pst_vrb->us_len = sizeof(vtcprsp_s);
	pst_vrb->ui_comp = 1;

	return;
}

static void do_register_resp(vtcpmsg_s *pst_msg)
{
	vrb_s 	*pst_vrb = NULL;
	vtcp_reg_rsp_s st_resp;

	tmng_regrsp_dec(pst_msg->pauc_payload, pst_msg->st_msghdr.un_msgprop.prop.len, &st_resp);
	pst_vrb = search_forvrb(VTCP_MSG_REGISTER, st_resp.us_seqnum);
	if (NULL == pst_vrb) {
		return;
	}
	
	memcpy(pst_vrb->auc_payload, &st_resp, sizeof(vtcp_reg_rsp_s));	
	pst_vrb->us_len = sizeof(vtcp_reg_rsp_s);
	pst_vrb->ui_comp = 1;

	return;
}

static int32_t dispatch(void)
{
	int32_t 	i_ret = 0;
	vtcpmsg_s 	st_msg;
	uint8_t		auc_payload[VTCP_PAYLOAD_LEN] = {0};
	vtcp_s 		*pst_vtcp = &gst_vtcp;
	
	i_ret = gotresp(&st_msg, auc_payload);
	if (0 != i_ret) {
		loge("vtcp got resp failed!");
		return -1;
	}
	switch (st_msg.st_msghdr.us_msgid) {
	case PLATFORM_MSG(VTCP_MSG_RESP):
		do_common_resp(&st_msg);	
	break;

	case PLATFORM_MSG(VTCP_MSG_REGISTER):
		do_register_resp(&st_msg);
	break;
	default:
		if (pst_vtcp->pf_cb)
			pst_vtcp->pf_cb(st_msg.st_msghdr.us_seqnum, st_msg.st_msghdr.us_msgid, 
							st_msg.pauc_payload, 
							st_msg.st_msghdr.un_msgprop.prop.len);
	break;
	}

	return i_ret;
}

void vtcp_setconf(vtcp_cfg_s *pst_val)
{
	vtcp_cfg_s *pst_cfg = &gst_vtcp.st_cfg;
	*pst_cfg = *pst_val;
	
	if (NULL == pst_cfg->pc_authfile) {
		pst_cfg->pc_authfile = VTCP_DEFAULT_AUTHFILE;
	}

	logd("vtcp server-%s:%d telnum-%02x%02x%02x%02x%02x%02x", 
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
	pst_vtcp->us_seqnum = 0;
	INIT_LIST_HEAD(&pst_vtcp->vrb_list);
	pthread_mutex_init(&pst_vtcp->st_lock, NULL);
	return sock_conn(pst_vtcp->st_cfg.pc_addr, pst_vtcp->st_cfg.us_port);
}

void vtcp_disconn(void)
{
	vrb_s *node, *node_tmp;
	vtcp_s *pst_vtcp = &gst_vtcp;
	
	sock_disconn();
	pthread_mutex_destroy(&pst_vtcp->st_lock);
	list_for_each_entry_safe(node, node_tmp, &pst_vtcp->vrb_list, list) {
		list_del(&node->list);
		free(node);
	}

	return;
}

int32_t vtcp_authsave(vtcp_authcode_s *pst_auth)
{
	int32_t i_fd 	= 0;
	int32_t i_len 	= 0;
	vtcp_cfg_s *pst_cfg = &gst_vtcp.st_cfg;

	if (NULL == pst_auth) {
		return -1;
	}

	i_fd = open(pst_cfg->pc_authfile, (O_CREAT|O_RDWR), 0655);
	if (0 > i_fd) {
		loge("vtcp save auth failed!");
		return -1;
	}
	i_len = write(i_fd, pst_auth->auc_code, pst_auth->ui_len);
	close(i_fd);

	return i_len != pst_auth->ui_len ? -1 : 0;
}

int32_t vtcp_authload(vtcp_authcode_s *pst_auth)
{
	int32_t i_fd 	= 0;
	int32_t i_len 	= 0;
	vtcp_cfg_s *pst_cfg = &gst_vtcp.st_cfg;

	if (NULL == pst_auth) {
		return -1;
	}
	
	i_fd = open(pst_cfg->pc_authfile, O_RDONLY);
	if (0 > i_fd) {
		loge("vtcp load auth failed!");
		return -1;
	}
	pst_auth->ui_len = lseek(i_fd, 0, SEEK_END);
	lseek(i_fd, 0, SEEK_SET);
	i_len = read(i_fd, pst_auth->auc_code, pst_auth->ui_len);
	close(i_fd);

	return i_len != pst_auth->ui_len ? -1 : 0;
}

void vtcp_authrm(void)
{
	vtcp_cfg_s *pst_cfg = &gst_vtcp.st_cfg;
	remove(pst_cfg->pc_authfile);
	return;
}

int32_t vtcp_isauth(void)
{
	/* Todo: check vtcp is authorised */
	vtcp_cfg_s *pst_cfg = &gst_vtcp.st_cfg;
	return access(pst_cfg->pc_authfile, F_OK);
}

void vtcp_loop(vtcp_cb pf_cb)
{
    fd_set st_rfds;
 	struct timeval st_timeval;
	int32_t i_ret 	= 0;
	int32_t i_maxfd = 0;
	vtcp_s 	*pst_vtcp = &gst_vtcp;
	
	pst_vtcp->pf_cb = pf_cb;
	if (i_maxfd <= sock_getfd()) {
		i_maxfd = sock_getfd();
	}	
	
	/* looprecv */
	for (;;) {		
		FD_ZERO(&st_rfds);
		FD_SET(sock_getfd(), &st_rfds);
		st_timeval.tv_sec  = 3;
		st_timeval.tv_usec = 0;
		i_ret = select(i_maxfd + 1, &st_rfds, NULL, NULL, &st_timeval);
		if (i_ret < 0) {
			loge("select failed!");
			break;
		} else if (i_ret == 0) {
			continue;
		} else {
			if (FD_ISSET(sock_getfd(), &st_rfds)) {
				i_ret = dispatch();
				if (0 != i_ret) {
					loge("socket connect wrong!");
					break;
				}
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
	vrb_s			*pst_vrb = NULL;	

	if ((NULL == pst_msg) || (NULL == pst_rsp)) {
		return -1;
	}
	
	/* Todo: encode term register msg payload */
	tmng_regmsg_enc(auc_payload, &us_len, pst_msg);
	
	/* Todo: create vrb */
	pst_vrb = new_vrb(VTCP_MSG_REGISTER, auc_payload, us_len);	
	if (NULL == pst_vrb) {
		loge("vrb alloc failed!");
		return -1;	
	}

	/* Todo: send register request message */
	sendreq(pst_vrb);
	
	/* Todo: wait and got response */
	while (wait_complete(pst_vrb->ui_comp));
	i_ret = pst_vrb->ui_comp == 1 ? 0 : -1;
	if (!i_ret) memcpy(pst_rsp, pst_vrb->auc_payload, pst_vrb->us_len);

	/* Todo: decode term register response */
	del_vrb(pst_vrb);

	return i_ret;
}

int32_t vtcp_authorise(vtcp_auth_msg_s *pst_msg, vtcprsp_s *pst_rsp)
{
	int32_t			i_ret = 0;
	uint16_t		us_len = 0;
	uint8_t			auc_payload[VTCP_PAYLOAD_LEN] = {0};
	vrb_s			*pst_vrb = NULL;	
	
	if ((NULL == pst_msg) || (NULL == pst_rsp)) {
		return -1;
	}
	
	/* Todo: encode term auth msg payload */
	tmng_authmsg_enc(auc_payload, &us_len, pst_msg);
	
	/* Todo: create vrb */
	pst_vrb = new_vrb(VTCP_MSG_AUTHORISE, auc_payload, us_len);	
	if (NULL == pst_vrb) {
		loge("vrb alloc failed!");
		return -1;	
	}

	/* Todo: send register request message */
	sendreq(pst_vrb);

	/* Todo: wait and got response */
	while (wait_complete(pst_vrb->ui_comp));
	i_ret = pst_vrb->ui_comp == 1 ? 0 : -1;
	if (!i_ret) memcpy(pst_rsp, pst_vrb->auc_payload, pst_vrb->us_len);
	
	/* Todo: del vrb */
	del_vrb(pst_vrb);

	return i_ret;
}

int32_t vtcp_hb(vtcprsp_s *pst_rsp)
{
	vrb_s	*pst_vrb = NULL;	
	int32_t  i_ret	 = 0;
	
	/* Todo: create vrb */
	pst_vrb = new_vrb(VTCP_MSG_HB, NULL, 0);	
	if (NULL == pst_vrb) {
		loge("vrb alloc failed!");
		return -1;
	}

	/* Todo: send heart beat request message */
	sendreq(pst_vrb);
	
	/* Todo: wait and got response */
	while (wait_complete(pst_vrb->ui_comp));
	i_ret = pst_vrb->ui_comp == 1 ? 0 : -1;
	if (!i_ret) memcpy(pst_rsp, pst_vrb->auc_payload, pst_vrb->us_len);

	/* Todo: del vrb */
	del_vrb(pst_vrb);

	return i_ret;
}

int32_t vtcp_unregister(vtcprsp_s *pst_rsp)
{
	vrb_s	*pst_vrb = NULL;	
	int32_t  i_ret	 = 0;
	
	/* Todo: create vrb */
	pst_vrb = new_vrb(VTCP_MSG_UNREGISTER, NULL, 0);	
	if (NULL == pst_vrb) {
		loge("vrb alloc failed!");
		return -1;	
	}

	/* Todo: send unregister request message */
	sendreq(pst_vrb);
	
	/* Todo: wait and got response */
	while (wait_complete(pst_vrb->ui_comp));
	i_ret = pst_vrb->ui_comp == 1 ? 0 : -1;
	if (!i_ret) memcpy(pst_rsp, pst_vrb->auc_payload, pst_vrb->us_len);
		
	/* Todo: del vrb */
	del_vrb(pst_vrb);

	return i_ret;
}

int32_t vtcp_commresp(vtcprsp_s *pst_rsp)
{
	int32_t 	i_ret = 0;
	uint16_t	us_len = 0;
	uint8_t		auc_payload[VTCP_PAYLOAD_LEN] = {0};

	/* Todo: encode term comm resp payload */
	tmng_commrsp_enc(auc_payload, &us_len, pst_rsp);

	/* Todo: send resp */
	sendrsp(VTCP_MSG_RESP, auc_payload, us_len);

	return i_ret;
}

void vtcp_setlog(uint32_t ui_level)
{
	gui_log_level = ui_level;
	return;
}

#ifdef __cplusplus
}
#endif

