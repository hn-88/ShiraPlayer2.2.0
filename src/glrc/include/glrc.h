#ifndef _GLRC_H_
#define _GLRC_H_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GLRC_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GLRC_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifdef GLRC_EXPORTS
#define GLRC_API __declspec(dllexport)
#else
#define GLRC_API __declspec(dllimport)
#endif

#define OPENGL15

#define GLRC_COMPOSITOR_2D  0x00000000	//!< Enable / disable composite processing.
#define GLRC_BLEND          0x00000001	//!< Enable / disable composition with blending.
#define GLRC_MESHDISTORTION 0x00000002	//!< Enable / disable to use a mesh for distortion.
#define GLRC_MODELVIEW      0x00000003	//!< Enable / disable model-view matrix modification.
#define GLRC_PROJECTION     0x00000004	//!< Enable / disable projection matrix modification.

//! RenderToTexture mode.
enum GLRC_RTT
{
	GLRC_RTT_AUTO,			//!< Automatically select proper Render To Texture type.
	GLRC_RTT_READPIXEL,		//!< Use glReadpixel(). Very slow and has the window size limitation.
	GLRC_RTT_COPYSUBTEX,	//!< Use glCopyTexSubImageD(). Better but has the window size limitation.
	GLRC_RTT_PBUFFER,		//!< Use P-Buffer. Without window size limitation, though still copying buffer and need to switch the context.
	GLRC_RTT_PBUFFER_RTT,	//!< Use P-Buffer and RenderToTexture extension. Not copying buffer but has the context switching.
	GLRC_RTT_FBO			//!< Very fast and no copy buffer / context switching. Best option, if you can use it (with the latest driver).
};

#define GLRC_FLIP_NONE       0x00000000		//!< No flipping.
#define GLRC_FLIP_HORIZONTAL 0x00000001		//!< Flip horizontally.
#define GLRC_FLIP_VERTICAL   0x00000002		//!< Flip vertically.

#define GLRC_EFFECT_NONE        0x00000000	//!< Disable all composite processing.
#define GLRC_EFFECT_DISTORTION  0x00000001	//!< Distortion using the specified distortion map.
#define GLRC_EFFECT_HDR         0x00000002	//!< High-dynamic rendering processing. You may need a floating point buffer with it.
#define GLRC_EFFECT_COLORMATRIX 0x00000004	//!< Color matrix conversion (linear color re-mapping).
#define GLRC_EFFECT_ALL         0xffffffff	//!< Enable all composite processing.

//! V-Sync waiting mode.
enum GLRC_VSYNC
{
	GLRC_VSYNC_UNKNOWN = 0,	//!< Unknown about current V-Sync waiting mode.
	GLRC_VSYNC_WAIT = 1,	//!< Limit refresh rate to your display device to avoid flickers.
	GLRC_VSYNC_NOWAIT = 2	//!< Rendering as first as possible (with flickers).
};

//! Error code
enum GLRC_ERROR
{
	GLRC_ERROR_NOERROR = 0,				//!< No error ;-)
	GLRC_ERROR_NOT_INITIALIZED,			//!< GLRC functions called without valid compositors.
	GLRC_ERROR_ALREADY_INITIALIZED,		//!< glrcInit() called more than once.
	GLRC_ERROR_UNKNOWN_ERROR			//!< Unknown error related on GLRC :-(
};

// for some environment...
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif // TRUE

