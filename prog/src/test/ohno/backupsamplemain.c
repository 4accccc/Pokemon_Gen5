//============================================================================================
/**
 * @file	samplemain.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "net/network_define.h"
#include "textprint.h"
#include "arc_def.h"
#include "../../ohno/test_flashread.h"

void	SampleBoot( HEAPID heapID );
void	SampleEnd( void );
BOOL	SampleMain( void );

#include "g3d_mapper.h"
#include "sample_net.h"
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
static void				RemoveGameSystem( SAMPLE_SETUP* gs );
static void				MainGameSystem( SAMPLE_SETUP* gs );
static G3D_MAPPER*		GetG3Dmapper( SAMPLE_SETUP* gs );

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

typedef struct _FLD_ACTCONT		FLD_ACTCONT;
static FLD_ACTCONT*		CreateFieldActSys( SAMPLE_SETUP* gs, HEAPID heapID );
static void				DeleteFieldActSys( FLD_ACTCONT* fldActCont );
static void				MainFieldActSys( FLD_ACTCONT* fldActCont );

static void _sendGamePlay( VecFx32* pVec  );

#include "samplemain.h"

#define	INTERVAL		(16)
#define	CH_MAX			(1)
#define	STRM_BUF_SIZE	(INTERVAL*2*CH_MAX*32)
#define	SWAV_HEAD_SIZE	(18)

//32バイトアライメントでヒープからの取り方がわからないので、とりあえず静的に
static	u8				strmBuffer[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);

//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
typedef struct {
    VecFx32         recvWork;
	HEAPID			heapID;
	int				seq;
	int				timer;

	SAMPLE_SETUP*	gs;
	CURSOR_CONT*	cursor;
	CURSOR_CONT*	cursorFriend;
	FLD_ACTCONT*	fldActCont;
	int				mapNum;

	NNSSndStrm		strm;
	u8				FS_strmBuffer[STRM_BUF_SIZE];
	int				FSReadPos;
	u32				strmReadPos;
	u32				strmWritePos;
}SAMPLE_WORK;

static	void	StrmSetUp(SAMPLE_WORK *sw);
static	void	StrmCBWaveDataStock(NNSSndStrmCallbackStatus status,
									int numChannels,
									void *buffer[],
									u32	len,
									NNSSndStrmFormat format,
									void *arg);
static	void	StrmBufferCopy(SAMPLE_WORK *sw,int size);

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
static void SoundWriteFlash(HEAPID id);
void	SampleBoot( HEAPID heapID )
{
	u8 chNoList[1]={0};

	sampleWork = GFL_HEAP_AllocClearMemory( heapID, sizeof(SAMPLE_WORK) );
	sampleWork->heapID = heapID;

//	GFL_UI_TP_Init( sampleWork->heapID );
    NNS_SndStrmInit( &sampleWork->strm );
	NNS_SndStrmAllocChannel( &sampleWork->strm, CH_MAX, chNoList);

    SoundWriteFlash(sampleWork->heapID);
	StrmSetUp(sampleWork);
}

void	SampleEnd( void )
{
//	GFL_UI_TP_Exit();

	GFL_HEAP_FreeMemory( sampleWork );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
static void Debug_Regenerate( void );
//------------------------------------------------------------------
BOOL	SampleMain( void )
{
	BOOL return_flag = FALSE,bSkip = FALSE;
	int i;

	sampleWork->timer++;
//	GFL_UI_TP_Main();
	StrmBufferCopy(sampleWork,0);

	switch( sampleWork->seq ){

	case 0:
		//基本システムセットアップ
		sampleWork->gs = SetupGameSystem( sampleWork->heapID );
		sampleWork->mapNum = 0;
		sampleWork->seq++;
#ifdef NET_WORK_ON
        InitSampleGameNet();
#endif
        break;

	case 1:
#ifdef NET_WORK_ON
        bSkip = ConnectSampleGameNet();  // 通信処理
        if( !bSkip && GFL_UI_KEY_GetTrg() ){  // キーが押されたら通信を待たずに開始
            EndSampleGameNet();
            ExitSampleGameNet();
            bSkip = TRUE;
        }
#else
        bSkip = TRUE;
#endif
		if( bSkip ){
            //セットアップ
            ResistData3Dmapper( GetG3Dmapper(sampleWork->gs), 
                                &resistMapTbl[sampleWork->mapNum].mapperData );
            {
                G3D_MAPPEROBJ_RESIST resistObjData;
                
                resistObjData.arcID = ARCID_SAMPLEMAP;
                resistObjData.data = resistObjTbl; 
                resistObjData.count = NELEMS(resistObjTbl); 
                ResistObjRes3Dmapper( GetG3Dmapper(sampleWork->gs), &resistObjData );
            }
            {
                G3D_MAPPERDDOBJ_RESIST resistDDobjData;
                
                resistDDobjData.arcID = ARCID_SAMPLEMAP;
                resistDDobjData.data = resistDDobjTbl; 
                resistDDobjData.count = NELEMS(resistDDobjTbl); 
                ResistDDobjRes3Dmapper( GetG3Dmapper(sampleWork->gs), &resistDDobjData );
            }
            sampleWork->cursor = CreateCursor( sampleWork->gs, sampleWork->heapID );
            sampleWork->fldActCont = CreateFieldActSys( sampleWork->gs, sampleWork->heapID );
            SetCursorTrans( sampleWork->cursor, &resistMapTbl[sampleWork->mapNum].startPos );
#ifdef NET_WORK_ON
            sampleWork->cursorFriend = CreateCursor( sampleWork->gs, sampleWork->heapID );
            SetCursorTrans( sampleWork->cursorFriend, &resistMapTbl[sampleWork->mapNum].startPos );
            GFL_NET_ReloadIcon();
#endif
			NNS_SndStrmStart( &sampleWork->strm );
            sampleWork->seq++;
        }
		break;

	case 2:
		if( GameEndCheck( GFL_UI_KEY_GetCont() ) == TRUE ){
			sampleWork->seq = 4;
			break;
		}
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_START ){
			sampleWork->mapNum++;
			if( sampleWork->mapNum >= NELEMS(resistMapTbl) ){
				sampleWork->mapNum = 0;
			}
			sampleWork->seq = 3;
			break;
		}
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_SELECT ){
			sampleWork->mapNum--;
			if( sampleWork->mapNum < 0 ){
				sampleWork->mapNum = NELEMS(resistMapTbl)-1;
			}
			sampleWork->seq = 3;
			break;
		}
		{
			VecFx32 pos;

			MainCursor( sampleWork->cursor );
#ifdef NET_WORK_ON
			FriendCursor( sampleWork->cursorFriend );
#endif
			GetCursorTrans( sampleWork->cursor, &pos );
			SetPos3Dmapper( GetG3Dmapper(sampleWork->gs), &pos );
			MainFieldActSys( sampleWork->fldActCont );
		}
		MainGameSystem( sampleWork->gs );
		break;

	case 3:
		ReleaseDDobjRes3Dmapper( GetG3Dmapper(sampleWork->gs) );
		ReleaseObjRes3Dmapper( GetG3Dmapper(sampleWork->gs) );
		DeleteFieldActSys( sampleWork->fldActCont );
		DeleteCursor( sampleWork->cursor );
#ifdef NET_WORK_ON
		DeleteCursor( sampleWork->cursorFriend );
#endif
        sampleWork->seq = 1;
		break;

	case 4:
		ReleaseDDobjRes3Dmapper( GetG3Dmapper(sampleWork->gs) );
		ReleaseObjRes3Dmapper( GetG3Dmapper(sampleWork->gs) );
		DeleteFieldActSys( sampleWork->fldActCont );
		DeleteCursor( sampleWork->cursor );
#ifndef NET_WORK_ON
		RemoveGameSystem( sampleWork->gs );
		return_flag = TRUE;
        break;
#else
		DeleteCursor( sampleWork->cursorFriend );
        EndSampleGameNet();
        sampleWork->seq++;
    case 5:
        if(ExitSampleGameNet()){
            RemoveGameSystem( sampleWork->gs );
            return_flag = TRUE;
        }
        break;
#endif
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
struct _SAMPLE_SETUP {
	GFL_G3D_UTIL*			g3Dutil;		//g3Dutil Lib ハンドル
	u16						g3DutilUnitIdx;	//g3Dutil Unitインデックス
	GFL_G3D_SCENE*			g3Dscene;		//g3Dscene Lib ハンドル
	GFL_G3D_CAMERA*			g3Dcamera;		//g3Dcamera Lib ハンドル
	GFL_G3D_LIGHTSET*		g3Dlightset;	//g3Dlight Lib ハンドル
	GFL_TCB*				g3dVintr;		//3D用vIntrTaskハンドル
	GFL_BBDACT_SYS*			bbdActSys;		//ビルボードアクトシステム設定ハンドル

	G3D_MAPPER*				g3Dmapper;

	HEAPID					heapID;
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
	GX_VRAM_SUB_BG_128_C,			//サブ2DエンジンのBGに割り当て
	GX_VRAM_SUB_BGEXTPLTT_NONE,		//サブ2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_OBJ_64_E,				//メイン2DエンジンのOBJに割り当て
	GX_VRAM_OBJEXTPLTT_NONE,		//メイン2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_SUB_OBJ_16_I,			//サブ2DエンジンのOBJに割り当て
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	//サブ2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_TEX_01_AB,				//テクスチャイメージスロットに割り当て
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
static void	bg_init( SAMPLE_SETUP* gs );
static void	bg_exit( SAMPLE_SETUP* gs );
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
static void	RemoveGameSystem( SAMPLE_SETUP* gs )
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
static void	MainGameSystem( SAMPLE_SETUP* gs )
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

static void	bg_init( SAMPLE_SETUP* gs )
{
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
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						DTCM_SIZE, gs->heapID, G3DsysSetup );
	GFL_BG_SetBGControl3D( G3D_FRM_PRI );

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
	GFL_DISP_SetDispOn();
}

static void	bg_exit( SAMPLE_SETUP* gs )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );

	GFL_G3D_Exit();
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
 * @brief		３Ｄデータロード
 */
