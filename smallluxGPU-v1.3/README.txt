SmallLuxGPU
===========

SmallLuxGPU is a test on how to introduce GPGPU rendering in Luxrender. The
test has been written using the ATI OpenCL SDK 2.0 on Linux but it should work
on any platform/implementation (i.e. NVIDIA).

The idea is to use the GPGPU only for ray intersections in order to minimize
the amount of the brand new code to write and to not loose any of the
functionality already available in Luxrender In order to test this idea, I wrote
a very simplified path tracer and ported Luxrender's BVH accelerator to OpenCL.

The path integrator has the particularity to work on a huge set (i.e. 300,000+)
of paths at the same time in order to generate a large amount of rays to trace
and to keep the GPGPU feed.

SmallLuxGPU includes some class from Luxrender and it is released under
GPL license. Binaries include glut32.dll Nate Robins's glut
(http://www.xmission.com/~nate/glut.html). Loft (http://www.luxrender.net/forum/gallery2.php?g2_itemId=519)
scene has been modeled by Pinko and Sponza (http://hdri.cgtechniques.com/~sponza/) from Marko Dabrovic.
BulletPhysics scene has been designed by Chiaroscuro (http://www.bulletphysics.org/ and http://www.youtube.com/watch?v=33rU1axSKhQ).

A video of SmallLuxGPU V1.0 is available here: http://vimeo.com/8487987
A video of SmallLuxGPU V1.1 is available here: http://vimeo.com/8799796


How to compile
==============

Just edit the Makefile and use an appropriate value for ATISTREAMSDKROOT.


Key bindings
============

Check the on screen help.


History
=======

Check http://www.luxrender.net/wiki/index.php?title=Luxrender_and_OpenCL#SmallLuxGPU for more details.
