#
#  Makefile
#  Licence : https://github.com/wolfviking0/webcl-translator/blob/master/LICENSE
#
#  Created by Anthony Liot.
#  Copyright (c) 2013 Anthony Liot. All rights reserved.
#

# Default parameter
DEB = 0
VAL = 0
NAT = 0
ORIG= 0
FAST= 1

# Chdir function
CHDIR_SHELL := $(SHELL)
define chdir
   $(eval _D=$(firstword $(1) $(@D)))
   $(info $(MAKE): cd $(_D)) $(eval SHELL = cd $(_D); $(CHDIR_SHELL))
endef

# Current Folder
CURRENT_ROOT:=$(PWD)

# Emscripten Folder
EMSCRIPTEN_ROOT:=$(CURRENT_ROOT)/../webcl-translator/emscripten

# Native build
ifeq ($(NAT),1)
$(info ************ NATIVE : NO DEPENDENCIES  ************)

CXX = clang++
CC  = clang

BUILD_FOLDER = $(CURRENT_ROOT)/bin/
EXTENSION = .out

ifeq ($(DEB),1)
$(info ************ NATIVE : DEBUG = 1        ************)

CFLAGS = -O0 -framework OpenCL -framework OpenGL -framework GLUT -framework CoreFoundation -framework AppKit -framework IOKit -framework CoreVideo -framework CoreGraphics

else
$(info ************ NATIVE : DEBUG = 0        ************)

CFLAGS = -O2 -framework OpenCL -framework OpenGL -framework GLUT -framework CoreFoundation -framework AppKit -framework IOKit -framework CoreVideo -framework CoreGraphics

endif

# Emscripten build
else
ifeq ($(ORIG),1)
$(info ************ EMSCRIPTEN : SUBMODULE     = 0 ************)

EMSCRIPTEN_ROOT:=$(CURRENT_ROOT)/../emscripten
else
$(info ************ EMSCRIPTEN : SUBMODULE     = 1 ************)
endif

CXX = $(EMSCRIPTEN_ROOT)/em++
CC  = $(EMSCRIPTEN_ROOT)/emcc

BUILD_FOLDER = $(CURRENT_ROOT)/build/
EXTENSION = .js
GLOBAL =

ifeq ($(DEB),1)
$(info ************ EMSCRIPTEN : DEBUG         = 1 ************)

GLOBAL += EMCC_DEBUG=1

CFLAGS = -s OPT_LEVEL=1 -s DEBUG_LEVEL=1 -s CL_PRINT_TRACE=1 -s WARN_ON_UNDEFINED_SYMBOLS=1 -s CL_DEBUG=1 -s CL_GRAB_TRACE=1 -s CL_CHECK_VALID_OBJECT=1
else
$(info ************ EMSCRIPTEN : DEBUG         = 0 ************)

CFLAGS = -s OPT_LEVEL=3 -s DEBUG_LEVEL=0 -s CL_PRINT_TRACE=0 -s DISABLE_EXCEPTION_CATCHING=0 -s WARN_ON_UNDEFINED_SYMBOLS=1 -s CL_DEBUG=0 -s CL_GRAB_TRACE=0 -s CL_CHECK_VALID_OBJECT=0
endif

ifeq ($(VAL),1)
$(info ************ EMSCRIPTEN : VALIDATOR     = 1 ************)

PREFIX = val_

CFLAGS += -s CL_VALIDATOR=1
else
$(info ************ EMSCRIPTEN : VALIDATOR     = 0 ************)
endif

ifeq ($(FAST),1)
$(info ************ EMSCRIPTEN : FAST_COMPILER = 1 ************)

GLOBAL += EMCC_FAST_COMPILER=1
else
$(info ************ EMSCRIPTEN : FAST_COMPILER = 0 ************)
endif

endif

SOURCES_mandelgpu		=	mandelGPU.c displayfunc.c
SOURCES_juliagpu		=	juliaGPU.c displayfunc.c
SOURCES_mandelbulbgpu	=	mandelbulbGPU.c displayfunc.c
SOURCES_smallptgpu1		=	smallptGPU.c displayfunc.c
SOURCES_smallptgpu2		=	smallptGPU.cpp renderconfig.cpp displayfunc.cpp renderdevice.cpp

