/*
 * log.h
 *
 *  Created on: Jun 12, 2018
 *      Author: jyin
 */

#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
extern uint32_t gui_log_level;
#define logd(format, arg...) \
	do { \
			if (gui_log_level > 0) \
				printf("DEBUG: " format "\n", ##arg); \
	} while (0)

#define logi(format, arg...) printf("INFOM: " format "\n", ##arg)

#define logw(format, arg...) printf("WARN: " format "\n", ##arg)

#define loge(format, arg...) printf("ERROR: " format "\n", ##arg)

static inline void log_set_level(uint32_t ui_level)
{
	gui_log_level = ui_level;
}

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DBG_H_ */
