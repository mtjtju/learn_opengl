#include <iostream>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>
#pragma comment(lib, "assimp-vc143-mtd.lib")

#include "Shader.h"
#include "camera_fps.h"
#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::cout;
using std::endl;

/*
ACCAT!~!!!!
VAO
VBO
EBO or IBO
Vertex Shader 顶点着色器
	in: 一个顶点
	out:一个顶点，处理后的属性
Primitive Assembly 图元装配
	in: 顶点着色器产生的顶点
	out:特定的图元（某种形状，可以是点、线或三角形等）
Geometry Shader 几何着色器
	in: 图元
	out:新的图元
Rasterization Stage 光栅化
	in: 图元
	out:裁剪后的片元（渲染一个像素所需的数据）
Alpha test, blending

texture太小---texture magnify问题---双线性插值
texture太大---texture minify问题----mipmap, 三线性插值

texture unit 纹理单元
	相当于指向具体纹理数据的指针，ogl有16个以上纹理单元，一个采样器可以指向其中一个

纹理
	练习：
		修改片段着色器，仅让笑脸图案朝另一个方向看，参考解答
			in fshader, u = 1.0 - u
		尝试用不同的纹理环绕方式，设定一个从0.0f到2.0f范围内的（而不是原来的0.0f到1.0f）纹理坐标。试试看能不能在箱子的角落放置4个笑脸：参考解答，结果。记得一定要试试其它的环绕方式。
			in vertex data, double uv
		尝试在矩形上只显示纹理图像的中间一部分，修改纹理坐标，达到能看见单个的像素的效果。尝试使用GL_NEAREST的纹理过滤方式让像素显示得更清晰：参考解答
			in vertex data, uv *= 0.05
		使用一个uniform变量作为mix函数的第三个参数来改变两个纹理可见度，使用上和下键来改变箱子或笑脸的可见度：参考解答。
			GLFW_PRESS检测按键事件，步长设小一点就不会突变

GLSL与线性代数
	两个向量相乘是对应元素相乘
opengl glm库与线性代数
	向量、矩阵和数字的+-*除，都作用于每个元素
	glm::vec4 vec(1, 0, 0, 1);
	glm::mat4 trans; // identity matrix
	trans = glm::translate(trans, glm::vec3(1, 1, 0));
	vec = trans * vec;
	cout << vec.x << ' ' << vec.y << ' ' << vec.z << endl;

camera
	lookat(pos, target, up)
	欧拉角：
		pitch俯仰角x，yaw偏航角y，roll滚转角z
		通过欧拉角计算相机方向向量：
			y = sin(pitch)
			x = cos(pitch) * cos(Yaw)
			z = cos(pitch) * sin(Yaw)
*/

/*
Shader
	in out: 分别定义了输入和输出
	fragment shader: 输出必须是vec4; 可以定义输入，从顶点着色器拿数据
	vertex shader: 从顶点数据直接读取输入，并依据layout解释数据; 可以定义输出。
	uniform: 可以在着色器中声明一个uniform变量，在主程序中定义。作用类似于全局变量。
*/

/*
glfw
回调函数
	glfwSetxxxCallback(window, fun_ptr)
	这个函数是glfw声明的, 用户实现
鼠标隐藏模式
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
*/

/*
Words
OpenGL
GLAD
视口
图形管线
顶点着色器:		输出为裁剪空间
片段着色器:		屏幕空间是输入
标准化设备坐标
顶点缓冲对象
顶点数组对象
元素缓冲对象
Uniform
GLSL
纹理过滤:		双线性和三线性
多级渐远纹理
GLM
局部空间
裁剪空间:		vshader的输出，w可能不是1
屏幕空间
LookAt矩阵
欧拉角
*/

/*
一种流程理解方式：
	GL像一个大对象实例，有一系列属性和函数
	一般是先声明一些独立对象，在赋值给GL的属性，再调用GL的一些函数：
	step1: glGenxxx		声明ptr
	step2: glBindxxx	ptr赋值给GL.x
	step3: glBufferData,glDraw....
						对GL做一些事情,也就是对GL.x做一些事,也就是对ptr做一些事
*/

