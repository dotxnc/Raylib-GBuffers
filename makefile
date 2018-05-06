rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRC = $(call rwildcard, src/, *.c) #$(wildcard src/*.cpp) $(wildcard src/engine/*.cpp)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
CFLAGS = --std=c11 -Wno-incompatible-pointer-types -Wno-int-conversion -Wno-unused-value -Isrc/
NAME = arcade
OUT =

LDFLAGS = -Wl,-allow-multiple-definition -static-libgcc
LIBS = 
comp = ${CC}
ifeq ($(OS),Windows_NT)
	OUT = $(NAME).exe
	# CC += clang
	LDFLAGS += -static
	LIBS = -lraylib -lopengl32 -lgdi32 -llua
else
	OUT = $(NAME)
	# CC += clang
	LIBS = -lraylib -lGL -lXxf86vm -lXext -lX11 -lXrandr -lXinerama -lXcursor -llua5.3 -lm -lpthread -ldl
endif

all: $(OUT)

$(OUT): $(OBJ)
	@ test -d bin || mkdir bin
	$(comp) -o bin/$(OUT) $(CFLAGS) $(OBJ) $(LIBS) $(LDFLAGS)

obj/%.o: src/%.c
	@ test -d $(@D) || mkdir $(@D)
	$(comp) -c $< -o $@ $(CFLAGS)

clean:
	rm -r obj/*
	rm bin/$(OUT)

run:
	cd bin; ./$(OUT)

dbg:
	cd bin; gdb $(OUT)
