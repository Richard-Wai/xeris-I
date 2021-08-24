######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#						  XERIS	 CORE								 #
#						    OPSEC%	     							 #
#																	 #
#	                Operations Secretariat							 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=opsec

CPPFLAGS += -I$(abspath ./include)
export CPPFLAGS

LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
export LDFLAGS

#Components
SYS_O     =sys.o msys.o
CDD_O     =cdd.o mcdd.o
XDM_O     =xdm.o mxdm.o
XS_O      =xs.o mxs.o
OS_O      =os.o os_eod.o
INT_O     =int.o mint.o
TX_O      =tx.o

OPSEC_O =$(SYS_O) $(INT_O) $(CDD_O) $(XDM_O) $(XS_O) $(OS_O) $(TX_O) 

export SYS_O CDD_O XDM_O XS_O OS_O INT_O TX_O

#Recipies

$(TARGET).elf: $(OPSEC_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

$(SYS_O):
	$(MAKE) -C ./sys all
	mv ./sys/*.o ./

$(CDD_O):
	$(MAKE) -C ./cdd all
	mv ./cdd/*.o ./

$(XDM_O):
	$(MAKE) -C ./xdm all
	mv ./xdm/*.o ./

$(XS_O):
	$(MAKE) -C ./xs all
	mv ./xs/*.o ./

$(OS_O):
	$(MAKE) -C ./os all
	mv ./os/*.o ./

$(INT_O):
	$(MAKE) -C ./int all
	mv ./int/*.o ./

$(TX_O):
	$(MAKE) -C ./tx all
	mv ./tx/*.o ./


.PHONY: clean

clean:
	rm *.o

