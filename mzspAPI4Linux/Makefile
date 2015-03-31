#
# Copyright (C) 2011-2012, Marvell International Ltd.
# All Rights Reserved.
#

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

ifeq ($(NOISY),1)
AT=
else
AT=@
endif

MAKEDIRPRINT=-s --no-print-directory

MZSPLIB=libmzsp
CLILIB=libcli
APP=mzspcli

LDFLAGS= -Lcli -Lmzsp
LDLIBS= -lmzsp -lcli

SOURCE = zigbee_cli.c zigbee_db.c mzspAPIdemo.c 
.PHONY: all clean

all: $(APP)

OBJS := $(SOURCE:%.c=%.o)

$(APP): $(OBJS)
	@echo " [out] $(APP)"
	${AT}${CC} -o $(APP) $(OBJS) $(LDFLAGS) $(LDLIBS) -lpthread -lm
	@echo "DONE!"

%.o: %.c
	@echo " [cc] $@"
	${AT}${CC} -o ${@} -c ${<}

clean: 
	rm -rf *.o
	rm -rf $(APP)
	@echo "DONE!"
