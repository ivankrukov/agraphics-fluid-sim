/*
 *  CSCI 444, Advanced Computer Graphics, Spring 2019
 *
 *  Project: lab1
 *  File: main.cpp
 *
 *  Description:
 *      Gives the starting point to draw a ground plane with a 
 *		pass through color shader.  Also renders text to the screen
 *
 *  Author:
 *      Dr. Jeffrey Paone, Colorado School of Mines
 *  
 *  Notes:
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

#include <deque>

#include <CSCI441/OpenGLUtils3.hpp>
#include <CSCI441/ShaderUtils3.hpp>
#include <CSCI441/ShaderProgram3.hpp>

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

// specify our Ground Vertex Information
const Vertex groundVertices[] = {
		{ -15.0f, -5.0f, -15.0f, 0.0f, 1.0f, 0.0f }, // 0 - BL
		{  15.0f, -5.0f, -15.0f, 1.0f, 0.0f, 0.0f }, // 1 - BR
		{  15.0f, -5.0f,  15.0f, 0.0f, 1.0f, 1.0f }, // 2 - TR
		{ -15.0f, -5.0f,  15.0f, 0.0f, 0.0f, 1.0f }  // 3 - TL
};
// specify our Ground Index Ordering
const GLushort groundIndices[] = {
	0, 2, 1, 	0, 3, 2
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

glm::vec3 cameraAngles( 1.82f, 2.01f, 15.0f );
glm::vec3 eyePoint(   10.0f, 10.0f, 10.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );

CSCI441::ShaderProgram *shaderShaderProgram = NULL;
GLuint ray_shader, ray_program;

const GLuint CUBE = 0, GROUND = 1;
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

struct TextShaderAttributeLocations {
	GLint text_texCoord_location;
} textShaderAttribLocs;

GLboolean mackHack = false;

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

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

// load our shaders and get locations for uniforms and attributes
void setupShaders() {
	// load our shader program
	shaderShaderProgram        		   = new CSCI441::ShaderProgram( "shaders/colorPassThrough.v.glsl", 
																	 "shaders/colorPassThrough.f.glsl" );

	// query all of our uniform locations
	shaderUniformLocs.mvpMatrix        = shaderShaderProgram->getUniformLocation( "MVP_Matrix" );
	
	// query all of our attribute locations
	shaderAttribLocs.position          = shaderShaderProgram->getAttributeLocation( "vPos" );
	shaderAttribLocs.color			   = shaderShaderProgram->getAttributeLocation("color");	
	
	textShaderProgram        = new CSCI441::ShaderProgram( "shaders/textShaderv410.v.glsl",
														   "shaders/textShaderv410.f.glsl" );
	textShaderUniformLocs.text_texture_location    = textShaderProgram->getUniformLocation( "tex" );
	textShaderUniformLocs.text_color_location      = textShaderProgram->getUniformLocation( "color" );
	textShaderUniformLocs.text_mvp_location        = textShaderProgram->getUniformLocation( "MVP_Matrix" );
	textShaderAttribLocs.text_texCoord_location    = textShaderProgram->getAttributeLocation( "coord" );	
	// TODO load in shader source
	ray_shader = glCreateShader(GL_COMPUTE_SHADER);
	std::string shaderSource;
	glShaderSource(ray_shader, 1, shaderSource.c_str(), NULL);
	glCompileShader(ray_shader);

	GLuint ray_program = glCreateProgram();
	glAttachShader(ray_progarm, ray_shader);
	glLinkProgram(ray_program);
	// TODO validate
}

// load in our model data to VAOs and VBOs
void setupBuffers() {
	// generate our vertex array object descriptors
	glGenVertexArrays( 2, vaods );

	// will be used to stroe VBO descriptors for ARRAY_BUFFER and ELEMENT_ARRAY_BUFFER
	GLuint vbods[2];

	//------------ BEGIN CUBE VAO ------------

	// bind our Cube VAO
	glBindVertexArray( vaods[CUBE] );

	// generate our vertex buffer object descriptors for the CUBE
	glGenBuffers( 2, vbods );
	// bind the VBO for our Cube Array Buffer
	glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
	// send the data to the GPU
	glBufferData( GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW );

	// bind the VBO for our Cube Element Array Buffer
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
	// send the data to the GPU
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW );

	// enable our position attribute
	glEnableVertexAttribArray( shaderAttribLocs.position );
	// map the position attribute to data within our buffer
	glVertexAttribPointer( shaderAttribLocs.position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) 0 );
	glEnableVertexAttribArray(shaderAttribLocs.color);
	glVertexAttribPointer(shaderAttribLocs.color, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) (sizeof(float) * 3));

	

	//------------  END  CUBE VAO ------------

	//------------ BEGIN GROUND VAO ------------

	// Draw Ground
	glBindVertexArray( vaods[GROUND] );

	// generate our vertex buffer object descriptors for the GROUND
	glGenBuffers( 2, vbods );
	// bind the VBO for our Ground Array Buffer
	glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
	// send the data to the GPU
	glBufferData( GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW );

	// bind the VBO for our Ground Element Array Buffer
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
	// send the data to the GPU
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW );

	// enable our position attribute
	glEnableVertexAttribArray( shaderAttribLocs.position );
	// map the position attribute to data within our buffer
	glVertexAttribPointer( shaderAttribLocs.position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) 0 );
	glEnableVertexAttribArray(shaderAttribLocs.color);
	glVertexAttribPointer(shaderAttribLocs.color, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) (sizeof(float) * 3));

	

	//------------  END  GROUND VAO------------
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

// handles drawing everything to our buffer
void renderScene( GLFWwindow *window ) {
	GLfloat time = glfwGetTime();

	// query our current window size, determine the aspect ratio, and set our viewport size
	GLfloat ratio;
	ratio = windowWidth / (GLfloat) windowHeight;
	glViewport(0, 0, windowWidth, windowHeight);

	// create our Model, View, Projection matrices
	glm::mat4 mMtx, vMtx, pMtx;
	mMtx = glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

	// compute our projection matrix
	pMtx = glm::perspective( 45.0f, ratio, 0.001f, 100.0f );
	// compute our view matrix based on our current camera setup
	vMtx = glm::lookAt( eyePoint,lookAtPoint, upVector );

	// precompute the modelview matrix
	glm::mat4 mvMtx = vMtx * mMtx;
	// precompute the modelviewprojection matrix
	glm::mat4 mvpMtx = pMtx * mvMtx;

	// use our Shading Program
	shaderShaderProgram->useProgram();

	// set all the dynamic uniform values
	glUniformMatrix4fv( shaderUniformLocs.mvpMatrix, 	   1, GL_FALSE, &mvpMtx[0][0] );

	// bind our Ground VAO
	glBindVertexArray( vaods[GROUND] );
	// draw our ground!
	glDrawElements( GL_TRIANGLES, sizeof(groundIndices)/sizeof(unsigned short), GL_UNSIGNED_SHORT, (void*)0 );

	// apply a rotation based on time to our Model matrix
	mMtx = glm::rotate( mMtx, time, glm::vec3( sin(time), cos(time), sin(time)-cos(time) ) );

	// update our modelview matrix
	mvMtx = vMtx * mMtx;
	// update our modelviewprojection matrix
	mvpMtx = pMtx * mvMtx;

	// update our dynamic uniform values
	glUniformMatrix4fv( shaderUniformLocs.mvpMatrix, 1, GL_FALSE, &mvpMtx[0][0] );
	
	// bind our Cube VAO
	glBindVertexArray( vaods[CUBE] );
	// draw our cube!
	glDrawElements( GL_TRIANGLES, sizeof(cubeIndices)/sizeof(unsigned short), GL_UNSIGNED_SHORT, (void*)0 );
}

void setupComputeShaders() {
	int tex_w = 512, tex_h = 512;
	GLuint tex_output;
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);

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
	setupFonts();						// load our fonts into memory
	setupComputeShaders();

	convertSphericalToCartesian();		// position our camera in a pretty place

	GLdouble lastTime = glfwGetTime();
	GLuint nbFrames = 0;
	GLdouble fps = 0;
	std::deque<GLdouble> fpsAvgs(9);
	GLdouble fpsAvg = 0;
	
	// as long as our window is open
	while( !glfwWindowShouldClose(window) ) {
		// clear the prior contents of our buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
		// compute shader work
		glUseProgram(ray_program);
		glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);
		
		// put in a barrier to wait for compute shaders to finish
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

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
		sprintf( fpsStr, "%.3f frames/sec (Avg: %.3f)", fps, fpsAvg);
		render_text(fpsStr, face, -1 + 8 * sx,   1 - 30 * sy, sx, sy);
		
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
