
//MIT License
//
//Copyright (c) 2018 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.




#ifndef __AT86RF215_H__
#define __AT86RF215_H__

#include <stdint.h>

#define RF_MODE_BBRF (0x00<<4)
#define RF_MODE_RF (0x01<<4)
#define RF_MODE_BBRF09 (0x04<<4)
#define RF_MODE_BBRF24 (0x05<<4)

#define CMD_RF_NOP 0x00
#define CMD_RF_SLEEP 0x01
#define CMD_RF_TRXOFF 0x02
#define CMD_RF_TXPREP 0x03
#define CMD_RF_TX 0x04
#define CMD_RF_RX 0x05
#define CMD_RF_RESET 0x07

#define STATE_TRXOFF 0x02
#define STATE_TXPREP 0x03
#define STATE_TX 0x04
#define STATE_RX 0x05
#define STATE_TRANSITION 0x06
#define STATE_RESET 0x07

#define IRQM_FBLI (1<<7)
#define IRQM_AGCR (1<<6)
#define IRQM_AGCH (1<<5)
#define IRQM_TXFE (1<<4)
#define IRQM_RXEM (1<<3)
#define IRQM_RXAM (1<<2)
#define IRQM_RXFE (1<<1)
#define IRQM_RXFS (1<<0)

#define IRQM_IQIFSF (1<<5)
#define IRQM_TRXERR (1<<4)
#define IRQM_BATLOW (1<<3)
#define IRQM_EDC (1<<2)
#define IRQM_TRXRDY (1<<1)
#define IRQM_WAKEUP (1<<0)

#define RF09_IRQS 0x0000
#define RF24_IRQS 0x0001
#define BBC0_IRQS 0x0002
#define BBC1_IRQS 0x0003
#define RF_RST 0x0005
#define RF_CFG 0x0006
#define RF_CLKO 0x0007
#define RF_BMDVC 0x0008
#define RF_XOC 0x0009
#define RF_IQIFC0 0x000A
#define RF_IQIFC1 0x000B
#define RF_IQIFC2 0x000C
#define RF_PN 0x000D
#define RF_VN 0x000E



