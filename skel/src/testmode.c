//===================================================================
/**
 * @file	testmode.c
 */
//===================================================================
#include "gflib.h"
#include "main.h"
#include "textprint.h"

#include "testmode.h"

void	TestModeSet(void);


//============================================================================================
//
//
//	メインコントロール
//
//
//============================================================================================
#define G3DUTIL_USE
typedef struct {
	int						seq;
	u16						listPosition;
	void*					chrbuf;
	GFL_BMP_DATA*			bmpHeader;
	GFL_TEXT_PRINTPARAM*	textParam;

	GFL_BMPWIN*				bmpwin[32];
#ifdef G3DUTIL_USE
	GFL_G3D_UTIL_SCENE*		g3Dscene;
#else
	GFL_G3D_RES*			g3Dres[4];
	GFL_G3D_RND*			g3Drnd[2];
	GFL_G3D_ANM*			g3Danm[2];
	GFL_G3D_OBJ*			g3Dobj[2];
#endif
	GFL_G3D_OBJSTATUS		status[2];

	u16						work[16];
}TESTMODE_WORK;

typedef struct {
	const char*	msg;
	u8 posx;
	u8 posy;
	u8 sizx;
	u8 sizy;
}TESTMODE_PRINTLIST;

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

#include "sample_graphic/titledemo.naix"
#include "testmode.dat"



//ＢＧ設定関数
static void	bg_init( void );
static void	bg_exit( void );
//ビットマップ設定関数
static void msg_bmpwin_make(TESTMODE_WORK * testmode, u8 bmpwinNum, const char* msg, u8 px, u8 py, u8 sx, u8 sy );
static void msg_bmpwin_trush( TESTMODE_WORK * testmode, u8 bmpwinNum );
static void msg_bmpwin_palset( TESTMODE_WORK * testmode, u8 bmpwinNum, u8 pal );
//２ＤＢＧ作成関数
static void	g2d_load( TESTMODE_WORK * testmode );
static void	g2d_unload( TESTMODE_WORK * testmode );
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
static BOOL	TestModeControl( TESTMODE_WORK * testmode )
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
		g2d_load(testmode);	//２Ｄデータ作成
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
			GFL_BMPWIN_MakeScrn( testmode->bmpwin[NUM_SELECT1+i] );
		}
		//ＢＧスクリーン転送リクエスト発行
		GFL_BG_LoadScreenReq( TEXT_FRM );
		testmode->seq++;

		g3d_control_effect(testmode);
		g3d_draw(testmode);		//３Ｄデータ描画
		break;

	case 3:
		//キー判定
		if( GFL_UI_KeyGetTrg() == PAD_BUTTON_A ) {
			if(		( testmode->listPosition == 0)		//わたなべのとき遷移
				||	( testmode->listPosition == 1)		//たまだのとき遷移
				||	( testmode->listPosition == 2)		//そがべのとき遷移
				||	( testmode->listPosition == 3)		//おおののとき遷移
			  )
			{
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
		g3d_control_effect(testmode);
		g3d_draw(testmode);		//３Ｄデータ描画
		break;

	case 4:
		//終了
		g3d_unload(testmode);	//３Ｄデータ破棄
		g2d_unload(testmode);	//２Ｄデータ破棄
		bg_exit();
		return_flag = TRUE;
		break;
	}
	return return_flag;
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
	//GFL_G3D_UtilsysInit( G3D_UTIL_RESSIZ, G3D_UTIL_OBJSIZ, G3D_UTIL_ANMSIZ, heapID );  
}

static void	bg_exit( void )
{
	//GFL_G3D_UtilsysExit();  
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
static void msg_bmpwin_make(TESTMODE_WORK * testmode, u8 bmpwinNum, const char* msg, u8 px, u8 py, u8 sx, u8 sy )
{
	//ビットマップ作成
	testmode->bmpwin[bmpwinNum] = GFL_BMPWIN_Create( TEXT_FRM, px, py, sx, sy, 0, 
														GFL_BG_CHRAREA_GET_B );

	//テキストをビットマップに表示
	testmode->textParam->bmp = GFL_BMPWIN_GetBmp( testmode->bmpwin[ bmpwinNum ] );
	GFL_TEXT_PrintSjisCode( msg, testmode->textParam );

	//ビットマップキャラクターをアップデート
	GFL_BMPWIN_UploadChar( testmode->bmpwin[bmpwinNum] );
	//ビットマップスクリーン作成
	GFL_BMPWIN_MakeScrn( testmode->bmpwin[bmpwinNum] );
}
	
static void msg_bmpwin_trush( TESTMODE_WORK * testmode, u8 bmpwinNum )
{
	GFL_BMPWIN_Delete( testmode->bmpwin[bmpwinNum] );
}
	
static void msg_bmpwin_palset( TESTMODE_WORK * testmode, u8 bmpwinNum, u8 pal )
{
	GFL_BMPWIN_SetPal( testmode->bmpwin[bmpwinNum], pal );
}
	
//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
static void	g2d_load( TESTMODE_WORK * testmode )
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
	GFL_BG_LoadScreenReq( TEXT_FRM );
}

