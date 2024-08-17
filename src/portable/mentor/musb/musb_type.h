/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

/******************************************************************************
*
* Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*  Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*
*  Redistributions in binary form must reproduce the above copyright
*  notice, this list of conditions and the following disclaimer in the
*  documentation and/or other materials provided with the
*  distribution.
*
*  Neither the name of Texas Instruments Incorporated nor the names of
*  its contributors may be used to endorse or promote products derived
*  from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*******************************************************************************/

#ifndef TUSB_MUSB_TYPE_H_
#define TUSB_MUSB_TYPE_H_

#include "stdint.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __IO
  #define __IO volatile
#endif

#ifndef __I
  #define __I  volatile const
#endif

#ifndef __O
  #define __O  volatile
#endif

#ifndef __R
  #define __R  volatile const
#endif

// 0: TX (device IN, host OUT)
// 1: RX (device OUT, host IN)
typedef struct TU_ATTR_PACKED {
  union {
    struct {
      __IO uint16_t tx_maxp;       // 0x00: TXMAXP
      union {
        __IO uint8_t csr0l;        // 0x02: CSR0
        __IO uint8_t tx_csrl;      // 0x02: TX CSRL
      };
      union {
        __IO uint8_t csr0h;        // 0x03: CSR0H
        __IO uint8_t tx_csrh;      // 0x03: TX CSRH
      };

      __IO uint16_t rx_maxp;       // 0x04: RX MAXP
      __IO uint8_t  rx_csrl;       // 0x06: RX CSRL
      __IO uint8_t  rx_csrh;       // 0x07: RX CSRH
    };

    struct {
      __IO uint16_t maxp;          // 0x00: MAXP
      __IO uint8_t  csrl;          // 0x02: CSRL
      __IO uint8_t  csrh;          // 0x03: CSRH
    }maxp_csr[2];
  };

  union {
    __IO uint16_t count0;      // 0x08: COUNT0
    __IO uint16_t rx_count;    // 0x08: RX COUNT
  };
  union {
    __IO uint8_t type0;        // 0x0A: TYPE0 (host only)
    __IO uint8_t tx_type;      // 0x0A: TX TYPE
  };
  __IO uint8_t tx_interval;    // 0x0B: TX INTERVAL
  __IO uint8_t rx_type;        // 0x0C: RX TYPE
  __IO uint8_t rx_interval;    // 0x0D: RX INTERVAL
  __IO uint8_t reserved_0x0e;  // 0x0E: Reserved
  union {
    __IO uint8_t config_data0; // 0x0F: CONFIG DATA
    struct {
      __IO uint8_t utmi_data_width : 1; // [0] UTMI Data Width
      __IO uint8_t softconn_en     : 1; // [1] Soft Connect Enable
      __IO uint8_t dynamic_fifo    : 1; // [2] Dynamic FIFO Sizing
      __IO uint8_t hb_tx_en        : 1; // [3] High Bandwidth TX ISO Enable
      __IO uint8_t hb_rx_en        : 1; // [4] High Bandwidth RX ISO Enable
      __IO uint8_t big_endian      : 1; // [5] Big Endian
      __IO uint8_t mp_tx_en        : 1; // [6] Auto splitting BULK TX Enable
      __IO uint8_t mp_rx_en        : 1; // [7] Auto amalgamation BULK RX Enable
    } config_data0_bit;

    __IO uint8_t fifo_size;    // 0x0F: FIFO_SIZE
    struct {
      __IO uint8_t tx : 4;  // [3:0] TX FIFO Size
      __IO uint8_t rx : 4;  // [7:4] RX FIFO Size
    }fifo_size_bit;
  };
} musb_ep_csr_t;

TU_VERIFY_STATIC(sizeof(musb_ep_csr_t) == 16, "size is not correct");

typedef struct TU_ATTR_PACKED {
  //------------- Common -------------//
  __IO uint8_t  faddr;             // 0x00: FADDR
  union {
    __IO uint8_t  power;             // 0x01: POWER
    struct {
      __IO uint8_t suspend_mode_en : 1; // [0] SUSPEND Mode Enable
      __IO uint8_t suspend_mode    : 1; // [1] SUSPEND Mode
      __IO uint8_t resume_mode     : 1; // [2] RESUME
      __IO uint8_t reset           : 1; // [3] RESET
      __IO uint8_t highspeed_mode  : 1; // [4] High Speed Mode
      __IO uint8_t highspeed_en    : 1; // [5] High Speed Enable
      __IO uint8_t soft_conn       : 1; // [6] Soft Connect/Disconnect
      __IO uint8_t iso_update      : 1; // [7] Isochronous Update
    } power_bit;
  };

  union {
    struct {
      __IO uint16_t intr_tx;           // 0x02: INTR_TX
      __IO uint16_t intr_rx;           // 0x04: INTR_RX
    };

    __IO uint16_t intr_ep[2];         // 0x02-0x05: INTR_EP0-1
  };

  union {
    struct {
      __IO uint16_t intr_txen;         // 0x06: INTR_TXEN
      __IO uint16_t intr_rxen;         // 0x08: INTR_RXEN
    };

    __IO uint16_t intren_ep[2];       // 0x06-0x09: INTREN_EP0-1
  };

  __IO uint8_t  intr_usb;          // 0x0A: INTRUSB
  __IO uint8_t  intr_usben;        // 0x0B: INTRUSBEN

  __IO uint16_t frame;             // 0x0C: FRAME
  __IO uint8_t  index;             // 0x0E: INDEX
  __IO uint8_t  testmode;          // 0x0F: TESTMODE

  //------------- Endpoint CSR (indexed) -------------//
  musb_ep_csr_t indexed_csr;       // 0x10-0x1F: Indexed CSR 0-15

  //------------- FIFOs -------------//
  __IO uint32_t fifo[16];          // 0x20-0x5C: FIFO 0-15

  // Common (2)
  __IO uint8_t  devctl;            // 0x60: DEVCTL
  __IO uint8_t  misc;              // 0x61: MISC

  //------------- Dynammic FIFO (indexed) -------------//
  union {
    struct {
      __IO uint8_t  txfifo_sz;         // 0x62: TXFIFO_SZ
      __IO uint8_t  rxfifo_sz;         // 0x63: RXFIFO_SZ
    };
    __IO uint8_t fifo_size[2];
  };

  union {
    struct {
      __IO uint16_t txfifo_addr;       // 0x64: TXFIFO_ADDR
      __IO uint16_t rxfifo_addr;       // 0x66: RXFIFO_ADDR
    };
    __IO uint16_t fifo_addr[2];
  };

  //------------- Additional Control and Configuration -------------//
  union {
    __O  uint32_t vcontrol;        // 0x68: PHY VCONTROL
    __IO uint32_t vstatus;         // 0x68: PHY VSTATUS
  };
  union {
    __IO uint16_t hwvers;            // 0x6C: HWVERS
    struct {
      __IO uint16_t minor : 10;     // [9:0] Minor
      __IO uint16_t major : 5;      // [14:10] Major
      __IO uint16_t rc    : 1;      // [15] Release Candidate
    } hwvers_bit;
  };
  __R  uint16_t rsv_0x6e_0x77[5];  // 0x6E-0x77: Reserved

   //------------- Additional Configuration -------------//
  union {
    __IO uint8_t  epinfo;            // 0x78: EPINFO
    struct {
      __IO uint8_t tx_ep_num : 4;    // [3:0] TX Endpoints
      __IO uint8_t rx_ep_num : 4;    // [7:4] RX Endpoints
    } epinfo_bit;
  };
  union {
    __IO uint8_t  raminfo;           // 0x79: RAMINFO
    struct {
      __IO uint8_t ram_bits    : 4;  // [3:0] RAM Address Bus Width
      __IO uint8_t dma_channel : 4;  // [7:4] DMA Channels
    }raminfo_bit;
  };
  union {
    __IO uint8_t  link_info;       // 0x7A: LINK_INFO
    __IO uint8_t  adi_softreset;   // 0x7A: AnalogDevice SOFTRESET
  };
  __IO uint8_t  vplen;             // 0x7B: VPLEN
  __IO uint8_t  hs_eof1;           // 0x7C: HS_EOF1
  __IO uint8_t  fs_eof1;           // 0x7D: FS_EOF1
  __IO uint8_t  ls_eof1;           // 0x7E: LS_EOF1
  __IO uint8_t  soft_rst;          // 0x7F: SOFT_RST

  //------------- Target Endpoints (multipoint option) -------------//
  __IO uint16_t ctuch;             // 0x80: CTUCH
  __IO uint16_t cthsrtn;           // 0x82: CTHSRTN
  __R  uint32_t rsv_0x84_0xff[31]; // 0x84-0xFF: Reserved

  //------------- Non-Indexed Endpoint CSRs -------------//
  // TI tm4c can access this directly, but should use indexed_csr for portability
  musb_ep_csr_t ep_csr[16];        // 0x100-0x1FF: EP0-15 CSR
} musb_regs_t;

TU_VERIFY_STATIC(sizeof(musb_regs_t) == 0x200, "size is not correct");

//--------------------------------------------------------------------+
// Helper
//--------------------------------------------------------------------+
TU_ATTR_ALWAYS_INLINE static inline musb_ep_csr_t* get_ep_csr(musb_regs_t* musb_regs, unsigned epnum) {
  musb_regs->index = epnum;
  return &musb_regs->indexed_csr;
}

//--------------------------------------------------------------------+
// Register Bit Field
//--------------------------------------------------------------------+

// 0x01: Power
#define USB_POWER_ISOUP         0x0080  // Isochronous Update
#define USB_POWER_SOFTCONN      0x0040  // Soft Connect/Disconnect
#define USB_POWER_HSENAB        0x0020  // High Speed Enable
#define USB_POWER_HSMODE        0x0010  // High Speed Enable
#define USB_POWER_RESET         0x0008  // RESET Signaling
#define USB_POWER_RESUME        0x0004  // RESUME Signaling
#define USB_POWER_SUSPEND       0x0002  // SUSPEND Mode
#define USB_POWER_PWRDNPHY      0x0001  // Power Down PHY

// Interrupt TX/RX Status and Enable: each bit is for an endpoint

// 0x6c: HWVERS
#define MUSB_HWVERS_RC_SHIFT    15
#define MUSB_HWVERS_RC_MASK     0x8000
#define MUSB_HWVERS_MAJOR_SHIFT 10
#define MUSB_HWVERS_MAJOR_MASK  0x7C00
#define MUSB_HWVERS_MINOR_SHIFT 0
#define MUSB_HWVERS_MINOR_MASK  0x03FF

// 0x12, 0x16: TX/RX CSRL
#define MUSB_CSRL_PACKET_READY(_rx)      (1u << 0)
#define MUSB_CSRL_FLUSH_FIFO(_rx)        (1u << ((_rx) ? 4 : 3))
#define MUSB_CSRL_SEND_STALL(_rx)        (1u << ((_rx) ? 5 : 4))
#define MUSB_CSRL_SENT_STALL(_rx)        (1u << ((_rx) ? 6 : 5))
#define MUSB_CSRL_CLEAR_DATA_TOGGLE(_rx) (1u << ((_rx) ? 7 : 6))

// 0x13, 0x17: TX/RX CSRH
#define MUSB_CSRH_DISABLE_DOUBLE_PACKET(_rx) (1u << 1)


