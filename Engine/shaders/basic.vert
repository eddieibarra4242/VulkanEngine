#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 normal;

layout(set = 0, binding = 0) uniform CameraInfo
{
    mat4 vp;
} cameraInfo;

void main() 
{
    gl_Position = cameraInfo.vp * vec4(inPosition, 1);
    normal = inNormal; //TODO: apply model mat
}
