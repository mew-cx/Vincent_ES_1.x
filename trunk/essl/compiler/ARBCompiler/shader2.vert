//
// Jeff Doyle (nfz) July 2004
//

// note: all vectors in object space
uniform mat4 glModelViewProjectionMatrix;
attribute vec4 glVertex;
attribute vec3 glNormal;
attribute vec4 glMultiTexCoord0;

uniform vec3 lightPosition;
uniform vec3 eyePosition;
uniform float passNumber;
uniform float furLength;

varying vec3 LightDir;
varying vec3 Normal;
varying vec3 EyeDir;
varying vec3 Position;
varying vec3 HalfAngle;
varying vec4 TexCoord;

void main(void)
{
    vec4 newPos = glVertex + vec4(glNormal * passNumber * furLength, 0.0);
    gl_Position = glModelViewProjectionMatrix * newPos;
    Position    = newPos.xyz;
    EyeDir      = normalize(eyePosition - newPos.xyz);
    LightDir    = normalize(lightPosition - newPos.xyz);
    HalfAngle   = normalize(LightDir + EyeDir);
    Normal      = glNormal;
    TexCoord    = glMultiTexCoord0;
} 
