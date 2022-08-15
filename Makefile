LIB_NAME = liblog.so
LIB_OBJS = log.o log-priv.o

TEST_NAME = logtest
TEST_OBJS = test.o

CTRL_NAME = logctrl
CTRL_OBJS = logctrl.o

LDFLAGS += -L$(INSTALLDIR)/lib 
LDFLAGS += -L./ -llog -lrt -lpthread

CFLAGS += -fPIC
CFLAGS += -I$(shell pwd)/include/

all: $(LIB_NAME) $(TEST_NAME) $(CTRL_NAME)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_NAME) : $(LIB_OBJS)
	$(CC) -o $@ $^ -lrt -fPIC -shared

$(TEST_NAME) : $(TEST_OBJS) 
	$(CC) -o $@ $^ $(LDFLAGS)

$(CTRL_NAME) : $(CTRL_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

install:
	rm -rf $(PRIVATE_APPSPATH)/include/liblog
	ln -sf $(shell pwd)/include $(PRIVATE_APPSPATH)/include/liblog
	install -m 0775 $(LIB_NAME) $(INSTALLDIR)/usr/lib/$(LIB_NAME)
	install -m 0775 $(CTRL_NAME) $(INSTALLDIR)/usr/sbin/$(CTRL_NAME)

clean:
	rm -rf $(LIB_NAME) $(TEST_NAME) $(CTRL_NAME)
	rm -rf $(LIB_OBJS) $(TEST_OBJS) $(CTRL_OBJS)
	rm -rf $(PRIVATE_APPSPATH)/include/liblog
	rm -rf $(INSTALLDIR)/usr/lib/$(LIB_NAME)
	rm -rf $(INSTALLDIR)/usr/sbin/$(CTRL_NAME)

.PHONY: all install clean

