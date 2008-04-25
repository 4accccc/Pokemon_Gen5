//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ステータスウインドウ
 */
//------------------------------------------------------------------
typedef struct _STATUSWIN_CONTROL	STATUSWIN_CONTROL;

typedef struct {
	HEAPID				heapID;
	TEAM_CONTROL**		p_tc;
	int					teamCount;
	GFL_BMPWIN*			targetBmpwin;
}STATUSWIN_SETUP;

// ステータスウインドウコントロールセット
extern STATUSWIN_CONTROL* AddStatusWinControl( STATUSWIN_SETUP* setup );
// ステータスウインドウコントロールメイン
extern void MainStatusWinControl( STATUSWIN_CONTROL* swc );
// ステータスウインドウコントロール終了
extern void RemoveStatusWinControl( STATUSWIN_CONTROL* swc );

// リロードセット
extern void SetStatusWinReload( STATUSWIN_CONTROL* swc );

//------------------------------------------------------------------
/**
 * @brief	メッセージウインドウ
 */
//------------------------------------------------------------------
typedef struct _MSGWIN_CONTROL	MSGWIN_CONTROL;

typedef struct {
	HEAPID				heapID;
	GFL_BMPWIN*			targetBmpwin;
}MSGWIN_SETUP;

// メッセージウインドウコントロールセット
extern MSGWIN_CONTROL* AddMessageWinControl( MSGWIN_SETUP* setup );
// メッセージウインドウコントロールメイン
extern void MainMessageWinControl( MSGWIN_CONTROL* mwc );
// メッセージウインドウコントロール終了
extern void RemoveMessageWinControl( MSGWIN_CONTROL* mwc );

typedef enum {
	GMSG_NULL = 0,
	GMSG_GAME_INFO,
}MSGID;

// メッセージセット
extern void PutMessageWin( MSGWIN_CONTROL* mwc, MSGID msgID );

//------------------------------------------------------------------
/**
 * @brief	マップウインドウ
 */
//------------------------------------------------------------------
typedef struct _MAPWIN_CONTROL	MAPWIN_CONTROL;

typedef struct {
	HEAPID				heapID;
	GFL_BMPWIN*			targetBmpwin;
	GAME_SYSTEM*		gs;
	TEAM_CONTROL**		p_tc;
	int					teamCount;
	PLAYER_CONTROL*		myPc;
	TEAM_CONTROL*		myTc;
}MAPWIN_SETUP;

// マップウインドウコントロールセット
extern MAPWIN_CONTROL* AddMapWinControl( MAPWIN_SETUP* setup );
// マップウインドウコントロールメイン
extern void MainMapWinControl( MAPWIN_CONTROL* mpwc );
// マップウインドウコントロール終了
extern void RemoveMapWinControl( MAPWIN_CONTROL* mpwc );

// デバッグ用
extern void ChangeMapWinControl( MAPWIN_CONTROL* mpwc, PLAYER_CONTROL* pc, TEAM_CONTROL* tc );