static void	g2d_unload( TESTMODE_WORK * testmode )
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
	GFL_TEXT_sysExit();
}

//------------------------------------------------------------------
/**
 * @brief		３Ｄデータ
 */
//------------------------------------------------------------------
static const char g3DarcPath[] = {"src/sample_graphic/titledemo.narc"};

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
static const GFL_G3D_UTIL_SCENE_RES g3Dscene_resTbl[] = {
	{ (u32)g3DarcPath, NARC_titledemo_title_air_nsbmd, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_titledemo_title_air_nsbta, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_titledemo_title_iar_nsbmd, GFL_G3D_UTIL_RESPATH },
	{ (u32)g3DarcPath, NARC_titledemo_title_iar_nsbta, GFL_G3D_UTIL_RESPATH },
};

static const GFL_G3D_UTIL_SCENE_ANM g3Dscene_anm1Tbl[] = {
	{ G3DRES_AIR_BTA, 0 },
};

static const GFL_G3D_UTIL_SCENE_ANM g3Dscene_anm2Tbl[] = {
	{ G3DRES_IAR_BTA, 0 },
};

static const GFL_G3D_UTIL_SCENE_OBJ g3Dscene_objTbl[] = {
	{ G3DRES_AIR_BMD, 0, G3DRES_AIR_BMD, g3Dscene_anm1Tbl, NELEMS(g3Dscene_anm1Tbl) },
	{ G3DRES_IAR_BMD, 0, G3DRES_IAR_BMD, g3Dscene_anm2Tbl, NELEMS(g3Dscene_anm2Tbl) },
};

static const GFL_G3D_UTIL_SCENE_SETUP g3Dscene_setup = {
	g3Dscene_resTbl, NELEMS(g3Dscene_resTbl),
	g3Dscene_objTbl, NELEMS(g3Dscene_objTbl),
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
	u16 heapID = GFL_HEAPID_APP;

	testmode->g3Dscene = GFL_G3D_UtilsysCreate( &g3Dscene_setup, heapID );
#else
	//		リソースセットアップ
	testmode->g3Dres[ G3DRES_AIR_BMD ] = GFL_G3D_ResCreatePath
										( g3DarcPath, NARC_titledemo_title_air_nsbmd );
	testmode->g3Dres[ G3DRES_AIR_BTA ] = GFL_G3D_ResCreatePath
										( g3DarcPath, NARC_titledemo_title_air_nsbta );
	testmode->g3Dres[ G3DRES_IAR_BMD ] = GFL_G3D_ResCreatePath
										( g3DarcPath, NARC_titledemo_title_iar_nsbmd );
	testmode->g3Dres[ G3DRES_IAR_BTA ] = GFL_G3D_ResCreatePath
										( g3DarcPath, NARC_titledemo_title_iar_nsbta );
	
	//		リソース転送
	GFL_G3D_VramLoadTex( testmode->g3Dres[ G3DRES_AIR_BMD ] );
	GFL_G3D_VramLoadTex( testmode->g3Dres[ G3DRES_IAR_BMD ] );
	//		レンダー作成
	testmode->g3Drnd[ G3D_AIR ] = GFL_G3D_RndCreate(	testmode->g3Dres[ G3DRES_AIR_BMD ], 0, 
														testmode->g3Dres[ G3DRES_AIR_BMD ] );
	testmode->g3Drnd[ G3D_IAR ] = GFL_G3D_RndCreate(	testmode->g3Dres[ G3DRES_IAR_BMD ], 0, 
														testmode->g3Dres[ G3DRES_IAR_BMD] );
	//		アニメ作成
	testmode->g3Danm[ G3D_AIR ] = GFL_G3D_AnmCreate(	testmode->g3Drnd[ G3D_AIR ], 
														testmode->g3Dres[ G3DRES_AIR_BTA ], 0 );
	testmode->g3Danm[ G3D_IAR ] = GFL_G3D_AnmCreate(	testmode->g3Drnd[ G3DIAR ], 
														testmode->g3Dres[ G3DRES_IAR_BTA ], 0 );
	//		オブジェクト作成
	testmode->g3Dobj[ G3D_AIR ] = GFL_G3D_ObjCreate(	testmode->g3Drnd[ G3D_AIR ], 
														&testmode->g3Danm[ G3D_AIR ], 1 );
	testmode->g3Dobj[ G3D_IAR ] = GFL_G3D_ObjCreate(	testmode->g3Drnd[ G3D_IAR ], 
														&testmode->g3Danm[ G3D_IAR ], 1 );
#endif
	//描画ステータスワーク設定
	testmode->status[ G3D_AIR ] = status0;
	testmode->status[ G3D_IAR ] = status1;

	//カメラセット
	GFL_G3D_sysProjectionSet(	GFL_G3D_PRJPERS, 
								FX_SinIdx( cameraPerspway ), FX_CosIdx( cameraPerspway ), 
								cameraAspect, 0, cameraNear, cameraFar, 0 );
	GFL_G3D_sysLookAtSet( (VecFx32*)&cameraPos, (VecFx32*)&cameraUp, (VecFx32*)&cameraTarget );

	testmode->work[0] = 0;
}
	
