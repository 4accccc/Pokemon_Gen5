//============================================================================================
/**
 * @file	samplemain.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "net/network_define.h"
#include "arc_def.h"

void	SampleBoot( HEAPID heapID );
void	SampleEnd( void );
BOOL	SampleMain( void );

#include "g3d_mapper.h"
#include "sample_net.h"

#include "sound/snd_viewer.h"
#include "sound/pm_sndsys.h"

#include "test/camera_adjust_view.h"
#include "system/fld_wipe_3dobj.h"


//------------------------------------------------------------------
/**
 * @brief   オーバーレイID宣言
 */
//------------------------------------------------------------------
FS_EXTERN_OVERLAY(fieldmap);

//============================================================================================
//
//
//		プロセスの定義
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	プロセスの初期化
 *
 * ここでヒープの生成や各種初期化処理を行う。
 * 初期段階ではmywkはNULLだが、GFL_PROC_AllocWorkを使用すると
 * 以降は確保したワークのアドレスとなる。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugWatanabeSample1ProcInit
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  // CAMADJUST用のオーバーレイロード
  GFL_OVERLAY_Load( FS_OVERLAY_ID(fieldmap) );

  
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WATANABE_DEBUG, 0x100000 );
	SampleBoot( HEAPID_WATANABE_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスのメイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugWatanabeSample1ProcMain
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	if( SampleMain() == TRUE ){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスの終了処理
 *
 * 単に終了した場合、親プロセスに処理が返る。
 * GFL_PROC_SysSetNextProcを呼んでおくと、終了後そのプロセスに
 * 処理が遷移する。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugWatanabeSample1ProcEnd
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SampleEnd();
	GFL_HEAP_DeleteHeap( HEAPID_WATANABE_DEBUG );


  // CAMADJUST用のオーバーレイアンロード
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(fieldmap) );
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugWatanabeSample1ProcData = {
	DebugWatanabeSample1ProcInit,
	DebugWatanabeSample1ProcMain,
	DebugWatanabeSample1ProcEnd,
};




//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	メイン制御
 *
 *
 *
 *
 *
 */
//============================================================================================
typedef struct _SAMPLE_SETUP SAMPLE_SETUP;

static SAMPLE_SETUP*	SetupGameSystem( HEAPID heapID );
static void						RemoveGameSystem( SAMPLE_SETUP* gs );
static void						MainGameSystem( SAMPLE_SETUP* gs );
static G3D_MAPPER*		GetG3Dmapper( SAMPLE_SETUP* gs );
static fx32*					GetWipeScale( SAMPLE_SETUP* gs );

#if 0
//アーカイブＩＮＤＥＸ
enum {
	ARCID_TEST3D = 0,
	ARCID_FLDACT,
	ARCID_FLDMAP,
	ARCID_SAMPLEMAP,
};
#endif

typedef struct _CURSOR_CONT	CURSOR_CONT;
static CURSOR_CONT*		CreateCursor( SAMPLE_SETUP*	gs, HEAPID heapID );
static void				DeleteCursor( CURSOR_CONT* cursor );
static void				MainCursor( CURSOR_CONT* cursor);
static void             FriendCursor( CURSOR_CONT* cursor );
static void				SetCursorTrans( CURSOR_CONT* cursor, const VecFx32* trans );
static void				GetCursorTrans( CURSOR_CONT* cursor, VecFx32* trans );
static u16*				GetCursorCameraAngleVPointer( CURSOR_CONT* cursor );
static u16*				GetCursorCameraAngleHPointer( CURSOR_CONT* cursor );
static fx32*			GetCursorCameraLengthPointer( CURSOR_CONT* cursor );
static u16*				GetCursorCameraFovyPointer( CURSOR_CONT* cursor );
static fx32*			GetCursorCameraFarPointer( CURSOR_CONT* cursor );

typedef struct _PC_ACTCONT PC_ACTCONT;
static PC_ACTCONT*		CreatePlayerAct( SAMPLE_SETUP* gs, HEAPID heapID );
static void				DeletePlayerAct( PC_ACTCONT* pcActCont );
static void				MainPlayerAct( PC_ACTCONT* pcActCont );
static void				MainFriendPlayerAct( PC_ACTCONT* pcActCont );
static void				SetPlayerActAnm( PC_ACTCONT* pcActCont, int anmSetID );
static void				SetPlayerActTrans( PC_ACTCONT* pcActCont, const VecFx32* trans );
static void				GetPlayerActTrans( PC_ACTCONT* pcActCont, VecFx32* trans );
static void				SetPlayerActDirection( PC_ACTCONT* pcActCont, const u16* direction );
static void				GetPlayerActDirection( PC_ACTCONT* pcActCont, u16* direction );

typedef struct _FLD_ACTCONT		FLD_ACTCONT;
static FLD_ACTCONT*		CreateFieldActSys( SAMPLE_SETUP* gs, HEAPID heapID );
static void				DeleteFieldActSys( FLD_ACTCONT* fldActCont );
static void				MainFieldActSys( FLD_ACTCONT* fldActCont );

static void _sendGamePlay( VecFx32* pVec  );

#include "samplemain.h"

static const GFL_SKB_SETUP skbData= {
	GFL_SKB_STRLEN_MAX, GFL_SKB_STRTYPE_SJIS,
	GFL_SKB_MODE_HIRAGANA, TRUE, PAD_BUTTON_START,
	GFL_DISPUT_BGID_S1, GFL_DISPUT_PALID_14, GFL_DISPUT_PALID_15,
};
static const GFL_SNDVIEWER_SETUP sndStatusData= {
	PAD_BUTTON_SELECT,
	GFL_DISPUT_BGID_S1, GFL_DISPUT_PALID_15,
	PMSND_GetNowSndHandlePointer,
	PMSND_GetBGMsoundNo,
	PMSND_GetBGMplayerNoIdx,
	PMSND_CheckOnReverb,
	GFL_SNDVIEWER_CONTROL_ENABLE | GFL_SNDVIEWER_CONTROL_EXIT,
};
static const GFL_CAMADJUST_SETUP camAdjustData= {
	PAD_BUTTON_START,
	GFL_DISPUT_BGID_S1, GFL_DISPUT_PALID_15,
};
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
typedef struct {
	VecFx32					recvWork;
	HEAPID					heapID;
	int							seq;
	int							timer;

	SAMPLE_SETUP*		gs;
	CURSOR_CONT*		cursor;
//	CURSOR_CONT*	cursorFriend;
	PC_ACTCONT*			pcActCont;
	PC_ACTCONT*			friendActCont;
	FLD_ACTCONT*		fldActCont;
	int							mapNum;

	GFL_SKB*				gflSkb;
	GFL_SNDVIEWER*	gflSndViewer;
	GFL_CAMADJUST*	gflCamAdjust;
	BOOL						subProcSw;

	void*						skbStrBuf;

}SAMPLE_WORK;

//------------------------------------------------------------------
/**
 * @brief	ローカル宣言
 */
//------------------------------------------------------------------
static BOOL GameEndCheck( int cont );

SAMPLE_WORK* sampleWork;

//------------------------------------------------------------------
/**
 * @brief	ワークの確保と破棄
 */
//------------------------------------------------------------------
void	SampleBoot( HEAPID heapID )
{
	sampleWork = GFL_HEAP_AllocClearMemory( heapID, sizeof(SAMPLE_WORK) );
	sampleWork->heapID = heapID;

	sampleWork->skbStrBuf = GFL_SKB_CreateSjisCodeBuffer( heapID );
//	GFL_UI_TP_Init( sampleWork->heapID );
}

void	SampleEnd( void )
{
//	GFL_UI_TP_Exit();
	GFL_SKB_DeleteSjisCodeBuffer( sampleWork->skbStrBuf );

	GFL_HEAP_FreeMemory( sampleWork );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
BOOL	SampleMain( void )
{
	BOOL return_flag = FALSE,bSkip = FALSE;
	int i;

	sampleWork->timer++;
//	GFL_UI_TP_Main();

	switch( sampleWork->seq ){

	case 0:
		//基本システムセットアップ
		sampleWork->gs = SetupGameSystem( sampleWork->heapID );
		sampleWork->mapNum = 0;
		sampleWork->seq++;

		//PMSND_PlayBGM(SEQ_BICYCLE);
    break;

	case 1:
		//セットアップ
		ResistData3Dmapper( GetG3Dmapper(sampleWork->gs), 
							&resistMapTbl[sampleWork->mapNum].mapperData );

		sampleWork->cursor = CreateCursor( sampleWork->gs, sampleWork->heapID );
		sampleWork->fldActCont = CreateFieldActSys( sampleWork->gs, sampleWork->heapID );
		{
			VecFx32 pos;
			u16		dir;

			pos = resistMapTbl[sampleWork->mapNum].startPos;
			dir = 0;
				
			sampleWork->pcActCont = CreatePlayerAct( sampleWork->gs, sampleWork->heapID );
			SetPlayerActTrans( sampleWork->pcActCont, &pos );
			SetPlayerActDirection( sampleWork->pcActCont, &dir );
		}
		sampleWork->subProcSw = FALSE;
		sampleWork->seq++;
		break;

	case 2:
		if( sampleWork->subProcSw == TRUE ){
			if(sampleWork->gflSkb != NULL ){
				if( GFL_SKB_Main( sampleWork->gflSkb ) == FALSE ){	
					OS_Printf( sampleWork->skbStrBuf );
					OS_Printf("\n");
					GFL_SKB_Delete(	sampleWork->gflSkb );
					sampleWork->subProcSw = FALSE;
					sampleWork->gflSkb = NULL;
				} 
			} else if(sampleWork->gflSndViewer != NULL ){
				if( GFL_SNDVIEWER_Main( sampleWork->gflSndViewer ) == FALSE ){	
					GFL_SNDVIEWER_Delete( sampleWork->gflSndViewer );
					sampleWork->subProcSw = FALSE;
					sampleWork->gflSndViewer = NULL;
				}
			} else if(sampleWork->gflCamAdjust != NULL ){
				if( GFL_CAMADJUST_Main( sampleWork->gflCamAdjust ) == FALSE ){	
					GFL_CAMADJUST_Delete( sampleWork->gflCamAdjust );
					sampleWork->subProcSw = FALSE;
					sampleWork->gflCamAdjust = NULL;
				}
			} else {
				sampleWork->subProcSw = FALSE;
			}
		} else {
			if( GameEndCheck( GFL_UI_KEY_GetCont() ) == TRUE ){
				sampleWork->seq = 4;
				break;
			}
			if( (GFL_UI_KEY_GetCont()&(PAD_BUTTON_SELECT | PAD_BUTTON_START))
					== (PAD_BUTTON_SELECT | PAD_BUTTON_START) ){
				sampleWork->mapNum--;
				if( sampleWork->mapNum < 0 ){
					sampleWork->mapNum = NELEMS(resistMapTbl)-1;
				}
				sampleWork->seq = 3;
				break;
			}
			if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_SELECT ){
				sampleWork->gflSndViewer = GFL_SNDVIEWER_Create(&sndStatusData, sampleWork->heapID);
				sampleWork->subProcSw = TRUE;
				break;
			}
			if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_START ){
				//sampleWork->gflSkb = GFL_SKB_Create
				//						( sampleWork->skbStrBuf, &skbData, sampleWork->heapID );
				sampleWork->gflCamAdjust = GFL_CAMADJUST_Create(&camAdjustData, sampleWork->heapID);

        GFL_CAMADJUST_SetCameraParam(	sampleWork->gflCamAdjust,
																			GetCursorCameraAngleVPointer(sampleWork->cursor),
																			GetCursorCameraAngleHPointer(sampleWork->cursor),
																			GetCursorCameraLengthPointer(sampleWork->cursor),
																			GetCursorCameraFovyPointer(sampleWork->cursor),
																			GetCursorCameraFarPointer(sampleWork->cursor));
        GFL_CAMADJUST_SetWipeParam(	sampleWork->gflCamAdjust, GetWipeScale(sampleWork->gs) );
				sampleWork->subProcSw = TRUE;
				break;
			}
		}

		MainPlayerAct( sampleWork->pcActCont );
		MainFieldActSys( sampleWork->fldActCont );
		{
			VecFx32 pos;
			u16		dir;

			GetPlayerActTrans( sampleWork->pcActCont, &pos );

			SetCursorTrans( sampleWork->cursor, &pos );
			SetPos3Dmapper( GetG3Dmapper(sampleWork->gs), &pos );
		}
		MainCursor( sampleWork->cursor );

		MainGameSystem( sampleWork->gs );
		break;

	case 3:
        ReleaseData3Dmapper( GetG3Dmapper(sampleWork->gs) );

		DeleteFieldActSys( sampleWork->fldActCont );
		DeletePlayerAct( sampleWork->pcActCont );
		DeleteCursor( sampleWork->cursor );
        sampleWork->seq = 1;
		break;

	case 4:
        ReleaseData3Dmapper( GetG3Dmapper(sampleWork->gs) );

		DeleteFieldActSys( sampleWork->fldActCont );
		DeletePlayerAct( sampleWork->pcActCont );
		DeleteCursor( sampleWork->cursor );

		RemoveGameSystem( sampleWork->gs );
		return_flag = TRUE;
        break;
	}
	return return_flag;
}

