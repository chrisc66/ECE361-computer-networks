#CFLAGS := -g -Wall -Werror -D_GNU_SOURCE
CFLAGS := -g -D_GNU_SOURCE

TARGETS := client

# Make sure that 'all' is the first target
all: depend $(TARGETS)

clean:
	rm -rf core *.o $(TARGETS)

realclean: clean
	rm -rf *~ *.bak .depend *.log *.out

tags:
	etags *.c *.h

OBJS := client.o

client: $(OBJS)

depend:
	$(CC) -MM *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
