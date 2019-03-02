/**
 * @file	: main.c
 * @brief	: vcp deamon
 * @version :
 * @author	: jyin
 * @date	: Jun 19, 2018
 */

#include "os.h"
#include "log.h"
#include "timer.h"
#include "vtcp.h"

#ifdef __cplusplus
extern "C" {
#endif

static timer_attr_s gst_timer;

static void *loop(void *pv_arg)
{
	vtcp_loop(NULL);
	return NULL;
}

static int32_t device_register(void)
{
	vtcp_cfg_s st_cfg;
	vtcp_reg_msg_s st_regmsg;
	vtcp_reg_rsp_s st_regrsp;
	uint8_t auc_manufacid[5] = {'R', 'F', 'D', 'E', 'V'};
	uint8_t auc_ttype[8] = {'D', 'V', 'R', ' ', ' ', ' ', ' ', ' '};
	uint8_t auc_tid[7] = {'G', 'A', '6', '8', '8', 'T', '6'};

	st_regmsg.us_provid = 0x0036;
	st_regmsg.us_cityid = 0x0100;
	memcpy(st_regmsg.auc_manufacid, auc_manufacid, sizeof(auc_manufacid));
	memcpy(st_regmsg.auc_ttype, auc_ttype, sizeof(auc_ttype));
	memcpy(st_regmsg.auc_tid, auc_tid, sizeof(auc_tid));
	st_regmsg.uc_pltcolor = 0x1e;
	strcpy(st_regmsg.ac_pltnums, "688t6");
	vtcp_register(&st_regmsg, &st_regrsp);
	logi("reg resp code: %02x len: %d %s", st_regrsp.uc_retcode, st_regrsp.ui_authcode_len, st_regrsp.ac_authcode);
	if (VTCP_REGISTER_SUCC == st_regrsp.uc_retcode) {
		/* Todo: save the auth code */
		vtcp_getconf(&st_cfg);
		memset(st_cfg.st_authcode.auc_code, 0x0, sizeof(vtcp_authcode_s));
		st_cfg.st_authcode.ui_len = st_regrsp.ui_authcode_len;
		memcpy(st_cfg.st_authcode.auc_code, st_regrsp.ac_authcode, st_cfg.st_authcode.ui_len);
		vtcp_setconf(&st_cfg);
	}
	return st_regrsp.uc_retcode;
}

int32_t device_unregister(void)
{
	vtcprsp_s st_resp;
	
	vtcp_unregister(&st_resp);
	logi("unreg resp: seqnum-%04x msgid-%04x retcode-%02x", 
		st_resp.us_seqnum, st_resp.us_msgid, st_resp.uc_retcode);

	return 0;
}

int32_t device_auth(void)
{
	vtcp_cfg_s st_cfg;
	vtcp_auth_msg_s st_authmsg;
	vtcprsp_s st_resp;

	vtcp_getconf(&st_cfg);
	st_authmsg.us_len = st_cfg.st_authcode.ui_len;
	memcpy(st_authmsg.ac_authcode, st_cfg.st_authcode.auc_code, st_authmsg.us_len);
	vtcp_authorise(&st_authmsg, &st_resp);
	logi("auth resp: seqnum-%04x msgid-%04x retcode-%02x", 
		st_resp.us_seqnum, st_resp.us_msgid, st_resp.uc_retcode);
	
	return st_resp.uc_retcode;
}

void vtcp_timer(void *pv_arg)
{
	vtcprsp_s st_resp;

	vtcp_hb(&st_resp);
	logi("hb resp: seqnum-%04x msgid-%04x retcode-%02x", 
		st_resp.us_seqnum, st_resp.us_msgid, st_resp.uc_retcode);
}

void device_hb(void)
{
	/* Todo: setup timer */
	gst_timer.pf_cb 	= vtcp_timer;
	gst_timer.pv_arg 	= NULL;
	timer_setup(&gst_timer, 5, 0);
	return;
}

int32_t main(int argc, char *pc_argv[])
{
	char ch = 0;
	int32_t i_ret = 0;
	vtcp_cfg_s st_cfg;
	pthread_t tid;

	if (3 != argc) {
		loge("invalid param");
		return -1;
	}
//	log_set_level(1);
	st_cfg.pc_addr = (const char *)pc_argv[1];
	st_cfg.us_port = strtoul(pc_argv[2], NULL, 0); 	
	st_cfg.auc_telnum[0] = bin2bcd(0x01);
	st_cfg.auc_telnum[1] = bin2bcd(0x99);
	st_cfg.auc_telnum[2] = bin2bcd(0x17);
	st_cfg.auc_telnum[3] = bin2bcd(0x90);
	st_cfg.auc_telnum[4] = bin2bcd(0x00);
	st_cfg.auc_telnum[5] = bin2bcd(0x25);
	vtcp_setconf(&st_cfg);	
	i_ret = vtcp_conn();
	if (0 != i_ret) {
		loge("vtcp conn failed!");
		return -1;
	}
	
	i_ret = pthread_create(&tid, 0, loop, NULL);
	if (0 != i_ret) {
		vtcp_disconn();
		return -1;
	}
	
	i_ret = device_register();
	if (VTCP_REGISTER_SUCC == i_ret) {
		device_auth();	
		device_hb();
	}

	for (;;) {
		ch = getchar();
		if ('q' == ch) {
			timer_del(&gst_timer);
			break;
		}
		
	}
	device_unregister();
	vtcp_disconn();

	return 0;
}

#ifdef __cplusplus
}
#endif

