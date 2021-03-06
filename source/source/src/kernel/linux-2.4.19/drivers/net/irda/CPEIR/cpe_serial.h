/****************************************************************************
* Porting to Linux on 20030822											   *
* Author: Paul Chiang													   *
* Version: 0.1								  							   * 
* History: 								  								   *
*          0.1 new creation						   						   *
* Todo: 																   *
****************************************************************************/
#ifndef CPE_SERIAL_H
#define CPE_SERIAL_H
#include <linux/types.h>
#include <linux/config.h>
#include <asm/arch-cpe/hardware.h>	// for IO_ADDRESS
//#include <stdio.h>
//#include <Stdlib.h>

#define UINT32 	uint
#define UINT8  	unchar
#define UINT16 	ushort
#define INT32 	int
#define INT8 	char
#define BOOL	char 
#define UINT64 long long
#define FALSE 0     
#define TRUE 1 
#define ON 1
#define OFF 0

#define SERIAL_THR                     0x00	 		/*  Transmitter Holding Register(Write).*/
#define SERIAL_RBR                     0x00	 		/*  Receive Buffer register (Read).*/
#define SERIAL_IER                     0x04	 		/*  Interrupt Enable register.*/
#define SERIAL_IIR                     0x08	 		/*  Interrupt Identification register(Read).*/
#define SERIAL_FCR                     0x08	 		/*  FIFO control register(Write).*/
#define SERIAL_LCR                     0x0C	 		/*  Line Control register.*/
#define SERIAL_MCR                     0x10	 		/*  Modem Control Register.*/
#define SERIAL_LSR                     0x14	 		/*  Line status register(Read) .*/
#define SERIAL_MSR                     0x18	 		/*  Modem Status register (Read).*/
#define SERIAL_SPR                     0x1C     	/*  Scratch pad register */
#define SERIAL_DLL                     0x0      	/*  Divisor Register LSB */
#define SERIAL_DLM                     0x4      	/*  Divisor Register MSB */
#define SERIAL_PSR                     0x8     		/* Prescale Divison Factor */

#define SERIAL_MDR						0x20
#define SERIAL_ACR						0x24
#define SERIAL_TXLENL					0x28
#define SERIAL_TXLENH					0x2C
#define SERIAL_MRXLENL					0x30
#define SERIAL_MRXLENH					0x34
#define SERIAL_PLR						0x38
#define SERIAL_FMIIR_PIO				0x3C

/* IER Register */
#define SERIAL_IER_DR                  0x1      	/* Data ready Enable */
#define SERIAL_IER_TE                  0x2      	/* THR Empty Enable */
#define SERIAL_IER_RLS                 0x4      	/* Receive Line Status Enable */
#define SERIAL_IER_MS                  0x8      	/* Modem Staus Enable */


/* IIR Register */
#define SERIAL_IIR_NONE                0x1			/* No interrupt pending */
#define SERIAL_IIR_RLS                 0x6			/* Receive Line Status */
#define SERIAL_IIR_DR                  0x4			/* Receive Data Ready */
#define SERIAL_IIR_TIMEOUT             0xc			/* Receive Time Out */
#define SERIAL_IIR_TE                  0x2			/* THR Empty */
#define SERIAL_IIR_MODEM               0x0			/* Modem Status */

#define SERIAL_IIR_ID		0x6		//paulong

/* FCR Register */
#define SERIAL_FCR_FE                  0x1 	 		/* FIFO Enable */
#define SERIAL_FCR_RXFR                0x2 	 		/* Rx FIFO Reset */
#define SERIAL_FCR_TXFR                0x4 	 		/* Tx FIFO Reset */

/* LCR Register */
#define SERIAL_LCR_LEN5                0x0
#define SERIAL_LCR_LEN6                0x1
#define SERIAL_LCR_LEN7                0x2
#define SERIAL_LCR_LEN8                0x3

#define SERIAL_LCR_STOP                0x4
#define SERIAL_LCR_EVEN                0x18 	 	/* Even Parity */
#define SERIAL_LCR_ODD                 0x8      	/* Odd Parity */
#define SERIAL_LCR_PE                  0x8			/* Parity Enable */
#define SERIAL_LCR_SETBREAK            0x40	 		/* Set Break condition */
#define SERIAL_LCR_STICKPARITY         0x20	 		/* Stick Parity Enable */
#define SERIAL_LCR_DLAB                0x80     	/* Divisor Latch Access Bit */

/* LSR Register */
#define SERIAL_LSR_DR                  0x1      	/* Data Ready */
#define SERIAL_LSR_OE                  0x2      	/* Overrun Error */
#define SERIAL_LSR_PE                  0x4      	/* Parity Error */
#define SERIAL_LSR_FE                  0x8      	/* Framing Error */
#define SERIAL_LSR_BI                  0x10     	/* Break Interrupt */
#define SERIAL_LSR_THRE                0x20     	/* THR Empty */
#define SERIAL_LSR_TE                  0x40     	/* Transmitte Empty */
#define SERIAL_LSR_DE                  0x80     	/* FIFO Data Error */

/* MCR Register */
#define SERIAL_MCR_DTR                 0x1		/* Data Terminal Ready */
#define SERIAL_MCR_RTS                 0x2		/* Request to Send */
#define SERIAL_MCR_OUT1                0x4		/* output	1 */
#define SERIAL_MCR_OUT2                0x8		/* output2 or global interrupt enable */
#define SERIAL_MCR_LPBK                0x10	 	/* loopback mode */


