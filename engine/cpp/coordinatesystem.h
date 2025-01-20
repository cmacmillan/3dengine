#pragma once

// Summary of rules concerning the coordinate systems of this engine:
//  These may not all 100% true currently
//	Our coordinate systems are all right handed
//  Our 3d coordinate system is x=forward, y=left, z=up
//  This gets projected into a clip space which is x=right, y=up, z=back (z forward = negative)
//  Our 2d coordinate system is x=right, y=up, z=back (z forward = negative)
//  For a skybox image x axis = center of the image, 
