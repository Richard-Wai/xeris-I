######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#	                     Bootstrap Shot                    			 #
#																	 #
######################################################################


#CPPFLAGS, CFLAGS, DEFS, TARGET_ARCH are imported from root Makefile #

TARGET=strap

LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)
LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
LDFLAGS += -e main

#Components
MAIN_O  =strap.o 


#Recipies

$(TARGET).elf: $(MAIN_O)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@ 

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;



.PHONY: clean

clean:
	rm *.o

