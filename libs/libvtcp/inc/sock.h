/**
 * @file	: sock.h
 * @brief	: tcp socket operate
 * @version :
 * @author	: jyin
 * @date	: Jun 19, 2018
 */

#ifndef __SOCK_H__
#define __SOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sock_vtcp {
	int32_t				i_fd;
	pthread_mutex_t		st_lock;
} sock_vtcp_s;

extern int32_t sock_conn(const char *pc_addr, uint16_t us_port);

extern void sock_disconn(void);

extern int32_t sock_send(const void *pv_buf, uint32_t ui_len);

extern int32_t sock_recv(void *pv_buf, uint32_t ui_len, int32_t i_timeout);

extern int32_t sock_getfd(void);

#ifdef __cplusplus
}
#endif
#endif

