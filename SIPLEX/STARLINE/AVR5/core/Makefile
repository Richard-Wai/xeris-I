######################################################################
#																	 #
#				  XERIS/APEX System I, Release 1					 #
#																	 #
#						  XERIS	 CORE								 #
#						    SIPLEX%	     							 #
#																	 #
#	    Strategic Imparative Planning Liason Executive   			 #
#																	 #
######################################################################



all: $(CORE_O)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

.PHONY: all clean

clean:
	rm *.o

