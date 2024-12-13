#include "Level.h"
#include "CS300Parser.h"
#include "Controls.h"
#include "models.h"
#include <GLFW/glfw3.h>
#include <GL/GL.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "animations.h"
#include <chrono>
#include <vector>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>
#include <sstream>
#include "ShadowMap.h"


Level* Level::ptr = nullptr;

int Level::Initialize()
{
	// Initialize GLFW
	if (!glfwInit()) {
		return -1;
	}

	// Create a window
	window = glfwCreateWindow(W_WIDTH, W_HEIGHT, "Assignment 1", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}

	glViewport(0, 0, W_WIDTH, W_HEIGHT);

	//Set callbacks
	glfwSetKeyCallback(window, Controls::keyCallback);


	model = new Model("PLANE");

	//Load Scene	
	parser.LoadDataFromFile("data/scenes/scene_A3.txt");


	//Convert from parser->obj to Model
	for (auto o : parser.objects)
	{
		allObjects.push_back(new Model(o));
	}
	for (auto light : parser.lights)
	{
		light.obj.sca = { 1.f,1.f,1.f };
		//TODO: 

		MyAllLights.push_back(new Light(light));
		int idx = 0;
		//allObjects.push_back(new Model(light.obj));		
		if (light.type == "SPOT" || light.type == "POINT"|| light.type == "DIR")
		{
			light.startPos = light.pos;
			light.obj.StartPos = light.startPos;
			MyAllLights[idx]->m->transf.StartPos = light.pos;
			idx++;
		}

	}


	//Save camera
	cam.fovy = parser.fovy;
	cam.width = parser.width;
	cam.height = parser.height;
	cam.nearPlane = parser.nearPlane;
	cam.farPlane = parser.farPlane;
	cam.camPos = parser.camPos;
	cam.camTarget = parser.camTarget;
	cam.camUp = parser.camUp;
	cam.camRight = glm::vec3(1, 0, 0);
	cam.camFront = glm::vec3(0, 0, 100);

	{
		MyViewPort.x = 0;
		MyViewPort.y = 0;
		MyViewPort.width = 500;
		MyViewPort.height = 500;
	}

	//ShadowMap
	int shadow_map_w;
	int shadow_map_h;
	glfwGetWindowSize(window, &shadow_map_w, &shadow_map_h);
	shadow_map = new ShadowMap(shadow_map_w, shadow_map_h);
	shadow_map->Bind();
	shadow_map->UnBind();




	//depth만 보여주는 텍스쳐 생성	


	//Shader program
	ReloadShaderProgram();
	LoadViewPortShader();
	LoadShadowShader();
	//LoadShadowShader();	
	glEnable(GL_CULL_FACE);

	//glFrontFace(GL_CW);	


	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_GREATER);
	//glClearDepth(0);
	return 0;
}

void Level::LightUpdate(float _dt)
{
	time += _dt;
	std::vector<CS300Parser::Light>& light = parser.lights;
	for (int i = 0; i < MyAllLights.size(); i++)
	{
		MyAllLights[i]->m->transf.pos = MyAllLights[i]->m->transf.StartPos;
		if (light[i].anims.size() > 0)
		{
			for (int j = 0; j < light[i].anims.size(); j++)
			{
				MyAllLights[i]->m->transf.pos = light[i].anims[j].Update(MyAllLights[i]->m->transf.pos, time);
				light[i].pos = light[i].anims[j].Update(MyAllLights[i]->m->transf.pos, time);
			}
		}
		light[i].obj.sca = { 1.f,1.f,1.f };
	}
}

int Level::GetType(std::string _str)
{
	if (_str == "POINT")
		return 1;
	else if (_str == "DIR")
		return 2;
	else if (_str == "SPOT")
		return 3;
}

void Level::calculate_normal_avg(Model* _obj)
{
	if (_obj->normal_vertices.size() > 3)
	{
		_obj->normal_vertices.clear();
		int nor_size = _obj->points.size();
		for (int i = 0; i < nor_size; i++)
		{
			glm::vec3 start = _obj->points[i];
			glm::vec3 normal = _obj->normals[i];
			glm::vec3 end = glm::vec3(0, 0, 0);
			int cnt = 0;
			for (int j = 0; j < nor_size; j++)
			{
				if (start == _obj->points[j])
				{
					cnt++;
					end += (_obj->normals[j]);
				}
			}
			_obj->normal_vertices.push_back(start);
			_obj->normal_vertices.push_back(start + glm::normalize(end));
		}
	}
	int a = 0;
}

