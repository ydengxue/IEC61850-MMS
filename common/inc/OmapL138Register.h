/*****************************************************************************************************
* FileName:                    OmapL138Register.h
*
* Description:                 OmapL138寄存器定义
*
* Author:                      YanDengxue, Fiberhome-Fuhua
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-10-10 11:00       --           1.00             Create
*****************************************************************************************************/
#ifndef _OmapL138_Register_H_
#define _OmapL138_Register_H_

#include "UserTypesDef.h"

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================================================
// 宏定义
//====================================================================================================
#ifndef IO_ADDRESS
#define IO_ADDRESS(x) (x)
#endif

//====================================================================================================
// 寄存器定义
//====================================================================================================
//----------------------------------------------------------------------------------------------------
// System Control Module register structure
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 REVID;//0x00
    VUint8  RSVD0[4];//0x04
    VUint32 DIEIDR[4];//0x08
    VUint32 DEVIDR;//0x18
    VUint8  RSVD1[4];//0x1C
    VUint32 BOOTCFG;//0x20
    VUint8  RSVD2[20];//0x24
    VUint32 KICKR[2];//0x38
    VUint32 HOSTCFG[2];//0x40
    VUint8  RSVD3[152];//0x48
    VUint32 IRAWSTRAT;//0xE0
    VUint32 IENSTAT;//0xE4
    VUint32 IENSET;//0xE8
    VUint32 IENCLR;//0xEC
    VUint32 EOI;//0xF0
    VUint32 FLTADDRR;//0xF4
    VUint32 FLTSTAT;//0xF8
    VUint8  RSVD4[20];//0xFC
    VUint32 MSTPRI[3];//0x110
    VUint8  RSVD5[4];//0x11C
    VUint32 PINMUX[20];//0x120
    VUint32 SUSPSRC;//0x170
    VUint32 CHIPSIG;//0x174
    VUint32 CHIPSIG_CLR;//0x178
    VUint32 CFGCHIP[5];//0x17C
} SYSCFG0_REGS;
#define SYSCFG0_ADDR 0x01C14000u
#define pSYSCFG0     ((SYSCFG0_REGS *)SYSCFG0_ADDR)
#define pSYSCFG0_VA  ((SYSCFG0_REGS *)(IO_ADDRESS(SYSCFG0_ADDR)))

typedef struct
{
    VUint32 VTPIO_CTL;//0x00
    VUint32 DDR_SLEW;//0x04
    VUint32 DEEPSLEEP;//0x08
    VUint32 PUPD_ENA;//0x0C
    VUint32 PUPD_SEL;//0x10
    VUint32 RXACTIVE;//0x14
    VUint32 PWRDN;//0x18
} SYSCFG1_REGS;
#define pSYSCFG1 ((SYSCFG1_REGS *)0x01E2C000u)

//----------------------------------------------------------------------------------------------------
// PLL Register structure - See sprufk4.pdf, Chapter 7 for more details.
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 PID;// 0x00
    VUint8  RSVD0[224];// 0x04
    VUint32 RSTYPE;// 0xE4
    VUint8  RSVD1[24]; // 0xE8
    VUint32 PLLCTL;// 0x100
    VUint32 OCSEL;// 0x104
    VUint8  RSVD2[8]; // 0x108
    VUint32 PLLM;// 0x110
    VUint32 PREDIV;// 0x114
    VUint32 PLLDIV1;// 0x118
    VUint32 PLLDIV2;// 0x11C
    VUint32 PLLDIV3;// 0x120
    VUint32 OSCDIV;// 0x124
    VUint32 POSTDIV;// 0x128
    VUint8  RSVD5[12];// 0x12C
    VUint32 PLLCMD;// 0x138
    VUint32 PLLSTAT;// 0x13C
    VUint32 ALNCTL;// 0x140
    VUint32 DCHANGE;// 0x144
    VUint32 CKEN;// 0x148
    VUint32 CKSTAT;// 0x14C
    VUint32 SYSTAT;// 0x150
    VUint8  RSVD6[12];// 0x154
    VUint32 PLLDIV4;// 0x160
    VUint32 PLLDIV5;// 0x164
    VUint32 PLLDIV6;// 0x168
    VUint32 PLLDIV7;// 0x16C
} PLL0_REGS;
#define pPLL0 ((PLL0_REGS *)0x01C11000u)

typedef struct
{
    VUint32 PID;// 0x00
    VUint8  RSVD0[252];// 0x04
    VUint32 PLLCTL;// 0x100
    VUint32 OCSEL;// 0x104
    VUint8  RSVD1[8]; // 0x108
    VUint32 PLLM;// 0x110
    VUint8  RSVD2[4];// 0x114
    VUint32 PLLDIV1;// 0x118
    VUint32 PLLDIV2;// 0x11C
    VUint32 PLLDIV3;// 0x120
    VUint32 OSCDIV;// 0x124
    VUint32 POSTDIV;// 0x128
    VUint8  RSVD3[12];// 0x12C
    VUint32 PLLCMD;// 0x138
    VUint32 PLLSTAT;// 0x13C
    VUint32 ALNCTL;// 0x140
    VUint32 DCHANGE;// 0x144
    VUint32 CKEN;// 0x148
    VUint32 CKSTAT;// 0x14C
    VUint32 SYSTAT;// 0x150
} PLL1_REGS;
#define pPLL1 ((PLL1_REGS *)0x01E1A000u)

