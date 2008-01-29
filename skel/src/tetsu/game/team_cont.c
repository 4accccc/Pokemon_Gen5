//============================================================================================
/**
 * @file	team_cont.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"

#include "team_cont.h"
//============================================================================================
//
//
//	チームコントロール
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
#define CASTLE_EMPTY			(0)
#define SUMMON_EMPTY			(0)
#define PLAYER_EMPTY			(NULL)

typedef struct {
	u16			type;
	u16			hpNow;
	u16			hpMax;
	VecFx32		trans;
	int			objID;
}SUMMON_STATUS;

typedef struct {
	u16			type;
	u16			hpNow;
	u16			hpMax;
	VecFx32		trans;
	int			objID;
}CASTLE_STATUS;

struct _TEAM_CONTROL {
	HEAPID			heapID;
	GAME_SYSTEM*	gs;

	CASTLE_STATUS	castle;

	SUMMON_STATUS	summon[TEAM_SUMMON_COUNT_MAX];

	PLAYER_CONTROL*	player[TEAM_PLAYER_COUNT_MAX];
	int				playerCount;

	GFL_BMP_DATA*	mapArea;
	BOOL			mapAreaDrawFlag;
	u32				mapAreaScore;
};

typedef struct {
	u16			type;
	u16			hp;
	u16			areaIconType;
	u16			radius;
}SUMMON_TYPETABLE;

#define MAPAREA_SIZX	(16)	//キャラ単位
#define MAPAREA_SIZY	(16)	//キャラ単位

#define MAPAREAMASK_COL	(3)//(PLT_2D_COL_BLACK)
					

static void CalcCastleDamage( TEAM_CONTROL* gc, int* damage );
static const SUMMON_TYPETABLE	summonTypeTable[9];

//------------------------------------------------------------------
/**
 * @brief	チームコントロール起動
 */
//------------------------------------------------------------------
TEAM_CONTROL* AddTeamControl( GAME_SYSTEM* gs, HEAPID heapID )
{
	int i;
	TEAM_CONTROL* tc = GFL_HEAP_AllocClearMemory( heapID, sizeof(TEAM_CONTROL) );
	tc->heapID = heapID;
	tc->gs = gs;
	tc->playerCount = 0;

	tc->castle.type = CASTLE_EMPTY;

	for( i=0; i<TEAM_SUMMON_COUNT_MAX; i++ ){ 
		tc->summon[i].type = SUMMON_EMPTY;
	}

	for( i=0; i<TEAM_PLAYER_COUNT_MAX; i++ ){ 
		tc->player[i] = PLAYER_EMPTY;
	}
	tc->mapArea = GFL_BMP_Create( MAPAREA_SIZX, MAPAREA_SIZY, GFL_BMP_16_COLOR, heapID );
	GFL_BMP_Clear( tc->mapArea, MAPAREAMASK_COL );
#if 1
	//背景色パレット作成＆転送
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( heapID, 16*2 );
		plt[2] = 0x07c0;
		plt[3] = 0x05e0;
		GFL_BG_LoadPalette( GFL_BG_FRAME0_S, plt, 8*2, 16*2 );	//サブ画面の背景色転送
		GFL_HEAP_FreeMemory( plt );
	}
#endif
	tc->mapAreaDrawFlag = TRUE;
	return tc;
}

//------------------------------------------------------------------
/**
 * @brief	チームコントロール終了
 */
//------------------------------------------------------------------
void RemoveTeamControl( TEAM_CONTROL* tc )
{
	int i;

	GFL_BMP_Delete( tc->mapArea );
	
	for( i=0; i<TEAM_PLAYER_COUNT_MAX; i++ ){ 
		ReleasePlayer( tc, i );
	}
	GFL_HEAP_FreeMemory( tc ); 
}

//------------------------------------------------------------------
/**
 * @brief	チームメインコントロール
 */
