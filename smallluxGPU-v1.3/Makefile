#
# SmallLuxGPU Makefile
#

# ATI SDK
OCL_SDKROOT_INCLUDE=/home/david/src/ati-stream-sdk-v2.0-lnx64/include
OCL_SDKROOT_LIB=/home/david/src/ati-stream-sdk-v2.0-lnx64/lib/x86_64
# NVIDIA SDK
#OCL_SDKROOT_INCLUDE=/home/david/src/NVIDIA_GPU_Computing_SDK/sdk/OpenCL/common/inc
#OCL_SDKROOT_LIB=.

CXX=g++
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# ATTENTION: -O3 doesn't work with QBVH
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
CPPFLAGS=-ftree-vectorize -msse -msse2 -msse3 -mssse3 -fvariable-expansion-in-unroller \
	-Wall -I$(OCL_SDKROOT_INCLUDE) -Icore
LDFLAGS=-L$(OCL_SDKROOT_LIB) -lOpenCL -lglut /lib/libboost_thread-gcc43-mt-1_39.a -lpthread

# Jens's patch for MacOS, comment the 2 lines above and un-comment the lines below
#CCFLAGS=-O2 -ftree-vectorize -msse -msse2 -msse3 -mssse3 -undefined dynamic_lookup -fvariable-expansion-in-unroller \
#	-cl-fast-relaxed-math -cl-mad-enable -Wall -framework OpenCL -framework OpenGl -framework Glut

OBJECTS=qbvhaccel.o displayfunc.o mesh.o path.o scene.o \
	smallluxGPU.o renderthread.o intersectiondevice.o \
	core/bbox.o core/matrix4x4.o core/transform.o plymesh/rply.o

.PHONY: clean

default: all

all: smallluxGPU

smallluxGPU: $(OBJECTS)
	$(CXX) -O3 $(CPPFLAGS) -o smallluxGPU $(OBJECTS) $(LDFLAGS)

#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# ATTENTION: -O3 doesn't work with QBVH
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
qbvhaccel.o: qbvhaccel.cpp
	$(CXX) -c -O2 $(CPPFLAGS) -o qbvhaccel.o qbvhaccel.cpp

%.o : %.cpp
	$(CXX) -c -O3 $(CPPFLAGS) $< -o $@

$(OBJECTS): Makefile plymesh/rply.h core/smalllux.h core/bbox.h core/matrix4x4.h core/normal.h \
	core/point.h core/randomgen.h core/ray.h core/spectrum.h core/transform.h core/vector.h core/vector_normal.h \
	sampler.h qbvhaccel.h camera.h displayfunc.h film.h light.h mesh.h path.h raybuffer.h renderconfig.h scene.h triangle.h \
	samplebuffer.h renderthread.h intersectiondevice.h

clean:
	rm -rf smallluxGPU image.ppm smallluxGPU-v1.3 smallluxgpu-v1.3.tgz $(OBJECTS)

tgz: all
	mkdir smallluxGPU-v1.3
	cp -r smallluxGPU SmallLuxGPU.exe glut32.dll \
		Makefile \
		*.cl *.cpp *.h plymesh core \
		*.bat \
		render.cfg \
		LICENSE.txt README.txt smallluxGPU-v1.3
	rm -f smallluxGPU-v1.3/plymesh/*.o smallluxGPU-v1.3/core/*.o
	mkdir smallluxGPU-v1.3/scenes
	cp scenes/*.scn scenes/*.ply smallluxGPU-v1.3/scenes
	tar zcvf smallluxgpu-v1.3.tgz smallluxGPU-v1.3
	rm -rf smallluxGPU-v1.3
