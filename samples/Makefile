# makefile discription.
# basic build file for mruby library (Ruby part)

# compiler, linker (gcc), archiver, parser generator
export CC = gcc
export LL = gcc
export AR = ar
export YACC = bison

# project-specific macros
# extension of the executable-file is modifiable(.exe .out ...)
BASEDIR = .
TARGET := hello
LIBR := ../lib/libmrubyvm.a
MLIB := $(TARGET).o
CLIB := $(TARGET).c
INIT := init_$(TARGET).c
DLIB := $(TARGET).ctmp
RLIB := $(TARGET).rb
DEPLIB := $(TARGET).d driver.d
OBJS :=  driver.o $(MLIB) ../mrblib/mrblib.o
CAT := /bin/cat

export CP := cp
export RM_F := rm -f
export CAT := cat


# libraries, includes
LIBS = -lm
INCLUDES = -I$(BASEDIR)/../src -I$(BASEDIR)/../include

ifeq ($(strip $(COMPILE_MODE)),)
  # default compile option
  COMPILE_MODE = debug
endif

ifeq ($(COMPILE_MODE),debug)
  CFLAGS = -g
else ifeq ($(COMPILE_MODE),release)
  CFLAGS = -O3
else ifeq ($(COMPILE_MODE),small)
  CFLAGS = -Os
endif

ALL_CFLAGS = -Wall -Werror-implicit-function-declaration $(CFLAGS)
ifeq ($(OS),Windows_NT)
  MAKE_FLAGS = CC=$(CC) LL=$(LL) ALL_CFLAGS="$(ALL_CFLAGS)"
else
  MAKE_FLAGS = CC='$(CC)' LL='$(LL)' ALL_CFLAGS='$(ALL_CFLAGS)'
endif

# mruby compiler and test driver
ifeq ($(OS),Windows_NT)
MRBC_CMD = ../bin/mrbc.exe
EXE := $(TARGET).exe
else
MRBC_CMD = $(HOME)/git/mruby/bin/mrbc
EXE := $(TARGET)
endif


##############################
# generic build targets, rules

.PHONY : test
all : $(EXE)

obj : $(OBJS)

# executable constructed using linker from object files
$(EXE) : $(OBJS) $(LIBR)
	$(LL) -o $@ $(CFLAGS) $(OBJS) $(LIBR) $(LIBS)

-include $(OBJS:.o=.d)

# objects compiled from source
$(OBJS) : %.o : %.c
	$(CC) $(ALL_CFLAGS) -MMD $(INCLUDES) -c $< -o $@

# Compile C source from merged mruby source
$(CLIB) : $(INIT) $(RLIB)
	$(MRBC_CMD) -Chello_irep -o$(DLIB) $(RLIB); $(CAT) $(INIT) $(DLIB) > $@

# clean up
.PHONY : clean
clean :
	@echo "make: removing targets, objects and depend files of `pwd`"
	-$(RM_F) $(MLIB) $(CLIB) $(DLIB) $(DEPLIB) $(OBJS) $(EXE)