//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_IS register.
//
//*****************************************************************************
#define USB_IS_VBUSERR          0x0080  // VBUS Error (OTG only)
#define USB_IS_SESREQ           0x0040  // SESSION REQUEST (OTG only)
#define USB_IS_DISCON           0x0020  // Session Disconnect (OTG only)
#define USB_IS_CONN             0x0010  // Session Connect
#define USB_IS_SOF              0x0008  // Start of Frame
#define USB_IS_BABBLE           0x0004  // Babble Detected
#define USB_IS_RESET            0x0004  // RESET Signaling Detected
#define USB_IS_RESUME           0x0002  // RESUME Signaling Detected
#define USB_IS_SUSPEND          0x0001  // SUSPEND Signaling Detected

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_IE register.
//
//*****************************************************************************
#define USB_IE_VBUSERR          0x0080  // Enable VBUS Error Interrupt (OTG only)
#define USB_IE_SESREQ           0x0040  // Enable Session Request (OTG only)
#define USB_IE_DISCON           0x0020  // Enable Disconnect Interrupt
#define USB_IE_CONN             0x0010  // Enable Connect Interrupt
#define USB_IE_SOF              0x0008  // Enable Start-of-Frame Interrupt
#define USB_IE_BABBLE           0x0004  // Enable Babble Interrupt
#define USB_IE_RESET            0x0004  // Enable RESET Interrupt
#define USB_IE_RESUME           0x0002  // Enable RESUME Interrupt
#define USB_IE_SUSPND           0x0001  // Enable SUSPEND Interrupt

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_FRAME register.
//
//*****************************************************************************
#define USB_FRAME_M             0x07FF  // Frame Number
#define USB_FRAME_S             0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TEST register.
//
//*****************************************************************************
#define USB_TEST_FORCEH         0x0080  // Force Host Mode
#define USB_TEST_FIFOACC        0x0040  // FIFO Access
#define USB_TEST_FORCEFS        0x0020  // Force Full-Speed Mode
#define USB_TEST_FORCEHS        0x0010  // Force High-Speed Mode
#define USB_TEST_TESTPKT        0x0008  // Test Packet Mode Enable
#define USB_TEST_TESTK          0x0004  // Test_K Mode Enable
#define USB_TEST_TESTJ          0x0002  // Test_J Mode Enable
#define USB_TEST_TESTSE0NAK     0x0001  // Test_SE0_NAK Test Mode Enable

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DEVCTL register.
//
//*****************************************************************************
#define USB_DEVCTL_DEV          0x0080  // Device Mode (OTG only)
#define USB_DEVCTL_FSDEV        0x0040  // Full-Speed Device Detected
#define USB_DEVCTL_LSDEV        0x0020  // Low-Speed Device Detected
#define USB_DEVCTL_VBUS_M       0x0018  // VBUS Level (OTG only)
#define USB_DEVCTL_VBUS_NONE    0x0000  // Below SessionEnd
#define USB_DEVCTL_VBUS_SEND    0x0008  // Above SessionEnd, below AValid
#define USB_DEVCTL_VBUS_AVALID  0x0010  // Above AValid, below VBUSValid
#define USB_DEVCTL_VBUS_VALID   0x0018  // Above VBUSValid
#define USB_DEVCTL_HOST         0x0004  // Host Mode
#define USB_DEVCTL_HOSTREQ      0x0002  // Host Request (OTG only)
#define USB_DEVCTL_SESSION      0x0001  // Session Start/End (OTG only)

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_CCONF register.
//
//*****************************************************************************
#define USB_CCONF_TXEDMA        0x0002  // TX Early DMA Enable
#define USB_CCONF_RXEDMA        0x0001  // TX Early DMA Enable

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXFIFOSZ register.
//
//*****************************************************************************
#define USB_TXFIFOSZ_DPB        0x0010  // Double Packet Buffer Support
#define USB_TXFIFOSZ_SIZE_M     0x000F  // Max Packet Size
#define USB_TXFIFOSZ_SIZE_8     0x0000  // 8
#define USB_TXFIFOSZ_SIZE_16    0x0001  // 16
#define USB_TXFIFOSZ_SIZE_32    0x0002  // 32
#define USB_TXFIFOSZ_SIZE_64    0x0003  // 64
#define USB_TXFIFOSZ_SIZE_128   0x0004  // 128
#define USB_TXFIFOSZ_SIZE_256   0x0005  // 256
#define USB_TXFIFOSZ_SIZE_512   0x0006  // 512
#define USB_TXFIFOSZ_SIZE_1024  0x0007  // 1024
#define USB_TXFIFOSZ_SIZE_2048  0x0008  // 2048

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXFIFOSZ register.
//
//*****************************************************************************
#define USB_RXFIFOSZ_DPB        0x0010  // Double Packet Buffer Support
#define USB_RXFIFOSZ_SIZE_M     0x000F  // Max Packet Size
#define USB_RXFIFOSZ_SIZE_8     0x0000  // 8
#define USB_RXFIFOSZ_SIZE_16    0x0001  // 16
#define USB_RXFIFOSZ_SIZE_32    0x0002  // 32
#define USB_RXFIFOSZ_SIZE_64    0x0003  // 64
#define USB_RXFIFOSZ_SIZE_128   0x0004  // 128
#define USB_RXFIFOSZ_SIZE_256   0x0005  // 256
#define USB_RXFIFOSZ_SIZE_512   0x0006  // 512
#define USB_RXFIFOSZ_SIZE_1024  0x0007  // 1024
#define USB_RXFIFOSZ_SIZE_2048  0x0008  // 2048

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXFIFOADD
// register.
//
//*****************************************************************************
#define USB_TXFIFOADD_ADDR_M    0x01FF  // Transmit/Receive Start Address
#define USB_TXFIFOADD_ADDR_S    0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXFIFOADD
// register.
//
//*****************************************************************************
#define USB_RXFIFOADD_ADDR_M    0x01FF  // Transmit/Receive Start Address
#define USB_RXFIFOADD_ADDR_S    0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_ULPIVBUSCTL
// register.
//
//*****************************************************************************
#define USB_ULPIVBUSCTL_USEEXTVBUSIND  0x0002  // Use External VBUS Indicator
#define USB_ULPIVBUSCTL_USEEXTVBUS     0x0001  // Use External VBUS

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_ULPIREGDATA
// register.
//
//*****************************************************************************
#define USB_ULPIREGDATA_REGDATA_M      0x00FF  // Register Data
#define USB_ULPIREGDATA_REGDATA_S      0
//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_ULPIREGADDR
// register.
//
//*****************************************************************************
#define USB_ULPIREGADDR_ADDR_M  0x00FF  // Register Address
#define USB_ULPIREGADDR_ADDR_S  0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_ULPIREGCTL
// register.
//
//*****************************************************************************
#define USB_ULPIREGCTL_RDWR     0x0004  // Read/Write Control
#define USB_ULPIREGCTL_REGCMPLT 0x0002  // Register Access Complete
#define USB_ULPIREGCTL_REGACC   0x0001  // Initiate Register Access

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_EPINFO register.
//
//*****************************************************************************
#define USB_EPINFO_RXEP_M       0x00F0  // RX Endpoints
#define USB_EPINFO_TXEP_M       0x000F  // TX Endpoints
#define USB_EPINFO_RXEP_S       4
#define USB_EPINFO_TXEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RAMINFO register.
//
//*****************************************************************************
#define USB_RAMINFO_DMACHAN_M   0x00F0  // DMA Channels
#define USB_RAMINFO_RAMBITS_M   0x000F  // RAM Address Bus Width
#define USB_RAMINFO_DMACHAN_S   4
#define USB_RAMINFO_RAMBITS_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_CONTIM register.
//
//*****************************************************************************
#define USB_CONTIM_WTCON_M      0x00F0  // Connect Wait
#define USB_CONTIM_WTID_M       0x000F  // Wait ID
#define USB_CONTIM_WTCON_S      4
#define USB_CONTIM_WTID_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_VPLEN register.
//
//*****************************************************************************
#define USB_VPLEN_VPLEN_M       0x00FF  // VBUS Pulse Length
#define USB_VPLEN_VPLEN_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_HSEOF register.
//
//*****************************************************************************
#define USB_HSEOF_HSEOFG_M      0x00FF  // HIgh-Speed End-of-Frame Gap
#define USB_HSEOF_HSEOFG_S      0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_FSEOF register.
//
//*****************************************************************************
#define USB_FSEOF_FSEOFG_M      0x00FF  // Full-Speed End-of-Frame Gap
#define USB_FSEOF_FSEOFG_S      0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_LSEOF register.
//
//*****************************************************************************
#define USB_LSEOF_LSEOFG_M      0x00FF  // Low-Speed End-of-Frame Gap
#define USB_LSEOF_LSEOFG_S      0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_CSRL0 register.
//
//*****************************************************************************
#define USB_CSRL0_NAKTO         0x0080  // NAK Timeout
#define USB_CSRL0_SETENDC       0x0080  // Setup End Clear
#define USB_CSRL0_STATUS        0x0040  // STATUS Packet
#define USB_CSRL0_RXRDYC        0x0040  // RXRDY Clear
#define USB_CSRL0_REQPKT        0x0020  // Request Packet
#define USB_CSRL0_STALL         0x0020  // Send Stall
#define USB_CSRL0_SETEND        0x0010  // Setup End
#define USB_CSRL0_ERROR         0x0010  // Error
#define USB_CSRL0_DATAEND       0x0008  // Data End
#define USB_CSRL0_SETUP         0x0008  // Setup Packet
#define USB_CSRL0_STALLED       0x0004  // Endpoint Stalled
#define USB_CSRL0_TXRDY         0x0002  // Transmit Packet Ready
#define USB_CSRL0_RXRDY         0x0001  // Receive Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_CSRH0 register.
//
//*****************************************************************************
#define USB_CSRH0_DISPING       0x0008  // PING Disable
#define USB_CSRH0_DTWE          0x0004  // Data Toggle Write Enable
#define USB_CSRH0_DT            0x0002  // Data Toggle
#define USB_CSRH0_FLUSH         0x0001  // Flush FIFO

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_COUNT0 register.
//
//*****************************************************************************
#define USB_COUNT0_COUNT_M      0x007F  // FIFO Count
#define USB_COUNT0_COUNT_S      0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TYPE0 register.
//
//*****************************************************************************
#define USB_TYPE0_SPEED_M       0x00C0  // Operating Speed
#define USB_TYPE0_SPEED_HIGH    0x0040  // High
#define USB_TYPE0_SPEED_FULL    0x0080  // Full
#define USB_TYPE0_SPEED_LOW     0x00C0  // Low

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_NAKLMT register.
//
//*****************************************************************************
#define USB_NAKLMT_NAKLMT_M     0x001F  // EP0 NAK Limit
#define USB_NAKLMT_NAKLMT_S     0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXMAXP1 register.
//
//*****************************************************************************
#define USB_TXMAXP1_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_TXMAXP1_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRL1 register.
//
//*****************************************************************************
#define USB_TXCSRL1_NAKTO       0x0080  // NAK Timeout
#define USB_TXCSRL1_CLRDT       0x0040  // Clear Data Toggle
#define USB_TXCSRL1_STALLED     0x0020  // Endpoint Stalled
#define USB_TXCSRL1_STALL       0x0010  // Send STALL
#define USB_TXCSRL1_SETUP       0x0010  // Setup Packet
#define USB_TXCSRL1_FLUSH       0x0008  // Flush FIFO
#define USB_TXCSRL1_ERROR       0x0004  // Error
#define USB_TXCSRL1_UNDRN       0x0004  // Underrun
#define USB_TXCSRL1_FIFONE      0x0002  // FIFO Not Empty
#define USB_TXCSRL1_TXRDY       0x0001  // Transmit Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRH1 register.
//
//*****************************************************************************
#define USB_TXCSRH1_AUTOSET     0x0080  // Auto Set
#define USB_TXCSRH1_ISO         0x0040  // Isochronous Transfers
#define USB_TXCSRH1_MODE        0x0020  // Mode
#define USB_TXCSRH1_DMAEN       0x0010  // DMA Request Enable
#define USB_TXCSRH1_FDT         0x0008  // Force Data Toggle
#define USB_TXCSRH1_DMAMOD      0x0004  // DMA Request Mode
#define USB_TXCSRH1_DTWE        0x0002  // Data Toggle Write Enable
#define USB_TXCSRH1_DT          0x0001  // Data Toggle

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXMAXP1 register.
//
//*****************************************************************************
#define USB_RXMAXP1_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_RXMAXP1_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRL1 register.
//
//*****************************************************************************
#define USB_RXCSRL1_CLRDT       0x0080  // Clear Data Toggle
#define USB_RXCSRL1_STALLED     0x0040  // Endpoint Stalled
#define USB_RXCSRL1_STALL       0x0020  // Send STALL
#define USB_RXCSRL1_REQPKT      0x0020  // Request Packet
#define USB_RXCSRL1_FLUSH       0x0010  // Flush FIFO
#define USB_RXCSRL1_DATAERR     0x0008  // Data Error
#define USB_RXCSRL1_NAKTO       0x0008  // NAK Timeout
#define USB_RXCSRL1_OVER        0x0004  // Overrun
#define USB_RXCSRL1_ERROR       0x0004  // Error
#define USB_RXCSRL1_FULL        0x0002  // FIFO Full
#define USB_RXCSRL1_RXRDY       0x0001  // Receive Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRH1 register.
//
//*****************************************************************************
#define USB_RXCSRH1_AUTOCL      0x0080  // Auto Clear
#define USB_RXCSRH1_AUTORQ      0x0040  // Auto Request
#define USB_RXCSRH1_ISO         0x0040  // Isochronous Transfers
#define USB_RXCSRH1_DMAEN       0x0020  // DMA Request Enable
#define USB_RXCSRH1_DISNYET     0x0010  // Disable NYET
#define USB_RXCSRH1_PIDERR      0x0010  // PID Error
#define USB_RXCSRH1_DMAMOD      0x0008  // DMA Request Mode
#define USB_RXCSRH1_DTWE        0x0004  // Data Toggle Write Enable
#define USB_RXCSRH1_DT          0x0002  // Data Toggle
#define USB_RXCSRH1_INCOMPRX    0x0001  // Incomplete RX Transmission
                                            // Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCOUNT1 register.
