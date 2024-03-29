#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "stdlib.h"
#include <limits>
#include <stdio.h>
#include <iostream>

const int width  = 900;
const int height = 900;

float* Interpolate (int x0, float y0, int x1, float y1) {
    if(x0 == x1) {
        float* dependentValue = (float*)malloc(sizeof(float)*1);
        dependentValue[0] = y0;
        return dependentValue;
    }

    float *dependentValues = (float*) malloc(sizeof(float) * abs((x1 - x0 + 1)));

    float d = (y1-y0)/(float)(x1-x0);
    float interpolatedY = y0;
    for(int i = x0; i <= x1; i++) {
        dependentValues[i - x0] = interpolatedY;
        interpolatedY += d;
    }
    return dependentValues;
}

void DrawLine(Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color) {

    int x0 = v1.x;
    int y0 = v1.y;
    int x1 = v2.x;
    int y1 = v2.y;

    int dx = x1 - x0;
    int dy = y1 - y0;

    // the dy dx comparisons ensure that:
    // 1. the variable with greater amount of change is the independent variable. Ensures that jumps in the dependent value aren't as large.
    // 2. the start vertex is always to the left of the end vertex w.r.t. independent variable.
    // end result is that x0,y0 are to the bottom left of x1,y1, though they might not be similar to the original x0,y0,x1,y1. Quite dirty programming cause reuse a variable, but refactor later if needed.

    if(abs(dx) > abs(dy)) {
        if(dx < 0) {
            // need to ensure that x0 is of lesser value than x1
            int swapX = x0;
            int swapY = y0;
            x0 = x1;
            y0 = y1;
            x1 = swapX;
            y1 = swapY;
        }

        float* dependentValues = Interpolate(x0,y0,x1,y1);
        for (int i = x0; i <= x1; i++) {
            image.set(i, dependentValues[i - x0], color);
        }
    }

    if(abs(dy) > abs(dx)) {
        // line is more vertical than horizontal.
        // need to swap. Want the independent variable to be y and dependent variable to be x.

        // need to ensure y0 is of lesser value than y1
        if(dy < 0) {
            int swapX = x0;
            int swapY = y0;
            x0 = x1;
            y0 = y1;
            x1 = swapX;
            y1 = swapY;
        }
        float* dependentValues = Interpolate(y0,x0,y1,x1);
        for(int i = y0; i <= y1; i++) {
            image.set(dependentValues[i - y0], i, color);
        }
    }
}

void Swap2dVertices(Vec2i &v0, Vec2i &v1) {
    Vec2i interim = v0;
    v0 = v1;
    v1 = interim;
}

// Was code to draw simple triangles, not used except as proof of progress
void RenderSimpleLineTriangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color) {
    DrawLine(v0, v1,image,color);
    DrawLine(v0, v2,image,color);
    DrawLine(v1, v2,image,color);
}

