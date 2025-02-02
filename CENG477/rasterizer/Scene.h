#ifndef _SCENE_H_
#define _SCENE_H_
#include "Vec3.cpp"
#include "Vec4.cpp"
#include "Color.cpp"
#include "Rotation.cpp"
#include "Scaling.cpp"
#include "Translation.cpp"
#include "Camera.cpp"
#include "Mesh.cpp"

class Scene
{
public:
	Color backgroundColor;
	bool cullingEnabled;

	std::vector<std::vector<Color> > image;
	std::vector<Camera *> cameras;
	std::vector<Vec3 *> vertices;
	std::vector<Color *> colorsOfVertices;
	std::vector<Scaling *> scalings;
	std::vector<Rotation *> rotations;
	std::vector<Translation *> translations;
	std::vector<Mesh *> meshes;

	Scene(const char *xmlPath);

	void initializeImage(Camera *camera);
	int makeBetweenZeroAnd255(double value);
	void writeImageToPPMFile(Camera *camera);
	void convertPPMToPNG(std::string ppmFileName, int osType);
	void forwardRenderingPipeline(Camera *camera);
};

#endif
