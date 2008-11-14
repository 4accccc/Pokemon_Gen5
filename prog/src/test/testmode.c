//===================================================================
/**
 * @file	testmode.c
 */
//===================================================================
#include <gflib.h>
#include <textprint.h>

#include "system\gfl_use.h"
#include "system\main.h"

#include "test\testmode.h"

#include "gamesystem/game_init.h"

void	TestModeSet(void);

//============================================================================================
//
//
//	メインコントロール
//
//
//============================================================================================
#define G3DUTIL_USE

enum{
	STARTSELECT_RET_NON = 0,
	STARTSELECT_RET_FIX,
	STARTSELECT_RET_BACK,
};

enum{
	TESTMODE_RET_NON = 0,
	TESTMODE_RET_FIX,
	TESTMODE_RET_BACK,
};

typedef struct {
	HEAPID					heapID;
	int						seq;
	u16						listPosition;
	u16 					select_mode;
	void*					chrbuf;
	GFL_BMP_DATA*			bmpHeader;
	GFL_TEXT_PRINTPARAM*	textParam;

	GFL_BMPWIN*				bmpwin[32];
#ifdef G3DUTIL_USE
	GFL_G3D_UTIL*			g3Dutil;
	u16						g3DutilUnitIdx;
#else
	GFL_G3D_RES*			g3Dres[4];
	GFL_G3D_RND*			g3Drnd[2];
	GFL_G3D_ANM*			g3Danm[2];
	GFL_G3D_OBJ*			g3Dobj[2];
#endif
	GFL_G3D_OBJSTATUS		status[2];

	GFL_TCB*				dbl3DdispVintr;

	u16						work[16];
	GFL_PTC_PTR				ptc;
	u8						spa_work[PARTICLE_LIB_HEAP_SIZE];
}TESTMODE_WORK;

typedef struct {
	const char*	msg;
	u8 posx;
	u8 posy;
	u8 sizx;
	u8 sizy;
}TESTMODE_PRINTLIST;

enum {
	MSG_WHITE = 0,
	MSG_RED,
};

#if 0
static	const	char	*GraphicFileTable[]={
	"test_graphic/spa.narc",
};
#endif

#include "../../arc/test_graphic/titledemo.naix"
#include "testmode.dat"

//ＢＧ設定関数
static void	bg_init( HEAPID heapID );
static void	bg_exit( void );
//ビットマップ設定関数
static void msg_bmpwin_make(TESTMODE_WORK * testmode, u8 bmpwinNum, const char* msg, u8 px, u8 py, u8 sx, u8 sy );
static void msg_bmpwin_trush( TESTMODE_WORK * testmode, u8 bmpwinNum );
static void msg_bmpwin_palset( TESTMODE_WORK * testmode, u8 bmpwinNum, u8 pal );
//２ＤＢＧ作成関数
static void	g2d_load_title( TESTMODE_WORK * testmode );
static void g2d_draw_title( TESTMODE_WORK * testmode );
static void	g2d_unload_title( TESTMODE_WORK * testmode );
static void	g2d_load_startsel( TESTMODE_WORK * testmode );
static void g2d_draw_startsel( TESTMODE_WORK * testmode );
static void	g2d_unload_startsel( TESTMODE_WORK * testmode );
static void	g2d_load_testmode( TESTMODE_WORK * testmode );
static void g2d_draw_testmode( TESTMODE_WORK * testmode );
static void	g2d_unload_testmode( TESTMODE_WORK * testmode );

//３ＤＢＧ作成関数
static void	g3d_load( TESTMODE_WORK * testmode );
static void g3d_draw( TESTMODE_WORK * testmode );
static void	g3d_unload( TESTMODE_WORK * testmode );
//エフェクト
static void g3d_control_effect( TESTMODE_WORK * testmode );

//------------------------------------------------------------------
/**
 * @brief	選択リストポジション取得
 */
//------------------------------------------------------------------
static u16	TestModeSelectPosGet( TESTMODE_WORK * testmode )
{
	return testmode->listPosition;
}

//------------------------------------------------------------------
/**
 * @brief	リスト選択
 */
//------------------------------------------------------------------
static BOOL TitleControl( TESTMODE_WORK *testmode )
{
	BOOL return_flag = FALSE;
	
	switch( testmode->seq ){
	case 0:
		bg_init( testmode->heapID );
		testmode->dbl3DdispVintr = GFUser_VIntr_CreateTCB
						( GFL_G3D_DOUBLE3D_VblankIntrTCB, NULL, 0 );
		//パーティクルリソース読み込み
//		testmode->ptc = GFL_PTC_Create
//					( testmode->spa_work, PARTICLE_LIB_HEAP_SIZE, TRUE, testmode->heapID );
//		GFL_PTC_SetResource(testmode->ptc,GFL_PTC_LoadArcResource(0,0,testmode->heapID),TRUE,NULL);

		//testmode->listPosition = 0;
		testmode->seq++;
		break;
	case 1:
		//タイトル画面作成 3D
		g3d_load(testmode);	//３Ｄデータ作成
		testmode->seq++;
	case 2:
		//タイトル画面作成 2D
		g2d_load_title(testmode);	//２Ｄデータ作成
		testmode->seq++;
		break;
	case 3:
		{
			int pad = GFL_UI_KEY_GetTrg();
			if( pad == PAD_BUTTON_A || pad == PAD_BUTTON_START ){
				testmode->select_mode = NUM_TITLESELECT_START;
				testmode->seq++;
			}else if( pad == PAD_BUTTON_SELECT ){
				testmode->select_mode = NUM_TITLESELECT_DEBUG;
				testmode->seq++;
			}
		}
		
		//パレットチェンジによるカーソル描画
		msg_bmpwin_palset( testmode, NUM_TITLE_0, MSG_WHITE);
		GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_TITLE_0] );
		msg_bmpwin_palset( testmode, NUM_TITLE_1, MSG_WHITE);
		GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_TITLE_1] );
		msg_bmpwin_palset( testmode, NUM_TITLE_2, MSG_RED );
		GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_TITLE_2] );
		g3d_control_effect(testmode);
		g2d_draw_title(testmode);		//２Ｄデータ描画
		g3d_draw(testmode);		//３Ｄデータ描画
		
		break;
	case 4:
		//タイトル抜け
		g2d_unload_title(testmode);	//２Ｄデータ破棄
		testmode->seq = 0;
		return_flag = TRUE;
		break;
	}
	
	return return_flag;
}

