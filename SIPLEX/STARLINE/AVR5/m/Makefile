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


.PHONY: clean all

all: $(M_O)

%.o: %.S
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

clean:
	rm *.o
