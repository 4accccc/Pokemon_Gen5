//============================================================================================
/**
 * @file	mouse_event.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "ui.h"

#include "setup.h"

#include "mouse_event.h"
//============================================================================================
//
//
//	マウスイベント判定
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
#define NO_HIT_ICON (-1)

typedef struct {
	u8		startx;
	u8		starty;
	u8		nowx;
	u8		nowy;
}ATKMODE_STATUS;

struct _MOUSE_EVENT_SYS {
	HEAPID			heapID;
	GAME_SYSTEM*	gs;

	u16				mouseActionMode;

	u16				mouseCursorID;
	VecFx32			mouseCursorPos;

	int				selectIcon;
	u32				eventFlag;
	BOOL			jumpTrg;
	u32				jumpTrgEnableCount;

	u32				attackModeFrameCounter;
	BOOL			scrDrawFlag;
};

typedef struct {
	BOOL	trg;
	BOOL	cont;
	u32		x;
	u32		y;
}TP_STATUS;

enum {
	EVENT_MODE_NONE = 0,
	EVENT_MODE_MOVE,
	EVENT_MODE_ICON,
};

enum {
	MOUSE_ACTION_STOP = 0,
	MOUSE_ACTION_NORMAL,
	MOUSE_ACTION_ATTACK,
	MOUSE_ACTION_GROUND_SELECT,
	MOUSE_ACTION_GROUNDMAKE_UP,
	MOUSE_ACTION_GROUNDMAKE_DOWN,
};

typedef struct {
	MOUSE_EVENT		me;
	u8				px;
	u8				py;
	u8				sx;
	u8				sy;
	BOOL			moveEnable;
	BOOL			selectMode;
}ICON_AREA;

static void MainMouseEventNormal( MOUSE_EVENT_SYS* mes, TP_STATUS* tp );
static void MainMouseEventGroundSelect( MOUSE_EVENT_SYS* mes, TP_STATUS* tp );

static void setMouseEvent( MOUSE_EVENT_SYS* mes, u32 checkEventID );
static void resetMouseEvent( MOUSE_EVENT_SYS* mes, u32 checkEventID );
static BOOL setJumpTrg( MOUSE_EVENT_SYS* mes );
static void resetJumpTrg( MOUSE_EVENT_SYS* mes );
static BOOL checkJumpTrg( MOUSE_EVENT_SYS* mes );

static BOOL GetCursorVec( u32 tpx, u32 tpy, VecFx32* cursorPos );
//------------------------------------------------------------------
/**
 * @brief	マウスイベント判定システム起動と終了
 */
//------------------------------------------------------------------
MOUSE_EVENT_SYS* InitMouseEvent( GAME_SYSTEM* gs, HEAPID heapID )
{
	MOUSE_EVENT_SYS* mes = GFL_HEAP_AllocClearMemory( heapID, sizeof(MOUSE_EVENT_SYS) );

	mes->heapID = heapID;
	mes->gs = gs;

	GFL_BG_ChangeScreenPalette( PLAYICON_FRM, 0, 0, 32, 24, PLAYICON_PLTT );
	GFL_BG_LoadScreenReq( PLAYICON_FRM );

	mes->selectIcon = -1;
	mes->mouseActionMode = MOUSE_ACTION_STOP;
	mes->scrDrawFlag = FALSE;

	resetJumpTrg( mes );
	ClearMouseEvent( mes );
	return mes;
}

void ExitMouseEvent( MOUSE_EVENT_SYS* mes )
{
	GFL_HEAP_FreeMemory( mes );
}

//------------------------------------------------------------------
/**
 * @brief	マウスイベント判定
 */
//------------------------------------------------------------------
// gflib ui.cから引用　※uiが1/30 Frameに対応していないため暫定
//	円形としての当たり判定（単発）
static BOOL _circle_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y )
{
	x = (tbl->circle.x - x) * (tbl->circle.x - x);
	y = (tbl->circle.y - y) * (tbl->circle.y - y);

	if( x + y < (tbl->circle.r * tbl->circle.r) ) { return TRUE; }
	return FALSE;
}

//	矩形としての当たり判定（単発）
static BOOL _rect_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y )
{
	if( ((u32)( x - tbl->rect.left) < (u32)(tbl->rect.right - tbl->rect.left))
	&	((u32)( y - tbl->rect.top) < (u32)(tbl->rect.bottom - tbl->rect.top))
	){ return TRUE; }
	return FALSE;
}

