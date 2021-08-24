######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#					     Support Group								 #
#						    s/clr%	     							 #
#																	 #
#	                Case File Clear Secretariat 		  			 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=s_clr


LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
LDFLAGS += -e main

#Components
DD_O      =clr.o
M_O       =m.o

CLR_O =$(DD_O) $(M_O)

MACH=./m/avr/m5.S

#Recipies

$(TARGET).elf: $(CLR_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

m.o: $(MACH)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

.PHONY: clean

clean:
	rm *.o

