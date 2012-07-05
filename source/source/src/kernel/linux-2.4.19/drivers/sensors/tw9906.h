#ifndef __TW9906_H__
#define __TW9906_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/i2c-proc.h>
#include <linux/init.h>
#include <video/plmedia.h>
#include <video/plsensor.h>

#define	TW9906_SYSCTL_READ			0x1000
#define	TW9906_SYSCTL_WRITE			0x1001
#define TW9906_CHIP_ID				0x08

//	Sensor Core registers; page #0...
//	write 0 to register 0xF0 to switch to page #0

#define TW9906REG_VER_ID			0x00
#define TW9906REG_STATUS1			0x01
#define TW9906REG_INFORM			0x02
#define TW9906REG_OPFORM			0x03
#define TW9906REG_HSYNC				0x04
#define TW9906REG_OCNTL1			0x05
#define TW9906REG_ACNTL				0x06
#define TW9906REG_CROP_HI			0x07
#define TW9906REG_VDELAY_LO			0x08
#define TW9906REG_VACTIVE_LO		0x09
#define TW9906REG_HDELAY_LO			0x0A
#define TW9906REG_HACTIVE_LO		0x0B
#define TW9906REG_CNTRL1			0x0C
#define TW9906REG_VSCALE_LO			0x0D
#define TW9906REG_SCALE_HI			0x0E
#define TW9906REG_HSCALE_LO			0x0F
#define TW9906REG_BRIGHT			0x10
#define TW9906REG_CONTRAST			0x11
#define TW9906REG_SHARPNESS			0x12
#define TW9906REG_SAT_U				0x13
#define TW9906REG_SAT_V				0x14
#define TW9906REG_HUE				0x15
#define TW9906REG_RESERVED16		0x16
#define TW9906REG_VSHARP			0x17
#define TW9906REG_CORING			0x18
#define TW9906REG_VBICNTL			0x19
#define TW9906REG_ANALOGCTRL1		0x1A
#define TW9906REG_OCNTL2			0x1B
#define TW9906REG_SDT				0x1C
#define TW9906REG_SDTR				0x1D
#define TW9906REG_CVFMT				0x1E
#define TW9906REG_TEST				0x1F
#define TW9906REG_CLMPG				0x20
#define TW9906REG_IAGC				0x21
#define TW9906REG_AGCGAIN			0x22
#define TW9906REG_PEAKWT			0x23
#define TW9906REG_CLMPL				0x24
#define TW9906REG_SYNCT				0x25
#define TW9906REG_MISSCNT			0x26
#define TW9906REG_PCLAMP			0x27
#define TW9906REG_VCNTL1			0x28
#define TW9906REG_VCNTL2			0x29
#define TW9906REG_CKILL				0x2A
#define TW9906REG_COMB				0x2B
#define TW9906REG_LDLY				0x2C
#define TW9906REG_MISC1				0x2D
#define TW9906REG_LOOP				0x2E
#define TW9906REG_MISC2				0x2F
#define TW9906REG_MVSN				0x30
#define TW9906REG_STATUS2			0x31
#define TW9906REG_HFREF				0x32
#define TW9906REG_CLMD				0x33
#define TW9906REG_IDCNTL			0x34
#define TW9906REG_CLCNTL1			0x35
//	0x36~0x3F ... were gone
#define TW9906REG_ACKIL				0x40
#define TW9906REG_ACKIM				0x41
#define TW9906REG_ACKIH				0x42
#define TW9906REG_ACKNL				0x43
#define TW9906REG_ACKNM				0x44
#define TW9906REG_ACKNH				0x45
#define TW9906REG_SDIV				0x46
#define TW9906REG_LRDIV				0x47
#define TW9906REG_ACCNTL			0x48
//	0x49~0x4A ... were gone
#define TW9906REG_FILLDATA			0x50
#define TW9906REG_SDID				0x51
#define TW9906REG_DID				0x52
//	0x53~0x54 ... were gone
#define TW9906REG_VVBI				0x55
//0x56~0x6A TW9906REG_LCTL6~TW9906REG_LCTL26
#define TW9906REG_LCTL6				0x56
#define TW9906REG_LCTL7				0x57
#define TW9906REG_LCTL8				0x58
#define TW9906REG_LCTL9				0x59
#define TW9906REG_LCTL10			0x5A
#define TW9906REG_LCTL11			0x5B
#define TW9906REG_LCTL12			0x5C
#define TW9906REG_LCTL13			0x5D
#define TW9906REG_LCTL14			0x5E
#define TW9906REG_LCTL15			0x5F
#define TW9906REG_LCTL16			0x60
#define TW9906REG_LCTL17			0x61
#define TW9906REG_LCTL18			0x62
#define TW9906REG_LCTL19			0x63
#define TW9906REG_LCTL20			0x64
#define TW9906REG_LCTL21			0x65
#define TW9906REG_LCTL22			0x66
#define TW9906REG_LCTL23			0x67
#define TW9906REG_LCTL24			0x68
#define TW9906REG_LCTL25			0x69
#define TW9906REG_LCTL26			0x6A
#define TW9906REG_HSBEGIN			0x6B
#define TW9906REG_HSEND				0x6C
#define TW9906REG_OVSDLY			0x6D
#define TW9906REG_OVSEND			0x6E
#define TW9906REG_VBIDELAY			0x6F
//	0x70~0x93 ... were gone
#define TW9906REG_F2VCNT			0x94
#define TW9906REG_F2VDELAY_LO		0x95
#define TW9906REG_F2VACTIVE_LO		0x96
#define TW9906REG_STATUS1MODE		0x97
#define TW9906REG_STATUS2MODE		0x98
#define TW9906REG_STATUS3MODE		0x99
#define TW9906REG_INTRAWCLEAR		0x9A
#define TW9906REG_TTXF1MASKPAT1		0x9B
#define TW9906REG_TTXF1MASKPAT2		0x9C
#define TW9906REG_TTXF1MASKPAT3		0x9D
#define TW9906REG_TTXF1MASKPAT4		0x9E
#define TW9906REG_TTXF1MASKPAT5		0x9F
#define TW9906REG_TTXF2MASKPAT1		0xA0
#define TW9906REG_TTXF2MASKPAT2		0xA1
#define TW9906REG_TTXF2MASKPAT3		0xA2
#define TW9906REG_TTXF2MASKPAT4		0xA3
#define TW9906REG_TTXF2MASKPAT5		0xA4
#define TW9906REG_TTXFILCTL			0xA5
#define TW9906REG_TTXFRMASK			0xA6
#define TW9906REG_TTXFRPAT			0xA7
#define TW9906REG_GEM2XMASK_LO		0xA8
#define TW9906REG_GEM2X_HI			0xA9
#define TW9906REG_GEM2XPAT_LO		0xAA
#define TW9906REG_SVBIERRORMODE		0xAB
//	0xAC ... was gone
#define TW9906REG_FTHRESHOLD		0xAD
#define TW9906REG_LINENUMBERINT		0xAE
#define TW9906REG_VBIREGLOAD		0xAF
#define TW9906REG_INT1MASK			0xB0
#define TW9906REG_INT2MASK			0xB1
#define TW9906REG_INT3MASK			0xB2
#define TW9906REG_INT1CLEAR			0xB3
#define TW9906REG_INT2CLEAR			0xB4
#define TW9906REG_INT3CLEAR			0xB5
#define TW9906REG_INT1RAWSTATUS		0xB6
#define TW9906REG_INT2RAWSTATUS		0xB7
#define TW9906REG_INT3RAWSTATUS		0xB8
#define TW9906REG_INT1STATUS		0xB9
#define TW9906REG_INT2STATUS		0xBA
#define TW9906REG_INT3STATUS		0xBB
#define TW9906REG_SVBIWCOUNT		0xBC
#define TW9906REG_CCF1DATA1			0xBD
#define TW9906REG_CCF1DATA2			0xBE
#define TW9906REG_CCF2DATA1			0xBF
#define TW9906REG_CCF2DATA2			0xC0
#define TW9906REG_VITCFRAME1		0xC1
#define TW9906REG_VITCFRAME2		0xC2
#define TW9906REG_VITCSEC1			0xC3
#define TW9906REG_VITCSEC2			0xC4
#define TW9906REG_VITCMIN1			0xC5
#define TW9906REG_VITCMIN2			0xC6
#define TW9906REG_VITCHOUR1			0xC7
#define TW9906REG_VITCHOUR2			0xC8
#define TW9906REG_VITCCRC			0xC9
//	following  0xCA~0xDB register are under 625 lines system
#define TW9906REG_WSS_LO			0xCA
#define TW9906REG_WSS_HI			0xCB
#define TW9906REG_VPSDATA1			0xCC
#define TW9906REG_VPSDATA2			0xCD
#define TW9906REG_VPSDATA3			0xCE
#define TW9906REG_VPSDATA4			0xCF
#define TW9906REG_VPSDATA5			0xD0
#define TW9906REG_VPSDATA6			0xD1
#define TW9906REG_VPSDATA7			0xD2
#define TW9906REG_VPSDATA8			0xD3
#define TW9906REG_VPSDATA9			0xD4
#define TW9906REG_VPSDATA10			0xD5
#define TW9906REG_VPSDATA11			0xD6
#define TW9906REG_VPSDATA12			0xD7
#define TW9906REG_VPSDATA13			0xD8
#define TW9906REG_VPSSCODE1			0xD9
#define TW9906REG_VPSSCODE2			0xDA
#define TW9906REG_625TFCODE			0xDB
//	following  0xCA~0xDB register are under 525 lines system
#define TW9906REG_WSS_LO			0xCA
#define TW9906REG_WSS_HI			0xCB
#define TW9906REG_GEM2XF1FRAME_LO	0xCC
#define TW9906REG_GEM2XF1FRAME_HI	0xCD
#define TW9906REG_GEM2XF1DATA1		0xCE
#define TW9906REG_GEM2XF1DATA2		0xCF
#define TW9906REG_GEM2XF1DATA3		0xD0
#define TW9906REG_GEM2XF1DATA4		0xD1
#define TW9906REG_GEM1XF1DATA1		0xD2
#define TW9906REG_GEM1XF1DATA2		0xD3
#define TW9906REG_GEM2XF2FRAME_LO	0xD4
#define TW9906REG_GEM2XF2FRAME_HI	0xD5
#define TW9906REG_GEM2XF2DATA1		0xD6
#define TW9906REG_GEM2XF2DATA2		0xD7
#define TW9906REG_GEM2XF2DATA3		0xD8
#define TW9906REG_GEM2XF2DATA4		0xD9
#define TW9906REG_GEM1XF2DATA1		0xDA
#define TW9906REG_GEM1XF2DATA2		0xDB
//	normal...
#define TW9906REG_VCHIPRA			0xDC
#define TW9906REG_VCHIPG			0xDD
#define TW9906REG_525TFCODE			0xDE
#define TW9906REG_SVBIRDATA			0xDF
#define TW9906REG_EDSSTATUS			0xE0
#define TW9906REG_EDSDATA1			0xE1
#define TW9906REG_EDSDATA2			0xE2
#define TW9906REG_EDSDATA3			0xE3
#define TW9906REG_EDSDATA4			0xE4
#define TW9906REG_EDSDATA5			0xE5
#define TW9906REG_EDSDATA6			0xE6
#define TW9906REG_EDSDATA7			0xE7
#define TW9906REG_EDSDATA8			0xE8
#define TW9906REG_EDSDATA9			0xE9
#define TW9906REG_EDSDATA10			0xEA


#define	TW9906_I2C_IOADDR			0x44

#define STD_ON_DETECTION			0x80

#define TW9906COMPOSITE_SEL			0x44	// 01000100b
#define TW9906SVIDEO_SEL			0x50	// 01010000b
#define TW9906COMPONENT_SEL			0x6E	// 01101110b


typedef struct _tagTW9906Data 
{
	int 				nSysCtlID;
	struct semaphore	UpdateLock;
	char 				cValid;			// !=0 if following fields are valid
	unsigned long		uLastUpdated;	// In jiffies 
	unsigned short		wReg;
	unsigned char 		wRead, wWrite;	// Register values
	devfs_handle_t		DevFS;
	struct i2c_client	*pClient;
	PSENSOR				pSensor;
} 	TW9906DATA;

typedef TW9906DATA *PTW9906DATA;

#endif