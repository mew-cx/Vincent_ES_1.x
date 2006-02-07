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
