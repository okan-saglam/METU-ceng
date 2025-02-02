#include <iostream>
#include "parser.h"
#include "ppm.h"
#include <cmath>

typedef struct{
    parser::Vec3f a;
    parser::Vec3f b;
} Ray;

typedef struct{
    parser::Vec3f u_vector;
    parser::Vec3f v_vector;
    parser::Vec3f w_vector;
    parser::Vec3f m_in_plane;
    parser::Vec3f q_corner_plane;
} CameraCalculation;

parser::Vec3f normalizeVector(parser::Vec3f vec){
    parser::Vec3f result;
    float magnitude = std::sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
    if(magnitude == 0) return vec; // Already normalized
    result.x = vec.x / magnitude;
    result.y = vec.y / magnitude;
    result.z = vec.z / magnitude;
    return result;
}

parser::Vec3f addVectors(parser::Vec3f vec1, parser::Vec3f vec2){
    parser::Vec3f result;
    result.x = vec1.x + vec2.x;
    result.y = vec1.y + vec2.y;
    result.z = vec1.z + vec2.z;
    return result;
}

parser::Vec3f scalarMultiplication(parser::Vec3f vec, float scalar){
    parser::Vec3f result;
    result.x = vec.x*scalar;
    result.y = vec.y*scalar;
    result.z = vec.z*scalar;
    return result;
}

parser::Vec3f crossProduct(parser::Vec3f vec1, parser::Vec3f vec2){
    parser::Vec3f result;
    result.x = vec1.y*vec2.z - vec1.z*vec2.y;
    result.y = vec1.z*vec2.x - vec1.x*vec2.z;
    result.z = vec1.x*vec2.y - vec1.y*vec2.x;
    return result;
}

float dotProduct(parser::Vec3f vec1, parser::Vec3f vec2){
    float result;
    result = vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z;
    return result;
}

std::vector<CameraCalculation> calculateCameras(std::vector<parser::Camera> cameras, std::vector<CameraCalculation> &result){
    int size = cameras.size();
    int i;
    for(i=0 ; i<size ; i++){
        CameraCalculation camera;
        camera.v_vector = cameras[i].up;
        camera.w_vector = scalarMultiplication(cameras[i].gaze, -1);
        camera.u_vector = crossProduct(camera.v_vector, camera.w_vector);
        camera.m_in_plane = addVectors(cameras[i].position, scalarMultiplication(cameras[i].gaze, cameras[i].near_distance));
        camera.q_corner_plane = addVectors(camera.m_in_plane, 
                                addVectors( scalarMultiplication(camera.u_vector, cameras[i].near_plane.x), 
                                            scalarMultiplication(camera.v_vector, cameras[i].near_plane.w)));
        result.push_back(camera);
    }
    return result;
}


Ray generateRay(int i , int j, parser::Camera camera, CameraCalculation camera_calculation){
    double pixel_height, pixel_width;
    double s_u, s_v;
    Ray result;

    float r = camera.near_plane.y;
    float l = camera.near_plane.x;
    float t = camera.near_plane.w;
    float b = camera.near_plane.z;

    pixel_height = (t-b) / camera.image_height;
    pixel_width = (r-l) / camera.image_width;

    s_u = (i+0.5)*pixel_width;
    s_v = (j+0.5)*pixel_height;

    parser::Vec3f s =   addVectors(camera_calculation.q_corner_plane,
                        addVectors( scalarMultiplication(camera_calculation.u_vector, s_u),
                                    scalarMultiplication(camera_calculation.v_vector, -s_v)));
    parser::Vec3f d = normalizeVector(addVectors(s, scalarMultiplication(camera.position, -1)));
    
    result.a = camera.position;
    result.b = d;
    return result;
}

double intersectSphere(Ray ray, parser::Sphere sphere, parser::Vec3f center){
    parser::Vec3f o_minus_c = addVectors(ray.a, scalarMultiplication(center, -1));
    double A = dotProduct(ray.b, ray.b);
    double B = dotProduct(scalarMultiplication(ray.b, 2), o_minus_c);
    double C = dotProduct(o_minus_c, o_minus_c) - sphere.radius*sphere.radius;
    double discriminant = B*B - 4*A*C;
    double t;
    if(discriminant < 0){
        return -1;
    }
    else if(discriminant == 0){
        t = -B/(2*A);
    }
    else{
        float t1 = (-B + std::sqrt(discriminant))/(2*A);
        float t2 = (-B - std::sqrt(discriminant))/(2*A);
        if (t1>t2){
            t = t2;
        }
        else{
            t = t1;
        }
    }
    return t;

}

