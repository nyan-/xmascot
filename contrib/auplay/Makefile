# Makefile for auplay

# compile command
CC = gcc

# compile debug options
CDEBUGFLAGS=\
#	-g

# compile options
CCOPTIONS=\
	-O2

#file remove command
RM = rm -f

OBJS = auplay.o

auplay : $(OBJS)
	$(RM) $@
	$(CC) -o $@ $(CDEBUGFLAG) $(CCOPTIONS) $(OBJS)

auplay.o : auplay.c auplay.h
	$(CC) $(CDEBUGFLAG) $(CCOPTIONS) -c $<

clean :
	$(RM) auplay *.CKP *.ln *.BAK *.bak *.o core errs ,* *~ *.a .emacs_* tags TAGS make.log MakeOut    *.core "#"*
