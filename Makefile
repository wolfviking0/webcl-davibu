EMCC:=../../webcl-translator/emscripten

EMSCRIPTEN = $(EMCC)
CXX = $(EMSCRIPTEN)/emcc
AR = $(EMSCRIPTEN)/emar
EMCONFIGURE = $(EMSCRIPTEN)/emconfigure
EMMAKE = $(EMSCRIPTEN)/emmake

CHDIR_SHELL := $(SHELL)
define chdir
   $(eval _D=$(firstword $(1) $(@D)))
   $(info $(MAKE): cd $(_D)) $(eval SHELL = cd $(_D); $(CHDIR_SHELL))
endef

DEB=0
VAL=0

ifeq ($(VAL),1)
PREFIX = val_
$(info ************  Mode VALIDATOR : Enabled ************)
else
PREFIX = 
$(info ************  Mode VALIDATOR : Disabled ************)
endif

DEBUG = -O0 -s OPENCL_VALIDATOR=$(VAL) -s OPENCL_PRINT_TRACE=1 -s DISABLE_EXCEPTION_CATCHING=0 -s WARN_ON_UNDEFINED_SYMBOLS=1 -s OPENCL_PROFILE=1 -s OPENCL_DEBUG=1 -s OPENCL_GRAB_TRACE=1 -s OPENCL_CHECK_VALID_OBJECT=1

NO_DEBUG = -03 -s OPENCL_VALIDATOR=$(VAL) -s WARN_ON_UNDEFINED_SYMBOLS=0 -s OPENCL_PROFILE=1 -s OPENCL_DEBUG=0 -s OPENCL_GRAB_TRACE=0 -s OPENCL_PRINT_TRACE=0 -s OPENCL_CHECK_VALID_OBJECT=0

ifeq ($(DEB),1)
MODE=$(DEBUG)
$(info ************  Mode DEBUG : Enabled ************)
else
MODE=$(NO_DEBUG)
$(info ************  Mode DEBUG : Disabled ************)
endif

$(info )
$(info )

#----------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------#
# BUILD
#----------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------#		

all: mandelgpu_sample juliagpu_sample mandelbulbgpu_sample smallptgpuv1_sample smallptgpuv2_sample

mandelgpu_sample:
	$(call chdir,MandelGPU-v1.3/)
	JAVA_HEAP_SIZE=8096m EMCC_DEBUG=$(DEB) $(CXX) \
		mandelGPU.c \
		displayfunc.c \
	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 \
	--preload-file rendering_kernel_float4.cl \
	--preload-file rendering_kernel.cl \
	-o ../build/$(PREFIX)dav_mandelgpu.js

juliagpu_sample:
	$(call chdir,mandelbulbGPU-v1.0/)
	JAVA_HEAP_SIZE=8096m EMCC_DEBUG=$(DEB) $(CXX) \
		mandelbulbGPU.c \
		displayfunc.c \
	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 \
	--preload-file preprocessed_rendering_kernel.cl \
	-o ../build/$(PREFIX)dav_juliagpu.js

mandelbulbgpu_sample:
	$(call chdir,JuliaGPU-v1.2/)
	JAVA_HEAP_SIZE=8096m EMCC_DEBUG=$(DEB) $(CXX) \
		juliaGPU.c \
		displayfunc.c \
	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 \
	--preload-file preprocessed_rendering_kernel.cl \
	-o ../build/$(PREFIX)dav_mandelbulbgpu.js

smallptgpuv1_sample:
	$(call chdir,smallptGPU-v1.6/)
	JAVA_HEAP_SIZE=8096m EMCC_DEBUG=$(DEB) $(CXX) \
		smallptGPU.c \
		displayfunc.c \
	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 \
	--preload-file preprocessed_rendering_kernel.cl \
	--preload-file preprocessed_rendering_kernel_dl.cl \
	--preload-file scene_build_complex.pl \
	--preload-file scenes/caustic.scn \
	--preload-file scenes/caustic3.scn \
	--preload-file scenes/complex.scn \
	--preload-file scenes/cornell_large.scn \
	--preload-file scenes/cornell.scn \
	--preload-file scenes/simple.scn \
	-o ../build/$(PREFIX)dav_smallptgpuv1.js

smallptgpuv2_sample:
	$(call chdir,SmallptGPU-v2.0/)
	JAVA_HEAP_SIZE=8096m EMCC_DEBUG=$(DEB) $(CXX) \
		smallptGPU.cpp \
		renderconfig.cpp \
		displayfunc.cpp \
		renderdevice.cpp \
	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 -s TOTAL_MEMORY=1024*1024*50 \
	--preload-file rendering_kernel.cl \
	--preload-file scene_build_complex.pl \
	--preload-file scenes/caustic.scn \
	--preload-file scenes/caustic3.scn \
	--preload-file scenes/complex.scn \
	--preload-file scenes/cornell_large.scn \
	--preload-file scenes/cornell.scn \
	--preload-file scenes/simple.scn \
	-o ../build/$(PREFIX)dav_smallptgpuv2.js

smallluxgpu_sample:
	$(call chdir,smallluxGPU-v1.3/)
	JAVA_HEAP_SIZE=8096m EMCC_DEBUG=$(DEB) $(CXX) \
		displayfunc.cpp \
		intersectiondevice.cpp \
		mesh.cpp \
		path.cpp \
		qbvhaccel.cpp \
		renderthread.cpp \
		scene.cpp \
		smallluxGPU.cpp \
		core/bbox.cpp \
		core/matrix4x4.cpp \
		core/transform.cpp \
		plymesh/rply.cpp \
		-I$(EMSCRIPTEN)/system/include -I./core -I./plymesh \
	$(MODE) -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 -s TOTAL_MEMORY=1024*1024*50 \
	--preload-file qbvh_kernel.cl \
	--preload-file render.cfg \	
	--preload-file scenes/luxball.ply \
	--preload-file scenes/simple-lights.ply \
	--preload-file scenes/simple.ply \
	--preload-file scenes/sponza-lights.ply \
	--preload-file scenes/sponza.ply \
	--preload-file scenes/bigmonkey-lights.ply \
	--preload-file scenes/bigmonkey.ply \
	--preload-file scenes/bigmonkey.scn \
	--preload-file scenes/bulletphysics-lights.ply \
	--preload-file scenes/bulletphysics.ply \
	--preload-file scenes/bulletphysics.scn \
	--preload-file scenes/interp-lights.ply \
	--preload-file scenes/interp.ply \
	--preload-file scenes/interp.scn \
	--preload-file scenes/kitchen_night.scn \
	--preload-file scenes/kitchen-lights_night.ply \
	--preload-file scenes/kitchen-lights.ply \
	--preload-file scenes/kitchen.ply \
	--preload-file scenes/kitchen.scn \
	--preload-file scenes/loft-lights.ply \
	--preload-file scenes/loft.ply \
	--preload-file scenes/loft.scn \
	--preload-file scenes/luxball-lights.ply \
	--preload-file scenes/luxball.scn \
	--preload-file scenes/simple.scn \
	--preload-file scenes/sponza.scn \
	-o ../build/$(PREFIX)dav_smallptgpuv2.js

clean:
	$(call chdir,build/)
	mkdir tmp
	cp memoryprofiler.js tmp/
	cp settings.js tmp/
	rm -f *.data
	rm -f *.js
	rm -f *.map
	cp tmp/memoryprofiler.js ./
	cp tmp/settings.js ./
	rm -rf tmp/
	$(CXX) --clear-cache