//	矩形、円のあたり判定をテーブル分行う
static int _tblHitCheck( const GFL_UI_TP_HITTBL *tbl, const u16 x, const u16 y )
{
    int i;

    for(i = 0; tbl[i].circle.code != GFL_UI_TP_HIT_END; i++){
        if( tbl[i].circle.code == GFL_UI_TP_USE_CIRCLE ) {
            if( _circle_hitcheck( &tbl[i], x, y ) ){ return i; }
        } else {
            if( _rect_hitcheck( &tbl[i], x, y ) ){ return i; }
        }
    }
    return GFL_UI_TP_HIT_NONE;
}

//------------------------------------------------------------------
/**
 * @brief	マウスイベント判定
 */
//------------------------------------------------------------------
static void MainMouseEventAnyTime( MOUSE_EVENT_SYS* mes, TP_STATUS* tp );
static void MainMouseEventLimited( MOUSE_EVENT_SYS* mes, TP_STATUS* tp );
static void ResetMouseEventLimited( MOUSE_EVENT_SYS* mes );

void MainMouseEvent( MOUSE_EVENT_SYS* mes, BOOL tpTrg, BOOL tpCont, u32 tpX, u32 tpY, BOOL limit )
{
	TP_STATUS tp;

	tp.trg = tpTrg;
	tp.cont = tpCont;
	if(( tpTrg == FALSE )&&( tpCont == FALSE ) ){
		tp.x = 0;
		tp.y = 0;
	} else {
		tp.x = tpX;
		tp.y = tpY;
	}

	//ClearMouseEvent( mes );
	MainMouseEventAnyTime( mes, &tp );
	//if(( mes->eventFlag == 0 )&&( limit == FALSE )){
	if( limit == FALSE ){
		//常時判定部でイベント検出されたら限定項目は呼ばない
		MainMouseEventLimited( mes, &tp );
	} else { 
		ResetMouseEventLimited( mes );
	}
	if( mes->scrDrawFlag == TRUE ){
		GFL_BG_LoadScreenReq( PLAYICON_FRM );
	}
}

//------------------------------------------------------------------
static void setMouseIconEffect( const GFL_UI_TP_HITTBL *tbl, int tblPos )
{
	int	i;
	u8	px, py, sx, sy, pltt;

    for(i = 0; tbl[i].circle.code != GFL_UI_TP_HIT_END; i++){
		px = tbl[i].rect.left /8;
		py = tbl[i].rect.top /8;
		sx = (tbl[i].rect.right - tbl[i].rect.left) /8;
		if((tbl[i].rect.right - tbl[i].rect.left) %8){ sx++; }
		sy = (tbl[i].rect.bottom - tbl[i].rect.top) /8;
		if((tbl[i].rect.bottom - tbl[i].rect.top) %8){ sy++; }

		if( i == tblPos ){
			pltt = PLAYICON_PLTT+1;	//点灯
		} else {
			pltt = PLAYICON_PLTT;	//消灯
		}
		GFL_BG_ChangeScreenPalette( PLAYICON_FRM, px, py, sx, sy, pltt );
	}
}

static void clearMouseIconEffect( const GFL_UI_TP_HITTBL *tbl )
{
	setMouseIconEffect( tbl, GFL_UI_TP_HIT_NONE );
}

//------------------------------------------------------------------
/**
 * @brief	マウスイベント判定(常時項目)
 */
//------------------------------------------------------------------
static const GFL_UI_TP_HITTBL tpHitTbl1[] = {
	{ 16, 176-1, 0, 12-1 },		//左
	{ 16, 176-1, 244, 256 },	//右

	{GFL_UI_TP_HIT_END,0,0,0},			//終了データ
};

static const MOUSE_EVENT tpHitComTbl1[] = {	//tpHitTbl1と対応
	MOUSE_EVENT_CAMERAMOVE_L,	//カメラ左移動
	MOUSE_EVENT_CAMERAMOVE_R,	//カメラ右移動
};

