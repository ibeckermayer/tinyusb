/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Koji Kitayama
 * Portions copyrighted (c) 2021 Roland Winistoerfer
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

#include "tusb_option.h"

// Since TinyUSB doesn't use SOF for now, and this interrupt too often (1ms interval)
// We disable SOF for now until needed later on
#define USE_SOF     0

#if CFG_TUD_ENABLED && defined(TUP_USBIP_RUSB2)

#include "device/dcd.h"
#include "rusb2_type.h"

#if !defined(CFG_TUSB_RHPORT0_MODE) && !defined(CFG_TUSB_RHPORT1_MODE)
// fallback
#define CFG_TUSB_RHPORT0_MODE   OPT_MODE_DEVICE
#define CFG_TUSB_RHPORT1_MODE   0
#endif

#if TU_CHECK_MCU(OPT_MCU_RX63X, OPT_MCU_RX65X, OPT_MCU_RX72N)
  #include "rusb2_rx.h"
#elif TU_CHECK_MCU(OPT_MCU_RAXXX)
  #include "rusb2_ra.h"
  #if defined(RENESAS_CORTEX_M23)
    #define D0FIFO CFIFO
    #define D0FIFOSEL CFIFOSEL
    #define D0FIFOSEL_b CFIFOSEL_b
    #define D1FIFOSEL CFIFOSEL
    #define D1FIFOSEL_b CFIFOSEL_b
    #define D0FIFOCTR CFIFOCTR
    #define D0FIFOCTR_b CFIFOCTR_b
  #endif
#else
  #error "Unsupported MCU"
#endif

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM
//--------------------------------------------------------------------+

/* LINK core registers */
#if defined(__CCRX__)
  #define RUSB2 ((rusb2_reg_t __evenaccess*) RUSB2_REG_BASE)
#elif defined(__RX__)
  #define RUSB2 ((rusb2_reg_t*) RUSB2_REG_BASE)
#elif (CFG_TUSB_RHPORT1_MODE & OPT_MODE_DEVICE)
  #define RUSB2 ((R_USB_HS0_Type*)R_USB_HS0_BASE)
#else
  #define RUSB2 ((R_USB_FS0_Type*)R_USB_FS0_BASE)
#endif

/* Start of definition of packed structs (used by the CCRX toolchain) */
TU_ATTR_PACKED_BEGIN
TU_ATTR_BIT_FIELD_ORDER_BEGIN

typedef struct TU_ATTR_PACKED {
  union {
    struct {
      uint16_t      : 8;
      uint16_t TRCLR: 1;
      uint16_t TRENB: 1;
      uint16_t      : 0;
    };
    uint16_t TRE;
  };
  uint16_t TRN;
} reg_pipetre_t;

typedef union TU_ATTR_PACKED {
  struct {
    volatile uint8_t u8;
    volatile uint8_t reserved8;
  };
  volatile uint16_t u16;
} hw_fifo_t;

typedef union TU_ATTR_PACKED {
  struct {
    volatile uint8_t reserved8[3];
    volatile uint8_t u8;
  };
  struct {
    volatile uint16_t reserved16;
    volatile uint16_t u16;
  };
  volatile uint32_t u32;
} hw_fifo32_t;

typedef struct TU_ATTR_PACKED
{
  void      *buf;      /* the start address of a transfer data buffer */
  uint16_t  length;    /* the number of bytes in the buffer */
  uint16_t  remaining; /* the number of bytes remaining in the buffer */
  struct {
    uint32_t ep  : 8;  /* an assigned endpoint address */
    uint32_t ff  : 1;  /* `buf` is TU_FUFO or POD */
    uint32_t     : 0;
  };
} pipe_state_t;

TU_ATTR_PACKED_END  // End of definition of packed structs (used by the CCRX toolchain)
TU_ATTR_BIT_FIELD_ORDER_END

// Transfer conditions specifiable for each pipe:
// - Pipe 0: Control transfer with 64-byte single buffer
// - Pipes 1 and 2: Bulk isochronous transfer continuous transfer mode with programmable buffer size up
//   to 2 KB and optional double buffer
// - Pipes 3 to 5: Bulk transfer continuous transfer mode with programmable buffer size up to 2 KB and
//   optional double buffer
// - Pipes 6 to 9: Interrupt transfer with 64-byte single buffer
enum {
  PIPE_1ST_BULK = 3,
  PIPE_1ST_INTERRUPT = 6,
  PIPE_COUNT = 10,
};

typedef struct
{
  pipe_state_t pipe[10];
  uint8_t ep[2][16];   /* a lookup table for a pipe index from an endpoint address */
} dcd_data_t;

static dcd_data_t _dcd;

//--------------------------------------------------------------------+
// INTERNAL OBJECT & FUNCTION DECLARATION
//--------------------------------------------------------------------+