// Was code to draw simple triangles, not used except as proof of progress
void RenderSimpleFilledTriangle(Vec2i screen_coords[], TGAImage &image, TGAColor color) {

    Vec2i v0 = screen_coords[0];
    Vec2i v1 = screen_coords[1];
    Vec2i v2 = screen_coords[2];
    
    // DrawLineTriangle(v0,v1,v2,image, color);

  // the line between bottomost and topmost forms one side of the triangle. The other side is made of 2 lines. One from top to middle, then one from middle to bottom.

  // sort the vertices from bottom to up in the y direction.
    if(v0.y > v1.y) Swap2dVertices(v0,v1); // v1 cfm more than v0
    if(v0.y > v2.y) Swap2dVertices(v0,v2); // v2 and v1 cfm more than v0
    if(v1.y > v2.y) Swap2dVertices(v1,v2); // v2 cfm more than v1

  // for each y and thereby each line, get the x value that corresponds to the y value.
    float* x01_InterpolatedValues = Interpolate(v0.y, v0.x, v1.y, v1.x);
    float* x12_InterpolatedValues = Interpolate(v1.y, v1.x, v2.y, v2.x);
    float* x02_InterpolatedValues = Interpolate(v0.y, v0.x, v2.y, v2.x);

  // combine the two lines to form the other side of the triangle.
    float* x012_InterpolatedValues = (float*)malloc(sizeof(float) * (v2.y - v0.y + 1)); // allocate array space for the 2 shorter sides combined together.
    for(int y = v0.y; y < v1.y; y++) {
        // x012_InterpolatedValues[y - v0.y] = x01_InterpolatedValues[y-v0.y];
        *(x012_InterpolatedValues + y - v0.y) = *(x01_InterpolatedValues + y - v0.y);
    }
    for(int y = v1.y; y <= v2.y; y++) {
        // x012_InterpolatedValues[y-v0.y] = x12_InterpolatedValues[y-v1.y];
        *(x012_InterpolatedValues + y - v0.y) = *(x12_InterpolatedValues + y - v1.y);
    }

    float* x_LeftInterpolatedValues;
    float* x_RightInterpolatedValues;
    int middleIndex = (v2.y - v0.y) / 2 + 1;
    if(x012_InterpolatedValues[middleIndex] < x02_InterpolatedValues[middleIndex]) {
        x_LeftInterpolatedValues = x012_InterpolatedValues;
        x_RightInterpolatedValues = x02_InterpolatedValues;
    } else {
        x_LeftInterpolatedValues = x02_InterpolatedValues;
        x_RightInterpolatedValues = x012_InterpolatedValues;
    }

    // then interpolate the line between the two x values.
    // for each interpolation, draw the pixel values.
    for(int y = v0.y; y <= v2.y; y++) {
        int xStart = x_LeftInterpolatedValues[y-v0.y];
        int xEnd = x_RightInterpolatedValues[y-v0.y];
        for(int x = xStart; x <= xEnd; x++) {
            image.set(x, y, color);
        }
    }
}

void SwapVec3iVertices(Vec3i &v0, Vec3i &v1) {
    Vec3i swap = v0;
    v0 = v1;
    v1 = swap;
}

void SwapVec3fVertices(Vec3f &v0, Vec3f &v1) {
    Vec3f swap = v0;
    v0 = v1;
    v1 = swap;
}

Vec3i ConvertIntoScreenCoord(Vec3f v) {
    int x = (v.x+1.)*width/2.;
    int y = (v.y+1.)*height/2.;
    // using height as the scaling value is arbitrary. Just needed to convert the z values into bigger integer values so that they won't cause problems with all the integer calculations and functions.
    int z = (v.z+1.)*height/2.;
    return Vec3i(x,y,z);
}

float* GetZBuffer() {
    static float zBuffer[height*width];
    static bool created;
    if (!created) {
        for (int i=width*height; i--; zBuffer[i] = -std::numeric_limits<float>::max());
        created = true;
    }
    return zBuffer;
}

