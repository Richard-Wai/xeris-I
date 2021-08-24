######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#						  XERIS	 CORE								 #
#						 FLATLINE%OMNI$								 #
#																	 #
######################################################################

TARGET=OMNI

#Machine-dependent
ifeq ($(ARCH),avr5)
PFX_OMNI        =./arch/avr5/
endif

.PHONY: all clean

all: $(OMNI_O)

%.o: $(PFX_OMNI)%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<


clean:
	rm *.o