//
//*****************************************************************************
#define USB_RXCOUNT1_COUNT_M    0x1FFF  // Receive Packet Count
#define USB_RXCOUNT1_COUNT_S    0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXTYPE1 register.
//
//*****************************************************************************
#define USB_TXTYPE1_SPEED_M     0x00C0  // Operating Speed
#define USB_TXTYPE1_SPEED_DFLT  0x0000  // Default
#define USB_TXTYPE1_SPEED_HIGH  0x0040  // High
#define USB_TXTYPE1_SPEED_FULL  0x0080  // Full
#define USB_TXTYPE1_SPEED_LOW   0x00C0  // Low
#define USB_TXTYPE1_PROTO_M     0x0030  // Protocol
#define USB_TXTYPE1_PROTO_CTRL  0x0000  // Control
#define USB_TXTYPE1_PROTO_ISOC  0x0010  // Isochronous
#define USB_TXTYPE1_PROTO_BULK  0x0020  // Bulk
#define USB_TXTYPE1_PROTO_INT   0x0030  // Interrupt
#define USB_TXTYPE1_TEP_M       0x000F  // Target Endpoint Number
#define USB_TXTYPE1_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXINTERVAL1
// register.
//
//*****************************************************************************
#define USB_TXINTERVAL1_NAKLMT_M 0x00FF  // NAK Limit
#define USB_TXINTERVAL1_TXPOLL_M 0x00FF  // TX Polling
#define USB_TXINTERVAL1_TXPOLL_S 0
#define USB_TXINTERVAL1_NAKLMT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXTYPE1 register.
//
//*****************************************************************************
#define USB_RXTYPE1_SPEED_M     0x00C0  // Operating Speed
#define USB_RXTYPE1_SPEED_DFLT  0x0000  // Default
#define USB_RXTYPE1_SPEED_HIGH  0x0040  // High
#define USB_RXTYPE1_SPEED_FULL  0x0080  // Full
#define USB_RXTYPE1_SPEED_LOW   0x00C0  // Low
#define USB_RXTYPE1_PROTO_M     0x0030  // Protocol
#define USB_RXTYPE1_PROTO_CTRL  0x0000  // Control
#define USB_RXTYPE1_PROTO_ISOC  0x0010  // Isochronous
#define USB_RXTYPE1_PROTO_BULK  0x0020  // Bulk
#define USB_RXTYPE1_PROTO_INT   0x0030  // Interrupt
#define USB_RXTYPE1_TEP_M       0x000F  // Target Endpoint Number
#define USB_RXTYPE1_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXINTERVAL1
// register.
//
//*****************************************************************************
#define USB_RXINTERVAL1_TXPOLL_M 0x00FF  // RX Polling
#define USB_RXINTERVAL1_NAKLMT_M 0x00FF  // NAK Limit
#define USB_RXINTERVAL1_TXPOLL_S 0
#define USB_RXINTERVAL1_NAKLMT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXMAXP2 register.
//
//*****************************************************************************
#define USB_TXMAXP2_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_TXMAXP2_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRL2 register.
//
//*****************************************************************************
#define USB_TXCSRL2_NAKTO       0x0080  // NAK Timeout
#define USB_TXCSRL2_CLRDT       0x0040  // Clear Data Toggle
#define USB_TXCSRL2_STALLED     0x0020  // Endpoint Stalled
#define USB_TXCSRL2_SETUP       0x0010  // Setup Packet
#define USB_TXCSRL2_STALL       0x0010  // Send STALL
#define USB_TXCSRL2_FLUSH       0x0008  // Flush FIFO
#define USB_TXCSRL2_ERROR       0x0004  // Error
#define USB_TXCSRL2_UNDRN       0x0004  // Underrun
#define USB_TXCSRL2_FIFONE      0x0002  // FIFO Not Empty
#define USB_TXCSRL2_TXRDY       0x0001  // Transmit Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRH2 register.
//
//*****************************************************************************
#define USB_TXCSRH2_AUTOSET     0x0080  // Auto Set
#define USB_TXCSRH2_ISO         0x0040  // Isochronous Transfers
#define USB_TXCSRH2_MODE        0x0020  // Mode
#define USB_TXCSRH2_DMAEN       0x0010  // DMA Request Enable
#define USB_TXCSRH2_FDT         0x0008  // Force Data Toggle
#define USB_TXCSRH2_DMAMOD      0x0004  // DMA Request Mode
#define USB_TXCSRH2_DTWE        0x0002  // Data Toggle Write Enable
#define USB_TXCSRH2_DT          0x0001  // Data Toggle

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXMAXP2 register.
//
//*****************************************************************************
#define USB_RXMAXP2_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_RXMAXP2_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRL2 register.
//
//*****************************************************************************
#define USB_RXCSRL2_CLRDT       0x0080  // Clear Data Toggle
#define USB_RXCSRL2_STALLED     0x0040  // Endpoint Stalled
#define USB_RXCSRL2_REQPKT      0x0020  // Request Packet
#define USB_RXCSRL2_STALL       0x0020  // Send STALL
#define USB_RXCSRL2_FLUSH       0x0010  // Flush FIFO
#define USB_RXCSRL2_DATAERR     0x0008  // Data Error
#define USB_RXCSRL2_NAKTO       0x0008  // NAK Timeout
#define USB_RXCSRL2_ERROR       0x0004  // Error
#define USB_RXCSRL2_OVER        0x0004  // Overrun
#define USB_RXCSRL2_FULL        0x0002  // FIFO Full
#define USB_RXCSRL2_RXRDY       0x0001  // Receive Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRH2 register.
//
//*****************************************************************************
#define USB_RXCSRH2_AUTOCL      0x0080  // Auto Clear
#define USB_RXCSRH2_AUTORQ      0x0040  // Auto Request
#define USB_RXCSRH2_ISO         0x0040  // Isochronous Transfers
#define USB_RXCSRH2_DMAEN       0x0020  // DMA Request Enable
#define USB_RXCSRH2_DISNYET     0x0010  // Disable NYET
#define USB_RXCSRH2_PIDERR      0x0010  // PID Error
#define USB_RXCSRH2_DMAMOD      0x0008  // DMA Request Mode
#define USB_RXCSRH2_DTWE        0x0004  // Data Toggle Write Enable
#define USB_RXCSRH2_DT          0x0002  // Data Toggle
#define USB_RXCSRH2_INCOMPRX    0x0001  // Incomplete RX Transmission
                                            // Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCOUNT2 register.
