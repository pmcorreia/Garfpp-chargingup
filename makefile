OBJDIR = $(GARFIELD_HOME)/Object
SRCDIR = $(GARFIELD_HOME)/Source
INCDIR = $(GARFIELD_HOME)/Include
HEEDDIR = $(GARFIELD_HOME)/Heed
LIBDIR = $(GARFIELD_HOME)/Library

# Compiler flags
CFLAGS = -Wall -Wextra -Wno-long-long \
	`root-config --cflags` \
	-O3 -fno-common -c \
	-I$(INCDIR) -Iinclude \
	-std=c++0x \
	-fopenmp
# Debug flags
# CFLAGS += -g

LDFLAGS = `root-config --glibs` -lGeom -lgfortran -lm
LDFLAGS += -L$(LIBDIR) -lGarfield -std=c++0x -O3 -fopenmp

#LDFLAGS += -g
all: thgem_chargingup_openMP thgem_chargingup_norim

thgem_chargingup_openMP: src/thgem_chargingup_openMP.cpp
	$(CXX) $(CFLAGS) src/thgem_chargingup_openMP.cpp
	$(CXX) -o bin/thgem_chargingup_openMP thgem_chargingup_openMP.o $(LDFLAGS)
	rm thgem_chargingup_openMP.o
 
thgem_chargingup_norim: src/thgem_chargingup_norim.cpp
	$(CXX) $(CFLAGS) src/thgem_chargingup_norim.cpp
	$(CXX) -o bin/thgem_chargingup_norim thgem_chargingup_norim.o $(LDFLAGS)
	rm thgem_chargingup_norim.o 

clean: 
	rm -rf bin/thgem_chargingup_*
    
