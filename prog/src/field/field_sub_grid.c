//======================================================================
/**
 * @file	field_sub_grid.c
 * @brief	フィールドメイン処理サブ（グリッド移動）
 *
 * このソースはfield_main.cにインクルードされています。
 * 最終的にはちゃんと分割管理されますが、実験環境のために
 * しばらくはこの形式です。
 */
//======================================================================
#include "arc/test_graphic/testgridmap_a.naix"
#include "arc/fieldmap/zone_id.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include  "field/weather_no.h"
#include  "weather.h"

//======================================================================
//	define
//======================================================================
enum
{
	GRIDPROC_INIT = 0,
	GRIDPROC_MAIN,
	GRIDPROC_DEBUG00,
	GRIDPROC_DEBUG01,
};

//#define FLDMAPGRID_ZONEUPDATE

#define PAD_KEY_BIT (PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT)

#define NUM_FX32(a) (FX32_ONE*(a))
#define FX32_NUM(a) ((a)/FX32_ONE)

#define GRID_VALUE	(16)
#define GRID_VALUE_FX32 (GRID_VALUE*FX32_ONE)
#define GRID_VALUE_HALF_FX32 (GRID_VALUE_FX32/2)

#if 0
#define SIZE_GRID(a)((a)/GRID_VALUE)
#define GRID_SIZE(a) ((a)*GRID_VALUE)
#define SIZE_GRID_FX32(a) (FX32_NUM(SIZE_GRID(a)))
#define GRID_SIZE_FX32(a) (NUM_FX32(GRID_SIZE(a)))
#endif

///マップヒットチェックビット
typedef enum
{
	MAPHITBIT_NON		= (0),		///<無し
	MAPHITBIT_ATTR		= (1<<0),	///<アトリビュートヒット
	MAPHITBIT_DATA		= (1<<1),	///<データエラーヒット
	MAPHITBIT_OUT		= (1<<2),	///<範囲外エラービット
	MAPHITBIT_HEIGHT	= (1<<3),	///<高さヒット
}MAPHITBIT;

#define MAPHITBIT_MASK_ERROR (MAPHITBIT_DATA|MAPHITBIT_OUT)

#define MAPHIT_HEIGHT_OVER (FX32_ONE*17)

#if 0
const u16 TestOBJCodeTbl[];
const TestOBJCodeMax;

const u16 TestOBJCodeTbl[] =
{HERO,BOY1,GIRL1,MAN1,WOMAN1,MIDDLEMAN1,KABI32,OLDWOMAN1};
const int TestOBJCodeMax = NELEMS( TestOBJCodeTbl );
#endif

//自機動作種類
typedef enum
{
	PLAYER_MOVE_STOP,
	PLAYER_MOVE_WALK,
	PLAYER_MOVE_TURN,
	PLAYER_MOVE_HITCH,
}PLAYER_MOVE;

typedef enum
{
	PLAYER_SET_NON,
	PLAYER_SET_STOP,
	PLAYER_SET_WALK,
	PLAYER_SET_TURN,
	PLAYER_SET_HITCH,
}PLAYER_SET;

//======================================================================
//	typedef struct
//======================================================================
typedef struct _FGRID_CONT		FGRID_CONT;
typedef struct _FGRID_PLAYER	FGRID_PLAYER;

//--------------------------------------------------------------
///	FGRID_CONT
//--------------------------------------------------------------
struct _FGRID_CONT
{
	HEAPID heapID;
	FIELD_MAIN_WORK *pFieldWork;
	
	FGRID_PLAYER *pGridPlayer;
	
	LOCATION location;

	u16 *pMapAttr[4];
	
	int proc_switch;
	int scale_num;
	int debug_camera_num;
};

//--------------------------------------------------------------
///	FGRID_PLAYER
//--------------------------------------------------------------
struct _FGRID_PLAYER
{
	u16 dir;
	s16 gx;
	s16 gy;
	s16 gz;
	VecFx32 vec_pos;
	
	fx16 scale_size;	//FX16_ONE*8
	
	u16 move_flag;
	s16 move_dir;
	VecFx32 move_val;
	VecFx32 move_count;
	PLAYER_ANIME_FLAG anime_flag;
	
	const FGRID_CONT *pGridCont;
	PC_ACTCONT *pActCont;
	
	FLDMMDL *pFldMMdl;
	
	int move_state;
};

//--------------------------------------------------------------
///	camera
//--------------------------------------------------------------
typedef struct
{
	u16 len;
	fx32 hi;
}GRID_CAMERA_DATA;

//======================================================================
//	proto
//======================================================================
extern FLDMMDL * Player_GetFldMMdl( PC_ACTCONT *pcActCont );

static FGRID_CONT * FGridCont_Init(
	FIELD_MAIN_WORK *fieldWork, const VecFx32 *pos, u16 dir );
static void FGridCont_Delete( FIELD_MAIN_WORK *fieldWork );

static void GridProc_Main( FIELD_MAIN_WORK *fieldWork, VecFx32 *pos );
static void GridProc_DEBUG00( FIELD_MAIN_WORK *fieldWork, VecFx32 *pos );
static void GridProc_DEBUG01( FIELD_MAIN_WORK *fieldWork, VecFx32 *pos );

static FGRID_PLAYER * FGridPlayer_Init(
	FGRID_CONT *pGridCont, const VecFx32 *pos, u16 dir );
static void FGridPlayer_Delete( FGRID_CONT *pGridCont );
static void FGridPlayer_Move(
	FGRID_PLAYER *pJiki, u32 key_trg, u32 key_cont );
static void FGridPlayer_ScaleSet( FGRID_PLAYER *pJiki, fx16 scale );
static fx16 FGridPlayer_ScaleGet( FGRID_PLAYER *pJiki );

static BOOL GridVecPosMove(
	const FLDMAPPER *g3Dmapper,
	FLDMAPPER_GRIDINFODATA *gridInfoData,
	VecFx32 *pos, VecFx32 *vecMove );
static BOOL GetMapAttr(
	const FLDMAPPER *g3Dmapper, const VecFx32 *pos, u32 *attr );
static fx32 GetMapHeight(
	const FLDMAPPER *g3Dmapper, const VecFx32 *pos );
static MAPHITBIT MapHitCheck(
	const FGRID_CONT *pGridCont, const VecFx32 *now, const VecFx32 *next );
static void FldWorkPlayerWorkPosSet(
		FIELD_MAIN_WORK *fieldWork, const VecFx32 *pos );
static void FldWorkPlayerWorkDirSet(
	FIELD_MAIN_WORK *fieldWork, u16 dir );
static void Jiki_UpdatePlayerWork( FIELD_MAIN_WORK *fieldWork );

static void GridMap_SetupNPC( FIELD_MAIN_WORK *fieldWork );

static void MMdlList_Init( MMDL_LIST *mlist, int list_id, HEAPID heapID );

#if 0
static BOOL gridmap_CheckPlayerPosUpdate(
	FIELD_MAIN_WORK *pFieldMainWork, FGRID_CONT *pGridCont );
static BOOL gridmap_CheckMoveZoneChange(
	FIELD_MAIN_WORK *pFieldMainWork, FGRID_CONT *pGridCont );
static void gridmap_MoveZoneChange(
	FIELD_MAIN_WORK *pFieldMainWork, FGRID_CONT *pGridCont );
#endif

//--------------------------------------------------------------
///	data
//--------------------------------------------------------------
static const GRID_CAMERA_DATA DATA_CameraTbl[] =
{
	{ 0x78, 0xd8000 },	//DPぽい
	{ 0x78, 0xb8000 },	//ちょっと近い
	{ 0x78, 0x80000 },	//だいぶ近い
	{ 0x50, 0x40000 },	//凄い近い
	{ 0x108, 0xd8000 },	//遠くを見渡せる
};

#define GRIDCAMERA_MAX (NELEMS(DATA_CameraTbl))

// length 50H height 0x20000H

#define GRIDCAMERA_LENGTH_HALF (0x50)
#define GRIDCAMERA_HEIGHT_HALF (0x20000)
#define BBDSCALE_ONE (FX16_ONE*8-1)
#define BBDSCALE_HALF (BBDSCALE_ONE/2)
#define BBDSCALE_PER1 ((BBDSCALE_ONE-BBDSCALE_HALF)/100)

