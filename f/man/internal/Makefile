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

MACH=./arch/avr/m5.S

.PHONY: all clean

all: $(INTERN_O)

intern_m.o: $(MACH)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;


clean:
	rm *.a *.o

