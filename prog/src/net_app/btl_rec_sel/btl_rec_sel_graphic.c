//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		btl_rec_sel_graphic.c
 *	@brief	グラフィック設定
 *	@author	Koji Kawada
 *	@data		2010.01.27
 *	@version {1.0}
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
 *
 * 	@noto   ui_template_graphic.cをコピペして名前を変更し、今回の画面に合うように少し変更しただけです。
 *
 * 	モジュール名：BTL_REC_SEL_GRAPHIC
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "print/gf_font.h"
#include "system/gfl_use.h"

//外部公開
#include "btl_rec_sel_graphic.h"
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
#define GRAPHIC_G3D_USE	//OFFにすると3D使用しません
//-------------------------------------
///	バンク設定
//=====================================
static const GFL_DISP_VRAM sc_vramSetTable =
{
	GX_VRAM_BG_128_A,						// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,				// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_128_B,					// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,	      // サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_0_D,						// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_0_F,				// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_128K,// メイン画面OBJマッピングモード		
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
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D	//グラフィックモード、メインBG面設定、サブBG面設定、BG0面設定
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
#if 0	//3D設定のためいらない
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
#endif
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
			0, 0, 0x0800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME3_M,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},

	//SUB---------------------------
	{	
		GFL_BG_FRAME0_S,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME1_S,	//設定するフレーム
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
		GFL_BG_FRAME2_S,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
#if 0	//使わない場合は、このテーブルから消してください。(#if#endifじゃなくても大丈夫です)
	{	
		GFL_BG_FRAME3_S,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
#endif
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
#if 1	//上下画面つなげない場合
{	
	0, 0,		//メインサーフェースの左上X,Y座標
	0, 512,	//サブサーフェースの左上X,Y座標
	4, 124,	//メインOAM管理開始～終了	（通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
	4, 124,	//差bOAM管理開始～終了	（通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
	0,			//セルVram転送管理数
	32,32,32,32,	//ｷｬﾗ、ﾊﾟﾚｯﾄ、ｾﾙｱﾆﾒ、ﾏﾙﾁｾﾙの登録できるリソース管理最大数
  16, 16,				//メイン、サブのCGRVRAM管理領域開始オフセット（通信アイコン用に16以上にしてください）
};
#else	//上下画面つなげたい場合
{	
	0, 0,		//メインサーフェースの左上X,Y座標
	0, 192,	//サブサーフェースの左上X,Y座標
	4, 124,	//メインOAM管理開始～終了	（通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
	4, 124,	//差bOAM管理開始～終了	（通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
	0,			//セルVram転送管理数
	32,32,32,32,	//ｷｬﾗ、ﾊﾟﾚｯﾄ、ｾﾙｱﾆﾒ、ﾏﾙﾁｾﾙの登録できるリソース管理最大数
  16, 16,				//メイン、サブのCGRVRAM管理領域開始オフセット（通信アイコン用に16以上にしてください）
};
#endif
//-------------------------------------
///	ワーク作成最大数
//=====================================
#define GRAPHIC_OBJ_CLWK_CREATE_MAX	(128)

#endif //GRAPHIC_OBJ_USE
//=============================================================================
/**
 *					３Ｄ設定
*/
//=============================================================================
#ifdef GRAPHIC_G3D_USE
//-------------------------------------
///	テクスチャ、ﾊﾟﾚｯﾄのVRAMｻｲｽﾞ
//=====================================
#define GRAPHIC_G3D_TEXSIZE	(GFL_G3D_TEX128K)	//バンクのテクスチャイメージスロットｻｲｽﾞとあわせてください
#define GRAPHIC_G3D_PLTSIZE	(GFL_G3D_PLT32K)	//バンクのﾊﾟﾚｯﾄイメージスロットｻｲｽﾞとあわせてください

//-------------------------------------
///	カメラ位置
//=====================================
//static const VecFx32 sc_CAMERA_PER_POS		= { 0,0,FX32_CONST( 70 ) };	//位置
//static const VecFx32 sc_CAMERA_PER_UP			= { 0,FX32_ONE,0 };					//上方向
//static const VecFx32 sc_CAMERA_PER_TARGET	= { 0,0,FX32_CONST( 0 ) };	//ターゲット

static const VecFx32 sc_CAMERA_PER_POS	= { FX_F32_TO_FX32( 6.7f ), FX_F32_TO_FX32( 6.7f ), FX_F32_TO_FX32( 17.3f ) };
static const VecFx32 sc_CAMERA_PER_UP = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 2.6f ), FX_F32_TO_FX32( 0.0f ) };
static const VecFx32 sc_CAMERA_PER_TARGET		= { 0,						FX32_ONE,				0 };