INCLUDES_mandelgpu		=	-I./
INCLUDES_juliagpu		=	-I./
INCLUDES_mandelbulbgpu	= 	-I./
INCLUDES_smallptgpu1	=	-I./
INCLUDES_smallptgpu2	=	-I./ -I$(EMSCRIPTEN_ROOT)/system/include/

ifeq ($(NAT),0)

KERNEL_mandelgpu		= 	--preload-file rendering_kernel_float4.cl
KERNEL_juliagpu			= 	--preload-file preprocessed_rendering_kernel_julia.cl
KERNEL_mandelbulbgpu	= 	--preload-file preprocessed_rendering_kernel_mandelbulb.cl
KERNEL_smallptgpu1		= 	--preload-file preprocessed_rendering_kernel_dl.cl --preload-file scene_build_complex.pl --preload-file scenes/caustic.scn --preload-file scenes/caustic3.scn --preload-file scenes/complex.scn --preload-file scenes/cornell_large.scn --preload-file scenes/cornell.scn --preload-file scenes/simple.scn 
KERNEL_smallptgpu2		= 	--preload-file rendering_kernel.cl --preload-file scene_build_complex.pl --preload-file scenes/caustic.scn --preload-file scenes/caustic3.scn --preload-file scenes/complex.scn --preload-file scenes/cornell_large.scn --preload-file scenes/cornell.scn --preload-file scenes/simple.scn 

CFLAGS_mandelgpu		=	-s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1
CFLAGS_juliagpu			=	-s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1
CFLAGS_mandelbulbgpu	=	-s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1
CFLAGS_smallptgpu1		=	-s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1
CFLAGS_smallptgpu2		=	-s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1

VALPARAM_mandelgpu		=	-s CL_VAL_PARAM='[""]'
VALPARAM_juliagpu		=	-s CL_VAL_PARAM='[""]'
VALPARAM_mandelbulbgpu	=	-s CL_VAL_PARAM='[""]'
VALPARAM_smallptgpu1	=	-s CL_VAL_PARAM='[""]'
VALPARAM_smallptgpu2	=	-s CL_VAL_PARAM='[""]'

else

COPY_mandelgpu			= 	cp rendering_kernel_float4.cl $(BUILD_FOLDER) &&
COPY_juliagpu			= 	cp preprocessed_rendering_kernel_julia.cl $(BUILD_FOLDER) &&
COPY_mandelbulbgpu		= 	cp preprocessed_rendering_kernel_mandelbulb.cl $(BUILD_FOLDER) &&
COPY_smallptgpu1		= 	mkdir -p $(BUILD_FOLDER)scenes/ && cp -rf scenes/ $(BUILD_FOLDER)scenes/ && cp preprocessed_rendering_kernel_dl.cl $(BUILD_FOLDER) &&
COPY_smallptgpu2		= 	mkdir -p $(BUILD_FOLDER)scenes/ && cp -rf scenes/ $(BUILD_FOLDER)scenes/ && cp rendering_kernel.cl $(BUILD_FOLDER) &&

endif

.PHONY:    
	all clean

all: \
	all_1 all_2 all_3

all_1: \
	mandelgpu_sample juliagpu_sample

all_2: \
	mandelbulbgpu_sample smallptgpu1_sample

all_3: \
	smallptgpu2_sample

# Create build folder is necessary))
mkdir:
	mkdir -p $(BUILD_FOLDER);

mandelgpu_sample: mkdir
	$(call chdir,MandelGPU-v1.3/)
	$(COPY_mandelgpu) 		$(GLOBAL) $(CC)	 $(CFLAGS) $(CFLAGS_mandelgpu)		$(INCLUDES_mandelgpu)		$(SOURCES_mandelgpu)		$(VALPARAM_mandelgpu) 		$(KERNEL_mandelgpu) 		-o $(BUILD_FOLDER)$(PREFIX)mandelgpu$(EXTENSION) 