static void g3d_draw( TESTMODE_WORK * testmode )
{
	GFL_G3D_OBJ* g3Dobj[2];
#ifdef G3DUTIL_USE
	g3Dobj[ G3D_AIR ] = GFL_G3D_UtilsysObjHandleGet( testmode->g3Dscene, G3D_AIR  );
	g3Dobj[ G3D_IAR ] = GFL_G3D_UtilsysObjHandleGet( testmode->g3Dscene, G3D_IAR  );
#else
	g3Dobj[ G3D_AIR ] = testmode->g3Dobj[ G3D_AIR ];
	g3Dobj[ G3D_IAR ] = testmode->g3Dobj[ G3D_IAR ];
#endif
	GFL_G3D_DrawStart();
	GFL_G3D_DrawLookAt();
	{
		GFL_G3D_ObjDraw( g3Dobj[ G3D_AIR ], &testmode->status[ G3D_AIR ] );
		GFL_G3D_ObjDraw( g3Dobj[ G3D_IAR ], &testmode->status[ G3D_IAR ] );
	}
	GFL_G3D_DrawEnd();

	GFL_G3D_ObjContAnmFrameAutoLoop( g3Dobj[ G3D_AIR ], 0, FX32_ONE ); 
	GFL_G3D_ObjContAnmFrameAutoLoop( g3Dobj[ G3D_IAR ], 0, FX32_ONE ); 
}
	
static void g3d_unload( TESTMODE_WORK * testmode )
{
#ifdef G3DUTIL_USE
	GFL_G3D_UtilsysDelete( testmode->g3Dscene );
#else
	GFL_G3D_ObjDelete( testmode->g3Dobj[ G3D_IAR ] );
	GFL_G3D_ObjDelete( testmode->g3Dobj[ G3D_AIR ] );

	GFL_G3D_AnmDelete( testmode->g3Danm[ G3D_IAR ] );
	GFL_G3D_AnmDelete( testmode->g3Danm[ G3D_AIR ] );

	GFL_G3D_RndDelete( testmode->g3Drnd[ G3D_IAR ] );
	GFL_G3D_RndDelete( testmode->g3Drnd[ G3D_AIR ] );

	GFL_G3D_VramUnloadTex( testmode->g3Dres[ G3DRES_IAR_BMD ] );
	GFL_G3D_VramUnloadTex( testmode->g3Dres[ G3DRES_AIR_BMD ] );

	GFL_G3D_ResDelete( testmode->g3Dres[ G3DRES_IAR_BTA ] );
	GFL_G3D_ResDelete( testmode->g3Dres[ G3DRES_IAR_BMD ] );
	GFL_G3D_ResDelete( testmode->g3Dres[ G3DRES_AIR_BTA ] );
	GFL_G3D_ResDelete( testmode->g3Dres[ G3DRES_AIR_BMD ] );
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
	testmode = GFL_PROC_AllocWork( proc, sizeof(TESTMODE_WORK), GFL_HEAPID_APP );
	GFL_STD_MemClear(testmode, sizeof(TESTMODE_WORK));
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
	if( TestModeControl(testmode) == TRUE ){
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief		終了
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	TESTMODE_WORK * testmode = mywk;
	switch( TestModeSelectPosGet(testmode) ) {
	case 0:
		//わたなべ
		GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &DebugWatanabeMainProcData, NULL);
		break;
	case 1:
		//たまだ
		GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &DebugTamadaMainProcData, NULL);
		break;
	case 2:
		//そがべ
		GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &DebugSogabeMainProcData, NULL);
		break;
	case 3:
		//おおの
		GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &DebugOhnoMainProcData, NULL);
		break;
	default:
		//たや
		//なかむら
		//たかはし
		break;
	}
	GFL_PROC_FreeWork(mywk);

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