//
//*****************************************************************************
#define USB_RXCOUNT2_COUNT_M    0x1FFF  // Receive Packet Count
#define USB_RXCOUNT2_COUNT_S    0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXTYPE2 register.
//
//*****************************************************************************
#define USB_TXTYPE2_SPEED_M     0x00C0  // Operating Speed
#define USB_TXTYPE2_SPEED_DFLT  0x0000  // Default
#define USB_TXTYPE2_SPEED_HIGH  0x0040  // High
#define USB_TXTYPE2_SPEED_FULL  0x0080  // Full
#define USB_TXTYPE2_SPEED_LOW   0x00C0  // Low
#define USB_TXTYPE2_PROTO_M     0x0030  // Protocol
#define USB_TXTYPE2_PROTO_CTRL  0x0000  // Control
#define USB_TXTYPE2_PROTO_ISOC  0x0010  // Isochronous
#define USB_TXTYPE2_PROTO_BULK  0x0020  // Bulk
#define USB_TXTYPE2_PROTO_INT   0x0030  // Interrupt
#define USB_TXTYPE2_TEP_M       0x000F  // Target Endpoint Number
#define USB_TXTYPE2_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXINTERVAL2
// register.
//
//*****************************************************************************
#define USB_TXINTERVAL2_TXPOLL_M 0x00FF  // TX Polling
#define USB_TXINTERVAL2_NAKLMT_M 0x00FF  // NAK Limit
#define USB_TXINTERVAL2_NAKLMT_S 0
#define USB_TXINTERVAL2_TXPOLL_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXTYPE2 register.
//
//*****************************************************************************
#define USB_RXTYPE2_SPEED_M     0x00C0  // Operating Speed
#define USB_RXTYPE2_SPEED_DFLT  0x0000  // Default
#define USB_RXTYPE2_SPEED_HIGH  0x0040  // High
#define USB_RXTYPE2_SPEED_FULL  0x0080  // Full
#define USB_RXTYPE2_SPEED_LOW   0x00C0  // Low
#define USB_RXTYPE2_PROTO_M     0x0030  // Protocol
#define USB_RXTYPE2_PROTO_CTRL  0x0000  // Control
#define USB_RXTYPE2_PROTO_ISOC  0x0010  // Isochronous
#define USB_RXTYPE2_PROTO_BULK  0x0020  // Bulk
#define USB_RXTYPE2_PROTO_INT   0x0030  // Interrupt
#define USB_RXTYPE2_TEP_M       0x000F  // Target Endpoint Number
#define USB_RXTYPE2_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXINTERVAL2
// register.
//
//*****************************************************************************
#define USB_RXINTERVAL2_TXPOLL_M 0x00FF  // RX Polling
#define USB_RXINTERVAL2_NAKLMT_M 0x00FF  // NAK Limit
#define USB_RXINTERVAL2_TXPOLL_S 0
#define USB_RXINTERVAL2_NAKLMT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXMAXP3 register.
//
//*****************************************************************************
#define USB_TXMAXP3_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_TXMAXP3_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRL3 register.
//
//*****************************************************************************
#define USB_TXCSRL3_NAKTO       0x0080  // NAK Timeout
#define USB_TXCSRL3_CLRDT       0x0040  // Clear Data Toggle
#define USB_TXCSRL3_STALLED     0x0020  // Endpoint Stalled
#define USB_TXCSRL3_SETUP       0x0010  // Setup Packet
#define USB_TXCSRL3_STALL       0x0010  // Send STALL
#define USB_TXCSRL3_FLUSH       0x0008  // Flush FIFO
#define USB_TXCSRL3_ERROR       0x0004  // Error
#define USB_TXCSRL3_UNDRN       0x0004  // Underrun
#define USB_TXCSRL3_FIFONE      0x0002  // FIFO Not Empty
#define USB_TXCSRL3_TXRDY       0x0001  // Transmit Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRH3 register.
//
//*****************************************************************************
#define USB_TXCSRH3_AUTOSET     0x0080  // Auto Set
#define USB_TXCSRH3_ISO         0x0040  // Isochronous Transfers
#define USB_TXCSRH3_MODE        0x0020  // Mode
#define USB_TXCSRH3_DMAEN       0x0010  // DMA Request Enable
#define USB_TXCSRH3_FDT         0x0008  // Force Data Toggle
#define USB_TXCSRH3_DMAMOD      0x0004  // DMA Request Mode
#define USB_TXCSRH3_DTWE        0x0002  // Data Toggle Write Enable
#define USB_TXCSRH3_DT          0x0001  // Data Toggle

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXMAXP3 register.
//
//*****************************************************************************
#define USB_RXMAXP3_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_RXMAXP3_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRL3 register.
//
//*****************************************************************************
#define USB_RXCSRL3_CLRDT       0x0080  // Clear Data Toggle
#define USB_RXCSRL3_STALLED     0x0040  // Endpoint Stalled
#define USB_RXCSRL3_STALL       0x0020  // Send STALL
#define USB_RXCSRL3_REQPKT      0x0020  // Request Packet
#define USB_RXCSRL3_FLUSH       0x0010  // Flush FIFO
#define USB_RXCSRL3_DATAERR     0x0008  // Data Error
#define USB_RXCSRL3_NAKTO       0x0008  // NAK Timeout
#define USB_RXCSRL3_ERROR       0x0004  // Error
#define USB_RXCSRL3_OVER        0x0004  // Overrun
#define USB_RXCSRL3_FULL        0x0002  // FIFO Full
#define USB_RXCSRL3_RXRDY       0x0001  // Receive Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRH3 register.
//
//*****************************************************************************
#define USB_RXCSRH3_AUTOCL      0x0080  // Auto Clear
#define USB_RXCSRH3_AUTORQ      0x0040  // Auto Request
#define USB_RXCSRH3_ISO         0x0040  // Isochronous Transfers
#define USB_RXCSRH3_DMAEN       0x0020  // DMA Request Enable
#define USB_RXCSRH3_DISNYET     0x0010  // Disable NYET
#define USB_RXCSRH3_PIDERR      0x0010  // PID Error
#define USB_RXCSRH3_DMAMOD      0x0008  // DMA Request Mode
#define USB_RXCSRH3_DTWE        0x0004  // Data Toggle Write Enable
#define USB_RXCSRH3_DT          0x0002  // Data Toggle
#define USB_RXCSRH3_INCOMPRX    0x0001  // Incomplete RX Transmission
                                            // Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCOUNT3 register.
//
//*****************************************************************************
#define USB_RXCOUNT3_COUNT_M    0x1FFF  // Receive Packet Count
#define USB_RXCOUNT3_COUNT_S    0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXTYPE3 register.
//
//*****************************************************************************
#define USB_TXTYPE3_SPEED_M     0x00C0  // Operating Speed
#define USB_TXTYPE3_SPEED_DFLT  0x0000  // Default
#define USB_TXTYPE3_SPEED_HIGH  0x0040  // High
#define USB_TXTYPE3_SPEED_FULL  0x0080  // Full
#define USB_TXTYPE3_SPEED_LOW   0x00C0  // Low
#define USB_TXTYPE3_PROTO_M     0x0030  // Protocol
#define USB_TXTYPE3_PROTO_CTRL  0x0000  // Control
#define USB_TXTYPE3_PROTO_ISOC  0x0010  // Isochronous
#define USB_TXTYPE3_PROTO_BULK  0x0020  // Bulk
#define USB_TXTYPE3_PROTO_INT   0x0030  // Interrupt
#define USB_TXTYPE3_TEP_M       0x000F  // Target Endpoint Number
#define USB_TXTYPE3_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXINTERVAL3
// register.
//
//*****************************************************************************
#define USB_TXINTERVAL3_TXPOLL_M 0x00FF  // TX Polling
#define USB_TXINTERVAL3_NAKLMT_M 0x00FF  // NAK Limit
#define USB_TXINTERVAL3_TXPOLL_S 0
#define USB_TXINTERVAL3_NAKLMT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXTYPE3 register.
//
//*****************************************************************************
#define USB_RXTYPE3_SPEED_M     0x00C0  // Operating Speed
#define USB_RXTYPE3_SPEED_DFLT  0x0000  // Default
#define USB_RXTYPE3_SPEED_HIGH  0x0040  // High
#define USB_RXTYPE3_SPEED_FULL  0x0080  // Full
#define USB_RXTYPE3_SPEED_LOW   0x00C0  // Low
#define USB_RXTYPE3_PROTO_M     0x0030  // Protocol
#define USB_RXTYPE3_PROTO_CTRL  0x0000  // Control
#define USB_RXTYPE3_PROTO_ISOC  0x0010  // Isochronous
#define USB_RXTYPE3_PROTO_BULK  0x0020  // Bulk
#define USB_RXTYPE3_PROTO_INT   0x0030  // Interrupt
#define USB_RXTYPE3_TEP_M       0x000F  // Target Endpoint Number
#define USB_RXTYPE3_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXINTERVAL3
// register.
//
//*****************************************************************************
#define USB_RXINTERVAL3_TXPOLL_M 0x00FF  // RX Polling
#define USB_RXINTERVAL3_NAKLMT_M 0x00FF  // NAK Limit
#define USB_RXINTERVAL3_TXPOLL_S 0
#define USB_RXINTERVAL3_NAKLMT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXMAXP4 register.
//
//*****************************************************************************
#define USB_TXMAXP4_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_TXMAXP4_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRL4 register.
//
//*****************************************************************************
#define USB_TXCSRL4_NAKTO       0x0080  // NAK Timeout
#define USB_TXCSRL4_CLRDT       0x0040  // Clear Data Toggle
#define USB_TXCSRL4_STALLED     0x0020  // Endpoint Stalled
#define USB_TXCSRL4_SETUP       0x0010  // Setup Packet
#define USB_TXCSRL4_STALL       0x0010  // Send STALL
#define USB_TXCSRL4_FLUSH       0x0008  // Flush FIFO
#define USB_TXCSRL4_ERROR       0x0004  // Error
#define USB_TXCSRL4_UNDRN       0x0004  // Underrun
#define USB_TXCSRL4_FIFONE      0x0002  // FIFO Not Empty
#define USB_TXCSRL4_TXRDY       0x0001  // Transmit Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRH4 register.
//
//*****************************************************************************
#define USB_TXCSRH4_AUTOSET     0x0080  // Auto Set
#define USB_TXCSRH4_ISO         0x0040  // Isochronous Transfers
#define USB_TXCSRH4_MODE        0x0020  // Mode
#define USB_TXCSRH4_DMAEN       0x0010  // DMA Request Enable
#define USB_TXCSRH4_FDT         0x0008  // Force Data Toggle
#define USB_TXCSRH4_DMAMOD      0x0004  // DMA Request Mode
#define USB_TXCSRH4_DTWE        0x0002  // Data Toggle Write Enable
#define USB_TXCSRH4_DT          0x0001  // Data Toggle

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXMAXP4 register.
//
//*****************************************************************************
#define USB_RXMAXP4_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_RXMAXP4_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRL4 register.
//
//*****************************************************************************
#define USB_RXCSRL4_CLRDT       0x0080  // Clear Data Toggle
#define USB_RXCSRL4_STALLED     0x0040  // Endpoint Stalled
#define USB_RXCSRL4_STALL       0x0020  // Send STALL
#define USB_RXCSRL4_REQPKT      0x0020  // Request Packet
#define USB_RXCSRL4_FLUSH       0x0010  // Flush FIFO
#define USB_RXCSRL4_NAKTO       0x0008  // NAK Timeout
#define USB_RXCSRL4_DATAERR     0x0008  // Data Error
#define USB_RXCSRL4_OVER        0x0004  // Overrun
#define USB_RXCSRL4_ERROR       0x0004  // Error
#define USB_RXCSRL4_FULL        0x0002  // FIFO Full
#define USB_RXCSRL4_RXRDY       0x0001  // Receive Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRH4 register.
//
//*****************************************************************************
#define USB_RXCSRH4_AUTOCL      0x0080  // Auto Clear
#define USB_RXCSRH4_AUTORQ      0x0040  // Auto Request
#define USB_RXCSRH4_ISO         0x0040  // Isochronous Transfers
#define USB_RXCSRH4_DMAEN       0x0020  // DMA Request Enable
#define USB_RXCSRH4_DISNYET     0x0010  // Disable NYET
#define USB_RXCSRH4_PIDERR      0x0010  // PID Error
#define USB_RXCSRH4_DMAMOD      0x0008  // DMA Request Mode
#define USB_RXCSRH4_DTWE        0x0004  // Data Toggle Write Enable
#define USB_RXCSRH4_DT          0x0002  // Data Toggle
#define USB_RXCSRH4_INCOMPRX    0x0001  // Incomplete RX Transmission
                                            // Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCOUNT4 register.
