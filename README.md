# SDR_at86rf215
SDR design based on the AT86RF215 transceiver with IQ LVDS / FPGA interface. Ethernet is 100Mbps.
Firmware and HDL was written and operational in 2018. It supports stand-alone reception of P25. This implementation does not do P25 P2 or P25 Trunk-tracking.

<BR>To compile:
<BR>install the gcc arm toolchain version gcc-arm-none-eabi-7-2018-q2-update/
<BR>In the firmware directory, create a symbolic link to the toolchain: e.g.  ln -s ../gcc-arm-none-eabi-7-2018-q2-update arm-toolchain
<BR>Then type 'make'
<BR>
<img src="https://raw.githubusercontent.com/tvelliott/SDR_at86rf215/main/images/SDR_at86rf215.png">
