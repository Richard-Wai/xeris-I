######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#					    Facilities Group							 #
#						    f/man%	     							 #
#																	 #
#	                Human Relations Secretariat 		  			 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=f_man

CPPFLAGS += -I$(abspath ./include)
export CPPFLAGS

LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
export LDFLAGS

#Components
STAND_O   =../../STAND/stand.o
DD_O      =man.o
ROM_O     =rom.o
INTERN_O  =intern.o intern_m.o
ITOA_O    =itoa.o
TELCOM_O  =telcom.o telcom_cfg.o
STU_O     =stu.o

MAN_O =$(DD_O) $(ROM_O) $(INTERN_O) $(ITOA_O) $(TELCOM_O) $(STU_O) $(STAND_O)

export DD_O ROM_O INTERN_O ITOA_O TELCOM_O STU_O

#Recipies

$(TARGET).elf: $(MAN_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

$(ROM_O):
	$(MAKE) -C ./rom all
	mv ./rom/*.o ./

$(INTERN_O):
	$(MAKE) -C ./internal all
	mv ./internal/*.o ./

$(ITOA_O):
	$(MAKE) -C ./itoa all
	mv ./itoa/*.o ./

$(TELCOM_O):
	$(MAKE) -C ./telcom all
	mv ./telcom/*.o ./

$(STU_O):
	$(MAKE) -C ./stu all
	mv ./stu/*.o ./

$(STAND_O):
	$(MAKE) -C ../../STAND all

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

.PHONY: clean

clean:
	rm *.o

