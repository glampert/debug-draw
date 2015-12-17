/*

    This file was generated with gl3w_gen.py, part of gl3w
    (hosted at https://github.com/skaslev/gl3w)

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

*/

#include <GL/gl3w.h>

/* --------------------------------------------------------------------------------------------- */

#ifdef _WIN32

/* ------------------------------------
 * Windows
 * ------------------------------------ */

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

static HMODULE gl3w_libgl = NULL;

static int gl3w_open_libgl(void)
{
    if (!gl3w_libgl)
    {
        gl3w_libgl = LoadLibraryA("opengl32.dll");
    }

    return gl3w_libgl != NULL;
}

static void gl3w_close_libgl(void)
{
    if (gl3w_libgl)
    {
        FreeLibrary(gl3w_libgl);
        gl3w_libgl = NULL;
    }
}

static GL3WglProc gl3w_fn(const char *proc)
{
    GL3WglProc res;
    res = (GL3WglProc) wglGetProcAddress(proc);

    if (!res)
    {
        res = (GL3WglProc) GetProcAddress(gl3w_libgl, proc);
    }

    return res;
}

#elif defined(__APPLE__) || defined(__APPLE_CC__)

/* ------------------------------------
 * Mac OS
 * ------------------------------------ */

#include <Carbon/Carbon.h>

static CFBundleRef gl3w_cfBundle = NULL;
static CFURLRef gl3w_cfBundleURL = NULL;

static int gl3w_open_libgl(void)
{
    if (gl3w_cfBundle)
    {
        return 1; /* Already init */
    }

    gl3w_cfBundleURL = CFURLCreateWithFileSystemPath(
                            kCFAllocatorDefault,
                            CFSTR("/System/Library/Frameworks/OpenGL.framework"),
                            kCFURLPOSIXPathStyle, true);
    if (!gl3w_cfBundleURL)
    {
        return 0;
    }

    gl3w_cfBundle = CFBundleCreate(kCFAllocatorDefault, gl3w_cfBundleURL);
    if (!gl3w_cfBundle)
    {
        CFRelease(gl3w_cfBundleURL);
        return 0;
    }

    return 1;
}

static void gl3w_close_libgl(void)
{
    if (gl3w_cfBundle)
    {
        CFRelease(gl3w_cfBundle);
        gl3w_cfBundle = NULL;
    }
    if (gl3w_cfBundleURL)
    {
        CFRelease(gl3w_cfBundleURL);
        gl3w_cfBundleURL = NULL;
    }
}

static GL3WglProc gl3w_fn(const char *proc)
{
    GL3WglProc res;
    CFStringRef procName;

    procName = CFStringCreateWithCString(kCFAllocatorDefault, proc, kCFStringEncodingASCII);
    res = (GL3WglProc) CFBundleGetFunctionPointerForName(gl3w_cfBundle, procName);
    CFRelease(procName);

    return res;
}

#else

/* ------------------------------------
 * GLX
 * ------------------------------------ */

#include <dlfcn.h>
#include <GL/glx.h>

static void *gl3w_libgl = NULL;

static int gl3w_open_libgl(void)
{
    if (!gl3w_libgl)
    {
        gl3w_libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
    }

    return gl3w_libgl != NULL;
}

static void gl3w_close_libgl(void)
{
    if (gl3w_libgl)
    {
        dlclose(gl3w_libgl);
        gl3w_libgl = NULL;
    }
}

static GL3WglProc gl3w_fn(const char *proc)
{
    GL3WglProc res;
    res = (GL3WglProc) glXGetProcAddress((const GLubyte *) proc);

    if (!res)
    {
        res = (GL3WglProc) dlsym(gl3w_libgl, proc);
    }

    return res;
}

#endif

/* ------------------------------------
 * GL3W API
 * ------------------------------------ */

static struct {
	int major, minor;
} gl3w_version;

static void gl3w_load_all_functions(void);

static int gl3w_parse_version(void)
{
    if (!glGetIntegerv)
    {
        return 0;
    }

    glGetIntegerv(GL_MAJOR_VERSION, &gl3w_version.major);
    glGetIntegerv(GL_MINOR_VERSION, &gl3w_version.minor);

    if (gl3w_version.major < 3)
    {
        return 0;
    }

    return 1;
}

int gl3wInit(void)
{
    if (!gl3w_open_libgl())
    {
        return 0;
    }

    gl3w_load_all_functions();
    return gl3w_parse_version();
}

void gl3wShutdown(void)
{
    gl3w_close_libgl();
}

int gl3wIsSupported(int major, int minor)
{
    if (major < 3)
    {
        return 0;
    }
    if (gl3w_version.major == major)
    {
        return gl3w_version.minor >= minor;
    }
    return gl3w_version.major >= major;
}

GL3WglProc gl3wGetProcAddress(const char *proc)
{
    if (!proc)
    {
        return NULL;
    }
    return gl3w_fn(proc);
}

/* --------------------------------------------------------------------------------------------- */

