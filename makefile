CC = cc
LD = cc
CCFLAGS = -Wall -Wextra -Wpedantic -Werror -std=c17
LDFLAGS = 

SRC = ./src
BIN = ./bin
SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(BIN)/%.o, $(SRCS))
OUT = $(BIN)/emu

.PHONY: all clean test

all: $(OUT)
$(OUT): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(BIN)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -c -o $@ $^

clean:
	rm -fr $(BIN) $(OUT)

test: $(OUT)
	$(OUT)

