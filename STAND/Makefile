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

#Recipies

MACH=./avr/kick5.s

.PHONY: all clean

all: stand.o

stand.o: $(MACH)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

clean:
	rm *.a *.o

