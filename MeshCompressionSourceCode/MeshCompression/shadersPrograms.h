#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Vertex shader source code
static const char* vsCamera = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
})";


static const char* fsCamera = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
})";


static const char* fsTransform = R"(
#version 330 core
out vec4 FragColor;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
})";


static const char* vsTransform = R"(
#version 330 core
layout(location = 0) in vec3 aPos;


uniform mat4 transform;

void main()
{
	gl_Position = transform * vec4(aPos, 1.0);

})";