//----------------------------------------------------------------------------------------------------
// Power/Sleep Ctrl Register structure - See sprufk4.pdf, Chapter 7
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 PID;// 0x000
    VUint8  RSVD0[16];// 0x004
    VUint8  RSVD1[4];// 0x014
    VUint32 INTEVAL;// 0x018
    VUint8  RSVD2[36];// 0x01C
    VUint32 MERRPR0;// 0x040
    VUint32 MERRPR1;// 0x044
    VUint8  RSVD3[8];// 0x048
    VUint32 MERRCR0;// 0x050
    VUint32 MERRCR1;// 0x054
    VUint8  RSVD4[8];// 0x058
    VUint32 PERRPR;// 0x060
    VUint8  RSVD5[4];// 0x064
    VUint32 PERRCR;// 0x068
    VUint8  RSVD6[4];// 0x06C
    VUint32 EPCPR;// 0x070
    VUint8  RSVD7[4];// 0x074
    VUint32 EPCCR;// 0x078
    VUint8  RSVD8[144];// 0x07C
    VUint8  RSVD9[20];// 0x10C
    VUint32 PTCMD;// 0x120
    VUint8  RSVD10[4];// 0x124
    VUint32 PTSTAT;// 0x128
    VUint8  RSVD11[212];// 0x12C
    VUint32 PDSTAT0;// 0x200
    VUint32 PDSTAT1;// 0x204
    VUint8  RSVD12[248];// 0x208
    VUint32 PDCTL0;// 0x300
    VUint32 PDCTL1;// 0x304
    VUint8  RSVD13[248];// 0x308
    VUint32 MCKOUT0;// 0x400
    VUint32 MCKOUT1;// 0x404
    VUint8  RSVD14[1016];// 0x408
    VUint32 MDSTAT[41];// 0x800
    VUint8  RSVD15[348];// 0x8A4
    VUint32 MDCTL[41];// 0xA00
} PSC_REGS;
#define PSC0_ADDR 0x01C10000u
#define pPSC0      ((PSC_REGS *)PSC0_ADDR)
#define pPSC0_VA   ((PSC_REGS *)(IO_ADDRESS(PSC0_ADDR)))
#define PSC1_ADDR 0x01E27000u
#define pPSC1      ((PSC_REGS *)PSC1_ADDR)
#define pPSC1_VA   ((PSC_REGS *)(IO_ADDRESS(PSC1_ADDR)))

//----------------------------------------------------------------------------------------------------
// EMIFA Register structure - From EMIF 2.5 Spec
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 PID;// 0x00
    VUint32 AWCC;// 0x04
    VUint32 SDCR;// 0x08
    VUint32 SDRCR;// 0x0C

    VUint32 CE2CFG;// 0x10
    VUint32 CE3CFG;// 0x14
    VUint32 CE4CFG;// 0x18
    VUint32 CE5CFG;// 0x1C

    VUint32 SDTIMR;// 0x20
    VUint32 SDRSTAT;// 0x24
    VUint32 DDRPHYCR;// 0x28
    VUint32 DDRPHYSR;// 0x2C

    VUint32 SDRACCR;// 0x30
    VUint32 SDRACT;// 0x34
    VUint32 DDRPHYREV;// 0x38
    VUint32 SDSRETR;// 0x3C

    VUint32 INTRAW;// 0x40
    VUint32 INTMSK;// 0x44
    VUint32 INTMSKSET;// 0x48
    VUint32 INTMSKCLR;// 0x4C

    VUint32 IOCR;// 0x50
    VUint32 IOSR;// 0x54
    VUint8  RSVD0[4];// 0x58
    VUint32 ONENANDCTL;// 0x5C

    VUint32 NANDFCR;// 0x60
    VUint32 NANDFSR;// 0x64
    VUint32 PMCR;// 0x68
    VUint8  RSVD1[4];// 0x6C

    VUint32 NANDF1ECC;// 0x70
    VUint32 NANDF2ECC;// 0x74
    VUint32 NANDF3ECC;// 0x78
    VUint32 NANDF4ECC;// 0x7C

    VUint8  RSVD2[4];// 0x80
    VUint32 IODFTEXECNT;// 0x84
    VUint32 IODFTGBLCTRL;// 0x88
    VUint8  RSVD3[4];// 0x8C

    VUint32 IODFTMISRLSB;// 0x90
    VUint32 IODFTMISRMID;// 0x94
    VUint32 IODFTMISRMSB;// 0x98
    VUint8  RSVD4[20];// 0x9C

    VUint32 MODRELNUM;// 0xB0
    VUint8  RSVD5[8];// 0xB4
    VUint32 NAND4BITECCLOAD;// 0xBC

    VUint32 NAND4BITECC1;// 0xC0
    VUint32 NAND4BITECC2;// 0xC4
    VUint32 NAND4BITECC3;// 0xC8
    VUint32 NAND4BITECC4;// 0xCC

    VUint32 NANDERRADD1;// 0xD0
    VUint32 NANDERRADD2;// 0xD4
    VUint32 NANDERRVAL1;// 0xD8
    VUint32 NANDERRVAL2;// 0xDC
} EMIFA_REGS;
#define pEMIFA ((EMIFA_REGS *)0x68000000u)

//=======================================================================================
// DDR2 Memory Ctrl Register structure - See sprueh7d.pdf for more details.
//=======================================================================================
typedef struct _DEVICE_DDR2_REGS_
{
    VUint8  RSVD0[4];// 0x00
    VUint32 SDRSTAT;// 0x04
    VUint32 SDCR;// 0x08
    VUint32 SDRCR;// 0x0C
    VUint32 SDTIMR;// 0x10
    VUint32 SDTIMR2;// 0x14
    VUint8  RSVD1[4];// 0x18
    VUint32 SDCR2;// 0x1C
    VUint32 PBBPR;// 0x20
    VUint8  RSVD2[156];// 0x24
    VUint32 IRR;// 0xC0
    VUint32 IMR;// 0xC4
    VUint32 IMSR;// 0xC8
    VUint32 IMCR;// 0xCC
    VUint8  RSVD3[20];// 0xD0
    VUint32 DDRPHYCR;// 0xE4
    VUint8  RSVD4[8];// 0xE8
} DDR_REGS;
#define pDDR ((DDR_REGS *)0xB0000000u)
#define pVTPIO_CTL  ((VUint32 *)(0x01e2c000u))
#define pDDR_SLEW   ((VUint32 *)(0x01E2C004u))

