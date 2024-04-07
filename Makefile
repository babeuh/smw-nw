TARGET_EXEC:=smw

SRCS:=$(wildcard src/*.c src/snes/*.c)
OBJS:=$(SRCS:%.c=%.o)

PYTHON:=/usr/bin/env python3
#CFLAGS:=$(if $(CFLAGS),$(CFLAGS),-Os -flto -fno-strict-aliasing -Werror )
CFLAGS:=$(if $(CFLAGS),$(CFLAGS),-ggdb3 -fno-strict-aliasing -Werror )
CFLAGS:=${CFLAGS} $(shell sdl2-config --cflags) -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -I.

SDLFLAGS:=$(shell sdl2-config --libs) -lm

.PHONY: all clean clean_obj

all: $(TARGET_EXEC) smw_assets.dat
$(TARGET_EXEC): $(OBJS) $(RES)
	$(CC) $^ -o $@ $(LDFLAGS) $(SDLFLAGS)
	#strip $(TARGET_EXEC)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
smw_assets.dat:
	@echo "Extracting game resources"
	$(PYTHON) assets/restool.py

clean: clean_obj clean_gen
clean_obj:
	@$(RM) $(OBJS) $(TARGET_EXEC)
clean_gen:
	@$(RM) $(RES) smw_assets.dat
	@rm -rf assets/__pycache__
