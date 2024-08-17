/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Koji KITAYAMA
 * Copyright (c) 2024, Brent Kowal (Analog Devices, Inc)
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

#if CFG_TUD_ENABLED && defined(TUP_USBIP_MUSB)

#if __GNUC__ > 8 && defined(__ARM_FEATURE_UNALIGNED)
/* GCC warns that an address may be unaligned, even though
 * the target CPU has the capability for unaligned memory access. */
_Pragma("GCC diagnostic ignored \"-Waddress-of-packed-member\"");
#endif

#include "musb_type.h"
#include "device/dcd.h"

// Following symbols must be defined by port header
// - musb_dcd_int_enable/disable/clear/get_enable
// - musb_dcd_int_handler_enter/exit
// - musb_dcd_setup_fifo: Configuration of the EP's FIFO
#if defined(TUP_USBIP_MUSB_TI)
  #include "musb_ti.h"
#elif defined(TUP_USBIP_MUSB_ADI)
  #include "musb_max32.h"
#else
  #error "Unsupported MCU"
#endif

#define MUSB_REGS(rhport)   ((musb_regs_t*) MUSB_BASES[rhport])

#define MUSB_DEBUG 2

/*------------------------------------------------------------------
 * MACRO TYPEDEF CONSTANT ENUM DECLARATION
 *------------------------------------------------------------------*/

#define REQUEST_TYPE_INVALID  (0xFFu)

typedef union {
  uint8_t   u8;
  uint16_t  u16;
  uint32_t  u32;
} hw_fifo_t;

typedef struct TU_ATTR_PACKED
{
  void      *buf;      /* the start address of a transfer data buffer */
  uint16_t  length;    /* the number of bytes in the buffer */
  uint16_t  remaining; /* the number of bytes remaining in the buffer */
} pipe_state_t;

typedef struct
{
  tusb_control_request_t setup_packet;
  uint16_t     remaining_ctrl; /* The number of bytes remaining in data stage of control transfer. */
  int8_t       status_out;
  pipe_state_t pipe0;
  pipe_state_t pipe[2][TUP_DCD_ENDPOINT_MAX-1];   /* pipe[direction][endpoint number - 1] */
  uint16_t     pipe_buf_is_fifo[2]; /* Bitmap. Each bit means whether 1:TU_FIFO or 0:POD. */
} dcd_data_t;

/*------------------------------------------------------------------
 * INTERNAL OBJECT & FUNCTION DECLARATION
 *------------------------------------------------------------------*/
static dcd_data_t _dcd;

TU_ATTR_ALWAYS_INLINE static inline void fifo_reset(musb_regs_t* musb, unsigned epnum, unsigned dir_in) {
  musb->index = epnum;
  const uint8_t is_rx = 1 - dir_in;

#if MUSB_CFG_DYNAMIC_FIFO
  musb->fifo_size[is_rx] = 0;
  musb->fifo_addr[is_rx] = 0;
#elif defined(TUP_USBIP_MUSB_ADI)
  // Analog have custom double buffered in csrh register, disable it
  musb->indexed_csr.maxp_csr[is_rx].csrh |= MUSB_CSRH_DISABLE_DOUBLE_PACKET(is_rx);
#endif
}

static void pipe_write_packet(void *buf, volatile void *fifo, unsigned len)
{
  volatile hw_fifo_t *reg = (volatile hw_fifo_t*)fifo;
  uintptr_t addr = (uintptr_t)buf;
  while (len >= 4) {
    reg->u32 = *(uint32_t const *)addr;
    addr += 4;
    len  -= 4;
  }
  if (len >= 2) {
    reg->u16 = *(uint16_t const *)addr;
    addr += 2;
    len  -= 2;
  }
  if (len) {
    reg->u8 = *(uint8_t const *)addr;
  }
}