static int StartSelectControl( TESTMODE_WORK * testmode )
{
	int return_flag = STARTSELECT_RET_NON;
	int i;
	
	switch( testmode->seq ){
	case 0:
		//初期化
		testmode->seq++;
		break;
	case 1:
		//テストモード作成
		g2d_load_startsel(testmode);	//２Ｄデータ作成
		testmode->seq++;
		break;
	case 2:
		//パレットチェンジによるカーソル描画
		for(i=0;i<NELEMS(startselList);i++){
			if( i == testmode->listPosition ){
				//現在のカーソル位置を赤文字で表現
				msg_bmpwin_palset( testmode, NUM_STARTSEL_0+i, MSG_RED );
			}else{
				msg_bmpwin_palset( testmode, NUM_STARTSEL_0+i, MSG_WHITE );
			}
			//ビットマップスクリーン再描画
			GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_STARTSEL_0+i] );
		}
		testmode->seq++;

		g3d_control_effect(testmode);
		g2d_draw_startsel(testmode);		//２Ｄデータ描画
		g3d_draw(testmode);		//３Ｄデータ描画
		break;
	case 3:
		{	//キー判定
			int trg = GFL_UI_KEY_GetTrg();
			
			if( trg == PAD_BUTTON_A ){
				testmode->seq++;
			}else if( trg == PAD_BUTTON_B ){
				testmode->seq = 5;
			}else if( trg == PAD_KEY_UP ){
				if( testmode->listPosition > 0 ){
					testmode->listPosition--;
					testmode->seq--;
				}
			}else if( trg == PAD_KEY_DOWN ){
				if( testmode->listPosition < NELEMS(startselList)-1 ){
					testmode->listPosition++;
					testmode->seq--;
				}
			}
		}
		
		g3d_control_effect(testmode);
		g2d_draw_startsel(testmode);		//２Ｄデータ描画
		g3d_draw(testmode);		//３Ｄデータ描画
		break;
	case 4:
		//終了
		GFL_TCB_DeleteTask( testmode->dbl3DdispVintr );
		g3d_unload(testmode);	//３Ｄデータ破棄
		g2d_unload_startsel(testmode);	//２Ｄデータ破棄
		testmode->seq = 0;
		testmode->listPosition = 0;
		bg_exit();
		return_flag = STARTSELECT_RET_FIX;
		break;
	case 5:
		//タイトルに戻る
		g2d_unload_startsel( testmode );
		testmode->listPosition = 0;
		return_flag = STARTSELECT_RET_BACK;
		break;
	}
	
	return return_flag;
}

static int TestModeControl( TESTMODE_WORK * testmode )
{
	BOOL return_flag = TESTMODE_RET_NON;
	int i;

	switch( testmode->seq ){
	case 0:
		//初期化
		testmode->seq++;
		break;
	case 1:
		//テストモード作成
		g2d_load_testmode(testmode);	//２Ｄデータ作成
		testmode->seq++;
		break;
	case 2:
		//パレットチェンジによるカーソル描画
		for(i=0;i<NELEMS(selectList);i++){
			if( i == testmode->listPosition ){
				//現在のカーソル位置を赤文字で表現
				msg_bmpwin_palset( testmode, NUM_SELECT1+i, MSG_RED );
			}else{
				msg_bmpwin_palset( testmode, NUM_SELECT1+i, MSG_WHITE );
			}
			//ビットマップスクリーン再描画
			GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_SELECT1+i] );
		}
		//ＢＧスクリーン転送リクエスト発行
		//GFL_BG_LoadScreenReq( TEXT_FRM );
		testmode->seq++;
		g3d_control_effect(testmode);
		g2d_draw_testmode(testmode);		//２Ｄデータ描画
		g3d_draw(testmode);		//３Ｄデータ描画
		break;

	case 3:
		//キー判定
		{
			int trg = GFL_UI_KEY_GetTrg();
			
			if( trg == PAD_BUTTON_A ) {
				testmode->seq++;
			}else if( trg == PAD_BUTTON_B ){
				testmode->seq = 5;
			}else if( trg == PAD_KEY_UP ){
				if( testmode->listPosition > 0 ){
					testmode->listPosition--;
					testmode->seq--;
				}
			}else if( trg == PAD_KEY_DOWN ){
				if( testmode->listPosition < NELEMS(selectList)-1 ){
					testmode->listPosition++;
					testmode->seq--;
				}
			}
		}
		g3d_control_effect(testmode);
		g2d_draw_testmode(testmode);		//２Ｄデータ描画
		g3d_draw(testmode);		//３Ｄデータ描画
