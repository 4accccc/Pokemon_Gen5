//===================================================================
/**
 * @file	testmode.c
 */
//===================================================================
#include "gflib.h"
#include "main.h"
#include "textprint.h"

#include "testmode.h"

#include "sample_graphic/titledemo.naix"

//------------------------------------------------------------------
typedef struct {
	int						seq;
	u16						listPosition;
	void*					chrbuf;
	GFL_BMP_DATA*			bmpHeader;
	GFL_TEXT_PRINTPARAM*	textParam;

	GFL_BMPWIN*				bmpwin[32];

	GFL_G3D_RES*			g3Dres[8];
	GFL_G3D_OBJ*			g3Dobj[8];

	u16						work[16];
}TESTMODE_WORK;

TESTMODE_WORK* testmode;

void	TestModeInit(void);
void	TestModeMain(void);

static BOOL	TestModeControl( void );
static const GFL_PROC_DATA TestMainProcData;

enum {
	NUM_TITLE = 0,
	NUM_URL,
	NUM_SELECT1,
	NUM_SELECT2,
	NUM_SELECT3,
	NUM_SELECT4,
	NUM_SELECT5,
};

enum {
	MSG_WHITE = 0,
	MSG_RED,
};

static const char	test_index1[] = 
{"ゲームフリーク　プログラムライブラリ　テストモード\nGame Freak Libraries TestMode"};
static const char	test_index2[] = {"http://www.gamefreak.co.jp"};

static const char	test_select1[]	= {"わたなべ　てつや　　Tetsuya Watanabe"};
static const char	test_select2[]	= {"たまだ　そうすけ　　Sousuke Tamada"};
static const char	test_select3[]	= {"そがべ　ひさし　　Hisashi Sogabe"};
static const char	test_select4[]	= {"おおの　かつみ　　Katsumi Ohno"};
static const char	test_select5[]	= {"たや　まさお　　Masao Taya"};
static const char	test_select6[]	= {"なかむら　ひろゆき　　Hiroyuki Nakamura"};
static const char	test_select7[]	= {"まつだ　よしのり　　Yoshinori Matsuda"};
static const char	test_select8[]	= {"かがや　けいた　　Keita Kagaya"};
static const char	test_select9[]	= {"ごとう　だいすけ　　Daisuke Gotou"};
static const char	test_select10[]	= {"のはら　さとし　　Satoshi Nohara"};
static const char	test_select11[]	= {"たかはし　ともや　　Tomoya Takahashi"};
static const char	test_select12[]	= {"もり　あきと　　Akito Mori"};
static const char	test_select13[]	= {"おおた　ともみち　　Tomomichi Ohta"};
static const char	test_select14[]	= {"いわさわ　みゆき　　Miyuki Iwasawa"};
static const char	test_select15[]	= {"さいとう　のぞむ　　Nozomu Saitou"};

typedef struct {
	const char*	msg;
	u8 posx;
	u8 posy;
	u8 sizx;
	u8 sizy;
}TESTMODE_PRINTLIST;

static const TESTMODE_PRINTLIST indexList[] = {
	{ test_index1, 2,  1, 30, 2 },
	{ test_index2, 2, 22, 30, 1 },
};

static const TESTMODE_PRINTLIST selectList[] = {
#if 0
	{ test_select1,  4,  5, 24, 1 },
	{ test_select2,  4,  6, 24, 1 },
	{ test_select3,  4,  7, 24, 1 },
	{ test_select4,  4,  8, 24, 1 },
	{ test_select5,  4,  9, 24, 1 },
	{ test_select6,  4, 10, 24, 1 },
	{ test_select7,  4, 11, 24, 1 },
	{ test_select8,  4, 12, 24, 1 },
	{ test_select9,  4, 13, 24, 1 },
	{ test_select10, 4, 14, 24, 1 },
	{ test_select11, 4, 15, 24, 1 },
	{ test_select12, 4, 16, 24, 1 },
	{ test_select13, 4, 17, 24, 1 },
	{ test_select14, 4, 18, 24, 1 },
	{ test_select15, 4, 19, 24, 1 },
#else
	{ test_select1,  4,  5, 24, 1 },
	{ test_select2,  4,  7, 24, 1 },
	{ test_select3,  4,  9, 24, 1 },
	{ test_select4,  4, 11, 24, 1 },
	{ test_select5,  4, 13, 24, 1 },
	{ test_select6,  4, 15, 24, 1 },
	{ test_select11, 4, 17, 24, 1 },
#endif
};

