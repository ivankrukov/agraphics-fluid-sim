#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// sampler rects aren't [0, 1]
layout(location=1) uniform int texScale;

out vec2 texCoord;

void main() {
    gl_Position = vec4( 1.0, 1.0, 0.5, 1.0 );
    texCoord = vec2( 1.0, 1.0 ) * texScale;
    EmitVertex();

    gl_Position = vec4(-1.0, 1.0, 0.5, 1.0 );
    texCoord = vec2( 0.0, 1.0 ) * texScale; 
    EmitVertex();

    gl_Position = vec4( 1.0,-1.0, 0.5, 1.0 );
    texCoord = vec2( 1.0, 0.0 ) * texScale; 
    EmitVertex();

    gl_Position = vec4(-1.0,-1.0, 0.5, 1.0 );
    texCoord = vec2( 0.0, 0.0 ); 
    EmitVertex();

    EndPrimitive(); 
}
