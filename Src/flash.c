
#include "flash.h"


uint8_t Flash_Begin(void)
{
    /* Clear Flash flags */
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
                   
    if (HAL_FLASH_GetStatus() != HAL_OK)
    {
        return; 
    }

    return ;
}

uint8_t Flash_EraseSector(uint32_t sector)
{
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t SectorError;

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Sector = sector;
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    HAL_FLASHEx_Erase(&eraseInit, &SectorError);
}

uint8_t Flash_EraseAllApp(void)
{
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t SectorError;

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Sector = sector;
    eraseInit.NbSectors = APP_TOTAL_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    HAL_FLASHEx_Erase(&eraseInit, &SectorError);
}


uint8_t Flash_Write(uint32_t address, uint32_t *data, uint32_t size)
{
    for(uint32_t i = 0; i < size; i += 4)
    {
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, data[i / 4]))
        {
            return ;
        }
    }

    return;
}


uint8_t Flash_Read(uint32_t address, uint32_t *data, uint32_t size)
{
    for(uint32_t i = 0; i < size; i += 4)
    {
        data[i / 4] = *(uint32_t *)(address + i);
    }

    return;
}


uint8_t Flash_End(void)
{
    /* Clear Flash flags */
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
                   
    if (HAL_FLASH_GetStatus() != HAL_OK)
    {
        return; 
    }

    return ;
}


uint8_t GetProtectionStatus(void)
{

}