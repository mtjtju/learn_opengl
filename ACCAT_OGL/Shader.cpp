#pragma once

#include "Shader.h"

using std::cout;
using std::endl;

/*
ifstream---stringstream---string---char*
ifstream.exception(failbit | badbit)
ifstream.open(const char*)
stringstream << ifstream.rdbuf()
ifstream.close()
stringstream.str() returns string
string.c_str() returns char*
somestuff;
*/
Shader::Shader(const char* vertex_path, const char* fragment_path)
{
	std::ifstream vshader_ifstream, fshader_ifstream;
	vshader_ifstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fshader_ifstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	vshader_ifstream.open(vertex_path);
	fshader_ifstream.open(fragment_path);

	std::stringstream vshader_sstream, fshader_sstream;
	vshader_sstream << vshader_ifstream.rdbuf();
	fshader_sstream << fshader_ifstream.rdbuf();
	vshader_ifstream.close();
	fshader_ifstream.close();

	std::string vshader_string, fshader_string;
	vshader_string = vshader_sstream.str();
	fshader_string = fshader_sstream.str();

	const char* vshader_cstr, * fshader_cstr;
	vshader_cstr = vshader_string.c_str();
	fshader_cstr = fshader_string.c_str();

	// 生成program
	unsigned int vshader_id, fshader_id;
	vshader_id = glCreateShader(GL_VERTEX_SHADER);
	fshader_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vshader_id, 1, &vshader_cstr, NULL);
	glShaderSource(fshader_id, 1, &fshader_cstr, NULL);
	glCompileShader(vshader_id);
	glCompileShader(fshader_id);

	// shader program obj 着色器程序对象
	ID = glCreateProgram();
	glAttachShader(ID, vshader_id);
	glAttachShader(ID, fshader_id);
	glLinkProgram(ID);

	// 处理编译错误
	int success;
	char infoLog[512];
	glGetShaderiv(vshader_id, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader_id, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << endl;
		cout << infoLog << endl;
	}
	glGetShaderiv(fshader_id, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader_id, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << endl;
		cout << infoLog << endl;
	}
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		cout << "ERROR::SHADER::LINK_FAILED" << endl;
		cout << infoLog << endl;
	}

	// 链接后就不需要了
	glDeleteShader(vshader_id);
	glDeleteShader(fshader_id);
}

void Shader::use()
{
	glUseProgram(ID);
}

void Shader::set_bool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}

void Shader::set_int(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::set_float(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::set_4f(const std::string& name, float f0, float f1, float f2, float f3) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), f0, f1, f2, f3);
}

void Shader::set_vec3(const std::string& name, glm::vec3 f) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), f.x, f.y, f.z);
}

void Shader::set_mat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