#define TEXT_FRM	(GFL_BG_FRAME3_M)
//------------------------------------------------------------------
/**
 * @brief		初期化
 *
 * 現状、中身はルートプロセスの生成のみとなっている
 */
//------------------------------------------------------------------
void	TestModeInit(void)
{
	GFL_PROC_SysCallProc(NO_OVERLAY_ID, &TestMainProcData, NULL);
//	testmode = GFL_HEAP_AllocMemoryClear(GFL_HEAPID_APP,sizeof(TESTMODE_WORK));
}

//------------------------------------------------------------------
/**
 * @brief		メイン
 *
 * TestModeInitでセットされたプロセスが動作しているため、
 * 現状は中身は何もない状態となっている。
 */
//------------------------------------------------------------------
void	TestModeMain(void)
{
	/* 
	if( TestModeControl() == TRUE ){
		GFL_HEAP_FreeMemory( testmode );
	}
	*/
}


//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void	bg_init( void )
{
	u16 heapID = GFL_HEAPID_APP;

	//ＢＧシステム起動
	GFL_BG_sysInit( heapID );

	//ＶＲＡＭ設定
	{
		GX_SetBankForTex(GX_VRAM_TEX_01_AB);
		GX_SetBankForBG(GX_VRAM_BG_64_E);
		GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0_G); 
	}
	{
		//ＢＧモード設定
		GFL_BG_SYS_HEADER bgsysHeader = {
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
		};	
		GFL_BG_InitBG( &bgsysHeader );
	}
	{
		//ＢＧコントロール設定

		//BG3( message frame )
		GFL_BG_BGCNT_HEADER bgCont3 = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x04000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_BGCNT_HEADER bgCont0 = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x04000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_BGControlSet( TEXT_FRM, &bgCont3, GFL_BG_MODE_TEXT );
		GFL_BG_PrioritySet( TEXT_FRM, 0 );
		GFL_BG_VisibleSet( TEXT_FRM, VISIBLE_ON );
	}
	//ビットマップウインドウシステムの起動
	{
		GFL_BMPWIN_sysInit( heapID );
	}
	//３Ｄシステム起動
	{
		GFL_G3D_sysInit( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						0x1000, heapID, NULL );
		//BG0( 3D frame )
		GFL_BG_BGControlSet3D( 1 );
	}
}

static void	bg_exit( void )
{
	GFL_G3D_sysExit();
	GFL_BMPWIN_sysExit();
	GFL_BG_BGControlExit( TEXT_FRM );
	GFL_BG_sysExit();
}

//------------------------------------------------------------------
/**
 * @brief		メッセージビットマップウインドウコントロール
 */
//------------------------------------------------------------------
//作成
static void msg_bmpwin_make( u8 bmpwinNum, const char* msg, u8 px, u8 py, u8 sx, u8 sy )
{
	testmode->bmpwin[bmpwinNum] = GFL_BMPWIN_Create( TEXT_FRM, px, py, sx, sy, 0, 0 );

	testmode->textParam->bmp = GFL_BMPWIN_GetBmp( testmode->bmpwin[ bmpwinNum ] );
	testmode->textParam->writex = 0;
	testmode->textParam->writey = 0;
	GFL_TEXT_PrintSjisCode( msg, testmode->textParam );

	GFL_BMPWIN_UploadChar( testmode->bmpwin[bmpwinNum] );
	GFL_BMPWIN_MakeScrn( testmode->bmpwin[bmpwinNum] );
}
	
//破棄
static void msg_bmpwin_trush( u8 bmpwinNum )
{
	GFL_BMPWIN_Delete( testmode->bmpwin[bmpwinNum] );
}
	
//パレット再設定
static void msg_bmpwin_palset( u8 bmpwinNum, u8 pal )
{
	GFL_BMPWIN_SetPal( testmode->bmpwin[bmpwinNum], pal );
}
	
