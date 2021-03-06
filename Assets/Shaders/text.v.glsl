#version 330 core 

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec4 vertexColor;
layout (location = 2) in vec2 vertexUV; 

out DATA
{
	vec2 position;
	vec4 color;
	vec2 uv; 
}fs_out;

uniform mat4 projection;

void main() {

    gl_Position = projection * vec4(vertexPosition, 0.0, 1.0);

	fs_out.position = vertexPosition; 
	fs_out.color = vec4(vertexColor.x, vertexColor.y, vertexColor.z, vertexColor.w); 
	fs_out.uv = vec2(vertexUV.x, -vertexUV.y); 
}