//------------------------------------------------------------------
void MainTeamControl( TEAM_CONTROL* tc, BOOL onGameFlag )
{
	int i, j;
#if 0
	//プレーヤー動作
	{
		PLAYER_CONTROL* pc;
		SUMMON_STATUS*	ss;
		PLAYER_BUILD_COMMAND buildCommand;
		VecFx32 setTrans;
		for( i=0; i<TEAM_PLAYER_COUNT_MAX; i++ ){ 
			if( tc->player[i] != PLAYER_EMPTY ){
				pc = tc->player[i];
			
				MainPlayerControl( pc );
				buildCommand = GetPlayerBuildCommand( pc );
	
				if( onGameFlag == TRUE ){
	
					if( buildCommand == PBC_SUMMON ){
						GetPlayerControlTrans( pc, &setTrans );
						if( GetMapAreaMaskStatus( tc, &setTrans ) == TRUE ){
							if( CreateSummonObject( tc, 1, &setTrans ) != -1 ){
								MakeTeamMapAreaMask( tc );
							}
						}
					}
					if( GetPlayerDeadFlag( pc ) == TRUE ){	//死亡時の拠点ダメージ
						int deadDamage = 100;
	
						SetCastleDamage( tc, &deadDamage );
						ResetPlayerDeadFlag( pc );
					}
				} else {
					if( buildCommand == PBC_BUILD_CASTLE ){
						GetPlayerControlTrans( pc, &setTrans );
						if( CreateCastle( tc, &setTrans ) != -1 ){
							MakeTeamMapAreaMask( tc );
						}
					}
				}
				ResetPlayerBuildCommand( pc );
				ResetPlayerDeadFlag( pc );
			}
		}
	}
	//召喚物動作
	if( onGameFlag == TRUE ){
		int summonHp, summonHpMax;

		for( i=0; i<TEAM_SUMMON_COUNT_MAX; i++ ){ 
			if( tc->summon[i].type != SUMMON_EMPTY ){
				GetSummonObjectHP( tc, i, &summonHp, &summonHpMax );
				if( summonHp == 0 ){
					DeleteSummonObject( tc, i );
					MakeTeamMapAreaMask( tc );
				}
			}
		}
	}
#endif
}
	
//------------------------------------------------------------------
/**
 * @brief	プレーヤー登録
 */