static void pipe_read_packet(void *buf, volatile void *fifo, unsigned len)
{
  volatile hw_fifo_t *reg = (volatile hw_fifo_t*)fifo;
  uintptr_t addr = (uintptr_t)buf;
  while (len >= 4) {
    *(uint32_t *)addr = reg->u32;
    addr += 4;
    len  -= 4;
  }
  if (len >= 2) {
    *(uint16_t *)addr = reg->u16;
    addr += 2;
    len  -= 2;
  }
  if (len) {
    *(uint8_t *)addr = reg->u8;
  }
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

static void process_setup_packet(uint8_t rhport) {
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  uint32_t *p = (void*)&_dcd.setup_packet;
  volatile uint32_t *fifo_ptr = &musb_regs->fifo[0];

  p[0]        = *fifo_ptr;
  p[1]        = *fifo_ptr;

  _dcd.pipe0.buf       = NULL;
  _dcd.pipe0.length    = 0;
  _dcd.pipe0.remaining = 0;
  dcd_event_setup_received(rhport, (const uint8_t*)(uintptr_t)&_dcd.setup_packet, true);

  const unsigned len    = _dcd.setup_packet.wLength;
  _dcd.remaining_ctrl   = len;
  const unsigned dir_in = tu_edpt_dir(_dcd.setup_packet.bmRequestType);
  /* Clear RX FIFO and reverse the transaction direction */
  if (len && dir_in) {
    musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, 0);
    ep_csr->csr0l = USB_CSRL0_RXRDYC;
  }
}

static bool handle_xfer_in(uint8_t rhport, uint_fast8_t ep_addr)
{
  unsigned epnum = tu_edpt_number(ep_addr);
  unsigned epnum_minus1 = epnum - 1;
  pipe_state_t  *pipe = &_dcd.pipe[tu_edpt_dir(ep_addr)][epnum_minus1];
  const unsigned rem  = pipe->remaining;

  if (!rem) {
    pipe->buf = NULL;
    return true;
  }

  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epnum);
  const unsigned mps = ep_csr->tx_maxp;
  const unsigned len = TU_MIN(mps, rem);
  void          *buf = pipe->buf;
  volatile void *fifo_ptr = &musb_regs->fifo[epnum];
  // TU_LOG1("   %p mps %d len %d rem %d\r\n", buf, mps, len, rem);
  if (len) {
    if (_dcd.pipe_buf_is_fifo[TUSB_DIR_IN] & TU_BIT(epnum_minus1)) {
      pipe_read_write_packet_ff(buf, fifo_ptr, len, TUSB_DIR_IN);
    } else {
      pipe_write_packet(buf, fifo_ptr, len);
      pipe->buf       = buf + len;
    }
    pipe->remaining = rem - len;
  }
  ep_csr->tx_csrl = USB_TXCSRL1_TXRDY;
  // TU_LOG1(" TXCSRL%d = %x %d\r\n", epnum, ep_csr->tx_csrl, rem - len);
  return false;
}

static bool handle_xfer_out(uint8_t rhport, uint_fast8_t ep_addr)
{
  unsigned epnum = tu_edpt_number(ep_addr);
  unsigned epnum_minus1 = epnum - 1;
  pipe_state_t  *pipe = &_dcd.pipe[tu_edpt_dir(ep_addr)][epnum_minus1];
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epnum);
  // TU_LOG1(" RXCSRL%d = %x\r\n", epnum_minus1 + 1, ep_csr->rx_csrl);

  TU_ASSERT(ep_csr->rx_csrl & USB_RXCSRL1_RXRDY);

  const unsigned mps = ep_csr->rx_maxp;
  const unsigned rem = pipe->remaining;
  const unsigned vld = ep_csr->rx_count;
  const unsigned len = TU_MIN(TU_MIN(rem, mps), vld);
  void          *buf = pipe->buf;
  volatile void *fifo_ptr = &musb_regs->fifo[epnum];
  if (len) {
    if (_dcd.pipe_buf_is_fifo[TUSB_DIR_OUT] & TU_BIT(epnum_minus1)) {
      pipe_read_write_packet_ff(buf, fifo_ptr, len, TUSB_DIR_OUT);
    } else {
      pipe_read_packet(buf, fifo_ptr, len);
      pipe->buf       = buf + len;
    }
    pipe->remaining = rem - len;
  }
  if ((len < mps) || (rem == len)) {
    pipe->buf = NULL;
    return NULL != buf;
  }
  ep_csr->rx_csrl = 0; /* Clear RXRDY bit */
  return false;
}

