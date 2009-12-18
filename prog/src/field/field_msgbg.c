//======================================================================
/**
 * @file	field_msgbg.c
 * @brief	フィールド　メッセージBG関連
 * @authaor	kagaya
 * @date	2008.12.11
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

#include "field_bg_def.h"
#include "field/field_msgbg.h"

#include "sound/pm_sndsys.h"

#include "gamesystem\msgspeed.h"

#include "winframe.naix"


//======================================================================
//	define
//======================================================================
#ifdef DEBUG_ONLY_FOR_yoshida
#define TEST_TALKMSGWIN_TYPE (TALKMSGWIN_TYPE_GIZA)
#else
#define TEST_TALKMSGWIN_TYPE (TALKMSGWIN_TYPE_NORMAL)
#endif

#define FLDMSGBG_BGFRAME ( FLDBG_MFRM_MSG )	///<使用BGフレーム

#define FLDMSGBG_PRINT_MAX (4)				///<PRINT関連要素数最大
#define FLDMSGBG_PRINT_STREAM_MAX (1) ///<PRINT_STREAM稼働数

#define FLDMSGBG_STRLEN (128)				///<文字列長さ標準

#define SPWIN_HEIGHT_FONT_SPACE (2)

#define MSG_SKIP_BTN (PAD_BUTTON_A|PAD_BUTTON_B)
#define MSG_LAST_BTN (PAD_BUTTON_A|PAD_BUTTON_B)

#define BGWIN_SCROLL_SPEED (12)
#define BGWIN_MSG_SCROLL_SPEED (4)

#define PAD_KEY_ALL (PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT)

//--------------------------------------------------------------
//  メッセージウィンドウ、キャラオフセット
//--------------------------------------------------------------
enum
{
  CHARNO_CLEAR = 0, ///<クリアキャラ 0
  CHARNO_WINFRAME = 1, ///<ウィンドウフレーム 1-10 (chara 9
  CHARNO_BALLOONWIN = 11, ///<吹き出しウィンドウ 11-30 (chara 18
  CHARNO_SYSWIN = 31, ///<システムウィンドウ 31-50 (chara 18
  CHARNO_OTHERSWIN = 51, ///<その他ウィンドウ 46- (使用時に初期化
};

//--------------------------------------------------------------
//  メッセージウィンドウ　使用パレット
//--------------------------------------------------------------
enum
{
  PANO_BGWIN = 10, //BGウィンドウ
  PANO_SPWIN = 10, //特殊ウィンドウ
  PANO_FONT_TALKMSGWIN = 11, ///<吹き出しフォントパレットNo
  PANO_TALKMSGWIN = 12, ///<吹き出しウィンドウパレットNo
  PANO_MENU = 13, ///<メニューパレットNo
  PANO_FONT = 14, ///<フォントパレットNo
};

//--------------------------------------------------------------
/// カーソルフラグ
//--------------------------------------------------------------
enum
{
  CURSOR_FLAG_NONE = 0,
  CURSOR_FLAG_WRITE,
  CURSOR_FLAG_END,
};

//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
/// KEYCURSOR_WORK
//--------------------------------------------------------------
typedef struct
{
  u8 cursor_anm_no;
  u8 cursor_anm_frame;
  GFL_BMP_DATA *bmp_cursor;
}KEYCURSOR_WORK;

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
	u16 bgFrame;
	u16 deriveWin_plttNo;
  
	GFL_FONT *fontHandle;
	PRINT_QUE *printQue;
	FLDMSGPRINT msgPrintTbl[FLDMSGBG_PRINT_MAX];
  
  FLDSUBMSGWIN *subMsgWinTbl[FLDSUBMSGWIN_MAX];
  
  TALKMSGWIN_SYS *talkMsgWinSys;
  
  GFL_TCBLSYS *printTCBLSys;
  GFL_G3D_CAMERA *g3Dcamera;
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
///	FLDSYSWIN
//--------------------------------------------------------------
struct _TAG_FLDSYSWIN
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
  int msg_wait;
  PRINT_STREAM *printStream;
};

//--------------------------------------------------------------
///	FLDMSGWIN_STREAM
//--------------------------------------------------------------
struct _TAG_FLDMSGWIN_STREAM
{
	GFL_BMPWIN *bmpwin;
	FLDMSGPRINT_STREAM *msgPrintStream;
  FLDMSGPRINT *msgPrint;
  const GFL_MSGDATA *msgData; //ユーザーから
  STRBUF *strBuf;
	FLDMSGBG *fmb;
   
  u8 flag_cursor;
  u8 flag_key_pause_clear;
  KEYCURSOR_WORK cursor_work;
};

//--------------------------------------------------------------
///	FLDSYSWIN_STREAM
//--------------------------------------------------------------
struct _TAG_FLDSYSWIN_STREAM
{
	GFL_BMPWIN *bmpwin;
	FLDMSGPRINT_STREAM *msgPrintStream;
  FLDMSGPRINT *msgPrint;
  const GFL_MSGDATA *msgData; //ユーザーから
  STRBUF *strBuf;
	FLDMSGBG *fmb;
   
  u8 flag_cursor;
  u8 flag_key_pause_clear;
  KEYCURSOR_WORK cursor_work;
};

//--------------------------------------------------------------
/// FLDBGWIN
//--------------------------------------------------------------
struct _TAG_FLDBGWIN
{
  FLDMSGBG *fmb;
  
  GFL_BMPWIN *bmpwin;
  GFL_BMP_DATA *bmp_new;
  GFL_BMP_DATA *bmp_old;
  
  const STRBUF *strBuf; //ユーザーから
  STRBUF *strTemp;
  
  int seq_no;
  int y;
  int scroll_y;
  u32 line;
  
  KEYCURSOR_WORK cursor_work;
};

//--------------------------------------------------------------
/// FLDSPWIN
//--------------------------------------------------------------
struct _TAG_FLDSPWIN
{
	GFL_BMPWIN *bmpwin;
  STRBUF *strBuf;
	FLDMSGBG *fmb;
  
  GFL_BMP_DATA *bmp_bg;
  KEYCURSOR_WORK cursor_work;
};

//======================================================================
//	proto
//======================================================================
static void FldMenuFuncH_BmpMenuListH(
	BMPMENULIST_HEADER *menuH, const FLDMENUFUNC_HEADER *fmenuH );

static PRINTSTREAM_STATE fldMsgPrintStream_ProcPrint(
    FLDMSGPRINT_STREAM *stm );

static BOOL fldSubMsgWin_Print( FLDSUBMSGWIN *subwin );

static void bgwin_InitGraphic(
    u32 bgframe, u32 plttno, u32 type, HEAPID heapID );
static void bgwin_WriteWindow(
    u8 frm, u8 px, u8 py, u8 sx, u8 sy, u16 cgx, u8 pal );
static void bgwin_CleanWindow(
    u8 frm, u8 px, u8 py, u8 sx, u8 sy, u16 cgx, u8 pal );
static BOOL bgwin_CheckStrLineEOM(
    const STRBUF *str_src, STRBUF *str_temp, u32 line );
static BOOL bgwin_PrintStr(
    GFL_BMP_DATA *bmp, GFL_FONT *font,
    const STRBUF *str_src, STRBUF *str_temp, u32 *line, u8 col );
static BOOL bgwin_ScrollBmp(
    GFL_BMP_DATA *bmp, GFL_BMP_DATA *old, GFL_BMP_DATA *new,
    int y, u16 n_col );

static void keyCursor_Init( KEYCURSOR_WORK *work, HEAPID heapID );
static void keyCursor_Delete( KEYCURSOR_WORK *work );
static void keyCursor_Clear(
    KEYCURSOR_WORK *work, GFL_BMP_DATA *bmp, u16 n_col );
static void keyCursor_Write(
    KEYCURSOR_WORK *work, GFL_BMP_DATA *bmp, u16 n_col );
static void keyCursor_WriteBmpBG(
    KEYCURSOR_WORK *work, GFL_BMP_DATA *bmp, GFL_BMP_DATA *bmp_bg );

static void syswin_InitGraphic( HEAPID heapID );
static void syswin_WriteWindow( const GFL_BMPWIN *bmpwin );
static GFL_BMPWIN * syswin_InitBmp( u8 pos_y, HEAPID heapID );
static void syswin_ClearBmp( GFL_BMPWIN *bmpwin );
static void syswin_DeleteBmp( GFL_BMPWIN *bmpwin );

static void setBGResource( FLDMSGBG *fmb );
static void setBlendAlpha( BOOL on );

static GFL_BMPWIN * winframe_InitBmp( u32 bgFrame, HEAPID heapID,
	u16 pos_x, u16 pos_y, u16 size_x, u16 size_y, u16 pltt_no );
static void winframe_DeleteBmp( GFL_BMPWIN *bmpwin );
static void winframe_SetPaletteBlack( u32 heapID );
static void winframe_SetPaletteWhith( u32 heapID );

static int FldMsgBG_SetFldSubMsgWin( FLDMSGBG *fmb, FLDSUBMSGWIN *subwin );
static FLDSUBMSGWIN * FldMsgBG_DeleteFldSubMsgWin( FLDMSGBG *fmb, int id );

static const FLDMENUFUNC_HEADER DATA_MenuHeader_YesNo;
static const u8 ALIGN4 skip_cursor_Character[128];

//======================================================================
//	FLDMSGBG	フィールドメッセージBG関連
//======================================================================
//--------------------------------------------------------------
//  10月ROM用 仮対処 ウィンドウの縁カラーを無効に 0x50001a0
//--------------------------------------------------------------
#if 0
static void debug_ROM091030_WindowColorOFF( HEAPID heapID )
{
  u16 *buf = GFL_HEAP_AllocMemoryLo( heapID, 32 );
  u32 offs = HW_BG_PLTT + (32*PANO_MENU);
  DC_FlushRange( (void*)offs, 32 );
  MI_CpuCopy( (void*)offs, buf, 32 );
  buf[1] = 0;
  DC_FlushRange( (void*)buf, 32 );
  GX_LoadBGPltt( (void*)buf, offs-HW_BG_PLTT, 32 );
  GFL_HEAP_FreeMemory( buf );
}
#endif

#if 0
static void debug_ROM091030_WindowColorON( HEAPID heapID )
{
  u16 *buf = GFL_HEAP_AllocMemoryLo( heapID, 32 );
  u32 offs = HW_BG_PLTT + (32*PANO_MENU);
  DC_FlushRange( (void*)offs, 32 );
  MI_CpuCopy( (void*)offs, buf, 32 );
  buf[1] = 0x7fff;
  DC_FlushRange( (void*)buf, 32 );
  GX_LoadBGPltt( (void*)buf, offs-HW_BG_PLTT, 32 );
  GFL_HEAP_FreeMemory( buf );
}
#endif

//--------------------------------------------------------------
/**
 * FLDMSGBG 作成
 * @param	heapID	HEAPID
 * @param g3Dcamera GFL_G3D_CAMERA* 吹き出しウィンドウで使用 NULL=使用しない
 * @retval	FLDMAPBG	FLDMAPBG*
 */
