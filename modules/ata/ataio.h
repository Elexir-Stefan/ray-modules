#ifndef _ATA_IO_H
#define _ATA_IO_H

/**
 * Ports to access IDE controller
 */
#define ATA_PRIMARY	0x1F0
#define ATA_SECONDARY	0x170

#define ATA_CONTROL_REG_PRIM	0x3F6
#define ATA_CONTROL_REG_SECD	0x376

/**
 * control bits
 */
#define CTRL_nIEN		0x02
#define CTRL_SRST		0x04
#define CTRL_HOB		0x80

/**
 * Relative registers for each controller
 * (WRITING)
 */
#define PORT_DATA		0
#define PORT_FEATURE		1
#define PORT_SECTOR_COUNT	2
#define PORT_LBA		3
#define PORT_CYLINDER_LSB	4
#define PORT_CYLINDER_MSB	5
#define PORT_DRIVE_SELECT	6
#define PORT_COMMAND		7

/**
 * Relative registers for each controller
 * (READING)
 */
#define PORT_STATUS		7
#define PORT_ERROR		1


/**
 * Error bits
 */
#define ATA_ERR_ABRT		0x04


#define CMD_TEST		0xA0



#define CTRL_MASTER		0x00
#define CTRL_SLAVE		0x10

#define CMD_MAGIC		0x88

/**
 * Flags for status register
 */

#define FLAG_ERR		0x01
#define FLAG_DRQ		0x08
#define FLAG_SRV		0x10
#define FLAG_DF			0x20
#define FLAG_READY		0x40
#define FLAG_BUSY		0x80


#endif