TU_ATTR_ALWAYS_INLINE static inline bool is_highspeed(uint8_t rhport) {
  return rhport == 1;
}

static unsigned find_pipe(unsigned xfer)
{
  switch (xfer) {
    case TUSB_XFER_ISOCHRONOUS:
      for (int i = 1; i < PIPE_1ST_BULK; ++i) {
        if (0 == _dcd.pipe[i].ep) return i;
      }
      break;

    case TUSB_XFER_BULK:
      for (int i = PIPE_1ST_BULK; i < PIPE_1ST_INTERRUPT; ++i) {
        if (0 == _dcd.pipe[i].ep) return i;
      }
      for (int i = 1; i < PIPE_1ST_BULK; ++i) {
        if (0 == _dcd.pipe[i].ep) return i;
      }
      break;

    case TUSB_XFER_INTERRUPT:
      for (int i = PIPE_1ST_INTERRUPT; i < PIPE_COUNT; ++i) {
        if (0 == _dcd.pipe[i].ep) return i;
      }
      break;

    default:
      /* No support for control transfer */
      break;
  }
  return 0;
}

static volatile uint16_t* get_pipectr(unsigned num)
{
  if (num) {
    return (volatile uint16_t*)&(RUSB2->PIPE_CTR[num - 1]);
  } else {
    return (volatile uint16_t*)&(RUSB2->DCPCTR);
  }
}

static volatile reg_pipetre_t* get_pipetre(unsigned num)
{
  volatile reg_pipetre_t* tre = NULL;
  if ((1 <= num) && (num <= 5)) {
    tre = (volatile reg_pipetre_t*)&(RUSB2->PIPE_TR[num - 1].E);
  }
  return tre;
}

static volatile uint16_t* ep_addr_to_pipectr(uint8_t rhport, unsigned ep_addr)
{
  (void)rhport;
  const unsigned epn = tu_edpt_number(ep_addr);
  if (epn) {
    const unsigned dir = tu_edpt_dir(ep_addr);
    const unsigned num = _dcd.ep[dir][epn];
    return get_pipectr(num);
  } else {
    return get_pipectr(0);
  }
}

static unsigned edpt0_max_packet_size(void)
{
  return RUSB2->DCPMAXP_b.MXPS;
}

static unsigned edpt_max_packet_size(unsigned num)
{
  RUSB2->PIPESEL = num;
  return RUSB2->PIPEMAXP;
}

static inline void pipe_wait_for_ready(unsigned num)
{
  while (RUSB2->D0FIFOSEL_b.CURPIPE != num) ;
  while (!RUSB2->D0FIFOCTR_b.FRDY) ;
}

static void pipe_write_packet(void *buf, volatile void *fifo, unsigned len)
{
#if (CFG_TUSB_RHPORT1_MODE & OPT_MODE_DEVICE)
  volatile hw_fifo32_t *reg = (volatile hw_fifo32_t*) fifo;
#else
  volatile hw_fifo_t *reg = (volatile hw_fifo_t*) fifo;
#endif
  uintptr_t addr = (uintptr_t)buf;
  while (len >= 2) {
    reg->u16 = *(const uint16_t *)addr;
    addr += 2;
    len  -= 2;
  }
  if (len > 0) {
    reg->u8 = *(const uint8_t *)addr;
    ++addr;
  }
}

static void pipe_read_packet(void *buf, volatile void *fifo, unsigned len)
{
  uint8_t *p   = (uint8_t*)buf;
  volatile uint8_t *reg = (volatile uint8_t*)fifo;  /* byte access is always at base register address */
  while (len--) *p++ = *reg;
}

static void pipe_read_write_packet_ff(tu_fifo_t *f, volatile void *fifo, unsigned len, unsigned dir)
{
  static const struct {
    void (*tu_fifo_get_info)(tu_fifo_t *f, tu_fifo_buffer_info_t *info);
    void (*tu_fifo_advance)(tu_fifo_t *f, uint16_t n);
    void (*pipe_read_write)(void *buf, volatile void *fifo, unsigned len);
  } ops[] = {
    /* OUT */ {tu_fifo_get_write_info,tu_fifo_advance_write_pointer,pipe_read_packet},
    /* IN  */ {tu_fifo_get_read_info, tu_fifo_advance_read_pointer, pipe_write_packet},
  };
  tu_fifo_buffer_info_t info;
  ops[dir].tu_fifo_get_info(f, &info);
  unsigned total_len = len;
  len = TU_MIN(total_len, info.len_lin);
  ops[dir].pipe_read_write(info.ptr_lin, fifo, len);
  unsigned rem = total_len - len;
  if (rem) {
    len = TU_MIN(rem, info.len_wrap);
    ops[dir].pipe_read_write(info.ptr_wrap, fifo, len);
    rem -= len;
  }
  ops[dir].tu_fifo_advance(f, total_len - rem);
}

