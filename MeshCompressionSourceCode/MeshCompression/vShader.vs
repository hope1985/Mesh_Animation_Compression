#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = aPos;  // Pass the fragment position to the fragment shader
    Normal =vec3(0.0,1.0,0.0);
}