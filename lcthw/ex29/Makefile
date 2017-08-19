print-%: ; @echo $*=$($*)
CFLAGS=-Wall -g -Wextra -rdynamic -DNDEBUG $(OPTFLAGS)

TARGET=/build/lib.so

SOURCE=$(wildcard src/**/*.c src/*.c)
SHARE_OBJECT=$(patsubst %.c,%.so,$(SOURCE))
OBJECT=$(patsubst %.c,%.o,$(SOURCE))

all: build $(TARGET)

$(TARGET):
	cc $(CFLAGS) -fPIC -shared $(SOURCE) -o build/lib.so


build:
	@mkdir -p build
	@mkdir -p bin
 
clean:
	rm -rf build $(OBJECT) $(SAHRE_OBJECT)
