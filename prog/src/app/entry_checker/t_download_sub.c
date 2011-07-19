//=============================================================================
/**
 *
 *	@file		t_download_sub.c
 *	@brief  トーナメントダウンロード画面(sub)
 *	@author
 *	@date
 *
 *  共通部分をまとめた([TdCmn_*]と命名)
 *
 */
//=============================================================================
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"

#include "print/gf_font.h"
#include "font/font.naix"
#include "print/printsys.h"
//
#include "ui/print_tool.h"
#include "system/bmp_winframe.h"
//
#include "arc_def.h"
//
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"
#include "app/app_printsys_common.h"
#include "gamesystem/msgspeed.h"
//
#include "system/bmp_oam.h"
//
#include "t_download_graphic.h"
#include "t_download_local.h"

#include "t_download_sub.h"

#include "win_record_gra.naix"		// データは流用

//=============================================================================
//=============================================================================
/**
 *								定義
 */
//=============================================================================
// パレット
// 上：BG (0-2:BG、3:FONT)
// 上：OBJ
// 下：BG (0,1:BG、8,9,11,12:FONT,WIN)
// 下：OBJ(0-10:パネル、11-14:cmn)
enum{
	// 何本目か
	PLTID_SYSWIN_BASE			= 8,	// font(syswin)
	PLTID_SYSWIN_FONT			= 9,	// font(syswin)
	PLTID_YESNO_WIN_BASE		= 11,	// win(yes/no window)(注：2本使用する(色変え含む:11,12))
	//
	PLTID_UP_FONT				= 3,	// font(up)
	//
	// (obj)
	PLTID_OBJ_CMN				= 11,	// cmnのpalのoffset
	//
	// 何番目か
	MES_CLEAR_COLOR				= 0,	// (通常)クリアカラー
	MES_SYSWIN_CLEAR_COLOR		= 15,	// (システムウインドウの)クリアカラー
};

//--------------
// フラグ
//--------------
// OBJのフラグ(T_DOWNLOAD_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),		// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),		// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),		// VFLIP
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}T_DOWNLOAD_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}T_DOWNLOAD_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}T_DOWNLOAD_BGSCN_DATA;

//-----------------------------------------
// 文字用の定義(表示領域の設定:CANVAS)
//------------------------------------------
typedef struct{
	u8				frmnumber;		// BG上下(frame定義なので注意)(0xffの場合は終了判定)
	u8				x;				// 座標
	u8				y;
	u8				width;			// サイズ
	u8				height;
	u8				palnum;			// パレット番号
}T_DOWNLOAD_CANVAS_DATA;

//-----------------------------------------
// OBJ用の定義
//------------------------------------------
typedef struct{
	u8				anmseq;			// アニメシーケンス(0xff終了コード)
	u8				softpri;		// OBJ(プライオリティ)
	u8				bgpri;			// BG(プライオリティ)
	u8				visible;		// 表示/非表示
	u8				palnum;			// パレット番号(現在未使用だが、しばらく残し)
	u8				dispflag;		// 表示フラグ
	u8				empty_num;		// empty番号(これから座標を参照)
	u8				pad;
}T_DOWNLOAD_OBJ_DATA;

//-----------------------------------------
// BMPOAMの定義
//------------------------------------------
typedef struct{
	u8				width;			// サイズ(8x8単位)(0xffの場合は終了判定)
	u8				height;
	u8				softpri;		// プライオリティ(obj)
	u8				bgpri;			// プライオリティ(bg)
	u8				visible;		// 表示/非表示
	u8				paloffset;		// パレットオフセット(nclの何番目か)
	u8				empty_num;		// empty番号
	u8				pad;
}T_DOWNLOAD_BMPOAM_DATA;

//=============================================================================
/**
 *								データ
 */