juliagpu_sample: mkdir
	$(call chdir,JuliaGPU-v1.2/)
	$(COPY_juliagpu) 		$(GLOBAL) $(CC)  $(CFLAGS) $(CFLAGS_juliagpu)		$(INCLUDES_juliagpu)		$(SOURCES_juliagpu)			$(VALPARAM_juliagpu) 		$(KERNEL_juliagpu) 			-o $(BUILD_FOLDER)$(PREFIX)juliagpu$(EXTENSION) 

mandelbulbgpu_sample: mkdir
	$(call chdir,mandelbulbGPU-v1.0/)
	$(COPY_mandelbulbgpu) 	$(GLOBAL) $(CC)  $(CFLAGS) $(CFLAGS_mandelbulbgpu)	$(INCLUDES_mandelbulbgpu)	$(SOURCES_mandelbulbgpu)	$(VALPARAM_mandelbulbgpu) 	$(KERNEL_mandelbulbgpu) 	-o $(BUILD_FOLDER)$(PREFIX)mandelbulbgpu$(EXTENSION) 

smallptgpu1_sample: mkdir
	$(call chdir,smallptGPU-v1.6/)
	$(COPY_smallptgpu1) 	$(GLOBAL) $(CC)  $(CFLAGS) $(CFLAGS_smallptgpu1)	$(INCLUDES_smallptgpu1)		$(SOURCES_smallptgpu1)		$(VALPARAM_smallptgpu1) 	$(KERNEL_smallptgpu1) 		-o $(BUILD_FOLDER)$(PREFIX)smallptgpu1$(EXTENSION) 

smallptgpu2_sample: mkdir
	$(call chdir,SmallptGPU-v2.0/)
	$(COPY_smallptgpu2) 	$(GLOBAL) $(CXX) $(CFLAGS) $(CFLAGS_smallptgpu2)	$(INCLUDES_smallptgpu2)		$(SOURCES_smallptgpu2)		$(VALPARAM_smallptgpu2) 	$(KERNEL_smallptgpu2) 		-o $(BUILD_FOLDER)$(PREFIX)smallptgpu2$(EXTENSION) 

clean:
	rm -rf bin/
	mkdir -p bin/
	mkdir -p tmp/
	cp build/memoryprofiler.js tmp/ && cp build/settings.js tmp/ && cp build/index.html tmp/
	rm -rf build/
	mkdir -p build/
	cp tmp/memoryprofiler.js build/ && cp tmp/settings.js build/ && cp tmp/index.html build/
	rm -rf tmp/
	$(EMSCRIPTEN_ROOT)/emcc --clear-cache



# mandelgpu_sample:
# 	$(call chdir,MandelGPU-v1.3/)
# 	JAVA_HEAP_SIZE=8096m $(EMCCDEBUG)=1 $(CC) \
# 		mandelGPU.c \
# 		displayfunc.c \
# 	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 \
# 	--preload-file rendering_kernel_float4.cl \
# 	--preload-file rendering_kernel.cl \
# 	-o ../build/$(PREFIX)dav_mandelgpu.js

# mandelbulbgpu_sample:
# 	$(call chdir,mandelbulbGPU-v1.0/)
# 	JAVA_HEAP_SIZE=8096m $(EMCCDEBUG)=1 $(CC) \
# 		mandelbulbGPU.c \
# 		displayfunc.c \
# 	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 \
# 	--preload-file preprocessed_rendering_kernel.cl \
# 	-o ../build/$(PREFIX)dav_mandelbulbgpu.js

# juliagpu_sample:
# 	$(call chdir,JuliaGPU-v1.2/)
# 	JAVA_HEAP_SIZE=8096m $(EMCCDEBUG)=1 $(CC) \
# 		juliaGPU.c \
# 		displayfunc.c \
# 	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 \
# 	--preload-file preprocessed_rendering_kernel.cl \
# 	-o ../build/$(PREFIX)dav_juliagpu.js

