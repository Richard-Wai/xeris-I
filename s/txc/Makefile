######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#					     Support Group								 #
#						    s/txc%	     							 #
#																	 #
#	                 Ticket Exchange Secretariat   		  			 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=s_txc


LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
LDFLAGS += -e main

#Components
DD_O      =txc.o txc_m.o


#Recipies

$(TARGET).elf: $(DD_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

txc_m.o: ./m/avr/m5.S
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

.PHONY: clean

clean:
	rm *.o

