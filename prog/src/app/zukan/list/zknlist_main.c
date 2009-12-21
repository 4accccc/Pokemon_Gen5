//============================================================================================
/**
 * @file		zknlist_main.c
 * @brief		図鑑リスト画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZKNLISTMAIN
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/gfl_use.h"
#include "print/global_msg.h"
#include "font/font.naix"
#include "app/app_menu_common.h"
#include "msg/msg_zukan_list.h"
#include "message.naix"

#include "zknlist_main.h"
#include "zknlist_bmp.h"
#include "zknlist_obj.h"
#include "zknlist_bgwfrm.h"
#include "zukan_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	EXP_BUF_SIZE	( 128 )		// テンポラリメッセージバッファサイズ

#define	BLEND_EV1		( 6 )				// ブレンド係数
#define	BLEND_EV2		( 10 )			// ブレンド係数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void VBlankTask( GFL_TCB * tcb, void * work );
static void ListCallBack( void * work, s16 nowPos, s16 nowScroll, s16 oldPos, s16 oldScroll, u32 mv );


//============================================================================================
//	グローバル
//============================================================================================

// VRAM割り振り
static const GFL_DISP_VRAM VramTbl = {
	GX_VRAM_BG_128_A,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_128_D,					// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,		// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_NONE,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,					// テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_128K		// サブOBJマッピングモード
};






//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTMAIN_InitVBlank( ZKNLISTMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTMAIN_ExitVBlank( ZKNLISTMAIN_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理
 *
 * @param		tcb			GFL_TCB
 * @param		wk			図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
//	ZKNLISTMAIN_WORK * wk = work;

	GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();

//	PaletteFadeTrans( syswk->app->pfd );

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

void ZKNLISTMAIN_InitVram(void)
{
	GFL_DISP_SetBank( &VramTbl );
}

const GFL_DISP_VRAM * ZKNLISTMAIN_GetVramBankData(void)
{
	return &VramTbl;
}


void ZKNLISTMAIN_InitBg(void)
{
	GFL_BG_Init( HEAPID_ZUKAN_LIST );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：カバー？
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0, HEAPID_ZUKAN_LIST );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );
	}
	{	// メイン画面：タッチバー
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x4000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：リスト
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_M );
	}
	{	// メイン画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_TEXT );
	}

	{	// サブ画面：カバー？
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 0x20, 0, HEAPID_ZUKAN_LIST );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S );
	}
	{	// サブ画面：フレーム
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// サブ画面：リスト
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_S );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
	}

	{	// サブ画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &cnth, GFL_BG_MODE_TEXT );
	}

	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
}

void ZKNLISTMAIN_ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

	GFL_BG_Exit();
}

void ZKNLISTMAIN_LoadBgGraphic(void)
{
	ARCHANDLE * ah;

	ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_LIST );

/*
	NARC_zukan_gra_list_list_bgd_NCLR = 8,
	NARC_zukan_gra_list_list_bgu_NCLR = 9,
	NARC_zukan_gra_list_list_objd_NCLR = 10,
	NARC_zukan_gra_list_list_bgd_NCGR = 11,
	NARC_zukan_gra_list_list_bgu_NCGR = 12,
	NARC_zukan_gra_list_list_objd_NCGR = 13,
	NARC_zukan_gra_list_listbase_bgd_NCGR = 14,
	NARC_zukan_gra_list_listbase_bgu_NCGR = 15,
	NARC_zukan_gra_list_listframe_bgd_NCGR = 16,
	NARC_zukan_gra_list_listframe_bgu_NCGR = 17,
	NARC_zukan_gra_list_list_objd_NCER = 18,
	NARC_zukan_gra_list_listbase_bgd_NSCR = 19,
	NARC_zukan_gra_list_listbase_bgu_NSCR = 20,
	NARC_zukan_gra_list_listbtn_bgd_NSCR = 21,
	NARC_zukan_gra_list_listbtn_bgu_NSCR = 22,
	NARC_zukan_gra_list_listtitle_bgu_NSCR = 23,
	NARC_zukan_gra_list_list_objd_NANR = 24
*/


	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_zukan_gra_list_list_bgd_NCLR, PALTYPE_MAIN_BG, 0, 0x20*4, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_zukan_gra_list_list_bgu_NCLR, PALTYPE_SUB_BG, 0, 0x20*4, HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_list_listframe_bgd_NCGR, GFL_BG_FRAME2_M, 0, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_list_listbase_bgd_NCGR, GFL_BG_FRAME3_M, 0, 0, FALSE, HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_list_listframe_bgu_NCGR, GFL_BG_FRAME2_S, 0, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_list_listbase_bgu_NCGR, GFL_BG_FRAME3_S, 0, 0, FALSE, HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_list_listbase_bgd_NSCR,
		GFL_BG_FRAME3_M, 0, 0, FALSE, HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_list_listtitle_bgu_NSCR,
		GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_list_listbase_bgu_NSCR,
		GFL_BG_FRAME3_S, 0, 0, FALSE, HEAPID_ZUKAN_LIST );

	GFL_ARC_CloseDataHandle( ah );