//
//*****************************************************************************
#define USB_RXCOUNT4_COUNT_M    0x1FFF  // Receive Packet Count
#define USB_RXCOUNT4_COUNT_S    0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXTYPE4 register.
//
//*****************************************************************************
#define USB_TXTYPE4_SPEED_M     0x00C0  // Operating Speed
#define USB_TXTYPE4_SPEED_DFLT  0x0000  // Default
#define USB_TXTYPE4_SPEED_HIGH  0x0040  // High
#define USB_TXTYPE4_SPEED_FULL  0x0080  // Full
#define USB_TXTYPE4_SPEED_LOW   0x00C0  // Low
#define USB_TXTYPE4_PROTO_M     0x0030  // Protocol
#define USB_TXTYPE4_PROTO_CTRL  0x0000  // Control
#define USB_TXTYPE4_PROTO_ISOC  0x0010  // Isochronous
#define USB_TXTYPE4_PROTO_BULK  0x0020  // Bulk
#define USB_TXTYPE4_PROTO_INT   0x0030  // Interrupt
#define USB_TXTYPE4_TEP_M       0x000F  // Target Endpoint Number
#define USB_TXTYPE4_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXINTERVAL4
// register.
//
//*****************************************************************************
#define USB_TXINTERVAL4_TXPOLL_M 0x00FF  // TX Polling
#define USB_TXINTERVAL4_NAKLMT_M 0x00FF  // NAK Limit
#define USB_TXINTERVAL4_NAKLMT_S 0
#define USB_TXINTERVAL4_TXPOLL_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXTYPE4 register.
//
//*****************************************************************************
#define USB_RXTYPE4_SPEED_M     0x00C0  // Operating Speed
#define USB_RXTYPE4_SPEED_DFLT  0x0000  // Default
#define USB_RXTYPE4_SPEED_HIGH  0x0040  // High
#define USB_RXTYPE4_SPEED_FULL  0x0080  // Full
#define USB_RXTYPE4_SPEED_LOW   0x00C0  // Low
#define USB_RXTYPE4_PROTO_M     0x0030  // Protocol
#define USB_RXTYPE4_PROTO_CTRL  0x0000  // Control
#define USB_RXTYPE4_PROTO_ISOC  0x0010  // Isochronous
#define USB_RXTYPE4_PROTO_BULK  0x0020  // Bulk
#define USB_RXTYPE4_PROTO_INT   0x0030  // Interrupt
#define USB_RXTYPE4_TEP_M       0x000F  // Target Endpoint Number
#define USB_RXTYPE4_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXINTERVAL4
// register.
//
//*****************************************************************************
#define USB_RXINTERVAL4_TXPOLL_M 0x00FF  // RX Polling
#define USB_RXINTERVAL4_NAKLMT_M 0x00FF  // NAK Limit
#define USB_RXINTERVAL4_NAKLMT_S 0
#define USB_RXINTERVAL4_TXPOLL_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXMAXP5 register.
//
//*****************************************************************************
#define USB_TXMAXP5_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_TXMAXP5_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRL5 register.
//
//*****************************************************************************
#define USB_TXCSRL5_NAKTO       0x0080  // NAK Timeout
#define USB_TXCSRL5_CLRDT       0x0040  // Clear Data Toggle
#define USB_TXCSRL5_STALLED     0x0020  // Endpoint Stalled
#define USB_TXCSRL5_SETUP       0x0010  // Setup Packet
#define USB_TXCSRL5_STALL       0x0010  // Send STALL
#define USB_TXCSRL5_FLUSH       0x0008  // Flush FIFO
#define USB_TXCSRL5_ERROR       0x0004  // Error
#define USB_TXCSRL5_UNDRN       0x0004  // Underrun
#define USB_TXCSRL5_FIFONE      0x0002  // FIFO Not Empty
#define USB_TXCSRL5_TXRDY       0x0001  // Transmit Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRH5 register.
//
//*****************************************************************************
#define USB_TXCSRH5_AUTOSET     0x0080  // Auto Set
#define USB_TXCSRH5_ISO         0x0040  // Isochronous Transfers
#define USB_TXCSRH5_MODE        0x0020  // Mode
#define USB_TXCSRH5_DMAEN       0x0010  // DMA Request Enable
#define USB_TXCSRH5_FDT         0x0008  // Force Data Toggle
#define USB_TXCSRH5_DMAMOD      0x0004  // DMA Request Mode
#define USB_TXCSRH5_DTWE        0x0002  // Data Toggle Write Enable
#define USB_TXCSRH5_DT          0x0001  // Data Toggle

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXMAXP5 register.
//
//*****************************************************************************
#define USB_RXMAXP5_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_RXMAXP5_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRL5 register.
//
//*****************************************************************************
#define USB_RXCSRL5_CLRDT       0x0080  // Clear Data Toggle
#define USB_RXCSRL5_STALLED     0x0040  // Endpoint Stalled
#define USB_RXCSRL5_STALL       0x0020  // Send STALL
#define USB_RXCSRL5_REQPKT      0x0020  // Request Packet
#define USB_RXCSRL5_FLUSH       0x0010  // Flush FIFO
#define USB_RXCSRL5_NAKTO       0x0008  // NAK Timeout
#define USB_RXCSRL5_DATAERR     0x0008  // Data Error
#define USB_RXCSRL5_ERROR       0x0004  // Error
#define USB_RXCSRL5_OVER        0x0004  // Overrun
#define USB_RXCSRL5_FULL        0x0002  // FIFO Full
#define USB_RXCSRL5_RXRDY       0x0001  // Receive Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRH5 register.
//
//*****************************************************************************
#define USB_RXCSRH5_AUTOCL      0x0080  // Auto Clear
#define USB_RXCSRH5_AUTORQ      0x0040  // Auto Request
#define USB_RXCSRH5_ISO         0x0040  // Isochronous Transfers
#define USB_RXCSRH5_DMAEN       0x0020  // DMA Request Enable
#define USB_RXCSRH5_DISNYET     0x0010  // Disable NYET
#define USB_RXCSRH5_PIDERR      0x0010  // PID Error
#define USB_RXCSRH5_DMAMOD      0x0008  // DMA Request Mode
#define USB_RXCSRH5_DTWE        0x0004  // Data Toggle Write Enable
#define USB_RXCSRH5_DT          0x0002  // Data Toggle
#define USB_RXCSRH5_INCOMPRX    0x0001  // Incomplete RX Transmission
                                            // Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCOUNT5 register.
//
//*****************************************************************************
#define USB_RXCOUNT5_COUNT_M    0x1FFF  // Receive Packet Count
#define USB_RXCOUNT5_COUNT_S    0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXTYPE5 register.
//
//*****************************************************************************
#define USB_TXTYPE5_SPEED_M     0x00C0  // Operating Speed
#define USB_TXTYPE5_SPEED_DFLT  0x0000  // Default
#define USB_TXTYPE5_SPEED_HIGH  0x0040  // High
#define USB_TXTYPE5_SPEED_FULL  0x0080  // Full
#define USB_TXTYPE5_SPEED_LOW   0x00C0  // Low
#define USB_TXTYPE5_PROTO_M     0x0030  // Protocol
#define USB_TXTYPE5_PROTO_CTRL  0x0000  // Control
#define USB_TXTYPE5_PROTO_ISOC  0x0010  // Isochronous
#define USB_TXTYPE5_PROTO_BULK  0x0020  // Bulk
#define USB_TXTYPE5_PROTO_INT   0x0030  // Interrupt
#define USB_TXTYPE5_TEP_M       0x000F  // Target Endpoint Number
#define USB_TXTYPE5_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXINTERVAL5
// register.
//
//*****************************************************************************
#define USB_TXINTERVAL5_TXPOLL_M 0x00FF  // TX Polling
#define USB_TXINTERVAL5_NAKLMT_M 0x00FF  // NAK Limit
#define USB_TXINTERVAL5_NAKLMT_S 0
#define USB_TXINTERVAL5_TXPOLL_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXTYPE5 register.
//
//*****************************************************************************
#define USB_RXTYPE5_SPEED_M     0x00C0  // Operating Speed
#define USB_RXTYPE5_SPEED_DFLT  0x0000  // Default
#define USB_RXTYPE5_SPEED_HIGH  0x0040  // High
#define USB_RXTYPE5_SPEED_FULL  0x0080  // Full
#define USB_RXTYPE5_SPEED_LOW   0x00C0  // Low
#define USB_RXTYPE5_PROTO_M     0x0030  // Protocol
#define USB_RXTYPE5_PROTO_CTRL  0x0000  // Control
#define USB_RXTYPE5_PROTO_ISOC  0x0010  // Isochronous
#define USB_RXTYPE5_PROTO_BULK  0x0020  // Bulk
#define USB_RXTYPE5_PROTO_INT   0x0030  // Interrupt
#define USB_RXTYPE5_TEP_M       0x000F  // Target Endpoint Number
#define USB_RXTYPE5_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXINTERVAL5
// register.
//
//*****************************************************************************
#define USB_RXINTERVAL5_TXPOLL_M 0x00FF  // RX Polling
#define USB_RXINTERVAL5_NAKLMT_M 0x00FF  // NAK Limit
#define USB_RXINTERVAL5_TXPOLL_S 0
#define USB_RXINTERVAL5_NAKLMT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXMAXP6 register.
//
//*****************************************************************************
#define USB_TXMAXP6_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_TXMAXP6_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRL6 register.
//
//*****************************************************************************
#define USB_TXCSRL6_NAKTO       0x0080  // NAK Timeout
#define USB_TXCSRL6_CLRDT       0x0040  // Clear Data Toggle
#define USB_TXCSRL6_STALLED     0x0020  // Endpoint Stalled
#define USB_TXCSRL6_STALL       0x0010  // Send STALL
#define USB_TXCSRL6_SETUP       0x0010  // Setup Packet
#define USB_TXCSRL6_FLUSH       0x0008  // Flush FIFO
#define USB_TXCSRL6_ERROR       0x0004  // Error
#define USB_TXCSRL6_UNDRN       0x0004  // Underrun
#define USB_TXCSRL6_FIFONE      0x0002  // FIFO Not Empty
#define USB_TXCSRL6_TXRDY       0x0001  // Transmit Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRH6 register.
//
//*****************************************************************************
#define USB_TXCSRH6_AUTOSET     0x0080  // Auto Set
#define USB_TXCSRH6_ISO         0x0040  // Isochronous Transfers
#define USB_TXCSRH6_MODE        0x0020  // Mode
#define USB_TXCSRH6_DMAEN       0x0010  // DMA Request Enable
#define USB_TXCSRH6_FDT         0x0008  // Force Data Toggle
#define USB_TXCSRH6_DMAMOD      0x0004  // DMA Request Mode
#define USB_TXCSRH6_DTWE        0x0002  // Data Toggle Write Enable
#define USB_TXCSRH6_DT          0x0001  // Data Toggle

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXMAXP6 register.
//
//*****************************************************************************
#define USB_RXMAXP6_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_RXMAXP6_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRL6 register.
//
//*****************************************************************************
#define USB_RXCSRL6_CLRDT       0x0080  // Clear Data Toggle
#define USB_RXCSRL6_STALLED     0x0040  // Endpoint Stalled
#define USB_RXCSRL6_REQPKT      0x0020  // Request Packet
#define USB_RXCSRL6_STALL       0x0020  // Send STALL
#define USB_RXCSRL6_FLUSH       0x0010  // Flush FIFO
#define USB_RXCSRL6_NAKTO       0x0008  // NAK Timeout
#define USB_RXCSRL6_DATAERR     0x0008  // Data Error
#define USB_RXCSRL6_ERROR       0x0004  // Error
#define USB_RXCSRL6_OVER        0x0004  // Overrun
#define USB_RXCSRL6_FULL        0x0002  // FIFO Full
#define USB_RXCSRL6_RXRDY       0x0001  // Receive Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRH6 register.
//
//*****************************************************************************
#define USB_RXCSRH6_AUTOCL      0x0080  // Auto Clear
#define USB_RXCSRH6_AUTORQ      0x0040  // Auto Request
#define USB_RXCSRH6_ISO         0x0040  // Isochronous Transfers
#define USB_RXCSRH6_DMAEN       0x0020  // DMA Request Enable
#define USB_RXCSRH6_DISNYET     0x0010  // Disable NYET
#define USB_RXCSRH6_PIDERR      0x0010  // PID Error
#define USB_RXCSRH6_DMAMOD      0x0008  // DMA Request Mode
#define USB_RXCSRH6_DTWE        0x0004  // Data Toggle Write Enable
#define USB_RXCSRH6_DT          0x0002  // Data Toggle
#define USB_RXCSRH6_INCOMPRX    0x0001  // Incomplete RX Transmission
                                            // Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCOUNT6 register.
