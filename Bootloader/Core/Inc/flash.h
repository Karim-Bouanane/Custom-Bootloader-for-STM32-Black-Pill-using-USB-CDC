#ifndef __FLASH_H
#define __FLASH_H

/* Includes --------------------------------------------------------------*/



/* Macro definitions --------------------------------------------------------------*/

#define FLASH_TOTAL_SECTORS         8		// Sector 0 - 7
#define BOOTLOADER_TOTAL_SECTORS    2		// Sector 0 and Sector 1
#define APP_START_SECTOR			2		// Sector 2
#define APP_TOTAL_SECTORS           TOTAL_SECTORS - APP_START_SECTOR	// 6 Sectors

//#define SECTOR_0_SIZE				16		// 16 kilobytes
//#define SECTOR_1_SIZE				16		// 16 kilobytes
//#define SECTOR_2_SIZE				16		// 16 kilobytes
//#define SECTOR_3_SIZE				16		// 16 kilobytes
//#define SECTOR_4_SIZE				64		// 64 kilobytes
//#define SECTOR_5_SIZE				128		// 128 kilobytes
//#define SECTOR_6_SIZE				128		// 128 kilobytes
//#define SECTOR_7_SIZE				128 	// 128 kilobytes

#define FLASH_SECTOR_0_ADDRESS		(uint32_t)0x08000000
#define FLASH_SECTOR_1_ADDRESS		(uint32_t)0x08004000
#define FLASH_SECTOR_2_ADDRESS		(uint32_t)0x08008000
#define FLASH_SECTOR_3_ADDRESS		(uint32_t)0x0800C000
#define FLASH_SECTOR_4_ADDRESS		(uint32_t)0x08010000
#define FLASH_SECTOR_5_ADDRESS		(uint32_t)0x08020000
#define FLASH_SECTOR_6_ADDRESS		(uint32_t)0x08040000
#define FLASH_SECTOR_7_ADDRESS		(uint32_t)0x08060000

#define PAGE_SIZE					(uint16_t)0x400	 	// 1 kilobytes
#define FLASH_SIZE					(uint32_t)0X80000	// 512 kilobytes
#define RAM_SIZE					(uint32_t)0x2000	// 128 kilobytes

#define FLASH_BASE_ADDRESS			FLASH_SECTOR_0_ADDRESS

#define APP_START_ADDRESS 			(uint32_t)0x08008000
#define APP_END_ADDRESS 			(uint32_t)0x08080000

#define RAM_BASE_ADDRESS			(uint32_t)0x20000000
#define RAM_END_ADDRESS				(uint32_t)0x20020000


/* Enumerations --------------------------------------------------------------*/

/**
 * @brief  Error codes for flash operations.
 */
typedef enum
{
    FLASH_OK = 0,                   /*!< No error */
    FLASH_NO_APP,                   /*!< No application found in flash */
    FLASH_UNL_ERROR,                /*!< Flash unlock failed */
    FLASH_ERASE_ERROR,              /*!< Flash erase failed */
    FLASH_WRITE_ERROR,              /*!< Flash write failed */
    FLASH_READ_OVER_ERROR,          /*!< Flash read exceeds address range */
    FLASH_WRITE_OVER_ERROR,         /*!< Flash write exceeds address range */
    FLASH_WRITE_CORR_ERROR,         /*!< Flash write incorrect */

} e_Flash_Errors;


/* Functions -----------------------------------------------------------------*/

uint8_t Flash_Init(void);
uint8_t Flash_EraseSector(uint8_t sector);
uint8_t Flash_Read_Word(uint32_t address, uint32_t *data, uint32_t size);
uint8_t Flash_Write_Word(uint32_t address, uint32_t *data, uint32_t size);
uint32_t Flash_GetChecksum(uint32_t start_address, uint32_t size);


#endif /* __FLASH_H */