/*
//	GFL_ARCHDL_UTIL_TransVramBgCharacter(
	GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
		ah, NARC_box_gra_box_m_bg2_lz_NCGR,
		GFL_BG_FRAME2_M, 0, TRUE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_box_gra_box_m_bg2_lz_NSCR,
		GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_m_bg3_lz_NCGR,
		GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_box_gra_box_m_bg3_lz_NSCR,
		GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_box_gra_box_m_bg_NCLR, PALTYPE_MAIN_BG, 0, 0x20*4, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_selwin_lz_NCGR, GFL_BG_FRAME1_M,
		BOX2MAIN_SELWIN_CGX_POS, BOX2MAIN_SELWIN_CGX_SIZ*0x20*2, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_box_gra_selwin_NCLR, PALTYPE_MAIN_BG,
		BOX2MAIN_BG_PAL_SELWIN*0x20, 0x40, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_s_bg3_lz_NCGR,
		GFL_BG_FRAME1_S, 0, 0, TRUE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_box_gra_box_s_bg3_lz_NSCR,
		GFL_BG_FRAME1_S, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_s_bg3_lz_NCGR,
		GFL_BG_FRAME2_S, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_box_gra_box_s_bg_NCLR, PALTYPE_SUB_BG, 0, 0, HEAPID_BOX_APP );
*/


	// タッチバー
	ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
		ah, APP_COMMON_GetBarCharArcIdx(), GFL_BG_FRAME1_M, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, APP_COMMON_GetBarPltArcIdx(), PALTYPE_MAIN_BG,
		ZKNLISTMAIN_MBG_PAL_TOUCHBAR*0x20, 0x20, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, APP_COMMON_GetBarScrnArcIdx(),
		GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_BG_ChangeScreenPalette(
		GFL_BG_FRAME1_M, ZKNLISTMAIN_TOUCH_BAR_PX, ZKNLISTMAIN_TOUCH_BAR_PY,
		ZKNLISTMAIN_TOUCH_BAR_SX, ZKNLISTMAIN_TOUCH_BAR_SY, ZKNLISTMAIN_MBG_PAL_TOUCHBAR );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );

	GFL_ARC_CloseDataHandle( ah );

/*



	syswk->app->syswinInfo = BmpWinFrame_GraphicSetAreaMan(
														GFL_BG_FRAME0_M,
														BOX2MAIN_BG_PAL_SYSWIN,
														MENU_TYPE_SYSTEM,
														HEAPID_BOX_APP );

*/

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		ZKNLISTMAIN_MBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_LIST );
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		ZKNLISTMAIN_SBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_LIST );

}

