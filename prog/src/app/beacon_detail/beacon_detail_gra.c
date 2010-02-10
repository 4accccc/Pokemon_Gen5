//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		beacon_detail_gra.c
 *	@brief	すれ違い詳細画面グラフィック設定
 *	@author Miyuki Iwasawa	
 *	@data		2010.02.01
 *	@note 
 *	{	
 *	○概要
 *	ui_template_graphicはファイルごとコピペして使うことができる、グラフィック設定モジュールです。
 *	リソース読み込み、CLWK作成等は行わず、グラフィックの設定のみ行います。
 *	基本的に、.c上部のデータの書き換えだけで、BG,OBJ,G3Dの設定を変更でき、
 *	毎回書くような処理を軽減する目的で書かれております。
 *
 *	例えば、GFL_BG_BGCNT_HEADERの設定を変えておけば、
 *	勝手に設定が読み込まれる、と言った感じです。
 *	
 *	○中身
 *	ui_template_graphic.cの中身は、データ部と、関数部にわかれております。
 *	・以下、データ…と書かれたところから、以上がデータと書かれたところまでが
 *	データ部です。（現時点で22行～319行）
 * 	この中を変えるだけで、簡単に描画設定が終わる、というのを狙っております。
 *
 *	・構造体宣言から下が関数部です。
 *	基本変更する必要はありませんが、アプリ内で自分好みに変えたいときは、
 *	ご変更ください。
 *
 *	○使用方法
 *	１．使用したいフォルダへコピーしリネーム ui_template, UI_TEMPLATE -> config CONFIGなど
 *	２．共通設定にて、使用グラフィックのマクロを設定、バンク設定
 *	３．BG,OBJ,3Dなど、使用するグラフィックを設定
 *	４．外部公開関数を使い、使用したいソースと接続
 * 	}
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "print/gf_font.h"
#include "system/gfl_use.h"

//外部公開
#include "beacon_detail_gra.h"
#include "beacon_detail_def.h"

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_
//
//	以下、データ	この箇所を変更すれば、OKです
//		↓検索でジャンプ
//		共通設定
//		ＢＧ設定
//		ＯＢＪ設定
//		３Ｄ設定
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_
//=============================================================================
/**
 *					共通設定
*/
//=============================================================================
//-------------------------------------
///	使用グラフィックマクロ設定
//=====================================
#define GRAPHIC_BG_USE	//OFFにするとBG使用しません
#define GRAPHIC_OBJ_USE	//OFFにするとOBJ使用しません

//-------------------------------------
///	バンク設定
//=====================================
static const GFL_DISP_VRAM sc_vramSetTable =
{
	GX_VRAM_BG_256_AD,						// メイン2DエンジンのBG
//	GX_VRAM_BGEXTPLTT_0123_E,     // メイン2DエンジンのBG拡張パレット
	GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,				// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_128_B,					// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,	      // サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_NONE,						// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,				// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_32K,// メイン画面OBJマッピングモード		
	GX_OBJVRAMMODE_CHAR_1D_32K,// サブ画面OBJマッピングモード
};