static bool pipe0_xfer_in(void)
{
  pipe_state_t *pipe = &_dcd.pipe[0];
  const unsigned rem = pipe->remaining;
  if (!rem) {
    pipe->buf = NULL;
    return true;
  }
  const unsigned mps = edpt0_max_packet_size();
  const unsigned len = TU_MIN(mps, rem);
  void          *buf = pipe->buf;
  if (len) {
    if (pipe->ff) {
      pipe_read_write_packet_ff((tu_fifo_t*)buf, (volatile void*)&RUSB2->CFIFO, len, TUSB_DIR_IN);
    } else {
      pipe_write_packet(buf, (volatile void*)&RUSB2->CFIFO, len);
      pipe->buf = (uint8_t*)buf + len;
    }
  }
  if (len < mps) {
    RUSB2->CFIFOCTR = RUSB2_CFIFOCTR_BVAL_Msk;
  }
  pipe->remaining = rem - len;
  return false;
}

static bool pipe0_xfer_out(void)
{
  pipe_state_t *pipe = &_dcd.pipe[0];
  const unsigned rem = pipe->remaining;

  const unsigned mps = edpt0_max_packet_size();
  const unsigned vld = RUSB2->CFIFOCTR_b.DTLN;
  const unsigned len = TU_MIN(TU_MIN(rem, mps), vld);
  void          *buf = pipe->buf;
  if (len) {
    if (pipe->ff) {
      pipe_read_write_packet_ff((tu_fifo_t*)buf, (volatile void*)&RUSB2->CFIFO, len, TUSB_DIR_OUT);
    } else {
      pipe_read_packet(buf, (volatile void*)&RUSB2->CFIFO, len);
      pipe->buf = (uint8_t*)buf + len;
    }
  }
  if (len < mps) {
    RUSB2->CFIFOCTR = RUSB2_CFIFOCTR_BCLR_Msk;
  }
  pipe->remaining = rem - len;
  if ((len < mps) || (rem == len)) {
    pipe->buf = NULL;
    return true;
  }
  return false;
}

static bool pipe_xfer_in(unsigned num)
{
  pipe_state_t  *pipe = &_dcd.pipe[num];
  const unsigned rem  = pipe->remaining;

  if (!rem) {
    pipe->buf = NULL;
    return true;
  }

  RUSB2->D0FIFOSEL = num | RUSB2_FIFOSEL_MBW_16BIT | (TU_BYTE_ORDER == TU_BIG_ENDIAN ? RUSB2_FIFOSEL_BIGEND : 0);
  const unsigned mps  = edpt_max_packet_size(num);
  pipe_wait_for_ready(num);
  const unsigned len  = TU_MIN(rem, mps);
  void          *buf  = pipe->buf;
  if (len) {
    if (pipe->ff) {
      pipe_read_write_packet_ff((tu_fifo_t*)buf, (volatile void*)&RUSB2->D0FIFO, len, TUSB_DIR_IN);
    } else {
      pipe_write_packet(buf, (volatile void*)&RUSB2->D0FIFO, len);
      pipe->buf = (uint8_t*)buf + len;
    }
  }
  if (len < mps) {
    RUSB2->D0FIFOCTR = RUSB2_CFIFOCTR_BVAL_Msk;
  }
  RUSB2->D0FIFOSEL = 0;
  while (RUSB2->D0FIFOSEL_b.CURPIPE) {} /* if CURPIPE bits changes, check written value */
  pipe->remaining = rem - len;
  return false;
}

static bool pipe_xfer_out(unsigned num)
{
  pipe_state_t  *pipe = &_dcd.pipe[num];
  const unsigned rem  = pipe->remaining;

  RUSB2->D0FIFOSEL = num | RUSB2_FIFOSEL_MBW_8BIT;
  const unsigned mps  = edpt_max_packet_size(num);
  pipe_wait_for_ready(num);
  const unsigned vld  = RUSB2->D0FIFOCTR_b.DTLN;
  const unsigned len  = TU_MIN(TU_MIN(rem, mps), vld);
  void          *buf  = pipe->buf;
  if (len) {
    if (pipe->ff) {
      pipe_read_write_packet_ff((tu_fifo_t*)buf, (volatile void*)&RUSB2->D0FIFO, len, TUSB_DIR_OUT);
    } else {
      pipe_read_packet(buf, (volatile void*)&RUSB2->D0FIFO, len);
      pipe->buf = (uint8_t*)buf + len;
    }
  }
  if (len < mps) {
    RUSB2->D0FIFOCTR = RUSB2_CFIFOCTR_BCLR_Msk;
  }
  RUSB2->D0FIFOSEL = 0;
  while (RUSB2->D0FIFOSEL_b.CURPIPE) {} /* if CURPIPE bits changes, check written value */
  pipe->remaining = rem - len;
  if ((len < mps) || (rem == len)) {
    pipe->buf = NULL;
    return NULL != buf;
  }
  return false;
}