double intersectTriangle(Ray &ray, parser::Face &face, std::vector<parser::Vec3f> &vertex_data){
    double a,b,c,d,e,f,g,h,i,j,k,l;
    double beta, gamma, t;
    double A;

    parser::Vec3f ma, mb, mc;
    ma = vertex_data[face.v0_id -1];
    mb = vertex_data[face.v1_id -1];
    mc = vertex_data[face.v2_id -1];

    a = ma.x - mb.x;
    b = ma.y - mb.y;
    c = ma.z - mb.z;

    d = ma.x - mc.x;
    e = ma.y - mc.y;
    f = ma.z - mc.z;

    g = ray.b.x;
    h = ray.b.y;
    i = ray.b.z;

    j = ma.x - ray.a.x;
    k = ma.y - ray.a.y;
    l = ma.z - ray.a.z;

    A = a * (e*i - f*h) - d * (b*i - c*h) + g * (b*f - c*e);

    beta = (j * (e*i - f*h) - d * (k*i - l*h) + g * (k*f - e*l)) / A;

    gamma = (a * (k*i - l*h) - j * (b*i - c*h) + g * (b*l - c*k)) / A;

    t = (a * (e*l - f*k) - d * (b*l - c*k) + j * (b*f - c*e)) / A;

    if(beta+gamma <= 1 && 0<=beta && 0<=gamma && 0 < t) return t;
    else return -1;

}

float findDistanceSquare(parser::Vec3f vec1, parser::Vec3f vec2){
    float result;
    result = std::pow((vec1.x-vec2.x), 2) + std::pow((vec1.y-vec2.y),2) + std::pow((vec1.z-vec2.z),2);
    return result;
}

void faceNormal(parser::Face face, std::vector<parser::Vec3f> vertex_data, parser::Vec3f &normal){
    parser::Vec3f minus_a, b, c;
    parser::Vec3f b_to_a, c_to_a;

    minus_a = scalarMultiplication(vertex_data[face.v0_id - 1], -1);
    b = vertex_data[face.v1_id - 1];
    c = vertex_data[face.v2_id - 1];
    
    b_to_a = addVectors(b, minus_a);
    c_to_a = addVectors(c, minus_a);

    normal = normalizeVector(crossProduct(b_to_a, c_to_a)); 
}

parser::Vec3f totalColor(parser::Vec3f clr){
    parser::Vec3f result;
    int x = (int)(clr.x + 0.5);
    if( x > 255){
        result.x = 255;
    }
    else if(x < 0){
        result.x = 0;
    }
    else{
        result.x = x;
    }
    int y = (int)(clr.y + 0.5);
    if(y > 255){
        result.y = 255;
    }
    else if(y < 0){
        result.y = 0;
    }
    else{
        result.y = y;
    }
    int z = (int)(clr.z + 0.5);
    if( z> 255){
        result.z = 255;
    }
    else if(z < 0){
        result.z = 0;
    }
    else{
        result.z = z;
    }
    return result;
}


void computeIntersection(parser::Scene &scene, int &sphere_no, int &triangle_no, int &mesh_no, int &face_of_mesh_no, double &Tmin, double &t, Ray &ray, parser::Material &material, parser::Vec3f &point_on_objects, parser::Vec3f &normal){
    
    int number_of_spheres = scene.spheres.size();
    for(int k=0 ; k<number_of_spheres ; k++){
        parser::Sphere sphere = scene.spheres[k];

        t = intersectSphere(ray, sphere, scene.vertex_data[sphere.center_vertex_id - 1]);

        if(t >= 0){ // Reflection 1 vs 0
            if(t < Tmin){
                Tmin = t;
                sphere_no = k;
                material = scene.materials[scene.spheres[sphere_no].material_id - 1];
                point_on_objects = addVectors(ray.a, scalarMultiplication(ray.b, Tmin));
                normal = addVectors(point_on_objects, 
                    scalarMultiplication(scene.vertex_data[scene.spheres[sphere_no].center_vertex_id - 1], -1));
                normal = normalizeVector(normal);
            }
        }
    }

    int number_of_triangles = scene.triangles.size(); 
    for(int k=0 ; k<number_of_triangles ; k++){
        parser::Triangle triangle = scene.triangles[k];

        t = intersectTriangle(ray, triangle.indices, scene.vertex_data);

        if(t >= 0){
            if(t < Tmin){
                Tmin = t;
                triangle_no = k;
                material = scene.materials[scene.triangles[triangle_no].material_id - 1];
                point_on_objects = addVectors(ray.a, scalarMultiplication(ray.b, Tmin));
                faceNormal(scene.triangles[triangle_no].indices, scene.vertex_data, normal);
            }
        }
    }

    int number_of_meshes = scene.meshes.size();
    for(int k=0 ; k<number_of_meshes ; k++){
        int number_of_faces_of_meshes = scene.meshes[k].faces.size();
        for(int l=0 ; l<number_of_faces_of_meshes ; l++){
            t = intersectTriangle(ray, scene.meshes[k].faces[l], scene.vertex_data);
            if(t >= 0){
                if(t < Tmin){
                    Tmin = t;
                    mesh_no = k;
                    face_of_mesh_no = l;
                    material = scene.materials[scene.meshes[mesh_no].material_id - 1];
                    point_on_objects = addVectors(ray.a, scalarMultiplication(ray.b, Tmin));
                    faceNormal(scene.meshes[mesh_no].faces[face_of_mesh_no], scene.vertex_data, normal);
                }
            }
        }
    }
}