//		if(GFL_PTC_GetEmitterNum(testmode->ptc)<5){
//			VecFx32	pos={0,0,0};
//			GFL_PTC_CreateEmitter(testmode->ptc,0,&pos);
//		}
		break;

	case 4:
		//終了
		GFL_TCB_DeleteTask( testmode->dbl3DdispVintr );
		g3d_unload(testmode);	//３Ｄデータ破棄
		g2d_unload_testmode(testmode);	//２Ｄデータ破棄
		bg_exit();
		return_flag = TESTMODE_RET_FIX;
		break;
	case 5:
		//タイトルへ戻る
		g2d_unload_testmode(testmode);	//２Ｄデータ破棄
		testmode->listPosition = 0;
		return_flag = TESTMODE_RET_BACK;
		break;
	}
	
	return return_flag;
}

#if 0
static BOOL	TestModeControl( TESTMODE_WORK * testmode )
{
	BOOL return_flag = FALSE;
	int i;

	switch( testmode->seq ){

	case 0:
		//初期化
		bg_init( testmode->heapID );
		testmode->dbl3DdispVintr = GFUser_VIntr_CreateTCB
						( GFL_G3D_DOUBLE3D_VblankIntrTCB, NULL, 0 );

		//パーティクルリソース読み込み
//		testmode->ptc = GFL_PTC_Create
//					( testmode->spa_work, PARTICLE_LIB_HEAP_SIZE, TRUE, testmode->heapID );
//		GFL_PTC_SetResource(testmode->ptc,GFL_PTC_LoadArcResource(0,0,testmode->heapID),TRUE,NULL);

		//testmode->listPosition = 0;
		testmode->seq++;
		break;

	case 1:
		//タイトル画面作成
		g2d_load_title(testmode);	//２Ｄデータ作成
		g3d_load(testmode);	//３Ｄデータ作成
		testmode->seq++;
		break;

	case 2:
		//パレットチェンジによるカーソル描画
		for(i=0;i<NELEMS(selectList);i++){
			if( i == testmode->listPosition ){
				//現在のカーソル位置を赤文字で表現
				msg_bmpwin_palset( testmode, NUM_SELECT1+i, MSG_RED );
			}else{
				msg_bmpwin_palset( testmode, NUM_SELECT1+i, MSG_WHITE );
			}
			//ビットマップスクリーン再描画
			GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_SELECT1+i] );
		}
		//ＢＧスクリーン転送リクエスト発行
		//GFL_BG_LoadScreenReq( TEXT_FRM );
		testmode->seq++;

		g3d_control_effect(testmode);
		g2d_draw(testmode);		//２Ｄデータ描画
		g3d_draw(testmode);		//３Ｄデータ描画
		break;

	case 3:
		//キー判定
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A ) {
			testmode->seq++;
		} else if( GFL_UI_KEY_GetTrg() == PAD_KEY_UP ){
			if( testmode->listPosition > 0 ){
				testmode->listPosition--;
				testmode->seq--;
			}
		} else if( GFL_UI_KEY_GetTrg() == PAD_KEY_DOWN ){
			if( testmode->listPosition < NELEMS(selectList)-1 ){
				testmode->listPosition++;
				testmode->seq--;
			}
		}
		g3d_control_effect(testmode);
		g2d_draw(testmode);		//２Ｄデータ描画
		g3d_draw(testmode);		//３Ｄデータ描画
//		if(GFL_PTC_GetEmitterNum(testmode->ptc)<5){
//			VecFx32	pos={0,0,0};
//			GFL_PTC_CreateEmitter(testmode->ptc,0,&pos);
//		}
		break;

	case 4:
		//終了
		GFL_TCB_DeleteTask( testmode->dbl3DdispVintr );
		g3d_unload(testmode);	//３Ｄデータ破棄
		g2d_unload(testmode);	//２Ｄデータ破棄
		bg_exit();
		return_flag = TRUE;
		break;
	}
	return return_flag;
}
#endif

//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void	bg_init( HEAPID heapID )
{
	//ＢＧシステム起動
	GFL_BG_Init( heapID );

	//ＶＲＡＭ設定
	GX_SetBankForTex(GX_VRAM_TEX_01_AB);
	GX_SetBankForBG(GX_VRAM_BG_64_E);
	GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0_G); 

	//ＢＧモード設定
	GFL_BG_SetBGMode( &bgsysHeader );

	//ＢＧコントロール設定
	GFL_BG_SetBGControl( TEXT_FRM, &bgCont3, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( TEXT_FRM, TEXT_FRM_PRI );
	GFL_BG_SetVisible( TEXT_FRM, VISIBLE_ON );

	//ビットマップウインドウシステムの起動
	GFL_BMPWIN_Init( heapID );

	//３Ｄシステム起動
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, heapID, NULL );
	GFL_G3D_DOUBLE3D_Init( heapID );

	GFL_BG_SetBGControl3D( G3D_FRM_PRI );

	//ARCシステム初期化
//	GFL_ARC_Init(&GraphicFileTable[0],NELEMS(GraphicFileTable));	gfl_use.cで1回だけ初期化に変更

	//パーティクルシステム起動
//	GFL_PTC_Init(heapID);

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_DISP_SetDispOn();
}

static void	bg_exit( void )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );

//	GFL_PTC_Exit();
//	GFL_ARC_Exit();

	GFL_G3D_DOUBLE3D_Exit();
	GFL_G3D_Exit();
	GFL_BMPWIN_Exit();
	GFL_BG_FreeBGControl( TEXT_FRM );
	GFL_BG_Exit();
}

//------------------------------------------------------------------
/**
 * @brief		メッセージビットマップウインドウコントロール
 */
