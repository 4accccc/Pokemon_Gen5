//======================================================================
/**
 * @file	debugwin_sys.h
 * @brief	汎用的デバッグシステム
 * @author	ariizumi
 * @data	09/05/01
 */
//======================================================================

#include "print/gf_font.h"


#ifndef DEBUGWIN_SYS_H__
#define DEBUGWIN_SYS_H__

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#if DEBUG_ONLY_FOR_ariizumi_nobuhiko
#define USE_DEBUGWIN_SYSTEM (0)
#else
#define USE_DEBUGWIN_SYSTEM (0)
#endif

#define DEBUGWIN_TRG_KEY (PAD_BUTTON_SELECT)

//各種退避領域サイズ(WBではエラー画面から拝借するのでサイズをあわせてる
#define DEBWIN_CHAR_TEMP_AREA (0x4000)
#define DEBWIN_SCRN_TEMP_AREA (0x800)
#define DEBWIN_PLTT_TEMP_AREA (0x20)
//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
void DEBUGWIN_InitSystem( u8* charArea , u16* scrnArea , u16* plttArea );
void DEBUGWIN_UpdateSystem(void);
void DEBUGWIN_ExitSystem(void);
BOOL DEBUGWIN_IsActive(void);

//--------------------------------------------------------------
//	プロック毎に行う行う初期化・開放
//--------------------------------------------------------------
void DEBUGWIN_InitProc( const u32 frmnum , GFL_FONT *fontHandle );
void DEBUGWIN_ExitProc( void );


#endif //DEBUGWIN_SYS_H__
