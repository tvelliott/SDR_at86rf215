PROJ = ddr_in
PIN_DEF = ddr_in.pcf
DEVICE = hx8k
TBENCH_m = ddr_in_tb

all: $(PROJ).rpt $(PROJ).bin 
test: $(TBENCH_m).vcd

%.blif: %.v
	yosys -p 'synth_ice40 -top ddr_in -blif $@' $<

%.asc: $(PIN_DEF) %.blif
	#arachne-pnr -d $(subst hx,,$(subst lp,,$(DEVICE))) -o $@ -p $^
	arachne-pnr --max-passes 2000 -d $(subst hx,,$(subst lp,,$(DEVICE))) -o $@ -w $^

%.bin: %.asc
	icepack $< $@

%.rpt: %.asc
	icetime -d $(DEVICE) -mtr $@ $<

%.vcd: $(TBENCH_m).v
	iverilog -o $(TBENCH_m).dsn $(TBENCH_m).v 
	vvp $(TBENCH_m).dsn 
	gtkwave ddr_in.vcd 

prog: $(PROJ).bin
	iceprog $<

sudo-prog: $(PROJ).bin
	@echo 'Executing prog as root!!!'
	sudo iceprog $<

clean:
	rm -f $(PROJ).blif $(PROJ).asc $(PROJ).rpt $(PROJ).bin $(PROJ).vcd $(PROJ).dsn outputs.txt *.dsn

.SECONDARY:
.PHONY: all prog clean