//------------------------------------------------------------------
static void msg_bmpwin_make
			(TESTMODE_WORK * testmode, u8 bmpwinNum, const char* msg, u8 px, u8 py, u8 sx, u8 sy )
{
	//ビットマップ作成
	testmode->bmpwin[bmpwinNum] = GFL_BMPWIN_Create( TEXT_FRM, px, py, sx, sy, 0, 
														GFL_BG_CHRAREA_GET_B );

	//テキストをビットマップに表示
	testmode->textParam->bmp = GFL_BMPWIN_GetBmp( testmode->bmpwin[ bmpwinNum ] );
	testmode->textParam->writex = 0;
	testmode->textParam->writey = 0;
	GFL_TEXT_PrintSjisCode( msg, testmode->textParam );

	//ビットマップキャラクターをアップデート
	GFL_BMPWIN_TransVramCharacter( testmode->bmpwin[bmpwinNum] );
	//ビットマップスクリーン作成
	//GFL_BMPWIN_MakeScreen( testmode->bmpwin[bmpwinNum] );
}
	
static void msg_bmpwin_trush( TESTMODE_WORK * testmode, u8 bmpwinNum )
{
	GFL_BMPWIN_Delete( testmode->bmpwin[bmpwinNum] );
}
	
static void msg_bmpwin_palset( TESTMODE_WORK * testmode, u8 bmpwinNum, u8 pal )
{
	GFL_BMPWIN_SetPalette( testmode->bmpwin[bmpwinNum], pal );
}
	
//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------

//--------------------------------------------------------------
//	タイトル
//--------------------------------------------------------------
static void	g2d_load_title( TESTMODE_WORK * testmode )
{
	//フォントパレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( testmode->heapID, 16*2 );
		plt[0] = G2D_BACKGROUND_COL;
		plt[1] = G2D_FONT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 0 );
		plt[1] = G2D_FONTSELECT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 16*2 );
	
		GFL_HEAP_FreeMemory( plt );
	}
	
	//文字表示パラメータワーク作成
	{
		GFL_TEXT_PRINTPARAM* param = GFL_HEAP_AllocClearMemoryLo
						(testmode->heapID,sizeof(GFL_TEXT_PRINTPARAM));
		*param = default_param;
		testmode->textParam = param;
	}
	
	//文字表示ビットマップの作成
	{
		int i;
		//選択項目ビットマップの作成
		for(i=0;i<NELEMS(titleList);i++){
			msg_bmpwin_make( testmode, i, titleList[i].msg,
				titleList[i].posx, titleList[i].posy,
				titleList[i].sizx, titleList[i].sizy );
		}
	}
}

static void g2d_draw_title( TESTMODE_WORK * testmode )
{
	int	i;
	
	GFL_BG_ClearScreen( TEXT_FRM );
	
	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		//表題ビットマップの表示
		GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_TITLE_0] );
		
		//選択項目ビットマップの表示
		for(i=0;i<NUM_TITLE_MAX;i++){
			GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_TITLE_0+i] );
		}
		//GFL_BG_SetVisible( TEXT_FRM, VISIBLE_ON );
	}
	GFL_BG_LoadScreenReq( TEXT_FRM );
}

static void	g2d_unload_title( TESTMODE_WORK * testmode )
{
	int i;

	//選択項目ビットマップの破棄
	for(i=0;i<NELEMS(titleList);i++){
		msg_bmpwin_trush( testmode, NUM_TITLE_0+i );
	}
	GFL_HEAP_FreeMemory( testmode->textParam );
}

//--------------------------------------------------------------
//	さいしょから　つづきから
//--------------------------------------------------------------
static void	g2d_load_startsel( TESTMODE_WORK * testmode )
{
	//フォントパレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( testmode->heapID, 16*2 );
		plt[0] = G2D_BACKGROUND_COL;
		plt[1] = G2D_FONT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 0 );
		plt[1] = G2D_FONTSELECT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 16*2 );
	
		GFL_HEAP_FreeMemory( plt );
	}
	
	//文字表示パラメータワーク作成
	{
		GFL_TEXT_PRINTPARAM* param = GFL_HEAP_AllocClearMemoryLo
						(testmode->heapID,sizeof(GFL_TEXT_PRINTPARAM));
		*param = default_param;
		testmode->textParam = param;
	}
	
	//文字表示ビットマップの作成
	{
		int i;
		//選択項目ビットマップの作成
		for(i=0;i<NELEMS(startselList);i++){
			msg_bmpwin_make( testmode, i, startselList[i].msg,
				startselList[i].posx, startselList[i].posy,
				startselList[i].sizx, startselList[i].sizy );
		}
	}
}

static void g2d_draw_startsel( TESTMODE_WORK * testmode )
{
	int	i;
	
	GFL_BG_ClearScreen( TEXT_FRM );
	
	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		//選択項目ビットマップの表示
		for(i=0;i<NUM_STARTSEL_MAX;i++){
			GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_STARTSEL_0+i] );
		}
		//GFL_BG_SetVisible( TEXT_FRM, VISIBLE_ON );
	}
	GFL_BG_LoadScreenReq( TEXT_FRM );
}

static void	g2d_unload_startsel( TESTMODE_WORK * testmode )
{
	int i;
	
	//選択項目ビットマップの破棄
	for(i=0;i<NELEMS(startselList);i++){
		msg_bmpwin_trush( testmode, NUM_STARTSEL_0+i );
	}
	GFL_HEAP_FreeMemory( testmode->textParam );
}

