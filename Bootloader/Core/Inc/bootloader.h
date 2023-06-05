
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
	BL_CHKS_ERROR				= 0x7F, /*!< Application checksum incorrect */
	BL_CMD_INVALID_ERROR,
	BL_INVALID_STATE_ERROR,

} e_Bootloader_Errors;


typedef enum
{
	CMD_ACK				= 0x10,
	CMD_PACKET_ACK		= 0x20,
	CMD_PACKET			= 0x30,
	CMD_ERROR			= 0x40,
	CMD_EXECUTE			= 0x60,
	CMD_ERASE_APP		= 0x70,
	CMD_DOWNLOAD_FW		= 0x80

} e_Bootloader_CMD;


/* Functions --------------------------------------------------------------*/

void Bootloader_Run(void);
void Bootloader_JumToApplication(void);
bool Bootloader_CheckApplicationExist(void);
uint8_t Bootloader_EraseApplication(void);
uint8_t Bootloader_DownloadFW(uint32_t *app_size);
uint8_t Bootloader_VerifyAppChecksum(uint32_t app_checksum, uint32_t app_size);


#endif /* __BOOTLOADER_H */
