//============================================================================================
/**
 * @file	field_main.c
 * @brief	
 * @author	
 * @date	
 *
 * 元ファイル tetsu/sample/samplemain.c
 */
//============================================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "net/network_define.h"
#include "textprint.h"
#include "arc_def.h"

#include "field_g3d_mapper.h"
#include "field_net.h"

#include "field_main.h"
#include "field_common.h"
#include "field_actor.h"
#include "field_player.h"
#include "field_camera.h"

#include "field_debug.h"

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

static FIELD_SETUP*	SetupGameSystem( HEAPID heapID );
static void				RemoveGameSystem( FIELD_SETUP* gs );
static void				MainGameSystem( FIELD_SETUP* gs );

static void _sendGamePlay( VecFx32* pVec  );



typedef enum {
	GAMEMODE_NORMAL = 0,
	GAMEMODE_FINISH,
}GAMEMODE;

//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
struct _FIELD_MAIN_WORK
{
    VecFx32         recvWork;
	HEAPID			heapID;
	GAMEMODE		gamemode;
	int				seq;
	int				timer;
	
	const DEPEND_FUNCTIONS * ftbl;
	FIELD_SETUP*	gs;
	FIELD_CAMERA*	camera_control;
//	CURSOR_CONT*	cursorFriend;#endif
	PC_ACTCONT*		pcActCont;
//	PC_ACTCONT*		friendActCont;
	FLD_ACTCONT*	fldActCont;
	int				mapNum;
	
	int				key_cont;
	
	int d_menu_flag;
	DEBUG_FLDMENU *d_menu;

	void *pGridCont;
};

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _DEPEND_FUNCTIONS{
	void (*create_func)(FIELD_MAIN_WORK*, VecFx32*, u16);
	void (*main_func)(FIELD_MAIN_WORK*, VecFx32*);
	void (*delete_func)(FIELD_MAIN_WORK*);
};

//------------------------------------------------------------------
/**
 * @brief	ローカル宣言
 */
//------------------------------------------------------------------
static BOOL GameEndCheck( int cont );

FIELD_MAIN_WORK* fieldWork;

static BOOL DebugMenuProc( FIELD_MAIN_WORK *fldWork );

//------------------------------------------------------------------
/**
 * @brief	ワークの確保と破棄
 */
//------------------------------------------------------------------
void	FieldBoot( HEAPID heapID )
{
	fieldWork = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_MAIN_WORK) );
	fieldWork->heapID = heapID;
	fieldWork->gamemode = GAMEMODE_NORMAL;

//	GFL_UI_TP_Init( fieldWork->heapID );
}

