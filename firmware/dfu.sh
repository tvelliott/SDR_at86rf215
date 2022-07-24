#dfu-suffix -a build/sdr_h7_DFU.bin -v 0483 -p df11 MOVED this to Makefile
dfu-util --device 0483:df11 -a 0 -s 0x08000000 -D build/sdr_h7_DFU.bin
