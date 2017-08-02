Names:
	Matt Quinn
	Courtney Pasco

Platform:
	Mac

Source Code:
	Matt submitted the source code from his Mac. Courtney is hosting the web page from her instructional account.

To Run with Toon Shading:

	Append the tag '-toon' to command line arguments:

	./as1 -kd .5 .5 .5 -ks .5 .5 .5 -dl -1 -1 -1 0 .5 1 -sp 2 -toon

	./as1 -kd .5 .5 .5 -ks .5 .5 .5 -dl -1 -1 -1 0 .5 1 -spu 2 -spv 30 -toon

To Run with A-S Materials:

	Append the tag '-asm' to command line arguments

	./as1 -kd .5 .5 .5 -ks .5 .5 .5 -dl -1 -1 -1 0 .5 1 -sp 2 -asm

	./as1 -kd .5 .5 .5 -ks .5 .5 .5 -dl -1 -1 -1 0 .5 1 -spu 2 -spv 30 -asm

-ka r g b

This is the ambient color coefficients of the sphere material. The parameters r g b are numbers
between 0 and 1 inclusive.

-kd r g b

This is the diffuse color coefficients of the sphere material. The parameters r g b are numbers
between 0 and 1 inclusive.

-ks r g b

This is the specular color coefficients of the sphere material. The parameters r g b are numbers
between 0 and 1 inclusive.

-spu pu

This is the power coefficient on the specular term in the u direction for an anisotropic material.
It is a number between 0 and max_float.

-spv pv

This is the power coefficient on the specular term in the v direction for an anisotropic material.
It is a number between 0 and max_float.

-sp p

This is the power coefficient on the specular term for an isotropic material. It is a number between
0 and max_float. (i.e. the same as setting pu and pv the the same value.)

-pl x y z r g b

This adds a point light to the scene. The x y z values are the location of the light. The r g b
values are it's color. Note that the x y z values are relative to the sphere. That is, the center of
the sphere is at the origin and the radius of the sphere defines one unit of length. The Y direction
is UP, the X direction is to the right on the screen, and the Z direction is "in your face." The
r g b value are between 0 and max_float, NOT between 0 and 1 (that is, the r g b values encode
the brightness of the light). 

-dl x y z r g b

This adds a directional light to the scene. The x y z values are the direction that the light
points in. The r g b values are it's color. See -pl for coordinate system notes. 