//======================================================================
//	フィールドグリッドタイプ　メイン
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	初期化処理（デフォルト）
 */
//------------------------------------------------------------------
static void GridMoveCreate(
	FIELD_MAIN_WORK * fieldWork, VecFx32 * pos, u16 dir )
{
	//initFLDMMDL(fieldWork);
	
	fieldWork->pcActCont = CreatePlayerActGrid(
			fieldWork, pos, fieldWork->heapID );
	
//	SetGridPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );
	
	fieldWork->pGridCont = FGridCont_Init( fieldWork, pos, dir );
	
	
	//ビルボード設定
	{
		VecFx32 scale = {
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
		};
		GFL_BBD_SetScale(
			GFL_BBDACT_GetBBDSystem(fieldWork->bbdActSys), &scale );
	}
	
	//マップ描画オフセット
	{
		VecFx32 offs = { -FX32_ONE*8, 0, FX32_ONE*8 };
		FLDMAPPER_SetDrawOffset(
			GetFieldG3Dmapper(fieldWork), &offs );
	}
}

#if 0
static void GridMoveCreate(
	FIELD_MAIN_WORK * fieldWork, VecFx32 * pos, u16 dir)
{
	{
		GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
		PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
		int zone_id = PLAYERWORK_getZoneID( player );
		
		fieldWork->fldActCont = NULL;
		
		if( ZONEDATA_DEBUG_IsSampleObjUse(zone_id) == TRUE ){
			fieldWork->fldActCont =
				FLD_CreateFieldActSys( fieldWork, fieldWork->heapID );
			FLDACT_TestSetup( fieldWork->fldActCont );
		}
	}
	
	fieldWork->pcActCont = CreatePlayerActGrid(
			fieldWork, fieldWork->heapID );
	
	SetGridPlayerActTrans( fieldWork->pcActCont, pos );
	SetPlayerActDirection( fieldWork->pcActCont, &dir );
	
	fieldWork->pGridCont = FGridCont_Init( fieldWork, pos, dir );
	
	//カメラ設定
	FIELD_CAMERA_SetLengthOnXZ( fieldWork->camera_control, DATA_CameraTbl[0].len );
	FIELD_CAMERA_SetHeightOnXZ( fieldWork->camera_control, DATA_CameraTbl[0].hi );
	
	//ビルボード設定
	{
		VecFx32 scale = {
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
		};
		GFL_BBD_SetScale(
			GFL_BBDACT_GetBBDSystem(fieldWork->bbdActSys), &scale );
	}

	//マップ描画オフセット
	{
		VecFx32 offs = { -FX32_ONE*8, 0, FX32_ONE*8 };
		FLDMAPPER_SetDrawOffset(
			GetFieldG3Dmapper(fieldWork), &offs );
	}
}
#endif

//------------------------------------------------------------------
/**
 * @brief	メイン処理（デフォルト）
 */
//------------------------------------------------------------------
static void GridMoveMain( FIELD_MAIN_WORK* fieldWork, VecFx32 * pos )
{
	FGRID_CONT *pGridCont = fieldWork->pGridCont;
	
	switch( pGridCont->proc_switch ){
	case GRIDPROC_INIT:
		{
#if 0
			GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
			PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
			int zone_id = PLAYERWORK_getZoneID( player );
			
			if( ZONEDATA_DEBUG_IsSampleObjUse(zone_id) == TRUE ){
				GridMap_SetupNPC( fieldWork );
			}
#endif
		}
		pGridCont->proc_switch = GRIDPROC_MAIN;
	case GRIDPROC_MAIN: 
		GridProc_Main( fieldWork, pos );
		break;
	case GRIDPROC_DEBUG00:
		GridProc_DEBUG00( fieldWork, pos );
		break;
	case GRIDPROC_DEBUG01:
		GridProc_DEBUG01( fieldWork, pos );
		break;
	}
}

//------------------------------------------------------------------
/**
 * @brief	終了処理（デフォルト）
 */
//------------------------------------------------------------------
static void GridMoveDelete( FIELD_MAIN_WORK* fieldWork )
{
	FGridCont_Delete( fieldWork );
	
	DeletePlayerActGrid( fieldWork->pcActCont );
#if 0	
	FLDMMDLSYS_Push( fieldWork->fldMMdlSys );
	FLDMMDLSYS_DeleteProc( fieldWork->fldMMdlSys );
	fieldWork->fldMMdlSys = NULL;
#endif
}

//======================================================================
//	グリッド　メインプロセス
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド	メイン
 */
//--------------------------------------------------------------
static void GridProc_Main( FIELD_MAIN_WORK *fieldWork, VecFx32 *pos )
{
	FGRID_CONT *pGridCont = fieldWork->pGridCont;

#if 0	//メインに移動
	{	//座標、ゾーン更新チェック
		if( gridmap_CheckPlayerPosUpdate(fieldWork,pGridCont) == TRUE ){
			if( gridmap_CheckMoveZoneChange(fieldWork,pGridCont) == TRUE ){
				gridmap_MoveZoneChange( fieldWork, pGridCont );
			}
		}
	}
#endif

	{	//自機移動
		FGRID_PLAYER *pGridPlayer = pGridCont->pGridPlayer;
		FGridPlayer_Move( pGridPlayer, 0, fieldWork->key_cont );
	}

#if 0
	if( (GFL_UI_KEY_GetTrg()&PAD_BUTTON_L) ){
		pGridCont->debug_camera_num++;
		pGridCont->debug_camera_num %= GRIDCAMERA_MAX;
		FIELD_CAMERA_SetLengthOnXZ( fieldWork->camera_control,
				DATA_CameraTbl[pGridCont->debug_camera_num].len );
		FIELD_CAMERA_SetHeightOnXZ( fieldWork->camera_control,
				DATA_CameraTbl[pGridCont->debug_camera_num].hi );
		OS_Printf( "テストカメラ　No.%d\n", pGridCont->debug_camera_num );
	}
#endif

	//アクター更新
	
	//自機情報の反映
	Jiki_UpdatePlayerWork( fieldWork );
	GetPlayerActTrans( fieldWork->pcActCont, pos );
	
	//カメラへの反映
//	FIELD_CAMERA_SetPos( fieldWork->camera_control, pos );
//	FLD_SetCameraDirection( fieldWork->camera_control, &dir );
}

#define CM_RT_SPEED (FX32_ONE/8)
#define CM_HEIGHT_MV (FX32_ONE*2)