static bool edpt_n_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes)
{
  unsigned epnum = tu_edpt_number(ep_addr);
  unsigned epnum_minus1 = epnum - 1;
  unsigned dir_in       = tu_edpt_dir(ep_addr);

  pipe_state_t *pipe = &_dcd.pipe[dir_in][epnum_minus1];
  pipe->buf          = buffer;
  pipe->length       = total_bytes;
  pipe->remaining    = total_bytes;

  if (dir_in) {
    handle_xfer_in(rhport, ep_addr);
  } else {
    musb_regs_t* musb_regs = MUSB_REGS(rhport);
    musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epnum);
    if (ep_csr->rx_csrl & USB_RXCSRL1_RXRDY) ep_csr->rx_csrl = 0;
  }
  return true;
}

static bool edpt0_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes)
{
  (void)rhport;
  TU_ASSERT(total_bytes <= 64); /* Current implementation supports for only up to 64 bytes. */
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, 0);
  const unsigned req = _dcd.setup_packet.bmRequestType;
  TU_ASSERT(req != REQUEST_TYPE_INVALID || total_bytes == 0);

  if (req == REQUEST_TYPE_INVALID || _dcd.status_out) {
    /* STATUS OUT stage.
     * MUSB controller automatically handles STATUS OUT packets without
     * software helps. We do not have to do anything. And STATUS stage
     * may have already finished and received the next setup packet
     * without calling this function, so we have no choice but to
     * invoke the callback function of status packet here. */
    // TU_LOG1(" STATUS OUT ep_csr->csr0l = %x\r\n", ep_csr->csr0l);
    _dcd.status_out = 0;
    if (req == REQUEST_TYPE_INVALID) {
      dcd_event_xfer_complete(rhport, ep_addr, total_bytes, XFER_RESULT_SUCCESS, false);
    } else {
      /* The next setup packet has already been received, it aborts
       * invoking callback function to avoid confusing TUSB stack. */
      TU_LOG1("Drop CONTROL_STAGE_ACK\r\n");
    }
    return true;
  }
  const unsigned dir_in = tu_edpt_dir(ep_addr);
  if (tu_edpt_dir(req) == dir_in) { /* DATA stage */
    TU_ASSERT(total_bytes <= _dcd.remaining_ctrl);
    const unsigned rem = _dcd.remaining_ctrl;
    const unsigned len = TU_MIN(TU_MIN(rem, 64), total_bytes);
    volatile void *fifo_ptr = &musb_regs->fifo[0];
    if (dir_in) {
      pipe_write_packet(buffer, fifo_ptr, len);

      _dcd.pipe0.buf       = buffer + len;
      _dcd.pipe0.length    = len;
      _dcd.pipe0.remaining = 0;

      _dcd.remaining_ctrl  = rem - len;
      if ((len < 64) || (rem == len)) {
        _dcd.setup_packet.bmRequestType = REQUEST_TYPE_INVALID; /* Change to STATUS/SETUP stage */
        _dcd.status_out = 1;
        /* Flush TX FIFO and reverse the transaction direction. */
        ep_csr->csr0l = USB_CSRL0_TXRDY | USB_CSRL0_DATAEND;
      } else {
        ep_csr->csr0l = USB_CSRL0_TXRDY; /* Flush TX FIFO to return ACK. */
      }
      // TU_LOG1(" IN ep_csr->csr0l = %x\r\n", ep_csr->csr0l);
    } else {
      // TU_LOG1(" OUT ep_csr->csr0l = %x\r\n", ep_csr->csr0l);
      _dcd.pipe0.buf       = buffer;
      _dcd.pipe0.length    = len;
      _dcd.pipe0.remaining = len;
      ep_csr->csr0l = USB_CSRL0_RXRDYC; /* Clear RX FIFO to return ACK. */
    }
  } else if (dir_in) {
    // TU_LOG1(" STATUS IN ep_csr->csr0l  = %x\r\n", ep_csr->csr0l);
    _dcd.pipe0.buf = NULL;
    _dcd.pipe0.length    = 0;
    _dcd.pipe0.remaining = 0;
    /* Clear RX FIFO and reverse the transaction direction */
    ep_csr->csr0l = USB_CSRL0_RXRDYC | USB_CSRL0_DATAEND;
  }
  return true;
}

