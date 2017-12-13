#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "include/IRSend.h"
#include "driver/rmt.h"

#define CLK_DIV           100
#define TICK_10_US        (80000000 / CLK_DIV / 100000)

#define NEC_BITS          32
#define NEC_HDR_MARK    9000
#define NEC_HDR_SPACE   4500
#define NEC_BIT_MARK     560
#define NEC_ONE_SPACE   1690
#define NEC_ZERO_SPACE   560
#define NEC_RPT_SPACE   2250

uint8_t IRSend_Pin;
uint8_t IRSend_Chanel;

static inline void IRSendItem(rmt_item32_t* item, int high_us, int low_us)
{
  item->level0 = 1;
  item->duration0 = (high_us) / 10 * TICK_10_US;
  item->level1 = 0;
  item->duration1 = (low_us) / 10 * TICK_10_US;
}

static void IRSendHeader(rmt_item32_t* item)
{
    IRSendItem(item, NEC_HDR_MARK, NEC_HDR_SPACE);
}

static void IRSendBitOne(rmt_item32_t* item)
{
    IRSendItem(item, NEC_BIT_MARK, NEC_ONE_SPACE);
}

static void IRSendBitZero(rmt_item32_t* item)
{
    IRSendItem(item, NEC_BIT_MARK, NEC_ZERO_SPACE);
}

static void nec_fill_item_end(rmt_item32_t* item)
{
    IRSendItem(item, NEC_BIT_MARK, 0);
}

static void IRSendBuildItem(int channel, rmt_item32_t* item, uint16_t addr, uint16_t cmd_data)
{
  IRSendHeader(item);
  item++;
  for(uint8_t j = 0; j < 16; j++)
  {
    if(addr & 0x1)
    {
      IRSendBitOne(item);
    }
    else
    {
      IRSendBitZero(item);
    }
    item++;
    addr = addr >> 1;
  }

  for(uint8_t j = 0; j < 16; j++)
  {
    if(cmd_data & 0x1)
    {
      IRSendBitOne(item);
    }
    else
    {
      IRSendBitZero(item);
    }
    item++;
    cmd_data = cmd_data >> 1;
  }
  nec_fill_item_end(item);
}

void IRSendInit(uint8_t pin, uint8_t port)
{
  IRSend_Pin = pin;
  IRSend_Chanel = port;

  rmt_config_t IRSend;
  IRSend.rmt_mode = RMT_MODE_TX;
  IRSend.channel = IRSend_Chanel;
  IRSend.clk_div = CLK_DIV;
  IRSend.gpio_num = IRSend_Pin;
  IRSend.mem_block_num = 1;
  IRSend.tx_config.loop_en = false;
  IRSend.tx_config.carrier_freq_hz = 38000; //NEC 38kHz
  IRSend.tx_config.carrier_duty_percent = 50;
  IRSend.tx_config.carrier_level = RMT_CARRIER_LEVEL_HIGH;
  IRSend.tx_config.carrier_en = true;
  IRSend.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
  IRSend.tx_config.idle_output_en = true;

  ESP_ERROR_CHECK(rmt_config(&IRSend));
  ESP_ERROR_CHECK(rmt_driver_install(IRSend_Chanel, 0, 0));
}

void IRSendIR(uint8_t data)
{
  uint16_t addressSend = 65280;
  uint8_t reverseData = ~data;
  uint16_t dataSend;
  dataSend = reverseData;
  dataSend = dataSend << 8;
  dataSend |= data;

  size_t size = sizeof(rmt_item32_t) * 34;
  rmt_item32_t* item = (rmt_item32_t*) malloc(size);

  IRSendBuildItem(IRSend_Chanel, (rmt_item32_t*) item, addressSend, dataSend);
  printf("%d %d\n", addressSend, dataSend);
  rmt_write_items(IRSend_Chanel, item, 34, true);
  rmt_wait_tx_done(IRSend_Chanel);
  free(item);
}
