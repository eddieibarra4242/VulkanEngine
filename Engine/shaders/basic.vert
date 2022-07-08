#version 450

layout(location = 0) in vec3 position;

const vec3[3] colors = {
    vec3(1, 0, 0),
    vec3(0, 0, 1),
    vec3(0, 1, 0)
};

layout(location = 0) out vec3 color;

layout(set = 0, binding = 0) uniform CameraInfo
{
    mat4 vp;
} cameraInfo;

void main() 
{
    gl_Position = cameraInfo.vp * vec4(position, 1);
    color = colors[gl_VertexIndex % 3];
}