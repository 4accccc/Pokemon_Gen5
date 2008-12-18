//======================================================================
/**
 * @file	field_msgbg.h
 * @brief	フィールド　メッセージBG関連
 * @authaor	kagaya
 * @data	2008.12.11
 */
//======================================================================
#ifndef __FLDMSGBG_H__
#define	__FLDMSGBG_H__
#include <gflib.h>
#include "system/gfl_use.h"
#include "print/printsys.h"
#include "print/gf_font.h"

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
///	メニュー戻り値
//--------------------------------------------------------------
#define FLDMENUFUNC_NULL (0xffffffff)		///<特に無し
#define FLDMENUFUNC_CANCEL (0xfffffffe)		///<キャンセル

//--------------------------------------------------------------
///	メニュースキップ操作
//--------------------------------------------------------------
#define FLDMENUFUNC_SKIP_NON	(0)			///<スキップ無し
#define FLDMENUFUNC_SKIP_LRKEY	(1)			///<LRキースキップ
#define FLDMENUFUNC_SKIP_LRBTN	(2)			///<LRボタンスキップ

//======================================================================
//	struct
//======================================================================
typedef struct _TAG_FLDMSGBG FLDMSGBG;			///<FLDMSGBG
typedef struct _TAG_FLDMSGPRINT FLDMSGPRINT;	///<FLDMSGPRINT
typedef struct _TAG_FLDMSGWIN FLDMSGWIN;		///<FLDMSGWIN
typedef struct _TAG_FLDMENUFUNC FLDMENUFUNC;	///<FLDMENUFUNC

//--------------------------------------------------------------
///	FLDMENUFUNC_LISTDATA
//--------------------------------------------------------------
typedef struct
{
	const void *str;		///<表示する文字列
	u32 param;				///<選択された際に返すパラメタ
}FLDMENUFUNC_LISTDATA;

//--------------------------------------------------------------
///	FLDMENUFUNC_LIST
//--------------------------------------------------------------
typedef struct
{
	u32 str_id;				///<表示する文字列ID
	void *selectParam;		///<選択された際に返すパラメタ
}FLDMENUFUNC_LIST;

//--------------------------------------------------------------
///	FLDMENUFUNC_HEADER
//--------------------------------------------------------------
typedef struct
{
	u16		count;				//リスト項目数
	u16		line;				//表示最大項目数
	u8		rabel_x;			//ラベル表示Ｘ座標
	u8		data_x;				//項目表示Ｘ座標
	u8		cursor_x;			//カーソル表示Ｘ座標
	u8		line_y:4;			//表示Ｙ座標
	u8		f_col:4;			//表示文字色
	u8		b_col:4;			//表示背景色
	u8		s_col:4;			//表示文字影色
	u16		msg_spc:3;			//文字間隔Ｘ
	u16		line_spc:4;			//文字間隔Ｙ
	u16		page_skip:2; //ページスキップタイプ:FLDMENUFUNC_SKIP_NON等
	u16		font_size_x;		//文字サイズX(ドット
	u16		font_size_y;		//文字サイズY(ドット
	
	u16		bmppos_x;			//表示座標X キャラ単位
	u16		bmppos_y;			//表示座標Y キャラ単位
	u16		bmpsize_x;			//表示サイズX キャラ単位
	u16		bmpsize_y;			//表示サイズY キャラ単位
}FLDMENUFUNC_HEADER;

//======================================================================
//	extern
//======================================================================
//メッセージBG
extern FLDMSGBG * FLDMSGBG_Setup( HEAPID heapID );
extern void FLDMSGBG_Delete( FLDMSGBG *fmb );
extern void FLDMSGBG_PrintMain( FLDMSGBG *fmb );
extern void FLDMSGBG_ClearPrintQue( FLDMSGBG *fmb );
extern BOOL FLDMSGBG_CheckFinishPrint( FLDMSGBG *fmb );
extern PRINT_QUE * FLDMSGBG_GetPrintQue( FLDMSGBG *fmb );
extern GFL_MSGDATA * FLDMSGBG_CreateMSGDATA( FLDMSGBG *fmb, u32 arcDatIDMsg );