#define RF09_IRQM 0x0100
#define RF09_AUXS 0x0101
#define RF09_STATE 0x0102
#define RF09_CMD 0x0103
#define RF09_CS 0x0104
#define RF09_CCF0L 0x0105
#define RF09_CCF0H 0x0106
#define RF09_CNL 0x0107
#define RF09_CNM 0x0108
#define RF09_RXBWC 0x0109
#define RF09_RXDFE 0x010A
#define RF09_AGCC 0x010B
#define RF09_AGCS 0x010C
#define RF09_RSSI 0x010D
#define RF09_EDC 0x010E
#define RF09_EDD 0x010F
#define RF09_EDV 0x0110
#define RF09_RNDV 0x0111
#define RF09_TXCUTC 0x0112
#define RF09_TXDFE 0x0113
#define RF09_PAC 0x0114
#define RF09_PADFE 0x0116
#define RF09_PLL 0x0121
#define RF09_PLLCF 0x0122
#define RF09_TXCI 0x0125
#define RF09_TXCQ 0x0126
#define RF09_TXDACI 0x0127
#define RF09_TXDACQ 0x0128
#define RF24_IRQM 0x0200
#define RF24_AUXS 0x0201
#define RF24_STATE 0x0202
#define RF24_CMD 0x0203
#define RF24_CS 0x0204
#define RF24_CCF0L 0x0205
#define RF24_CCF0H 0x0206
#define RF24_CNL 0x0207
#define RF24_CNM 0x0208
#define RF24_RXBWC 0x0209
#define RF24_RXDFE 0x020A
#define RF24_AGCC 0x020B
#define RF24_AGCS 0x020C
#define RF24_RSSI 0x020D
#define RF24_EDC 0x020E
#define RF24_EDD 0x020F
#define RF24_EDV 0x0210
#define RF24_RNDV 0x0211
#define RF24_TXCUTC 0x0212
#define RF24_TXDFE 0x0213
#define RF24_PAC 0x0214
#define RF24_PADFE 0x0216
#define RF24_PLL 0x0221
#define RF24_PLLCF 0x0222
#define RF24_TXCI 0x0225
#define RF24_TXCQ 0x0226
#define RF24_TXDACI 0x0227
#define RF24_TXDACQ 0x0228
#define BBC0_IRQM 0x0300
#define BBC0_PC 0x0301
#define BBC0_PS 0x0302
#define BBC0_RXFLL 0x0304
#define BBC0_RXFLH 0x0305
#define BBC0_TXFLL 0x0306
#define BBC0_TXFLH 0x0307
#define BBC0_FBLL 0x0308
#define BBC0_FBLH 0x0309
#define BBC0_FBLIL 0x030A
#define BBC0_FBLIH 0x030B
#define BBC0_OFDMPHRTX 0x030C
#define BBC0_OFDMPHRRX 0x030D
#define BBC0_OFDMC 0x030E
#define BBC0_OFDMSW 0x030F
#define BBC0_OQPSKC0 0x0310
#define BBC0_OQPSKC1 0x0311
#define BBC0_OQPSKC2 0x0312
#define BBC0_OQPSKC3 0x0313
#define BBC0_OQPSKPHRTX 0x0314
#define BBC0_OQPSKPHRRX 0x0315
#define BBC0_AFC0 0x0320
#define BBC0_AFC1 0x0321
#define BBC0_AFFTM 0x0322
#define BBC0_AFFVM 0x0323
#define BBC0_AFS 0x0324
#define BBC0_MACEA0 0x0325
#define BBC0_MACEA1 0x0326
#define BBC0_MACEA2 0x0327
#define BBC0_MACEA3 0x0328
#define BBC0_MACEA4 0x0329
#define BBC0_MACEA5 0x032A
#define BBC0_MACEA6 0x032B
#define BBC0_MACEA7 0x032C
#define BBC0_MACPID0F0 0x032D
#define BBC0_MACPID1F0 0x032E
#define BBC0_MACSHA0F0 0x032F
#define BBC0_MACSHA1F0 0x0330
#define BBC0_MACPID0F1 0x0331
#define BBC0_MACPID1F1 0x0332
#define BBC0_MACSHA0F1 0x0333
#define BBC0_MACSHA1F1 0x0334
#define BBC0_MACPID0F2 0x0335
#define BBC0_MACPID1F2 0x0336
#define BBC0_MACSHA0F2 0x0337
#define BBC0_MACSHA1F2 0x0338
#define BBC0_MACPID0F3 0x0339
#define BBC0_MACPID1F3 0x033A
#define BBC0_MACSHA0F3 0x033B
#define BBC0_MACSHA1F3 0x033C
#define BBC0_AMCS 0x0340
#define BBC0_AMEDT 0x0341
#define BBC0_AMAACKPD 0x0342
#define BBC0_AMAACKTL 0x0343
#define BBC0_AMAACKTH 0x0344
#define BBC0_FSKC0 0x0360
#define BBC0_FSKC1 0x0361
#define BBC0_FSKC2 0x0362
#define BBC0_FSKC3 0x0363
#define BBC0_FSKC4 0x0364
#define BBC0_FSKPLL 0x0365
#define BBC0_FSKSFD0L 0x0366
#define BBC0_FSKSFD0H 0x0367
#define BBC0_FSKSFD1L 0x0368
#define BBC0_FSKSFD1H 0x0369
#define BBC0_FSKPHRTX 0x036A
#define BBC0_FSKPHRRX 0x036B
#define BBC0_FSKRPC 0x036C
#define BBC0_FSKRPCONT 0x036D
#define BBC0_FSKRPCOFFT 0x036E
#define BBC0_FSKRRXFLL 0x0370
#define BBC0_FSKRRXFLH 0x0371
#define BBC0_FSKDM 0x0372
#define BBC0_FSKPE0 0x0373
#define BBC0_FSKPE1 0x0374
#define BBC0_FSKPE2 0x0375
#define BBC0_PMUC 0x0380
#define BBC0_PMUVAL 0x0381
#define BBC0_PMUQF 0x0382
#define BBC0_PMUI 0x0383
#define BBC0_PMUQ 0x0384
#define BBC0_CNTC 0x0390
#define BBC0_CNT0 0x0391
#define BBC0_CNT1 0x0392
#define BBC0_CNT2 0x0393
#define BBC0_CNT3 0x0394
#define BBC1_IRQM 0x0400
#define BBC1_PC 0x0401
#define BBC1_PS 0x0402
#define BBC1_RXFLL 0x0404
#define BBC1_RXFLH 0x0405
#define BBC1_TXFLL 0x0406
#define BBC1_TXFLH 0x0407
#define BBC1_FBLL 0x0408
#define BBC1_FBLH 0x0409
#define BBC1_FBLIL 0x040A
#define BBC1_FBLIH 0x040B
#define BBC1_OFDMPHRTX 0x040C
#define BBC1_OFDMPHRRX 0x040D
#define BBC1_OFDMC 0x040E
#define BBC1_OFDMSW 0x040F
#define BBC1_OQPSKC0 0x0410
#define BBC1_OQPSKC1 0x0411
#define BBC1_OQPSKC2 0x0412
#define BBC1_OQPSKC3 0x0413
#define BBC1_OQPSKPHRTX 0x0414
#define BBC1_OQPSKPHRRX 0x0415
#define BBC1_AFC0 0x0420
#define BBC1_AFC1 0x0421
#define BBC1_AFFTM 0x0422
#define BBC1_AFFVM 0x0423
#define BBC1_AFS 0x0424
#define BBC1_MACEA0 0x0425
#define BBC1_MACEA1 0x0426
#define BBC1_MACEA2 0x0427
#define BBC1_MACEA3 0x0428
#define BBC1_MACEA4 0x0429
#define BBC1_MACEA5 0x042A
#define BBC1_MACEA6 0x042B
#define BBC1_MACEA7 0x042C
#define BBC1_MACPID0F0 0x042D
#define BBC1_MACPID1F0 0x042E
#define BBC1_MACSHA0F0 0x042F
#define BBC1_MACSHA1F0 0x0430
#define BBC1_MACPID0F1 0x0431
#define BBC1_MACPID1F1 0x0432
#define BBC1_MACSHA0F1 0x0433
#define BBC1_MACSHA1F1 0x0434
#define BBC1_MACPID0F2 0x0435
#define BBC1_MACPID1F2 0x0436
#define BBC1_MACSHA0F2 0x0437
#define BBC1_MACSHA1F2 0x0438
#define BBC1_MACPID0F3 0x0439
#define BBC1_MACPID1F3 0x043A
#define BBC1_MACSHA0F3 0x043B
#define BBC1_MACSHA1F3 0x043C
#define BBC1_AMCS 0x0440
#define BBC1_AMEDT 0x0441
#define BBC1_AMAACKPD 0x0442
#define BBC1_AMAACKTL 0x0443
#define BBC1_AMAACKTH 0x0444
#define BBC1_FSKC0 0x0460
#define BBC1_FSKC1 0x0461
#define BBC1_FSKC2 0x0462
#define BBC1_FSKC3 0x0463
#define BBC1_FSKC4 0x0464
#define BBC1_FSKPLL 0x0465
#define BBC1_FSKSFD0L 0x0466
#define BBC1_FSKSFD0H 0x0467
#define BBC1_FSKSFD1L 0x0468
#define BBC1_FSKSFD1H 0x0469
#define BBC1_FSKPHRTX 0x046A
#define BBC1_FSKPHRRX 0x046B
#define BBC1_FSKRPC 0x046C
#define BBC1_FSKRPCONT 0x046D
#define BBC1_FSKRPCOFFT 0x046E
#define BBC1_FSKRRXFLL 0x0470
#define BBC1_FSKRRXFLH 0x0471
#define BBC1_FSKDM 0x0472
#define BBC1_FSKPE0 0x0473
#define BBC1_FSKPE1 0x0474
#define BBC1_FSKPE2 0x0475
#define BBC1_PMUC 0x0480
#define BBC1_PMUVAL 0x0481
#define BBC1_PMUQF 0x0482
#define BBC1_PMUI 0x0483
#define BBC1_PMUQ 0x0484
#define BBC1_CNTC 0x0490
#define BBC1_CNT0 0x0491
#define BBC1_CNT1 0x0492
#define BBC1_CNT2 0x0493
#define BBC1_CNT3 0x0494
#define BBC0_FBRXS 0x2000
#define BBC0_FBRXE 0x27FE
#define BBC0_FBTXS 0x2800
#define BBC0_FBTXE 0x2FFE
#define BBC1_FBRXS 0x3000
#define BBC1_FBRXE 0x37FE
#define BBC1_FBTXS 0x3800
#define BBC1_FBTXE 0x3FFE