//--------------------------------------------------------------------------------------------
/**
 * @brief		アルファブレンド設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTMAIN_SetBlendAlpha(void)
{
	G2S_SetBlendAlpha(
		GX_BLEND_PLANEMASK_BG2,
		GX_BLEND_PLANEMASK_BG3,
		BLEND_EV1, BLEND_EV2 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連初期化
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTMAIN_InitMsg( ZKNLISTMAIN_WORK * wk )
{
	wk->mman = GFL_MSG_Create(
							GFL_MSG_LOAD_NORMAL,
							ARCID_MESSAGE, NARC_message_zukan_list_dat, HEAPID_ZUKAN_LIST );
	wk->font = GFL_FONT_Create(
												ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_ZUKAN_LIST );
	wk->wset = WORDSET_Create( HEAPID_ZUKAN_LIST );
	wk->que  = PRINTSYS_QUE_Create( HEAPID_ZUKAN_LIST );
	wk->exp  = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_ZUKAN_LIST );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTMAIN_ExitMsg( ZKNLISTMAIN_WORK * wk )
{
	GFL_STR_DeleteBuffer( wk->exp );
	PRINTSYS_QUE_Delete( wk->que );
	WORDSET_Delete( wk->wset );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );
}


#define	LIST_MONS_MASK		( 0x0fffffff )
#define	LIST_INFO_MASK		( 0xf0000000 )
#define	LIST_INFO_SHIFT		( 28 )

#define	SET_LIST_PARAM(p,m)	( ( p << LIST_INFO_SHIFT ) | m )
#define	GET_LIST_MONS(a)		( a & LIST_MONS_MASK )
#define	GET_LIST_INFO(a)		( ( a & LIST_INFO_MASK ) >> LIST_INFO_SHIFT )

enum {
	LIST_INFO_MONS_NONE = 0,
	LIST_INFO_MONS_SEE,
	LIST_INFO_MONS_GET,
};


//--------------------------------------------------------------------------------------------
/**
 * @brief	  リスト作成
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTMAIN_MakeList( ZKNLISTMAIN_WORK * wk )
{
	STRBUF * srcStr;
	STRBUF * cpyStr;
	u32	i;

	wk->list = ZKNLISTMAIN_CreateList( MONSNO_END, HEAPID_ZUKAN_LIST );

	// 仮です。
	for( i=1; i<=MONSNO_END; i++ ){
//	for( i=1; i<=4; i++ ){
//	for( i=1; i<=12; i++ ){
		if( ( i % 3 ) == 0 ){
			srcStr = GFL_MSG_CreateString( wk->mman, str_name_01 );
			ZKNLISTMAIN_AddListData( wk->list, srcStr, SET_LIST_PARAM(LIST_INFO_MONS_NONE,i) );
		}else{
			srcStr = GFL_MSG_CreateString( GlobalMsg_PokeName, i );
			cpyStr = GFL_STR_CreateCopyBuffer( srcStr, HEAPID_ZUKAN_LIST );
			GFL_STR_DeleteBuffer( srcStr );
			ZKNLISTMAIN_AddListData( wk->list, cpyStr, SET_LIST_PARAM(i%3,i) );
		}
	}

/*
	srcStr = GFL_MSG_CreateString( GlobalMsg_PokeName, 1 );
	cpyStr = GFL_STR_CreateCopyBuffer( srcStr, HEAPID_ZUKAN_LIST );
	GFL_STR_DeleteBuffer( srcStr );
	ZKNLISTMAIN_AddListData( wk->list, cpyStr, SET_LIST_PARAM(LIST_INFO_MONS_GET,1) );
	for( i=1; i<256; i++ ){
		srcStr = GFL_MSG_CreateString( wk->mman, str_name_01 );
		ZKNLISTMAIN_AddListData( wk->list, srcStr, SET_LIST_PARAM(LIST_INFO_MONS_NONE,i) );
	}
	srcStr = GFL_MSG_CreateString( GlobalMsg_PokeName, i );
	cpyStr = GFL_STR_CreateCopyBuffer( srcStr, HEAPID_ZUKAN_LIST );
	GFL_STR_DeleteBuffer( srcStr );
	ZKNLISTMAIN_AddListData( wk->list, cpyStr, SET_LIST_PARAM(LIST_INFO_MONS_SEE,i) );
*/

	ZKNLISTMAIN_InitList( wk->list, 0, 7, 0, wk, ListCallBack );

	ZKNLISTMAIN_InitListPut( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  リスト削除
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTMAIN_FreeList( ZKNLISTMAIN_WORK * wk )
{
	ZKNLISTMAIN_ExitList( wk->list );
}

u32 ZKNLISTMAIN_GetListMons( ZUKAN_LIST_WORK * wk, u32 pos )
{
	return GET_LIST_MONS( ZKNLISTMAIN_GetListParam(wk,pos) );
}

u32 ZKNLISTMAIN_GetListInfo( ZUKAN_LIST_WORK * wk, u32 pos )
{
	return GET_LIST_INFO( ZKNLISTMAIN_GetListParam(wk,pos) );
}

static void PutListOne( ZKNLISTMAIN_WORK * wk, u32 idx, s32 listPos )
{
	ZKNLISTBMP_PutPokeList( wk, ZKNLISTBMP_WINIDX_NAME_M1+idx, listPos );
	ZKNLISTBGWFRM_PutPokeList( wk, ZKNLISTBGWFRM_IDX_NAME_M1+idx, listPos );
	ZKNLISTOBJ_PutPokeList( wk, ZKNLISTOBJ_IDX_POKEICON+idx, listPos, TRUE );
}

static void PutListOneSub( ZKNLISTMAIN_WORK * wk, u32 idx, s32 listPos )
{
	ZKNLISTBMP_PutPokeList( wk, ZKNLISTBMP_WINIDX_NAME_S1+idx, listPos );
	ZKNLISTBGWFRM_PutPokeList( wk, ZKNLISTBGWFRM_IDX_NAME_S1+idx, listPos );
	ZKNLISTOBJ_PutPokeList( wk, ZKNLISTOBJ_IDX_POKEICON_S+idx, listPos, FALSE );
}

void ZKNLISTMAIN_InitListPut( ZKNLISTMAIN_WORK * wk )
{
	s16	scroll;
	s16	i;

	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, 0 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_S, GFL_BG_SCROLL_Y_SET, 0 );
	ZKNLISTOBJ_InitScrollList( wk );
	wk->listBgScroll = 0;

	scroll = ZKNLISTMAIN_GetListScroll( wk->list );

	for( i=0; i<ZKNLISTMAIN_GetListPosMax(wk->list); i++ ){
		PutListOne( wk, i, scroll+i );
		PutListOneSub( wk, i, scroll+i-7 );
		BGWINFRM_FramePut( wk->wfrm, ZKNLISTBGWFRM_IDX_NAME_M1+i, 16, i*3 );
		BGWINFRM_FramePut( wk->wfrm, ZKNLISTBGWFRM_IDX_NAME_S1+i, 16, i*3+3 );
	}

	ZKNLISTMAIN_PutListCursor( wk, 2, ZKNLISTMAIN_GetListPos(wk->list) );
	ZKNLISTOBJ_ChgListPosAnm( wk, ZKNLISTMAIN_GetListPos(wk->list), TRUE );
	ZKNLISTOBJ_PutListPosPokeGra( wk, ZKNLISTMAIN_GetListCursorPos(wk->list) );

	wk->listPutIndex = 0;
}

