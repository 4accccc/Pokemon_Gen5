////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   field_cars.c
 * @brief  ゾーンID ZONE_ID_H01 の橋の上に車を走らせるシステム
 * @author obata_toshihiro
 * @date   2009.07.28
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "field_cars.h"
#include "arc/arc_def.h"
#include "arc/fieldmap/buildmodel_outdoor.naix"
#include "arc/fieldmap/zone_id.h"


//==============================================================================
/**
 * @brief 定数
 */
//==============================================================================

// トレーラーの数
#define TRAILER_NUM (2)

// トレーラの移動範囲
#define RAIL_MIN_Z (1200)
#define RAIL_MAX_Z (4230)

#define RAIL_X_01 (1756)
#define RAIL_X_02 (1895)
#define RAIL_Y    (490) 

// トレーラーの移動速度
#define TRAILER_SPEED_MAX (8)
#define TRAILER_SPEED_MIN (5)

// 発車範囲(プレイヤー位置)
#define START_MIN_Z   (1500)
#define START_MAX_Z  (4000)  

// 発車間隔(単位:フレーム)
#define START_MIN_INTERVAL   (60)
#define START_MAX_INTERVAL  (150)


// 船の数
#define SHIP_NUM (1)

// 船の移動範囲
#define SHIP_RAIL_MIN_X (600) 
#define SHIP_RAIL_MAX_X (3000) 
#define SHIP_RAIL_Y     (-35)
#define SHIP_RAIL_Z     (2386)  

// 船の発車範囲(プレイヤー位置)
#define SHIP_START_MIN_Z   (1286)
#define SHIP_START_MAX_Z  (3786)  

// 船の移動速度
#define SHIP_SPEED_MAX (3)
#define SHIP_SPEED_MIN (1)

// 船の発車間隔(単位:フレーム)
#define SHIP_START_MIN_INTERVAL   (60)
#define SHIP_START_MAX_INTERVAL  (150)



//============================================================================== 
/**
 * @brief インデックス
 */
//==============================================================================

// リソース・インデックス
enum RES_INDEX
{
	RES_INDEX_TRAILER_BMD,	// トレーラーのモデル
	RES_INDEX_SHIP_BMD,		// 船のモデル
	RES_INDEX_MAX
};

// レンダー・インデックス
enum RND_INDEX
{
	RND_INDEX_TRAILER,		// トレーラー
	RND_INDEX_SHIP,			// 船
	RND_INDEX_MAX
};

// 3Dオブジェクト・インデックス
enum OBJ_INDEX
{
	OBJ_INDEX_TRAILER,		// トレーラー
	OBJ_INDEX_SHIP,			// 船
	OBJ_INDEX_MAX
};


//============================================================================== 
/**
 * @brief 構造体
 */
//============================================================================== 

//------------
// トレーラー
//------------
typedef struct 
{
	BOOL              active;	// 動いているかどうか
	GFL_G3D_OBJSTATUS status;	// ステータス
	fx32              speed;	// 移動速度
}
TRAILER; 

//-----
// 船
//-----
typedef struct 
{
	BOOL              active;	// 動いているかどうか
	GFL_G3D_OBJSTATUS status;	// ステータス
	fx32              speed;	// 移動速度
}
SHIP; 

//------------
// タイマー
//------------
typedef struct
{
	int count;
}
TIMER;

//------------
// システム
//------------
struct _FIELD_CARS
{
	HEAPID heapID;	// ヒープID

	FIELD_PLAYER* pFieldPlayer;	// 監視対象プレイヤー

	GFL_G3D_RES* pRes[ RES_INDEX_MAX ];	// リソース
	GFL_G3D_RND* pRnd[ RND_INDEX_MAX ];	// 3Dレンダー
	GFL_G3D_OBJ* pObj[ OBJ_INDEX_MAX ]; // 3Dオブジェクト

	TRAILER trailer[ TRAILER_NUM ];			// トレーラー
	TIMER   timerOfTrailer[ TRAILER_NUM ];	// タイマー

	SHIP    ship[ SHIP_NUM ];			// 船
	TIMER   timerOfShip[ TRAILER_NUM ];	// タイマー

	BOOL active;	// 動作フラグ
};



//============================================================================== 
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//============================================================================== 

