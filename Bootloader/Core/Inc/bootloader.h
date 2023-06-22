
#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

/* Includes --------------------------------------------------------------*/

#include <stdbool.h>


/* Typedef --------------------------------------------------------------*/

typedef void (*pFunction)(void);

typedef enum
{
    BL_STATE_IDLE,
	BL_STATE_ABORT,
	BL_STATE_EXECUTE,
	BL_STATE_ERASE_APP,
	BL_STATE_SEND_ERROR,
	BL_STATE_DOWNLOAD_FW

} e_Bootloader_State;


typedef enum
{
	BL_OK						= 0,			// Bootloader operation successful
	BL_CHKS_MISMATCH			= 0x7F, 		// Application checksum incorrect
	BL_CMD_INVALID,								// Invalid command
	BL_INVALID_STATE,							// Invalid state
	BL_RECEIVE_TIMEOUT,							// Receive timeout reached
	BL_DOWNLOAD_FAILED,							// Firmware download failed
	BL_NO_USER_APP								// No user application found

} e_Bootloader_Status;


typedef enum
{
	CMD_ID_ACK				= 0x10,				// Command ID: Acknowledge
	CMD_ID_PACKET			= 0x20,				// Command ID: Packet
	CMD_ID_PACKET_ACK		= 0x30,				// Command ID: Packet Acknowledge
	CMD_ID_PACKET_NACK		= 0x40,				// Command ID: Packet Negative Acknowledge
	CMD_ID_ERROR			= 0x50,				// Command ID: Error
	CMD_ID_EXECUTE			= 0x60,				// Command ID: Execute
	CMD_ID_ERASE_APP		= 0x70,				// Command ID: Erase Application
	CMD_ID_DOWNLOAD_FW		= 0x80				// Command ID: Download Firmware

} e_Bootloader_CMD_ID;


/* Functions --------------------------------------------------------------*/

void Bootloader_Run(void);
void Bootloader_JumToApplication(void);
bool Bootloader_CheckApplicationExist(void);
uint8_t Bootloader_EraseApplication(void);
uint8_t Bootloader_DownloadFW(uint16_t total_packets);
uint8_t Bootloader_VerifyAppChecksum(uint32_t app_checksum, uint32_t app_word_size);

#endif /* __BOOTLOADER_H */
