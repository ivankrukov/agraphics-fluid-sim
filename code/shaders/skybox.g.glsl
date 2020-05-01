/*
 *  CSCI 444, Advanced Computer Graphics, Spring 2019
 *
 *  Project: lab6
 *  File: skybox.g.glsl
 *
 *  Description:
 *      Lab6
 *
 *  Author:
 *      Dr. Jeffrey Paone, Colorado School of Mines
 *
 *  Notes:
 *
 */

// we are using OpenGL 4.1 Core profile
#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

// ***** GEOMETRY SHADER UNIFORMS *****
uniform vec3 eyePosition;
uniform vec3 lookAtPoint;
uniform vec3 upVector;
uniform float aspectRatio;

// ***** GEOMETRY SHADER INPUT *****

// ***** GEOMETRY SHADER OUTPUT *****
out vec3 texCoord;

// ***** GEOMETRY SHADER SUBROUTINES *****

void main() {

	vec3 basisX, basisY, basisZ;
	basisZ = normalize(eyePosition - lookAtPoint);
	basisX = normalize(cross(basisZ, upVector));
	basisY = normalize(cross(basisX, basisZ));

	gl_Position = vec4(-1, 1, -1, 1);
	texCoord = -aspectRatio * basisX + basisY - basisZ;
	

	EmitVertex();

	gl_Position = vec4(-1, -1, -1, 1);
	texCoord = -aspectRatio * basisX - basisY - basisZ;

	EmitVertex();
	gl_Position = vec4(1, 1, -1, 1);
	texCoord = aspectRatio * basisX + basisY - basisZ;

	EmitVertex();
	gl_Position = vec4(1, -1, -1, 1);
	texCoord = aspectRatio * basisX - basisY - basisZ;
	
	EmitVertex();
	EndPrimitive();

}