static void process_setup_packet(uint8_t rhport)
{
  uint16_t setup_packet[4];
  if (0 == (RUSB2->INTSTS0 & RUSB2_INTSTS0_VALID_Msk)) return;
  RUSB2->CFIFOCTR = RUSB2_CFIFOCTR_BCLR_Msk;
  setup_packet[0] = tu_le16toh(RUSB2->USBREQ);
  setup_packet[1] = RUSB2->USBVAL;
  setup_packet[2] = RUSB2->USBINDX;
  setup_packet[3] = RUSB2->USBLENG;
  RUSB2->INTSTS0 = ~((uint16_t)RUSB2_INTSTS0_VALID_Msk);
  dcd_event_setup_received(rhport, (const uint8_t*)&setup_packet[0], true);
}

static void process_status_completion(uint8_t rhport)
{
  uint8_t ep_addr;
  /* Check the data stage direction */
  if (RUSB2->CFIFOSEL & RUSB2_CFIFOSEL_ISEL_WRITE) {
    /* IN transfer. */
    ep_addr = tu_edpt_addr(0, TUSB_DIR_IN);
  } else {
    /* OUT transfer. */
    ep_addr = tu_edpt_addr(0, TUSB_DIR_OUT);
  }
  dcd_event_xfer_complete(rhport, ep_addr, 0, XFER_RESULT_SUCCESS, true);
}

static bool process_pipe0_xfer(int buffer_type, uint8_t ep_addr, void* buffer, uint16_t total_bytes)
{
  /* configure fifo direction and access unit settings */
  if (ep_addr) { /* IN, 2 bytes */
    RUSB2->CFIFOSEL = RUSB2_CFIFOSEL_ISEL_WRITE | RUSB2_FIFOSEL_MBW_16BIT |
                         (TU_BYTE_ORDER == TU_BIG_ENDIAN ? RUSB2_FIFOSEL_BIGEND : 0);
    while (!(RUSB2->CFIFOSEL & RUSB2_CFIFOSEL_ISEL_WRITE)) ;
  } else { /* OUT, a byte */
    RUSB2->CFIFOSEL = RUSB2_FIFOSEL_MBW_8BIT;
    while (RUSB2->CFIFOSEL & RUSB2_CFIFOSEL_ISEL_WRITE) ;
  }

  pipe_state_t *pipe = &_dcd.pipe[0];
  pipe->ff        = buffer_type;
  pipe->length    = total_bytes;
  pipe->remaining = total_bytes;
  if (total_bytes) {
    pipe->buf     = buffer;
    if (ep_addr) { /* IN */
      TU_ASSERT(RUSB2->DCPCTR_b.BSTS && (RUSB2->USBREQ & 0x80));
      pipe0_xfer_in();
    }
    RUSB2->DCPCTR = RUSB2_PIPE_CTR_PID_BUF;
  } else {
    /* ZLP */
    pipe->buf        = NULL;
    RUSB2->DCPCTR = RUSB2_DCPCTR_CCPL_Msk | RUSB2_PIPE_CTR_PID_BUF;
  }
  return true;
}

static bool process_pipe_xfer(int buffer_type, uint8_t ep_addr, void* buffer, uint16_t total_bytes)
{
  const unsigned epn = tu_edpt_number(ep_addr);
  const unsigned dir = tu_edpt_dir(ep_addr);
  const unsigned num = _dcd.ep[dir][epn];

  TU_ASSERT(num);

  pipe_state_t *pipe  = &_dcd.pipe[num];
  pipe->ff        = buffer_type;
  pipe->buf       = buffer;
  pipe->length    = total_bytes;
  pipe->remaining = total_bytes;
  if (dir) { /* IN */
    if (total_bytes) {
      pipe_xfer_in(num);
    } else { /* ZLP */
      RUSB2->D0FIFOSEL = num;
      pipe_wait_for_ready(num);
      RUSB2->D0FIFOCTR = RUSB2_CFIFOCTR_BVAL_Msk;
      RUSB2->D0FIFOSEL = 0;
      while (RUSB2->D0FIFOSEL_b.CURPIPE) ; /* if CURPIPE bits changes, check written value */
    }
  } else {
#if defined(__CCRX__)
    __evenaccess volatile reg_pipetre_t *pt = get_pipetre(num);
#else
    volatile reg_pipetre_t *pt = get_pipetre(num);
#endif
    if (pt) {
      const unsigned     mps = edpt_max_packet_size(num);
      volatile uint16_t *ctr = get_pipectr(num);
      if (*ctr & 0x3) *ctr = RUSB2_PIPE_CTR_PID_NAK;
      pt->TRE   = TU_BIT(8);
      pt->TRN   = (total_bytes + mps - 1) / mps;
      pt->TRENB = 1;
      *ctr = RUSB2_PIPE_CTR_PID_BUF;
    }
  }
  //  TU_LOG1("X %x %d %d\r\n", ep_addr, total_bytes, buffer_type);
  return true;
}

