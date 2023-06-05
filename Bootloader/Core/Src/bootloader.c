
/* Includes --------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bootloader.h"
#include "usbd_cdc_if.h"
#include "flash.h"


/* Macro Definition --------------------------------------------------------------*/

#define MAX_TIMEOUT				5000
#define NO_TIMEOUT				0
#define PACKET_HEADER_SIZE		5

/* Global variables --------------------------------------------------------------*/

static uint8_t packet_buffer[APP_RX_DATA_SIZE] = {0};

uint8_t error_id = BL_OK;


/* Static Functions --------------------------------------------------------------*/

static void SendError(void)
{
	uint8_t error_msg[4] = {0};

	error_msg[0] = CMD_ERROR;
	error_msg[1] = error_id;
	error_msg[2] = 0;				// padding
	error_msg[3] = 0;				// padding

	CDC_Transmit_FS(error_msg, 4);
}


static void SendCmdAck(uint8_t command_id)
{
	uint8_t cmd_ack_msg[4] = {0};

	cmd_ack_msg[0] = CMD_ACK;
	cmd_ack_msg[1] = command_id;
	cmd_ack_msg[2] = 0; 			// padding
	cmd_ack_msg[3] = 0; 			// padding

	CDC_Transmit_FS(cmd_ack_msg, 4);
}


static void SendPacketAck(uint8_t packet_number, uint16_t packet_size)
{
	uint8_t packet_ack_msg[4] = {0};

	packet_ack_msg[0] = CMD_PACKET_ACK;
	packet_ack_msg[1] = packet_number;
	packet_ack_msg[2] = (uint8_t)(packet_size);
	packet_ack_msg[3] = (uint8_t)(packet_size >> 8);

	CDC_Transmit_FS(packet_ack_msg, 4);
}


/* Functions --------------------------------------------------------------*/

void Bootloader_Run(void)
{

    uint8_t status;
    uint16_t packet_size = 0;
    e_BootloaderState currentState = STATE_IDLE;

	while(1)
	{
	    status = Flash_Init();

	    if(status != FLASH_OK)
	    {
			error_id = status;
			currentState = STATE_SEND_ERROR;
	    }

		switch (currentState)
		{
			case STATE_IDLE:

				packet_size = CDC_Get_Received_Data_FS(packet_buffer, MAX_TIMEOUT);

				if(packet_size == 4)
				{
					switch (packet_buffer[0])
					{
						case CMD_EXECUTE:
							currentState = STATE_EXECUTE;
							SendCmdAck(STATE_EXECUTE);
							break;

						case CMD_DOWNLOAD_FW:
							currentState = STATE_DOWNLOAD_FW;
							SendCmdAck(STATE_DOWNLOAD_FW);
							break;

						case CMD_ERASE_APP:
							currentState = STATE_ERASE_APP;
							SendCmdAck(STATE_ERASE_APP);
							break;

						default:
							error_id = BL_CMD_INVALID_ERROR;
							currentState = STATE_SEND_ERROR;
							break;
					}
				}

				break;


			case STATE_ABORT:

				SendError();
				Bootloader_EraseApplication();
				currentState = STATE_IDLE;

				break;


			case STATE_EXECUTE:

				CDC_Transmit_FS((uint8_t *)"Executing the application...\n", 29);
				Bootloader_JumToApplication();

				break;


			case STATE_SEND_ERROR:

				CDC_Transmit_FS((uint8_t *)"Error occured.\n", 32);
				SendError();
				currentState = STATE_IDLE;

				break;


			case STATE_DOWNLOAD_FW:

				CDC_Transmit_FS((uint8_t *)"Downloading the new firmware...\n", 32);

				/*status = Bootloader_DownloadFW();

				if(status != BL_OK)
				{
					currentState = STATE_ABORT;
				}
				else
				{
					CDC_Transmit_FS((uint8_t *)"The new firmware was successfully downloaded.\n", 38);
					currentState = STATE_EXECUTE;
				}
				*/

				break;


			case STATE_ERASE_APP:

				CDC_Transmit_FS((uint8_t *)"Erasing the application in progress...\n", 39);
				status = Bootloader_EraseApplication();

				if(status != BL_OK)
				{
					error_id = status;
					currentState = STATE_SEND_ERROR;
					CDC_Transmit_FS((uint8_t *)"Failed to erase the application.\n", 32);
				}
				else
				{
					currentState = STATE_IDLE;
					CDC_Transmit_FS((uint8_t *)"Application was successfully erased.\n", 32);
				}

				break;


			default:

				error_id = BL_INVALID_STATE_ERROR;
				currentState = STATE_SEND_ERROR;

				break;
		}
	}
}


void Bootloader_JumToApplication(void)
{
    uint32_t application_entry_point_address = (*(volatile uint32_t *)(APP_START_ADDRESS + 4));

    pFunction application_entry_point = (pFunction)application_entry_point_address ;

    // Disable interrupts
    __disable_irq();

    // Reset peripherals
    HAL_RCC_DeInit();
    HAL_DeInit();

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


bool Bootloader_CheckApplicationExist(void)
{
    uint32_t stack_address = 0;

    Flash_Read_Word(APP_START_ADDRESS, &stack_address, (uint32_t)1);

    if ((stack_address < RAM_BASE_ADDRESS) && ((stack_address - RAM_BASE_ADDRESS) > RAM_SIZE))
    {
        return false;
    }

    return true;
}

/**
 * @brief	This function erases the application area in flash memory.
 * @return	Flash error code ::eFlashErrorCodes
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

/*

uint8_t Bootloader_DownloadFW(uint32_t *app_size)
{
	uint8_t status;

	uint8_t try = 0;
	uint32_t total_size;
	uint32_t write_address = APP_START_ADDRESS;

	uint8_t cmd_id;
	uint16_t packet_number = 0;
	uint16_t packet_track_num = 0;

	uint16_t packet_size;
	uint16_t recv_size;


	do
	{
		recv_size = CDC_Get_Received_Data_FS(packet_buffer, 2000);

		cmd_id = packet_buffer[0];
		packet_number = (uint16_t)packet_buffer[1] + ((uint16_t)packet_buffer[2] << 8);
		packet_size = packet_buffer[3] + (packet_buffer[4] << 8);

		if((cmd_id == CMD_PACKET) && (recv_size == packet_size) && (packet_number == packet_track_num) )
		{
			SendPacketAck(packet_number, packet_size);
			packet_track_num ++;
			try = 0;

			status = Flash_Write_Word(write_address, &packet_buffer[5], packet_size - PACKET_HEADER_SIZE);

			if(status != FLASH_OK)
			{

			}

			total_size += packet_size - PACKET_HEADER_SIZE;
		}
		else
		{
			SendPacketAck(packet_track_num, recv_size);
			try++;
		}

	}while((recv_size != 0) && (try != 3) && (recv_size != PACKET_HEADER_SIZE));

	if(1)
	{
		void;
	}

    return BL_OK;
}
*/


uint8_t Bootloader_VerifyAppChecksum(uint32_t app_checksum, uint32_t app_size)
{
	uint32_t calculatedCRC;

	calculatedCRC = Flash_GetChecksum(APP_START_ADDRESS, app_size);

	if(app_checksum != calculatedCRC)
	{
		return BL_CHKS_ERROR;
	}

	return BL_OK;
}


