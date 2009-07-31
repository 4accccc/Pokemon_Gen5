//======================================================================
/**
 * @file	field_msgbg.c
 * @brief	フィールド　メッセージBG関連
 * @authaor	kagaya
 * @data	2008.12.11
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "system/bmp_menu.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"

#include "arc_def.h"
#include "font/font.naix"

#include "message.naix"
#include "msg/msg_yesnomenu.h"

#include "field/field_msgbg.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

/*
extern GFL_BMPWIN * TALKMSGWIN_GetBmpWin( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx );
*/

//======================================================================
//	define
//======================================================================
#define FLDMSGBG_BGFRAME (GFL_BG_FRAME1_M)	///<使用BGフレーム

#define FLDMSGBG_PANO_TALKMSGWIN (12) ///<吹き出し会話ウィンドウパレットNo
#define FLDMSGBG_PANO_MENU (13) 			///<メニューパレットNo
#define FLDMSGBG_PANO_FONT (14)				///<フォントパレットNo

#define FLDMSGBG_PRINT_MAX (4)				///<PRINT関連要素数最大
#define FLDMSGBG_PRINT_STREAM_MAX (1) ///<PRINT_STREAM稼働数

#define FLDMSGBG_STRLEN (128)				///<文字列長さ標準

#define FLDMSGBG_CHAROFFS_TALKMSGWIN (32) ///<TALKWINMSGキャラオフセット

//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	FLDMSGPRINT
//--------------------------------------------------------------
struct _TAG_FLDMSGPRINT
{
	FLDMSGBG *fmb;
	
	BOOL printTransFlag;
	GFL_FONT *fontHandle;	//FLDMSGBGから
	PRINT_QUE *printQue;	//FLDMSGBGから
	GFL_MSGDATA *msgData;	//ユーザーから
	GFL_BMPWIN *bmpwin;		//ユーザーから
	
	STRBUF *strBuf;
	PRINT_UTIL printUtil;
};

//--------------------------------------------------------------
///	FLDMSGBG
//--------------------------------------------------------------
struct _TAG_FLDMSGBG
{
	HEAPID heapID;
	u32 bgFrame;
	
	GFL_FONT *fontHandle;
	PRINT_QUE *printQue;
	FLDMSGPRINT msgPrintTbl[FLDMSGBG_PRINT_MAX];
  
  TALKMSGWIN_SYS *talkMsgWinSys;
  
  GFL_TCBLSYS *printTCBLSys;
};

//--------------------------------------------------------------
///	FLDMSGWIN
//--------------------------------------------------------------
struct _TAG_FLDMSGWIN
{
	GFL_BMPWIN *bmpwin;		//ユーザーから
	FLDMSGPRINT *msgPrint;
	FLDMSGBG *fmb;
};

//--------------------------------------------------------------
///	FLDMENUFUNC
//--------------------------------------------------------------
struct _TAG_FLDMENUFUNC
{
	FLDMSGBG *fmb;
	FLDMSGPRINT *msgPrint;
	GFL_BMPWIN *bmpwin;
	
	BMP_MENULIST_DATA *pMenuListData;
	BMPMENULIST_WORK *pMenuListWork;
};

//--------------------------------------------------------------
/// FLDMSGPRINT_STREAM
//--------------------------------------------------------------
struct _TAG_FLDMSGPRINT_STREAM
{
  u8 flag_key_trg;
  u8 flag_key_cont;
  PRINT_STREAM *printStream;
};

//--------------------------------------------------------------
///	FLDMSGWIN_STREAM
//--------------------------------------------------------------
struct _TAG_FLDMSGWIN_STREAM
{
	GFL_BMPWIN *bmpwin;
	FLDMSGPRINT_STREAM *msgPrintStream;
  const GFL_MSGDATA *msgData; //ユーザーから
  STRBUF *strBuf;
	FLDMSGBG *fmb;
};

//======================================================================
//	proto
//======================================================================
static void FldMenuFuncH_BmpMenuListH(
	BMPMENULIST_HEADER *menuH, const FLDMENUFUNC_HEADER *fmenuH );

static GFL_BMPWIN * FldBmpWinFrame_Init( u32 bgFrame, HEAPID heapID,
	u16 pos_x, u16 pos_y, u16 size_x, u16 size_y );
static void FldBmpWinFrame_Delete( GFL_BMPWIN *bmpwin );

static const FLDMENUFUNC_HEADER DATA_MenuHeader_YesNo;

//======================================================================
//	FLDMSGBG	フィールドメッセージBG関連
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMSGBG セットアップ
 * @param	heapID	HEAPID
 * @param g3Dcamera GFL_G3D_CAMERA* 吹き出しウィンドウで使用 NULL=使用しない
 * @retval	FLDMAPBG	FLDMAPBG*
 */