static bool process_edpt_xfer(int buffer_type, uint8_t ep_addr, void* buffer, uint16_t total_bytes)
{
  const unsigned epn = tu_edpt_number(ep_addr);
  if (0 == epn) {
    return process_pipe0_xfer(buffer_type, ep_addr, buffer, total_bytes);
  } else {
    return process_pipe_xfer(buffer_type, ep_addr, buffer, total_bytes);
  }
}

static void process_pipe0_bemp(uint8_t rhport)
{
  bool completed = pipe0_xfer_in();
  if (completed) {
    pipe_state_t *pipe = &_dcd.pipe[0];
    dcd_event_xfer_complete(rhport, tu_edpt_addr(0, TUSB_DIR_IN),
                            pipe->length, XFER_RESULT_SUCCESS, true);
  }
}

static void process_pipe_brdy(uint8_t rhport, unsigned num)
{
  pipe_state_t  *pipe = &_dcd.pipe[num];
  const unsigned dir  = tu_edpt_dir(pipe->ep);
  bool completed;

  if (dir) { /* IN */
    completed = pipe_xfer_in(num);
  } else {
    if (num) {
      completed = pipe_xfer_out(num);
    } else {
      completed = pipe0_xfer_out();
    }
  }
  if (completed) {
    dcd_event_xfer_complete(rhport, pipe->ep,
                            pipe->length - pipe->remaining,
                            XFER_RESULT_SUCCESS, true);
    //  TU_LOG1("C %d %d\r\n", num, pipe->length - pipe->remaining);
  }
}

static void process_bus_reset(uint8_t rhport)
{
  rusb2_reg_t* rusb = RUSB2_REG(rhport);

  rusb->BEMPENB = 1;
  rusb->BRDYENB = 1;
  rusb->CFIFOCTR = RUSB2_CFIFOCTR_BCLR_Msk;

  rusb->D0FIFOSEL = 0;
  while (rusb->D0FIFOSEL_b.CURPIPE) {} /* if CURPIPE bits changes, check written value */

  rusb->D1FIFOSEL = 0;
  while (rusb->D1FIFOSEL_b.CURPIPE) {} /* if CURPIPE bits changes, check written value */

  volatile uint16_t *ctr = (volatile uint16_t*)((uintptr_t) (&rusb->PIPE_CTR[0]));
  volatile uint16_t *tre = (volatile uint16_t*)((uintptr_t) (&rusb->PIPE_TR[0].E));

  for (int i = 1; i <= 5; ++i) {
    rusb->PIPESEL = i;
    rusb->PIPECFG = 0;
    *ctr = RUSB2_PIPE_CTR_ACLRM_Msk;
    *ctr = 0;
    ++ctr;
    *tre = TU_BIT(8);
    tre += 2;
  }

  for (int i = 6; i <= 9; ++i) {
    rusb->PIPESEL = i;
    rusb->PIPECFG = 0;
    *ctr = RUSB2_PIPE_CTR_ACLRM_Msk;
    *ctr = 0;
    ++ctr;
  }
  tu_varclr(&_dcd);

  TU_LOG3("Bus reset, RHST = %u\r\n", rusb->DVSTCTR0_b.RHST);
  tusb_speed_t speed;
  switch(rusb->DVSTCTR0 & RUSB2_DVSTCTR0_RHST_Msk) {
    case RUSB2_DVSTCTR0_RHST_LS:
      speed = TUSB_SPEED_LOW;
      break;

    case RUSB2_DVSTCTR0_RHST_FS:
      speed = TUSB_SPEED_FULL;
      break;

    case RUSB2_DVSTCTR0_RHST_HS:
      speed = TUSB_SPEED_HIGH;
      break;

    default:
      TU_ASSERT(false, );
  }

  dcd_event_bus_reset(rhport, speed, true);
}

static void process_set_address(uint8_t rhport)
{
  const uint32_t addr = RUSB2->USBADDR & 0xFF;
  if (!addr) return;
  const tusb_control_request_t setup_packet = {
#if defined(__CCRX__)
      .bmRequestType = { 0 },  /* Note: CCRX needs the braces over this struct member */
#else
      .bmRequestType = 0,
#endif
      .bRequest      = TUSB_REQ_SET_ADDRESS,
      .wValue        = addr,
      .wIndex        = 0,
      .wLength       = 0,
    };
  dcd_event_setup_received(rhport, (const uint8_t*)&setup_packet, true);
}

/*------------------------------------------------------------------*/
/* Device API
 *------------------------------------------------------------------*/

#if 0 // previously present in the rx driver before generalization
static uint32_t disable_interrupt(void)
{
  uint32_t pswi;
#if defined(__CCRX__)
  pswi = get_psw() & 0x010000;
  clrpsw_i();
#else
  pswi = __builtin_rx_mvfc(0) & 0x010000;
  __builtin_rx_clrpsw('I');
#endif
  return pswi;
}

