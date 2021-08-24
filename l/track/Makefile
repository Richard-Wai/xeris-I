######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#					     Logistics Group							 #
#					     Track Registrar							 #
#							l/track%								 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=l_track

LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
LDFLAGS += -e main
export LDFLAGS

#Components
TRACK_O =op_tx.o track.o

export CORE_O M_O

#Recipies

$(TARGET).elf: $(TRACK_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;



.PHONY: clean

clean:
	rm *.o

