PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJS = main.o NDIReceiver.o VideoMonitor.o Log.o

#ifeq ($(BUILD_MODE),debug)
#	CFLAGS += -g
#else ifeq ($(BUILD_MODE),run)
#	CFLAGS += -O2
#else
#	$(error Build mode $(BUILD_MODE) not supported by this Makefile)
#endif

CFLAGS +=  -L/opt/vc/lib -lbcm_host -lndi -lbrcmEGL -lbrcmGLESv2 -Iinclude -I/opt/vc/include

all:	sdlplayer

sdlplayer:	$(OBJS)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

%.o:	$(PROJECT_ROOT)src/%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)src/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr sdlplayer $(OBJS)
