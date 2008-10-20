//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ステータスバーコントロール
 */
//------------------------------------------------------------------
typedef struct _STATUS_CONTROL	STATUS_CONTROL;

typedef struct {
	HEAPID				heapID;
	GAME_SYSTEM*		gs;
	TEAM_CONTROL**		p_tc;
	int					teamCount;
	CAMERA_CONTROL*		cc;
	PLAYER_CONTROL*		myPc;
}STATUS_SETUP;

// ステータスコントロールセット
extern STATUS_CONTROL* AddStatusControl( STATUS_SETUP* setup );
// ステータスコントロールメイン
extern void MainStatusControl( STATUS_CONTROL* stc );
// ステータスコントロール終了
extern void RemoveStatusControl( STATUS_CONTROL* stc );


extern void ChangeStatusControl( STATUS_CONTROL* stc , PLAYER_CONTROL* pc );


