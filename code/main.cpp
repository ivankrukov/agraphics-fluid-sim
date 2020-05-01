/*
 *  CSCI 444, Advanced Computer Graphics, Spring 2019
 *
 *  Project: fp
 *  File: main.cpp
 *
 *  Description:
 *		
 *
 *  Author:
 *     Ivan Krukov
 *  
 */
//*************************************************************************************

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include <deque>

#include <CSCI441/OpenGLUtils3.hpp>
#include <CSCI441/ShaderUtils3.hpp>
#include <CSCI441/ShaderProgram3.hpp>
#include <SOIL/SOIL.h>

//*************************************************************************************

// Structure definitions

struct Vertex {
	GLfloat px, py, pz;	// point location x,y,z
	GLfloat r, g, b;	// color r,g,b
};

// specify our Cube Vertex information
const Vertex cubeVertices[] = {
		{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f }, // 0 - bln
		{  0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f }, // 1 - brn
		{  0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f }, // 2 - trn

		{  -0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f }, // 1 - brn
		{  -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f }, // 1 - brn
		{  0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 1.0f }, // 1 - brn
		{  0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f }, // 1 - brn
		{  -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f }, // 1 - brn
};
// specify our Cube Index Ordering
const GLushort cubeIndices[] = {
		0, 2, 1,   
		0, 3, 2,
		1, 2, 5,
		5, 2, 6,
		2, 7, 6,
		3, 7, 2,
		0, 1, 4,
		1, 5, 4,
		4, 5, 6,
		4, 6, 7,
		0, 4, 3,
		4, 7, 3,
};


struct character_info {
  GLfloat ax; // advance.x
  GLfloat ay; // advance.y

  GLfloat bw; // bitmap.width;
  GLfloat bh; // bitmap.rows;

  GLfloat bl; // bitmap_left;
  GLfloat bt; // bitmap_top;

  GLfloat tx; // x offset of glyph in texture coordinates
} font_characters[128];

//*************************************************************************************
//
// Global Parameters

GLint windowWidth, windowHeight;
GLboolean shiftDown = false;
GLboolean leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

// glm::vec3 cameraAngles( 3.52f, 1.835f, 10.82f );
glm::vec3 cameraAngles( 1.99496, 1.740, 14.6 );
glm::vec3 eyePoint(   -4.5f, 3.0f, 10.0f );
glm::vec3 lookAtPoint( 4.0f,  0.0f,  4.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );

CSCI441::ShaderProgram *shaderShaderProgram = NULL;

const GLuint WATER = 0;
const GLuint SKYBOX = 1;
GLuint vaods[2];

struct ShaderUniformLocations {
	GLint mvpMatrix;
} shaderUniformLocs;

struct ShaderAttributeLocations {
	GLint position;
	GLint color;	
} shaderAttribLocs;

FT_Face face;
GLuint font_texture_handle, text_vao_handle, text_vbo_handle;
GLint atlas_width, atlas_height;

CSCI441::ShaderProgram *textShaderProgram = NULL;

struct TextShaderUniformLocations {
	GLint text_texture_location;
	GLint text_color_location;
	GLint text_mvp_location;
} textShaderUniformLocs;

struct TextShaderAttributeLocations { GLint text_texCoord_location;
} textShaderAttribLocs;

GLboolean mackHack = false;

// Compute shader params
// Takes inspiration from http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch38.html

CSCI441::ShaderProgram *skyBoxShaderProgram = NULL;

struct SkyboxShaderUniformLocations {
	GLint eyePosition;
	GLint lookAtPoint;
	GLint upVector;
	GLint aspectRatio;
} skyboxShaderUniformLocs;

GLuint cubeMapTextureHandle;

#define JACOBI_DIFFUSE_ITER 30
#define JACOBI_PRESSURE_ITER 30
int tex_dim = 300;

float waterDim = 8;
float resolution = 30; // resolution points between (0, 0) and (0, 1)
int num_indices;

glm::vec4 black(0.0f, 0.0f, 0.0f, 1.0f);
float last_time = 0; float gridScale = 1;
GLfloat force[] = {10, 10, 0, 0};
GLfloat viscosity = 1.6f;
GLfloat dissipation = 0.99f;
GLfloat heightScale = 2.0f;
// debug -- only apply force to first n frames
bool forceModKey = true; // false -> modify quad rendering; true -> change force applied
bool quadDebug = false;
bool doNavierStokes = true;
bool showWireframe = false;
bool tweakLevelSet = true;

struct calculusShaderLocations {
	GLint subroutine = 0; // name computeValue
	GLuint subJacobi = 0, subGradient = 1, subDivergence = 2;
	GLint updateParameter = 1, sampleParameter = 2, centerSample = 3;
	GLint alpha = 4, rBeta = 5, halfrdx = 6;
} calculusUniforms;

struct boundaryShaderLocations {
	GLint subroutine = 0;
	GLuint subNavierBC = 0, subLevelFixEdge = 1, subLevelFixCenter = 2, subFixFunction = 3;
	GLint updatedValue = 1, stateField = 2, scale = 3, time = 4;
} boundaryUniforms;

struct advectShaderLocations {
	GLint advected = 0, toAdvect = 1, velocity = 2;
	GLint timeStep = 3, gridScale = 4, dissipation = 5;
} advectUniforms;

struct forceShaderLocations {
	GLint subroutine = 0;
	GLuint subRiver = 0, subWhirlpool = 1, subRipple = 2, subInward = 3, subConstant = 4;
	GLuint newVelocity = 0, velocity = 1, force = 2, timeStep = 3;
	GLuint currentSubroutine = subRiver;

