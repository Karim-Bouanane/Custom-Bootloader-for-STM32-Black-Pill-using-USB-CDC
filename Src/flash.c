
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flash.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"

extern CRC_HandleTypeDef hcrc;


/**
 * @brief	This function unlocks the flash memory for writing.
 * @param	None
 * @return	Flash error code ::eFlashErrorCodes
 *			- FLASH_UNL_ERROR: Flash unlocking failed.
 *			- FLASH_OK: Flash unlocking successful.
 */
uint8_t Flash_Begin(void)
{
	// Attempt to unlock the flash
    if (HAL_FLASH_Unlock() == HAL_ERROR)
    {
    	return FLASH_UNL_ERROR;
    }

    // Clear Flash flags
    __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
    		FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_RDERR | FLASH_FLAG_BSY);

    return FLASH_OK;
}

/**
 * @brief	This function erases a specified flash sector.
 * @param 	sector: The sector number to be erased.
 * @return	Flash error code ::eFlashErrorCodes
 *			- FLASH_ERASE_ERROR: The erase operation failed.
 *			- FLASH_OK: The erase operation was successful.
 */
uint8_t Flash_EraseSector(uint8_t sector)
{
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t SectorError;

    // Configure the erase operation
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Sector = sector;
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    // Perform the flash erase operation
    if (HAL_FLASHEx_Erase(&eraseInit, &SectorError) == HAL_ERROR)
    {
    	return FLASH_ERASE_ERROR;
    }

    return FLASH_OK;
}


/**
 * @brief	This function erases the application area in flash memory.
 * @return	Flash error code ::eFlashErrorCodes
 *			- FLASH_ERASE_ERROR: The erase operation failed.
 *			- FLASH_OK: The erase operation was successful.
 */
uint8_t Flash_EraseApplication(void)
{
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t SectorError;

    // Configure the erase operation
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Sector = APP_START_SECTOR;
    eraseInit.NbSectors = APP_TOTAL_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    // Perform the flash erase operation
    if (HAL_FLASHEx_Erase(&eraseInit, &SectorError) == HAL_ERROR)
    {
        return FLASH_ERASE_ERROR;
    }

    return FLASH_OK;
}

/**
 * @brief	This function writes data to the specified address in flash memory.
 * @param	address: The address in flash memory where the data will be written.
 * @param	data: Pointer to the data array to be written.
 * @param	size: The size of the data array in words (each word is 4 bytes).
 * @return	Flash error code ::eFlashErrorCodes
 *         - FLASH_OK: The write operation was successful.
 *         - FLASH_WRITE_OVER_ERROR: The write operation exceeds the flash memory boundary.
 *         - FLASH_WRITE_CORR_ERROR: The written data is incorrect.
 *         - FLASH_WRITE_ERROR: The write operation failed.
 */
uint8_t Flash_Write(uint32_t address, uint32_t *data, uint32_t size)
{
    // Check if the write operation exceeds the flash memory boundary
    if ((address < APP_START_ADDRESS) ||
    	((address + (size * 4)) > (FLASH_BASE_ADDRESS + FLASH_SIZE)))
    {
        HAL_FLASH_Lock();
        return FLASH_WRITE_OVER_ERROR;
    }

    // Perform the write operation
    for (uint32_t i = 0; i < size; i += 1)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + (i * 4), data[i]) == HAL_OK)
        {
            // Verify the written data
            if (*(uint32_t*)(address + (i * 4)) != data[i])
            {
                HAL_FLASH_Lock();
                return FLASH_WRITE_CORR_ERROR;
            }
        }
        else
        {
            HAL_FLASH_Lock();
            return FLASH_WRITE_ERROR;
        }
    }

    return FLASH_OK;
}

/**
 * @brief	This function reads data from the flash memory.
 * @param	address: The start address of the flash memory to read from.
 * @param	data: Pointer to the buffer where the read data will be stored.
 * @param	size: The size of data to read (in words, each word is 4 bytes).
 * @return	Flasg error code ::eFlashErrorCodes
 * 			- FLASH_OK: The flash read operation was successful.
 * 			- FLASH_READ_OVER_ERROR: The read operation exceeded the flash memory boundaries.
 */
uint8_t Flash_Read(uint32_t address, uint32_t *data, uint32_t size)
{
    if ((address < FLASH_BASE_ADDRESS) ||
        ((address + (size * 4)) > (FLASH_BASE_ADDRESS + FLASH_SIZE)))
    {
        HAL_FLASH_Lock();
        return FLASH_READ_OVER_ERROR;
    }

    for (uint32_t i = 0; i < size; i += 1)
    {
        data[i] = *(uint32_t *)(address + (i * 4));
    }

    return FLASH_OK;
}


/**
 * @brief 	This function ends the flash memory operation by locking the flash.
 * @param	None
 * @return	Flash error code ::eFlashErrorCodes
 * 			- FLASH_OK: The flash was successfully locked.
 */
uint8_t Flash_End(void)
{
    HAL_FLASH_Lock();

    return FLASH_OK;
}


/**
 * @brief	This function calculates the checksum of the user application in flash memory.
 * @param	checksum_value: Pointer to the variable where the calculated checksum will be stored.
 * @return	Flash error code ::eBootloaderErrorCodes
 * 			- FLASH_OK: The checksum calculation was successful.
 */
uint8_t Flash_CalculateAppChecksum(uint32_t *checksum_value)
{
    uint32_t app_size = *(uint32_t *)APP_SIZE_ADDRESS;

    // Calculate the checksum of the user application using the CRC peripheral
    *checksum_value = HAL_CRC_Calculate(&hcrc, (uint32_t *)APP_START_ADDRESS, app_size);

    return FLASH_OK;
}

/**
 * @brief	This function writes the checksum value to the flash memory.
 * @param	checksum_value: The checksum value to be written.
 * @return	Flash error code ::eFlashErrorCodes
 *			- FLASH_OK: The write operation was successful.
 * 			- FLASH_WRITE_CHKS_ERROR: The write operation failed.
 */
uint8_t Flash_WriteChecksum(uint32_t checksum_value)
{
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP_CRC_ADDRESS, checksum_value) == HAL_ERROR)
    {
        HAL_FLASH_Lock();
        return FLASH_WRITE_CHKS_ERROR;
    }

    return FLASH_OK;
}

/**
 * @brief	This function verifies the checksum value stored in flash memory with the calculated checksum of the application.
 * @param	None
 * @return	Flash error code ::eFlashErrorCodes
 *			- FLASH_CHKS_ERROR: Checksum verification failed
 *			- FLASH_OK: Checksum verification successful
 */
uint8_t Flash_VerifyChecksum(void)
{
    uint32_t calculatedCrc = 0;

    // Calculate the checksum of the application
    Flash_CalculateAppChecksum(&calculatedCrc);

    // Compare the calculated checksum with the stored checksum in flash memory
    if ((*(uint32_t*)APP_CRC_ADDRESS) != calculatedCrc)
    {
        return FLASH_CHKS_ERROR;
    }

    return FLASH_OK;
}


/**
 * @brief	This function writes the size of the application to the specified address in flash memory.
 * @param	app_size: Size of the application
 * @return	Flash error code ::eFlashErrorCodes
 *			- FLASH_WRITE_APPSIZE_ERROR: Error occurred while writing the application size
 *			- FLASH_OK: Application size written successfully
 */
uint8_t Flash_WriteAppSize(uint32_t app_size)
{
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP_SIZE_ADDRESS, app_size) == HAL_ERROR)
    {
        HAL_FLASH_Lock();
        return FLASH_WRITE_APPSIZE_ERROR;
    }

    return FLASH_OK;
}