//----------------------------------------------------------------------------------------------------
// GPIO Register structure
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 REVID;// 0x00
    VUint8  RSVD0[4];// 0x04
    VUint32 BINTEN;// 0x08
    VUint8  RSVD1[4];// 0x0C

    VUint32 DIR01;// 0x10
    VUint32 OUT_DATA01;// 0x14
    VUint32 SET_DATA01;// 0x18
    VUint32 CLR_DATA01;// 0x1C
    VUint32 IN_DATA01;// 0x20
    VUint32 SET_RIS_TRIG01;// 0x24
    VUint32 CLR_RIS_TRIG01;// 0x28
    VUint32 SET_FAL_TRIG01;// 0x2C
    VUint32 CLR_FAL_TRIG01;// 0x30
    VUint32 INTSTAT01;// 0x34

    VUint32 DIR23;// 0x38
    VUint32 OUT_DATA23;// 0x3C
    VUint32 SET_DATA23;// 0x40
    VUint32 CLR_DATA23;// 0x44
    VUint32 IN_DATA23;// 0x48
    VUint32 SET_RIS_TRIG23;// 0x4C
    VUint32 CLR_RIS_TRIG23;// 0x50
    VUint32 SET_FAL_TRIG23;// 0x54
    VUint32 CLR_FAL_TRIG23;// 0x58
    VUint32 INTSTAT23;// 0x5C

    VUint32 DIR45;// 0x60
    VUint32 OUT_DATA45;// 0x64
    VUint32 SET_DATA45;// 0x68
    VUint32 CLR_DATA45;// 0x6C
    VUint32 IN_DATA45;// 0x70
    VUint32 SET_RIS_TRIG45;// 0x74
    VUint32 CLR_RIS_TRIG45;// 0x78
    VUint32 SET_FAL_TRIG45;// 0x7C
    VUint32 CLR_FAL_TRIG45;// 0x80
    VUint32 INTSTAT45;// 0x84

    VUint32 DIR67;// 0x88
    VUint32 OUT_DATA67;// 0x8C
    VUint32 SET_DATA67;// 0x90
    VUint32 CLR_DATA67;// 0x94
    VUint32 IN_DATA67;// 0x98
    VUint32 SET_RIS_TRIG67;// 0x9C
    VUint32 CLR_RIS_TRIG67;// 0xA0
    VUint32 SET_FAL_TRIG67;// 0xA4
    VUint32 CLR_FAL_TRIG67;// 0xA8
    VUint32 INTSTAT67;// 0xAC

    VUint32 DIR8;// 0xB0
    VUint32 OUT_DATA8;// 0xB4
    VUint32 SET_DATA8;// 0xB8
    VUint32 CLR_DATA8;// 0xBC
    VUint32 IN_DATA8;// 0xC0
    VUint32 SET_RIS_TRIG8;// 0xC4
    VUint32 CLR_RIS_TRIG8;// 0xC8
    VUint32 SET_FAL_TRIG8;// 0xCC
    VUint32 CLR_FAL_TRIG8;// 0xD0
    VUint32 INTSTAT8;// 0xD4
} GPIO_REGS;
#define GPIO_ADDR 0x01E26000u
#define pGPIO     ((GPIO_REGS *)GPIO_ADDR)
#define pGPIO_VA  ((GPIO_REGS *)(IO_ADDRESS(GPIO_ADDR)))

//----------------------------------------------------------------------------------------------------
// UART Register structure - See sprufm6c.pdf for more details.
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 RBR;// 0x00
    VUint32 IER;// 0x04
    VUint32 IIR;// 0x08
    VUint32 LCR;// 0x0C
    VUint32 MCR;// 0x10
    VUint32 LSR;// 0x14
    VUint32 MSR;// 0x18
    VUint32 SSR;// 0x1C
    VUint8  DLL;// 0x20
    VUint8  RSVD1[3];// 0x21
    VUint8  DLH;// 0x24
    VUint8  RSVD2[3];// 0x25
    VUint32 PID1;// 0x28
    VUint32 PID2;// 0x2C
    VUint32 PWREMU_MGNT;// 0x30
    VUint32 MDR;// 0x34
} UART_REGS;

#define THR RBR
#define FCR IIR
#define pUART0 ((UART_REGS *)0x01C42000u)
#define pUART1 ((UART_REGS *)0x01D0C000u)
#define pUART2 ((UART_REGS *)0x01D0D000u)

//----------------------------------------------------------------------------------------------------
// Timer Register structure - See spruee5a.pdf for more details.
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 PID;// 0x00
    VUint32 EMUMGT;// 0x04
    VUint32 GPINTGPEN;// 0x08
    VUint32 GPDATGPDIR;// 0x0C
    VUint32 TIM12;// 0x10
    VUint32 TIM34;// 0x14
    VUint32 PRD12;// 0x18
    VUint32 PRD34;// 0x1C
    VUint32 TCR;// 0x20
    VUint32 TGCR;// 0x24
    VUint32 WDTCR;// 0x28
    VUint8  RSVD1[8];// 0x2C
    VUint32 REL12;// 0x34
    VUint32 REL34;// 0x38
    VUint32 CAP12;// 0x3C
    VUint32 CAP34;// 0x40
    VUint32 INTCTLSTAT;// 0x44
    VUint8  RSVD2[24];// 0x48
    VUint32 CMP[8];// 0x60
} TIMER_REGS;
#define pTIMER0 ((TIMER_REGS *)0x01C20000u)
#define pTIMER1 ((TIMER_REGS *)0x01C21000u)
#define pTIMER2 ((TIMER_REGS *)0x01F0C000u)
#define pTIMER3 ((TIMER_REGS *)0x01F0D000u)

