
/* Includes --------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bootloader.h"
#include "usbd_cdc_if.h"
#include "flash.h"


/* Macro Definition --------------------------------------------------------------*/

#define MAX_TIMEOUT				(uint32_t)0xFFFFFFFF			// Maximum timeout value (infinite)
#define NO_TIMEOUT				(uint32_t)0x00					// Disable timeout

#define CMD_PACKET_SIZE			7								// Size of the command packet
#define CMD_RESP_PACKET_SIZE	3								// Size of the command response packet


/* Global variables --------------------------------------------------------------*/

static uint8_t packet_buffer[128] = {0};						// Buffer to store received packets
static uint8_t error_id;										// Save the actual error id to be sent


/* Static Functions --------------------------------------------------------------*/

/**
 * @brief	Send the error command with the error identifier.
 * @param	None
 * @return	None
 */
static void SendError(void)
{
	uint8_t error_msg[CMD_RESP_PACKET_SIZE] = {0};

	error_msg[0] = CMD_ID_ERROR;
	error_msg[1] = error_id;
	error_msg[2] = 0;				// padding to complete CMD_RESP_PACKET_SIZE

	while(CDC_Transmit_FS(error_msg, CMD_RESP_PACKET_SIZE) == USBD_BUSY);
}

/**
 * @brief	Send command acknowledgment message.
 * @param	command_id: The ID of the command being acknowledged.
 * @return	None
 */
static void SendCmdAck(uint8_t command_id)
{
	uint8_t cmd_ack_msg[CMD_RESP_PACKET_SIZE] = {0};

	cmd_ack_msg[0] = CMD_ID_ACK;
	cmd_ack_msg[1] = command_id;
	cmd_ack_msg[2] = 0; 			// padding to complete CMD_RESP_PACKET_SIZE

	while(CDC_Transmit_FS(cmd_ack_msg, CMD_RESP_PACKET_SIZE) == USBD_BUSY);
}

/**
 * @brief	Send packet acknowledgment message.
 * @param	packet_number: The number of the packet being acknowledged.
 * @return	None
 */
static void SendPacketAck(uint16_t packet_number)
{
	uint8_t packet_ack_msg[CMD_RESP_PACKET_SIZE] = {0};

	packet_ack_msg[0] = CMD_ID_PACKET_ACK;
	packet_ack_msg[1] = (uint8_t)(packet_number);			// Set the lower byte of the packet number
	packet_ack_msg[2] = (uint8_t)(packet_number >> 8);		// Set the upper byte of the packet number

	while(CDC_Transmit_FS(packet_ack_msg, CMD_RESP_PACKET_SIZE) == USBD_BUSY);
}

/**
 * @brief	Send packet non-acknowledgment message.
 * @param	packet_number: The number of the packet being non-acknowledged.
 * @return	None
 */
static void SendPacketNAck(uint16_t packet_number)
{
	uint8_t packet_nack_msg[CMD_RESP_PACKET_SIZE] = {0};

	packet_nack_msg[0] = CMD_ID_PACKET_NACK;
	packet_nack_msg[1] = (uint8_t)(packet_number);			// Set the lower byte of the packet number
	packet_nack_msg[2] = (uint8_t)(packet_number >> 8);		// Set the upper byte of the packet number

	while(CDC_Transmit_FS(packet_nack_msg, CMD_RESP_PACKET_SIZE) == USBD_BUSY);
}


/* Functions --------------------------------------------------------------*/

/**
 * @brief	Run the Bootloader state machine
 * @param	None
 * @return	None
 */
