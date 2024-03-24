.PHONY: all clean

EXECUTABLE = cvc

SRCDIR = .
OBJDIR = obj
BINDIR = bin

SOURCES = main.c
SOURCES += lib/cargs/cargs.c

INCLUDES = -I.
INCLUDES += -Ilib/cargs

OBJS = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o) )

CC = gcc
CFLAGS =\
	-c\
	-std=c11\
	-Wall\
	-Wextra\
	-Wpedantic\
	-fsanitize=undefined\
	-fsanitize-undefined-trap-on-error\
	-g\
	-D_DEFAULT_SOURCE

LD = gcc
LDFLAGS =\
	-fsanitize=undefined\
	-fsanitize-undefined-trap-on-error

all: $(BINDIR)/$(EXECUTABLE)

incs:
	gcc -E -Wp,-v -

$(OBJDIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(BINDIR)/$(EXECUTABLE): $(OBJS)
	mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(BINDIR)/$(EXECUTABLE)
