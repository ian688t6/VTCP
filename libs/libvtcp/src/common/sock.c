/**
 * @file	: sock.c
 * @brief	: tcp socket operate
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

static sock_vtcp_s gst_sock;

int32_t sock_conn(const char *pc_addr, uint16_t us_port)
{
	int32_t i_ret = 0;
	sock_vtcp_s *pst_sock = &gst_sock;
	struct sockaddr_in st_addrinfo;

	memset(pst_sock, 0x0, sizeof(sock_vtcp_s));
	pst_sock->i_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (0 > pst_sock->i_fd) {
		loge("sock conn create failed!");
		return -1;
	}
	
	bzero(&st_addrinfo, 0x0);
	st_addrinfo.sin_family 		= PF_INET;
	st_addrinfo.sin_addr.s_addr = inet_addr(pc_addr);
	st_addrinfo.sin_port 		= htons(us_port);
	
	i_ret = connect(pst_sock->i_fd, (struct sockaddr_in *)&st_addrinfo, sizeof(st_addrinfo));
	if (0 > i_ret) {
		close(pst_sock->i_fd);
		loge("sock connect failed!");
		return -1;
	}
	pthread_mutex_init(&pst_sock->st_lock, NULL);
	
	return 0;
}

void sock_disconn(void)
{
	sock_vtcp_s *pst_sock = &gst_sock;
	
	if (0 < pst_sock->i_fd) {
		close(pst_sock->i_fd);
	}
	pthread_mutex_destroy(&pst_sock->st_lock);
	
	return;
}

int32_t sock_send(const void *pv_buf, uint32_t ui_len)
{
	sock_vtcp_s *pst_sock = &gst_sock;
	return send(pst_sock->i_fd, pv_buf, ui_len, 0);
}

int32_t sock_recv(void *pv_buf, uint32_t ui_len, int32_t i_timeout)
{
	int32_t i_ret = 0; 
	sock_vtcp_s *pst_sock = &gst_sock;
	
	i_ret = recv(pst_sock->i_fd, pv_buf, ui_len, 0);
	
	return i_ret;
}

int32_t sock_getfd(void)
{
	sock_vtcp_s *pst_sock = &gst_sock;
	return pst_sock->i_fd;
}

#ifdef __cplusplus
}
#endif

