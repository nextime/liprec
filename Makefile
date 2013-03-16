#/***********************************************************************
#    This file is part of LiPRec, License Plate REcognition.
#
#    Copyright (C) 2012 Franco (nextime) Lanza <nextime@nexlab.it>
#
#    LiPRec is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    LiPRec is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with LiPRec.  If not, see <http://www.gnu.org/licenses/>.
#************************************************************************/

OBJECTS =libliprec.o libliprec.so liprec
CFLAGS= -O3 -march=native  -Wall 
CPPFLAGS= -fpermissive -O3 -march=native -Wall 
CPPFLAGS+=$(shell pkg-config --cflags opencv)
CPPFLAGS=-L. -L/usr/lib -I/usr/include
LDFLAGS=-ltesseract
LDFLAGS+=$(shell pkg-config --cflags --libs opencv)

all: ${OBJECTS} 
	
debug: CFLAGS+=-D__DEBUG -g
debug: CPPFLAGS+=-D__DEBUG
debug: clean all

verbosedebug: CFLAGS+=-D__SHOWIMAGES
verbosedebug: CPPFLAGS+=-D__SHOWIMAGES
verbosedebug: debug

libliprec.o: libliprec.cpp
	$(CXX) libliprec.cpp -fPIC -c -o libliprec.o $(CPPFLAGS)
libliprec.so: 
	$(CXX) -o libliprec.so -Wall -shared libliprec.o $(LDFLAGS)

liprec: liprec.cpp
	$(CXX) liprec.cpp -o liprec -lliprec ${LDFLAGS} $(CPPFLAGS)

lib_install:
	install -m 0644 libliprec.so /usr/lib
	install -m 0644 liprec.h /usr/include
	ldconfig

install: lib_install
	install -m 0755 liprec /usr/bin/

clean:
	rm -f $(OBJECTS)