//--------------------------------------------------------------
/**
 * グリッド　デバッグ0 カメラ変化
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void GridProc_DEBUG00( FIELD_MAIN_WORK *fieldWork, VecFx32 *pos )
{
	int trg,cont;
	u16 dir,leng;
	fx32 height;
	FGRID_CONT *pGridCont = fieldWork->pGridCont;
	FGRID_PLAYER *pGridPlayer = pGridCont->pGridPlayer;
	FIELD_CAMERA *camera = fieldWork->camera_control;
	
	trg = GFL_UI_KEY_GetTrg();
	cont = GFL_UI_KEY_GetCont();
	
	if( trg & PAD_BUTTON_B ){
		pGridCont->proc_switch = GRIDPROC_MAIN;
		TAMADA_Printf("switch to GRIDPROC_MAIN\n");
	}else{
		dir = FIELD_CAMERA_GetDirectionOnXZ( camera );
		leng = FIELD_CAMERA_GetLengthOnXZ( camera );
		height = FIELD_CAMERA_GetHeightOnXZ( camera );

		if( cont & PAD_BUTTON_R ){
			dir -= CM_RT_SPEED;
		}
	
		if( cont & PAD_BUTTON_L ){
			dir += CM_RT_SPEED;
		}
	
		if( cont & PAD_KEY_LEFT ){
			if( leng > 8 ){
				leng -= 8;
			}
		}
		if( cont & PAD_KEY_RIGHT ){
			if( leng < 4096 ){
				leng += 8;
			}
		}
	
		if( cont & PAD_KEY_UP ){
			height -= CM_HEIGHT_MV;
		}
	
		if( cont & PAD_KEY_DOWN ){
			height += CM_HEIGHT_MV;
		}
		
		if( trg & (PAD_BUTTON_A) ){
			OS_Printf( "Camera dir = %xH, length = %xH, height = %xH\n",
					dir, leng, height );
		}
	
		FIELD_CAMERA_SetDirectionOnXZ( camera, dir );
		FIELD_CAMERA_SetLengthOnXZ( camera, leng );
		FIELD_CAMERA_SetHeightOnXZ( camera, height );
	}

#if 0	
	{
		if( fieldWork->fldActCont != NULL ){
			FLD_MainFieldActSys( fieldWork->fldActCont );
		}
		
		GetPlayerActTrans( fieldWork->pcActCont, pos );
		FIELD_CAMERA_SetPos( fieldWork->camera_control, pos );
	//	FLD_SetCameraDirection( fieldWork->camera_control, &dir );
	
	}
#endif
}

//--------------------------------------------------------------
/**
 * グリッド　デバッグ1 スケール調節
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void GridProc_DEBUG01( FIELD_MAIN_WORK *fieldWork, VecFx32 *pos )
{
	int trg,cont;
	u16 dir,leng;
	fx32 height;
	fx32 scale;
	FGRID_CONT *pGridCont = fieldWork->pGridCont;
	FGRID_PLAYER *pGridPlayer = pGridCont->pGridPlayer;
	FIELD_CAMERA *camera = fieldWork->camera_control;

	trg = GFL_UI_KEY_GetTrg();
	cont = GFL_UI_KEY_GetCont();
	scale = FGridPlayer_ScaleGet( pGridPlayer );
	
	if( trg & PAD_BUTTON_B ){
		pGridCont->proc_switch = GRIDPROC_MAIN;
		TAMADA_Printf("switch to GRIDPROC_MAIN\n");
	}else{
		if( cont & (PAD_KEY_UP|PAD_KEY_DOWN) ){
			fx16 val = 0x80;	//1=0x1000
		
			if( cont & PAD_KEY_UP ){
				scale -= val;
				
				if( scale < BBDSCALE_HALF ){
					scale = BBDSCALE_HALF;
				}
			}else if( cont & PAD_KEY_DOWN ){
				scale += val;
				
				if( scale > BBDSCALE_ONE ){
					scale = BBDSCALE_ONE;
				}
			}
		
			{
				u16 length;
				int pcent;
				fx32 p_scale;
				fx32 p_height_1,p_height,height;
				fx32 p_length_1,p_length;
				
				p_scale = (scale - BBDSCALE_HALF);
				pcent = p_scale / BBDSCALE_PER1;
				
				p_length_1 = NUM_FX32(0x78 - 0x50) / 100;
				p_length = p_length_1 * pcent;
				length = 0x50 + FX32_NUM(p_length);
				
				p_height_1 = (0xd8000 - 0x20000) / 100;
				p_height = p_height_1 * pcent;
				height = 0x20000 + p_height;
				
				FIELD_CAMERA_SetLengthOnXZ( fieldWork->camera_control, length );
				FIELD_CAMERA_SetHeightOnXZ( fieldWork->camera_control, height );
				
				OS_Printf( "scale = %xH, %dパーセント\n", scale, pcent );
			}

			FGridPlayer_ScaleSet( pGridPlayer, scale );
		}
	}
	

	GetPlayerActTrans( fieldWork->pcActCont, pos );
}

//======================================================================
//	グリッド処理
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド処理　初期化
 * @param	fieldWork	FIELD_MAIN_WORK
 * @retval	FGRID_CONT*
 */
//--------------------------------------------------------------
static FGRID_CONT * FGridCont_Init(
	FIELD_MAIN_WORK *fieldWork, const VecFx32 *pos, u16 dir )
{
	FGRID_CONT *pGridCont;
	
	pGridCont = GFL_HEAP_AllocClearMemory(
			fieldWork->heapID, sizeof(FGRID_CONT) );
	
	pGridCont->heapID = fieldWork->heapID;
	pGridCont->pFieldWork = fieldWork;
	
	{
		if( (dir>0x2000) && (dir<0x6000) ){
			dir = DIR_LEFT;
		}else if( (dir >= 0x6000) && (dir <= 0xa000) ){
			dir = DIR_DOWN;
		} else if( (dir > 0xa000)&&(dir < 0xe000)){
			dir = DIR_RIGHT;
		} else {
			dir = DIR_UP;
		}
		
		pGridCont->pGridPlayer = FGridPlayer_Init( pGridCont, pos, dir );
	}
	
#if 0	
	{
		u32 size;
		ARCHANDLE *arcH;
		arcH = GFL_ARC_OpenDataHandle(
			ARCID_FLDMAP_TESTGRIDMAP_ATTR, pGridCont->heapID );
		
		pGridCont->pMapAttr[0] = GFL_ARC_LoadDataAllocByHandle( arcH,
			NARC_testgridmap_a_map14_07a_dat, pGridCont->heapID );
		pGridCont->pMapAttr[1] = GFL_ARC_LoadDataAllocByHandle( arcH,
			NARC_testgridmap_a_map15_07a_dat, pGridCont->heapID );
		pGridCont->pMapAttr[2] = GFL_ARC_LoadDataAllocByHandle( arcH,
			NARC_testgridmap_a_map14_08a_dat, pGridCont->heapID );
		pGridCont->pMapAttr[3] = GFL_ARC_LoadDataAllocByHandle( arcH,
			NARC_testgridmap_a_map15_08a_dat, pGridCont->heapID );
		
		GFL_ARC_CloseDataHandle( arcH );
	}
#endif
	
	{	//グリッド用位置情報更新
		MAP_MATRIX *mat = fieldWork->pMapMatrix;
		GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
		const LOCATION *s_lc = GAMEDATA_GetStartLocation( gdata );
		
		pGridCont->location = *s_lc;
		pGridCont->location.zone_id =
			MAP_MATRIX_GetVectorPosZoneID( mat, s_lc->pos.x, s_lc->pos.z );
	}
	
	return( pGridCont );
}

//--------------------------------------------------------------
/**
 * グリッド処理　削除
 * @param	fieldWork	FIELD_MAIN_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FGridCont_Delete( FIELD_MAIN_WORK *fieldWork )
{
	FGRID_CONT *pGridCont;
	pGridCont = fieldWork->pGridCont;
#if 0	
	GFL_HEAP_FreeMemory( pGridCont->pMapAttr[0] );
	GFL_HEAP_FreeMemory( pGridCont->pMapAttr[1] );
	GFL_HEAP_FreeMemory( pGridCont->pMapAttr[2] );
	GFL_HEAP_FreeMemory( pGridCont->pMapAttr[3] );
#endif	
	FGridPlayer_Delete( pGridCont );
	
	GFL_HEAP_FreeMemory( pGridCont );
	fieldWork->pGridCont = NULL;
}

//======================================================================
//	グリッド　自機
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド自機　初期化
 * @param	pGridCont	FGRID_CONT
 * @retval	FGRID_PLAYER*
 */
//--------------------------------------------------------------
static FGRID_PLAYER * FGridPlayer_Init(
	FGRID_CONT *pGridCont, const VecFx32 *org_pos, u16 dir )
{
	VecFx32 pos;
	int gx,gy,gz;
	FGRID_PLAYER *pJiki;
	
	pJiki = GFL_HEAP_AllocClearMemory(
			pGridCont->heapID, sizeof(FGRID_PLAYER) );
	pGridCont->pGridPlayer = pJiki;
	
	pJiki->pGridCont = pGridCont;
	pJiki->pActCont = pGridCont->pFieldWork->pcActCont;
	
	pJiki->dir = dir;
	pJiki->move_dir = dir;

	//グリッド単位に直す
	gx = SIZE_GRID_FX32( org_pos->x );
	gy = SIZE_GRID_FX32( org_pos->y );
	gz = SIZE_GRID_FX32( org_pos->z );
	pos.x = GRID_SIZE_FX32( gx ) + GRID_VALUE_HALF_FX32;
	pos.y = GRID_SIZE_FX32( gy );
	pos.z = GRID_SIZE_FX32( gz ) + GRID_VALUE_HALF_FX32;
	
	pJiki->vec_pos = pos;
	pJiki->scale_size = FX16_ONE*8-1;
	
	SetGridPlayerActTrans( pJiki->pActCont, &pJiki->vec_pos );
		
	pJiki->anime_flag = PLAYER_ANIME_FLAG_STOP;
	return( pJiki );
}