void ZKNLISTMAIN_PutListCursor( ZKNLISTMAIN_WORK * wk, u8 pal, s16 pos )
{
	int	scroll;
	u16	i;
	s8	py;

	scroll = GFL_BG_GetScrollY( GFL_BG_FRAME2_M ) % 256;
	py = scroll / 8 + pos * ZKNLISTMAIN_LIST_SY;

	for( i=0; i<ZKNLISTMAIN_LIST_SY; i++ ){
		if( py < 0 ){
			py += 32;
		}else if( py >= 32 ){
			py -= 32;
		}
		GFL_BG_ChangeScreenPalette( GFL_BG_FRAME2_M, ZKNLISTMAIN_LIST_PX, py, ZKNLISTMAIN_LIST_SX, 1, pal );
		py++;
	}
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_M );
}

static void ListCallBack( void * work, s16 nowPos, s16 nowScroll, s16 oldPos, s16 oldScroll, u32 mv )
{
	ZKNLISTMAIN_WORK * wk;
	
	wk = work;

	switch( mv ){
	// カーソル移動
	case ZKNLISTMAIN_LIST_MOVE_UP:
	case ZKNLISTMAIN_LIST_MOVE_DOWN:
		ZKNLISTMAIN_PutListCursor( wk, 1, oldPos );
		ZKNLISTMAIN_PutListCursor( wk, 2, nowPos );
		ZKNLISTOBJ_ChgListPosAnm( wk, oldPos, FALSE );
		ZKNLISTOBJ_ChgListPosAnm( wk, nowPos, TRUE );
		ZKNLISTOBJ_PutListPosPokeGra( wk, nowPos+nowScroll );
		return;

	// ページスクロール
	case ZKNLISTMAIN_LIST_MOVE_LEFT:
	case ZKNLISTMAIN_LIST_MOVE_RIGHT:
		ZKNLISTMAIN_InitListPut( wk );
		ZKNLISTOBJ_SetListScrollBarPos( wk );
		return;

	// 上方向にスクロール
	case ZKNLISTMAIN_LIST_MOVE_SCROLL_UP:
		wk->listPutIndex -= 1;
		if( wk->listPutIndex < 0 ){
			wk->listPutIndex = 7;
		}
		ZKNLISTMAIN_PutListCursor( wk, 1, oldPos );
		ZKNLISTOBJ_ChgListPosAnm( wk, oldPos, FALSE );
		PutListOne( wk, wk->listPutIndex, nowScroll );
		PutListOneSub( wk, wk->listPutIndex, nowScroll-7 );
		ZKNLISTOBJ_PutScrollList( wk, wk->listPutIndex, ZKNLISTBGWFRM_LISTPUT_UP );
		ZKNLISTOBJ_SetListScrollBarPos( wk );
		ZKNLISTBGWFRM_PutScrollList( wk, ZKNLISTBGWFRM_IDX_NAME_M1+wk->listPutIndex, ZKNLISTBGWFRM_LISTPUT_UP );
		ZKNLISTBGWFRM_PutScrollListSub( wk, ZKNLISTBGWFRM_IDX_NAME_S1+wk->listPutIndex, ZKNLISTBGWFRM_LISTPUT_UP );
		return;

	// 下方向にスクロール
	case ZKNLISTMAIN_LIST_MOVE_SCROLL_DOWN:
		{
			u8	idx = wk->listPutIndex + 7;
			if( idx >= 8 ){
				idx -= 8;
			}
			ZKNLISTMAIN_PutListCursor( wk, 1, oldPos );
			ZKNLISTOBJ_ChgListPosAnm( wk, oldPos, FALSE );
			PutListOne( wk, idx, nowScroll+6 );
			PutListOneSub( wk, idx, nowScroll-1 );
			ZKNLISTOBJ_PutScrollList( wk, idx, ZKNLISTBGWFRM_LISTPUT_DOWN );
			ZKNLISTOBJ_SetListScrollBarPos( wk );
			ZKNLISTBGWFRM_PutScrollList( wk, ZKNLISTBGWFRM_IDX_NAME_M1+idx, ZKNLISTBGWFRM_LISTPUT_DOWN );
			ZKNLISTBGWFRM_PutScrollListSub( wk, ZKNLISTBGWFRM_IDX_NAME_S1+idx, ZKNLISTBGWFRM_LISTPUT_DOWN );
			wk->listPutIndex += 1;
			if( wk->listPutIndex >= 8 ){
				wk->listPutIndex = 0;
			}
		}
		return;

	// 位置を直接指定
	case ZKNLISTMAIN_LIST_MOVE_POS_DIRECT:
		ZKNLISTMAIN_PutListCursor( wk, 1, oldPos );
		ZKNLISTMAIN_PutListCursor( wk, 2, nowPos );
		ZKNLISTOBJ_ChgListPosAnm( wk, oldPos, FALSE );
		ZKNLISTOBJ_ChgListPosAnm( wk, nowPos, TRUE );
		ZKNLISTOBJ_PutListPosPokeGra( wk, nowPos+nowScroll );
		return;

	// スクロール値を直接指定
	case ZKNLISTMAIN_LIST_MOVE_SCROLL_DIRECT:
		ZKNLISTMAIN_InitListPut( wk );
		return;
	}
}











