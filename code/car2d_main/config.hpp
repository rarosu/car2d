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
const std::string FILE_CONFIG = "config.yaml";
const std::string FILE_MESH2D_VS = "plain_2d.vert";
const std::string FILE_MESH2D_FS = "plain_2d.frag";

const int UNIFORM_FRAME_BINDING = 1;
const int UNIFORM_INSTANCE_BINDING = 2;

struct PerFrame
{
	glm::mat3x4 view_matrix;
	glm::mat3x4 projection_matrix;
};

struct PerInstance
{
	glm::mat3x4 model_matrix;
};