// トレーラー動作
static void MoveTrailer( TRAILER* p_trailer );

// 船動作
static void MoveShip( SHIP* p_ship );

// システム初期化
static void LoadResource( FIELD_CARS* p_sys );
static void CreateRender( FIELD_CARS* p_sys );
static void CreateObject( FIELD_CARS* p_sys );
static void InitTrailers( FIELD_CARS* p_sys );
static void InitShips( FIELD_CARS* p_sys );
static void InitTimers( FIELD_CARS* p_sys );

// システム進行
static void ActTrailers( FIELD_CARS* p_sys );
static void ActShips( FIELD_CARS* p_sys );


//============================================================================== 
/**
 * @brief 公開関数の実装
 */
//============================================================================== 

//-------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 *
 * @param p_player 監視対象のプレイヤーオブジェクト
 * @param zone_id  ゾーンID
 * @param heap_id  使用するヒープID
 *
 * @return 作成したシステム
 */
//-------------------------------------------------------------------------------
FIELD_CARS* FIELD_CARS_Create( FIELD_PLAYER* p_player, u16 zone_id, HEAPID heap_id )
{
	FIELD_CARS* p_sys;

	// システム本体を作成
	p_sys = GFL_HEAP_AllocMemory( heap_id, sizeof( FIELD_CARS ) );

	// システム初期化
	p_sys->heapID       = heap_id;
	p_sys->pFieldPlayer = p_player;
	p_sys->active       = ( zone_id == ZONE_ID_H01 );

	// トレーラー初期化
	InitTrailers( p_sys );

	// 船の初期化
	InitShips( p_sys );

	// タイマー初期化
	InitTimers( p_sys );

	// リソース読み込み
	LoadResource( p_sys ); 

	// レンダー作成
	CreateRender( p_sys );

	// 3Dオブジェクトの作成
	CreateObject( p_sys );

	return p_sys;
}



//------------------------------------------------------------------------------- 
/**
 * @brief 破棄関数
 *
 * @param p_sys 破棄するシステム
 */
//------------------------------------------------------------------------------- 
void FIELD_CARS_Delete( FIELD_CARS* p_sys )
{
	int i;

	// 3Dオブジェクト
	for( i=0; i<OBJ_INDEX_MAX; i++ )
	{
		GFL_G3D_OBJECT_Delete( p_sys->pObj[i] );
	}

	// 3Dレンダー
	for( i=0; i<RND_INDEX_MAX; i++ )
	{
		GFL_G3D_RENDER_Delete( p_sys->pRnd[i] );
	}
	
	// リソース
	for( i=0; i<RES_INDEX_MAX; i++ )
	{
		GFL_G3D_DeleteResource( p_sys->pRes[i] );
	}
	
	// システム本体
	GFL_HEAP_FreeMemory( p_sys );
}



//------------------------------------------------------------------------------- 
/**
 * @brief 進行関数
 *
 * @param 進行するシステム
 */
//------------------------------------------------------------------------------- 
void FIELD_CARS_Process( FIELD_CARS* p_sys )
{
	// 動作フラグが立っていなければ, 何もしない
	if( p_sys->active != TRUE ) return;

	// トレーラーの動作
	ActTrailers( p_sys ); 

	// 船の動作
	ActShips( p_sys );
}


//------------------------------------------------------------------------------- 
/**
 * @brief 描画関数
 *
 * @param p_sys 描画するシステム
 */
//------------------------------------------------------------------------------- 
void FIELD_CARS_Draw( FIELD_CARS* p_sys )
{ 
	int i;

	// 動作フラグが立っていなければ, 何もしない
	if( p_sys->active != TRUE ) return;

	// 動作中のトレーラーのみを描画する
	for( i=0; i<TRAILER_NUM; i++ )
	{ 
		if( p_sys->trailer[i].active == TRUE )
		{ 
			GFL_G3D_DRAW_DrawObject( p_sys->pObj[ OBJ_INDEX_TRAILER ], &p_sys->trailer[i].status );
		}
	}

	// 動作中の船のみを描画する
	for( i=0; i<SHIP_NUM; i++ )
	{ 
		if( p_sys->ship[i].active == TRUE )
		{ 
			GFL_G3D_DRAW_DrawObject( p_sys->pObj[ OBJ_INDEX_SHIP ], &p_sys->ship[i].status );
		}
	}
}