void	FieldEnd( void )
{
//	GFL_UI_TP_Exit();

	GFL_HEAP_FreeMemory( fieldWork );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
BOOL	FieldMain( void )
{
	BOOL return_flag = FALSE,bSkip = FALSE;
	int i;

	fieldWork->timer++;
//	GFL_UI_TP_Main();

	switch( fieldWork->seq ){

	case 0:
		//基本システムセットアップ
		fieldWork->gs = SetupGameSystem( fieldWork->heapID );
		fieldWork->mapNum = 0;
		fieldWork->seq++;
        break;

	case 1:
		fieldWork->ftbl = resistMapTbl[fieldWork->mapNum].dep_funcs;
		{
            //セットアップ
            ResistDataFieldG3Dmapper( GetFieldG3Dmapper(fieldWork->gs), 
                                &resistMapTbl[fieldWork->mapNum].mapperData );

			//登録テーブルごとに個別の初期化処理を呼び出し
			{
				VecFx32 pos;
				u16		dir;

				pos = resistMapTbl[fieldWork->mapNum].startPos;
				dir = 0;
				fieldWork->ftbl->create_func( fieldWork, &pos, dir );
			}
			
			{	//デバッグメニュー
				fieldWork->d_menu = FldDebugMenu_Init(
						fieldWork, fieldWork->mapNum, fieldWork->heapID );
			}

            fieldWork->seq++;
        }
		break;

	case 2:
		if( GameEndCheck( GFL_UI_KEY_GetCont() ) == TRUE ){
			fieldWork->gamemode = GAMEMODE_FINISH;
			fieldWork->seq = 3;
			break;
		}
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_START ){
			fieldWork->mapNum++;
			if( fieldWork->mapNum >= (resistMapTblCount) ){
				fieldWork->mapNum = 0;
			}
			fieldWork->seq = 3;
			break;
		}
#if 0
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_SELECT ){
			fieldWork->mapNum--;
			if( fieldWork->mapNum < 0 ){
				fieldWork->mapNum = (resistMapTblCount)-1;
			}
			fieldWork->seq = 3;
			break;
		}
#endif
		
		if( !DebugMenuProc( fieldWork ) ) {
		
			VecFx32 pos;
			fieldWork->key_cont = GFL_UI_KEY_GetCont();
			//登録テーブルごとに個別のメイン処理を呼び出し
			fieldWork->ftbl->main_func( fieldWork, &pos );
			//Mapシステムに位置を渡している。これがないとマップ移動しないので注意
			SetPosFieldG3Dmapper( GetFieldG3Dmapper(fieldWork->gs), &pos );
		}

		MainGameSystem( fieldWork->gs );
		break;

	case 3:
		//登録テーブルごとに個別の終了処理を呼び出し
		fieldWork->ftbl->delete_func(fieldWork);

        ReleaseDataFieldG3Dmapper( GetFieldG3Dmapper(fieldWork->gs) );

		if (fieldWork->gamemode != GAMEMODE_FINISH) {
			fieldWork->seq = 1;
		} else {
			fieldWork->seq = 4;
		}
		
		FldDebugMenu_Delete( fieldWork->d_menu );
		break;

	case 4:
		RemoveGameSystem( fieldWork->gs );
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
	int resetCont = PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START;

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
//#define BACKGROUND_COL	(GX_RGB(23,29,31))		//背景色
//#define FOG_COL			(GX_RGB(31,31,31))		//フォグ色
#define BACKGROUND_COL	(GX_RGB(30,31,31))		//背景色
#define FOG_COL			(GX_RGB(31,31,31))		//フォグ色
#define DTCM_SIZE		(0x1000)				//DTCMエリアのサイズ

//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
struct _FIELD_SETUP {
	GFL_G3D_UTIL*			g3Dutil;		//g3Dutil Lib ハンドル
	u16						g3DutilUnitIdx;	//g3Dutil Unitインデックス
	GFL_G3D_SCENE*			g3Dscene;		//g3Dscene Lib ハンドル
	GFL_G3D_CAMERA*			g3Dcamera;		//g3Dcamera Lib ハンドル
	GFL_G3D_LIGHTSET*		g3Dlightset;	//g3Dlight Lib ハンドル
	GFL_TCB*				g3dVintr;		//3D用vIntrTaskハンドル
	GFL_BBDACT_SYS*			bbdActSys;		//ビルボードアクトシステム設定ハンドル

	FLD_G3D_MAPPER*				g3Dmapper;

	HEAPID					heapID;
};

//------------------------------------------------------------------
/**
 * @brief	ディスプレイ環境データ
 */
//------------------------------------------------------------------
///ＶＲＡＭバンク設定構造体
static const GFL_BG_DISPVRAM dispVram = {
	GX_VRAM_BG_128_D,				//メイン2DエンジンのBGに割り当て 
	GX_VRAM_BGEXTPLTT_NONE,			//メイン2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_SUB_BG_32_H,			//サブ2DエンジンのBGに割り当て
	GX_VRAM_SUB_BGEXTPLTT_NONE,		//サブ2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_OBJ_64_E,				//メイン2DエンジンのOBJに割り当て
	GX_VRAM_OBJEXTPLTT_NONE,		//メイン2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_SUB_OBJ_16_I,			//サブ2DエンジンのOBJに割り当て
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	//サブ2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_TEX_012_ABC,			//テクスチャイメージスロットに割り当て
	GX_VRAM_TEXPLTT_0_G,			//テクスチャパレットスロットに割り当て
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

//static const char font_path[] = {"gfl_font.dat"};

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
	{ 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 1, {{  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
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
static void	bg_init( FIELD_SETUP* gs );
static void	bg_exit( FIELD_SETUP* gs );
//３Ｄ関数
static void g3d_load( FIELD_SETUP* gs );
static void g3d_control( FIELD_SETUP* gs );
static void g3d_draw( FIELD_SETUP* gs );
static void	g3d_unload( FIELD_SETUP* gs );
static void	g3d_vblank( GFL_TCB* tcb, void* work );

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
static FIELD_SETUP*	SetupGameSystem( HEAPID heapID )
{
	FIELD_SETUP*	gs = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_SETUP) );
	gs->heapID = heapID;

	//乱数初期化
	GFL_STD_MtRandInit(0);

	//ARCシステム初期化
//	GFL_ARC_Init( &GraphicFileTable[0], NELEMS(GraphicFileTable) );		gfl_use.cで1回だけ初期化に変更

	//VRAMクリア
	GFL_DISP_ClearVRAM( GX_VRAM_D );
	//VRAM設定
	GFL_DISP_SetBank( &dispVram );

	//BG初期化
	bg_init( gs );

	//３Ｄデータのロード
	g3d_load( gs );
	gs->g3dVintr = GFUser_VIntr_CreateTCB( g3d_vblank, (void*)gs, 0 );

	return gs;
}

//------------------------------------------------------------------
/**
 * @brief	セットアップ関数
 */
//------------------------------------------------------------------
static void	RemoveGameSystem( FIELD_SETUP* gs )
{
	GFL_TCB_DeleteTask( gs->g3dVintr );
	g3d_unload( gs );	//３Ｄデータ破棄

	bg_exit( gs );
//	GFL_ARC_Exit();

	GFL_HEAP_FreeMemory( gs );
}

//------------------------------------------------------------------
/**
 * @brief	システムメイン関数
 */
//------------------------------------------------------------------
static void	MainGameSystem( FIELD_SETUP* gs )
{
	g3d_control( gs );
	g3d_draw( gs );
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

static void	bg_init( FIELD_SETUP* gs )
{
	//フォント読み込み
//	GFL_TEXT_CreateSystem( font_path );
//	GFL_TEXT_CreateSystem( NULL );	//システムフォント使用

	//ＢＧシステム起動
	GFL_BG_Init( gs->heapID );

	//背景色パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( gs->heapID, 16*2 );
		plt[0] = BACKGROUND_COL;
		GFL_BG_LoadPalette( GFL_BG_FRAME0_M, plt, 16*2, 0 );	//メイン画面の背景色転送
		GFL_BG_LoadPalette( GFL_BG_FRAME0_S, plt, 16*2, 0 );	//サブ画面の背景色転送
		GFL_HEAP_FreeMemory( plt );
	}
	//ＢＧモード設定
	GFL_BG_SetBGMode( &bgsysHeader );

	//ＢＧコントロール設定
	G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 16, 8 );

	//３Ｄシステム起動
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX384K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, gs->heapID, G3DsysSetup );
	GFL_BG_SetBGControl3D( G3D_FRM_PRI );

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
	GFL_DISP_SetDispOn();
}

static void	bg_exit( FIELD_SETUP* gs )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );

	GFL_G3D_Exit();
	GFL_BG_Exit();
