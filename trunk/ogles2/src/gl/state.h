#ifndef GLES_STATE_STATE_H
#define GLES_STATE_STATE_H 1

/*
** ==========================================================================
**
** state.h			GL Rendering State Declarations
**
** --------------------------------------------------------------------------
**
** 06-13-2005		Hans-Martin Will	initial version
**
** --------------------------------------------------------------------------
**
** Copyright (c) 2003-2006, Hans-Martin Will. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
**	 *  Redistributions of source code must retain the above copyright
** 		notice, this list of conditions and the following disclaimer.
**   *	Redistributions in binary form must reproduce the above copyright
** 		notice, this list of conditions and the following disclaimer in the
** 		documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
** OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
** THE POSSIBILITY OF SUCH DAMAGE.
**
** ==========================================================================
*/

#include "framebuffer/framebuffer.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

#define NIL			(~0u)
#define BOUND		(0u)

/*
** --------------------------------------------------------------------------
** Macros
** --------------------------------------------------------------------------
*/

#define GLES_CLAMP(v)  ((v) > 1.0f ? 1.0f : (v) < 0.0f ? 0.0f : (v))

/*
** --------------------------------------------------------------------------
** Geometry Types
** --------------------------------------------------------------------------
*/
typedef struct Vec2f {
	GLfloat			x;
	GLfloat			y;
} Vec2f;

typedef struct Vec2i {
	GLint			x;
	GLint			y;
} Vec2i;

typedef struct Vec3f {
	GLfloat			x;
	GLfloat			y;
	GLfloat			z;
} Vec3f;

typedef struct Vec3i {
	GLint			x;
	GLint			y;
	GLint			z;
} Vec3i;

typedef struct Vec4f {
	GLfloat			x;
	GLfloat			y;
	GLfloat			z;
	GLfloat			w;
} Vec4f;

typedef struct Vec4i {
	GLint			x;
	GLint			y;
	GLint			z;
	GLint			w;
} Vec4i;

typedef struct Color {
	GLclampf		red;				/* red component 0 .. 1				*/
	GLclampf		green;				/* green component 0 .. 1			*/
	GLclampf		blue;				/* blue component 0 .. 1			*/
	GLclampf		alpha;				/* alpha component 0 .. 1			*/
} Color;

typedef struct ColorMask {
	GLboolean		red;				/* red component					*/
	GLboolean		green;				/* green component					*/
	GLboolean		blue;				/* blue component					*/
	GLboolean		alpha;				/* alpha component					*/
} ColorMask;

typedef struct Rect {
	GLint			x;					/* lower left corner x				*/
	GLint			y;					/* lower left corner y				*/
	GLsizei			width;				/* width in pixels					*/
	GLsizei			height;				/* height in pixels					*/
} Rect;

typedef struct Size {
	GLsizei			width;				/* width in pixels					*/
	GLsizei			height;				/* height in pixels					*/
	GLsizei			depth;				/* depth in pixels					*/
} Size;

typedef struct Cube {
	GLint			x;					/* lower left corner x				*/
	GLint			y;					/* lower left corner y				*/
	GLint			z;					/* lower left corner z				*/
	GLsizei			width;				/* width in pixels					*/
	GLsizei			height;				/* height in pixels					*/
	GLsizei			depth;				/* depth in pixels					*/
} Cube;

/*
** --------------------------------------------------------------------------
** Shader Data Structures
** --------------------------------------------------------------------------
*/
typedef union Cell {
	GLfloat			fval;
	GLint			ival;
	GLboolean		bval;
} Cell;

typedef union Vertex {
	struct {
		Vec4f			position;			/* gl_Position					*/
		GLfloat			pointSize;			/* gl_PointSize					*/
	}				v;
	GLfloat			data[GLES_MAX_VARYING_FLOATS];
} Vertex;

typedef struct RasterVertex {
	Vec4f			screen;					/* screen coordinates			*/
	const GLfloat *	varyingData;			/* varying vertex data			*/
} RasterVertex;

/*
** --------------------------------------------------------------------------
** Vertex Attribute Arrays
** --------------------------------------------------------------------------
*/

typedef struct Array {
	const void *	ptr;				/* pointer to array data			*/
	GLuint			boundBuffer;		/* index of bound buffer object		*/
	GLint			size;				/* number of array elements			*/
	GLsizei			stride;				/* stride between cons. elements	*/
	GLenum			type;				/* data type of elements			*/
	GLboolean		normalized;			/* normalized data?					*/
	GLboolean		enabled;			/* array enabled?					*/
} Array;

/*
** --------------------------------------------------------------------------
** Vertex Buffers
** --------------------------------------------------------------------------
*/