//generic form
#define RF09_BASE 0x0100
#define RF24_BASE 0x0200
#define BBC0_BASE 0x0300
#define BBC1_BASE 0x0400
#define FB0_BASE 0x2000
#define FB1_BASE 0x3000

#define RFXX_IRQM 0x0000
#define RFXX_AUXS 0x0001
#define RFXX_STATE 0x0002
#define RFXX_CMD 0x0003
#define RFXX_CS 0x0004
#define RFXX_CCF0L 0x0005
#define RFXX_CCF0H 0x0006
#define RFXX_CNL 0x0007
#define RFXX_CNM 0x0008
#define RFXX_RXBWC 0x0009
#define RFXX_RXDFE 0x000A
#define RFXX_AGCC 0x000B
#define RFXX_AGCS 0x000C
#define RFXX_RSSI 0x000D
#define RFXX_EDC 0x000E
#define RFXX_EDD 0x000F
#define RFXX_EDV 0x0010
#define RFXX_RNDV 0x0011
#define RFXX_TXCUTC 0x0012
#define RFXX_TXDFE 0x0013
#define RFXX_PAC 0x0014
#define RFXX_PADFE 0x0016
#define RFXX_PLL 0x0021
#define RFXX_PLLCF 0x0022
#define RFXX_TXCI 0x0025
#define RFXX_TXCQ 0x0026
#define RFXX_TXDACI 0x0027
#define RFXX_TXDACQ 0x0028