//	GFL_TEXT_DeleteSystem();
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
 * @brief		３Ｄデータロード
 */
static void	g3d_trans_BBD( GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz );
//------------------------------------------------------------------
//作成
static void g3d_load( FIELD_SETUP* gs )
{
	//配置物設定

	//g3Dutilを使用し配列管理をする
	gs->g3Dutil = GFL_G3D_UTIL_Create( G3D_UTIL_RESCOUNT, G3D_UTIL_OBJCOUNT, gs->heapID );
	//g3Dsceneを使用し管理をする
	gs->g3Dscene = GFL_G3D_SCENE_Create( gs->g3Dutil, 
						G3D_SCENE_OBJCOUNT, G3D_OBJWORK_SZ, G3D_ACC_COUNT, TRUE, gs->heapID );

	gs->g3Dmapper = CreateFieldG3Dmapper( gs->heapID );
	gs->bbdActSys = GFL_BBDACT_CreateSys
					( G3D_BBDACT_RESMAX, G3D_BBDACT_ACTMAX, g3d_trans_BBD, gs->heapID );

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
}
	
//動作
static void g3d_control( FIELD_SETUP* gs )
{
	GFL_G3D_SCENE_Main( gs->g3Dscene ); 
	MainFieldG3Dmapper( gs->g3Dmapper );
	GFL_BBDACT_Main( gs->bbdActSys );
}

