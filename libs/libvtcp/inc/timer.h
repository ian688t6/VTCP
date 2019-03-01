/**
 * @file	: timer.h
 * @brief	: 定时器
 * @version :
 * @author	: jyin
 * @date	: Jun 19, 2018
 */

#ifndef __TIMER_H__
#define __TIMER_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*timer_callback)(void *pv_arg);

typedef struct
{
	pthread_t 		i_timer_id;
	uint32_t		ui_flag;
	struct timeval 	st_timeval;
	timer_callback	pf_cb;
	void 			*pv_arg;
} timer_attr_s;

extern int32_t timer_setup(timer_attr_s *pst_timer, uint64_t ul_sec, uint64_t ul_usec);

extern void timer_stop(timer_attr_s *pst_timer);

extern void timer_del(timer_attr_s *pst_timer);

#ifdef __cplusplus
}
#endif
#endif /* TIMER_H_ */