typedef struct {
	STRBUF * str;
	u32	prm;
}LIST_DATA;

struct _ZUKAN_LIST_WORK {
	LIST_DATA * dat;
	s16	pos;
	s16	scroll;
	s16	posMax;
	s16	listMax;
	s16	listSize;
	void * work;
	pZUKAN_LIST_CALLBACK	func;
};

ZUKAN_LIST_WORK * ZKNLISTMAIN_CreateList( u32 siz, HEAPID heapID )
{
	ZUKAN_LIST_WORK * wk;
	wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(ZUKAN_LIST_WORK) );
	wk->dat = GFL_HEAP_AllocClearMemory( heapID, sizeof(LIST_DATA)*siz );
	wk->listSize = siz;
	return wk;
}

void ZKNLISTMAIN_ExitList( ZUKAN_LIST_WORK * wk )
{
	s32	i;

	for( i=0; i<wk->listMax; i++ ){
		GFL_STR_DeleteBuffer( wk->dat[i].str );
	}
	GFL_HEAP_FreeMemory( wk->dat );
	GFL_HEAP_FreeMemory( wk );
}

void ZKNLISTMAIN_AddListData( ZUKAN_LIST_WORK * wk, STRBUF * str, u32 prm )
{
	GF_ASSERT( wk->listMax < wk->listSize );

	wk->dat[wk->listMax].str = str;
	wk->dat[wk->listMax].prm = prm;
	wk->listMax++;
}

