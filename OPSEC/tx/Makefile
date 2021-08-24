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

#Recipies

MACH=./m/avr/m5.S

.PHONY: all clean

all: $(TX_O)

tx.o: $(MACH)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

clean:
	rm *.a *.o