// The main method.
// Does z-buffer check.
// Does conversion from world to camera to viewport to screen coordinates.
void DrawTriangle(Vec3f world_coords[], Vec3f texture_coords[], TGAImage &image, TGAImage &textureImage) {
    Vec3i v0 = ConvertIntoScreenCoord(world_coords[0]);
    Vec3i v1 = ConvertIntoScreenCoord(world_coords[1]);
    Vec3i v2 = ConvertIntoScreenCoord(world_coords[2]);
    if(v0.y > v1.y) {
        SwapVec3fVertices(texture_coords[0], texture_coords[1]);
        SwapVec3iVertices(v0,v1); // v1 cfm more than v0
    }
    if(v0.y > v2.y) {
        SwapVec3fVertices(texture_coords[0], texture_coords[2]);
        SwapVec3iVertices(v0,v2); // v2 and v1 cfm more than v0
    }
    if(v1.y > v2.y) {
        SwapVec3fVertices(texture_coords[1], texture_coords[2]);
        SwapVec3iVertices(v1,v2); // v2 cfm more than v1
    }

    // for each y value on the edge, get the interpolated x values.
    float* x01_InterpolatedValues = Interpolate(v0.y, v0.x, v1.y, v1.x);
    float* x12_InterpolatedValues = Interpolate(v1.y, v1.x, v2.y, v2.x);
    float* x02_InterpolatedValues = Interpolate(v0.y, v0.x, v2.y, v2.x); // remember that x02 is the longest edge.

    // for each y value on the edge, get the interpolated z values.
    float* z01_InterpolatedValues = Interpolate(v0.y, v0.z, v1.y, v1.z);
    float* z12_InterpolatedValues = Interpolate(v1.y, v1.z, v2.y, v2.z);
    float* z02_InterpolatedValues = Interpolate(v0.y, v0.z, v2.y, v2.z); // remember that z02 is the longest edge.

    // for each y value on the edge, get the interpolated u texture value (remember that the texture values are still normalised)
    float* u01_Interpolated = Interpolate(v0.y, texture_coords[0].x, v1.y,texture_coords[1].x);
    float* u12_Interpolated = Interpolate(v1.y, texture_coords[1].x, v2.y,texture_coords[2].x);
    float* u02_Interpolated = Interpolate(v0.y, texture_coords[0].x, v2.y,texture_coords[2].x);

    // for each y value on the edge, get the interpolated v texture value (remember that the texture values are still normalised)
    float* v01_Interpolated = Interpolate(v0.y, texture_coords[0].y, v1.y,texture_coords[1].y);
    float* v12_Interpolated = Interpolate(v1.y, texture_coords[1].y, v2.y,texture_coords[2].y);
    float* v02_Interpolated = Interpolate(v0.y, texture_coords[0].y, v2.y,texture_coords[2].y);

    // combine the two shorter lines to form the other side of the triangle. Guaranteed to be from 0-1 then from 1-2.  
    float* x012_InterpolatedValues = (float*)malloc(sizeof(float) * (v2.y - v0.y + 1)); // allocate array space for the 2 shorter sides combined together.
    float* z012_InterpolatedValues = (float*)malloc(sizeof(float) * (v2.y - v0.y + 1)); // allocate array space for the 2 shorter sides combined together.
    float* u012_Interpolated = (float*)malloc(sizeof(float) * (v2.y - v0.y + 1));
    float* v012_Interpolated = (float*)malloc(sizeof(float) * (v2.y - v0.y + 1));
    for(int y = v0.y; y < v1.y; y++) {
        x012_InterpolatedValues[y-v0.y] = x01_InterpolatedValues[y-v0.y];
        z012_InterpolatedValues[y-v0.y] = z01_InterpolatedValues[y-v0.y];
        u012_Interpolated[y-v0.y] = u01_Interpolated[y-v0.y];
        v012_Interpolated[y-v0.y] = v01_Interpolated[y-v0.y];
    }
    for(int y = v1.y; y <= v2.y; y++) {
        x012_InterpolatedValues[y-v0.y] = x12_InterpolatedValues[y-v1.y];
        z012_InterpolatedValues[y-v0.y] = z12_InterpolatedValues[y-v1.y];
        u012_Interpolated[y-v0.y] = u12_Interpolated[y-v1.y];
        v012_Interpolated[y-v0.y] = v12_Interpolated[y-v1.y];
    }

    // Now interpolate the values. Begin from the left line and interpolate across to the right line.
    float* x_LeftInterpolatedValues;
    float* x_RightInterpolatedValues;
    float* z_LeftInterpolatedValues;
    float* z_RightInterpolatedValues;

    float* u_LeftInterpolatedValues;
    float* u_RightInterpolatedValues;
    float* v_LeftInterpolatedValues;
    float* v_RightInterpolatedValues;

    // check which line is on the left and which is on the right. Does so by comparing the mid value of the x coordinates on the interpolated lines.
    int middleIndex = (v2.y - v0.y) / 2 + 1;
    if(x012_InterpolatedValues[middleIndex] < x02_InterpolatedValues[middleIndex]) {
        x_LeftInterpolatedValues = x012_InterpolatedValues;
        x_RightInterpolatedValues = x02_InterpolatedValues;
        z_LeftInterpolatedValues = z012_InterpolatedValues;
        z_RightInterpolatedValues = z02_InterpolatedValues;

        u_LeftInterpolatedValues = u012_Interpolated;
        u_RightInterpolatedValues = u02_Interpolated;
        v_LeftInterpolatedValues = v012_Interpolated;
        v_RightInterpolatedValues = v02_Interpolated;

    } else {
        x_LeftInterpolatedValues = x02_InterpolatedValues;
        x_RightInterpolatedValues = x012_InterpolatedValues;
        z_LeftInterpolatedValues = z02_InterpolatedValues;
        z_RightInterpolatedValues = z012_InterpolatedValues;

        u_LeftInterpolatedValues = u02_Interpolated;
        u_RightInterpolatedValues = u012_Interpolated;
        v_LeftInterpolatedValues = v02_Interpolated;
        v_RightInterpolatedValues = v012_Interpolated;
    }

    float* zBuffer = GetZBuffer();
    int textureImageWidth = textureImage.get_width();
    int textureImageHeight = textureImage.get_height();

    // then interpolate the values between the two lines. Since we're going horizontal, the interpolation this time is based on the x coordinate values.
    for(int y = v0.y; y <= v2.y; y++) {
        // assigning these start and end values are mostly for convenience, not strictly necessary.
        int xStart = x_LeftInterpolatedValues[y-v0.y];
        int xEnd = x_RightInterpolatedValues[y-v0.y];
        int zStart = z_LeftInterpolatedValues[y-v0.y];
        int zEnd = z_RightInterpolatedValues[y-v0.y];

        float uStart = u_LeftInterpolatedValues[y-v0.y];
        float uEnd = u_RightInterpolatedValues[y-v0.y];
        float vStart = v_LeftInterpolatedValues[y-v0.y];
        float vEnd = v_RightInterpolatedValues[y-v0.y];

        // interpolate the values of z, u and v from one edge to another.
        // the previous z interpolations determined the value of z at the edges. But now you want to know the value of z at every pixel within the horizontal line being drawn.
        float* interpolatedZ = Interpolate(xStart, zStart, xEnd, zEnd);
        float* interpolatedU = Interpolate(xStart, uStart, xEnd, uEnd);
        float* interpolatedV = Interpolate(xStart, vStart, xEnd, vEnd);
        
        for(int x = xStart; x <= xEnd; x++) {
            // have to check the z-buffer for this pixel. Are there previous existing values for this pixel? If so, does this new value have a z-value greater than the previous value?
            int bufferIndex = y * height + x;
            float zValue = interpolatedZ[x - xStart];
            if(zBuffer[bufferIndex] < zValue) {
                zBuffer[bufferIndex] = zValue;

                int uCoord = (int)(interpolatedU[x-xStart] * textureImageWidth);
                int vCoord = (int)(interpolatedV[x-xStart] * textureImageHeight);

                TGAColor texturePixel = textureImage.get(uCoord, vCoord);
                image.set(x, y, texturePixel);
            }
            // Scratch used perspective projection, but tiny just goes from camera to canvas. No perspective projection into viewport first.
        }
    }
}

