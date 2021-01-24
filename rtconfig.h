#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 100
#define IDLE_THREAD_STACK_SIZE 256
#define RT_DEBUG

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX

/* Memory Management */

#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart2"
#define RT_VER_NUM 0x40003
#define ARCH_ARM
#define RT_USING_CPU_FFS
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M4

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80

/* Device virtual file system */


/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SERIAL
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_SPI
#define RT_USING_QSPI
#define RT_USING_SFUD
#define RT_SFUD_USING_SFDP
#define RT_SFUD_USING_FLASH_INFO_TABLE
#define RT_SFUD_USING_QSPI
#define RT_SFUD_SPI_MAX_HZ 50000000

/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC

/* Network */

/* Socket abstraction layer */


/* Network interface device */


/* light weight TCP/IP stack */


/* AT commands */


/* VBUS(Virtual Software BUS) */


/* Utilities */


/* RT-Thread online packages */

/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */


/* IoT Cloud */


/* security packages */


/* language packages */


/* multimedia packages */


/* tools packages */


/* system packages */

#define PKG_USING_FAL
#define FAL_DEBUG_CONFIG
#define FAL_DEBUG 1
#define FAL_PART_HAS_TABLE_CFG
#define PKG_USING_FAL_V00500
#define PKG_FAL_VER_NUM 0x00500
#define PKG_USING_FLASHDB
#define FDB_USING_KVDB
#define FDB_USING_FAL_MODE
#define FDB_WRITE_GRAN_8BITS
#define FDB_WRITE_GRAN 8
#define FDB_DEBUG_ENABLE
#define PKG_USING_FLASHDB_V10000
#define PKG_FLASHDB_VER_NUM 0x10000

/* Micrium: Micrium software products porting for RT-Thread */


/* peripheral libraries and drivers */


/* miscellaneous packages */


/* samples: kernel and components samples */


/* games: games run on RT-Thread console */

#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32L4

/* Hardware Drivers Config */

#define SOC_STM32L475VE

/* Onboard Peripheral Drivers */

#define BSP_USING_QSPI_FLASH

/* On-chip Peripheral Drivers */

#define BSP_USING_UART
#define BSP_USING_UART2
#define BSP_USING_ON_CHIP_FLASH
#define BSP_USING_QSPI

/* Board extended module Drivers */

#define PKG_USING_BSDIFF_OTA

#endif