//------------------------------------------------------------------
/**
 * @brief	終了チェック
 */
//------------------------------------------------------------------
static BOOL GameEndCheck( int cont )
{
	int resetCont = PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_SELECT;

	if( (cont & resetCont ) == resetCont ){
		return TRUE;
	} else {
		return FALSE;
	}
}





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	セットアップ
 *
 *
 *
 *
 *
 */
//============================================================================================
//#define BACKGROUND_COL	(GX_RGB(23,29,31))	//背景色
//#define FOG_COL			(GX_RGB(31,31,31))	//フォグ色
#define BACKGROUND_COL		(GX_RGB(30,31,31))	//背景色
#define BACKGROUND_S_COL	(GX_RGB(0,0,0))		//背景色
#define FOG_COL			(GX_RGB(31,31,31))		//フォグ色
#define DTCM_SIZE		(0x1000)				//DTCMエリアのサイズ

//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
struct _SAMPLE_SETUP {
	GFL_G3D_UTIL*			g3Dutil;		//g3Dutil Lib ハンドル
	u16								g3DutilUnitIdx;	//g3Dutil Unitインデックス
	GFL_G3D_SCENE*		g3Dscene;		//g3Dscene Lib ハンドル
	GFL_G3D_CAMERA*		g3Dcamera;		//g3Dcamera Lib ハンドル
	GFL_G3D_LIGHTSET*	g3Dlightset;	//g3Dlight Lib ハンドル
	GFL_BBDACT_SYS*		bbdActSys;		//ビルボードアクトシステム設定ハンドル

	GFL_TCB*					g2dVintr;		//2Dシステム用vIntrTaskハンドル
	GFL_TCB*					g3dVintr;		//3Dシステム用vIntrTaskハンドル

	G3D_MAPPER*				g3Dmapper;		//マップ表示コントローラ

	GFL_BMPWIN*				bmpwin;			//bitmapWin個別ハンドル

	FLD_WIPEOBJ*			fldWipeObj;
	fx32							wipeScale;

	HEAPID						heapID;
};

//------------------------------------------------------------------
/**
 * @brief	ディスプレイ環境データ
 */