static void	g3d_trans_BBD( GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz );
//------------------------------------------------------------------
//作成
static void g3d_load( SAMPLE_SETUP* gs )
{
	//配置物設定

	//g3Dutilを使用し配列管理をする
	gs->g3Dutil = GFL_G3D_UTIL_Create( G3D_UTIL_RESCOUNT, G3D_UTIL_OBJCOUNT, gs->heapID );
	//g3Dsceneを使用し管理をする
	gs->g3Dscene = GFL_G3D_SCENE_Create( gs->g3Dutil, 
						G3D_SCENE_OBJCOUNT, G3D_OBJWORK_SZ, G3D_ACC_COUNT, TRUE, gs->heapID );

	gs->g3Dmapper = Create3Dmapper( gs->heapID );
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
	GFL_G3D_SCENE_Draw( gs->g3Dscene );  
}

//破棄
static void g3d_unload( SAMPLE_SETUP* gs )
{
	GFL_G3D_LIGHT_Delete( gs->g3Dlightset );
	GFL_G3D_CAMERA_Delete( gs->g3Dcamera );

	GFL_BBDACT_DeleteSys( gs->bbdActSys );
	Delete3Dmapper( gs->g3Dmapper );

	GFL_G3D_SCENE_Delete( gs->g3Dscene );  
	GFL_G3D_UTIL_Delete( gs->g3Dutil );
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
	G3D_MAPPER_INFODATA gridInfoDataNext;
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
		gridInfoDataNext = gridInfo.gridData[p];	//グリッドデータ更新
	}
	if( initSw == FALSE ){
		if(FX_Mul((height-pos->y),(height-pos->y))>=FX_Mul(WALK_LIMIT_HEIGHT,WALK_LIMIT_HEIGHT)){
			return FALSE;
		}
	} 
	*gridInfoData = gridInfoDataNext;								//グリッドデータ更新
	VEC_Set( pos, posNext.x, gridInfoData->height, posNext.z );		//位置情報更新

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
	HEAPID				heapID;
	SAMPLE_SETUP*		gs;
	u16					unitIdx;
	u16					resIdx;
	u16					objIdx;

	u16					cursorIdx;
	VecFx32				trans;
	fx32				cameraHeight;
	u16					cameraLength;
	u16					direction;

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

