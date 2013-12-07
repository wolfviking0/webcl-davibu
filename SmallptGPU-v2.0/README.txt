SmallptGPU2
===========

SmallptGPU2 is a small and simple demo written in OpenCL in order to test the
performance of this new standard. It is based on Kevin Beason's Smallpt available
at http://www.kevinbeason.com/smallpt/
SmallptGPU2 has been written using the ATI OpenCL SDK 2.0 on Linux but it
should work on any platform/implementation.

glut32.dll has been downloaded from Nate Robins's http://www.xmission.com/~nate/glut.html


How to compile
==============

Just edit the Makefile and use an appropriate value for ATISTREAMSDKROOT.


Key bindings
============

'p' - save image.ppm
ESC - exit
Arrow keys - rotate camera left/right/up/down
'a' and 'd' - move camera left and right
'w' and 's' - move camera forward and backward
'r' and 'f' - move camera up and down
PageUp and PageDown - move camera target up and down
' ' - refresh the window
'+' and '-' - to select next/previous object
'2', '3', '4', '5', '6', '8', '9' - to move selected object

History
=======

V2.0 - First release of SmallptGPU2
