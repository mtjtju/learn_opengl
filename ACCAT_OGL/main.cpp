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

/*
Words
OpenGL
GLAD
�ӿ�
ͼ�ι���
������ɫ��:		���Ϊ�ü��ռ�
Ƭ����ɫ��:		��Ļ�ռ�������
��׼���豸����
���㻺�����
�����������
Ԫ�ػ������
Uniform
GLSL
�������:		˫���Ժ�������
�༶��Զ����
GLM
�ֲ��ռ�
�ü��ռ�:		vshader�������w���ܲ���1
��Ļ�ռ�
LookAt����
ŷ����
*/

/*
һ��������ⷽʽ��
	GL��һ�������ʵ������һϵ�����Ժͺ���
	һ����������һЩ���������ڸ�ֵ��GL�����ԣ��ٵ���GL��һЩ������
	step1: glGenxxx		����ptr
	step2: glBindxxx	ptr��ֵ��GL.x
	step3: glBufferData,glDraw....
						��GL��һЩ����,Ҳ���Ƕ�GL.x��һЩ��,Ҳ���Ƕ�ptr��һЩ��
*/

/*
ƽ�й⣺direction
���Դ��ǿ��ϵ��=1/����Ķ���ʽ��<=1.0
		����λ�û���Ҫ����ֵ�����壺�����һ����ͳ������ϵ��
�۹�spotlight:
		������׶�嶨�壬׵��֮���ǿ��������ֵ�������Բ�ֵ0~1
		������
			λ��
			����
			���й�ǣ�׶��ǵ�һ�룬һ��������ֵ��
			���й�ǣ�׶��ǵ�һ�룬һ��������ֵ��

�ṹ�����͵�uniform��ֵ:
	���ݳ�Ա���������ʹ�������ֵ�����Ʋ���Ϊ �ṹ�������+'.'+��Ա������
�ṹ�����������uniform��ֵ:
	���ݳ�Ա���������ʹ�������ֵ�����Ʋ���Ϊ �ṹ�����������+[idx]+'.'+��Ա������

*/

/*
���Դʵ��
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
	// ��ʼ��------------------------------------------------------------
	glfwInit(); // init glfw
	// (setting option, val)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// ����ģʽ
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int window_width = width, window_height = height;
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

	Shader shader_blinnphong("shaders/blinnphong.vt", "shaders/blinnphong.fr");
	Shader shader_blinnphong_light("shaders/blinnphong_light_obj.vt", "shaders/blinnphong_light_obj.fr");

	// ���ݴ���------------------------------------------------------------
	// ģ������
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};
	glm::vec3 obj_positions[] = {
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
	unsigned int VAO, VBO;

	// ����������һ��ָ�� my_ptr
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); // �ȵ��ã����ᱣ������EBO������ָ�������

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int textureID = loadTexture("../textures/container2.png");
	unsigned int texture_spec_ID = loadTexture("../textures/container2_specular.png");
	unsigned int texture_green_ID = loadTexture("../textures/matrix.jpg");
	shader_blinnphong.use();
	shader_blinnphong.set_int("obj_mt.diff", 0);
	shader_blinnphong.set_int("obj_mt.spec", 1);
	shader_blinnphong.set_int("obj_mt.emi", 2);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
		glClearColor(0.01f, 0.01f, 0.01f, 0.1f); // �����Ļ��ɫ
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		shader_blinnphong.use();
		shader_blinnphong.set_float("obj_mt.gloss", 64.f);
		shader_blinnphong.set_vec3("cam_pos", cam.Position);
		glm::vec3 obj_clr(1.f, 0.5f, 0.3f), light_clr(1.f, 1.f, 1.f);
		glm::vec3 dim_red(0.3f, 0.0f, 0.0f), dim_spot = 0.3f * light_clr;
		glm::vec3 light0_pos(0.5, 0.5, -4), light_spot_pos(0.2, 0.4, 1.2);
		shader_blinnphong.set_vec3("light[0].amb", 0.2f * dim_red);
		shader_blinnphong.set_vec3("light[0].diff", 0.5f * dim_red);
		shader_blinnphong.set_vec3("light[0].spec", dim_red);
		shader_blinnphong.set_vec3("light[0].pos", light0_pos);
		shader_blinnphong.set_vec3("light_para[0].amb", 0.1f * light_clr);
		shader_blinnphong.set_vec3("light_para[0].diff", 0.3f * light_clr);
		shader_blinnphong.set_vec3("light_para[0].spec", light_clr);
		shader_blinnphong.set_vec3("light_para[0].dir", glm::vec3(-2, 1, 1));
		shader_blinnphong.set_vec3("light_spot[0].amb", 0.2f * dim_spot);
		shader_blinnphong.set_vec3("light_spot[0].diff", 0.5f * dim_spot);
		shader_blinnphong.set_vec3("light_spot[0].spec", dim_spot);
		shader_blinnphong.set_vec3("light_spot[0].dir", glm::vec3(0, 0, -1));
		shader_blinnphong.set_vec3("light_spot[0].pos", light_spot_pos);
		shader_blinnphong.set_float("light_spot[0].inner_theta", 0.98f);
		shader_blinnphong.set_float("light_spot[0].outer_theta", 0.94f);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_spec_ID);
		// glActiveTexture(GL_TEXTURE2);
		// glBindTexture(GL_TEXTURE_2D, texture_green_ID);

		glBindVertexArray(VAO);

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

		shader_blinnphong.set_mat4("p", p);
		shader_blinnphong.set_mat4("v", v);

		for (int i = 0; i < 10; i++)
		{
			glm::mat4 m(1.0);
			m = glm::translate(m, obj_positions[i]);
			float angle = 20.f * i;
			m = glm::rotate(m, angle, glm::vec3(1.f, 0.3f, 0.5f));
			shader_blinnphong.set_mat4("m", m);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		shader_blinnphong_light.use();
		shader_blinnphong_light.set_vec3("light_clr", dim_red);
		glm::mat4 m(1.0);
		m = glm::translate(m, light0_pos);
		m = glm::scale(m, glm::vec3(0.2f));
		shader_blinnphong_light.set_mat4("p", p);
		shader_blinnphong_light.set_mat4("v", v);
		shader_blinnphong_light.set_mat4("m", m);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shader_blinnphong_light.set_vec3("light_clr", dim_spot);
		m = glm::mat4(1.0);
		m = glm::translate(m, light_spot_pos);
		m = glm::scale(m, glm::vec3(0.2f));
		shader_blinnphong_light.set_mat4("m", m);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ˫���壺һ�����Ա�����ʾ����һ�����浱ǰ������Ⱦ��ֵ
		glfwSwapBuffers(window); // ���ƻ���
		glfwPollEvents(); // �����û���¼�����

	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
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
