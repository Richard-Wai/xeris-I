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

#Recipies

m_secreg = ./m/avr/m5.S

.PHONY: all clean

all: $(SECREG_O)

pullsrf.o: $(m_secreg)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $^;

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;


clean:
	rm *.a *.o

