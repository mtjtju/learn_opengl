#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
	unsigned int ID;
	Shader(const char* vertex_path, const char* fragment_path);
	void use();
	void set_bool(const std::string& name, bool value) const;
	void set_int(const std::string& name, int value) const;
	void set_float(const std::string& name, float value) const;
	void set_4f(const std::string& name, float f0, float f1, float f2, float f3) const;
	void set_vec3(const std::string& name, glm::vec3 f) const;
	void set_mat4(const std::string& name, const glm::mat4& mat) const;
};