#define BBCX_IRQM 0x0000
#define BBCX_PC 0x0001
#define BBCX_PS 0x0002
#define BBCX_RXFLL 0x0004
#define BBCX_RXFLH 0x0005
#define BBCX_TXFLL 0x0006
#define BBCX_TXFLH 0x0007
#define BBCX_FBLL 0x0008
#define BBCX_FBLH 0x0009
#define BBCX_FBLIL 0x000A
#define BBCX_FBLIH 0x000B
#define BBCX_OFDMPHRTX 0x000C
#define BBCX_OFDMPHRRX 0x000D
#define BBCX_OFDMC 0x000E
#define BBCX_OFDMSW 0x000F
#define BBCX_OQPSKC0 0x0010
#define BBCX_OQPSKC1 0x0011
#define BBCX_OQPSKC2 0x0012
#define BBCX_OQPSKC3 0x0013
#define BBCX_OQPSKPHRTX 0x0014
#define BBCX_OQPSKPHRRX 0x0015
#define BBCX_AFC0 0x0020
#define BBCX_AFC1 0x0021
#define BBCX_AFFTM 0x0022
#define BBCX_AFFVM 0x0023
#define BBCX_AFS 0x0024
#define BBCX_MACEA0 0x0025
#define BBCX_MACEA1 0x0026
#define BBCX_MACEA2 0x0027
#define BBCX_MACEA3 0x0028
#define BBCX_MACEA4 0x0029
#define BBCX_MACEA5 0x002A
#define BBCX_MACEA6 0x002B
#define BBCX_MACEA7 0x002C
#define BBCX_MACPID0F0 0x002D
#define BBCX_MACPID1F0 0x002E
#define BBCX_MACSHA0F0 0x002F
#define BBCX_MACSHA1F0 0x0030
#define BBCX_MACPID0F1 0x0031
#define BBCX_MACPID1F1 0x0032
#define BBCX_MACSHA0F1 0x0033
#define BBCX_MACSHA1F1 0x0034
#define BBCX_MACPID0F2 0x0035
#define BBCX_MACPID1F2 0x0036
#define BBCX_MACSHA0F2 0x0037
#define BBCX_MACSHA1F2 0x0038
#define BBCX_MACPID0F3 0x0039
#define BBCX_MACPID1F3 0x003A
#define BBCX_MACSHA0F3 0x003B
#define BBCX_MACSHA1F3 0x003C
#define BBCX_AMCS 0x0040
#define BBCX_AMEDT 0x0041
#define BBCX_AMAACKPD 0x0042
#define BBCX_AMAACKTL 0x0043
#define BBCX_AMAACKTH 0x0044
#define BBCX_FSKC0 0x0060
#define BBCX_FSKC1 0x0061
#define BBCX_FSKC2 0x0062
#define BBCX_FSKC3 0x0063
#define BBCX_FSKC4 0x0064
#define BBCX_FSKPLL 0x0065
#define BBCX_FSKSFD0L 0x0066
#define BBCX_FSKSFD0H 0x0067
#define BBCX_FSKSFD1L 0x0068
#define BBCX_FSKSFD1H 0x0069
#define BBCX_FSKPHRTX 0x006A
#define BBCX_FSKPHRRX 0x006B
#define BBCX_FSKRPC 0x006C
#define BBCX_FSKRPCONT 0x006D
#define BBCX_FSKRPCOFFT 0x006E
#define BBCX_FSKRRXFLL 0x0070
#define BBCX_FSKRRXFLH 0x0071
#define BBCX_FSKDM 0x0072
#define BBCX_FSKPE0 0x0073
#define BBCX_FSKPE1 0x0074
#define BBCX_FSKPE2 0x0075
#define BBCX_PMUC 0x0080
#define BBCX_PMUVAL 0x0081
#define BBCX_PMUQF 0x0082
#define BBCX_PMUI 0x0083
#define BBCX_PMUQ 0x0084
#define BBCX_CNTC 0x0090
#define BBCX_CNT0 0x0091
#define BBCX_CNT1 0x0092
#define BBCX_CNT2 0x0093
#define BBCX_CNT3 0x0094


