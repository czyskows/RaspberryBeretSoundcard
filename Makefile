obj-m := raspberry-beret-soundcard.o

HEADERS := /lib/modules/$(shell uname -r)/build

default:
	${MAKE} -C ${HEADERS} SUBDIRS=${PWD} modules

clean:
	${MAKE} -C ${HEADERS} SUBDIRS=${PWD} clean
