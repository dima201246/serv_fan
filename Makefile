SF_VERSION = 1.0

CC = g++
FLAGS = -Wall -c
LDFLAGS	= -Llib -lconf_lang_x32 -Wl,-rpath,lib

vpath %.cpp ./header

EXE		:= serv_fan_debug
EXE_REL	:= serv_fan

MAIN	:= main.cpp
POCO	:= header/port_control.cpp

MAINOBJ	:= $(patsubst %.cpp,%.o,$(MAIN))
POCOOBJ	:= $(patsubst %.cpp,%.o,$(POCO))

all: $(EXE)

realise: $(EXE_REL) clean-all

start:
		./$(EXE)

# $^ - имена всех пререквизитов.  $? - именна тех пререквизитов что были обновлены


$(EXE_REL): $(MAINOBJ) $(POCOOBJ)
		$(CC) -O2 $^ -o $@ $(LDFLAGS)
		chmod u=rwx,g=rx,o=rx ./$(EXE_REL)

$(EXE): $(MAINOBJ) $(POCOOBJ)
		$(CC) -g3 $^ -o $@ $(LDFLAGS)
		chmod u=rwx,g=rx,o=rx ./$(EXE)

$(MAINOBJ):	$(MAIN)
		$(CC) -D_sf_version=\"$(SF_VERSION)\" -D_DEBUG $(FLAGS) $< -o $@

$(POCOOBJ):	$(POCO)
		$(CC) $(FLAGS) $< -o $@

install:
		cp $(EXE_REL) /usr/bin/$(EXE_REL)
		cp lib/libconf_lang_x32.so /usr/lib/libconf_lang_x32.so

clean:
		rm -f *.o

clean-all:
		rm -f *.o
		rm $(EXE)