static void process_ep0(uint8_t rhport)
{
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, 0);
  uint_fast8_t csrl = ep_csr->csr0l;

  // TU_LOG1(" EP0 ep_csr->csr0l = %x\r\n", csrl);

  if (csrl & USB_CSRL0_STALLED) {
    /* Returned STALL packet to HOST. */
    ep_csr->csr0l = 0; /* Clear STALL */
    return;
  }

  unsigned req = _dcd.setup_packet.bmRequestType;
  if (csrl & USB_CSRL0_SETEND) {
    TU_LOG1("   ABORT by the next packets\r\n");
    ep_csr->csr0l = USB_CSRL0_SETENDC;
    if (req != REQUEST_TYPE_INVALID && _dcd.pipe0.buf) {
      /* DATA stage was aborted by receiving STATUS or SETUP packet. */
      _dcd.pipe0.buf = NULL;
      _dcd.setup_packet.bmRequestType = REQUEST_TYPE_INVALID;
      dcd_event_xfer_complete(rhport,
                              req & TUSB_DIR_IN_MASK,
                              _dcd.pipe0.length - _dcd.pipe0.remaining,
                              XFER_RESULT_SUCCESS, true);
    }
    req = REQUEST_TYPE_INVALID;
    if (!(csrl & USB_CSRL0_RXRDY)) return; /* Received SETUP packet */
  }

  if (csrl & USB_CSRL0_RXRDY) {
    /* Received SETUP or DATA OUT packet */
    if (req == REQUEST_TYPE_INVALID) {
      /* SETUP */
      TU_ASSERT(sizeof(tusb_control_request_t) == ep_csr->count0,);
      process_setup_packet(rhport);
      return;
    }
    if (_dcd.pipe0.buf) {
      /* DATA OUT */
      const unsigned vld = ep_csr->count0;
      const unsigned rem = _dcd.pipe0.remaining;
      const unsigned len = TU_MIN(TU_MIN(rem, 64), vld);
      volatile void *fifo_ptr = &musb_regs->fifo[0];
      pipe_read_packet(_dcd.pipe0.buf, fifo_ptr, len);

      _dcd.pipe0.remaining = rem - len;
      _dcd.remaining_ctrl -= len;

      _dcd.pipe0.buf = NULL;
      dcd_event_xfer_complete(rhport,
                              tu_edpt_addr(0, TUSB_DIR_OUT),
                              _dcd.pipe0.length - _dcd.pipe0.remaining,
                              XFER_RESULT_SUCCESS, true);
    }
    return;
  }

  /* When CSRL0 is zero, it means that completion of sending a any length packet
   * or receiving a zero length packet. */
  if (req != REQUEST_TYPE_INVALID && !tu_edpt_dir(req)) {
    /* STATUS IN */
    if (*(const uint16_t*)(uintptr_t)&_dcd.setup_packet == 0x0500) {
      /* The address must be changed on completion of the control transfer. */
      musb_regs->faddr = (uint8_t)_dcd.setup_packet.wValue;
    }
    _dcd.setup_packet.bmRequestType = REQUEST_TYPE_INVALID;
    dcd_event_xfer_complete(rhport,
                            tu_edpt_addr(0, TUSB_DIR_IN),
                            _dcd.pipe0.length - _dcd.pipe0.remaining,
                            XFER_RESULT_SUCCESS, true);
    return;
  }
  if (_dcd.pipe0.buf) {
    /* DATA IN */
    _dcd.pipe0.buf = NULL;
    dcd_event_xfer_complete(rhport,
                            tu_edpt_addr(0, TUSB_DIR_IN),
                            _dcd.pipe0.length - _dcd.pipe0.remaining,
                            XFER_RESULT_SUCCESS, true);
  }
}