//------------------------------------------------------------------
int RegistPlayer( TEAM_CONTROL* tc, PLAYER_STATUS* ps, int playerObjID, int netID, VecFx32* trans )
{
	int	i;

	for( i=0; i<TEAM_PLAYER_COUNT_MAX; i++ ){
		if( tc->player[i] == PLAYER_EMPTY ){
			tc->player[i] = AddPlayerControl(	tc->gs,
												G3DSCOBJ_PLAYER1 + playerObjID,
												netID,
												tc->heapID );
			SetPlayerStatus( tc->player[i], ps ); 
			SetPlayerControlTrans( tc->player[i], trans );
			tc->playerCount++;
			return i;
		}
	}
	return -1;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤー登録削除
 */
//------------------------------------------------------------------
void ReleasePlayer( TEAM_CONTROL* tc, int playerID )
{
	if( tc->player[playerID] != PLAYER_EMPTY ){
		RemovePlayerControl( tc->player[playerID] ); 
		tc->player[playerID] = PLAYER_EMPTY;
		tc->playerCount--;
	}
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤー登録数取得
 */
//------------------------------------------------------------------
int GetTeamPlayerCount( TEAM_CONTROL* tc )
{
	return tc->playerCount;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーコントローラ取得
 */
//------------------------------------------------------------------
PLAYER_CONTROL* GetTeamPlayerControl( TEAM_CONTROL* tc, int playerID )
{
	if( tc->player[playerID] != PLAYER_EMPTY ){
		return tc->player[playerID];
	}
	return NULL;
}

//------------------------------------------------------------------
/**
 * @brief	すべての登録プレーヤーのコントローラに対してコールバック処理を実行する
 */
//------------------------------------------------------------------
void ProcessingAllTeamPlayer( TEAM_CONTROL* tc, TEAM_PLAYER_CALLBACK* proc, void* work )
{
	int i, num;

	for( i=0, num=0; i<TEAM_PLAYER_COUNT_MAX; i++ ){
		if( tc->player[i] != PLAYER_EMPTY ){
			proc( tc->player[i], num, work );
			num++;
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	拠点エリアダメージ取得
 */
//------------------------------------------------------------------
void GetCastleAreaDamage( TEAM_CONTROL* tc, int* damage )
{
	*damage = tc->mapAreaScore/100;
}

//------------------------------------------------------------------
/**
 * @brief	拠点ダメージ設定
 */
//------------------------------------------------------------------
void SetCastleDamage( TEAM_CONTROL* tc, int* damage )
{
	CASTLE_STATUS* cs = &tc->castle;

	if( cs->type == CASTLE_EMPTY ){
		return;
	}
	if( cs->hpNow < *damage ){
		cs->hpNow = 0;
	} else {
		cs->hpNow -= *damage;
	}
}

//------------------------------------------------------------------
/**
 * @brief	拠点設置
 */
//------------------------------------------------------------------
int CreateCastle( TEAM_CONTROL* tc, VecFx32* trans )
{
	CASTLE_STATUS* cs = &tc->castle;

	if( cs->type == CASTLE_EMPTY ){
		cs->objID	= AddObject3Dmap( Get_GS_SceneMap( tc->gs ), EXMAPOBJ_WALL, trans );
		cs->type	= 1;
		cs->hpMax	= 50000;
		cs->hpNow	= cs->hpMax;
		cs->trans	= *trans;
		return 0;
	}
	return -1;
}

//------------------------------------------------------------------
/**
 * @brief	拠点削除
 */
//------------------------------------------------------------------
void DeleteCastle( TEAM_CONTROL* tc )
{
	CASTLE_STATUS* cs = &tc->castle;

	if( cs->type == CASTLE_EMPTY ){
		RemoveObject3Dmap( Get_GS_SceneMap( tc->gs ), cs->objID ); 
		cs->type = CASTLE_EMPTY;
	}
}

//------------------------------------------------------------------
/**
 * @brief	拠点ＨＰ取得
 */
//------------------------------------------------------------------
BOOL GetCastleHP( TEAM_CONTROL* tc, int* hp, int* hpMax )
{
	CASTLE_STATUS* cs = &tc->castle;

	if( cs->type == CASTLE_EMPTY ){
		return FALSE;
	} else {
		*hp = cs->hpNow;
		*hpMax = cs->hpMax;
		return TRUE;
	}
}

//------------------------------------------------------------------
/**
 * @brief	拠点座標取得
 */
//------------------------------------------------------------------
BOOL GetCastleTrans( TEAM_CONTROL* tc, VecFx32* trans )
{
	CASTLE_STATUS* cs = &tc->castle;

	if( cs->type == CASTLE_EMPTY ){
		return FALSE;
	} else {
		*trans = cs->trans;
		return TRUE;
	}
}

//------------------------------------------------------------------
/**
 * @brief	召喚物設置
 */
//------------------------------------------------------------------
int CreateSummonObject( TEAM_CONTROL* tc, int summonTypeID, VecFx32* trans )
{
	int	i;
	SUMMON_STATUS* ss;

	GF_ASSERT( summonTypeID < NELEMS(summonTypeTable) );

	for( i=0; i<TEAM_SUMMON_COUNT_MAX; i++ ){
		ss = &tc->summon[i];

		if( ss->type == SUMMON_EMPTY ){
			ss->objID	= AddObject3Dmap( Get_GS_SceneMap( tc->gs ), 
											summonTypeTable[summonTypeID].type, trans );
			ss->type	= summonTypeID;
			ss->hpMax	= summonTypeTable[summonTypeID].hp;
			ss->hpNow	= ss->hpMax;
			ss->trans	= *trans;
			return i;
		}
	}
	return -1;
}

//------------------------------------------------------------------
/**
 * @brief	召喚物削除
 */
//------------------------------------------------------------------
void DeleteSummonObject( TEAM_CONTROL* tc, int summonID )
{
	SUMMON_STATUS* ss = &tc->summon[summonID];

	if( ss->type != SUMMON_EMPTY ){
		RemoveObject3Dmap( Get_GS_SceneMap( tc->gs ), ss->objID ); 
		ss->type = SUMMON_EMPTY;
	}
}

//------------------------------------------------------------------
/**
 * @brief	召喚物描画スイッチ設定
 */
//------------------------------------------------------------------
void SetSummonObjectDrawSW( TEAM_CONTROL* tc, int summonID, BOOL* drawSW )
{
	SUMMON_STATUS* ss = &tc->summon[summonID];

	if( ss->type != SUMMON_EMPTY ){
		Set3DmapDrawSw( Get_GS_SceneMap( tc->gs ), ss->objID, drawSW ); 
	}
}

//------------------------------------------------------------------
/**
 * @brief	召喚物描画スイッチ取得
 */
//------------------------------------------------------------------
void GetSummonObjectDrawSW( TEAM_CONTROL* tc, int summonID, BOOL* drawSW )
{
	SUMMON_STATUS* ss = &tc->summon[summonID];

	if( ss->type != SUMMON_EMPTY ){
		Get3DmapDrawSw( Get_GS_SceneMap( tc->gs ), ss->objID, drawSW ); 
	}
}

//------------------------------------------------------------------
/**
 * @brief	召喚物ダメージ設定
 */
//------------------------------------------------------------------
void SetSummonObjectDamage( TEAM_CONTROL* tc, int summonID, int* damage )
{
	SUMMON_STATUS* ss = &tc->summon[summonID];

	if( ss->type == SUMMON_EMPTY ){
		return;
	}
	if( ss->hpNow < *damage ){
		ss->hpNow = 0;
	} else {
		ss->hpNow -= *damage;
	}
}

//------------------------------------------------------------------
/**
 * @brief	召喚物ＨＰ取得
 */
//------------------------------------------------------------------
void GetSummonObjectHP( TEAM_CONTROL* tc, int summonID, int* hp, int* hpMax )
{
	SUMMON_STATUS* ss = &tc->summon[summonID];

	if( ss->type != SUMMON_EMPTY ){
		*hp = ss->hpNow;
		*hpMax = ss->hpMax;
	} else {
		*hp = 0;
		*hpMax = 0;
	}
}

//------------------------------------------------------------------
/**
 * @brief	召喚物座標取得
 */
//------------------------------------------------------------------
void GetSummonObjectTrans( TEAM_CONTROL* tc, int summonID, VecFx32* trans )
{
	SUMMON_STATUS* ss = &tc->summon[summonID];

	if( ss->type != SUMMON_EMPTY ){
		*trans = ss->trans;
	} else {
		trans->x = 0;
		trans->y = 0;
		trans->z = 0;
	}
}

//------------------------------------------------------------------
/**
 * @brief	すべての登録召喚物に対してコールバック処理を実行する
 */
//------------------------------------------------------------------
void ProcessingAllTeamSummonObject( TEAM_CONTROL* tc, TEAM_SUMMON_CALLBACK* proc, void* work )
{
	int i, num;
	SUMMON_STATUS* ss;

	for( i=0, num=0; i<TEAM_SUMMON_COUNT_MAX; i++ ){
		ss = &tc->summon[i];
		if( ss->type != SUMMON_EMPTY ){
			proc( tc, i, num, work );
			num++;
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	塔データ
 */
//------------------------------------------------------------------
static const SUMMON_TYPETABLE	summonTypeTable[] = {
	{ 0, 0, 0 },	//dummy
	{ EXMAPOBJ_WALL, 100, 0, 8 },
	{ EXMAPOBJ_WALL, 100, 0, 8 },
	{ EXMAPOBJ_WALL, 100, 0, 8 },
	{ EXMAPOBJ_WALL, 100, 0, 8 },
	{ EXMAPOBJ_WALL, 100, 0, 8 },
	{ EXMAPOBJ_WALL, 100, 0, 8 },
	{ EXMAPOBJ_WALL, 100, 0, 8 },
	{ EXMAPOBJ_WALL, 100, 0, 8 },
};

//------------------------------------------------------------------
/**
 * @brief	領域管理ビットマップ変更フラグ取得
 */
//------------------------------------------------------------------
BOOL GetMapAreaMaskDrawFlag( TEAM_CONTROL* tc )
{
	return tc->mapAreaDrawFlag;
}

//------------------------------------------------------------------
/**
 * @brief	領域管理ビットマップ変更フラグセット
 */
//------------------------------------------------------------------
void SetMapAreaMaskDrawFlag( TEAM_CONTROL* tc, BOOL* sw )
{
	tc->mapAreaDrawFlag = *sw;
}

//------------------------------------------------------------------
/**
 * @brief	領域管理ビットマップ取得
 */
//------------------------------------------------------------------
void GetMapAreaMask( TEAM_CONTROL* tc, GFL_BMP_DATA** mapArea )
{
	*mapArea = tc->mapArea;
}

//------------------------------------------------------------------
/**
 * @brief	領域管理ビットマップ上の座標取得
 */
//------------------------------------------------------------------
#define MAP_CENTER_POSX ( 8*MAPAREA_SIZX/2 )
#define MAP_CENTER_POSY ( 8*MAPAREA_SIZY/2 )
#define MAP_LENGTHX		( 8*32 )
#define MAP_LENGTHZ		( 8*32 )

void GetMapAreaMaskPos( VecFx32* trans, u16* posx, u16* posy )
{
	*posx = MAP_CENTER_POSX + ( trans->x / FX32_ONE ) * MAPAREA_SIZX / MAP_LENGTHX;
	*posy = MAP_CENTER_POSY + ( trans->z / FX32_ONE ) * MAPAREA_SIZY / MAP_LENGTHZ;
}

//------------------------------------------------------------------
/**
 * @brief	領域管理ビットマップ上の状態取得
 */
//------------------------------------------------------------------
BOOL GetMapAreaMaskStatus( TEAM_CONTROL* tc, VecFx32* trans )
{
	u16		posx, posy, chrx, chry, dotx, doty;
	u32*	bmp  = (u32*)GFL_BMP_GetCharacterAdrs( tc->mapArea );
	u32		data, mask;

	GetMapAreaMaskPos( trans, &posx, &posy );
	chrx = posx/8;
	dotx = posx%8;
	chry = posy/8;
	doty = posy%8;
	data = bmp[ (chry*MAPAREA_SIZX+chrx)*(0x20/sizeof(u32)) + doty ];
	mask = 0x0f<<dotx*4;

	if( data & mask ){
		return FALSE;
	} else {
		return TRUE;
	}
}


//------------------------------------------------------------------
/**
 * @brief	領域管理ビットマップ作成
 */
//------------------------------------------------------------------
static const u32 dummyMaskTable[] = {
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0x11000000,
	0x00000000,0x10000000,0x11100000,0x11111000,0x11111100,0x11111111,0x11111111,0x11111111,
	0x11111000,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x00011111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x00000000,0x00000001,0x00000111,0x00011111,0x00111111,0x11111111,0x11111111,0x11111111,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000001,0x00000011,

	0x11100000,0x11100000,0x11110000,0x11111000,0x11111000,0x11111100,0x11111100,0x11111110,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x00000111,0x00000111,0x00001111,0x00011111,0x00011111,0x00111111,0x00111111,0x01111111,

	0x11111110,0x11111110,0x11111110,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x01111111,0x01111111,0x01111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,

	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111110,0x11111110,0x11111110,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x01111111,0x01111111,0x01111111,

	0x11111110,0x11111100,0x11111100,0x11111000,0x11111000,0x11110000,0x11100000,0x11100000,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x01111111,0x00111111,0x00111111,0x00011111,0x00011111,0x00001111,0x00000111,0x00000111,

	0x11000000,0x10000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x11111111,0x11111111,0x11111111,0x11111100,0x11111000,0x11100000,0x10000000,0x00000000,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111000,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x00011111,
	0x11111111,0x11111111,0x11111111,0x00111111,0x00011111,0x00000111,0x00000001,0x00000000,
	0x00000011,0x00000001,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
};

static void DrawMapAreaMask
		( GFL_BMP_DATA* mapArea, VecFx32* trans, int radius, HEAPID heapID )
{
	GFL_BMP_DATA* circleBmp;
	int bmpSize,bmpWidth;
	u16 posx, posy;

	bmpWidth = radius*2;
	bmpSize = bmpWidth/8;	//キャラ数計算
	if( bmpWidth%8 ){
		bmpSize++;
	}
	circleBmp = GFL_BMP_Create( bmpSize, bmpSize, GFL_BMP_16_COLOR, heapID );
	GFL_BMP_Clear( circleBmp, 0 );

	GetMapAreaMaskPos( trans, &posx, &posy );
#if 1
	{
		u8* dst = GFL_BMP_GetCharacterAdrs( circleBmp );
		GFL_STD_MemCopy32( dummyMaskTable, dst, bmpSize*bmpSize*0x20 );
		DC_FlushRange( dummyMaskTable, bmpSize*bmpSize*0x20 );
	}
#endif
	GFL_BMP_Print( circleBmp, mapArea, 0, 0, posx-radius,  posy-radius, radius*2, radius*2, 0 );
	GFL_BMP_Delete( circleBmp );
}
	
void MakeTeamMapAreaMask( TEAM_CONTROL* tc )
{
	int i, radius;
	CASTLE_STATUS*	cs;
	SUMMON_STATUS*	ss;
	GFL_BMP_DATA*	mapAreaTmp;

	mapAreaTmp = GFL_BMP_Create( MAPAREA_SIZX, MAPAREA_SIZY, GFL_BMP_16_COLOR, tc->heapID );
	GFL_BMP_Clear( mapAreaTmp, 0 );

	cs = &tc->castle;
	if( cs->type != CASTLE_EMPTY ){
		radius = 3*8;
		DrawMapAreaMask( mapAreaTmp, &cs->trans, radius, tc->heapID );
	}

	for( i=0; i<TEAM_SUMMON_COUNT_MAX; i++ ){
		ss = &tc->summon[i];
		if( ss->type != SUMMON_EMPTY ){
			radius = summonTypeTable[ ss->type ].radius;
			radius = 3*8;
			DrawMapAreaMask( mapAreaTmp, &ss->trans, radius, tc->heapID );
		}
	}
	{
		u32*	src = (u32*)GFL_BMP_GetCharacterAdrs( mapAreaTmp );
		u32*	dst = (u32*)GFL_BMP_GetCharacterAdrs( tc->mapArea );
		u16		siz = (MAPAREA_SIZX*8) * (MAPAREA_SIZY*8);
		u32		refData, setData;
		int		i, j, count;

		count = 0;

		for( i=0; i<siz/8; i++ ){
			refData = src[i];
			setData = 0;
			for( j=0; j<8; j++ ){
				if( !(refData & (0x0f<<j*4)) ){
					setData |= ( MAPAREAMASK_COL<<j*4);
				} else {
					count++;
				}
			}
			dst[i] = setData;
		}
		tc->mapAreaScore = count;
	}
	GFL_BMP_Delete( mapAreaTmp );

	tc->mapAreaDrawFlag = TRUE;
}