//------------------------------------------------------------------
///ＶＲＡＭバンク設定構造体
static const GFL_DISP_VRAM dispVram = {
	GX_VRAM_BG_16_F,				//メイン2DエンジンのBGに割り当て 
	GX_VRAM_BGEXTPLTT_NONE,			//メイン2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_SUB_BG_32_H,			//サブ2DエンジンのBGに割り当て
	GX_VRAM_SUB_BGEXTPLTT_NONE,		//サブ2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_OBJ_64_E,				//メイン2DエンジンのOBJに割り当て
	GX_VRAM_OBJEXTPLTT_NONE,		//メイン2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_SUB_OBJ_16_I,			//サブ2DエンジンのOBJに割り当て
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	//サブ2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_TEX_012_ABC,			//テクスチャイメージスロットに割り当て
	GX_VRAM_TEXPLTT_0_G,			//テクスチャパレットスロットに割り当て
	GX_OBJVRAMMODE_CHAR_1D_64K,		// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};

//------------------------------------------------------------------
/**
 * @brief	アーカイブテーブル
 */
//------------------------------------------------------------------
#include "test_graphic/fld_act.naix"

#if 0
static	const	char	*GraphicFileTable[]={
	"test_graphic/test3d.narc",
	"test_graphic/fld_act.narc",
	"test_graphic/fld_map.narc",
	"test_graphic/sample_map.narc",
};
#endif

//------------------------------------------------------------------
/**
 * @brief	３Ｄグラフィック環境データ
 */
//------------------------------------------------------------------
//カメラ初期設定データ
static const VecFx32 cameraTarget	= { 0, 0, 0 };
static const VecFx32 cameraPos	= { 0, (FX32_ONE * 64), (FX32_ONE * 128) };

//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light0Tbl[] = {
#if 1
	{ 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(16,16,16) } },
	{ 1, {{  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(0,0,0) } },
	{ 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(0,0,0) } },
	{ 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
#else
	{ 0, {{ 0, -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(0,0,31) } },
	{ 1, {{ 0, -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(0,0,0) } },
	{ 2, {{ 0, -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(0,0,0) } },
	{ 3, {{ 0, -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
#endif
};
static const GFL_G3D_LIGHTSET_SETUP light0Setup = { light0Tbl, NELEMS(light0Tbl) };
//------------------------------------------------------------------
/**
 * @brief	３Ｄリソースデータ
 */
//------------------------------------------------------------------
#define G3D_FRM_PRI			(1)			//３Ｄ面の描画プライオリティー
#define G3D_UTIL_RESCOUNT	(512)		//g3Dutilで使用するリソースの最大設定可能数
#define G3D_UTIL_OBJCOUNT	(128)		//g3Dutilで使用するオブジェクトの最大設定可能数
#define G3D_SCENE_OBJCOUNT	(256)		//g3Dsceneで使用するsceneObjの最大設定可能数
#define G3D_OBJWORK_SZ		(64)		//g3Dsceneで使用するsceneObjのワークサイズ
#define G3D_ACC_COUNT		(32)		//g3Dsceneで使用するsceneObjAccesaryの最大設定可能数
#define G3D_BBDACT_RESMAX	(64)		//billboardActで使用するリソースの最大設定可能数
#define G3D_BBDACT_ACTMAX	(256)		//billboardActで使用するオブジェクトの最大設定可能数

static const GXRgb edgeColorTable[8] = {
	GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),
	GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),
};
//------------------------------------------------------------------
/**
 * @brief	ローカル宣言
 */
//------------------------------------------------------------------
//ＢＧ設定関数
static void	bg_init( SAMPLE_SETUP* gs );
static void	bg_exit( SAMPLE_SETUP* gs );
//２Ｄ関数
static void	g2d_load( SAMPLE_SETUP* gs );
static void g2d_draw( SAMPLE_SETUP* gs );
static void	g2d_unload( SAMPLE_SETUP* gs );
static void	g2d_vblank( GFL_TCB* tcb, void* work );
//３Ｄ関数
static void g3d_load( SAMPLE_SETUP* gs );
static void g3d_control( SAMPLE_SETUP* gs );
static void g3d_draw( SAMPLE_SETUP* gs );
static void	g3d_unload( SAMPLE_SETUP* gs );
static void	g3d_vblank( GFL_TCB* tcb, void* work );

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
static SAMPLE_SETUP*	SetupGameSystem( HEAPID heapID )
{
	SAMPLE_SETUP*	gs = GFL_HEAP_AllocClearMemory( heapID, sizeof(SAMPLE_SETUP) );
	gs->heapID = heapID;

	//乱数初期化
	GFL_STD_MtRandInit(0);

	//VRAMクリア
	GFL_DISP_ClearVRAM( GX_VRAM_D );
	//VRAM設定
	GFL_DISP_SetBank( &dispVram );

	//BG初期化
	bg_init( gs );

	g2d_load( gs );	//２Ｄデータのロード
	g3d_load( gs );	//３Ｄデータのロード

	return gs;
}

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
static void	RemoveGameSystem( SAMPLE_SETUP* gs )
{
	g3d_unload( gs );	//３Ｄデータ破棄
	g2d_unload( gs );	//２Ｄデータ破棄

	bg_exit( gs );
//	GFL_ARC_Exit();

	GFL_HEAP_FreeMemory( gs );
}

//------------------------------------------------------------------
/**
 * @brief	システムメイン関数
 */
//------------------------------------------------------------------
static void	MainGameSystem( SAMPLE_SETUP* gs )
{
//	GFL_UI_TP_Main();

	g3d_control( gs );
	g3d_draw( gs );
	g2d_draw( gs );
}

//------------------------------------------------------------------
/**
 * @brief		ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void G3DsysSetup( void );
static const GFL_BG_SYS_HEADER bgsysHeader = {
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};	

static const GFL_BG_BGCNT_HEADER textBGcont = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x3800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};
#define TEXTBG_FRAME	(GFL_BG_FRAME1_S)
#define TEXTBG_PAL		(15)

static void	bg_init( SAMPLE_SETUP* gs )
{
	//ＢＧシステム起動
	GFL_BG_Init( gs->heapID );

	//背景色パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( gs->heapID, 16*2 );
		plt[0] = BACKGROUND_COL;
		GFL_BG_LoadPalette( GFL_BG_FRAME0_M, plt, 16*2, 0 );	//メイン画面の背景色転送
		plt[0] = BACKGROUND_S_COL;
		GFL_BG_LoadPalette( GFL_BG_FRAME0_S, plt, 16*2, 0 );	//サブ画面の背景色転送
		GFL_HEAP_FreeMemory( plt );
	}
	//ＢＧモード設定
	GFL_BG_SetBGMode( &bgsysHeader );

	//ＢＧコントロール設定
	GFL_BG_SetBGControl( TEXTBG_FRAME, &textBGcont, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( TEXTBG_FRAME, 0 );
	GFL_BG_SetVisible( TEXTBG_FRAME, VISIBLE_ON );

	//G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 31, 8 );
	//２Ｄシステム起動
	GFL_BMPWIN_Init( gs->heapID );

	//３Ｄシステム起動
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX384K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, gs->heapID, G3DsysSetup );
	GFL_BG_SetBGControl3D( G3D_FRM_PRI );

	//ディスプレイ面の選択
	//GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_DISP_SetDispOn();
}

static void	bg_exit( SAMPLE_SETUP* gs )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );

	GFL_G3D_Exit();
	GFL_BMPWIN_Exit();

	GFL_BG_FreeBGControl( TEXTBG_FRAME );
	GFL_BG_Exit();
}

// ３Ｄセットアップコールバック
static void G3DsysSetup( void )
{
	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	G3X_SetShading( GX_SHADING_TOON );
	G3X_AntiAlias( TRUE );
	G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
	G3X_AlphaBlend( TRUE );
#if 1
	// フォグセットアップ
    {
        u32     fog_table[8];
        int     i;

        //G3X_SetFog(TRUE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x1000, 0x01000 );
        G3X_SetFog(TRUE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x0400, 0x00e00 );

        G3X_SetFogColor(FOG_COL, 0);

        for ( i=0; i<8; i++ ){
            fog_table[i] = (u32)(((i*16)<<0) | ((i*16+4)<<8) | ((i*16+8)<<16) | ((i*16+12)<<24));
        }
        G3X_SetFogTable(&fog_table[0]);
	}

#else
	G3X_SetFog( FALSE, 0, 0, 0 );
#endif
	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog

	G3X_SetEdgeColorTable( edgeColorTable ); 
	//G3X_EdgeMarking( TRUE );
	G3X_EdgeMarking( FALSE );

	// ビューポートの設定
	G3_ViewPort(0, 0, 255, 191);
	GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_W );
}
//------------------------------------------------------------------
/**
 * @brief		２Ｄデータコントロール
 */
//------------------------------------------------------------------
#define FONT_BCOL (0)
#define FONT_FCOL (0x7fff)

#define BCOL_P (1)
#define FCOL_P (2)

#define BMPWIN_POS_PX (1)
#define BMPWIN_POS_PY (20)
#define BMPWIN_POS_SX (30)
#define BMPWIN_POS_SY (3)

static void	g2d_load( SAMPLE_SETUP* gs )
{
	//透明キャラ領域確保
	GFL_BG_AllocCharacterArea( TEXTBG_FRAME, 1, GFL_BG_CHRAREA_GET_F );
	//パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( gs->heapID, 0x20 );

		plt[BCOL_P] = FONT_BCOL;
		plt[FCOL_P] = FONT_FCOL;
		GFL_BG_LoadPalette( TEXTBG_FRAME, plt, 0x20, 0x20*TEXTBG_PAL );
		GFL_HEAP_FreeMemory( plt );
	}
	//文字表示ビットマップの作成
	gs->bmpwin = GFL_BMPWIN_Create( TEXTBG_FRAME,
									BMPWIN_POS_PX, BMPWIN_POS_PY,
									BMPWIN_POS_SX, BMPWIN_POS_SY,
									TEXTBG_PAL, GFL_BG_CHRAREA_GET_F );
	//ビットマップキャラクターをアップデート
	GFL_BG_ClearScreen( TEXTBG_FRAME );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gs->bmpwin ), BCOL_P );
	GFL_BMPWIN_TransVramCharacter( gs->bmpwin );
	GFL_BMPWIN_MakeScreen( gs->bmpwin );
	GFL_BG_LoadScreenReq( TEXTBG_FRAME );

	gs->g2dVintr = GFUser_VIntr_CreateTCB( g2d_vblank, (void*)gs, 0 );
}

static void g2d_draw( SAMPLE_SETUP* gs )
{
//	GFL_BMPWIN_MakeScreen( gs->bmpwin );
//	GFL_BG_LoadScreenReq( TEXTBG_FRAME );
}

static void	g2d_unload( SAMPLE_SETUP* gs )
{
	GFL_TCB_DeleteTask( gs->g2dVintr );

	GFL_BMPWIN_Delete( gs->bmpwin );
}

static void	g2d_vblank( GFL_TCB* tcb, void* work )
{
	SAMPLE_SETUP* gs =  (SAMPLE_SETUP*)work;
	GFL_CLACT_SYS_VBlankFunc();

}

//------------------------------------------------------------------
/**
 * @brief		３Ｄデータロード
 */
static void	g3d_trans_BBD( GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz );
//------------------------------------------------------------------
//作成
static void g3d_load( SAMPLE_SETUP* gs )
{
	gs->fldWipeObj = FLD_WIPEOBJ_Create(gs->heapID);

	//配置物設定

	//g3Dutilを使用し配列管理をする
	gs->g3Dutil = GFL_G3D_UTIL_Create( G3D_UTIL_RESCOUNT, G3D_UTIL_OBJCOUNT, gs->heapID );
	//g3Dsceneを使用し管理をする
	gs->g3Dscene = GFL_G3D_SCENE_Create( gs->g3Dutil, 
						G3D_SCENE_OBJCOUNT, G3D_OBJWORK_SZ, G3D_ACC_COUNT, TRUE, gs->heapID );

	gs->g3Dmapper = Create3Dmapper( gs->heapID );
	gs->bbdActSys = GFL_BBDACT_CreateSys
					( G3D_BBDACT_RESMAX, G3D_BBDACT_ACTMAX, g3d_trans_BBD, gs->heapID );
	{
		VecFx32 bbdScale = { FX32_ONE*2, FX32_ONE*2, FX32_ONE*2 };
		GFL_BBD_SetScale( GFL_BBDACT_GetBBDSystem(gs->bbdActSys), &bbdScale );
	}

	//カメラ作成
	gs->g3Dcamera = GFL_G3D_CAMERA_CreateDefault( &cameraPos, &cameraTarget, gs->heapID );
	{
		//fx32 far = 1500 << FX32_SHIFT;
		//fx32 far = 4096 << FX32_SHIFT;
		fx32 far = 1024 << FX32_SHIFT;

		GFL_G3D_CAMERA_SetFar( gs->g3Dcamera, &far );
	}
	//ライト作成
	gs->g3Dlightset = GFL_G3D_LIGHT_Create( &light0Setup, gs->heapID );

	//カメラライト0反映
	GFL_G3D_CAMERA_Switching( gs->g3Dcamera );
	GFL_G3D_LIGHT_Switching( gs->g3Dlightset );

	gs->g3dVintr = GFUser_VIntr_CreateTCB( g3d_vblank, (void*)gs, 0 );
}
	
//動作
static void g3d_control( SAMPLE_SETUP* gs )
{
	GFL_G3D_SCENE_Main( gs->g3Dscene ); 
	Main3Dmapper( gs->g3Dmapper );
	GFL_BBDACT_Main( gs->bbdActSys );
}

//描画
static void g3d_draw( SAMPLE_SETUP* gs )
{
	GFL_G3D_CAMERA_Switching( gs->g3Dcamera );
	GFL_G3D_LIGHT_Switching( gs->g3Dlightset );
	Draw3Dmapper( gs->g3Dmapper, gs->g3Dcamera );
	GFL_BBDACT_Draw( gs->bbdActSys, gs->g3Dcamera, gs->g3Dlightset );
	//FLD_WIPEOBJ_Main(gs->fldWipeObj, FX32_ONE*4);
	FLD_WIPEOBJ_Main(gs->fldWipeObj, gs->wipeScale);
	GFL_G3D_SCENE_Draw( gs->g3Dscene );  
}

//破棄
static void g3d_unload( SAMPLE_SETUP* gs )
{
	GFL_TCB_DeleteTask( gs->g3dVintr );

	GFL_G3D_LIGHT_Delete( gs->g3Dlightset );
	GFL_G3D_CAMERA_Delete( gs->g3Dcamera );

	GFL_BBDACT_DeleteSys( gs->bbdActSys );
	Delete3Dmapper( gs->g3Dmapper );

	GFL_G3D_SCENE_Delete( gs->g3Dscene );  
	GFL_G3D_UTIL_Delete( gs->g3Dutil );

	FLD_WIPEOBJ_Delete(gs->fldWipeObj);
}
	
static void	g3d_vblank( GFL_TCB* tcb, void* work )
{
	SAMPLE_SETUP* gs =  (SAMPLE_SETUP*)work;
}

//BBD用VRAM転送関数
static void	g3d_trans_BBD( GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz )
{
	NNS_GFD_DST_TYPE transType;

	if( type == GFL_BBDACT_TRANSTYPE_DATA ){
		transType = NNS_GFD_DST_3D_TEX_VRAM;
	} else {
		transType = NNS_GFD_DST_3D_TEX_PLTT;
	}
	NNS_GfdRegisterNewVramTransferTask( transType, dst, (void*)src, siz );
}

//------------------------------------------------------------------
/**
 * @brief	システム取得
 */
//------------------------------------------------------------------
static GFL_G3D_CAMERA* GetG3Dcamera( SAMPLE_SETUP* gs )
{
	return gs->g3Dcamera;
}

static G3D_MAPPER* GetG3Dmapper( SAMPLE_SETUP* gs )
{
	return gs->g3Dmapper;
}

static GFL_BBDACT_SYS* GetBbdActSys( SAMPLE_SETUP* gs )
{
	return gs->bbdActSys;
}

static fx32* GetWipeScale( SAMPLE_SETUP* gs )
{
	return &gs->wipeScale;
}
	

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	コントローラ
 *
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	移動方向の地形に沿ったベクトル取得
 */
//------------------------------------------------------------------
#define WALK_LIMIT_HEIGHT ( 16 * FX32_ONE )

static void GetGroundMoveVec
		( const VecFx16* vecN, const VecFx32* pos, const VecFx32* vecMove, VecFx32* result )
{
	VecFx32	vecN32, posNext;
	fx32	by, valD;

	VEC_Add( pos, vecMove, &posNext );

	VEC_Set( &vecN32, vecN->x, vecN->y, vecN->z );
	valD = -( FX_Mul(vecN32.x,pos->x) + FX_Mul(vecN32.y,pos->y) + FX_Mul(vecN32.z,pos->z) ); 
	by = -( FX_Mul( vecN32.x, posNext.x ) + FX_Mul( vecN32.z, posNext.z ) + valD );
	posNext.y = FX_Div( by, vecN32.y );

	VEC_Subtract( &posNext, pos, result );
	VEC_Normalize( result, result );
}

static BOOL CalcSetGroundMove( G3D_MAPPER* g3Dmapper, G3D_MAPPER_INFODATA* gridInfoData, 
								VecFx32* pos, VecFx32* vecMove, fx32 speed )
{
	G3D_MAPPER_GRIDINFO gridInfo;
	VecFx32	posNext, vecGround;
	fx32	height = 0;
	BOOL	initSw = FALSE;

	//VEC_Normalize( &vecMove, &vecMove );
	//
	if( (gridInfoData->vecN.x|gridInfoData->vecN.y|gridInfoData->vecN.z) == 0 ){
		//vecN = {0,0,0}の場合は初期状態
		VecFx16	vecNinit = { 0, FX16_ONE, 0 };

		GetGroundMoveVec( &vecNinit, pos, vecMove, &vecGround );
		initSw = TRUE;
	} else {
		GetGroundMoveVec( &gridInfoData->vecN, pos, vecMove, &vecGround );
	}
	VEC_MultAdd( speed, &vecGround, pos, &posNext );
	if( posNext.y < 0 ){
		posNext.y = 0;	//ベースライン
	}
	if( Check3DmapperOutRange( g3Dmapper, &posNext ) == TRUE ){
		return FALSE;
	}

	//プレーヤー用動作。この位置中心に高さデータが存在するため、すべて取得して設定
	if( Get3DmapperGridInfo( g3Dmapper, &posNext, &gridInfo ) == FALSE ){
		return FALSE;
	}

	if( gridInfo.count ){
		int		i = 0, p;
		fx32	h_tmp, diff1, diff2;

		height = gridInfo.gridData[0].height;
		p = 0;
		i++;
		for( ; i<gridInfo.count; i++ ){
			h_tmp = gridInfo.gridData[i].height;

			diff1 = height - pos->y;
			diff2 = h_tmp - pos->y;

			if( FX_Mul( diff2, diff2 ) < FX_Mul( diff1, diff1 ) ){
				height = h_tmp;
				p = i;
			}
		}
#if 0
		if( initSw == FALSE ){
			//移動制限テスト
			if(FX_Mul((height-pos->y),(height-pos->y))
					>=FX_Mul(WALK_LIMIT_HEIGHT,WALK_LIMIT_HEIGHT)){
				return FALSE;
			}
		} 
#endif
		*gridInfoData = gridInfo.gridData[p];	//グリッドデータ更新
		VEC_Set( pos, posNext.x, gridInfoData->height, posNext.z );		//位置情報更新
	}
	return TRUE;
}
	
//============================================================================================
/**
 * @brief	カーソル
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
struct _CURSOR_CONT {
	HEAPID					heapID;
	SAMPLE_SETUP*		gs;
	u16							unitIdx;
	u16							resIdx;
	u16							objIdx;

	u16							cursorIdx;
	VecFx32					trans;
	fx32						length;
	u16							angleV;
	u16							angleH;
	u16							fovy;
	fx32						far;

	G3D_MAPPER_INFODATA gridInfoData;
};

//------------------------------------------------------------------
/**
 * @brief	セットアップ
 */
//------------------------------------------------------------------
#include "test_graphic/test3d.naix"

enum {
	G3DRES_CURSOR = 0,
};
//３Ｄグラフィックリソーステーブル
static const GFL_G3D_UTIL_RES cursor_g3Dutil_resTbl[] = {
	{ ARCID_TEST3D, NARC_haruka_test_wall_nsbmd, GFL_G3D_UTIL_RESARC },
};

enum {
	G3DOBJ_CURSOR = 0,
};
//３Ｄオブジェクト定義テーブル
static const GFL_G3D_UTIL_OBJ cursor_g3Dutil_objTbl[] = {
	{ G3DRES_CURSOR, 0, G3DRES_CURSOR, NULL, 0 },
};

//---------------------
//g3Dscene 初期設定テーブルデータ
static const GFL_G3D_UTIL_SETUP cursor_g3Dsetup = {
	cursor_g3Dutil_resTbl, NELEMS(cursor_g3Dutil_resTbl),
	cursor_g3Dutil_objTbl, NELEMS(cursor_g3Dutil_objTbl),
};

static const GFL_G3D_SCENEOBJ_DATA cursorObject[] = {
	{	G3DOBJ_CURSOR, 0, 1, 16, TRUE, TRUE,
		{	{ 0, 0, 0 },
			//{ FX32_ONE/8, FX32_ONE/8, FX32_ONE/8 },
			{ FX32_ONE/64, FX32_ONE/64, FX32_ONE/64 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};

#define	CAMERA_LENGTH	(16*FX32_ONE)
#define	CAMERA_TARGET_HEIGHT	(4 * FX32_ONE)
#define PITCH_LIMIT (0x200)

static void CalcCameraPos
	(const fx32 length, const u16 angleV, const u16 angleH, const VecFx32* target, VecFx32* dst);
//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
static CURSOR_CONT*	CreateCursor( SAMPLE_SETUP*	gs, HEAPID heapID )
{
	CURSOR_CONT* cursor = GFL_HEAP_AllocClearMemory( heapID, sizeof(CURSOR_CONT) );

	cursor->heapID = heapID;
	cursor->gs = gs;

	VEC_Set( &cursor->trans, 0, 0, 0 );
	cursor->length = CAMERA_LENGTH;
	cursor->angleV = 0;
	cursor->angleH = 0;
	InitGet3DmapperGridInfoData( &cursor->gridInfoData );

	//３Ｄデータセットアップ
	cursor->unitIdx = GFL_G3D_SCENE_AddG3DutilUnit( cursor->gs->g3Dscene, &cursor_g3Dsetup );
	cursor->resIdx = GFL_G3D_SCENE_GetG3DutilUnitResIdx( cursor->gs->g3Dscene, cursor->unitIdx );
	cursor->objIdx = GFL_G3D_SCENE_GetG3DutilUnitObjIdx( cursor->gs->g3Dscene, cursor->unitIdx );
	cursor->cursorIdx = GFL_G3D_SCENEOBJ_Add(	cursor->gs->g3Dscene, 
												cursorObject, NELEMS(cursorObject),
												cursor->objIdx );
	
	GFL_G3D_SCENEOBJ_SetPos(	GFL_G3D_SCENEOBJ_Get(cursor->gs->g3Dscene, cursor->cursorIdx),
								&cursor->trans );
	return cursor;
}

//------------------------------------------------------------------
/**
 * @brief	終了
 */
//------------------------------------------------------------------
static void	DeleteCursor( CURSOR_CONT* cursor )
{
	GFL_G3D_SCENEOBJ_Remove( cursor->gs->g3Dscene, cursor->cursorIdx, NELEMS(cursorObject) ); 
	GFL_G3D_SCENE_DelG3DutilUnit( cursor->gs->g3Dscene, cursor->unitIdx );

	GFL_HEAP_FreeMemory( cursor );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
static void	MainCursor( CURSOR_CONT* cursor )
{
	VecFx32	pos, target;

	target = cursor->trans;
	target.y += (CAMERA_TARGET_HEIGHT);

	CalcCameraPos(cursor->length, cursor->angleV, cursor->angleH, &target, &pos);

	//GFL_G3D_SCENEOBJ_SetPos(	GFL_G3D_SCENEOBJ_Get(cursor->gs->g3Dscene, cursor->cursorIdx), 
	//							&cursor->trans );
	GFL_G3D_CAMERA_SetTarget( cursor->gs->g3Dcamera, &target );
	GFL_G3D_CAMERA_SetPos( cursor->gs->g3Dcamera, &pos );
}



static void	FriendCursor( CURSOR_CONT* cursor )
{
    GFL_STD_MemCopy((const void*)&sampleWork->recvWork ,&cursor->trans, sizeof(VecFx32));
	GFL_G3D_SCENEOBJ_SetPos(	GFL_G3D_SCENEOBJ_Get(cursor->gs->g3Dscene, cursor->cursorIdx), 
								&cursor->trans );
}

//-----------------------------------------------------------------------------------------
//　距離とアングルによるカメラ位置計算
//-----------------------------------------------------------------------------------------
static void CalcCameraPos
	(const fx32 length, const u16 angleV, const u16 angleH, const VecFx32* target, VecFx32* dst)
{
	VecFx32 vecCamera;
	fx16 sinYaw = FX_SinIdx(angleV);
	fx16 cosYaw = FX_CosIdx(angleV);
	fx16 sinPitch = FX_SinIdx(angleH);
	fx16 cosPitch = FX_CosIdx(angleH);

	if(cosPitch < 0){ cosPitch = -cosPitch; }	// 裏周りしないようにマイナス値はプラス値に補正
	if(cosPitch < PITCH_LIMIT){ cosPitch = PITCH_LIMIT; }	// 0値近辺は不正表示になるため補正

	// カメラの座標計算
	VEC_Set( &vecCamera, sinYaw * cosPitch, sinPitch * FX16_ONE, cosYaw * cosPitch);
	VEC_Normalize(&vecCamera, &vecCamera);
	VEC_MultAdd( length, &vecCamera, target, dst );
}

//------------------------------------------------------------------
/**
 * @brief	
 */
//------------------------------------------------------------------
static void	SetCursorTrans( CURSOR_CONT* cursor, const VecFx32* trans )
{
	cursor->trans = *trans;
}
static void	GetCursorTrans( CURSOR_CONT* cursor, VecFx32* trans )
{
	*trans = cursor->trans;
}

static u16* GetCursorCameraAngleVPointer( CURSOR_CONT* cursor )
{
	return &cursor->angleV;
}

static u16* GetCursorCameraAngleHPointer( CURSOR_CONT* cursor )
{
	return &cursor->angleH;
}

static fx32* GetCursorCameraLengthPointer( CURSOR_CONT* cursor )
{
	return &cursor->length;
}

static u16* GetCursorCameraFovyPointer( CURSOR_CONT* cursor )
{
	return &cursor->fovy;
}

static fx32* GetCursorCameraFarPointer( CURSOR_CONT* cursor )
{
	return &cursor->far;
}

//============================================================================================
/**
 * @brief	プレーヤーアクター
 */
//============================================================================================
#define MV_SPEED		(2*FX32_ONE)

struct _PC_ACTCONT {
	HEAPID					heapID;
	SAMPLE_SETUP*			gs;
	u16						cameraRotate;
	GFL_BBDACT_RESUNIT_ID	bbdActResUnitID;
	u16						bbdActResCount;
	GFL_BBDACT_ACTUNIT_ID	bbdActActUnitID;
	u16						bbdActActCount;
	u16						direction;
	VecFx32					trans;
	int						anmSetID;
	BOOL					anmSetReq;			

	G3D_MAPPER_INFODATA		gridInfoData;
};

enum {
	ANMTYPE_STOP = 0,
	ANMTYPE_WALK,
	ANMTYPE_RUN,
	ANMTYPE_JUMP,
};

static const GFL_BBDACT_RESDATA playerBBDactResTable[] = {
	{ ARCID_FLDACT, NARC_fld_act_tex32x32_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x1024, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
//	{ ARCID_FLDACT, NARC_fld_act_hero_nsbtx,
//		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x1024, 32, 32, GFL_BBDACT_RESTYPE_TRANSSRC },
};

static const GFL_BBDACT_ANM PCstopLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopRAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopDAnm[] = {
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCwalkLAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkRAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkUAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 20, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkDAnm[] = {
	{ 22, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 23, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCrunLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 13, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCjumpLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};

enum {
	PCACTSTOP_UP = 0,
	PCACTSTOP_DOWN,
	PCACTSTOP_LEFT,
	PCACTSTOP_RIGHT,

	PCACTWALK_UP,
	PCACTWALK_DOWN,
	PCACTWALK_LEFT,
	PCACTWALK_RIGHT,

	PCACTRUN_UP,
	PCACTRUN_DOWN,
	PCACTRUN_LEFT,
	PCACTRUN_RIGHT,

	PCACTJUMP_UP,
	PCACTJUMP_DOWN,
	PCACTJUMP_LEFT,
	PCACTJUMP_RIGHT,
};

static const GFL_BBDACT_ANM* playerBBDactAnmTable[] = { 
	PCstopUAnm, PCstopDAnm, PCstopLAnm, PCstopRAnm,
	PCwalkUAnm, PCwalkDAnm, PCwalkLAnm, PCwalkRAnm,
	PCrunUAnm, PCrunDAnm, PCrunLAnm, PCrunRAnm,
	PCjumpUAnm, PCjumpDAnm, PCjumpLAnm, PCjumpRAnm,
};

static const int playerBBDanmOffsTblMine[] = { 
	PCACTSTOP_LEFT - PCACTSTOP_UP,
	PCACTSTOP_DOWN - PCACTSTOP_UP,
	PCACTSTOP_RIGHT - PCACTSTOP_UP,
	PCACTSTOP_UP - PCACTSTOP_UP,
};

static u16 getCameraRotate( GFL_G3D_CAMERA* g3Dcamera )
{
	VecFx32 vec, camPos, target;
	
	GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
	GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

	VEC_Subtract( &target, &camPos, &vec );
	return FX_Atan2Idx( -vec.z, vec.x ) - 0x4000;
}

static int getPlayerBBDanm( int anmSetID, u16 dir, const int* anmOffsTable )
{
	int	anmBase, datOffs;

	if( (dir > 0x2000)&&(dir < 0x6000)){
		datOffs = 0;
	} else if( (dir >= 0x6000)&&(dir <= 0xa000)){
		datOffs = 1;
	} else if( (dir > 0xa000)&&(dir < 0xe000)){
		datOffs = 2;
	} else {
		datOffs = 3;
	}
	switch( anmSetID ){
	default:
	case ANMTYPE_STOP:
		anmBase = PCACTSTOP_UP;
		break;
	case ANMTYPE_WALK:
		anmBase = PCACTWALK_UP;
		break;
	case ANMTYPE_RUN:
		anmBase = PCACTRUN_UP;
		break;
	case ANMTYPE_JUMP:
		anmBase = PCACTJUMP_UP;
		break;
	}
	return anmBase + anmOffsTable[ datOffs ];
}

static void playerBBDactFunc( GFL_BBDACT_SYS* bbdActSys, int actIdx, void* work )
{
	GFL_BBD_SYS*	bbdSys = GFL_BBDACT_GetBBDSystem( bbdActSys );
	PC_ACTCONT*	pcActCont = (PC_ACTCONT*)work;
	VecFx32	trans = pcActCont->trans;
	u16		anmID;
	u16		dir;

	dir = pcActCont->direction - getCameraRotate( GetG3Dcamera(pcActCont->gs) );
	anmID = getPlayerBBDanm( pcActCont->anmSetID, dir, playerBBDanmOffsTblMine );

	//カメラ補正(アニメ向きの変更をするのに参照)
	GFL_BBDACT_SetAnimeIdxContinue( GetBbdActSys( pcActCont->gs ), actIdx, anmID );
	//位置補正
	trans.x = pcActCont->trans.x;
	//trans.y = pcActCont->trans.y + FX32_ONE*7;	//補正
	trans.y = pcActCont->trans.y + FX32_ONE*16;	//補正
	trans.z = pcActCont->trans.z;
	GFL_BBD_SetObjectTrans( bbdSys, actIdx, &trans );
}

static PC_ACTCONT*	CreatePlayerAct( SAMPLE_SETUP*	gs, HEAPID heapID )
{
	PC_ACTCONT*	pcActCont = GFL_HEAP_AllocClearMemory( heapID, sizeof(PC_ACTCONT) );
	GFL_BBDACT_SYS* bbdActSys = GetBbdActSys( gs );
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	int		i, objIdx;
	VecFx32	trans;
	u8		alpha;
	BOOL	drawEnable;
	u16		setActNum;

	pcActCont->gs = gs;
	SetPlayerActAnm( pcActCont, ANMTYPE_STOP );
	InitGet3DmapperGridInfoData( &pcActCont->gridInfoData );
	
	//リソースセットアップ
	pcActCont->bbdActResUnitID = GFL_BBDACT_AddResourceUnit( bbdActSys, playerBBDactResTable,
														NELEMS(playerBBDactResTable) );
	actData.resID = 0;
	actData.sizX = FX16_ONE*8-1;
	actData.sizY = FX16_ONE*8-1;
	
	actData.trans.x = 0;
	actData.trans.y = 0;
	actData.trans.z = 0;

	actData.alpha = 31;
	actData.drawEnable = TRUE;
	actData.lightMask = GFL_BBD_LIGHTMASK_01;
	actData.func = playerBBDactFunc;
	actData.work = pcActCont;

	pcActCont->bbdActActUnitID = GFL_BBDACT_AddAct
									( bbdActSys, pcActCont->bbdActResUnitID, &actData, 1 );
	//GFL_BBDACT_BindActTexRes
	//		( bbdActSys, pcActCont->bbdActActUnitID, pcActCont->bbdActResUnitID+1 );
	GFL_BBDACT_BindActTexResLoad
		( bbdActSys, pcActCont->bbdActActUnitID, ARCID_FLDACT, NARC_fld_act_hero_nsbtx );

	GFL_BBDACT_SetAnimeTable( bbdActSys, pcActCont->bbdActActUnitID, 
								playerBBDactAnmTable, NELEMS(playerBBDactAnmTable) );
	GFL_BBDACT_SetAnimeIdxOn( bbdActSys, pcActCont->bbdActActUnitID, 0 );
	return pcActCont;
}

static void	DeletePlayerAct( PC_ACTCONT* pcActCont )
{
	GFL_BBDACT_SYS* bbdActSys = GetBbdActSys( pcActCont->gs );

	GFL_BBDACT_RemoveAct( bbdActSys, pcActCont->bbdActActUnitID, 1 );
	GFL_BBDACT_RemoveResourceUnit
				( bbdActSys, pcActCont->bbdActResUnitID, NELEMS(playerBBDactResTable) );

	GFL_HEAP_FreeMemory( pcActCont );
}

static void	MainPlayerAct( PC_ACTCONT* pcActCont )
{
	VecFx32	vecMove = { 0, 0, 0 };
	int		key;
	u16		dir;
	BOOL	mvFlag = FALSE;

	key = GFL_UI_KEY_GetCont();

	dir = getCameraRotate( GetG3Dcamera(pcActCont->gs) );

	if( key & PAD_KEY_UP ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0x8000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0x8000) );
		pcActCont->direction = dir;
	}
	if( key & PAD_KEY_DOWN ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0x0000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0x0000) );
		pcActCont->direction = dir + 0x8000;
	}
	if( key & PAD_KEY_LEFT ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0xc000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0xc000) );
		pcActCont->direction = dir + 0x4000;
	}
	if( key & PAD_KEY_RIGHT ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(dir + 0x4000) );
		vecMove.z = FX_CosIdx( (u16)(dir + 0x4000) );
		pcActCont->direction = dir + 0xc000;
	}
	CalcSetGroundMove( GetG3Dmapper(pcActCont->gs), &pcActCont->gridInfoData, 
								&pcActCont->trans, &vecMove, MV_SPEED );
    
	if( mvFlag == TRUE ){
		SetPlayerActAnm( pcActCont, ANMTYPE_WALK );
	} else {
		SetPlayerActAnm( pcActCont, ANMTYPE_STOP );
	}
}

static void	MainFriendPlayerAct( PC_ACTCONT* pcActCont )
{
    GFL_STD_MemCopy((const void*)&sampleWork->recvWork ,&pcActCont->trans, sizeof(VecFx32));
}

static void	SetPlayerActAnm( PC_ACTCONT* pcActCont, int anmSetID )
{
	int		anmID;
	u16		dir = pcActCont->direction - getCameraRotate( GetG3Dcamera(pcActCont->gs) );

	if( pcActCont->anmSetID != anmSetID ){
		pcActCont->anmSetID = anmSetID;

		anmID = getPlayerBBDanm( pcActCont->anmSetID, dir, playerBBDanmOffsTblMine );
		GFL_BBDACT_SetAnimeIdx( GetBbdActSys(pcActCont->gs), pcActCont->bbdActActUnitID, anmID );
	}
}

static void	SetPlayerActTrans( PC_ACTCONT* pcActCont, const VecFx32* trans )
{
	VEC_Set( &pcActCont->trans, trans->x, trans->y, trans->z );
}

static void	GetPlayerActTrans( PC_ACTCONT* pcActCont, VecFx32* trans )
{
	VEC_Set( trans, pcActCont->trans.x, pcActCont->trans.y, pcActCont->trans.z );
}

static void	SetPlayerActDirection( PC_ACTCONT* pcActCont, const u16* direction )
{
	pcActCont->direction  = *direction;
}

static void	GetPlayerActDirection( PC_ACTCONT* pcActCont, u16* direction )
{
	*direction = pcActCont->direction;
}

//============================================================================================
/**
 * @brief	フィールドアクター
 */
//============================================================================================
//------------------------------------------------------------------
/**
 *
 * @brief	型宣言
 *
 */
//------------------------------------------------------------------
#define FLD_BBDACT_ACTMAX	(253)
#define WORK_SIZ			(8)

typedef struct {
	//FLD_ACTCONT*			fldActCont;
	G3D_MAPPER_INFODATA		gridInfoData;
	u16						work[WORK_SIZ];
}FLD_ACTWORK;

struct _FLD_ACTCONT {
	HEAPID					heapID;
	SAMPLE_SETUP*			gs;
	u16						cameraRotate;
	GFL_BBDACT_RESUNIT_ID	bbdActResUnitID;
	u16						bbdActResCount;
	GFL_BBDACT_ACTUNIT_ID	bbdActActUnitID;
	u16						bbdActActCount;
	FLD_ACTWORK				actWork[FLD_BBDACT_ACTMAX];
};

static void	initActWork( FLD_ACTCONT* fldActCont, FLD_ACTWORK* actWork );
static void	calcCameraRotate( FLD_ACTCONT* fldActCont );
//------------------------------------------------------------------
/**
 *
 * @brief	セットアップデータ
 *
 */
//------------------------------------------------------------------
static const GFL_BBDACT_RESDATA testResTable[] = {
	{ ARCID_FLDACT, NARC_fld_act_hero_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_achamo_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_artist_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_badman_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_beachgirl_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_idol_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_lady_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_oldman1_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_policeman_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_rivel_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_waiter_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
};

 

static const GFL_BBDACT_ANM walkLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM walkRAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM walkUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM walkDAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM testAnm1[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 4, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 5, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 6, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM testAnm2[] = {
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 16 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 24 },
	{ 4, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 32 },
	{ 5, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 48 },
	{ 6, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 7, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 8, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 9, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 10, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ GFL_BBDACT_ANMCOM_LOOP, 0, 0, GFL_BBDACT_ANMLOOPPARAM( 4, 5 ) },
	{ 11, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 64 },
	{ GFL_BBDACT_ANMCOM_CHG, 0, 0, 0 },
};
static const GFL_BBDACT_ANM testAnm3[] = {
	{ 0, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 32, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 64, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 1 },
};

enum {
	ACTWALK_UP = 0,
	ACTWALK_DOWN,
	ACTWALK_LEFT,
	ACTWALK_RIGHT,
};

static const GFL_BBDACT_ANM* testAnmTable[] = { walkUAnm, walkDAnm, walkLAnm, walkRAnm };

static void testFunc( GFL_BBDACT_SYS* bbdActSys, int actIdx, void* work )
{
	GFL_BBD_SYS*	bbdSys = GFL_BBDACT_GetBBDSystem( bbdActSys );
	FLD_ACTCONT*	fldActCont = (FLD_ACTCONT*)work;
	FLD_ACTWORK*	actWork = &fldActCont->actWork[actIdx];

	if( actWork->work[0] == 0 ){
		u32	timer = GFUser_GetPublicRand(8);

		actWork->work[0] = (timer+2) * 30;
		actWork->work[1] = GFUser_GetPublicRand( 0x10000 );
	} else {
		actWork->work[0]--;
	}
	{
		VecFx32 nowTrans, nextTrans, vecMove, vecGround, rotVec;
		u16	 theta = actWork->work[1];
		fx32 speed = FX32_ONE;
		BOOL mvf;
		{
			u16	 dir = actWork->work[1] - fldActCont->cameraRotate;
			if( (dir > 0x2000)&&(dir < 0x6000)){
				//OS_Printf("右　rotate = %x\n", dir );
				GFL_BBDACT_SetAnimeIdxContinue( bbdActSys, actIdx, ACTWALK_RIGHT );
			} else if( (dir >= 0x6000)&&(dir <= 0xa000)){
				//OS_Printf("上　rotate = %x\n", dir );
				GFL_BBDACT_SetAnimeIdxContinue( bbdActSys, actIdx, ACTWALK_UP );
			} else if( (dir > 0xa000)&&(dir < 0xe000)){
				//OS_Printf("左　rotate = %x\n", dir );
				GFL_BBDACT_SetAnimeIdxContinue( bbdActSys, actIdx, ACTWALK_LEFT );
			} else {
				//OS_Printf("下　rotate = %x\n", dir );
				GFL_BBDACT_SetAnimeIdxContinue( bbdActSys, actIdx, ACTWALK_DOWN );
			}
		}
		GFL_BBD_GetObjectTrans( bbdSys, actIdx, &nowTrans );

		vecMove.x = FX_SinIdx( theta );
		vecMove.y = 0;
		vecMove.z = FX_CosIdx( theta );

		mvf = CalcSetGroundMove( GetG3Dmapper( fldActCont->gs ), &actWork->gridInfoData, 
									&nowTrans, &vecMove, FX32_ONE );
		if( mvf == TRUE ){
			VecFx32 setTrans;

			//VEC_Set( &setTrans, nowTrans.x, nowTrans.y + FX32_ONE*7, nowTrans.z );
			VEC_Set( &setTrans, nowTrans.x, nowTrans.y + FX32_ONE*16, nowTrans.z );
			GFL_BBD_SetObjectTrans( bbdSys, actIdx, &setTrans );
		} else {
			actWork->work[0] = 0;
		}
	}
}

#define TEST_NPC_SETNUM	(250)
static void testSetUp( FLD_ACTCONT* fldActCont )
{
	GFL_BBDACT_SYS* bbdActSys = GetBbdActSys( fldActCont->gs );	
	GFL_BBDACT_ACTDATA* actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	int		i, objIdx;
	VecFx32	trans;
	u8		alpha;
	BOOL	drawEnable;
	u16		setActNum;

	//リソースセットアップ
	fldActCont->bbdActResCount = NELEMS(testResTable);
	fldActCont->bbdActResUnitID = GFL_BBDACT_AddResourceUnit(	bbdActSys, 
																testResTable, 
																fldActCont->bbdActResCount );
	fldActCont->bbdActActCount = FLD_BBDACT_ACTMAX;

	//ＮＰＣアクターセットアップ
	{
		u16	setActNum = TEST_NPC_SETNUM;
		GFL_BBDACT_ACTDATA* actData = GFL_HEAP_AllocClearMemory( fldActCont->heapID,
													setActNum*sizeof(GFL_BBDACT_ACTDATA) );
		fx32 mapSizex, mapSizez;

		Get3DmapperSize( GetG3Dmapper( fldActCont->gs ), &mapSizex, &mapSizez );

		for( i=0; i<setActNum; i++ ){
			actData[i].resID = GFUser_GetPublicRand( 10 )+1;
			actData[i].sizX = FX16_ONE*8-1;
			actData[i].sizY = FX16_ONE*8-1;
	
			actData[i].trans.x = (GFUser_GetPublicRand(mapSizex) );
			actData[i].trans.y = 0;
			actData[i].trans.z = (GFUser_GetPublicRand(mapSizez) );
			
			actData[i].alpha = 31;
			actData[i].drawEnable = TRUE;
			actData[i].lightMask = GFL_BBD_LIGHTMASK_01;
			actData[i].func = testFunc;
			actData[i].work = fldActCont;
		}
		fldActCont->bbdActActUnitID = GFL_BBDACT_AddAct( bbdActSys, fldActCont->bbdActResUnitID,
														actData, setActNum );
		for( i=0; i<setActNum; i++ ){
			GFL_BBDACT_SetAnimeTable( bbdActSys, fldActCont->bbdActActUnitID+i, 
										testAnmTable, NELEMS(testAnmTable) );
			GFL_BBDACT_SetAnimeIdxOn( bbdActSys, fldActCont->bbdActActUnitID+i, 0 );
		}
		GFL_HEAP_FreeMemory( actData );
	}
}

static void testRelease( FLD_ACTCONT* fldActCont )
{
	GFL_BBDACT_SYS* bbdActSys = GetBbdActSys( fldActCont->gs );	
	u16	setActNum = FLD_BBDACT_ACTMAX;

	GFL_BBDACT_RemoveAct( bbdActSys, fldActCont->bbdActActUnitID, TEST_NPC_SETNUM );
		
	GFL_BBDACT_RemoveResourceUnit(	bbdActSys, 
									fldActCont->bbdActResUnitID, fldActCont->bbdActResCount );
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクトシステム作成
 */
//------------------------------------------------------------------
static FLD_ACTCONT*	CreateFieldActSys( SAMPLE_SETUP* gs, HEAPID heapID )
{
	FLD_ACTCONT* fldActCont = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLD_ACTCONT) );
	int	i;

	fldActCont->heapID = heapID;
	fldActCont->gs = gs;

	for( i=0; i<FLD_BBDACT_ACTMAX; i++ ){ initActWork( fldActCont, &fldActCont->actWork[i] ); }

	testSetUp( fldActCont );	//テスト

	return fldActCont;
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクトシステム破棄
 */
//------------------------------------------------------------------
static void	DeleteFieldActSys( FLD_ACTCONT* fldActCont )
{
	testRelease( fldActCont );	//テスト
	GFL_HEAP_FreeMemory( fldActCont ); 
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクト動作関数
 */
//------------------------------------------------------------------
static void	MainFieldActSys( FLD_ACTCONT* fldActCont )
{
	//カメラ回転算出(ビルボードそのものには関係ない。アニメ向きの変更をするのに参照)
	VecFx32 vec, camPos, target;
	GFL_G3D_CAMERA* g3Dcamera = GetG3Dcamera( fldActCont->gs );

	GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
	GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

	VEC_Subtract( &target, &camPos, &vec );
	
	fldActCont->cameraRotate = FX_Atan2Idx( -vec.z, vec.x ) - 0x4000;
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクトワーク初期化
 */
//------------------------------------------------------------------
static void	initActWork( FLD_ACTCONT* fldActCont, FLD_ACTWORK* actWork )
{
	int i;

	//actWork->fldActCont = fldActCont;
	InitGet3DmapperGridInfoData( &actWork->gridInfoData );

	for( i=0; i<WORK_SIZ; i++ ){
		actWork->work[i] = 0;
	}
}