//描画
static void g3d_draw( FIELD_SETUP* gs )
{
	GFL_G3D_CAMERA_Switching( gs->g3Dcamera );
	GFL_G3D_LIGHT_Switching( gs->g3Dlightset );
	DrawFieldG3Dmapper( gs->g3Dmapper, gs->g3Dcamera );
	GFL_BBDACT_Draw( gs->bbdActSys, gs->g3Dcamera, gs->g3Dlightset );
	GFL_G3D_SCENE_Draw( gs->g3Dscene );  
}

//破棄
static void g3d_unload( FIELD_SETUP* gs )
{
	GFL_G3D_LIGHT_Delete( gs->g3Dlightset );
	GFL_G3D_CAMERA_Delete( gs->g3Dcamera );

	GFL_BBDACT_DeleteSys( gs->bbdActSys );
	DeleteFieldG3Dmapper( gs->g3Dmapper );

	GFL_G3D_SCENE_Delete( gs->g3Dscene );  
	GFL_G3D_UTIL_Delete( gs->g3Dutil );
}
	
static void	g3d_vblank( GFL_TCB* tcb, void* work )
{
	FIELD_SETUP* gs =  (FIELD_SETUP*)work;
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
GFL_G3D_CAMERA* GetG3Dcamera( FIELD_SETUP* gs )
{
	return gs->g3Dcamera;
}

FLD_G3D_MAPPER* GetFieldG3Dmapper( FIELD_SETUP* gs )
{
	return gs->g3Dmapper;
}

GFL_BBDACT_SYS* GetBbdActSys( FIELD_SETUP* gs )
{
	return gs->bbdActSys;
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

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL CalcSetGroundMove( const FLD_G3D_MAPPER* g3Dmapper, FLD_G3D_MAPPER_INFODATA* gridInfoData, 
								VecFx32* pos, VecFx32* vecMove, fx32 speed )
{
	FLD_G3D_MAPPER_GRIDINFO gridInfo;
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
	if( CheckFieldG3DmapperOutRange( g3Dmapper, &posNext ) == TRUE ){
		OS_Printf("マップ範囲外で移動不可\n");
		return FALSE;
	}

	//プレーヤー用動作。この位置中心に高さデータが存在するため、すべて取得して設定
	if( GetFieldG3DmapperGridInfo( g3Dmapper, &posNext, &gridInfo ) == FALSE ){
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
//============================================================================================
#include "field_sub_normal.c"
#include "field_sub_nogrid.c"
#include "field_sub_grid.c"

const DEPEND_FUNCTIONS FieldBaseFunctions = {
	NormalCreate,
	NormalMain,
	NormalDelete,
};

const DEPEND_FUNCTIONS FieldGridFunctions = {
	GridMoveCreate,
	GridMoveMain,
	GridMoveDelete,
};

const DEPEND_FUNCTIONS FieldNoGridFunctions = {
	NoGridCreate,
	NoGridMain,
	NoGridDelete,
};

//======================================================================
//	debug
//======================================================================
//--------------------------------------------------------------
///	デバッグメニュー処理
//--------------------------------------------------------------
static BOOL DebugMenuProc( FIELD_MAIN_WORK *fldWork )
{
	if( fldWork->d_menu_flag == FALSE ){
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_SELECT ){
			FldDebugMenu_Create( fldWork->d_menu );
			fldWork->d_menu_flag = TRUE;
		}
	}else{	//起動中
		if( FldDebugMenu_Main(fldWork->d_menu) == TRUE ){
			fldWork->d_menu_flag = FALSE;
		}
	}
	return fldWork->d_menu_flag;
}
