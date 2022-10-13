#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faceVertexIndices_(), faceTextureIndices_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;

        if (!line.compare(0, 2, "v ")) {
            iss >> trash; // trashes the 'v' character
            Vec3f v;
            for (int i=0;i<3;i++) {
                iss >> v.raw[i];
            }
            verts_.push_back(v);
        }

        else if(!line.compare(0,3, "vt ")) {
            iss >> trash;
            Vec3f vt;
            for (int i=0;i<3;i++) {
                iss >> vt.raw[i];
            }
            vertTextures_.push_back(vt);
        }
        
         else if (!line.compare(0, 2, "f ")) {
            std::vector<int> verticesIndex, texturesIndex;
            int vertex, texture, normal;
            iss >> trash; // trashes the 'f' character
            while (iss >> vertex >> trash >> texture >> trash >> normal) {
                // The three numbers (vertex, texture, normal) held by each group are the vertex coordinates, the texture coordinate and the vertex normal.
                // trash throws away the forward slashes (/)
                vertex--; // in wavefront obj all indices start at 1, not zero. So need to adjust by -1.
                verticesIndex.push_back(vertex); // will gain 3 index values per face
                texturesIndex.push_back(texture); // will gain 3 index values per face
            }
            faceVertexIndices_.push_back(verticesIndex);
            faceTextureIndices_.push_back(texturesIndex);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faceVertexIndices_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faceVertexIndices_.size();
}

std::vector<int> Model::faceVertexIndices(int faceIndex) {
    return faceVertexIndices_[faceIndex];
}

std::vector<int> Model::faceTextureIndices(int faceIndex) {
    return faceTextureIndices_[faceIndex];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vertTextures(int i) {
    return vertTextures_[i];
}