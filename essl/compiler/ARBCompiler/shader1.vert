uniform mat4 glModelViewProjectionMatrix;
attribute vec4 glVertex;

void main() {	
	gl_Position = glModelViewProjectionMatrix * glVertex;
}
