JuliaGPU
========

JuliaGPU is a small and simple demo written in OpenCL in order to test the
performance of this new standard. It is based on Keenan Crane's qjulia
available at http://www.cs.caltech.edu/~keenan/project_qjulia.html.
The idea of Ambient Occlusion comes from Tom Beddard's
http://www.subblue.com/blog/2009/9/20/quaternion_julia.
JuliaGPU has been written using the ATI OpenCL SDK beta4 on Linux but it
should work on any platform/implementation (i.e. NVIDIA).

glut32.dll has been downloaded from Nate Robins's http://www.xmission.com/~nate/glut.html


How to compile
==============

Just edit the Makefile and use an appropriate value for ATISTREAMSDKROOT.


Key bindings
============

Check the on screen help.


History
=======

V1.2 - Updated to ATI SDK 2.0, improved normal calculation

V1.1 - Added checkered background, added fast mode rendering

V1.0 - First release
