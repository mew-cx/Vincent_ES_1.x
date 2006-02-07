uniform mat4 MVPmatrix;
attribute vec4 vertex;
attribute vec4 vertexColor;
attribute vec2 vertexTexcoord;

varying mediump vec4 outColor;
varying mediump vec2 texcoord;

void main() {
	outColor = vertexColor;
	texcoord = vertexTexcoord;
	gl_Position = MVPmatrix * vertex;
}
