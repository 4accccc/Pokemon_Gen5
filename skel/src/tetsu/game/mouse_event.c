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

struct _MOUSE_EVENT_SYS {
	HEAPID			heapID;
	GAME_SYSTEM*	gs;

	u16				g3DutilUnitIdx;
	u16				g3DutilObjIdx;

	u16				mouseActionMode;

	u16				mouseCursorID;
	VecFx32			mouseCursorPos;
	int				eventMode;

	u32				eventFlag;
	BOOL			jumpTrg;
	u32				jumpTrgEnableCount;

	u32				attackFrameCounter;
	BOOL			scrClrFlag;
};

enum {
	EVENT_MODE_NONE = 0,
	EVENT_MODE_MOVE,
	EVENT_MODE_ICON,
	EVENT_MODE_ATTACK,
};

enum {
	MOUSE_ACTION_STOP = 0,
	MOUSE_ACTION_NORMAL,
	MOUSE_ACTION_ATTACK,
};

typedef struct {
	MOUSE_EVENT		me;
	u8				px;
	u8				py;
	u8				sx;
	u8				sy;
	BOOL			moveEnable;
}ICON_AREA;

static const GFL_UI_TP_HITTBL icontp_data[] = {
	{ 16, 176-1, 0, 12-1 },		//左
	{ 16, 176-1, 244, 256-1 },	//右

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
	{ MOUSE_EVENT_CAMERAMOVE_L, 0, 2, 2, 20, TRUE },			//左
	{ MOUSE_EVENT_CAMERAMOVE_R, 30, 2, 2, 20, TRUE },			//右

	{ MOUSE_EVENT_ACTION_1, 3, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_2, 5, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_3, 7, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_4, 9, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_5, 11, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_6, 13, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_7, 15, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_8, 17, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_9, 19, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_10, 21, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_11, 23, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_12, 25, 1, 2, 2, FALSE },
	{ MOUSE_EVENT_ACTION_13, 27, 1, 2, 2, FALSE },

	{ MOUSE_EVENT_OPEN_MENU, 28, 21, 2, 2, FALSE },			//メニュー
};

static const GFL_UI_TP_HITTBL attacktp_data_trg[] = {
	{ GFL_UI_TP_USE_CIRCLE, 128, 128, 16 },

	{GFL_UI_TP_HIT_END,0,0,0},			//終了データ
};

static const GFL_UI_TP_HITTBL attacktp_data_direct[] = {
	{ 0, 128-1, 104, 152-1 },			//前方
	{ 128, 192-1, 104, 152-1 },			//後方
	{ 0, 128-1, 0, 128-1 },				//左上
	{ 0, 128-1, 128, 256-1 },			//右上
	{ 128, 192-1, 0, 128-1 },			//左下
	{ 128, 192-1, 128, 256-1 },			//右下

	{GFL_UI_TP_HIT_END,0,0,0},			//終了データ
};

static const MOUSE_EVENT attack_data[] = {	//attacktp_data_directと対応
	MOUSE_EVENT_ATTACK_1,				//前方
	MOUSE_EVENT_ATTACK_2,				//後方
	MOUSE_EVENT_ATTACK_3,				//左上
	MOUSE_EVENT_ATTACK_4,				//右上
	MOUSE_EVENT_ATTACK_5,				//左下
	MOUSE_EVENT_ATTACK_6,				//右下
};
//------------------------------------------------------------------
/**
 * @brief	３Ｄセットアップデータ
 */
//------------------------------------------------------------------
#include "src/sample_graphic/haruka.naix"
static const char g3DarcPath2[] = {"src/sample_graphic/haruka.narc"};

//------------------------------------------------------------------
enum {
	G3DRES_MOUSE_CURSOR = 0,
};

//３Ｄグラフィックリソーステーブル
static const GFL_G3D_UTIL_RES g3DresTbl[] = {
	{ (u32)g3DarcPath2, NARC_haruka_test_wall_nsbmd, GFL_G3D_UTIL_RESPATH },
};

//------------------------------------------------------------------
enum {
	G3DOBJ_MOUSE_CURSOR = 0,
};

//３Ｄオブジェクト定義テーブル
static const GFL_G3D_UTIL_OBJ g3DobjTbl[] = {
	{ G3DRES_MOUSE_CURSOR, 0, G3DRES_MOUSE_CURSOR, NULL, 0 },
};

//------------------------------------------------------------------
//設定テーブルデータ
static const GFL_G3D_UTIL_SETUP g3Dsetup = {
	g3DresTbl, NELEMS(g3DresTbl),
	g3DobjTbl, NELEMS(g3DobjTbl),
};