//--------------------------------------------------------------
//	テストモード
//--------------------------------------------------------------
static void	g2d_load_testmode( TESTMODE_WORK * testmode )
{
	//フォントパレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( testmode->heapID, 16*2 );
		plt[0] = G2D_BACKGROUND_COL;
		plt[1] = G2D_FONT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 0 );
		plt[1] = G2D_FONTSELECT_COL;
		GFL_BG_LoadPalette( TEXT_FRM, plt, 16*2, 16*2 );
	
		GFL_HEAP_FreeMemory( plt );
	}
	//文字表示パラメータワーク作成
	{
		GFL_TEXT_PRINTPARAM* param = GFL_HEAP_AllocClearMemoryLo
										( testmode->heapID,sizeof(GFL_TEXT_PRINTPARAM));
		*param = default_param;
		testmode->textParam = param;
	}
	//文字表示ビットマップの作成
	{
		int i;

		//表題ビットマップの作成
		for(i=0;i<NELEMS(indexList);i++){
			msg_bmpwin_make( testmode, NUM_TITLE+i, indexList[i].msg,
							indexList[i].posx, indexList[i].posy, 
							indexList[i].sizx, indexList[i].sizy );
		}
		//選択項目ビットマップの作成
		for(i=0;i<NELEMS(selectList);i++){
			msg_bmpwin_make( testmode, NUM_SELECT1+i, selectList[i].msg,
							selectList[i].posx, selectList[i].posy,
							selectList[i].sizx, selectList[i].sizy );
		}
	}
	//ＢＧスクリーン転送リクエスト発行
	//GFL_BG_LoadScreenReq( TEXT_FRM );
}

static void g2d_draw_testmode( TESTMODE_WORK * testmode )
{
	int	i;

	GFL_BG_ClearScreen( TEXT_FRM );

	if( GFL_G3D_DOUBLE3D_GetFlip() ){
		//表題ビットマップの表示
		GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_TITLE] );

		//選択項目ビットマップの表示
		for(i=0;i<NUM_COUNT_PAGE1;i++){
			GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_SELECT_PAGE1+i] );
		}
		//GFL_BG_SetVisible( TEXT_FRM, VISIBLE_ON );
	} else {
		//表題ビットマップの表示
		GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_URL] );

		//選択項目ビットマップの表示
#if 0
		for(i=0;i<NUM_COUNT_PAGE2;i++){
			GFL_BMPWIN_MakeScreen( testmode->bmpwin[NUM_SELECT_PAGE2+i] );
		}
#endif
		//GFL_BG_SetVisible( TEXT_FRM, VISIBLE_OFF );
	}
	GFL_BG_LoadScreenReq( TEXT_FRM );
}

static void	g2d_unload_testmode( TESTMODE_WORK * testmode )
{
	int i;

	//選択項目ビットマップの破棄
	for(i=0;i<NELEMS(selectList);i++){
		msg_bmpwin_trush( testmode, NUM_SELECT1+i );
	}
	//表題ビットマップの破棄
	for(i=0;i<NELEMS(indexList);i++){
		msg_bmpwin_trush( testmode, NUM_TITLE+i );
	}
	GFL_HEAP_FreeMemory( testmode->textParam );
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄデータ
 */
//------------------------------------------------------------------
static const char g3DarcPath[] = {"titledemo.narc"};

enum {
	G3DRES_AIR_BMD = 0,
	G3DRES_AIR_BTA,
	G3DRES_IAR_BMD,
	G3DRES_IAR_BTA,
};

enum {
	G3D_AIR = 0,
	G3D_IAR,
};

#ifdef G3DUTIL_USE

#define G3DUTIL_RESCOUNT	(4)
#define G3DUTIL_OBJCOUNT	(2)

static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ (u32)g3DarcPath, NARC_titledemo_title_air_nsbmd, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_titledemo_title_air_nsbta, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_titledemo_title_iar_nsbmd, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_titledemo_title_iar_nsbta, GFL_G3D_UTIL_RESPATH },
};

static const GFL_G3D_UTIL_ANM g3Dutil_anm1Tbl[] = {
	{ G3DRES_AIR_BTA, 0 },
};

static const GFL_G3D_UTIL_ANM g3Dutil_anm2Tbl[] = {
	{ G3DRES_IAR_BTA, 0 },
};

static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{ G3DRES_AIR_BMD, 0, G3DRES_AIR_BMD, g3Dutil_anm1Tbl, NELEMS(g3Dutil_anm1Tbl) },
	{ G3DRES_IAR_BMD, 0, G3DRES_IAR_BMD, g3Dutil_anm2Tbl, NELEMS(g3Dutil_anm2Tbl) },
};

static const GFL_G3D_UTIL_SETUP g3Dutil_setup = {
	g3Dutil_resTbl, NELEMS(g3Dutil_resTbl),
	g3Dutil_objTbl, NELEMS(g3Dutil_objTbl),
};
#endif

//------------------------------------------------------------------
/**
 * @brief		３Ｄデータコントロール
 */
//------------------------------------------------------------------
static void g3d_load( TESTMODE_WORK * testmode )
{
#ifdef G3DUTIL_USE
	testmode->g3Dutil = GFL_G3D_UTIL_Create( G3DUTIL_RESCOUNT, G3DUTIL_OBJCOUNT, testmode->heapID );
	testmode->g3DutilUnitIdx = GFL_G3D_UTIL_AddUnit( testmode->g3Dutil, &g3Dutil_setup );

	{//		アニメーションを有効にする
		u16				objIdx;
		GFL_G3D_OBJ*	g3Dobj;

		objIdx = GFL_G3D_UTIL_GetUnitObjIdx( testmode->g3Dutil, testmode->g3DutilUnitIdx );

		g3Dobj = GFL_G3D_UTIL_GetObjHandle( testmode->g3Dutil, objIdx+G3D_AIR );
		GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle( testmode->g3Dutil, G3D_AIR  ), 0 ); 
		g3Dobj = GFL_G3D_UTIL_GetObjHandle( testmode->g3Dutil, objIdx+G3D_IAR );
		GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle( testmode->g3Dutil, G3D_IAR  ), 0 ); 
	}
