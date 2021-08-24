######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#					    Facilities Group							 #
#						    f/rdc%	     							 #
#																	 #
#	                Recurring Dispatch Commission 		  			 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=f_rdc

CPPFLAGS += -I$(abspath ./include)
export CPPFLAGS

LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
LDFLAGS += -e main

#Components
RDC_O =globl.o rdc.o sub.o ctl.o agt.o tx.o

#Recipies

$(TARGET).elf: $(RDC_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

.PHONY: clean

clean:
	rm *.o

