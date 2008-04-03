//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
#include "player_cont.h"

#define TEAM_SUMMON_COUNT_MAX	(6)
#define TEAM_PLAYER_COUNT_MAX	(1)

typedef struct _TEAM_CONTROL	TEAM_CONTROL;

//	チームコントロール起動
extern TEAM_CONTROL* AddTeamControl( GAME_SYSTEM* gs, HEAPID heapID );
// チームコントロール終了
extern void RemoveTeamControl( TEAM_CONTROL* tc );
// チームメインコントロール
extern void MainTeamControl( TEAM_CONTROL* tc, BOOL onGameFlag );

// プレーヤー登録
extern int RegistPlayer
			( TEAM_CONTROL* tc, PLAYER_STATUS* ps, int playerObjID, int netID, VecFx32* trans );
// プレーヤー登録削除
extern void ReleasePlayer( TEAM_CONTROL* tc, int playerID );
// プレーヤー登録数取得
extern int GetTeamPlayerCount( TEAM_CONTROL* tc );
// プレーヤーコントローラ取得
extern PLAYER_CONTROL* GetTeamPlayerControl( TEAM_CONTROL* tc, int playerID );
// すべての登録プレーヤーのコントローラに対してコールバック処理を実行する
//  ↓コールバック定義:引数( コントローラ, 現在の実行順番カウンタ, ワーク )
typedef void	(TEAM_PLAYER_CALLBACK)( PLAYER_CONTROL*, int, void* );	//
extern void ProcessingAllTeamPlayer( TEAM_CONTROL* tc, TEAM_PLAYER_CALLBACK* proc, void* work );

// 拠点エリアダメージ取得
extern void GetCastleAreaDamage( TEAM_CONTROL* tc, int* damage );
// 拠点ダメージ設定
extern void SetCastleDamage( TEAM_CONTROL* tc, int* damage );
// 拠点設置
extern int CreateCastle( TEAM_CONTROL* tc, VecFx32* trans );
// 拠点削除
extern void DeleteCastle( TEAM_CONTROL* tc );
// 拠点ＨＰ取得
extern BOOL GetCastleHP( TEAM_CONTROL* tc, int* hp, int* hpMax );
// 拠点座標取得
extern BOOL GetCastleTrans( TEAM_CONTROL* tc, VecFx32* trans );

// 召喚物設置
int CreateSummonObject( TEAM_CONTROL* tc, int summonTypeID, VecFx32* trans );
// 召喚物削除
extern void DeleteSummonObject( TEAM_CONTROL* tc, int summonID );
// 召喚物描画スイッチ設定
extern void SetSummonObjectDrawSW( TEAM_CONTROL* tc, int summonID, BOOL* drawSW );
// 召喚物描画スイッチ取得
extern void GetSummonObjectDrawSW( TEAM_CONTROL* tc, int summonID, BOOL* drawSW );
// 召喚物ダメージ設定
extern void SetSummonObjectDamage( TEAM_CONTROL* tc, int summonID, int* damage );
// 召喚物ＨＰ取得
extern void GetSummonObjectHP( TEAM_CONTROL* tc, int summonID, int* hp, int* hpMax );
// 召喚物座標取得
extern void GetSummonObjectTrans( TEAM_CONTROL* tc, int summonID, VecFx32* trans );
// すべての登録召喚物に対してコールバック処理を実行する
//  ↓コールバック定義:引数( 登録ＩＤ, 現在の実行順番カウンタ, ワーク )
typedef void	(TEAM_SUMMON_CALLBACK)( TEAM_CONTROL* , int, int, void* );	//
extern void ProcessingAllTeamSummonObject
			( TEAM_CONTROL* tc, TEAM_SUMMON_CALLBACK* proc, void* work );

// 領域マスクビットマップ作成
extern void MakeTeamMapAreaMask( TEAM_CONTROL* tc );
// 領域マスクビットマップ変更フラグ取得
extern BOOL GetMapAreaMaskDrawFlag( TEAM_CONTROL* tc );
// 領域マスクビットマップ変更フラグセット
extern void SetMapAreaMaskDrawFlag( TEAM_CONTROL* tc, BOOL* sw );
// 領域判定マスクビットマップ取得
extern void GetMapAreaMask( TEAM_CONTROL* tc, GFL_BMP_DATA** mapArea );
// 領域マスク上の座標取得
extern void GetMapAreaMaskPos( VecFx32* trans, u16* posx, u16* posy );
// 領域管理ビットマップ上の状態取得
extern BOOL GetMapAreaMaskStatus( TEAM_CONTROL* tc, VecFx32* trans );