//-------------------------------------
///	プロジェクション
//		プロジェクションの細かい調整は各自でおこなってください
//=====================================
//正射影カメラ。射影カメラ
static inline GFL_G3D_CAMERA* GRAPHIC_G3D_CAMERA_Create
		( const VecFx32* cp_pos, const VecFx32* cp_up, const VecFx32* cp_target, HEAPID heapID )
{
#if 1	//射影
	return GFL_G3D_CAMERA_Create(	GFL_G3D_PRJPERS, 
									FX_SinIdx( defaultCameraFovy/2 *PERSPWAY_COEFFICIENT ),
									FX_CosIdx( defaultCameraFovy/2 *PERSPWAY_COEFFICIENT ),
									defaultCameraAspect, 0,
									defaultCameraNear, defaultCameraFar, 0,
									cp_pos, cp_up, cp_target, heapID );
#else	//正射影	クリップ面は適当です
	return GFL_G3D_CAMERA_CreateOrtho( 
		// const fx32 top, const fx32 bottom, const fx32 left, const fx32 right, 
			FX32_CONST(24), -FX32_CONST(24), -FX32_CONST(32), FX32_CONST(32),
									defaultCameraNear, defaultCameraFar, 0,
									cp_pos, cp_up, cp_target, heapID );
#endif
}

//-------------------------------------
///	3D設定コールバック関数
//=====================================
static void Graphic_3d_SetUp( void )
{
	// ３Ｄ使用面の設定(表示＆プライオリティー)
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
	G2_SetBG0Priority(0);

	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	//	フォグ、エッジマーキング、トゥーンシェードの詳細設定は
	//	テーブルを作成して設定してください
	G3X_SetShading( GX_SHADING_HIGHLIGHT );	//シェーディング
	G3X_AntiAlias( FALSE );									//	アンチエイリアス
	G3X_AlphaTest( FALSE, 0 );							//	アルファテスト　　オフ
	G3X_AlphaBlend( TRUE );									//	アルファブレンド　オン
	G3X_EdgeMarking( FALSE );								//	エッジマーキング
	G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );	//フォグ

	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog
	// ビューポートの設定
	G3_ViewPort(0, 0, 255, 191);

	// ライト設定
	{
		static const GFL_G3D_LIGHT sc_GFL_G3D_LIGHT[] = 
		{
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
		};
		int i;
		
		for( i=0; i<NELEMS(sc_GFL_G3D_LIGHT); i++ ){
			GFL_G3D_SetSystemLight( i, &sc_GFL_G3D_LIGHT[i] );
		}
	}

	//レンダリングスワップバッファ
	GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
}

#endif //GRAPHIC_G3D_USE
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
} GRAPHIC_OBJ_WORK;
//-------------------------------------
///	3D描画環境
//=====================================
typedef struct 
{
	GFL_G3D_CAMERA		*p_camera;
} GRAPHIC_G3D_WORK;

