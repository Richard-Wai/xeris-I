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


#Recipies

.PHONY: all clean

m_LD0_AVR5 =./m/avr5/ld0.S

all: $(CTXM_O)

ld0.o: $(m_LD0_AVR5)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $^;

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;


clean:
	rm *.a *.o

