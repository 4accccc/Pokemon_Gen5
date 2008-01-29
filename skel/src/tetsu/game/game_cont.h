//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
#include "team_cont.h"

#define TEAM_COUNT_MAX	(2) 

typedef struct {
	int				netID;
	int				objID;
	PLAYER_STATUS*	ps;
}GAME_CONT_SETUP_PLAYER;

typedef struct {
	GAME_CONT_SETUP_PLAYER	player[TEAM_PLAYER_COUNT_MAX];
	int						playerCount;
	VecFx32					startTrans;
}GAME_CONT_SETUP_TEAM;

typedef struct {
	GAME_CONT_SETUP_TEAM	team[TEAM_COUNT_MAX];
	int						teamCount;
	int						myID;
}GAME_CONT_SETUP;

typedef struct {
	u16						trg;
	u16						cont;
	PLAYER_STATUS_NETWORK	psn;
}GAME_NETWORK_PLAYDATA;

typedef struct _GAME_CONTROL	GAME_CONTROL;

// ゲームコントロールセット
extern GAME_CONTROL* AddGameControl( GAME_SYSTEM* gs, GAME_CONT_SETUP* setup, HEAPID heapID );
// ゲームコントロールメイン
extern void MainGameControl( GAME_CONTROL* gc );
//	ゲームメインコントロール後のフラグリセット処理
extern void ResetGameControl( GAME_CONTROL* gc );
// ゲームコントロール終了
extern void RemoveGameControl( GAME_CONTROL* gc );

// 通信コマンドセット
//キーステータス
extern BOOL SetGameControlKeyCommand( GAME_CONTROL* gc, int netID, int trg, int cont ); 
extern void ResetAllGameControlKeyCommand( GAME_CONTROL* gc ); 
//ゲームプレイ情報データ
extern void GetGameNetWorkPlayData( GAME_CONTROL* gc, int netID, GAME_NETWORK_PLAYDATA* gnd ); 
extern void SetGameNetWorkPlayData( GAME_CONTROL* gc, int netID, GAME_NETWORK_PLAYDATA* gnd ); 
extern void ResetAllGameNetWorkPlayData( GAME_CONTROL* gc ); 

// チームコントローラ取得
extern TEAM_CONTROL*	GetTeamControl( GAME_CONTROL* gc, int teamID ); 
// チーム数取得
extern int				GetTeamCount( GAME_CONTROL* gc );


// プレーヤー切り替え（デバッグ用）
extern void	ChangeControlPlayer( GAME_CONTROL* gc, int playNetID );