//----------------------------------------------------------------------------------------------------
// Spi Register structure
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 SPIGCR0;// 0x00
    VUint32 SPIGCR1;// 0x04
    VUint32 SPIINT;// 0x08
    VUint32 SPILVL;// 0x0C
    VUint32 SPIFLG;// 0x10
    VUint32 SPIPC0;// 0x14
    VUint32 SPIPC1;// 0x18
    VUint32 SPIPC2;// 0x1C
    VUint32 SPIPC3;// 0x20
    VUint32 SPIPC4;// 0x24
    VUint32 SPIPC5;// 0x28
    VUint32 SPIPC6;// 0x2C
    VUint32 SPIPC7;// 0x30
    VUint32 SPIPC8;// 0x34
    VUint32 SPIDAT0;// 0x38
    VUint32 SPIDAT1;// 0x3C
    VUint32 SPIBUF;// 0x40
    VUint32 SPIEMU;// 0x44
    VUint32 SPIDELAY;// 0x48
    VUint32 SPIDEF;// 0x4C
    VUint32 SPIFMT0;// 0x50
    VUint32 SPIFMT1;// 0x54
    VUint32 SPIFMT2;// 0x58
    VUint32 SPIFMT3;// 0x5C
    VUint8  RSVD0[4];// 0x60
    VUint32 INTVEC1;// 0x64
    VUint32 RSVD1[2];// 0x68
    VUint32 MIBSPIE;// 0x70
} SPI_REGS;
#define pSPI0 ((SPI_REGS *)0x01C41000u)
#define pSPI1 ((SPI_REGS *)0x01F0E000u)

//----------------------------------------------------------------------------------------------------
// Emac Register structure
//----------------------------------------------------------------------------------------------------
typedef struct EMAC_DESCRIPTOR_REGS
{
    struct  EMAC_DESCRIPTOR_REGS *next;// Pointer to next descriptor in chain
    VUint8  const *buffer;// Pointer to data buffer
    VUint32 buffer_offset_and_length;// Buffer Offset(MSW) | Buffer Length(LSW)
    VUint32 flags_and_packet_length;//Flags(MSW) | Packet Length(LSW)
} EMAC_DESCRIPTOR_REGS;

