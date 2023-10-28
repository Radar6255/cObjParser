# Found from https://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

PROG= ../CGame/lib_parser.a
TEST_PROG = a.out
CC= gcc
GCCFLAGS =      -g
CFLAGS =        -g
WARNFLAGS = -Wall -ansi -std=gnu11
WARN_ADDITIONAL = -Wshadow

HFILES= $(call rwildcard,.,*.h)
CFILES= $(call rwildcard,.,*.c)
IMGFILES = $(call rwildcard,assets,*.png)

# SOURCEFILES= $(CPPFILES) $(HFILES)	
OFILES=$(CFILES:.c=.o)

all: $(PROG) 

SUFFIXES: .C .o 

%.o:%.c $(HFILES)
	$(CC) -o $*.o -c $(GCCFLAGS) $(WARNFLAGS) $*.c -pthread 

$(PROG): $(OFILES)
	# $(CC) -o $(TEST_PROG) $(OFILES) -pthread
	ar -rcs $(PROG) $(OFILES)

clean:
	rm -f $(OFILES) $(PROG) 

# build: game/glFunctions/headers/texturesList.h


# %.o : %.c
# 	$(CC) -o $*.o -c $(GCCFLAGS) $(WARNFLAGS) $*.c -lm