# smallptgpuv1_sample:
# 	$(call chdir,smallptGPU-v1.6/)
# 	JAVA_HEAP_SIZE=8096m $(EMCCDEBUG)=1 $(CC) \
# 		smallptGPU.c \
# 		displayfunc.c \
# 	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 \
# 	--preload-file preprocessed_rendering_kernel.cl \
# 	--preload-file preprocessed_rendering_kernel_dl.cl \
# 	--preload-file scene_build_complex.pl \
# 	--preload-file scenes/caustic.scn \
# 	--preload-file scenes/caustic3.scn \
# 	--preload-file scenes/complex.scn \
# 	--preload-file scenes/cornell_large.scn \
# 	--preload-file scenes/cornell.scn \
# 	--preload-file scenes/simple.scn \
# 	-o ../build/$(PREFIX)dav_smallptgpuv1.js

# smallptgpuv2_sample:
# 	$(call chdir,SmallptGPU-v2.0/)
# 	JAVA_HEAP_SIZE=8096m $(EMCCDEBUG)=1 $(CXX) \
# 		smallptGPU.cpp \
# 		renderconfig.cpp \
# 		displayfunc.cpp \
# 		renderdevice.cpp \
# 	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 -s TOTAL_MEMORY=1024*1024*50 \
# 	--preload-file rendering_kernel.cl \
# 	--preload-file scene_build_complex.pl \
# 	--preload-file scenes/caustic.scn \
# 	--preload-file scenes/caustic3.scn \
# 	--preload-file scenes/complex.scn \
# 	--preload-file scenes/cornell_large.scn \
# 	--preload-file scenes/cornell.scn \
# 	--preload-file scenes/simple.scn \
# 	-o ../build/$(PREFIX)dav_smallptgpuv2.js

# smallluxgpu_sample:
# 	$(call chdir,smallluxGPU-v1.3/)
# 	JAVA_HEAP_SIZE=8096m $(EMCCDEBUG)=1 $(CXX) \
# 		displayfunc.cpp \
# 		intersectiondevice.cpp \
# 		mesh.cpp \
# 		path.cpp \
# 		qbvhaccel.cpp \
# 		renderthread.cpp \
# 		scene.cpp \
# 		smallluxGPU.cpp \
# 		core/bbox.cpp \
# 		core/matrix4x4.cpp \
# 		core/transform.cpp \
# 		plymesh/rply.cpp \
# 		-I$(EMSCRIPTEN)/system/include -I./core -I./plymesh \
# 	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 -s TOTAL_MEMORY=1024*1024*50 \
# 	--preload-file qbvh_kernel.cl \
# 	--preload-file render.cfg \	
# 	--preload-file scenes/luxball.ply \
# 	--preload-file scenes/simple-lights.ply \
# 	--preload-file scenes/simple.ply \
# 	--preload-file scenes/sponza-lights.ply \
# 	--preload-file scenes/sponza.ply \
# 	--preload-file scenes/bigmonkey-lights.ply \
# 	--preload-file scenes/bigmonkey.ply \
# 	--preload-file scenes/bigmonkey.scn \
# 	--preload-file scenes/bulletphysics-lights.ply \
# 	--preload-file scenes/bulletphysics.ply \
# 	--preload-file scenes/bulletphysics.scn \
# 	--preload-file scenes/interp-lights.ply \
# 	--preload-file scenes/interp.ply \
# 	--preload-file scenes/interp.scn \
# 	--preload-file scenes/kitchen_night.scn \
# 	--preload-file scenes/kitchen-lights_night.ply \
# 	--preload-file scenes/kitchen-lights.ply \
# 	--preload-file scenes/kitchen.ply \
# 	--preload-file scenes/kitchen.scn \
# 	--preload-file scenes/loft-lights.ply \
# 	--preload-file scenes/loft.ply \
# 	--preload-file scenes/loft.scn \
# 	--preload-file scenes/luxball-lights.ply \
# 	--preload-file scenes/luxball.scn \
# 	--preload-file scenes/simple.scn \
# 	--preload-file scenes/sponza.scn \
# 	-o ../build/$(PREFIX)dav_smallptgpuv2.js

# clean:
# 	$(call chdir,build/)
# 	rm -rf tmp/	
# 	mkdir tmp
# 	cp memoryprofiler.js tmp/
# 	cp settings.js tmp/
# 	rm -f *.data
# 	rm -f *.js
# 	rm -f *.map
# 	cp tmp/memoryprofiler.js ./
# 	cp tmp/settings.js ./
# 	rm -rf tmp/
# 	$(CXX) --clear-cache