typedef struct
{
    VUint32 TXREVID;// 0x00
    VUint32 TXCONTROL;// 0x04
    VUint32 TXTEARDOWN;// 0x08
    VUint8  RSVD0[4];// 0x0C
    VUint32 RXREVID;// 0x10
    VUint32 RXCONTROL;// 0x14
    VUint32 RXTEARDOWN;// 0x18
    VUint8  RSVD1[100];// 0x1C
    VUint32 TXINTSTATRAW;// 0x80
    VUint32 TXINTSTATMASKED;// 0x84
    VUint32 TXINTMASKSET;// 0x88
    VUint32 TXINTMASKCLEAR;// 0x8C
    VUint32 MACINVECTOR;// 0x90
    VUint32 MACEOIVECTOR;// 0x94
    VUint8  RSVD2[8];// 0x98
    VUint32 RXINTSTATRAW;// 0xA0
    VUint32 RXINTSTATMASKED;// 0xA4
    VUint32 RXINTMASKSET;// 0xA8
    VUint32 RXINTMASKCLEAR;// 0xAC
    VUint32 MACINTSTATRAW;// 0xB0
    VUint32 MACINTSTATMASKED;// 0xB4
    VUint32 MACINTMASKSET;// 0xB8
    VUint32 MACINTMASKCLEAR;// 0xBC
    VUint8  RSVD3[64];// 0xC0
    VUint32 RXMBPENABLE;// 0x100 Control
    VUint32 RXUNICASTSET;// 0x104
    VUint32 RXUNICASTCLEAR;// 0x108
    VUint32 RXMAXLEN;// 0x10C Control
    VUint32 RXBUFFEROFFSET;// 0x110
    VUint32 RXFILTERLOWTHRESH;// 0x114 Control
    VUint8  RSVD4[8];// 0x118
    VUint32 RX0FLOWTHRESH;// 0x120 Control
    VUint32 RX1FLOWTHRESH;// 0x124
    VUint32 RX2FLOWTHRESH;// 0x128
    VUint32 RX3FLOWTHRESH;// 0x12C
    VUint32 RX4FLOWTHRESH;// 0x130
    VUint32 RX5FLOWTHRESH;// 0x134
    VUint32 RX6FLOWTHRESH;// 0x138
    VUint32 RX7FLOWTHRESH;// 0x13C
    VUint32 RX0FREEBUFFER;// 0x140
    VUint32 RX1FREEBUFFER;// 0x144
    VUint32 RX2FREEBUFFER;// 0x148
    VUint32 RX3FREEBUFFER;// 0x14C
    VUint32 RX4FREEBUFFER;// 0x150
    VUint32 RX5FREEBUFFER;// 0x154
    VUint32 RX6FREEBUFFER;// 0x158
    VUint32 RX7FREEBUFFER;// 0x15C
    VUint32 MACCONTROL;// 0x160 Control
    VUint32 MACSTATUS;// 0x164
    VUint32 EMCONTROL;// 0x168
    VUint32 FIFOCONTROL;// 0x16C Control
    VUint32 MACCONFIG;// 0x170
    VUint32 SOFTRESET;// 0x174 Control
    VUint8  RSVD5[88];// 0x178
    VUint32 MACSRCADDRLO;// 0x1D0 Control
    VUint32 MACSRCADDRHI;// 0x1D4 Control
    VUint32 MACHASH1;// 0x1D8 Control Mac Hash
    VUint32 MACHASH2;// 0x1DC Control Mac Hash
    VUint32 BOFFTEST;// 0x1E0
    VUint32 TPACETEST;// 0x1E4
    VUint32 RXPAUSE;// 0x1E8
    VUint32 TXPAUSE;// 0x1EC
    VUint8  RSVD6[16];// 0x1F0
    VUint32 RXGOODFRAMES;// 0x200
    VUint32 RXBCASTFRAMES;// 0x204
    VUint32 RXMCASTFRAMES;// 0x208
    VUint32 RXPAUSEFRAMES;// 0x20C
    VUint32 RXCRCERRORS;// 0x210
    VUint32 RXALIGNCODEERRORS;// 0x214
    VUint32 RXOVERSIZED;// 0x218
    VUint32 RXJABBER;// 0x21C
    VUint32 RXUNDERSIZED;// 0x220
    VUint32 RXFRAGMENTS;// 0x224
    VUint32 RXFILTERED;// 0x228
    VUint32 RXQOSFILTERED;// 0x22C
    VUint32 RXOCTETS;// 0x230
    VUint32 TXGOODFRAMES;// 0x234
    VUint32 TXBCASTFRAMES;// 0x238
    VUint32 TXMCASTFRAMES;// 0x23C
    VUint32 TXPAUSEFRAMES;// 0x240
    VUint32 TXDEFERRED;// 0x244
    VUint32 TXCOLLISION;// 0x248
    VUint32 TXSINGLECOLL;// 0x24C
    VUint32 TXMULTICOLL;// 0x250
    VUint32 TXEXCESSIVECOLL;// 0x254
    VUint32 TXLATECOLL;// 0x258
    VUint32 TXUNDERRUN;// 0x25C
    VUint32 TXCARRIERSENSE;// 0x260
    VUint32 TXOCTETS;// 0x264
    VUint32 FRAME64;// 0x268
    VUint32 FRAME65T127;// 0x26C
    VUint32 FRAME128T255;// 0x270
    VUint32 FRAME256T511;// 0x274
    VUint32 FRAME512T1023;// 0x278
    VUint32 FRAME1024TUP;// 0x27C
    VUint32 NETOCTETS;// 0x280
    VUint32 RXSOFOVERRUNS;// 0x284
    VUint32 RXMOFOVERRUNS;// 0x288
    VUint32 RXDMAOVERRUNS;// 0x28C
    VUint8  RSVD7[624];// 0x290
    VUint32 MACADDRLO;// 0x500 Control
    VUint32 MACADDRHI;// 0x504 Control
    VUint32 MACINDEX;// 0x508 Control
    VUint8  RSVD8[244];// 0x50C
    EMAC_DESCRIPTOR_REGS* TX0HDP;// 0x600 Control
    EMAC_DESCRIPTOR_REGS* TX1HDP;// 0x604
    EMAC_DESCRIPTOR_REGS* TX2HDP;// 0x608
    EMAC_DESCRIPTOR_REGS* TX3HDP;// 0x60C
    EMAC_DESCRIPTOR_REGS* TX4HDP;// 0x610
    EMAC_DESCRIPTOR_REGS* TX5HDP;// 0x614
    EMAC_DESCRIPTOR_REGS* TX6HDP;// 0x618
    EMAC_DESCRIPTOR_REGS* TX7HDP;// 0x61C
    EMAC_DESCRIPTOR_REGS* RX0HDP;// 0x620 Control
    EMAC_DESCRIPTOR_REGS* RX1HDP;// 0x624
    EMAC_DESCRIPTOR_REGS* RX2HDP;// 0x628
    EMAC_DESCRIPTOR_REGS* RX3HDP;// 0x62C
    EMAC_DESCRIPTOR_REGS* RX4HDP;// 0x630
    EMAC_DESCRIPTOR_REGS* RX5HDP;// 0x634
    EMAC_DESCRIPTOR_REGS* RX6HDP;// 0x638
    EMAC_DESCRIPTOR_REGS* RX7HDP;// 0x63C
    EMAC_DESCRIPTOR_REGS* TX0CP;// 0x640 Control
    EMAC_DESCRIPTOR_REGS* TX1CP;// 0x644
    EMAC_DESCRIPTOR_REGS* TX2CP;// 0x648
    EMAC_DESCRIPTOR_REGS* TX3CP;// 0x64C
    EMAC_DESCRIPTOR_REGS* TX4CP;// 0x650
    EMAC_DESCRIPTOR_REGS* TX5CP;// 0x654
    EMAC_DESCRIPTOR_REGS* TX6CP;// 0x658
    EMAC_DESCRIPTOR_REGS* TX7CP;// 0x65C
    EMAC_DESCRIPTOR_REGS* RX0CP;// 0x660 Control
    EMAC_DESCRIPTOR_REGS* RX1CP;// 0x664
    EMAC_DESCRIPTOR_REGS* RX2CP;// 0x668
    EMAC_DESCRIPTOR_REGS* RX3CP;// 0x66C
    EMAC_DESCRIPTOR_REGS* RX4CP;// 0x670
    EMAC_DESCRIPTOR_REGS* RX5CP;// 0x674
    EMAC_DESCRIPTOR_REGS* RX6CP;// 0x678
    EMAC_DESCRIPTOR_REGS* RX7CP;// 0x67C
} EMAC_REGS;