PFNGLACTIVESHADERPROGRAMPROC                         gl3wActiveShaderProgram;
PFNGLACTIVETEXTUREPROC                               gl3wActiveTexture;
PFNGLATTACHSHADERPROC                                gl3wAttachShader;
PFNGLBEGINCONDITIONALRENDERPROC                      gl3wBeginConditionalRender;
PFNGLBEGINQUERYPROC                                  gl3wBeginQuery;
PFNGLBEGINQUERYINDEXEDPROC                           gl3wBeginQueryIndexed;
PFNGLBEGINTRANSFORMFEEDBACKPROC                      gl3wBeginTransformFeedback;
PFNGLBINDATTRIBLOCATIONPROC                          gl3wBindAttribLocation;
PFNGLBINDBUFFERPROC                                  gl3wBindBuffer;
PFNGLBINDBUFFERBASEPROC                              gl3wBindBufferBase;
PFNGLBINDBUFFERRANGEPROC                             gl3wBindBufferRange;
PFNGLBINDBUFFERSBASEPROC                             gl3wBindBuffersBase;
PFNGLBINDBUFFERSRANGEPROC                            gl3wBindBuffersRange;
PFNGLBINDFRAGDATALOCATIONPROC                        gl3wBindFragDataLocation;
PFNGLBINDFRAGDATALOCATIONINDEXEDPROC                 gl3wBindFragDataLocationIndexed;
PFNGLBINDFRAMEBUFFERPROC                             gl3wBindFramebuffer;
PFNGLBINDIMAGETEXTUREPROC                            gl3wBindImageTexture;
PFNGLBINDIMAGETEXTURESPROC                           gl3wBindImageTextures;
PFNGLBINDPROGRAMPIPELINEPROC                         gl3wBindProgramPipeline;
PFNGLBINDRENDERBUFFERPROC                            gl3wBindRenderbuffer;
PFNGLBINDSAMPLERPROC                                 gl3wBindSampler;
PFNGLBINDSAMPLERSPROC                                gl3wBindSamplers;
PFNGLBINDTEXTUREPROC                                 gl3wBindTexture;
PFNGLBINDTEXTUREUNITPROC                             gl3wBindTextureUnit;
PFNGLBINDTEXTURESPROC                                gl3wBindTextures;
PFNGLBINDTRANSFORMFEEDBACKPROC                       gl3wBindTransformFeedback;
PFNGLBINDVERTEXARRAYPROC                             gl3wBindVertexArray;
PFNGLBINDVERTEXBUFFERPROC                            gl3wBindVertexBuffer;
PFNGLBINDVERTEXBUFFERSPROC                           gl3wBindVertexBuffers;
PFNGLBLENDCOLORPROC                                  gl3wBlendColor;
PFNGLBLENDEQUATIONPROC                               gl3wBlendEquation;
PFNGLBLENDEQUATIONSEPARATEPROC                       gl3wBlendEquationSeparate;
PFNGLBLENDEQUATIONSEPARATEIPROC                      gl3wBlendEquationSeparatei;
PFNGLBLENDEQUATIONSEPARATEIARBPROC                   gl3wBlendEquationSeparateiARB;
PFNGLBLENDEQUATIONIPROC                              gl3wBlendEquationi;
PFNGLBLENDEQUATIONIARBPROC                           gl3wBlendEquationiARB;
PFNGLBLENDFUNCPROC                                   gl3wBlendFunc;
PFNGLBLENDFUNCSEPARATEPROC                           gl3wBlendFuncSeparate;
PFNGLBLENDFUNCSEPARATEIPROC                          gl3wBlendFuncSeparatei;
PFNGLBLENDFUNCSEPARATEIARBPROC                       gl3wBlendFuncSeparateiARB;
PFNGLBLENDFUNCIPROC                                  gl3wBlendFunci;
PFNGLBLENDFUNCIARBPROC                               gl3wBlendFunciARB;
PFNGLBLITFRAMEBUFFERPROC                             gl3wBlitFramebuffer;
PFNGLBLITNAMEDFRAMEBUFFERPROC                        gl3wBlitNamedFramebuffer;
PFNGLBUFFERDATAPROC                                  gl3wBufferData;
PFNGLBUFFERPAGECOMMITMENTARBPROC                     gl3wBufferPageCommitmentARB;
PFNGLBUFFERSTORAGEPROC                               gl3wBufferStorage;
PFNGLBUFFERSUBDATAPROC                               gl3wBufferSubData;
PFNGLCHECKFRAMEBUFFERSTATUSPROC                      gl3wCheckFramebufferStatus;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC                 gl3wCheckNamedFramebufferStatus;
PFNGLCLAMPCOLORPROC                                  gl3wClampColor;
PFNGLCLEARPROC                                       gl3wClear;
PFNGLCLEARBUFFERDATAPROC                             gl3wClearBufferData;
PFNGLCLEARBUFFERSUBDATAPROC                          gl3wClearBufferSubData;
PFNGLCLEARBUFFERFIPROC                               gl3wClearBufferfi;
PFNGLCLEARBUFFERFVPROC                               gl3wClearBufferfv;
PFNGLCLEARBUFFERIVPROC                               gl3wClearBufferiv;
PFNGLCLEARBUFFERUIVPROC                              gl3wClearBufferuiv;
PFNGLCLEARCOLORPROC                                  gl3wClearColor;
PFNGLCLEARDEPTHPROC                                  gl3wClearDepth;
PFNGLCLEARDEPTHFPROC                                 gl3wClearDepthf;
PFNGLCLEARNAMEDBUFFERDATAPROC                        gl3wClearNamedBufferData;
PFNGLCLEARNAMEDBUFFERSUBDATAPROC                     gl3wClearNamedBufferSubData;
PFNGLCLEARNAMEDFRAMEBUFFERFIPROC                     gl3wClearNamedFramebufferfi;
PFNGLCLEARNAMEDFRAMEBUFFERFVPROC                     gl3wClearNamedFramebufferfv;
PFNGLCLEARNAMEDFRAMEBUFFERIVPROC                     gl3wClearNamedFramebufferiv;
PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC                    gl3wClearNamedFramebufferuiv;
PFNGLCLEARSTENCILPROC                                gl3wClearStencil;
PFNGLCLEARTEXIMAGEPROC                               gl3wClearTexImage;
PFNGLCLEARTEXSUBIMAGEPROC                            gl3wClearTexSubImage;
PFNGLCLIENTWAITSYNCPROC                              gl3wClientWaitSync;
PFNGLCLIPCONTROLPROC                                 gl3wClipControl;
PFNGLCOLORMASKPROC                                   gl3wColorMask;
PFNGLCOLORMASKIPROC                                  gl3wColorMaski;
PFNGLCOMPILESHADERPROC                               gl3wCompileShader;
PFNGLCOMPILESHADERINCLUDEARBPROC                     gl3wCompileShaderIncludeARB;
PFNGLCOMPRESSEDTEXIMAGE1DPROC                        gl3wCompressedTexImage1D;
PFNGLCOMPRESSEDTEXIMAGE2DPROC                        gl3wCompressedTexImage2D;
PFNGLCOMPRESSEDTEXIMAGE3DPROC                        gl3wCompressedTexImage3D;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC                     gl3wCompressedTexSubImage1D;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC                     gl3wCompressedTexSubImage2D;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC                     gl3wCompressedTexSubImage3D;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC                 gl3wCompressedTextureSubImage1D;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC                 gl3wCompressedTextureSubImage2D;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC                 gl3wCompressedTextureSubImage3D;
PFNGLCOPYBUFFERSUBDATAPROC                           gl3wCopyBufferSubData;
PFNGLCOPYIMAGESUBDATAPROC                            gl3wCopyImageSubData;
PFNGLCOPYNAMEDBUFFERSUBDATAPROC                      gl3wCopyNamedBufferSubData;
PFNGLCOPYTEXIMAGE1DPROC                              gl3wCopyTexImage1D;
PFNGLCOPYTEXIMAGE2DPROC                              gl3wCopyTexImage2D;
PFNGLCOPYTEXSUBIMAGE1DPROC                           gl3wCopyTexSubImage1D;
PFNGLCOPYTEXSUBIMAGE2DPROC                           gl3wCopyTexSubImage2D;
PFNGLCOPYTEXSUBIMAGE3DPROC                           gl3wCopyTexSubImage3D;
PFNGLCOPYTEXTURESUBIMAGE1DPROC                       gl3wCopyTextureSubImage1D;
PFNGLCOPYTEXTURESUBIMAGE2DPROC                       gl3wCopyTextureSubImage2D;
PFNGLCOPYTEXTURESUBIMAGE3DPROC                       gl3wCopyTextureSubImage3D;
PFNGLCREATEBUFFERSPROC                               gl3wCreateBuffers;
PFNGLCREATEFRAMEBUFFERSPROC                          gl3wCreateFramebuffers;
PFNGLCREATEPROGRAMPROC                               gl3wCreateProgram;
PFNGLCREATEPROGRAMPIPELINESPROC                      gl3wCreateProgramPipelines;
PFNGLCREATEQUERIESPROC                               gl3wCreateQueries;
PFNGLCREATERENDERBUFFERSPROC                         gl3wCreateRenderbuffers;
PFNGLCREATESAMPLERSPROC                              gl3wCreateSamplers;
PFNGLCREATESHADERPROC                                gl3wCreateShader;
PFNGLCREATESHADERPROGRAMVPROC                        gl3wCreateShaderProgramv;
PFNGLCREATESYNCFROMCLEVENTARBPROC                    gl3wCreateSyncFromCLeventARB;
PFNGLCREATETEXTURESPROC                              gl3wCreateTextures;
PFNGLCREATETRANSFORMFEEDBACKSPROC                    gl3wCreateTransformFeedbacks;
PFNGLCREATEVERTEXARRAYSPROC                          gl3wCreateVertexArrays;
PFNGLCULLFACEPROC                                    gl3wCullFace;
PFNGLDEBUGMESSAGECALLBACKPROC                        gl3wDebugMessageCallback;
PFNGLDEBUGMESSAGECALLBACKARBPROC                     gl3wDebugMessageCallbackARB;
PFNGLDEBUGMESSAGECONTROLPROC                         gl3wDebugMessageControl;
PFNGLDEBUGMESSAGECONTROLARBPROC                      gl3wDebugMessageControlARB;
PFNGLDEBUGMESSAGEINSERTPROC                          gl3wDebugMessageInsert;
PFNGLDEBUGMESSAGEINSERTARBPROC                       gl3wDebugMessageInsertARB;
PFNGLDELETEBUFFERSPROC                               gl3wDeleteBuffers;
PFNGLDELETEFRAMEBUFFERSPROC                          gl3wDeleteFramebuffers;
PFNGLDELETENAMEDSTRINGARBPROC                        gl3wDeleteNamedStringARB;
PFNGLDELETEPROGRAMPROC                               gl3wDeleteProgram;
PFNGLDELETEPROGRAMPIPELINESPROC                      gl3wDeleteProgramPipelines;
PFNGLDELETEQUERIESPROC                               gl3wDeleteQueries;
PFNGLDELETERENDERBUFFERSPROC                         gl3wDeleteRenderbuffers;
PFNGLDELETESAMPLERSPROC                              gl3wDeleteSamplers;
PFNGLDELETESHADERPROC                                gl3wDeleteShader;
PFNGLDELETESYNCPROC                                  gl3wDeleteSync;
PFNGLDELETETEXTURESPROC                              gl3wDeleteTextures;
PFNGLDELETETRANSFORMFEEDBACKSPROC                    gl3wDeleteTransformFeedbacks;
PFNGLDELETEVERTEXARRAYSPROC                          gl3wDeleteVertexArrays;
PFNGLDEPTHFUNCPROC                                   gl3wDepthFunc;
PFNGLDEPTHMASKPROC                                   gl3wDepthMask;
PFNGLDEPTHRANGEPROC                                  gl3wDepthRange;
PFNGLDEPTHRANGEARRAYVPROC                            gl3wDepthRangeArrayv;
PFNGLDEPTHRANGEINDEXEDPROC                           gl3wDepthRangeIndexed;
PFNGLDEPTHRANGEFPROC                                 gl3wDepthRangef;
PFNGLDETACHSHADERPROC                                gl3wDetachShader;
PFNGLDISABLEPROC                                     gl3wDisable;
PFNGLDISABLEVERTEXARRAYATTRIBPROC                    gl3wDisableVertexArrayAttrib;
PFNGLDISABLEVERTEXATTRIBARRAYPROC                    gl3wDisableVertexAttribArray;
PFNGLDISABLEIPROC                                    gl3wDisablei;
PFNGLDISPATCHCOMPUTEPROC                             gl3wDispatchCompute;
PFNGLDISPATCHCOMPUTEGROUPSIZEARBPROC                 gl3wDispatchComputeGroupSizeARB;
PFNGLDISPATCHCOMPUTEINDIRECTPROC                     gl3wDispatchComputeIndirect;
PFNGLDRAWARRAYSPROC                                  gl3wDrawArrays;
PFNGLDRAWARRAYSINDIRECTPROC                          gl3wDrawArraysIndirect;
PFNGLDRAWARRAYSINSTANCEDPROC                         gl3wDrawArraysInstanced;
PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC             gl3wDrawArraysInstancedBaseInstance;
PFNGLDRAWBUFFERPROC                                  gl3wDrawBuffer;
PFNGLDRAWBUFFERSPROC                                 gl3wDrawBuffers;
PFNGLDRAWELEMENTSPROC                                gl3wDrawElements;
PFNGLDRAWELEMENTSBASEVERTEXPROC                      gl3wDrawElementsBaseVertex;
PFNGLDRAWELEMENTSINDIRECTPROC                        gl3wDrawElementsIndirect;
PFNGLDRAWELEMENTSINSTANCEDPROC                       gl3wDrawElementsInstanced;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC           gl3wDrawElementsInstancedBaseInstance;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC             gl3wDrawElementsInstancedBaseVertex;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC gl3wDrawElementsInstancedBaseVertexBaseInstance;
PFNGLDRAWRANGEELEMENTSPROC                           gl3wDrawRangeElements;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC                 gl3wDrawRangeElementsBaseVertex;
PFNGLDRAWTRANSFORMFEEDBACKPROC                       gl3wDrawTransformFeedback;
PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC              gl3wDrawTransformFeedbackInstanced;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC                 gl3wDrawTransformFeedbackStream;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC        gl3wDrawTransformFeedbackStreamInstanced;
PFNGLENABLEPROC                                      gl3wEnable;
PFNGLENABLEVERTEXARRAYATTRIBPROC                     gl3wEnableVertexArrayAttrib;
PFNGLENABLEVERTEXATTRIBARRAYPROC                     gl3wEnableVertexAttribArray;
PFNGLENABLEIPROC                                     gl3wEnablei;
PFNGLENDCONDITIONALRENDERPROC                        gl3wEndConditionalRender;
PFNGLENDQUERYPROC                                    gl3wEndQuery;
PFNGLENDQUERYINDEXEDPROC                             gl3wEndQueryIndexed;
PFNGLENDTRANSFORMFEEDBACKPROC                        gl3wEndTransformFeedback;
PFNGLFENCESYNCPROC                                   gl3wFenceSync;
PFNGLFINISHPROC                                      gl3wFinish;
PFNGLFLUSHPROC                                       gl3wFlush;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC                      gl3wFlushMappedBufferRange;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC                 gl3wFlushMappedNamedBufferRange;
PFNGLFRAMEBUFFERPARAMETERIPROC                       gl3wFramebufferParameteri;
PFNGLFRAMEBUFFERRENDERBUFFERPROC                     gl3wFramebufferRenderbuffer;
PFNGLFRAMEBUFFERTEXTUREPROC                          gl3wFramebufferTexture;
PFNGLFRAMEBUFFERTEXTURE1DPROC                        gl3wFramebufferTexture1D;
PFNGLFRAMEBUFFERTEXTURE2DPROC                        gl3wFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTURE3DPROC                        gl3wFramebufferTexture3D;
PFNGLFRAMEBUFFERTEXTURELAYERPROC                     gl3wFramebufferTextureLayer;
PFNGLFRONTFACEPROC                                   gl3wFrontFace;
PFNGLGENBUFFERSPROC                                  gl3wGenBuffers;
PFNGLGENFRAMEBUFFERSPROC                             gl3wGenFramebuffers;
PFNGLGENPROGRAMPIPELINESPROC                         gl3wGenProgramPipelines;
PFNGLGENQUERIESPROC                                  gl3wGenQueries;
PFNGLGENRENDERBUFFERSPROC                            gl3wGenRenderbuffers;
PFNGLGENSAMPLERSPROC                                 gl3wGenSamplers;
PFNGLGENTEXTURESPROC                                 gl3wGenTextures;
PFNGLGENTRANSFORMFEEDBACKSPROC                       gl3wGenTransformFeedbacks;
PFNGLGENVERTEXARRAYSPROC                             gl3wGenVertexArrays;
PFNGLGENERATEMIPMAPPROC                              gl3wGenerateMipmap;
PFNGLGENERATETEXTUREMIPMAPPROC                       gl3wGenerateTextureMipmap;
PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC              gl3wGetActiveAtomicCounterBufferiv;
PFNGLGETACTIVEATTRIBPROC                             gl3wGetActiveAttrib;
PFNGLGETACTIVESUBROUTINENAMEPROC                     gl3wGetActiveSubroutineName;
PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC              gl3wGetActiveSubroutineUniformName;
PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC                gl3wGetActiveSubroutineUniformiv;
PFNGLGETACTIVEUNIFORMPROC                            gl3wGetActiveUniform;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC                   gl3wGetActiveUniformBlockName;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC                     gl3wGetActiveUniformBlockiv;
PFNGLGETACTIVEUNIFORMNAMEPROC                        gl3wGetActiveUniformName;
PFNGLGETACTIVEUNIFORMSIVPROC                         gl3wGetActiveUniformsiv;
PFNGLGETATTACHEDSHADERSPROC                          gl3wGetAttachedShaders;
PFNGLGETATTRIBLOCATIONPROC                           gl3wGetAttribLocation;
PFNGLGETBOOLEANI_VPROC                               gl3wGetBooleani_v;
PFNGLGETBOOLEANVPROC                                 gl3wGetBooleanv;
PFNGLGETBUFFERPARAMETERI64VPROC                      gl3wGetBufferParameteri64v;
PFNGLGETBUFFERPARAMETERIVPROC                        gl3wGetBufferParameteriv;
PFNGLGETBUFFERPOINTERVPROC                           gl3wGetBufferPointerv;
PFNGLGETBUFFERSUBDATAPROC                            gl3wGetBufferSubData;
PFNGLGETCOMPRESSEDTEXIMAGEPROC                       gl3wGetCompressedTexImage;
PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC                   gl3wGetCompressedTextureImage;
PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC                gl3wGetCompressedTextureSubImage;
PFNGLGETDEBUGMESSAGELOGPROC                          gl3wGetDebugMessageLog;
PFNGLGETDEBUGMESSAGELOGARBPROC                       gl3wGetDebugMessageLogARB;
PFNGLGETDOUBLEI_VPROC                                gl3wGetDoublei_v;
PFNGLGETDOUBLEVPROC                                  gl3wGetDoublev;
PFNGLGETERRORPROC                                    gl3wGetError;
PFNGLGETFLOATI_VPROC                                 gl3wGetFloati_v;
PFNGLGETFLOATVPROC                                   gl3wGetFloatv;
PFNGLGETFRAGDATAINDEXPROC                            gl3wGetFragDataIndex;
PFNGLGETFRAGDATALOCATIONPROC                         gl3wGetFragDataLocation;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC         gl3wGetFramebufferAttachmentParameteriv;
PFNGLGETFRAMEBUFFERPARAMETERIVPROC                   gl3wGetFramebufferParameteriv;
PFNGLGETGRAPHICSRESETSTATUSPROC                      gl3wGetGraphicsResetStatus;
PFNGLGETGRAPHICSRESETSTATUSARBPROC                   gl3wGetGraphicsResetStatusARB;
PFNGLGETIMAGEHANDLEARBPROC                           gl3wGetImageHandleARB;
PFNGLGETINTEGER64I_VPROC                             gl3wGetInteger64i_v;
PFNGLGETINTEGER64VPROC                               gl3wGetInteger64v;
PFNGLGETINTEGERI_VPROC                               gl3wGetIntegeri_v;
PFNGLGETINTEGERVPROC                                 gl3wGetIntegerv;
PFNGLGETINTERNALFORMATI64VPROC                       gl3wGetInternalformati64v;
PFNGLGETINTERNALFORMATIVPROC                         gl3wGetInternalformativ;
PFNGLGETMULTISAMPLEFVPROC                            gl3wGetMultisamplefv;
PFNGLGETNAMEDBUFFERPARAMETERI64VPROC                 gl3wGetNamedBufferParameteri64v;
PFNGLGETNAMEDBUFFERPARAMETERIVPROC                   gl3wGetNamedBufferParameteriv;
PFNGLGETNAMEDBUFFERPOINTERVPROC                      gl3wGetNamedBufferPointerv;
PFNGLGETNAMEDBUFFERSUBDATAPROC                       gl3wGetNamedBufferSubData;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC    gl3wGetNamedFramebufferAttachmentParameteriv;
PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC              gl3wGetNamedFramebufferParameteriv;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC             gl3wGetNamedRenderbufferParameteriv;
PFNGLGETNAMEDSTRINGARBPROC                           gl3wGetNamedStringARB;
PFNGLGETNAMEDSTRINGIVARBPROC                         gl3wGetNamedStringivARB;
PFNGLGETOBJECTLABELPROC                              gl3wGetObjectLabel;
PFNGLGETOBJECTPTRLABELPROC                           gl3wGetObjectPtrLabel;
PFNGLGETPOINTERVPROC                                 gl3wGetPointerv;
PFNGLGETPROGRAMBINARYPROC                            gl3wGetProgramBinary;
PFNGLGETPROGRAMINFOLOGPROC                           gl3wGetProgramInfoLog;
PFNGLGETPROGRAMINTERFACEIVPROC                       gl3wGetProgramInterfaceiv;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC                   gl3wGetProgramPipelineInfoLog;
PFNGLGETPROGRAMPIPELINEIVPROC                        gl3wGetProgramPipelineiv;
PFNGLGETPROGRAMRESOURCEINDEXPROC                     gl3wGetProgramResourceIndex;
PFNGLGETPROGRAMRESOURCELOCATIONPROC                  gl3wGetProgramResourceLocation;
PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC             gl3wGetProgramResourceLocationIndex;
PFNGLGETPROGRAMRESOURCENAMEPROC                      gl3wGetProgramResourceName;
PFNGLGETPROGRAMRESOURCEIVPROC                        gl3wGetProgramResourceiv;
PFNGLGETPROGRAMSTAGEIVPROC                           gl3wGetProgramStageiv;
PFNGLGETPROGRAMIVPROC                                gl3wGetProgramiv;
PFNGLGETQUERYBUFFEROBJECTI64VPROC                    gl3wGetQueryBufferObjecti64v;
PFNGLGETQUERYBUFFEROBJECTIVPROC                      gl3wGetQueryBufferObjectiv;
PFNGLGETQUERYBUFFEROBJECTUI64VPROC                   gl3wGetQueryBufferObjectui64v;
PFNGLGETQUERYBUFFEROBJECTUIVPROC                     gl3wGetQueryBufferObjectuiv;
PFNGLGETQUERYINDEXEDIVPROC                           gl3wGetQueryIndexediv;
PFNGLGETQUERYOBJECTI64VPROC                          gl3wGetQueryObjecti64v;
PFNGLGETQUERYOBJECTIVPROC                            gl3wGetQueryObjectiv;
PFNGLGETQUERYOBJECTUI64VPROC                         gl3wGetQueryObjectui64v;
PFNGLGETQUERYOBJECTUIVPROC                           gl3wGetQueryObjectuiv;
PFNGLGETQUERYIVPROC                                  gl3wGetQueryiv;
PFNGLGETRENDERBUFFERPARAMETERIVPROC                  gl3wGetRenderbufferParameteriv;
PFNGLGETSAMPLERPARAMETERIIVPROC                      gl3wGetSamplerParameterIiv;
PFNGLGETSAMPLERPARAMETERIUIVPROC                     gl3wGetSamplerParameterIuiv;
PFNGLGETSAMPLERPARAMETERFVPROC                       gl3wGetSamplerParameterfv;
PFNGLGETSAMPLERPARAMETERIVPROC                       gl3wGetSamplerParameteriv;
PFNGLGETSHADERINFOLOGPROC                            gl3wGetShaderInfoLog;
PFNGLGETSHADERPRECISIONFORMATPROC                    gl3wGetShaderPrecisionFormat;
PFNGLGETSHADERSOURCEPROC                             gl3wGetShaderSource;
PFNGLGETSHADERIVPROC                                 gl3wGetShaderiv;
PFNGLGETSTRINGPROC                                   gl3wGetString;
PFNGLGETSTRINGIPROC                                  gl3wGetStringi;
PFNGLGETSUBROUTINEINDEXPROC                          gl3wGetSubroutineIndex;
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC                gl3wGetSubroutineUniformLocation;
PFNGLGETSYNCIVPROC                                   gl3wGetSynciv;
PFNGLGETTEXIMAGEPROC                                 gl3wGetTexImage;
PFNGLGETTEXLEVELPARAMETERFVPROC                      gl3wGetTexLevelParameterfv;
PFNGLGETTEXLEVELPARAMETERIVPROC                      gl3wGetTexLevelParameteriv;
PFNGLGETTEXPARAMETERIIVPROC                          gl3wGetTexParameterIiv;
PFNGLGETTEXPARAMETERIUIVPROC                         gl3wGetTexParameterIuiv;
PFNGLGETTEXPARAMETERFVPROC                           gl3wGetTexParameterfv;
PFNGLGETTEXPARAMETERIVPROC                           gl3wGetTexParameteriv;
PFNGLGETTEXTUREHANDLEARBPROC                         gl3wGetTextureHandleARB;
PFNGLGETTEXTUREIMAGEPROC                             gl3wGetTextureImage;
PFNGLGETTEXTURELEVELPARAMETERFVPROC                  gl3wGetTextureLevelParameterfv;
PFNGLGETTEXTURELEVELPARAMETERIVPROC                  gl3wGetTextureLevelParameteriv;
PFNGLGETTEXTUREPARAMETERIIVPROC                      gl3wGetTextureParameterIiv;
PFNGLGETTEXTUREPARAMETERIUIVPROC                     gl3wGetTextureParameterIuiv;
PFNGLGETTEXTUREPARAMETERFVPROC                       gl3wGetTextureParameterfv;
PFNGLGETTEXTUREPARAMETERIVPROC                       gl3wGetTextureParameteriv;
PFNGLGETTEXTURESAMPLERHANDLEARBPROC                  gl3wGetTextureSamplerHandleARB;
PFNGLGETTEXTURESUBIMAGEPROC                          gl3wGetTextureSubImage;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC                 gl3wGetTransformFeedbackVarying;
PFNGLGETTRANSFORMFEEDBACKI64_VPROC                   gl3wGetTransformFeedbacki64_v;
PFNGLGETTRANSFORMFEEDBACKI_VPROC                     gl3wGetTransformFeedbacki_v;
PFNGLGETTRANSFORMFEEDBACKIVPROC                      gl3wGetTransformFeedbackiv;
PFNGLGETUNIFORMBLOCKINDEXPROC                        gl3wGetUniformBlockIndex;
PFNGLGETUNIFORMINDICESPROC                           gl3wGetUniformIndices;
PFNGLGETUNIFORMLOCATIONPROC                          gl3wGetUniformLocation;
PFNGLGETUNIFORMSUBROUTINEUIVPROC                     gl3wGetUniformSubroutineuiv;
PFNGLGETUNIFORMDVPROC                                gl3wGetUniformdv;
PFNGLGETUNIFORMFVPROC                                gl3wGetUniformfv;
PFNGLGETUNIFORMIVPROC                                gl3wGetUniformiv;
PFNGLGETUNIFORMUIVPROC                               gl3wGetUniformuiv;
PFNGLGETVERTEXARRAYINDEXED64IVPROC                   gl3wGetVertexArrayIndexed64iv;
PFNGLGETVERTEXARRAYINDEXEDIVPROC                     gl3wGetVertexArrayIndexediv;
PFNGLGETVERTEXARRAYIVPROC                            gl3wGetVertexArrayiv;
PFNGLGETVERTEXATTRIBIIVPROC                          gl3wGetVertexAttribIiv;
PFNGLGETVERTEXATTRIBIUIVPROC                         gl3wGetVertexAttribIuiv;
PFNGLGETVERTEXATTRIBLDVPROC                          gl3wGetVertexAttribLdv;
PFNGLGETVERTEXATTRIBLUI64VARBPROC                    gl3wGetVertexAttribLui64vARB;
PFNGLGETVERTEXATTRIBPOINTERVPROC                     gl3wGetVertexAttribPointerv;
PFNGLGETVERTEXATTRIBDVPROC                           gl3wGetVertexAttribdv;
PFNGLGETVERTEXATTRIBFVPROC                           gl3wGetVertexAttribfv;
PFNGLGETVERTEXATTRIBIVPROC                           gl3wGetVertexAttribiv;
PFNGLGETNCOMPRESSEDTEXIMAGEPROC                      gl3wGetnCompressedTexImage;
PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC                   gl3wGetnCompressedTexImageARB;
PFNGLGETNTEXIMAGEPROC                                gl3wGetnTexImage;
PFNGLGETNTEXIMAGEARBPROC                             gl3wGetnTexImageARB;
PFNGLGETNUNIFORMDVPROC                               gl3wGetnUniformdv;
PFNGLGETNUNIFORMDVARBPROC                            gl3wGetnUniformdvARB;
PFNGLGETNUNIFORMFVPROC                               gl3wGetnUniformfv;
PFNGLGETNUNIFORMFVARBPROC                            gl3wGetnUniformfvARB;
PFNGLGETNUNIFORMIVPROC                               gl3wGetnUniformiv;
PFNGLGETNUNIFORMIVARBPROC                            gl3wGetnUniformivARB;
PFNGLGETNUNIFORMUIVPROC                              gl3wGetnUniformuiv;
PFNGLGETNUNIFORMUIVARBPROC                           gl3wGetnUniformuivARB;
PFNGLHINTPROC                                        gl3wHint;
PFNGLINVALIDATEBUFFERDATAPROC                        gl3wInvalidateBufferData;
PFNGLINVALIDATEBUFFERSUBDATAPROC                     gl3wInvalidateBufferSubData;
PFNGLINVALIDATEFRAMEBUFFERPROC                       gl3wInvalidateFramebuffer;
PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC              gl3wInvalidateNamedFramebufferData;
PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC           gl3wInvalidateNamedFramebufferSubData;
PFNGLINVALIDATESUBFRAMEBUFFERPROC                    gl3wInvalidateSubFramebuffer;
PFNGLINVALIDATETEXIMAGEPROC                          gl3wInvalidateTexImage;
PFNGLINVALIDATETEXSUBIMAGEPROC                       gl3wInvalidateTexSubImage;
PFNGLISBUFFERPROC                                    gl3wIsBuffer;
PFNGLISENABLEDPROC                                   gl3wIsEnabled;
PFNGLISENABLEDIPROC                                  gl3wIsEnabledi;
PFNGLISFRAMEBUFFERPROC                               gl3wIsFramebuffer;
PFNGLISIMAGEHANDLERESIDENTARBPROC                    gl3wIsImageHandleResidentARB;
PFNGLISNAMEDSTRINGARBPROC                            gl3wIsNamedStringARB;
PFNGLISPROGRAMPROC                                   gl3wIsProgram;
PFNGLISPROGRAMPIPELINEPROC                           gl3wIsProgramPipeline;
PFNGLISQUERYPROC                                     gl3wIsQuery;
PFNGLISRENDERBUFFERPROC                              gl3wIsRenderbuffer;
PFNGLISSAMPLERPROC                                   gl3wIsSampler;
PFNGLISSHADERPROC                                    gl3wIsShader;
PFNGLISSYNCPROC                                      gl3wIsSync;
PFNGLISTEXTUREPROC                                   gl3wIsTexture;
PFNGLISTEXTUREHANDLERESIDENTARBPROC                  gl3wIsTextureHandleResidentARB;
PFNGLISTRANSFORMFEEDBACKPROC                         gl3wIsTransformFeedback;
PFNGLISVERTEXARRAYPROC                               gl3wIsVertexArray;
PFNGLLINEWIDTHPROC                                   gl3wLineWidth;
PFNGLLINKPROGRAMPROC                                 gl3wLinkProgram;
PFNGLLOGICOPPROC                                     gl3wLogicOp;
PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC               gl3wMakeImageHandleNonResidentARB;
PFNGLMAKEIMAGEHANDLERESIDENTARBPROC                  gl3wMakeImageHandleResidentARB;
PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC             gl3wMakeTextureHandleNonResidentARB;
PFNGLMAKETEXTUREHANDLERESIDENTARBPROC                gl3wMakeTextureHandleResidentARB;
PFNGLMAPBUFFERPROC                                   gl3wMapBuffer;
PFNGLMAPBUFFERRANGEPROC                              gl3wMapBufferRange;
PFNGLMAPNAMEDBUFFERPROC                              gl3wMapNamedBuffer;
PFNGLMAPNAMEDBUFFERRANGEPROC                         gl3wMapNamedBufferRange;
PFNGLMEMORYBARRIERPROC                               gl3wMemoryBarrier;
PFNGLMEMORYBARRIERBYREGIONPROC                       gl3wMemoryBarrierByRegion;
PFNGLMINSAMPLESHADINGPROC                            gl3wMinSampleShading;
PFNGLMINSAMPLESHADINGARBPROC                         gl3wMinSampleShadingARB;
PFNGLMULTIDRAWARRAYSPROC                             gl3wMultiDrawArrays;
PFNGLMULTIDRAWARRAYSINDIRECTPROC                     gl3wMultiDrawArraysIndirect;
PFNGLMULTIDRAWARRAYSINDIRECTCOUNTARBPROC             gl3wMultiDrawArraysIndirectCountARB;
PFNGLMULTIDRAWELEMENTSPROC                           gl3wMultiDrawElements;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC                 gl3wMultiDrawElementsBaseVertex;
PFNGLMULTIDRAWELEMENTSINDIRECTPROC                   gl3wMultiDrawElementsIndirect;
PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTARBPROC           gl3wMultiDrawElementsIndirectCountARB;
PFNGLNAMEDBUFFERDATAPROC                             gl3wNamedBufferData;
PFNGLNAMEDBUFFERPAGECOMMITMENTARBPROC                gl3wNamedBufferPageCommitmentARB;
PFNGLNAMEDBUFFERPAGECOMMITMENTEXTPROC                gl3wNamedBufferPageCommitmentEXT;
PFNGLNAMEDBUFFERSTORAGEPROC                          gl3wNamedBufferStorage;
PFNGLNAMEDBUFFERSUBDATAPROC                          gl3wNamedBufferSubData;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC                  gl3wNamedFramebufferDrawBuffer;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC                 gl3wNamedFramebufferDrawBuffers;
PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC                  gl3wNamedFramebufferParameteri;
PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC                  gl3wNamedFramebufferReadBuffer;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC                gl3wNamedFramebufferRenderbuffer;
PFNGLNAMEDFRAMEBUFFERTEXTUREPROC                     gl3wNamedFramebufferTexture;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC                gl3wNamedFramebufferTextureLayer;
PFNGLNAMEDRENDERBUFFERSTORAGEPROC                    gl3wNamedRenderbufferStorage;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC         gl3wNamedRenderbufferStorageMultisample;
PFNGLNAMEDSTRINGARBPROC                              gl3wNamedStringARB;
PFNGLOBJECTLABELPROC                                 gl3wObjectLabel;
PFNGLOBJECTPTRLABELPROC                              gl3wObjectPtrLabel;
PFNGLPATCHPARAMETERFVPROC                            gl3wPatchParameterfv;
PFNGLPATCHPARAMETERIPROC                             gl3wPatchParameteri;
PFNGLPAUSETRANSFORMFEEDBACKPROC                      gl3wPauseTransformFeedback;
PFNGLPIXELSTOREFPROC                                 gl3wPixelStoref;
PFNGLPIXELSTOREIPROC                                 gl3wPixelStorei;
PFNGLPOINTPARAMETERFPROC                             gl3wPointParameterf;
PFNGLPOINTPARAMETERFVPROC                            gl3wPointParameterfv;
PFNGLPOINTPARAMETERIPROC                             gl3wPointParameteri;
PFNGLPOINTPARAMETERIVPROC                            gl3wPointParameteriv;
PFNGLPOINTSIZEPROC                                   gl3wPointSize;
PFNGLPOLYGONMODEPROC                                 gl3wPolygonMode;
PFNGLPOLYGONOFFSETPROC                               gl3wPolygonOffset;
PFNGLPOPDEBUGGROUPPROC                               gl3wPopDebugGroup;
PFNGLPRIMITIVERESTARTINDEXPROC                       gl3wPrimitiveRestartIndex;
PFNGLPROGRAMBINARYPROC                               gl3wProgramBinary;
PFNGLPROGRAMPARAMETERIPROC                           gl3wProgramParameteri;
PFNGLPROGRAMUNIFORM1DPROC                            gl3wProgramUniform1d;
PFNGLPROGRAMUNIFORM1DVPROC                           gl3wProgramUniform1dv;
PFNGLPROGRAMUNIFORM1FPROC                            gl3wProgramUniform1f;
PFNGLPROGRAMUNIFORM1FVPROC                           gl3wProgramUniform1fv;
PFNGLPROGRAMUNIFORM1IPROC                            gl3wProgramUniform1i;
PFNGLPROGRAMUNIFORM1IVPROC                           gl3wProgramUniform1iv;
PFNGLPROGRAMUNIFORM1UIPROC                           gl3wProgramUniform1ui;
PFNGLPROGRAMUNIFORM1UIVPROC                          gl3wProgramUniform1uiv;
PFNGLPROGRAMUNIFORM2DPROC                            gl3wProgramUniform2d;
PFNGLPROGRAMUNIFORM2DVPROC                           gl3wProgramUniform2dv;
PFNGLPROGRAMUNIFORM2FPROC                            gl3wProgramUniform2f;
PFNGLPROGRAMUNIFORM2FVPROC                           gl3wProgramUniform2fv;
PFNGLPROGRAMUNIFORM2IPROC                            gl3wProgramUniform2i;
PFNGLPROGRAMUNIFORM2IVPROC                           gl3wProgramUniform2iv;
PFNGLPROGRAMUNIFORM2UIPROC                           gl3wProgramUniform2ui;
PFNGLPROGRAMUNIFORM2UIVPROC                          gl3wProgramUniform2uiv;
PFNGLPROGRAMUNIFORM3DPROC                            gl3wProgramUniform3d;
PFNGLPROGRAMUNIFORM3DVPROC                           gl3wProgramUniform3dv;
PFNGLPROGRAMUNIFORM3FPROC                            gl3wProgramUniform3f;
PFNGLPROGRAMUNIFORM3FVPROC                           gl3wProgramUniform3fv;
PFNGLPROGRAMUNIFORM3IPROC                            gl3wProgramUniform3i;
PFNGLPROGRAMUNIFORM3IVPROC                           gl3wProgramUniform3iv;
PFNGLPROGRAMUNIFORM3UIPROC                           gl3wProgramUniform3ui;
PFNGLPROGRAMUNIFORM3UIVPROC                          gl3wProgramUniform3uiv;
PFNGLPROGRAMUNIFORM4DPROC                            gl3wProgramUniform4d;
PFNGLPROGRAMUNIFORM4DVPROC                           gl3wProgramUniform4dv;
PFNGLPROGRAMUNIFORM4FPROC                            gl3wProgramUniform4f;
PFNGLPROGRAMUNIFORM4FVPROC                           gl3wProgramUniform4fv;
PFNGLPROGRAMUNIFORM4IPROC                            gl3wProgramUniform4i;
PFNGLPROGRAMUNIFORM4IVPROC                           gl3wProgramUniform4iv;
PFNGLPROGRAMUNIFORM4UIPROC                           gl3wProgramUniform4ui;
PFNGLPROGRAMUNIFORM4UIVPROC                          gl3wProgramUniform4uiv;
PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC                 gl3wProgramUniformHandleui64ARB;
PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC                gl3wProgramUniformHandleui64vARB;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC                     gl3wProgramUniformMatrix2dv;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC                     gl3wProgramUniformMatrix2fv;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC                   gl3wProgramUniformMatrix2x3dv;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC                   gl3wProgramUniformMatrix2x3fv;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC                   gl3wProgramUniformMatrix2x4dv;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC                   gl3wProgramUniformMatrix2x4fv;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC                     gl3wProgramUniformMatrix3dv;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC                     gl3wProgramUniformMatrix3fv;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC                   gl3wProgramUniformMatrix3x2dv;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC                   gl3wProgramUniformMatrix3x2fv;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC                   gl3wProgramUniformMatrix3x4dv;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC                   gl3wProgramUniformMatrix3x4fv;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC                     gl3wProgramUniformMatrix4dv;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC                     gl3wProgramUniformMatrix4fv;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC                   gl3wProgramUniformMatrix4x2dv;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC                   gl3wProgramUniformMatrix4x2fv;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC                   gl3wProgramUniformMatrix4x3dv;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC                   gl3wProgramUniformMatrix4x3fv;
PFNGLPROVOKINGVERTEXPROC                             gl3wProvokingVertex;
PFNGLPUSHDEBUGGROUPPROC                              gl3wPushDebugGroup;
PFNGLQUERYCOUNTERPROC                                gl3wQueryCounter;
PFNGLREADBUFFERPROC                                  gl3wReadBuffer;
PFNGLREADPIXELSPROC                                  gl3wReadPixels;
PFNGLREADNPIXELSPROC                                 gl3wReadnPixels;
PFNGLREADNPIXELSARBPROC                              gl3wReadnPixelsARB;
PFNGLRELEASESHADERCOMPILERPROC                       gl3wReleaseShaderCompiler;
PFNGLRENDERBUFFERSTORAGEPROC                         gl3wRenderbufferStorage;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC              gl3wRenderbufferStorageMultisample;
PFNGLRESUMETRANSFORMFEEDBACKPROC                     gl3wResumeTransformFeedback;
PFNGLSAMPLECOVERAGEPROC                              gl3wSampleCoverage;
PFNGLSAMPLEMASKIPROC                                 gl3wSampleMaski;
PFNGLSAMPLERPARAMETERIIVPROC                         gl3wSamplerParameterIiv;
PFNGLSAMPLERPARAMETERIUIVPROC                        gl3wSamplerParameterIuiv;
PFNGLSAMPLERPARAMETERFPROC                           gl3wSamplerParameterf;
PFNGLSAMPLERPARAMETERFVPROC                          gl3wSamplerParameterfv;
PFNGLSAMPLERPARAMETERIPROC                           gl3wSamplerParameteri;
PFNGLSAMPLERPARAMETERIVPROC                          gl3wSamplerParameteriv;
PFNGLSCISSORPROC                                     gl3wScissor;
PFNGLSCISSORARRAYVPROC                               gl3wScissorArrayv;
PFNGLSCISSORINDEXEDPROC                              gl3wScissorIndexed;
PFNGLSCISSORINDEXEDVPROC                             gl3wScissorIndexedv;
PFNGLSHADERBINARYPROC                                gl3wShaderBinary;
PFNGLSHADERSOURCEPROC                                gl3wShaderSource;
PFNGLSHADERSTORAGEBLOCKBINDINGPROC                   gl3wShaderStorageBlockBinding;
PFNGLSTENCILFUNCPROC                                 gl3wStencilFunc;
PFNGLSTENCILFUNCSEPARATEPROC                         gl3wStencilFuncSeparate;
PFNGLSTENCILMASKPROC                                 gl3wStencilMask;
PFNGLSTENCILMASKSEPARATEPROC                         gl3wStencilMaskSeparate;
PFNGLSTENCILOPPROC                                   gl3wStencilOp;
PFNGLSTENCILOPSEPARATEPROC                           gl3wStencilOpSeparate;
PFNGLTEXBUFFERPROC                                   gl3wTexBuffer;
PFNGLTEXBUFFERRANGEPROC                              gl3wTexBufferRange;
PFNGLTEXIMAGE1DPROC                                  gl3wTexImage1D;
PFNGLTEXIMAGE2DPROC                                  gl3wTexImage2D;
PFNGLTEXIMAGE2DMULTISAMPLEPROC                       gl3wTexImage2DMultisample;
PFNGLTEXIMAGE3DPROC                                  gl3wTexImage3D;
PFNGLTEXIMAGE3DMULTISAMPLEPROC                       gl3wTexImage3DMultisample;
PFNGLTEXPAGECOMMITMENTARBPROC                        gl3wTexPageCommitmentARB;
PFNGLTEXPARAMETERIIVPROC                             gl3wTexParameterIiv;
PFNGLTEXPARAMETERIUIVPROC                            gl3wTexParameterIuiv;
PFNGLTEXPARAMETERFPROC                               gl3wTexParameterf;
PFNGLTEXPARAMETERFVPROC                              gl3wTexParameterfv;
PFNGLTEXPARAMETERIPROC                               gl3wTexParameteri;
PFNGLTEXPARAMETERIVPROC                              gl3wTexParameteriv;
PFNGLTEXSTORAGE1DPROC                                gl3wTexStorage1D;
PFNGLTEXSTORAGE2DPROC                                gl3wTexStorage2D;
PFNGLTEXSTORAGE2DMULTISAMPLEPROC                     gl3wTexStorage2DMultisample;
PFNGLTEXSTORAGE3DPROC                                gl3wTexStorage3D;
PFNGLTEXSTORAGE3DMULTISAMPLEPROC                     gl3wTexStorage3DMultisample;
PFNGLTEXSUBIMAGE1DPROC                               gl3wTexSubImage1D;
PFNGLTEXSUBIMAGE2DPROC                               gl3wTexSubImage2D;
PFNGLTEXSUBIMAGE3DPROC                               gl3wTexSubImage3D;
PFNGLTEXTUREBARRIERPROC                              gl3wTextureBarrier;
PFNGLTEXTUREBUFFERPROC                               gl3wTextureBuffer;
PFNGLTEXTUREBUFFERRANGEPROC                          gl3wTextureBufferRange;
PFNGLTEXTUREPARAMETERIIVPROC                         gl3wTextureParameterIiv;
PFNGLTEXTUREPARAMETERIUIVPROC                        gl3wTextureParameterIuiv;
PFNGLTEXTUREPARAMETERFPROC                           gl3wTextureParameterf;
PFNGLTEXTUREPARAMETERFVPROC                          gl3wTextureParameterfv;
PFNGLTEXTUREPARAMETERIPROC                           gl3wTextureParameteri;
PFNGLTEXTUREPARAMETERIVPROC                          gl3wTextureParameteriv;
PFNGLTEXTURESTORAGE1DPROC                            gl3wTextureStorage1D;
PFNGLTEXTURESTORAGE2DPROC                            gl3wTextureStorage2D;
PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC                 gl3wTextureStorage2DMultisample;
PFNGLTEXTURESTORAGE3DPROC                            gl3wTextureStorage3D;
PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC                 gl3wTextureStorage3DMultisample;
PFNGLTEXTURESUBIMAGE1DPROC                           gl3wTextureSubImage1D;
PFNGLTEXTURESUBIMAGE2DPROC                           gl3wTextureSubImage2D;
PFNGLTEXTURESUBIMAGE3DPROC                           gl3wTextureSubImage3D;
PFNGLTEXTUREVIEWPROC                                 gl3wTextureView;
PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC                 gl3wTransformFeedbackBufferBase;
PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC                gl3wTransformFeedbackBufferRange;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC                   gl3wTransformFeedbackVaryings;
PFNGLUNIFORM1DPROC                                   gl3wUniform1d;
PFNGLUNIFORM1DVPROC                                  gl3wUniform1dv;
PFNGLUNIFORM1FPROC                                   gl3wUniform1f;
PFNGLUNIFORM1FVPROC                                  gl3wUniform1fv;
PFNGLUNIFORM1IPROC                                   gl3wUniform1i;
PFNGLUNIFORM1IVPROC                                  gl3wUniform1iv;
PFNGLUNIFORM1UIPROC                                  gl3wUniform1ui;
PFNGLUNIFORM1UIVPROC                                 gl3wUniform1uiv;
PFNGLUNIFORM2DPROC                                   gl3wUniform2d;
PFNGLUNIFORM2DVPROC                                  gl3wUniform2dv;
PFNGLUNIFORM2FPROC                                   gl3wUniform2f;
PFNGLUNIFORM2FVPROC                                  gl3wUniform2fv;
PFNGLUNIFORM2IPROC                                   gl3wUniform2i;
PFNGLUNIFORM2IVPROC                                  gl3wUniform2iv;
PFNGLUNIFORM2UIPROC                                  gl3wUniform2ui;
PFNGLUNIFORM2UIVPROC                                 gl3wUniform2uiv;
PFNGLUNIFORM3DPROC                                   gl3wUniform3d;
PFNGLUNIFORM3DVPROC                                  gl3wUniform3dv;
PFNGLUNIFORM3FPROC                                   gl3wUniform3f;
PFNGLUNIFORM3FVPROC                                  gl3wUniform3fv;
PFNGLUNIFORM3IPROC                                   gl3wUniform3i;
PFNGLUNIFORM3IVPROC                                  gl3wUniform3iv;
PFNGLUNIFORM3UIPROC                                  gl3wUniform3ui;
PFNGLUNIFORM3UIVPROC                                 gl3wUniform3uiv;
PFNGLUNIFORM4DPROC                                   gl3wUniform4d;
PFNGLUNIFORM4DVPROC                                  gl3wUniform4dv;
PFNGLUNIFORM4FPROC                                   gl3wUniform4f;
PFNGLUNIFORM4FVPROC                                  gl3wUniform4fv;
PFNGLUNIFORM4IPROC                                   gl3wUniform4i;
PFNGLUNIFORM4IVPROC                                  gl3wUniform4iv;
PFNGLUNIFORM4UIPROC                                  gl3wUniform4ui;
PFNGLUNIFORM4UIVPROC                                 gl3wUniform4uiv;
PFNGLUNIFORMBLOCKBINDINGPROC                         gl3wUniformBlockBinding;
PFNGLUNIFORMHANDLEUI64ARBPROC                        gl3wUniformHandleui64ARB;
PFNGLUNIFORMHANDLEUI64VARBPROC                       gl3wUniformHandleui64vARB;
PFNGLUNIFORMMATRIX2DVPROC                            gl3wUniformMatrix2dv;
PFNGLUNIFORMMATRIX2FVPROC                            gl3wUniformMatrix2fv;
PFNGLUNIFORMMATRIX2X3DVPROC                          gl3wUniformMatrix2x3dv;
PFNGLUNIFORMMATRIX2X3FVPROC                          gl3wUniformMatrix2x3fv;
PFNGLUNIFORMMATRIX2X4DVPROC                          gl3wUniformMatrix2x4dv;
PFNGLUNIFORMMATRIX2X4FVPROC                          gl3wUniformMatrix2x4fv;
PFNGLUNIFORMMATRIX3DVPROC                            gl3wUniformMatrix3dv;
PFNGLUNIFORMMATRIX3FVPROC                            gl3wUniformMatrix3fv;
PFNGLUNIFORMMATRIX3X2DVPROC                          gl3wUniformMatrix3x2dv;
PFNGLUNIFORMMATRIX3X2FVPROC                          gl3wUniformMatrix3x2fv;
PFNGLUNIFORMMATRIX3X4DVPROC                          gl3wUniformMatrix3x4dv;
PFNGLUNIFORMMATRIX3X4FVPROC                          gl3wUniformMatrix3x4fv;
PFNGLUNIFORMMATRIX4DVPROC                            gl3wUniformMatrix4dv;
PFNGLUNIFORMMATRIX4FVPROC                            gl3wUniformMatrix4fv;
PFNGLUNIFORMMATRIX4X2DVPROC                          gl3wUniformMatrix4x2dv;
PFNGLUNIFORMMATRIX4X2FVPROC                          gl3wUniformMatrix4x2fv;
PFNGLUNIFORMMATRIX4X3DVPROC                          gl3wUniformMatrix4x3dv;
PFNGLUNIFORMMATRIX4X3FVPROC                          gl3wUniformMatrix4x3fv;
PFNGLUNIFORMSUBROUTINESUIVPROC                       gl3wUniformSubroutinesuiv;
PFNGLUNMAPBUFFERPROC                                 gl3wUnmapBuffer;
PFNGLUNMAPNAMEDBUFFERPROC                            gl3wUnmapNamedBuffer;
PFNGLUSEPROGRAMPROC                                  gl3wUseProgram;
PFNGLUSEPROGRAMSTAGESPROC                            gl3wUseProgramStages;
PFNGLVALIDATEPROGRAMPROC                             gl3wValidateProgram;
PFNGLVALIDATEPROGRAMPIPELINEPROC                     gl3wValidateProgramPipeline;
PFNGLVERTEXARRAYATTRIBBINDINGPROC                    gl3wVertexArrayAttribBinding;
PFNGLVERTEXARRAYATTRIBFORMATPROC                     gl3wVertexArrayAttribFormat;
PFNGLVERTEXARRAYATTRIBIFORMATPROC                    gl3wVertexArrayAttribIFormat;
PFNGLVERTEXARRAYATTRIBLFORMATPROC                    gl3wVertexArrayAttribLFormat;
PFNGLVERTEXARRAYBINDINGDIVISORPROC                   gl3wVertexArrayBindingDivisor;
PFNGLVERTEXARRAYELEMENTBUFFERPROC                    gl3wVertexArrayElementBuffer;
PFNGLVERTEXARRAYVERTEXBUFFERPROC                     gl3wVertexArrayVertexBuffer;
PFNGLVERTEXARRAYVERTEXBUFFERSPROC                    gl3wVertexArrayVertexBuffers;
PFNGLVERTEXATTRIB1DPROC                              gl3wVertexAttrib1d;
PFNGLVERTEXATTRIB1DVPROC                             gl3wVertexAttrib1dv;
PFNGLVERTEXATTRIB1FPROC                              gl3wVertexAttrib1f;
PFNGLVERTEXATTRIB1FVPROC                             gl3wVertexAttrib1fv;
PFNGLVERTEXATTRIB1SPROC                              gl3wVertexAttrib1s;
PFNGLVERTEXATTRIB1SVPROC                             gl3wVertexAttrib1sv;
PFNGLVERTEXATTRIB2DPROC                              gl3wVertexAttrib2d;
PFNGLVERTEXATTRIB2DVPROC                             gl3wVertexAttrib2dv;
PFNGLVERTEXATTRIB2FPROC                              gl3wVertexAttrib2f;
PFNGLVERTEXATTRIB2FVPROC                             gl3wVertexAttrib2fv;
PFNGLVERTEXATTRIB2SPROC                              gl3wVertexAttrib2s;
PFNGLVERTEXATTRIB2SVPROC                             gl3wVertexAttrib2sv;
PFNGLVERTEXATTRIB3DPROC                              gl3wVertexAttrib3d;
PFNGLVERTEXATTRIB3DVPROC                             gl3wVertexAttrib3dv;
PFNGLVERTEXATTRIB3FPROC                              gl3wVertexAttrib3f;
PFNGLVERTEXATTRIB3FVPROC                             gl3wVertexAttrib3fv;
PFNGLVERTEXATTRIB3SPROC                              gl3wVertexAttrib3s;
PFNGLVERTEXATTRIB3SVPROC                             gl3wVertexAttrib3sv;
PFNGLVERTEXATTRIB4NBVPROC                            gl3wVertexAttrib4Nbv;
PFNGLVERTEXATTRIB4NIVPROC                            gl3wVertexAttrib4Niv;
PFNGLVERTEXATTRIB4NSVPROC                            gl3wVertexAttrib4Nsv;
PFNGLVERTEXATTRIB4NUBPROC                            gl3wVertexAttrib4Nub;
PFNGLVERTEXATTRIB4NUBVPROC                           gl3wVertexAttrib4Nubv;
PFNGLVERTEXATTRIB4NUIVPROC                           gl3wVertexAttrib4Nuiv;
PFNGLVERTEXATTRIB4NUSVPROC                           gl3wVertexAttrib4Nusv;
PFNGLVERTEXATTRIB4BVPROC                             gl3wVertexAttrib4bv;
PFNGLVERTEXATTRIB4DPROC                              gl3wVertexAttrib4d;
PFNGLVERTEXATTRIB4DVPROC                             gl3wVertexAttrib4dv;
PFNGLVERTEXATTRIB4FPROC                              gl3wVertexAttrib4f;
PFNGLVERTEXATTRIB4FVPROC                             gl3wVertexAttrib4fv;
PFNGLVERTEXATTRIB4IVPROC                             gl3wVertexAttrib4iv;
PFNGLVERTEXATTRIB4SPROC                              gl3wVertexAttrib4s;
PFNGLVERTEXATTRIB4SVPROC                             gl3wVertexAttrib4sv;
PFNGLVERTEXATTRIB4UBVPROC                            gl3wVertexAttrib4ubv;
PFNGLVERTEXATTRIB4UIVPROC                            gl3wVertexAttrib4uiv;
PFNGLVERTEXATTRIB4USVPROC                            gl3wVertexAttrib4usv;
PFNGLVERTEXATTRIBBINDINGPROC                         gl3wVertexAttribBinding;
PFNGLVERTEXATTRIBDIVISORPROC                         gl3wVertexAttribDivisor;
PFNGLVERTEXATTRIBFORMATPROC                          gl3wVertexAttribFormat;
PFNGLVERTEXATTRIBI1IPROC                             gl3wVertexAttribI1i;
PFNGLVERTEXATTRIBI1IVPROC                            gl3wVertexAttribI1iv;
PFNGLVERTEXATTRIBI1UIPROC                            gl3wVertexAttribI1ui;
PFNGLVERTEXATTRIBI1UIVPROC                           gl3wVertexAttribI1uiv;
PFNGLVERTEXATTRIBI2IPROC                             gl3wVertexAttribI2i;
PFNGLVERTEXATTRIBI2IVPROC                            gl3wVertexAttribI2iv;
PFNGLVERTEXATTRIBI2UIPROC                            gl3wVertexAttribI2ui;
PFNGLVERTEXATTRIBI2UIVPROC                           gl3wVertexAttribI2uiv;
PFNGLVERTEXATTRIBI3IPROC                             gl3wVertexAttribI3i;
PFNGLVERTEXATTRIBI3IVPROC                            gl3wVertexAttribI3iv;
PFNGLVERTEXATTRIBI3UIPROC                            gl3wVertexAttribI3ui;
PFNGLVERTEXATTRIBI3UIVPROC                           gl3wVertexAttribI3uiv;
PFNGLVERTEXATTRIBI4BVPROC                            gl3wVertexAttribI4bv;
PFNGLVERTEXATTRIBI4IPROC                             gl3wVertexAttribI4i;
PFNGLVERTEXATTRIBI4IVPROC                            gl3wVertexAttribI4iv;
PFNGLVERTEXATTRIBI4SVPROC                            gl3wVertexAttribI4sv;
PFNGLVERTEXATTRIBI4UBVPROC                           gl3wVertexAttribI4ubv;
PFNGLVERTEXATTRIBI4UIPROC                            gl3wVertexAttribI4ui;
PFNGLVERTEXATTRIBI4UIVPROC                           gl3wVertexAttribI4uiv;
PFNGLVERTEXATTRIBI4USVPROC                           gl3wVertexAttribI4usv;
PFNGLVERTEXATTRIBIFORMATPROC                         gl3wVertexAttribIFormat;
PFNGLVERTEXATTRIBIPOINTERPROC                        gl3wVertexAttribIPointer;
PFNGLVERTEXATTRIBL1DPROC                             gl3wVertexAttribL1d;
PFNGLVERTEXATTRIBL1DVPROC                            gl3wVertexAttribL1dv;
PFNGLVERTEXATTRIBL1UI64ARBPROC                       gl3wVertexAttribL1ui64ARB;
PFNGLVERTEXATTRIBL1UI64VARBPROC                      gl3wVertexAttribL1ui64vARB;
PFNGLVERTEXATTRIBL2DPROC                             gl3wVertexAttribL2d;
PFNGLVERTEXATTRIBL2DVPROC                            gl3wVertexAttribL2dv;
PFNGLVERTEXATTRIBL3DPROC                             gl3wVertexAttribL3d;
PFNGLVERTEXATTRIBL3DVPROC                            gl3wVertexAttribL3dv;
PFNGLVERTEXATTRIBL4DPROC                             gl3wVertexAttribL4d;
PFNGLVERTEXATTRIBL4DVPROC                            gl3wVertexAttribL4dv;
PFNGLVERTEXATTRIBLFORMATPROC                         gl3wVertexAttribLFormat;
PFNGLVERTEXATTRIBLPOINTERPROC                        gl3wVertexAttribLPointer;
PFNGLVERTEXATTRIBP1UIPROC                            gl3wVertexAttribP1ui;
PFNGLVERTEXATTRIBP1UIVPROC                           gl3wVertexAttribP1uiv;
PFNGLVERTEXATTRIBP2UIPROC                            gl3wVertexAttribP2ui;
PFNGLVERTEXATTRIBP2UIVPROC                           gl3wVertexAttribP2uiv;
PFNGLVERTEXATTRIBP3UIPROC                            gl3wVertexAttribP3ui;
PFNGLVERTEXATTRIBP3UIVPROC                           gl3wVertexAttribP3uiv;
PFNGLVERTEXATTRIBP4UIPROC                            gl3wVertexAttribP4ui;
PFNGLVERTEXATTRIBP4UIVPROC                           gl3wVertexAttribP4uiv;
PFNGLVERTEXATTRIBPOINTERPROC                         gl3wVertexAttribPointer;
PFNGLVERTEXBINDINGDIVISORPROC                        gl3wVertexBindingDivisor;
PFNGLVIEWPORTPROC                                    gl3wViewport;
PFNGLVIEWPORTARRAYVPROC                              gl3wViewportArrayv;
PFNGLVIEWPORTINDEXEDFPROC                            gl3wViewportIndexedf;
PFNGLVIEWPORTINDEXEDFVPROC                           gl3wViewportIndexedfv;
PFNGLWAITSYNCPROC                                    gl3wWaitSync;