void ZKNLISTMAIN_InitList( ZUKAN_LIST_WORK * wk, s16 pos, s16 posMax, s16 scroll, void * work, pZUKAN_LIST_CALLBACK func )
{
	wk->pos    = pos;
	wk->posMax = posMax;
	if( wk->posMax > wk->listMax ){
		wk->posMax = wk->listMax;
	}
	wk->scroll = scroll;
	wk->work   = work;
	wk->func   = func;
}

s16 ZKNLISTMAIN_GetListPos( ZUKAN_LIST_WORK * wk )
{
	return wk->pos;
}

s16 ZKNLISTMAIN_GetListScroll( ZUKAN_LIST_WORK * wk )
{
	return wk->scroll;
}

s16 ZKNLISTMAIN_GetListPosMax( ZUKAN_LIST_WORK * wk )
{
	return wk->posMax;
}

s16 ZKNLISTMAIN_GetListMax( ZUKAN_LIST_WORK * wk )
{
	return wk->listMax;
}

s16 ZKNLISTMAIN_GetListCursorPos( ZUKAN_LIST_WORK * wk )
{
	return ( wk->pos + wk->scroll );
}

s16 ZKNLISTMAIN_GetListScrollMax( ZUKAN_LIST_WORK * wk )
{
	if( wk->posMax > wk->listMax ){
		return 0;
	}
	return ( wk->listMax - wk->posMax );
}

STRBUF * ZKNLISTMAIN_GetListStr( ZUKAN_LIST_WORK * wk, u32 pos )
{
	return wk->dat[pos].str;
}

u32 ZKNLISTMAIN_GetListParam( ZUKAN_LIST_WORK * wk, u32 pos )
{
	return wk->dat[pos].prm;
}

void ZKNLISTMAIN_SetListPos( ZUKAN_LIST_WORK * wk, s16 pos )
{
	wk->pos = pos;
}

void ZKNLISTMAIN_SetListScroll( ZUKAN_LIST_WORK * wk, s16 scroll )
{
	wk->scroll = scroll;
}




