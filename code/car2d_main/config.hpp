#pragma once

#include <string>
#include <glm/glm.hpp>

const float DT = 1.0f / 60.0f;
const int OPENGL_VERSION_MAJOR = 4;
const int OPENGL_VERSION_MINOR = 4;
const std::string PROJECT_ROOT = "../../../";
const std::string DIRECTORY_ASSETS = PROJECT_ROOT + "assets/";
const std::string DIRECTORY_CARS = DIRECTORY_ASSETS + "cars/";
const std::string DIRECTORY_SHADERS = DIRECTORY_ASSETS + "shaders/";
const std::string DIRECTORY_MAPS = DIRECTORY_ASSETS + "maps/";
const std::string DIRECTORY_TEXTURES = DIRECTORY_ASSETS + "textures/";
const std::string FILE_CONFIG = "config.yaml";
const std::string FILE_PLAIN2D_VS = "plain_2d.vert";
const std::string FILE_PLAIN2D_FS = "plain_2d.frag";
const std::string FILE_MESH2D_VS = "mesh_2d.vert";
const std::string FILE_MESH2D_FS = "mesh_2d.frag";
const std::string FILE_TERRAIN_VS = "terrain.vert";
const std::string FILE_TERRAIN_FS = "terrain.frag";

const int UNIFORM_FRAME_BINDING = 1;
const int UNIFORM_INSTANCE_BINDING = 2;

const int TEXTURE_DIFFUSE_BINDING = 0;

struct PerFrame
{
	glm::mat3x4 view_matrix;
	glm::mat3x4 projection_matrix;
};

struct PerInstance
{
	glm::mat3x4 model_matrix;
};