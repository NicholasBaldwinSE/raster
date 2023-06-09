// Local includes.
#include "RayshadeParser.hpp"
#include "World.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Config.hpp"

// Global includes
#include <string>
#include <iostream>

/*
 * COLOR CORRECTION
 * Color struct and associated types used from
 * the trace project, mostly to make file writing simpler.
*/
template <typename T>
struct Color {
	typedef T value_type;

	T r, g, b;
};

typedef Color<unsigned char> uColor;
typedef Color<float>         fColor;

void printVec(glm::vec3 vec) {
    std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl;
}

float clamp(float d) {
    if (d < 0) {
        return 0.f;
    } else if (d > 1) {
        return 1.f;
    } else {
        return d;
    }
}

uColor colorCorrect(glm::vec3 color) {
    return {
        // Conversion  :     Clamp         : Gamma    : Numerical conversion
        (unsigned char) (pow(clamp(color.x), 1 / 2.2) * 255.f),
        (unsigned char) (pow(clamp(color.y), 1 / 2.2) * 255.f), 
        (unsigned char) (pow(clamp(color.z), 1 / 2.2) * 255.f)
    };
}

/*
 * BARYCENTRICS
 * Cramer's Rule.
*/
void calcBary(glm::vec2 point, glm::vec2 vert1, glm::vec2 vert2, glm::vec2 vert3, glm::vec3 *bary) {
    glm::vec2 v0, v1, v2;
    v0 = vert2 - vert1;
    v1 = vert3 - vert1;
    v2 = point - vert1;

    float denom = v0.x * v1.y - v1.x * v0.y;

    float a, b, c;
    a = (v2.x * v1.y - v1.x * v2.y) / denom;
    b = (v0.x * v2.y - v2.x * v0.y) / denom;
    c = 1.0f - a - b;

    bary->x = c;
    bary->y = a;
    bary->z = b;
}