typedef struct Buffer {
	const void *	data;				/* buffer data pointer				*/
	GLsizeiptr		size;				/* size of buffer content			*/
	GLenum			bufferType;			/* type of buffer					*/
	GLenum			usage;				/* buffer usage mode				*/
	GLenum			access;				/* buffer access type				*/
	GLboolean		mapped;				/* buffer mapped to memory?			*/
	void *			mapPointer;			/* current mapping address			*/
} Buffer;

/*
** --------------------------------------------------------------------------
** Textures
** --------------------------------------------------------------------------
*/

typedef struct Image2D {
	void *					data;		/* image data						*/
	GLsizei					width;		/* width in pixels					*/
	GLsizei					height;		/* height in pixels					*/
	const BufferFormat *	format;		/* image format						*/
} Image2D;

typedef struct Image3D {
	void *					data;		/* image data						*/
	GLsizei					width;		/* width in pixels					*/
	GLsizei					height;		/* height in pixels					*/
	GLsizei					depth;		/* depth in pixels					*/
	const BufferFormat *	format;		/* image format						*/
} Image3D;

typedef struct TextureBase {
	GLenum			textureType;		/* GL_TEXTURE_2D, ..._3D,			*/
										/* ...CUBE_MAP						*/
	GLenum			minFilter;			/* minification filter mode			*/
	GLenum			magFilter;			/* magnfication filter mode			*/
	GLenum			wrapS;				/* s coordinate wrapping mode		*/
	GLenum			wrapT;				/* t coordinate wrapping mode		*/
	GLenum			wrapR;				/* r coordinate wrapping mode		*/
	GLboolean		isComplete;			/* is the texture complete?			*/
} TextureBase;

typedef struct Texture2D {
	TextureBase		base;
	Image2D			image[GLES_MAX_MIPMAP_LEVELS];
} Texture2D;

typedef struct Texture3D {
	TextureBase		base;
	Image3D			image[GLES_MAX_MIPMAP_LEVELS];
} Texture3D;

typedef struct TextureCube {
	TextureBase		base;
	Image2D			positiveX[GLES_MAX_MIPMAP_LEVELS];	/* positive x axis	*/
	Image2D			negativeX[GLES_MAX_MIPMAP_LEVELS];	/* negative x axis	*/
	Image2D			positiveY[GLES_MAX_MIPMAP_LEVELS];	/* positive y axis	*/
	Image2D			negativeY[GLES_MAX_MIPMAP_LEVELS];	/* negative Y axis	*/
	Image2D			positiveZ[GLES_MAX_MIPMAP_LEVELS];	/* positive z axis	*/
	Image2D			negativeZ[GLES_MAX_MIPMAP_LEVELS];	/* negative z axis	*/

} TextureCube;

typedef struct TextureState {
	Texture2D	texture2D;				/* 2D texture state					*/
	Texture3D	texture3D;				/* 3D texture state					*/
	TextureCube	textureCube;			/* cube map texture state			*/
} TextureState;

typedef union Texture {
	TextureBase	base;					/* common object type info			*/
	Texture2D	texture2D;				/* actual texture state				*/
	Texture3D	texture3D;				/* actual texture state				*/
	TextureCube	textureCube;			/* cube map texture state			*/
} Texture;

typedef struct FrameBuffer {
	Image2D		colorBuffer;			/* color buffer piece = 2d image	*/
	GLuint *	stencilDepthBuffer;		/* stencil & depth buffer			*/
} FrameBuffer;

/*
** --------------------------------------------------------------------------
** Shaders
** --------------------------------------------------------------------------
*/

typedef struct Attrib {
	char *		name;					/* null terminated attribute name	*/
	GLsizei		location;				/* location (index)					*/
	GLsizei		size;					/* number of words					*/
	GLenum		type;					/* type of attribute				*/
} Attrib;

typedef struct Shader {
	GLenum		type;					/* shader type						*/

	/* list of uniforms */
	GLsizei		numUniforms;
	Attrib *	uniforms;

	/* list of attributes */
	GLsizei		numVertexAttribs;
	Attrib *	vertexAttribs;

	/* list of varying */
	GLsizei		numVarying;
	Attrib *	varying;

} Shader;

/*
** --------------------------------------------------------------------------
** Shader Programs
** --------------------------------------------------------------------------
*/
typedef struct Program {
	GLuint		vertexShader;			/* associated vertex shader			*/
	GLuint		fragmentShader;			/* associated fragment shader		*/

	/* list of uniforms */
	GLsizei		numUniforms;
	Attrib *	uniforms;

	/* list of attributes */
	GLsizei		numVertexAttribs;
	Attrib *	vertexAttribs;

	/* list of varying */
	GLsizei		numVarying;
	Attrib *	varying;

	/* linkage state */
	GLboolean	isValid;				/* is this a valid program?			*/
} Program;

