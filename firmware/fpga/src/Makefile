PROJ = fpga_top
PIN_DEF = pinout.pcf
DEVICE = hx8k

all: $(PROJ).rpt $(PROJ).bin

#%.blif: %.v
%.json: %.v ddr_in.v ddr_out.v bram_256_16.v cic.v
	#yosys -l /tmp/yosys.out -q -p 'synth_ice40 -top top -blif $@' $<
	yosys -q -p 'synth_ice40 -top top -json $@' $<

#%.asc: $(PIN_DEF) %.blif
%.asc: $(PIN_DEF) %.json
	#arachne-pnr -r -m 90 -d $(subst hx,,$(subst lp,,$(DEVICE))) -o $@ -p $^ -P ct256
	nextpnr-ice40 --seed 6 --freq 100 --$(DEVICE) --json $(PROJ).json --asc $@ --pcf $<
	#nextpnr-ice40 --freq 100 --gui --$(DEVICE) --json $(PROJ).json --asc $@ --pcf $<

%.bin: %.asc
	icepack $< $@

%.rpt: %.asc
	icetime -d $(DEVICE) -mtr $@ $<

clean:
	rm -f $(PROJ).blif $(PROJ).asc $(PROJ).bin

.PHONY: all prog clean