static void enable_interrupt(uint32_t pswi)
{
#if defined(__CCRX__)
  set_psw(get_psw() | pswi);
#else
  __builtin_rx_mvtc(0, __builtin_rx_mvfc(0) | pswi);
#endif
}
#endif

void dcd_init(uint8_t rhport)
{
  rusb2_reg_t* rusb = RUSB2_REG(rhport);

  rusb2_module_start(rhport, true);

  if ( is_highspeed(rhport) ) {
    rusb->SYSCFG_b.HSE = 1;

    // leave CLKSEL as default (0x11) 24Mhz

    // Power and reset UTMI Phy
    uint16_t physet = (rusb->PHYSET | RUSB2_PHYSET_PLLRESET_Msk) & ~RUSB2_PHYSET_DIRPD_Msk;
    rusb->PHYSET = physet;
    R_BSP_SoftwareDelay((uint32_t) 1, BSP_DELAY_UNITS_MILLISECONDS);
    rusb->PHYSET_b.PLLRESET = 0;

    // set UTMI to operating mode and wait for PLL lock confirmation
    rusb->LPSTS_b.SUSPENDM = 1;
    while (!rusb->PLLSTA_b.PLLLOCK) {}

    rusb->SYSCFG_b.DRPD = 0;
    rusb->SYSCFG_b.USBE = 1;

    // Set CPU bus wait time (fine tunne later)
    // rusb2->BUSWAIT |= 0x0F00U;

    rusb->PHYSET_b.REPSEL = 1;

    rusb->CFIFOSEL_b.MBW = 1;
    rusb->D0FIFOSEL_b.MBW = 1;
    rusb->D1FIFOSEL_b.MBW = 1;
  } else {
    rusb->SYSCFG_b.SCKE = 1;
    while (!rusb->SYSCFG_b.SCKE) {}
    rusb->SYSCFG_b.DRPD = 0;
    rusb->SYSCFG_b.DCFM = 0;
    rusb->SYSCFG_b.USBE = 1;

    // MCU specific PHY init
    rusb2_phy_init();

    rusb->PHYSLEW = 0x5;
    rusb->DPUSR0R_FS_b.FIXPHY0 = 0u; /* USB_BASE Transceiver Output fixed */

    // rusb2->USBMC = (uint16_t) (rusb2->USBMC | RUSB2_USBMC_VDCEN_Msk);
  }

  /* Setup default control pipe */
  rusb->DCPMAXP_b.MXPS = 64;

  rusb->INTSTS0 = 0;
  rusb->INTENB0 = RUSB2_INTSTS0_VBINT_Msk | RUSB2_INTSTS0_BRDY_Msk | RUSB2_INTSTS0_BEMP_Msk |
                  RUSB2_INTSTS0_DVST_Msk | RUSB2_INTSTS0_CTRT_Msk | (USE_SOF ? RUSB2_INTSTS0_SOFR_Msk : 0) |
                  RUSB2_INTSTS0_RESM_Msk | RUSB2_INTSTS0_NRDY_Msk;
  rusb->BEMPENB = 1;
  rusb->BRDYENB = 1;

  if (rusb->INTSTS0_b.VBSTS) {
    dcd_connect(rhport);
  }
}

void dcd_int_enable(uint8_t rhport)
{
  rusb2_int_enable(rhport);
}

void dcd_int_disable(uint8_t rhport)
{
  rusb2_int_disable(rhport);
}

void dcd_set_address(uint8_t rhport, uint8_t dev_addr)
{
  (void)rhport;
  (void)dev_addr;
}

void dcd_remote_wakeup(uint8_t rhport)
{
  rusb2_reg_t* rusb = RUSB2_REG(rhport);
  rusb->DVSTCTR0_b.WKUP = 1;
}

void dcd_connect(uint8_t rhport)
{
  rusb2_reg_t* rusb = RUSB2_REG(rhport);

  if ( is_highspeed(rhport)) {
    rusb->SYSCFG_b.CNEN = 1;
  }
  rusb->SYSCFG_b.DPRPU = 1;
}

void dcd_disconnect(uint8_t rhport)
{
  rusb2_reg_t* rusb = RUSB2_REG(rhport);
  rusb->SYSCFG_b.DPRPU = 0;
}

void dcd_sof_enable(uint8_t rhport, bool en)
{
  (void) rhport;
  (void) en;

  // TODO implement later
}