//
//*****************************************************************************
#define USB_RXCOUNT6_COUNT_M    0x1FFF  // Receive Packet Count
#define USB_RXCOUNT6_COUNT_S    0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXTYPE6 register.
//
//*****************************************************************************
#define USB_TXTYPE6_SPEED_M     0x00C0  // Operating Speed
#define USB_TXTYPE6_SPEED_DFLT  0x0000  // Default
#define USB_TXTYPE6_SPEED_HIGH  0x0040  // High
#define USB_TXTYPE6_SPEED_FULL  0x0080  // Full
#define USB_TXTYPE6_SPEED_LOW   0x00C0  // Low
#define USB_TXTYPE6_PROTO_M     0x0030  // Protocol
#define USB_TXTYPE6_PROTO_CTRL  0x0000  // Control
#define USB_TXTYPE6_PROTO_ISOC  0x0010  // Isochronous
#define USB_TXTYPE6_PROTO_BULK  0x0020  // Bulk
#define USB_TXTYPE6_PROTO_INT   0x0030  // Interrupt
#define USB_TXTYPE6_TEP_M       0x000F  // Target Endpoint Number
#define USB_TXTYPE6_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXINTERVAL6
// register.
//
//*****************************************************************************
#define USB_TXINTERVAL6_TXPOLL_M 0x00FF  // TX Polling
#define USB_TXINTERVAL6_NAKLMT_M 0x00FF  // NAK Limit
#define USB_TXINTERVAL6_TXPOLL_S 0
#define USB_TXINTERVAL6_NAKLMT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXTYPE6 register.
//
//*****************************************************************************
#define USB_RXTYPE6_SPEED_M     0x00C0  // Operating Speed
#define USB_RXTYPE6_SPEED_DFLT  0x0000  // Default
#define USB_RXTYPE6_SPEED_HIGH  0x0040  // High
#define USB_RXTYPE6_SPEED_FULL  0x0080  // Full
#define USB_RXTYPE6_SPEED_LOW   0x00C0  // Low
#define USB_RXTYPE6_PROTO_M     0x0030  // Protocol
#define USB_RXTYPE6_PROTO_CTRL  0x0000  // Control
#define USB_RXTYPE6_PROTO_ISOC  0x0010  // Isochronous
#define USB_RXTYPE6_PROTO_BULK  0x0020  // Bulk
#define USB_RXTYPE6_PROTO_INT   0x0030  // Interrupt
#define USB_RXTYPE6_TEP_M       0x000F  // Target Endpoint Number
#define USB_RXTYPE6_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXINTERVAL6
// register.
//
//*****************************************************************************
#define USB_RXINTERVAL6_TXPOLL_M 0x00FF  // RX Polling
#define USB_RXINTERVAL6_NAKLMT_M 0x00FF  // NAK Limit
#define USB_RXINTERVAL6_NAKLMT_S 0
#define USB_RXINTERVAL6_TXPOLL_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXMAXP7 register.
//
//*****************************************************************************
#define USB_TXMAXP7_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_TXMAXP7_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRL7 register.
//
//*****************************************************************************
#define USB_TXCSRL7_NAKTO       0x0080  // NAK Timeout
#define USB_TXCSRL7_CLRDT       0x0040  // Clear Data Toggle
#define USB_TXCSRL7_STALLED     0x0020  // Endpoint Stalled
#define USB_TXCSRL7_STALL       0x0010  // Send STALL
#define USB_TXCSRL7_SETUP       0x0010  // Setup Packet
#define USB_TXCSRL7_FLUSH       0x0008  // Flush FIFO
#define USB_TXCSRL7_ERROR       0x0004  // Error
#define USB_TXCSRL7_UNDRN       0x0004  // Underrun
#define USB_TXCSRL7_FIFONE      0x0002  // FIFO Not Empty
#define USB_TXCSRL7_TXRDY       0x0001  // Transmit Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXCSRH7 register.
//
//*****************************************************************************
#define USB_TXCSRH7_AUTOSET     0x0080  // Auto Set
#define USB_TXCSRH7_ISO         0x0040  // Isochronous Transfers
#define USB_TXCSRH7_MODE        0x0020  // Mode
#define USB_TXCSRH7_DMAEN       0x0010  // DMA Request Enable
#define USB_TXCSRH7_FDT         0x0008  // Force Data Toggle
#define USB_TXCSRH7_DMAMOD      0x0004  // DMA Request Mode
#define USB_TXCSRH7_DTWE        0x0002  // Data Toggle Write Enable
#define USB_TXCSRH7_DT          0x0001  // Data Toggle

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXMAXP7 register.
//
//*****************************************************************************
#define USB_RXMAXP7_MAXLOAD_M   0x07FF  // Maximum Payload
#define USB_RXMAXP7_MAXLOAD_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRL7 register.
//
//*****************************************************************************
#define USB_RXCSRL7_CLRDT       0x0080  // Clear Data Toggle
#define USB_RXCSRL7_STALLED     0x0040  // Endpoint Stalled
#define USB_RXCSRL7_REQPKT      0x0020  // Request Packet
#define USB_RXCSRL7_STALL       0x0020  // Send STALL
#define USB_RXCSRL7_FLUSH       0x0010  // Flush FIFO
#define USB_RXCSRL7_DATAERR     0x0008  // Data Error
#define USB_RXCSRL7_NAKTO       0x0008  // NAK Timeout
#define USB_RXCSRL7_ERROR       0x0004  // Error
#define USB_RXCSRL7_OVER        0x0004  // Overrun
#define USB_RXCSRL7_FULL        0x0002  // FIFO Full
#define USB_RXCSRL7_RXRDY       0x0001  // Receive Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCSRH7 register.
//
//*****************************************************************************
#define USB_RXCSRH7_AUTOCL      0x0080  // Auto Clear
#define USB_RXCSRH7_ISO         0x0040  // Isochronous Transfers
#define USB_RXCSRH7_AUTORQ      0x0040  // Auto Request
#define USB_RXCSRH7_DMAEN       0x0020  // DMA Request Enable
#define USB_RXCSRH7_PIDERR      0x0010  // PID Error
#define USB_RXCSRH7_DISNYET     0x0010  // Disable NYET
#define USB_RXCSRH7_DMAMOD      0x0008  // DMA Request Mode
#define USB_RXCSRH7_DTWE        0x0004  // Data Toggle Write Enable
#define USB_RXCSRH7_DT          0x0002  // Data Toggle
#define USB_RXCSRH7_INCOMPRX    0x0001  // Incomplete RX Transmission
                                            // Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXCOUNT7 register.
//
//*****************************************************************************
#define USB_RXCOUNT7_COUNT_M    0x1FFF  // Receive Packet Count
#define USB_RXCOUNT7_COUNT_S    0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXTYPE7 register.
//
//*****************************************************************************
#define USB_TXTYPE7_SPEED_M     0x00C0  // Operating Speed
#define USB_TXTYPE7_SPEED_DFLT  0x0000  // Default
#define USB_TXTYPE7_SPEED_HIGH  0x0040  // High
#define USB_TXTYPE7_SPEED_FULL  0x0080  // Full
#define USB_TXTYPE7_SPEED_LOW   0x00C0  // Low
#define USB_TXTYPE7_PROTO_M     0x0030  // Protocol
#define USB_TXTYPE7_PROTO_CTRL  0x0000  // Control
#define USB_TXTYPE7_PROTO_ISOC  0x0010  // Isochronous
#define USB_TXTYPE7_PROTO_BULK  0x0020  // Bulk
#define USB_TXTYPE7_PROTO_INT   0x0030  // Interrupt
#define USB_TXTYPE7_TEP_M       0x000F  // Target Endpoint Number
#define USB_TXTYPE7_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXINTERVAL7
// register.
//
//*****************************************************************************
#define USB_TXINTERVAL7_TXPOLL_M 0x00FF  // TX Polling
#define USB_TXINTERVAL7_NAKLMT_M 0x00FF  // NAK Limit
#define USB_TXINTERVAL7_NAKLMT_S 0
#define USB_TXINTERVAL7_TXPOLL_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXTYPE7 register.
//
//*****************************************************************************
#define USB_RXTYPE7_SPEED_M     0x00C0  // Operating Speed
#define USB_RXTYPE7_SPEED_DFLT  0x0000  // Default
#define USB_RXTYPE7_SPEED_HIGH  0x0040  // High
#define USB_RXTYPE7_SPEED_FULL  0x0080  // Full
#define USB_RXTYPE7_SPEED_LOW   0x00C0  // Low
#define USB_RXTYPE7_PROTO_M     0x0030  // Protocol
#define USB_RXTYPE7_PROTO_CTRL  0x0000  // Control
#define USB_RXTYPE7_PROTO_ISOC  0x0010  // Isochronous
#define USB_RXTYPE7_PROTO_BULK  0x0020  // Bulk
#define USB_RXTYPE7_PROTO_INT   0x0030  // Interrupt
#define USB_RXTYPE7_TEP_M       0x000F  // Target Endpoint Number
#define USB_RXTYPE7_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXINTERVAL7
// register.
//
//*****************************************************************************
#define USB_RXINTERVAL7_TXPOLL_M 0x00FF  // RX Polling
#define USB_RXINTERVAL7_NAKLMT_M 0x00FF  // NAK Limit
#define USB_RXINTERVAL7_NAKLMT_S 0
#define USB_RXINTERVAL7_TXPOLL_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMAINTR register.
//
//*****************************************************************************
#define USB_DMAINTR_CH7         0x0080  // Channel 7 DMA Interrupt
#define USB_DMAINTR_CH6         0x0040  // Channel 6 DMA Interrupt
#define USB_DMAINTR_CH5         0x0020  // Channel 5 DMA Interrupt
#define USB_DMAINTR_CH4         0x0010  // Channel 4 DMA Interrupt
#define USB_DMAINTR_CH3         0x0008  // Channel 3 DMA Interrupt
#define USB_DMAINTR_CH2         0x0004  // Channel 2 DMA Interrupt
#define USB_DMAINTR_CH1         0x0002  // Channel 1 DMA Interrupt
#define USB_DMAINTR_CH0         0x0001  // Channel 0 DMA Interrupt

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACTL0 register.
//
//*****************************************************************************
#define USB_DMACTL0_BRSTM_M     0x0600  // Burst Mode
#define USB_DMACTL0_BRSTM_ANY   0x0000  // Bursts of unspecified length
#define USB_DMACTL0_BRSTM_INC4  0x0200  // INCR4 or unspecified length
#define USB_DMACTL0_BRSTM_INC8  0x0400  // INCR8, INCR4 or unspecified
                                            // length
#define USB_DMACTL0_BRSTM_INC16 0x0600  // INCR16, INCR8, INCR4 or
                                            // unspecified length
