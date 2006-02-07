uniform sampler2D tex;

varying mediump vec4 outColor;
varying mediump vec2 texcoord;

void main() {
	mediump vec4 texcolor;
	texcolor = texture2D(tex, texcoord);
	gl_FragColor = (texcolor + outColor) * mediump vec4(0.5);
}
