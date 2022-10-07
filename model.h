#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faceVertexCoordinates_;
	std::vector<std::vector<int> > faceTextureCoordinates_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	std::vector<int> faceVertexCoordinates(int idx);
	std::vector<int> faceTextureCoordinates(int idx);
};

#endif //__MODEL_H__