#else
	//		リソースセットアップ
	testmode->g3Dres[ G3DRES_AIR_BMD ] = GFL_G3D_CreateResourcePath
										( g3DarcPath, NARC_titledemo_title_air_nsbmd );
	testmode->g3Dres[ G3DRES_AIR_BTA ] = GFL_G3D_CreateResourcePath
										( g3DarcPath, NARC_titledemo_title_air_nsbta );
	testmode->g3Dres[ G3DRES_IAR_BMD ] = GFL_G3D_CreateResourcePath
										( g3DarcPath, NARC_titledemo_title_iar_nsbmd );
	testmode->g3Dres[ G3DRES_IAR_BTA ] = GFL_G3D_CreateResourcePath
										( g3DarcPath, NARC_titledemo_title_iar_nsbta );
	
	//		リソース転送
	GFL_G3D_TransVramTexture( testmode->g3Dres[ G3DRES_AIR_BMD ] );
	GFL_G3D_TransVramTexture( testmode->g3Dres[ G3DRES_IAR_BMD ] );
	//		レンダー作成
	testmode->g3Drnd[ G3D_AIR ] = GFL_G3D_RENDER_Create(	testmode->g3Dres[ G3DRES_AIR_BMD ], 0, 
															testmode->g3Dres[ G3DRES_AIR_BMD ] );
	testmode->g3Drnd[ G3D_IAR ] = GFL_G3D_RENDER_Create(	testmode->g3Dres[ G3DRES_IAR_BMD ], 0, 
															testmode->g3Dres[ G3DRES_IAR_BMD] );
	//		アニメ作成
	testmode->g3Danm[ G3D_AIR ] = GFL_G3D_ANIME_Create(	testmode->g3Drnd[ G3D_AIR ], 
														testmode->g3Dres[ G3DRES_AIR_BTA ], 0 );
	testmode->g3Danm[ G3D_IAR ] = GFL_G3D_ANIME_Create(	testmode->g3Drnd[ G3D_IAR ], 
														testmode->g3Dres[ G3DRES_IAR_BTA ], 0 );
	//		オブジェクト作成
	testmode->g3Dobj[ G3D_AIR ] = GFL_G3D_OBJECT_Create(	testmode->g3Drnd[ G3D_AIR ], 
															&testmode->g3Danm[ G3D_AIR ], 1 );
	testmode->g3Dobj[ G3D_IAR ] = GFL_G3D_OBJECT_Create(	testmode->g3Drnd[ G3D_IAR ], 
															&testmode->g3Danm[ G3D_IAR ], 1 );
	//		アニメーションを有効にする
	GFL_G3D_OBJECT_EnableAnime( testmode->g3Dobj[ G3D_AIR ], 0 ); 
	GFL_G3D_OBJECT_EnableAnime( testmode->g3Dobj[ G3D_IAR ], 0 ); 
#endif

	//描画ステータスワーク設定
	testmode->status[ G3D_AIR ] = status0;
	testmode->status[ G3D_IAR ] = status1;
#if 1
	//カメラセット
	{
		GFL_G3D_PROJECTION initProjection = { GFL_G3D_PRJPERS, 0, 0, cameraAspect, 0, 
												cameraNear, cameraFar, 0 };
		initProjection.param1 = FX_SinIdx( cameraPerspway ); 
		initProjection.param2 = FX_CosIdx( cameraPerspway ); 
		GFL_G3D_SetSystemProjection( &initProjection );	
		GFL_G3D_SetSystemLookAt( &cameraLookAt );	
	}
#endif
	testmode->work[0] = 0;
}
	
static void g3d_draw( TESTMODE_WORK * testmode )
{
	GFL_G3D_OBJ* g3Dobj[2];

#ifdef G3DUTIL_USE
	{
		u16				objIdx;
		objIdx = GFL_G3D_UTIL_GetUnitObjIdx( testmode->g3Dutil, testmode->g3DutilUnitIdx );

		g3Dobj[ G3D_AIR ] = GFL_G3D_UTIL_GetObjHandle( testmode->g3Dutil, objIdx+G3D_AIR );
		g3Dobj[ G3D_IAR ] = GFL_G3D_UTIL_GetObjHandle( testmode->g3Dutil, objIdx+G3D_IAR );
	}
#else
	g3Dobj[ G3D_AIR ] = testmode->g3Dobj[ G3D_AIR ];
	g3Dobj[ G3D_IAR ] = testmode->g3Dobj[ G3D_IAR ];
#endif
	GFL_G3D_DRAW_Start();

//	GFL_PTC_Main();

#if 0
	//カメラセット
	{
		GFL_G3D_PROJECTION initProjection = { GFL_G3D_PRJPERS, 0, 0, cameraAspect, 0, 
												cameraNear, cameraFar, 0 };
		initProjection.param1 = FX_SinIdx( cameraPerspway ); 
		initProjection.param2 = FX_CosIdx( cameraPerspway ); 
		GFL_G3D_SetSystemProjection( &initProjection );	
		GFL_G3D_SetSystemLookAt( &cameraLookAt );	
	}
#endif

	GFL_G3D_DRAW_SetLookAt();
	{
		if( GFL_G3D_DOUBLE3D_GetFlip() ){
			GFL_G3D_DRAW_DrawObject( g3Dobj[ G3D_AIR ], &testmode->status[ G3D_AIR ] );
		} else {
			GFL_G3D_DRAW_DrawObject( g3Dobj[ G3D_IAR ], &testmode->status[ G3D_IAR ] );
		}
	}
	GFL_G3D_DRAW_End();
	GFL_G3D_DOUBLE3D_SetSwapFlag();

	GFL_G3D_OBJECT_LoopAnimeFrame( g3Dobj[ G3D_AIR ], 0, FX32_ONE ); 
	GFL_G3D_OBJECT_LoopAnimeFrame( g3Dobj[ G3D_IAR ], 0, FX32_ONE ); 
}
	