//------------------------------------------------------------------
static const GFL_G3D_SCENEOBJ_DATA mouseCursorData[] = {
	{ 
		G3DOBJ_MOUSE_CURSOR, 0, 0, 24, FALSE, FALSE, 
		{	{ 0, 0, 0 },
			{ FX32_ONE/8 , FX32_ONE/8, FX32_ONE/8 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};

static void MainMouseEventNormal( MOUSE_EVENT_SYS* mes );
static void MainMouseEventAttack( MOUSE_EVENT_SYS* mes );

static void setMouseEvent( MOUSE_EVENT_SYS* mes, u32 checkEventID );
static void resetMouseEvent( MOUSE_EVENT_SYS* mes, u32 checkEventID );
static void clearMouseEvent( MOUSE_EVENT_SYS* mes );
static void setJumpTrg( MOUSE_EVENT_SYS* mes );
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
	mes->eventMode = EVENT_MODE_NONE;
	GFL_BG_LoadScreenReq( PLAYICON_FRM );

	mes->mouseActionMode = MOUSE_ACTION_STOP;
	mes->scrClrFlag = FALSE;

	resetJumpTrg( mes );
	clearMouseEvent( mes );
	//３Ｄデータセットアップ
	{
		GFL_G3D_SCENE* g3Dscene = Get_GS_G3Dscene( mes->gs );
		mes->g3DutilUnitIdx = GFL_G3D_SCENE_AddG3DutilUnit( g3Dscene, &g3Dsetup );
		mes->g3DutilObjIdx = GFL_G3D_SCENE_GetG3DutilUnitObjIdx( g3Dscene, mes->g3DutilUnitIdx );


		mes->mouseCursorID = GFL_G3D_SCENEOBJ_Add( g3Dscene,	
										mouseCursorData, NELEMS(mouseCursorData),
										mes->g3DutilObjIdx );
	}
	return mes;
}

void ExitMouseEvent( MOUSE_EVENT_SYS* mes )
{
	{
		GFL_G3D_SCENE* g3Dscene = Get_GS_G3Dscene( mes->gs );

		GFL_G3D_SCENEOBJ_Remove( g3Dscene, mes->mouseCursorID, NELEMS(mouseCursorData) );
		GFL_G3D_SCENE_DelG3DutilUnit( g3Dscene, mes->g3DutilUnitIdx );
	}

	GFL_HEAP_FreeMemory( mes );
}

//------------------------------------------------------------------
/**
 * @brief	マウスイベント判定
 */
//------------------------------------------------------------------
void MainMouseEvent( MOUSE_EVENT_SYS* mes )
{
	switch( mes->mouseActionMode ){
		case MOUSE_ACTION_STOP:
			break;
		case MOUSE_ACTION_NORMAL:
			MainMouseEventNormal( mes );
			break;
		case MOUSE_ACTION_ATTACK:
			MainMouseEventAttack( mes );
			break;
	}
}

//------------------------------------------------------------------
// 通常モード

static void MainMouseEventNormal( MOUSE_EVENT_SYS* mes )
{
	u32					tpx, tpy;
	int					tpContTblPos;
	GFL_G3D_SCENEOBJ*	g3DsceneObj;
	BOOL				cursorDrawSw = FALSE;
	BOOL				scrClearReq = FALSE;

	g3DsceneObj = GFL_G3D_SCENEOBJ_Get( Get_GS_G3Dscene( mes->gs ), mes->mouseCursorID );

	clearMouseEvent( mes );

	if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){
		scrClearReq = TRUE;
		mes->eventMode = EVENT_MODE_NONE;
		GFL_G3D_SCENEOBJ_SetDrawSW( g3DsceneObj, &cursorDrawSw );
		if( checkJumpTrg( mes ) == TRUE ){
			setMouseEvent( mes, MOUSE_EVENT_JUMP );
		}
		resetJumpTrg( mes );
	} else {
		//攻撃トリガ判定
		tpContTblPos = GFL_UI_TP_HitCont( attacktp_data_trg );
		if( tpContTblPos != GFL_UI_TP_HIT_NONE ){
			if( mes->eventMode != EVENT_MODE_MOVE ){
				scrClearReq = TRUE;
				cursorDrawSw = FALSE;
				GFL_G3D_SCENEOBJ_SetDrawSW( g3DsceneObj, &cursorDrawSw );
				resetJumpTrg( mes );	//ジャンプ判定削除
				clearMouseEvent( mes );	//移動判定削除
				mes->eventMode = EVENT_MODE_ATTACK;
				mes->attackFrameCounter = 0;
				mes->mouseActionMode = MOUSE_ACTION_ATTACK;	//攻撃モードに移行
				return;
			}
		}
		//移動判定
		if( GetCursorVec( tpx, tpy, &mes->mouseCursorPos ) == TRUE ){
			cursorDrawSw = TRUE;
			GFL_G3D_SCENEOBJ_SetDrawSW( g3DsceneObj, &cursorDrawSw );
			GFL_G3D_SCENEOBJ_SetPos( g3DsceneObj, &mes->mouseCursorPos );
			mes->eventMode = EVENT_MODE_MOVE;
			if( mes->jumpTrg == TRUE ){
				if( mes->jumpTrgEnableCount ){
					mes->jumpTrgEnableCount--;
					setMouseEvent( mes, MOUSE_EVENT_MOVESTART );
				} else {
					setMouseEvent( mes, MOUSE_EVENT_MOVE );
				}
			}else{
				setJumpTrg( mes );
				setMouseEvent( mes, MOUSE_EVENT_MOVESTART );
			}
		}
		//アイコン判定
		tpContTblPos = GFL_UI_TP_HitCont( icontp_data );
		if( tpContTblPos == GFL_UI_TP_HIT_NONE ){
			scrClearReq = TRUE;
		} else {
			GFL_BG_ChangeScreenPalette( PLAYICON_FRM, 0, 0, 32, 24, PLAYICON_PLTT );
			GFL_BG_ChangeScreenPalette( PLAYICON_FRM,
										iconarea_data[ tpContTblPos ].px,
										iconarea_data[ tpContTblPos ].py,
										iconarea_data[ tpContTblPos ].sx,
										iconarea_data[ tpContTblPos ].sy,
										PLAYICON_PLTT+1 );
			GFL_G3D_SCENEOBJ_SetDrawSW( g3DsceneObj, &cursorDrawSw );
			mes->scrClrFlag = FALSE;
			if( iconarea_data[ tpContTblPos ].moveEnable == FALSE ){
				cursorDrawSw = FALSE;
				GFL_G3D_SCENEOBJ_SetDrawSW( g3DsceneObj, &cursorDrawSw );
				mes->eventMode = EVENT_MODE_ICON;
				resetJumpTrg( mes );	//ジャンプ判定削除
				clearMouseEvent( mes );	//移動判定削除
			}
			setMouseEvent( mes, iconarea_data[ tpContTblPos ].me );
		}
	}
	if( scrClearReq == TRUE ){
		if( mes->scrClrFlag == FALSE ){
			mes->scrClrFlag = TRUE;
			GFL_BG_ChangeScreenPalette( PLAYICON_FRM, 0, 0, 32, 24, PLAYICON_PLTT );
			GFL_BG_LoadScreenReq( PLAYICON_FRM );
		}
	} else {
		GFL_BG_LoadScreenReq( PLAYICON_FRM );
	}
}

//------------------------------------------------------------------
// 攻撃モード
static void MainMouseEventAttack( MOUSE_EVENT_SYS* mes )
{
	u32		tpx, tpy;
	int		tpContTblPos;
	BOOL	tpOnFlag = GFL_UI_TP_GetPointCont( &tpx, &tpy );

	clearMouseEvent( mes );

	if( mes->attackFrameCounter < 4 ){
		//4の間押しっぱなしかどうか確認
		if( tpOnFlag == FALSE ){
			mes->mouseActionMode = MOUSE_ACTION_NORMAL;	//通常モードに復帰
		}
		mes->attackFrameCounter++;
		return;
	}
	//4wait後方向確認
	tpContTblPos = GFL_UI_TP_HitCont( attacktp_data_direct );
	if( tpContTblPos == GFL_UI_TP_HIT_NONE ){
		tpContTblPos = 0;	//取得エラーの場合0に補正
	}
	setMouseEvent( mes, attack_data[ tpContTblPos ] );
	mes->mouseActionMode = MOUSE_ACTION_NORMAL;	//通常モードに復帰
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

static void clearMouseEvent( MOUSE_EVENT_SYS* mes )
{
	mes->eventFlag = 0;
}

//------------------------------------------------------------------
static void setJumpTrg( MOUSE_EVENT_SYS* mes )
{
	mes->jumpTrg = TRUE;
	mes->jumpTrgEnableCount = 2;
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

	//原点(0,0,0)を通るy=0水平面を暫定の地形とする
	posRef.x = 0;
	posRef.y = 0;
	posRef.z = 0;
	vecN.x = 0;
	vecN.y = FX32_ONE;
	vecN.z = 0;

	result = GFL_G3D_Calc_GetClossPointRayPlane
				( &posRay, &vecRay, &posRef, &vecN, cursorPos, 0 );
	if( result == GFL_G3D_CALC_TRUE ){
		return TRUE;
	}
	return FALSE;
}