/*
** --------------------------------------------------------------------------
** Rendering Surface
** --------------------------------------------------------------------------
*/

/*
** TODO: Maybe we should introduce a lookup table of surface format
** descriptors, this would include
**	the GL enumeration constant identifying this type
**	the number of bits
**	the base internal format if applicable
**	the number of bytes
**	the alignment / byte steping value
*/

typedef struct Surface {
	GLES_gpumem_t	colorBuffer;		/* color buffer address				*/
	GLES_gpumem_t	depthBuffer;		/* depth buffer address				*/
	GLES_gpumem_t	stencilBuffer;		/* stencil buffer address			*/

	GLuint			width, height;		/* surface dimensions				*/

	const BufferFormat * colorFormat;	/* format of color buffer:			*/
	const BufferFormat * depthFormat;	/* format of depth buffer:			*/
	const BufferFormat * stencilFormat;	/* format of stencil buffer:		*/
} Surface;

/*
** --------------------------------------------------------------------------
** GL State
** --------------------------------------------------------------------------
*/

typedef struct State {

	/*
	** ----------------------------------------------------------------------
	** Exposed State
	** ----------------------------------------------------------------------
	*/

	/* array state */
	Array			vertexAttribArray[GLES_MAX_VERTEX_ATTRIBS];

	/* attribute state */
	Vec4f			vertexAttrib[GLES_MAX_VERTEX_ATTRIBS * GLES_VERTEX_ATTRIB_COMPONENTS];
	Vec4f			uniform[GLES_MAX_VERTEX_UNIFORM_COMPONENTS];

	/* buffer state */
	Buffer			buffers[GLES_MAX_BUFFERS];	/* defined buffers				*/
	GLuint			arrayBuffer;				/* currently active array buffer*/
	GLuint			elementArrayBuffer;			/* currently active index buffer*/

	GLuint			bufferFreeList[GLES_MAX_BUFFERS];	/* free list for bufs	*/
	GLuint			bufferFreeListHead;				/* head of free list	*/

	/* texture state */
	TextureState	textureState;				/* default texture state		*/
	GLuint			texture2D;					/* current 2D texture			*/
	GLuint			texture3D;					/* current 3D texture			*/
	GLuint			textureCube;				/* current cube map texture		*/

	Texture 		textures[GLES_MAX_TEXTURES];	/* bound textures		*/

	GLuint			textureFreeList[GLES_MAX_TEXTURES];	/* free list for texs.	*/
	GLuint			textureFreeListHead;			/* head of free list	*/

	/* shader state */
	Shader			shaders[GLES_MAX_SHADERS];		/* defined shaders		*/

	GLuint			shaderFreeList[GLES_MAX_SHADERS];	/* free list for shds.	*/
	GLuint			shaderFreeListHead;				/* head of free list	*/

	/* program state */
	Program			programs[GLES_MAX_PROGRAMS];	/* defined programs		*/
	GLuint			program;						/* current program		*/

	GLuint			programFreeList[GLES_MAX_SHADERS];	/* free list for prgs	*/
	GLuint			programFreeListHead;			/* head of free list	*/

	/* rendering state */
	GLboolean		cullFaceEnabled;		/* polygon culling enabled		*/
	GLenum			cullMode;				/* culling mode					*/
	GLenum			frontFace;				/* front face orientation		*/

	/* rasterization state */
	GLfloat			lineWidth;				/* line rasterization width		*/
	GLfloat			pointSize;				/* point rasterization size		*/
	GLfloat			polygonOffsetFactor;	/* polygon offset factor		*/
	GLfloat			polygonOffsetUnits;		/* polygon offset units			*/

	/* fragment processing state */
	GLboolean		blendEnabled;			/* is blending enabled			*/
	Color			blendColor;				/* color for blending calc.		*/
	GLenum			blendEqnModeRBG;		/* blending equation rgb		*/
	GLenum			blendEqnModeAlpha;		/* blending equation alpha		*/
	GLenum			blendFuncSrcRGB;		/* blending function src rgb	*/
	GLenum			blendFuncDstRGB;		/* blending function dst rgb	*/
	GLenum			blendFuncSrcAlpha;		/* blending function src alpha	*/
	GLenum			blendFuncDstAlpha;		/* blending function dst alpha	*/
	ColorMask		colorMask;				/* color write mask				*/

	GLboolean		depthTestEnabled;		/* is depth test enabled		*/
	GLenum			depthFunc;				/* depth test function			*/
	GLboolean		depthMask;				/* depth write mask				*/

	/* no multi-sampling support at this point */
	GLboolean		multiSampleEnabled;		/* multi-sampling?				*/
	GLboolean		sampleAlphaToCoverageEnabled;
	GLboolean		sampleAlphaToOneEnabled;
	GLboolean		sampleCoverageEnabled;
	GLclampf		sampleCovValue;			/* sample coverage value		*/
	GLboolean		sampleCovInvert;		/* invert flag					*/

	GLboolean		scissorTestEnabled;		/* is the scissor test enabled	*/
	Rect			scissorRect;			/* scissor rectangle			*/

	GLboolean		stencilTestEnabled;		/* is the stencil test enabled	*/
	GLenum			stencilFrontFunc;		/* stencil function front face	*/
	GLint			stencilFrontRef;		/* stencil ref. front face		*/
	GLuint			stencilFrontMask;		/* stencil mask front face		*/
	GLenum			stencilBackFunc;		/* stencil function back face	*/
	GLint			stencilBackRef;			/* stencil ref. back face		*/
	GLuint			stencilBackMask;		/* stencil mask back face		*/
	GLenum			stencilFrontFail;		/* stencil op. fail front		*/
	GLenum			stencilFrontZfail;		/* stencil op. zfail front		*/
	GLenum			stencilFrontZpass;		/* stencil op. zpass front		*/
	GLenum			stencilBackFail;		/* stencil op. fail back		*/
	GLenum			stencilBackZfail;		/* stencil op. zfail back		*/
	GLenum			stencilBackZpass;		/* stencil op. zpass back		*/
	GLuint			stencilMask;			/* stencil write mask			*/

	/* clear values */
	Color			clearColor;				/* clear color					*/
	GLclampf		clearDepth;				/* clear depth value			*/
	GLint			clearStencil;			/* clear stencil value			*/

	/* viewport state */
	Rect			viewport;				/* viewport area				*/
	GLclampf		zNear;					/* depth range near plane		*/
	GLclampf		zFar;					/* depth range far plane		*/

	Vec2f			viewportOrigin;			/* derived state				*/
	Vec2f			viewportScale;			/* derived state				*/
	GLfloat			depthOrigin;			/* derived state				*/
	GLfloat			depthScale;				/* derived state				*/

	/* general settings */
	GLuint			packAlignment;			/* image pixel packing			*/
	GLuint			unpackAlignment;		/* image pixel packing			*/

	/* hints */
	GLenum			fragmentShaderDerivativeHint;
	GLenum			generateMipmapHint;

	/* error state */
	GLenum			lastError;				/* last error that occurred		*/

	/*
	** ----------------------------------------------------------------------
	** Internal Execution State
	** ----------------------------------------------------------------------
	*/

	/* indices and number of currently active attribute arrays */
	GLsizei			enabledAttribArrays[GLES_MAX_VERTEX_ATTRIBS];
	GLsizei			numEnabledAttribArrays;

	/* storage area for vertex attributes as input to vertex processing */
	Vec4f			tempVertexAttrib[GLES_MAX_VERTEX_ATTRIBS];

	/* storage area for results of vertex processing */
	Vertex			vertexQueue[GLES_MAX_VERTEX_QUEUE];

} State;