void Bootloader_Run(void)
{
    uint8_t status;
    uint16_t total_packets = 0;
    uint32_t app_total_words = 0;
    uint32_t app_checksum = 0;

    e_Bootloader_State currentState = BL_STATE_IDLE;

    // Initialize the Flash Memory
	status = Flash_Init();

	if(status != FLASH_OK)
	{
		error_id = status;
		currentState = BL_STATE_SEND_ERROR;
	}

    while(1)
    {

    	switch (currentState)
    	{
    		case BL_STATE_IDLE:

    			CDC_FlushRxBuffer_FS();

    			status = CDC_ReadRxBuffer_FS(packet_buffer, CMD_PACKET_SIZE, MAX_TIMEOUT);

    			if(status == USBD_OK)
    			{
    				switch (packet_buffer[0])
    				{
    					case CMD_ID_EXECUTE:
    						currentState = BL_STATE_EXECUTE;
    						break;

    					case CMD_ID_DOWNLOAD_FW:
    		    			total_packets = ((uint16_t)packet_buffer[1] & 0xFF) | (((uint16_t)packet_buffer[2] << 8) & 0xFF00);

    		    			app_checksum = ((uint32_t)packet_buffer[3] & 0xFF) | (((uint32_t)packet_buffer[4] << 8) & 0xFF00) |
    		    					(((uint32_t)packet_buffer[5] << 16) & 0xFF0000) | (((uint32_t)packet_buffer[6] << 24) & 0xFF000000);

    						currentState = BL_STATE_DOWNLOAD_FW;
    						break;

    					case CMD_ID_ERASE_APP:
    						currentState = BL_STATE_ERASE_APP;
    						break;

    					default:
    						error_id = BL_CMD_INVALID;
    						currentState = BL_STATE_SEND_ERROR;
    						break;
    				}
    			}

    			break;

    		// This state comes after failing to download the new firmware
    		case BL_STATE_ABORT:

    			Bootloader_EraseApplication();
    			SendError();
    			currentState = BL_STATE_IDLE;

    			break;


    		case BL_STATE_EXECUTE:

    			if(Bootloader_CheckApplicationExist() == true)
    			{
					SendCmdAck(CMD_ID_EXECUTE);
    				Bootloader_JumToApplication();
    			}
    			else
    			{
    				error_id = BL_NO_USER_APP;
    				currentState = BL_STATE_SEND_ERROR;
    			}

    			break;


    		case BL_STATE_SEND_ERROR:

    			SendError();
    			currentState = BL_STATE_IDLE;

    			break;


    		case BL_STATE_DOWNLOAD_FW:

    			SendCmdAck(CMD_ID_DOWNLOAD_FW);

    			status = Bootloader_DownloadFW(total_packets);

    			if(status == BL_OK)
    			{
        			app_total_words = (total_packets * 64) / 4;								// Calculate total words in the application
    				status = Bootloader_VerifyAppChecksum(app_checksum, app_total_words);	// Verify application checksum
    			}

    			if(status == BL_OK)
    			{
        			currentState = BL_STATE_EXECUTE;
    			}
    			else
    			{
    				error_id = status;
    				currentState = BL_STATE_ABORT;
    			}

    			break;


    		case BL_STATE_ERASE_APP:

    			status = Bootloader_EraseApplication();

    			if(status != BL_OK)
    			{
    				error_id = status;
    				currentState = BL_STATE_SEND_ERROR;
    			}
    			else
    			{
					SendCmdAck(CMD_ID_ERASE_APP);
					currentState = BL_STATE_IDLE;
    			}

    			break;


    		default:

    			error_id = BL_INVALID_STATE;
    			currentState = BL_STATE_SEND_ERROR;

    			break;
    	}
    }

}

/**
 * @brief	Jumps to the user application
 * @param	None
 * @return	None
 */
void Bootloader_JumToApplication(void)
{
    uint32_t application_entry_point_address = *(volatile uint32_t *)(APP_BASE_ADDRESS + 4);

    pFunction application_entry_point = (pFunction)application_entry_point_address ;

    // Reset peripherals
    HAL_RCC_DeInit();
    HAL_DeInit();

    // Reset Systick
    SysTick->CTRL = 0;  // Disable SysTick
    SysTick->VAL = 0;   // Reset current value
    SysTick->LOAD = 0;  // Reset reload value

    // Set the vector table base address
    SCB->VTOR = APP_BASE_ADDRESS;

    // Set the stack pointer
    __set_MSP(*(volatile uint32_t*)(APP_BASE_ADDRESS));

    // Jump to the application
    application_entry_point();
}

