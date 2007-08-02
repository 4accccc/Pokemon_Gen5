//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _PLAYER_CONTROL	PLAYER_CONTROL;

typedef struct {
	char		name[10];
	s16			hp;
	s16			sp;
	s16			hpMax;
	s16			spMax;
	s16			attack;
	s16			defence;
	s16			agility;
	s16			intel;
	s16			mind;
}PLAYER_STATUS;

typedef enum {
	PCC_NOP = 0,
	PCC_STAY,

	PCC_WALK_FRONT,
	PCC_WALK_BACK,
	PCC_WALK_LEFT,
	PCC_WALK_RIGHT,
	PCC_WALK_FRONT_LEFT,
	PCC_WALK_FRONT_RIGHT,
	PCC_WALK_BACK_LEFT,
	PCC_WALK_BACK_RIGHT,

	PCC_RUN_FRONT,
	PCC_RUN_BACK,
	PCC_RUN_LEFT,
	PCC_RUN_RIGHT,
	PCC_RUN_FRONT_LEFT,
	PCC_RUN_FRONT_RIGHT,
	PCC_RUN_BACK_LEFT,
	PCC_RUN_BACK_RIGHT,

	PCC_ATTACK,
	PCC_SIT,
	PCC_WEPONCHANGE,

	PCC_PUTON,

}PLAYER_CONTROL_COMMAND;

// プレーヤーコントロールセット
extern PLAYER_CONTROL* AddPlayerControl( GAME_SYSTEM* gs, int targetAct, HEAPID heapID );
// プレーヤーコントロールメイン
extern void MainPlayerControl( PLAYER_CONTROL* pc );
// プレーヤーコントロール終了
extern void RemovePlayerControl( PLAYER_CONTROL* pc );
// プレーヤー方向の取得
extern void GetPlayerDirection( PLAYER_CONTROL* pc, u16* direction );
// プレーヤーコントロール座標の取得と設定
extern void GetPlayerControlTrans( PLAYER_CONTROL* pc, VecFx32* trans );
extern void SetPlayerControlTrans( PLAYER_CONTROL* pc, const VecFx32* trans );
//プレーヤーコントロール方向の取得と設定
extern void GetPlayerControlDirection( PLAYER_CONTROL* pc, u16* direction );
extern void SetPlayerControlDirection( PLAYER_CONTROL* pc, const u16* direction );
//プレーヤーステータスポインタの取得
extern PLAYER_STATUS* GetPlayerStatusPointer( PLAYER_CONTROL* pc );
//プレーヤーステータスの取得と設定
extern void GetPlayerStatus( PLAYER_CONTROL* pc, PLAYER_STATUS* status );
extern void SetPlayerStatus( PLAYER_CONTROL* pc, const PLAYER_STATUS* status );
// プレーヤーコマンドの設定
extern void SetPlayerControlCommand( PLAYER_CONTROL* pc, const PLAYER_CONTROL_COMMAND command );
// スキルコマンドの取得とリセット
extern int GetPlayerSkillCommand( PLAYER_CONTROL* pc );
extern void ResetPlayerSkillCommand( PLAYER_CONTROL* pc );
// スキル実行中フラグの設定とリセット
extern void SetPlayerSkillBusyFlag( PLAYER_CONTROL* pc );
extern void ResetPlayerSkillBusyFlag( PLAYER_CONTROL* pc );
// ヒット判定可能フラグの取得
extern BOOL GetPlayerHitEnableFlag( PLAYER_CONTROL* pc );
// ＨＰ減少値の設定
extern void SetPlayerDamage( PLAYER_CONTROL* pc, const s16 damage );
// ＤＯＴ（時間経過によるＨＰ減少値）の設定
extern void SetPlayerDamageOnTime
				( PLAYER_CONTROL* pc, const u8 count, const u8 timer, const s8 value );
// リジェネレート値の設定
extern void SetPlayerRegenerate
				( PLAYER_CONTROL* pc, const u8 count, const u8 timer, const s8 value );


