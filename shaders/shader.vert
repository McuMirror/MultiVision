#version 330
in vec4 vertexIn;
in vec2 textureIn;
out vec2 textureOut;

uniform mat4 projection; // 增加投影矩阵

void main(void) {
    gl_Position = projection  * vertexIn; // 乘上影矩阵
    textureOut = textureIn;
}
