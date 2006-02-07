//
// Jeff Doyle (nfz) July 2004
//

precision mediump float;

uniform vec4 glFrontMaterialambient;
uniform vec4 glFrontMaterialspecular;
uniform vec4 glFrontMaterialdiffuse;
uniform float glFrontMaterialshininess;

uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform sampler2D furTU;

varying vec3 LightDir;
varying vec3 Normal;
varying vec3 EyeDir;
varying vec3 Position;
varying vec3 HalfAngle;
varying vec4 TexCoord;

vec4 specular(vec3 N) {
    return glFrontMaterialspecular * pow(max(dot(normalize(HalfAngle), N), 0.0), glFrontMaterialshininess);
}

vec4 diffuse(vec3 N) {
    return glFrontMaterialdiffuse * max(dot(normalize(LightDir), N), 0.0);
}

void main(void) {
   vec3 N = normalize (Normal);
   vec4 furColor = texture2D(furTU, TexCoord.xy);
   gl_FragColor =  furColor * (Ka * glFrontMaterialambient + Kd * diffuse(N) + Ks * specular(N));
}
