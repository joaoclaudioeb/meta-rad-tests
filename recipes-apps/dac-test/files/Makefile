APP = dac-test

SRC = \
	src/main.c \
	src/device-drivers/dac81408.c \
	src/drivers/gpio_linux.c \
	src/drivers/spi_linux.c

OBJ = $(SRC:.c=.o)

CFLAGS += -Iinclude
LDLIBS += -lgpiod

all: $(APP)

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

clean:
	rm -f $(APP) $(OBJ)
