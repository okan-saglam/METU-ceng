#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#include "tinyxml2.cpp"
#include "Triangle.cpp"
#include "Helpers.cpp"
#include "Scene.h"

using namespace tinyxml2;
using namespace std;

#define M_PI 3.14159265358979323846




/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLElement *xmlElement;

	xmlDoc.LoadFile(xmlPath);

	XMLNode *rootNode = xmlDoc.FirstChild();

	// read background color
	xmlElement = rootNode->FirstChildElement("BackgroundColor");
	str = xmlElement->GetText();
	sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

	// read culling
	xmlElement = rootNode->FirstChildElement("Culling");
	if (xmlElement != NULL)
	{
		str = xmlElement->GetText();

		if (strcmp(str, "enabled") == 0)
		{
			this->cullingEnabled = true;
		}
		else
		{
			this->cullingEnabled = false;
		}
	}

	// read cameras
	xmlElement = rootNode->FirstChildElement("Cameras");
	XMLElement *camElement = xmlElement->FirstChildElement("Camera");
	XMLElement *camFieldElement;
	while (camElement != NULL)
	{
		Camera *camera = new Camera();

		camElement->QueryIntAttribute("id", &camera->cameraId);

		// read projection type
		str = camElement->Attribute("type");

		if (strcmp(str, "orthographic") == 0)
		{
			camera->projectionType = ORTOGRAPHIC_PROJECTION;
		}
		else
		{
			camera->projectionType = PERSPECTIVE_PROJECTION;
		}

		camFieldElement = camElement->FirstChildElement("Position");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->position.x, &camera->position.y, &camera->position.z);

		camFieldElement = camElement->FirstChildElement("Gaze");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->gaze.x, &camera->gaze.y, &camera->gaze.z);

		camFieldElement = camElement->FirstChildElement("Up");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->v.x, &camera->v.y, &camera->v.z);

		camera->gaze = normalizeVec3(camera->gaze);
		camera->u = crossProductVec3(camera->gaze, camera->v);
		camera->u = normalizeVec3(camera->u);

		camera->w = inverseVec3(camera->gaze);
		camera->v = crossProductVec3(camera->u, camera->gaze);
		camera->v = normalizeVec3(camera->v);

		camFieldElement = camElement->FirstChildElement("ImagePlane");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
			   &camera->left, &camera->right, &camera->bottom, &camera->top,
			   &camera->near, &camera->far, &camera->horRes, &camera->verRes);

		camFieldElement = camElement->FirstChildElement("OutputName");
		str = camFieldElement->GetText();
		camera->outputFilename = string(str);

		this->cameras.push_back(camera);

		camElement = camElement->NextSiblingElement("Camera");
	}

	// read vertices
	xmlElement = rootNode->FirstChildElement("Vertices");
	XMLElement *vertexElement = xmlElement->FirstChildElement("Vertex");
	int vertexId = 1;

	while (vertexElement != NULL)
	{
		Vec3 *vertex = new Vec3();
		Color *color = new Color();

		vertex->colorId = vertexId;

		str = vertexElement->Attribute("position");
		sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

		str = vertexElement->Attribute("color");
		sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

		this->vertices.push_back(vertex);
		this->colorsOfVertices.push_back(color);

		vertexElement = vertexElement->NextSiblingElement("Vertex");

		vertexId++;
	}

	// read translations
	xmlElement = rootNode->FirstChildElement("Translations");
	XMLElement *translationElement = xmlElement->FirstChildElement("Translation");
	while (translationElement != NULL)
	{
		Translation *translation = new Translation();

		translationElement->QueryIntAttribute("id", &translation->translationId);

		str = translationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

		this->translations.push_back(translation);

		translationElement = translationElement->NextSiblingElement("Translation");
	}

	// read scalings
	xmlElement = rootNode->FirstChildElement("Scalings");
	XMLElement *scalingElement = xmlElement->FirstChildElement("Scaling");
	while (scalingElement != NULL)
	{
		Scaling *scaling = new Scaling();

		scalingElement->QueryIntAttribute("id", &scaling->scalingId);
		str = scalingElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

		this->scalings.push_back(scaling);

		scalingElement = scalingElement->NextSiblingElement("Scaling");
	}

	// read rotations
	xmlElement = rootNode->FirstChildElement("Rotations");
	XMLElement *rotationElement = xmlElement->FirstChildElement("Rotation");
	while (rotationElement != NULL)
	{
		Rotation *rotation = new Rotation();

		rotationElement->QueryIntAttribute("id", &rotation->rotationId);
		str = rotationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

		this->rotations.push_back(rotation);

		rotationElement = rotationElement->NextSiblingElement("Rotation");
	}

	// read meshes
	xmlElement = rootNode->FirstChildElement("Meshes");

	XMLElement *meshElement = xmlElement->FirstChildElement("Mesh");
	while (meshElement != NULL)
	{
		Mesh *mesh = new Mesh();

		meshElement->QueryIntAttribute("id", &mesh->meshId);

		// read projection type
		str = meshElement->Attribute("type");

		if (strcmp(str, "wireframe") == 0)
		{
			mesh->type = WIREFRAME_MESH;
		}
		else
		{
			mesh->type = SOLID_MESH;
		}

		// read mesh transformations
		XMLElement *meshTransformationsElement = meshElement->FirstChildElement("Transformations");
		XMLElement *meshTransformationElement = meshTransformationsElement->FirstChildElement("Transformation");

		while (meshTransformationElement != NULL)
		{
			char transformationType;
			int transformationId;

			str = meshTransformationElement->GetText();
			sscanf(str, "%c %d", &transformationType, &transformationId);

			mesh->transformationTypes.push_back(transformationType);
			mesh->transformationIds.push_back(transformationId);

			meshTransformationElement = meshTransformationElement->NextSiblingElement("Transformation");
		}

		mesh->numberOfTransformations = mesh->transformationIds.size();

		// read mesh faces
		char *row;
		char *cloneStr;
		int v1, v2, v3;
		XMLElement *meshFacesElement = meshElement->FirstChildElement("Faces");
		str = meshFacesElement->GetText();
		cloneStr = strdup(str);

		row = strtok(cloneStr, "\n");
		while (row != NULL)
		{
			int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);

			if (result != EOF)
			{
				mesh->triangles.push_back(Triangle(v1, v2, v3));
			}
			row = strtok(NULL, "\n");
		}
		mesh->numberOfTriangles = mesh->triangles.size();
		this->meshes.push_back(mesh);

		meshElement = meshElement->NextSiblingElement("Mesh");
	}
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
	if (this->image.empty())
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			vector<Color> rowOfColors;

			for (int j = 0; j < camera->verRes; j++)
			{
				rowOfColors.push_back(this->backgroundColor);
			}

			this->image.push_back(rowOfColors);
		}
	}
	else
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			for (int j = 0; j < camera->verRes; j++)
			{
				this->image[i][j].r = this->backgroundColor.r;
				this->image[i][j].g = this->backgroundColor.g;
				this->image[i][j].b = this->backgroundColor.b;
			}
		}
	}
}

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}