static void process_edpt_n(uint8_t rhport, uint_fast8_t ep_addr)
{
  bool completed;
  const unsigned dir_in = tu_edpt_dir(ep_addr);
  const unsigned epn = tu_edpt_number(ep_addr);
  const unsigned epn_minus1 = epn - 1;

  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epn);
  if (dir_in) {
    // TU_LOG1(" TX CSRL%d = %x\r\n", epn, ep_csr->tx_csrl);
    if (ep_csr->tx_csrl & USB_TXCSRL1_STALLED) {
      ep_csr->tx_csrl &= ~(USB_TXCSRL1_STALLED | USB_TXCSRL1_UNDRN);
      return;
    }
    completed = handle_xfer_in(rhport, ep_addr);
  } else {
    // TU_LOG1(" RX CSRL%d = %x\r\n", epn, ep_csr->rx_csrl);
    if (ep_csr->rx_csrl & USB_RXCSRL1_STALLED) {
      ep_csr->rx_csrl &= ~(USB_RXCSRL1_STALLED | USB_RXCSRL1_OVER);
      return;
    }
    completed = handle_xfer_out(rhport, ep_addr);
  }

  if (completed) {
    pipe_state_t *pipe = &_dcd.pipe[dir_in][epn_minus1];
    dcd_event_xfer_complete(rhport, ep_addr,
                            pipe->length - pipe->remaining,
                            XFER_RESULT_SUCCESS, true);
  }
}

// Upon BUS RESET is detected, hardware havs already done:
// faddr = 0, index = 0, flushes all ep fifos, clears all ep csr, enabled all ep interrupts
static void process_bus_reset(uint8_t rhport) {
  musb_regs_t* musb = MUSB_REGS(rhport);
  /* When bmRequestType is REQUEST_TYPE_INVALID(0xFF), a control transfer state is SETUP or STATUS stage. */
  _dcd.setup_packet.bmRequestType = REQUEST_TYPE_INVALID;
  _dcd.status_out = 0;
  /* When pipe0.buf has not NULL, DATA stage works in progress. */
  _dcd.pipe0.buf = NULL;

  musb->intr_txen = 1; /* Enable only EP0 */
  musb->intr_rxen = 0;

  /* Clear FIFO settings */
  for (unsigned i = 1; i < TUP_DCD_ENDPOINT_MAX; ++i) {
    fifo_reset(musb, i, 0);
    fifo_reset(musb, i, 1);
  }
  dcd_event_bus_reset(rhport, (musb->power & USB_POWER_HSMODE) ? TUSB_SPEED_HIGH : TUSB_SPEED_FULL, true);
}

/*------------------------------------------------------------------
 * Device API
 *------------------------------------------------------------------*/

#if CFG_TUSB_DEBUG >= MUSB_DEBUG
void print_musb_info(musb_regs_t* musb_regs) {
  // print version, epinfo, raminfo, config_data0, fifo_size
  TU_LOG1("musb version = %u.%u\r\n", musb_regs->hwvers_bit.major, musb_regs->hwvers_bit.minor);
  TU_LOG1("Number of endpoints: %u TX, %u RX\r\n", musb_regs->epinfo_bit.tx_ep_num, musb_regs->epinfo_bit.rx_ep_num);
  TU_LOG1("RAM Info: %u DMA Channel, %u RAM address width\r\n", musb_regs->raminfo_bit.dma_channel, musb_regs->raminfo_bit.ram_bits);

  musb_regs->index = 0;
  TU_LOG1("config_data0 = 0x%x\r\n", musb_regs->indexed_csr.config_data0);

#if MUSB_CFG_DYNAMIC_FIFO
  TU_LOG1("Dynamic FIFO configuration\r\n");
#else
  for (uint8_t i=1; i <= musb_regs->epinfo_bit.tx_ep_num; i++) {
    musb_regs->index = i;
    TU_LOG1("FIFO %u Size: TX %u RX %u\r\n", i, musb_regs->indexed_csr.fifo_size_bit.tx, musb_regs->indexed_csr.fifo_size_bit.rx);
  }
#endif
}
#endif