static void g3d_unload( TESTMODE_WORK * testmode )
{
#ifdef G3DUTIL_USE
	GFL_G3D_UTIL_DelUnit( testmode->g3Dutil, testmode->g3DutilUnitIdx );
	GFL_G3D_UTIL_Delete( testmode->g3Dutil );
#else
	GFL_G3D_OBJECT_Delete( testmode->g3Dobj[ G3D_IAR ] );
	GFL_G3D_OBJECT_Delete( testmode->g3Dobj[ G3D_AIR ] );

	GFL_G3D_ANIME_Delete( testmode->g3Danm[ G3D_IAR ] );
	GFL_G3D_ANIME_Delete( testmode->g3Danm[ G3D_AIR ] );

	GFL_G3D_RENDER_Delete( testmode->g3Drnd[ G3D_IAR ] );
	GFL_G3D_RENDER_Delete( testmode->g3Drnd[ G3D_AIR ] );

	GFL_G3D_FreeVramTexture( testmode->g3Dres[ G3DRES_IAR_BMD ] );
	GFL_G3D_FreeVramTexture( testmode->g3Dres[ G3DRES_AIR_BMD ] );

	GFL_G3D_DeleteResource( testmode->g3Dres[ G3DRES_IAR_BTA ] );
	GFL_G3D_DeleteResource( testmode->g3Dres[ G3DRES_IAR_BMD ] );
	GFL_G3D_DeleteResource( testmode->g3Dres[ G3DRES_AIR_BTA ] );
	GFL_G3D_DeleteResource( testmode->g3Dres[ G3DRES_AIR_BMD ] );
#endif
}
	
//------------------------------------------------------------------
/**
 * @brief	３Ｄ動作
 */
//------------------------------------------------------------------
static inline void rotateCalc( VecFx32* rotSrc, MtxFx33* rotDst )
{
	MtxFx33 tmp;

	MTX_RotX33(	rotDst, FX_SinIdx((u16)rotSrc->x), FX_CosIdx((u16)rotSrc->x) );

	MTX_RotY33(	&tmp, FX_SinIdx((u16)rotSrc->y), FX_CosIdx((u16)rotSrc->y) );
	MTX_Concat33( rotDst, &tmp, rotDst );

	MTX_RotZ33(	&tmp, FX_SinIdx((u16)rotSrc->z), FX_CosIdx((u16)rotSrc->z) );
	MTX_Concat33( rotDst, &tmp, rotDst );
}

static void g3d_control_effect( TESTMODE_WORK * testmode )
{
	MtxFx33 rotate;
	VecFx32 rotate_tmp = { 0, 0, 0 };
	GFL_G3D_OBJ* g3Dobj;

	//回転計算
	rotate_tmp.y = g3DanmRotateSpeed * testmode->work[0];	//Ｙ軸回転
	rotateCalc( &rotate_tmp, &testmode->status[0].rotate );

	rotate_tmp.y = -g3DanmRotateSpeed * testmode->work[0];	//Ｙ軸回転
	rotateCalc( &rotate_tmp, &testmode->status[1].rotate );

	testmode->work[0]++;
}
	
//------------------------------------------------------------------
extern const GFL_PROC_DATA DebugWatanabeMainProcData;
extern const GFL_PROC_DATA TestProg1MainProcData;
extern const GFL_PROC_DATA DebugOhnoMainProcData;
extern const GFL_PROC_DATA DebugLayoutMainProcData;
extern const GFL_PROC_DATA DebugTayaMainProcData;
extern const GFL_PROC_DATA DebugMatsudaMainProcData;
extern const GFL_PROC_DATA DebugMatsudaNetProcData;
extern const GFL_PROC_DATA DebugMatsudaIrcMatchProcData;
extern const GFL_PROC_DATA DebugGotoMainProcData;
extern const GFL_PROC_DATA DebugSogabeMainProcData;
extern const GFL_PROC_DATA DebugAriizumiMainProcData;
extern const GFL_PROC_DATA DebugDLPlayMainProcData;

extern const GFL_PROC_DATA DebugFieldProcData;

