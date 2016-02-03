CC      = g++
CCFLAGS = -O2 -Wall

SEQ_PROG = SeqCounter
SEQ_OBJS = Auxiliary.o SeqCounter.o

PAR_PROG = ParCounter
PAR_OBJS = Auxiliary.o ParCounter.o


all: $(SEQ_PROG) $(PAR_PROG)

$(SEQ_PROG): $(SEQ_OBJS)
	$(CC) -o $(SEQ_PROG) $(SEQ_OBJS)
$(PAR_PROG): $(PAR_OBJS)
	$(CC) -o $(PAR_PROG) $(PAR_OBJS)

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f $(SEQ_OBJS) $(PAR_OBJS)

realclean: clean
	rm -f $(SEQ_PROG) $(PAR_PROG)


# dependencies
