CFLAG := -Wall -Werror -std=c11 -pedantic -Isrc
CLIBS := -lpthread -lz -lssl -lcrypto

TARGET := main
SRCDIR := src
OBJDIR := build/obj

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

.PHONY: all run
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(shell dirname $(TARGET))
	$(CC) $(CFLAGS) -o $@ $^ $(CLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)

