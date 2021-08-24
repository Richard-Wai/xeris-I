######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#						  XERIS CORE								 #
#						FLATLINE%ULIF$								 #
#																	 #
######################################################################

#Machine-dependent
#avr5
ifeq ($(ARCH),avr5)
## UART
ifeq ($(FLATLINE_ULIF),UART)
IF_DIR+=./arch/avr5/UART
endif

endif

.PHONY: all clean

#CFLAGS, CPPFLAGS imported

all: $(ULIF_O)

ulif.o: ulif.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

if.o:
	$(MAKE) -C $(IF_DIR) $<
	mv $(IF_DIR)/if.o ./

clean:
	rm *.o