u32 ZKNLISTMAIN_Main( ZUKAN_LIST_WORK * wk )
{
	if( GFL_UI_KEY_GetCont() & PAD_KEY_UP  ){
		if( wk->pos != 0 ){
			s16	tmp = wk->pos;
			wk->pos--;
			wk->func( wk->work, wk->pos, wk->scroll, tmp, wk->scroll, ZKNLISTMAIN_LIST_MOVE_UP );
			return ZKNLISTMAIN_LIST_MOVE_UP;
		}else if( wk->scroll != 0 ){
			s16	tmp = wk->scroll;
			wk->scroll--;
			wk->func( wk->work, wk->pos, wk->scroll, wk->pos, tmp, ZKNLISTMAIN_LIST_MOVE_SCROLL_UP );
			return ZKNLISTMAIN_LIST_MOVE_SCROLL_UP;
		}
		return ZKNLISTMAIN_LIST_MOVE_NONE;
	}

	if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN ){
		if( wk->pos < (wk->posMax-1) ){
			s16	tmp = wk->pos;
			wk->pos++;
			wk->func( wk->work, wk->pos, wk->scroll, tmp, wk->scroll, ZKNLISTMAIN_LIST_MOVE_DOWN );
			return ZKNLISTMAIN_LIST_MOVE_DOWN;
		}else if( wk->scroll < (wk->listMax-wk->posMax) ){
			s16	tmp = wk->scroll;
			wk->scroll++;
			wk->func( wk->work, wk->pos, wk->scroll, wk->pos, tmp, ZKNLISTMAIN_LIST_MOVE_SCROLL_DOWN );
			return ZKNLISTMAIN_LIST_MOVE_SCROLL_DOWN;
		}
		return ZKNLISTMAIN_LIST_MOVE_NONE;
	}

	if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT ){
		return ZKNLISTMAIN_MoveLeft( wk );
	}

	if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT ){
		return ZKNLISTMAIN_MoveRight( wk );
	}

	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ){
		if( wk->pos != 0 || wk->scroll != 0 ){
			s16	tmpPos    = wk->pos;
			s16	tmpScroll = wk->scroll;
			wk->scroll = 0;
			wk->pos    = 0;
			wk->func( wk->work, wk->pos, wk->scroll, tmpPos, tmpScroll, ZKNLISTMAIN_LIST_MOVE_LEFT );
			return ZKNLISTMAIN_LIST_MOVE_LEFT;
		}
		return ZKNLISTMAIN_LIST_MOVE_NONE;
	}

	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ){
		if( wk->pos != (wk->posMax-1) || wk->scroll != (wk->listMax-wk->posMax) ){
			s16	tmpPos    = wk->pos;
			s16	tmpScroll = wk->scroll;
			wk->scroll = wk->listMax - wk->posMax;
			wk->pos    = wk->posMax - 1;
			wk->func( wk->work, wk->pos, wk->scroll, tmpPos, tmpScroll, ZKNLISTMAIN_LIST_MOVE_RIGHT );
			return ZKNLISTMAIN_LIST_MOVE_RIGHT;
		}
		return ZKNLISTMAIN_LIST_MOVE_NONE;
	}

	return ZKNLISTMAIN_LIST_MOVE_NONE;
}

u32 ZKNLISTMAIN_MoveLeft( ZUKAN_LIST_WORK * wk )
{
	if( wk->scroll != 0 ){
		s16	tmpPos    = wk->pos;
		s16	tmpScroll = wk->scroll;
		wk->scroll -= wk->posMax;
		if( wk->scroll < 0 ){
			wk->scroll = 0;
			wk->pos    = 0;
		}
		wk->func( wk->work, wk->pos, wk->scroll, tmpPos, tmpScroll, ZKNLISTMAIN_LIST_MOVE_LEFT );
		return ZKNLISTMAIN_LIST_MOVE_LEFT;
	}else if( wk->pos != 0 ){
		s16	tmp = wk->pos;
		wk->pos = 0;
		wk->func( wk->work, wk->pos, wk->scroll, tmp, wk->scroll, ZKNLISTMAIN_LIST_MOVE_LEFT );
		return ZKNLISTMAIN_LIST_MOVE_LEFT;
	}
	return ZKNLISTMAIN_LIST_MOVE_NONE;
}

