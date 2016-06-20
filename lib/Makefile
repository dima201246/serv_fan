CL_VERSION = 2.4

CC = g++
FLAGS = -O2 -Wall -c -fPIC
ARCH = 32

vpath %.cpp .

OUT_LIB := libconf_lang_x$(ARCH).so

LAN := lang_out.cpp
CONF := configurator.cpp

LANGOBJ := $(patsubst %.cpp,%.o,$(LAN))
CONFOBJ := $(patsubst %.cpp,%.o,$(CONF))

all: $(OUT_LIB)

$(OUT_LIB): $(LANGOBJ) $(CONFOBJ)
		$(CC) -shared -o $@ $^ -m$(ARCH)

$(LANGOBJ):	$(LAN)
		$(CC) $(FLAGS) $< -m$(ARCH)

$(CONFOBJ): $(CONF)
		$(CC) -D_CL_VERSION=\"$(CL_VERSION)\" $(FLAGS) $< -m$(ARCH)

for_hos:
		$(CC) -D_CL_VERSION=\"$(CL_VERSION)\" -D_HOS_LIB -fPIC -c $(CONF)
		$(CC) -shared -o  libconfigurator_x$(ARCH).so configurator.o

install:
		cp $(OUT_LIB) /usr/lib/$(OUT_LIB)

clean:
		rm -f *.o

clean-all:
		rm -f *.o
		rm $(OUT_LIB)