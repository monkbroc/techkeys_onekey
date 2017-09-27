SHELL=/bin/bash
.DEFAULT_GOAL := all
# Clear suffixes to disable implicit rules
.SUFFIXES:
include Makefile.config

ifeq ($(OS), Windows_NT)
	RM = rmdir /s /q
	MKDIR = mkdir
	fix_path = $(subst /,\,$1)
else
	RM = rm -fr
	MKDIR = mkdir -p
	fix_path = $1
endif

obj/techkeys:
	$(MKDIR) $(call fix_path, obj/techkeys)
obj/techkeys/%.d: %.c | obj/techkeys
	@$(CC) $(CFLAGS) $(OPTS_techkeys) -DPLATFORM_techkeys -MM -MT "$(@:.d=.o) $@" $< > $@
obj/techkeys/%.o: %.c | obj/techkeys
	$(CC) $(CFLAGS) $(OPTS_techkeys) -DPLATFORM_techkeys -c -o $@ $<

#define INCLUDE_TEMPLATE=
#-include $$(SOURCES_$1:%.c=obj/$1/%.d) $$(BASE_SOURCES:%.c=obj/$1/%.d)
#endef
#$(foreach target,$(TARGETS),$(eval $(call INCLUDE_TEMPLATE,$(target))))

bin/$(RESULT_techkeys).elf: $(SOURCES_techkeys:%.c=obj/techkeys/%.o) $(BASE_SOURCES:%.c=obj/techkeys/%.o) | bin
	$(CC) $(CFLAGS) $(LDFLAGS) $(OPTS_techkeys) -o $@ $^

bin:
	$(MKDIR) bin

bin/%.hex: bin/%.elf
	avr-objcopy -O ihex $^ $@

.PHONY: techkeys
techkeys: bin/$(RESULT_techkeys).hex

.PHONY: flash
# Must have DFU programmer installed
# http://dfu-programmer.sourceforge.net/
# With the techkey plugged in the computer's USB, short circuit the
# flash pin (next to the USB connector) for 1 second
# Check that the device appears in lsusb as ID 03eb:2fef Atmel Corp
# Then make flash to program
flash:
	dfu-programmer $(MICROCONTROLLER) erase
	dfu-programmer $(MICROCONTROLLER) flash bin/$(RESULT_techkeys).hex
	dfu-programmer $(MICROCONTROLLER) start



.PHONY: clean all
all: $(TARGETS)

clean:
	$(RM) bin obj
