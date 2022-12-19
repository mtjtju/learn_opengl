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

/*
ACCAT!~!!!!
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

texture̫С---texture magnify����---˫���Բ�ֵ
texture̫��---texture minify����----mipmap, �����Բ�ֵ

texture unit ����Ԫ
	�൱��ָ������������ݵ�ָ�룬ogl��16����������Ԫ��һ������������ָ������һ��

����
	��ϰ��
		�޸�Ƭ����ɫ��������Ц��ͼ������һ�����򿴣��ο����
			in fshader, u = 1.0 - u
		�����ò�ͬ�������Ʒ�ʽ���趨һ����0.0f��2.0f��Χ�ڵģ�������ԭ����0.0f��1.0f���������ꡣ���Կ��ܲ��������ӵĽ������4��Ц�����ο���𣬽�����ǵ�һ��Ҫ���������Ļ��Ʒ�ʽ��
			in vertex data, double uv
		�����ھ�����ֻ��ʾ����ͼ����м�һ���֣��޸��������꣬�ﵽ�ܿ������������ص�Ч��������ʹ��GL_NEAREST��������˷�ʽ��������ʾ�ø��������ο����
			in vertex data, uv *= 0.05
		ʹ��һ��uniform������Ϊmix�����ĵ������������ı���������ɼ��ȣ�ʹ���Ϻ��¼����ı����ӻ�Ц���Ŀɼ��ȣ��ο����
			GLFW_PRESS��ⰴ���¼���������Сһ��Ͳ���ͻ��

GLSL�����Դ���
	������������Ƕ�ӦԪ�����
opengl glm�������Դ���
	��������������ֵ�+-*������������ÿ��Ԫ��
	glm::vec4 vec(1, 0, 0, 1);
	glm::mat4 trans; // identity matrix
	trans = glm::translate(trans, glm::vec3(1, 1, 0));
	vec = trans * vec;
	cout << vec.x << ' ' << vec.y << ' ' << vec.z << endl;

camera
	lookat(pos, target, up)
	ŷ���ǣ�
		pitch������x��yawƫ����y��roll��ת��z
		ͨ��ŷ���Ǽ����������������
			y = sin(pitch)
			x = cos(pitch) * cos(Yaw)
			z = cos(pitch) * sin(Yaw)
*/

/*
Shader
	in out: �ֱ�������������
	fragment shader: ���������vec4; ���Զ������룬�Ӷ�����ɫ��������
	vertex shader: �Ӷ�������ֱ�Ӷ�ȡ���룬������layout��������; ���Զ��������
	uniform: ��������ɫ��������һ��uniform���������������ж��塣����������ȫ�ֱ�����
*/

/*
glfw
�ص�����
	glfwSetxxxCallback(window, fun_ptr)
	���������glfw������, �û�ʵ��
�������ģʽ
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
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
	// ��ʼ��------------------------------------------------------------
	glfwInit(); // init glfw
	// (setting option, val)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// ����ģʽ
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int window_width = 800, window_height = 600;
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "ACCATopengl", NULL, NULL);
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
	glViewport(0, 0, window_width, window_height);

	Shader shader_texture("shaders/vshader_mvp.vert", "shaders/fshader_mvp.frag");

	// ���ݴ���------------------------------------------------------------
	// ģ������
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

	// ���ơ����˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // s, t��������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// ���ݴ����������
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // s, t��������
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

	// VAO �������VBO�еĶ�����ɶ
	//		������EBO������ָ�룬��������disable\enalbe���ã�����ͨ��ָ���ҵ�VBO�е�����
	//		ÿ�ΰ����൱�����°�EBO�����������ϵ�ֵ
	// VBO �������Դ��д��涥��Ķ���
	unsigned int VAO, VBO, EBO;

	// ����������һ��ָ�� my_ptr
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// ������:opengl.arry_buffer = my_ptr
	// ���Դ˺����array_buffer�õĶ���my_ptr
	glBindVertexArray(VAO); // �ȵ��ã����ᱣ������EBO������ָ�������
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// �����ڴ�ռ䲢�����ݴ��������
	// gl_static_draw��ʾ���ݻ��������
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

	// param: 
	//		Ŀ�����Ե�����, ��location��Ӧ
	//		����������ά��
	//		type, map to [0,1], �鲽��
	//		������ʼƫ����, ��������ÿһ�������е���ʼλ��
	// ��������0�����ӵ���������ʱ�󶨵�GL_ARRAY_BUFFER��VBO
	// ��ν�Ķ�������0�Ǹ�shader�ģ�����location=0�ı������Ҷ�������0������
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	// Ĭ�϶��������ǽ��õģ��������ã������ڵ�����ָ��һ����VAO�������������±�
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	float s = 0.0, step = 0.001;

	// ���
	//		��ת
	//		ǰ�������ƶ�
	
	// glm::lookAt(eye, center, up)

	glm::mat4 v, p, mvp;
	cam.Position = glm::vec3(0, 0, 6);
	cam.Front.z = -1;
	cam.Up = glm::vec3(0, 1, 0);
	float delta_t = 0, cur_t = 0, pre_t = 0;

	// Render Loop�����Ͻ������벢����------------------------------------------
	glEnable(GL_DEPTH_TEST);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	while (!glfwWindowShouldClose(window)) // ��鴰����û�б�Ҫ���˳�
	{
		// ����
		processInput(window);

		// ��Ⱦָ��
		glClearColor(0.2, 0.3, 0.3, 1.0); // �����Ļ��ɫ
		glClear(GL_COLOR_BUFFER_BIT); // �����ɫ����

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


		// ˫���壺һ�����Ա�����ʾ����һ�����浱ǰ������Ⱦ��ֵ
		glfwSwapBuffers(window); // ���ƻ���
		glfwPollEvents(); // �����û���¼�����

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
	glfwTerminate(); // �ͷ���Դ

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
		prex = x,
		prey = y,
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