//============================================================================== 
/**
 * @brief 非公開関数の実装
 */
//============================================================================== 

//------------------------------------------------------------------------------- 
/**
 * @brief トレーラーを動かす
 *
 * @param p_trailer 動かすトレーラー
 */
//------------------------------------------------------------------------------- 
static void MoveTrailer( TRAILER* p_trailer )
{
	// アクティブでなければ, 動かない
	if( p_trailer->active != TRUE ) return;

	// 移動
	p_trailer->status.trans.z += p_trailer->speed;

	// 移動範囲を超えたら, 動作終了
	if( ( FX_Whole( p_trailer->status.trans.z ) < RAIL_MIN_Z ) || ( RAIL_MAX_Z < FX_Whole( p_trailer->status.trans.z ) ) )
	{
		p_trailer->active = FALSE;
	}
}

//------------------------------------------------------------------------------- 
/**
 * @brief 船を動かす
 *
 * @param p_ship 動かすトレーラー
 */
//------------------------------------------------------------------------------- 
static void MoveShip( SHIP* p_ship )
{ 
	// アクティブでなければ, 動かない
	if( p_ship->active != TRUE ) return;

	// 移動
	p_ship->status.trans.x += p_ship->speed;

	// 移動範囲を超えたら, 動作終了
	if( ( FX_Whole( p_ship->status.trans.x ) < SHIP_RAIL_MIN_X ) || ( SHIP_RAIL_MAX_X < FX_Whole( p_ship->status.trans.x ) ) )
	{
		p_ship->active = FALSE;
	}
}

//------------------------------------------------------------------------------- 
/**
 * @brief リソースを読み込む
 *
 * @param p_sys 読み込むシステム
 */
//------------------------------------------------------------------------------- 
static void LoadResource( FIELD_CARS* p_sys )
{
	// トレーラーのモデル
	p_sys->pRes[ RES_INDEX_TRAILER_BMD ] = 
		GFL_G3D_CreateResourceArc( 
				ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_trailer_01_nsbmd );

	// 船のモデル
	p_sys->pRes[ RES_INDEX_SHIP_BMD ] = 
		GFL_G3D_CreateResourceArc( 
				ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_h01_ship_nsbmd );

	// テクスチャ転送
	GFL_G3D_TransVramTexture( p_sys->pRes[ RES_INDEX_TRAILER_BMD ] );
	GFL_G3D_TransVramTexture( p_sys->pRes[ RES_INDEX_SHIP_BMD ] );

	// DEBUG:
	{
		int i; 
		for( i=0; i<RES_INDEX_MAX; i++ )
		{
			GF_ASSERT_MSG( p_sys->pRes[i] != NULL, "リソース読み込み失敗" );
		}
	}
}

//------------------------------------------------------------------------------- 
/**
 * @brief 3Dレンダーを作成する
 *
 * @param p_sys 作成対象のシステム
 */ 
//------------------------------------------------------------------------------- 
static void CreateRender( FIELD_CARS* p_sys )
{
	// トレーラー
	p_sys->pRnd[ RND_INDEX_TRAILER ] = 
		GFL_G3D_RENDER_Create(
				p_sys->pRes[ RES_INDEX_TRAILER_BMD ],
				0,
				p_sys->pRes[ RES_INDEX_TRAILER_BMD ] ); 

	// 船
	p_sys->pRnd[ RND_INDEX_SHIP ] = 
		GFL_G3D_RENDER_Create(
				p_sys->pRes[ RES_INDEX_SHIP_BMD ],
				0,
				p_sys->pRes[ RES_INDEX_SHIP_BMD ] ); 

	// DEBUG:
	{
		int i; 
		for( i=0; i<RND_INDEX_MAX; i++ )
		{
			GF_ASSERT_MSG( p_sys->pRnd[i] != NULL, "レンダー作成失敗" );
		}
	}
}

//------------------------------------------------------------------------------- 
/**
 * @brief 3Dオブジェクトを作成する
 *
 * @param p_sys 作成対象のシステム
 */
