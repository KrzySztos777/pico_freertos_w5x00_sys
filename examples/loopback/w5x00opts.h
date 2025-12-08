#ifndef W5X00OPTS_H
#define W5X00OPTS_H

// //performance options
// #define W5X00_PRINTF                    printf//for debugging- may be set to NULL if you don't want to see it
// #define W5X00_INIT_DELAY_MS             1500//delay in ms at the beginning of the task. Just.
// #define W5X00_INTERRUPT                 1//if enabled then SPI is not polled but it waits for GPIO interrupt
#define W5X00_USE_SPI_DMA                  1//0//set DMA if SPI is used
// #define W5X00_CHECK_LINK_TIMEOUT_MS     100//if no traffic for this time then check link status. if 0 then link always up
#define W5X00_SPI_SPEED                 (60000 * 1000)//spi speed

// //task options
// #define W5X00_POLL_SLEEP()              taskYIELD() //function during SPI polling. taskYIELD fastest. May be also vTaskDelay(1). IMPORTANT: with very high priority of this task vTaskDelay(1) is recomennded to avoid starving of another ones
// #define W5X00_DRAIN_SLEEP()             NULL//function during draining packets from W5x00. NULL is fastest. May be also taskYIELD or vTaskDelay(1)
// #define W5X00_DONT_SET_IRQ_CB           0//if you USES IRQ right now- not only for W5x00 then you have to call W5X00_INT_CB(gpio,events) from your IRQ handler if you want use IRQ also for W5X00. 
// #define W5X00_THREAD_NAME               "w5x00_thread"//name of W5x00 thread
// #define W5X00_THREAD_STACKSIZE          256//stacksize for w5x00 thread
// #define W5X00_THREAD_PRIO               (configMAX_PRIORITIES-2)//priority for w5x00 thread
// #define W5X00_SLEEP_MS(ms)              vTaskDelay(pdMS_TO_TICKS(ms))

#endif