typedef struct
{
    EMAC_DESCRIPTOR_REGS DESCRIPTOR[512];
    VUint32 REVID;// 0x00
    VUint32 SOFTRESET;// 0x04
    VUint8  RSVD0[4];// 0x08
    VUint32 INTCONTROL;// 0x0C
    VUint32 C0RXTHRESHEN;// 0x10
    VUint32 C0RXEN;// 0x14
    VUint32 C0TXEN;// 0x18
    VUint32 C0MISCEN;// 0x1C
    VUint32 C1RXTHRESHEN;// 0x20
    VUint32 C1RXEN;// 0x24
    VUint32 C1TXEN;// 0x28
    VUint32 C1MISCEN;// 0x2C
    VUint32 C2RXTHRESHEN;// 0x30
    VUint32 C2RXEN;// 0x34
    VUint32 C2TXEN;// 0x38
    VUint32 C2MISCEN;// 0x3C
    VUint32 C0RXTHRESHSTAT;// 0x40
    VUint32 C0RXSTAT;// 0x44
    VUint32 C0TXSTAT;// 0x48
    VUint32 C0MISCSTAT;// 0x4C
    VUint32 C1RXTHRESHSTAT;// 0x50
    VUint32 C1RXSTAT;// 0x54
    VUint32 C1TXSTAT;// 0x58
    VUint32 C1MISCSTAT;// 0x5C
    VUint32 C2RXTHRESHSTAT;// 0x60
    VUint32 C2RXSTAT;// 0x64
    VUint32 C2TXSTAT;// 0x68
    VUint32 C2MISCSTAT;// 0x6C
    VUint32 C0RXIMAX;// 0x70
    VUint32 C0TXIMAX;// 0x74
    VUint32 C1RXIMAX;// 0x78
    VUint32 C1TXIMAX;// 0x7C
    VUint32 C2RXIMAX;// 0x80
    VUint32 C2TXIMAX;// 0x84
} EMAC_CONTROL_REGS;

typedef struct
{
    VUint32 REVID;// 0x00
    VUint32 CONTROL;// 0x04
    VUint32 ALIVE;// 0x08
    VUint32 LINK;// 0x0C
    VUint32 LINKINTRAW;// 0x10
    VUint32 LINKINTMASKED;// 0x14
    VUint8  RSVD0[8];// 0x18
    VUint32 USERINTRAW;// 0x20
    VUint32 USERINTMASKED;// 0x24
    VUint32 USERINTMASKSET;// 0x28
    VUint32 USERINTMASKCLEAR;// 0x2C
    VUint8  RSVD1[80];// 0x30
    VUint32 USERACCESS0;// 0x80
    VUint32 USERPHYSEL0;// 0x84
    VUint32 USERACCESS1;// 0x88
    VUint32 USERPHYSEL1;// 0x8C
} MDIO_REGS;
#define pEMAC_CONTROL  ((EMAC_CONTROL_REGS *)0x01E20000u)
#define pEMAC          ((EMAC_REGS *)0x01E23000u)
#define pMDIO          ((MDIO_REGS *)0x01E24000u)

//----------------------------------------------------------------------------------------------------
// EDMA Register structure
//----------------------------------------------------------------------------------------------------
// Register Overlay Structure for DRA
typedef struct
{
    VUint32 DRAE;// 0x00
    VUint8  RSVD6[4];// 0x04
} EDMA3CC_DRA_REGS;

// Register Overlay Structure for QUEUE
typedef struct
{
    VUint32 EVENT[16];// 0x00
} EDMA3CC_QUEUE_REGS;

// Register Overlay Structure for SHADOW
typedef struct
{
    VUint32 ER;// 0x00
    VUint8  RSVD0[4];// 0x04
    VUint32 ECR;// 0x08
    VUint8  RSVD1[4];// 0x0C
    VUint32 ESR;// 0x10
    VUint8  RSVD2[4];// 0x14
    VUint32 CER;// 0x18
    VUint8  RSVD3[4];// 0x1C
    VUint32 EER;// 0x20
    VUint8  RSVD4[4];// 0x24
    VUint32 EECR;// 0x28
    VUint8  RSVD5[4];// 0x2C
    VUint32 EESR;// 0x30
    VUint8  RSVD6[4];// 0x34
    VUint32 SER;// 0x38
    VUint8  RSVD7[4];// 0x3C
    VUint32 SECR;// 0x40
    VUint8  RSVD8[12];// 0x44
    VUint32 IER;// 0x50
    VUint8  RSVD9[4];// 0x54
    VUint32 IECR;// 0x58
    VUint8  RSVD10[4];// 0x5C
    VUint32 IESR;// 0x60
    VUint8  RSVD11[4];// 0x64
    VUint32 IPR;// 0x68
    VUint8  RSVD12[4];// 0x6C
    VUint32 ICR;// 0x70
    VUint8  RSVD13[4];// 0x74
    VUint32 IEVAL;// 0x78
    VUint8  RSVD14[4];// 0x7C
    VUint32 QER;// 0x80
    VUint32 QEER;// 0x84
    VUint32 QEECR;// 0x88
    VUint32 QEESR;// 0x8C
    VUint32 QSER;// 0x90
    VUint32 QSECR;// 0x94
    VUint8  RSVD15[360];// 0x98
} EDMA3CC_SHADOW_REGS;

// Register Overlay Structure for PARAMENTRY
typedef struct
{
    VUint32 OPT;// 0x00
    VUint32 SRC;// 0x04
    VUint32 A_B_CNT;// 0x08
    VUint32 DST;// 0x0C
    VUint32 SRC_DST_BIDX;// 0x10
    VUint32 LINK_BCNTRLD;// 0x14
    VUint32 SRC_DST_CIDX;// 0x18
    VUint32 CCNT;// 0x1C
} EDMA3CC_PARAM_SET_REGS;

