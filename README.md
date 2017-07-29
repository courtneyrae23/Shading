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

To Run with Mulitple Spheres:
 
	Default (if nothing specified) is -s 0 0 0 1. If one sphere specified, default is overridden with the values specified.  Additional spheres are specified with additional -s x y z r tags. 

	./as1 -ka 0 .05 1 -kd 0 .4 .8 -ks 0 .2 .4 -sp 3 -pl 0 0 -5 2 2 2 -pl 0 4 -5 1 1 1 -pl 5 5 -5 1 1 1 -dl 1 -1 -.2 1 1 1 -s -3 -3 -5 1 -s 3 3 -5 1 -s -3 3 -7 3 -s 3 -3 -2 .7 -s -5 2 -5 1