//メッセージ表示
extern FLDMSGPRINT * FLDMSGPRINT_SetupPrint(
	FLDMSGBG *fmb, GFL_MSGDATA *msgData, GFL_BMPWIN *bmpwin );
extern void FLDMSGPRINT_Delete( FLDMSGPRINT *msgPrint );
extern void FLDMSGPRINT_Print(
	FLDMSGPRINT *msgPrint, u32 x, u32 y, u32 strID );
extern void FLDMSGPRINT_PrintStrBuf(
	FLDMSGPRINT *msgPrint, u32 x, u32 y, STRBUF *strBuf );
extern BOOL FLDMSGPRINT_CheckPrintTrans( FLDMSGPRINT *msgPrint );
extern void FLDMSGPRINT_ChangeBmpWin(
		FLDMSGPRINT *msgPrint, GFL_BMPWIN *bmpwin );
extern void FLDMSGPRINT_ClearBmp( FLDMSGPRINT *msgPrint );
extern PRINT_QUE * FLDMSGPRINT_GetPrintQue( FLDMSGPRINT *msgPrint );
extern GFL_FONT * FLDMSGPRINT_GetFontHandle( FLDMSGPRINT *msgPrint );
extern PRINT_UTIL * FLDMSGPRINT_GetPrintUtil( FLDMSGPRINT *msgPrint );
extern STRBUF * FLDMSGPRINT_GetStrBuf( FLDMSGPRINT *msgPrint );
extern GFL_MSGDATA * FLDMSGPRINT_GetMsgData( FLDMSGPRINT *msgPrint );

//メッセージウィンドウ
extern FLDMSGWIN * FLDMSGWIN_Add( FLDMSGBG *fmb, GFL_MSGDATA *msgData,
	u16 bmppos_x, u16 bmppos_y, u16 bmpsize_x, u16 bmpsize_y );
extern void FLDMSGWIN_Delete( FLDMSGWIN *msgWin );
extern void FLDMSGWIN_Print( FLDMSGWIN *msgWin, u16 x, u16 y, u32 strID );
extern void FLDMSGWIN_PrintStrBuf( FLDMSGWIN *msgWin, u16 x, u16 y, STRBUF *strBuf );
extern BOOL FLDMSGWIN_CheckPrintTrans( FLDMSGWIN *msgWin );
extern void FLDMSGWIN_ClearWindow( FLDMSGWIN *msgWin );
extern FLDMSGWIN * FLDMSGWIN_AddTalkWin( FLDMSGBG *fmb, GFL_MSGDATA *msgData );

//メニュー
extern FLDMENUFUNC * FLDMENUFUNC_AddMenu(
	FLDMSGBG *fmb, GFL_MSGDATA *msgData,
	const FLDMENUFUNC_HEADER *pMenuHead,
	FLDMENUFUNC_LISTDATA *pMenuListData );
extern void FLDMENUFUNC_DeleteMenu( FLDMENUFUNC *menuFunc );
extern u32 FLDMENUFUNC_ProcMenu( FLDMENUFUNC *menuFunc );
extern FLDMENUFUNC_LISTDATA * FLDMENUFUNC_CreateListData( int max, HEAPID heapID );
extern void FLDMENUFUNC_DeleteListData( FLDMENUFUNC_LISTDATA *listData );
extern FLDMENUFUNC_LISTDATA * FLDMENUFUNC_CreateMakeListData(
	const FLDMENUFUNC_LIST *menuList,
	int max, GFL_MSGDATA *msgData, HEAPID heapID );
extern void FLDMENUFUNC_AddStringListData( FLDMENUFUNC_LISTDATA *listData,
		const STRBUF *strBuf, u32 param, HEAPID heapID  );
extern void FLDMENUFUNC_AddArcStringListData(
		FLDMENUFUNC_LISTDATA *listData,
		GFL_MSGDATA *msgData, u32 strID, u32 param, HEAPID heapID );
extern void FLDMENUFUNC_InputHeaderListSize(
	FLDMENUFUNC_HEADER *head, u16 count,
	u16 bmppos_x, u16 bmppos_y, u16 bmpsize_x, u16 bmpsize_y );

#endif //__FLDMSGBG_H__