#define BBCX_FBRXS 0x0000
#define BBCX_FBRXE 0x07FE
#define BBCX_FBTXS 0x0800
#define BBCX_FBTXE 0x0FFE

void atrf_dump_regs();
void do_tx_test( uint16_t RF_BASE );
void transmit_frame( uint16_t RF_BASE, uint8_t *buffer, int len );
void atrf_init_900();
double set_frequency_900mhz( double freq );
void atrf_init_24();
void check_tx_sync();
void set_iqifc1_mode( uint8_t mode );
uint8_t check_at86_rf_irq( uint16_t RF_IRQS_ADDR );
uint8_t check_at86_bbx_irq( uint16_t BBX_IRQS_ADDR );
void at86rf_read_id();
void atrf_check_init();
void _do_channel_scan();
void clear_cf_histo();
int at86rf215_set_channel_24( int channel );
int at86rf215_set_channel_900( int channel );
void lock_agc( void );
void unlock_agc( void );
void wait_for_tx_rx_ready( uint16_t RF_IRQS_ADDR );
uint8_t at86rf215_write_single( uint16_t addr, uint8_t val );
uint8_t at86rf215_read_single( uint16_t addr );
extern void rf900_int_config( void );
extern int testing_tx_idx;
extern int do_channel_scan;
extern int is_fcc;
extern uint8_t tx_buffer[1514];
#endif