/* MSR Register */
#define SERIAL_MSR_DELTACTS            0x1		/* Delta CTS */
#define SERIAL_MSR_DELTADSR            0x2		/* Delta DSR */
#define SERIAL_MSR_TERI                0x4		/* Trailing Edge RI */
#define SERIAL_MSR_DELTACD             0x8		/* Delta CD */
#define SERIAL_MSR_CTS                 0x10	 	/* Clear To Send */
#define SERIAL_MSR_DSR                 0x20	 	/* Data Set Ready */
#define SERIAL_MSR_RI                  0x40	 	/* Ring Indicator */
#define SERIAL_MSR_DCD                 0x80	 	/* Data Carrier Detect */


/* MDR register */
#define SERIAL_MDR_MODE_SEL				0x03
#define SERIAL_MDR_UART					0x0
#define SERIAL_MDR_SIR					0x1
#define SERIAL_MDR_FIR					0x2

/* ACR register */
#define SERIAL_ACR_TXENABLE				0x1
#define SERIAL_ACR_RXENABLE				0x2
#define SERIAL_ACR_SET_EOT				0x4


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */

//extern UINT32 DebugSerialPort;
//extern UINT32 SystemSerialPort;

extern void fLib_SerialInit(UINT32 port, UINT32 baudrate, UINT32 parity,UINT32 num,UINT32 len);
extern void fLib_SetSerialFifoCtrl(UINT32 port, UINT32 level_tx, UINT32 level_rx,UINT32 resettx, UINT32 resetrx); //V1.20//ADA10022002
extern void fLib_DisableSerialFifo(UINT32 port);
extern void fLib_SetSerialInt(UINT32 port, UINT32 IntMask);

extern char fLib_GetSerialChar(UINT32 port);
extern void fLib_PutSerialChar(UINT32 port, char Ch);
extern void fLib_PutSerialStr(UINT32 port, char *Str);

extern void fLib_EnableSerialInt(UINT32 port, UINT32 mode);
extern void fLib_DisableSerialInt(UINT32 port, UINT32 mode);

extern void fLib_SerialRequestToSend(UINT32 port);
extern void fLib_SerialStopToSend(UINT32 port);
extern void fLib_SerialDataTerminalReady(UINT32 port);
extern void fLib_SerialDataTerminalNotReady(UINT32 port);

extern void fLib_SetSerialLineBreak(UINT32 port);
extern void fLib_SetSerialLoopBack(UINT32 port,UINT32 onoff);
extern UINT32 fLib_SerialIntIdentification(UINT32 port);

extern UINT32 fLib_ReadSerialLineStatus(UINT32 port);
extern UINT32 fLib_ReadSerialModemStatus(UINT32 port);

extern void fLib_SetSerialMode(UINT32 port, UINT32 mode);
extern void fLib_EnableIRMode(UINT32 port, UINT32 TxEnable, UINT32 RxEnable);

extern void fLib_Modem_call(UINT32 port, char *tel);
extern void fLib_Modem_waitcall(UINT32 port);
extern int  fLib_Modem_getchar(UINT32 port,int TIMEOUT);
extern BOOL fLib_Modem_putchar(UINT32 port, INT8 Ch);
//extern BOOL fLib_Modem_Initial(UINT32 port, int baudRate, UINT32 parity, UINT32 stopbit, UINT32 databit);
//extern void fLib_Modem_close(UINT32 port);  

extern void fLib_WriteSerialChar(UINT32 port, char Ch);
extern void fLib_EnableMCROuts(UINT32 port, UINT32 index);
extern void fLib_DisableMCROuts(UINT32 port, UINT32 index);


 #ifdef CONFIG_UCLINUX	// uclinux without mmu
//	#error "CONFIG_UCLINUX"
	#define OUTW(port,value) 	 		outl(value,port)
	#define INW(port) 		 			inl(port)
	//#define OUTB(port,value) 	 		outb(value,port)
	//#define OUTB1(value,port) 		outb(value,port)		//paulong test test?! BYTE access ok!?
	#define INB(port) 		 			inb(port)	
	#define REQUEST_IO_REGION(a,b,c) 	request_region(a,b,c)
	#define RELEASE_IO_REGION(a,b) 		release_region(a,b)	
	#define CHECK_IO_REGION(a,b) 		check_region(a,b)
 #else	//arm linux with mmu
  	
	#undef 	IO_ADDRESS1
	#define IO_ADDRESS1(x) ((((x >> 4)&0xffff0000) + (x & 0xffff)) | IO_BASE) 
	
	#define OUTW(port,value) 			outl(value,(IO_ADDRESS1((port))))
	#define INW(port) 					inl((IO_ADDRESS1((port))))
	//#define OUTB(port,value) 			outb(value,(IO_ADDRESS1(port))))
	//#define OUTB1(value,port) 		outb(value,(IO_ADDRESS1((port))))
	#define INB(port) 					inb((IO_ADDRESS1((port))))
	#define REQUEST_IO_REGION(a,b,c) 	request_region((IO_ADDRESS1((a))),b,c)	
	#define RELEASE_IO_REGION(a,b) 		release_region((IO_ADDRESS1((a))),b)		
	#define CHECK_IO_REGION(a,b) 		check_region((IO_ADDRESS1((a))),b)	
 #endif

	void fLib_ResetSerialInt(UINT32 port);	//paulong 

#endif	//#ifndef CPE_SERIAL_H