#define	CAMERA_LENGTH	(16)
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
	cursor->cameraHeight = 0;
	cursor->cameraLength = CAMERA_LENGTH;
	cursor->direction = 0;
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
#define MV_SPEED		(2*FX32_ONE)
#define RT_SPEED		(FX32_ONE/8)
#define	CAMERA_TARGET_HEIGHT	(4)//(8)
static void	MainCursor( CURSOR_CONT* cursor )
{
	VecFx32	pos, target;
	VecFx32	vecMove = { 0, 0, 0 };
	VecFx32	vecUD = { 0, 0, 0 };
	int		key;
	BOOL	mvFlag = FALSE;

	key = GFL_UI_KEY_GetCont();

	if( key & PAD_BUTTON_B ){
		if( cursor->cameraLength > 8 ){
			cursor->cameraLength -= 8;
		}
		//vecMove.y = -MV_SPEED;
	}
	if( key & PAD_BUTTON_A ){
		if( cursor->cameraLength < 4096 ){
			cursor->cameraLength += 8;
		}
		//vecMove.y = MV_SPEED;
	}
	if( key & PAD_BUTTON_Y ){
		cursor->cameraHeight -= MV_SPEED;
	}
	if( key & PAD_BUTTON_X ){
		cursor->cameraHeight += MV_SPEED;
	}
	if( key & PAD_KEY_UP ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(cursor->direction + 0x8000) );
		vecMove.z = FX_CosIdx( (u16)(cursor->direction + 0x8000) );
	}
	if( key & PAD_KEY_DOWN ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(cursor->direction + 0x0000) );
		vecMove.z = FX_CosIdx( (u16)(cursor->direction + 0x0000) );
	}
	if( key & PAD_KEY_LEFT ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(cursor->direction + 0xc000) );
		vecMove.z = FX_CosIdx( (u16)(cursor->direction + 0xc000) );
	}
	if( key & PAD_KEY_RIGHT ){
		mvFlag = TRUE;
		vecMove.x = FX_SinIdx( (u16)(cursor->direction + 0x4000) );
		vecMove.z = FX_CosIdx( (u16)(cursor->direction + 0x4000) );
	}

    
	if( key & PAD_BUTTON_R ){
		cursor->direction -= RT_SPEED;
	}
	if( key & PAD_BUTTON_L ){
		cursor->direction += RT_SPEED;
	}
	CalcSetGroundMove( GetG3Dmapper(cursor->gs), &cursor->gridInfoData, 
								&cursor->trans, &vecMove, MV_SPEED );
	//cursor->cameraHeight += vecMove.y;

