#include <iostream>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "camera_fps.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::cout;
using std::endl;

/* ACCAT!~!!!!
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double x, double y);
void scroll_callback(GLFWwindow* window, double x, double y);

bool first_mouse = true;
int width = 800, height = 600;
double prex = width / 2;
double prey = height / 2;
double mouse_speed = 0.05;
double pitch = 0, yaw = -90;
float fov = 45.f;
glm::vec3 cam_front;
Camera cam;


int main() {
	// 初始化------------------------------------------------------------
	glfwInit(); // init glfw
	// (setting option, val)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// 和新模式
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int window_width = 800, window_height = 600;
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

	Shader shader_texture("shaders/vshader_mvp.vert", "shaders/fshader_mvp.frag");

	// 数据传输------------------------------------------------------------
	// 模型输入
	float uvscale = 1.0;
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};


	// texture ----------------------------------
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("../textures/container.jpg", &width, &height, &nrChannels, 0);
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	// 环绕、过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // s, t是两个轴
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// 数据传到纹理对象
	if (data)
	{
		//           type,mipmap level,form,   w,    h,     0,   save rgb as byte(char), data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture" << endl;
	}
	stbi_image_free(data);

	int width_face, height_face, nrChannels_face;
	unsigned int texture_face;
	unsigned char* face = stbi_load("../textures/awesomeface.png", &width_face, &height_face, &nrChannels_face, 0);
	glGenTextures(1, &texture_face);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_face);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // s, t是两个轴
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (face)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_face, height_face, 0, GL_RGBA, GL_UNSIGNED_BYTE, face);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture" << endl;
	}
	stbi_image_free(face);

	shader_texture.use();
	glUniform1i(glGetUniformLocation(shader_texture.ID, "ourTexture"), 0);
	glUniform1i(glGetUniformLocation(shader_texture.ID, "faceTexture"), 1);

	// VAO 负责解释VBO中的顶点是啥
	//		储存了EBO，属性指针，顶点属性disable\enalbe调用，可以通过指针找到VBO中的数据
	//		每次绑定它相当于重新绑定EBO并设置了以上的值
	// VBO 负责在显存中储存顶点的对象
	unsigned int VAO, VBO, EBO;

	// 类似于声明一个指针 my_ptr
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// 类似于:opengl.arry_buffer = my_ptr
	// 所以此后调用array_buffer用的都是my_ptr
	glBindVertexArray(VAO); // 先调用，它会保存后面的EBO和属性指针的设置
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// 开辟内存空间并把数据存入对象中
	// gl_static_draw表示数据基本不会变
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

	// param: 
	//		目标属性的索引, 与location对应
	//		属性向量的维度
	//		type, map to [0,1], 组步长
	//		缓冲起始偏移量, 该属性在每一组数据中的起始位置
	// 顶点属性0会链接到函数调用时绑定到GL_ARRAY_BUFFER的VBO
	// 所谓的顶点属性0是给shader的，例如location=0的变量会找顶点属性0的数据
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	// 默认顶点属性是禁用的，这里启用，与现在的属性指针一起存进VAO，参数是属性下标
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	float s = 0.0, step = 0.001;

	// 相机
	//		旋转
	//		前后左右移动
	
	// glm::lookAt(eye, center, up)

	glm::mat4 v, p, mvp;
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
		glClearColor(0.2, 0.3, 0.3, 1.0); // 清空屏幕颜色
		glClear(GL_COLOR_BUFFER_BIT); // 清空颜色缓冲

		shader_texture.use();
		glBindVertexArray(VAO);

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			s = std::min(1.f, s + step),
			shader_texture.set_float("scale", s);
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			s = std::max(0.f, s - step),
			shader_texture.set_float("scale", s);

		p = glm::perspective(fov, window_width * 1.f / window_height, 0.1f, 100.f);
		cur_t = glfwGetTime();
		delta_t = cur_t - pre_t;
		pre_t = cur_t;
		for (int i = 0; i < 10; i++)
		{
			glm::mat4 m;
			m = glm::translate(m, cubePositions[i]);
			if (i % 2 == 0)
				m = glm::rotate(m, (float)glfwGetTime()*55 + 30.f * i, glm::vec3(0.5, 1, 0));

			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				cam.ProcessKeyboard(Camera_Movement::FORWARD, delta_t);
			else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				cam.ProcessKeyboard(Camera_Movement::BACKWARD, delta_t);
			else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				cam.ProcessKeyboard(Camera_Movement::LEFT, delta_t);
			else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				cam.ProcessKeyboard(Camera_Movement::RIGHT, delta_t);
			v = cam.GetViewMatrix();

			mvp = p * v * m;
			unsigned int mvpLoc = glGetUniformLocation(shader_texture.ID, "mvp");
			glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		// 双缓冲：一个用以保持显示，另一个储存当前正在渲染的值
		glfwSwapBuffers(window); // 绘制缓冲
		glfwPollEvents(); // 检查有没有事件发生

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
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
