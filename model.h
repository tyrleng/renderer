#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> vertTextures_;
	std::vector<std::vector<int>> faceVertexIndices_;
	std::vector<std::vector<int>> faceTextureIndices_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec3f vertTextures(int i);
	std::vector<int> faceVertexIndices(int faceIndex);
	std::vector<int> faceTextureIndices(int faceIndex);
};

#endif //__MODEL_H__
