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

	PCC_WALK,
	PCC_RUN,
	PCC_JUMP,
}PLAYER_CONTROL_COMMAND;

typedef enum {
	PSC_NOP = 0,
}PLAYER_SKILL_COMMAND;

typedef struct {
	VecFx32					trans;
	u16						direction;
	BOOL					anmSetFlag;
	BOOL					anmSetID;
	PLAYER_SKILL_COMMAND	skillCommand;

}PLAYER_STATUS_NETWORK;

// プレーヤーコントロールセット
extern PLAYER_CONTROL* AddPlayerControl( GAME_SYSTEM* gs, int netID, HEAPID heapID );
// プレーヤーコントロールメイン
extern void MainPlayerControl( PLAYER_CONTROL* pc );
// プレーヤーコントロールメイン(ネットワーク)
extern void MainNetWorkPlayerControl( PLAYER_CONTROL* pc, PLAYER_STATUS_NETWORK* psn );
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
//プレーヤーコントロール可能かどうかのチェック
extern BOOL CheckPlayerControlEnable( PLAYER_CONTROL* pc );
//プレーヤーステータスの取得と設定
extern void GetPlayerStatus( PLAYER_CONTROL* pc, PLAYER_STATUS* status );
extern void SetPlayerStatus( PLAYER_CONTROL* pc, const PLAYER_STATUS* status );
// プレーヤーコマンドの設定
extern void SetPlayerControlCommand( PLAYER_CONTROL* pc, const PLAYER_CONTROL_COMMAND command );
// プレーヤー攻撃コマンドの設定
extern void SetPlayerAttackCommand
	( PLAYER_CONTROL* pc, const PLAYER_CONTROL_COMMAND command, int attackID );
// プレーヤー移動コマンドの設定
extern void SetPlayerMoveCommand
	( PLAYER_CONTROL* pc, PLAYER_CONTROL_COMMAND command, VecFx32* mvDir );
// スキルコマンドの取得とリセット
extern PLAYER_SKILL_COMMAND GetPlayerSkillCommand( PLAYER_CONTROL* pc );
extern void ResetPlayerSkillCommand( PLAYER_CONTROL* pc );
// スキル実行中フラグの設定とリセット
extern void SetPlayerSkillBusyFlag( PLAYER_CONTROL* pc );
extern void ResetPlayerSkillBusyFlag( PLAYER_CONTROL* pc );
// ヒット判定可能フラグの取得
extern BOOL GetPlayerHitEnableFlag( PLAYER_CONTROL* pc );
// ネットＩＤの取得
extern int GetPlayerNetID( PLAYER_CONTROL* pc );
// ネットワーク用ステータスの取得とリセット
extern void GetPlayerNetStatus( PLAYER_CONTROL* pc, PLAYER_STATUS_NETWORK* pNetStatus );
extern void ResetPlayerNetStatus( PLAYER_STATUS_NETWORK* pNetStatus );
// プレーヤー移動開始設定
extern void SetPlayerMove
				( PLAYER_CONTROL* pc, VecFx32* mvDir, fx32 speed, u16 theta );
// プレーヤー移動終了判定
extern BOOL CheckPlayerMoveEnd( PLAYER_CONTROL* pc );