//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
static void	g2d_load( void )
{
	u16 heapID = GFL_HEAPID_APP;

	//フォント読み込み
	GFL_TEXT_sysInit( "gfl_graphic/gfl_font.dat" );
	//パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocMemoryLowClear( heapID, 16*2 );
		plt[0] = 0x5041;	//青(背景)

		plt[1] = 0x7fff;	//白
		GFL_BG_PaletteSet( TEXT_FRM, plt, 16*2, 0 );
		plt[1] = 0x001f;	//赤
		GFL_BG_PaletteSet( TEXT_FRM, plt, 16*2, 16*2 );

		GFL_HEAP_FreeMemory( plt );
	}
	//文字表示パラメータワーク作成
	{
		GFL_TEXT_PRINTPARAM* param = GFL_HEAP_AllocMemoryLowClear
										( heapID,sizeof(GFL_TEXT_PRINTPARAM));
		param->spacex = 1;
		param->spacey = 1;
		param->colorF = 1;
		param->colorB = 0;
		param->mode = GFL_TEXT_WRITE_16;
		testmode->textParam = param;
	}
	//文字表示ビットマップの作成
	{
		int i;

		for(i=0;i<NELEMS(indexList);i++){
			msg_bmpwin_make( NUM_TITLE+i, indexList[i].msg,
							indexList[i].posx, indexList[i].posy, 
							indexList[i].sizx, indexList[i].sizy );
		}
		for(i=0;i<NELEMS(selectList);i++){
			msg_bmpwin_make( NUM_SELECT1+i, selectList[i].msg,
							selectList[i].posx, selectList[i].posy,
							selectList[i].sizx, selectList[i].sizy );
		}
	}
	GFL_BG_LoadScreenReq( TEXT_FRM );
}