u32 ZKNLISTMAIN_MoveRight( ZUKAN_LIST_WORK * wk )
{
	if( wk->scroll < (wk->listMax-wk->posMax) ){
		s16	tmpPos    = wk->pos;
		s16	tmpScroll = wk->scroll;
		wk->scroll += wk->posMax;
		if( wk->scroll > (wk->listMax-wk->posMax) ){
			wk->scroll = wk->listMax - wk->posMax;
			wk->pos    = wk->posMax - 1;
		}
		wk->func( wk->work, wk->pos, wk->scroll, tmpPos, tmpScroll, ZKNLISTMAIN_LIST_MOVE_RIGHT );
		return ZKNLISTMAIN_LIST_MOVE_RIGHT;
	}else if( wk->pos < (wk->posMax-1) ){
		s16	tmp = wk->pos;
		wk->pos = wk->posMax - 1;
		wk->func( wk->work, wk->pos, wk->scroll, tmp, wk->scroll, ZKNLISTMAIN_LIST_MOVE_RIGHT );
		return ZKNLISTMAIN_LIST_MOVE_RIGHT;
	}
	return ZKNLISTMAIN_LIST_MOVE_NONE;
}

void ZKNLISTMAIN_SetPosDirect( ZUKAN_LIST_WORK * wk, s16 pos )
{
	if( pos != wk->pos ){
		s16	tmpPos    = wk->pos;
		s16	tmpScroll = wk->scroll;
		ZKNLISTMAIN_SetListPos( wk, pos );
		wk->func( wk->work, wk->pos, wk->scroll, tmpPos, tmpScroll, ZKNLISTMAIN_LIST_MOVE_POS_DIRECT );
	}
}

void ZKNLISTMAIN_SetScrollDirect( ZUKAN_LIST_WORK * wk, s16 scroll )
{
	if( scroll != wk->scroll ){
		s16	tmpPos    = wk->pos;
		s16	tmpScroll = wk->scroll;
		ZKNLISTMAIN_SetListScroll( wk, scroll );
		wk->func( wk->work, wk->pos, wk->scroll, tmpPos, tmpScroll, ZKNLISTMAIN_LIST_MOVE_SCROLL_DIRECT );
	}
}

u32 ZKNLISTMAIN_SetListDirect( ZUKAN_LIST_WORK * wk, s16 pos, s16 mv, BOOL autoFlag )
{
	if( mv < 0  ){
		if( wk->scroll != 0 ){
			s16	tmpPos    = wk->pos;
			s16	tmpScroll = wk->scroll;
			wk->scroll--;
			if( autoFlag == FALSE ){
				wk->pos++;
			}
			wk->func( wk->work, wk->pos, wk->scroll, tmpPos, tmpScroll, ZKNLISTMAIN_LIST_MOVE_SCROLL_UP );
			return ZKNLISTMAIN_LIST_MOVE_SCROLL_UP;
		}
		return ZKNLISTMAIN_LIST_MOVE_NONE;
	}

	if( mv > 0 ){
		if( wk->scroll < (wk->listMax-wk->posMax) ){
			s16	tmpPos    = wk->pos;
			s16	tmpScroll = wk->scroll;
			wk->scroll++;
			if( autoFlag == FALSE ){
				wk->pos--;
			}
			wk->func( wk->work, wk->pos, wk->scroll, tmpPos, tmpScroll, ZKNLISTMAIN_LIST_MOVE_SCROLL_DOWN );
			return ZKNLISTMAIN_LIST_MOVE_SCROLL_DOWN;
		}
		return ZKNLISTMAIN_LIST_MOVE_NONE;
	}

	return ZKNLISTMAIN_LIST_MOVE_NONE;
}