#if defined(__cplusplus)
extern "C"
{
#endif //__cplusplus

	GLRC_API void glrcInit();
	GLRC_API void glrcDestroy();

	GLRC_API void glrcGenCompositors(unsigned int n, unsigned int* compositors);
	GLRC_API void glrcDeleteCompositors(unsigned int n, unsigned int* compositors);
	GLRC_API int  glrcIsCompositor(unsigned int compositorIndex);

	GLRC_API void glrcBindCompositor(unsigned int target, unsigned int compositorIndex);

	GLRC_API void glrcSetupBuffer(enum GLRC_RTT rttType, int width, int height, int bHasAlpha, int bFPBuffer);
	GLRC_API enum GLRC_RTT glrcGetBufferType();
	GLRC_API int  glrcGetBufferWidth();
	GLRC_API int  glrcGetBufferHeight();
	GLRC_API int  glrcHasAlpha();
	GLRC_API int  glrcIsFPBuffer();

	GLRC_API void glrcViewport(float x, float y, float width, float height);
	GLRC_API float glrcGetRenderX();
	GLRC_API float glrcGetRenderY();
	GLRC_API float glrcGetRenderWidth();
	GLRC_API float glrcGetRenderHeight();

	GLRC_API void glrcEnable(unsigned int target);
	GLRC_API void glrcDisable(unsigned int target);
	GLRC_API int  glrcIsEnabled(unsigned int target);

	GLRC_API void glrcBeginRender();
	GLRC_API void glrcEndRender();

	GLRC_API void glrcSetRotation(float angle);
	GLRC_API float glrcGetRotation();

    GLRC_API void glrcSetScale(float x, float y);
	GLRC_API float glrcGetScaleX();
	GLRC_API float glrcGetScaleY();

	GLRC_API void glrcSetFlip(unsigned int flip);
	GLRC_API unsigned int glrcGetFlip();

	GLRC_API void glrcSetModelViewMatrix(float matrix[4][4]);
	GLRC_API void glrcGetModelViewMatrix(float matrix[4][4]);
/*	GLRC_API void glrcSetModelViewTranslation(float x, float y, float z); */
	GLRC_API void glrcGetModelViewTranslation(float* x, float* y, float* z);
/*	GLRC_API void glrcSetModelViewRotation(float angle, float x, float y, float z); */
	GLRC_API void glrcGetModelViewRotation(float* angle, float* x, float* y, float* z);
/*	GLRC_API void glrcSetModelViewQuatenion(float x, float y, float z, float w); */
	GLRC_API void glrcGetModelViewQuatenion(float* x, float* y, float* z, float* w);
/*	GLRC_API void glrcSetModelViewScaling(float x, float y, float z); */
	GLRC_API void glrcGetModelViewScaling(float* x, float* y, float* z);
	GLRC_API void glrcSetProjectionMatrix(float matrix[4][4]);
	GLRC_API void glrcGetProjectionMatrix(float matrix[4][4]);
    GLRC_API void glrcSetProjectionFrustum(float frustum[6]);
    GLRC_API void glrcGetProjectionFrustum(float frustum[6]);
    GLRC_API int  glrcLoadMatrixFile(const char* fileName);

	GLRC_API void glrcSetEffectMode(unsigned int effectMode);
	GLRC_API unsigned int glrcGetEffectMode();

	GLRC_API void glrcSetDistortionMap(const char* fileName);
	GLRC_API const char* glrcGetDistortionMapFileName();
    GLRC_API void glrcSetDistortionMeshResolution(int horResolution, int vertResolution);
    GLRC_API int  glrcGetHorizontalResolutionOfDisotrtionMesh();
    GLRC_API int  glrcGetHorizontalResolutionOfDisotrtionMesh();

	GLRC_API void glrcSetBlendMap(const char* fileName);
	GLRC_API const char* glrcGetBlendMapFileName();

	GLRC_API void  glrcSetExposure(float exposure);
	GLRC_API float glrcGetExposure();
	GLRC_API void  glrcSetGamma(float gamma);
	GLRC_API float glrcGetGamma();

	GLRC_API void glrcSetColorMatrix(float colorMatrix[4][4]);
	GLRC_API void glrcGetColorMatrix(float colorMatrix[4][4]);

    GLRC_API int glrcSaveBuffer(const char* fileName);
    GLRC_API int glrcSaveSnapshot(const char* fileName, int width, int height, int bHasAlpha, int bFPBuffer, void (*pDisplayFunc)(void));

	GLRC_API void glrcSetError(enum GLRC_ERROR error);
	GLRC_API enum GLRC_ERROR glrcGetError();

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _GLRC_H_