// Register Overlay Structure
typedef struct
{
    VUint32 REVID;// 0x00
    VUint32 CCCFG;// 0x04
    VUint8  RSVD0[504];// 0x08
    VUint32 QCHMAP[8];// 0x200
    VUint8  RSVD1[32];// 0x220
    VUint32 DMAQNUM[8];// 0x240
    VUint32 QDMAQNUM;// 0x260
    VUint8  RSVD2[32];// 0x264
    VUint32 QUEPRI;// 0x284
    VUint8  RSVD3[120];// 0x288
    VUint32 EMR;// 0x300
    VUint8  RSVD4[4];// 0x304
    VUint32 EMCR;// 0x308
    VUint8  RSVD5[4];// 0x30C
    VUint32 QEMR;// 0x310
    VUint32 QEMCR;// 0x314
    VUint32 CCERR;// 0x318
    VUint32 CCERRCLR;// 0x31C
    VUint32 EEVAL;// 0x320
    VUint8  RSVD7[28];// 0x324
    EDMA3CC_DRA_REGS DRA[4];// 0x340
    VUint8  RSVD8[32];// 0x360
    VUint32 QRAE[4];// 0x380
    VUint8  RSVD9[112];// 0x390
    EDMA3CC_QUEUE_REGS QUEUE[2];// 0x400
    VUint8  RSVD10[384];// 0x480
    VUint32 QSTAT[3];// 0x600
    VUint8  RSVD11[20];// 0x60C
    VUint32 QWMTHRA;// 0x620
    VUint8  RSVD12[28];// 0x624
    VUint32 CCSTAT;// 0x640
    VUint8  RSVD13[2492];// 0x644
    VUint32 ER;// 0x1000
    VUint8  RSVD14[4];// 0x1004
    VUint32 ECR;// 0x1008
    VUint8  RSVD15[4];// 0x100C
    VUint32 ESR;// 0x1010
    VUint8  RSVD16[4];// 0x1014
    VUint32 CER;// 0x1018
    VUint8  RSVD17[4];// 0x101C
    VUint32 EER;// 0x1020
    VUint8  RSVD18[4];// 0x1024
    VUint32 EECR;// 0x1028
    VUint8  RSVD19[4];// 0x102C
    VUint32 EESR;// 0x1030
    VUint8  RSVD20[4];// 0x1034
    VUint32 SER;// 0x1038
    VUint8  RSVD21[4];// 0x103C
    VUint32 SECR;// 0x1040
    VUint8  RSVD22[12];// 0x1044
    VUint32 IER;// 0x1050
    VUint8  RSVD23[4];// 0x1054
    VUint32 IECR;// 0x1058
    VUint8  RSVD24[4];// 0x105C
    VUint32 IESR;// 0x1060
    VUint8  RSVD25[4];// 0x1064
    VUint32 IPR;// 0x1068
    VUint8  RSVD26[4];// 0x106C
    VUint32 ICR;// 0x1070
    VUint8  RSVD27[4];// 0x1074
    VUint32 IEVAL;// 0x1078
    VUint8  RSVD28[4];// 0x107C
    VUint32 QER;// 0x1080
    VUint32 QEER;// 0x1084
    VUint32 QEECR;// 0x1088
    VUint32 QEESR;// 0x108C
    VUint32 QSER;// 0x1090
    VUint32 QSECR;// 0x1094
    VUint8  RSVD30[3944];// 0x1098
    EDMA3CC_SHADOW_REGS SHADOW[4];// 0x2000
    VUint8  RSVD31[6144];// 0x2800
    EDMA3CC_PARAM_SET_REGS PARAMSET[128];// 0x4000
} EDMA3CC_REGS;

// Register Overlay Structure for DFIREG
typedef struct
{
    VUint32 DFOPT;// 0x00
    VUint32 DFSRC;// 0x04
    VUint32 DFCNT;// 0x08
    VUint32 DFDST;// 0x0C
    VUint32 DFBIDX;// 0x10
    VUint32 DFMPPRXY;// 0x14
    VUint8  RSVD5[40];// 0x18
} EDMA3TC_DFIREG_REGS;

// Register Overlay Structure
typedef struct
{
    VUint32 REVID;// 0x00
    VUint32 TCCFG;// 0x04
    VUint8  RSVD0[248];// 0x08
    VUint32 TCSTAT;// 0x100
    VUint8  RSVD1[28];// 0x104
    VUint32 ERRSTAT;// 0x120
    VUint32 ERREN;// 0x124
    VUint32 ERRCLR;// 0x128
    VUint32 ERRDET;// 0x12C
    VUint32 ERRCMD;// 0x130
    VUint8  RSVD2[12];// 0x134
    VUint32 RDRATE;// 0x140
    VUint8  RSVD3[252];// 0x144
    VUint32 SAOPT;// 0x240
    VUint32 SASRC;// 0x244
    VUint32 SACNT;// 0x248
    VUint32 SADST;// 0x24C
    VUint32 SABIDX;// 0x250
    VUint32 SAMPPRXY;// 0x254
    VUint32 SACNTRLD;// 0x258
    VUint32 SASRCBREF;// 0x25C
    VUint32 SADSTBREF;// 0x260
    VUint8  RSVD4[28];// 0x264
    VUint32 DFCNTRLD;// 0x280
    VUint32 DFSRCBREF;// 0x284
    VUint32 DFDSTBREF;// 0x288
    VUint8  RSVD6[116];// 0x28C
    EDMA3TC_DFIREG_REGS DFIREG[4];// 0x300
} EDMA3TC_REGS;
#define pEDMA0CC   ((EDMA3CC_REGS *)0x01C00000u)
#define pEDMA0TC0  ((EDMA3TC_REGS *)0x01C08000u)
#define pEDMA0TC1  ((EDMA3TC_REGS *)0x01C08400u)
#define pEDMA1CC   ((EDMA3CC_REGS *)0x01E30000u)
#define pEDMA1TC0  ((EDMA3TC_REGS *)0x01E38000u)

