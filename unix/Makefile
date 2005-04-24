# Copyright (c) 2005 David Kelso <david@kelso.id.au>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


PROG = main
SRCS = main.c slip.c decode.c

OBJS = $(SRCS:.c=.o)
LDFLAGS =
CFLAGS = -Wall -Werror -g


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