/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
	ofstream fout;

	fout.open(camera->outputFilename.c_str());

	fout << "P3" << endl;
	fout << "# " << camera->outputFilename << endl;
	fout << camera->horRes << " " << camera->verRes << endl;
	fout << "255" << endl;

	for (int j = camera->verRes - 1; j >= 0; j--)
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
	os_type == 1 		-> Ubuntu
	os_type == 2 		-> Windows
	os_type == other	-> No conversion
*/
void Scene::convertPPMToPNG(string ppmFileName, int osType)
{
	string command;

	// call command on Ubuntu
	if (osType == 1)
	{
		command = "./magick " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// call command on Windows
	else if (osType == 2)
	{
		command = "magick " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// default action - don't do conversion
	else
	{
	}
}


/*
	Transformations, clipping, culling, rasterization are done here.
*/
void draw_line(Vec4 p0, Vec4 p1, Color temp0, Color temp1, std::vector<std::vector<Color>>& img, int& horRes, int& verRes,double**depthBuffer) {
    int x, y;
    double dx, dy, d;
    Color c, c0, c1, dc;

    dx = p1.x - p0.x;
    dy = p1.y - p0.y;

    c0 = temp0;
    c1 = temp1;

    if (std::abs(dx) >= std::abs(dy)) {
        if (dx < 0) {
            std::swap(p0, p1);
            std::swap(c0, c1);
            dx = -dx;
            dy = -dy;
        }

        if (dy > 0) {
            y = p0.y;
            d = (p0.y - p1.y) + 0.5 * (p1.x - p0.x);
        } else {           
            y = p0.y;
            d = (p0.y - p1.y) - 0.5 * (p1.x - p0.x);
        }

        c = c0;
        dc.r = (c1.r - c0.r) / dx;
        dc.g = (c1.g - c0.g) / dx;
        dc.b = (c1.b - c0.b) / dx;

        for (x = p0.x; x <= p1.x; ++x) {
            if(((x >= 0) && (x < horRes)) && ((y >= 0) && (y < verRes))){
				double t = (x-p0.x)/(double)(p1.x-p0.x); 
				double z = p0.z*(1-t)+p1.z*t;

				if(z < depthBuffer[x][y]) {
					depthBuffer[x][y] = z;
					img[x][y].r = std::round(c.r);
					img[x][y].g = std::round(c.g);
					img[x][y].b = std::round(c.b);
            	}
			}

            if (dy > 0 && d < 0) {
                y++;
                d += (p0.y - p1.y) + (p1.x - p0.x);
            } 
			else if (dy < 0 && d > 0) {
                y--;
                d += (p0.y - p1.y) - (p1.x - p0.x);
            } 
			else {
                d += (p0.y - p1.y);
            }

            c.r += dc.r;
            c.g += dc.g;
            c.b += dc.b;
        }
    } 
	else {
       
        if (dy < 0) {
            
            std::swap(p0, p1);
            std::swap(c0, c1);
            dx = -dx;
            dy = -dy;
        }

        if (dx > 0) {
           
            x = p0.x;
            d = (p0.x - p1.x) + 0.5 * (p1.y - p0.y);
        } else {
            
            x = p0.x;
            d = (p0.x - p1.x) - 0.5 * (p1.y - p0.y);
        }

        c = c0;
        dc.r = (c1.r - c0.r) / dy;
        dc.g = (c1.g - c0.g) / dy;
        dc.b = (c1.b - c0.b) / dy;

        for (y = p0.y; y <= p1.y; ++y) {
            if(((x >= 0) && (x < horRes)) && ((y >= 0) && (y < verRes))){
				
				double t = (y-p0.y)/(double)(p1.y-p0.y); 
				double z = p0.z*(1-t)+p1.z*t;

				if(z < depthBuffer[x][y]) {
					depthBuffer[x][y] = z;
					img[x][y].r = std::round(c.r);
					img[x][y].g = std::round(c.g);
					img[x][y].b = std::round(c.b);
            	}

				
			}

            if (dx > 0 && d < 0) {
                x++;
                d += (p0.x - p1.x) + (p1.y - p0.y);
            } else if (dx < 0 && d > 0) {
                x--;
                d += (p0.x - p1.x) - (p1.y - p0.y);
            } else {
                d += (p0.x - p1.x);
            }

            c.r += dc.r;
            c.g += dc.g;
            c.b += dc.b;
        }
    }
}

double f01(double x, double y, Vec4 &p0, Vec4 &p1){
	return x*(p0.y - p1.y) + y*(p1.x - p0.x) + p0.x*p1.y - p0.y*p1.x;
}

double f12(double x, double y, Vec4 &p1, Vec4 &p2){
	return x*(p1.y - p2.y) + y*(p2.x - p1.x) + p1.x*p2.y - p1.y*p2.x;
}

double f20(double x, double y, Vec4 &p2, Vec4 &p0){
	return x*(p2.y - p0.y) + y*(p0.x - p2.x) + p2.x*p0.y - p2.y*p0.x;
}


void rasterization(Vec4 &p0, Vec4 &p1, Vec4 &p2, int id0, int id1, int id2, std::vector<Color*>& color_of_vertices, std::vector<std::vector<Color>>& img,double **depth_buffer, int& horRes, int& verRes){
	double y_min, x_min, y_max, x_max;
	Color c0, c1, c2;

	y_min = p0.y < p1.y ? p0.y : p1.y;
	y_min = y_min < p2.y ? y_min : p2.y;

	y_max = p0.y > p1.y ? p0.y : p1.y;
	y_max = y_max > p2.y ? y_max : p2.y;

	x_min = p0.x < p1.x ? p0.x : p1.x;
	x_min = x_min < p2.x ? x_min : p2.x;

	x_max = p0.x > p1.x ? p0.x : p1.x;
	x_max = x_max > p2.x ? x_max : p2.x;

	c0 = *color_of_vertices[id0];
	c1 = *color_of_vertices[id1];
	c2 = *color_of_vertices[id2];

	for(int i=y_min ; i <= y_max ; i++){
		for(int j=x_min ; j <= x_max ; j++){
			double alpha, beta, gamma;
			alpha = f12(j, i, p1, p2) / f12(p0.x, p0.y, p1, p2);
			beta = f20(j, i, p2, p0) / f20(p1.x, p1.y, p2, p0);
			gamma = f01(j, i, p0, p1) / f01(p2.x, p2.y, p0, p1);

			if(alpha >= 0 && beta >= 0 && gamma >= 0){
				
				double z = alpha * p0.z + beta * p1.z + gamma * p2.z;
				
				if((j >= 0 && j < horRes) && (i >= 0 && i < verRes)){
					if(z<depth_buffer[j][i]){
						
						depth_buffer[j][i] = z;

						Color c;
						c.r = alpha*c0.r + beta*c1.r + gamma*c2.r;
						c.g = alpha*c0.g + beta*c1.g + gamma*c2.g;
						c.b = alpha*c0.b + beta*c1.b + gamma*c2.b;

						img[j][i].r = round(c.r);
						img[j][i].g = round(c.g);
						img[j][i].b = round(c.b);

					}
				}


			}
		}
	}
}

bool visible(double den, double num, double& tE, double& tL) {
    double t;
	if (den > 0) { 
        t = num / den; 
		if(t>tL){
			return false;
		}
		if(t>tE){
			tE=t;
		}
	}
	else if (den < 0){
		t = num / den;
		if(t<tE){
			return false;
		}
		if(t>tL){
			tL=t;
		} 
	}
	else if(num>0){
		return false;
	}
	return true;
}



void clipping(Vec4& p0, Vec4& p1, Color &result_0, Color &result_1, Color p0_color, Color p1_color){

	double tE=0,tL=1;
	bool vis = false;
	Vec4 p0_v2, p1_v2;
	
	p0_v2.x = p0.x*p1.t;
	p0_v2.y = p0.y*p1.t;
	p0_v2.z = p0.z*p1.t;
	p0_v2.t = p0.t*p1.t;
	
	p1_v2.x = p1.x*p0.t;
	p1_v2.y = p1.y*p0.t;
	p1_v2.z = p1.z*p0.t;
	p1_v2.t = p1.t*p0.t;
	
	double new_t = p0.t*p1.t;

	double dx,dy,dz;
	dx = p1_v2.x-p0_v2.x;
	dy = p1_v2.y-p0_v2.y;
	dz = p1_v2.z-p0_v2.z;

	if(visible(dx,-new_t-p0_v2.x,tE,tL)){
		if(visible(-dx,p0_v2.x-new_t,tE,tL)){
			if(visible(dy,-new_t-p0_v2.y,tE,tL)){
				if(visible(-dy,p0_v2.y-new_t,tE,tL)){
					if(visible(dz,-new_t-p0_v2.z,tE,tL)){
						if(visible(-dz,p0_v2.z-new_t,tE,tL)){
							vis = true;

							if(tL < 1){
								
								p1.x = (p0_v2.x + tL*dx)/p0.t;
								p1.y = (p0_v2.y + tL*dy)/p0.t;
								p1.z = (p0_v2.z + tL*dz)/p0.t;

								result_1.r = (1-tL)*p0_color.r + tL*p1_color.r;
								result_1.g = (1-tL)*p0_color.g + tL*p1_color.g;
								result_1.b = (1-tL)*p0_color.b + tL*p1_color.b;
							}

							if(tE > 0){
								
								p0.x = (p0_v2.x + tE*dx)/p1.t;
								p0.y = (p0_v2.y + tE*dy)/p1.t;
								p0.z = (p0_v2.z + tE*dz)/p1.t;

								result_0.r = (1-tE)*p0_color.r + tE*p1_color.r;
								result_0.g = (1-tE)*p0_color.g + tE*p1_color.g;
								result_0.b = (1-tE)*p0_color.b + tE*p1_color.b;
							}
							
						}
					}
				}
			}
		}
	}

}


Vec3 calculate_normal(Vec4& p0,Vec4& p1, Vec4& p2){
	Vec3 p0_to_p1,p0_to_p2;
	
	p0_to_p1.x = p1.x-p0.x;
	p0_to_p1.y = p1.y-p0.y;
	p0_to_p1.z = p1.z-p0.z;
	
	p0_to_p2.x = p2.x-p0.x;
	p0_to_p2.y = p2.y-p0.y;
	p0_to_p2.z = p2.z-p0.z;

	return crossProductVec3(normalizeVec3(p0_to_p1),normalizeVec3(p0_to_p2));

}


void Scene::forwardRenderingPipeline(Camera *camera)
{
	// TODO: Implement this function

	double** depth_buffer = new double*[camera->horRes];
    for (int i = 0; i < camera->horRes; i++) {
        depth_buffer[i] = new double[camera->verRes];
        for (int j = 0; j < camera->verRes; j++) {
            depth_buffer[i][j] = INFINITY; 
        }
    }
	
	Matrix4 camera_transform = getIdentityMatrix();
	
	camera_transform.values[0][3]= -camera->position.x;
	camera_transform.values[1][3]= -camera->position.y;
	camera_transform.values[2][3]= -camera->position.z;

	Matrix4 identity = getIdentityMatrix();

	identity.values[0][0] = camera->u.x;
	identity.values[0][1] = camera->u.y;
	identity.values[0][2] = camera->u.z;

	identity.values[1][0] = camera->v.x;
	identity.values[1][1] = camera->v.y;
	identity.values[1][2] = camera->v.z;

	identity.values[2][0] = camera->w.x;
	identity.values[2][1] = camera->w.y;
	identity.values[2][2] = camera->w.z;

	camera_transform = multiplyMatrixWithMatrix(identity,camera_transform);


	Matrix4 viewing_part2 = getIdentityMatrix();

	if(camera->projectionType == ORTOGRAPHIC_PROJECTION){		
		Matrix4 orthographic_projection = getIdentityMatrix();
		orthographic_projection.values[0][0] = 2 / (camera->right - camera->left);
		orthographic_projection.values[1][1] = 2 / (camera->top - camera->bottom);
		orthographic_projection.values[2][2] = -2 / (camera->far - camera->near);

		orthographic_projection.values[0][3] = - (camera->right + camera->left) / (camera->right - camera->left);
		orthographic_projection.values[1][3] = - (camera->top + camera->bottom) / (camera->top - camera->bottom);
		orthographic_projection.values[2][3] = - (camera->far + camera->near) / (camera->far - camera->near);
	
		viewing_part2 = orthographic_projection;
	}

	else{
		
		viewing_part2.values[0][0] = 2*camera->near/ (camera->right - camera->left);
		viewing_part2.values[1][1] = 2*camera->near/ (camera->top - camera->bottom);
		viewing_part2.values[2][2] = - (camera->far + camera->near) / (camera->far - camera->near);
		viewing_part2.values[3][3] = 0;
		
		viewing_part2.values[0][2] =  (camera->right + camera->left) / (camera->right - camera->left);
		viewing_part2.values[1][2] =  (camera->top + camera->bottom) / (camera->top - camera->bottom);
		viewing_part2.values[2][3] = -2*(camera->near*camera->far)  / (camera->far - camera->near);

		
		viewing_part2.values[3][2] = - 1;

	}

	Matrix4 viewport = Matrix4();

	viewport.values[0][0] = camera->horRes/ 2;
	viewport.values[1][1] = camera->verRes/ 2;
	viewport.values[2][2] = 0.5;
	
	viewport.values[0][3] =  (camera->horRes-1 )/ 2;
	viewport.values[1][3] =  (camera->verRes-1) / 2;
	viewport.values[2][3] = 0.5;
	


	int mesh_size = meshes.size();
	int i,j;

	for(i=0 ; i<mesh_size ; i++){
		Mesh *mesh = meshes[i];
		int transformation_count = mesh->numberOfTransformations;
		
		Matrix4 modellingTransformations = getIdentityMatrix();
		
		for(j=0 ; j<transformation_count ; j++){
			

			Matrix4 identity=getIdentityMatrix();
			
			if (mesh->transformationTypes[j] == 't'){
				Translation *translation = translations[mesh->transformationIds[j] - 1];
				identity.values[0][3] = translation->tx;
				identity.values[1][3] = translation->ty;
				identity.values[2][3] = translation->tz;
				modellingTransformations = multiplyMatrixWithMatrix(identity, modellingTransformations);
			}

			else if (mesh->transformationTypes[j] == 's'){
				Scaling *scale = scalings[mesh->transformationIds[j] - 1];
				identity.values[0][0] = scale->sx;
				identity.values[1][1] = scale->sy;
				identity.values[2][2] = scale->sz;

				modellingTransformations = multiplyMatrixWithMatrix(identity, modellingTransformations);
			}
			
			else{
				Rotation *rotation = rotations[mesh->transformationIds[j] - 1];
				Vec3 *u, *v, *w;
				
				double smallest = rotation->ux < rotation->uy ? rotation->ux : rotation->uy;
				smallest = smallest < rotation->uz ? smallest : rotation->uz;
				
				u = new Vec3(rotation->ux, rotation->uy, rotation->uz);
				
				if(smallest == rotation->ux){
					if(rotation->uy == 0){
						v = new Vec3(0, -rotation->uz, rotation->uy);	
					}
					else{
						v = new Vec3(0, rotation->uz, -rotation->uy);
					}
					
				}
				else if(smallest == rotation->uy){

					if(rotation->ux==0){
						v = new Vec3(-rotation->uz, 0, rotation->ux);
					}

					else{
						v = new Vec3(rotation->uz, 0, -rotation->ux);
					}
					
				}
				else{
					
					if(rotation->ux==0){
						v = new Vec3(-rotation->uy, rotation->ux, 0);
					}

					else{
						v = new Vec3(rotation->uy, -rotation->ux, 0);
					}	
					
				}

				normalizeVec3(*u);
				
				w = new Vec3();
				
				*w = crossProductVec3(*u, *v);

				normalizeVec3(*v);
				normalizeVec3(*w);


				identity.values[0][0] = u->x;
				identity.values[0][1] = u->y;
				identity.values[0][2] = u->z;

				identity.values[1][0] = v->x;
				identity.values[1][1] = v->y;
				identity.values[1][2] = v->z;

				identity.values[2][0] = w->x;
				identity.values[2][1] = w->y;
				identity.values[2][2] = w->z;

				Matrix4 transpose_m = identity;
				
				std::swap(transpose_m.values[0][1], transpose_m.values[1][0]);
				std::swap(transpose_m.values[0][2], transpose_m.values[2][0]);
				std::swap(transpose_m.values[0][3], transpose_m.values[3][0]);
				std::swap(transpose_m.values[1][2], transpose_m.values[2][1]);
				std::swap(transpose_m.values[1][3], transpose_m.values[3][1]);
				std::swap(transpose_m.values[2][3], transpose_m.values[3][2]);
		
				Matrix4 rotationAroundX = getIdentityMatrix();
				rotationAroundX.values[1][1] = cos(rotation->angle*M_PI / 180);
				rotationAroundX.values[1][2] = -sin(rotation->angle*M_PI / 180);
				
				rotationAroundX.values[2][1] = sin(rotation->angle*M_PI / 180);
				rotationAroundX.values[2][2] = cos(rotation->angle*M_PI / 180);

				identity = multiplyMatrixWithMatrix(rotationAroundX, identity);
				identity = multiplyMatrixWithMatrix(transpose_m, identity);
				modellingTransformations = multiplyMatrixWithMatrix(identity, modellingTransformations);
			}
			
		}

		Matrix4 before_pers_div = multiplyMatrixWithMatrix(viewing_part2,multiplyMatrixWithMatrix(camera_transform,modellingTransformations));
		
		int vertex_size = vertices.size();
		
		std::vector<double> result_of_nv;

		int face_count = mesh->triangles.size();


		Vec4 temp;
		std::vector<Vec4> vertices_before_clipping;
		std::vector<Color*> color_before_clipping;

		color_before_clipping = colorsOfVertices;

		for(int i = 0; i<vertex_size ; i++){	
			temp.x = vertices[i]->x;
			temp.y = vertices[i]->y;
			temp.z = vertices[i]->z;
			temp.t = 1;
			temp.colorId = vertices[i]->colorId;
			temp = multiplyMatrixWithVec4(before_pers_div,temp);
			vertices_before_clipping.push_back(temp);
		}

		
		for (int i = 0 ; i<face_count; i++){
			int vertexId1 = mesh->triangles[i].vertexIds[0]-1;
			int vertexId2 = mesh->triangles[i].vertexIds[1]-1;
			int vertexId3 = mesh->triangles[i].vertexIds[2]-1;

			Vec4 p1 = vertices_before_clipping[vertexId1];
			Vec4 p2 = vertices_before_clipping[vertexId2];
			Vec4 p3 = vertices_before_clipping[vertexId3];
			Color temp1, temp2, temp3;
			
			temp1 = *colorsOfVertices[vertexId1];
			temp2 = *colorsOfVertices[vertexId2];
			temp3 = *colorsOfVertices[vertexId3];
			Vec3 v;
			double res;

			v.x = (p1.x+p2.x+p3.x)/3;
			v.y = (p1.y+p2.y+p3.y)/3;
			v.z = (p1.z+p2.z+p3.z)/3;
			res = dotProductVec3(calculate_normal(p1,p2,p3), v);
			if(res < 0 && cullingEnabled){
				continue;
			}
			
			if(mesh->type == WIREFRAME_MESH){
				
				clipping(p1,p2,temp1,temp2,*colorsOfVertices[vertexId1],*colorsOfVertices[vertexId2]);

				p1.x = p1.x/p1.t;
				p1.y = p1.y/p1.t;
				p1.z = p1.z/p1.t;
				p1.t = 1;

				p2.x = p2.x/p2.t;
				p2.y = p2.y/p2.t;
				p2.z = p2.z/p2.t;
				p2.t = 1;

				p1 = multiplyMatrixWithVec4(viewport,p1);
				p2 = multiplyMatrixWithVec4(viewport,p2);

				draw_line(p1,p2,temp1,temp2,image,camera->horRes, camera->verRes,depth_buffer);

				p1 = vertices_before_clipping[vertexId1];
				temp1 = *colorsOfVertices[vertexId1];
				
				clipping(p1,p3,temp1,temp3,*colorsOfVertices[vertexId1],*colorsOfVertices[vertexId3]);

				p1.x = p1.x/p1.t;
				p1.y = p1.y/p1.t;
				p1.z = p1.z/p1.t;
				p1.t = 1;

				p3.x = p3.x/p3.t;
				p3.y = p3.y/p3.t;
				p3.z = p3.z/p3.t;
				p3.t = 1;

				p1 = multiplyMatrixWithVec4(viewport,p1);
				p3 = multiplyMatrixWithVec4(viewport,p3);		

				draw_line(p1,p3,temp1,temp3,image,camera->horRes, camera->verRes,depth_buffer);	

				p2 = vertices_before_clipping[vertexId2];
				p3 = vertices_before_clipping[vertexId3];
				temp2 = *colorsOfVertices[vertexId2];
				temp3 = *colorsOfVertices[vertexId3];

				clipping(p2,p3,temp2,temp3,*colorsOfVertices[vertexId2],*colorsOfVertices[vertexId3]);

				p2.x = p2.x/p2.t;
				p2.y = p2.y/p2.t;
				p2.z = p2.z/p2.t;
				p2.t = 1;

				p3.x = p3.x/p3.t;
				p3.y = p3.y/p3.t;
				p3.z = p3.z/p3.t;
				p3.t = 1;

				p2 = multiplyMatrixWithVec4(viewport,p2);
				p3 = multiplyMatrixWithVec4(viewport,p3);

				draw_line(p2,p3,temp2,temp3,image,camera->horRes, camera->verRes,depth_buffer);
			}

			else{
				
				p1.x = p1.x/p1.t;
				p1.y = p1.y/p1.t;
				p1.z = p1.z/p1.t;
				p1.t = 1;

				p2.x = p2.x/p2.t;
				p2.y = p2.y/p2.t;
				p2.z = p2.z/p2.t;
				p2.t = 1;

				p3.x = p3.x/p3.t;
				p3.y = p3.y/p3.t;
				p3.z = p3.z/p3.t;
				p3.t = 1;

				p1 = multiplyMatrixWithVec4(viewport,p1);
				p2 = multiplyMatrixWithVec4(viewport,p2);
				p3 = multiplyMatrixWithVec4(viewport,p3);

				rasterization(p1, p2, p3, vertexId1, vertexId2, vertexId3, colorsOfVertices, image, depth_buffer, camera->horRes, camera->verRes);
		
			}
		
		}

	}

}
