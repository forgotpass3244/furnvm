
CC = cc

# -Wall -Wextra -O3 -march=native -flto
CFLAGS  = -Wall -Wextra -O3 -march=native -flto # asan: -g -Wall -Wextra -fsanitize=address
PICFLAGS = -fPIC
LDFLAGS = -Wl,-rpath,'$$ORIGIN' # asan: -fsanitize=address

BUILDDIR = build
LIBNAME  = libfurnvm.so
LIB_SRCDIR = furnvmsrc

MAIN_OBJ = $(BUILDDIR)/CLI.o
DISASM_OBJ = $(BUILDDIR)/Disassembler.o

LIB_OBJS = \
	$(BUILDDIR)/Memory.o \
	$(BUILDDIR)/Machine.o \
	$(BUILDDIR)/System.o \
	$(BUILDDIR)/SyscallRequester.o \
	$(BUILDDIR)/File.o \
	$(BUILDDIR)/Directory.o

all: $(BUILDDIR)/furnvm

# Final executable
$(BUILDDIR)/furnvm: $(MAIN_OBJ) $(DISASM_OBJ) $(BUILDDIR)/$(LIBNAME)
	$(CC) $(MAIN_OBJ) $(DISASM_OBJ) -L$(BUILDDIR) -lfurnvm $(LDFLAGS) -o $@

# Shared library
$(BUILDDIR)/$(LIBNAME): $(LIB_OBJS)
	$(CC) -shared $(LIB_OBJS) $(LDFLAGS) -o $@

# Main object (no PIC needed)
$(BUILDDIR)/CLI.o: CLI.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Disassembler object (explicit)
$(BUILDDIR)/Disassembler.o: ./Disassembler.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Library objects (PIC)
$(BUILDDIR)/%.o: $(LIB_SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(PICFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
