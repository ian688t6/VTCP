/**
 * @file	: timer.c
 * @brief	: 定时器
 * @version :
 * @author	: jyin
 * @date	: Jun 19, 2018
 */

#include "os.h"
#include "log.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

static void *timer_task(void *pv_arg)
{
	struct timeval st_timeval;
	timer_attr_s *pst_timer = (timer_attr_s *)pv_arg;

	pst_timer->pf_cb(pst_timer->pv_arg);
	for (;;)
	{
		st_timeval.tv_sec 	= pst_timer->st_timeval.tv_sec;
		st_timeval.tv_usec 	= pst_timer->st_timeval.tv_usec;
		pthread_testcancel();
		select(0,NULL,NULL,NULL,&st_timeval);
		pthread_testcancel();
		if (pst_timer->ui_flag == 0)
			break;
		pst_timer->pf_cb(pst_timer->pv_arg);
	}

	return NULL;
}

int32_t timer_setup(timer_attr_s *pst_timer, uint64_t ul_sec, uint64_t ul_msec)
{
	int32_t i_ret = 0;

	if ((NULL == pst_timer) || (NULL == pst_timer->pf_cb))
	{
		loge("timer_setup failed input param null");
		return -1;
	}

	pst_timer->ui_flag = 1;
	pst_timer->st_timeval.tv_sec = ul_sec;
	pst_timer->st_timeval.tv_usec = (ul_msec % 1000) * 1000;
	i_ret = pthread_create(&pst_timer->i_timer_id, NULL, timer_task, (void *)pst_timer);
	if (0 != i_ret)
	{
		loge("pthread create failed!");
		return -1;
	}

	return 0;
}

void timer_stop(timer_attr_s *pst_timer)
{
	if (NULL == pst_timer) {
		return;
	}
	pst_timer->ui_flag = 0;

	return;
}

void timer_del(timer_attr_s *pst_timer)
{
	void *status;

	pthread_cancel(pst_timer->i_timer_id);
	pthread_join(pst_timer->i_timer_id, &status);

	return;
}
#ifdef __cplusplus
}
#endif
