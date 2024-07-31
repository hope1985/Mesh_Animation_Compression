//#version 330 core
//out vec4 FragColor;

//uniform vec3 ambientColor;
//uniform vec3 objectColor;

//void main() {
//    vec3 ambient = ambientColor * objectColor;
//    FragColor = vec4(ambient, 1.0);
//}

//#version 330 core
//out vec4 FragColor;
//
//in vec3 FragPos;
//
//float random(vec2 co) {
//    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
//}
//
//void main() {
//    // Generate a random vector
//    vec3 randomVector = normalize(vec3(random(FragPos.xy), random(FragPos.yz), random(FragPos.xz)));
//
//    // Output the random vector as color (for visualization)
//    FragColor = vec4(randomVector, 1.0);
//}

#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

//uniform vec3 lightPos;
//uniform vec3 viewPos;
uniform vec3 lightColor;
//uniform vec3 objectColor;

void main() {
    // Ambient
    //float ambientStrength = 0.1;
    //vec3 ambient = ambientStrength * lightColor;
    float c=(abs(FragPos.x)+abs(FragPos.y)+abs(FragPos.z))/3;;
    FragColor =vec4(2*c,3*c,0.5*c, 1.0); //vec4(ambient* objectColor, 1.0);

    // Diffuse
    //vec3 lightDir = normalize(lightPos - FragPos);
    //float diff = max(dot(Normal, lightDir), 0.0);
    //vec3 diffuse = diff * lightColor;
    
    // Specular
    //float specularStrength = 0.5;
    //vec3 viewDir = normalize(viewPos - FragPos);
    //vec3 reflectDir = reflect(-lightDir, Normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    //vec3 specular = specularStrength * spec * lightColor;

    // Final color
    //vec3 result = (ambient + diffuse + specular) * objectColor;
    //FragColor = vec4(result, 1.0);
}