/* --------------------------------------------------------------------------------------------- */

static void gl3w_load_all_functions(void)
{
	gl3wActiveShaderProgram                         = ( PFNGLACTIVESHADERPROGRAMPROC                         ) gl3w_fn("glActiveShaderProgram");
	gl3wActiveTexture                               = ( PFNGLACTIVETEXTUREPROC                               ) gl3w_fn("glActiveTexture");
	gl3wAttachShader                                = ( PFNGLATTACHSHADERPROC                                ) gl3w_fn("glAttachShader");
	gl3wBeginConditionalRender                      = ( PFNGLBEGINCONDITIONALRENDERPROC                      ) gl3w_fn("glBeginConditionalRender");
	gl3wBeginQuery                                  = ( PFNGLBEGINQUERYPROC                                  ) gl3w_fn("glBeginQuery");
	gl3wBeginQueryIndexed                           = ( PFNGLBEGINQUERYINDEXEDPROC                           ) gl3w_fn("glBeginQueryIndexed");
	gl3wBeginTransformFeedback                      = ( PFNGLBEGINTRANSFORMFEEDBACKPROC                      ) gl3w_fn("glBeginTransformFeedback");
	gl3wBindAttribLocation                          = ( PFNGLBINDATTRIBLOCATIONPROC                          ) gl3w_fn("glBindAttribLocation");
	gl3wBindBuffer                                  = ( PFNGLBINDBUFFERPROC                                  ) gl3w_fn("glBindBuffer");
	gl3wBindBufferBase                              = ( PFNGLBINDBUFFERBASEPROC                              ) gl3w_fn("glBindBufferBase");
	gl3wBindBufferRange                             = ( PFNGLBINDBUFFERRANGEPROC                             ) gl3w_fn("glBindBufferRange");
	gl3wBindBuffersBase                             = ( PFNGLBINDBUFFERSBASEPROC                             ) gl3w_fn("glBindBuffersBase");
	gl3wBindBuffersRange                            = ( PFNGLBINDBUFFERSRANGEPROC                            ) gl3w_fn("glBindBuffersRange");
	gl3wBindFragDataLocation                        = ( PFNGLBINDFRAGDATALOCATIONPROC                        ) gl3w_fn("glBindFragDataLocation");
	gl3wBindFragDataLocationIndexed                 = ( PFNGLBINDFRAGDATALOCATIONINDEXEDPROC                 ) gl3w_fn("glBindFragDataLocationIndexed");
	gl3wBindFramebuffer                             = ( PFNGLBINDFRAMEBUFFERPROC                             ) gl3w_fn("glBindFramebuffer");
	gl3wBindImageTexture                            = ( PFNGLBINDIMAGETEXTUREPROC                            ) gl3w_fn("glBindImageTexture");
	gl3wBindImageTextures                           = ( PFNGLBINDIMAGETEXTURESPROC                           ) gl3w_fn("glBindImageTextures");
	gl3wBindProgramPipeline                         = ( PFNGLBINDPROGRAMPIPELINEPROC                         ) gl3w_fn("glBindProgramPipeline");
	gl3wBindRenderbuffer                            = ( PFNGLBINDRENDERBUFFERPROC                            ) gl3w_fn("glBindRenderbuffer");
	gl3wBindSampler                                 = ( PFNGLBINDSAMPLERPROC                                 ) gl3w_fn("glBindSampler");
	gl3wBindSamplers                                = ( PFNGLBINDSAMPLERSPROC                                ) gl3w_fn("glBindSamplers");
	gl3wBindTexture                                 = ( PFNGLBINDTEXTUREPROC                                 ) gl3w_fn("glBindTexture");
	gl3wBindTextureUnit                             = ( PFNGLBINDTEXTUREUNITPROC                             ) gl3w_fn("glBindTextureUnit");
	gl3wBindTextures                                = ( PFNGLBINDTEXTURESPROC                                ) gl3w_fn("glBindTextures");
	gl3wBindTransformFeedback                       = ( PFNGLBINDTRANSFORMFEEDBACKPROC                       ) gl3w_fn("glBindTransformFeedback");
	gl3wBindVertexArray                             = ( PFNGLBINDVERTEXARRAYPROC                             ) gl3w_fn("glBindVertexArray");
	gl3wBindVertexBuffer                            = ( PFNGLBINDVERTEXBUFFERPROC                            ) gl3w_fn("glBindVertexBuffer");
	gl3wBindVertexBuffers                           = ( PFNGLBINDVERTEXBUFFERSPROC                           ) gl3w_fn("glBindVertexBuffers");
	gl3wBlendColor                                  = ( PFNGLBLENDCOLORPROC                                  ) gl3w_fn("glBlendColor");
	gl3wBlendEquation                               = ( PFNGLBLENDEQUATIONPROC                               ) gl3w_fn("glBlendEquation");
	gl3wBlendEquationSeparate                       = ( PFNGLBLENDEQUATIONSEPARATEPROC                       ) gl3w_fn("glBlendEquationSeparate");
	gl3wBlendEquationSeparatei                      = ( PFNGLBLENDEQUATIONSEPARATEIPROC                      ) gl3w_fn("glBlendEquationSeparatei");
	gl3wBlendEquationSeparateiARB                   = ( PFNGLBLENDEQUATIONSEPARATEIARBPROC                   ) gl3w_fn("glBlendEquationSeparateiARB");
	gl3wBlendEquationi                              = ( PFNGLBLENDEQUATIONIPROC                              ) gl3w_fn("glBlendEquationi");
	gl3wBlendEquationiARB                           = ( PFNGLBLENDEQUATIONIARBPROC                           ) gl3w_fn("glBlendEquationiARB");
	gl3wBlendFunc                                   = ( PFNGLBLENDFUNCPROC                                   ) gl3w_fn("glBlendFunc");
	gl3wBlendFuncSeparate                           = ( PFNGLBLENDFUNCSEPARATEPROC                           ) gl3w_fn("glBlendFuncSeparate");
	gl3wBlendFuncSeparatei                          = ( PFNGLBLENDFUNCSEPARATEIPROC                          ) gl3w_fn("glBlendFuncSeparatei");
	gl3wBlendFuncSeparateiARB                       = ( PFNGLBLENDFUNCSEPARATEIARBPROC                       ) gl3w_fn("glBlendFuncSeparateiARB");
	gl3wBlendFunci                                  = ( PFNGLBLENDFUNCIPROC                                  ) gl3w_fn("glBlendFunci");
	gl3wBlendFunciARB                               = ( PFNGLBLENDFUNCIARBPROC                               ) gl3w_fn("glBlendFunciARB");
	gl3wBlitFramebuffer                             = ( PFNGLBLITFRAMEBUFFERPROC                             ) gl3w_fn("glBlitFramebuffer");
	gl3wBlitNamedFramebuffer                        = ( PFNGLBLITNAMEDFRAMEBUFFERPROC                        ) gl3w_fn("glBlitNamedFramebuffer");
	gl3wBufferData                                  = ( PFNGLBUFFERDATAPROC                                  ) gl3w_fn("glBufferData");
	gl3wBufferPageCommitmentARB                     = ( PFNGLBUFFERPAGECOMMITMENTARBPROC                     ) gl3w_fn("glBufferPageCommitmentARB");
	gl3wBufferStorage                               = ( PFNGLBUFFERSTORAGEPROC                               ) gl3w_fn("glBufferStorage");
	gl3wBufferSubData                               = ( PFNGLBUFFERSUBDATAPROC                               ) gl3w_fn("glBufferSubData");
	gl3wCheckFramebufferStatus                      = ( PFNGLCHECKFRAMEBUFFERSTATUSPROC                      ) gl3w_fn("glCheckFramebufferStatus");
	gl3wCheckNamedFramebufferStatus                 = ( PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC                 ) gl3w_fn("glCheckNamedFramebufferStatus");
	gl3wClampColor                                  = ( PFNGLCLAMPCOLORPROC                                  ) gl3w_fn("glClampColor");
	gl3wClear                                       = ( PFNGLCLEARPROC                                       ) gl3w_fn("glClear");
	gl3wClearBufferData                             = ( PFNGLCLEARBUFFERDATAPROC                             ) gl3w_fn("glClearBufferData");
	gl3wClearBufferSubData                          = ( PFNGLCLEARBUFFERSUBDATAPROC                          ) gl3w_fn("glClearBufferSubData");
	gl3wClearBufferfi                               = ( PFNGLCLEARBUFFERFIPROC                               ) gl3w_fn("glClearBufferfi");
	gl3wClearBufferfv                               = ( PFNGLCLEARBUFFERFVPROC                               ) gl3w_fn("glClearBufferfv");
	gl3wClearBufferiv                               = ( PFNGLCLEARBUFFERIVPROC                               ) gl3w_fn("glClearBufferiv");
	gl3wClearBufferuiv                              = ( PFNGLCLEARBUFFERUIVPROC                              ) gl3w_fn("glClearBufferuiv");
	gl3wClearColor                                  = ( PFNGLCLEARCOLORPROC                                  ) gl3w_fn("glClearColor");
	gl3wClearDepth                                  = ( PFNGLCLEARDEPTHPROC                                  ) gl3w_fn("glClearDepth");
	gl3wClearDepthf                                 = ( PFNGLCLEARDEPTHFPROC                                 ) gl3w_fn("glClearDepthf");
	gl3wClearNamedBufferData                        = ( PFNGLCLEARNAMEDBUFFERDATAPROC                        ) gl3w_fn("glClearNamedBufferData");
	gl3wClearNamedBufferSubData                     = ( PFNGLCLEARNAMEDBUFFERSUBDATAPROC                     ) gl3w_fn("glClearNamedBufferSubData");
	gl3wClearNamedFramebufferfi                     = ( PFNGLCLEARNAMEDFRAMEBUFFERFIPROC                     ) gl3w_fn("glClearNamedFramebufferfi");
	gl3wClearNamedFramebufferfv                     = ( PFNGLCLEARNAMEDFRAMEBUFFERFVPROC                     ) gl3w_fn("glClearNamedFramebufferfv");
	gl3wClearNamedFramebufferiv                     = ( PFNGLCLEARNAMEDFRAMEBUFFERIVPROC                     ) gl3w_fn("glClearNamedFramebufferiv");
	gl3wClearNamedFramebufferuiv                    = ( PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC                    ) gl3w_fn("glClearNamedFramebufferuiv");
	gl3wClearStencil                                = ( PFNGLCLEARSTENCILPROC                                ) gl3w_fn("glClearStencil");
	gl3wClearTexImage                               = ( PFNGLCLEARTEXIMAGEPROC                               ) gl3w_fn("glClearTexImage");
	gl3wClearTexSubImage                            = ( PFNGLCLEARTEXSUBIMAGEPROC                            ) gl3w_fn("glClearTexSubImage");
	gl3wClientWaitSync                              = ( PFNGLCLIENTWAITSYNCPROC                              ) gl3w_fn("glClientWaitSync");
	gl3wClipControl                                 = ( PFNGLCLIPCONTROLPROC                                 ) gl3w_fn("glClipControl");
	gl3wColorMask                                   = ( PFNGLCOLORMASKPROC                                   ) gl3w_fn("glColorMask");
	gl3wColorMaski                                  = ( PFNGLCOLORMASKIPROC                                  ) gl3w_fn("glColorMaski");
	gl3wCompileShader                               = ( PFNGLCOMPILESHADERPROC                               ) gl3w_fn("glCompileShader");
	gl3wCompileShaderIncludeARB                     = ( PFNGLCOMPILESHADERINCLUDEARBPROC                     ) gl3w_fn("glCompileShaderIncludeARB");
	gl3wCompressedTexImage1D                        = ( PFNGLCOMPRESSEDTEXIMAGE1DPROC                        ) gl3w_fn("glCompressedTexImage1D");
	gl3wCompressedTexImage2D                        = ( PFNGLCOMPRESSEDTEXIMAGE2DPROC                        ) gl3w_fn("glCompressedTexImage2D");
	gl3wCompressedTexImage3D                        = ( PFNGLCOMPRESSEDTEXIMAGE3DPROC                        ) gl3w_fn("glCompressedTexImage3D");
	gl3wCompressedTexSubImage1D                     = ( PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC                     ) gl3w_fn("glCompressedTexSubImage1D");
	gl3wCompressedTexSubImage2D                     = ( PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC                     ) gl3w_fn("glCompressedTexSubImage2D");
	gl3wCompressedTexSubImage3D                     = ( PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC                     ) gl3w_fn("glCompressedTexSubImage3D");
	gl3wCompressedTextureSubImage1D                 = ( PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC                 ) gl3w_fn("glCompressedTextureSubImage1D");
	gl3wCompressedTextureSubImage2D                 = ( PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC                 ) gl3w_fn("glCompressedTextureSubImage2D");
	gl3wCompressedTextureSubImage3D                 = ( PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC                 ) gl3w_fn("glCompressedTextureSubImage3D");
	gl3wCopyBufferSubData                           = ( PFNGLCOPYBUFFERSUBDATAPROC                           ) gl3w_fn("glCopyBufferSubData");
	gl3wCopyImageSubData                            = ( PFNGLCOPYIMAGESUBDATAPROC                            ) gl3w_fn("glCopyImageSubData");
	gl3wCopyNamedBufferSubData                      = ( PFNGLCOPYNAMEDBUFFERSUBDATAPROC                      ) gl3w_fn("glCopyNamedBufferSubData");
	gl3wCopyTexImage1D                              = ( PFNGLCOPYTEXIMAGE1DPROC                              ) gl3w_fn("glCopyTexImage1D");
	gl3wCopyTexImage2D                              = ( PFNGLCOPYTEXIMAGE2DPROC                              ) gl3w_fn("glCopyTexImage2D");
	gl3wCopyTexSubImage1D                           = ( PFNGLCOPYTEXSUBIMAGE1DPROC                           ) gl3w_fn("glCopyTexSubImage1D");
	gl3wCopyTexSubImage2D                           = ( PFNGLCOPYTEXSUBIMAGE2DPROC                           ) gl3w_fn("glCopyTexSubImage2D");
	gl3wCopyTexSubImage3D                           = ( PFNGLCOPYTEXSUBIMAGE3DPROC                           ) gl3w_fn("glCopyTexSubImage3D");
	gl3wCopyTextureSubImage1D                       = ( PFNGLCOPYTEXTURESUBIMAGE1DPROC                       ) gl3w_fn("glCopyTextureSubImage1D");
	gl3wCopyTextureSubImage2D                       = ( PFNGLCOPYTEXTURESUBIMAGE2DPROC                       ) gl3w_fn("glCopyTextureSubImage2D");
	gl3wCopyTextureSubImage3D                       = ( PFNGLCOPYTEXTURESUBIMAGE3DPROC                       ) gl3w_fn("glCopyTextureSubImage3D");
	gl3wCreateBuffers                               = ( PFNGLCREATEBUFFERSPROC                               ) gl3w_fn("glCreateBuffers");
	gl3wCreateFramebuffers                          = ( PFNGLCREATEFRAMEBUFFERSPROC                          ) gl3w_fn("glCreateFramebuffers");
	gl3wCreateProgram                               = ( PFNGLCREATEPROGRAMPROC                               ) gl3w_fn("glCreateProgram");
	gl3wCreateProgramPipelines                      = ( PFNGLCREATEPROGRAMPIPELINESPROC                      ) gl3w_fn("glCreateProgramPipelines");
	gl3wCreateQueries                               = ( PFNGLCREATEQUERIESPROC                               ) gl3w_fn("glCreateQueries");
	gl3wCreateRenderbuffers                         = ( PFNGLCREATERENDERBUFFERSPROC                         ) gl3w_fn("glCreateRenderbuffers");
	gl3wCreateSamplers                              = ( PFNGLCREATESAMPLERSPROC                              ) gl3w_fn("glCreateSamplers");
	gl3wCreateShader                                = ( PFNGLCREATESHADERPROC                                ) gl3w_fn("glCreateShader");
	gl3wCreateShaderProgramv                        = ( PFNGLCREATESHADERPROGRAMVPROC                        ) gl3w_fn("glCreateShaderProgramv");
	gl3wCreateSyncFromCLeventARB                    = ( PFNGLCREATESYNCFROMCLEVENTARBPROC                    ) gl3w_fn("glCreateSyncFromCLeventARB");
	gl3wCreateTextures                              = ( PFNGLCREATETEXTURESPROC                              ) gl3w_fn("glCreateTextures");
	gl3wCreateTransformFeedbacks                    = ( PFNGLCREATETRANSFORMFEEDBACKSPROC                    ) gl3w_fn("glCreateTransformFeedbacks");
	gl3wCreateVertexArrays                          = ( PFNGLCREATEVERTEXARRAYSPROC                          ) gl3w_fn("glCreateVertexArrays");
	gl3wCullFace                                    = ( PFNGLCULLFACEPROC                                    ) gl3w_fn("glCullFace");
	gl3wDebugMessageCallback                        = ( PFNGLDEBUGMESSAGECALLBACKPROC                        ) gl3w_fn("glDebugMessageCallback");
	gl3wDebugMessageCallbackARB                     = ( PFNGLDEBUGMESSAGECALLBACKARBPROC                     ) gl3w_fn("glDebugMessageCallbackARB");
	gl3wDebugMessageControl                         = ( PFNGLDEBUGMESSAGECONTROLPROC                         ) gl3w_fn("glDebugMessageControl");
	gl3wDebugMessageControlARB                      = ( PFNGLDEBUGMESSAGECONTROLARBPROC                      ) gl3w_fn("glDebugMessageControlARB");
	gl3wDebugMessageInsert                          = ( PFNGLDEBUGMESSAGEINSERTPROC                          ) gl3w_fn("glDebugMessageInsert");
	gl3wDebugMessageInsertARB                       = ( PFNGLDEBUGMESSAGEINSERTARBPROC                       ) gl3w_fn("glDebugMessageInsertARB");
	gl3wDeleteBuffers                               = ( PFNGLDELETEBUFFERSPROC                               ) gl3w_fn("glDeleteBuffers");
	gl3wDeleteFramebuffers                          = ( PFNGLDELETEFRAMEBUFFERSPROC                          ) gl3w_fn("glDeleteFramebuffers");
	gl3wDeleteNamedStringARB                        = ( PFNGLDELETENAMEDSTRINGARBPROC                        ) gl3w_fn("glDeleteNamedStringARB");
	gl3wDeleteProgram                               = ( PFNGLDELETEPROGRAMPROC                               ) gl3w_fn("glDeleteProgram");
	gl3wDeleteProgramPipelines                      = ( PFNGLDELETEPROGRAMPIPELINESPROC                      ) gl3w_fn("glDeleteProgramPipelines");
	gl3wDeleteQueries                               = ( PFNGLDELETEQUERIESPROC                               ) gl3w_fn("glDeleteQueries");
	gl3wDeleteRenderbuffers                         = ( PFNGLDELETERENDERBUFFERSPROC                         ) gl3w_fn("glDeleteRenderbuffers");
	gl3wDeleteSamplers                              = ( PFNGLDELETESAMPLERSPROC                              ) gl3w_fn("glDeleteSamplers");
	gl3wDeleteShader                                = ( PFNGLDELETESHADERPROC                                ) gl3w_fn("glDeleteShader");
	gl3wDeleteSync                                  = ( PFNGLDELETESYNCPROC                                  ) gl3w_fn("glDeleteSync");
	gl3wDeleteTextures                              = ( PFNGLDELETETEXTURESPROC                              ) gl3w_fn("glDeleteTextures");
	gl3wDeleteTransformFeedbacks                    = ( PFNGLDELETETRANSFORMFEEDBACKSPROC                    ) gl3w_fn("glDeleteTransformFeedbacks");
	gl3wDeleteVertexArrays                          = ( PFNGLDELETEVERTEXARRAYSPROC                          ) gl3w_fn("glDeleteVertexArrays");
	gl3wDepthFunc                                   = ( PFNGLDEPTHFUNCPROC                                   ) gl3w_fn("glDepthFunc");
	gl3wDepthMask                                   = ( PFNGLDEPTHMASKPROC                                   ) gl3w_fn("glDepthMask");
	gl3wDepthRange                                  = ( PFNGLDEPTHRANGEPROC                                  ) gl3w_fn("glDepthRange");
	gl3wDepthRangeArrayv                            = ( PFNGLDEPTHRANGEARRAYVPROC                            ) gl3w_fn("glDepthRangeArrayv");
	gl3wDepthRangeIndexed                           = ( PFNGLDEPTHRANGEINDEXEDPROC                           ) gl3w_fn("glDepthRangeIndexed");
	gl3wDepthRangef                                 = ( PFNGLDEPTHRANGEFPROC                                 ) gl3w_fn("glDepthRangef");
	gl3wDetachShader                                = ( PFNGLDETACHSHADERPROC                                ) gl3w_fn("glDetachShader");
	gl3wDisable                                     = ( PFNGLDISABLEPROC                                     ) gl3w_fn("glDisable");
	gl3wDisableVertexArrayAttrib                    = ( PFNGLDISABLEVERTEXARRAYATTRIBPROC                    ) gl3w_fn("glDisableVertexArrayAttrib");
	gl3wDisableVertexAttribArray                    = ( PFNGLDISABLEVERTEXATTRIBARRAYPROC                    ) gl3w_fn("glDisableVertexAttribArray");
	gl3wDisablei                                    = ( PFNGLDISABLEIPROC                                    ) gl3w_fn("glDisablei");
	gl3wDispatchCompute                             = ( PFNGLDISPATCHCOMPUTEPROC                             ) gl3w_fn("glDispatchCompute");
	gl3wDispatchComputeGroupSizeARB                 = ( PFNGLDISPATCHCOMPUTEGROUPSIZEARBPROC                 ) gl3w_fn("glDispatchComputeGroupSizeARB");
	gl3wDispatchComputeIndirect                     = ( PFNGLDISPATCHCOMPUTEINDIRECTPROC                     ) gl3w_fn("glDispatchComputeIndirect");
	gl3wDrawArrays                                  = ( PFNGLDRAWARRAYSPROC                                  ) gl3w_fn("glDrawArrays");
	gl3wDrawArraysIndirect                          = ( PFNGLDRAWARRAYSINDIRECTPROC                          ) gl3w_fn("glDrawArraysIndirect");
	gl3wDrawArraysInstanced                         = ( PFNGLDRAWARRAYSINSTANCEDPROC                         ) gl3w_fn("glDrawArraysInstanced");
	gl3wDrawArraysInstancedBaseInstance             = ( PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC             ) gl3w_fn("glDrawArraysInstancedBaseInstance");
	gl3wDrawBuffer                                  = ( PFNGLDRAWBUFFERPROC                                  ) gl3w_fn("glDrawBuffer");
	gl3wDrawBuffers                                 = ( PFNGLDRAWBUFFERSPROC                                 ) gl3w_fn("glDrawBuffers");
	gl3wDrawElements                                = ( PFNGLDRAWELEMENTSPROC                                ) gl3w_fn("glDrawElements");
	gl3wDrawElementsBaseVertex                      = ( PFNGLDRAWELEMENTSBASEVERTEXPROC                      ) gl3w_fn("glDrawElementsBaseVertex");
	gl3wDrawElementsIndirect                        = ( PFNGLDRAWELEMENTSINDIRECTPROC                        ) gl3w_fn("glDrawElementsIndirect");
	gl3wDrawElementsInstanced                       = ( PFNGLDRAWELEMENTSINSTANCEDPROC                       ) gl3w_fn("glDrawElementsInstanced");
	gl3wDrawElementsInstancedBaseInstance           = ( PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC           ) gl3w_fn("glDrawElementsInstancedBaseInstance");
	gl3wDrawElementsInstancedBaseVertex             = ( PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC             ) gl3w_fn("glDrawElementsInstancedBaseVertex");
	gl3wDrawElementsInstancedBaseVertexBaseInstance = ( PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC ) gl3w_fn("glDrawElementsInstancedBaseVertexBaseInstance");
	gl3wDrawRangeElements                           = ( PFNGLDRAWRANGEELEMENTSPROC                           ) gl3w_fn("glDrawRangeElements");
	gl3wDrawRangeElementsBaseVertex                 = ( PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC                 ) gl3w_fn("glDrawRangeElementsBaseVertex");
	gl3wDrawTransformFeedback                       = ( PFNGLDRAWTRANSFORMFEEDBACKPROC                       ) gl3w_fn("glDrawTransformFeedback");
	gl3wDrawTransformFeedbackInstanced              = ( PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC              ) gl3w_fn("glDrawTransformFeedbackInstanced");
	gl3wDrawTransformFeedbackStream                 = ( PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC                 ) gl3w_fn("glDrawTransformFeedbackStream");
	gl3wDrawTransformFeedbackStreamInstanced        = ( PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC        ) gl3w_fn("glDrawTransformFeedbackStreamInstanced");
	gl3wEnable                                      = ( PFNGLENABLEPROC                                      ) gl3w_fn("glEnable");
	gl3wEnableVertexArrayAttrib                     = ( PFNGLENABLEVERTEXARRAYATTRIBPROC                     ) gl3w_fn("glEnableVertexArrayAttrib");
	gl3wEnableVertexAttribArray                     = ( PFNGLENABLEVERTEXATTRIBARRAYPROC                     ) gl3w_fn("glEnableVertexAttribArray");
	gl3wEnablei                                     = ( PFNGLENABLEIPROC                                     ) gl3w_fn("glEnablei");
	gl3wEndConditionalRender                        = ( PFNGLENDCONDITIONALRENDERPROC                        ) gl3w_fn("glEndConditionalRender");
	gl3wEndQuery                                    = ( PFNGLENDQUERYPROC                                    ) gl3w_fn("glEndQuery");
	gl3wEndQueryIndexed                             = ( PFNGLENDQUERYINDEXEDPROC                             ) gl3w_fn("glEndQueryIndexed");
	gl3wEndTransformFeedback                        = ( PFNGLENDTRANSFORMFEEDBACKPROC                        ) gl3w_fn("glEndTransformFeedback");
	gl3wFenceSync                                   = ( PFNGLFENCESYNCPROC                                   ) gl3w_fn("glFenceSync");
	gl3wFinish                                      = ( PFNGLFINISHPROC                                      ) gl3w_fn("glFinish");
	gl3wFlush                                       = ( PFNGLFLUSHPROC                                       ) gl3w_fn("glFlush");
	gl3wFlushMappedBufferRange                      = ( PFNGLFLUSHMAPPEDBUFFERRANGEPROC                      ) gl3w_fn("glFlushMappedBufferRange");
	gl3wFlushMappedNamedBufferRange                 = ( PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC                 ) gl3w_fn("glFlushMappedNamedBufferRange");
	gl3wFramebufferParameteri                       = ( PFNGLFRAMEBUFFERPARAMETERIPROC                       ) gl3w_fn("glFramebufferParameteri");
	gl3wFramebufferRenderbuffer                     = ( PFNGLFRAMEBUFFERRENDERBUFFERPROC                     ) gl3w_fn("glFramebufferRenderbuffer");
	gl3wFramebufferTexture                          = ( PFNGLFRAMEBUFFERTEXTUREPROC                          ) gl3w_fn("glFramebufferTexture");
	gl3wFramebufferTexture1D                        = ( PFNGLFRAMEBUFFERTEXTURE1DPROC                        ) gl3w_fn("glFramebufferTexture1D");
	gl3wFramebufferTexture2D                        = ( PFNGLFRAMEBUFFERTEXTURE2DPROC                        ) gl3w_fn("glFramebufferTexture2D");
	gl3wFramebufferTexture3D                        = ( PFNGLFRAMEBUFFERTEXTURE3DPROC                        ) gl3w_fn("glFramebufferTexture3D");
	gl3wFramebufferTextureLayer                     = ( PFNGLFRAMEBUFFERTEXTURELAYERPROC                     ) gl3w_fn("glFramebufferTextureLayer");
	gl3wFrontFace                                   = ( PFNGLFRONTFACEPROC                                   ) gl3w_fn("glFrontFace");
	gl3wGenBuffers                                  = ( PFNGLGENBUFFERSPROC                                  ) gl3w_fn("glGenBuffers");
	gl3wGenFramebuffers                             = ( PFNGLGENFRAMEBUFFERSPROC                             ) gl3w_fn("glGenFramebuffers");
	gl3wGenProgramPipelines                         = ( PFNGLGENPROGRAMPIPELINESPROC                         ) gl3w_fn("glGenProgramPipelines");
	gl3wGenQueries                                  = ( PFNGLGENQUERIESPROC                                  ) gl3w_fn("glGenQueries");
	gl3wGenRenderbuffers                            = ( PFNGLGENRENDERBUFFERSPROC                            ) gl3w_fn("glGenRenderbuffers");
	gl3wGenSamplers                                 = ( PFNGLGENSAMPLERSPROC                                 ) gl3w_fn("glGenSamplers");
	gl3wGenTextures                                 = ( PFNGLGENTEXTURESPROC                                 ) gl3w_fn("glGenTextures");
	gl3wGenTransformFeedbacks                       = ( PFNGLGENTRANSFORMFEEDBACKSPROC                       ) gl3w_fn("glGenTransformFeedbacks");
	gl3wGenVertexArrays                             = ( PFNGLGENVERTEXARRAYSPROC                             ) gl3w_fn("glGenVertexArrays");
	gl3wGenerateMipmap                              = ( PFNGLGENERATEMIPMAPPROC                              ) gl3w_fn("glGenerateMipmap");
	gl3wGenerateTextureMipmap                       = ( PFNGLGENERATETEXTUREMIPMAPPROC                       ) gl3w_fn("glGenerateTextureMipmap");
	gl3wGetActiveAtomicCounterBufferiv              = ( PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC              ) gl3w_fn("glGetActiveAtomicCounterBufferiv");
	gl3wGetActiveAttrib                             = ( PFNGLGETACTIVEATTRIBPROC                             ) gl3w_fn("glGetActiveAttrib");
	gl3wGetActiveSubroutineName                     = ( PFNGLGETACTIVESUBROUTINENAMEPROC                     ) gl3w_fn("glGetActiveSubroutineName");
	gl3wGetActiveSubroutineUniformName              = ( PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC              ) gl3w_fn("glGetActiveSubroutineUniformName");
	gl3wGetActiveSubroutineUniformiv                = ( PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC                ) gl3w_fn("glGetActiveSubroutineUniformiv");
	gl3wGetActiveUniform                            = ( PFNGLGETACTIVEUNIFORMPROC                            ) gl3w_fn("glGetActiveUniform");
	gl3wGetActiveUniformBlockName                   = ( PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC                   ) gl3w_fn("glGetActiveUniformBlockName");
	gl3wGetActiveUniformBlockiv                     = ( PFNGLGETACTIVEUNIFORMBLOCKIVPROC                     ) gl3w_fn("glGetActiveUniformBlockiv");
	gl3wGetActiveUniformName                        = ( PFNGLGETACTIVEUNIFORMNAMEPROC                        ) gl3w_fn("glGetActiveUniformName");
	gl3wGetActiveUniformsiv                         = ( PFNGLGETACTIVEUNIFORMSIVPROC                         ) gl3w_fn("glGetActiveUniformsiv");
	gl3wGetAttachedShaders                          = ( PFNGLGETATTACHEDSHADERSPROC                          ) gl3w_fn("glGetAttachedShaders");
	gl3wGetAttribLocation                           = ( PFNGLGETATTRIBLOCATIONPROC                           ) gl3w_fn("glGetAttribLocation");
	gl3wGetBooleani_v                               = ( PFNGLGETBOOLEANI_VPROC                               ) gl3w_fn("glGetBooleani_v");
	gl3wGetBooleanv                                 = ( PFNGLGETBOOLEANVPROC                                 ) gl3w_fn("glGetBooleanv");
	gl3wGetBufferParameteri64v                      = ( PFNGLGETBUFFERPARAMETERI64VPROC                      ) gl3w_fn("glGetBufferParameteri64v");
	gl3wGetBufferParameteriv                        = ( PFNGLGETBUFFERPARAMETERIVPROC                        ) gl3w_fn("glGetBufferParameteriv");
	gl3wGetBufferPointerv                           = ( PFNGLGETBUFFERPOINTERVPROC                           ) gl3w_fn("glGetBufferPointerv");
	gl3wGetBufferSubData                            = ( PFNGLGETBUFFERSUBDATAPROC                            ) gl3w_fn("glGetBufferSubData");
	gl3wGetCompressedTexImage                       = ( PFNGLGETCOMPRESSEDTEXIMAGEPROC                       ) gl3w_fn("glGetCompressedTexImage");
	gl3wGetCompressedTextureImage                   = ( PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC                   ) gl3w_fn("glGetCompressedTextureImage");
	gl3wGetCompressedTextureSubImage                = ( PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC                ) gl3w_fn("glGetCompressedTextureSubImage");
	gl3wGetDebugMessageLog                          = ( PFNGLGETDEBUGMESSAGELOGPROC                          ) gl3w_fn("glGetDebugMessageLog");
	gl3wGetDebugMessageLogARB                       = ( PFNGLGETDEBUGMESSAGELOGARBPROC                       ) gl3w_fn("glGetDebugMessageLogARB");
	gl3wGetDoublei_v                                = ( PFNGLGETDOUBLEI_VPROC                                ) gl3w_fn("glGetDoublei_v");
	gl3wGetDoublev                                  = ( PFNGLGETDOUBLEVPROC                                  ) gl3w_fn("glGetDoublev");
	gl3wGetError                                    = ( PFNGLGETERRORPROC                                    ) gl3w_fn("glGetError");
	gl3wGetFloati_v                                 = ( PFNGLGETFLOATI_VPROC                                 ) gl3w_fn("glGetFloati_v");
	gl3wGetFloatv                                   = ( PFNGLGETFLOATVPROC                                   ) gl3w_fn("glGetFloatv");
	gl3wGetFragDataIndex                            = ( PFNGLGETFRAGDATAINDEXPROC                            ) gl3w_fn("glGetFragDataIndex");
	gl3wGetFragDataLocation                         = ( PFNGLGETFRAGDATALOCATIONPROC                         ) gl3w_fn("glGetFragDataLocation");
	gl3wGetFramebufferAttachmentParameteriv         = ( PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC         ) gl3w_fn("glGetFramebufferAttachmentParameteriv");
	gl3wGetFramebufferParameteriv                   = ( PFNGLGETFRAMEBUFFERPARAMETERIVPROC                   ) gl3w_fn("glGetFramebufferParameteriv");
	gl3wGetGraphicsResetStatus                      = ( PFNGLGETGRAPHICSRESETSTATUSPROC                      ) gl3w_fn("glGetGraphicsResetStatus");
	gl3wGetGraphicsResetStatusARB                   = ( PFNGLGETGRAPHICSRESETSTATUSARBPROC                   ) gl3w_fn("glGetGraphicsResetStatusARB");
	gl3wGetImageHandleARB                           = ( PFNGLGETIMAGEHANDLEARBPROC                           ) gl3w_fn("glGetImageHandleARB");
	gl3wGetInteger64i_v                             = ( PFNGLGETINTEGER64I_VPROC                             ) gl3w_fn("glGetInteger64i_v");
	gl3wGetInteger64v                               = ( PFNGLGETINTEGER64VPROC                               ) gl3w_fn("glGetInteger64v");
	gl3wGetIntegeri_v                               = ( PFNGLGETINTEGERI_VPROC                               ) gl3w_fn("glGetIntegeri_v");
	gl3wGetIntegerv                                 = ( PFNGLGETINTEGERVPROC                                 ) gl3w_fn("glGetIntegerv");
	gl3wGetInternalformati64v                       = ( PFNGLGETINTERNALFORMATI64VPROC                       ) gl3w_fn("glGetInternalformati64v");
	gl3wGetInternalformativ                         = ( PFNGLGETINTERNALFORMATIVPROC                         ) gl3w_fn("glGetInternalformativ");
	gl3wGetMultisamplefv                            = ( PFNGLGETMULTISAMPLEFVPROC                            ) gl3w_fn("glGetMultisamplefv");
	gl3wGetNamedBufferParameteri64v                 = ( PFNGLGETNAMEDBUFFERPARAMETERI64VPROC                 ) gl3w_fn("glGetNamedBufferParameteri64v");
	gl3wGetNamedBufferParameteriv                   = ( PFNGLGETNAMEDBUFFERPARAMETERIVPROC                   ) gl3w_fn("glGetNamedBufferParameteriv");
	gl3wGetNamedBufferPointerv                      = ( PFNGLGETNAMEDBUFFERPOINTERVPROC                      ) gl3w_fn("glGetNamedBufferPointerv");
	gl3wGetNamedBufferSubData                       = ( PFNGLGETNAMEDBUFFERSUBDATAPROC                       ) gl3w_fn("glGetNamedBufferSubData");
	gl3wGetNamedFramebufferAttachmentParameteriv    = ( PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC    ) gl3w_fn("glGetNamedFramebufferAttachmentParameteriv");
	gl3wGetNamedFramebufferParameteriv              = ( PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC              ) gl3w_fn("glGetNamedFramebufferParameteriv");
	gl3wGetNamedRenderbufferParameteriv             = ( PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC             ) gl3w_fn("glGetNamedRenderbufferParameteriv");
	gl3wGetNamedStringARB                           = ( PFNGLGETNAMEDSTRINGARBPROC                           ) gl3w_fn("glGetNamedStringARB");
	gl3wGetNamedStringivARB                         = ( PFNGLGETNAMEDSTRINGIVARBPROC                         ) gl3w_fn("glGetNamedStringivARB");
	gl3wGetObjectLabel                              = ( PFNGLGETOBJECTLABELPROC                              ) gl3w_fn("glGetObjectLabel");
	gl3wGetObjectPtrLabel                           = ( PFNGLGETOBJECTPTRLABELPROC                           ) gl3w_fn("glGetObjectPtrLabel");
	gl3wGetPointerv                                 = ( PFNGLGETPOINTERVPROC                                 ) gl3w_fn("glGetPointerv");
	gl3wGetProgramBinary                            = ( PFNGLGETPROGRAMBINARYPROC                            ) gl3w_fn("glGetProgramBinary");
	gl3wGetProgramInfoLog                           = ( PFNGLGETPROGRAMINFOLOGPROC                           ) gl3w_fn("glGetProgramInfoLog");
	gl3wGetProgramInterfaceiv                       = ( PFNGLGETPROGRAMINTERFACEIVPROC                       ) gl3w_fn("glGetProgramInterfaceiv");
	gl3wGetProgramPipelineInfoLog                   = ( PFNGLGETPROGRAMPIPELINEINFOLOGPROC                   ) gl3w_fn("glGetProgramPipelineInfoLog");
	gl3wGetProgramPipelineiv                        = ( PFNGLGETPROGRAMPIPELINEIVPROC                        ) gl3w_fn("glGetProgramPipelineiv");
	gl3wGetProgramResourceIndex                     = ( PFNGLGETPROGRAMRESOURCEINDEXPROC                     ) gl3w_fn("glGetProgramResourceIndex");
	gl3wGetProgramResourceLocation                  = ( PFNGLGETPROGRAMRESOURCELOCATIONPROC                  ) gl3w_fn("glGetProgramResourceLocation");
	gl3wGetProgramResourceLocationIndex             = ( PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC             ) gl3w_fn("glGetProgramResourceLocationIndex");
	gl3wGetProgramResourceName                      = ( PFNGLGETPROGRAMRESOURCENAMEPROC                      ) gl3w_fn("glGetProgramResourceName");
	gl3wGetProgramResourceiv                        = ( PFNGLGETPROGRAMRESOURCEIVPROC                        ) gl3w_fn("glGetProgramResourceiv");
	gl3wGetProgramStageiv                           = ( PFNGLGETPROGRAMSTAGEIVPROC                           ) gl3w_fn("glGetProgramStageiv");
	gl3wGetProgramiv                                = ( PFNGLGETPROGRAMIVPROC                                ) gl3w_fn("glGetProgramiv");
	gl3wGetQueryBufferObjecti64v                    = ( PFNGLGETQUERYBUFFEROBJECTI64VPROC                    ) gl3w_fn("glGetQueryBufferObjecti64v");
	gl3wGetQueryBufferObjectiv                      = ( PFNGLGETQUERYBUFFEROBJECTIVPROC                      ) gl3w_fn("glGetQueryBufferObjectiv");
	gl3wGetQueryBufferObjectui64v                   = ( PFNGLGETQUERYBUFFEROBJECTUI64VPROC                   ) gl3w_fn("glGetQueryBufferObjectui64v");
	gl3wGetQueryBufferObjectuiv                     = ( PFNGLGETQUERYBUFFEROBJECTUIVPROC                     ) gl3w_fn("glGetQueryBufferObjectuiv");
	gl3wGetQueryIndexediv                           = ( PFNGLGETQUERYINDEXEDIVPROC                           ) gl3w_fn("glGetQueryIndexediv");
	gl3wGetQueryObjecti64v                          = ( PFNGLGETQUERYOBJECTI64VPROC                          ) gl3w_fn("glGetQueryObjecti64v");
	gl3wGetQueryObjectiv                            = ( PFNGLGETQUERYOBJECTIVPROC                            ) gl3w_fn("glGetQueryObjectiv");
	gl3wGetQueryObjectui64v                         = ( PFNGLGETQUERYOBJECTUI64VPROC                         ) gl3w_fn("glGetQueryObjectui64v");
	gl3wGetQueryObjectuiv                           = ( PFNGLGETQUERYOBJECTUIVPROC                           ) gl3w_fn("glGetQueryObjectuiv");
	gl3wGetQueryiv                                  = ( PFNGLGETQUERYIVPROC                                  ) gl3w_fn("glGetQueryiv");
	gl3wGetRenderbufferParameteriv                  = ( PFNGLGETRENDERBUFFERPARAMETERIVPROC                  ) gl3w_fn("glGetRenderbufferParameteriv");
	gl3wGetSamplerParameterIiv                      = ( PFNGLGETSAMPLERPARAMETERIIVPROC                      ) gl3w_fn("glGetSamplerParameterIiv");
	gl3wGetSamplerParameterIuiv                     = ( PFNGLGETSAMPLERPARAMETERIUIVPROC                     ) gl3w_fn("glGetSamplerParameterIuiv");
	gl3wGetSamplerParameterfv                       = ( PFNGLGETSAMPLERPARAMETERFVPROC                       ) gl3w_fn("glGetSamplerParameterfv");
	gl3wGetSamplerParameteriv                       = ( PFNGLGETSAMPLERPARAMETERIVPROC                       ) gl3w_fn("glGetSamplerParameteriv");
	gl3wGetShaderInfoLog                            = ( PFNGLGETSHADERINFOLOGPROC                            ) gl3w_fn("glGetShaderInfoLog");
	gl3wGetShaderPrecisionFormat                    = ( PFNGLGETSHADERPRECISIONFORMATPROC                    ) gl3w_fn("glGetShaderPrecisionFormat");
	gl3wGetShaderSource                             = ( PFNGLGETSHADERSOURCEPROC                             ) gl3w_fn("glGetShaderSource");
	gl3wGetShaderiv                                 = ( PFNGLGETSHADERIVPROC                                 ) gl3w_fn("glGetShaderiv");
	gl3wGetString                                   = ( PFNGLGETSTRINGPROC                                   ) gl3w_fn("glGetString");
	gl3wGetStringi                                  = ( PFNGLGETSTRINGIPROC                                  ) gl3w_fn("glGetStringi");
	gl3wGetSubroutineIndex                          = ( PFNGLGETSUBROUTINEINDEXPROC                          ) gl3w_fn("glGetSubroutineIndex");
	gl3wGetSubroutineUniformLocation                = ( PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC                ) gl3w_fn("glGetSubroutineUniformLocation");
	gl3wGetSynciv                                   = ( PFNGLGETSYNCIVPROC                                   ) gl3w_fn("glGetSynciv");
	gl3wGetTexImage                                 = ( PFNGLGETTEXIMAGEPROC                                 ) gl3w_fn("glGetTexImage");
	gl3wGetTexLevelParameterfv                      = ( PFNGLGETTEXLEVELPARAMETERFVPROC                      ) gl3w_fn("glGetTexLevelParameterfv");
	gl3wGetTexLevelParameteriv                      = ( PFNGLGETTEXLEVELPARAMETERIVPROC                      ) gl3w_fn("glGetTexLevelParameteriv");
	gl3wGetTexParameterIiv                          = ( PFNGLGETTEXPARAMETERIIVPROC                          ) gl3w_fn("glGetTexParameterIiv");
	gl3wGetTexParameterIuiv                         = ( PFNGLGETTEXPARAMETERIUIVPROC                         ) gl3w_fn("glGetTexParameterIuiv");
	gl3wGetTexParameterfv                           = ( PFNGLGETTEXPARAMETERFVPROC                           ) gl3w_fn("glGetTexParameterfv");
	gl3wGetTexParameteriv                           = ( PFNGLGETTEXPARAMETERIVPROC                           ) gl3w_fn("glGetTexParameteriv");
	gl3wGetTextureHandleARB                         = ( PFNGLGETTEXTUREHANDLEARBPROC                         ) gl3w_fn("glGetTextureHandleARB");
	gl3wGetTextureImage                             = ( PFNGLGETTEXTUREIMAGEPROC                             ) gl3w_fn("glGetTextureImage");
	gl3wGetTextureLevelParameterfv                  = ( PFNGLGETTEXTURELEVELPARAMETERFVPROC                  ) gl3w_fn("glGetTextureLevelParameterfv");
	gl3wGetTextureLevelParameteriv                  = ( PFNGLGETTEXTURELEVELPARAMETERIVPROC                  ) gl3w_fn("glGetTextureLevelParameteriv");
	gl3wGetTextureParameterIiv                      = ( PFNGLGETTEXTUREPARAMETERIIVPROC                      ) gl3w_fn("glGetTextureParameterIiv");
	gl3wGetTextureParameterIuiv                     = ( PFNGLGETTEXTUREPARAMETERIUIVPROC                     ) gl3w_fn("glGetTextureParameterIuiv");
	gl3wGetTextureParameterfv                       = ( PFNGLGETTEXTUREPARAMETERFVPROC                       ) gl3w_fn("glGetTextureParameterfv");
	gl3wGetTextureParameteriv                       = ( PFNGLGETTEXTUREPARAMETERIVPROC                       ) gl3w_fn("glGetTextureParameteriv");
	gl3wGetTextureSamplerHandleARB                  = ( PFNGLGETTEXTURESAMPLERHANDLEARBPROC                  ) gl3w_fn("glGetTextureSamplerHandleARB");
	gl3wGetTextureSubImage                          = ( PFNGLGETTEXTURESUBIMAGEPROC                          ) gl3w_fn("glGetTextureSubImage");
	gl3wGetTransformFeedbackVarying                 = ( PFNGLGETTRANSFORMFEEDBACKVARYINGPROC                 ) gl3w_fn("glGetTransformFeedbackVarying");
	gl3wGetTransformFeedbacki64_v                   = ( PFNGLGETTRANSFORMFEEDBACKI64_VPROC                   ) gl3w_fn("glGetTransformFeedbacki64_v");
	gl3wGetTransformFeedbacki_v                     = ( PFNGLGETTRANSFORMFEEDBACKI_VPROC                     ) gl3w_fn("glGetTransformFeedbacki_v");
	gl3wGetTransformFeedbackiv                      = ( PFNGLGETTRANSFORMFEEDBACKIVPROC                      ) gl3w_fn("glGetTransformFeedbackiv");
	gl3wGetUniformBlockIndex                        = ( PFNGLGETUNIFORMBLOCKINDEXPROC                        ) gl3w_fn("glGetUniformBlockIndex");
	gl3wGetUniformIndices                           = ( PFNGLGETUNIFORMINDICESPROC                           ) gl3w_fn("glGetUniformIndices");
	gl3wGetUniformLocation                          = ( PFNGLGETUNIFORMLOCATIONPROC                          ) gl3w_fn("glGetUniformLocation");
	gl3wGetUniformSubroutineuiv                     = ( PFNGLGETUNIFORMSUBROUTINEUIVPROC                     ) gl3w_fn("glGetUniformSubroutineuiv");
	gl3wGetUniformdv                                = ( PFNGLGETUNIFORMDVPROC                                ) gl3w_fn("glGetUniformdv");
	gl3wGetUniformfv                                = ( PFNGLGETUNIFORMFVPROC                                ) gl3w_fn("glGetUniformfv");
	gl3wGetUniformiv                                = ( PFNGLGETUNIFORMIVPROC                                ) gl3w_fn("glGetUniformiv");
	gl3wGetUniformuiv                               = ( PFNGLGETUNIFORMUIVPROC                               ) gl3w_fn("glGetUniformuiv");
	gl3wGetVertexArrayIndexed64iv                   = ( PFNGLGETVERTEXARRAYINDEXED64IVPROC                   ) gl3w_fn("glGetVertexArrayIndexed64iv");
	gl3wGetVertexArrayIndexediv                     = ( PFNGLGETVERTEXARRAYINDEXEDIVPROC                     ) gl3w_fn("glGetVertexArrayIndexediv");
	gl3wGetVertexArrayiv                            = ( PFNGLGETVERTEXARRAYIVPROC                            ) gl3w_fn("glGetVertexArrayiv");
	gl3wGetVertexAttribIiv                          = ( PFNGLGETVERTEXATTRIBIIVPROC                          ) gl3w_fn("glGetVertexAttribIiv");
	gl3wGetVertexAttribIuiv                         = ( PFNGLGETVERTEXATTRIBIUIVPROC                         ) gl3w_fn("glGetVertexAttribIuiv");
	gl3wGetVertexAttribLdv                          = ( PFNGLGETVERTEXATTRIBLDVPROC                          ) gl3w_fn("glGetVertexAttribLdv");
	gl3wGetVertexAttribLui64vARB                    = ( PFNGLGETVERTEXATTRIBLUI64VARBPROC                    ) gl3w_fn("glGetVertexAttribLui64vARB");
	gl3wGetVertexAttribPointerv                     = ( PFNGLGETVERTEXATTRIBPOINTERVPROC                     ) gl3w_fn("glGetVertexAttribPointerv");
	gl3wGetVertexAttribdv                           = ( PFNGLGETVERTEXATTRIBDVPROC                           ) gl3w_fn("glGetVertexAttribdv");
	gl3wGetVertexAttribfv                           = ( PFNGLGETVERTEXATTRIBFVPROC                           ) gl3w_fn("glGetVertexAttribfv");
	gl3wGetVertexAttribiv                           = ( PFNGLGETVERTEXATTRIBIVPROC                           ) gl3w_fn("glGetVertexAttribiv");
	gl3wGetnCompressedTexImage                      = ( PFNGLGETNCOMPRESSEDTEXIMAGEPROC                      ) gl3w_fn("glGetnCompressedTexImage");
	gl3wGetnCompressedTexImageARB                   = ( PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC                   ) gl3w_fn("glGetnCompressedTexImageARB");
	gl3wGetnTexImage                                = ( PFNGLGETNTEXIMAGEPROC                                ) gl3w_fn("glGetnTexImage");
	gl3wGetnTexImageARB                             = ( PFNGLGETNTEXIMAGEARBPROC                             ) gl3w_fn("glGetnTexImageARB");
	gl3wGetnUniformdv                               = ( PFNGLGETNUNIFORMDVPROC                               ) gl3w_fn("glGetnUniformdv");
	gl3wGetnUniformdvARB                            = ( PFNGLGETNUNIFORMDVARBPROC                            ) gl3w_fn("glGetnUniformdvARB");
	gl3wGetnUniformfv                               = ( PFNGLGETNUNIFORMFVPROC                               ) gl3w_fn("glGetnUniformfv");
	gl3wGetnUniformfvARB                            = ( PFNGLGETNUNIFORMFVARBPROC                            ) gl3w_fn("glGetnUniformfvARB");
	gl3wGetnUniformiv                               = ( PFNGLGETNUNIFORMIVPROC                               ) gl3w_fn("glGetnUniformiv");
	gl3wGetnUniformivARB                            = ( PFNGLGETNUNIFORMIVARBPROC                            ) gl3w_fn("glGetnUniformivARB");
	gl3wGetnUniformuiv                              = ( PFNGLGETNUNIFORMUIVPROC                              ) gl3w_fn("glGetnUniformuiv");
	gl3wGetnUniformuivARB                           = ( PFNGLGETNUNIFORMUIVARBPROC                           ) gl3w_fn("glGetnUniformuivARB");
	gl3wHint                                        = ( PFNGLHINTPROC                                        ) gl3w_fn("glHint");
	gl3wInvalidateBufferData                        = ( PFNGLINVALIDATEBUFFERDATAPROC                        ) gl3w_fn("glInvalidateBufferData");
	gl3wInvalidateBufferSubData                     = ( PFNGLINVALIDATEBUFFERSUBDATAPROC                     ) gl3w_fn("glInvalidateBufferSubData");
	gl3wInvalidateFramebuffer                       = ( PFNGLINVALIDATEFRAMEBUFFERPROC                       ) gl3w_fn("glInvalidateFramebuffer");
	gl3wInvalidateNamedFramebufferData              = ( PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC              ) gl3w_fn("glInvalidateNamedFramebufferData");
	gl3wInvalidateNamedFramebufferSubData           = ( PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC           ) gl3w_fn("glInvalidateNamedFramebufferSubData");
	gl3wInvalidateSubFramebuffer                    = ( PFNGLINVALIDATESUBFRAMEBUFFERPROC                    ) gl3w_fn("glInvalidateSubFramebuffer");
	gl3wInvalidateTexImage                          = ( PFNGLINVALIDATETEXIMAGEPROC                          ) gl3w_fn("glInvalidateTexImage");
	gl3wInvalidateTexSubImage                       = ( PFNGLINVALIDATETEXSUBIMAGEPROC                       ) gl3w_fn("glInvalidateTexSubImage");
	gl3wIsBuffer                                    = ( PFNGLISBUFFERPROC                                    ) gl3w_fn("glIsBuffer");
	gl3wIsEnabled                                   = ( PFNGLISENABLEDPROC                                   ) gl3w_fn("glIsEnabled");
	gl3wIsEnabledi                                  = ( PFNGLISENABLEDIPROC                                  ) gl3w_fn("glIsEnabledi");
	gl3wIsFramebuffer                               = ( PFNGLISFRAMEBUFFERPROC                               ) gl3w_fn("glIsFramebuffer");
	gl3wIsImageHandleResidentARB                    = ( PFNGLISIMAGEHANDLERESIDENTARBPROC                    ) gl3w_fn("glIsImageHandleResidentARB");
	gl3wIsNamedStringARB                            = ( PFNGLISNAMEDSTRINGARBPROC                            ) gl3w_fn("glIsNamedStringARB");
	gl3wIsProgram                                   = ( PFNGLISPROGRAMPROC                                   ) gl3w_fn("glIsProgram");
	gl3wIsProgramPipeline                           = ( PFNGLISPROGRAMPIPELINEPROC                           ) gl3w_fn("glIsProgramPipeline");
	gl3wIsQuery                                     = ( PFNGLISQUERYPROC                                     ) gl3w_fn("glIsQuery");
	gl3wIsRenderbuffer                              = ( PFNGLISRENDERBUFFERPROC                              ) gl3w_fn("glIsRenderbuffer");
	gl3wIsSampler                                   = ( PFNGLISSAMPLERPROC                                   ) gl3w_fn("glIsSampler");
	gl3wIsShader                                    = ( PFNGLISSHADERPROC                                    ) gl3w_fn("glIsShader");
	gl3wIsSync                                      = ( PFNGLISSYNCPROC                                      ) gl3w_fn("glIsSync");
	gl3wIsTexture                                   = ( PFNGLISTEXTUREPROC                                   ) gl3w_fn("glIsTexture");
	gl3wIsTextureHandleResidentARB                  = ( PFNGLISTEXTUREHANDLERESIDENTARBPROC                  ) gl3w_fn("glIsTextureHandleResidentARB");
	gl3wIsTransformFeedback                         = ( PFNGLISTRANSFORMFEEDBACKPROC                         ) gl3w_fn("glIsTransformFeedback");
	gl3wIsVertexArray                               = ( PFNGLISVERTEXARRAYPROC                               ) gl3w_fn("glIsVertexArray");
	gl3wLineWidth                                   = ( PFNGLLINEWIDTHPROC                                   ) gl3w_fn("glLineWidth");
	gl3wLinkProgram                                 = ( PFNGLLINKPROGRAMPROC                                 ) gl3w_fn("glLinkProgram");
	gl3wLogicOp                                     = ( PFNGLLOGICOPPROC                                     ) gl3w_fn("glLogicOp");
	gl3wMakeImageHandleNonResidentARB               = ( PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC               ) gl3w_fn("glMakeImageHandleNonResidentARB");
	gl3wMakeImageHandleResidentARB                  = ( PFNGLMAKEIMAGEHANDLERESIDENTARBPROC                  ) gl3w_fn("glMakeImageHandleResidentARB");
	gl3wMakeTextureHandleNonResidentARB             = ( PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC             ) gl3w_fn("glMakeTextureHandleNonResidentARB");
	gl3wMakeTextureHandleResidentARB                = ( PFNGLMAKETEXTUREHANDLERESIDENTARBPROC                ) gl3w_fn("glMakeTextureHandleResidentARB");
	gl3wMapBuffer                                   = ( PFNGLMAPBUFFERPROC                                   ) gl3w_fn("glMapBuffer");
	gl3wMapBufferRange                              = ( PFNGLMAPBUFFERRANGEPROC                              ) gl3w_fn("glMapBufferRange");
	gl3wMapNamedBuffer                              = ( PFNGLMAPNAMEDBUFFERPROC                              ) gl3w_fn("glMapNamedBuffer");
	gl3wMapNamedBufferRange                         = ( PFNGLMAPNAMEDBUFFERRANGEPROC                         ) gl3w_fn("glMapNamedBufferRange");
	gl3wMemoryBarrier                               = ( PFNGLMEMORYBARRIERPROC                               ) gl3w_fn("glMemoryBarrier");
	gl3wMemoryBarrierByRegion                       = ( PFNGLMEMORYBARRIERBYREGIONPROC                       ) gl3w_fn("glMemoryBarrierByRegion");
	gl3wMinSampleShading                            = ( PFNGLMINSAMPLESHADINGPROC                            ) gl3w_fn("glMinSampleShading");
	gl3wMinSampleShadingARB                         = ( PFNGLMINSAMPLESHADINGARBPROC                         ) gl3w_fn("glMinSampleShadingARB");
	gl3wMultiDrawArrays                             = ( PFNGLMULTIDRAWARRAYSPROC                             ) gl3w_fn("glMultiDrawArrays");
	gl3wMultiDrawArraysIndirect                     = ( PFNGLMULTIDRAWARRAYSINDIRECTPROC                     ) gl3w_fn("glMultiDrawArraysIndirect");
	gl3wMultiDrawArraysIndirectCountARB             = ( PFNGLMULTIDRAWARRAYSINDIRECTCOUNTARBPROC             ) gl3w_fn("glMultiDrawArraysIndirectCountARB");
	gl3wMultiDrawElements                           = ( PFNGLMULTIDRAWELEMENTSPROC                           ) gl3w_fn("glMultiDrawElements");
	gl3wMultiDrawElementsBaseVertex                 = ( PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC                 ) gl3w_fn("glMultiDrawElementsBaseVertex");
	gl3wMultiDrawElementsIndirect                   = ( PFNGLMULTIDRAWELEMENTSINDIRECTPROC                   ) gl3w_fn("glMultiDrawElementsIndirect");
	gl3wMultiDrawElementsIndirectCountARB           = ( PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTARBPROC           ) gl3w_fn("glMultiDrawElementsIndirectCountARB");
	gl3wNamedBufferData                             = ( PFNGLNAMEDBUFFERDATAPROC                             ) gl3w_fn("glNamedBufferData");
	gl3wNamedBufferPageCommitmentARB                = ( PFNGLNAMEDBUFFERPAGECOMMITMENTARBPROC                ) gl3w_fn("glNamedBufferPageCommitmentARB");
	gl3wNamedBufferPageCommitmentEXT                = ( PFNGLNAMEDBUFFERPAGECOMMITMENTEXTPROC                ) gl3w_fn("glNamedBufferPageCommitmentEXT");
	gl3wNamedBufferStorage                          = ( PFNGLNAMEDBUFFERSTORAGEPROC                          ) gl3w_fn("glNamedBufferStorage");
	gl3wNamedBufferSubData                          = ( PFNGLNAMEDBUFFERSUBDATAPROC                          ) gl3w_fn("glNamedBufferSubData");
	gl3wNamedFramebufferDrawBuffer                  = ( PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC                  ) gl3w_fn("glNamedFramebufferDrawBuffer");
	gl3wNamedFramebufferDrawBuffers                 = ( PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC                 ) gl3w_fn("glNamedFramebufferDrawBuffers");
	gl3wNamedFramebufferParameteri                  = ( PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC                  ) gl3w_fn("glNamedFramebufferParameteri");
	gl3wNamedFramebufferReadBuffer                  = ( PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC                  ) gl3w_fn("glNamedFramebufferReadBuffer");
	gl3wNamedFramebufferRenderbuffer                = ( PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC                ) gl3w_fn("glNamedFramebufferRenderbuffer");
	gl3wNamedFramebufferTexture                     = ( PFNGLNAMEDFRAMEBUFFERTEXTUREPROC                     ) gl3w_fn("glNamedFramebufferTexture");
	gl3wNamedFramebufferTextureLayer                = ( PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC                ) gl3w_fn("glNamedFramebufferTextureLayer");
	gl3wNamedRenderbufferStorage                    = ( PFNGLNAMEDRENDERBUFFERSTORAGEPROC                    ) gl3w_fn("glNamedRenderbufferStorage");
	gl3wNamedRenderbufferStorageMultisample         = ( PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC         ) gl3w_fn("glNamedRenderbufferStorageMultisample");
	gl3wNamedStringARB                              = ( PFNGLNAMEDSTRINGARBPROC                              ) gl3w_fn("glNamedStringARB");
	gl3wObjectLabel                                 = ( PFNGLOBJECTLABELPROC                                 ) gl3w_fn("glObjectLabel");
	gl3wObjectPtrLabel                              = ( PFNGLOBJECTPTRLABELPROC                              ) gl3w_fn("glObjectPtrLabel");
	gl3wPatchParameterfv                            = ( PFNGLPATCHPARAMETERFVPROC                            ) gl3w_fn("glPatchParameterfv");
	gl3wPatchParameteri                             = ( PFNGLPATCHPARAMETERIPROC                             ) gl3w_fn("glPatchParameteri");
	gl3wPauseTransformFeedback                      = ( PFNGLPAUSETRANSFORMFEEDBACKPROC                      ) gl3w_fn("glPauseTransformFeedback");
	gl3wPixelStoref                                 = ( PFNGLPIXELSTOREFPROC                                 ) gl3w_fn("glPixelStoref");
	gl3wPixelStorei                                 = ( PFNGLPIXELSTOREIPROC                                 ) gl3w_fn("glPixelStorei");
	gl3wPointParameterf                             = ( PFNGLPOINTPARAMETERFPROC                             ) gl3w_fn("glPointParameterf");
	gl3wPointParameterfv                            = ( PFNGLPOINTPARAMETERFVPROC                            ) gl3w_fn("glPointParameterfv");
	gl3wPointParameteri                             = ( PFNGLPOINTPARAMETERIPROC                             ) gl3w_fn("glPointParameteri");
	gl3wPointParameteriv                            = ( PFNGLPOINTPARAMETERIVPROC                            ) gl3w_fn("glPointParameteriv");
	gl3wPointSize                                   = ( PFNGLPOINTSIZEPROC                                   ) gl3w_fn("glPointSize");
	gl3wPolygonMode                                 = ( PFNGLPOLYGONMODEPROC                                 ) gl3w_fn("glPolygonMode");
	gl3wPolygonOffset                               = ( PFNGLPOLYGONOFFSETPROC                               ) gl3w_fn("glPolygonOffset");
	gl3wPopDebugGroup                               = ( PFNGLPOPDEBUGGROUPPROC                               ) gl3w_fn("glPopDebugGroup");
	gl3wPrimitiveRestartIndex                       = ( PFNGLPRIMITIVERESTARTINDEXPROC                       ) gl3w_fn("glPrimitiveRestartIndex");
	gl3wProgramBinary                               = ( PFNGLPROGRAMBINARYPROC                               ) gl3w_fn("glProgramBinary");
	gl3wProgramParameteri                           = ( PFNGLPROGRAMPARAMETERIPROC                           ) gl3w_fn("glProgramParameteri");
	gl3wProgramUniform1d                            = ( PFNGLPROGRAMUNIFORM1DPROC                            ) gl3w_fn("glProgramUniform1d");
	gl3wProgramUniform1dv                           = ( PFNGLPROGRAMUNIFORM1DVPROC                           ) gl3w_fn("glProgramUniform1dv");
	gl3wProgramUniform1f                            = ( PFNGLPROGRAMUNIFORM1FPROC                            ) gl3w_fn("glProgramUniform1f");
	gl3wProgramUniform1fv                           = ( PFNGLPROGRAMUNIFORM1FVPROC                           ) gl3w_fn("glProgramUniform1fv");
	gl3wProgramUniform1i                            = ( PFNGLPROGRAMUNIFORM1IPROC                            ) gl3w_fn("glProgramUniform1i");
	gl3wProgramUniform1iv                           = ( PFNGLPROGRAMUNIFORM1IVPROC                           ) gl3w_fn("glProgramUniform1iv");
	gl3wProgramUniform1ui                           = ( PFNGLPROGRAMUNIFORM1UIPROC                           ) gl3w_fn("glProgramUniform1ui");
	gl3wProgramUniform1uiv                          = ( PFNGLPROGRAMUNIFORM1UIVPROC                          ) gl3w_fn("glProgramUniform1uiv");
	gl3wProgramUniform2d                            = ( PFNGLPROGRAMUNIFORM2DPROC                            ) gl3w_fn("glProgramUniform2d");
	gl3wProgramUniform2dv                           = ( PFNGLPROGRAMUNIFORM2DVPROC                           ) gl3w_fn("glProgramUniform2dv");
	gl3wProgramUniform2f                            = ( PFNGLPROGRAMUNIFORM2FPROC                            ) gl3w_fn("glProgramUniform2f");
	gl3wProgramUniform2fv                           = ( PFNGLPROGRAMUNIFORM2FVPROC                           ) gl3w_fn("glProgramUniform2fv");
	gl3wProgramUniform2i                            = ( PFNGLPROGRAMUNIFORM2IPROC                            ) gl3w_fn("glProgramUniform2i");
	gl3wProgramUniform2iv                           = ( PFNGLPROGRAMUNIFORM2IVPROC                           ) gl3w_fn("glProgramUniform2iv");
	gl3wProgramUniform2ui                           = ( PFNGLPROGRAMUNIFORM2UIPROC                           ) gl3w_fn("glProgramUniform2ui");
	gl3wProgramUniform2uiv                          = ( PFNGLPROGRAMUNIFORM2UIVPROC                          ) gl3w_fn("glProgramUniform2uiv");
	gl3wProgramUniform3d                            = ( PFNGLPROGRAMUNIFORM3DPROC                            ) gl3w_fn("glProgramUniform3d");
	gl3wProgramUniform3dv                           = ( PFNGLPROGRAMUNIFORM3DVPROC                           ) gl3w_fn("glProgramUniform3dv");
	gl3wProgramUniform3f                            = ( PFNGLPROGRAMUNIFORM3FPROC                            ) gl3w_fn("glProgramUniform3f");
	gl3wProgramUniform3fv                           = ( PFNGLPROGRAMUNIFORM3FVPROC                           ) gl3w_fn("glProgramUniform3fv");
	gl3wProgramUniform3i                            = ( PFNGLPROGRAMUNIFORM3IPROC                            ) gl3w_fn("glProgramUniform3i");
	gl3wProgramUniform3iv                           = ( PFNGLPROGRAMUNIFORM3IVPROC                           ) gl3w_fn("glProgramUniform3iv");
	gl3wProgramUniform3ui                           = ( PFNGLPROGRAMUNIFORM3UIPROC                           ) gl3w_fn("glProgramUniform3ui");
	gl3wProgramUniform3uiv                          = ( PFNGLPROGRAMUNIFORM3UIVPROC                          ) gl3w_fn("glProgramUniform3uiv");
	gl3wProgramUniform4d                            = ( PFNGLPROGRAMUNIFORM4DPROC                            ) gl3w_fn("glProgramUniform4d");
	gl3wProgramUniform4dv                           = ( PFNGLPROGRAMUNIFORM4DVPROC                           ) gl3w_fn("glProgramUniform4dv");
	gl3wProgramUniform4f                            = ( PFNGLPROGRAMUNIFORM4FPROC                            ) gl3w_fn("glProgramUniform4f");
	gl3wProgramUniform4fv                           = ( PFNGLPROGRAMUNIFORM4FVPROC                           ) gl3w_fn("glProgramUniform4fv");
	gl3wProgramUniform4i                            = ( PFNGLPROGRAMUNIFORM4IPROC                            ) gl3w_fn("glProgramUniform4i");
	gl3wProgramUniform4iv                           = ( PFNGLPROGRAMUNIFORM4IVPROC                           ) gl3w_fn("glProgramUniform4iv");
	gl3wProgramUniform4ui                           = ( PFNGLPROGRAMUNIFORM4UIPROC                           ) gl3w_fn("glProgramUniform4ui");
	gl3wProgramUniform4uiv                          = ( PFNGLPROGRAMUNIFORM4UIVPROC                          ) gl3w_fn("glProgramUniform4uiv");
	gl3wProgramUniformHandleui64ARB                 = ( PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC                 ) gl3w_fn("glProgramUniformHandleui64ARB");
	gl3wProgramUniformHandleui64vARB                = ( PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC                ) gl3w_fn("glProgramUniformHandleui64vARB");
	gl3wProgramUniformMatrix2dv                     = ( PFNGLPROGRAMUNIFORMMATRIX2DVPROC                     ) gl3w_fn("glProgramUniformMatrix2dv");
	gl3wProgramUniformMatrix2fv                     = ( PFNGLPROGRAMUNIFORMMATRIX2FVPROC                     ) gl3w_fn("glProgramUniformMatrix2fv");
	gl3wProgramUniformMatrix2x3dv                   = ( PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC                   ) gl3w_fn("glProgramUniformMatrix2x3dv");
	gl3wProgramUniformMatrix2x3fv                   = ( PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC                   ) gl3w_fn("glProgramUniformMatrix2x3fv");
	gl3wProgramUniformMatrix2x4dv                   = ( PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC                   ) gl3w_fn("glProgramUniformMatrix2x4dv");
	gl3wProgramUniformMatrix2x4fv                   = ( PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC                   ) gl3w_fn("glProgramUniformMatrix2x4fv");
	gl3wProgramUniformMatrix3dv                     = ( PFNGLPROGRAMUNIFORMMATRIX3DVPROC                     ) gl3w_fn("glProgramUniformMatrix3dv");
	gl3wProgramUniformMatrix3fv                     = ( PFNGLPROGRAMUNIFORMMATRIX3FVPROC                     ) gl3w_fn("glProgramUniformMatrix3fv");
	gl3wProgramUniformMatrix3x2dv                   = ( PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC                   ) gl3w_fn("glProgramUniformMatrix3x2dv");
	gl3wProgramUniformMatrix3x2fv                   = ( PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC                   ) gl3w_fn("glProgramUniformMatrix3x2fv");
	gl3wProgramUniformMatrix3x4dv                   = ( PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC                   ) gl3w_fn("glProgramUniformMatrix3x4dv");
	gl3wProgramUniformMatrix3x4fv                   = ( PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC                   ) gl3w_fn("glProgramUniformMatrix3x4fv");
	gl3wProgramUniformMatrix4dv                     = ( PFNGLPROGRAMUNIFORMMATRIX4DVPROC                     ) gl3w_fn("glProgramUniformMatrix4dv");
	gl3wProgramUniformMatrix4fv                     = ( PFNGLPROGRAMUNIFORMMATRIX4FVPROC                     ) gl3w_fn("glProgramUniformMatrix4fv");
	gl3wProgramUniformMatrix4x2dv                   = ( PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC                   ) gl3w_fn("glProgramUniformMatrix4x2dv");
	gl3wProgramUniformMatrix4x2fv                   = ( PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC                   ) gl3w_fn("glProgramUniformMatrix4x2fv");
	gl3wProgramUniformMatrix4x3dv                   = ( PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC                   ) gl3w_fn("glProgramUniformMatrix4x3dv");
	gl3wProgramUniformMatrix4x3fv                   = ( PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC                   ) gl3w_fn("glProgramUniformMatrix4x3fv");
	gl3wProvokingVertex                             = ( PFNGLPROVOKINGVERTEXPROC                             ) gl3w_fn("glProvokingVertex");
	gl3wPushDebugGroup                              = ( PFNGLPUSHDEBUGGROUPPROC                              ) gl3w_fn("glPushDebugGroup");
	gl3wQueryCounter                                = ( PFNGLQUERYCOUNTERPROC                                ) gl3w_fn("glQueryCounter");
	gl3wReadBuffer                                  = ( PFNGLREADBUFFERPROC                                  ) gl3w_fn("glReadBuffer");
	gl3wReadPixels                                  = ( PFNGLREADPIXELSPROC                                  ) gl3w_fn("glReadPixels");
	gl3wReadnPixels                                 = ( PFNGLREADNPIXELSPROC                                 ) gl3w_fn("glReadnPixels");
	gl3wReadnPixelsARB                              = ( PFNGLREADNPIXELSARBPROC                              ) gl3w_fn("glReadnPixelsARB");
	gl3wReleaseShaderCompiler                       = ( PFNGLRELEASESHADERCOMPILERPROC                       ) gl3w_fn("glReleaseShaderCompiler");
	gl3wRenderbufferStorage                         = ( PFNGLRENDERBUFFERSTORAGEPROC                         ) gl3w_fn("glRenderbufferStorage");
	gl3wRenderbufferStorageMultisample              = ( PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC              ) gl3w_fn("glRenderbufferStorageMultisample");
	gl3wResumeTransformFeedback                     = ( PFNGLRESUMETRANSFORMFEEDBACKPROC                     ) gl3w_fn("glResumeTransformFeedback");
	gl3wSampleCoverage                              = ( PFNGLSAMPLECOVERAGEPROC                              ) gl3w_fn("glSampleCoverage");
	gl3wSampleMaski                                 = ( PFNGLSAMPLEMASKIPROC                                 ) gl3w_fn("glSampleMaski");
	gl3wSamplerParameterIiv                         = ( PFNGLSAMPLERPARAMETERIIVPROC                         ) gl3w_fn("glSamplerParameterIiv");
	gl3wSamplerParameterIuiv                        = ( PFNGLSAMPLERPARAMETERIUIVPROC                        ) gl3w_fn("glSamplerParameterIuiv");
	gl3wSamplerParameterf                           = ( PFNGLSAMPLERPARAMETERFPROC                           ) gl3w_fn("glSamplerParameterf");
	gl3wSamplerParameterfv                          = ( PFNGLSAMPLERPARAMETERFVPROC                          ) gl3w_fn("glSamplerParameterfv");
	gl3wSamplerParameteri                           = ( PFNGLSAMPLERPARAMETERIPROC                           ) gl3w_fn("glSamplerParameteri");
	gl3wSamplerParameteriv                          = ( PFNGLSAMPLERPARAMETERIVPROC                          ) gl3w_fn("glSamplerParameteriv");
	gl3wScissor                                     = ( PFNGLSCISSORPROC                                     ) gl3w_fn("glScissor");
	gl3wScissorArrayv                               = ( PFNGLSCISSORARRAYVPROC                               ) gl3w_fn("glScissorArrayv");
	gl3wScissorIndexed                              = ( PFNGLSCISSORINDEXEDPROC                              ) gl3w_fn("glScissorIndexed");
	gl3wScissorIndexedv                             = ( PFNGLSCISSORINDEXEDVPROC                             ) gl3w_fn("glScissorIndexedv");
	gl3wShaderBinary                                = ( PFNGLSHADERBINARYPROC                                ) gl3w_fn("glShaderBinary");
	gl3wShaderSource                                = ( PFNGLSHADERSOURCEPROC                                ) gl3w_fn("glShaderSource");
	gl3wShaderStorageBlockBinding                   = ( PFNGLSHADERSTORAGEBLOCKBINDINGPROC                   ) gl3w_fn("glShaderStorageBlockBinding");
	gl3wStencilFunc                                 = ( PFNGLSTENCILFUNCPROC                                 ) gl3w_fn("glStencilFunc");
	gl3wStencilFuncSeparate                         = ( PFNGLSTENCILFUNCSEPARATEPROC                         ) gl3w_fn("glStencilFuncSeparate");
	gl3wStencilMask                                 = ( PFNGLSTENCILMASKPROC                                 ) gl3w_fn("glStencilMask");
	gl3wStencilMaskSeparate                         = ( PFNGLSTENCILMASKSEPARATEPROC                         ) gl3w_fn("glStencilMaskSeparate");
	gl3wStencilOp                                   = ( PFNGLSTENCILOPPROC                                   ) gl3w_fn("glStencilOp");
	gl3wStencilOpSeparate                           = ( PFNGLSTENCILOPSEPARATEPROC                           ) gl3w_fn("glStencilOpSeparate");
	gl3wTexBuffer                                   = ( PFNGLTEXBUFFERPROC                                   ) gl3w_fn("glTexBuffer");
	gl3wTexBufferRange                              = ( PFNGLTEXBUFFERRANGEPROC                              ) gl3w_fn("glTexBufferRange");
	gl3wTexImage1D                                  = ( PFNGLTEXIMAGE1DPROC                                  ) gl3w_fn("glTexImage1D");
	gl3wTexImage2D                                  = ( PFNGLTEXIMAGE2DPROC                                  ) gl3w_fn("glTexImage2D");
	gl3wTexImage2DMultisample                       = ( PFNGLTEXIMAGE2DMULTISAMPLEPROC                       ) gl3w_fn("glTexImage2DMultisample");
	gl3wTexImage3D                                  = ( PFNGLTEXIMAGE3DPROC                                  ) gl3w_fn("glTexImage3D");
	gl3wTexImage3DMultisample                       = ( PFNGLTEXIMAGE3DMULTISAMPLEPROC                       ) gl3w_fn("glTexImage3DMultisample");
	gl3wTexPageCommitmentARB                        = ( PFNGLTEXPAGECOMMITMENTARBPROC                        ) gl3w_fn("glTexPageCommitmentARB");
	gl3wTexParameterIiv                             = ( PFNGLTEXPARAMETERIIVPROC                             ) gl3w_fn("glTexParameterIiv");
	gl3wTexParameterIuiv                            = ( PFNGLTEXPARAMETERIUIVPROC                            ) gl3w_fn("glTexParameterIuiv");
	gl3wTexParameterf                               = ( PFNGLTEXPARAMETERFPROC                               ) gl3w_fn("glTexParameterf");
	gl3wTexParameterfv                              = ( PFNGLTEXPARAMETERFVPROC                              ) gl3w_fn("glTexParameterfv");
	gl3wTexParameteri                               = ( PFNGLTEXPARAMETERIPROC                               ) gl3w_fn("glTexParameteri");
	gl3wTexParameteriv                              = ( PFNGLTEXPARAMETERIVPROC                              ) gl3w_fn("glTexParameteriv");
	gl3wTexStorage1D                                = ( PFNGLTEXSTORAGE1DPROC                                ) gl3w_fn("glTexStorage1D");
	gl3wTexStorage2D                                = ( PFNGLTEXSTORAGE2DPROC                                ) gl3w_fn("glTexStorage2D");
	gl3wTexStorage2DMultisample                     = ( PFNGLTEXSTORAGE2DMULTISAMPLEPROC                     ) gl3w_fn("glTexStorage2DMultisample");
	gl3wTexStorage3D                                = ( PFNGLTEXSTORAGE3DPROC                                ) gl3w_fn("glTexStorage3D");
	gl3wTexStorage3DMultisample                     = ( PFNGLTEXSTORAGE3DMULTISAMPLEPROC                     ) gl3w_fn("glTexStorage3DMultisample");
	gl3wTexSubImage1D                               = ( PFNGLTEXSUBIMAGE1DPROC                               ) gl3w_fn("glTexSubImage1D");
	gl3wTexSubImage2D                               = ( PFNGLTEXSUBIMAGE2DPROC                               ) gl3w_fn("glTexSubImage2D");
	gl3wTexSubImage3D                               = ( PFNGLTEXSUBIMAGE3DPROC                               ) gl3w_fn("glTexSubImage3D");
	gl3wTextureBarrier                              = ( PFNGLTEXTUREBARRIERPROC                              ) gl3w_fn("glTextureBarrier");
	gl3wTextureBuffer                               = ( PFNGLTEXTUREBUFFERPROC                               ) gl3w_fn("glTextureBuffer");
	gl3wTextureBufferRange                          = ( PFNGLTEXTUREBUFFERRANGEPROC                          ) gl3w_fn("glTextureBufferRange");
	gl3wTextureParameterIiv                         = ( PFNGLTEXTUREPARAMETERIIVPROC                         ) gl3w_fn("glTextureParameterIiv");
	gl3wTextureParameterIuiv                        = ( PFNGLTEXTUREPARAMETERIUIVPROC                        ) gl3w_fn("glTextureParameterIuiv");
	gl3wTextureParameterf                           = ( PFNGLTEXTUREPARAMETERFPROC                           ) gl3w_fn("glTextureParameterf");
	gl3wTextureParameterfv                          = ( PFNGLTEXTUREPARAMETERFVPROC                          ) gl3w_fn("glTextureParameterfv");
	gl3wTextureParameteri                           = ( PFNGLTEXTUREPARAMETERIPROC                           ) gl3w_fn("glTextureParameteri");
	gl3wTextureParameteriv                          = ( PFNGLTEXTUREPARAMETERIVPROC                          ) gl3w_fn("glTextureParameteriv");
	gl3wTextureStorage1D                            = ( PFNGLTEXTURESTORAGE1DPROC                            ) gl3w_fn("glTextureStorage1D");
	gl3wTextureStorage2D                            = ( PFNGLTEXTURESTORAGE2DPROC                            ) gl3w_fn("glTextureStorage2D");
	gl3wTextureStorage2DMultisample                 = ( PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC                 ) gl3w_fn("glTextureStorage2DMultisample");
	gl3wTextureStorage3D                            = ( PFNGLTEXTURESTORAGE3DPROC                            ) gl3w_fn("glTextureStorage3D");
	gl3wTextureStorage3DMultisample                 = ( PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC                 ) gl3w_fn("glTextureStorage3DMultisample");
	gl3wTextureSubImage1D                           = ( PFNGLTEXTURESUBIMAGE1DPROC                           ) gl3w_fn("glTextureSubImage1D");
	gl3wTextureSubImage2D                           = ( PFNGLTEXTURESUBIMAGE2DPROC                           ) gl3w_fn("glTextureSubImage2D");
	gl3wTextureSubImage3D                           = ( PFNGLTEXTURESUBIMAGE3DPROC                           ) gl3w_fn("glTextureSubImage3D");
	gl3wTextureView                                 = ( PFNGLTEXTUREVIEWPROC                                 ) gl3w_fn("glTextureView");
	gl3wTransformFeedbackBufferBase                 = ( PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC                 ) gl3w_fn("glTransformFeedbackBufferBase");
	gl3wTransformFeedbackBufferRange                = ( PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC                ) gl3w_fn("glTransformFeedbackBufferRange");
	gl3wTransformFeedbackVaryings                   = ( PFNGLTRANSFORMFEEDBACKVARYINGSPROC                   ) gl3w_fn("glTransformFeedbackVaryings");
	gl3wUniform1d                                   = ( PFNGLUNIFORM1DPROC                                   ) gl3w_fn("glUniform1d");
	gl3wUniform1dv                                  = ( PFNGLUNIFORM1DVPROC                                  ) gl3w_fn("glUniform1dv");
	gl3wUniform1f                                   = ( PFNGLUNIFORM1FPROC                                   ) gl3w_fn("glUniform1f");
	gl3wUniform1fv                                  = ( PFNGLUNIFORM1FVPROC                                  ) gl3w_fn("glUniform1fv");
	gl3wUniform1i                                   = ( PFNGLUNIFORM1IPROC                                   ) gl3w_fn("glUniform1i");
	gl3wUniform1iv                                  = ( PFNGLUNIFORM1IVPROC                                  ) gl3w_fn("glUniform1iv");
	gl3wUniform1ui                                  = ( PFNGLUNIFORM1UIPROC                                  ) gl3w_fn("glUniform1ui");
	gl3wUniform1uiv                                 = ( PFNGLUNIFORM1UIVPROC                                 ) gl3w_fn("glUniform1uiv");
	gl3wUniform2d                                   = ( PFNGLUNIFORM2DPROC                                   ) gl3w_fn("glUniform2d");
	gl3wUniform2dv                                  = ( PFNGLUNIFORM2DVPROC                                  ) gl3w_fn("glUniform2dv");
	gl3wUniform2f                                   = ( PFNGLUNIFORM2FPROC                                   ) gl3w_fn("glUniform2f");
	gl3wUniform2fv                                  = ( PFNGLUNIFORM2FVPROC                                  ) gl3w_fn("glUniform2fv");
	gl3wUniform2i                                   = ( PFNGLUNIFORM2IPROC                                   ) gl3w_fn("glUniform2i");
	gl3wUniform2iv                                  = ( PFNGLUNIFORM2IVPROC                                  ) gl3w_fn("glUniform2iv");
	gl3wUniform2ui                                  = ( PFNGLUNIFORM2UIPROC                                  ) gl3w_fn("glUniform2ui");
	gl3wUniform2uiv                                 = ( PFNGLUNIFORM2UIVPROC                                 ) gl3w_fn("glUniform2uiv");
	gl3wUniform3d                                   = ( PFNGLUNIFORM3DPROC                                   ) gl3w_fn("glUniform3d");
	gl3wUniform3dv                                  = ( PFNGLUNIFORM3DVPROC                                  ) gl3w_fn("glUniform3dv");
	gl3wUniform3f                                   = ( PFNGLUNIFORM3FPROC                                   ) gl3w_fn("glUniform3f");
	gl3wUniform3fv                                  = ( PFNGLUNIFORM3FVPROC                                  ) gl3w_fn("glUniform3fv");
	gl3wUniform3i                                   = ( PFNGLUNIFORM3IPROC                                   ) gl3w_fn("glUniform3i");
	gl3wUniform3iv                                  = ( PFNGLUNIFORM3IVPROC                                  ) gl3w_fn("glUniform3iv");
	gl3wUniform3ui                                  = ( PFNGLUNIFORM3UIPROC                                  ) gl3w_fn("glUniform3ui");
	gl3wUniform3uiv                                 = ( PFNGLUNIFORM3UIVPROC                                 ) gl3w_fn("glUniform3uiv");
	gl3wUniform4d                                   = ( PFNGLUNIFORM4DPROC                                   ) gl3w_fn("glUniform4d");
	gl3wUniform4dv                                  = ( PFNGLUNIFORM4DVPROC                                  ) gl3w_fn("glUniform4dv");
	gl3wUniform4f                                   = ( PFNGLUNIFORM4FPROC                                   ) gl3w_fn("glUniform4f");
	gl3wUniform4fv                                  = ( PFNGLUNIFORM4FVPROC                                  ) gl3w_fn("glUniform4fv");
	gl3wUniform4i                                   = ( PFNGLUNIFORM4IPROC                                   ) gl3w_fn("glUniform4i");
	gl3wUniform4iv                                  = ( PFNGLUNIFORM4IVPROC                                  ) gl3w_fn("glUniform4iv");
	gl3wUniform4ui                                  = ( PFNGLUNIFORM4UIPROC                                  ) gl3w_fn("glUniform4ui");
	gl3wUniform4uiv                                 = ( PFNGLUNIFORM4UIVPROC                                 ) gl3w_fn("glUniform4uiv");
	gl3wUniformBlockBinding                         = ( PFNGLUNIFORMBLOCKBINDINGPROC                         ) gl3w_fn("glUniformBlockBinding");
	gl3wUniformHandleui64ARB                        = ( PFNGLUNIFORMHANDLEUI64ARBPROC                        ) gl3w_fn("glUniformHandleui64ARB");
	gl3wUniformHandleui64vARB                       = ( PFNGLUNIFORMHANDLEUI64VARBPROC                       ) gl3w_fn("glUniformHandleui64vARB");
	gl3wUniformMatrix2dv                            = ( PFNGLUNIFORMMATRIX2DVPROC                            ) gl3w_fn("glUniformMatrix2dv");
	gl3wUniformMatrix2fv                            = ( PFNGLUNIFORMMATRIX2FVPROC                            ) gl3w_fn("glUniformMatrix2fv");
	gl3wUniformMatrix2x3dv                          = ( PFNGLUNIFORMMATRIX2X3DVPROC                          ) gl3w_fn("glUniformMatrix2x3dv");
	gl3wUniformMatrix2x3fv                          = ( PFNGLUNIFORMMATRIX2X3FVPROC                          ) gl3w_fn("glUniformMatrix2x3fv");
	gl3wUniformMatrix2x4dv                          = ( PFNGLUNIFORMMATRIX2X4DVPROC                          ) gl3w_fn("glUniformMatrix2x4dv");
	gl3wUniformMatrix2x4fv                          = ( PFNGLUNIFORMMATRIX2X4FVPROC                          ) gl3w_fn("glUniformMatrix2x4fv");
	gl3wUniformMatrix3dv                            = ( PFNGLUNIFORMMATRIX3DVPROC                            ) gl3w_fn("glUniformMatrix3dv");
	gl3wUniformMatrix3fv                            = ( PFNGLUNIFORMMATRIX3FVPROC                            ) gl3w_fn("glUniformMatrix3fv");
	gl3wUniformMatrix3x2dv                          = ( PFNGLUNIFORMMATRIX3X2DVPROC                          ) gl3w_fn("glUniformMatrix3x2dv");
	gl3wUniformMatrix3x2fv                          = ( PFNGLUNIFORMMATRIX3X2FVPROC                          ) gl3w_fn("glUniformMatrix3x2fv");
	gl3wUniformMatrix3x4dv                          = ( PFNGLUNIFORMMATRIX3X4DVPROC                          ) gl3w_fn("glUniformMatrix3x4dv");
	gl3wUniformMatrix3x4fv                          = ( PFNGLUNIFORMMATRIX3X4FVPROC                          ) gl3w_fn("glUniformMatrix3x4fv");
	gl3wUniformMatrix4dv                            = ( PFNGLUNIFORMMATRIX4DVPROC                            ) gl3w_fn("glUniformMatrix4dv");
	gl3wUniformMatrix4fv                            = ( PFNGLUNIFORMMATRIX4FVPROC                            ) gl3w_fn("glUniformMatrix4fv");
	gl3wUniformMatrix4x2dv                          = ( PFNGLUNIFORMMATRIX4X2DVPROC                          ) gl3w_fn("glUniformMatrix4x2dv");
	gl3wUniformMatrix4x2fv                          = ( PFNGLUNIFORMMATRIX4X2FVPROC                          ) gl3w_fn("glUniformMatrix4x2fv");
	gl3wUniformMatrix4x3dv                          = ( PFNGLUNIFORMMATRIX4X3DVPROC                          ) gl3w_fn("glUniformMatrix4x3dv");
	gl3wUniformMatrix4x3fv                          = ( PFNGLUNIFORMMATRIX4X3FVPROC                          ) gl3w_fn("glUniformMatrix4x3fv");
	gl3wUniformSubroutinesuiv                       = ( PFNGLUNIFORMSUBROUTINESUIVPROC                       ) gl3w_fn("glUniformSubroutinesuiv");
	gl3wUnmapBuffer                                 = ( PFNGLUNMAPBUFFERPROC                                 ) gl3w_fn("glUnmapBuffer");
	gl3wUnmapNamedBuffer                            = ( PFNGLUNMAPNAMEDBUFFERPROC                            ) gl3w_fn("glUnmapNamedBuffer");
	gl3wUseProgram                                  = ( PFNGLUSEPROGRAMPROC                                  ) gl3w_fn("glUseProgram");
	gl3wUseProgramStages                            = ( PFNGLUSEPROGRAMSTAGESPROC                            ) gl3w_fn("glUseProgramStages");
	gl3wValidateProgram                             = ( PFNGLVALIDATEPROGRAMPROC                             ) gl3w_fn("glValidateProgram");
	gl3wValidateProgramPipeline                     = ( PFNGLVALIDATEPROGRAMPIPELINEPROC                     ) gl3w_fn("glValidateProgramPipeline");
	gl3wVertexArrayAttribBinding                    = ( PFNGLVERTEXARRAYATTRIBBINDINGPROC                    ) gl3w_fn("glVertexArrayAttribBinding");
	gl3wVertexArrayAttribFormat                     = ( PFNGLVERTEXARRAYATTRIBFORMATPROC                     ) gl3w_fn("glVertexArrayAttribFormat");
	gl3wVertexArrayAttribIFormat                    = ( PFNGLVERTEXARRAYATTRIBIFORMATPROC                    ) gl3w_fn("glVertexArrayAttribIFormat");
	gl3wVertexArrayAttribLFormat                    = ( PFNGLVERTEXARRAYATTRIBLFORMATPROC                    ) gl3w_fn("glVertexArrayAttribLFormat");
	gl3wVertexArrayBindingDivisor                   = ( PFNGLVERTEXARRAYBINDINGDIVISORPROC                   ) gl3w_fn("glVertexArrayBindingDivisor");
	gl3wVertexArrayElementBuffer                    = ( PFNGLVERTEXARRAYELEMENTBUFFERPROC                    ) gl3w_fn("glVertexArrayElementBuffer");
	gl3wVertexArrayVertexBuffer                     = ( PFNGLVERTEXARRAYVERTEXBUFFERPROC                     ) gl3w_fn("glVertexArrayVertexBuffer");
	gl3wVertexArrayVertexBuffers                    = ( PFNGLVERTEXARRAYVERTEXBUFFERSPROC                    ) gl3w_fn("glVertexArrayVertexBuffers");
	gl3wVertexAttrib1d                              = ( PFNGLVERTEXATTRIB1DPROC                              ) gl3w_fn("glVertexAttrib1d");
	gl3wVertexAttrib1dv                             = ( PFNGLVERTEXATTRIB1DVPROC                             ) gl3w_fn("glVertexAttrib1dv");
	gl3wVertexAttrib1f                              = ( PFNGLVERTEXATTRIB1FPROC                              ) gl3w_fn("glVertexAttrib1f");
	gl3wVertexAttrib1fv                             = ( PFNGLVERTEXATTRIB1FVPROC                             ) gl3w_fn("glVertexAttrib1fv");
	gl3wVertexAttrib1s                              = ( PFNGLVERTEXATTRIB1SPROC                              ) gl3w_fn("glVertexAttrib1s");
	gl3wVertexAttrib1sv                             = ( PFNGLVERTEXATTRIB1SVPROC                             ) gl3w_fn("glVertexAttrib1sv");
	gl3wVertexAttrib2d                              = ( PFNGLVERTEXATTRIB2DPROC                              ) gl3w_fn("glVertexAttrib2d");
	gl3wVertexAttrib2dv                             = ( PFNGLVERTEXATTRIB2DVPROC                             ) gl3w_fn("glVertexAttrib2dv");
	gl3wVertexAttrib2f                              = ( PFNGLVERTEXATTRIB2FPROC                              ) gl3w_fn("glVertexAttrib2f");
	gl3wVertexAttrib2fv                             = ( PFNGLVERTEXATTRIB2FVPROC                             ) gl3w_fn("glVertexAttrib2fv");
	gl3wVertexAttrib2s                              = ( PFNGLVERTEXATTRIB2SPROC                              ) gl3w_fn("glVertexAttrib2s");
	gl3wVertexAttrib2sv                             = ( PFNGLVERTEXATTRIB2SVPROC                             ) gl3w_fn("glVertexAttrib2sv");
	gl3wVertexAttrib3d                              = ( PFNGLVERTEXATTRIB3DPROC                              ) gl3w_fn("glVertexAttrib3d");
	gl3wVertexAttrib3dv                             = ( PFNGLVERTEXATTRIB3DVPROC                             ) gl3w_fn("glVertexAttrib3dv");
	gl3wVertexAttrib3f                              = ( PFNGLVERTEXATTRIB3FPROC                              ) gl3w_fn("glVertexAttrib3f");
	gl3wVertexAttrib3fv                             = ( PFNGLVERTEXATTRIB3FVPROC                             ) gl3w_fn("glVertexAttrib3fv");
	gl3wVertexAttrib3s                              = ( PFNGLVERTEXATTRIB3SPROC                              ) gl3w_fn("glVertexAttrib3s");
	gl3wVertexAttrib3sv                             = ( PFNGLVERTEXATTRIB3SVPROC                             ) gl3w_fn("glVertexAttrib3sv");
	gl3wVertexAttrib4Nbv                            = ( PFNGLVERTEXATTRIB4NBVPROC                            ) gl3w_fn("glVertexAttrib4Nbv");
	gl3wVertexAttrib4Niv                            = ( PFNGLVERTEXATTRIB4NIVPROC                            ) gl3w_fn("glVertexAttrib4Niv");
	gl3wVertexAttrib4Nsv                            = ( PFNGLVERTEXATTRIB4NSVPROC                            ) gl3w_fn("glVertexAttrib4Nsv");
	gl3wVertexAttrib4Nub                            = ( PFNGLVERTEXATTRIB4NUBPROC                            ) gl3w_fn("glVertexAttrib4Nub");
	gl3wVertexAttrib4Nubv                           = ( PFNGLVERTEXATTRIB4NUBVPROC                           ) gl3w_fn("glVertexAttrib4Nubv");
	gl3wVertexAttrib4Nuiv                           = ( PFNGLVERTEXATTRIB4NUIVPROC                           ) gl3w_fn("glVertexAttrib4Nuiv");
	gl3wVertexAttrib4Nusv                           = ( PFNGLVERTEXATTRIB4NUSVPROC                           ) gl3w_fn("glVertexAttrib4Nusv");
	gl3wVertexAttrib4bv                             = ( PFNGLVERTEXATTRIB4BVPROC                             ) gl3w_fn("glVertexAttrib4bv");
	gl3wVertexAttrib4d                              = ( PFNGLVERTEXATTRIB4DPROC                              ) gl3w_fn("glVertexAttrib4d");
	gl3wVertexAttrib4dv                             = ( PFNGLVERTEXATTRIB4DVPROC                             ) gl3w_fn("glVertexAttrib4dv");
	gl3wVertexAttrib4f                              = ( PFNGLVERTEXATTRIB4FPROC                              ) gl3w_fn("glVertexAttrib4f");
	gl3wVertexAttrib4fv                             = ( PFNGLVERTEXATTRIB4FVPROC                             ) gl3w_fn("glVertexAttrib4fv");
	gl3wVertexAttrib4iv                             = ( PFNGLVERTEXATTRIB4IVPROC                             ) gl3w_fn("glVertexAttrib4iv");
	gl3wVertexAttrib4s                              = ( PFNGLVERTEXATTRIB4SPROC                              ) gl3w_fn("glVertexAttrib4s");
	gl3wVertexAttrib4sv                             = ( PFNGLVERTEXATTRIB4SVPROC                             ) gl3w_fn("glVertexAttrib4sv");
	gl3wVertexAttrib4ubv                            = ( PFNGLVERTEXATTRIB4UBVPROC                            ) gl3w_fn("glVertexAttrib4ubv");
	gl3wVertexAttrib4uiv                            = ( PFNGLVERTEXATTRIB4UIVPROC                            ) gl3w_fn("glVertexAttrib4uiv");
	gl3wVertexAttrib4usv                            = ( PFNGLVERTEXATTRIB4USVPROC                            ) gl3w_fn("glVertexAttrib4usv");
	gl3wVertexAttribBinding                         = ( PFNGLVERTEXATTRIBBINDINGPROC                         ) gl3w_fn("glVertexAttribBinding");
	gl3wVertexAttribDivisor                         = ( PFNGLVERTEXATTRIBDIVISORPROC                         ) gl3w_fn("glVertexAttribDivisor");
	gl3wVertexAttribFormat                          = ( PFNGLVERTEXATTRIBFORMATPROC                          ) gl3w_fn("glVertexAttribFormat");
	gl3wVertexAttribI1i                             = ( PFNGLVERTEXATTRIBI1IPROC                             ) gl3w_fn("glVertexAttribI1i");
	gl3wVertexAttribI1iv                            = ( PFNGLVERTEXATTRIBI1IVPROC                            ) gl3w_fn("glVertexAttribI1iv");
	gl3wVertexAttribI1ui                            = ( PFNGLVERTEXATTRIBI1UIPROC                            ) gl3w_fn("glVertexAttribI1ui");
	gl3wVertexAttribI1uiv                           = ( PFNGLVERTEXATTRIBI1UIVPROC                           ) gl3w_fn("glVertexAttribI1uiv");
	gl3wVertexAttribI2i                             = ( PFNGLVERTEXATTRIBI2IPROC                             ) gl3w_fn("glVertexAttribI2i");
	gl3wVertexAttribI2iv                            = ( PFNGLVERTEXATTRIBI2IVPROC                            ) gl3w_fn("glVertexAttribI2iv");
	gl3wVertexAttribI2ui                            = ( PFNGLVERTEXATTRIBI2UIPROC                            ) gl3w_fn("glVertexAttribI2ui");
	gl3wVertexAttribI2uiv                           = ( PFNGLVERTEXATTRIBI2UIVPROC                           ) gl3w_fn("glVertexAttribI2uiv");
	gl3wVertexAttribI3i                             = ( PFNGLVERTEXATTRIBI3IPROC                             ) gl3w_fn("glVertexAttribI3i");
	gl3wVertexAttribI3iv                            = ( PFNGLVERTEXATTRIBI3IVPROC                            ) gl3w_fn("glVertexAttribI3iv");
	gl3wVertexAttribI3ui                            = ( PFNGLVERTEXATTRIBI3UIPROC                            ) gl3w_fn("glVertexAttribI3ui");
	gl3wVertexAttribI3uiv                           = ( PFNGLVERTEXATTRIBI3UIVPROC                           ) gl3w_fn("glVertexAttribI3uiv");
	gl3wVertexAttribI4bv                            = ( PFNGLVERTEXATTRIBI4BVPROC                            ) gl3w_fn("glVertexAttribI4bv");
	gl3wVertexAttribI4i                             = ( PFNGLVERTEXATTRIBI4IPROC                             ) gl3w_fn("glVertexAttribI4i");
	gl3wVertexAttribI4iv                            = ( PFNGLVERTEXATTRIBI4IVPROC                            ) gl3w_fn("glVertexAttribI4iv");
	gl3wVertexAttribI4sv                            = ( PFNGLVERTEXATTRIBI4SVPROC                            ) gl3w_fn("glVertexAttribI4sv");
	gl3wVertexAttribI4ubv                           = ( PFNGLVERTEXATTRIBI4UBVPROC                           ) gl3w_fn("glVertexAttribI4ubv");
	gl3wVertexAttribI4ui                            = ( PFNGLVERTEXATTRIBI4UIPROC                            ) gl3w_fn("glVertexAttribI4ui");
	gl3wVertexAttribI4uiv                           = ( PFNGLVERTEXATTRIBI4UIVPROC                           ) gl3w_fn("glVertexAttribI4uiv");
	gl3wVertexAttribI4usv                           = ( PFNGLVERTEXATTRIBI4USVPROC                           ) gl3w_fn("glVertexAttribI4usv");
	gl3wVertexAttribIFormat                         = ( PFNGLVERTEXATTRIBIFORMATPROC                         ) gl3w_fn("glVertexAttribIFormat");
	gl3wVertexAttribIPointer                        = ( PFNGLVERTEXATTRIBIPOINTERPROC                        ) gl3w_fn("glVertexAttribIPointer");
	gl3wVertexAttribL1d                             = ( PFNGLVERTEXATTRIBL1DPROC                             ) gl3w_fn("glVertexAttribL1d");
	gl3wVertexAttribL1dv                            = ( PFNGLVERTEXATTRIBL1DVPROC                            ) gl3w_fn("glVertexAttribL1dv");
	gl3wVertexAttribL1ui64ARB                       = ( PFNGLVERTEXATTRIBL1UI64ARBPROC                       ) gl3w_fn("glVertexAttribL1ui64ARB");
	gl3wVertexAttribL1ui64vARB                      = ( PFNGLVERTEXATTRIBL1UI64VARBPROC                      ) gl3w_fn("glVertexAttribL1ui64vARB");
	gl3wVertexAttribL2d                             = ( PFNGLVERTEXATTRIBL2DPROC                             ) gl3w_fn("glVertexAttribL2d");
	gl3wVertexAttribL2dv                            = ( PFNGLVERTEXATTRIBL2DVPROC                            ) gl3w_fn("glVertexAttribL2dv");
	gl3wVertexAttribL3d                             = ( PFNGLVERTEXATTRIBL3DPROC                             ) gl3w_fn("glVertexAttribL3d");
	gl3wVertexAttribL3dv                            = ( PFNGLVERTEXATTRIBL3DVPROC                            ) gl3w_fn("glVertexAttribL3dv");
	gl3wVertexAttribL4d                             = ( PFNGLVERTEXATTRIBL4DPROC                             ) gl3w_fn("glVertexAttribL4d");
	gl3wVertexAttribL4dv                            = ( PFNGLVERTEXATTRIBL4DVPROC                            ) gl3w_fn("glVertexAttribL4dv");
	gl3wVertexAttribLFormat                         = ( PFNGLVERTEXATTRIBLFORMATPROC                         ) gl3w_fn("glVertexAttribLFormat");
	gl3wVertexAttribLPointer                        = ( PFNGLVERTEXATTRIBLPOINTERPROC                        ) gl3w_fn("glVertexAttribLPointer");
	gl3wVertexAttribP1ui                            = ( PFNGLVERTEXATTRIBP1UIPROC                            ) gl3w_fn("glVertexAttribP1ui");
	gl3wVertexAttribP1uiv                           = ( PFNGLVERTEXATTRIBP1UIVPROC                           ) gl3w_fn("glVertexAttribP1uiv");
	gl3wVertexAttribP2ui                            = ( PFNGLVERTEXATTRIBP2UIPROC                            ) gl3w_fn("glVertexAttribP2ui");
	gl3wVertexAttribP2uiv                           = ( PFNGLVERTEXATTRIBP2UIVPROC                           ) gl3w_fn("glVertexAttribP2uiv");
	gl3wVertexAttribP3ui                            = ( PFNGLVERTEXATTRIBP3UIPROC                            ) gl3w_fn("glVertexAttribP3ui");
	gl3wVertexAttribP3uiv                           = ( PFNGLVERTEXATTRIBP3UIVPROC                           ) gl3w_fn("glVertexAttribP3uiv");
	gl3wVertexAttribP4ui                            = ( PFNGLVERTEXATTRIBP4UIPROC                            ) gl3w_fn("glVertexAttribP4ui");
	gl3wVertexAttribP4uiv                           = ( PFNGLVERTEXATTRIBP4UIVPROC                           ) gl3w_fn("glVertexAttribP4uiv");
	gl3wVertexAttribPointer                         = ( PFNGLVERTEXATTRIBPOINTERPROC                         ) gl3w_fn("glVertexAttribPointer");
	gl3wVertexBindingDivisor                        = ( PFNGLVERTEXBINDINGDIVISORPROC                        ) gl3w_fn("glVertexBindingDivisor");
	gl3wViewport                                    = ( PFNGLVIEWPORTPROC                                    ) gl3w_fn("glViewport");
	gl3wViewportArrayv                              = ( PFNGLVIEWPORTARRAYVPROC                              ) gl3w_fn("glViewportArrayv");
	gl3wViewportIndexedf                            = ( PFNGLVIEWPORTINDEXEDFPROC                            ) gl3w_fn("glViewportIndexedf");
	gl3wViewportIndexedfv                           = ( PFNGLVIEWPORTINDEXEDFVPROC                           ) gl3w_fn("glViewportIndexedfv");
	gl3wWaitSync                                    = ( PFNGLWAITSYNCPROC                                    ) gl3w_fn("glWaitSync");
}