static void MainMouseEventAnyTime( MOUSE_EVENT_SYS* mes, TP_STATUS* tp )
{
	if( tp->cont == TRUE ){
		int tblPos = _tblHitCheck( tpHitTbl1, tp->x, tp->y );
		setMouseIconEffect( tpHitTbl1, tblPos );

		if( tblPos != GFL_UI_TP_HIT_NONE ){
			setMouseEvent( mes, tpHitComTbl1[ tblPos ] );
		}
	} else {
		clearMouseIconEffect( tpHitTbl1 );
	}
	mes->scrDrawFlag = TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	マウスイベント判定(限定項目)
 */
//------------------------------------------------------------------
static const GFL_UI_TP_HITTBL icontp_data[] = {
	{ 8, 24-1, 24, 40-1 },
	{ 8, 24-1, 40, 56-1 },
	{ 8, 24-1, 56, 72-1 },
	{ 8, 24-1, 72, 88-1 },
	{ 8, 24-1, 88, 104-1 },
	{ 8, 24-1, 104, 120-1 },
	{ 8, 24-1, 120, 136-1 },
	{ 8, 24-1, 136, 152-1 },
	{ 8, 24-1, 152, 168-1 },
	{ 8, 24-1, 168, 184-1 },
	{ 8, 24-1, 184, 200-1 },
	{ 8, 24-1, 200, 216-1 },
	{ 8, 24-1, 216, 232-1 },

	{ 168, 184-1, 224, 240-1 },	//メニュー

	{GFL_UI_TP_HIT_END,0,0,0},			//終了データ
};

static const ICON_AREA iconarea_data[] = {	// icontp_dataと対応
	{ MOUSE_EVENT_ACTION_1,			 3,  1,  2,  2, FALSE, TRUE },
	{ MOUSE_EVENT_ACTION_2,			 5,  1,  2,  2, FALSE, TRUE },
	{ MOUSE_EVENT_ACTION_3,			 7,  1,  2,  2, FALSE, FALSE },
	{ MOUSE_EVENT_GROUNDMAKE_UP,	 9,  1,  2,  2, FALSE, TRUE },
	{ MOUSE_EVENT_GROUNDMAKE_DOWN,	11,  1,  2,  2, FALSE, TRUE },
	{ MOUSE_EVENT_ACTION_6,			13,  1,  2,  2, FALSE, FALSE },
	{ MOUSE_EVENT_ACTION_7,			15,  1,  2,  2, FALSE, FALSE },
	{ MOUSE_EVENT_ACTION_8,			17,  1,  2,  2, FALSE, FALSE },
	{ MOUSE_EVENT_ACTION_9,			19,  1,  2,  2, FALSE, FALSE },
	{ MOUSE_EVENT_ACTION_10,		21,  1,  2,  2, FALSE, FALSE },
	{ MOUSE_EVENT_ACTION_11,		23,  1,  2,  2, FALSE, FALSE },
	{ MOUSE_EVENT_ACTION_12,		25,  1,  2,  2, FALSE, FALSE },
	{ MOUSE_EVENT_ACTION_13,		27,  1,  2,  2, FALSE, FALSE },

	{ MOUSE_EVENT_OPEN_MENU,		28, 21,  2,  2, FALSE, FALSE },			//メニュー
};

static void MainMouseEventLimited( MOUSE_EVENT_SYS* mes, TP_STATUS* tp )
{
	switch( mes->mouseActionMode ){
		case MOUSE_ACTION_STOP:
			break;
		case MOUSE_ACTION_NORMAL:
			MainMouseEventNormal( mes, tp );
			break;
		case MOUSE_ACTION_GROUND_SELECT:
			MainMouseEventGroundSelect( mes, tp );
			break;
	}
}

static void ResetMouseEventLimited( MOUSE_EVENT_SYS* mes )
{
	resetJumpTrg( mes );	//ジャンプ判定削除
}

//------------------------------------------------------------------
// 通常モード
static const GFL_UI_TP_HITTBL attacktp_data_trg[] = {
	{ GFL_UI_TP_USE_CIRCLE, 128, 128, 16 },

	{GFL_UI_TP_HIT_END,0,0,0},			//終了データ
};

static void MainMouseEventNormal( MOUSE_EVENT_SYS* mes, TP_STATUS* tp )
{
	int	tblPos;

	//アイコン判定
	if( tp->trg == TRUE ){
			
		tblPos = _tblHitCheck( icontp_data, tp->x, tp->y );
		if( tblPos != GFL_UI_TP_HIT_NONE ){
			setMouseIconEffect( icontp_data, tblPos );

			resetJumpTrg( mes );	//ジャンプ判定削除
			mes->selectIcon = tblPos;

			if( iconarea_data[ tblPos ].selectMode == TRUE ){
				mes->mouseActionMode = MOUSE_ACTION_GROUND_SELECT;	//地形選択モードに移行
			} else {
				setMouseEvent( mes, iconarea_data[ tblPos ].me );
			}
			return;
		}
		mes->selectIcon = -1;
	}
	clearMouseIconEffect( icontp_data );

	if( tp->cont == TRUE ){
		//----暫定回避
		tblPos = _tblHitCheck( icontp_data, tp->x, tp->y );
		if( tblPos != GFL_UI_TP_HIT_NONE ){
			return;
		}
		//移動判定
		if( GetCursorVec( tp->x, tp->y, &mes->mouseCursorPos ) == TRUE ){
			if( setJumpTrg( mes ) == TRUE ){
				setMouseEvent( mes, MOUSE_EVENT_MOVESTART );
			} else {
				setMouseEvent( mes, MOUSE_EVENT_MOVE );
			}
		}
	} else {
		if( checkJumpTrg( mes ) == TRUE ){
			setMouseEvent( mes, MOUSE_EVENT_JUMP );
		}
		resetJumpTrg( mes );

	}
}

//------------------------------------------------------------------
static void MainMouseEventGroundSelect( MOUSE_EVENT_SYS* mes, TP_STATUS* tp )
{
	int tpContTblPos;

	if( tp->trg == TRUE ){
		if( GetCursorVec( tp->x, tp->y, &mes->mouseCursorPos ) == TRUE ){
			setMouseEvent( mes, iconarea_data[ mes->selectIcon ].me );

			clearMouseIconEffect( icontp_data );
			mes->mouseActionMode = MOUSE_ACTION_NORMAL;	//通常モードに復帰
		}
	}
}

//------------------------------------------------------------------
static void setMouseEvent( MOUSE_EVENT_SYS* mes, u32 checkEventID )
{
	u32 setBit = ( 0x00000001 << checkEventID );

	mes->eventFlag |= setBit;
}

static void resetMouseEvent( MOUSE_EVENT_SYS* mes, u32 checkEventID )
{
	u32 setBit = (( 0x00000001 << checkEventID ) ^0xffffffff );

	mes->eventFlag &= setBit;
}

//------------------------------------------------------------------
static BOOL setJumpTrg( MOUSE_EVENT_SYS* mes )
{
	if( mes->jumpTrg == TRUE ){
		if( mes->jumpTrgEnableCount ){
			mes->jumpTrgEnableCount--;
			return TRUE;
		} else {
			return FALSE;
		}
	}else{
		mes->jumpTrg = TRUE;
		mes->jumpTrgEnableCount = 2;
		return TRUE;
	}
}

static void resetJumpTrg( MOUSE_EVENT_SYS* mes )
{
	mes->jumpTrg = FALSE;
	mes->jumpTrgEnableCount = 0;
}

static BOOL checkJumpTrg( MOUSE_EVENT_SYS* mes )
{
	if( mes->jumpTrg == TRUE ){
		if( mes->jumpTrgEnableCount ){
			return TRUE;
		}
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	マウスイベント取得開始
 */
//------------------------------------------------------------------
void StartMouseEvent( MOUSE_EVENT_SYS* mes )
{
	mes->mouseActionMode = MOUSE_ACTION_NORMAL;
}

//------------------------------------------------------------------
/**
 * @brief	マウスイベント取得
 */
//------------------------------------------------------------------
BOOL CheckMouseEvent( MOUSE_EVENT_SYS* mes, u32 checkEventID )
{
	u32 chkBit = ( 0x00000001 << checkEventID );

	if( mes->eventFlag & chkBit ){
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL CheckResetMouseEvent( MOUSE_EVENT_SYS* mes, u32 checkEventID )
{
	BOOL flg = CheckMouseEvent( mes, checkEventID );
	resetMouseEvent( mes, checkEventID );

	return flg;
}

void ClearMouseEvent( MOUSE_EVENT_SYS* mes )
{
	mes->eventFlag = 0;
}

//------------------------------------------------------------------
/**
 * @brief	マウス位置取得
 */
//------------------------------------------------------------------
void GetMousePos( MOUSE_EVENT_SYS* mes, VecFx32* pos )
{
	*pos = mes->mouseCursorPos;
}

//------------------------------------------------------------------
/**
 * @brief	マウスカーソル地形あたり判定
 */
//------------------------------------------------------------------
static BOOL GetCursorVec( u32 tpx, u32 tpy, VecFx32* cursorPos )
{
	VecFx32 posRay, vecRay, posRef, vecN;
	VecFx32 pNear, pFar;
	GFL_G3D_CALC_RESULT result;

	//タッチパネル座標→ワールド座標系への変換 -1でビューポート外
	if( NNS_G3dScrPosToWorldLine( tpx, tpy, &pNear, &pFar ) == -1 ){
		return FALSE;
	}
	//レイ位置の設定
	posRay = pNear;
	//レイ進行ベクトルの算出
	VEC_Subtract( &pFar, &pNear, &vecRay );
	VEC_Normalize( &vecRay, &vecRay );	//正規化

	return GetRayPosOnMap( &posRay, &vecRay, cursorPos );
}