//--------------------------------------------------------------------+
// Endpoint API
//--------------------------------------------------------------------+
bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const * ep_desc)
{
  (void)rhport;

  const unsigned ep_addr = ep_desc->bEndpointAddress;
  const unsigned epn     = tu_edpt_number(ep_addr);
  const unsigned dir     = tu_edpt_dir(ep_addr);
  const unsigned xfer    = ep_desc->bmAttributes.xfer;

  const unsigned mps = tu_edpt_packet_size(ep_desc);
  if (xfer == TUSB_XFER_ISOCHRONOUS && mps > 256) {
    /* USBa supports up to 256 bytes */
    return false;
  }

  const unsigned num = find_pipe(xfer);
  if (!num) return false;
  _dcd.pipe[num].ep = ep_addr;
  _dcd.ep[dir][epn] = num;

  /* setup pipe */
  dcd_int_disable(rhport);
  #if (CFG_TUSB_RHPORT1_MODE & OPT_MODE_DEVICE)
  RUSB2->PIPEBUF = 0x7C08;
  #endif
  RUSB2->PIPESEL = num;
  RUSB2->PIPEMAXP = mps;
  volatile uint16_t *ctr = get_pipectr(num);
  *ctr = RUSB2_PIPE_CTR_ACLRM_Msk | RUSB2_PIPE_CTR_SQCLR_Msk;
  *ctr = 0;
  unsigned cfg = (dir << 4) | epn;
  if (xfer == TUSB_XFER_BULK) {
    cfg |= (RUSB2_PIPECFG_TYPE_BULK | RUSB2_PIPECFG_SHTNAK_Msk | RUSB2_PIPECFG_DBLB_Msk);
  } else if (xfer == TUSB_XFER_INTERRUPT) {
    cfg |= RUSB2_PIPECFG_TYPE_INT;
  } else {
    cfg |= (RUSB2_PIPECFG_TYPE_ISO | RUSB2_PIPECFG_DBLB_Msk);
  }
  RUSB2->PIPECFG = cfg;
  RUSB2->BRDYSTS = 0x1FFu ^ TU_BIT(num);
  RUSB2->BRDYENB |= TU_BIT(num);
  if (dir || (xfer != TUSB_XFER_BULK)) {
    *ctr = RUSB2_PIPE_CTR_PID_BUF;
  }
  // TU_LOG1("O %d %x %x\r\n", RUSB2->PIPESEL, RUSB2->PIPECFG, RUSB2->PIPEMAXP);
  dcd_int_enable(rhport);

  return true;
}

void dcd_edpt_close_all(uint8_t rhport)
{
  unsigned i = TU_ARRAY_SIZE(_dcd.pipe);
  dcd_int_disable(rhport);
  while (--i) { /* Close all pipes except 0 */
    const unsigned ep_addr = _dcd.pipe[i].ep;
    if (!ep_addr) continue;
    dcd_edpt_close(rhport, ep_addr);
  }
  dcd_int_enable(rhport);
}

void dcd_edpt_close(uint8_t rhport, uint8_t ep_addr)
{
  (void)rhport;
  const unsigned epn = tu_edpt_number(ep_addr);
  const unsigned dir = tu_edpt_dir(ep_addr);
  const unsigned num = _dcd.ep[dir][epn];

  RUSB2->BRDYENB &= ~TU_BIT(num);
  volatile uint16_t *ctr = get_pipectr(num);
  *ctr = 0;
  RUSB2->PIPESEL = num;
  RUSB2->PIPECFG = 0;
  _dcd.pipe[num].ep = 0;
  _dcd.ep[dir][epn] = 0;
}

bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t* buffer, uint16_t total_bytes)
{
  bool r;
  dcd_int_disable(rhport);
  r = process_edpt_xfer(0, ep_addr, buffer, total_bytes);
  dcd_int_enable(rhport);
  return r;
}

bool dcd_edpt_xfer_fifo(uint8_t rhport, uint8_t ep_addr, tu_fifo_t * ff, uint16_t total_bytes)
{
  // USB buffers always work in bytes so to avoid unnecessary divisions we demand item_size = 1
  TU_ASSERT(ff->item_size == 1);
  bool r;
  dcd_int_disable(rhport);
  r = process_edpt_xfer(1, ep_addr, ff, total_bytes);
  dcd_int_enable(rhport);
  return r;
}

void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr)
{
  volatile uint16_t *ctr = ep_addr_to_pipectr(rhport, ep_addr);
  if (!ctr) return;
  dcd_int_disable(rhport);
  const uint32_t pid = *ctr & 0x3;
  *ctr = pid | RUSB2_PIPE_CTR_PID_STALL;
  *ctr = RUSB2_PIPE_CTR_PID_STALL;
  dcd_int_enable(rhport);
}

void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr)
{
  volatile uint16_t *ctr = ep_addr_to_pipectr(rhport, ep_addr);
  if (!ctr) return;
  dcd_int_disable(rhport);
  *ctr = RUSB2_PIPE_CTR_SQCLR_Msk;

  if (tu_edpt_dir(ep_addr)) { /* IN */
    *ctr = RUSB2_PIPE_CTR_PID_BUF;
  } else {
    const unsigned num = _dcd.ep[0][tu_edpt_number(ep_addr)];
    RUSB2->PIPESEL = num;
    if (RUSB2->PIPECFG_b.TYPE != 1) {
      *ctr = RUSB2_PIPE_CTR_PID_BUF;
    }
  }
  dcd_int_enable(rhport);
}