#define USB_DMACTL0_ERR         0x0100  // Bus Error Bit
#define USB_DMACTL0_EP_M        0x00F0  // Endpoint number
#define USB_DMACTL0_IE          0x0008  // DMA Interrupt Enable
#define USB_DMACTL0_MODE        0x0004  // DMA Transfer Mode
#define USB_DMACTL0_DIR         0x0002  // DMA Direction
#define USB_DMACTL0_ENABLE      0x0001  // DMA Transfer Enable
#define USB_DMACTL0_EP_S        4

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMAADDR0 register.
//
//*****************************************************************************
#define USB_DMAADDR0_ADDR_M     0xFFFFFFFC  // DMA Address
#define USB_DMAADDR0_ADDR_S     2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACOUNT0
// register.
//
//*****************************************************************************
#define USB_DMACOUNT0_COUNT_M   0xFFFFFFFC  // DMA Count
#define USB_DMACOUNT0_COUNT_S   2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACTL1 register.
//
//*****************************************************************************
#define USB_DMACTL1_BRSTM_M     0x0600  // Burst Mode
#define USB_DMACTL1_BRSTM_ANY   0x0000  // Bursts of unspecified length
#define USB_DMACTL1_BRSTM_INC4  0x0200  // INCR4 or unspecified length
#define USB_DMACTL1_BRSTM_INC8  0x0400  // INCR8, INCR4 or unspecified
                                            // length
#define USB_DMACTL1_BRSTM_INC16 0x0600  // INCR16, INCR8, INCR4 or
                                            // unspecified length
#define USB_DMACTL1_ERR         0x0100  // Bus Error Bit
#define USB_DMACTL1_EP_M        0x00F0  // Endpoint number
#define USB_DMACTL1_IE          0x0008  // DMA Interrupt Enable
#define USB_DMACTL1_MODE        0x0004  // DMA Transfer Mode
#define USB_DMACTL1_DIR         0x0002  // DMA Direction
#define USB_DMACTL1_ENABLE      0x0001  // DMA Transfer Enable
#define USB_DMACTL1_EP_S        4

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMAADDR1 register.
//
//*****************************************************************************
#define USB_DMAADDR1_ADDR_M     0xFFFFFFFC  // DMA Address
#define USB_DMAADDR1_ADDR_S     2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACOUNT1
// register.
//
//*****************************************************************************
#define USB_DMACOUNT1_COUNT_M   0xFFFFFFFC  // DMA Count
#define USB_DMACOUNT1_COUNT_S   2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACTL2 register.
//
//*****************************************************************************
#define USB_DMACTL2_BRSTM_M     0x0600  // Burst Mode
#define USB_DMACTL2_BRSTM_ANY   0x0000  // Bursts of unspecified length
#define USB_DMACTL2_BRSTM_INC4  0x0200  // INCR4 or unspecified length
#define USB_DMACTL2_BRSTM_INC8  0x0400  // INCR8, INCR4 or unspecified
                                            // length
#define USB_DMACTL2_BRSTM_INC16 0x0600  // INCR16, INCR8, INCR4 or
                                            // unspecified length
#define USB_DMACTL2_ERR         0x0100  // Bus Error Bit
#define USB_DMACTL2_EP_M        0x00F0  // Endpoint number
#define USB_DMACTL2_IE          0x0008  // DMA Interrupt Enable
#define USB_DMACTL2_MODE        0x0004  // DMA Transfer Mode
#define USB_DMACTL2_DIR         0x0002  // DMA Direction
#define USB_DMACTL2_ENABLE      0x0001  // DMA Transfer Enable
#define USB_DMACTL2_EP_S        4

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMAADDR2 register.
//
//*****************************************************************************
#define USB_DMAADDR2_ADDR_M     0xFFFFFFFC  // DMA Address
#define USB_DMAADDR2_ADDR_S     2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACOUNT2
// register.
//
//*****************************************************************************
#define USB_DMACOUNT2_COUNT_M   0xFFFFFFFC  // DMA Count
#define USB_DMACOUNT2_COUNT_S   2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACTL3 register.
//
//*****************************************************************************
#define USB_DMACTL3_BRSTM_M     0x0600  // Burst Mode
#define USB_DMACTL3_BRSTM_ANY   0x0000  // Bursts of unspecified length
#define USB_DMACTL3_BRSTM_INC4  0x0200  // INCR4 or unspecified length
#define USB_DMACTL3_BRSTM_INC8  0x0400  // INCR8, INCR4 or unspecified
                                            // length
#define USB_DMACTL3_BRSTM_INC16 0x0600  // INCR16, INCR8, INCR4 or
                                            // unspecified length
#define USB_DMACTL3_ERR         0x0100  // Bus Error Bit
#define USB_DMACTL3_EP_M        0x00F0  // Endpoint number
#define USB_DMACTL3_IE          0x0008  // DMA Interrupt Enable
#define USB_DMACTL3_MODE        0x0004  // DMA Transfer Mode
#define USB_DMACTL3_DIR         0x0002  // DMA Direction
#define USB_DMACTL3_ENABLE      0x0001  // DMA Transfer Enable
#define USB_DMACTL3_EP_S        4

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMAADDR3 register.
//
//*****************************************************************************
#define USB_DMAADDR3_ADDR_M     0xFFFFFFFC  // DMA Address
#define USB_DMAADDR3_ADDR_S     2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACOUNT3
// register.
//
//*****************************************************************************
#define USB_DMACOUNT3_COUNT_M   0xFFFFFFFC  // DMA Count
#define USB_DMACOUNT3_COUNT_S   2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACTL4 register.
//
//*****************************************************************************
#define USB_DMACTL4_BRSTM_M     0x0600  // Burst Mode
#define USB_DMACTL4_BRSTM_ANY   0x0000  // Bursts of unspecified length
#define USB_DMACTL4_BRSTM_INC4  0x0200  // INCR4 or unspecified length
#define USB_DMACTL4_BRSTM_INC8  0x0400  // INCR8, INCR4 or unspecified
                                            // length
#define USB_DMACTL4_BRSTM_INC16 0x0600  // INCR16, INCR8, INCR4 or
                                            // unspecified length
#define USB_DMACTL4_ERR         0x0100  // Bus Error Bit
#define USB_DMACTL4_EP_M        0x00F0  // Endpoint number
#define USB_DMACTL4_IE          0x0008  // DMA Interrupt Enable
#define USB_DMACTL4_MODE        0x0004  // DMA Transfer Mode
#define USB_DMACTL4_DIR         0x0002  // DMA Direction
#define USB_DMACTL4_ENABLE      0x0001  // DMA Transfer Enable
#define USB_DMACTL4_EP_S        4

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMAADDR4 register.
//
//*****************************************************************************
#define USB_DMAADDR4_ADDR_M     0xFFFFFFFC  // DMA Address
#define USB_DMAADDR4_ADDR_S     2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACOUNT4
// register.
//
//*****************************************************************************
#define USB_DMACOUNT4_COUNT_M   0xFFFFFFFC  // DMA Count
#define USB_DMACOUNT4_COUNT_S   2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACTL5 register.
//
//*****************************************************************************
#define USB_DMACTL5_BRSTM_M     0x0600  // Burst Mode
#define USB_DMACTL5_BRSTM_ANY   0x0000  // Bursts of unspecified length
#define USB_DMACTL5_BRSTM_INC4  0x0200  // INCR4 or unspecified length
#define USB_DMACTL5_BRSTM_INC8  0x0400  // INCR8, INCR4 or unspecified
                                            // length
#define USB_DMACTL5_BRSTM_INC16 0x0600  // INCR16, INCR8, INCR4 or
                                            // unspecified length
#define USB_DMACTL5_ERR         0x0100  // Bus Error Bit
#define USB_DMACTL5_EP_M        0x00F0  // Endpoint number
#define USB_DMACTL5_IE          0x0008  // DMA Interrupt Enable
#define USB_DMACTL5_MODE        0x0004  // DMA Transfer Mode
#define USB_DMACTL5_DIR         0x0002  // DMA Direction
#define USB_DMACTL5_ENABLE      0x0001  // DMA Transfer Enable
#define USB_DMACTL5_EP_S        4

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMAADDR5 register.
//
//*****************************************************************************
#define USB_DMAADDR5_ADDR_M     0xFFFFFFFC  // DMA Address
#define USB_DMAADDR5_ADDR_S     2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACOUNT5
// register.
//
//*****************************************************************************
#define USB_DMACOUNT5_COUNT_M   0xFFFFFFFC  // DMA Count
#define USB_DMACOUNT5_COUNT_S   2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACTL6 register.
//
//*****************************************************************************
#define USB_DMACTL6_BRSTM_M     0x0600  // Burst Mode
#define USB_DMACTL6_BRSTM_ANY   0x0000  // Bursts of unspecified length
#define USB_DMACTL6_BRSTM_INC4  0x0200  // INCR4 or unspecified length
#define USB_DMACTL6_BRSTM_INC8  0x0400  // INCR8, INCR4 or unspecified
                                            // length
#define USB_DMACTL6_BRSTM_INC16 0x0600  // INCR16, INCR8, INCR4 or
                                            // unspecified length
#define USB_DMACTL6_ERR         0x0100  // Bus Error Bit
#define USB_DMACTL6_EP_M        0x00F0  // Endpoint number
#define USB_DMACTL6_IE          0x0008  // DMA Interrupt Enable
#define USB_DMACTL6_MODE        0x0004  // DMA Transfer Mode
#define USB_DMACTL6_DIR         0x0002  // DMA Direction
#define USB_DMACTL6_ENABLE      0x0001  // DMA Transfer Enable
#define USB_DMACTL6_EP_S        4

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMAADDR6 register.
//
//*****************************************************************************
#define USB_DMAADDR6_ADDR_M     0xFFFFFFFC  // DMA Address
#define USB_DMAADDR6_ADDR_S     2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACOUNT6
// register.
//
//*****************************************************************************
#define USB_DMACOUNT6_COUNT_M   0xFFFFFFFC  // DMA Count
#define USB_DMACOUNT6_COUNT_S   2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACTL7 register.
//
//*****************************************************************************
#define USB_DMACTL7_BRSTM_M     0x0600  // Burst Mode
#define USB_DMACTL7_BRSTM_ANY   0x0000  // Bursts of unspecified length
#define USB_DMACTL7_BRSTM_INC4  0x0200  // INCR4 or unspecified length
#define USB_DMACTL7_BRSTM_INC8  0x0400  // INCR8, INCR4 or unspecified
                                            // length
#define USB_DMACTL7_BRSTM_INC16 0x0600  // INCR16, INCR8, INCR4 or
                                            // unspecified length
#define USB_DMACTL7_ERR         0x0100  // Bus Error Bit
#define USB_DMACTL7_EP_M        0x00F0  // Endpoint number
#define USB_DMACTL7_IE          0x0008  // DMA Interrupt Enable
#define USB_DMACTL7_MODE        0x0004  // DMA Transfer Mode
#define USB_DMACTL7_DIR         0x0002  // DMA Direction
#define USB_DMACTL7_ENABLE      0x0001  // DMA Transfer Enable
#define USB_DMACTL7_EP_S        4

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMAADDR7 register.
//
//*****************************************************************************
#define USB_DMAADDR7_ADDR_M     0xFFFFFFFC  // DMA Address
#define USB_DMAADDR7_ADDR_S     2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DMACOUNT7
// register.
//
//*****************************************************************************
#define USB_DMACOUNT7_COUNT_M   0xFFFFFFFC  // DMA Count
#define USB_DMACOUNT7_COUNT_S   2

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RQPKTCOUNT1
// register.
//
//*****************************************************************************
#define USB_RQPKTCOUNT1_M       0xFFFF  // Block Transfer Packet Count
#define USB_RQPKTCOUNT1_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RQPKTCOUNT2
// register.
//
//*****************************************************************************
#define USB_RQPKTCOUNT2_M       0xFFFF  // Block Transfer Packet Count
#define USB_RQPKTCOUNT2_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RQPKTCOUNT3
// register.
//
//*****************************************************************************
#define USB_RQPKTCOUNT3_M       0xFFFF  // Block Transfer Packet Count
#define USB_RQPKTCOUNT3_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RQPKTCOUNT4
// register.
//
//*****************************************************************************
#define USB_RQPKTCOUNT4_COUNT_M 0xFFFF  // Block Transfer Packet Count
#define USB_RQPKTCOUNT4_COUNT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RQPKTCOUNT5
// register.
//
//*****************************************************************************
#define USB_RQPKTCOUNT5_COUNT_M 0xFFFF  // Block Transfer Packet Count
#define USB_RQPKTCOUNT5_COUNT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RQPKTCOUNT6
// register.
//
//*****************************************************************************
#define USB_RQPKTCOUNT6_COUNT_M 0xFFFF  // Block Transfer Packet Count
#define USB_RQPKTCOUNT6_COUNT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RQPKTCOUNT7
// register.
//
//*****************************************************************************
#define USB_RQPKTCOUNT7_COUNT_M 0xFFFF  // Block Transfer Packet Count
#define USB_RQPKTCOUNT7_COUNT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_RXDPKTBUFDIS
// register.
//
//*****************************************************************************
#define USB_RXDPKTBUFDIS_EP7    0x0080  // EP7 RX Double-Packet Buffer
                                            // Disable
