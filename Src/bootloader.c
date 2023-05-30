

// State machine states
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


// Bootloader state machine function
void BootloaderStateMachine(void)
{
    static BootloaderState currentState = STATE_IDLE;

    // Check for incoming command
    if (IsCommandReceived())
    {
        uint8_t command = ReceiveCommand();

        switch (currentState)
        {
            case STATE_IDLE:
                switch (command)
                {
                    case CMD_ERASE_APPLICATION:
                        currentState = STATE_ERASE_APPLICATION;
                        break;
                    case CMD_EXECUTE:
                        currentState = STATE_EXECUTE;
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
}



/**
 * @brief  This function performs the jump to the user application in flash.
 * @details The function carries out the following operations:
 *  - De-initialize the clock and peripheral configuration
 *  - Stop the systick
 *  - Set the vector table location (if ::SET_VECTOR_TABLE is enabled)
 *  - Sets the stack pointer location
 *  - Perform the jump
 */
/*
void Bootloader_JumpToApplication(void)
{
    uint32_t JumpAddress = *(__IO uint32_t*)(APP_ADDRESS + 4);
    pFunction Jump       = (pFunction)JumpAddress;

    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

#if(SET_VECTOR_TABLE)
    SCB->VTOR = APP_ADDRESS;
#endif

    __set_MSP(*(__IO uint32_t*)APP_ADDRESS);
    Jump();
}
*/

/**
 * @brief  This function performs the jump to the MCU System Memory (ST
 *         Bootloader).
 * @details The function carries out the following operations:
 *  - De-initialize the clock and peripheral configuration
 *  - Stop the systick
 *  - Remap the system flash memory
 *  - Perform the jump
 */

/*
void Bootloader_JumpToSysMem(void)
{
    uint32_t JumpAddress = *(__IO uint32_t*)(SYSMEM_ADDRESS + 4);
    pFunction Jump       = (pFunction)JumpAddress;

    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

    __set_MSP(*(__IO uint32_t*)SYSMEM_ADDRESS);
    Jump();

    while(1)
        ;
}

*/

bool Bootloader_CheckApplicationExist(void)
{
    // Call the Flash_Read function to read the application start address
    uint32_t appStartAddress;
    if (Flash_Read(APP_START_ADDRESS, &appStartAddress, sizeof(uint32_t)) != FLASH_OK)
    {
        // Error occurred while reading from flash
        return false;
    }

    // Check if the application start address is valid
    if (appStartAddress == 0xFFFFFFFF || appStartAddress == 0x00000000)
    {
        // Invalid application start address
        return false;
    }

    return true;
}


bool Bootloader_EraseApplication(void)
{
    // Call the Flash_EraseApplication function from the flash library
    if (Flash_EraseApplication() != FLASH_OK)
    {
        // Error occurred while erasing application sectors
        return false;
    }

    return true;
}

bool Bootloader_WriteApplication(uint32_t *data, uint32_t size)
{
    // Call the Flash_Write function from the flash library
    if (Flash_Write(APP_START_ADDRESS, data, size) != FLASH_OK)
    {
        // Error occurred while writing application data to flash
        return false;
    }

    return true;
}

bool Bootloader_VerifyApplicationChecksum(void)
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

bool Bootloader_UpdateApplicationSize(uint32_t appSize)
{
    // Call the Flash_WriteAppSize function from the flash library
    if (Flash_WriteAppSize(appSize) != FLASH_OK)
    {
        // Error occurred while updating the application size
        return false;
    }

    return true;
}