	const char* getMode() {
		if ( currentSubroutine == subRiver)
			return "River (const.)";
		else if ( currentSubroutine == subWhirlpool)
			return "Whirlpool";
		else if ( currentSubroutine == subRipple)
			return "Ripple";
		else if ( currentSubroutine == subInward)
			return "Pull-in";
		else if ( currentSubroutine == subConstant)
			return "Fade (no application)";
		else
			return "Unknown";
	}
} forceUniforms;

struct renderShaderLocations {
	// vertex
	GLint vPos = 0;	
	GLint scaleFactors = 0, mvpMatrix = 1, mvMatrix = 2, normalMatrix = 4,
		  mMatrix = 3, worldCamera = 5;
	GLint bumpBinding = 0;
	// frag
	GLint LightUBO = 0, MaterialUBO = 1;
	GLint cubeBinding = 0;

	GLint brdf = 0;
	GLuint subSchlick = 0, subGaussian = 1;
} renderLocations;

#define NUM_FORCES 5

struct quadShaderLocations {
	GLint subroutine = 0;
	GLint texScale = 1;
	GLuint subVelocity = 0, subPressure = 1,
		   subLevelSet = 2, subDebug = 3;
	GLuint currentSubroutine = subLevelSet;

	const char* getMode() {
		if (currentSubroutine == subVelocity)
			return "Velocity";
		else if (currentSubroutine == subPressure)
			return "Pressure";
		else if (currentSubroutine == subLevelSet)
			return "Level Set";
		else if (currentSubroutine == subDebug)
			return "Debug";
		else
			return "Unknown";
	}
	
} quadUniforms;

#define NUM_DISPLAYS 4
// 3.17504 1.75057 11.3601
// -0.373804 2.03132 11.1708


struct Light {
	// glm::vec4 position = glm::vec4(1, 0.2f, 0, 0); // directional
	glm::vec4 position = glm::vec4(-13.0f, 4.0f, -15.6f, 0); // directional
	glm::vec3 L = glm::vec3(1.85f, 1.71f, 1.96f); // TODO check
} lightAttributes;

const char* lightNames[2] = {"LightInfo.position", "LightInfo.L"};
GLuint lightBlockIndex = 0;
GLubyte* lightUniformBuffer;
GLuint lightUBO;
GLint lightOffsets[2];
int lightSize;

struct Material {
	GLfloat rough = 0.43;
	GLboolean metal = true;
	glm::vec3 color = glm::vec3(26/255.0f, 96/255.0f, 195/255.0f);
} materialAttributes;

const char* materialNames[3] = {"MaterialInfo.Rough", "MaterialInfo.Metal", "MaterialInfo.Color"};
GLuint materialBlockIndex = 1;
GLubyte* materialUniformBuffer;
GLuint materialUBO;
GLint materialOffsets[3];
int materialSize;

GLuint quad_program, render_program;

GLuint calculus_shader, calculus_program;
GLuint boundary_shader, boundary_program;
GLuint advect_shader, advect_program;
GLuint force_shader, force_program;
GLuint init_shader, init_program;
GLuint normal_shader, normal_program;

GLuint velocityTextures[2];
GLuint pressureTextures[2];
GLuint debugTexture;
// level sets effectively represent a distance from the center of a cell to
// fluid surface; values < 0 represent cells containing water; air otherwise; 0 = surface
GLuint levelSets[2];
GLuint normalMap;
GLuint calculationTexture; // store intermediary values (i.e divergence)

//*************************************************************************************

// Helper Funcs

void convertSphericalToCartesian() {
	eyePoint.x = cameraAngles.z * sinf( cameraAngles.x ) * sinf( cameraAngles.y );
	eyePoint.y = cameraAngles.z * -cosf( cameraAngles.y );
	eyePoint.z = cameraAngles.z * -cosf( cameraAngles.x ) * sinf( cameraAngles.y );
}

//*************************************************************************************

// GLFW Event Callbacks

// print errors from GLFW
static void error_callback(int error, const char* description) {
	fprintf(stderr, "[ERROR]: %s\n", description);
}

// handle key events
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((key == GLFW_KEY_ESCAPE || key == 'Q') && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	else if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
		shiftDown = true;
	else if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		shiftDown = false;
	else if (action == GLFW_PRESS) {
		if(key == GLFW_KEY_F)
			forceModKey = !forceModKey;
		else if(key == GLFW_KEY_D)
			quadDebug = !quadDebug;
		else if (key == GLFW_KEY_C) {
			if (forceModKey) {
				forceUniforms.currentSubroutine = (forceUniforms.currentSubroutine + 1) % NUM_FORCES;
			} else {
				quadUniforms.currentSubroutine = (quadUniforms.currentSubroutine + 1) % NUM_DISPLAYS;
			}
		} else if (key == GLFW_KEY_SPACE) {
			glUseProgram(boundary_program);
			glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &boundaryUniforms.subLevelFixCenter);
			glBindImageTexture(0, levelSets[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_RECTANGLE, levelSets[0]);
			glUniform1f(boundaryUniforms.scale, tex_dim / 2.0f - 20);
			glDispatchCompute(40, 40, 1);
			std::cout << cameraAngles.x << " " << cameraAngles.y << " " << cameraAngles.z << std::endl;
			std::cout << eyePoint.x << " " << eyePoint.y << " " << eyePoint.z << std::endl;
		} else if (key == GLFW_KEY_P) {
			doNavierStokes = !doNavierStokes;
		} else if (key == GLFW_KEY_W) {
			showWireframe = !showWireframe;
		} else if (key == GLFW_KEY_R) {
			glUseProgram(init_program);
			glBindImageTexture(0, levelSets[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glDispatchCompute(tex_dim, tex_dim, 1);
		} else if (key == GLFW_KEY_S) {
			tweakLevelSet = !tweakLevelSet;
		}
	}
}

// handle mouse clicks
static void mouseClick_callback(GLFWwindow* window, int button, int action, int mods) {
	if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS ) {
		leftMouseDown = true;
	} else {
		leftMouseDown = false;
		mousePosition.x = -9999.0f;
		mousePosition.y = -9999.0f;
	}
}

// handle mouse positions
static void mousePos_callback(GLFWwindow* window, double xpos, double ypos) {
	// make sure movement is in bounds of the window
	// glfw captures mouse movement on entire screen
	if( xpos > 0 && xpos < windowWidth ) {
		if( ypos > 0 && ypos < windowHeight ) {
			// active motion
			if( leftMouseDown ) {
				if( (mousePosition.x - -9999.0f) < 0.001f ) {
					mousePosition.x = xpos;
					mousePosition.y = ypos;
				} else {
					if( !shiftDown ) {
						cameraAngles.x += (xpos - mousePosition.x)*0.005f;
						cameraAngles.y += (ypos - mousePosition.y)*0.005f;

						if( cameraAngles.y < 0 ) cameraAngles.y = 0.0f + 0.001f;
						if( cameraAngles.y >= M_PI ) cameraAngles.y = M_PI - 0.001f;
					} else {
						double totChgSq = (xpos - mousePosition.x) + (ypos - mousePosition.y);
						cameraAngles.z += totChgSq*0.01f;

						if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
						if( cameraAngles.z >= 50.0f ) cameraAngles.z = 50.0f;
					}
					convertSphericalToCartesian();


					mousePosition.x = xpos;
					mousePosition.y = ypos;
				}
			}
			// passive motion
			else {

			}
		}
	}
}

// handle scroll events
static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset ) {
	GLdouble totChgSq = yOffset;
	cameraAngles.z += totChgSq*0.01f;

	if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
	if( cameraAngles.z >= 50.0f ) cameraAngles.z = 50.0f;
	
	convertSphericalToCartesian();
}

