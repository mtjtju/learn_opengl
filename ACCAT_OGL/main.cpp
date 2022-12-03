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
Vertex Shader ������ɫ��
	in: һ������
	out:һ�����㣬����������
Primitive Assembly ͼԪװ��
	in: ������ɫ�������Ķ���
	out:�ض���ͼԪ��ĳ����״�������ǵ㡢�߻������εȣ�
Geometry Shader ������ɫ��
	in: ͼԪ
	out:�µ�ͼԪ
Rasterization Stage ��դ��
	in: ͼԪ
	out:�ü����ƬԪ����Ⱦһ��������������ݣ�
Alpha test, blending
*/


/*
Shader
	in out: �ֱ�������������
	fragment shader: ���������vec4; ���Զ������룬�Ӷ�����ɫ��������
	vertex shader: �Ӷ�������ֱ�Ӷ�ȡ���룬������layout��������; ���Զ��������
	uniform: ��������ɫ��������һ��uniform���������������ж��塣����������ȫ�ֱ�����
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
	// ��ʼ��------------------------------------------------------------
	glfwInit(); // init glfw
	// (setting option, val)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// ����ģʽ
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "ACCATopengl", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}

	// ����������Ϊ��ǰ�̵߳���������
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	// ע��������ڴ�Сʱ�����ӿڴ�С�Ļص�����
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// ���յ���Ļ�ռ�
	glViewport(0, 0, 800, 600);

	Shader shaderPgYellow("shaders/simple_vshader.vert", "shaders/fshader_Yellow.frag");
	Shader shaderPgBlue("shaders/simple_vshader.vert", "shaders/fshader_Blue.frag");

	// ���ݴ���------------------------------------------------------------
	// ģ������
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

	// VAO �������VBO�еĶ�����ɶ
	//		������EBO������ָ�룬��������disable\enalbe���ã�����ͨ��ָ���ҵ�VBO�е�����
	//		ÿ�ΰ����൱�����°�EBO�����������ϵ�ֵ
	// VBO �������Դ��д��涥��Ķ���
	unsigned int VAOs[2], VBOs[2], EBOs[2];

	// ����������һ��ָ�� my_ptr
	glGenVertexArrays(2, VAOs);
	glGenBuffers(2, VBOs);
	glGenBuffers(2, EBOs);

	// ������:opengl.arry_buffer = my_ptr
	// ���Դ˺����array_buffer�õĶ���my_ptr
	glBindVertexArray(VAOs[0]); // �ȵ��ã����ᱣ������EBO������ָ�������
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);

	// �����ڴ�ռ䲢�����ݴ��������
	// gl_static_draw��ʾ���ݻ��������
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices_with_clr, vertices_with_clr, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices_clr, indices_clr, GL_STATIC_DRAW);

	// param: 
	//		Ŀ�����Ե�����, ��location��Ӧ
	//		����������ά��
	//		type, map to [0,1], �鲽��
	//		������ʼƫ����, ��������ÿһ�������е���ʼλ��
	// ��������0�����ӵ���������ʱ�󶨵�GL_ARRAY_BUFFER��VBO
	// ��ν�Ķ�������0�Ǹ�shader�ģ�����location=0�ı������Ҷ�������0������
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	// Ĭ�϶��������ǽ��õģ��������ã������ڵ�����ָ��һ����VAO�������������±�
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


	// Render Loop�����Ͻ������벢����------------------------------------------
	while (!glfwWindowShouldClose(window)) // ��鴰����û�б�Ҫ���˳�
	{
		// ����
		processInput(window);

		// ��Ⱦָ��
		glClearColor(0.2, 0.3, 0.3, 1.0); // �����Ļ��ɫ
		glClear(GL_COLOR_BUFFER_BIT); // �����ɫ����

		shaderPgYellow.use();
		glBindVertexArray(VAOs[0]);
		shaderPgYellow.set_4f("translation", .5f, 0.f, 0.f, 0.f);
		// params: type, start idx, count
		// glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		//glBindVertexArray(0); // ���

		// gluniform4f�ڵ�ǰ�����shader�����ñ���ֵ
		shaderPgBlue.use();
		glBindVertexArray(VAOs[1]);
		float timeValue = glfwGetTime();
		float greenValue = (sin(timeValue) / 2.f) + .5f;
		shaderPgBlue.set_4f("ourColor", 0.f, greenValue, 0.f, 1.f);
		shaderPgBlue.set_4f("translation", .5f, 0.f, 0.f, 0.f);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		// ˫���壺һ�����Ա�����ʾ����һ�����浱ǰ������Ⱦ��ֵ
		glfwSwapBuffers(window); // ���ƻ���
		glfwPollEvents(); // �����û���¼�����
	}
	glfwTerminate(); // �ͷ���Դ

	return 0;
}