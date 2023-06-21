
#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

/* Includes --------------------------------------------------------------*/

#include <stdbool.h>


/* Typedef --------------------------------------------------------------*/

typedef void (*pFunction)(void);

typedef enum
{
    STATE_IDLE,
	STATE_ABORT,
	STATE_EXECUTE,
	STATE_ERASE_APP,
	STATE_SEND_ERROR,
	STATE_DOWNLOAD_FW

} e_BootloaderState;


typedef enum
{
	BL_OK						= 0,
	BL_CHKS_MISMATCH			= 0x7F, /*!< Application checksum incorrect */
	BL_CMD_INVALID,
	BL_INVALID_STATE,
	BL_RECEIVE_TIMEOUT,
	BL_DOWNLOAD_FAILED,
	BL_NO_USER_APP

} e_Bootloader_Status;


typedef enum
{
	CMD_ID_ACK				= 0x10,
	CMD_ID_PACKET			= 0x20,
	CMD_ID_PACKET_ACK		= 0x30,
	CMD_ID_PACKET_NACK		= 0x40,
	CMD_ID_ERROR			= 0x50,
	CMD_ID_EXECUTE			= 0x60,
	CMD_ID_ERASE_APP		= 0x70,
	CMD_ID_DOWNLOAD_FW		= 0x80

} e_Bootloader_CMD_ID;


/* Functions --------------------------------------------------------------*/

void Bootloader_Run(void);
void Bootloader_JumToApplication(void);
bool Bootloader_CheckApplicationExist(void);
uint8_t Bootloader_EraseApplication(void);
uint8_t Bootloader_DownloadFW(uint16_t total_packets);
uint8_t Bootloader_VerifyAppChecksum(uint32_t app_checksum, uint32_t app_word_size);

void test(void);

#endif /* __BOOTLOADER_H */
