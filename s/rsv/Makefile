######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#					     Support Group								 #
#						    s/rsv%	     							 #
#																	 #
#	                Case File Reserve Secretariat 		  			 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=s_rsv


LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
export LDFLAGS

#Components
DD_O      =rsv.o
M_O       =m.o

RSV_O =$(DD_O) $(M_O)

MACH=./m/avr/m5.S

#Recipies

$(TARGET).elf: $(RSV_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

m.o: $(MACH)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

.PHONY: clean

clean:
	rm *.o