static void	g2d_unload( void )
{
	int i;

	for(i=0;i<NELEMS(selectList);i++){
		msg_bmpwin_trush( NUM_SELECT1+i );
	}
	for(i=0;i<NELEMS(indexList);i++){
		msg_bmpwin_trush( NUM_TITLE+i );
	}
	GFL_HEAP_FreeMemory( testmode->textParam );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄデータコントロール
 */
//------------------------------------------------------------------
#define TD_TITLEP_CMR_CPX_ED			(FX32_ONE * -148)
#define TD_TITLEP_CMR_CPY_ED			(FX32_ONE * 58)
#define TD_TITLEP_CMR_CPZ_ED			(FX32_ONE * 190)
#define TD_TITLEP_CMR_TPX_ED			(FX32_ONE * 8)
#define TD_TITLEP_CMR_TPY_ED			(FX32_ONE * -5)
#define TD_TITLEP_CMR_TPZ_ED			(FX32_ONE * 1)

#define TD_TITLE_CAMERA_PERSPWAY		(0x0b60)
#define TD_TITLE_CAMERA_CLIP_NEAR		(0)
#define TD_TITLE_CAMERA_CLIP_FAR		(FX32_ONE*100)

//作成
static void g3d_load( void )
{
	const char* path = "sample_graphic/titledemo.narc";

	//モデル＆テクスチャリソース読み込み
	testmode->g3Dres[0] = GFL_G3D_ResourceCreatePath( path, NARC_titledemo_title_air_nsbmd ); 
	//アニメーションリソース読み込み
	testmode->g3Dres[1] = GFL_G3D_ResourceCreatePath( path, NARC_titledemo_title_air_nsbta ); 
	//テクスチャリソース転送
	GFL_G3D_TransVramTex( testmode->g3Dres[0] );

	//３Ｄオブジェクト作成
	testmode->g3Dobj[0] = GFL_G3D_ObjCreate(	testmode->g3Dres[0], 0,		//model
												testmode->g3Dres[0],		//texture
												testmode->g3Dres[1], 0 );	//animation
	{
		//カメラセット
		VecFx32	targetPos = {TD_TITLEP_CMR_TPX_ED, TD_TITLEP_CMR_TPY_ED, TD_TITLEP_CMR_TPZ_ED};
		VecFx32	cameraPos = {TD_TITLEP_CMR_CPX_ED, TD_TITLEP_CMR_CPY_ED, TD_TITLEP_CMR_CPZ_ED};
		VecFx32 cameraUp = { 0, FX32_ONE, 0 };

		GFL_G3D_sysProjectionSet( GFL_G3D_PRJPERS,
						FX_SinIdx( TD_TITLE_CAMERA_PERSPWAY ), 
						FX_CosIdx( TD_TITLE_CAMERA_PERSPWAY ), 
						( FX32_ONE * 4 / 3 ), 0, 
						( 1 << FX32_SHIFT ), ( 1024 << FX32_SHIFT ), 0 );
		GFL_G3D_sysLookAtSet( &cameraPos, &cameraUp, &targetPos );
	}
	testmode->work[0] = 0;
}
	
//破棄
static void g3d_unload( void )
{
	GFL_G3D_ObjDelete( testmode->g3Dobj[0] ); 

	GFL_G3D_ResourceDelete( testmode->g3Dres[1] ); 
	GFL_G3D_ResourceDelete( testmode->g3Dres[0] ); 
}
	
static void g3d_draw( void )
{
	VecFx32 trans	= { 0, 0, 0 };											//座標
	MtxFx33 rotate	= { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };	//回転
	VecFx32 scale	= { FX32_ONE, FX32_ONE, FX32_ONE };						//スケール

	//描画開始
	GFL_G3D_DrawStart();
	//カメラグローバルステート設定		
 	GFL_G3D_DrawLookAt();

	//各オブジェクト描画
	{
		//オブジェクト描画開始
		GFL_G3D_ObjDrawStart();
		//オブジェクト情報計算
		{
			//回転計算
			VecFx32 rotate_tmp = { 0, 0, 0 };
			rotate_tmp.y = 0x100 * testmode->work[0];	//Ｙ軸回転
			GFL_G3D_ObjDrawRotateCalcYX( &rotate_tmp, &rotate );
		}
		//オブジェクト情報転送
		GFL_G3D_ObjDrawStatusSet( &trans, &rotate, &scale );
		//オブジェクト描画
		GFL_G3D_ObjDraw( testmode->g3Dobj[0] );

		//アニメーションコントロール
		if( GFL_G3D_ObjContAnmFrameInc( testmode->g3Dobj[0], FX32_ONE ) == FALSE ){
			GFL_G3D_ObjContAnmFrameReset( testmode->g3Dobj[0] );
		}
	}
	//描画終了（バッファスワップ）
	GFL_G3D_DrawEnd();							

	testmode->work[0]++;
}
	
//------------------------------------------------------------------
/**
 * @brief	プリント実験
 */
//------------------------------------------------------------------
static BOOL	TestModeControl( void )
{
	BOOL return_flag = FALSE;
	int i;

	switch( testmode->seq ){

	case 0:
		//初期化
		bg_init();
		testmode->listPosition = 0;
		testmode->seq++;
		break;

	case 1:
		//画面作成
		g2d_load();	//２Ｄデータ作成
		g3d_load();	//３Ｄデータ作成
		testmode->seq++;
		break;

	case 2:
		//パレットチェンジによるカーソル描画
		for(i=0;i<NELEMS(selectList);i++){
			if( i == testmode->listPosition ){
				msg_bmpwin_palset( NUM_SELECT1+i, MSG_RED );
			}else{
				msg_bmpwin_palset( NUM_SELECT1+i, MSG_WHITE );
			}
			GFL_BMPWIN_MakeScrn( testmode->bmpwin[NUM_SELECT1+i] );
		}
		GFL_BG_LoadScreenReq( TEXT_FRM );
		testmode->seq++;
		g3d_draw();		//３Ｄデータ描画
		break;

	case 3:
		//キー判定
		if( GFL_UI_KeyGetTrg() == PAD_BUTTON_A ) {
			if( testmode->listPosition == 1) {
				//とりあえずたまだのときだけ遷移する
				testmode->seq++;
			}
			if( testmode->listPosition == 3) {
				//とりあえずおおののときだけ遷移する
				testmode->seq++;
			}
		} else if( GFL_UI_KeyGetTrg() == PAD_KEY_UP ){
			if( testmode->listPosition > 0 ){
				testmode->listPosition--;
				testmode->seq--;
			}
		} else if( GFL_UI_KeyGetTrg() == PAD_KEY_DOWN ){
			if( testmode->listPosition < NELEMS(selectList)-1 ){
				testmode->listPosition++;
				testmode->seq--;
			}
		}
		g3d_draw();		//３Ｄデータ描画
		break;

	case 4:
		//終了
		g3d_unload();	//３Ｄデータ破棄
		g2d_unload();	//２Ｄデータ破棄
		bg_exit();
		return_flag = TRUE;
		break;
	}
	return return_flag;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	testmode = GFL_HEAP_AllocMemoryClear(GFL_HEAPID_APP,sizeof(TESTMODE_WORK));
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	if( TestModeControl() == TRUE ){
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	switch (testmode->listPosition) {
	case 0:
		//わたなべ
		break;
	case 1:
		//たまだ
		GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &DebugTamadaMainProcData, NULL);
		break;
	case 2:
		//そがべ
		break;
	case 3:
		//おおの
		GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &DebugOhnoMainProcData, NULL);
		break;
	case 4:
		//みつはら
		break;
	default:
		break;
	}
	GFL_HEAP_FreeMemory( testmode );
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static const GFL_PROC_DATA TestMainProcData = {
	TestModeProcInit,
	TestModeProcMain,
	TestModeProcEnd,
};

//------------------------------------------------------------------
//------------------------------------------------------------------