/*
** For simplicity, let's start out with a single global state
*/
extern State OglesGlobalState;

#define GLES_GET_STATE() (&OglesGlobalState)


/*
** --------------------------------------------------------------------------
** State Management Functions
** --------------------------------------------------------------------------
*/

void GlesRecordError(State * state, GLenum error);

void GlesRecordInvalidEnum(State * state);
void GlesRecordInvalidValue(State * state);
void GlesRecordOutOfMemory(State * state);

GLboolean GlesValidateEnum(State * state, GLenum value, const GLenum * values, GLuint numValues);

void InitState(State * state);

void InitImage2D(Image2D * image);
void DeleteImage2D(State * state, Image2D * image);

void InitImage3D(Image3D * image);
void DeleteImage3D(State * state, Image3D * image);

void InitTexture2D(Texture2D * texture);
void DeleteTexture2D(State * state, Texture2D * texture);

void InitTexture3D(Texture3D * texture);
void DeleteTexture3D(State * state, Texture3D * texture);

void InitTextureCube(TextureCube * texture);
void DeleteTextureCube(State * state, TextureCube * texture);

void InitArray(Array * array);
void InitBuffer(Buffer * buffer);
void InitProgram(Program * program);
void InitShader(Shader * shader, GLenum shaderType);

void * Allocate(State * state, GLsizeiptr size);
void Deallocate(State * state, void * mem);

void GenObjects(State * state, GLuint freeListHead, GLuint * freeList, GLuint maxElements, GLsizei n, GLuint *objs);
GLuint BindObject(GLuint freeListHead, GLuint * freeList, GLuint maxElements);
void UnbindObject(GLuint freeListHead, GLuint * freeList, GLuint maxElements, GLuint obj);
GLboolean IsBoundObject(GLuint freeListHead, GLuint * freeList, GLuint maxElements, GLuint obj);


#endif /* ndef GLES_STATE_STATE_H */
