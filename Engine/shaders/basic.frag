#version 450

layout(location = 0) in vec3 normal;

layout(location = 0) out vec4 colorOut;

const vec3 lightDir = normalize(vec3(1, 1, 1));

void main() 
{
    float lightAmt = clamp(dot(lightDir, normal), 0, 1);
    colorOut = vec4(vec3(lightAmt), 1);
}
