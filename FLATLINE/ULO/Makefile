######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#						  XERIS	 CORE								 #
#						 FLATLINE%ULO$								 #
#																	 #
######################################################################

TARGET=ULO

#Machine-independent

#CFLAGS, CPPFLAGS imported

.PHONY: all clean

all: $(ULO_O)

ulo.o: ulo.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $^

clean:
	rm *.o
