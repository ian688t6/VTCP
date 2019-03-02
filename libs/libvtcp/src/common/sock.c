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

static int32_t gi_sockfd;

int32_t sock_conn(const char *pc_addr, uint16_t us_port)
{
	int32_t i_ret = 0;
	struct sockaddr_in st_addrinfo;

	gi_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (0 > gi_sockfd) {
		loge("sock conn create failed!");
		return -1;
	}
	
	bzero(&st_addrinfo, 0x0);
	st_addrinfo.sin_family 		= PF_INET;
	st_addrinfo.sin_addr.s_addr = inet_addr(pc_addr);
	st_addrinfo.sin_port 		= htons(us_port);
	
	i_ret = connect(gi_sockfd, (struct sockaddr_in *)&st_addrinfo, sizeof(st_addrinfo));
	if (0 > i_ret) {
		close(gi_sockfd);
		loge("sock connect failed!");
		return -1;
	}
	
	return 0;
}

void sock_disconn(void)
{
	if (0 < gi_sockfd) {
		close(gi_sockfd);
	}
	
	return;
}

int32_t sock_send(const void *pv_buf, uint32_t ui_len)
{
	return send(gi_sockfd, pv_buf, ui_len, 0);
}

int32_t sock_recv(void *pv_buf, uint32_t ui_len, int32_t i_timeout)
{
	return recv(gi_sockfd, pv_buf, ui_len, 0);
}

int32_t sock_getfd(void)
{
	return gi_sockfd;
}

#ifdef __cplusplus
}
#endif

