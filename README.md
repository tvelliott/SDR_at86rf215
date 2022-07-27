# SDR_at86rf215
SDR design based on the AT86RF215 transceiver with IQ LVDS / FPGA interface. Ethernet is 100Mbps.
Firmware and HDL was written and operational in 2018. It supports stand-alone reception of P25. The firmware implementation is P25 P1 only. It does not currently do P25 P2 or P25 trunk-tracking, but the hardware is capable.

<BR>To compile:
<BR>install the gcc arm toolchain version gcc-arm-none-eabi-7-2018-q2-update/   
<BR>https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads/7-2018-q2-update
<BR><BR>In the firmware directory, create a symbolic link to the toolchain: 
<BR>e.g.  ln -s ../gcc-arm-none-eabi-7-2018-q2-update  arm-toolchain
<BR>Then type 'make'
<BR>
<BR>
To compile the verilog code and place-route for the FPGA, you will need to install project ice-storm,Yosys, and nextpnr. In order to run the testbench for the DDR verilog code, you will also need to install iverilog and gtkwave.
<BR>
<BR>https://github.com/YosysHQ/icestorm
<BR>https://github.com/YosysHQ/yosys
<BR>https://github.com/YosysHQ/nextpnr
<BR>https://github.com/steveicarus/iverilog
<BR>https://github.com/gtkwave/gtkwave
<BR><BR>
AT86RF215 12-bit I/Q Receiver PCB / ICE40 BGA-256 FPGA / STM32H743 MCU / 100Mbps Ethernet / Stereo I2S Audio
<img src="https://raw.githubusercontent.com/tvelliott/SDR_at86rf215/main/images/SDR_at86rf215.png">
<BR>
Dual-antenna VHF/UHF Tuner PCB (1MHz to 1200 MHz), IF output is 914 MHz and fed to the AT86RF215 receiver.
<BR>
<img src="https://github.com/tvelliott/SDR_at86rf215/blob/main/images/ext_tuner_pcb.png">
<BR>
FPGA Routing / Usage
<BR>
<img src="https://github.com/tvelliott/SDR_at86rf215/blob/main/images/fpga_place1.png">
