PROG = main
SRCS = main.c slip.c slipchar.c

OBJS = $(SRCS:.c=.o)
LDFLAGS =
CFLAGS = -Wall


# TARGETS
all: .depend $(PROG)

# Default configuration for making .o files from .c files
.c.o:
	$(CC) $(CFLAGS) -c $<


# Link the program from the object files
$(PROG): $(OBJS)
	@echo Linking
	$(CC) $(LDFLAGS) $(OBJS) -o $(PROG)

clean:
	@echo Cleaning
	-rm -f .depend
	-rm -f $(OBJS)
	-rm -f $(PROG)

.depend: $(SRCS)
	@echo Making dependency tree
	mkdep $(SRCS)

# Include the dependency tree
# This gives the object file for each source file and all it's dependencies
# including source and header files. Since no commands are listed, they are
# compiled with the default rule as listed above
.if exists(.depend)
include .depend
.endif
