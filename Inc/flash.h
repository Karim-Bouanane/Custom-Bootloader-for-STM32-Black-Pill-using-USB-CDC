#ifndef __FLASH_H
#define __FLASH_H

#define TOTAL_SECTORS               8
#define BOOTLOADER_TOTAL_SECTORS    2
#define APP_TOTAL_SECTORS           TOTAL_SECTORS - BOOTLOADER_TOTAL_SECTORS


uint8_t Flash_Begin(void);
uint8_t Flash_EraseSector(uint32_t sector);
uint8_t Flash_WritePage(uint32_t address, uint32_t *data, uint32_t size);
uint8_t Flash_Read(uint32_t address, uint32_t *data, uint32_t size);
uint8_t Flash_End(void);
uint8_t GetProtectionStatus(void);


#endif /* __FLASH_H */