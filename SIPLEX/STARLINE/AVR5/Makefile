######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#						  XERIS	 CORE								 #
#						    SIPLEX%	     							 #
#																	 #
#	    Strategic Imparative Planning Liason Executive   			 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=siplex

CPPFLAGS += -I$(abspath ./include)
export CPPFLAGS

LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
export LDFLAGS

#Components
CORE_O  =globl.o siplex.o exec.o init.o route.o int.o
DESKS_O =subdesk.o sysdesk.o
M_O     =mexec.o kick.o assist.o


SIPLEX_O =$(CORE_O) $(DESKS_O) $(M_O)

export CORE_O DESKS_O M_O

#Recipies

$(TARGET).elf: $(SIPLEX_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

$(CORE_O):
	$(MAKE) -C ./core all
	mv ./core/*.o ./

$(DESKS_O):
	$(MAKE) -C ./desks all
	mv ./desks/*.o ./

$(M_O):
	$(MAKE) -C ./m all
	mv ./m/*.o ./

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;



.PHONY: clean

clean:
	rm *.o

