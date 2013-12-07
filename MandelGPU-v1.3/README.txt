MandelCPU vs. MandelGPU
=======================

MandelGPU is a small and simple demo written in OpenCL in order to test the
performance of this new  standard. It has been written using the
ATI OpenCL SDK beta4 on Linux but it should work on any platform/implementation.

glut32.dll has been downloaded from Nate Robins's http://www.xmission.com/~nate/glut.html


How to compile
==============

Just edit the Makefile and use an appropriate value for ATISTREAMSDKROOT.


Key bindings
============

's' - save image.ppm
ESC or 'q' or 'Q' - exit
'+' - increase the max. interations by 32
'-' - decrease the max. interations by 32
Arrow keys - move left/right/up/down
PageUp and PageDown - to zoom in/out
' ' - refresh the window
You can use the mouse button 0 and grab to move too
You can use the mouse button 2 and grab to scale too


History
=======

V1.3 - Updated for ATI SDK 2.0

V1.2 - Jens's patch for MacOS, Szaq's patch for NVIDIA OpenCL and Windows, Fixed
peformance estimation, added Windows binaries

V1.1 - Fixed window resize problem, added support for loading different
kernels, added float4 kernel

V1.0 - First release