//*************************************************************************************

// Setup Funcs

// setup GLFW
GLFWwindow* setupGLFW() {
	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit(EXIT_FAILURE);
	}

	// create a 4.1 Core OpenGL Context
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow *window = glfwCreateWindow(640, 480, "Lab1", NULL, NULL);
	if( !window ) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// register callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouseClick_callback);
	glfwSetCursorPosCallback(window, mousePos_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// return our window
	return window;
}

// setup OpenGL parameters
void setupOpenGL() {
	glEnable( GL_DEPTH_TEST );							// turn on depth testing
	glEnable(GL_TEXTURE_RECTANGLE);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glDepthFunc( GL_LESS );								// use less than test
	glFrontFace( GL_CCW );								// front faces are CCW
	glEnable(GL_BLEND);									// turn on alpha blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// blend w/ 1-a
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );				// clear our screen to black

	// initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum glewResult = glewInit();

	// check for an error
	if( glewResult != GLEW_OK ) {
		printf( "[ERROR]: Error initalizing GLEW\n");
		exit(EXIT_FAILURE);
	}

	// print information about our current OpenGL set up
	CSCI441::OpenGLUtils::printOpenGLInfo();
}

void printLog(GLuint handle) {
	bool sDEBUG = true;
	int status;
    int infologLength = 0;
    int maxLength;
    bool isShader;

    /* check if the handle is to a vertex/fragment shader */
    if( glIsShader( handle ) ) {
        glGetShaderiv(  handle, GL_INFO_LOG_LENGTH, &maxLength );

        isShader = true;
    }
    /* check if the handle is to a shader program */
    else {
        glGetProgramiv( handle, GL_INFO_LOG_LENGTH, &maxLength );

        isShader = false;
    }

    /* create a buffer of designated length */
    char infoLog[maxLength];

    if( isShader ) {
    	glGetShaderiv( handle, GL_COMPILE_STATUS, &status );
    	if( sDEBUG ) printf( "[INFO]: |   Shader  Handle %2d: Compile%-26s |\n", handle, (status == 1 ? "d Successfully" : "r Error") );

        /* get the info log for the vertex/fragment shader */
        glGetShaderInfoLog(  handle, maxLength, &infologLength, infoLog );

        if( infologLength > 0 ) {
			/* print info to terminal */
        	if( sDEBUG ) printf( "[INFO]: |   %s Handle %d: %s\n", (isShader ? "Shader" : "Program"), handle, infoLog );
        }
    } else {
    	glGetProgramiv( handle, GL_LINK_STATUS, &status );
    	if( sDEBUG ) printf("[INFO]: |   Program Handle %2d: Linke%-28s |\n", handle, (status == 1 ? "d Successfully" : "r Error") );

        /* get the info log for the shader program */
        glGetProgramInfoLog( handle, maxLength, &infologLength, infoLog );

        if( infologLength > 0 ) {
			/* print info to terminal */
        	if( sDEBUG ) printf( "[INFO]: |   %s Handle %d: %s\n", (isShader ? "Shader" : "Program"), handle, infoLog );
        }
    }
}

void printLogHead() {
	printf("[INFO]: /--------------------------------------------------------\\\n");
}
void printLogFoot() {
	printf("[INFO]: \\--------------------------------------------------------/\n\n");
}

void readFile( const char* filename, char* &output ){
	std::string buf = std::string("");
	std::string line;

	std::ifstream in( filename );
    while( getline(in, line) ) {
        buf += line + "\n";
    }
    output = new char[buf.length()+1];
    strncpy(output, buf.c_str(), buf.length());
    output[buf.length()] = '\0';

    in.close();
}


GLuint compileShader(const char* fileName, GLenum shaderType) {
	char* shaderSource;
	readFile(fileName, shaderSource);
	GLuint shaderHandle = glCreateShader(shaderType);
	glShaderSource(shaderHandle, 1, (const char**)&shaderSource, NULL);
	delete[] shaderSource;
	glCompileShader(shaderHandle);
	std::cout << "[INFO]: |   Compiling " << fileName << std::endl;
	printLog(shaderHandle);
	return shaderHandle;
}


