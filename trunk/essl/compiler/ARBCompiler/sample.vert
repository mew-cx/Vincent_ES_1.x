attribute vec4 texcoord;
uniform sampler2D texsampler;

void main() {
	texture2D(texsampler, texcoord.xy);
}
