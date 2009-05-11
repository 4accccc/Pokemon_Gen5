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
#define USE_DEBUGWIN_SYSTEM (1)
#else
#define USE_DEBUGWIN_SYSTEM (0)
#endif

#define DEBUGWIN_TRG_KEY (PAD_BUTTON_SELECT)

//各種退避領域サイズ(WBではエラー画面から拝借するのでサイズをあわせてる
#define DEBUGWIN_CHAR_TEMP_AREA (0x4000)
#define DEBUGWIN_SCRN_TEMP_AREA (0x800)
#define DEBUGWIN_PLTT_TEMP_AREA (0x20)
//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
/// 項目選択中のコールバック

typedef struct _DEBUGWIN_ITEM DEBUGWIN_ITEM;
typedef struct _DEBUGWIN_GROUP DEBUGWIN_GROUP;

typedef void (*DEBUGWIN_UPDATE_FUNC)( void* userWork , DEBUGWIN_ITEM* item );
typedef void (*DEBUGWIN_DRAW_FUNC)( void* userWork , DEBUGWIN_ITEM* itemWo );

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
//--------------------------------------------------------------
//	gfl_useで行う初期化・開放・実行処理
//--------------------------------------------------------------
void DEBUGWIN_InitSystem( u8* charArea , u16* scrnArea , u16* plttArea );
void DEBUGWIN_UpdateSystem(void);
void DEBUGWIN_ExitSystem(void);
BOOL DEBUGWIN_IsActive(void);

//--------------------------------------------------------------
//	プロック毎に行う行う初期化・開放
//--------------------------------------------------------------
void DEBUGWIN_InitProc( const u32 frmnum , GFL_FONT *fontHandle );
void DEBUGWIN_ExitProc( void );

//--------------------------------------------------------------
//	グループの処理
//--------------------------------------------------------------
void DEBUGWIN_AddGroupToTop( const u8 id , const char *nameStr , const heapId );
void DEBUGWIN_AddGroupToGroup( const u8 id , const char *nameStr , const u8 parentGroupId, const heapId );

void DEBUGWIN_RemoveGroup( const u8 id );

//--------------------------------------------------------------
//	アイテムの処理
//--------------------------------------------------------------
//アイテムをグループに追加する(名前固定
void DEBUGWIN_AddItemToGroup( const char *nameStr , DEBUGWIN_UPDATE_FUNC updateFunc , void *work , const u8 parentGroupId , const heapId );

//アイテムをグループに追加する(名前変更可
void DEBUGWIN_AddItemToGroupEx( DEBUGWIN_UPDATE_FUNC updateFunc , DEBUGWIN_DRAW_FUNC drawFunc , void *work , const u8 parentGroupId , const heapId );

//アイテムの名前を変更する(DEBUGWIN_DRAW_FUNC内で使う
void DEBUGWIN_ITEM_SetName( DEBUGWIN_ITEM* item , const char *nameStr );
void DEBUGWIN_ITEM_SetNameV( DEBUGWIN_ITEM* item , char *nameStr , ...);

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
//画面全更新
void DEBUGWIN_RefreshScreen( void );
//ゲームを1フレーム進める
void DEBUGWIN_UpdateFrame( void );

#endif //DEBUGWIN_SYS_H__