//--------DRIVING CODE-----------

void DrawSimpleFilledTriangles(TGAImage &image) {

    const TGAColor white = TGAColor(255,255,255,255);
    const TGAColor red = TGAColor(255,0,0,255);
    const TGAColor green = TGAColor(0,255,0,255);
    const TGAColor yellow = TGAColor(255,255,0,255);
    const TGAColor blue = TGAColor(0,0,255,255);

    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    Vec2i t3[3] = {Vec2i(200, 50), Vec2i(230, 280), Vec2i(260, 140)};
    Vec2i t4[3] = {Vec2i(10,200), Vec2i(80, 280), Vec2i(140, 210)};
    RenderSimpleFilledTriangle(t0, image, red); 
    RenderSimpleFilledTriangle(t1, image, white); 
    RenderSimpleFilledTriangle(t2, image, green);
    RenderSimpleFilledTriangle(t3, image, yellow);
    RenderSimpleFilledTriangle(t4, image, blue);
    image.flip_vertically();
    image.write_tga_file("./output/SimpleFilledTriangle.tga");
}

// No z-buffer checking
// No lighting calculation
void DrawSimpleLineOrFilledModelTriangles(Model* model, TGAImage &image) {
    
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->faceVertexIndices(i);

        Vec2i* screen_coords = (Vec2i*)malloc(sizeof(Vec2i) * 3);

        // for every vertex in the face
        for (int j=0; j<3; j++) { 

            Vec3f vertInWorld = model->vertPositions(face[j]);
            // the wavefront obj vertices are stored in normalised range of (-1 to 1). By adding 1, we normalise to (0,2).
            // That's why division by 2 happens, to normalise down to (0,1).
            int x = (vertInWorld.x+1.)*width/2.;
            int y = (vertInWorld.y+1.)*height/2.;            

            screen_coords[j] = Vec2i(x,y);

        }

        // RenderSimpleLineTriangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
        RenderSimpleFilledTriangle(screen_coords, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image.
    // image.write_tga_file("./output/LineTriangleModel.tga");
    // Enabling this also requires enabling RenderSimpleFilledTriangle()
    image.write_tga_file("./output/FilledTriangleModel.tga");

    delete model;
}

void DrawModel(Model* model, TGAImage &image) {
    Vec3f light_dir(0,0,-1); // define light_dir, I guess it's using right hand coord system?

    TGAImage textureImage;
    textureImage.read_tga_file("obj/diablo3_pose_diffuse.tga");
    textureImage.flip_vertically();

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> faceVertices = model->faceVertexIndices(i); // returns the 3 coordinate indices for the face.
        std::vector<int> faceTextures = model->faceTextureIndices(i); // returns the 3 texture indices for the face.
        Vec3f* world_coords = (Vec3f*)malloc(sizeof(Vec3f) * 3);
        Vec3f* texture_coords = (Vec3f*)malloc(sizeof(Vec3f) * 3);
       for (int j=0; j<3; j++) {
            Vec3f v = model->vertPositions(faceVertices[j]); // get the actual vertex coordinate values based on the index passed in from the face. The Vec3f encapsulates the x,y,z values.
            world_coords[j]  = v; // every 3 vertex coordinate values form one world_coords face.
            Vec3f uv = model->vertTextures(faceTextures[j]); // get the actual vertex texture coordinate values based on the index passed in from the face. The Vec3f encapsulates the u,v,z values.
            texture_coords[j] = uv; // every 3 values form one texture_coords face.
        }
        // cross product of the two lines of the triangles. Gives the normal of triangle.
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
        n.normalize();
        float intensity = n*light_dir; // dot product for the degree to which the light vector and the normal are parallel. The more the better illuminated.
        if (intensity>0) {
            DrawTriangle(world_coords, texture_coords, image, textureImage);
        }
        free(world_coords);
    }
    image.flip_vertically();
    image.write_tga_file("./output/Model.tga");
}

// Read in texture file.
// Find the 3 vt corresponding to each vertice in a face.
// interpolate the vt values along the 3 lines
// for each y value, then interpolate across the line.
// then multiply the lighting with the vt colour.

int main(int argc, char** argv) {
    Model *model = NULL;
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/diablo.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    // DrawSimpleFilledTriangles(image);
    DrawSimpleLineOrFilledModelTriangles(model, image); 
    // DrawModel(model, image);
    return 0;
}