parser::Vec3f pixelColor(parser::Scene &scene, Ray &ray, parser::Vec3f &color, int &rec_count){
    
    parser::Vec3f ambient_part, diffuse_part, specular_part, reflection_part ;
              

    parser::Vec3f diffuse_part_specific, specular_part_specific;
    
    ambient_part.x = 0; ambient_part.y = 0; ambient_part.z = 0;
    diffuse_part.x = 0; diffuse_part.y = 0; diffuse_part.z = 0;
    specular_part.x = 0; specular_part.y = 0; specular_part.z = 0;
    reflection_part.x = 0; reflection_part.y = 0;  reflection_part.z = 0; 

    double Tmin = 44444;
    double t = -1;
    int sphere_no = -1;
    int triangle_no = -1;
    int mesh_no = -1;
    int face_of_mesh_no = -1;

    if (rec_count == 0){
        color.x = scene.background_color.x;
        color.y = scene.background_color.y;
        color.z = scene.background_color.z;
    }

    else{
        color.x = 0;
        color.y = 0;
        color.z = 0;
    }

    parser::Vec3f point_on_objects, normal;
    parser::Material material;

    computeIntersection(scene, sphere_no, triangle_no, mesh_no, face_of_mesh_no,
                        Tmin, t, ray,
                        material, point_on_objects, normal);

    if(sphere_no == -1 && triangle_no == -1 && mesh_no == -1){
        return color;
    }
    else{
        ambient_part.x = scene.ambient_light.x * material.ambient.x;
        ambient_part.y = scene.ambient_light.y * material.ambient.y;
        ambient_part.z = scene.ambient_light.z * material.ambient.z;
    }

    Ray reflection_ray;
    

    if(material.is_mirror && rec_count < scene.max_recursion_depth){
        reflection_ray.b = normalizeVector(addVectors(ray.b,scalarMultiplication(normal,2*dotProduct(normal,scalarMultiplication(ray.b,-1)))));
        reflection_ray.a = addVectors(point_on_objects,scalarMultiplication(normal,scene.shadow_ray_epsilon));
        rec_count++;
        reflection_part = pixelColor(scene, reflection_ray, color, rec_count);
        
        reflection_part.x *= material.mirror.x;
        reflection_part.y *= material.mirror.y;
        reflection_part.z *= material.mirror.z;
    }



    int number_of_point_lights = scene.point_lights.size();
    
    float cos_N_L, cos_H_N, distance_square, distance;
    
    parser::Vec3f light_position, to_light_vector;
    
    for(int k=0 ; k<number_of_point_lights ; k++){
        
        diffuse_part_specific.x = 0; diffuse_part_specific.y = 0; diffuse_part_specific.z = 0;
        specular_part_specific.x = 0; specular_part_specific.y = 0; specular_part_specific.z = 0;

        light_position = scene.point_lights[k].position;
        to_light_vector = normalizeVector(  addVectors(light_position, 
                                            scalarMultiplication(point_on_objects, -1)));
        
        cos_N_L = dotProduct(normal, to_light_vector);
        distance_square = findDistanceSquare(point_on_objects, light_position);
        distance = std::sqrt(distance_square);

        bool point_in_shadow = false;
        Ray shadow_ray;
        shadow_ray.a = addVectors(point_on_objects,scalarMultiplication(to_light_vector,scene.shadow_ray_epsilon));
        shadow_ray.b = normalizeVector(to_light_vector);
        
        int number_of_spheres = scene.spheres.size();
        for(int l=0 ; l<number_of_spheres && !point_in_shadow ; l++){
            float t2;
            parser::Sphere sphere = scene.spheres[l];
            t2 = intersectSphere(shadow_ray, sphere, scene.vertex_data[sphere.center_vertex_id - 1]);
            
            if(scene.shadow_ray_epsilon < t2 && t2 < distance){
                point_in_shadow = true;
            }
        }

        int number_of_triangles = scene.triangles.size();
        if(!point_in_shadow){
            for(int l=0 ; l<number_of_triangles ; l++){
                float t2;
                parser::Triangle triangle = scene.triangles[l];

                t2 = intersectTriangle(shadow_ray, triangle.indices, scene.vertex_data);

                if(scene.shadow_ray_epsilon < t2 && t2 < distance){
                    point_in_shadow = true;
                    break;
                }
            }
        }

        int number_of_meshes = scene.meshes.size();
        if(!point_in_shadow){
            for(int k=0 ; k<number_of_meshes ; k++){
                int number_of_faces_of_meshes = scene.meshes[k].faces.size();
                for(int l=0 ; l<number_of_faces_of_meshes ; l++){
                    double t2;
                    t2 = intersectTriangle(shadow_ray, scene.meshes[k].faces[l], scene.vertex_data);
                    
                    if(scene.shadow_ray_epsilon < t2 && t2 < distance){
                        point_in_shadow = true;
                        break;
                    }
                }
                if(point_in_shadow) break;
            }
        }
        
        parser::Vec3f half_vector;
        
        if(!point_in_shadow){
            if(0<=cos_N_L && cos_N_L<=1){
                parser::Vec3f diffuse = scalarMultiplication(scalarMultiplication(scene.point_lights[k].intensity, 1/distance_square), cos_N_L);
                diffuse_part_specific.x = diffuse.x * material.diffuse.x;
                diffuse_part_specific.y = diffuse.y * material.diffuse.y;
                diffuse_part_specific.z = diffuse.z * material.diffuse.z;
            }
            half_vector = normalizeVector(addVectors(to_light_vector, scalarMultiplication(ray.b, -1)));
            cos_H_N = dotProduct(half_vector, normal);
            if(0<=cos_H_N && cos_H_N<=1){
                float constant = std::pow(cos_H_N, material.phong_exponent);
                parser::Vec3f specular = scalarMultiplication(scene.point_lights[k].intensity, constant / distance_square);
                specular_part_specific.x = specular.x * material.specular.x;
                specular_part_specific.y = specular.y * material.specular.y;
                specular_part_specific.z = specular.z * material.specular.z;
            }
        }
        diffuse_part = addVectors(diffuse_part, diffuse_part_specific);
        specular_part = addVectors(specular_part, specular_part_specific);
    }

    if(rec_count != 0){
        color = addVectors(addVectors(ambient_part, diffuse_part), addVectors(specular_part, reflection_part));
    }
    else{
        color = addVectors(addVectors(ambient_part, diffuse_part), specular_part);
    }


    return color;
}


int main(int argc, char* argv[])
{
    // Sample usage for reading an XML scene file
    parser::Scene scene;

    scene.loadFromXml(argv[1]);

    int i,j,c;

    std::vector<CameraCalculation> camera_calculations;
 
    calculateCameras(scene.cameras, camera_calculations);
    

    for(c=0; c <scene.cameras.size(); c++){
        
        int image_height = scene.cameras[c].image_height;
        int image_width = scene.cameras[c].image_width;

        unsigned char* image = new unsigned char [image_width * image_height * 3];
        
        parser::Vec3f color;
       

        int a = 0;

        for(i=0 ; i<image_height ; i++){
            for(j=0 ; j<image_width ; j++){
                 
                
                Ray ray = generateRay(j,i,scene.cameras[c],camera_calculations[c]);
                
                int rec_count = 0;

                color = pixelColor(scene, ray, color,rec_count);

                color = totalColor(color);

                image[a++] = color.x;
                image[a++] = color.y;
                image[a++] = color.z;
            }
        }
        
        const char* image_name = scene.cameras[c].image_name.c_str();
        write_ppm(image_name, image, image_width, image_height);
    }

    
    
    
    return 0;


}