#define USB_RXDPKTBUFDIS_EP6    0x0040  // EP6 RX Double-Packet Buffer
                                            // Disable
#define USB_RXDPKTBUFDIS_EP5    0x0020  // EP5 RX Double-Packet Buffer
                                            // Disable
#define USB_RXDPKTBUFDIS_EP4    0x0010  // EP4 RX Double-Packet Buffer
                                            // Disable
#define USB_RXDPKTBUFDIS_EP3    0x0008  // EP3 RX Double-Packet Buffer
                                            // Disable
#define USB_RXDPKTBUFDIS_EP2    0x0004  // EP2 RX Double-Packet Buffer
                                            // Disable
#define USB_RXDPKTBUFDIS_EP1    0x0002  // EP1 RX Double-Packet Buffer
                                            // Disable

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_TXDPKTBUFDIS
// register.
//
//*****************************************************************************
#define USB_TXDPKTBUFDIS_EP7    0x0080  // EP7 TX Double-Packet Buffer
                                            // Disable
#define USB_TXDPKTBUFDIS_EP6    0x0040  // EP6 TX Double-Packet Buffer
                                            // Disable
#define USB_TXDPKTBUFDIS_EP5    0x0020  // EP5 TX Double-Packet Buffer
                                            // Disable
#define USB_TXDPKTBUFDIS_EP4    0x0010  // EP4 TX Double-Packet Buffer
                                            // Disable
#define USB_TXDPKTBUFDIS_EP3    0x0008  // EP3 TX Double-Packet Buffer
                                            // Disable
#define USB_TXDPKTBUFDIS_EP2    0x0004  // EP2 TX Double-Packet Buffer
                                            // Disable
#define USB_TXDPKTBUFDIS_EP1    0x0002  // EP1 TX Double-Packet Buffer
                                            // Disable

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_CTO register.
//
//*****************************************************************************
#define USB_CTO_CCTV_M          0xFFFF  // Configurable Chirp Timeout Value
#define USB_CTO_CCTV_S          0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_HHSRTN register.
//
//*****************************************************************************
#define USB_HHSRTN_HHSRTN_M     0xFFFF  // HIgh Speed to UTM Operating
                                            // Delay
#define USB_HHSRTN_HHSRTN_S     0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_HSBT register.
//
//*****************************************************************************
#define USB_HSBT_HSBT_M         0x000F  // High Speed Timeout Adder
#define USB_HSBT_HSBT_S         0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_LPMATTR register.
//
//*****************************************************************************
#define USB_LPMATTR_ENDPT_M     0xF000  // Endpoint
#define USB_LPMATTR_RMTWAK      0x0100  // Remote Wake
#define USB_LPMATTR_HIRD_M      0x00F0  // Host Initiated Resume Duration
#define USB_LPMATTR_LS_M        0x000F  // Link State
#define USB_LPMATTR_LS_L1       0x0001  // Sleep State (L1)
#define USB_LPMATTR_ENDPT_S     12
#define USB_LPMATTR_HIRD_S      4

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_LPMCNTRL register.
//
//*****************************************************************************
#define USB_LPMCNTRL_NAK        0x0010  // LPM NAK
#define USB_LPMCNTRL_EN_M       0x000C  // LPM Enable
#define USB_LPMCNTRL_EN_NONE    0x0000  // LPM and Extended transactions
                                            // are not supported. In this case,
                                            // the USB does not respond to LPM
                                            // transactions and LPM
                                            // transactions cause a timeout
#define USB_LPMCNTRL_EN_EXT     0x0004  // LPM is not supported but
                                            // extended transactions are
                                            // supported. In this case, the USB
                                            // does respond to an LPM
                                            // transaction with a STALL
#define USB_LPMCNTRL_EN_LPMEXT  0x000C  // The USB supports LPM extended
                                            // transactions. In this case, the
                                            // USB responds with a NYET or an
                                            // ACK as determined by the value
                                            // of TXLPM and other conditions
#define USB_LPMCNTRL_RES        0x0002  // LPM Resume
#define USB_LPMCNTRL_TXLPM      0x0001  // Transmit LPM Transaction Enable

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_LPMIM register.
//
//*****************************************************************************
#define USB_LPMIM_ERR           0x0020  // LPM Error Interrupt Mask
#define USB_LPMIM_RES           0x0010  // LPM Resume Interrupt Mask
#define USB_LPMIM_NC            0x0008  // LPM NC Interrupt Mask
#define USB_LPMIM_ACK           0x0004  // LPM ACK Interrupt Mask
#define USB_LPMIM_NY            0x0002  // LPM NY Interrupt Mask
#define USB_LPMIM_STALL         0x0001  // LPM STALL Interrupt Mask

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_LPMRIS register.
//
//*****************************************************************************
#define USB_LPMRIS_ERR          0x0020  // LPM Interrupt Status
#define USB_LPMRIS_RES          0x0010  // LPM Resume Interrupt Status
#define USB_LPMRIS_NC           0x0008  // LPM NC Interrupt Status
#define USB_LPMRIS_ACK          0x0004  // LPM ACK Interrupt Status
#define USB_LPMRIS_NY           0x0002  // LPM NY Interrupt Status
#define USB_LPMRIS_LPMST        0x0001  // LPM STALL Interrupt Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_LPMFADDR register.
//
//*****************************************************************************
#define USB_LPMFADDR_ADDR_M     0x007F  // LPM Function Address
#define USB_LPMFADDR_ADDR_S     0

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_EPC register.
//
//*****************************************************************************
#define USB_EPC_PFLTACT_M       0x0300  // Power Fault Action
#define USB_EPC_PFLTACT_UNCHG   0x0000  // Unchanged
#define USB_EPC_PFLTACT_TRIS    0x0100  // Tristate
#define USB_EPC_PFLTACT_LOW     0x0200  // Low
#define USB_EPC_PFLTACT_HIGH    0x0300  // High
#define USB_EPC_PFLTAEN         0x0040  // Power Fault Action Enable
#define USB_EPC_PFLTSEN_HIGH    0x0020  // Power Fault Sense
#define USB_EPC_PFLTEN          0x0010  // Power Fault Input Enable
#define USB_EPC_EPENDE          0x0004  // EPEN Drive Enable
#define USB_EPC_EPEN_M          0x0003  // External Power Supply Enable
                                            // Configuration
#define USB_EPC_EPEN_LOW        0x0000  // Power Enable Active Low
#define USB_EPC_EPEN_HIGH       0x0001  // Power Enable Active High
#define USB_EPC_EPEN_VBLOW      0x0002  // Power Enable High if VBUS Low
                                            // (OTG only)
#define USB_EPC_EPEN_VBHIGH     0x0003  // Power Enable High if VBUS High
                                            // (OTG only)

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_EPCRIS register.
//
//*****************************************************************************
#define USB_EPCRIS_PF           0x0001  // USB Power Fault Interrupt Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_EPCIM register.
//
//*****************************************************************************
#define USB_EPCIM_PF            0x0001  // USB Power Fault Interrupt Mask

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_EPCISC register.
//
//*****************************************************************************
#define USB_EPCISC_PF           0x0001  // USB Power Fault Interrupt Status
                                            // and Clear

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DRRIS register.
//
//*****************************************************************************
#define USB_DRRIS_RESUME        0x0001  // RESUME Interrupt Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DRIM register.
//
//*****************************************************************************
#define USB_DRIM_RESUME         0x0001  // RESUME Interrupt Mask

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_DRISC register.
//
//*****************************************************************************
#define USB_DRISC_RESUME        0x0001  // RESUME Interrupt Status and
                                            // Clear

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_GPCS register.
//
//*****************************************************************************
#define USB_GPCS_DEVMOD_M       0x0007  // Device Mode
#define USB_GPCS_DEVMOD_OTG     0x0000  // Use USB0VBUS and USB0ID pin
#define USB_GPCS_DEVMOD_HOST    0x0002  // Force USB0VBUS and USB0ID low
#define USB_GPCS_DEVMOD_DEV     0x0003  // Force USB0VBUS and USB0ID high
#define USB_GPCS_DEVMOD_HOSTVBUS                                              \
                                0x0004  // Use USB0VBUS and force USB0ID
                                            // low
#define USB_GPCS_DEVMOD_DEVVBUS 0x0005  // Use USB0VBUS and force USB0ID
                                            // high

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_VDC register.
//
//*****************************************************************************
#define USB_VDC_VBDEN           0x0001  // VBUS Droop Enable

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_VDCRIS register.
//
//*****************************************************************************
#define USB_VDCRIS_VD           0x0001  // VBUS Droop Raw Interrupt Status

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_VDCIM register.
//
//*****************************************************************************
#define USB_VDCIM_VD            0x0001  // VBUS Droop Interrupt Mask

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_VDCISC register.
//
//*****************************************************************************
#define USB_VDCISC_VD           0x0001  // VBUS Droop Interrupt Status and
                                            // Clear

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_PP register.
//
//*****************************************************************************
#define USB_PP_ECNT_M           0xFF00  // Endpoint Count
#define USB_PP_USB_M            0x00C0  // USB Capability
#define USB_PP_USB_DEVICE       0x0040  // DEVICE
#define USB_PP_USB_HOSTDEVICE   0x0080  // HOST
#define USB_PP_USB_OTG          0x00C0  // OTG
#define USB_PP_ULPI             0x0020  // ULPI Present
#define USB_PP_PHY              0x0010  // PHY Present
#define USB_PP_TYPE_M           0x000F  // Controller Type
#define USB_PP_TYPE_0           0x0000  // The first-generation USB
                                            // controller
#define USB_PP_TYPE_1           0x0001  // The second-generation USB
                                            // controller revision
#define USB_PP_ECNT_S           8

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_PC register.
//
//*****************************************************************************
#define USB_PC_ULPIEN           0x00010000  // ULPI Enable

//*****************************************************************************
//
// The following are defines for the bit fields in the USB_O_CC register.
//
//*****************************************************************************
#define USB_CC_CLKEN            0x0200  // USB Clock Enable
#define USB_CC_CSD              0x0100  // Clock Source/Direction
#define USB_CC_CLKDIV_M         0x000F  // PLL Clock Divisor
#define USB_CC_CLKDIV_S         0

#ifdef __cplusplus
 }
#endif

#endif