//------------------------------------------------------------------------------- 
static void CreateObject( FIELD_CARS* p_sys )
{
	// トレーラー
	p_sys->pObj[ OBJ_INDEX_TRAILER ]
		= GFL_G3D_OBJECT_Create( p_sys->pRnd[ RND_INDEX_TRAILER ], NULL, 0 );

	// 船
	p_sys->pObj[ OBJ_INDEX_SHIP ]
		= GFL_G3D_OBJECT_Create( p_sys->pRnd[ RND_INDEX_SHIP ], NULL, 0 );

	// DEBUG:
	{
		int i; 
		for( i=0; i<OBJ_INDEX_MAX; i++ )
		{
			GF_ASSERT_MSG( p_sys->pObj[i] != NULL, "オブジェクト作成失敗" );
		}
	}
}

//------------------------------------------------------------------------------- 
/**
 * @brief トレーラーを初期化する
 *
 * @param p_sys 初期化対象のシステム
 */
//------------------------------------------------------------------------------- 
static void InitTrailers( FIELD_CARS* p_sys )
{
	p_sys->trailer[0].active = FALSE;
	VEC_Set( &p_sys->trailer[0].status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
	MTX_Identity33( &p_sys->trailer[0].status.rotate );

	p_sys->trailer[1].active = FALSE;
	VEC_Set( &p_sys->trailer[1].status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
	MTX_RotY33( &p_sys->trailer[1].status.rotate, FX_SinIdx( 32768 ), FX_CosIdx( 32768 ) );	// y軸180度回転
}

//------------------------------------------------------------------------------- 
/**
 * @brief 船を初期化する
 *
 * @param p_sys 初期化対象のシステム
 */
//------------------------------------------------------------------------------- 
static void InitShips( FIELD_CARS* p_sys )
{
	p_sys->ship[0].active = FALSE;
	VEC_Set( &p_sys->ship[0].status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
	MTX_Identity33( &p_sys->ship[0].status.rotate ); 
}

//------------------------------------------------------------------------------- 
/**
 * @brief タイマーを初期化する
 *
 * @param p_sys 初期化対象のシステム
 */
//------------------------------------------------------------------------------- 
static void InitTimers( FIELD_CARS* p_sys )
{
	int i;

	// トレーラー用タイマー
	for( i=0; i<TRAILER_NUM; i++ )
	{
		p_sys->timerOfTrailer[i].count = 0;
	}

	// 船用タイマー
	for( i=0; i<SHIP_NUM; i++ )
	{
		p_sys->timerOfShip[i].count = 0;
	}
}

//------------------------------------------------------------------------------- 
/**
 * @brief トレーラーを動かす
 *
 * @param p_sys 処理対象のシステム
 */
//------------------------------------------------------------------------------- 
static void ActTrailers( FIELD_CARS* p_sys )
{
	int i;
	VecFx32 pos;
	BOOL player_is_in_area = FALSE;	// プレイヤーが発車領域内にいるかどうか

	// プレイヤーの位置判定
	FIELD_PLAYER_GetPos( p_sys->pFieldPlayer, &pos );
	if( ( START_MIN_Z < FX_Whole( pos.z ) ) && ( FX_Whole( pos.z ) < START_MAX_Z ) )
	{
		player_is_in_area = TRUE;
	}

	// 全てのトレーラをチェックする
	for( i=0; i<TRAILER_NUM; i++ )
	{
		// 動いている場合
		if( p_sys->trailer[i].active == TRUE )
		{
			// 移動させる
			MoveTrailer( &p_sys->trailer[i] );

			// 移動が終わったら, タイマーセット
			if( p_sys->trailer[i].active == FALSE )
			{
				p_sys->timerOfTrailer[i].count
					= START_MIN_INTERVAL + GFL_STD_MtRand( START_MAX_INTERVAL - START_MIN_INTERVAL ); 

				// 念のため
				if( START_MAX_INTERVAL < p_sys->timerOfTrailer[i].count ) p_sys->timerOfTrailer[i].count = START_MAX_INTERVAL;

				// DEBUG:
				//OBATA_Printf("[%d]SetTimer : count = %d\n", i, p_sys->timerOfTrailer[i].count );
			}
		}
		// 動いていない場合
		else 
		{ 
			// 発車領域内にいれば, 発車判定を行う
			if( player_is_in_area )
			{
				// 時間経過で発車
				if( --p_sys->timerOfTrailer[i].count < 0 )
				{
					fx32 x = 0;
					fx32 z = 0;
					fx32 s = 0;

					switch( i )
					{
						case 0:	
							x = (RAIL_X_01) << FX32_SHIFT;
							z = (RAIL_MAX_Z) << FX32_SHIFT; 
							s = -( GFL_STD_MtRand( TRAILER_SPEED_MAX - TRAILER_SPEED_MIN ) + TRAILER_SPEED_MIN ) << FX32_SHIFT;	
							break;
						case 1:	
							x = (RAIL_X_02) << FX32_SHIFT;
							z = (RAIL_MIN_Z) << FX32_SHIFT; 
							s = ( GFL_STD_MtRand( TRAILER_SPEED_MAX - TRAILER_SPEED_MIN ) + TRAILER_SPEED_MIN ) << FX32_SHIFT;	
							break;
					}

					p_sys->trailer[i].active = TRUE;
					p_sys->trailer[i].status.trans.x  = x;
					p_sys->trailer[i].status.trans.y  = (RAIL_Y) << FX32_SHIFT;
					p_sys->trailer[i].status.trans.z  = z;
					p_sys->trailer[i].speed  = s;

					// DEBUG:
					//OBATA_Printf( "[%d]TrailerStart : speed = %d\n", i, FX_Whole( p_sys->trailer[i].speed ) );
				}
			} 
		}
	}
}

//------------------------------------------------------------------------------- 
/**
 * @brief 船を動かす
 *
 * @param p_sys 処理対象のシステム
 */
//------------------------------------------------------------------------------- 
static void ActShips( FIELD_CARS* p_sys )
{
	int i;
	VecFx32 pos;
	BOOL player_is_in_area = FALSE;	// プレイヤーが発車領域内にいるかどうか

	// プレイヤーの位置判定
	FIELD_PLAYER_GetPos( p_sys->pFieldPlayer, &pos );
	if( ( SHIP_START_MIN_Z < FX_Whole( pos.z ) ) && ( FX_Whole( pos.z ) < SHIP_START_MAX_Z ) )
	{
		player_is_in_area = TRUE;
	}

	// 全ての船をチェックする
	for( i=0; i<SHIP_NUM; i++ )
	{
		// 動いている場合
		if( p_sys->ship[i].active == TRUE )
		{
			// 移動させる
			MoveShip( &p_sys->ship[i] );

			// 移動が終わったら, タイマーセット
			if( p_sys->ship[i].active == FALSE )
			{
				p_sys->timerOfShip[i].count
					= SHIP_START_MIN_INTERVAL + GFL_STD_MtRand( SHIP_START_MAX_INTERVAL - SHIP_START_MIN_INTERVAL ); 

				// 念のため
				if( START_MAX_INTERVAL < p_sys->timerOfShip[i].count ) p_sys->timerOfShip[i].count = START_MAX_INTERVAL;

				// DEBUG:
				//OBATA_Printf("[%d]SetTimer : count = %d\n", i, p_sys->timerOfShip[i].count );
			}
		}
		// 動いていない場合
		else 
		{ 
			// 発車領域内にいれば, 発車判定を行う
			if( player_is_in_area )
			{
				// 時間経過で発車
				if( --p_sys->timerOfShip[i].count < 0 )
				{
					fx32 x = 0;
					fx32 z = 0;
					fx32 s = 0;

					switch( i )
					{
						case 0:	
							x = (SHIP_RAIL_MIN_X) << FX32_SHIFT;
							z = (SHIP_RAIL_Z) << FX32_SHIFT; 
							s = ( GFL_STD_MtRand( SHIP_SPEED_MAX - SHIP_SPEED_MIN ) + SHIP_SPEED_MIN ) << FX32_SHIFT;	
							break;
					}

					p_sys->ship[i].active = TRUE;
					p_sys->ship[i].status.trans.x  = x;
					p_sys->ship[i].status.trans.y  = (SHIP_RAIL_Y) << FX32_SHIFT;
					p_sys->ship[i].status.trans.z  = z;
					p_sys->ship[i].speed  = s;

					// DEBUG:
					//OBATA_Printf( "[%d]ShipStart : speed = %d\n", i, FX_Whole( p_sys->ship[i].speed ) );
				}
			} 
		}
	}
}