// load our shaders and get locations for uniforms and attributes
void setupShaders() {
	// load skybox program
	skyBoxShaderProgram = new CSCI441::ShaderProgram( "shaders/skybox.v.glsl",
													  "shaders/skybox.g.glsl",
													  "shaders/skybox.f.glsl" );

	// query all of uniform locations
	skyboxShaderUniformLocs.eyePosition   = skyBoxShaderProgram->getUniformLocation("eyePosition");
	skyboxShaderUniformLocs.lookAtPoint   = skyBoxShaderProgram->getUniformLocation("lookAtPoint");
	skyboxShaderUniformLocs.upVector      = skyBoxShaderProgram->getUniformLocation("upVector");
	skyboxShaderUniformLocs.aspectRatio   = skyBoxShaderProgram->getUniformLocation("aspectRatio");

	
	// text rendering program
	textShaderProgram        = new CSCI441::ShaderProgram( "shaders/text/textShaderv410.v.glsl",
														   "shaders/text/textShaderv410.f.glsl" );
	textShaderUniformLocs.text_texture_location    = textShaderProgram->getUniformLocation( "tex" );
	textShaderUniformLocs.text_color_location      = textShaderProgram->getUniformLocation( "color" );
	textShaderUniformLocs.text_mvp_location        = textShaderProgram->getUniformLocation( "MVP_Matrix" );
	textShaderAttribLocs.text_texCoord_location    = textShaderProgram->getAttributeLocation( "coord" );	

	// programs for Navier stokes and rendering grunt work
	calculus_program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, calculus_program, -1, "Calculus Routines");
	boundary_program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, boundary_program, -1, "Boundary Conditions");
	advect_program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, advect_program, -1, "Advection");
	force_program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, force_program, -1, "Force Application");
	quad_program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, quad_program, -1, "Quad Render Program");
	init_program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, init_program, -1, "Initialization Program");
	normal_program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, normal_program, -1, "Normal Calculator Program");

	render_program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, render_program, -1, "Wave Rendering Program");

	printLogHead();
	GLuint quadV, quadG, quadF;
	quadV = compileShader("shaders/quad/quad.v.glsl", GL_VERTEX_SHADER);
	quadG = compileShader("shaders/quad/quad.g.glsl", GL_GEOMETRY_SHADER);
	quadF = compileShader("shaders/quad/quad.f.glsl", GL_FRAGMENT_SHADER);
	glAttachShader(quad_program, quadV);
	glAttachShader(quad_program, quadG);
	glAttachShader(quad_program, quadF);
	glLinkProgram(quad_program);
	printLog(quad_program);
	printLogFoot();

	printLogHead();
	calculus_shader = compileShader("shaders/fluidsim/calculus.c.glsl", GL_COMPUTE_SHADER);
	glAttachShader(calculus_program, calculus_shader);
	glLinkProgram(calculus_program);
	printLog(calculus_program);
	printLogFoot();

	printLogHead();
	boundary_shader = compileShader("shaders/fluidsim/boundary.c.glsl", GL_COMPUTE_SHADER);
	glAttachShader(boundary_program, boundary_shader);
	glLinkProgram(boundary_program);
	printLog(boundary_program);
	printLogFoot();

	printLogHead();
	advect_shader = compileShader("shaders/fluidsim/advect.c.glsl", GL_COMPUTE_SHADER);
	glAttachShader(advect_program, advect_shader);
	glLinkProgram(advect_program);
	printLog(advect_program);
	printLogFoot();

	printLogHead();
	force_shader = compileShader("shaders/fluidsim/force.c.glsl", GL_COMPUTE_SHADER);
	glAttachShader(force_program, force_shader);
	glLinkProgram(force_program);
	printLog(force_program);
	printLogFoot();

	printLogHead();
	init_shader = compileShader("shaders/fluidsim/init.c.glsl", GL_COMPUTE_SHADER);
	glAttachShader(init_program, init_shader);
	glLinkProgram(init_program);
	printLog(init_program);
	printLogFoot();

	printLogHead();
	normal_shader = compileShader("shaders/fluidsim/normals.c.glsl", GL_COMPUTE_SHADER);
	glAttachShader(normal_program, normal_shader);
	glLinkProgram(normal_program);
	printLog(normal_program);
	printLogFoot();

	printLogHead();
	GLuint renderV, renderF;
	renderV = compileShader("shaders/render/fluid.v.glsl", GL_VERTEX_SHADER);
	glAttachShader(render_program, renderV);
	renderF = compileShader("shaders/render/fluid.f.glsl", GL_FRAGMENT_SHADER);
	glAttachShader(render_program, renderF);
	glLinkProgram(render_program);
	printLog(render_program);
	printLogFoot();
}

