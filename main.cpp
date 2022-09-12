#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "stdlib.h"
#include <stdio.h>

const TGAColor white = TGAColor(255,255,255,255);
const TGAColor red = TGAColor(255,0,0,255);
const TGAColor green = TGAColor(0,255,0,255);
const TGAColor yellow = TGAColor(255,255,0,255);
const TGAColor blue = TGAColor(0,0,255,255);

const int width  = 300;
const int height = 300;

float* Interpolate (int x0, int y0, int x1, int y1) {
    if(x0 == x1) {
        return NULL;
    }

    float *dependentValues = (float*) malloc(sizeof(int) * (x1-x0));

    float d = (y1-y0)/(float)(x1-x0);
    float interpolatedY = y0;
    for(int i = x0; i < x1; i++) {
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
        for (int i = x0; i < x1; i++) {
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
        for(int i = y0; i < y1; i++) {
            image.set(dependentValues[i - y0], i, color);
        }
    }
}

void DrawLineTriangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color) {
    DrawLine(v0, v1,image,color);
    DrawLine(v0, v2,image,color);
    DrawLine(v1, v2,image,color);
}

void SwapVertices(Vec2i &v0, Vec2i &v1) {
    Vec2i interim = v0;
    v0 = v1;
    v1 = interim;
}

void DrawFilledTriangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color) {
    
    DrawLineTriangle(v0,v1,v2,image, color);

  // the line between bottomost and topmost forms one side of the triangle. The other side is made of 2 lines. One from top to middle, then one from middle to bottom.

  // sort the vertices from bottom to up in the y direction.
    if(v0.y > v1.y) SwapVertices(v0,v1); // v1 cfm more than v0
    if(v0.y > v2.y) SwapVertices(v0,v2); // v2 and v1 cfm more than v0
    if(v1.y > v2.y) SwapVertices(v1,v2); // v2 cfm more than v1

  // for each y and then each line, get the x value that corresponds to the y value.
    float* x01_InterpolatedValues = Interpolate(v0.y, v0.x, v1.y, v1.x);
    float* x12_InterpolatedValues = Interpolate(v1.y, v1.x, v2.y, v2.x);
    float* x02_InterpolatedValues = Interpolate(v0.y, v0.x, v2.y, v2.x);

  // combine the two lines to form the other side of the triangle.
    float* x012_InterpolatedValues = (float*)malloc(sizeof(float) * (v2.y - v0.y + 1)); // allocate array space for the 2 shorter sides combined together.
    for(int y = v0.y; y < v1.y; y++) {
        x012_InterpolatedValues[y - v0.y] = x01_InterpolatedValues[y-v0.y];
    }
    for(int y = v1.y; y <= v2.y; y++) {
        x012_InterpolatedValues[y-v0.y] = x12_InterpolatedValues[y-v1.y];
    }

    float* x_LeftInterpolatedValues;
    float* x_RightInterpolatedValues;
    int middleIndex = (v2.y - v0.y + 1) / 2;
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
        for(int x = xStart; x < xEnd; x++) {
            image.set(x, y, color);
        }
    }
}

// int main(int argc, char** argv) {
//     // setvbuf(stdout, NULL, _IOLBF, 0);

//     TGAImage image(width,height, TGAImage::RGB);
   
//     Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
//     Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
//     Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
//     Vec2i t3[3] = {Vec2i(200, 50), Vec2i(230, 280), Vec2i(260, 140)};
//     Vec2i t4[3] = {Vec2i(10,200), Vec2i(80, 280), Vec2i(140, 210)};
//     DrawFilledTriangle(t0[0], t0[1], t0[2], image, red); 
//     DrawFilledTriangle(t1[0], t1[1], t1[2], image, white); 
//     DrawFilledTriangle(t2[0], t2[1], t2[2], image, green);
//     DrawFilledTriangle(t3[0], t3[1], t3[2], image, yellow);
//     DrawFilledTriangle(t4[0], t4[1], t4[2], image, blue);
//     image.flip_vertically();
//     image.write_tga_file("outputTriangle.tga");
//     return 0;
// }

Model *model = NULL;

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);

        Vec2i* screen_coords = (Vec2i*)malloc(sizeof(Vec2i) * 3);

        // for every vertex in the face
        for (int j=0; j<3; j++) { 

            Vec3f vertInWorld = model->vert(face[j]);

            int x = (vertInWorld.x+1.)*width/2.;
            int y = (vertInWorld.y+1.)*height/2.;            

            screen_coords[j] = Vec2i(x,y);

            // Draw a line betweeen the current vertice plus the adjacent vertice. 
            // the modulus creates the loop around for the end vertice because it'll need to use the starting vertice.
            // Vec3f v0 = model->vert(face[j]);
            // Vec3f v1 = model->vert(face[(j+1)%3]);
            // int x0 = (v0.x+1.)*width/2.;
            // int y0 = (v0.y+1.)*height/2.;
            // int x1 = (v1.x+1.)*width/2.;
            // int y1 = (v1.y+1.)*height/2.;
            // Vec2i pointStart = Vec2i(x0,y0);
            // Vec2i pointEnd = Vec2i(x1,y1);
            // DrawLine(pointStart, pointEnd, image, white);
        }
        DrawFilledTriangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("outputWireRenderHead.tga");
    delete model;
    return 0;
}