/*
平行光：direction
点光源：强度系数=1/距离的二次式，<=1.0
		除了位置还需要三个值来定义：二次项、一次项和常数项的系数
聚光spotlight:
		由两个锥体定义，椎体之间光强按照余弦值进行线性插值0~1
		参数：
			位置
			方向
			内切光角（锥体角的一半，一般是余弦值）
			外切光角（锥体角的一半，一般是余弦值）

结构体类型的uniform赋值:
	根据成员变量的类型传具体数值，名称参数为 结构体变量名+'.'+成员变量名
结构体类型数组的uniform赋值:
	根据成员变量的类型传具体数值，名称参数为 结构体数组变量名+[idx]+'.'+成员变量名

*/

/*
多光源实操
*/

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double x, double y);
void scroll_callback(GLFWwindow* window, double x, double y);
unsigned int loadTexture(char const* path);

bool first_mouse = true;
int width = 1200, height = 900;
double prex = width / 2;
double prey = height / 2;
double mouse_speed = 0.05;
double pitch = 0, yaw = -90;
float fov = 45.f;
glm::vec3 cam_front;
Camera cam;
glm::vec3 light_pos(1.2f, 1.f, 2.f);


int main() {
	// 初始化------------------------------------------------------------
	glfwInit(); // init glfw
	// (setting option, val)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// 和新模式
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int window_width = width, window_height = height;
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "ACCATopengl", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}

	// 将窗口设置为当前线程的主上下文
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	// 注册调整窗口大小时调整视口大小的回调函数
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 最终的屏幕空间
	glViewport(0, 0, window_width, window_height);

	Shader shader("shaders/shader_assimp.vt", "shaders/shader_assimp.fr");
	Model nano("../models/nanosuit/nanosuit.obj");

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	cam.Position = glm::vec3(0, 0, 6);
	cam.Front.z = -1;
	cam.Up = glm::vec3(0, 1, 0);
	float delta_t = 0, cur_t = 0, pre_t = 0;

	// Render Loop，不断接受输入并绘制------------------------------------------
	glEnable(GL_DEPTH_TEST);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	while (!glfwWindowShouldClose(window)) // 检查窗口有没有被要求退出
	{
		// 输入
		processInput(window);

		// 渲染指令
		glClearColor(0.01f, 0.01f, 0.01f, 0.1f); // 清空屏幕颜色
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		shader.use();

		cur_t = glfwGetTime();
		delta_t = cur_t - pre_t;
		pre_t = cur_t;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cam.ProcessKeyboard(Camera_Movement::FORWARD, delta_t);
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cam.ProcessKeyboard(Camera_Movement::BACKWARD, delta_t);
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cam.ProcessKeyboard(Camera_Movement::LEFT, delta_t);
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cam.ProcessKeyboard(Camera_Movement::RIGHT, delta_t);
		else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			cam.ProcessKeyboard(Camera_Movement::UP, delta_t);
		else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			cam.ProcessKeyboard(Camera_Movement::DOWN, delta_t);
		glm::mat4 v = cam.GetViewMatrix();
		glm::mat4 p = glm::perspective(fov, window_width * 1.f / window_height, 0.1f, 100.f);
		glm::mat4 m = glm::mat4(1.0f);
		m = glm::translate(m, glm::vec3(0.f, -0.5f, 0.f));
		m = glm::scale(m, glm::vec3(0.2f, 0.2f, 0.2f));

		shader.set_mat4("p", p);
		shader.set_mat4("v", v);
		shader.set_mat4("m", m);
		nano.Draw(shader);

		// 双缓冲：一个用以保持显示，另一个储存当前正在渲染的值
		glfwSwapBuffers(window); // 绘制缓冲
		glfwPollEvents(); // 检查有没有事件发生

	}
	glfwTerminate(); // 释放资源

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double x, double y)
{
	if (first_mouse)
		prex = width / 2.0,
		prey = height / 2.0,
		first_mouse = false;

	double yd = prey - y, xd = x - prex;
	prey = y, prex = x;
	cam.ProcessMouseMovement(xd, yd);
}

void scroll_callback(GLFWwindow* window, double x, double y)
{
	// cout << y << endl;
	cam.ProcessMouseScroll(y);
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
