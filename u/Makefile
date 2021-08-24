######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=u0

LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
export LDFLAGS

#Components
MAIN_O  =u0.o 
M_O     =u0_m.o


U_O =$(MAIN_O) $(M_O)

export CORE_O M_O

#Recipies

$(TARGET).elf: $(U_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

u0_m.o: u0.S
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;



.PHONY: clean

clean:
	rm *.o

