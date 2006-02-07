uniform mat4 MVPmatrix;
attribute vec4 vertex;
attribute vec4 vertexColor;
attribute vec3 vertexTexcoord;

varying mediump vec4 outColor;
varying mediump vec2 texcoord;

vec4 testFunc(vec4 param) {
	return abs(param);
}

void main() {
	outColor = testFunc(vertexColor);
	texcoord = vec4(vertexTexcoord, 5).wy;
	gl_Position = MVPmatrix * vertex;
}