void dcd_init(uint8_t rhport) {
  musb_regs_t* musb_regs = MUSB_REGS(rhport);

#if CFG_TUSB_DEBUG >= MUSB_DEBUG
  print_musb_info(musb_regs);
#endif

  musb_regs->intr_usben |= USB_IE_SUSPND;
  musb_dcd_int_clear(rhport);
  musb_dcd_phy_init(rhport);
  dcd_connect(rhport);
}

void dcd_int_enable(uint8_t rhport) {
  musb_dcd_int_enable(rhport);
}

void dcd_int_disable(uint8_t rhport) {
  musb_dcd_int_disable(rhport);
}

// Receive Set Address request, mcu port must also include status IN response
void dcd_set_address(uint8_t rhport, uint8_t dev_addr)
{
  (void)dev_addr;
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, 0);

  _dcd.pipe0.buf       = NULL;
  _dcd.pipe0.length    = 0;
  _dcd.pipe0.remaining = 0;
  /* Clear RX FIFO to return ACK. */
  ep_csr->csr0l = USB_CSRL0_RXRDYC | USB_CSRL0_DATAEND;
}

// Wake up host
void dcd_remote_wakeup(uint8_t rhport) {
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_regs->power |= USB_POWER_RESUME;

  unsigned cnt = SystemCoreClock / 1000;
  while (cnt--) __NOP();

  musb_regs->power &= ~USB_POWER_RESUME;
}

// Connect by enabling internal pull-up resistor on D+/D-
void dcd_connect(uint8_t rhport)
{
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_regs->power |= TUD_OPT_HIGH_SPEED ? USB_POWER_HSENAB : 0;
  musb_regs->power |= USB_POWER_SOFTCONN;
}

// Disconnect by disabling internal pull-up resistor on D+/D-
void dcd_disconnect(uint8_t rhport)
{
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_regs->power &= ~USB_POWER_SOFTCONN;
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

// Configure endpoint's registers according to descriptor
bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const * ep_desc)
{
  const unsigned ep_addr = ep_desc->bEndpointAddress;
  const unsigned epn     = tu_edpt_number(ep_addr);
  const unsigned dir_in  = tu_edpt_dir(ep_addr);
  const unsigned xfer    = ep_desc->bmAttributes.xfer;
  const unsigned mps     = tu_edpt_packet_size(ep_desc);

  TU_ASSERT(epn < TUP_DCD_ENDPOINT_MAX);

  pipe_state_t *pipe = &_dcd.pipe[dir_in][epn - 1];
  pipe->buf       = NULL;
  pipe->length    = 0;
  pipe->remaining = 0;

  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epn);

  const uint8_t is_rx = 1 - dir_in;
  ep_csr->maxp_csr[is_rx].maxp = mps;
  ep_csr->maxp_csr[is_rx].csrh = (xfer == TUSB_XFER_ISOCHRONOUS) ? USB_RXCSRH1_ISO : 0;

  uint8_t csrl = MUSB_CSRL_CLEAR_DATA_TOGGLE(is_rx);
  if (ep_csr->maxp_csr[is_rx].csrl & MUSB_CSRL_PACKET_READY(is_rx)) {
    csrl |= MUSB_CSRL_FLUSH_FIFO(is_rx);
  }
  ep_csr->maxp_csr[is_rx].csrl = csrl;
  musb_regs->intren_ep[is_rx] |= TU_BIT(epn);

  /* Setup FIFO */
  musb_dcd_setup_fifo(rhport, epn, dir_in, mps);

  return true;
}

