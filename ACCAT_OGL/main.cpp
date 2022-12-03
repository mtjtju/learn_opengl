#include <iostream>
#include <glad/glad.h>
#include <glfw3.h>

#include "Shader.h"

using std::cout;
using std::endl;

/*
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
*/


/*
Shader
	in out: 分别定义了输入和输出
	fragment shader: 输出必须是vec4; 可以定义输入，从顶点着色器拿数据
	vertex shader: 从顶点数据直接读取输入，并依据layout解释数据; 可以定义输出。
	uniform: 可以在着色器中声明一个uniform变量，在主程序中定义。作用类似于全局变量。
	const char* vertexShaderSource =
	"#version 330 core\n"
	"layout (location=0) in vec3 aPos;\n"
	"layout (location=1) in vec3 aColor;\n"
	"out vec3 vertexColor;\n"
	"void main()\n"
	"{\n"
	" gl_Position = vec4(aPos, 1.f);\n"
	" vertexColor = aColor;\n"
	"}\n";

	const char* fragmentShaderYellow =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec3 vertexColor;\n"
	"void main()\n"
	"{\n"
	"	FragColor = vec4(vertexColor, 1.f);\n"
	"}\n";

	const char* fragmentShaderBlue =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"uniform vec4 ourColor;\n"
	"void main()\n"
	"{\n"
	"	FragColor = ourColor;\n"
	"}\n";
*/

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main() {
	// 初始化------------------------------------------------------------
	glfwInit(); // init glfw
	// (setting option, val)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// 和新模式
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "ACCATopengl", NULL, NULL);
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
	glViewport(0, 0, 800, 600);

	Shader shaderPgYellow("shaders/simple_vshader.vert", "shaders/fshader_Yellow.frag");
	Shader shaderPgBlue("shaders/simple_vshader.vert", "shaders/fshader_Blue.frag");

	// 数据传输------------------------------------------------------------
	// 模型输入
	float vertices[] = {
		-.5,-.5,0,
		.5, -.5, 0,
		.5, .5, 0,
		-.5,.5,0
	};
	float vertices_with_clr[] = {
		-.5, -.5, 0,  0, 1, 0,
		.5, -.5, 0,   1, 0, 0,
		-.5, .5, 0,    0, 0, 1
	};
	unsigned int indices_clr[] = { 0, 1, 2 };
	unsigned int indices0[] = { 0, 1, 3 };
	unsigned int indices1[] = { 1, 2, 3 };

	// VAO 负责解释VBO中的顶点是啥
	//		储存了EBO，属性指针，顶点属性disable\enalbe调用，可以通过指针找到VBO中的数据
	//		每次绑定它相当于重新绑定EBO并设置了以上的值
	// VBO 负责在显存中储存顶点的对象
	unsigned int VAOs[2], VBOs[2], EBOs[2];

	// 类似于声明一个指针 my_ptr
	glGenVertexArrays(2, VAOs);
	glGenBuffers(2, VBOs);
	glGenBuffers(2, EBOs);

	// 类似于:opengl.arry_buffer = my_ptr
	// 所以此后调用array_buffer用的都是my_ptr
	glBindVertexArray(VAOs[0]); // 先调用，它会保存后面的EBO和属性指针的设置
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);

	// 开辟内存空间并把数据存入对象中
	// gl_static_draw表示数据基本不会变
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices_with_clr, vertices_with_clr, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices_clr, indices_clr, GL_STATIC_DRAW);

	// param: 
	//		目标属性的索引, 与location对应
	//		属性向量的维度
	//		type, map to [0,1], 组步长
	//		缓冲起始偏移量, 该属性在每一组数据中的起始位置
	// 顶点属性0会链接到函数调用时绑定到GL_ARRAY_BUFFER的VBO
	// 所谓的顶点属性0是给shader的，例如location=0的变量会找顶点属性0的数据
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	// 默认顶点属性是禁用的，这里启用，与现在的属性指针一起存进VAO，参数是属性下标
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(VAOs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices1, indices1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	// Render Loop，不断接受输入并绘制------------------------------------------
	while (!glfwWindowShouldClose(window)) // 检查窗口有没有被要求退出
	{
		// 输入
		processInput(window);

		// 渲染指令
		glClearColor(0.2, 0.3, 0.3, 1.0); // 清空屏幕颜色
		glClear(GL_COLOR_BUFFER_BIT); // 清空颜色缓冲

		shaderPgYellow.use();
		glBindVertexArray(VAOs[0]);
		shaderPgYellow.set_4f("translation", .5f, 0.f, 0.f, 0.f);
		// params: type, start idx, count
		// glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		//glBindVertexArray(0); // 解绑

		// gluniform4f在当前激活的shader中设置变量值
		shaderPgBlue.use();
		glBindVertexArray(VAOs[1]);
		float timeValue = glfwGetTime();
		float greenValue = (sin(timeValue) / 2.f) + .5f;
		shaderPgBlue.set_4f("ourColor", 0.f, greenValue, 0.f, 1.f);
		shaderPgBlue.set_4f("translation", .5f, 0.f, 0.f, 0.f);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		// 双缓冲：一个用以保持显示，另一个储存当前正在渲染的值
		glfwSwapBuffers(window); // 绘制缓冲
		glfwPollEvents(); // 检查有没有事件发生
	}
	glfwTerminate(); // 释放资源

	return 0;
}