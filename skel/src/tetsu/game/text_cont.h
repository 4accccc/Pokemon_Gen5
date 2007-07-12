//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _STATUSWIN_CONTROL	STATUSWIN_CONTROL;

// ステータスウインドウコントロールセット
extern STATUSWIN_CONTROL* AddStatusWinControl
			( PLAYER_STATUS* targetStatus, GFL_BMPWIN* targetBmpwin, HEAPID heapID );
// ステータスウインドウコントロールメイン
extern void MainStatusWinControl( STATUSWIN_CONTROL* swc );
// ステータスウインドウコントロール終了
extern void RemoveStatusWinControl( STATUSWIN_CONTROL* swc );

// リロードセット
extern void SetStatusWinReload( STATUSWIN_CONTROL* swc );

