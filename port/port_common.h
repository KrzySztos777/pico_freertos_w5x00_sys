/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PORT_COMMON_H_
#define _PORT_COMMON_H_


/* Porting defines from bare-metal to FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

#define W5X00_SLEEP_MS(ms)      vTaskDelay(pdMS_TO_TICKS(ms))
#define W5X00_PRINTF            printf//NULL
/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
/* Common */
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/critical_section.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"

#endif /* _PORT_COMMON_H_ */