//--------------------------------------------------------------
FLDMSGBG * FLDMSGBG_Setup( HEAPID heapID, GFL_G3D_CAMERA *g3Dcamera )
{
	FLDMSGBG *fmb;
	
	fmb = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDMSGBG) );
	fmb->heapID = heapID;
	fmb->bgFrame = FLDMSGBG_BGFRAME;
	
  KAGAYA_Printf( "FLDMAPBG 初期HEAPID = %d\n", fmb->heapID );
  
	{	//BG初期化
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		
		GFL_BG_SetBGControl( fmb->bgFrame, &bgcntText, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible( fmb->bgFrame, VISIBLE_ON );
		
		GFL_BG_SetPriority( fmb->bgFrame, 0 );
		GFL_BG_SetPriority( GFL_BG_FRAME0_M, 3 );
		
		GFL_BG_FillCharacter( fmb->bgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( fmb->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		
		GFL_BG_LoadScreenReq( fmb->bgFrame );
	}
	
	{	//フォントパレット
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_FONT, NARC_font_default_nclr,
			PALTYPE_MAIN_BG, FLDMSGBG_PANO_FONT*32, 32, fmb->heapID );
	}
	
	{	//window frame
		BmpWinFrame_GraphicSet( fmb->bgFrame, 1,
			FLDMSGBG_PANO_MENU, MENU_TYPE_SYSTEM, heapID );
	}
	
	{	//font
		fmb->fontHandle = GFL_FONT_Create(
			ARCID_FONT, NARC_font_large_nftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, fmb->heapID );
	}
	
	{	//print
		fmb->printQue = PRINTSYS_QUE_Create( fmb->heapID );
	}
	
  { //PRINT_STREAM TCB
    fmb->printTCBLSys = GFL_TCBL_Init(
        fmb->heapID, fmb->heapID, FLDMSGBG_PRINT_STREAM_MAX, 4 );
  }
  
  { //TALKMSGWIN
    if( g3Dcamera != NULL ){
      TALKMSGWIN_SYS_SETUP setup;
      setup.heapID = fmb->heapID;
      setup.g3Dcamera = g3Dcamera;
      setup.fontHandle = fmb->fontHandle;
      setup.chrNumOffs = FLDMSGBG_CHAROFFS_TALKMSGWIN;
      setup.ini.frameID = FLDMSGBG_BGFRAME;
      setup.ini.winPltID = FLDMSGBG_PANO_TALKMSGWIN;
      setup.ini.fontPltID = FLDMSGBG_PANO_FONT;
      fmb->talkMsgWinSys = TALKMSGWIN_SystemCreate( &setup );
    }
  }
  
  FLDMSGBG_SetBlendAlpha();
	return( fmb );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG 削除
 * @param	fmb	FLDMSGBG
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_Delete( FLDMSGBG *fmb )
{
	int i = 0;
	FLDMSGPRINT *msgPrint = fmb->msgPrintTbl;
	
  if( fmb->talkMsgWinSys != NULL ){
    TALKMSGWIN_SystemDelete( fmb->talkMsgWinSys );
  }
  
  GFL_TCBL_Exit( fmb->printTCBLSys );
  
//GFL_BG_FreeCharacterArea( fmb->bgFrame, 0x00, 0x20 );
	GFL_BG_FillCharacterRelease( fmb->bgFrame, 0x00, 0x20 );

	GFL_BG_FreeBGControl( fmb->bgFrame );
	
	do{
		#if 0
		if( msgPrint->msgData != NULL ){
			GFL_MSG_Delete( msgPrint->msgData );
		}
		#endif
		if( msgPrint->strBuf != NULL ){
			GFL_STR_DeleteBuffer( msgPrint->strBuf );
		}
		msgPrint++;
		i++;
	}while( i < FLDMSGBG_PRINT_MAX );
	
	PRINTSYS_QUE_Delete( fmb->printQue );
	GFL_FONT_Delete( fmb->fontHandle );
	GFL_HEAP_FreeMemory( fmb );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG プリント処理
 * @param	fmb	FLDMSGBG
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_PrintMain( FLDMSGBG *fmb )
{
	int i;
	FLDMSGPRINT *msgPrint = fmb->msgPrintTbl;
	
	PRINTSYS_QUE_Main( fmb->printQue );
	
	for( i = 0; i < FLDMSGBG_PRINT_MAX; i++, msgPrint++ ){
		if( msgPrint->printQue != NULL ){
			msgPrint->printTransFlag = PRINT_UTIL_Trans(
				&msgPrint->printUtil, msgPrint->printQue );
		}
	}
  
  GFL_TCBL_Main( fmb->printTCBLSys );
  
  if( fmb->talkMsgWinSys != NULL ){
    TALKMSGWIN_SystemMain( fmb->talkMsgWinSys );
    TALKMSGWIN_SystemDraw2D( fmb->talkMsgWinSys );
  }
}

//--------------------------------------------------------------
/**
 * FLDMSGBG G3D描画処理
 * @param fmb FLDMSGBG
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_PrintG3D( FLDMSGBG *fmb )
{
  if( fmb->talkMsgWinSys != NULL ){
    TALKMSGWIN_SystemDraw3D( fmb->talkMsgWinSys );
  }
}

//--------------------------------------------------------------
/**
 * FLDMSGBG プリントキューに貯まっている処理を全てクリア
 * @param	fmb	FLDMSGBG
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_ClearPrintQue( FLDMSGBG *fmb )
{
	PRINTSYS_QUE_Clear( fmb->printQue );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG プリントキューに貯まっている処理を全て実行
 * @param	fmb	FLDMSGBG
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_AllPrint( FLDMSGBG *fmb )
{
	while( FLDMSGBG_CheckFinishPrint(fmb) != TRUE ){
		FLDMSGBG_PrintMain( fmb );
	}
}

//--------------------------------------------------------------
/**
 * FLDMSGBG プリントキューの処理が全て完了しているかチェック
 * @param	fmb	FLDMSGBG
 * @retval	BOOL TRUE=完了
 */
//--------------------------------------------------------------
BOOL FLDMSGBG_CheckFinishPrint( FLDMSGBG *fmb )
{
	return( PRINTSYS_QUE_IsFinished(fmb->printQue) );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG 使用しているPRINT_QUEを返す
 * @param	fmb	FLDMSGBG
 * @retval	PRINT_QUE*
 */
//--------------------------------------------------------------
PRINT_QUE * FLDMSGBG_GetPrintQue( FLDMSGBG *fmb )
{
	return( fmb->printQue );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG 使用しているGFL_FONTを返す
 * @param	fmb	FLDMSGBG
 * @retval	GFL_FONT*
 */
//--------------------------------------------------------------
GFL_FONT * FLDMSGBG_GetFontHandle( FLDMSGBG *fmb )
{
  return( fmb->fontHandle );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG FLDMSGBGで使用しているHEAPIDでGFL_MSGDATA初期化。
 * 戻り値GFL_MSGDATAの削除は各自で行う事。
 * @param	fmb	FLDMSGBG
 * @param	arcDatIDMsg	メッセージが含まれるアーカイブデータID
 * @retval	GFL_MSGDATA*
 */
//--------------------------------------------------------------
GFL_MSGDATA * FLDMSGBG_CreateMSGDATA( FLDMSGBG *fmb, u32 arcDatIDMsg )
{
	GFL_MSGDATA *msgData;
	msgData = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, arcDatIDMsg, fmb->heapID );
	return( msgData );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG MSGDATAの削除。
 * @param msgData GFL_MSGDATA
 * @retval nothing
 * @note Create()と対になる関数が欲しいとの要望により。
 * 中身はGFL_MSGDATAを呼んでいるだけ。
 */
//--------------------------------------------------------------
void FLDMSGBG_DeleteMSGDATA( GFL_MSGDATA *msgData )
{
  GFL_MSG_Delete( msgData );
}

//======================================================================
//	FLDMSGPRINT	フィールド文字表示関連
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMSGPRINT プリント設定
 * @param	fmb	FLDMSGBG
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @param	bmpwin	表示する初期化済みのGFL_BMPWIN
 * @retval	FLDMSGPRINT*
 */
//--------------------------------------------------------------
FLDMSGPRINT * FLDMSGPRINT_SetupPrint(
	FLDMSGBG *fmb, GFL_MSGDATA *msgData, GFL_BMPWIN *bmpwin )
{
	int i = 0;
	FLDMSGPRINT *msgPrint = fmb->msgPrintTbl;
	
	do{
		if( msgPrint->printQue == NULL ){
			msgPrint->fmb = fmb;
			msgPrint->bmpwin = bmpwin;
			msgPrint->printTransFlag = TRUE;
			msgPrint->fontHandle = fmb->fontHandle;
			msgPrint->printQue = fmb->printQue;
			msgPrint->strBuf = GFL_STR_CreateBuffer(
					FLDMSGBG_STRLEN, fmb->heapID );
			#if 0
			msgPrint->msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
					ARCID_MESSAGE, arcDatIDMsg, fmb->heapID );
			#else
			msgPrint->msgData = msgData;
			#endif
			PRINT_UTIL_Setup( &msgPrint->printUtil, bmpwin );
			return( msgPrint );
		}
		msgPrint++;
		i++;
	}while( i < FLDMSGBG_PRINT_MAX );
	
	GF_ASSERT( 0 );
	return( NULL );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT プリント削除
 * @param	msgPrint	FLDMSGPRINT FLDMSGPRINT_SetupPrint()の戻り値
 * @retval	nothing 
 * FLDMSGPRINT_SetupPrint()で指定したmsgData,bmpwinの削除は各自で行う事。
 */
//--------------------------------------------------------------
void FLDMSGPRINT_Delete( FLDMSGPRINT *msgPrint )
{
	msgPrint->fontHandle = NULL;
	msgPrint->printQue = NULL;
	GFL_STR_DeleteBuffer( msgPrint->strBuf );
	msgPrint->strBuf = NULL;
	#if 0
	GFL_MSG_Delete( msgPrint->msgData );
	#endif
	msgPrint->msgData = NULL;
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT プリント
 * @param	msgPrint	FLDMSGPRINT
 * @param	x		表示X座標
 * @param	y		表示Y座標
 * @param	strID	メッセージデータ 文字列ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGPRINT_Print( FLDMSGPRINT *msgPrint, u32 x, u32 y, u32 strID )
{
	GF_ASSERT( msgPrint->msgData );
	GFL_MSG_GetString( msgPrint->msgData, strID, msgPrint->strBuf );
	PRINT_UTIL_Print( &msgPrint->printUtil, msgPrint->printQue,
		x, y, msgPrint->strBuf, msgPrint->fontHandle );		
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT プリント カラー指定有り
 * @param	msgPrint	FLDMSGPRINT
 * @param	x		表示X座標
 * @param	y		表示Y座標
 * @param	strID	メッセージデータ 文字列ID
 * @param color PRINTSYS_LSB
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGPRINT_PrintColor(
    FLDMSGPRINT *msgPrint, u32 x, u32 y, u32 strID, PRINTSYS_LSB color )
{
	GF_ASSERT( msgPrint->msgData );
	GFL_MSG_GetString( msgPrint->msgData, strID, msgPrint->strBuf );
  PRINT_UTIL_PrintColor( &msgPrint->printUtil, msgPrint->printQue,
		x, y, msgPrint->strBuf, msgPrint->fontHandle, color );		
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT プリント　STRBUF指定
 * @param	msgPrint	FLDMSGPRINT
 * @param	x		表示X座標
 * @param	y		表示Y座標
 * @param	strBuf	表示するSTRBUF
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGPRINT_PrintStrBuf(
		FLDMSGPRINT *msgPrint, u32 x, u32 y, STRBUF *strBuf )
{
	PRINT_UTIL_Print( &msgPrint->printUtil, msgPrint->printQue,
		x, y, strBuf, msgPrint->fontHandle );		
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT　プリント転送チェック
 * @param	msgPrint	FLDMSGPRINT
 * @retval	BOOL	TRUE=転送終了
 */
//--------------------------------------------------------------
BOOL FLDMSGPRINT_CheckPrintTrans( FLDMSGPRINT *msgPrint )
{
	return( msgPrint->printTransFlag );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT 表示するビットマップウィンドウを変更する
 * @param	msgPrint	FLDMSGPRINT
 * @param	bmpwin	表示する初期化済みのGFL_BMPWIN
 * @retval	nothing
 * 以前に指定したbmpwinの削除は各自で行う事。
 */
//--------------------------------------------------------------
void FLDMSGPRINT_ChangeBmpWin( FLDMSGPRINT *msgPrint, GFL_BMPWIN *bmpwin )
{
	msgPrint->bmpwin = bmpwin;
	PRINT_UTIL_Setup( &msgPrint->printUtil, bmpwin );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT ビットマップクリア
 * @param	msgPrint
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGPRINT_ClearBmp( FLDMSGPRINT *msgPrint )
{
	GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( msgPrint->bmpwin );
	GFL_BMP_Clear( bmp, 0xff );
	GFL_BG_LoadScreenReq( msgPrint->fmb->bgFrame );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT ビットマップ矩形クリア
 * @param	msgPrint
 * @param x 書き込み先書き込み開始X座標（ドット）
 * @param y 書き込み先書き込み開始Y座標（ドット）
 * @param size_x  描画範囲Xサイズ（ドット）
 * @param size_y  描画範囲Yサイズ（ドット）
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGPRINT_FillClearBmp(
    FLDMSGPRINT *msgPrint, u32 x, u32 y, u32 size_x, u32 size_y )
{
	GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( msgPrint->bmpwin );
  GFL_BMP_Fill( bmp, x, y, size_x, size_y, 0xff );
	GFL_BG_LoadScreenReq( msgPrint->fmb->bgFrame );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT　PRINT_QUE取得
 * @param	msgPrint	FLDMSGPRINT
 * @retval	PRINT_QUE*
 */
//--------------------------------------------------------------
PRINT_QUE * FLDMSGPRINT_GetPrintQue( FLDMSGPRINT *msgPrint )
{
	return( msgPrint->printQue );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT　GFL_FONT取得
 * @param	msgPrint	FLDMSGPRINT
 * @retval	GFL_FONT*
 */
//--------------------------------------------------------------
GFL_FONT * FLDMSGPRINT_GetFontHandle( FLDMSGPRINT *msgPrint )
{
	return( msgPrint->fontHandle );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT　PRINT_UTIL取得
 * @param	msgPrint	FLDMSGPRINT
 * @retval	PRINT_UTIL*
 */
//--------------------------------------------------------------
PRINT_UTIL * FLDMSGPRINT_GetPrintUtil( FLDMSGPRINT *msgPrint )
{
	return( &msgPrint->printUtil );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT　STRBUF取得
 * @param	msgPrint	FLDMSGPRINT
 * @retval	STRBUF*
 */
//--------------------------------------------------------------
STRBUF * FLDMSGPRINT_GetStrBuf( FLDMSGPRINT *msgPrint )
{
	return( msgPrint->strBuf );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT MSGDATA取得
 * @param	msgPrint	FLDMSGPRINT
 * @retval	MSGDATA*
 */
//--------------------------------------------------------------
GFL_MSGDATA * FLDMSGPRINT_GetMsgData( FLDMSGPRINT *msgPrint )
{
	return( msgPrint->msgData );
}

//======================================================================
//	FLDMSGWIN	メッセージウィンドウ関連
//======================================================================
//--------------------------------------------------------------
/**
 * メッセージウィンドウ　追加
 * @param	fmb	FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @param	bmppos_x		//表示座標X キャラ単位
 * @param	bmppos_y		//表示座標Y キャラ単位
 * @param	bmpsize_x		//表示サイズX キャラ単位
 * @param	bmpsize_y		//表示サイズY キャラ単位
 * @retval	FLDMSGWIN*
 */
//--------------------------------------------------------------
FLDMSGWIN * FLDMSGWIN_Add( FLDMSGBG *fmb, GFL_MSGDATA *msgData,
	u16 bmppos_x, u16 bmppos_y, u16 bmpsize_x, u16 bmpsize_y )
{
	FLDMSGWIN *msgWin;
	
	msgWin = GFL_HEAP_AllocClearMemory( fmb->heapID, sizeof(FLDMSGWIN) );
	msgWin->fmb = fmb;
	msgWin->bmpwin = FldBmpWinFrame_Init(
		fmb->bgFrame, fmb->heapID, bmppos_x, bmppos_y, bmpsize_x, bmpsize_y );
	msgWin->msgPrint = FLDMSGPRINT_SetupPrint( fmb, msgData, msgWin->bmpwin );
	
  FLDMSGBG_SetBlendAlpha();
	return( msgWin );
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ 削除。
 * FLDMSGWIN_Add()引数msgDataは各自で行う事。
 * @param	msgWin	FLDMSGWIN*
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_Delete( FLDMSGWIN *msgWin )
{
	FldBmpWinFrame_Delete( msgWin->bmpwin );
	FLDMSGPRINT_Delete( msgWin->msgPrint );
	GFL_HEAP_FreeMemory( msgWin );
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ　メッセージ表示
 * @param	msgWin	FLDMSGWIN
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param	strID	メッセージデータ 文字列ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_Print( FLDMSGWIN *msgWin, u16 x, u16 y, u32 strID )
{
	FLDMSGPRINT_Print( msgWin->msgPrint, x, y, strID );
	GFL_BG_LoadScreenReq( FLDMSGBG_BGFRAME );
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ　メッセージ表示 STRBUF指定
 * @param	msgWin	FLDMSGWIN
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param	strBuf	STRBUF
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_PrintStrBuf( FLDMSGWIN *msgWin, u16 x, u16 y, STRBUF *strBuf )
{
	FLDMSGPRINT_PrintStrBuf( msgWin->msgPrint, x, y, strBuf );
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ　転送終了チェック
 * @param	msgWin	FLDMSGWIN
 * @retval	BOOL	TRUE=転送終了
 */
//--------------------------------------------------------------
BOOL FLDMSGWIN_CheckPrintTrans( FLDMSGWIN *msgWin )
{
	return( FLDMSGPRINT_CheckPrintTrans(msgWin->msgPrint) );
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ　メッセージ表示クリア
 * @param	msgWin	FLDMSGWIN
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_ClearWindow( FLDMSGWIN *msgWin )
{
	FLDMSGPRINT_ClearBmp( msgWin->msgPrint );
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ　メッセージ指定領域クリア
 * @param msgWin FLDMSGWIN
 * @param x 書き込み先書き込み開始X座標（ドット）
 * @param y 書き込み先書き込み開始Y座標（ドット）
 * @param size_x  描画範囲Xサイズ（ドット）
 * @param size_y  描画範囲Yサイズ（ドット）
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_FillClearWindow(
    FLDMSGWIN *msgWin, u32 x, u32 y, u32 size_x, u32 size_y )
{
  FLDMSGPRINT_FillClearBmp( msgWin->msgPrint, x, y, size_x, size_y );
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ　追加　会話ウィンドウタイプ
 * @param	fmb	FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @retval	FLDMSGWIN*
 */
//--------------------------------------------------------------
FLDMSGWIN * FLDMSGWIN_AddTalkWin( FLDMSGBG *fmb, GFL_MSGDATA *msgData )
{
	return( FLDMSGWIN_Add(fmb,msgData,1,19,30,4) );
}

//======================================================================
//	FLDMENUFUNC　フィールドメニュー関連
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMENUFUNC メニュー追加
 * @param	fmb	FLDMSGBG
 * @param	pMenuHead FLDMENUFUNC_HEADER
 * @param	pMenuListData  FLDMENUFUNC_LISTDATA* Delete時に自動開放される
 * @param list_pos リスト初期位置
 * @param cursor_pos カーソル初期位置
 * @retval	FLDMENUFUNC*
 */
//--------------------------------------------------------------
FLDMENUFUNC * FLDMENUFUNC_AddMenuList( FLDMSGBG *fmb,
	const FLDMENUFUNC_HEADER *pMenuHead,
	FLDMENUFUNC_LISTDATA *pMenuListData,
  u16 list_pos, u16 cursor_pos )
{
	FLDMENUFUNC *menuFunc;
	BMPMENULIST_HEADER menuH;
	
	menuFunc = GFL_HEAP_AllocClearMemory( fmb->heapID, sizeof(FLDMENUFUNC) );
	FldMenuFuncH_BmpMenuListH( &menuH, pMenuHead );
	
	menuFunc->fmb = fmb;
	menuFunc->pMenuListData = (BMP_MENULIST_DATA *)pMenuListData;
	
	menuFunc->bmpwin = FldBmpWinFrame_Init(
		fmb->bgFrame, fmb->heapID,
		pMenuHead->bmppos_x, pMenuHead->bmppos_y,
		pMenuHead->bmpsize_x, pMenuHead->bmpsize_y );
	
	menuFunc->msgPrint = FLDMSGPRINT_SetupPrint(
			fmb, NULL, menuFunc->bmpwin );
	
	menuH.msgdata = NULL;
	menuH.print_util = FLDMSGPRINT_GetPrintUtil( menuFunc->msgPrint );
	menuH.print_que = FLDMSGPRINT_GetPrintQue( menuFunc->msgPrint );
	menuH.font_handle = fmb->fontHandle;
	menuH.win = menuFunc->bmpwin;
	menuH.list = (const BMP_MENULIST_DATA *)pMenuListData;
	
	menuFunc->pMenuListWork =
		BmpMenuList_Set( &menuH, list_pos, cursor_pos, fmb->heapID );
//	BmpMenuList_SetCursorString( menuFunc->pMenuListWork, 0 );
	BmpMenuList_SetCursorBmp( menuFunc->pMenuListWork, fmb->heapID );
  
  FLDMSGBG_SetBlendAlpha();
	return( menuFunc );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC メニュー追加　リスト位置、カーソル位置省略版
 * @param	fmb	FLDMSGBG
 * @param	pMenuHead FLDMENUFUNC_HEADER
 * @param	pMenuListData  FLDMENUFUNC_LISTDATA* Delete時に自動開放される
 * @retval	FLDMENUFUNC*
 */
//--------------------------------------------------------------
FLDMENUFUNC * FLDMENUFUNC_AddMenu( FLDMSGBG *fmb,
	const FLDMENUFUNC_HEADER *pMenuHead,
	FLDMENUFUNC_LISTDATA *pMenuListData )
{
  return( FLDMENUFUNC_AddMenuList(fmb,pMenuHead,pMenuListData,0,0) );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC	削除
 * @param	menuFunc	FLDMENUFUNC*
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMENUFUNC_DeleteMenu( FLDMENUFUNC *menuFunc )
{
	BmpMenuList_Exit( menuFunc->pMenuListWork, NULL, NULL );
	BmpMenuWork_ListDelete( menuFunc->pMenuListData );
	FldBmpWinFrame_Delete( menuFunc->bmpwin );
	FLDMSGPRINT_Delete( menuFunc->msgPrint );
	GFL_HEAP_FreeMemory( menuFunc );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC メニュー処理
 * @param	menuFunc	FLDMENUFUNC*
 * @retval	u32		リスト選択位置のパラメータ
 * FLDMENUFUNC_NULL = 選択中。FLDMENUFUNC_CANCEL = キャンセル
 */
//--------------------------------------------------------------
u32 FLDMENUFUNC_ProcMenu( FLDMENUFUNC *menuFunc )
{
	u32 ret = BmpMenuList_Main( menuFunc->pMenuListWork );
	
	if( ret == BMPMENULIST_NULL ){
		return( FLDMENUFUNC_NULL );
	}
	
	if( ret == BMPMENULIST_CANCEL ){
		return( FLDMENUFUNC_CANCEL );
	}
	
	return( ret );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAを作成
 * @param	max	リスト数
 * @param	heapID	作成用ヒープID
 * @retval	FLDMENUFUNC_LISTDATA*
 */
//--------------------------------------------------------------
FLDMENUFUNC_LISTDATA * FLDMENUFUNC_CreateListData( int max, HEAPID heapID )
{
	BMP_MENULIST_DATA *listData;
	listData = BmpMenuWork_ListCreate( max, heapID );
	return( (FLDMENUFUNC_LISTDATA*)listData );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAを削除
 * @param	listData	FLDMENUFUNC_LISTDATA
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMENUFUNC_DeleteListData( FLDMENUFUNC_LISTDATA *listData )
{
	BmpMenuWork_ListDelete( (BMP_MENULIST_DATA*)listData );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAを作成し
 * FLDMENUFUNC_LISTからメニュー表示用文字列を追加する
 * @param	menuList	FLDMENUFUNC_LIST
 * @param	max			menuList要素数
 * @param	msgData		文字列を
 * @param	heapID		FLDMENUFUNC_LISTDATA作成用ヒープID
 * @retval	FLDMENUFUNC_LISTDATA
 */
//--------------------------------------------------------------
FLDMENUFUNC_LISTDATA * FLDMENUFUNC_CreateMakeListData(
	const FLDMENUFUNC_LIST *menuList,
	int max, GFL_MSGDATA *msgData, HEAPID heapID )
{
	int i;
	FLDMENUFUNC_LISTDATA *listData;
	
	listData = FLDMENUFUNC_CreateListData( max, heapID );
	
	for( i = 0; i < max; i++ ){
		BmpMenuWork_ListAddArchiveString(
			(BMP_MENULIST_DATA*)listData, msgData,
			menuList[i].str_id, (u32)menuList[i].selectParam, heapID );
	}
	
	return( listData );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAに文字列&パラメータをセット
 * @param	listData	FLDMENUFUNC_LISTDATA
 * @param	strBuf		STRBUF
 * @param	param		パラメタ
 * @param	heapID		ヒープID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMENUFUNC_AddStringListData( FLDMENUFUNC_LISTDATA *listData,
		const STRBUF *strBuf, u32 param, HEAPID heapID  )
{	
	BmpMenuWork_ListAddString(
		(BMP_MENULIST_DATA*)listData, strBuf, param, heapID );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAに文字列&パラメータをセット
 * @param	listData	FLDMENUFUNC_LISTDATA
 * @param	strBuf		STRBUF
 * @param	param		パラメタ
 * @param	heapID		ヒープID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMENUFUNC_AddArcStringListData( FLDMENUFUNC_LISTDATA *listData,
		GFL_MSGDATA *msgData, u32 strID, u32 param, HEAPID heapID )
{	
	BmpMenuWork_ListAddArchiveString(
		(BMP_MENULIST_DATA*)listData, msgData, strID, param, heapID );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAに格納されているリストの最大文字数を取得
 * @param listData FLDMENUFUNC_LISTDATA
 * @retval u32 リスト内、最大文字数
 */
//--------------------------------------------------------------
u32 FLDMENUFUNC_GetListLengthMax( const FLDMENUFUNC_LISTDATA *listData )
{
  u32 len;
  len = BmpMenuWork_GetListMaxLength( (const BMP_MENULIST_DATA*)listData );
  return( len );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAに格納されているリスト数を取得
 * @param listData FLDMENUFUNC_LISTDATA
 * @retval u32 リスト内、最大文字数
 */
//--------------------------------------------------------------
u32 FLDMENUFUNC_GetListMax( const FLDMENUFUNC_LISTDATA *listData )
{
  u32 len;
  len = BmpMenuWork_GetListMax( (const BMP_MENULIST_DATA*)listData );
  return( len );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAに格納されているリストの最大文字数から
 * 必要なメニュー横幅を取得。
 * @param listData FLDMENUFUNC_LISTDATA
 * @param font_size フォント横サイズ
 * @retval u32 メニュー横幅 キャラ単位
 */
//--------------------------------------------------------------
u32 FLDMENUFUNC_GetListMenuWidth(
    const FLDMENUFUNC_LISTDATA *listData, u32 font_size )
{
  u32 c,len = FLDMENUFUNC_GetListLengthMax( listData );
  len *= font_size;
  c = len / 8;
  if( (len & 0x07) ){ c++; }
  return( c );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAに格納されているリスト数から
 * 必要なメニュー縦幅を取得。
 * @param listData FLDMENUFUNC_LISTDATA
 * @param font_size フォント横サイズ
 * @retval u32 メニュー縦幅 キャラ単位
 */
//--------------------------------------------------------------
u32 FLDMENUFUNC_GetListMenuHeight(
    const FLDMENUFUNC_LISTDATA *listData, u32 font_size )
{
  u32 c,len = FLDMENUFUNC_GetListMax( listData );
  len *= font_size;
  c = len / 8;
  if( (len & 0x07) ){ c++; }
  return( c );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_HEADERにリスト数、座標系情報を追加
 * @param	head	FLDMENUFUNC_HEADER
 * @param	list_count	リスト数
 * @param	bmppos_x		//表示座標X キャラ単位
 * @param	bmppos_y		//表示座標Y キャラ単位
 * @param	bmpsize_x		//表示サイズX キャラ単位
 * @param	bmpsize_y		//表示サイズY キャラ単位
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMENUFUNC_InputHeaderListSize(
	FLDMENUFUNC_HEADER *head, u16 count,
	u16 bmppos_x, u16 bmppos_y, u16 bmpsize_x, u16 bmpsize_y )
{
	head->count = count;
	head->bmppos_x = bmppos_x;
	head->bmppos_y = bmppos_y;
	head->bmpsize_x = bmpsize_x;
	head->bmpsize_y = bmpsize_y;
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_HEADER -> BMPMENULIST_HEADER
 * @param	menuH	BMPMENULIST_HEADER
 * @param	fmenuH	FLDMENUFUNC_HEADER
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMenuFuncH_BmpMenuListH(
	BMPMENULIST_HEADER *menuH, const FLDMENUFUNC_HEADER *fmenuH )
{
	GFL_STD_MemClear( menuH, sizeof(BMPMENULIST_HEADER) );
	menuH->count = fmenuH->count;			//リスト項目数
	menuH->line = fmenuH->line;				//表示最大項目数
	menuH->rabel_x = fmenuH->rabel_x;		//ラベル表示Ｘ座標
	menuH->data_x = fmenuH->data_x;			//項目表示Ｘ座標
	menuH->cursor_x = fmenuH->cursor_x;		//カーソル表示Ｘ座標
	menuH->line_y = fmenuH->line_y;			//表示Ｙ座標
	menuH->f_col = fmenuH->f_col;			//表示文字色
	menuH->b_col = fmenuH->b_col;			//表示背景色
	menuH->s_col = fmenuH->s_col;			//表示文字影色
	menuH->msg_spc = fmenuH->msg_spc;		//文字間隔Ｘ
	menuH->line_spc = fmenuH->line_spc;		//文字間隔Ｙ
	menuH->page_skip = fmenuH->page_skip;	//ページスキップタイプ
	menuH->font_size_x = fmenuH->font_size_x;	//文字サイズX(ドット
	menuH->font_size_y = fmenuH->font_size_y;	//文字サイズY(ドット
}

//======================================================================
//  はい、いいえ選択メニュー
//======================================================================
//--------------------------------------------------------------
/**
 * はい、いいえ選択メニュー　追加
 * @param fmb FLDMSGBG
 * @param pos FLDMENUFUNC_YESNO カーソル初期位置
 * @retval FLDMENUFUNC*
 * メニュー削除の際はFLDMENUFUNC_DeleteMenu()を呼ぶ事。
 */
//--------------------------------------------------------------
FLDMENUFUNC * FLDMENUFUNC_AddYesNoMenu(
    FLDMSGBG *fmb, FLDMENUFUNC_YESNO cursor_pos )
{
  u32 max = 2;
	GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
	FLDMENUFUNC_LISTDATA *listData;
  FLDMENUFUNC_HEADER menuH = DATA_MenuHeader_YesNo;
  const FLDMENUFUNC_LIST menuList[2] = {
    { msgid_yesno_yes, (void*)0 }, { msgid_yesno_no, (void*)1 }, };
  
  KAGAYA_Printf( "fmb HEAPID = %d\n", fmb->heapID );

  msgData = FLDMSGBG_CreateMSGDATA( fmb, NARC_message_yesnomenu_dat );
  listData = FLDMENUFUNC_CreateMakeListData(
            menuList, max, msgData, fmb->heapID );
  GFL_MSG_Delete( msgData );
  
	FLDMENUFUNC_InputHeaderListSize( &menuH, max, 24, 10, 7, 4 );
  menuFunc = FLDMENUFUNC_AddMenuList( fmb, &menuH, listData, 0, cursor_pos );
  return( menuFunc );
}

//--------------------------------------------------------------
/**
 * はい、いいえ選択メニュー 動作
 * @param menuFunc FLDMENUFUNC*
 * @retval u32 FLDMENUFUNC_YESNO
 */
//--------------------------------------------------------------
FLDMENUFUNC_YESNO FLDMENUFUNC_ProcYesNoMenu( FLDMENUFUNC *menuFunc )
{
  u32 ret = FLDMENUFUNC_ProcMenu( menuFunc );
  
  if( ret == FLDMENUFUNC_NULL ){
    return( FLDMENUFUNC_YESNO_NULL );
  }
  
  if( ret == FLDMENUFUNC_CANCEL ){
    return( FLDMENUFUNC_YESNO_NO );
  }
  
  return( ret );
}

//======================================================================
//  FLDMSGPRINT_STREAM メッセージ　プリントストリーム関連
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMSGPRINT_STREAM プリント設定
 * @param fmb FLDMSGBG
 * @param strbuf 表示するSTRBUF
 * @param bmpwin 表示する初期化済みのGFL_BMPWIN
 * @param x 描画開始X座標(ドット)
 * @param y 描画開始Y座標(ドット)
 * @param   wait		１文字描画ごとのウェイトフレーム数
 * @retval FLDMSGPRINT_STREAM
 */
//--------------------------------------------------------------
FLDMSGPRINT_STREAM * FLDMSGPRINT_STREAM_SetupPrint(
	FLDMSGBG *fmb, const STRBUF *strbuf,
  GFL_BMPWIN *bmpwin, u16 x, u16 y, int wait )
{
  FLDMSGPRINT_STREAM *stm = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDMSGPRINT_STREAM) );
  
  stm->printStream = PRINTSYS_PrintStream(
      bmpwin, x, y,
      strbuf, fmb->fontHandle,
      wait,
      fmb->printTCBLSys, 0,
      fmb->heapID, 0x0f );

  return( stm );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT_STREAM 削除
 * @param stm FLDMSGPRINT_STREAM*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGPRINT_STREAM_Delete( FLDMSGPRINT_STREAM *stm )
{
  PRINTSYS_PrintStreamDelete( stm->printStream );
  GFL_HEAP_FreeMemory( stm );
}

//--------------------------------------------------------------
/**
 * FLDMSGPRINT_STREAM プリント
 * @param stm 
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
BOOL FLDMSGPRINT_STREAM_ProcPrint( FLDMSGPRINT_STREAM *stm )
{
  int trg,cont;
  PRINTSTREAM_STATE state;
  
  trg = GFL_UI_KEY_GetTrg();
  cont = GFL_UI_KEY_GetCont();
  state = PRINTSYS_PrintStreamGetState( stm->printStream );
  
  switch( state ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    if( (trg & PAD_BUTTON_A) ){
      stm->flag_key_trg = TRUE;
    }
    
    if( stm->flag_key_trg == TRUE && (cont & PAD_BUTTON_A) ){
      PRINTSYS_PrintStreamShortWait( stm->printStream, 0 );
    }else{
      PRINTSYS_PrintStreamShortWait( stm->printStream, 2 );
    }
    break;
  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( (trg & PAD_BUTTON_A) ){
      PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
      PRINTSYS_PrintStreamReleasePause( stm->printStream );
      stm->flag_key_trg = FALSE;
    }
    break;
  case PRINTSTREAM_STATE_DONE: //終了
    return( TRUE );
  }
  
  return( FALSE );
}

//======================================================================
//  FLDMSGWIN_STREAM
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ追加
 * @param	fmb	FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @param	bmppos_x		//表示座標X キャラ単位
 * @param	bmppos_y		//表示座標Y キャラ単位
 * @param	bmpsize_x		//表示サイズX キャラ単位
 * @param	bmpsize_y		//表示サイズY キャラ単位
 * @retval FLDMSGWIN_STREAM*
 */
//--------------------------------------------------------------
FLDMSGWIN_STREAM * FLDMSGWIN_STREAM_Add(
    FLDMSGBG *fmb, const GFL_MSGDATA *msgData,
    u16 bmppos_x, u16 bmppos_y, u16 bmpsize_x, u16 bmpsize_y )
{
  FLDMSGWIN_STREAM *msgWin;
	
	msgWin = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDMSGWIN_STREAM) );
	msgWin->fmb = fmb;
  msgWin->msgData = msgData;
	msgWin->bmpwin = FldBmpWinFrame_Init(
		fmb->bgFrame, fmb->heapID, bmppos_x, bmppos_y, bmpsize_x, bmpsize_y );
  msgWin->strBuf = GFL_STR_CreateBuffer( FLDMSGBG_STRLEN, fmb->heapID );
  
  FLDMSGBG_SetBlendAlpha();
  return( msgWin );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ 削除
 * FLDMSGWIN_STREAM_Add()引数msgDataは各自で行う事。
 * @param msgWin FLDMSGWIN_STREAM
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_STREAM_Delete( FLDMSGWIN_STREAM *msgWin )
{
	FldBmpWinFrame_Delete( msgWin->bmpwin );
  
  if( msgWin->msgPrintStream != NULL ){
	  FLDMSGPRINT_STREAM_Delete( msgWin->msgPrintStream );
  }
  
  GFL_STR_DeleteBuffer( msgWin->strBuf );
	GFL_HEAP_FreeMemory( msgWin );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ メッセージ表示開始
 * @param msgWin FLDMSGWIN_STREAM*
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param strID メッセージデータ　文字列ID
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_STREAM_PrintStart(
    FLDMSGWIN_STREAM *msgWin, u16 x, u16 y, u32 strID )
{
  if( msgWin->msgPrintStream != NULL ){
    FLDMSGPRINT_STREAM_Delete( msgWin->msgPrintStream );
  }
  
  GF_ASSERT( msgWin->msgData );
  GFL_MSG_GetString( msgWin->msgData, strID, msgWin->strBuf );
  
  msgWin->msgPrintStream = FLDMSGPRINT_STREAM_SetupPrint(
  	msgWin->fmb, msgWin->strBuf, msgWin->bmpwin, x, y, 4 );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ メッセージ表示開始 STRBUF指定
 * @param msgWin FLDMSGWIN_STREAM*
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param strBuf 表示するSTRBUF
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_STREAM_PrintStrBufStart(
    FLDMSGWIN_STREAM *msgWin, u16 x, u16 y, const STRBUF *strBuf )
{
  if( msgWin->msgPrintStream != NULL ){
    FLDMSGPRINT_STREAM_Delete( msgWin->msgPrintStream );
  }
  
  msgWin->msgPrintStream = FLDMSGPRINT_STREAM_SetupPrint(
  	msgWin->fmb, strBuf, msgWin->bmpwin, x, y, 2 );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ メッセージ表示
 * @param msgWin FLDMSGWIN_STREAM*
 * @retval BOOL TRUE=表示終了,FALSE=表示中
 */
//--------------------------------------------------------------
BOOL FLDMSGWIN_STREAM_Print( FLDMSGWIN_STREAM *msgWin )
{
  if( FLDMSGPRINT_STREAM_ProcPrint(msgWin->msgPrintStream) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ追加 会話ウィンドウタイプ
 * @param fmb FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @retval FLDMSGWIN_STREAM
 */
//--------------------------------------------------------------
FLDMSGWIN_STREAM * FLDMSGWIN_STREAM_AddTalkWin(
    FLDMSGBG *fmb, GFL_MSGDATA *msgData )
{
  return( FLDMSGWIN_STREAM_Add(fmb,msgData,1,19,30,4) );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ メッセージクリア
 * @param msgWin FLDMSGWIN_STREAM
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_STREAM_ClearMessage( FLDMSGWIN_STREAM *msgWin )
{
  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( msgWin->bmpwin );
  GFL_BMP_Clear( bmp, 0xff);
	GFL_BG_LoadScreenReq( msgWin->fmb->bgFrame );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ ウィンドウクリア
 * @param msgWin FLDMSGWIN_STREAM
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_STREAM_ClearWindow( FLDMSGWIN_STREAM *msgWin )
{
  BmpWinFrame_Clear( msgWin->bmpwin, 0 );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ ウィンドウ描画
 * @param msgWin FLDMSGWIN_STREAM
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_STREAM_WriteWindow( FLDMSGWIN_STREAM *msgWin )
{
	BmpWinFrame_Write( msgWin->bmpwin,
      WINDOW_TRANS_ON, 1, FLDMSGBG_PANO_MENU );
  FLDMSGWIN_STREAM_ClearMessage( msgWin );
}

//======================================================================
//  FLDTALKMSGWIN
//======================================================================
//--------------------------------------------------------------
/// FLDTALKMSGWIN
//--------------------------------------------------------------
struct _TAG_FLDTALKMSGWIN
{
  u8 flag_key_trg;
  u8 flag_key_cont;
  STRBUF *strBuf;
  int talkMsgWinIdx;
  TALKMSGWIN_SYS *talkMsgWinSys; //FLDMSGBGより
  
  s16 cursor_anm_no;
  s16 cursor_anm_frame;
  u16 cursor_put_flag;
  u16 cursor_clear_flag;
  GFL_BMP_DATA *cursor_bmp;
};

#if 0
const u8 ALIGN4 skip_cursor_Character[128] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x44,0x44,0x44,0x44,
0x42,0x44,0x44,0x44,0x20,0x44,0x44,0x24, 0x00,0x42,0x44,0x22,0x00,0x20,0x24,0x02,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x24,0x00,0x00,0x00,
0x22,0x00,0x00,0x00,0x02,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x22,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
#else
const u8 ALIGN4 skip_cursor_Character[128] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x22,0x22,0x22,0x22,
0x21,0x22,0x22,0x22,0x10,0x22,0x22,0x12, 0x00,0x21,0x22,0x11,0x00,0x10,0x12,0x01,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x12,0x00,0x00,0x00,
0x11,0x00,0x00,0x00,0x01,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x11,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
#endif

//--------------------------------------------------------------
/**
 * FLDTALKMSGWIN 吹き出しウィンドウセット
 * @param fmb FLDMSGBG
 * @param idx FLDTALKMSGWIN_IDX
 * @param strBuf 表示するSTRBUF
 * @param pos 吹き出し元座標
 * @retval FLDTALKMSGWIN*
 * @note 引数pos,strBufは表示中、常に参照されるので
 * 削除されるまで保持して下さい
 */
//--------------------------------------------------------------
static void fldTalkMsgWin_Add(
    FLDMSGBG *fmb, FLDTALKMSGWIN *tmsg,
    FLDTALKMSGWIN_IDX idx, const VecFx32 *pos, STRBUF *strBuf )
{
  GF_ASSERT( fmb->talkMsgWinSys != NULL );
  
  tmsg->talkMsgWinSys = fmb->talkMsgWinSys;
  tmsg->talkMsgWinIdx = idx;
  
  { //キー送りカーソル
#if 0
	  void * arc;
	  NNSG2dCharacterData *dat;
    
    arc = GFL_ARC_UTIL_Load( ARCID_PL_KEYCURSOR, 0, 0, fmb->heapID );
    GF_ASSERT( arc != NULL );
		NNS_G2dGetUnpackedBGCharacterData( arc, &dat );
       
    tmsg->cursor_bmp = GFL_BMP_CreateWithData(
        (u8*)dat->pRawData, 2, 2, GFL_BMP_16_COLOR, fmb->heapID );
    GFL_HEAP_FreeMemory( arc );
#else
    tmsg->cursor_bmp = GFL_BMP_CreateWithData(
        (u8*)skip_cursor_Character,
        2, 2, GFL_BMP_16_COLOR, fmb->heapID );
#endif
  }

#if 0 
  TALKMSGWIN_CreateFloatWindowIdx(
      tmsg->talkMsgWinSys,
      0,
      (VecFx32*)pos, strBuf,
      1, 19,
      30, 4,
      15 );
#else
  KAGAYA_Printf( "ウィンドウ %d,%d,%d\n", pos->x, pos->y, pos->z );
  
  FLDMSGBG_SetBlendAlpha();

  switch( idx ){
  case FLDTALKMSGWIN_IDX_UPPER:
    TALKMSGWIN_CreateFixWindowUpper( tmsg->talkMsgWinSys,
        FLDTALKMSGWIN_IDX_UPPER, (VecFx32*)pos, strBuf, 15 );
  case FLDTALKMSGWIN_IDX_LOWER:
    TALKMSGWIN_CreateFixWindowLower( tmsg->talkMsgWinSys,
        FLDTALKMSGWIN_IDX_LOWER, (VecFx32*)pos, strBuf, 15 );
    break;
  default:
    TALKMSGWIN_CreateFixWindowAuto( tmsg->talkMsgWinSys,
        FLDTALKMSGWIN_IDX_AUTO, (VecFx32*)pos, strBuf, 15 );
  }
  
  TALKMSGWIN_OpenWindow( tmsg->talkMsgWinSys, tmsg->talkMsgWinIdx );
#endif
}

//--------------------------------------------------------------
/**
 * FLDTALKMSGWIN 吹き出しウィンドウセット
 * @param fmb FLDMSGBG
 * @param idx FLDTALKMSGWIN_TYPE
 * @param pos 吹き出し元座標
 * @param msgData 使用するGFL_MSGDATA
 * @param msgID メッセージ アーカイブデータID
 * @retval FLDTALKMSGWIN*
 * @note 引数pos,strBufは表示中、常に参照されるので
 * 削除されるまで保持して下さい
 */
//--------------------------------------------------------------
FLDTALKMSGWIN * FLDTALKMSGWIN_Add( FLDMSGBG *fmb,
    FLDTALKMSGWIN_IDX idx, const VecFx32 *pos,
    const GFL_MSGDATA *msgData, u32 msgID )
{
  FLDTALKMSGWIN *tmsg = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDTALKMSGWIN) );
  tmsg->strBuf = GFL_STR_CreateBuffer(
					FLDMSGBG_STRLEN, fmb->heapID );
  GFL_MSG_GetString( msgData, msgID, tmsg->strBuf );
  fldTalkMsgWin_Add( fmb, tmsg, idx, pos, tmsg->strBuf );
  return( tmsg );
}

//--------------------------------------------------------------
/**
 * FLDTALKMSGWIN 吹き出しウィンドウセット
 * @param fmb FLDMSGBG
 * @param idx FLDTALKMSGWIN_TYPE
 * @param strBuf 表示するSTRBUF
 * @param pos 吹き出し元座標
 * @retval FLDTALKMSGWIN*
 * @note 引数pos,strBufは表示中、常に参照されるので
 * 削除されるまで保持して下さい
 */
//--------------------------------------------------------------
FLDTALKMSGWIN * FLDTALKMSGWIN_AddStrBuf( FLDMSGBG *fmb,
    FLDTALKMSGWIN_IDX idx, const VecFx32 *pos, STRBUF *strBuf )
{
  FLDTALKMSGWIN *tmsg = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDTALKMSGWIN) );
  fldTalkMsgWin_Add( fmb, tmsg, idx, pos, strBuf );
  return( tmsg );
}

//--------------------------------------------------------------
/**
 * FLDTALKMSGWIN 吹き出しウィンドウ削除
 * @param tmsg FLDTALKMSGWIN
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDTALKMSGWIN_Delete( FLDTALKMSGWIN *tmsg )
{
  if( tmsg->strBuf != NULL ){
		GFL_STR_DeleteBuffer( tmsg->strBuf );
  }
  
  TALKMSGWIN_DeleteWindow( tmsg->talkMsgWinSys, tmsg->talkMsgWinIdx );
  
  { //キー送りカーソル
		GFL_BMP_Delete( tmsg->cursor_bmp );
  }

  GFL_HEAP_FreeMemory( tmsg );
}

//--------------------------------------------------------------
/**
 * FLDTALKMSGWIN 吹き出しウィンドウ 表示
 * @param tmsg FLDTALKMSGWIN
 * @retval BOOL TRUE=表示終了,FALSE=表示中
 */
//--------------------------------------------------------------
BOOL FLDTALKMSGWIN_Print( FLDTALKMSGWIN *tmsg )
{
  int trg,cont;
  PRINTSTREAM_STATE state;
  PRINT_STREAM *stream;
  PRINTSTREAM_PAUSE_TYPE pause_type;

  if( TALKMSGWIN_CheckPrintOn(
        tmsg->talkMsgWinSys,tmsg->talkMsgWinIdx) == FALSE ){
    return( FALSE );
  }

  trg = GFL_UI_KEY_GetTrg();
  cont = GFL_UI_KEY_GetCont();
  stream = TALKMSGWIN_GetPrintStream(
      tmsg->talkMsgWinSys, tmsg->talkMsgWinIdx );
  state = PRINTSYS_PrintStreamGetState( stream );
  
  switch( state ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    tmsg->cursor_clear_flag = FALSE;

    if( (trg & PAD_BUTTON_A) ){
      tmsg->flag_key_trg = TRUE;
    }
    
    if( tmsg->flag_key_trg == TRUE && (cont & PAD_BUTTON_A) ){
      PRINTSYS_PrintStreamShortWait( stream, 0 );
    }else{
      PRINTSYS_PrintStreamShortWait( stream, 2 );
    }
    break;
  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( (trg & PAD_BUTTON_A) ){
      PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
      PRINTSYS_PrintStreamReleasePause( stream );
      tmsg->flag_key_trg = FALSE;
      
      if( tmsg->cursor_put_flag == TRUE ){
        u16 x,y,offs;
        u16 tbl[3] = { 0, 1, 2 };
        GFL_BMPWIN *twin_bmp = TALKMSGWIN_GetBmpWin(
            tmsg->talkMsgWinSys,tmsg->talkMsgWinIdx );
		    GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( twin_bmp );
        
        x = GFL_BMP_GetSizeX( bmp ) - 11;
        y = GFL_BMP_GetSizeY( bmp ) - 9;
        offs = tbl[tmsg->cursor_anm_no];

        GFL_BMP_Fill( bmp, x, y+offs, 10, 7, 0x0f );
		    GFL_BMPWIN_TransVramCharacter( twin_bmp );
        
        tmsg->cursor_anm_no = 0;
        tmsg->cursor_anm_frame = 0;
        tmsg->cursor_put_flag = FALSE;
        tmsg->cursor_clear_flag = TRUE;
      }
    }
    
    pause_type = PRINTSYS_PrintStreamGetPauseType( stream );
    
//    if( pause_type == PRINTSTREAM_PAUSE_LINEFEED ){
    if( tmsg->cursor_clear_flag == FALSE ){
      u16 x,y,offs;
      u16 tbl[3] = { 0, 1, 2 };
      GFL_BMPWIN *twin_bmp = TALKMSGWIN_GetBmpWin(
          tmsg->talkMsgWinSys,tmsg->talkMsgWinIdx );
		  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( twin_bmp );
      
      x = GFL_BMP_GetSizeX( bmp ) - 11;
      y = GFL_BMP_GetSizeY( bmp ) - 9;
      offs = tbl[tmsg->cursor_anm_no];
      
      GFL_BMP_Fill( bmp, x, y+offs, 10, 7, 0x0f );
      
      tmsg->cursor_anm_frame++;
      if( tmsg->cursor_anm_frame >= 4 ){
        tmsg->cursor_anm_frame = 0;
        tmsg->cursor_anm_no++;
        tmsg->cursor_anm_no %= 3;
      }
		  
      offs = tbl[tmsg->cursor_anm_no];
      
      GFL_BMP_Print( tmsg->cursor_bmp, bmp, 0, 2, x, y+offs, 10, 7, 0x00 );
		  GFL_BMPWIN_TransVramCharacter( twin_bmp );
      tmsg->cursor_put_flag = TRUE;
    }
    break;
  case PRINTSTREAM_STATE_DONE: //終了
    return( TRUE );
  }
  
  return( FALSE );
}

//======================================================================
//  その他
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドメッセージBG ブレンドアルファセット
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_SetBlendAlpha( void )
{
#if 0 //フィールドマップ側で対応
  { //バトルのウィンドウ変更に対応
    #if 0
    int mask =
      GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 |
      GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | 
      GX_WND_PLANEMASK_OBJ;
    G2_SetWnd0InsidePlane( mask, TRUE );
    #else
    GX_SetVisibleWnd( GX_WNDMASK_NONE );
    #endif
  }
#endif
  
  {
    int plane1 = GX_BLEND_PLANEMASK_BG1; 
  	int plane2 = GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG2|
      GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ;
  	G2_SetBlendAlpha( plane1, plane2, 31, 8 );
  }
}

//======================================================================
//	パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * ビットマップウィンドウフレーム　初期化
 * @param	bgframe	BGフレーム
 * @param	heapID	HEAPID
 * @param	pos_x	ビットマップ左上X キャラ単位
 * @param	pos_y	ビットマップ左上Y キャラ単位
 * @param	size_x　ビットマップサイズX キャラ単位
 * @param	size_y	ビットマップサイズY キャラ単位
 * @retval	GFL_BMPWIN	作成されたGFL_BMPWIN
 */
//--------------------------------------------------------------
static GFL_BMPWIN * FldBmpWinFrame_Init( u32 bgFrame, HEAPID heapID,
	u16 pos_x, u16 pos_y, u16 size_x, u16 size_y )
{
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;

	bmpwin = GFL_BMPWIN_Create( bgFrame,
		pos_x, pos_y, size_x, size_y,
		FLDMSGBG_PANO_FONT, GFL_BMP_CHRAREA_GET_B );

	bmp = GFL_BMPWIN_GetBmp( bmpwin );
		
	GFL_BMP_Clear( bmp, 0xff );
	GFL_BMPWIN_MakeScreen( bmpwin );
	GFL_BMPWIN_TransVramCharacter( bmpwin );
	GFL_BG_LoadScreenReq( bgFrame );
	
	BmpWinFrame_Write( bmpwin, WINDOW_TRANS_ON, 1, FLDMSGBG_PANO_MENU );
	
	return( bmpwin );
}

//--------------------------------------------------------------
/**
 * ビットマップウィンドウフレーム　削除
 * @param	bmpwin	FldBmpWinFrame_Init()の戻り値
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldBmpWinFrame_Delete( GFL_BMPWIN *bmpwin )
{
	BmpWinFrame_Clear( bmpwin, 0 );
	GFL_BMPWIN_Delete( bmpwin );
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
//  はい、いいえ選択メニューヘッダー
//--------------------------------------------------------------
static const FLDMENUFUNC_HEADER DATA_MenuHeader_YesNo =
{
	1,		//リスト項目数
	10,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
//	0,		//表示Ｙ座標
	3,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_NON,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	0,		//表示座標X キャラ単位
	0,		//表示座標Y キャラ単位
	0,		//表示サイズX キャラ単位
	0,		//表示サイズY キャラ単位
};