//--------------------------------------------------------------
/**
 * グリッド自機　削除
 * @param	pGridCont	FGRID_CONT
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FGridPlayer_Delete( FGRID_CONT *pGridCont )
{
	FGRID_PLAYER *pJiki = pGridCont->pGridPlayer;
	GFL_HEAP_FreeMemory( pJiki );
	pGridCont->pGridPlayer = NULL;
}

///デバッグプリント
static void DEBUG_FIELDMAP_GRID_Print( FGRID_PLAYER *pJiki )
{
	const FGRID_CONT *pGridCont = pJiki->pGridCont;
	FIELD_MAIN_WORK *pFieldMainWork =
		(FIELD_MAIN_WORK *)pGridCont->pFieldWork;
	MAP_MATRIX *pMatrix = pFieldMainWork->pMapMatrix;
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( pFieldMainWork->gsys );
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
	const VecFx32 *pPos = PLAYERWORK_getPosition( player );
	int gx = SIZE_GRID_FX32( pPos->x );
	int gz = SIZE_GRID_FX32( pPos->z );
	int bx = gx / 32;
	int bz = gz / 32;
	u32 zone_id = MAP_MATRIX_GetBlockPosZoneID( pMatrix, bx, bz );
	OS_Printf( "BlockPos X %d Z %d, ゾーンID %x\n", bx, bz, zone_id );
}

#if 0
static void DEBUG_PrintAttr( FGRID_PLAYER *pJiki )
{
	const FLDMAPPER *mapper = 
		GetFieldG3Dmapper( pJiki->pGridCont->pFieldWork );
	fx32 now_x = pJiki->vec_pos.x;
	fx32 now_y = pJiki->vec_pos.y;
	fx32 now_z = pJiki->vec_pos.z;
	u32 now_gx = SIZE_GRID_FX32( now_x );
	u32 now_gy = SIZE_GRID_FX32( now_y );
	u32 now_gz = SIZE_GRID_FX32( now_z );
	int gx,gz;
	VecFx32 pos;
	
	OS_Printf(
		"アトリビュート 現在位置 X=%d(0x%xH),Y=%d(0x%xH),Z=%d(0x%xH)\n",
		now_gx, (now_x>>(4*3)),
		now_gy, (now_y>>(4*3)),
		now_gz, (now_z>>(4*3)) );
	
	for( gz = now_gz-1; gz < (now_gz+2); gz++ ){
		for( gx = now_gx-1; gx < (now_gx+2); gx++ ){
			u32 attr = 0;
		
			if( gx < 0 || gz < 0 ){
				OS_Printf( "OVER " );
			}else{
				pos.x = GRID_SIZE_FX32( gx );
				pos.y = now_y;
				pos.z = GRID_SIZE_FX32( gz );
			
				if( GetMapAttr(mapper,&pos,&attr) == TRUE ){
					OS_Printf( "%04d ", attr );
				}else{
					OS_Printf( "OVER  " ); 
				}
			}
		}
		OS_Printf( "\n" );
	}
}
#endif

//サウンドテスト
static void FGridPlayer_SountTest_Stop( void )
{
	//　↓ サウンドテスト（停止）
	u16 trackBit = 0xfcff; // track 9,10 OFF
	PMSND_ChangeBGMtrack(trackBit);
	//　↑
}
static void FGridPlayer_SountTest_Move( void )
{
	//　↓ サウンドテスト（移動）
	u16 trackBit = 0xffff; // 全track ON
	PMSND_ChangeBGMtrack(trackBit);
	//　↑
}

static PLAYER_SET FGridPlayer_CheckMoveStart_Walk(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

//--------------------------------------------------------------
/**
 * グリッド自機　移動開始チェック　停止中
 * @param
 * @retval
 */
