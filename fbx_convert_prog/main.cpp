
//// IF THERE ARE GLFW ERRRORS ON RUN: use the tools -> nugetpackage manager and uninstall then reinstall glfw ////

/*
CPE/CSC 474 Lab base code Eckhardt/Dahl
based on CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

//Midterm 2 Reivew, Gravity between various particles

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
#include "Line.h"
#include "bone.h"
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;
shared_ptr<Shape> plane;
mat4 linint_between_two_orientations(vec3 ez_aka_lookto_1, vec3 ey_aka_up_1, vec3 ez_aka_lookto_2, vec3 ey_aka_up_2, float t);


//*************************************************************************************************
//
//			TEST YOUR ANIMATION
//
//*************************************************************************************************

class cstar // star information
{
	public:
		vec3 pos, speed;
};
cstar mystar1, mystar2;

mat4 test_animation()
	{
	static float t = 0.0;
	t += 0.01;
	//float f = (sin(t)+1.)/2.0;//
	mat4 mt;
	vec3 ez1, ey1, ez2, ey2, ez3, ey3, ez4, ey4;

	ez1 = vec3(0, 0, 1);
	ey1 = vec3(0, 1, 0);

	ez2 = vec3(0.157114, -0.0207484, 0.987362);
	ey2 = vec3(-0.978864, 0.129268, 0.158478);

	ez3 = vec3(0.799965, -0.0758681, 0.595231);
	ey3 = vec3(-0.356247, 0.73818, 0.572869);

	ez4 = vec3(0.883246, -0.0758681, -0.462732);
	ey4 = vec3(0.361728, 0.73818, 0.569423);



	if (t <= 1)					mt = linint_between_two_orientations(ez1, ey1, ez2, ey2, t);
	else if (t <= 2)			mt = linint_between_two_orientations(ez2, ey2, ez3, ey3, t - 1.);
	else if (t <= 3)			mt = linint_between_two_orientations(ez3, ey3, ez4, ey4, t - 2.);
	else if (t <= 4)			mt = linint_between_two_orientations(ez4, ey4, ez1, ey1, t - 3.);
	else
		{
		t = 0;
		mt = linint_between_two_orientations(ez1, ey1, ez2, ey2, t);
		}


	return mt;
	}
//*************************************************************************************************
//
//			END - TEST YOUR ANIMATION
//
//*************************************************************************************************

mat4 linint_between_two_orientations(vec3 ez_aka_lookto_1, vec3 ey_aka_up_1, vec3 ez_aka_lookto_2, vec3 ey_aka_up_2, float t)
	{
	mat4 m1, m2;
	quat q1, q2;
	vec3 ex, ey, ez;
	ey = ey_aka_up_1;
	ez = ez_aka_lookto_1;
	ex = cross(ey, ez);
	m1[0][0] = ex.x;		m1[0][1] = ex.y;		m1[0][2] = ex.z;		m1[0][3] = 0;
	m1[1][0] = ey.x;		m1[1][1] = ey.y;		m1[1][2] = ey.z;		m1[1][3] = 0;
	m1[2][0] = ez.x;		m1[2][1] = ez.y;		m1[2][2] = ez.z;		m1[2][3] = 0;
	m1[3][0] = 0;			m1[3][1] = 0;			m1[3][2] = 0;			m1[3][3] = 1.0f;
	ey = ey_aka_up_2;
	ez = ez_aka_lookto_2;
	ex = cross(ey, ez);
	m2[0][0] = ex.x;		m2[0][1] = ex.y;		m2[0][2] = ex.z;		m2[0][3] = 0;
	m2[1][0] = ey.x;		m2[1][1] = ey.y;		m2[1][2] = ey.z;		m2[1][3] = 0;
	m2[2][0] = ez.x;		m2[2][1] = ez.y;		m2[2][2] = ez.z;		m2[2][3] = 0;
	m2[3][0] = 0;			m2[3][1] = 0;			m2[3][2] = 0;			m2[3][3] = 1.0f;
	q1 = quat(m1);
	q2 = quat(m2);
	quat qt = slerp(q1, q2, t); //<---
	qt = normalize(qt);
	mat4 mt = mat4(qt);
	//mt = transpose(mt);		 //<---
	return mt;
	}




double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 10*ftime;
		}
		else if (s == 1)
		{
			speed = -10*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -3*ftime;
		else if(d==1)
			yangle = 3*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, psky, pplane;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexBufferIDimat, VertexNormDBox, VertexTexBox, IndexBufferIDBox;

	//texture data
	GLuint Texture;
	GLuint Texture2;

	//line
	Line linerender;
	Line smoothrender;
	vector<vec3> line;
	
	//animation matrices:
	mat4 animmat[200];
	int animmatsize=0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
	
		
		if (key == GLFW_KEY_C && action == GLFW_RELEASE)
		{
			if (smoothrender.is_active())
				smoothrender.reset();
			else
				{
				vector<vec3> cardinal;
				cardinal_curve(cardinal, line, 5, 1.0);
				smoothrender.re_init_line(cardinal);
				}
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = 0;
			newPt[1] = 0;

			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
			//update the vertex array with the updated points
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	bone *root = NULL;
	int size_stick = 0;
	all_animations all_animation;
	void initGeom(const std::string& resourceDirectory)
	{

		for (int ii = 0; ii < 200; ii++)
			animmat[ii] = mat4(1);
		
		readtobone("test.fbx",&all_animation,&root);
		root->set_animations(&all_animation,animmat,animmatsize);
		
			
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/skybox.obj");
		shape->resize();
		shape->init();

		plane = make_shared<Shape>();
		plane->loadMesh(resourceDirectory + "/FA18.obj");
		plane->resize();
		plane->init();

		////generate the VAO
		//glGenVertexArrays(1, &VertexArrayID);
		//glBindVertexArray(VertexArrayID);

		////generate vertex buffer to hand off to OGL
		//glGenBuffers(1, &VertexBufferID);
		////set the current state to focus on our vertex buffer
		//glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		//
		//vector<vec3> pos;
		//vector<unsigned int> imat;
		//root->write_to_VBOs(vec3(0, 0, 0), pos, imat);
		//size_stick = pos.size();
		////actually memcopy the data - only do this once
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*pos.size(), pos.data(), GL_DYNAMIC_DRAW);
		//glEnableVertexAttribArray(0);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		////indices of matrix:
		//glGenBuffers(1, &VertexBufferIDimat);
		//glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDimat);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(uint)*imat.size(), imat.data(), GL_DYNAMIC_DRAW);
		//glEnableVertexAttribArray(1);
		//glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 0, (void*)0);

		//glBindVertexArray(0);
			
		//set up vbo and vao for particles
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0,  1.0,//LD
			1.0, -1.0,  1.0,//RD
			1.0,  1.0,  1.0,//RU
			-1.0,  1.0,  1.0,//LU
		};
		//make it a bit smaller
		for (int i = 0; i < 12; i++)
			cube_vertices[i] *= 0.5;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		GLfloat cube_norm[] = {
			// front colors
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,

		};
		glGenBuffers(1, &VertexNormDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		glm::vec2 cube_tex[] = {
			// front colors
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(0.0, 0.0),

		};
		glGenBuffers(1, &VertexTexBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {

			// front
			0, 1, 2,
			2, 3, 0,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);


		

	

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/retro_mushroom_super_3.png";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		//texture 2
		str = resourceDirectory + "/grid.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		smoothrender.init();
		linerender.init();
		line.push_back(vec3(0,0,-3));
		line.push_back(vec3(5,-1,-3));
		line.push_back(vec3(5,3,-8));
		line.push_back(vec3(2,2,-10));
		line.push_back(vec3(-3,-2,-15));
		line.push_back(vec3(0,1.5,-20));
		linerender.re_init_line(line);

		//prepare stars
		mystar1.pos = vec3(0, 0.7, -4);
		mystar1.speed = vec3(0, 15, 0); //speeds should be perpendicular so they go into orbit
		mystar2.pos = vec3(0, -0.7, -4);
		mystar2.speed = vec3(15, 0, 0);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("Manim");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertimat");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		



		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

		pplane = std::make_shared<Program>();
		pplane->setVerbose(true);
		pplane->setShaderNames(resourceDirectory + "/plane_vertex.glsl", resourceDirectory + "/plane_frag.glsl");
		if (!pplane->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
			}
		pplane->addUniform("P");
		pplane->addUniform("V");
		pplane->addUniform("M");
		pplane->addUniform("campos");
		pplane->addAttribute("vertPos");
		pplane->addAttribute("vertNor");
		pplane->addAttribute("vertTex");
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/

	
	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();
		static double totaltime_ms=0;
		totaltime_ms += frametime*1000.0;
		static double totaltime_untilframe_ms = 0;
		totaltime_untilframe_ms += frametime*1000.0;

		for (int ii = 0; ii < 200; ii++)
			animmat[ii] = mat4(1);


		//animation frame system
		int anim_step_width_ms = 8490 / 204;
		static int frame = 0;
		if (totaltime_untilframe_ms >= anim_step_width_ms)
			{
			totaltime_untilframe_ms = 0;
			frame++;
			}
		root->play_animation(frame,"axisneurontestfile_Avatar00");	//name of current animation	


		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);		
		if (width < height)
			{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * SSky;

		// Draw the sky using GLSL.
		psky->bind();		
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		shape->draw(psky, false);			//render!!!!!!!
		glEnable(GL_DEPTH_TEST);	
		psky->unbind();

		



		// Draw the plane using GLSL.
		glm::mat4 TransPlane = glm::translate(glm::mat4(1.0f), vec3(0,0,-3));
		glm::mat4 SPlane = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
		sangle = -3.1415926 / 2.;
		glm::mat4 RotateXPlane = glm::rotate(glm::mat4(1.0f), sangle, vec3(1,0,0));
		
		mat4 anim = test_animation();
		M = TransPlane*anim*RotateXPlane;

		pplane->bind();
		glUniformMatrix4fv(pplane->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pplane->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pplane->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pplane->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		//plane->draw(pplane, false);			//render!!!!!!!
		pplane->unbind();




		//-------- stuff for gravity btwn 2 spheres -------//
		prog->bind();
		

		//---------- 2x spheres rendering ------------------------////

		//particle collisions logic, starts are initialzed in initGeom
		static float m1 = 1, m2 = 2, G = 1E-1;
		glm::vec3 dist = glm::vec3(mystar1.pos - mystar2.pos);
		float r2 = length(dist) * length(dist);
		dist = normalize(dist);
		vec3 F = dist * ((G * m1 * m2) / r2);
		mystar1.speed -= F / m1;  // set them in opposite directions
		mystar2.speed += F / m2;

		mystar1.pos += mystar1.speed * (float)frametime * (float)0.01;
		mystar2.pos += mystar2.speed * (float)frametime * (float)0.01;

		//send the matrices to the shaders, old bone stuff
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);	
		glBindVertexArray(VertexArrayID);
		//actually draw from vertex 0, 3 vertices
		//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		mat4 Vi = glm::transpose(V);
		Vi[0][3] = 0;
		Vi[1][3] = 0;
		Vi[2][3] = 0;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);

		

		glm::mat4 TransS = glm::translate(glm::mat4(1.0f), mystar1.pos);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.1));
		M = TransS * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(prog->getUniform("Manim"), 200, GL_FALSE, &animmat[0][0][0]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0); //actually draw the 1st particle
		

		//draw the 2nd particle
		TransS = glm::translate(glm::mat4(1.0f), mystar2.pos);
		S = glm::scale(glm::mat4(1.0f), glm::vec3(0.1));
		M = TransS * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(prog->getUniform("Manim"), 200, GL_FALSE, &animmat[0][0][0]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0); //actually draw the 2nd particle

		glBindVertexArray(0);
		prog->unbind();

		




	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