//-------------------------------------
///	グラフィックメインワーク
//=====================================
struct _BTL_REC_SEL_GRAPHIC_WORK
{
	GRAPHIC_BG_WORK		bg;
	GRAPHIC_OBJ_WORK	obj;
	GRAPHIC_G3D_WORK		g3d;
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
//-------------------------------------
///	3D
//=====================================
#ifdef GRAPHIC_G3D_USE
static void GRAPHIC_G3D_Init( GRAPHIC_G3D_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_G3D_Exit( GRAPHIC_G3D_WORK *p_wk );
static void GRAPHIC_G3D_StartDraw( GRAPHIC_G3D_WORK *p_wk );
static void GRAPHIC_G3D_EndDraw( GRAPHIC_G3D_WORK *p_wk );
static GFL_G3D_CAMERA * GRAPHIC_G3D_GetCamera( const GRAPHIC_G3D_WORK *cp_wk );
static void Graphic_3d_SetUp( void );
#endif //GRAPHIC_G3D_USE

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
BTL_REC_SEL_GRAPHIC_WORK * BTL_REC_SEL_GRAPHIC_Init( int display_select, HEAPID heapID )
{	
	BTL_REC_SEL_GRAPHIC_WORK * p_wk;
	p_wk	= GFL_HEAP_AllocMemory(heapID, sizeof(BTL_REC_SEL_GRAPHIC_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(BTL_REC_SEL_GRAPHIC_WORK) );

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
#ifdef GRAPHIC_G3D_USE
	GRAPHIC_G3D_Init( &p_wk->g3d, heapID );
#endif //GRAPHIC_G3D_USE

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
void BTL_REC_SEL_GRAPHIC_Exit( BTL_REC_SEL_GRAPHIC_WORK *p_wk )
{	
	//VBLANKTask消去
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

	//モジュール破棄
#ifdef GRAPHIC_G3D_USE
	GRAPHIC_G3D_Exit( &p_wk->g3d );
#endif //GRAPHIC_G3D_USE
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
	GFL_STD_MemClear( p_wk, sizeof(BTL_REC_SEL_GRAPHIC_WORK) );
	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック2D描画処理
 *
 *	@param	GRAPHIC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BTL_REC_SEL_GRAPHIC_2D_Draw( BTL_REC_SEL_GRAPHIC_WORK *p_wk )
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
 *	@brief	グラフィック3D描画開始処理
 *
 *	@param	GRAPHIC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BTL_REC_SEL_GRAPHIC_3D_StartDraw( BTL_REC_SEL_GRAPHIC_WORK *p_wk )
{	
#ifdef GRAPHIC_G3D_USE
	GRAPHIC_G3D_StartDraw( &p_wk->g3d );
#endif //GRAPHIC_G3D_USE
}
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック3D描画開始終了処理
 *
 *	@param	GRAPHIC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BTL_REC_SEL_GRAPHIC_3D_EndDraw( BTL_REC_SEL_GRAPHIC_WORK *p_wk )
{	
#ifdef GRAPHIC_G3D_USE
	GRAPHIC_G3D_EndDraw( &p_wk->g3d );
#endif //GRAPHIC_G3D_USE
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
GFL_CLUNIT * BTL_REC_SEL_GRAPHIC_GetClunit( const BTL_REC_SEL_GRAPHIC_WORK *cp_wk )
{	
#ifdef GRAPHIC_OBJ_USE
	return GRAPHIC_OBJ_GetUnit( &cp_wk->obj );
#else
	return NULL;
#endif //GRAPHIC_OBJ_USE
}
//----------------------------------------------------------------------------
/**
 *	@brief	G3D_CAMERAの取得
 *
 *	@param	const GRAPHIC_WORK *cp_wk		ワーク
 *
 *	@return	CAMERA取得
 */
//-----------------------------------------------------------------------------
GFL_G3D_CAMERA * BTL_REC_SEL_GRAPHIC_GetCamera( const BTL_REC_SEL_GRAPHIC_WORK *cp_wk )
{	
#ifdef GRAPHIC_G3D_USE
	return GRAPHIC_G3D_GetCamera( &cp_wk->g3d );
#else
	return NULL;
#endif //GRAPHIC_G3D_USE
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
	BTL_REC_SEL_GRAPHIC_WORK *p_wk	= p_work;
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
	GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );

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
//=============================================================================
/**
 *					GRAPHIC_G3D
 */
//=============================================================================
#ifdef GRAPHIC_G3D_USE
//----------------------------------------------------------------------------
/**
 *	@brief	３D環境の初期化
 *
 *	@param	p_wk			ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_G3D_Init( GRAPHIC_G3D_WORK *p_wk, HEAPID heapID )
{
	GFL_G3D_Init( GFL_G3D_VMANLNK, GRAPHIC_G3D_TEXSIZE,
			GFL_G3D_VMANLNK, GRAPHIC_G3D_PLTSIZE, 0, heapID, Graphic_3d_SetUp );

	p_wk->p_camera = GRAPHIC_G3D_CAMERA_Create( &sc_CAMERA_PER_POS, &sc_CAMERA_PER_UP, &sc_CAMERA_PER_TARGET, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	３D環境の破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_G3D_Exit( GRAPHIC_G3D_WORK *p_wk )
{
	GFL_G3D_CAMERA_Delete( p_wk->p_camera );
	GFL_G3D_Exit();
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画開始
 *	
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_G3D_StartDraw( GRAPHIC_G3D_WORK *p_wk )
{	

	GFL_G3D_DRAW_Start();
	GFL_G3D_CAMERA_Switching( p_wk->p_camera );
	GFL_G3D_DRAW_SetLookAt();
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画終了
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_G3D_EndDraw( GRAPHIC_G3D_WORK *p_wk )
{	
	GFL_G3D_DRAW_End();
}
//----------------------------------------------------------------------------
/**
 *	@brief	カメラ取得
 *
 *	@param	const GRAPHIC_G3D_WORK *cp_wk		ワーク
 *
 *	@return	カメラ
 */
//-----------------------------------------------------------------------------
static GFL_G3D_CAMERA * GRAPHIC_G3D_GetCamera( const GRAPHIC_G3D_WORK *cp_wk )
{	
	return cp_wk->p_camera;
}
#endif// GRAPHIC_G3D_USE