//--------------------------------------------------------------------+
// ISR
//--------------------------------------------------------------------+
void dcd_int_handler(uint8_t rhport)
{
  rusb2_reg_t* rusb = RUSB2_REG(rhport);

  uint16_t is0 = rusb->INTSTS0;

  /* clear active bits except VALID (don't write 0 to already cleared bits according to the HW manual) */
  rusb->INTSTS0 = ~((RUSB2_INTSTS0_CTRT_Msk | RUSB2_INTSTS0_DVST_Msk | RUSB2_INTSTS0_SOFR_Msk |
                     RUSB2_INTSTS0_RESM_Msk | RUSB2_INTSTS0_VBINT_Msk) & is0) | RUSB2_INTSTS0_VALID_Msk;

  // VBUS changes
  if ( is0 & RUSB2_INTSTS0_VBINT_Msk ) {
    if ( rusb->INTSTS0_b.VBSTS ) {
      dcd_connect(rhport);
    } else {
      dcd_disconnect(rhport);
    }
  }

  // Resumed
  if ( is0 & RUSB2_INTSTS0_RESM_Msk ) {
    dcd_event_bus_signal(rhport, DCD_EVENT_RESUME, true);
#if (0 == USE_SOF)
    rusb->INTENB0_b.SOFE = 0;
#endif
  }

  // SOF received
  if ( (is0 & RUSB2_INTSTS0_SOFR_Msk) && rusb->INTENB0_b.SOFE ) {
    // USBD will exit suspended mode when SOF event is received
    dcd_event_bus_signal(rhport, DCD_EVENT_SOF, true);
#if (0 == USE_SOF)
    rusb->INTENB0_b.SOFE = 0;
#endif
  }

  // Device state changes
  if ( is0 & RUSB2_INTSTS0_DVST_Msk ) {
    switch (is0 & RUSB2_INTSTS0_DVSQ_Msk) {
      case RUSB2_INTSTS0_DVSQ_STATE_DEF:
        process_bus_reset(rhport);
        break;

      case RUSB2_INTSTS0_DVSQ_STATE_ADDR:
        process_set_address(rhport);
        break;

      case RUSB2_INTSTS0_DVSQ_STATE_SUSP0:
      case RUSB2_INTSTS0_DVSQ_STATE_SUSP1:
      case RUSB2_INTSTS0_DVSQ_STATE_SUSP2:
      case RUSB2_INTSTS0_DVSQ_STATE_SUSP3:
        dcd_event_bus_signal(rhport, DCD_EVENT_SUSPEND, true);
#if (0 == USE_SOF)
        rusb->INTENB0_b.SOFE = 1;
#endif

      default: break;
    }
  }

  if ( is0 & RUSB2_INTSTS0_NRDY_Msk ) {
    rusb->NRDYSTS = 0;
  }

  // Control transfer stage changes
  if ( is0 & RUSB2_INTSTS0_CTRT_Msk ) {
    if ( is0 & RUSB2_INTSTS0_CTSQ_CTRL_RDATA ) {
      /* A setup packet has been received. */
      process_setup_packet(rhport);
    } else if ( 0 == (is0 & RUSB2_INTSTS0_CTSQ_Msk) ) {
      /* A ZLP has been sent/received. */
      process_status_completion(rhport);
    }
  }

  // Buffer empty
  if ( is0 & RUSB2_INTSTS0_BEMP_Msk ) {
    const unsigned s = rusb->BEMPSTS;
    rusb->BEMPSTS = 0;
    if ( s & 1 ) {
      process_pipe0_bemp(rhport);
    }
  }

  // Buffer ready
  if ( is0 & RUSB2_INTSTS0_BRDY_Msk ) {
    const unsigned m = rusb->BRDYENB;
    unsigned s = rusb->BRDYSTS & m;
    /* clear active bits (don't write 0 to already cleared bits according to the HW manual) */
    rusb->BRDYSTS = ~s;
    while (s) {
#if defined(__CCRX__)
      static const int Mod37BitPosition[] = {
        -1, 0, 1, 26, 2, 23, 27, 0, 3, 16, 24, 30, 28, 11, 0, 13, 4,
        7, 17, 0, 25, 22, 31, 15, 29, 10, 12, 6, 0, 21, 14, 9, 5,
        20, 8, 19, 18
      };

      const unsigned num = Mod37BitPosition[(-s & s) % 37];
#else
      const unsigned num = __builtin_ctz(s);
#endif
      process_pipe_brdy(rhport, num);
      s &= ~TU_BIT(num);
    }
  }
}

#endif