void dcd_edpt_close_all(uint8_t rhport)
{
  musb_regs_t* musb = MUSB_REGS(rhport);
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_dcd_int_disable(rhport);
  musb->intr_txen = 1; /* Enable only EP0 */
  musb->intr_rxen = 0;
  for (unsigned i = 1; i < TUP_DCD_ENDPOINT_MAX; ++i) {
    musb_ep_csr_t* ep_csr = get_ep_csr(musb, i);
    ep_csr->tx_maxp = 0;
    ep_csr->tx_csrh = 0;
    if (ep_csr->tx_csrl & USB_TXCSRL1_TXRDY)
      ep_csr->tx_csrl = USB_TXCSRL1_CLRDT | USB_TXCSRL1_FLUSH;
    else
      ep_csr->tx_csrl = USB_TXCSRL1_CLRDT;

    ep_csr->rx_maxp = 0;
    ep_csr->rx_csrh = 0;
    if (ep_csr->rx_csrl & USB_RXCSRL1_RXRDY) {
      ep_csr->rx_csrl = USB_RXCSRL1_CLRDT | USB_RXCSRL1_FLUSH;
    } else {
      ep_csr->rx_csrl = USB_RXCSRL1_CLRDT;
    }

    fifo_reset(musb, i, 0);
    fifo_reset(musb, i, 1);

  }
  if (ie) musb_dcd_int_enable(rhport);
}

void dcd_edpt_close(uint8_t rhport, uint8_t ep_addr)
{
  unsigned const epn    = tu_edpt_number(ep_addr);
  unsigned const dir_in = tu_edpt_dir(ep_addr);
  musb_regs_t* musb = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb, epn);
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_dcd_int_disable(rhport);
  if (dir_in) {
    musb->intr_txen  &= ~TU_BIT(epn);
    ep_csr->tx_maxp = 0;
    ep_csr->tx_csrh = 0;
    if (ep_csr->tx_csrl & USB_TXCSRL1_TXRDY) {
      ep_csr->tx_csrl = USB_TXCSRL1_CLRDT | USB_TXCSRL1_FLUSH;
    } else {
      ep_csr->tx_csrl = USB_TXCSRL1_CLRDT;
    }
  } else {
    musb->intr_rxen  &= ~TU_BIT(epn);
    ep_csr->rx_maxp = 0;
    ep_csr->rx_csrh = 0;
    if (ep_csr->rx_csrl & USB_RXCSRL1_RXRDY) {
      ep_csr->rx_csrl = USB_RXCSRL1_CLRDT | USB_RXCSRL1_FLUSH;
    } else {
      ep_csr->rx_csrl = USB_RXCSRL1_CLRDT;
    }
  }
  fifo_reset(musb, epn, dir_in);
  if (ie) musb_dcd_int_enable(rhport);
}

// Submit a transfer, When complete dcd_event_xfer_complete() is invoked to notify the stack
bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t * buffer, uint16_t total_bytes)
{
  (void)rhport;
  bool ret;
  // TU_LOG1("X %x %d\r\n", ep_addr, total_bytes);
  unsigned const epnum = tu_edpt_number(ep_addr);
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_dcd_int_disable(rhport);
  if (epnum) {
    _dcd.pipe_buf_is_fifo[tu_edpt_dir(ep_addr)] &= ~TU_BIT(epnum - 1);
    ret = edpt_n_xfer(rhport, ep_addr, buffer, total_bytes);
  } else {
    ret = edpt0_xfer(rhport, ep_addr, buffer, total_bytes);
  }
  if (ie) musb_dcd_int_enable(rhport);
  return ret;
}

// Submit a transfer where is managed by FIFO, When complete dcd_event_xfer_complete() is invoked to notify the stack - optional, however, must be listed in usbd.c
bool dcd_edpt_xfer_fifo(uint8_t rhport, uint8_t ep_addr, tu_fifo_t * ff, uint16_t total_bytes)
{
  (void)rhport;
  bool ret;
  // TU_LOG1("X %x %d\r\n", ep_addr, total_bytes);
  unsigned const epnum = tu_edpt_number(ep_addr);
  TU_ASSERT(epnum);
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_dcd_int_disable(rhport);
  _dcd.pipe_buf_is_fifo[tu_edpt_dir(ep_addr)] |= TU_BIT(epnum - 1);
  ret = edpt_n_xfer(rhport, ep_addr, (uint8_t*)ff, total_bytes);
  if (ie) musb_dcd_int_enable(rhport);
  return ret;
}