/**
 * @brief	Checks if a user application exists in the flash memory.
 * @param	None
 * @return	True if a user application exists, false otherwise.
 */
bool Bootloader_CheckApplicationExist(void)
{
    uint32_t stack_address = 0;

    Flash_Read_Word(APP_BASE_ADDRESS, &stack_address, (uint32_t)1);

    if ((stack_address < RAM_BASE_ADDRESS) || ((stack_address - RAM_BASE_ADDRESS) > RAM_SIZE))
    {
        return false;
    }

    return true;
}

/**
 * @brief	This function erases the application area in flash memory.
 * @param	None
 * @return	Flash error code: e_Flash_Status
 *			- FLASH_ERASE_ERROR: The erase operation failed.
 *			- FLASH_OK: The erase operation was successful.
 */
uint8_t Bootloader_EraseApplication(void)
{
	uint8_t status;
	uint8_t try = 3;

	for(uint8_t sector_num = APP_START_SECTOR; sector_num < FLASH_TOTAL_SECTORS; sector_num++)
	{
		do
		{
			status = Flash_EraseSector(sector_num);

		} while((status != FLASH_OK) && --try);

		if(status != FLASH_OK)
		{
	    	break;
		}

		try = 3;
	}

    return status;
}

/**
 * @brief	Downloads the firmware packets and writes them to the flash memory.
 * @param	The total number of firmware packets to download.
 * @return	Bootloader status code: e_Bootloader_Status
 * 			- BL_DOWNLOAD_FAILED: Failed to download the new firmware.
 *			- BL_OK: The download operation was successful.
 */
uint8_t Bootloader_DownloadFW(uint16_t total_packets)
{
	uint8_t status;
	uint8_t try_nb = 3;
	uint16_t packet_num = 0;
	uint16_t packet_size = 64;
	uint16_t packet_total_words = packet_size / 4; // 64/4
	uint32_t rcv_timeout = 2000;
	uint32_t address = APP_BASE_ADDRESS;

	status = Bootloader_EraseApplication();

	if(status == BL_OK)
	{
		do
		{
			status = CDC_ReadRxBuffer_FS(packet_buffer, 64, rcv_timeout);

			if(status == USBD_OK)
			{
				SendPacketAck(packet_num);
				status = Flash_Write_Word(address, (uint32_t *)packet_buffer, packet_total_words);
				address = address + packet_size;
				packet_num ++;
				try_nb = 3;

				//while(CDC_Transmit_FS(packet_buffer, 64) == USBD_BUSY);
			}
			else if(try_nb > 0)
			{
				SendPacketNAck(packet_num);
				try_nb --;
			}
			else
			{
				break;
			}

		} while(packet_num < total_packets);

		if(packet_num != total_packets)
		{
			status = BL_DOWNLOAD_FAILED;
		}
	}

    return status;
}

/**
 * @brief	Verifies the checksum of the downloaded firmware.
 * @param 	app_checksum: The expected checksum of the firmware.
 * @param 	app_word_size: The size of the firmware in words.
 * @return	Bootloader status code: e_Bootloader_Status
 * 			- BL_CHKS_MISMATCH: The calculated checksum doesn't match with the expected checksum
 *			- BL_OK: The calculated checksum match with the expected checksum
 */
uint8_t Bootloader_VerifyAppChecksum(uint32_t app_checksum, uint32_t app_word_size)
{
	uint32_t calculatedCRC;

	calculatedCRC = Flash_GetChecksum(APP_BASE_ADDRESS, app_word_size);

	if(app_checksum != calculatedCRC)
	{
		return BL_CHKS_MISMATCH;
	}

	return BL_OK;
}


