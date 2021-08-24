######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#					     Support Group								 #
#						    s/isu%	     							 #
#																	 #
#	                Case File Issue Secretariat 		  			 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=s_isu


LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
export LDFLAGS

#Components
DD_O      =isu.o
M_O       =m.o

ISU_O =$(DD_O) $(M_O)

MACH=./m/avr/m5.S

#Recipies

$(TARGET).elf: $(ISU_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

m.o: $(MACH)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

.PHONY: clean

clean:
	rm *.o

