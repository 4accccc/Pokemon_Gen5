//============================================================================================
/**
 * @file	talkmsgwin.h
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
/*
 * システム使用上の注意：
 *	・GFL_BMPWINシステムを使用するので事前に初期化しておく必要あり
 */
#pragma once

typedef struct _TALKMSGWIN_SYS			TALKMSGWIN_SYS;

#define TALKMSGWIN_NUM	(32)
#define TALKMSGWIN_NULL (-1)

typedef enum {
  TALKMSGWIN_TYPE_NORMAL,
  TALKMSGWIN_TYPE_GIZA,
}TALKMSGWIN_TYPE;

typedef struct {
	int			frameID;
	u8			winPltID;
	u8			fontPltID;
}TALKMSGWIN_SYS_INI;

typedef struct {
	HEAPID							heapID;
	GFL_G3D_CAMERA*			g3Dcamera;
	GFL_FONT*						fontHandle;
	u16									chrNumOffs;

	TALKMSGWIN_SYS_INI	ini;

}TALKMSGWIN_SYS_SETUP;

//============================================================================================
/**
 *
 * @brief	会話ウインドウシステム関数
 *
 */
//============================================================================================
extern TALKMSGWIN_SYS*	TALKMSGWIN_SystemCreate( TALKMSGWIN_SYS_SETUP* setup );
extern void TALKMSGWIN_SystemMain( TALKMSGWIN_SYS* tmsgwinSys );
extern void TALKMSGWIN_SystemDraw2D( TALKMSGWIN_SYS* tmsgwinSys );
extern void TALKMSGWIN_SystemDraw3D( TALKMSGWIN_SYS* tmsgwinSys );
extern void TALKMSGWIN_SystemDelete( TALKMSGWIN_SYS* tmsgwinSys );
extern u32 TALKMSGWIN_SystemGetUsingChrNumber( TALKMSGWIN_SYS* tmsgwinSys );
//完成まで暫定
extern void TALKMSGWIN_SystemDebugOn( TALKMSGWIN_SYS* tmsgwinSys );

//============================================================================================
/**
 *
 * @brief	会話ウインドウ設定関数
 *
 */
//============================================================================================


extern void TALKMSGWIN_CreateWindowAlone(	TALKMSGWIN_SYS*		tmsgwinSys, 
																			int								tmsgwinIdx,
																			STRBUF*						msg,
																			u8								winpx,			
																			u8								winpy,			
																			u8								winsx,			
																			u8								winsy,			
																			u8								colIdx,
                                      TALKMSGWIN_TYPE   winType );

extern void TALKMSGWIN_CreateFloatWindowIdx(	TALKMSGWIN_SYS*		tmsgwinSys, 
																							int								tmsgwinIdx,
																							VecFx32*					pTarget,
																							STRBUF*						msg,
																							u8								winpx,			
																							u8								winpy,			
																							u8								winsx,			
																							u8								winsy,			
																							u8								colIdx,
                                              TALKMSGWIN_TYPE   winType );

extern void TALKMSGWIN_CreateFloatWindowIdxConnect(	TALKMSGWIN_SYS*		tmsgwinSys, 
																										int								tmsgwinIdx,
																										int								prev_tmsgwinIdx,
																										STRBUF*						msg,
																										u8								winpx,			
																										u8								winpy,			
																										u8								winsx,			
																										u8								winsy,			
																										u8								colIdx,
                                                    TALKMSGWIN_TYPE   winType );

extern void TALKMSGWIN_CreateFixWindowUpper(	TALKMSGWIN_SYS* tmsgwinSys, 
																							int							tmsgwinIdx,
																							VecFx32*				pTarget,
																							STRBUF*					msg,
																							u8							colIdx,
                                              TALKMSGWIN_TYPE   winType );

extern void TALKMSGWIN_CreateFixWindowLower(	TALKMSGWIN_SYS* tmsgwinSys,
																							int							tmsgwinIdx,
																							VecFx32*				pTarget,
																							STRBUF*					msg,
																							u8							colIdx,
                                              TALKMSGWIN_TYPE   winType );

extern void TALKMSGWIN_CreateFixWindowAuto(		TALKMSGWIN_SYS* tmsgwinSys,
																							int							tmsgwinIdx,
																							VecFx32*				pTarget,
																							STRBUF*					msg,
																							u8							colIdx,
                                              TALKMSGWIN_TYPE winType );

extern void TALKMSGWIN_DeleteWindow( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx );
extern void TALKMSGWIN_OpenWindow( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx );
extern void TALKMSGWIN_CloseWindow( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx );
extern BOOL TALKMSGWIN_CheckCloseStatus( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx );

extern BOOL						TALKMSGWIN_CheckPrintOn( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx );
extern PRINT_STREAM*	TALKMSGWIN_GetPrintStream( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx );	
extern GFL_BMPWIN * TALKMSGWIN_GetBmpWin( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx );
extern void TALKMSGWIN_ReTransWindowBG( TALKMSGWIN_SYS* tmsgwinSys );

extern GFL_BMPWIN * TALKMSGWIN_CreateBmpWindow( TALKMSGWIN_SYS *tmsgwinSys, 
																			u8								winpx,			
																			u8								winpy,			
																			u8								winsx,
																			u8								winsy,
                                      TALKMSGWIN_TYPE   winType );
extern void TALKMSGWIN_WriteBmpWindow(
    TALKMSGWIN_SYS *tmsgwinSys, GFL_BMPWIN *bmpwin, TALKMSGWIN_TYPE winType );
extern void TALKMSGWIN_ClearBmpWindow(
    TALKMSGWIN_SYS *tmsgwinSys, GFL_BMPWIN *bmpwin );
extern void TALKMSGWIN_CleanBmpWindow(
    TALKMSGWIN_SYS *tmsgwinSys, GFL_BMPWIN *bmpwin );
extern void TALKMSGWIN_DeleteBmpWindow(
    TALKMSGWIN_SYS *tmsgwinSys, GFL_BMPWIN *bmpwin );