void Level::ShadowMapDraw()
{
	//glViewport(MyViewPort.x, MyViewPort.y, MyViewPort.width, MyViewPort.height);
}

void Level::mainDraw()
{
	//use shader program		
	glUseProgram(shader->handle);

	//Calculate Camera Matrix
	glm::vec3 dir = glm::normalize(cam.camTarget - cam.camPos);
	dir = -dir;
	glm::vec3 r = glm::normalize(glm::cross(cam.camUp, dir));
	glm::mat4 V = glm::mat4(1);
	glm::vec3 up = glm::normalize(glm::cross(dir, r));

	V[0][0] = r.x;
	V[1][0] = r.y;
	V[2][0] = r.z;
	V[0][1] = up.x;
	V[1][1] = up.y;
	V[2][1] = up.z;
	V[0][2] = dir.x;
	V[1][2] = dir.y;
	V[2][2] = dir.z;
	V[3][0] = -dot(r, cam.camPos);
	V[3][1] = -dot(up, cam.camPos);
	V[3][2] = -dot(dir, cam.camPos);

	//cam.ViewMat = glm::lookAt(cam.camPos, cam.camTarget, up);
	cam.ViewMat = V;

	//The image is mirrored on X
	cam.ProjMat = glm::perspective(glm::radians(cam.fovy), cam.width / cam.height, cam.nearPlane+2, cam.farPlane);


	//For each object in the level
	for (auto o : allObjects)
		Render(o);

	for (auto light : MyAllLights)
		Render(light->m);

	glUseProgram(0);
}