// Stall endpoint
void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr) {
  unsigned const epn = tu_edpt_number(ep_addr);
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epn);
  musb_dcd_int_disable(rhport);
  if (0 == epn) {
    if (!ep_addr) { /* Ignore EP80 */
      _dcd.setup_packet.bmRequestType = REQUEST_TYPE_INVALID;
      _dcd.pipe0.buf = NULL;
      ep_csr->csr0l = USB_CSRL0_STALL;
    }
  } else {
    if (tu_edpt_dir(ep_addr)) { /* IN */
      ep_csr->tx_csrl = USB_TXCSRL1_STALL;
    } else { /* OUT */
      TU_ASSERT(!(ep_csr->rx_csrl & USB_RXCSRL1_RXRDY),);
      ep_csr->rx_csrl = USB_RXCSRL1_STALL;
    }
  }
  if (ie) musb_dcd_int_enable(rhport);
}

// clear stall, data toggle is also reset to DATA0
void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr)
{
  (void)rhport;
  unsigned const epn = tu_edpt_number(ep_addr);
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epn);
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_dcd_int_disable(rhport);
  if (tu_edpt_dir(ep_addr)) { /* IN */
    ep_csr->tx_csrl = USB_TXCSRL1_CLRDT;
  } else { /* OUT */
    ep_csr->rx_csrl = USB_RXCSRL1_CLRDT;
  }
  if (ie) musb_dcd_int_enable(rhport);
}

/*-------------------------------------------------------------------
 * ISR
 *-------------------------------------------------------------------*/
void dcd_int_handler(uint8_t rhport) {
  //Part specific ISR setup/entry
  musb_dcd_int_handler_enter(rhport);

  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  uint_fast8_t intr_usb = musb_regs->intr_usb; // a read will clear this interrupt status
  uint_fast8_t intr_tx = musb_regs->intr_tx; // a read will clear this interrupt status
  uint_fast8_t intr_rx = musb_regs->intr_rx; // a read will clear this interrupt status
  // TU_LOG1("D%2x T%2x R%2x\r\n", is, txis, rxis);

  intr_usb &= musb_regs->intr_usben; /* Clear disabled interrupts */
  if (intr_usb & USB_IS_DISCON) {
  }
  if (intr_usb & USB_IS_SOF) {
    dcd_event_bus_signal(rhport, DCD_EVENT_SOF, true);
  }
  if (intr_usb & USB_IS_RESET) {
    process_bus_reset(rhport);
  }
  if (intr_usb & USB_IS_RESUME) {
    dcd_event_bus_signal(rhport, DCD_EVENT_RESUME, true);
  }
  if (intr_usb & USB_IS_SUSPEND) {
    dcd_event_bus_signal(rhport, DCD_EVENT_SUSPEND, true);
  }

  intr_tx &= musb_regs->intr_txen; /* Clear disabled interrupts */
  if (intr_tx & TU_BIT(0)) {
    process_ep0(rhport);
    intr_tx &= ~TU_BIT(0);
  }
  while (intr_tx) {
    unsigned const num = __builtin_ctz(intr_tx);
    process_edpt_n(rhport, tu_edpt_addr(num, TUSB_DIR_IN));
    intr_tx &= ~TU_BIT(num);
  }

  intr_rx &= musb_regs->intr_rxen; /* Clear disabled interrupts */
  while (intr_rx) {
    unsigned const num = __builtin_ctz(intr_rx);
    process_edpt_n(rhport, tu_edpt_addr(num, TUSB_DIR_OUT));
    intr_rx &= ~TU_BIT(num);
  }

  //Part specific ISR exit
  musb_dcd_int_handler_exit(rhport);
}

#endif
