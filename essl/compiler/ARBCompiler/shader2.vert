//
//Copyright (C) 2005-2006  Falanx Microsystems AS
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of Falanx Microsystems AS nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//
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
