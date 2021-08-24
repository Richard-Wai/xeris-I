######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#						  XERIS	 CORE								 #
#						   FLATLINE%								 #
#																	 #
#	   First Load Assessment Telemetry, Load Initializtion and		 #
#						   Nominal Entry							 #
#																	 #
######################################################################

# This Makefile should be invoked from the root makefile			 #


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=flatline

CPPFLAGS += -I$(abspath ./include)
export CPPFLAGS

LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
export LDFLAGS

#Components
DD_O       =flatline.o
SYSBOOT_O  =sysboot.o horizon.o
ULIF_O     =ulif.o if.o
OMNI_O     =omni.o
export DD_O SYSBOOT_O ULIF_O OMNI_O

FLATLINE_O =$(DD_O) $(SYSBOOT_O) $(ULIF_O) $(OMNI_O)

#Recipies

$(TARGET).elf: $(FLATLINE_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

flatline.o: flatline.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

$(SYSBOOT_O):
	$(MAKE) -C ./SYSBOOT all
	mv ./SYSBOOT/*.o ./

$(ULIF_O):
	$(MAKE) -C ./ULIF all
	mv ./ULIF/*.o ./

$(OMNI_O):
	$(MAKE) -C ./OMNI all
	mv ./OMNI/*.o ./


.PHONY: clean

clean:
	rm *.a *.o