#ifdef NET_WORK_ON
    _sendGamePlay( &cursor->trans  );  // 自分の位置を相手に送信
#endif
    
	VEC_Set(&target,cursor->trans.x,cursor->trans.y+CAMERA_TARGET_HEIGHT*FX32_ONE,cursor->trans.z);
	pos.x = cursor->trans.x + cursor->cameraLength * FX_SinIdx(cursor->direction);
	pos.y = cursor->trans.y + cursor->cameraHeight;
	pos.z = cursor->trans.z + cursor->cameraLength * FX_CosIdx(cursor->direction);

	GFL_G3D_SCENEOBJ_SetPos(	GFL_G3D_SCENEOBJ_Get(cursor->gs->g3Dscene, cursor->cursorIdx), 
								&cursor->trans );
	GFL_G3D_CAMERA_SetTarget( cursor->gs->g3Dcamera, &target );
	GFL_G3D_CAMERA_SetPos( cursor->gs->g3Dcamera, &pos );

}



static void	FriendCursor( CURSOR_CONT* cursor )
{
    
    GFL_STD_MemCopy((const void*)&sampleWork->recvWork ,&cursor->trans, sizeof(VecFx32));
	GFL_G3D_SCENEOBJ_SetPos(	GFL_G3D_SCENEOBJ_Get(cursor->gs->g3Dscene, cursor->cursorIdx), 
								&cursor->trans );
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

//============================================================================================
/**
 * @brief	プレーヤーアクター
 */
//============================================================================================
typedef struct _PC_ACTCONT PC_ACTCONT;

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
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopRAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopDAnm[] = {
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCwalkLAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkRAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkUAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 20, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkDAnm[] = {
	{ 22, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 23, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCrunLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 16, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 13, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCjumpLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 2 },
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
	PC_ACTCONT*	pc = (PC_ACTCONT*)work;
	VecFx32	trans = pc->trans;
	int		anmID;
	u16		dir;

	dir = pc->direction - getCameraRotate( GetG3Dcamera(pc->gs) );
	anmID = getPlayerBBDanm( pc->anmSetID, dir, playerBBDanmOffsTblMine );

	//カメラ補正(アニメ向きの変更をするのに参照)
	GFL_BBDACT_SetAnimeIdxContinue( GetBbdActSys( pc->gs ), actIdx, anmID );
	//位置補正
	trans.x = pc->trans.x;
	trans.y = pc->trans.y + FX32_ONE*7;	//補正
	trans.z = pc->trans.z;
	GFL_BBD_SetObjectTrans( bbdSys, actIdx, &trans );
}

static void playerBBDactSetUp( PC_ACTCONT* pc )
{
	GFL_BBDACT_SYS* bbdActSys = GetBbdActSys( pc->gs );
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	int		i, objIdx;
	VecFx32	trans;
	u8		alpha;
	BOOL	drawEnable;
	u16		setActNum;

	//リソースセットアップ
	pc->bbdActResUnitID = GFL_BBDACT_AddResourceUnit( bbdActSys, playerBBDactResTable,
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
	actData.work = pc;

	pc->bbdActActUnitID = GFL_BBDACT_AddAct( bbdActSys, pc->bbdActResUnitID, &actData, 1 );
	//GFL_BBDACT_BindActTexRes( bbdActSys, pc->bbdActActUnitID, pc->bbdActResUnitID+1 );
	GFL_BBDACT_BindActTexResLoad
		( bbdActSys, pc->bbdActActUnitID, ARCID_FLDACT, NARC_fld_act_hero_nsbtx );

	GFL_BBDACT_SetAnimeTable( bbdActSys, pc->bbdActActUnitID, 
								playerBBDactAnmTable, NELEMS(playerBBDactAnmTable) );
	GFL_BBDACT_SetAnimeIdxOn( bbdActSys, pc->bbdActActUnitID, 0 );
}

static void playerBBDactRelease( PC_ACTCONT* pc )
{
	GFL_BBDACT_SYS* bbdActSys = GetBbdActSys( pc->gs );

	GFL_BBDACT_RemoveAct( bbdActSys, pc->bbdActActUnitID, 1 );
	GFL_BBDACT_RemoveResourceUnit(	bbdActSys, pc->bbdActResUnitID, NELEMS(playerBBDactResTable) );
}

static void playerBBDactSetAnm( PC_ACTCONT* pc )
{
	int		anmID;
	u16		dir;

	dir = pc->direction - getCameraRotate( GetG3Dcamera(pc->gs) );
	anmID = getPlayerBBDanm( pc->anmSetID, dir, playerBBDanmOffsTblMine );

	GFL_BBDACT_SetAnimeIdx( GetBbdActSys( pc->gs ), pc->bbdActActUnitID, anmID );
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

			VEC_Set( &setTrans, nowTrans.x, nowTrans.y + FX32_ONE*7, nowTrans.z );
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


//============================================================================================
//
//
//	通信関数
//
//
//============================================================================================
#ifdef NET_WORK_ON
//------------------------------------------------------------------
/**
 * @brief	受信ワーク初期化
 */
//------------------------------------------------------------------
static void _initRecvBuffer( void )
{
}

//------------------------------------------------------------------
/**
 * @brief	受信関数
 */
//------------------------------------------------------------------
// ローカル通信コマンドの定義
enum _gameCommand_e {
	_GAME_COM_PLAY = GFL_NET_CMD_DEBUG_OHNO,
};

//------------------------------------------------------------------
// _GAME_COM_PLAY　ゲーム情報送受信

static void _sendGamePlay( VecFx32* pVec  )
{
    SendSampleGameNet( _GAME_COM_PLAY, pVec );
}

static void _recvGamePlay
	(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    SAMPLE_WORK* psw = sampleWork;

    if(GetSampleNetID() != netID){
        GFL_STD_MemCopy(pData, &psw->recvWork, sizeof(VecFx32));
    }
}

//------------------------------------------------------------------
// ローカル通信テーブル
//const NetRecvFuncTable NetSamplePacketTbl[] = {
//    { _recvGamePlay, GFL_NET_COMMAND_SIZE(sizeof(VecFx32)), NULL },
//};

//---------------------------------------------------------------------------
//	ストリーム再生関数
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static	void	StrmSetUp(SAMPLE_WORK *sw)
{
	sw->FSReadPos=SWAV_HEAD_SIZE;
	sw->strmReadPos=0;
	sw->strmWritePos=0;
	StrmBufferCopy(sw,STRM_BUF_SIZE);
    
	NNS_SndStrmSetup( &sw->strm,
					  NNS_SND_STRM_FORMAT_PCM8,
					  &strmBuffer[0],
					  STRM_BUF_SIZE,
					  NNS_SND_STRM_TIMER_CLOCK/8000,
					  INTERVAL,
					  StrmCBWaveDataStock,
					  sw);
}

//---------------------------------------------------------------------------
// 波形データを補充するコールバックが割り込み禁止で呼ばれるので、
// FS_～が中から呼べないので、この関数でReadしておく
//---------------------------------------------------------------------------
#if 0
static	void	StrmBufferCopy(SAMPLE_WORK *sw,int size)
{
	FSFile	file;
	s32		ret;

    FS_InitFile(&file);
	FS_OpenFile(&file,"/mh.swav");
	FS_SeekFile(&file,sw->FSReadPos,FS_SEEK_SET);

	if(size){
		ret=FS_ReadFile(&file,&sw->FS_strmBuffer[0],size);
		sw->FSReadPos+=size;
	}
	else{
		while(sw->strmReadPos!=sw->strmWritePos){
			ret=FS_ReadFile(&file,&sw->FS_strmBuffer[sw->strmWritePos],32);
			if(ret==0){
				sw->FSReadPos=SWAV_HEAD_SIZE;
				FS_SeekFile(&file,sw->FSReadPos,FS_SEEK_SET);
				continue;
			}
			sw->FSReadPos+=32;
			sw->strmWritePos+=32;
			if(sw->strmWritePos>=STRM_BUF_SIZE){
				sw->strmWritePos=0;
			}
		}
	}
	FS_CloseFile(&file);
}
#else

static	void	StrmBufferCopy(SAMPLE_WORK *sw,int size)
{
	if(size){
        TEST_ReadFlashRead(&sw->FS_strmBuffer[sw->strmWritePos],sw->FSReadPos,size);
        sw->FSReadPos+=size;
    }
    else{
		while(sw->strmReadPos!=sw->strmWritePos){
            TEST_ReadFlashRead(&sw->FS_strmBuffer[sw->strmWritePos],sw->FSReadPos,32);
			sw->FSReadPos+=32;
			sw->strmWritePos+=32;
			if(sw->strmWritePos>=STRM_BUF_SIZE){
				sw->strmWritePos=0;
			}
		}
    }
}

#endif



//---------------------------------------------------------------------------
// 波形データを補充するコールバック
//---------------------------------------------------------------------------
static	void	StrmCBWaveDataStock(NNSSndStrmCallbackStatus status,
									int numChannels,
									void *buffer[],
									u32	len,
									NNSSndStrmFormat format,
									void *arg)
{
	SAMPLE_WORK *sw=(SAMPLE_WORK *)arg;
	int	i;
	u8	*strBuf;

	strBuf=buffer[0];
	
	for(i=0;i<len;i++){
		strBuf[i]=sw->FS_strmBuffer[i+sw->strmReadPos];
	}

	sw->strmReadPos+=len;
	if(sw->strmReadPos>=STRM_BUF_SIZE){
		sw->strmReadPos=0;
	}

	DC_FlushRange(strBuf,len);
}


static void SoundWriteFlash(HEAPID id)
{
	FSFile	file;
    FS_InitFile(&file);
	FS_OpenFile(&file,"/mh.swav");
    {
        int size = FS_GetLength(&file);
        void* pData = GFL_HEAP_AllocMemory(id,size);
        FS_ReadFile(&file,pData,size);
        FS_CloseFile(&file);
        TEST_ReadFlashStart(pData,size);
        GFL_HEAP_FreeMemory(pData);
    }
}


#endif