//=============================================================================
/**
 *					ＢＧ設定
 *						・BGの設定（初期設定～BG面設定）
 *						・BMPWINの初期化
*/
//=============================================================================
#ifdef GRAPHIC_BG_USE
//-------------------------------------
///	BGグラフィックモード設定
//=====================================
static const GFL_BG_SYS_HEADER sc_bgsys_header	=
{	
	GX_DISPMODE_GRAPHICS,GX_BGMODE_5,GX_BGMODE_0,GX_BG0_AS_2D	//グラフィックモード、メインBG面設定、サブBG面設定、BG0面設定
};
//-------------------------------------
///	BG面設定
//=====================================
static const struct 
{
	u32									frame;
	GFL_BG_BGCNT_HEADER	bgcnt_header;
	u32									mode;
	BOOL								is_visible;
}	sc_bgsetup[]	=
{	
	//MAIN------------------------
	{	
		GFL_BG_FRAME0_M,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,	//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME1_M,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME2_M,	//設定するフレーム
		{
			0, 0, 0x1000, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x20000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_256X16,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME3_M,	//設定するフレーム
		{
			0, 0, 0x1000, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x30000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_256X16,//BGの種類
		TRUE,	//初期表示
	},

	//SUB---------------------------
	{	
		GFL_BG_FRAME0_S,	//設定するフレーム
		{
			0, 0, 0x1000, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME1_S,	//設定するフレーム
		{
			0, 0, 0x1000, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME2_S,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME3_S,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
};

#endif //GRAPHIC_BG_USE
//=============================================================================
/**
 *					ＯＢＪ設定
*/
//=============================================================================
#ifdef GRAPHIC_OBJ_USE
//-------------------------------------
///	CLSYS_INITデータ
//=====================================
static const GFL_CLSYS_INIT sc_clsys_init	=
{	
	0, 0,		//メインサーフェースの左上X,Y座標
	0, 512,	//サブサーフェースの左上X,Y座標
	4, 124,	//メインOAM管理開始～終了	（通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
	4, 124,	//差bOAM管理開始～終了	（通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
	0,			//セルVram転送管理数
	32,32,32,32,	//ｷｬﾗ、ﾊﾟﾚｯﾄ、ｾﾙｱﾆﾒ、ﾏﾙﾁｾﾙの登録できるリソース管理最大数
  16, 16,				//メイン、サブのCGRVRAM管理領域開始オフセット（通信アイコン用に16以上にしてください）
};

//-------------------------------------
///	ワーク作成最大数
//=====================================
#define GRAPHIC_OBJ_CLWK_CREATE_MAX	(128)

#endif //GRAPHIC_OBJ_USE

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_
//
//	以上がデータ。	以下は基本的に書き換える必要がありません
//
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	BGワーク
//=====================================
typedef struct 
{
	int dummy;
} GRAPHIC_BG_WORK;
//-------------------------------------
///	OBJワーク
//=====================================
typedef struct 
{
	GFL_CLUNIT			*p_clunit;
  GFL_CLSYS_REND* cellRender;
} GRAPHIC_OBJ_WORK;

//-------------------------------------
///	グラフィックメインワーク
//=====================================
struct _BEACON_DETAIL_GRAPHIC_WORK
{
	GRAPHIC_BG_WORK		bg;
	GRAPHIC_OBJ_WORK	obj;
	GFL_TCB						*p_vblank_task;
};

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
//-------------------------------------
///	共通
//=====================================
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//-------------------------------------
///	BG
//=====================================
#ifdef GRAPHIC_BG_USE
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_Main( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
#endif //GRAPHIC_BG_USE
//-------------------------------------
///	OBJ
//=====================================
#ifdef GRAPHIC_OBJ_USE
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID );
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk );
static GFL_CLUNIT* GRAPHIC_OBJ_GetUnit( const GRAPHIC_OBJ_WORK *cp_wk );
#endif //GRAPHIC_OBJ_USE

//=============================================================================
/**
 *					グローバル
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック設定
 *
 *	@param	ディスプレイ選択	GX_DISP_SELECT_MAIN_SUB or GX_DISP_SELECT_SUB_MAIN
 *	@param	HEAPID heapID ヒープ
 */
//-----------------------------------------------------------------------------
BEACON_DETAIL_GRAPHIC_WORK * BEACON_DETAIL_GRAPHIC_Init( int display_select, HEAPID heapID )
{	
	BEACON_DETAIL_GRAPHIC_WORK * p_wk;
	p_wk	= GFL_HEAP_AllocMemory(heapID, sizeof(BEACON_DETAIL_GRAPHIC_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(BEACON_DETAIL_GRAPHIC_WORK) );

	//レジスタ初期化
	G2_BlendNone();
	G2S_BlendNone();	
	GX_SetVisibleWnd( GX_WNDMASK_NONE );
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );

	//VRAMクリアー
	GFL_DISP_ClearVRAM( 0 );

	//VRAMバンク設定
	GFL_DISP_SetBank( &sc_vramSetTable );

	//ディスプレイON
	GFL_DISP_SetDispSelect( display_select );
	GFL_DISP_SetDispOn();

	//表示
	GFL_DISP_GX_InitVisibleControl();
	GFL_DISP_GXS_InitVisibleControl();

	//フォント初期化
	GFL_FONTSYS_Init();

	//モジュール初期化
#ifdef GRAPHIC_BG_USE
	GRAPHIC_BG_Init( &p_wk->bg, heapID );
#endif //GRAPHIC_BG_USE
#ifdef GRAPHIC_OBJ_USE
	GRAPHIC_OBJ_Init( &p_wk->obj, &sc_vramSetTable, heapID );
#endif //GRAPHIC_OBJ_USE

	//VBlankTask登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB(Graphic_VBlankTask, p_wk, 0 );

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック破棄
 *
 *	@param	GRAPHIC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BEACON_DETAIL_GRAPHIC_Exit( BEACON_DETAIL_GRAPHIC_WORK *p_wk )
{	
	//VBLANKTask消去
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

	//モジュール破棄
#ifdef GRAPHIC_OBJ_USE
	GRAPHIC_OBJ_Exit( &p_wk->obj );
#endif //GRAPHIC_OBJ_USE
#ifdef GRAPHIC_BG_USE
	GRAPHIC_BG_Exit( &p_wk->bg );
#endif //GRAPHIC_BG_USE

	//デフォルト色へ戻す
	GFL_FONTSYS_SetDefaultColor();

	//レジスタ初期化
	G2_BlendNone();
	G2S_BlendNone();	
	GX_SetVisibleWnd( GX_WNDMASK_NONE );
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(BEACON_DETAIL_GRAPHIC_WORK) );
	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック2D描画処理
 *
 *	@param	GRAPHIC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BEACON_DETAIL_GRAPHIC_2D_Draw( BEACON_DETAIL_GRAPHIC_WORK *p_wk )
{	
#ifdef GRAPHIC_OBJ_USE
	GRAPHIC_OBJ_Main( &p_wk->obj );
#endif //GRAPHIC_OBJ_USE
#ifdef GRAPHIC_BG_USE
	GRAPHIC_BG_Main( &p_wk->bg );
#endif //GRAPHIC_BG_USE
}

//----------------------------------------------------------------------------
/**
 *	@brief	OBJCLUNITの取得
 *
 *	@param	const GRAPHIC_WORK *cp_wk		ワーク
 *
 *	@return	CLUNIT取得
 */
//-----------------------------------------------------------------------------
GFL_CLUNIT * BEACON_DETAIL_GRAPHIC_GetClunit( const BEACON_DETAIL_GRAPHIC_WORK *cp_wk )
{	
#ifdef GRAPHIC_OBJ_USE
	return GRAPHIC_OBJ_GetUnit( &cp_wk->obj );
#else
	return NULL;
#endif //GRAPHIC_OBJ_USE
}

//=============================================================================
/**
 *						共通
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィックVBLANK関数
 *
 *	@param	GRAPHIC_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work )
{
	BEACON_DETAIL_GRAPHIC_WORK *p_wk	= p_work;
#ifdef GRAPHIC_BG_USE
	GRAPHIC_BG_VBlankFunction( &p_wk->bg );
#endif //GRAPHIC_BG_USE
#ifdef GRAPHIC_OBJ_USE
	GRAPHIC_OBJ_VBlankFunction( &p_wk->obj );
#endif //GRAPHIC_OBJ_USE
}
//=============================================================================
/**
 *						BG
 */
//=============================================================================
#ifdef GRAPHIC_BG_USE
//----------------------------------------------------------------------------
/**
 *	@brief	BG	初期化
 *
 *	@param	GRAPHIC_BG_WORK *p_wk	ワーク
 *	@param	heapID								ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_BG_WORK) );

	//初期化
	GFL_BG_Init( heapID );
	GFL_BMPWIN_Init( heapID );

	//グラフィックモード設定
	{	
		GFL_BG_SetBGMode( &sc_bgsys_header );
	}

	//BG面設定
	{	
		int i;
		for( i = 0; i < NELEMS(sc_bgsetup); i++ )
		{	
			GFL_BG_SetBGControl( sc_bgsetup[i].frame, &sc_bgsetup[i].bgcnt_header, sc_bgsetup[i].mode );
			GFL_BG_ClearFrame( sc_bgsetup[i].frame );
			GFL_BG_SetVisible( sc_bgsetup[i].frame, sc_bgsetup[i].is_visible );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	BG	破棄
 *
 *	@param	GRAPHIC_BG_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk )
{	
	//BG面破棄
	{	
		int i;
		for( i = 0; i < NELEMS(sc_bgsetup); i++ )
		{	
			GFL_BG_FreeBGControl( sc_bgsetup[i].frame );
		}
	}

	//終了
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_BG_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	BG	メイン処理
 *
 *	@param	GRAPHIC_BG_WORK *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Main( GRAPHIC_BG_WORK *p_wk )
{	
}
//----------------------------------------------------------------------------
/**
 *	@brief	BG	VBlank関数
 *
 *	@param	GRAPHIC_BG_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk )
{	
	GFL_BG_VBlankFunc();
}
#endif //GRAPHIC_BG_USE
//=============================================================================
/**
 *				OBJ
 */
//=============================================================================
#ifdef GRAPHIC_OBJ_USE
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	初期化
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk			ワーク
 *	@param	GFL_DISP_VRAM* cp_vram_bank	バンクテーブル
 *	@param	heapID											ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_OBJ_WORK) );

	//システム作成
	GFL_CLACT_SYS_Create( &sc_clsys_init, cp_vram_bank, heapID );
	p_wk->p_clunit	= GFL_CLACT_UNIT_Create( GRAPHIC_OBJ_CLWK_CREATE_MAX, 0, heapID );

  //自作レンダラー作成
  {
    const GFL_REND_SURFACE_INIT renderInitData[ACT_SF_MAX] = {
      {0,0,256,192,CLSYS_DRAW_MAIN},
      {0,512,256,192,CLSYS_DRAW_SUB},
    };
    
    p_wk->cellRender = GFL_CLACT_USERREND_Create( renderInitData , ACT_SF_MAX , heapID );
    GFL_CLACT_USERREND_SetOAMAttrCulling( p_wk->cellRender, TRUE );
    GFL_CLACT_UNIT_SetUserRend( p_wk->p_clunit, p_wk->cellRender );
  }

//	GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );

	//表示
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	破棄
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk )
{	
	//システム破棄
  GFL_CLACT_USERREND_Delete( p_wk->cellRender );
	GFL_CLACT_UNIT_Delete( p_wk->p_clunit );
	GFL_CLACT_SYS_Delete();
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_OBJ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	メイン処理
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk )
{	
	GFL_CLACT_SYS_Main();
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	Vブランク処理
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk )
{	
	GFL_CLACT_SYS_VBlankFunc();
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画CLUNIT取得
 *
 *	@param	const GRAPHIC_OBJ_WORK *cp_wk	ワーク
 *
 *	@return	CLUNIT
 */
//-----------------------------------------------------------------------------
static GFL_CLUNIT* GRAPHIC_OBJ_GetUnit( const GRAPHIC_OBJ_WORK *cp_wk )
{	
	return cp_wk->p_clunit;
}
#endif// GRAPHIC_OBJ_USE


