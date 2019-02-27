/**
 * @file	: main.c
 * @brief	: vcp deamon
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
	uint8_t auc_manufacid[5] = {0x7e, 0x04, 0x03, 0x04, 0x03};
	uint8_t auc_ttype[8] = {0x05, 0x06, 0x05, 0x06, 0x05, 0x06, 0x05, 0x06};
	uint8_t auc_tid[7] = {0x13, 0x12, 0x14, 0x16, 0x15, 0x26, 0x25};

	st_regmsg.us_provid = 0x0001;
	st_regmsg.us_cityid = 0x0002;
	memcpy(st_regmsg.auc_manufacid, auc_manufacid, sizeof(auc_manufacid));
	memcpy(st_regmsg.auc_ttype, auc_ttype, sizeof(auc_ttype));
	memcpy(st_regmsg.auc_tid, auc_tid, sizeof(auc_tid));
	st_regmsg.uc_pltcolor = 0x1e;
	strcpy(st_regmsg.ac_pltnums, "688t6");
	vtcp_register(&st_regmsg, &st_regrsp);
	logi("reg resp code: %02x", st_regrsp.uc_retcode);
	if (VTCP_REGISTER_SUCC == st_regrsp.uc_retcode) {
		/* Todo: save the auth code */
		vtcp_getconf(&st_cfg);
		memset(st_cfg.st_authcode.auc_code, 0x0, sizeof(vtcp_authcode_s));
		st_cfg.st_authcode.ui_len = st_regrsp.ui_authcode_len;
		memcpy(st_cfg.st_authcode.auc_code, st_regrsp.ac_authcode, st_cfg.st_authcode.ui_len);
	}
	return st_regrsp.uc_retcode;
}

int32_t main(int argc, char *pc_argv[])
{
	int32_t i_ret = 0;
	vtcp_cfg_s st_cfg;
	pthread_t tid;

	if (3 != argc) {
		loge("invalid param");
		return -1;
	}
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
	
	}

	for (;;) {
		sleep(1);	
	}
	vtcp_disconn();

	return 0;
}

#ifdef __cplusplus
}
#endif