//------------------------------------------------------------------
static void CallSelectProc( TESTMODE_WORK * testmode )
{
	switch( TestModeSelectPosGet(testmode) ) {
	case SELECT_WATANABE:
		//わたなべ
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugWatanabeMainProcData, NULL);
		break;
	case SELECT_TAMADA:
		//たまだ
		//GFL_PROC_SysCallProc(FS_OVERLAY_ID(debug_tamada), &DebugTamadaMainProcData, NULL);
		{
			GAME_INIT_WORK * init_param = DEBUG_GetGameInitWork(GAMEINIT_MODE_DEBUG, 1);
			GFL_PROC_SysCallProc(
				NO_OVERLAY_ID, &GameMainProcData, init_param);
		}
		break;
	case SELECT_SOGABE:
		//そがべ
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugSogabeMainProcData, NULL);
		break;
	case SELECT_OHNO:
		//おおの
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugOhnoMainProcData, NULL);
//		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugGotoMainProcData, NULL);
	//	GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugLayoutMainProcData, NULL);
        break;
	case SELECT_TAYA:
		//たや
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugTayaMainProcData, NULL);
		break;
	case SELECT_TEST1:
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &TestProg1MainProcData, NULL);
		break;
	case SELECT_MATSUDA:
		//まつだ
//		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugMatsudaMainProcData, NULL);
//		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugMatsudaNetProcData, NULL);
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugMatsudaIrcMatchProcData, NULL);
		break;		
	case SELECT_KAGAYA:
		//かがや
		{
			GAME_INIT_WORK * init_param = DEBUG_GetGameInitWork(GAMEINIT_MODE_DEBUG, 2);
			GFL_PROC_SysCallProc(
				NO_OVERLAY_ID, &GameMainProcData, init_param);
		}
		break;
	case SELECT_ARIIZUMI:
		//ありいずみ
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugAriizumiMainProcData, NULL);
		break;
	case SELECT_DLPLAY:
		//DownloadPlay
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugDLPlayMainProcData, NULL);
		break;
	default:
		break;
	}
}


//============================================================================================
//
//
//	プロセスコントロール
//
//
//============================================================================================
static const	GFL_PROC_DATA TestMainProcData;

//------------------------------------------------------------------
/**
 * @brief		プロセス設定
 */
//------------------------------------------------------------------
void	TestModeSet(void)
{
	GFL_PROC_SysCallProc(NO_OVERLAY_ID, &TestMainProcData, NULL);
}

//------------------------------------------------------------------
/**
 * @brief		初期化
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	TESTMODE_WORK * testmode;
	HEAPID			heapID = HEAPID_TITLE;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, heapID, 0x30000 );

	testmode = GFL_PROC_AllocWork( proc, sizeof(TESTMODE_WORK), heapID );
	GFL_STD_MemClear(testmode, sizeof(TESTMODE_WORK));
	testmode->heapID = heapID;

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief		メイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	TESTMODE_WORK * testmode = mywk;
	
	switch( *seq ) {
	case 0:
		(*seq) ++;
		break;
	case 1:	//タイトル制御
		if( TitleControl(testmode) == TRUE ){
			if( testmode->select_mode == NUM_TITLESELECT_DEBUG ){
				(*seq) = 3;
			}else{
				(*seq)++;
			}
		}
		break;
	case 2:	//ロード制御
		{
			int ret = StartSelectControl( testmode );
			
			if( ret == STARTSELECT_RET_FIX ){
				switch( TestModeSelectPosGet(testmode) ) {
				case NUM_STARTSEL_CONTINUE:
				case NUM_STARTSEL_START:
					{
						GAME_INIT_WORK * init_param = DEBUG_GetGameInitWork(GAMEINIT_MODE_DEBUG, 0);
						GFL_PROC_SysCallProc(
							NO_OVERLAY_ID, &GameMainProcData, init_param);
					}
					(*seq) = 4;
					break;
				}
			}else if( ret == STARTSELECT_RET_BACK ){ //タイトルへ戻る
				testmode->seq = 2; //2D初期化から
				(*seq) = 1;
				break;
			}
		}
		break;
	case 3:	//テストモード制御
		{
			int ret = TestModeControl( testmode );
			
			if( ret == TESTMODE_RET_FIX ){
				CallSelectProc(testmode);
				(*seq) ++;
				//return GFL_PROC_RES_FINISH;
			}else if( ret == TESTMODE_RET_BACK ){	//タイトルへ戻る
				testmode->seq = 2; //2D初期化から
				(*seq) = 1;
			}
		}
		break;
	case 4:	//終了
		{
			HEAPID heapID_backup = testmode->heapID;
			u16 pos_backup = testmode->listPosition;

			GFL_STD_MemClear(testmode, sizeof(TESTMODE_WORK));

			testmode->listPosition = pos_backup;
			testmode->heapID = heapID_backup;
		}
		*seq = 0;
		break;
	}
	
	return GFL_PROC_RES_CONTINUE;
}

#if 0
static GFL_PROC_RESULT TestModeProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	TESTMODE_WORK * testmode = mywk;

	switch( *seq ) {
	case 0:
		(*seq) ++;
		break;
	case 1:
		if( TestModeControl(testmode) == TRUE ){
			CallSelectProc(testmode);
			(*seq) ++;
			//return GFL_PROC_RES_FINISH;
		}
		break;
	case 2:
		{
			HEAPID heapID_backup = testmode->heapID;
			u16 pos_backup = testmode->listPosition;

			GFL_STD_MemClear(testmode, sizeof(TESTMODE_WORK));

			testmode->listPosition = pos_backup;
			testmode->heapID = heapID_backup;
		}
		*seq = 0;
		break;
	}
	return GFL_PROC_RES_CONTINUE;
}
#endif

//------------------------------------------------------------------
/**
 * @brief		終了
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	TESTMODE_WORK * testmode = mywk;

	GFL_PROC_FreeWork(mywk);
	GFL_HEAP_DeleteHeap( HEAPID_WATANABE_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief		プロセス関数テーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA TestMainProcData = {
	TestModeProcInit,
	TestModeProcMain,
	TestModeProcEnd,
};






