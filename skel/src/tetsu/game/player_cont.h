//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _PLAYER_CONTROL	PLAYER_CONTROL;

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

}PLAYER_CONTROL_COMMAND;

// プレーヤーコントロールセット
extern PLAYER_CONTROL* AddPlayerControl( GAME_SYSTEM* gs, int targetAct, HEAPID heapID );
// プレーヤーコントロールメイン
extern void MainPlayerControl( PLAYER_CONTROL* pc );
// プレーヤーコントロール終了
extern void RemovePlayerControl( PLAYER_CONTROL* pc );
// プレーヤー方向の取得
extern void GetPlayerDirection( PLAYER_CONTROL* pc, u16* direction );
//プレーヤーコントロール方向の設定
extern void SetPlayerControlDirection( PLAYER_CONTROL* pc, u16* direction );
// プレーヤーコマンドの設定
extern void SetPlayerControlCommand( PLAYER_CONTROL* pc, PLAYER_CONTROL_COMMAND command );

