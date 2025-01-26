#pragma once

// Summary of rules concerning the coordinate systems of this engine:
//  ALL COORDINATE SYSTEMS ARE RIGHT HANDED INCLUDING CLIP SPACE
//  This means our clip space is z=1.0 at near plane z=0.0 at far plane
//  Our 3d coordinate system is x=forward, y=left, z=up
//  This gets projected into a clip space which is x=right, y=up, z=back (right handed)
//  Our 2d coordinate system is x=right, y=up, z=back
//  Remember d3d texture coordinates are top left corner = (0,0), so we have to flip the y
//  For a skybox image x axis = center of the image, 
//  Triagles wind counter clockwise