//--------------------------------------------------------------
static PLAYER_SET FGridPlayer_CheckMoveStart_Stop(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( FLDMMDL_CheckPossibleAcmd(fmmdl) == TRUE ){
		if( dir != DIR_NOT ){
			u16 old_dir;
			old_dir = FLDMMDL_GetDirDisp( fmmdl );
			
			if( dir != old_dir && debug_flag == FALSE ){
				return( PLAYER_SET_TURN );
			}
			
			return( FGridPlayer_CheckMoveStart_Walk(
				pJiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
		}
		
		return( PLAYER_SET_STOP );
	}
	
	return( PLAYER_SET_NON );
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動開始チェック　移動中
 * @param
 * @retval
 */
//--------------------------------------------------------------
static PLAYER_SET FGridPlayer_CheckMoveStart_Walk(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( FLDMMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( FGridPlayer_CheckMoveStart_Stop(
			pJiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	{
		u32 hit = FLDMMDL_HitCheckMoveDir( fmmdl, dir );
		
		if( debug_flag == TRUE ){
			if( hit != FLDMMDL_MOVEHITBIT_NON &&
				!(hit&FLDMMDL_MOVEHITBIT_OUTRANGE) ){
				hit = FLDMMDL_MOVEHITBIT_NON;
			}
		}
		
		if( hit == FLDMMDL_MOVEHITBIT_NON ){
			return( PLAYER_SET_WALK );
		}
	}

	return( PLAYER_SET_HITCH );
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動開始チェック　振り向き中
 * @param
 * @retval
 */
//--------------------------------------------------------------
static PLAYER_SET FGridPlayer_CheckMoveStart_Turn(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( FLDMMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( FGridPlayer_CheckMoveStart_Stop(
			pJiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( FGridPlayer_CheckMoveStart_Walk(
		pJiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動開始チェック　障害物ヒット中
 * @param
 * @retval
 */
//--------------------------------------------------------------
static PLAYER_SET FGridPlayer_CheckMoveStart_Hitch(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( FLDMMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( FGridPlayer_CheckMoveStart_Stop(
			pJiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( FGridPlayer_CheckMoveStart_Walk(
		pJiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動セット　特になし
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FGridPlayer_SetMove_Non(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動セット　停止
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FGridPlayer_SetMove_Stop(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	if( dir == DIR_NOT ){
		dir = FLDMMDL_GetDirDisp( fmmdl );
	}
	
	code = FLDMMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
	FLDMMDL_SetAcmd( fmmdl, code );
	pJiki->move_state = PLAYER_MOVE_STOP;
	
	FGridPlayer_SountTest_Stop();
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動セット　移動
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FGridPlayer_SetMove_Walk(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	if( debug_flag == TRUE ){
		code = AC_WALK_U_2F;
	}else if( key_cont & PAD_BUTTON_B ){
		code = AC_DASH_U_4F;
	}else{
		code = AC_WALK_U_8F;
	}
	
	if( key_cont & PAD_BUTTON_A ){ //kari
		code = AC_JUMP_U_2G_16F;
	}
	
	code = FLDMMDL_ChangeDirAcmdCode( dir, code );
	
	FLDMMDL_SetAcmd( fmmdl, code );
	pJiki->move_state = PLAYER_MOVE_WALK;
	
	FGridPlayer_SountTest_Move();
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動セット　振り向き
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FGridPlayer_SetMove_Turn(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = FLDMMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_8F );
	
	FLDMMDL_SetAcmd( fmmdl, code );
	pJiki->move_state = PLAYER_MOVE_TURN;
	
	FGridPlayer_SountTest_Stop();
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動セット　障害物ヒット
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FGridPlayer_SetMove_Hitch(
	FGRID_PLAYER *pJiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = FLDMMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_32F );
	
	FLDMMDL_SetAcmd( fmmdl, code );
	pJiki->move_state = PLAYER_MOVE_HITCH;
	
	FGridPlayer_SountTest_Stop();
}

//--------------------------------------------------------------
/**
 * グリッド自機　移動処理
 * @param	pJiki	FGRID_PLAYER
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FGridPlayer_Move(
	FGRID_PLAYER *pJiki, u32 key_trg, u32 key_cont )
{
	u16 dir;
	BOOL debug_flag;
	PLAYER_SET set;
	PC_ACTCONT *pcActCont = pJiki->pGridCont->pFieldWork->pcActCont;
	FLDMMDL *fmmdl = Player_GetFldMMdl( pcActCont );

	#ifdef DEBUG_ONLY_FOR_kagaya
	if( (GFL_UI_KEY_GetTrg()&PAD_BUTTON_Y) ){
		DEBUG_FIELDMAP_GRID_Print( pJiki );
	}
	#endif
	
	dir = DIR_NOT;
	if( (key_cont&PAD_KEY_UP) ){
		dir = DIR_UP;
	}else if( (key_cont&PAD_KEY_DOWN) ){
		dir = DIR_DOWN;
	}else if( (key_cont&PAD_KEY_LEFT) ){
		dir = DIR_LEFT;
	}else if( (key_cont&PAD_KEY_RIGHT) ){
		dir = DIR_RIGHT;
	}
	
	debug_flag = FALSE;
	if( key_cont & PAD_BUTTON_R ){
		debug_flag = TRUE;
	}
	
	set = PLAYER_SET_NON;
	switch( pJiki->move_state ){
	case PLAYER_MOVE_STOP:
		set = FGridPlayer_CheckMoveStart_Stop(
			pJiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_WALK:
		set = FGridPlayer_CheckMoveStart_Walk(
			pJiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_TURN:
		set = FGridPlayer_CheckMoveStart_Turn(
			pJiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_HITCH:
		set = FGridPlayer_CheckMoveStart_Hitch(
			pJiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	default:
		GF_ASSERT( 0 );
	}
	
	switch( set ){
	case PLAYER_SET_NON:
		FGridPlayer_SetMove_Non(
			pJiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_STOP:
		FGridPlayer_SetMove_Stop(
			pJiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_WALK:
		FGridPlayer_SetMove_Walk(
			pJiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_TURN:
		FGridPlayer_SetMove_Turn(
			pJiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_HITCH:
		FGridPlayer_SetMove_Hitch(
			pJiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	}
}

#if 0
	if( FLDMMDL_CheckPossibleAcmd(fmmdl) == TRUE ){
		int code;
		int dir = DIR_NOT;
	
		if( dir == DIR_NOT ){
		
		}

		else{

		}
		
		if( dir == DIR_NOT ){
			//　↓ サウンドテスト（停止）
			{
				u16 trackBit = 0xfcff; // track 9,10 OFF
				PMSND_ChangeBGMtrack(trackBit);
			}
			//　↑
			dir = FLDMMDL_GetDirDisp( fmmdl );
			code = FLDMMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
		}else{
			u32 hit;
			hit = FLDMMDL_HitCheckMoveDir( fmmdl, dir );
			
			//--debug移動チェック
			if( hit != FLDMMDL_MOVEHITBIT_NON &&
				(key_cont&PAD_BUTTON_R) &&
				!(hit&FLDMMDL_MOVEHITBIT_OUTRANGE) ){
				hit = FLDMMDL_MOVEHITBIT_NON;
			}
			//--
			
			if( hit == FLDMMDL_MOVEHITBIT_NON ){
				//　↓ サウンドテスト（移動）
				{
					u16 trackBit = 0xffff; // 全track ON
					PMSND_ChangeBGMtrack(trackBit);
				}
				//　↑
				#if 1 //debug移動
				if( (key_cont&PAD_BUTTON_R) ){
					code = FLDMMDL_ChangeDirAcmdCode( dir, AC_WALK_U_2F );
				}else{
					if( (key_cont & PAD_BUTTON_B) ){
						code = FLDMMDL_ChangeDirAcmdCode( dir, AC_DASH_U_4F );
					}else{
						code = FLDMMDL_ChangeDirAcmdCode( dir, AC_WALK_U_16F );
					}
				}
				#else
				if( (key_cont & PAD_BUTTON_B) ){
					code = FLDMMDL_ChangeDirAcmdCode( dir, AC_DASH_U_4F );
				}else{
					code = FLDMMDL_ChangeDirAcmdCode( dir, AC_WALK_U_8F );
				}
				#endif
			}else{
				code = FLDMMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_8F );
			}
		}
		
		FLDMMDL_SetAcmd( fmmdl, code );
	}
	
	#if 0
	FldWorkPlayerWorkDirSet(
		pJiki->pGridCont->pFieldWork, pJiki->dir );
	FldWorkPlayerWorkPosSet(
		pJiki->pGridCont->pFieldWork, &pJiki->vec_pos );
	#endif
}
#endif

#if 0
static void FGridPlayer_Move(
	FGRID_PLAYER *pJiki, u32 key_trg, u32 key_cont )
{
	PC_ACTCONT *pcActCont = pJiki->pGridCont->pFieldWork->pcActCont;
	
	//----とりあえず
	#ifdef DEBUG_ONLY_FOR_kagaya
	if( (GFL_UI_KEY_GetTrg()&PAD_BUTTON_Y) ){
		DEBUG_PrintAttr( pJiki );
	}
	#elif defined DEBUG_ONLY_FOR_nakatsui
	if( (GFL_UI_KEY_GetTrg()&PAD_BUTTON_Y) ){
		DEBUG_PrintAttr( pJiki );
	}
	#endif
	//----
	
	if( pJiki->move_flag == FALSE && (key_cont&PAD_KEY_BIT) ){
		int dir = DIR_NOT;
		
		if( (key_cont&PAD_KEY_UP) ){ dir = DIR_UP; }
		else if( (key_cont&PAD_KEY_DOWN) ){ dir = DIR_DOWN; }
		else if( (key_cont&PAD_KEY_LEFT) ){ dir = DIR_LEFT; }
		else if( (key_cont&PAD_KEY_RIGHT) ){ dir = DIR_RIGHT; }
		
		if( dir != DIR_NOT ){
			MAPHITBIT hit = MAPHITBIT_NON;
			fx32 val = GRID_VALUE_FX32;
			VecFx32 next = pJiki->vec_pos;

			if( pJiki->pGridCont->scale_num != 0 ){
				val >>= 1;
			}
			
			switch( dir ){
			case DIR_UP:	next.z -= val; break;
			case DIR_DOWN:	next.z += val; break;
			case DIR_LEFT:	next.x -= val; break;
			case DIR_RIGHT:	next.x += val; break;
			}

			#if 0
			hit = MapHitCheck( pJiki->pGridCont, &pJiki->vec_pos, &next );
			#else
			{
				FLDMMDL *fmmdl = Player_GetFldMMdl( pcActCont );
				int next_gx = SIZE_GRID_FX32( next.x );
				int next_gy = SIZE_GRID_FX32( next.y );
				int next_gz = SIZE_GRID_FX32( next.z );
				
				if( FLDMMDL_HitCheckMoveDir(fmmdl,dir) !=
					FLDMMDL_MOVEHITBIT_NON ){
					hit = 1;
				}
			}
			#endif
			if( hit == MAPHITBIT_NON ||
				((key_cont&PAD_BUTTON_R) && !(hit&MAPHITBIT_MASK_ERROR)) ){
				pJiki->move_flag = TRUE;
				pJiki->dir = dir;
				pJiki->move_dir = dir;
				pJiki->anime_flag = PLAYER_ANIME_FLAG_WALK;

				switch( dir ){
				case DIR_UP:	pJiki->move_val.z = NUM_FX32( -2 ); break;
				case DIR_DOWN:	pJiki->move_val.z = NUM_FX32( 2 ); break;
				case DIR_LEFT:	pJiki->move_val.x = NUM_FX32( -2 ); break;
				case DIR_RIGHT: pJiki->move_val.x = NUM_FX32( 2 ); break;
				}
				
				if( pJiki->pGridCont->scale_num != 0 ){
					pJiki->move_val.x >>= 1;
					pJiki->move_val.y >>= 1;
					pJiki->move_val.z >>= 1;
				}
				
				if( (key_cont & PAD_BUTTON_B) ){
					pJiki->move_val.x <<= 1;
					pJiki->move_val.y <<= 1;
					pJiki->move_val.z <<= 1;
					pJiki->anime_flag = PLAYER_ANIME_FLAG_RUN;
				}else if( (key_cont & PAD_BUTTON_R) ){
					pJiki->move_val.x <<= 2;
					pJiki->move_val.y <<= 2;
					pJiki->move_val.z <<= 2;
				}
			}else{	//障害物ヒット
				pJiki->dir = dir;
				pJiki->move_dir = dir;
				pJiki->anime_flag = PLAYER_ANIME_FLAG_STOP;
			}
		}
	}
	
	{
		int dir = 0;
		switch( pJiki->dir ){
		case DIR_UP:	dir = 0; break;
		case DIR_DOWN:	dir = 1; break;
		case DIR_LEFT:	dir = 2; break;
		case DIR_RIGHT:	dir = 3; break;
		}
		PlayerActGrid_AnimeSet( pcActCont, dir, pJiki->anime_flag );
		
		FldWorkPlayerWorkDirSet(
			pJiki->pGridCont->pFieldWork, pJiki->dir );
	}
	
	if( pJiki->move_flag == TRUE ){
		fx32 count = 0;
#if 0
		pJiki->vec_pos.x += pJiki->move_val.x;
		pJiki->vec_pos.y += pJiki->move_val.y;
		pJiki->vec_pos.z += pJiki->move_val.z;
#else
		GridVecPosMove(
			GetFieldG3Dmapper(pJiki->pGridCont->pFieldWork),
			PlayerActGrid_GridInfoGet(pcActCont),
			&pJiki->vec_pos, &pJiki->move_val );
#endif
		pJiki->move_count.x += pJiki->move_val.x;
		pJiki->move_count.y += pJiki->move_val.y;
		pJiki->move_count.z += pJiki->move_val.z;
		
		PlayerActGrid_Update( pcActCont, pJiki->dir, &pJiki->vec_pos );
		FldWorkPlayerWorkPosSet(
			pJiki->pGridCont->pFieldWork, &pJiki->vec_pos );
		
		switch( pJiki->move_dir ){
		case DIR_UP:
		case DIR_DOWN: count = pJiki->move_count.z; break;
		case DIR_LEFT:
		case DIR_RIGHT: count = pJiki->move_count.x; break;
		}
		
		if( count < 0 ){
			count = -count;
		}
		
//		OS_Printf( "count=%x, MAX %x\n", count, GRID_VALUE_FX32 );
		
		if( pJiki->pGridCont->scale_num != 0 ){
			count <<= 1;
		}

		if( count >= GRID_VALUE_FX32 ){
			pJiki->move_flag = FALSE;
			pJiki->move_dir = DIR_NOT;
			pJiki->move_val.x = 0;
			pJiki->move_val.y = 0;
			pJiki->move_val.z = 0;
			pJiki->move_count.x = 0;
			pJiki->move_count.y = 0;
			pJiki->move_count.z = 0;
			pJiki->anime_flag = PLAYER_ANIME_FLAG_STOP;
		}
	}
	
	#if 0
	if( (GFL_UI_KEY_GetTrg()&PAD_BUTTON_X) ){
		fx32 x = pJiki->vec_pos.x;
		fx32 z = pJiki->vec_pos.z;
		
		OS_Printf(
			"自機情報 GridX %d(%d,%xH), GridZ %d(%d,%xH), Y=%xH",
			SIZE_GRID_FX32(x), x, x,
			SIZE_GRID_FX32(z), z, z,
			pJiki->vec_pos.y );
		
		if( MapHitCheck(pJiki->pGridCont,x,z) == TRUE ){
			OS_Printf( "位置障害物\n" );
		}else{
			OS_Printf( "\n" );
		}
	}
	#endif
}
#endif

//--------------------------------------------------------------
/**
 * グリッド自機 スケール変更
 * @param	pJiki	FGRID_PLAYER
 * @param	size
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FGridPlayer_ScaleSet( FGRID_PLAYER *pJiki, fx16 scale )
{
	pJiki->scale_size = scale;
	PlayerActGrid_ScaleSizeSet( pJiki->pActCont, scale, scale );
}

//--------------------------------------------------------------
/**
 * グリッド自機 スケール取得
 * @param	pJiki	FGRID_PLAYER
 * @param	size
 * @retval	nothing
 */
//--------------------------------------------------------------
static fx16 FGridPlayer_ScaleGet( FGRID_PLAYER *pJiki )
{
	return( pJiki->scale_size );
}

//======================================================================
//	parts
//======================================================================
//--------------------------------------------------------------
/**
 * 指定座標を動かす
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL GridVecPosMove(
	const FLDMAPPER *g3Dmapper,
	FLDMAPPER_GRIDINFODATA *gridInfoData,
	VecFx32 *pos, VecFx32 *vecMove )
{
	fx32	height = 0;
	BOOL	initSw = FALSE;
	FLDMAPPER_GRIDINFO gridInfo;
	VecFx32	posNext, vecGround;
	
	VEC_Add( pos, vecMove, &posNext );
	
	if( posNext.y < 0 ){
		posNext.y = 0;	//ベースライン
	}
	
	if( FLDMAPPER_CheckOutRange(g3Dmapper,&posNext) == TRUE ){
		return FALSE;	//範囲外
	}
	
	if( FLDMAPPER_GetGridInfo(g3Dmapper,&posNext,&gridInfo) == FALSE ){
		return FALSE;	//マップデータが取得できない
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
		
		*gridInfoData = gridInfo.gridData[p];	//グリッドデータ更新
		VEC_Set( pos, posNext.x, gridInfoData->height, posNext.z );
	}
	
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * アトリビュート取得
 * @param
 * @retval	BOOL	TRUE=正常に取得出来た
 */
//--------------------------------------------------------------
static BOOL GetMapAttr(
	const FLDMAPPER *g3Dmapper, const VecFx32 *pos, u32 *attr )
{
	FLDMAPPER_GRIDINFO gridInfo;
	
	if( FLDMAPPER_GetGridInfo(g3Dmapper,pos,&gridInfo) == TRUE ){
		*attr = gridInfo.gridData[0].attr;
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * 高さ取得
 * @param
 * @retval	fx32	高さ
 */
//--------------------------------------------------------------
static fx32 GetMapHeight(
	const FLDMAPPER *g3Dmapper, const VecFx32 *pos )
{
	FLDMAPPER_GRIDINFO gridInfo;
	
	if( FLDMAPPER_GetGridInfo(g3Dmapper,pos,&gridInfo) == TRUE ){
		if( gridInfo.count ){
			int		i = 0,p = 0;
			fx32	h_tmp,diff1,diff2;
			fx32	height = 0;
			
			height = gridInfo.gridData[i].height;
			i++;
			
			for( ; i < gridInfo.count; i++ ){
				h_tmp = gridInfo.gridData[i].height;
				diff1 = height - pos->y;
				diff2 = h_tmp - pos->y;
				
				if( FX_Mul( diff2, diff2 ) < FX_Mul( diff1, diff1 ) ){
					height = h_tmp;
					p = i;
				}
			}

			return( height );
		}
	}	
	
	return( 0 );
}

//--------------------------------------------------------------
/**
 * マップヒットチェック
 * @param
 * @retval	
 */
//--------------------------------------------------------------
static MAPHITBIT MapHitCheck(
	const FGRID_CONT *pGridCont, const VecFx32 *now, const VecFx32 *next )
{
	u32 hit;
	VecFx32 pos;
	const FLDMAPPER *mapper;
	
	hit = MAPHITBIT_NON;
	mapper = GetFieldG3Dmapper( pGridCont->pFieldWork );
	
	if( FLDMAPPER_CheckOutRange(mapper,next) == TRUE ){
		hit |= MAPHITBIT_OUT;
		return( hit );
	}
	
	{
		u32 attr;
		fx32 height,diff;
		
		if( GetMapAttr(mapper,next,&attr) == FALSE ){
			hit |= MAPHITBIT_DATA;
		}
		
		if( attr != 0 ){
			hit |= MAPHITBIT_ATTR;
		}
		
		height = GetMapHeight( mapper, next );
		diff = now->y - height;
		
		if( diff < 0 ){
			diff = -diff;
		}
		
		if( diff >= MAPHIT_HEIGHT_OVER ){
			hit |= MAPHITBIT_HEIGHT;
		}
	}
	
	return( hit );
}

//--------------------------------------------------------------
/**
 * メインをカメラいじりに
 * @param
 * @retval
 */
//--------------------------------------------------------------
void DEBUG_FldGridProc_Camera( FIELD_MAIN_WORK *fieldWork )
{
	FGRID_CONT *pGridCont = fieldWork->pGridCont;
	pGridCont->proc_switch = GRIDPROC_DEBUG00;
	TAMADA_Printf("switch to GRIDPROC_DEBUG00\n");
}

//--------------------------------------------------------------
/**
 * スケールきりかえ
 * @param
 * @retval
 */
//--------------------------------------------------------------
void DEBUG_FldGridProc_ScaleChange( FIELD_MAIN_WORK *fieldWork )
{
	FGRID_CONT *pGridCont = fieldWork->pGridCont;
	FGRID_PLAYER *pJiki = pGridCont->pGridPlayer;

	pGridCont->scale_num++;
	pGridCont->scale_num %= 2;
	
	switch( pGridCont->scale_num ){
	case 0:
		{
			fx16 size = FX16_ONE * 8 - 1;
			FGridPlayer_ScaleSet( pJiki, size );
			FIELD_CAMERA_SetLengthOnXZ( fieldWork->camera_control,
					DATA_CameraTbl[pGridCont->debug_camera_num].len );
			FIELD_CAMERA_SetHeightOnXZ( fieldWork->camera_control,
					DATA_CameraTbl[pGridCont->debug_camera_num].hi );
		}
		break;
	case 1:
		{
			fx16 size = FX16_ONE * 4 - 1;
			FGridPlayer_ScaleSet( pJiki, size );
			FIELD_CAMERA_SetLengthOnXZ(
				fieldWork->camera_control, GRIDCAMERA_LENGTH_HALF );
			FIELD_CAMERA_SetHeightOnXZ(
				fieldWork->camera_control, GRIDCAMERA_HEIGHT_HALF );
		}
		break;
	}
}

//--------------------------------------------------------------
/**
 * スケール調節
 * @param
 * @retval
 */
//--------------------------------------------------------------
void DEBUG_FldGridProc_ScaleControl( FIELD_MAIN_WORK *fieldWork )
{
	fx16 scale;
	FGRID_CONT *pGridCont = fieldWork->pGridCont;
	pGridCont->proc_switch = GRIDPROC_DEBUG01;
	TAMADA_Printf("switch to GRIDPROC_DEBUG01\n");
}

//--------------------------------------------------------------
/**
 * 自機の方向を返す
 * @param	fieldWork	FIELD_MAIN_WORK
 * @retval	u16			DIR_UP等
 */
//--------------------------------------------------------------
u16 FieldMainGrid_GetPlayerDir( const FIELD_MAIN_WORK *fieldWork )
{
	const FGRID_CONT *pGridCont = fieldWork->pGridCont;
	
	if( pGridCont != NULL ){
		const FGRID_PLAYER *pJiki = pGridCont->pGridPlayer;
		return( pJiki->dir );
	}

	return( DIR_NOT );
}

//--------------------------------------------------------------
/**
 * 座標をPLAYER_WORKに反映する
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FldWorkPlayerWorkPosSet(
		FIELD_MAIN_WORK *fieldWork, const VecFx32 *pos )
{
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
	PLAYERWORK_setPosition( player, pos );
}

//--------------------------------------------------------------
/**
 * 方向をPLAYER_WORKに反映する
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void FldWorkPlayerWorkDirSet(
	FIELD_MAIN_WORK *fieldWork, u16 dir )
{
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
	u16 tbl[DIR_MAX4] = { 0x0000, 0x8000, 0x4000, 0xc000 };
	GF_ASSERT( dir < DIR_MAX4 );
	PLAYERWORK_setDirection( player, tbl[dir] );
}

static void Jiki_UpdatePlayerWork( FIELD_MAIN_WORK *fieldWork )
{
	VecFx32 pos;
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
	FLDMMDL *fmmdl = Player_GetFldMMdl( fieldWork->pcActCont );
	u16 tbl[DIR_MAX4] = { 0x0000, 0x8000, 0x4000, 0xc000 };
	int dir = FLDMMDL_GetDirDisp( fmmdl );
	
	FLDMMDL_GetVectorPos( fmmdl, &pos );

	PLAYERWORK_setDirection( player, tbl[dir] );
	PLAYERWORK_setPosition( player, &pos );
	SetPlayerActDirection( fieldWork->pcActCont, &tbl[dir] );
	SetGridPlayerActTrans( fieldWork->pcActCont, &pos );
}

#if 0	//実データ利用
static BOOL MapHitCheck( const FGRID_CONT *pGridCont, fx32 x, fx32 z )
{
	u16 attr;
	const u16 *buf = NULL;
	int gx = SIZE_GRID_FX32(x), gz = SIZE_GRID_FX32(z);
	
	if( gx < 0x20 ){
		if( gz < 0x20 ){
			buf = pGridCont->pMapAttr[0];
		}else if( gz < 0x40 ){
			gz -= 0x20;
			buf = pGridCont->pMapAttr[2];
		}
	}else if( gx < 0x40 ){
		gx -= 0x20;
		if( gz < 0x20 ){
			buf = pGridCont->pMapAttr[1];
		}else if( gz < 0x40 ){
			gz -= 0x20;
			buf = pGridCont->pMapAttr[3];
		}
	}
	
	if( buf == NULL ){
		return( FALSE );
	}
	
	attr = buf[gx+(gz*0x20)];
	
	if( ((attr&0x8000)>>15) ){
		return( TRUE );
	}
	
	return( FALSE );
}
#endif

//======================================================================
//======================================================================
//======================================================================
//======================================================================
#if 0
//--------------------------------------------------------------
//	アクター　コントロールセットアップ
//--------------------------------------------------------------
static FGRID_ACTCONT * GridAct_SetupCont(
	FLD_ACTCONT *fldActCont, u32 heapID, int max )
{
	FGRID_ACTCONT *pGridActCont;
	
	pGridActCont = GFL_HEAP_AllocClearMemory(
			heapID, sizeof(FGRID_ACTCONT) );
	
	pGridActCont->max = max;
	pGridActCont->heapID = heapID;
	
	pGridActCont->pGridActTbl = GFL_HEAP_AllocClearMemory(
			heapID, sizeof(FGRID_ACTOR)*max );
	
	pGridActCont->pFldActCont = fldActCont;
	
	GridAct_SetupBBD( fldActCont, max );
	return( pActCont );
}
#endif

#if 0
//--------------------------------------------------------------
// アクター	ビルボードシステムセットアップ
//--------------------------------------------------------------
static void GridAct_SetupBBD( FLD_ACTCONT *fldActCont, int max )
{
	GFL_BBDACT_SYS* bbdActSys = GetBbdActSys( fldActCont->fieldWork );	
	GFL_BBDACT_ACTDATA* actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	int		i, objIdx;
	VecFx32	trans;
	u8		alpha;
	BOOL	drawEnable;
	u16		setActNum;
	
	//リソースセットアップ
	fldActCont->bbdActResCount = NELEMS( testResTable );
	fldActCont->bbdActResUnitID = GFL_BBDACT_AddResourceUnit(
				bbdActSys, 
				testResTable, 
				fldActCont->bbdActResCount );
	
	fldActCont->bbdActActCount = FLD_BBDACT_ACTMAX;
}
#endif

#if 0
//--------------------------------------------------------------
///	アクター　アクター領域作成
//--------------------------------------------------------------
static void GridAct_ActWorkCreate( 

	//ＮＰＣアクターセットアップ
	{
		u16	setActNum = TEST_NPC_SETNUM;
		
		GFL_BBDACT_ACTDATA *actData = GFL_HEAP_AllocClearMemory(
				fldActCont->heapID,
				setActNum *s izeof(GFL_BBDACT_ACTDATA) );
		fx32 mapSizex, mapSizez;

		FLDMAPPER_GetSize(
			GetFieldG3Dmapper( fldActCont->fieldWork ), &mapSizex, &mapSizez );
		
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
#endif

//======================================================================
//	FLDMMDL NPC配置テスト
//======================================================================
//--------------------------------------------------------------
///	
//--------------------------------------------------------------
static const FLDMMDL_HEADER DATA_NpcHeader =
{
	0,	///<識別ID
	1,	///<表示するOBJコード
	MV_RND,	///<動作コード
	0,	///<イベントタイプ
	0,	///<イベントフラグ
	0,	///<イベントID
	0,	///<指定方向
	0,	///<指定パラメタ 0
	0,	///<指定パラメタ 1
	0,	///<指定パラメタ 2
	16,	///<X方向移動制限
	16,	///<Z方向移動制限
	90,	///<グリッドX
	86,	///<グリッドZ
	0,	///<Y値 fx32型
};

#define NPC_MAX (250)

//--------------------------------------------------------------
/**
 * npc配置テスト
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void GridMap_SetupNPC( FIELD_MAIN_WORK *fieldWork )
{
#if 0
	u32 attr;
	int i,gx,gy,gz;
	FLDMMDL *fmmdl;
	FLDMMDL_HEADER head;
	VecFx32 pos = {0,0,0};
	const FLDMAPPER *mapper = GetFieldG3Dmapper( fieldWork );

	for( i = 0; i < NPC_MAX; i++ ){
		do{
			gx = GFUser_GetPublicRand( 128 );
			gz = GFUser_GetPublicRand( 128 );
			pos.x = GRID_SIZE_FX32(gx) + GRID_HALF_FX32;
			pos.z = GRID_SIZE_FX32(gz) + GRID_HALF_FX32;
			
			if( GetMapAttr(mapper,&pos,&attr) == TRUE ){
				if( attr == 0 ){
					int no;
					head = DATA_NpcHeader;
					head.id = i + 1;
					do{
						no = GFUser_GetPublicRand( TestOBJCodeMax );
						head.obj_code = TestOBJCodeTbl[no];
					}while( head.obj_code == HERO ||
							head.obj_code == NONDRAW );
					OS_Printf( "%d Code %d\n", i, head.obj_code );
					head.gx = gx;
					head.gz = gz;
					fmmdl = FLDMMDLSYS_AddFldMMdl(
						fieldWork->fldMMdlSys, &head, 0 );
					break;
				}
			}
		}while( 1 );
	}
#endif
}

//======================================================================
//
//======================================================================
//--------------------------------------------------------------
/**
 * 現在のフィールドシステムがグリッド処理かチェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL FIELDMAP_CheckGridControl( FIELD_MAIN_WORK *fieldWork )
{
	if( fieldWork->pGridCont == NULL ){
		return( FALSE );
	}
	return( TRUE );
}

//======================================================================
//	動作モデルリスト
//======================================================================
#if 0
static void MMdlList_Init( MMDL_LIST *mlist, int list_id, HEAPID heapID )
{
	int i = 0;
	u16 *pList;
	pList = GFL_ARC_LoadDataAlloc( ARCID_FLDMMDL_LIST, list_id, heapID );
	mlist->count = 0;
	
	while( pList[i] != OBJCODEMAX ){
		OS_Printf( "モデルリスト　No %d = %d\n", i, pList[i] );
		mlist->id_list[i] = pList[i];
		i++;
		GF_ASSERT( i < MMDL_LIST_MAX );
	}

	OS_Printf( "モデルリスト総数 %d\n", i );
	
	mlist->count = i;
	mlist->id_list[i] = OBJCODEMAX;
	GFL_HEAP_FreeMemory( pList );
}
#endif

//======================================================================
//	
//======================================================================
//--------------------------------------------------------------
/**
 * 自機座標更新チェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
//static BOOL gridmap_CheckPlayerPosUpdate(
//	FIELD_MAIN_WORK *pFieldMainWork, FGRID_CONT *pGridCont )
static BOOL gridmap_CheckPlayerPosUpdate( FIELD_MAIN_WORK *pFieldMainWork )
{
	LOCATION *lc = &pFieldMainWork->location;
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( pFieldMainWork->gsys );
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
	const VecFx32 *pos = PLAYERWORK_getPosition( player );
	
	if( pos->x != lc->pos.x || pos->z != lc->pos.z ){
		lc->pos = *pos;
		return( TRUE );
	}
	
	return( FALSE );
}


//--------------------------------------------------------------
/**
 * ゾーン切り替えチェック 
 * @param
 * @retval
 */
//--------------------------------------------------------------
//static BOOL gridmap_CheckMoveZoneChange(
//	FIELD_MAIN_WORK *pFieldMainWork, FGRID_CONT *pGridCont )
static BOOL gridmap_CheckMoveZoneChange( FIELD_MAIN_WORK *pFieldMainWork )
{
	LOCATION *lc = &pFieldMainWork->location;
	MAP_MATRIX *mat = pFieldMainWork->pMapMatrix;
	
	if( MAP_MATRIX_CheckVectorPosRange(mat,lc->pos.x,lc->pos.z) == TRUE ){
		u32 zone_id =
			MAP_MATRIX_GetVectorPosZoneID( mat, lc->pos.x, lc->pos.z );
		
		//現状 IDが無い場合は更新しない
		if( zone_id != MAP_MATRIX_ZONE_ID_NON ){
			if( zone_id != (u32)lc->zone_id ){
				return( TRUE );
			}
		}
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時の処理
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void gridmap_MoveZoneChange( FIELD_MAIN_WORK *pFieldMainWork )
{
	LOCATION *lc = &pFieldMainWork->location;
	FLDMMDLSYS *fmmdlsys = pFieldMainWork->fldMMdlSys;
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( pFieldMainWork->gsys );
	EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
	MAP_MATRIX *mat = pFieldMainWork->pMapMatrix;
	u32 new_zone_id = MAP_MATRIX_GetVectorPosZoneID(
			mat, lc->pos.x, lc->pos.z );
	
	{	//旧ゾーン配置動作モデル削除
		FLDMMDLSYS_DeleteZoneUpdateFldMMdl( fmmdlsys );
	}
	
	GF_ASSERT( new_zone_id != MAP_MATRIX_ZONE_ID_NON );

	{	//次のイベントデータをロード
		EVENTDATA_SYS_Load( evdata, new_zone_id );
	}
		
	{	//新規ゾーン配置動作モデル
		u16 count = EVENTDATA_GetNpcCount( evdata );
		
		if( count ){
			const FLDMMDL_HEADER *header = EVENTDATA_GetNpcData( evdata );
			FLDMMDLSYS_SetFldMMdl( fmmdlsys, header, new_zone_id, count );
		}
	}

	{	//BGM切り替え
		u16 trackBit = 0xfcff;	// track 9,10 OFF
		#if 0
		u16 nextBGM = ZONEDATA_GetBGMID(
				new_zone_id, GAMEDATA_GetSeasonID(gdata) );
		#else
		u16 nextBGM = 0;
		
		switch( new_zone_id ){
		case ZONE_ID_T01:
			nextBGM = SEQ_WB_T_01; break;
		case ZONE_ID_C01:
			nextBGM = SEQ_WB_TITLE; break;
		case ZONE_ID_R01:
			nextBGM = SEQ_WB_R_A_SP; break;
		case ZONE_ID_D01:
			nextBGM = SEQ_WB_SHINKA; break;
			break;
		default:
			break;
		}
		#endif
		PMSND_PlayNextBGM_EX( nextBGM, trackBit );
	}
		
	{	//天候リクエスト
		u32 w_no = WEATHER_NO_NUM;
		FIELD_WEATHER *we = FIELDMAP_GetFieldWeather( pFieldMainWork );
		
		switch( new_zone_id ){
		case ZONE_ID_T01:
			w_no = WEATHER_NO_SNOW;
			break;
		case ZONE_ID_R01:
			w_no = WEATHER_NO_RAIN;
			break;
		}

		if( w_no != WEATHER_NO_NUM ){
			FIELD_WEATHER_Change( we, w_no );
		}
	}
	
	{	//ゾーンID更新
		PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
		PLAYERWORK_setZoneID( player, new_zone_id );
		lc->zone_id = new_zone_id;
	}
	
	KAGAYA_Printf(
		"ゾーン更新完了 %d -> %d\n", lc->zone_id, new_zone_id );
}

//--------------------------------------------------------------
/**
 * ゾーン更新	固まり次第、fieldmap.cへ
 */
//--------------------------------------------------------------
static BOOL fieldmap_UpdateZone( FIELD_MAIN_WORK *fieldWork )
{
	if( gridmap_CheckPlayerPosUpdate(fieldWork) == TRUE ){
		if( gridmap_CheckMoveZoneChange(fieldWork) == TRUE ){
			gridmap_MoveZoneChange( fieldWork );
			return( TRUE );
		}
	}
	
	return( FALSE );
}
