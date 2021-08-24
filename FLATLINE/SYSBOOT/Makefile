######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#						  XERIS	 CORE								 #
#					   FLATLINE%SYSBOOT$							 #
#																	 #
######################################################################

#Machine-dependent "_sysboot_kick"
#avr5
ifeq ($(ARCH),avr5)
FLATLINE_HORIZON =./arch/avr5/m5.S
endif

#CFLAGS, CPPFLAGS imported

.PHONY: all clean

all: $(SYSBOOT_O)

sysboot.o: sysboot.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

horizon.o: $(FLATLINE_HORIZON)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

clean:
	rm *.o
