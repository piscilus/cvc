.PHONY: debug release clean
.DEFAULT_GOAL = debug

TARGET = cvc

SOURCES  = main.c
SOURCES += lib/cargs/cargs.c

INCLUDES  = -I.
INCLUDES += -Ilib/cargs

OBJECTDIR = ./objects
OBJECTDIR_DBG = ./$(OBJECTDIR)/debug
OBJECTDIR_REL = ./$(OBJECTDIR)/release

CC = gcc
CFLAGS =\
	-c\
	-std=c99\
	-Wall\
	-Wextra\
	-Wpedantic
CFLAGS_DBG =\
	-fsanitize=undefined\
	-fsanitize-undefined-trap-on-error\
	-g\
	-O0
CFLAGS_REL =\
	-DNDEBUG\
	-O1

LD = gcc
LDFLAGS =
LDFLAGS_DBG =\
	-fsanitize=undefined\
	-fsanitize-undefined-trap-on-error\
	-Wl,-Map -Wl,debug/$(TARGET).map
LDFLAGS_REL =

OBJECTS_REL = $(addprefix $(OBJECTDIR_REL)/, $(SOURCES:.c=.o) )
OBJECTS_DBG = $(addprefix $(OBJECTDIR_DBG)/, $(SOURCES:.c=.o) )

$(OBJECTDIR_REL)/%.o: %.c
	-mkdir -p $(@D)
	$(CC) $(INCLUDES) $(CFLAGS) $(CFLAGS_REL) -o $@ -l $(@D) $<

$(OBJECTDIR_DBG)/%.o: %.c
	-mkdir -p $(@D)
	$(CC) $(INCLUDES) $(CFLAGS) $(CFLAGS_DBG) -o $@ -l $(@D) $<

debug/$(TARGET): $(OBJECTS_DBG)
	-mkdir -p $(@D)
	$(LD) $(OBJECTS_DBG) $(LDFLAGS) $(LDFLAGS_DBG) -o $@

release/$(TARGET): $(OBJECTS_REL)
	-mkdir -p $(@D)
	$(LD) $(OBJECTS_REL) $(LDFLAGS) $(LDFLAGS_REL) -o $@

debug: debug/$(TARGET)
release: release/$(TARGET)

clean:
	rm -rfd $(OBJECTDIR)
	rm -rfd debug
	rm -rfd release
