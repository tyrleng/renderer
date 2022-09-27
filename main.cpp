#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "stdlib.h"
#include <limits>
#include <stdio.h>

const TGAColor white = TGAColor(255,255,255,255);
const TGAColor red = TGAColor(255,0,0,255);
const TGAColor green = TGAColor(0,255,0,255);
const TGAColor yellow = TGAColor(255,255,0,255);
const TGAColor blue = TGAColor(0,0,255,255);

const int width  = 900;
const int height = 900;

float* Interpolate (int x0, int y0, int x1, int y1) {
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

void DrawLineTriangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color) {
    DrawLine(v0, v1,image,color);
    DrawLine(v0, v2,image,color);
    DrawLine(v1, v2,image,color);
}

void Swap2dVertices(Vec2i &v0, Vec2i &v1) {
    Vec2i interim = v0;
    v0 = v1;
    v1 = interim;
}

void DrawFilledTriangle(Vec2i screen_coords[], float zValue[], TGAImage &image, TGAColor color) {

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

Vec3i ConvertIntoScreenCoordAndZBuffer(Vec3f v) {
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

void DrawFilledTriangleExperimental(Vec3f world_coords[], TGAImage &image, TGAColor color) {
    Vec3i v0 = ConvertIntoScreenCoordAndZBuffer(world_coords[0]);
    Vec3i v1 = ConvertIntoScreenCoordAndZBuffer(world_coords[1]);
    Vec3i v2 = ConvertIntoScreenCoordAndZBuffer(world_coords[2]);

    if(v0.y > v1.y) {
        SwapVec3fVertices(world_coords[0], world_coords[1]);
        SwapVec3iVertices(v0,v1); // v1 cfm more than v0
    }
    if(v0.y > v2.y) {
        SwapVec3fVertices(world_coords[0], world_coords[2]);
        SwapVec3iVertices(v0,v2); // v2 and v1 cfm more than v0
    } 
    if(v1.y > v2.y) {
        SwapVec3fVertices(world_coords[1], world_coords[2]);
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

    // combine the two lines to form the other side of the triangle.
    float* x012_InterpolatedValues = (float*)malloc(sizeof(float) * (v2.y - v0.y + 1)); // allocate array space for the 2 shorter sides combined together.
    float* z012_InterpolatedValues = (float*)malloc(sizeof(float) * (v2.y - v0.y + 1)); // allocate array space for the 2 shorter sides combined together.

    for(int y = v0.y; y < v1.y; y++) {
        x012_InterpolatedValues[y-v0.y] = x01_InterpolatedValues[y-v0.y];
        z012_InterpolatedValues[y-v0.y] = z01_InterpolatedValues[y-v0.y];
    }
    for(int y = v1.y; y <= v2.y; y++) {
        x012_InterpolatedValues[y-v0.y] = x12_InterpolatedValues[y-v1.y];
        z012_InterpolatedValues[y-v0.y] = z12_InterpolatedValues[y-v1.y];
    }

    float* x_LeftInterpolatedValues;
    float* x_RightInterpolatedValues;
    float* z_LeftInterpolatedValues;
    float* z_RightInterpolatedValues;

    int middleIndex = (v2.y - v0.y) / 2 + 1;
    if(x012_InterpolatedValues[middleIndex] < x02_InterpolatedValues[middleIndex]) {
        x_LeftInterpolatedValues = x012_InterpolatedValues;
        x_RightInterpolatedValues = x02_InterpolatedValues;
        z_LeftInterpolatedValues = z012_InterpolatedValues;
        z_RightInterpolatedValues = z02_InterpolatedValues;
    } else {
        x_LeftInterpolatedValues = x02_InterpolatedValues;
        x_RightInterpolatedValues = x012_InterpolatedValues;
        z_LeftInterpolatedValues = z02_InterpolatedValues;
        z_RightInterpolatedValues = z012_InterpolatedValues;
    }

    float* zBuffer = GetZBuffer();

    // then interpolate the values between the two lines.
    // for each interpolation, draw the pixel values.
    for(int y = v0.y; y <= v2.y; y++) {
        int xStart = x_LeftInterpolatedValues[y-v0.y];
        int xEnd = x_RightInterpolatedValues[y-v0.y];
        int zStart = z_LeftInterpolatedValues[y-v0.y];
        int zEnd = z_RightInterpolatedValues[y-v0.y];

        // interpolate the values of z from one edge to another.
        // the previous z interpolations determined the value of z at the edges. But now you want to know the value of z at every pixel within the horizontal line being drawn.
        float* interpolatedZ = Interpolate(xStart, zStart, xEnd, zEnd);
        
        for(int x = xStart; x <= xEnd; x++) {
            int bufferIndex = y * height + x;
            float zValue = interpolatedZ[x - xStart];
            if(zBuffer[bufferIndex] < zValue) {
                zBuffer[bufferIndex] = zValue;
                image.set(x, y, color);
            }
            // For each pixel value, determine if the pixel should or shouldn't be displayed.
            // If displayed, write the z value to the depth buffer. Later pixels will then need to challenge this depth value.

            // Scratch used perspective projection, but tiny just goes from camera to canvas. No perspective projection into viewport first.
        }
    }
}

void DrawSimpleFilledTriangles(TGAImage &image) {
    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    Vec2i t3[3] = {Vec2i(200, 50), Vec2i(230, 280), Vec2i(260, 140)};
    Vec2i t4[3] = {Vec2i(10,200), Vec2i(80, 280), Vec2i(140, 210)};
    DrawFilledTriangle(t0, NULL, image, red); 
    DrawFilledTriangle(t1, NULL, image, white); 
    DrawFilledTriangle(t2, NULL, image, green);
    DrawFilledTriangle(t3, NULL, image, yellow);
    DrawFilledTriangle(t4, NULL, image, blue);
    image.flip_vertically();
    image.write_tga_file("outputTriangle.tga");
}

void DrawLineOrFilledHead(Model* model, TGAImage &image) {
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);

        Vec2i* screen_coords = (Vec2i*)malloc(sizeof(Vec2i) * 3);

        // for every vertex in the face
        for (int j=0; j<3; j++) { 

            Vec3f vertInWorld = model->vert(face[j]);
            // the wavefront obj vertices are stored in normalised range of (-1 to 1). By adding 1, we normalise to (0,2).
            // That's why division by 2 happens, to normalise down to (0,1).
            int x = (vertInWorld.x+1.)*width/2.;
            int y = (vertInWorld.y+1.)*height/2.;            

            screen_coords[j] = Vec2i(x,y);

        }
        DrawFilledTriangle(screen_coords, NULL, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
        DrawLineTriangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image.
    image.write_tga_file("outputFilledTriangleHead.tga");
    // image.write_tga_file("outputLIneTriangleHead.tga");
    delete model;
}

void DrawFlatIlluminatedHead(Model* model, TGAImage &image) {
    Vec3f light_dir(0,0,-1); // define light_dir, I guess it's using right hand coord system?

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i); 
        Vec3f* world_coords = (Vec3f*)malloc(sizeof(Vec3f) * 3); 
        // Vec2i screen_coords[3];
       for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            world_coords[j]  = v;
            // from "camera space" to screen space. But there isn't really a camera yet.
            // screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.); 
        }
        // cross product of the two lines of the triangles. Gives the normal of triangle.
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
        n.normalize();
        float intensity = n*light_dir; // dot product for the degree to which the light vector and the normal are parallel. The more the better illuminated.
        if (intensity>0) {
            // DrawFilledTriangle(screen_coords, NULL, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
            DrawFilledTriangleExperimental(world_coords, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
        free(world_coords);
    }
    image.flip_vertically();
    image.write_tga_file("outputFlatIlluminatedDepthHead.tga");
}

int main(int argc, char** argv) {
    Model *model = NULL;
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    // DrawLineOrFilledHead(model, image);    
    // DrawSimpleFilledTriangles(image);
    DrawFlatIlluminatedHead(model, image);
    return 0;
}