//--------------------------------------------------------------
FLDMSGBG * FLDMSGBG_Create( HEAPID heapID, GFL_G3D_CAMERA *g3Dcamera )
{
	FLDMSGBG *fmb;
	
	fmb = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDMSGBG) );
	fmb->heapID = heapID;
	fmb->bgFrame = FLDMSGBG_BGFRAME;
	fmb->g3Dcamera = g3Dcamera;
  
  {	//font
		fmb->fontHandle = GFL_FONT_Create(
			ARCID_FONT,
//    NARC_font_large_nftr, //旧フォントID
      NARC_font_large_gftr, //新フォントID
			GFL_FONT_LOADTYPE_FILE, FALSE, fmb->heapID );
	}
	
	{	//print
		fmb->printQue = PRINTSYS_QUE_Create( fmb->heapID );
	}
	
  { //PRINT_STREAM TCB
    fmb->printTCBLSys = GFL_TCBL_Init(
        fmb->heapID, fmb->heapID, FLDMSGBG_PRINT_STREAM_MAX, 4 );
  }
  
  //FLDMSGBG_SetupResource( fmb );
	return( fmb );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG リソースセットアップ
 * @param	fmb FLDMSGBG
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_SetupResource( FLDMSGBG *fmb )
{
  setBGResource( fmb );
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
 * FLDMSGBG BGリソースのみ破棄
 * @param fmb FLDMSGBG
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_ReleaseBGResouce( FLDMSGBG *fmb )
{
  if( fmb->talkMsgWinSys != NULL ){
    TALKMSGWIN_SystemDelete( fmb->talkMsgWinSys );
    fmb->talkMsgWinSys = NULL;
  }
  
	GFL_BG_FillCharacterRelease( fmb->bgFrame, 1, 0 );
  GFL_BG_FreeBGControl( fmb->bgFrame );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG BGリソースをリセット
 * @param *fmb FLDMSGBG
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_ResetBGResource( FLDMSGBG *fmb )
{
  if( fmb->talkMsgWinSys != NULL ){
    TALKMSGWIN_SystemDelete( fmb->talkMsgWinSys );
  }
  
  setBGResource( fmb );
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
	PRINTSYS_QUE_Main( fmb->printQue );
  
  {
    FLDSUBMSGWIN **subWin = fmb->subMsgWinTbl;
    for( i = 0; i < FLDSUBMSGWIN_MAX; i++, subWin++ ){
      if( (*subWin) != NULL ){
        fldSubMsgWin_Print( *subWin );
      }
    }
  }

  {
	  FLDMSGPRINT *msgPrint = fmb->msgPrintTbl;
	  for( i = 0; i < FLDMSGBG_PRINT_MAX; i++, msgPrint++ ){
		  if( msgPrint->printQue != NULL ){
			  msgPrint->printTransFlag = PRINT_UTIL_Trans(
				  &msgPrint->printUtil, msgPrint->printQue );
		  }
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
 * FLDMSGPRINT プリント　STRBUF指定、カラー指定あり
 * @param	msgPrint	FLDMSGPRINT
 * @param	x		表示X座標
 * @param	y		表示Y座標
 * @param	strBuf	表示するSTRBUF
 * @param color PRINTSYS_LSB
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMSGPRINT_PrintStrBufColor( FLDMSGPRINT *msgPrint,
    u32 x, u32 y, STRBUF *strBuf, PRINTSYS_LSB color )
{
  PRINT_UTIL_PrintColor( &msgPrint->printUtil, msgPrint->printQue,
		x, y, strBuf, msgPrint->fontHandle, color );		
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
 * メッセージウィンドウ　追加　メイン
 * @param	fmb	FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @param	bmppos_x		//表示座標X キャラ単位
 * @param	bmppos_y		//表示座標Y キャラ単位
 * @param	bmpsize_x		//表示サイズX キャラ単位
 * @param	bmpsize_y		//表示サイズY キャラ単位
 * @param pltt_no     使用するパレット番号
 * @retval	FLDMSGWIN*
 */
//--------------------------------------------------------------
static FLDMSGWIN * fldmsgwin_Add( FLDMSGBG *fmb, GFL_MSGDATA *msgData,
	u16 bmppos_x, u16 bmppos_y, u16 bmpsize_x, u16 bmpsize_y, u16 pltt_no )
{
	FLDMSGWIN *msgWin;
  
	msgWin = GFL_HEAP_AllocClearMemory( fmb->heapID, sizeof(FLDMSGWIN) );
	msgWin->fmb = fmb;
	msgWin->bmpwin = winframe_InitBmp( fmb->bgFrame, fmb->heapID,
    bmppos_x, bmppos_y, bmpsize_x, bmpsize_y, pltt_no );
	msgWin->msgPrint = FLDMSGPRINT_SetupPrint( fmb, msgData, msgWin->bmpwin );
	
  if( pltt_no == PANO_FONT ){
    winframe_SetPaletteBlack( fmb->heapID );
    setBlendAlpha( TRUE );
  }
  
	return( msgWin );
}

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
  fmb->deriveWin_plttNo = PANO_FONT;
  msgWin = fldmsgwin_Add( fmb, msgData,
      bmppos_x, bmppos_y, bmpsize_x, bmpsize_y, fmb->deriveWin_plttNo );
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
	winframe_DeleteBmp( msgWin->bmpwin );
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
 * メッセージウィンドウ　メッセージ表示 STRBUF指定、カラー指定あり
 * @param	msgWin	FLDMSGWIN
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param	strBuf	STRBUF
 * @param color PRINTSYS_LSB
 * @retval	nothing
 */
//-------------------------------------------------------------
void FLDMSGWIN_PrintStrBufColor( FLDMSGWIN *msgWin,
    u16 x, u16 y, STRBUF *strBuf, PRINTSYS_LSB color )
{
	FLDMSGPRINT_PrintStrBufColor( msgWin->msgPrint, x, y, strBuf, color );
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
//	FLDSYSWIN	システムウィンドウ関連
//======================================================================
//--------------------------------------------------------------
/**
 * システムウィンドウ　追加　メイン
 * @param	fmb	FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @retval	FLDSYSWIN*
 */
//--------------------------------------------------------------
static FLDSYSWIN * syswin_Add(
  FLDMSGBG *fmb, GFL_MSGDATA *msgData, u16 bmppos_y )
{
	FLDSYSWIN *sysWin;
  
	sysWin = GFL_HEAP_AllocClearMemory( fmb->heapID, sizeof(FLDSYSWIN) );
	sysWin->fmb = fmb;
	sysWin->bmpwin = syswin_InitBmp( bmppos_y, fmb->heapID );
	sysWin->msgPrint = FLDMSGPRINT_SetupPrint( fmb, msgData, sysWin->bmpwin );
	
  setBlendAlpha( TRUE );
	return( sysWin );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　追加
 * @param	fmb	FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @param	bmppos_x		//表示座標X キャラ単位
 * @param	bmppos_y		//表示座標Y キャラ単位
 * @param	bmpsize_x		//表示サイズX キャラ単位
 * @param	bmpsize_y		//表示サイズY キャラ単位
 * @retval	FLDSYSWIN*
 */
//--------------------------------------------------------------
FLDSYSWIN * FLDSYSWIN_Add(
    FLDMSGBG *fmb, GFL_MSGDATA *msgData, u16 bmppos_y )
{
  FLDSYSWIN *sysWin;
  fmb->deriveWin_plttNo = PANO_FONT;
  sysWin = syswin_Add( fmb, msgData, bmppos_y );
  return( sysWin );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ 削除。
 * FLDSYSWIN_Add()引数msgDataは各自で行う事。
 * @param	sysWin	FLDSYSWIN*
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_Delete( FLDSYSWIN *sysWin )
{
	syswin_DeleteBmp( sysWin->bmpwin );
	FLDMSGPRINT_Delete( sysWin->msgPrint );
	GFL_HEAP_FreeMemory( sysWin );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　メッセージ表示
 * @param	sysWin	FLDSYSWIN
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param	strID	メッセージデータ 文字列ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_Print( FLDSYSWIN *sysWin, u16 x, u16 y, u32 strID )
{
	FLDMSGPRINT_Print( sysWin->msgPrint, x, y, strID );
	GFL_BG_LoadScreenReq( FLDMSGBG_BGFRAME );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　メッセージ表示 STRBUF指定
 * @param	sysWin	FLDSYSWIN
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param	strBuf	STRBUF
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_PrintStrBuf( FLDSYSWIN *sysWin, u16 x, u16 y, STRBUF *strBuf )
{
	FLDMSGPRINT_PrintStrBuf( sysWin->msgPrint, x, y, strBuf );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　メッセージ表示 STRBUF指定、カラー指定あり
 * @param	sysWin	FLDSYSWIN
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param	strBuf	STRBUF
 * @param color PRINTSYS_LSB
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_PrintStrBufColor( FLDSYSWIN *sysWin,
    u16 x, u16 y, STRBUF *strBuf, PRINTSYS_LSB color )
{
	FLDMSGPRINT_PrintStrBufColor( sysWin->msgPrint, x, y, strBuf, color );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　転送終了チェック
 * @param	sysWin	FLDSYSWIN
 * @retval	BOOL	TRUE=転送終了
 */
//--------------------------------------------------------------
BOOL FLDSYSWIN_CheckPrintTrans( FLDSYSWIN *sysWin )
{
	return( FLDMSGPRINT_CheckPrintTrans(sysWin->msgPrint) );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　メッセージ表示クリア
 * @param	sysWin	FLDSYSWIN
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_ClearWindow( FLDSYSWIN *sysWin )
{
	FLDMSGPRINT_ClearBmp( sysWin->msgPrint );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　メッセージ指定領域クリア
 * @param sysWin FLDSYSWIN
 * @param x 書き込み先書き込み開始X座標（ドット）
 * @param y 書き込み先書き込み開始Y座標（ドット）
 * @param size_x  描画範囲Xサイズ（ドット）
 * @param size_y  描画範囲Yサイズ（ドット）
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_FillClearWindow(
    FLDSYSWIN *sysWin, u32 x, u32 y, u32 size_x, u32 size_y )
{
  FLDMSGPRINT_FillClearBmp( sysWin->msgPrint, x, y, size_x, size_y );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　追加　会話ウィンドウタイプ
 * @param	fmb	FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @retval	FLDSYSWIN*
 */
//--------------------------------------------------------------
FLDSYSWIN * FLDSYSWIN_AddTalkWin( FLDMSGBG *fmb, GFL_MSGDATA *msgData )
{
	return( FLDSYSWIN_Add(fmb,msgData,19) );
}

//======================================================================
//	FLDMENUFUNC　フィールドメニュー関連
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMENUFUNC メニュー追加 メイン
 * @param	fmb	FLDMSGBG
 * @param	pMenuHead FLDMENUFUNC_HEADER
 * @param	pMenuListData  FLDMENUFUNC_LISTDATA* Delete時に自動開放される
 * @param list_pos リスト初期位置
 * @param cursor_pos カーソル初期位置
 * @param pltt_no 使用するパレット番号
 * @retval	FLDMENUFUNC*
 */
//--------------------------------------------------------------
static FLDMENUFUNC * fldmenufunc_AddMenuList( FLDMSGBG *fmb,
	const FLDMENUFUNC_HEADER *pMenuHead,
	FLDMENUFUNC_LISTDATA *pMenuListData,
  u16 list_pos, u16 cursor_pos, u16 pltt_no )
{
	FLDMENUFUNC *menuFunc;
	BMPMENULIST_HEADER menuH;
	
	menuFunc = GFL_HEAP_AllocClearMemory( fmb->heapID, sizeof(FLDMENUFUNC) );
	FldMenuFuncH_BmpMenuListH( &menuH, pMenuHead );
	
	menuFunc->fmb = fmb;
	menuFunc->pMenuListData = (BMP_MENULIST_DATA *)pMenuListData;
	
	menuFunc->bmpwin = winframe_InitBmp(
		fmb->bgFrame, fmb->heapID,
		pMenuHead->bmppos_x, pMenuHead->bmppos_y,
		pMenuHead->bmpsize_x, pMenuHead->bmpsize_y, pltt_no );
	
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
  
  if( pltt_no == PANO_FONT ){
    winframe_SetPaletteBlack( fmb->heapID );
    setBlendAlpha( TRUE );
  }
	return( menuFunc );
}

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
  fmb->deriveWin_plttNo = PANO_FONT;
  menuFunc = fldmenufunc_AddMenuList( fmb, pMenuHead, pMenuListData,
    list_pos, cursor_pos, fmb->deriveWin_plttNo );
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
 * FLDMENUFUNC メニュー追加　イベント用
 * @param	fmb	FLDMSGBG
 * @param	pMenuHead FLDMENUFUNC_HEADER
 * @param	pMenuListData  FLDMENUFUNC_LISTDATA* Delete時に自動開放される
 * @param list_pos リスト初期位置
 * @param cursor_pos カーソル初期位置
 * @param cancel TRUE=キャンセル可
 * @retval	FLDMENUFUNC*
 * @note メニューカラーは派生ウィンドウのカラーを継続する
 */
//--------------------------------------------------------------
FLDMENUFUNC * FLDMENUFUNC_AddEventMenuList( FLDMSGBG *fmb,
	const FLDMENUFUNC_HEADER *pMenuHead,
	FLDMENUFUNC_LISTDATA *pMenuListData,
  u16 list_pos, u16 cursor_pos, BOOL cancel )
{
	FLDMENUFUNC *menuFunc;
  menuFunc = fldmenufunc_AddMenuList( fmb, pMenuHead, pMenuListData,
    list_pos, cursor_pos, fmb->deriveWin_plttNo );
  
  if( cancel == FALSE ){
    BmpMenuList_SetCancelMode(
        menuFunc->pMenuListWork, BMPMENULIST_CANCELMODE_NOT );
  }

  return( menuFunc );
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
	winframe_DeleteBmp( menuFunc->bmpwin );
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
 * @param space 文字列の表示間隔
 * @retval u32 メニュー横幅 キャラ単位
 */
//--------------------------------------------------------------
u32 FLDMENUFUNC_GetListMenuWidth(
    const FLDMENUFUNC_LISTDATA *listData, u32 font_size, u32 space )
{
  u32 c,len;
  u32 num = FLDMENUFUNC_GetListLengthMax( listData );
  num++; //カーソル分
  len = num * font_size;
  c = len / 8;
  if( (len & 0x07) ){ c++; }
  if( space ){ c += ((num*space)/8)+1; }
  return( c );
}

//--------------------------------------------------------------
/**
 * FLDMENUFUNC_LISTDATAに格納されているリスト数から
 * 必要なメニュー縦幅を取得。
 * @param listData FLDMENUFUNC_LISTDATA
 * @param font_size フォント縦サイズ
 * @param space 列の表示間隔
 * @retval u32 メニュー縦幅 キャラ単位
 */
//--------------------------------------------------------------
u32 FLDMENUFUNC_GetListMenuHeight(
    const FLDMENUFUNC_LISTDATA *listData, u32 font_size, u32 space )
{
  u32 c,len;
  u32 num = FLDMENUFUNC_GetListMax( listData );
  len = num * font_size;
  c = len / 8;
  if( (len & 0x07) ){ c++; }
  if( space ){ c += ((num*space)/8)+1; }
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
  menuFunc = fldmenufunc_AddMenuList( fmb, &menuH, listData,
      0, cursor_pos, fmb->deriveWin_plttNo );
  
#if 0 //Bキャンセル無効
  BmpMenuList_SetCancelMode(
      menuFunc->pMenuListWork, BMPMENULIST_CANCELMODE_NOT );
#else //Bキャンセル有効
  BmpMenuList_SetCancelMode(
      menuFunc->pMenuListWork, BMPMENULIST_CANCELMODE_USE );
#endif

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
    return( FLDMENUFUNC_YESNO_NULL ); //無し
  }
  
  if( ret == FLDMENUFUNC_CANCEL ){ //キャンセルはNOに
    return( FLDMENUFUNC_YESNO_NO );
  }
  
  return( ret );
}

//======================================================================
//  FLDMSGPRINT_STREAM メッセージ　プリントストリーム関連
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMSGPRINT_STREAM プリント設定　カラー指定あり
 * @param fmb FLDMSGBG
 * @param strbuf 表示するSTRBUF
 * @param bmpwin 表示する初期化済みのGFL_BMPWIN
 * @param x 描画開始X座標(ドット)
 * @param y 描画開始Y座標(ドット)
 * @param   wait		１文字描画ごとのウェイトフレーム数
 * @retval FLDMSGPRINT_STREAM
 */
//--------------------------------------------------------------
FLDMSGPRINT_STREAM * FLDMSGPRINT_STREAM_SetupPrintColor(
	FLDMSGBG *fmb, const STRBUF *strbuf,
  GFL_BMPWIN *bmpwin, u16 x, u16 y, int wait, u16 n_color )
{
  FLDMSGPRINT_STREAM *stm = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDMSGPRINT_STREAM) );
  
  stm->msg_wait = wait;
  
  stm->printStream = PRINTSYS_PrintStream(
      bmpwin, x, y,
      strbuf, fmb->fontHandle,
      wait,
      fmb->printTCBLSys, 0,
      fmb->heapID, n_color );

  return( stm );
}

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
  return( FLDMSGPRINT_STREAM_SetupPrintColor(
        fmb,strbuf,bmpwin,x,y,wait,0x0f) );
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
 * @retval PRINTSTREAM_STATE
 */
//--------------------------------------------------------------
static PRINTSTREAM_STATE fldMsgPrintStream_ProcPrint(
    FLDMSGPRINT_STREAM *stm )
{
  int trg,cont;
  PRINTSTREAM_STATE state;
  
  trg = GFL_UI_KEY_GetTrg();
  cont = GFL_UI_KEY_GetCont();
  state = PRINTSYS_PrintStreamGetState( stm->printStream );
  
  switch( state ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    if( (trg & MSG_SKIP_BTN) ){
      stm->flag_key_trg = TRUE;
    }
    
    if( stm->flag_key_trg == TRUE && (cont & MSG_SKIP_BTN) ){
      PRINTSYS_PrintStreamShortWait( stm->printStream, 0 );
    }
#if 0
    else{
      PRINTSYS_PrintStreamShortWait( stm->printStream, stm->msg_wait );
    }
#endif
    break;
  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( (trg & MSG_LAST_BTN) ){
      PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
      PRINTSYS_PrintStreamReleasePause( stm->printStream );
      stm->flag_key_trg = FALSE;
    }
    break;
  case PRINTSTREAM_STATE_DONE: //終了
    break;
  }
  
  return( state );
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
  PRINTSTREAM_STATE state;
  state = fldMsgPrintStream_ProcPrint( stm );
  
  if( state == PRINTSTREAM_STATE_DONE ){ //終了
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
	
  fmb->deriveWin_plttNo = PANO_FONT;
  
	msgWin = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDMSGWIN_STREAM) );
	msgWin->fmb = fmb;
  msgWin->msgData = msgData;
	msgWin->bmpwin = winframe_InitBmp( fmb->bgFrame, fmb->heapID,
      bmppos_x, bmppos_y, bmpsize_x, bmpsize_y, fmb->deriveWin_plttNo );
  msgWin->strBuf = GFL_STR_CreateBuffer( FLDMSGBG_STRLEN, fmb->heapID );
  
  keyCursor_Init( &msgWin->cursor_work, fmb->heapID );

  winframe_SetPaletteBlack( fmb->heapID );
  setBlendAlpha( TRUE );
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
	winframe_DeleteBmp( msgWin->bmpwin );
  
  if( msgWin->msgPrintStream != NULL ){
	  FLDMSGPRINT_STREAM_Delete( msgWin->msgPrintStream );
  }
  
  if( msgWin->msgPrint != NULL ){
    FLDMSGPRINT_Delete( msgWin->msgPrint );
  }
  
  GFL_STR_DeleteBuffer( msgWin->strBuf );
  keyCursor_Delete( &msgWin->cursor_work );
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
  	msgWin->fmb, msgWin->strBuf, msgWin->bmpwin, x, y, MSGSPEED_GetWait() );
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
  	msgWin->fmb, strBuf, msgWin->bmpwin, x, y, MSGSPEED_GetWait() );
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
#if 0
  if( FLDMSGPRINT_STREAM_ProcPrint(msgWin->msgPrintStream) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
#else
  int trg,cont;
  PRINTSTREAM_STATE state;
  state = fldMsgPrintStream_ProcPrint( msgWin->msgPrintStream );
  
  trg = GFL_UI_KEY_GetTrg();
  cont = GFL_UI_KEY_GetCont();

  switch( state ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    msgWin->flag_cursor = CURSOR_FLAG_NONE;
    msgWin->flag_key_pause_clear = FALSE;
    break;
  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( msgWin->flag_key_pause_clear == FALSE ){ //既にポーズクリア済みか？
		  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( msgWin->bmpwin );
      
      if( (trg & MSG_LAST_BTN) ){
        if( msgWin->flag_cursor == CURSOR_FLAG_WRITE ){
          keyCursor_Clear( &msgWin->cursor_work, bmp, 0x0f );
          
          msgWin->flag_key_pause_clear = TRUE;
          msgWin->flag_cursor = CURSOR_FLAG_END;
        }
      }
      
      if( msgWin->flag_cursor != CURSOR_FLAG_END ){
        keyCursor_Write( &msgWin->cursor_work, bmp, 0x0f );
        msgWin->flag_cursor = CURSOR_FLAG_WRITE;
      }
      
		  GFL_BMPWIN_TransVramCharacter( msgWin->bmpwin );
    }
    break;
  case PRINTSTREAM_STATE_DONE: //終了
    return( TRUE );
  }

  return( FALSE );
#endif
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
      WINDOW_TRANS_ON, 1, PANO_MENU );
  FLDMSGWIN_STREAM_ClearMessage( msgWin );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ　メッセージ一括表示
 * @param msgWin FLDMSGWIN_STREAM
 * @param x X表示座標
 * @param y Y表示座標
 * @param strBuf 表示するSTRBUF
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGWIN_STREAM_AllPrintStrBuf(
    FLDMSGWIN_STREAM *msgWin, u16 x, u16 y, STRBUF *strBuf )
{
  if( msgWin->msgPrint != NULL ){
    FLDMSGPRINT_Delete( msgWin->msgPrint );
  }
  
  msgWin->msgPrint = FLDMSGPRINT_SetupPrint(
      msgWin->fmb, NULL, msgWin->bmpwin );
  
  FLDMSGPRINT_PrintStrBuf( msgWin->msgPrint, x, y, strBuf );
}

//--------------------------------------------------------------
/**
 * FLDMSGWIN_STREAM メッセージウィンドウ　メッセージ一括表示　転送チェック
 * @param msgWin FLDMSGWIN_STREAM
 * @retval BOOL TRUE=転送終了
 */
//--------------------------------------------------------------
BOOL FLDMSGWIN_STREAM_CheckAllPrintTrans( FLDMSGWIN_STREAM *msgWin )
{
  return( FLDMSGPRINT_CheckPrintTrans(msgWin->msgPrint) );
}

//======================================================================
//  FLDSYSWIN_STREAM
//======================================================================
//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ追加
 * @param	fmb	FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @param	bmppos_y		//表示座標Y キャラ単位
 * @retval FLDSYSWIN_STREAM*
 */
//--------------------------------------------------------------
FLDSYSWIN_STREAM * FLDSYSWIN_STREAM_Add(
    FLDMSGBG *fmb, const GFL_MSGDATA *msgData, u16 bmppos_y )
{
  FLDSYSWIN_STREAM *sysWin;
	
  fmb->deriveWin_plttNo = PANO_FONT;
  
	sysWin = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDSYSWIN_STREAM) );
	sysWin->fmb = fmb;
  sysWin->msgData = msgData;
	sysWin->bmpwin = syswin_InitBmp( bmppos_y, fmb->heapID );
  sysWin->strBuf = GFL_STR_CreateBuffer( FLDMSGBG_STRLEN, fmb->heapID );
  
  keyCursor_Init( &sysWin->cursor_work, fmb->heapID );
  
  winframe_SetPaletteBlack( fmb->heapID );
  setBlendAlpha( TRUE );
  return( sysWin );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ 削除
 * FLDSYSWIN_STREAM_Add()引数msgDataは各自で行う事。
 * @param sysWin FLDSYSWIN_STREAM
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_STREAM_Delete( FLDSYSWIN_STREAM *sysWin )
{
	syswin_DeleteBmp( sysWin->bmpwin );
  
  if( sysWin->msgPrintStream != NULL ){
	  FLDMSGPRINT_STREAM_Delete( sysWin->msgPrintStream );
  }
  
  if( sysWin->msgPrint != NULL ){
    FLDMSGPRINT_Delete( sysWin->msgPrint );
  }
  
  GFL_STR_DeleteBuffer( sysWin->strBuf );
  keyCursor_Delete( &sysWin->cursor_work );
	GFL_HEAP_FreeMemory( sysWin );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ メッセージ表示開始
 * @param sysWin FLDSYSWIN_STREAM*
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param strID メッセージデータ　文字列ID
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_STREAM_PrintStart(
    FLDSYSWIN_STREAM *sysWin, u16 x, u16 y, u32 strID )
{
  if( sysWin->msgPrintStream != NULL ){
    FLDMSGPRINT_STREAM_Delete( sysWin->msgPrintStream );
  }
  
  GF_ASSERT( sysWin->msgData );
  GFL_MSG_GetString( sysWin->msgData, strID, sysWin->strBuf );
  
  sysWin->msgPrintStream = FLDMSGPRINT_STREAM_SetupPrint(
  	sysWin->fmb, sysWin->strBuf, sysWin->bmpwin, x, y, MSGSPEED_GetWait() );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ メッセージ表示開始 STRBUF指定
 * @param sysWin FLDSYSWIN_STREAM*
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param strBuf 表示するSTRBUF
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_STREAM_PrintStrBufStart(
    FLDSYSWIN_STREAM *sysWin, u16 x, u16 y, const STRBUF *strBuf )
{
  if( sysWin->msgPrintStream != NULL ){
    FLDMSGPRINT_STREAM_Delete( sysWin->msgPrintStream );
  }
  
  sysWin->msgPrintStream = FLDMSGPRINT_STREAM_SetupPrint(
  	sysWin->fmb, strBuf, sysWin->bmpwin, x, y, MSGSPEED_GetWait() );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ メッセージ表示
 * @param sysWin FLDSYSWIN_STREAM*
 * @retval BOOL TRUE=表示終了,FALSE=表示中
 */
//--------------------------------------------------------------
BOOL FLDSYSWIN_STREAM_Print( FLDSYSWIN_STREAM *sysWin )
{
  int trg,cont;
  PRINTSTREAM_STATE state;
  state = fldMsgPrintStream_ProcPrint( sysWin->msgPrintStream );
  
  trg = GFL_UI_KEY_GetTrg();
  cont = GFL_UI_KEY_GetCont();

  switch( state ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    sysWin->flag_cursor = CURSOR_FLAG_NONE;
    sysWin->flag_key_pause_clear = FALSE;
    break;
  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( sysWin->flag_key_pause_clear == FALSE ){ //既にポーズクリア済みか？
		  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( sysWin->bmpwin );
      
      if( (trg & MSG_LAST_BTN) ){
        if( sysWin->flag_cursor == CURSOR_FLAG_WRITE ){
          keyCursor_Clear( &sysWin->cursor_work, bmp, 0x0f );
          
          sysWin->flag_key_pause_clear = TRUE;
          sysWin->flag_cursor = CURSOR_FLAG_END;
        }
      }
      
      if( sysWin->flag_cursor != CURSOR_FLAG_END ){
        keyCursor_Write( &sysWin->cursor_work, bmp, 0x0f );
        sysWin->flag_cursor = CURSOR_FLAG_WRITE;
      }
      
		  GFL_BMPWIN_TransVramCharacter( sysWin->bmpwin );
    }
    break;
  case PRINTSTREAM_STATE_DONE: //終了
    return( TRUE );
  }

  return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ追加 会話ウィンドウタイプ
 * @param fmb FLDMSGBG*
 * @param	msgData	表示する初期化済みのGFL_MSGDATA NULL=使用しない
 * @retval FLDSYSWIN_STREAM
 */
//--------------------------------------------------------------
FLDSYSWIN_STREAM * FLDSYSWIN_STREAM_AddTalkWin(
    FLDMSGBG *fmb, GFL_MSGDATA *msgData )
{
  return( FLDSYSWIN_STREAM_Add(fmb,msgData,19) );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ メッセージクリア
 * @param sysWin FLDSYSWIN_STREAM
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_STREAM_ClearMessage( FLDSYSWIN_STREAM *sysWin )
{
  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( sysWin->bmpwin );
  GFL_BMP_Clear( bmp, 0xff);
	GFL_BG_LoadScreenReq( sysWin->fmb->bgFrame );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ ウィンドウクリア
 * @param sysWin FLDSYSWIN_STREAM
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_STREAM_ClearWindow( FLDSYSWIN_STREAM *sysWin )
{
  syswin_ClearBmp( sysWin->bmpwin );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ ウィンドウ描画
 * @param sysWin FLDSYSWIN_STREAM
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_STREAM_WriteWindow( FLDSYSWIN_STREAM *sysWin )
{
	syswin_WriteWindow( sysWin->bmpwin );
  FLDSYSWIN_STREAM_ClearMessage( sysWin );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ　メッセージ一括表示
 * @param sysWin FLDSYSWIN_STREAM
 * @param x X表示座標
 * @param y Y表示座標
 * @param strBuf 表示するSTRBUF
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSYSWIN_STREAM_AllPrintStrBuf(
    FLDSYSWIN_STREAM *sysWin, u16 x, u16 y, STRBUF *strBuf )
{
  if( sysWin->msgPrint != NULL ){
    FLDMSGPRINT_Delete( sysWin->msgPrint );
  }
  
  sysWin->msgPrint = FLDMSGPRINT_SetupPrint(
      sysWin->fmb, NULL, sysWin->bmpwin );
  
  FLDMSGPRINT_PrintStrBuf( sysWin->msgPrint, x, y, strBuf );
}

//--------------------------------------------------------------
/**
 * FLDSYSWIN_STREAM システムウィンドウ　メッセージ一括表示　転送チェック
 * @param sysWin FLDSYSWIN_STREAM
 * @retval BOOL TRUE=転送終了
 */
//--------------------------------------------------------------
BOOL FLDSYSWIN_STREAM_CheckAllPrintTrans( FLDSYSWIN_STREAM *sysWin )
{
  return( FLDMSGPRINT_CheckPrintTrans(sysWin->msgPrint) );
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
  u8 flag_key_pause_clear;
  u8 flag_cursor;
  STRBUF *strBuf;
  int talkMsgWinIdx;
  TALKMSGWIN_SYS *talkMsgWinSys; //FLDMSGBGより
  KEYCURSOR_WORK cursor_work;
};

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
    FLDTALKMSGWIN_IDX idx, const VecFx32 *pos, STRBUF *strBuf,
    TALKMSGWIN_TYPE type )
{
  GF_ASSERT( fmb->talkMsgWinSys != NULL );
  
  fmb->deriveWin_plttNo = PANO_FONT_TALKMSGWIN;

  tmsg->talkMsgWinSys = fmb->talkMsgWinSys;
  tmsg->talkMsgWinIdx = idx;
  
  winframe_SetPaletteWhith( fmb->heapID );
  setBlendAlpha( FALSE );

  switch( idx ){
  case FLDTALKMSGWIN_IDX_UPPER:
    TALKMSGWIN_CreateFixWindowUpper( tmsg->talkMsgWinSys,
        FLDTALKMSGWIN_IDX_UPPER, (VecFx32*)pos, strBuf, 15, type );
    break;
  case FLDTALKMSGWIN_IDX_LOWER:
    TALKMSGWIN_CreateFixWindowLower( tmsg->talkMsgWinSys,
        FLDTALKMSGWIN_IDX_LOWER, (VecFx32*)pos, strBuf, 15, type );
    break;
  default:
    TALKMSGWIN_CreateFixWindowAuto( tmsg->talkMsgWinSys,
        FLDTALKMSGWIN_IDX_AUTO, (VecFx32*)pos, strBuf, 15, type );
  }
  
  TALKMSGWIN_OpenWindow( tmsg->talkMsgWinSys, tmsg->talkMsgWinIdx );
  
  keyCursor_Init( &tmsg->cursor_work, fmb->heapID );
  
  KAGAYA_Printf( "ウィンドウ %d,%d,%d\n", pos->x, pos->y, pos->z );
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
    const GFL_MSGDATA *msgData, u32 msgID, TALKMSGWIN_TYPE type )
{
  FLDTALKMSGWIN *tmsg = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDTALKMSGWIN) );
  tmsg->strBuf = GFL_STR_CreateBuffer(
					FLDMSGBG_STRLEN, fmb->heapID );
  GFL_MSG_GetString( msgData, msgID, tmsg->strBuf );
  fldTalkMsgWin_Add( fmb, tmsg, idx, pos, tmsg->strBuf, type );
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
    FLDTALKMSGWIN_IDX idx, const VecFx32 *pos,
    STRBUF *strBuf, TALKMSGWIN_TYPE type )
{
  FLDTALKMSGWIN *tmsg = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDTALKMSGWIN) );
  fldTalkMsgWin_Add( fmb, tmsg, idx, pos, strBuf, type );
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
  keyCursor_Delete( &tmsg->cursor_work );
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
    tmsg->flag_cursor = CURSOR_FLAG_NONE;
    tmsg->flag_key_pause_clear = FALSE;
    
    if( (trg & MSG_SKIP_BTN) ){
      tmsg->flag_key_trg = TRUE;
    }
    
    if( tmsg->flag_key_trg == TRUE && (cont & MSG_SKIP_BTN) ){
      PRINTSYS_PrintStreamShortWait( stream, 0 );
    }
#if 0
    else{
      PRINTSYS_PrintStreamShortWait( stream, MSGSPEED_GetWait() );
    }
#endif
    break;
  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( tmsg->flag_key_pause_clear == FALSE ){ //既にポーズクリア済みか？
      GFL_BMPWIN *twin_bmp = TALKMSGWIN_GetBmpWin(
          tmsg->talkMsgWinSys, tmsg->talkMsgWinIdx );
		  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( twin_bmp );
      
      if( (trg & MSG_LAST_BTN) ){
        PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
        PRINTSYS_PrintStreamReleasePause( stream );
        
        if( tmsg->flag_cursor == CURSOR_FLAG_WRITE ){
          keyCursor_Clear( &tmsg->cursor_work, bmp, 0x0f );
        }
        
        tmsg->flag_key_trg = FALSE;
        tmsg->flag_key_pause_clear = TRUE;
        tmsg->flag_cursor = CURSOR_FLAG_END;
      }
      
      if( tmsg->flag_cursor != CURSOR_FLAG_END ){
        keyCursor_Write( &tmsg->cursor_work, bmp, 0x0f );
        tmsg->flag_cursor = CURSOR_FLAG_WRITE;
      }
      
		  GFL_BMPWIN_TransVramCharacter( twin_bmp );
    }
    break;
  case PRINTSTREAM_STATE_DONE: //終了
    return( TRUE );
  }
  
  return( FALSE );
}

//======================================================================
//  プレーンウィンドウ
//======================================================================
//--------------------------------------------------------------
/// FLDPLAINMSGWIN
//--------------------------------------------------------------
struct _TAG_FLDPLAINMSGWIN
{
  u8 flag_key_trg;
  u8 flag_key_cont;
  u8 flag_key_pause_clear;
  u8 flag_cursor;
  STRBUF *strBuf;
  int talkMsgWinIdx;
  TALKMSGWIN_SYS *talkMsgWinSys; //FLDMSGBGより
  KEYCURSOR_WORK cursor_work;
};

//--------------------------------------------------------------
/**
 * FLDPLAINMSGWIN プレーンウィンドウセット
 * @param fmb FLDMSGBG
 * @param up_down 0=up,1=down
 * @param strBuf 表示するSTRBUF
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldPlainMsgWin_Add(
    FLDMSGBG *fmb, FLDPLAINMSGWIN *plnwin,
    BOOL up_down, STRBUF *strBuf )
{
  TALKMSGWIN_TYPE type = TEST_TALKMSGWIN_TYPE;
  
  GF_ASSERT( fmb->talkMsgWinSys != NULL );
  
  fmb->deriveWin_plttNo = PANO_FONT_TALKMSGWIN;

  plnwin->talkMsgWinSys = fmb->talkMsgWinSys;
  plnwin->talkMsgWinIdx = FLDTALKMSGWIN_IDX_PLAIN;
  
  winframe_SetPaletteWhith( fmb->heapID );
  setBlendAlpha( FALSE );
  
  if( up_down == 0 ){
    TALKMSGWIN_CreateWindowAlone(
        plnwin->talkMsgWinSys, plnwin->talkMsgWinIdx,
        strBuf, 1, 2, 30, 5, GX_RGB(31,31,31), type );
  }else{
    TALKMSGWIN_CreateWindowAlone(
        plnwin->talkMsgWinSys, plnwin->talkMsgWinIdx,
        strBuf, 1, 17, 30, 5, GX_RGB(31,31,31), type );
  }
  
  TALKMSGWIN_OpenWindow( plnwin->talkMsgWinSys, plnwin->talkMsgWinIdx );
  keyCursor_Init( &plnwin->cursor_work, fmb->heapID );
}

//--------------------------------------------------------------
/**
 * FLDPLAINMSGWIN プレーンウィンドウセット
 * @param fmb FLDMSGBG
 * @param up_down 0=up,1=down
 * @param msgData 使用するGFL_MSGDATA
 * @param msgID メッセージ アーカイブデータID
 * @retval FLDPLAINMSGWIN*
 */
//--------------------------------------------------------------
FLDPLAINMSGWIN * FLDPLAINMSGWIN_Add( FLDMSGBG *fmb,
    BOOL up_down, const GFL_MSGDATA *msgData, u32 msgID )
{
  FLDPLAINMSGWIN *plnwin = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDPLAINMSGWIN) );
  plnwin->strBuf = GFL_STR_CreateBuffer(
					FLDMSGBG_STRLEN, fmb->heapID );
  GFL_MSG_GetString( msgData, msgID, plnwin->strBuf );
  fldPlainMsgWin_Add( fmb, plnwin, up_down, plnwin->strBuf );
  return( plnwin );
}

//--------------------------------------------------------------
/**
 * FLDPLAINMSGWIN プレーンウィンドウセット
 * @param fmb FLDMSGBG
 * @param up_down 0=up,1=down
 * @param	strBuf	表示するSTRBUF
 * @param msgID メッセージ アーカイブデータID
 * @retval FLDPLAINMSGWIN*
 */
//--------------------------------------------------------------
FLDPLAINMSGWIN * FLDPLAINMSGWIN_AddStrBuf( FLDMSGBG *fmb,
    BOOL up_down, STRBUF *strBuf )
{
  FLDPLAINMSGWIN *plnwin = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDPLAINMSGWIN) );
  fldPlainMsgWin_Add( fmb, plnwin, up_down, strBuf );
  return( plnwin );
}

//--------------------------------------------------------------
/**
 * FLDPLAINMSGWIN プレーンウィンドウ削除
 * @param tmsg FLDTALKMSGWIN
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDPLAINMSGWIN_Delete( FLDPLAINMSGWIN *plnwin )
{
  if( plnwin->strBuf != NULL ){
		GFL_STR_DeleteBuffer( plnwin->strBuf );
  }
  
  TALKMSGWIN_DeleteWindow( plnwin->talkMsgWinSys, plnwin->talkMsgWinIdx );
  keyCursor_Delete( &plnwin->cursor_work );
  GFL_HEAP_FreeMemory( plnwin );
}

//--------------------------------------------------------------
/**
 * FLDPLAINMSGWIN プレーンウィンドウ 表示
 * @param plnwin FLDTALKMSGWIN
 * @retval BOOL TRUE=表示終了,FALSE=表示中
 */
//--------------------------------------------------------------
BOOL FLDPLAINMSGWIN_Print( FLDPLAINMSGWIN *plnwin )
{
  int trg,cont;
  PRINTSTREAM_STATE state;
  PRINT_STREAM *stream;
  PRINTSTREAM_PAUSE_TYPE pause_type;

  if( TALKMSGWIN_CheckPrintOn(
        plnwin->talkMsgWinSys,plnwin->talkMsgWinIdx) == FALSE ){
    return( FALSE );
  }

  trg = GFL_UI_KEY_GetTrg();
  cont = GFL_UI_KEY_GetCont();
  stream = TALKMSGWIN_GetPrintStream(
      plnwin->talkMsgWinSys, plnwin->talkMsgWinIdx );
  state = PRINTSYS_PrintStreamGetState( stream );
  
  switch( state ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    plnwin->flag_cursor = CURSOR_FLAG_NONE;
    plnwin->flag_key_pause_clear = FALSE;
    
    if( (trg & MSG_SKIP_BTN) ){
      plnwin->flag_key_trg = TRUE;
    }
    
    if( plnwin->flag_key_trg == TRUE && (cont & MSG_SKIP_BTN) ){
      PRINTSYS_PrintStreamShortWait( stream, 0 );
    }
    break;
  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( plnwin->flag_key_pause_clear == FALSE ){ //既にポーズクリア済みか？
      GFL_BMPWIN *twin_bmp = TALKMSGWIN_GetBmpWin(
          plnwin->talkMsgWinSys, plnwin->talkMsgWinIdx );
		  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( twin_bmp );
      
      if( (trg & MSG_LAST_BTN) ){
        PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
        PRINTSYS_PrintStreamReleasePause( stream );
        
        if( plnwin->flag_cursor == CURSOR_FLAG_WRITE ){
          keyCursor_Clear( &plnwin->cursor_work, bmp, 0x0f );
        }
        
        plnwin->flag_key_trg = FALSE;
        plnwin->flag_key_pause_clear = TRUE;
        plnwin->flag_cursor = CURSOR_FLAG_END;
      }
      
      if( plnwin->flag_cursor != CURSOR_FLAG_END ){
        keyCursor_Write( &plnwin->cursor_work, bmp, 0x0f );
        plnwin->flag_cursor = CURSOR_FLAG_WRITE;
      }
      
		  GFL_BMPWIN_TransVramCharacter( twin_bmp );
    }
    break;
  case PRINTSTREAM_STATE_DONE: //終了
    return( TRUE );
  }
  
  return( FALSE );
}

//======================================================================
//  サブウィンドウ
//======================================================================
//--------------------------------------------------------------
/// FLDSUBMSGWIN
//--------------------------------------------------------------
struct _TAG_FLDSUBMSGWIN
{
  STRBUF *strBuf;
  int id; //ウィンドウID
  int talkMsgWinIdx;
  TALKMSGWIN_SYS *talkMsgWinSys; //FLDMSGBGより
  KEYCURSOR_WORK cursor_work;
};

//--------------------------------------------------------------
/**
 * FLDSUBMSGWIN サブウィンドウセット
 * @param fmb FLDMSGBG
 * @param strBuf 表示するSTRBUF
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldSubMsgWin_Add(
    FLDMSGBG *fmb, FLDSUBMSGWIN *subwin,
    STRBUF *strBuf, u16 idx, u8 x, u8 y, u8 sx, u8 sy, int id )
{
  TALKMSGWIN_TYPE type = TEST_TALKMSGWIN_TYPE;

  GF_ASSERT( fmb->talkMsgWinSys != NULL );
  
  fmb->deriveWin_plttNo = PANO_FONT_TALKMSGWIN;
  
  subwin->talkMsgWinSys = fmb->talkMsgWinSys;
  subwin->talkMsgWinIdx = idx;
  subwin->id = id;
  
  winframe_SetPaletteWhith( fmb->heapID );
  setBlendAlpha( FALSE );
  
  TALKMSGWIN_CreateWindowAlone(
    subwin->talkMsgWinSys, subwin->talkMsgWinIdx,
    strBuf, x, y, sx, sy, GX_RGB(31,31,31), type );
  
  TALKMSGWIN_OpenWindow( subwin->talkMsgWinSys, subwin->talkMsgWinIdx );
}

//--------------------------------------------------------------
/**
 * FLDPLAINMSGWIN サブウィンドウセット
 * @param fmb FLDMSGBG
 * @param msgData 使用するGFL_MSGDATA
 * @param msgID メッセージ アーカイブデータID
 * @retval FLDPLAINMSGWIN*
 */
//--------------------------------------------------------------
void FLDSUBMSGWIN_Add( FLDMSGBG *fmb,
    const GFL_MSGDATA *msgData, u32 msgID,
    int id, u8 x, u8 y, u8 sx, u8 sy )
{
  int idx;
  
  FLDSUBMSGWIN *subwin = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDSUBMSGWIN) );
  
  idx = FldMsgBG_SetFldSubMsgWin( fmb, subwin );
  idx += FLDTALKMSGWIN_IDX_SUBWIN0;
  
  subwin->strBuf = GFL_STR_CreateBuffer(
					FLDMSGBG_STRLEN_SUBWIN, fmb->heapID );
  
  GFL_MSG_GetString( msgData, msgID, subwin->strBuf );
  fldSubMsgWin_Add( fmb, subwin, subwin->strBuf,
      idx, x, y, sx, sy, id );
}

//--------------------------------------------------------------
/**
 * FLDPLAINMSGWIN サブウィンドウセット
 * @param fmb FLDMSGBG
 * @param up_down 0=up,1=down
 * @param strBuf 表示するSTRBUF 
 * @retval nothing
 * @note strBufはコピーして格納しますので、
 * セット後、開放しても問題ありません。
 */
//--------------------------------------------------------------
void FLDSUBMSGWIN_AddStrBuf( FLDMSGBG *fmb,
    const STRBUF *strBuf, int id, u8 x, u8 y, u8 sx, u8 sy )
{
  int idx;
  
  FLDSUBMSGWIN *subwin = GFL_HEAP_AllocClearMemory(
      fmb->heapID, sizeof(FLDSUBMSGWIN) );
  idx = FldMsgBG_SetFldSubMsgWin( fmb, subwin );
  idx += FLDTALKMSGWIN_IDX_SUBWIN0;
  
  {
    u32 len = GFL_STR_GetBufferLength( strBuf ) + 1;
    subwin->strBuf = GFL_STR_CreateBuffer( len, fmb->heapID );
	  GFL_STR_CopyBuffer( subwin->strBuf, strBuf );
  }
  
  fldSubMsgWin_Add(
      fmb, subwin, subwin->strBuf, idx, x, y, sx, sy, id );
}

//--------------------------------------------------------------
/**
 * FLDSUBMSGWIN サブウィンドウ削除　メイン
 * @param tmsg FLDTALKMSGWIN
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldSubMsgWin_Delete( FLDSUBMSGWIN *subwin )
{
  if( subwin->strBuf != NULL ){
		GFL_STR_DeleteBuffer( subwin->strBuf );
  }
  
  TALKMSGWIN_DeleteWindow( subwin->talkMsgWinSys, subwin->talkMsgWinIdx );
  GFL_HEAP_FreeMemory( subwin );
}

//--------------------------------------------------------------
/**
 * FLDSUBMSGWIN サブウィンドウ削除
 * @param tmsg FLDTALKMSGWIN
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSUBMSGWIN_Delete( FLDMSGBG *fmb, int id )
{
  FLDSUBMSGWIN *subwin = FldMsgBG_DeleteFldSubMsgWin( fmb, id );
  fldSubMsgWin_Delete( subwin );
}

//--------------------------------------------------------------
/**
 * FLDSUBMSGWIN サブウィンドウ全削除
 * @param
 * @retval
 */
//--------------------------------------------------------------
void FLDSUBMSGWIN_DeleteAll( FLDMSGBG *fmb )
{
  int i;
  for( i = 0; i < FLDSUBMSGWIN_MAX; i++ ){
    if( fmb->subMsgWinTbl[i] != NULL ){
      FLDSUBMSGWIN *subwin = fmb->subMsgWinTbl[i] = NULL;
      fmb->subMsgWinTbl[i] = NULL;
      fldSubMsgWin_Delete( subwin );
    }
  }
}

//--------------------------------------------------------------
/**
 * FLDSUBMSGWIN サブウィンドウが存在するかチェック
 * @param
 * @retval BOOL TRUE=存在する
 */
//--------------------------------------------------------------
BOOL FLDSUBMSGWIN_CheckExistWindow( FLDMSGBG *fmb )
{
  int i;
  for( i = 0; i < FLDSUBMSGWIN_MAX; i++ ){
    if( fmb->subMsgWinTbl[i] != NULL ){
      return( TRUE );
    }
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDSUBMSGWIN サブウィンドウ 表示
 * @param subwin FLDTALKMSGWIN
 * @retval BOOL TRUE=表示終了,FALSE=表示中
 */
//--------------------------------------------------------------
static BOOL fldSubMsgWin_Print( FLDSUBMSGWIN *subwin )
{
  int trg,cont;
  PRINTSTREAM_STATE state;
  PRINT_STREAM *stream;
  PRINTSTREAM_PAUSE_TYPE pause_type;
  
  if( TALKMSGWIN_CheckPrintOn(
        subwin->talkMsgWinSys,subwin->talkMsgWinIdx) == FALSE ){
    return( FALSE );
  }

  trg = GFL_UI_KEY_GetTrg();
  cont = GFL_UI_KEY_GetCont();
  stream = TALKMSGWIN_GetPrintStream(
      subwin->talkMsgWinSys, subwin->talkMsgWinIdx );
  state = PRINTSYS_PrintStreamGetState( stream );
  
  switch( state ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    break;
  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    break;
  case PRINTSTREAM_STATE_DONE: //終了
    return( TRUE );
  }
  
  return( FALSE );
}

//======================================================================
//  BGウィンドウ
//======================================================================
#define BGWIN_NCOL (0x0f)

//--------------------------------------------------------------
/**
 * フィールドBGウィンドウ　追加
 * @param fmb FLDMSGBG*
 * @param type FLDBGWIN_TYPE
 * @retval FLDBGWIN*
 */
//--------------------------------------------------------------
FLDBGWIN * FLDBGWIN_Add( FLDMSGBG *fmb, FLDBGWIN_TYPE type )
{
  FLDBGWIN *bgWin;
  
  bgWin = GFL_HEAP_AllocClearMemory( fmb->heapID, sizeof(FLDBGWIN) );
  bgWin->fmb = fmb;
  
  {
    bgWin->y = -48;
	  GFL_BG_SetScroll( bgWin->fmb->bgFrame, GFL_BG_SCROLL_X_SET, 0 );
	  GFL_BG_SetScroll( bgWin->fmb->bgFrame, GFL_BG_SCROLL_Y_SET, bgWin->y );
  }
  
  {
    GFL_BMP_DATA *bmp;

    bgWin->bmpwin = GFL_BMPWIN_Create( fmb->bgFrame,
        2, 19, 27, 4, PANO_FONT_TALKMSGWIN, GFL_BMP_CHRAREA_GET_B );
    bgWin->bmp_new = GFL_BMP_Create(
        27, 4, GFL_BMP_16_COLOR, fmb->heapID );
    bgWin->bmp_old = GFL_BMP_Create(
        27, 4, GFL_BMP_16_COLOR, fmb->heapID );
    
	  bmp = GFL_BMPWIN_GetBmp( bgWin->bmpwin );
	  GFL_BMP_Clear( bmp, BGWIN_NCOL );
	  GFL_BMPWIN_MakeScreen( bgWin->bmpwin );
	  GFL_BMPWIN_TransVramCharacter( bgWin->bmpwin );
    
    bgwin_InitGraphic(
      fmb->bgFrame, PANO_BGWIN, type, fmb->heapID );
    
    bgwin_WriteWindow( fmb->bgFrame, 
		  GFL_BMPWIN_GetPosX( bgWin->bmpwin ),
		  GFL_BMPWIN_GetPosY( bgWin->bmpwin ),
		  GFL_BMPWIN_GetSizeX( bgWin->bmpwin ),
		  GFL_BMPWIN_GetSizeY( bgWin->bmpwin ), CHARNO_OTHERSWIN, PANO_BGWIN );
    
		GFL_BG_LoadScreenReq( fmb->bgFrame );
  }
  
  keyCursor_Init( &bgWin->cursor_work, bgWin->fmb->heapID );
  
  fmb->deriveWin_plttNo = PANO_FONT_TALKMSGWIN;
  setBlendAlpha( FALSE );
  return( bgWin );
}

//--------------------------------------------------------------
/**
 * フィールドBGウィンドウ　削除
 * @param FLDBGWIN *bgWin
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDBGWIN_Delete( FLDBGWIN *bgWin )
{
	GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( bgWin->bmpwin );
  
  bgwin_CleanWindow( bgWin->fmb->bgFrame, 
		  GFL_BMPWIN_GetPosX( bgWin->bmpwin ),
		  GFL_BMPWIN_GetPosY( bgWin->bmpwin ),
		  GFL_BMPWIN_GetSizeX( bgWin->bmpwin ),
		  GFL_BMPWIN_GetSizeY( bgWin->bmpwin ), 0, BGWIN_NCOL );
  
	GFL_BMPWIN_ClearScreen( bgWin->bmpwin );
  
  GFL_BG_FillScreen( bgWin->fmb->bgFrame, 0,
		  GFL_BMPWIN_GetPosX( bgWin->bmpwin ),
      GFL_BMPWIN_GetPosY( bgWin->bmpwin ),
		  GFL_BMPWIN_GetSizeX( bgWin->bmpwin ), 
		  GFL_BMPWIN_GetSizeY( bgWin->bmpwin ), GFL_BG_SCRWRT_PALIN );
  
	GFL_BMPWIN_TransVramCharacter( bgWin->bmpwin );
	GFL_BMPWIN_Delete( bgWin->bmpwin );
  
	GFL_BG_LoadScreenReq( bgWin->fmb->bgFrame );
  GFL_BG_SetScroll( bgWin->fmb->bgFrame, GFL_BG_SCROLL_Y_SET, 0 );
  
  GFL_BMP_Delete( bgWin->bmp_old );
  GFL_BMP_Delete( bgWin->bmp_new );
  
  if( bgWin->strTemp != NULL ){
    GFL_STR_DeleteBuffer( bgWin->strTemp );
  }
  
  keyCursor_Delete( &bgWin->cursor_work );
  GFL_HEAP_FreeMemory( bgWin ); 
}

//--------------------------------------------------------------
/**
 * フィールドBGウィンドウ　プリント
 * @param FLDBGWIN *bgWin
 * @param	strID	メッセージデータ 文字列ID
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
BOOL FLDBGWIN_PrintStrBuf( FLDBGWIN *bgWin, const STRBUF *strBuf )
{
  switch( bgWin->seq_no ){
  case 0: //初期化
    bgWin->strBuf = strBuf;
#if 0
    bgWin->strTemp = GFL_STR_CreateCopyBuffer( strBuf, bgWin->fmb->heapID );
#else
	  bgWin->strTemp = GFL_STR_CreateBuffer( 
        GFL_STR_GetBufferLength(strBuf) + (EOM_CODESIZE*2),
        bgWin->fmb->heapID );
#endif
    
    bgwin_PrintStr(
        GFL_BMPWIN_GetBmp(bgWin->bmpwin), bgWin->fmb->fontHandle,
        bgWin->strBuf, bgWin->strTemp, &bgWin->line,
        BGWIN_NCOL );
    
    GFL_BMPWIN_TransVramCharacter( bgWin->bmpwin );
		GFL_BG_LoadScreenReq( bgWin->fmb->bgFrame );
    bgWin->seq_no++;
    break;
  case 1: //スクロール
    bgWin->y += BGWIN_SCROLL_SPEED;
    
    if( bgWin->y >= 0 ){
      bgWin->y = 0;
      bgWin->seq_no++;
    }
	  
    GFL_BG_SetScroll( bgWin->fmb->bgFrame, GFL_BG_SCROLL_Y_SET, bgWin->y );
    break;
  case 2: //終端チェック
    if( bgwin_CheckStrLineEOM(
          bgWin->strBuf,bgWin->strTemp,bgWin->line) == TRUE ){
      bgWin->seq_no = 5;
      break;
    }
    
    bgWin->seq_no++;
  case 3: //ページ送り待ち
    if( !(GFL_UI_KEY_GetTrg()&PAD_BUTTON_A) ){
      keyCursor_Write( &bgWin->cursor_work,
          GFL_BMPWIN_GetBmp(bgWin->bmpwin), BGWIN_NCOL );
		  GFL_BMPWIN_TransVramCharacter( bgWin->bmpwin );
      break;
    }
    
    keyCursor_Clear( &bgWin->cursor_work,
          GFL_BMPWIN_GetBmp(bgWin->bmpwin), BGWIN_NCOL );
    
    GFL_BMP_Copy( GFL_BMPWIN_GetBmp(bgWin->bmpwin), bgWin->bmp_old );
    
    bgwin_PrintStr(
        bgWin->bmp_new, bgWin->fmb->fontHandle,
        bgWin->strBuf, bgWin->strTemp, &bgWin->line,
        BGWIN_NCOL );
    bgWin->scroll_y = 0;
    
    bgWin->seq_no++;
  case 4: //スクロール
    if( bgwin_ScrollBmp(GFL_BMPWIN_GetBmp(bgWin->bmpwin),
          bgWin->bmp_old,bgWin->bmp_new,
          bgWin->scroll_y,BGWIN_NCOL) == TRUE ){
      bgWin->seq_no = 2;
    }else{
      bgWin->scroll_y += BGWIN_MSG_SCROLL_SPEED;
    }
    
	  GFL_BMPWIN_TransVramCharacter( bgWin->bmpwin );
	  GFL_BG_LoadScreenReq( bgWin->fmb->bgFrame );
    break;
  case 5: //終了へ　キー待ち
    if( !(GFL_UI_KEY_GetTrg() & (MSG_LAST_BTN|PAD_KEY_ALL)) ){
      break;
    }
    bgWin->seq_no++;
  case 6: //スクロール
    bgWin->y -= BGWIN_SCROLL_SPEED;
    
    if( bgWin->y <= -48 ){
      bgWin->y = -48;
    }
    
	  GFL_BG_SetScroll( bgWin->fmb->bgFrame, GFL_BG_SCROLL_Y_SET, bgWin->y );
    
    if( bgWin->y == -48 ){
      bgWin->seq_no++;
      return( TRUE );
    }  
    break;
  case 7:
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　グラフィック初期化
 * @param bgframe BGフレーム
 * @param plttno パレットナンバー
 * @param type wind
 * @retval nothing
 */
//--------------------------------------------------------------
static void bgwin_InitGraphic(
    u32 bgframe, u32 plttno, u32 type, HEAPID heapID )
{
  if( type >= FLDBGWIN_TYPE_MAX ){
    GF_ASSERT( 0 );
    type = FLDBGWIN_TYPE_INFO;
  }
  
  GFL_ARC_UTIL_TransVramBgCharacter(
      ARCID_FLDMAP_WINFRAME, NARC_winframe_bgwin_NCGR,
      bgframe, CHARNO_OTHERSWIN, 0, FALSE, heapID );
  
  GFL_ARC_UTIL_TransVramPaletteEx(
      ARCID_FLDMAP_WINFRAME, NARC_winframe_bgwin_NCLR,
      PALTYPE_MAIN_BG, type*0x20, plttno*0x20, 0x20, heapID );
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　描画
 * @param frm BGフレーム
 * @param px 左上X座標　キャラ単位
 * @param py 左上Y座標　キャラ単位
 * @param sx 横幅　キャラ単位
 * @param sy 縦幅　キャラ単位
 * @param cgx 書き込む先頭キャラナンバー
 * @param pal パレット
 * @retval nothing
 */
//--------------------------------------------------------------
static void bgwin_WriteWindow(
    u8 frm, u8 px, u8 py, u8 sx, u8 sy, u16 cgx, u8 pal )
{
	GFL_BG_FillScreen( frm, cgx, px-2, py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+1, px-1, py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+2, px, py-1, sx, 1, pal );
	GFL_BG_FillScreen( frm, cgx+3, px+sx,	py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+4, px+sx+1, py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+5, px+sx+2, py-1, 1, 1, pal );
  
	GFL_BG_FillScreen( frm, cgx+6, px-2, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx+7, px-1, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx+9, px+sx, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx+10, px+sx+1, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx+11, px+sx+2, py, 1, sy, pal );
  
	GFL_BG_FillScreen( frm, cgx+12, px-2, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+13, px-1, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+14, px, py+sy, sx, 1, pal );
	GFL_BG_FillScreen( frm, cgx+15, px+sx, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+16, px+sx+1, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+17, px+sx+2, py+sy, 1, 1, pal );
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　消去
 * @param frm BGフレーム
 * @param px 左上X座標　キャラ単位
 * @param py 左上Y座標　キャラ単位
 * @param sx 横幅　キャラ単位
 * @param sy 縦幅　キャラ単位
 * @param cgx 書き込むキャラナンバー
 * @param pal パレット
 * @retval nothing
 */
//--------------------------------------------------------------
static void bgwin_CleanWindow(
    u8 frm, u8 px, u8 py, u8 sx, u8 sy, u16 cgx, u8 pal )
{
	GFL_BG_FillScreen( frm, cgx, px-2, py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px-1, py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px, py-1, sx, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px+sx,	py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px+sx+1, py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px+sx+2, py-1, 1, 1, pal );
  
	GFL_BG_FillScreen( frm, cgx, px-2, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx, px-1, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx, px+sx, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx, px+sx+1, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx, px+sx+2, py, 1, sy, pal );
  
	GFL_BG_FillScreen( frm, cgx, px-2, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px-1, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px, py+sy, sx, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px+sx, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px+sx+1, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx, px+sx+2, py+sy, 1, 1, pal );
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　終端行チェック
 * @param str_src 文字列ソース
 * @param str_temp チェック用テンポラリ
 * @param line チェック行
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL bgwin_CheckStrLineEOM(
    const STRBUF *str_src, STRBUF *str_temp, u32 line )
{
  if( PRINTSYS_CopyLineStr(str_src,str_temp,line) == FALSE ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　BMPに一面分のフォント書き込み
 * @param bmp 書き込み先ビットマップ
 * @param font GFL_FONT
 * @param str_src ソース文字列
 * @param str_temp 書き込み用テンポラリ
 * @param line 書き込み行
 * @param col カラーナンバー
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL bgwin_PrintStr(
    GFL_BMP_DATA *bmp, GFL_FONT *font,
    const STRBUF *str_src, STRBUF *str_temp, u32 *line, u8 col )
{
  int x = 1, y = 1;
  GFL_BMP_Clear( bmp, col );
  
  if( PRINTSYS_CopyLineStr(str_src,str_temp,*line) == FALSE ){
    return( TRUE );
  }
  
  PRINTSYS_Print( bmp, x, y, str_temp, font );
  
  y += PRINTSYS_GetStrHeight(str_temp, font ) + 1;
  (*line)++;
  
  if( PRINTSYS_CopyLineStr(str_src,str_temp,*line) == FALSE ){
    return( TRUE );
  }
  
  PRINTSYS_Print( bmp, x, y, str_temp, font );
  (*line)++;
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　BMPスクロール
 * @param bmp 反映先ビットマップ
 * @param old スクロールして消すビットマップ
 * @param new スクロールして新規に出すビットマップ
 * @param y　スクロール値
 * @param n_col カラーナンバー
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL bgwin_ScrollBmp(
    GFL_BMP_DATA *bmp, GFL_BMP_DATA *old, GFL_BMP_DATA *new,
    int y, u16 n_col )
{
  int sizeX = GFL_BMP_GetSizeX( bmp );
  int sizeY = GFL_BMP_GetSizeY( bmp );
  
	GFL_BMP_Clear( bmp, n_col );
  
  if( y > sizeY ){
    y = sizeY;
  }

  GFL_BMP_Print( old, bmp,
        0, y,
        0, 0,
        sizeX, sizeY-y,
        n_col );
  
  GFL_BMP_Print( new, bmp,
        0, 0,
        0, sizeY-y,
        sizeX, y,
        n_col );
  
  if( y >= sizeY ){
    return( TRUE );
  }
  
  return( FALSE );
}

//======================================================================
//  特殊ウィンドウ 
//======================================================================
#define SPWIN_CHROFFS_SPACE (9)

//--------------------------------------------------------------
/**
 * 特殊ウィンドウ　グラフィック初期化
 * @param bgframe BGフレーム
 * @param plttno パレットナンバー
 * @param type wind
 * @retval  nothing
 */
//--------------------------------------------------------------
static void spwin_InitPalette(
    u32 bgframe, u32 plttno, HEAPID heapID )
{
  GFL_ARC_UTIL_TransVramPaletteEx(
      ARCID_FLDMAP_WINFRAME, NARC_winframe_spwin_NCLR,
      PALTYPE_MAIN_BG, 0, plttno*0x20, 0x20, heapID );
}

//--------------------------------------------------------------
/**
 * 特殊ウィンドウ 背景キャラビットマップ作成
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void spwin_CreateBmpBG(
    GFL_BMP_DATA *bmp_bg,
    FLDSPWIN_TYPE type, HEAPID heapID, u32 charOffs )
{
  u8 *buf;
  void *pData;
	NNSG2dCharacterData *pCharData;
  
  if( type >= FLDSPWIN_TYPE_MAX ){
    GF_ASSERT( 0 );
    type = FLDSPWIN_TYPE_LETTER;
  }
  
  type = FLDSPWIN_TYPE_LETTER; //現状、絵が一種類しかないので
  
  buf = GFL_BMP_GetCharacterAdrs( bmp_bg );
  
  pData = GFL_ARC_UTIL_Load(
      ARCID_FLDMAP_WINFRAME, 
      NARC_winframe_spwin_NCGR, 0, heapID );
  
  charOffs *= 0x20;

	NNS_G2dGetUnpackedBGCharacterData( pData, &pCharData );
  MI_CpuCopy( (u8*)(pCharData->pRawData)+(0x20*type)+charOffs, buf, 0x20 );
  GFL_HEAP_FreeMemory( pData );
}

//--------------------------------------------------------------
/**
 * 特殊ウィンドウ　BMP背景書き込み
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void spwin_WriteBmpBG(
    GFL_BMP_DATA *bmp, FLDSPWIN_TYPE type, HEAPID heapID )
{
  u16 x,y;
  GFL_BMP_DATA *bmp_bg;
  u16 size_x = GFL_BMP_GetSizeX( bmp ) >> 3;
  u16 size_y = GFL_BMP_GetSizeY( bmp ) >> 3;

  KAGAYA_Printf( "size x = %d, y = %d\n", size_x, size_y );
  bmp_bg = GFL_BMP_Create( 1, 1, GFL_BMP_16_COLOR, heapID );
  
  spwin_CreateBmpBG( bmp_bg, type, heapID, SPWIN_CHROFFS_SPACE );
  
  for( y = 0; y < size_y; y++ ){
    for( x = 0; x < size_x; x++ ){
      GFL_BMP_Print( bmp_bg, bmp,
          0, 0, x<<3, y<<3, 8, 8, GF_BMPPRT_NOTNUKI );
    }
  }
  
  //top
  spwin_CreateBmpBG( bmp_bg, type, heapID, 1 );
  
  for( x = 0, y = 0; x < size_x; x++ ){
    GFL_BMP_Print( bmp_bg, bmp,
      0, 0, x<<3, y<<3, 8, 8, GF_BMPPRT_NOTNUKI );
  }

  //bottom
  spwin_CreateBmpBG( bmp_bg, type, heapID, 17 );
  
  for( x = 0, y = size_y - 1; x < size_x; x++ ){
    GFL_BMP_Print( bmp_bg, bmp,
      0, 0, x<<3, y<<3, 8, 8, GF_BMPPRT_NOTNUKI );
  }

  //left
  spwin_CreateBmpBG( bmp_bg, type, heapID, 8 );
  
  for( x = 0, y = 0; y < size_y; y++ ){
    GFL_BMP_Print( bmp_bg, bmp,
      0, 0, x<<3, y<<3, 8, 8, GF_BMPPRT_NOTNUKI );
  }

  //right
  spwin_CreateBmpBG( bmp_bg, type, heapID, 10 );
  
  for( x = size_x - 1, y = 0; y < size_y; y++ ){
    GFL_BMP_Print( bmp_bg, bmp,
      0, 0, x<<3, y<<3, 8, 8, GF_BMPPRT_NOTNUKI );
  }
  
  //left top
  x = 0;
  y = 0;
  spwin_CreateBmpBG( bmp_bg, type, heapID, 0 );
  GFL_BMP_Print( bmp_bg, bmp, 0, 0, x<<3, y<<3, 8, 8, GF_BMPPRT_NOTNUKI );
  
  //right top
  x = size_x - 1;
  y = 0;
  spwin_CreateBmpBG( bmp_bg, type, heapID, 2 );
  GFL_BMP_Print( bmp_bg, bmp, 0, 0, x<<3, y<<3, 8, 8, GF_BMPPRT_NOTNUKI );

  //left bottom
  x = 0;
  y = size_y - 1;
  spwin_CreateBmpBG( bmp_bg, type, heapID, 16 );
  GFL_BMP_Print( bmp_bg, bmp, 0, 0, x<<3, y<<3, 8, 8, GF_BMPPRT_NOTNUKI );

  //right bottom
  x = size_x - 1;
  y = size_y - 1;
  spwin_CreateBmpBG( bmp_bg, type, heapID, 18 );
  GFL_BMP_Print( bmp_bg, bmp, 0, 0, x<<3, y<<3, 8, 8, GF_BMPPRT_NOTNUKI );
  
  GFL_BMP_Delete( bmp_bg );
}

//--------------------------------------------------------------
/**
 * 特殊ウィンドウ　文字列書き込み
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void spwin_Print( u16 x, u16 y,
    GFL_BMP_DATA *bmp, GFL_FONT *font, const STRBUF *strbuf, HEAPID heapID )
{
  STRBUF *buf;
  int line = 0;
  int height = GFL_FONT_GetLineHeight( font ) + SPWIN_HEIGHT_FONT_SPACE;
  
  line = GFL_STR_GetBufferLength( strbuf ) + 1;
  buf = GFL_STR_CreateBuffer( line, heapID );
  
  line = 0;
  
  while( PRINTSYS_CopyLineStr(strbuf,buf,line) == TRUE ){
    KAGAYA_Printf( "Print x = %d, y = %d, line = %d\n", x, y, line );
    PRINTSYS_Print( bmp, x, y, buf, font );
    y += height;
    line++;
  }
  
  GFL_STR_DeleteBuffer( buf );
}

//--------------------------------------------------------------
/**
 * FDLSPWIN 特殊ウィンドウ　初期化
 * @param fmb FLDMSGBG*
 * @param	bmppos_x		//表示座標X キャラ単位
 * @param	bmppos_y		//表示座標Y キャラ単位
 * @param	bmpsize_x		//表示サイズX キャラ単位
 * @param	bmpsize_y		//表示サイズY キャラ単位
 * @retval FLDSPWIN*
 */
//--------------------------------------------------------------
FLDSPWIN * FLDSPWIN_Add( FLDMSGBG *fmb, FLDSPWIN_TYPE type,
	u16 bmppos_x, u16 bmppos_y, u16 bmpsize_x, u16 bmpsize_y )
{
  FLDSPWIN *spWin;
  
  KAGAYA_Printf( "SPWIN x = %d, y = %d, w = %d, h = %d, type = %d\n",
    bmppos_x, bmppos_y, bmpsize_x, bmpsize_y, type );
  
  spWin = GFL_HEAP_AllocClearMemory( fmb->heapID, sizeof(FLDSPWIN) );
  spWin->fmb = fmb;
  
  spWin->bmpwin = GFL_BMPWIN_Create( fmb->bgFrame,
      bmppos_x, bmppos_y, bmpsize_x, bmpsize_y,
      PANO_SPWIN, GFL_BMP_CHRAREA_GET_B );
  
  spWin->bmp_bg = GFL_BMP_Create( 1, 1, GFL_BMP_16_COLOR, fmb->heapID );
  spwin_CreateBmpBG( spWin->bmp_bg, type, fmb->heapID, SPWIN_CHROFFS_SPACE );
  spwin_WriteBmpBG( GFL_BMPWIN_GetBmp(spWin->bmpwin), type, fmb->heapID );
  spwin_InitPalette( fmb->bgFrame, PANO_SPWIN, fmb->heapID );
  
  GFL_BMPWIN_MakeScreen( spWin->bmpwin );
	GFL_BMPWIN_TransVramCharacter( spWin->bmpwin );
	GFL_BG_LoadScreenReq( fmb->bgFrame );
  
  keyCursor_Init( &spWin->cursor_work, fmb->heapID );

  fmb->deriveWin_plttNo = PANO_FONT_TALKMSGWIN;
  setBlendAlpha( FALSE );
  return( spWin );
}

//--------------------------------------------------------------
/**
 * FDLSPWIN 特殊ウィンドウ　削除
 * @param fmb FLDMSGBG*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSPWIN_Delete( FLDSPWIN *spWin )
{
	GFL_BMPWIN_ClearScreen( spWin->bmpwin );
  GFL_BG_FillScreen( spWin->fmb->bgFrame, 0,
		  GFL_BMPWIN_GetPosX( spWin->bmpwin ),
      GFL_BMPWIN_GetPosY( spWin->bmpwin ),
		  GFL_BMPWIN_GetSizeX( spWin->bmpwin ), 
		  GFL_BMPWIN_GetSizeY( spWin->bmpwin ), GFL_BG_SCRWRT_PALIN );
	GFL_BMPWIN_TransVramCharacter( spWin->bmpwin );
	GFL_BG_LoadScreenReq( spWin->fmb->bgFrame );
  
  keyCursor_Delete( &spWin->cursor_work );

  GFL_BMP_Delete( spWin->bmp_bg );
  GFL_BMPWIN_Delete( spWin->bmpwin );
  GFL_HEAP_FreeMemory( spWin );
}

//--------------------------------------------------------------
/**
 * FLDSPWIN 特殊ウィンドウ メッセージ表示開始 STRBUF指定
 * @param msgWin FLDMSGWIN_STREAM*
 * @param	x		X表示座標
 * @param	y		Y表示座標
 * @param strBuf 表示するSTRBUF
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDSPWIN_PrintStrBufStart(
    FLDSPWIN *spWin, u16 x, u16 y, const STRBUF *strBuf )
{
  spwin_Print( x, y,
      GFL_BMPWIN_GetBmp(spWin->bmpwin),
      spWin->fmb->fontHandle,
      strBuf,
      spWin->fmb->heapID );
  
  GFL_BMPWIN_TransVramCharacter( spWin->bmpwin );
  GFL_BG_LoadScreenReq( spWin->fmb->bgFrame );
}

//--------------------------------------------------------------
/**
 * FLDSPWIN 特殊ウィンドウ メッセージ表示
 * @param msgWin FLDMSGWIN_STREAM*
 * @retval BOOL TRUE=表示終了,FALSE=表示中
 */
//--------------------------------------------------------------
BOOL FLDSPWIN_Print( FLDSPWIN *spWin )
{
  int trg,cont;
  PRINTSTREAM_STATE state;
  
  trg = GFL_UI_KEY_GetTrg();
  cont = GFL_UI_KEY_GetCont();
  
  if( (trg & MSG_LAST_BTN) ){
    return( TRUE );
  }
  
  keyCursor_WriteBmpBG( &spWin->cursor_work,
      GFL_BMPWIN_GetBmp(spWin->bmpwin), spWin->bmp_bg );
	GFL_BMPWIN_TransVramCharacter( spWin->bmpwin );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDSPWIN 特殊ウィンドウ メッセージから必要なウィンドウ横幅を取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
u32 FLDSPWIN_GetNeedWindowWidthCharaSize(
    FLDMSGBG *fmb, const STRBUF *strbuf, u32 margin )
{
  u32 w = PRINTSYS_GetStrWidth(strbuf,fmb->fontHandle,0) + (margin<<1);
  u32 c = w;
  w >>= 3;
  if( (c&0x07) ){
    w++;
  }
  return( w );
}

//--------------------------------------------------------------
/**
 * FLDSPWIN 特殊ウィンドウ メッセージから必要なウィンドウ縦幅を取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
u32 FLDSPWIN_GetNeedWindowHeightCharaSize(
    FLDMSGBG *fmb, const STRBUF *strbuf, u32 margin )
{
  u32 line;
  u32 h = PRINTSYS_GetStrHeight(strbuf,fmb->fontHandle) + (margin<<1);
  STRBUF *tmpbuf = GFL_STR_CreateCopyBuffer( strbuf, fmb->heapID );
  
  line = 0;
  while( PRINTSYS_CopyLineStr(strbuf,tmpbuf,line) == TRUE ){
    line++;
    if( line >= 24 ){
      break;
    }
  }
  OS_Printf( "界行数　系%d\n", line );
  line--;

  GFL_STR_DeleteBuffer( tmpbuf );
  
  line *= SPWIN_HEIGHT_FONT_SPACE;
  h += line;
  
  if( (h&0x07) ){
    h += 8;
  }
  
  h >>= 3;
  return( h );
}

//======================================================================
//  キー送りカーソル
//======================================================================
//--------------------------------------------------------------
/**
 * キー送りカーソル 初期化
 * @param work KEYCURSOR_WORK
 * @param heapID HEAPID
 * @retval nothing
 */
//--------------------------------------------------------------
static void keyCursor_Init( KEYCURSOR_WORK *work, HEAPID heapID )
{
  MI_CpuClear8( work, sizeof(KEYCURSOR_WORK) );
  work->bmp_cursor = GFL_BMP_CreateWithData(
        (u8*)skip_cursor_Character,
        2, 2, GFL_BMP_16_COLOR, heapID );
}

//--------------------------------------------------------------
/**
 * キー送りカーソル 削除
 * @param work KEYCURSOR_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void keyCursor_Delete( KEYCURSOR_WORK *work )
{
  GFL_BMP_Delete( work->bmp_cursor );
}

//--------------------------------------------------------------
/**
 * キー送りカーソル クリア
 * @param work KEYCURSOR_WORK
 * @param bmp 表示先GFL_BMP_DATA
 * @param n_col 透明色指定 0-15,GF_BMPPRT_NOTNUKI	
 * @retval nothing
 */
//--------------------------------------------------------------
static void keyCursor_Clear(
    KEYCURSOR_WORK *work, GFL_BMP_DATA *bmp, u16 n_col )
{
  u16 x,y,offs;
  u16 tbl[3] = { 0, 1, 2 };
  
  x = GFL_BMP_GetSizeX( bmp ) - 11;
  y = GFL_BMP_GetSizeY( bmp ) - 9;
  offs = tbl[work->cursor_anm_no];
  GFL_BMP_Fill( bmp, x, y+offs, 10, 7, n_col );
}

//--------------------------------------------------------------
/**
 * キー送りカーソル 表示
 * @param work KEYCURSOR_WORK
 * @param bmp 表示先GFL_BMP_DATA
 * @param n_col 透明色指定 0-15,GF_BMPPRT_NOTNUKI	
 * @retval nothing
 */
//--------------------------------------------------------------
static void keyCursor_Write(
    KEYCURSOR_WORK *work, GFL_BMP_DATA *bmp, u16 n_col )
{
  u16 x,y,offs;
  u16 tbl[3] = { 0, 1, 2 };
  
  keyCursor_Clear( work, bmp, n_col );
  
  work->cursor_anm_frame++;
  
  if( work->cursor_anm_frame >= 4 ){
    work->cursor_anm_frame = 0;
    work->cursor_anm_no++;
    work->cursor_anm_no %= 3;
  }

  x = GFL_BMP_GetSizeX( bmp ) - 11;
  y = GFL_BMP_GetSizeY( bmp ) - 9;
  offs = tbl[work->cursor_anm_no];
  
  GFL_BMP_Print( work->bmp_cursor, bmp, 0, 2, x, y+offs, 10, 7, 0x00 );
}

//--------------------------------------------------------------
/**
 * キー送りカーソル 表示　背景BITMAP指定
 * @param work KEYCURSOR_WORK
 * @param bmp 表示先GFL_BMP_DATA
 * @param bmp_bg 背景に張る1キャラ分のGFL_BMP_DATA
 * @retval nothing
 */
//--------------------------------------------------------------
static void keyCursor_WriteBmpBG(
    KEYCURSOR_WORK *work, GFL_BMP_DATA *bmp,
    GFL_BMP_DATA *bmp_bg )
{
  s16 x,y,offs;
  u16 tbl[3] = { 0, 1, 2 };
  
  x = GFL_BMP_GetSizeX( bmp ) - (8 - 16);
  y = GFL_BMP_GetSizeY( bmp ) - (8 - 16);
  
  if( x >= 0 && y >= 0 ){ //BGを張る領域がある
    s16 ix,iy;
    for( iy = y; iy < (y+16); iy += 4 ){
      for( ix = x; ix < (x+16); ix += 4 ){
        GFL_BMP_Print( bmp_bg, bmp, 0, 0, ix, iy, 4, 4, GF_BMPPRT_NOTNUKI );
      }
    }
  }
  
  work->cursor_anm_frame++;
  
  if( work->cursor_anm_frame >= 4 ){
    work->cursor_anm_frame = 0;
    work->cursor_anm_no++;
    work->cursor_anm_no %= 3;
  }
  
  x = GFL_BMP_GetSizeX( bmp ) - (8 - 10);
  y = GFL_BMP_GetSizeY( bmp ) - (8 - 7 - 3);
  offs = tbl[work->cursor_anm_no];
  
  GFL_BMP_Print( work->bmp_cursor, bmp, 0, 2, x, y+offs, 10, 7, 0x00 );
}

//======================================================================
//  システムウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * システムウィンドウ　グラフィック初期化
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void syswin_InitGraphic( HEAPID heapID )
{
  GFL_ARC_UTIL_TransVramBgCharacter(
    ARCID_FLDMAP_WINFRAME, NARC_winframe_syswin_NCGR,
    FLDMSGBG_BGFRAME, CHARNO_SYSWIN, 0, FALSE, heapID );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　描画
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void syswin_WriteWindow( const GFL_BMPWIN *bmpwin )
{
  u16 cgx = CHARNO_SYSWIN;
  u8 px = GFL_BMPWIN_GetPosX( bmpwin );
  u8 py = GFL_BMPWIN_GetPosY( bmpwin );
	u8 sx = GFL_BMPWIN_GetSizeX( bmpwin );
	u8 sy = GFL_BMPWIN_GetSizeY( bmpwin );
  u8 pal = PANO_MENU;
  u8 frm = FLDMSGBG_BGFRAME;
  
	GFL_BG_FillScreen( frm, cgx, px-2, py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+1, px-1, py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+2, px, py-1, sx, 1, pal );
	GFL_BG_FillScreen( frm, cgx+3, px+sx,	py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+4, px+sx+1, py-1, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+5, px+sx+2, py-1, 1, 1, pal );
  
	GFL_BG_FillScreen( frm, cgx+6, px-2, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx+7, px-1, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx+9, px+sx, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx+10, px+sx+1, py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx+11, px+sx+2, py, 1, sy, pal );
  
	GFL_BG_FillScreen( frm, cgx+12, px-2, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+13, px-1, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+14, px, py+sy, sx, 1, pal );
	GFL_BG_FillScreen( frm, cgx+15, px+sx, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+16, px+sx+1, py+sy, 1, 1, pal );
	GFL_BG_FillScreen( frm, cgx+17, px+sx+2, py+sy, 1, 1, pal );
  
  GFL_BG_LoadScreenReq( frm );
}

//--------------------------------------------------------------
/**
 * システムウィンドウ　ビットマップウィンドウフレーム　初期化
 * @param	bgframe	BGフレーム
 * @param	heapID	HEAPID
 * @param	pos_y	ビットマップ左上Y キャラ単位
 * @retval	GFL_BMPWIN	作成されたGFL_BMPWIN
 */
//--------------------------------------------------------------
static GFL_BMPWIN * syswin_InitBmp( u8 pos_y, HEAPID heapID )
{
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;
  
	bmpwin = GFL_BMPWIN_Create( FLDMSGBG_BGFRAME,
		2, pos_y, 27, 4,
		PANO_FONT, GFL_BMP_CHRAREA_GET_B );
  
	bmp = GFL_BMPWIN_GetBmp( bmpwin );
	
	GFL_BMP_Clear( bmp, 0xff );
	GFL_BMPWIN_MakeScreen( bmpwin );
	GFL_BMPWIN_TransVramCharacter( bmpwin );
  syswin_WriteWindow( bmpwin );

	return( bmpwin );
}

//--------------------------------------------------------------
/**
 * ビットマップウィンドウフレーム　クリア
 * @param	bmpwin	syswin_InitBmp()の戻り値
 * @retval	nothing
 */
//--------------------------------------------------------------
static void syswin_ClearBmp( GFL_BMPWIN *bmpwin )
{
	u8 frm = GFL_BMPWIN_GetFrame( bmpwin );
  
	GFL_BG_FillScreen(
		frm, CHARNO_CLEAR,
		GFL_BMPWIN_GetPosX( bmpwin ) - 2,
		GFL_BMPWIN_GetPosY( bmpwin ) - 1,
		GFL_BMPWIN_GetSizeX( bmpwin ) + 3 + 2,
		GFL_BMPWIN_GetSizeY( bmpwin ) + 2,
		0 );
  
  GFL_BG_LoadScreenReq( frm );
}

//--------------------------------------------------------------
/**
 * ビットマップウィンドウフレーム　削除
 * @param	bmpwin	syswin_InitBmp()の戻り値
 * @retval	nothing
 */
//--------------------------------------------------------------
static void syswin_DeleteBmp( GFL_BMPWIN *bmpwin )
{
  syswin_ClearBmp( bmpwin );
  GFL_BMPWIN_Delete( bmpwin );
}

//======================================================================
//  その他
//======================================================================
//--------------------------------------------------------------
/**
 * BGリソース初期化
 * @param fmb FLDMSGBG*
 * @retval nothing
 */
//--------------------------------------------------------------
static void setBGResource( FLDMSGBG *fmb )
{
	{	//BG初期化
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, FLDBG_MFRM_MSG_SCRSIZE, 0,
			GFL_BG_SCRSIZ_256x256, FLDBG_MFRM_MSG_COLORMODE,
			FLDBG_MFRM_MSG_SCRBASE, FLDBG_MFRM_MSG_CHARBASE, FLDBG_MFRM_MSG_CHARSIZE,
			GX_BG_EXTPLTT_01, FLDBG_MFRM_MSG_PRI, 0, 0, FALSE
		};
		
		GFL_BG_SetBGControl( fmb->bgFrame, &bgcntText, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible( fmb->bgFrame, VISIBLE_ON );
		
		GFL_BG_SetPriority( GFL_BG_FRAME0_M, FLDBG_MFRM_3D_PRI );
		
		GFL_BG_FillCharacter( fmb->bgFrame, CHARNO_CLEAR, 1, 0 );
		GFL_BG_FillScreen( fmb->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		
		GFL_BG_LoadScreenReq( fmb->bgFrame );
	}
	
	{	//フォントパレット
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_FONT, NARC_font_default_nclr, //黒
			PALTYPE_MAIN_BG, PANO_FONT_TALKMSGWIN*32, 32, fmb->heapID );
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_FONT, NARC_font_systemwin_nclr, //白
			PALTYPE_MAIN_BG, PANO_FONT*32, 32, fmb->heapID );
	}
	
	{	//window frame
		BmpWinFrame_GraphicSet( fmb->bgFrame, CHARNO_WINFRAME,
			PANO_MENU, MENU_TYPE_SYSTEM, fmb->heapID );
	}
  
  { //TALKMSGWIN
    if( fmb->g3Dcamera != NULL ){
      TALKMSGWIN_SYS_SETUP setup;
      setup.heapID = fmb->heapID;
      setup.g3Dcamera = fmb->g3Dcamera;
      setup.fontHandle = fmb->fontHandle;
      setup.chrNumOffs = CHARNO_BALLOONWIN;
      setup.ini.frameID = FLDMSGBG_BGFRAME;
      setup.ini.winPltID = PANO_TALKMSGWIN;
      setup.ini.fontPltID = PANO_FONT_TALKMSGWIN;
      fmb->talkMsgWinSys = TALKMSGWIN_SystemCreate( &setup );
    }
  }
	
  { //SYSWIN
    syswin_InitGraphic( fmb->heapID );
  }
  
  fmb->deriveWin_plttNo = PANO_FONT;
}

//--------------------------------------------------------------
/**
 * フィールドメッセージBG ブレンドアルファセット
 * @param on TRUE=半透明オン FALSE=半透明オフ
 * @retval nothing
 */
//--------------------------------------------------------------
static void setBlendAlpha( BOOL on )
{
  if( on == TRUE ){
    int plane1 = GX_BLEND_PLANEMASK_BG1; 
  	int plane2 = GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG2|
      GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ;
  	G2_SetBlendAlpha( plane1, plane2, 31, 8 );
  }else{
    int plane1 = GX_BLEND_PLANEMASK_NONE; 
    int plane2 = GX_BLEND_PLANEMASK_NONE; 
  	G2_SetBlendAlpha( plane1, plane2, 0, 0 );
  }
}

//--------------------------------------------------------------
/**
 * フィールドメッセージBG ブレンドアルファセット
 * @param set TRUE=半透明ON
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_SetBlendAlpha( BOOL set )
{
  setBlendAlpha( set );
}

//--------------------------------------------------------------
/**
 * キャラスクリーンデータ復帰
 * @param 
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMSGBG_RecoveryBG( FLDMSGBG *fmb )
{
  HEAPID heapID = fmb->heapID;
  
  {
    GFL_BG_FillCharacterRelease( fmb->bgFrame, 1, 0 );
		GFL_BG_FillCharacter( fmb->bgFrame, 0x00, 1, 0 );
		GFL_BG_FillScreen( fmb->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		
		GFL_BG_LoadScreenReq( fmb->bgFrame );
	}
	
	{	//フォントパレット
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_FONT, NARC_font_default_nclr, //黒
			PALTYPE_MAIN_BG, PANO_FONT_TALKMSGWIN*32, 32, fmb->heapID );
		GFL_ARC_UTIL_TransVramPalette(
			ARCID_FONT, NARC_font_systemwin_nclr, //白
			PALTYPE_MAIN_BG, PANO_FONT*32, 32, fmb->heapID );
	}
	
	{	//window frame
		BmpWinFrame_GraphicSet( fmb->bgFrame, CHARNO_WINFRAME,
			PANO_MENU, MENU_TYPE_SYSTEM, heapID );
	}

  { //バルーンウィンドウ
    TALKMSGWIN_ReTransWindowBG( fmb->talkMsgWinSys );
  }

  { //SYSWIN
    syswin_InitGraphic( heapID );
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
static GFL_BMPWIN * winframe_InitBmp( u32 bgFrame, HEAPID heapID,
	u16 pos_x, u16 pos_y, u16 size_x, u16 size_y, u16 pltt_no )
{
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;

	bmpwin = GFL_BMPWIN_Create( bgFrame,
		pos_x, pos_y, size_x, size_y,
//		PANO_FONT, GFL_BMP_CHRAREA_GET_B );
		pltt_no, GFL_BMP_CHRAREA_GET_B );

	bmp = GFL_BMPWIN_GetBmp( bmpwin );
	
	GFL_BMP_Clear( bmp, 0xff );
	GFL_BMPWIN_MakeScreen( bmpwin );
	GFL_BMPWIN_TransVramCharacter( bmpwin );
	GFL_BG_LoadScreenReq( bgFrame );
	
	BmpWinFrame_Write( bmpwin, WINDOW_TRANS_ON, 1, PANO_MENU );
	
	return( bmpwin );
}

//--------------------------------------------------------------
/**
 * ビットマップウィンドウフレーム　削除
 * @param	bmpwin	winframe_InitBmp()の戻り値
 * @retval	nothing
 */
//--------------------------------------------------------------
static void winframe_DeleteBmp( GFL_BMPWIN *bmpwin )
{
	BmpWinFrame_Clear( bmpwin, 0 );
	GFL_BMPWIN_Delete( bmpwin );
}

//--------------------------------------------------------------
/**
 * ビットマップウィンドウフレーム　黒背景パレットセット
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
static void winframe_SetPaletteBlack( u32 heapID )
{
  u32 pal = PANO_MENU;
  u32 arc = BmpWinFrame_WinPalArcGet();
  GFL_ARC_UTIL_TransVramPaletteEx(
      ARCID_FLDMAP_WINFRAME,
      arc, PALTYPE_MAIN_BG, 0x20*1, pal*0x20, 0x20, heapID );
}

//--------------------------------------------------------------
/**
 * ビットマップウィンドウフレーム　白背景パレットセット
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
static void winframe_SetPaletteWhith( u32 heapID )
{
  u32 pal = PANO_MENU;
  u32 arc = BmpWinFrame_WinPalArcGet();
  GFL_ARC_UTIL_TransVramPaletteEx(
      ARCID_FLDMAP_WINFRAME,
      arc, PALTYPE_MAIN_BG, 0x20*0, pal*0x20, 0x20, heapID );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG FLDSUBMSGWIN登録
 * @param fmb FLDMSGBG
 * @param subwin 登録するFLDSUBMSGWIN
 * @retval int ウィンドウインデックス
 */
//--------------------------------------------------------------
static int FldMsgBG_SetFldSubMsgWin( FLDMSGBG *fmb, FLDSUBMSGWIN *subwin )
{
  int i;
  for( i = 0; i < FLDSUBMSGWIN_MAX; i++ ){
    if( fmb->subMsgWinTbl[i] == NULL ){
      fmb->subMsgWinTbl[i] = subwin;
      return( i );
    }
  }
  
  GF_ASSERT( 0 );
  return( i );
}

//--------------------------------------------------------------
/**
 * FLDMSGBG FLDSUBMSGWIN削除
 * @param fmb FLDMSGBG
 * @param id 登録していたウィンドウID
 * @retval FLDSUBMSGWIN idで使用していたFLDSUBMSGWIN
 */
//--------------------------------------------------------------
static FLDSUBMSGWIN * FldMsgBG_DeleteFldSubMsgWin( FLDMSGBG *fmb, int id )
{
  int i;
  for( i = 0; i < FLDSUBMSGWIN_MAX; i++ ){
    if( fmb->subMsgWinTbl[i] != NULL && fmb->subMsgWinTbl[i]->id == id ){
      FLDSUBMSGWIN *subwin = fmb->subMsgWinTbl[i];
      fmb->subMsgWinTbl[i] = NULL;
      return( subwin );
    }
  }
  
  GF_ASSERT( 0 );
  return( NULL );
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

//--------------------------------------------------------------
/// 送りカーソルデータ 仮　PLから持ってきた
//--------------------------------------------------------------
#if 0
static const u8 ALIGN4 skip_cursor_Character[128] = {
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
static const u8 ALIGN4 skip_cursor_Character[128] = {
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

/*
パレット 0=300H
パレット 1=7fffH
パレット 2=62f5H
パレット 3=3d89H
パレット 4=3126H
パレット 5=0H
パレット 6=6f93H
パレット 7=4e6cH
パレット 8=3126H
パレット 9=77dbH
パレット 10=7fffH
パレット 11=0H
パレット 12=0H
パレット 13=0H
パレット 14=0H
パレット 15=0H
*/
