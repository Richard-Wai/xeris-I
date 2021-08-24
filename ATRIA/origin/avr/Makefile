######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#						  XERIS	 CORE								 #
#						    ATRIA%	     							 #
#																	 #
#	   Autonomous Terminal Registrar and Integration Authority		 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=atria

CPPFLAGS += -I$(abspath ./include)
export CPPFLAGS

LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
export LDFLAGS

#Components
CORE_O   =core.o
DD_O     =atria.o kick.o
SECREG_O =secreg.o pullsrf.o svc_opcert.o
CTXM_O   =ctxm.o initgen.o ld0.o
INTR_O   =intr.o sisr.o
ATRIA_O  =$(CORE_O) $(DD_O) $(SECREG_O) $(CTXM_O) $(INTR_O)

export DD_O
export SECREG_O
export CTXM_O
export INTR_O

#Recipies

$(TARGET).elf: $(ATRIA_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@

$(CORE_O): core/avr/core5.s
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;	

$(DD_O):
	$(MAKE) -C ./dd all
	mv ./dd/*.o ./

$(SECREG_O):
	$(MAKE) -C ./secreg all
	mv ./secreg/*.o ./

$(CTXM_O):
	$(MAKE) -C ./ctxm all
	mv ./ctxm/*.o ./

$(INTR_O):
	$(MAKE) -C ./intr all
	mv ./intr/*.o ./

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

%.o: %.s
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;


.PHONY: clean

clean:
	rm *.a *.o

