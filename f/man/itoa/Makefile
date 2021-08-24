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

.PHONY: all clean

all: $(ITOA_O)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;


clean:
	rm *.a *.o