void setupUniformBlocks() {
	// light
	glGetActiveUniformBlockiv(render_program, lightBlockIndex,
			GL_UNIFORM_BLOCK_DATA_SIZE, &lightSize);
	lightUniformBuffer = (GLubyte*) malloc(lightSize);

	GLuint lightIndices[2];
	glGetUniformIndices(render_program, 2, lightNames, lightIndices);
	glGetActiveUniformsiv(render_program, 2, lightIndices, GL_UNIFORM_OFFSET, lightOffsets);

	memcpy(lightUniformBuffer + lightOffsets[0], &lightAttributes.position, sizeof(glm::vec4));
	memcpy(lightUniformBuffer + lightOffsets[1], &lightAttributes.L, sizeof(glm::vec3));

	glGenBuffers(1, &lightUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
	glBufferData(GL_UNIFORM_BUFFER,
			lightSize,
			lightUniformBuffer,
			GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, lightBlockIndex, lightUBO);

	// material
	glGetActiveUniformBlockiv(render_program, materialBlockIndex,
			GL_UNIFORM_BLOCK_DATA_SIZE, &materialSize);
	materialUniformBuffer = (GLubyte*) malloc(materialSize);

	GLuint materialIndices[3];
	glGetUniformIndices(render_program, 3, materialNames, materialIndices);
	glGetActiveUniformsiv(render_program, 3, materialIndices, GL_UNIFORM_OFFSET, materialOffsets);

	memcpy(materialUniformBuffer + materialOffsets[0], &materialAttributes.rough, sizeof(GLfloat));
	memcpy(materialUniformBuffer + materialOffsets[1], &materialAttributes.metal, sizeof(GLboolean));
	memcpy(materialUniformBuffer + materialOffsets[2], &materialAttributes.color, sizeof(glm::vec3));

	glGenBuffers(1, &materialUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
	glBufferData(GL_UNIFORM_BUFFER,
			materialSize,
			materialUniformBuffer,
			GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, materialBlockIndex, materialUBO);
}

// load in our model data to VAOs and VBOs
void setupBuffers() {
	// generate our vertex array object descriptors
	glGenVertexArrays( 2, vaods );

	// will be used to stroe VBO descriptors for ARRAY_BUFFER and ELEMENT_ARRAY_BUFFER
	GLuint vbods[2];

	//------------ BEGIN WATER VAO ------------

	// bind our Cube VAO
	glBindVertexArray( vaods[WATER] );

	// generate our vertex buffer object descriptors for the WATER
	glGenBuffers( 2, vbods );
	// bind the VBO for our Cube Array Buffer
	glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );

	// set up a simple plane for vertex deformation
	std::vector<glm::vec2> groundMeshPoints;
	std::vector<unsigned short> groundMeshIndices;
	for (float i = 0; i < waterDim; i+= 1 / resolution) {
		for (float j = 0; j < waterDim; j+= 1 / resolution) {
			groundMeshPoints.push_back(glm::vec2(j, i));	
		}
	}

	int point_dim = waterDim * resolution + 1;
	for (int i = 0; i < point_dim - 1; i++) {
		if (i % 2 == 0) {
			for (int j = 0; j < point_dim; j++) {
				groundMeshIndices.push_back(j + i * point_dim);
				groundMeshIndices.push_back(j + (i + 1) * point_dim);
			}
		} else {
			for (int j = point_dim - 1; j > 0; j-- ) {
				groundMeshIndices.push_back(j + (i + 1) * point_dim);
				groundMeshIndices.push_back(j - 1 + i * point_dim);
			}
		}
	}
	num_indices = groundMeshIndices.size();
	// send the data to the GPU
	glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec2) * groundMeshPoints.size(), &groundMeshPoints[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * num_indices, &groundMeshIndices[0], GL_STATIC_DRAW );

	// map the position attribute to data within our buffer
	glEnableVertexAttribArray(renderLocations.vPos);
	glVertexAttribPointer(renderLocations.vPos, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

	// skybox vao
	glm::vec3 skyboxPoint = glm::vec3(0.0, 0.0, 0.0);

	// Draw Ground
	glBindVertexArray( vaods[SKYBOX] );

	// generate our vertex buffer object descriptors for the GROUND
	glGenBuffers( 2, vbods );
	// bind the VBO for our Ground Array Buffer
	glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
	// send the data to the GPU
	glBufferData( GL_ARRAY_BUFFER, sizeof(skyboxPoint), &skyboxPoint[0], GL_STATIC_DRAW );

	// enable our position attribute
	glEnableVertexAttribArray( shaderAttribLocs.position );
	// map the position attribute to data within our buffer
	glVertexAttribPointer( shaderAttribLocs.position, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

}

void setupSkybox() {
	// cubeMapTextureHandle = SOIL_load_OGL_cubemap(
	// 		"textures/colosseum/left.jpg",
	// 		"textures/colosseum/right.jpg",
	// 		"textures/colosseum/top.jpg",
	// 		"textures/colosseum/bottom.jpg",
	// 		"textures/colosseum/back.jpg",
	// 		"textures/colosseum/front.jpg",
	// 		SOIL_LOAD_RGB,
	// 		SOIL_CREATE_NEW_ID,
	// 		SOIL_FLAG_MIPMAPS);
	cubeMapTextureHandle = SOIL_load_OGL_cubemap(
			"textures/lakeMountains/right.png",
			"textures/lakeMountains/left.png",
			"textures/lakeMountains/top.png",
			"textures/lakeMountains/bottom.png",
			"textures/lakeMountains/back.png",
			"textures/lakeMountains/front.png",
			SOIL_LOAD_RGB,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS);

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureHandle);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void setupFonts() {
	FT_Library ft;

	if(FT_Init_FreeType(&ft)) {
	  fprintf(stderr, "Could not init freetype library\n");
	  exit(EXIT_FAILURE);
	}

	if(FT_New_Face(ft, "fonts/DroidSansMono.ttf", 0, &face)) {
	  fprintf(stderr, "Could not open font\n");
	  exit(EXIT_FAILURE);
	}

	FT_Set_Pixel_Sizes(face, 0, 20);

	FT_GlyphSlot g = face->glyph;
	GLuint w = 0;
	GLuint h = 0;

	for(int i = 32; i < 128; i++) {
	  if(FT_Load_Char(face, i, FT_LOAD_RENDER)) {
	    fprintf(stderr, "Loading character %c failed!\n", i);
	    continue;
	  }

	  w += g->bitmap.width;
	  h = (h > g->bitmap.rows ? h : g->bitmap.rows);
	}

	/* you might as well save this value as it is needed later on */
	atlas_width = w;
	atlas_height = h;

	glEnable( GL_TEXTURE_2D );
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &font_texture_handle);
	glBindTexture(GL_TEXTURE_2D, font_texture_handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

	GLint x = 0;

	for(int i = 32; i < 128; i++) {
	  if(FT_Load_Char(face, i, FT_LOAD_RENDER))
	    continue;

	  font_characters[i].ax = g->advance.x >> 6;
	  font_characters[i].ay = g->advance.y >> 6;

	  font_characters[i].bw = g->bitmap.width;
	  font_characters[i].bh = g->bitmap.rows;

	  font_characters[i].bl = g->bitmap_left;
	  font_characters[i].bt = g->bitmap_top;

	  font_characters[i].tx = (float)x / w;

	  glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

	  x += g->bitmap.width;
	}

	glGenVertexArrays(1, &text_vao_handle);
	glBindVertexArray(text_vao_handle);

	glGenBuffers(1, &text_vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, text_vbo_handle);
	glEnableVertexAttribArray(textShaderAttribLocs.text_texCoord_location);
	glVertexAttribPointer(textShaderAttribLocs.text_texCoord_location, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

void render_text(const char *text, FT_Face face, float x, float y, float sx, float sy) {
	struct point {
		GLfloat x;
		GLfloat y;
		GLfloat s;
		GLfloat t;
	} coords[6 * strlen(text)];

	GLint n = 0;

	for(const char *p = text; *p; p++) {
		int characterIndex = (int)*p;

		character_info character = font_characters[characterIndex];

		GLfloat x2 =  x + character.bl * sx;
		GLfloat y2 = -y - character.bt * sy;
		GLfloat w = character.bw * sx;
		GLfloat h = character.bh * sy;

		/* Advance the cursor to the start of the next character */
		x += character.ax * sx;
		y += character.ay * sy;

		/* Skip glyphs that have no pixels */
		if(!w || !h)
			continue;

		coords[n++] = (point){x2,     -y2    , character.tx,                                0};
		coords[n++] = (point){x2 + w, -y2    , character.tx + character.bw / atlas_width,   0};
		coords[n++] = (point){x2,     -y2 - h, character.tx,                                character.bh / atlas_height}; //remember: each glyph occupies a different amount of vertical space

		coords[n++] = (point){x2 + w, -y2    , character.tx + character.bw / atlas_width,   0};
		coords[n++] = (point){x2,     -y2 - h, character.tx,                                character.bh / atlas_height};
		coords[n++] = (point){x2 + w, -y2 - h, character.tx + character.bw / atlas_width,   character.bh / atlas_height};
	}

	glBindVertexArray(text_vao_handle);
	glBindBuffer(GL_ARRAY_BUFFER, text_vbo_handle);
	glBufferData(GL_ARRAY_BUFFER, sizeof( coords ), coords, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, n);
}

void clearTexture(GLint texture) {
	glClearTexImage(texture, 0, GL_RGBA, GL_FLOAT, &black);
}

/*
 *	Swaps the texture units between reading/writing
 *	textures[0] swaps with textures[1];
 *	On completion, the swap associates [0] as the texture for reads and [1] for writes
 */
void swapAndBindTexUnits(GLuint textures[2], GLuint readTex, GLuint writeTex) {
	GLuint temp = textures[0];
	textures[0] = textures[1];
	textures[1] = temp;
	glActiveTexture(GL_TEXTURE0 + readTex);
	glBindTexture(GL_TEXTURE_RECTANGLE, textures[0]);
	glBindImageTexture(writeTex, textures[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
}


void advect(GLuint quantity[2], GLfloat timeDelta) {
	glUseProgram(advect_program);

	// setup RW textures (advecting velocity)
	swapAndBindTexUnits(quantity, 1, 0);	
	glBindImageTexture(1, debugTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, velocityTextures[0]);

	//set timestep/grid scale
	glUniform1f(advectUniforms.timeStep, timeDelta);
	glUniform1f(advectUniforms.gridScale, 1 / gridScale);
	glUniform1f(advectUniforms.dissipation, dissipation);

	glDispatchCompute(tex_dim, tex_dim, 1);
	// glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void diffuse(GLfloat timeDelta) {
	// perform jacobi iteration to approximate the Poisson equation for diffusion
	glUseProgram(calculus_program);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &calculusUniforms.subJacobi);
	float rdx = 1.0f / gridScale;
	// (dx)^2/(v * dt)
	float alpha = (rdx * rdx) / (viscosity * timeDelta);
	glUniform1f(calculusUniforms.alpha, alpha);
	// 1/(4 + alpha)
	glUniform1f(calculusUniforms.rBeta, 1.0f / (4.0f + alpha));
	// glUniform1f(calculusUniforms.halfrdx, 0.5f / gridScale);
	
	// x, b = velocity texture unit (sampler2Ds)
	// output to velocity
	for (int i = 0; i < JACOBI_DIFFUSE_ITER; i++) {
		swapAndBindTexUnits(velocityTextures, 1, 0);	
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_RECTANGLE, velocityTextures[0]);

		glDispatchCompute(tex_dim, tex_dim, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

void applyForce(GLfloat timeDelta) {
	glUseProgram(force_program);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &forceUniforms.currentSubroutine);

	swapAndBindTexUnits(velocityTextures, 1, 0);
	// TODO move this to a one time call on force value updates
	glUniform4fv(forceUniforms.force, 1, force);
	glUniform1f(forceUniforms.timeStep, timeDelta);

	glDispatchCompute(tex_dim, tex_dim, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// apply pressure and subtract pressure gradient
void projection(GLfloat timeDelta) {
	// solve poisson pressure
	
	// get divergence of velocity
	glUseProgram(calculus_program);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &calculusUniforms.subDivergence);
	glUniform1f(calculusUniforms.halfrdx, 0.5f / gridScale);

	glBindImageTexture(0, calculationTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, velocityTextures[1]);
	// TEXTURE_2 unused in divergence

	glDispatchCompute(tex_dim, tex_dim, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// jacobi iters to solve pressure; x = pressure, b = <div v>
	// alpha = -dx^2; beta = 1/4
	float rdx = 1.0f / gridScale;
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &calculusUniforms.subJacobi);
	glUniform1f(calculusUniforms.alpha, -rdx * rdx);
	glUniform1f(calculusUniforms.rBeta, 1.0f/4.0f);

	// TEXTURE0 and 1 are set on swapAndBind
	// clear out pressure (inital guess of pressure = 0)
	// (I only clear out [1] to optimize; current [0] will get overwritten anyways)
	clearTexture(pressureTextures[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, calculationTexture);
	
	for (int i = 0; i < JACOBI_PRESSURE_ITER; i++) {
		swapAndBindTexUnits(pressureTextures, 1, 0);	

		glDispatchCompute(tex_dim, tex_dim, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		// glActiveTexture(GL_TEXTURE2);
		// glBindTexture(GL_TEXTURE_RECTANGLE, pressureTextures[0]);
	}

	// finally, compute the pressure gradient and subtract it from v
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &calculusUniforms.subGradient);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, pressureTextures[1]);
	swapAndBindTexUnits(velocityTextures, 2, 0);

	glDispatchCompute(tex_dim, tex_dim, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// evaluate boundary conditions
void boundary(GLfloat timeDelta) {
	glUseProgram(boundary_program);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &boundaryUniforms.subNavierBC);
	glUniform1f(boundaryUniforms.scale, 1);
	glUniform1f(boundaryUniforms.time, glfwGetTime());
	swapAndBindTexUnits(velocityTextures, 1, 0);
	glDispatchCompute(tex_dim, 1, 1);
	swapAndBindTexUnits(pressureTextures, 1, 0);
	glUniform1f(boundaryUniforms.scale, -1);
	glDispatchCompute(tex_dim, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

}

void doNavierComputations() {
	GLfloat timeDelta = glfwGetTime() - last_time;
	
	advect(velocityTextures, timeDelta);

	diffuse(timeDelta);

	applyForce(timeDelta);

	projection(timeDelta);

	boundary(timeDelta);

	advect(levelSets, timeDelta);

	if (tweakLevelSet) {
		glUseProgram(boundary_program);
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &boundaryUniforms.subFixFunction);
		glBindImageTexture(0, levelSets[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_RECTANGLE, levelSets[0]);
		glUniform1f(boundaryUniforms.scale, tex_dim / 2.0f - 20);
		glDispatchCompute(tex_dim, 1, 1);
	}

	// calculate normal map from the new level set
	glUseProgram(normal_program);
	glBindImageTexture(0, normalMap, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, levelSets[1]);
	glDispatchCompute(tex_dim, tex_dim, 1);
}

void renderQuad() {
	glUseProgram(quad_program);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &quadUniforms.currentSubroutine);
	glUniform1i(quadUniforms.texScale, tex_dim);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, velocityTextures[1]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, pressureTextures[0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, levelSets[1]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_RECTANGLE, debugTexture);

	glDrawArrays(GL_POINTS, 0, 1);
}

void renderWater() {
	glUseProgram(render_program);

	// query our current window size, determine the aspect ratio, and set our viewport size
	GLfloat ratio;
	ratio = windowWidth / (GLfloat) windowHeight;

	// create our Model, View, Projection matrices
	glm::mat4 mMtx, vMtx, pMtx;
	mMtx = glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

	// compute our projection matrix
	pMtx = glm::perspective( 45.0f, ratio, 0.001f, 1000.0f );
	// compute our view matrix based on our current camera setup
	vMtx = glm::lookAt( eyePoint,lookAtPoint, upVector );

	// precompute the modelview matrix
	glm::mat4 mvMtx = vMtx * mMtx;
	// precompute the modelviewprojection matrix
	glm::mat4 mvpMtx = pMtx * mvMtx;

	// apply a rotation based on time to our Model matrix
	// mMtx = glm::rotate( mMtx, time, glm::vec3( sin(time), cos(time), sin(time)-cos(time) ) );
	mMtx = glm::translate(mMtx, glm::vec3(0, 0, 0));

	// update our modelview matrix
	mvMtx = vMtx * mMtx;
	// update our modelviewprojection matrix
	mvpMtx = pMtx * mvMtx;

	glm::mat3 normalMtx = glm::mat3(mvMtx);

	// update our dynamic uniform values
	glUniformMatrix4fv( renderLocations.mvpMatrix, 1, GL_FALSE, &mvpMtx[0][0] );
	glUniformMatrix4fv( renderLocations.mvMatrix, 1, GL_FALSE, &mvMtx[0][0] );
	glUniformMatrix4fv( renderLocations.mMatrix, 1, GL_FALSE, &mMtx[0][0] );
	glUniformMatrix3fv( renderLocations.normalMatrix, 1, GL_FALSE, &normalMtx[0][0] );

	glUniform3fv(renderLocations.worldCamera, 1, &(eyePoint.x));

	float coordScale = tex_dim / waterDim;
	glUniform3f(renderLocations.scaleFactors, coordScale, coordScale, heightScale);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, normalMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureHandle);

	glUniformSubroutinesuiv(renderLocations.brdf, 1, &renderLocations.subSchlick);

	
	glBindVertexArray( vaods[WATER] );

	// render water
	if (showWireframe) {
		glDrawElements( GL_LINE_STRIP, num_indices, GL_UNSIGNED_SHORT, (void*)0 );
	} else {
		glDrawElements( GL_TRIANGLE_STRIP, num_indices, GL_UNSIGNED_SHORT, (void*)0 );
	}
}

void renderSkybox() {
	GLfloat ratio;
	ratio = windowWidth / (GLfloat) windowHeight;
	glDisable(GL_DEPTH_TEST);
	skyBoxShaderProgram->useProgram();
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureHandle);
	glUniform3fv( skyboxShaderUniformLocs.eyePosition, 1, &eyePoint[0] );
	glUniform3fv( skyboxShaderUniformLocs.lookAtPoint, 1, &lookAtPoint[0] );
	glUniform3fv( skyboxShaderUniformLocs.upVector, 1, &upVector[0] );
	glUniform1f( skyboxShaderUniformLocs.aspectRatio, ratio );

	glBindVertexArray( vaods[SKYBOX] );
	glDrawArrays( GL_POINTS, 0, 1 );
	glEnable(GL_DEPTH_TEST);
}

// handles drawing everything to our buffer
void renderScene( GLFWwindow *window ) {
	// GLfloat time = glfwGetTime();
	if (doNavierStokes) {
		doNavierComputations();
	}
	glViewport(0, 0, windowWidth, windowHeight);
	if (quadDebug) {
		renderQuad();
	} else {
		renderSkybox();
		renderWater();
	}
	last_time = glfwGetTime();
}

void genComputeTexture(GLuint* tex_out, std::string label) {
	glGenTextures(1, tex_out);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, *tex_out);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexStorage2D(GL_TEXTURE_RECTANGLE, 1, GL_RGBA16F, tex_dim, tex_dim);
	glObjectLabel(GL_TEXTURE, *tex_out, -1, label.c_str());
}

void genComputeTextures(GLuint tex_out[2], std::string label) {
	genComputeTexture(&tex_out[0], (label + " (unit 0)"));
	genComputeTexture(&tex_out[1], (label + " (unit 1)"));
}

void setupComputeShaders() {
	genComputeTextures(velocityTextures, "Velocity");
	genComputeTextures(pressureTextures, "Pressure");
	genComputeTextures(levelSets, "Level Set");

	genComputeTexture(&debugTexture, "Debug");
	genComputeTexture(&calculationTexture, "Intermediate Values");
	genComputeTexture(&normalMap, "Normal Map");

	int work_grp_cnt[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
	printf("max global (total) work group size x:%i y:%i z:%i\n",
			work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_size[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
			work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("max local work group invocations %i\n", work_grp_inv);
}

// program entry point
int main( int argc, char *argv[] ) {
	GLFWwindow *window = setupGLFW();	// setup GLFW and get our window
	setupOpenGL();						// setup OpenGL & GLEW
	setupShaders();						// load our shader programs, uniforms, and attribtues
	setupBuffers();						// load our models into GPU memory
	setupUniformBlocks();
	setupFonts();						// load our fonts into memory
	setupComputeShaders();
	setupSkybox();

	convertSphericalToCartesian();		// position our camera in a pretty place

	GLdouble lastTime = glfwGetTime();
	GLuint nbFrames = 0;
	GLdouble fps = 0;
	std::deque<GLdouble> fpsAvgs(9);
	GLdouble fpsAvg = 0;

	glUseProgram(init_program);
	glBindImageTexture(0, levelSets[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(tex_dim, tex_dim, 1);
	
	// as long as our window is open
	while( !glfwWindowShouldClose(window) ) {
		// clear the prior contents of our buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

		// render our scene
		renderScene( window );

		// Measure speed
		GLdouble currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 0.33f ){ // If last prinf() was more than 1 sec ago
			// printf and reset timer
			fps = GLdouble(nbFrames)/(currentTime - lastTime);
			nbFrames = 0;
			lastTime = currentTime;

			fpsAvgs.pop_front();
			fpsAvgs.push_back( fps );

			GLdouble totalFPS = 0;
			for( GLuint i = 0; i < fpsAvgs.size(); i++ ) {
				totalFPS += fpsAvgs.at(i);
			}
			fpsAvg = totalFPS / fpsAvgs.size();
		}

		glBindVertexArray(text_vao_handle);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, font_texture_handle);

		textShaderProgram->useProgram();
		glUniform1i(textShaderUniformLocs.text_texture_location, GL_TEXTURE0);

		glm::mat4 mvp = glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

		glUniformMatrix4fv(textShaderUniformLocs.text_mvp_location, 1, GL_FALSE, &mvp[0][0]);
		
		GLfloat white[4] = {1, 1, 1, 1};
		glUniform4fv(textShaderUniformLocs.text_color_location, 1, white);

		GLfloat sx = 2.0 / (GLfloat)windowWidth;
		GLfloat sy = 2.0 / (GLfloat)windowHeight;

		char fpsStr[80];
		sprintf( fpsStr, "%.3f frames/sec (Avg: %.3f) %s", fps, fpsAvg,
				quadDebug ? "<QUAD MODE>": "");
		render_text(fpsStr, face, -1 + 8 * sx,   1 - 30 * sy, sx, sy);
		char modeStr[200];
		sprintf( modeStr, "Force Mode: %s; Quad Texture: %s; C to change %s",
				forceUniforms.getMode(), quadUniforms.getMode(),
				forceModKey ? "force" : "texture display");
		render_text(modeStr, face, -1 + 8 * sx, 1 - 60 * sy, sx, sy);
		
		// swap the front and back buffers
		glfwSwapBuffers(window);
		// check for any events
		glfwPollEvents();
		
		// the following code is a hack for OSX Mojave
		// the window is initially black until it is moved
		// so instead of having the user manually move the window,
		// we'll automatically move it and then move it back
		if( !mackHack ) {
			GLint xpos, ypos;
			glfwGetWindowPos(window, &xpos, &ypos);
			glfwSetWindowPos(window, xpos+10, ypos+10);
			glfwSetWindowPos(window, xpos, ypos);
			mackHack = true;
		}
	}

	// destroy our window
	glfwDestroyWindow(window);
	// end GLFW
	glfwTerminate();

	// delete our shader program
	delete shaderShaderProgram;

	// SUCCESS!!
	return EXIT_SUCCESS;
}