//----------------------------------------------------------------------------------------------------
// Cache Register structure
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 L2CFG;
    VUint8  RSVD0[28];
    VUint32 L1PCFG;
    VUint32 L1PCC;
    VUint8  RSVD1[24];
    VUint32 L1DCFG;
    VUint32 L1DCC;
    VUint8  RSVD2[16312];
    VUint32 L2WBAR;
    VUint32 L2WWC;
    VUint8  RSVD3[8];
    VUint32 L2WIBAR;
    VUint32 L2WIWC;
    VUint32 L2IBAR;
    VUint32 L2IWC;
    VUint32 L1PIBAR;
    VUint32 L1PIWC;
    VUint8  RSVD4[8];
    VUint32 L1DWIBAR;
    VUint32 L1DWIWC;
    VUint8  RSVD5[8];
    VUint32 L1DWBAR;
    VUint32 L1DWWC;
    VUint32 L1DIBAR;
    VUint32 L1DIWC;
    VUint8  RSVD6[4016];
    VUint32 L2WB;
    VUint32 L2WBINV;
    VUint32 L2INV;
    VUint8  RSVD7[28];
    VUint32 L1PINV;
    VUint8  RSVD8[20];
    VUint32 L1DWB;
    VUint32 L1DWBINV;
    VUint32 L1DINV;
    VUint8  RSVD9[12212];
    VUint32 MAR[256];
} CACHE_REGS;
#define pCACHE    ((CACHE_REGS *)0x01840000u)

//----------------------------------------------------------------------------------------------------
// ARM Interrupt Control Register structure.
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 REVID;// 0x000
    VUint32 CR;// 0x004
    VUint8  RSVD0[8];// 0x008
    VUint32 GER;// 0x010
    VUint8  RSVD1[8];// 0x014
    VUint32 GNLR;// 0x01C
    VUint32 SISR;// 0x020
    VUint32 SICR;// 0x024
    VUint32 EISR;// 0x028
    VUint32 EICR;// 0x02C
    VUint8  RSVD2[4];// 0x030
    VUint32 HIEISR;// 0x034
    VUint32 HIEICR;// 0x038
    VUint8  RSVD3[20];// 0x03C
    VUint32 VBR;// 0x050
    VUint32 VSR;// 0x054
    VUint32 VNR;// 0x058
    VUint8  RSVD4[36];// 0x05C
    VUint32 GPIR;// 0x080
    VUint32 GPVR;// 0x084
    VUint8  RSVD5[376];// 0x088
    VUint32 SRSR[4];// 0x200
    VUint8  RSVD6[112];// 0x210
    VUint32 SECR[4];// 0x280
    VUint8  RSVD7[112];// 0x290
    VUint32 ESR[4];// 0x300
    VUint8  RSVD8[112];// 0x310
    VUint32 ECR[4];// 0x380
    VUint8  RSVD9[112];// 0x390
    VUint32 CMR[26];// 0x400
    VUint8  RSVD10[1176];// 0x468
    VUint32 HIPIR[2];// 0x900
    VUint8  RSVD11[2040];// 0x908
    VUint32 HINLR[2];// 0x1100
    VUint8  RSVD12[1016];// 0x1108
    VUint32 HIER;// 0x1500
    VUint8  RSVD13[252];// 0x1504
    VUint32 HIPVR[2];// 0x1600
} AINTC_REGS;
#define pAINTC  ((AINTC_REGS *)0xFFFEE000u)

//----------------------------------------------------------------------------------------------------
// PRU Register structure
//----------------------------------------------------------------------------------------------------
typedef struct
{
    VUint32 CONTROL;
    VUint32 STATUS;
    VUint32 WAKEUP;
    VUint32 CYCLECNT;
    VUint32 STALLCNT;
    VUint8  RSVD0[12];
    VUint32 CONTABBLKIDX0;
    VUint32 CONTABBLKIDX1;
    VUint32 CONTABPROPTR0;
    VUint32 CONTABPROPTR1;
    VUint8  RSVD1[976];
    VUint32 INTGPR0;
    VUint32 INTGPR1;
    VUint32 INTGPR2;
    VUint32 INTGPR3;
    VUint32 INTGPR4;
    VUint32 INTGPR5;
    VUint32 INTGPR6;
    VUint32 INTGPR7;
    VUint32 INTGPR8;
    VUint32 INTGPR9;
    VUint32 INTGPR10;
    VUint32 INTGPR11;
    VUint32 INTGPR12;
    VUint32 INTGPR13;
    VUint32 INTGPR14;
    VUint32 INTGPR15;
    VUint32 INTGPR16;
    VUint32 INTGPR17;
    VUint32 INTGPR18;
    VUint32 INTGPR19;
    VUint32 INTGPR20;
    VUint32 INTGPR21;
    VUint32 INTGPR22;
    VUint32 INTGPR23;
    VUint32 INTGPR24;
    VUint32 INTGPR25;
    VUint32 INTGPR26;
    VUint32 INTGPR27;
    VUint32 INTGPR28;
    VUint32 INTGPR29;
    VUint32 INTGPR30;
    VUint32 INTGPR31;
    VUint32 INTCTER0;
    VUint32 INTCTER1;
    VUint32 INTCTER2;
    VUint32 INTCTER3;
    VUint32 INTCTER4;
    VUint32 INTCTER5;
    VUint32 INTCTER6;
    VUint32 INTCTER7;
    VUint32 INTCTER8;
    VUint32 INTCTER9;
    VUint32 INTCTER10;
    VUint32 INTCTER11;
    VUint32 INTCTER12;
    VUint32 INTCTER13;
    VUint32 INTCTER14;
    VUint32 INTCTER15;
    VUint32 INTCTER16;
    VUint32 INTCTER17;
    VUint32 INTCTER18;
    VUint32 INTCTER19;
    VUint32 INTCTER20;
    VUint32 INTCTER21;
    VUint32 INTCTER22;
    VUint32 INTCTER23;
    VUint32 INTCTER24;
    VUint32 INTCTER25;
    VUint32 INTCTER26;
    VUint32 INTCTER27;
    VUint32 INTCTER28;
    VUint32 INTCTER29;
    VUint32 INTCTER30;
    VUint32 INTCTER31;
} PRU_REGS;

#define pPRU0                    ((PRU_REGS *)0x01C37000u)
#define PRU0_IRAM_BASEADDR       (0x01C38000u)

#define pPRU1                    ((PRU_REGS *)0x01C37800u)
#define PRU1_IRAM_BASEADDR       (0x01C3C000u)


#ifdef __cplusplus
}
#endif

#endif
