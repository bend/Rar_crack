CFLAGS+=-Wall -Werror -ansi -pedantic
LDFLAGS+=-lunrar -L.
EXEC=rarcrack
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJECTS) libunrar.so
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
libunrar.so:
	@echo "libunrar missing"
	@exit 1

.PHONY: clean mrproper

clean:
	@rm $(OBJECTS)

mrproper:
	@rm $(EXEC)