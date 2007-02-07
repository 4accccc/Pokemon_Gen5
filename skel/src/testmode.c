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

	u16						g3DresTblIdx;
	u16						g3DobjTblIdx;
	u16						g3DanmTblIdx;

	u16						work[16];
}TESTMODE_WORK;

typedef struct {
	const char*	msg;
	u8 posx;
	u8 posy;
	u8 sizx;
	u8 sizy;
}TESTMODE_PRINTLIST;

#include "testmode.dat"

TESTMODE_WORK* testmode;

void	TestModeInit(void);
void	TestModeMain(void);

static BOOL	TestModeControl( void );
static const GFL_PROC_DATA TestMainProcData;
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
	GX_SetBankForTex(GX_VRAM_TEX_01_AB);
	GX_SetBankForBG(GX_VRAM_BG_64_E);
	GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0_G); 

	//ＢＧモード設定
	GFL_BG_InitBG( &bgsysHeader );

	//ＢＧコントロール設定
	GFL_BG_BGControlSet( TEXT_FRM, &bgCont3, GFL_BG_MODE_TEXT );
	GFL_BG_PrioritySet( TEXT_FRM, TEXT_FRM_PRI );
	GFL_BG_VisibleSet( TEXT_FRM, VISIBLE_ON );

	//ビットマップウインドウシステムの起動
	GFL_BMPWIN_sysInit( heapID );

	//３Ｄシステム起動
	GFL_G3D_sysInit( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, heapID, NULL );
	GFL_BG_BGControlSet3D( G3D_FRM_PRI );
	GFL_G3D_UtilsysInit( G3D_UTIL_RESSIZ, G3D_UTIL_OBJSIZ, G3D_UTIL_ANMSIZ, heapID );  
}

static void	bg_exit( void )
{
	GFL_G3D_UtilsysExit();  
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
	testmode->bmpwin[bmpwinNum] = GFL_BMPWIN_Create( TEXT_FRM, px, py, sx, sy, 0, 
														GFL_BG_CHRAREA_GET_B );

	testmode->textParam->bmp = GFL_BMPWIN_GetBmp( testmode->bmpwin[ bmpwinNum ] );
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
enum {
	NUM_TITLE = 0,
	NUM_URL,
	NUM_SELECT1,
	NUM_SELECT2,
	NUM_SELECT3,
	NUM_SELECT4,
	NUM_SELECT5,
};
//------------------------------------------------------------------
static void	g2d_load( void )
{
	u16 heapID = GFL_HEAPID_APP;

	//フォント読み込み
	GFL_TEXT_sysInit( font_path );
	//パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocMemoryLowClear( heapID, 16*2 );
		plt[0] = G2D_BACKGROUND_COL;
		plt[1] = G2D_FONT_COL;
		GFL_BG_PaletteSet( TEXT_FRM, plt, 16*2, 0 );
		plt[1] = G2D_FONTSELECT_COL;
		GFL_BG_PaletteSet( TEXT_FRM, plt, 16*2, 16*2 );

		GFL_HEAP_FreeMemory( plt );
	}
	//文字表示パラメータワーク作成
	{
		GFL_TEXT_PRINTPARAM* param = GFL_HEAP_AllocMemoryLowClear
										( heapID,sizeof(GFL_TEXT_PRINTPARAM));
		*param = default_param;
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
//作成
static void g3d_load( void )
{
	GFL_G3D_UtilAllLoad( g3DresouceTable, NELEMS(g3DresouceTable), &testmode->g3DresTblIdx,
						 g3DobjectTable, NELEMS(g3DobjectTable), &testmode->g3DobjTblIdx,
						 g3DanimetionTable, NELEMS(g3DanimetionTable), &testmode->g3DanmTblIdx );

	//カメラセット
	GFL_G3D_sysProjectionSet(	GFL_G3D_PRJPERS, 
								FX_SinIdx( cameraPerspway ), FX_CosIdx( cameraPerspway ), 
								cameraAspect, 0, cameraNear, cameraFar, 0 );
	GFL_G3D_sysLookAtSet( (VecFx32*)&cameraPos, (VecFx32*)&cameraUp, (VecFx32*)&cameraTarget );

	testmode->work[0] = 0;
}
	
//描画
static void g3d_draw( void )
{
	GFL_G3D_UtilDraw();
}
	
//破棄
static void g3d_unload( void )
{
	GFL_G3D_UtilAllUnload(	NELEMS(g3DresouceTable), &testmode->g3DresTblIdx,
							NELEMS(g3DobjectTable), &testmode->g3DobjTblIdx,
							NELEMS(g3DanimetionTable), &testmode->g3DanmTblIdx );
}
	
//------------------------------------------------------------------
/**
 * @brief	プリント実験
 */
enum {
	MSG_WHITE = 0,
	MSG_RED,
};
static void g3d_control_effect( void );
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

		g3d_control_effect();
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
		g3d_control_effect();
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
static void g3d_control_effect( void )
{
	MtxFx33 rotate;
	VecFx32 rotate_tmp = { 0, 0, 0 };
	GFL_G3D_OBJ* g3Dobj;
	GFL_G3D_ANM* g3Danm;

	//回転計算
	{
		g3Dobj = GFL_G3D_UtilObjGet( testmode->g3DobjTblIdx + G3OBJ_AIR );
		g3Danm = GFL_G3D_UtilAnmGet( testmode->g3DanmTblIdx + G3ANM_AIR );

		rotate_tmp.y = g3DanmRotateSpeed * testmode->work[0];	//Ｙ軸回転
		GFL_G3D_UtilObjDrawRotateCalcYX( &rotate_tmp, &rotate );
		GFL_G3D_ObjContSetRotate( g3Dobj, &rotate );
		//アニメーションコントロール
		GFL_G3D_ObjContAnmFrameAutoLoop( g3Danm, g3DanmFrameSpeed );
	}
	{
		g3Dobj = GFL_G3D_UtilObjGet( testmode->g3DobjTblIdx + G3OBJ_IAR );
		g3Danm = GFL_G3D_UtilAnmGet( testmode->g3DanmTblIdx + G3ANM_IAR );

		rotate_tmp.y = -g3DanmRotateSpeed * testmode->work[0];	//Ｙ軸回転
		GFL_G3D_UtilObjDrawRotateCalcYX( &rotate_tmp, &rotate );
		GFL_G3D_ObjContSetRotate( g3Dobj, &rotate );
		//アニメーションコントロール
		GFL_G3D_ObjContAnmFrameAutoLoop( g3Danm, g3DanmFrameSpeed );
	}
	testmode->work[0]++;
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




