cd fpga/src/
make -f Makefile
cd ../../
java compress_fpga_image fpga/src/fpga_top.bin >fpga_image.h
touch fpga.c
make build/sdr_h7.elf
make build/sdr_h7.bin
