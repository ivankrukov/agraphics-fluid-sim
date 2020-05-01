/*
 *  CSCI 444, Advanced Computer Graphics, Spring 2019
 *
 *  Project: lab6
 *  File: skybox.f.glsl
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

// ***** FRAGMENT SHADER UNIFORMS *****
uniform samplerCube cubeMap;

// ***** FRAGMENT SHADER INPUT *****
in vec3 texCoord;

// ***** FRAGMENT SHADER OUTPUT *****
out vec4 fragColorOut;

// ***** FRAGMENT SHADER SUBROUTINES *****


void main() {
    // set the output color to the interpolated color
    fragColorOut = vec4( gl_FragCoord.x/640.0, gl_FragCoord.y/480.0, 0, 1 );

	fragColorOut = texture(cubeMap, texCoord);
	fragColorOut.a = 1;

	// if viewing the backside of the fragment, 
	// reverse the colors as a visual cue
	if( !gl_FrontFacing ) {
		fragColorOut.rgb = fragColorOut.bgr;
	}
}