void Level::SmallViewPortDraw()
{
	//use shader program		
	glUseProgram(viewport_shader->handle);
	glBindTextureUnit(0, shadow_map->m_iShadowMapTextureID);
	viewport_shader->setUniform("viewport_texture",0);
	glBindVertexArray(model->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glUseProgram(0);
}

void Level::LoadViewPortShader()
{
	std::stringstream v;
	std::stringstream f;

	std::ifstream file("SmallViewPortShader.vert");

	if (file.is_open())
	{
		v << file.rdbuf();
	}

	file.close();
	file.open("SmallViewPortShader.frag");
	f << file.rdbuf();
	file.close();

	viewport_shader = new cg::Program(v.str().c_str(), f.str().c_str());
}

void Level::RenderDepth(Model* obj)
{
	//use obj VBO
	glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
	//use obj VAO
	glBindVertexArray(obj->VAO);

	//Send model matrix to the shader
	glm::mat4x4 m2w = obj->ComputeMatrix();

	//Send view matrix to the shader
	shadowmap_shader->setUniform("model", cam.ProjMat * cam.ViewMat * m2w);

	glBindTextureUnit(0, obj->textureID);

	//draw
	glDrawArrays(GL_TRIANGLES, 0, obj->points.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Level::Run()
{
	glClearColor(0, 0, 0, 0);
	//glClearDepth(0);
	float TLastFrame = 0;

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		float TCurrentFrame = 0;

		std::chrono::time_point<std::chrono::steady_clock> time = std::chrono::steady_clock::now();

		//Update objects pos
		for (auto obj : allObjects)
			obj->ModelUpdate(TLastFrame);


		std::vector<CS300Parser::Light> all_lights = parser.lights;
		LightUpdate(TLastFrame);

		//////////////////////////////////////
		/// pass 1 - rendering to shadowmap///
		//////////////////////////////////////	

		glViewport(0, 0, W_WIDTH, W_HEIGHT);

		shadow_map->Bind();

		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(shadowmap_shader->handle);

		//Calculate Camera Matrix
		auto li = parser.lights[0];
		glm::vec3 temp_up = glm::vec3(0, 1, 0);
		glm::vec3 dir = glm::normalize(li.dir);
		dir = -dir;
		glm::vec3 r = glm::normalize(glm::cross(temp_up, dir));
		glm::mat4 V = glm::mat4(1);
		glm::vec3 up = glm::normalize(glm::cross(dir, r));

		V[0][0] = r.x;
		V[1][0] = r.y;
		V[2][0] = r.z;
		V[0][1] = up.x;
		V[1][1] = up.y;
		V[2][1] = up.z;
		V[0][2] = dir.x;
		V[1][2] = dir.y;
		V[2][2] = dir.z;
		V[3][0] = -dot(r, li.pos);
		V[3][1] = -dot(up, li.pos);
		V[3][2] = -dot(dir, li.pos);

		cam.ViewMat = V;

		//The image is mirrored on X
		cam.ProjMat = glm::perspective(glm::radians(cam.fovy), cam.width / cam.height, cam.nearPlane+2, cam.farPlane);

		


		//For each object in the level
		for (auto o : allObjects)
		{
			RenderDepth(o);
			
		}
		shadow_map->UnBind();
		glUseProgram(0);

		//////////////////////////////////////
		// pass 2 - rendering to screen///////
		//////////////////////////////////////	

		// Render graphics here
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, W_WIDTH, W_HEIGHT);
		mainDraw();

		glViewport(0, 0, 300, 300);
		SmallViewPortDraw();


		glfwSwapBuffers(window);
		glfwPollEvents();

		std::chrono::time_point<std::chrono::steady_clock> endtime = std::chrono::steady_clock::now();

		TCurrentFrame = std::chrono::duration<float>(endtime - time).count();
		TLastFrame = TCurrentFrame;
	}
	return;
}

bool showNormals = true;

void Level::Render(Model* obj, bool IsShaderMap)
{
	if (render_normal)
		RenderNormal(obj);

	//use obj VBO
	glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
	//use obj VAO
	glBindVertexArray(obj->VAO);

	//Send model matrix to the shader
	glm::mat4x4 m2w = obj->ComputeMatrix();

	//shadowmap texture
	//shader->setUniform("ShadowMapTexture", obj->textureID);
	shader->setUniform("ShadowMapTexture",9);	
	glBindTextureUnit(9, shadow_map->m_iShadowMapTextureID);



	//Light에 대한 ViewProjection Matrix
	auto light = parser.lights[0];
	auto light_pos = light.pos;
	auto light_dir = light.dir;
	auto light_center = light_pos + light_dir;

	auto li = parser.lights[0];
	glm::vec3 temp_up = glm::vec3(0, 1, 0);
	glm::vec3 dir = glm::normalize(li.dir);
	dir = -dir;
	glm::vec3 r = glm::normalize(glm::cross(temp_up, dir));
	glm::mat4 V = glm::mat4(1);
	glm::vec3 up = glm::normalize(glm::cross(dir, r));

	//glm::lookAt(light_pos, light_center, up); <-월드에서 카메라 뷰로 
	V[0][0] = r.x;
	V[1][0] = r.y;
	V[2][0] = r.z;
	V[0][1] = up.x;
	V[1][1] = up.y;
	V[2][1] = up.z;
	V[0][2] = dir.x;
	V[1][2] = dir.y;
	V[2][2] = dir.z;
	V[3][0] = -dot(r, li.pos);
	V[3][1] = -dot(up, li.pos);
	V[3][2] = -dot(dir, li.pos);

	lightMatrix = cam.ProjMat *V/*glm::lookAt(light_pos, light_center, up)*/; //lightProj x 카메라 행렬 

	//light matrix
	shader->setUniform("LightTransform", lightMatrix);
	//Send view matrix to the shader
	shader->setUniform("model", cam.ProjMat * cam.ViewMat * m2w);

	glBindTextureUnit(0, obj->textureID);

	glBindTextureUnit(4, obj->m_iNormalID);	


	shader->setUniform("myTextureSampler", 0);
	shader->setUniform("uNormalMap", 4);

	shader->setUniform("hasTexture", b_tex);
	shader->setUniform("normal", b_normal);

	if (obj->transf.name == "")
	{
		shader->setUniform("LightColorOn", true);
	}
	else
	{
		shader->setUniform("LightColorOn", false);
	}

	std::vector<CS300Parser::Light> all_lights = parser.lights;
	int MyLightsize = MyAllLights.size();
	int light_size = all_lights.size();
	shader->setUniform("uLightNum", light_size);

	glm::vec3 camdir = glm::normalize(Level::GetPtr()->cam.camPos) - (Level::GetPtr()->cam.camTarget);
	glm::vec3 campos = (Level::GetPtr()->cam.camPos);
	shader->setUniform("uCameraPos", campos);
	shader->setUniform("modeltoworld", m2w);


	for (int i = 0; i < MyLightsize; i++)
	{
		shader->setUniform("uLight[" + std::to_string(i) + "].type", GetType(all_lights[i].type));
		shader->setUniform("uLight[" + std::to_string(i) + "].col", all_lights[i].col);
		parser.lights[i].pos = MyAllLights[i]->m->transf.pos;

		shader->setUniform("uLight[" + std::to_string(i) + "].positionWorld", all_lights[i].pos);
		shader->setUniform("uLight[" + std::to_string(i) + "].amb", all_lights[i].amb);
		shader->setUniform("uLight[" + std::to_string(i) + "].att", all_lights[i].att);
		shader->setUniform("uLight[" + std::to_string(i) + "].dir", all_lights[i].dir);

		shader->setUniform("uLight[" + std::to_string(i) + "].inner", all_lights[i].inner);
		shader->setUniform("uLight[" + std::to_string(i) + "].outer", all_lights[i].outer);
		shader->setUniform("uLight[" + std::to_string(i) + "].falloff", all_lights[i].falloff);
	}

	CS300Parser::MaterialParameters mp;
	shader->setUniform("mp_ambient", mp.ambient);
	shader->setUniform("mp_diffuse", mp.diffuse);
	shader->setUniform("mp_specular", mp.specular);
	shader->setUniform("mp_shininess", obj->transf.ns);


	//draw
	glDrawArrays(GL_TRIANGLES, 0, obj->points.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Level::ShadowMapRender(Model* obj)
{
}

void Level::RenderNormal(Model* _obj)
{
	glBindBuffer(GL_ARRAY_BUFFER, _obj->normal_VBO);
	glBindVertexArray(_obj->normal_VAO);

	glm::mat4x4 m2w = _obj->ComputeMatrix();
	shader->setUniform("model", cam.ProjMat * cam.ViewMat * m2w);
	glDrawArrays(GL_LINES, 0, _obj->points.size() * 2);
}

Level::Level() : window(nullptr), shader(nullptr)
{

}

Level::~Level()
{
	for (auto m : allObjects)
		delete m;

	allObjects.clear();
}

void Level::Cleanup()
{
	// Clean up
	glfwDestroyWindow(window);
	glfwTerminate();

	DeletePtr();
}

Level* Level::GetPtr()
{
	if (!ptr)
		ptr = new Level;
	return ptr;
}

void Level::DeletePtr()
{
	if (ptr)
	{
		delete ptr;
	}

	ptr = nullptr;
}

void Level::LoadShadowShader()
{
	std::stringstream v;
	std::stringstream f;

	std::ifstream file("shadowmap.vert");

	if (file.is_open())
	{
		v << file.rdbuf();
	}

	file.close();
	file.open("shadowmap.frag");
	f << file.rdbuf();
	file.close();

	shadowmap_shader = new cg::Program(v.str().c_str(), f.str().c_str());
}

void Level::ReloadShaderProgram()
{
	glUseProgram(0);

	if (shader)
		delete shader;

	std::stringstream v;
	std::stringstream f;

	std::ifstream file("data/shaders/vert.vert");

	if (file.is_open())
	{
		v << file.rdbuf();
	}

	file.close();
	file.open("data/shaders/frag.frag");
	f << file.rdbuf();
	file.close();

	shader = new cg::Program(v.str().c_str(), f.str().c_str());


}

void Level::RotateCamY(float angle)
{
	cam.camPos = cam.camTarget - glm::vec3(glm::rotate(glm::identity<glm::mat4>(), glm::radians(angle), cam.camUp) * glm::vec4(cam.camTarget - cam.camPos, 1));
}

void Level::RotateCamX(float angle)
{
	glm::vec3 right = glm::cross(cam.camUp, cam.camPos - cam.camTarget);
	glm::vec3 rotVec = glm::vec3(glm::rotate(glm::identity<glm::mat4>(), glm::radians(-angle), right) * glm::vec4(cam.camTarget - cam.camPos, 1));

	if (abs(rotVec.z) > 0.1f)
		cam.camPos = cam.camTarget - rotVec;
}

void Level::RotateCamZ(float angle)
{
	cam.camPos += angle * (cam.camTarget - cam.camPos);
}

Model* Level::FindModel(std::string _name)
{
	int size = allObjects.size();
	for (int i = 0; i < size; i++)
	{
		if (allObjects[i]->transf.name == _name)
			return allObjects[i];
	}
	return nullptr;
}