//=============================================================================
//---------------
// BG関連
//---------------
// BGPAL
static const T_DOWNLOAD_BGPAL_DATA td_bgpal_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_win_record_gra_win_record_bg_u_NCLR,	0,	3,},		// 上、BG
	{ PALTYPE_MAIN_BG,	NARC_win_record_gra_win_record_bg_d_NCLR,	0,	2,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const T_DOWNLOAD_BGCHR_DATA td_bgchr_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_win_record_gra_win_record_bg_u_NCGR,	0,	0,},		// 上、BG
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_NCGR,	0,	0,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const T_DOWNLOAD_BGSCN_DATA td_bgscn_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_win_record_gra_win_record_bg_u_01_NSCR,	0,	0,},		// 上、下地
	{ GFL_BG_FRAME2_S,	NARC_win_record_gra_win_record_bg_u_02_NSCR,	0,	0,},		// 上、中間
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_01_NSCR,	0,	0,},		// 下、下地
	{ GFL_BG_FRAME2_M,	NARC_win_record_gra_win_record_bg_d_02_NSCR,	0,	0,},		// 下、下地

	{ 0xff, 0, 0, 0,}, // end code
};

//---------------
// CANVASの設定
//---------------
// (printutilと対応)
static const T_DOWNLOAD_CANVAS_DATA td_canvas_tbl[] = {
	{ GFL_BG_FRAME1_S,	1,	1,	30,	2,	3,},		// 上：配信日トーナメントリスト
	{ GFL_BG_FRAME1_S,	10,	4,	20,	2,	3,},		// 上：配信日
	{ GFL_BG_FRAME1_S,	1,	7,	30,	2,	3,},		// 上：ルール名
	{ GFL_BG_FRAME1_S,	1,	10,	28,	6,	3,},		// 上：レベル制限
	{ GFL_BG_FRAME1_S,	1,	17,	18,	2,	3,},		// 上：トーナメント名
	{ GFL_BG_FRAME1_S,	1,	20,	28,	4,	3,},		// 上：内容
	//
	{ GFL_BG_FRAME1_M,	1,	1,	30,	2,	9,},		// 下：タイトル
	{ GFL_BG_FRAME0_M,	1,	1,	30,	4,	9,},		// 下：meswin
	{ GFL_BG_FRAME0_M,	1,	10,	30,	4,	9,},		// 下：中央win

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(通常)
//------------------
// narc, resid, paloffset, palnum
static const s16 td_obj_main_info_tbl[] = {
	NARC_win_record_gra_win_record_obj_d_NCLR, T_DOWNLOAD_RESID_OBJ_PLT, 0, 11, // NCL
	NARC_win_record_gra_win_record_obj_d_NCGR, T_DOWNLOAD_RESID_OBJ_CHR, 0, 0,	// NCG
	NARC_win_record_gra_win_record_obj_d_NCER, T_DOWNLOAD_RESID_OBJ_CEL, 0, 0, // NCE
	NARC_win_record_gra_win_record_obj_d_NANR, -1, 0, 0,			// NAR?
};

// 情報
static const s16 td_obj_main_itbl[] = {
	T_DOWNLOAD_RESID_OBJ_PLT,
	T_DOWNLOAD_RESID_OBJ_CHR,
	T_DOWNLOAD_RESID_OBJ_CEL,
	//
	T_DOWNLOAD_OBJ_MAXNUM,
	T_DOWNLOAD_CLWKMODE_NORMAL,
};

// OBJのテーブルデータ(通常)
static const T_DOWNLOAD_OBJ_DATA td_obj_tbl[] = {
	{ 0,	0x30,	2,	1,	0,	1,	0,	0,},		// パネル0
	{ 1,	0x30,	2,	1,	0,	1,	1,	0,},		// パネル1
	{ 2,	0x30,	2,	1,	0,	1,	2,	0,},		// パネル2
	{ 3,	0x30,	2,	1,	0,	1,	3,	0,},		// パネル3
	{ 19,	0x30,	2,	1,	0,	1,	4,	0,},		// ゲージ上
	{ 19,	0x28,	2,	1,	0,	1,	4,	0,},		// 選択カーソル(仮)

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(cmn)
//------------------
// 情報
static const s16 td_obj_cmn_itbl[] = {
	T_DOWNLOAD_RESID_CMN_OBJ_PLT,
	T_DOWNLOAD_RESID_CMN_OBJ_CHR,
	T_DOWNLOAD_RESID_CMN_OBJ_CEL,
	//
	T_DOWNLOAD_CMN_OBJ_MAXNUM,
	T_DOWNLOAD_CLWKMODE_CMN,
};

// OBJのテーブルデータ(cmn)
static const T_DOWNLOAD_OBJ_DATA td_obj_cmn_tbl[] = {
	{ 4,	0x30,	2,	1,	0,	1,	7,	0,},		// 左マーク
	{ 5,	0x30,	2,	1,	0,	1,	8,	0,},		// 右マーク
	{ 0,	0x30,	2,	1,	0,	1,	9,	0,},		// ×マーク
	{ 1,	0x30,	2,	1,	0,	1, 10,	0,},		// 戻るマーク

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// BMPOAMの情報
//------------------
static const T_DOWNLOAD_BMPOAM_DATA td_bmpoam_tbl[] = {
	{ 24,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル00
	{ 24,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル01
	{ 24,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル02
	{ 24,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル03

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//=============================================================================
/**
 *								宣言
 */
//=============================================================================
// bg
static void td_init_bg( T_DOWNLOAD_MAIN_WORK* wk );
//
// string
static void td_init_string( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_string( T_DOWNLOAD_MAIN_WORK* wk );
//
// obj
static void td_init_obj_clwk( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const T_DOWNLOAD_OBJ_DATA *obj_adr, const s16 *info_adr );
static void td_init_obj_res( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr );
static void td_init_obj_res_cmn( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype );
static void td_init_obj( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_obj( T_DOWNLOAD_MAIN_WORK* wk );
//
// bmpoam
static void td_init_obj_bmpoam( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const T_DOWNLOAD_BMPOAM_DATA *bmp_adr, const s16 *info_adr );
//
// yes/no win
static void td_clear_frame( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_update_printstream( T_DOWNLOAD_MAIN_WORK* wk );


//=============================================================================
/**
 *								関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  データ関連初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_init_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	// yes/noウインドウ用
	wk->pAppTask 	= 0;
	wk->pAppTaskRes = APP_TASKMENU_RES_Create( GFL_BG_FRAME1_M, PLTID_YESNO_WIN_BASE, wk->font, wk->print_que, wk->heapID );
	
	// bg初期化
	td_init_bg( wk );
	// 文字列
	td_init_string( wk );
	// obj
	td_init_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  データ関連終了
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_exit_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	// 文字列
	td_exit_string( wk );
	// obj
	td_exit_obj( wk );
	// yes/no
	TdCmn_exit_yesnowin( wk );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_bg( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	s32		tmp;
	const T_DOWNLOAD_BGPAL_DATA	*padr = &td_bgpal_tbl[0];
	const T_DOWNLOAD_BGCHR_DATA	*cadr = &td_bgchr_tbl[0];
	const T_DOWNLOAD_BGSCN_DATA	*sadr = &td_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
	
	// パレット転送
	for(i=0;i<T_DOWNLOAD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(padr->type == 0xff){
			break;
		}
		tmp = padr->size;
		tmp *= 0x20;		// 転送サイズ
		GFL_ARCHDL_UTIL_TransVramPalette( handle, padr->arcid, padr->type, padr->offset, tmp, wk->heapID );
		padr++;
	}
	//
	// キャラ転送
	for(i=0;i<T_DOWNLOAD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<T_DOWNLOAD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(sadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramScreen( handle, sadr->arcid, sadr->frmnumber, sadr->offset, 0, 0, wk->heapID );
		sadr++;
	}
	
	// 終了
	GFL_ARC_CloseDataHandle( handle );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(現在、Canvasの作成)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_string( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	const T_DOWNLOAD_CANVAS_DATA	*adr = &td_canvas_tbl[0];
	
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		wk->PrintUtil[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		// スクリーンは一度でよいはず
		GFL_BMPWIN_MakeScreen( wk->PrintUtil[i].win );
		GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
		adr++;
	}
	//
	//-----------------
	// fontパレット転送
	//-----------------
	{
		ARCHANDLE	*p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		// Main
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, PLTID_SYSWIN_FONT*0x20, 0x20, wk->heapID );
		// Sub
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_SUB_BG,  PLTID_UP_FONT*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x)
 *	@param	const s32			表示座標(y)
 *	@param	const STRBUF* 		表示文字列
 *	@param	const u32			表示位置(左、右、中央など)
 *	@param	const PRINTSYS_LSB	カラー
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_disp_string( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col )
{
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->font,						// フォント
				col,							// カラー
				pmode );						// 位置
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示(仮)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x)
 *	@param	const s32			表示座標(y)
 *	@param	const s32			値
 *	@param	const s32			桁数
 *	@param	const u32			表示位置(左、右、中央など)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_disp_string_number( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum, const u32 pmode )
{
	STRBUF	*strbuf;
	
	// 数字を桁分0うめ表示
	strbuf = GFL_STR_CreateBuffer( 128, wk->heapID );
	STRTOOL_SetNumber( strbuf, num, lnum, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	//
//	if(mode == CVS_MAIN_TITLE){
	if(mode == 6){		// 要修正
		TdCmn_disp_string( wk, mode, x, y, strbuf, pmode, PRINTSYS_LSB_Make(15,2,0) );
	}else{
		TdCmn_disp_string( wk, mode, x, y, strbuf, pmode, PRINTSYS_LSB_Make(1,2,0) );
	}
	//
	GFL_STR_DeleteBuffer( strbuf );
}


//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示終了
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit_string( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
		GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
	}
	// PrintStream後処理
	if(wk->MsgBuffer){
		GFL_STR_DeleteBuffer( wk->MsgBuffer );
	}
	if(wk->PrintStream){
		PRINTSYS_PrintStreamDelete( wk->PrintStream );
	}
	// 文字送り用
//	APP_KEYCURSOR_Delete( wk->MsgCursorWork );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				キャンバスの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_clear_canvas( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	// clear
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_CLEAR_COLOR );
//	GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[mode].win );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const T_DOWNLOAD_OBJ_DATA* アドレス
 *	@param	const s16*				アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj_clwk( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const T_DOWNLOAD_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16						tmp;
	s16						pos[2];
	s16						buf[6];
	s32 					i;
	BOOL					bflg;
	GFL_CLWK_DATA 			cldata;
	CLWK_FLIP_TYPE			flip;
    GFL_CLUNIT*				p_clunit 	= T_DOWNLOAD_GRAPHIC_GetClunit( wk->graphic );
	// アドレスセット
	const T_DOWNLOAD_OBJ_DATA *adr 	= obj_adr;
	const s16 				*iadr 	= info_adr;
	GFL_CLWK				**clwk;			// アドレス注意
	
	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
	// チェック
	buf[0] = iadr[0];		// plt
	buf[1] = iadr[1];		// chr
	buf[2] = iadr[2];		// cel
	buf[3] = iadr[3];		// max
	buf[4] = iadr[4];		// mode(0:通常、1:cmn)
	//
	// アドレスセット
	if(buf[4] == T_DOWNLOAD_CLWKMODE_NORMAL){			// 通常
		clwk = &wk->wk_obj.clwk[0];
	}
	else if(buf[4] == T_DOWNLOAD_CLWKMODE_CMN){		// cmn
		clwk = &wk->wk_obj.cmn_clwk[0];
	}
	
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = TdCmn_get_empty(wk, tmp*2);
		pos[1] = TdCmn_get_empty(wk, tmp*2+1);
		cldata.pos_x 	= pos[0];
		cldata.pos_y 	= pos[1];
		cldata.anmseq 	= adr->anmseq;
		cldata.softpri	= adr->softpri;
		cldata.bgpri	= adr->bgpri;
		//
		clwk[i] =   GFL_CLACT_WK_Create( p_clunit, 
											wk->wk_obj.res[ buf[1] ],			// chr
											wk->wk_obj.res[ buf[0] ],			// plt
											wk->wk_obj.res[ buf[2] ],			// cel
											&cldata,
											drawtype,
											wk->heapID );
		//-------------
		// 表示/非表示
		//-------------
		bflg = FALSE;
		if(adr->visible == 1){
			bflg = TRUE;
		}
		GFL_CLACT_WK_SetDrawEnable( clwk[i], bflg );
		//----------
		// フラグ
		//----------
		// アニメ
		bflg = FALSE;
		if(adr->dispflag & FLG_OBJ_ANIM){
			bflg = TRUE;
		}
		GFL_CLACT_WK_SetAutoAnmFlag( clwk[i], bflg );
		//
		// Hフリップ
		if(adr->dispflag & FLG_OBJ_HFLIP){
			GFL_CLACT_WK_SetFlip( clwk[i], CLWK_FLIP_H, TRUE );
		}
		// Vフリップ
		if(adr->dispflag & FLG_OBJ_VFLIP){
			GFL_CLACT_WK_SetFlip( clwk[i], CLWK_FLIP_V, TRUE );
		}
		// 次のデータへ
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(res)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj_res( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
    GFL_CLUNIT*	p_clunit = T_DOWNLOAD_GRAPHIC_GetClunit( wk->graphic );
	const s16 *adr		 = info_adr;		// アドレスセット
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ adr[0+1] ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, adr[0], drawtype, adr[2], 0, adr[3], wk->heapID );
	wk->wk_obj.res[ adr[4+1] ] = GFL_CLGRP_CGR_Register(      p_handle, adr[4], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ adr[8+1] ] = GFL_CLGRP_CELLANIM_Register( p_handle, adr[8], adr[12], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(cmn res)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj_res_cmn( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype )
{
	// cmnのidを取得
	const u32 	arcid    = APP_COMMON_GetArcId();
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( arcid, wk->heapID );
    GFL_CLUNIT*	p_clunit = T_DOWNLOAD_GRAPHIC_GetClunit( wk->graphic );
	u32			idx[4];
	//
	idx[0] = APP_COMMON_GetBarIconPltArcIdx();		// pal
	idx[1] = APP_COMMON_GetBarIconCharArcIdx();		// ncg
	idx[2] = APP_COMMON_GetBarIconCellArcIdx(  APP_COMMON_MAPPING_128K );	// cell
	idx[3] = APP_COMMON_GetBarIconAnimeArcIdx( APP_COMMON_MAPPING_128K );	// anim
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ T_DOWNLOAD_RESID_CMN_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, idx[0], drawtype, 0x20*PLTID_OBJ_CMN, 0, 4, wk->heapID );
	wk->wk_obj.res[ T_DOWNLOAD_RESID_CMN_OBJ_CHR ] = GFL_CLGRP_CGR_Register(      p_handle, idx[1], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ T_DOWNLOAD_RESID_CMN_OBJ_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle, idx[2], idx[3], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化まとめ
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj( T_DOWNLOAD_MAIN_WORK* wk )
{
    GFL_CLUNIT*	p_clunit = T_DOWNLOAD_GRAPHIC_GetClunit( wk->graphic );
    //
	// リソース読み込み(通常)
	td_init_obj_res( wk, CLSYS_DRAW_MAIN, &td_obj_main_info_tbl[0] );
	// リソース読み込み(cmn)
	td_init_obj_res_cmn( wk, CLSYS_DRAW_MAIN );
	// CLWK登録(通常)
	td_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &td_obj_tbl[0], &td_obj_main_itbl[0] );
	// CLWK登録(cmn)
	td_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &td_obj_cmn_tbl[0], &td_obj_cmn_itbl[0] );
	//
	// BmpOAM初期化
	wk->BmpOamSys = BmpOam_Init( wk->heapID, p_clunit );
	// BMPOAM登録
	td_init_obj_bmpoam( wk, CLSYS_DRAW_MAIN, &td_bmpoam_tbl[0], &td_obj_main_info_tbl[0] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit_obj( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<T_DOWNLOAD_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
	}
	for(i=0;i<T_DOWNLOAD_CMN_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.cmn_clwk[i] );
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[T_DOWNLOAD_RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[T_DOWNLOAD_RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[T_DOWNLOAD_RESID_OBJ_CEL] );
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[T_DOWNLOAD_RESID_CMN_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[T_DOWNLOAD_RESID_CMN_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[T_DOWNLOAD_RESID_CMN_OBJ_CEL] );
	//
	// BmpOAM後処理
	for(i=0;i<T_DOWNLOAD_BMPOAM_MAXNUM;i++){
		BmpOam_ActorDel( wk->BmpOamWork[i] );
		GFL_BMP_Delete( wk->BmpData[i] );
	}
	BmpOam_Exit( wk->BmpOamSys );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_chg_objseq( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wmode == T_DOWNLOAD_CLWKMODE_NORMAL){			// 通常
		if(wnum >= T_DOWNLOAD_CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
	}
	else if(wmode == T_DOWNLOAD_CLWKMODE_CMN){			// 共通
		if(wnum >= T_DOWNLOAD_CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.cmn_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.cmn_clwk[wnum], TRUE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_set_obj_visible( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	if(wmode == T_DOWNLOAD_CLWKMODE_NORMAL){		// 通常
		if(wnum >= T_DOWNLOAD_CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
	}
	else if(wmode == T_DOWNLOAD_CLWKMODE_CMN){		// 共通
		if(wnum >= T_DOWNLOAD_CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.cmn_clwk[wnum], bflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *
 *	@retval	const BOOL			(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
BOOL TdCmn_is_anime_end( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 終了判定らしい
	if(wmode == T_DOWNLOAD_CLWKMODE_NORMAL){		// 通常
		if(wnum >= T_DOWNLOAD_CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	else if(wmode == T_DOWNLOAD_CLWKMODE_CMN){		// 共通
		if(wnum >= T_DOWNLOAD_CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.cmn_clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	return( bflg );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(bmpoam)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const T_DOWNLOAD_BMPOAM_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj_bmpoam( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const T_DOWNLOAD_BMPOAM_DATA *bmp_adr, const s16 *info_adr )
{
	s16							tmp;
	s32							i;
	s32							buf[2];
	BMPOAM_ACT_DATA				actdata;
	const T_DOWNLOAD_BMPOAM_DATA	*adr  = bmp_adr;
	const s16					*iadr = info_adr;
	GFL_CLACTPOS				cpos;
	//
	for(i=0;i<T_DOWNLOAD_BMPOAM_MAXNUM;i++){
		// 終了判定
		if(adr->width == 0xff){
			break;
		}
		// サイズ(colormodeは16で大丈夫だろう)
		buf[0] = (s32)(adr->width);
		buf[1] = (s32)(adr->height);
		wk->BmpData[i] = GFL_BMP_Create( buf[0], buf[1], GFL_BMP_16_COLOR, wk->heapID );
		// 左側
		tmp = adr->empty_num;
		cpos.x = TdCmn_get_empty( wk, tmp*2 );
		cpos.y = TdCmn_get_empty( wk, tmp*2+1 );
		//
		GFL_STD_MemClear( &actdata, sizeof(BMPOAM_ACT_DATA) );
		actdata.bmp			= wk->BmpData[i];
		actdata.x 			= cpos.x;
		actdata.y 			= cpos.y;
		actdata.pltt_index	= wk->wk_obj.res[ iadr[0+1] ];		// plt
		actdata.soft_pri	= adr->softpri;
		actdata.bg_pri		= adr->bgpri;
		actdata.pal_offset	= adr->paloffset;
		actdata.setSerface	= drawtype;
		actdata.draw_type	= drawtype;		// ?
		// 作成
		wk->BmpOamWork[i]	= BmpOam_ActorAdd( wk->BmpOamSys, &actdata );
		// visible
		if(adr->visible == 0){
			BmpOam_ActorSetDrawEnable( wk->BmpOamWork[i], FALSE );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OB文字列表示(bmpoam)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				ワーク番号
 *	@param	const STRBUF*			表示文字列
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_disp_bmpoam( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf )
{
	if(wnum >= T_DOWNLOAD_BMPOAM_MAXNUM){
		OS_Printf("bmpoamj max error:%d\n",wnum);
		return;
	}
	GFL_BMP_Clear( wk->BmpData[wnum], 0 );
	PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[wnum], 0, 0, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
	BmpOam_ActorBmpTrans( wk->BmpOamWork[wnum] );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
s16 TdCmn_get_empty( T_DOWNLOAD_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= T_DOWNLOAD_EMPTY_MAXNUM)){
		OS_Printf("empty error :%d\n",num);
		return(0);
	}
	
	adr = wk->EmptyBuffer;
	return( adr[num] );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *  @brief   はい/いいえウインドウ 開始
 *
 *  @param  T_DOWNLOAD_MAIN_WORK *wk
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
void TdCmn_init_yesnowin( T_DOWNLOAD_MAIN_WORK* wk )
{
	APP_TASKMENU_INITWORK appinit;
	
	appinit.heapId		= wk->heapID;
	appinit.itemNum		= 2;					// アイテム数
	appinit.itemWork	= &wk->appitem[0];		// 情報セットのバッファ
	//
	appinit.posType		= ATPT_RIGHT_DOWN;
	appinit.charPosX	= 32;
	appinit.charPosY	= 12;
	appinit.w			= APP_TASKMENU_PLATE_WIDTH_YN_WIN;
	appinit.h			= APP_TASKMENU_PLATE_HEIGHT_YN_WIN;
	//
	// 「はい」の設定
	wk->appitem[0].str		= GFL_STR_CreateBuffer(100, wk->heapID);
	// test(00)
	STRTOOL_SetNumber( wk->appitem[0].str, 0, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	wk->appitem[0].msgColor	= PRINTSYS_LSB_Make( 0xe, 0xf, 0 );
	wk->appitem[0].type		= APP_TASKMENU_WIN_TYPE_NORMAL;
	//
	// 「いいえ」の設定
	wk->appitem[1].str		= GFL_STR_CreateBuffer(100, wk->heapID);
	// test(01)
	STRTOOL_SetNumber( wk->appitem[1].str, 1, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	wk->appitem[1].msgColor	= PRINTSYS_LSB_Make( 0xe, 0xf, 0 );
	wk->appitem[1].type		= APP_TASKMENU_WIN_TYPE_NORMAL;
	//
	wk->pAppTask = APP_TASKMENU_OpenMenu( &appinit, wk->pAppTaskRes );
	//
	GFL_STR_DeleteBuffer( wk->appitem[0].str );
	GFL_STR_DeleteBuffer( wk->appitem[1].str );
	//
	// defaultでnoにする
	// (activeもあわせて変更)
	// yes側
	APP_TASKMENU_SetCursorPos( wk->pAppTask, 0 );
	APP_TASKMENU_SetActive( wk->pAppTask, FALSE );
	// no側
	APP_TASKMENU_SetCursorPos( wk->pAppTask, 1 );
	APP_TASKMENU_SetActive( wk->pAppTask, TRUE );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  はい/いいえウィンドウ終了
 *
 *  @param  T_DOWNLOAD_MAIN_WORK	*wk
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
void TdCmn_exit_yesnowin( T_DOWNLOAD_MAIN_WORK* wk )
{
	// 終了処理
	if(wk->pAppTask){
		APP_TASKMENU_CloseMenu( wk->pAppTask );
		wk->pAppTask = NULL;
	}
}

//-----------------------------------------------------------------------------
/**
 *  @brief  はい/いいえウィンドウ処理待ち
 *
 *  @param  T_DOWNLOAD_MAIN_WORK	*wk
 *
 *  @retval s32				(-1:未処理、0:NO、1:YES)
 */
//-----------------------------------------------------------------------------
s32  TdCmn_chk_yesnowin( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		flg=-1;
	s32		pos;
	
	// 念のため
	if(wk->pAppTask == 0){
		return( flg );
	}
	// 選択中
	if( APP_TASKMENU_IsFinish( wk->pAppTask ) ){
		pos = APP_TASKMENU_GetCursorPos( wk->pAppTask );
		if(pos == 0){		// yes
			flg = 1;
		}else{				// no
			flg = 0;
		}
		// 終了
		TdCmn_exit_yesnowin( wk );
		//
		// clear
		td_clear_frame( wk );
	}
	//
	return( flg );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ(セット)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			モード
 *	@param	const s32			メッセージid
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_set_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mesid )
{
	// PrintStream終了時にはdeleteしないとtaskが残ってしまうようだ
	// 念のため0check
	if( wk->PrintStream ){
		PRINTSYS_PrintStreamDelete( wk->PrintStream );
	}
	// Msgの再構築
	if(wk->MsgBuffer){
		GFL_STR_DeleteBuffer( wk->MsgBuffer );
	}
	// メッセージ仮バッファセット
	wk->MsgBuffer = GFL_MSG_CreateString( wk->msg[0], mesid );
	//
	// stream設定=>ここのwk->MsgBufferは終わるまで削除してはいけない
	wk->PrintStream = PRINTSYS_PrintStream(
				wk->PrintUtil[mode].win, 0, 0, wk->MsgBuffer, wk->font,
				MSGSPEED_GetWait(), wk->MsgTcblSys, 4, wk->heapID, MES_SYSWIN_CLEAR_COLOR );
	// スクリーン転送らしい
	GFL_BMPWIN_MakeTransWindow( wk->PrintUtil[mode].win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ(処理)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval	BOOL				(TRUE:再生中、FALSE:終了)
 */
//-----------------------------------------------------------------------------
BOOL TdCmn_main_meswin( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg;
	
	// 念のため
	if(wk->PrintStream == 0){
		return( FALSE );
	}
	//
	// 文字送りカーソル処理
//	APP_KEYCURSOR_Main( wk->MsgCursorWork, wk->PrintStream, wk->PrintUtil[CVS_MAIN_MESWIN].win );
	// streamの入力処理をまとめて処理してくれるらしい
//	bflg = APP_PRINTSYS_COMMON_PrintStreamFunc( &wk->PrintSysWork , wk->PrintStream );
	// 連結部分がうまくいかないため自前で処理
	bflg = td_update_printstream( wk );
	if( bflg == TRUE ){
		// 終了
		return( FALSE );
	}
	return( TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ完全初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				canvasの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_init_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	//-----------------------------------------------------------------------
	// キャラデータのウインドウをまるごと消去(frmのクリア)する必要があるため
	// 完全初期化の部分を切り出しておく
	//-----------------------------------------------------------------------
	// 会話ウィンドウセット(キャラデータを転送)
	BmpWinFrame_GraphicSet( GFL_BG_FRAME0_M, 1, PLTID_SYSWIN_BASE, MENU_TYPE_SYSTEM, wk->heapID);
	// 会話ウインドウ表示
	BmpWinFrame_Write( wk->PrintUtil[mode].win, WINDOW_TRANS_ON, 1, PLTID_SYSWIN_BASE );
	// 文字スクリーン作成
	GFL_BMPWIN_MakeScreen( wk->PrintUtil[mode].win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[mode].win) );
	// クリア
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_SYSWIN_CLEAR_COLOR );
	GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[mode].win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ全クリア(frmのクリア)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_clear_frame( T_DOWNLOAD_MAIN_WORK* wk )
{
	// clear
	GFL_BG_ClearFrame( GFL_BG_FRAME0_M );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  stream文字送り(yes/no)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:終了、FALSE:まだ)
 */
//-----------------------------------------------------------------------------
static BOOL td_update_printstream( T_DOWNLOAD_MAIN_WORK* wk )
{
	// APP_PRINTSYS_COMMON_PrintStreamFunc( &wk->PrintSysWork , wk->PrintStream );
	// を使ってしまうと最後の部分を入力しないと先にいかないことになってしまうので、
	// 自前で処理する
	BOOL	bflg=FALSE;
	s32 	status = PRINTSYS_PrintStreamGetState( wk->PrintStream );
	
	// 起動中
	if(status == PRINTSTREAM_STATE_RUNNING){
		if(GFL_UI_KEY_GetCont()&PAD_BUTTON_DECIDE){
			PRINTSYS_PrintStreamShortWait( wk->PrintStream, 0 );
		}
	}
	// 終わった
	else if( status == PRINTSTREAM_STATE_DONE ){
//		PRINTSYS_PrintStreamDelete( wk->PrintStream );
		bflg = TRUE;
//		return wk->next_seq;
	}
	// 次へ(ここにくるようだ)
	else if(status == PRINTSTREAM_STATE_PAUSE){
//		if(GFL_UI_KEY_GetTrg()&(PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) || GFL_UI_TP_GetTrg()){
			PRINTSYS_PrintStreamReleasePause( wk->PrintStream );
//			PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
//		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ全クリア(frmのクリア)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(canvasの番号)
 *	@param	const s32				(メッセージID)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_disp_centermes( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mesid )
{
	STRBUF		*strbuf;
	
	// 中央の文字表示
	TdCmn_init_meswin( wk, mode );
	//
	// メッセージ表示
	strbuf = GFL_MSG_CreateString( wk->msg[0], mesid );
	TdCmn_disp_string( wk, mode, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(1,2,0) );
	GFL_STR_DeleteBuffer( strbuf );
}