/*
 * MAIN FUNCTION
*/
int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cerr << "Need file argument." << std::endl;
        return -1;
    }

    World* world = parseFile(std::string(PROJECT_DATA_DIR) + argv[1]);

    if (!world) {
        std::cerr << "Failed to create world object. Aborting." << std::endl;
        // Change this to be a proper error at some point.
        return -1;
    }

    std::cout << world->polygons.size() << std::endl;
    std::cout << world->triangles.size() << std::endl;

    // Allocate memory.
    int pixels = (int) (world->screen.x * world->screen.y);
    std::vector<uColor> screenBuff(pixels, colorCorrect(world->background));
    std::vector<float> zBuff(pixels, INFINITY);

    // Preprocessing. Turn the polygons into triangles.
    for (Polygon polygon : world->polygons) {
        glm::vec3 a, b, c, d;

        // Get verts
        a = polygon.verts[0];
        b = polygon.verts[1];
        c = polygon.verts[2];
        d = polygon.verts[3];

        // Construct triangles
        Triangle tri1;
        Triangle tri2;

        tri1.surface = polygon.surface;
        tri2.surface = polygon.surface;

        tri1.verts[0] = a;
        tri1.verts[1] = b;
        tri1.verts[2] = c;

        tri2.verts[0] = a;
        tri2.verts[1] = c;
        tri2.verts[2] = d;

        // Calculate triangle normals
        glm::vec3 u1, v1, u2, v2;
        u1 = tri1.verts[1] - tri1.verts[0];
        v1 = tri1.verts[2] - tri1.verts[0];
        u2 = tri2.verts[1] - tri2.verts[0];
        v2 = tri2.verts[2] - tri2.verts[0];

        // We use temp norms since all verts will share the same norms
        glm::vec3 tempnorm1, tempnorm2;

        tempnorm1 = glm::normalize(glm::cross(u1, v1));
        tempnorm2 = glm::normalize(glm::cross(u2, v2));

        // Set normals.
        tri1.norms[0] = tempnorm1;
        tri1.norms[1] = tempnorm1;
        tri1.norms[2] = tempnorm1;

        tri2.norms[0] = tempnorm2;
        tri2.norms[1] = tempnorm2;
        tri2.norms[2] = tempnorm2;

        // Push them to the triangle list.
        world->triangles.push_back(tri1);
        world->triangles.push_back(tri2);
    }

    glm::mat4 view = glm::lookAt(world->eyep, world->lookp, world->up);
    glm::mat4 projection = glm::perspective(glm::radians(world->fov.y), (float) world->screen.x / (float) world->screen.y, 0.1f, 100.f);
    glm::mat4 raster = {
        {world->screen.x / 2, 0, 0, 0},
        {0, -world->screen.y / 2, 0, 0},
        {0, 0, .5f, 0},
        {world->screen.x / 2, world->screen.y / 2, .5f, 1},
    };

    // Barycentric Rasterization
    for (auto triangle : world->triangles) {
        // World -> Screen
        glm::vec4 vert1 = raster * projection * view * glm::vec4(triangle.verts[0], 1.f);
        glm::vec4 vert2 = raster * projection * view * glm::vec4(triangle.verts[1], 1.f);
        glm::vec4 vert3 = raster * projection * view * glm::vec4(triangle.verts[2], 1.f);
        vert1 /= vert1.w;
        vert2 /= vert2.w;
        vert3 /= vert3.w;

        // Get bounds.
        float xmin = glm::max(0.f, glm::min(vert1.x, glm::min(vert2.x, vert3.x)));
        float xmax = glm::min(world->screen.x, glm::max(vert1.x, glm::max(vert2.x, vert3.x)));
        float ymin = glm::max(0.f, glm::min(vert1.y, glm::min(vert2.y, vert3.y)));
        float ymax = glm::min(world->screen.y, glm::max(vert1.y, glm::max(vert2.y, vert3.y)));

        for (int y = ymin; y < ymax; y++) {
            for (int x = xmin; x < xmax; x++) {
                // Calc bary.
                glm::vec3 bary;
                calcBary({(float) x, (float) y}, {vert1.x, vert1.y}, {vert2.x, vert2.y}, {vert3.x, vert3.y}, &bary);

                // Rasterize
                if (bary[0] >= 0.f && bary[1] >= 0.f && bary[2] >= 0.f) {
                    int index = x + (y * (int) world->screen.x);
                    float z = (vert1.z * bary[0]) + (vert2.z * bary[1]) + (vert3.z * bary[2]);

                    if (z < zBuff[index]) {
                        zBuff[index] = z;
                        glm::vec3 color(0.f, 0.f, 0.f);

                        glm::vec3 worldTrianglePos 
                            = triangle.verts[0] * bary[0]
                            + triangle.verts[1] * bary[1]
                            + triangle.verts[2] * bary[2];
                        glm::vec3 triangleNorm
                            = triangle.norms[0] * bary[0]
                            + triangle.norms[1] * bary[1]
                            + triangle.norms[2] * bary[2];

                        glm::vec3 v = glm::normalize(world->eyep - worldTrianglePos);

                        // Blinn-Phong lighting
                        for (auto light : world->lights) {
                            color += triangle.surface.ambient * light.intensity;
                            
                            glm::vec3 lightVec = glm::normalize(light.pos - worldTrianglePos);

                            if (glm::dot(triangleNorm, lightVec) > 0) {
                                float intensity = light.intensity * glm::dot(triangleNorm, lightVec);
                                color += triangle.surface.diffuse * (float) intensity;

                                glm::vec3 H = glm::normalize(lightVec + v);
                                
                                if (glm::dot(triangleNorm, H) > 0) {
                                    color += triangle.surface.specular 
                                        * ((float) intensity 
                                        * (glm::pow(glm::dot(triangleNorm, H), triangle.surface.specpow)));
                                }
                            }
                        }

                        screenBuff[index] = colorCorrect(color);
                    }
                }
            }
        }
    }

    // Writing to PPM.
    std::fstream ppmfile(std::string(PROJECT_BUILD_DIR) + "raster.ppm", std::ios::out | std::ios::binary);

	if (ppmfile.fail()) {
		std::cerr << "Error opening" << std::endl;
		return -1;
	}

	ppmfile << "P6" << std::endl;
	ppmfile << world->screen.x << " " << world->screen.y << std::endl;
	ppmfile << "255" << std::endl;

	ppmfile.write((char*) (&screenBuff[0]), world->screen.x * world->screen.y * sizeof(uColor));

    return 0;
}