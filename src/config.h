#ifndef CONFIG_H_
#define CONFIG_H_

#define KERNEL_CORE_END_PAGE 400 // about 100kB
#define MCU_SAM3X8E // target sam3x8e

#define KERNEL_DYNAMIC_MEMORY 0x1512F
#define KERNEL_DYNAMIC_MEMORY_START 0x20071B00

#define CONTEXT_SWITCH_INTERVAL 10 // 10 ms

#define WORD 4
#define HWORD 2
#define LWORD 8



#endif /*CONFIG_H_*/
