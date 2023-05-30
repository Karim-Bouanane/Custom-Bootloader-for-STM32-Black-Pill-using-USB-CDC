
/* Includes --------------------------------------------------------------*/

#include "../Inc/flash.h"

/* Typedef --------------------------------------------------------------*/

typedef void (*pFunction)(void);

typedef enum
{
    STATE_IDLE,
	STATE_EXECUTE,
	STATE_SEND_ACK,
	STATE_SEND_ERROR,
	STATE_GET_FW_VER,
	STATE_DOWNLOAD_FW,
	STATE_ERASE_APPLICATION

} BootloaderState;

typedef enum
{
    CMD_READ = 0x10,
    CMD_WRITE = 0x20,
    CMD_ERASE_APP = 0x30,
    CMD_EXECUTE = 0x40,
    CMD_DOWNLOAD_FW = 0x50,
    CMD_GET_FW_VER = 0x60,
    CMD_ACK = 0x70,
    CMD_ERROR = 0x80

} BootloaderCMD;


/* Functions --------------------------------------------------------------*/

void BootloaderStateMachine(void)
{
    static BootloaderState currentState = STATE_IDLE;

    switch (currentState)
    {
        case STATE_IDLE:

            Bootloader_Wait_Commands();

            switch (command)
            {
                case CMD_EXECUTE:
                    currentState = STATE_EXECUTE;
                    break;

                case CMD_ERASE_APPLICATION:
                    currentState = STATE_ERASE_APPLICATION;
                    break;

                case CMD_DOWNLOAD_FW:
                    currentState = STATE_DOWNLOAD_FW;
                    break;

                default:
                    currentState = STATE_SEND_ERROR;
                    SendError(ERROR_INVALID_COMMAND);
                    break;
            }
            break;

        case STATE_EXECUTE:

            Bootloader_SendState(STATE_EXECUTE);
            Bootloader_JumToApplication();

            // Handle Read Memory command
            // Process the command, read memory, and send response
            // Transition to the appropriate state
            break;

        case STATE_SEND_ACK:
            // Handle Send Acknowledge command
                // Send acknowledgment response
                currentState = STATE_IDLE;
            break;

        case STATE_SEND_ERROR:
            // Handle Send Error command
            // Send error response
            currentState = STATE_IDLE;
            break;

        case STATE_GET_FW_VER:
            // Handle Send Error command
            // Send error response
            currentState = STATE_IDLE;
            break;

        case STATE_DOWNLOAD_FW:
            // Handle Download Firmware command
            // Process the command, download firmware, and send response
            // Transition to the appropriate state
            break;

        case STATE_ERASE_APPLICATION:
            // Handle Erase Application command
            // Process the command, erase application, and send response
            // Transition to the appropriate state
            break;

        default:
            // Invalid state
            currentState = STATE_SEND_ERROR;
            SendError(ERROR_INVALID_STATE);
            break;
    }
}

/***** *****/

void Bootloader_JumToApplication(void)
{
    uint32_t application_entry_point_address = *(volatile uint32_t *)(APP_START_ADDRESS + 4);

    pFunction application_entry_point = (pFunction)&application_entry_point_address;

    // Disable interrupts
    __disable_irq();

    // Reset peripherals
    HAL_RCC_DeInit(); // cant deinitialize it because we're using usb peripheral which needs HSE
    HAL_DeInit(); // cant deinitialize it because all peripherals will be reset

    // Reset Systick
    SysTick->CTRL = 0;  // Disable SysTick
    SysTick->VAL = 0;   // Reset current value
    SysTick->LOAD = 0;  // Reset reload value

    // Set the vector table base address
    SCB->VTOR = APP_START_ADDRESS;

    // Set the stack pointer
    __set_MSP(*(volatile uint32_t*)(APP_START_ADDRESS));

    // Jump to the application
    application_entry_point();
}


void Bootloader_JumpToSysMemory(void)
{
    uint32_t bootloader_rom_address = *(volatile uint32_t*)(SYSMEM_ADDRESS + 4);
    pFunction bootloader_rom = (pFunction)bootloader_rom_address;

    // Disable interrupts
    __disable_irq();

    // Reset peripherals
    HAL_RCC_DeInit();
    HAL_DeInit();

    // Reset Systick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

    // Set the stack pointer
    __set_MSP(*(volatile uint32_t*)SYSMEM_ADDRESS);

    // Jump to the bootloader 
    bootloader_rom();
}


uint8_t Bootloader_CheckApplicationExist(bool *app_exist)
{
    uint32_t appMainEntryAddr;

    if (Flash_Read(APP_START_ADDRESS + 4, &appMainEntryAddr, 1) != FLASH_OK)
    {
        return BL_ERROR;
    }

    // Check if the application start address is valid
    if (appMainEntryAddr < APP_START_ADDRESS || appMainEntryAddr > (APP_END_ADDRESS - 4))
    {
        // Invalid application start address
        *app_exist = false;
    }

    *app_exist = true;

    return BL_TRUE;
}


uint8_t Bootloader_EraseApplication(void)
{
    uint8_t try = 3;

    while(Flash_EraseApplication() != FLASH_OK && try--)
    {}

    if(try == 0)
    {
        return BL_ERASE_APP_ERROR;
    }

    return BL_OK;
}

uint8_t Bootloader_WriteApplication(uint32_t *data, uint32_t size)
{

    if (Flash_Write(APP_START_ADDRESS, data, size) != FLASH_OK)
    {
        return false;
    }

    return BL_OK;
}

uint8_t Bootloader_VerifyApplicationChecksum(void)
{
    // Calculate the checksum of the application using the Flash_CalculateAppChecksum function from the flash library
    uint32_t calculatedChecksum;
    if (Flash_CalculateAppChecksum(&calculatedChecksum) != FLASH_OK)
    {
        // Error occurred while calculating the checksum
        return false;
    }

    // Read the stored checksum from flash memory
    uint32_t storedChecksum;
    if (Flash_Read(APP_CRC_ADDRESS, &storedChecksum, sizeof(uint32_t)) != FLASH_OK)
    {
        // Error occurred while reading the stored checksum
        return false;
    }

    // Compare the calculated checksum with the stored checksum
    if (calculatedChecksum != storedChecksum)
    {
        // Checksum mismatch
        return false;
    }

    return true;
}

uint8_t Bootloader_UpdateApplicationSize(uint32_t appSize)
{
    // Call the Flash_WriteAppSize function from the flash library
    if (Flash_WriteAppSize(appSize) != FLASH_OK)
    {
        return BL_;
    }

    return true;
}


