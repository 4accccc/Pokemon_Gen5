//============================================================================================
/**
 * @file		box2_ui.c
 * @brief		ボックス画面 キータッチ関連
 * @author	Hiroyuki Nakamura
 * @date		09.10.05
 *
 *	モジュール名：BOX2_UI
 */
//============================================================================================
#include <gflib.h>

#include "system/cursor_move.h"

#include "system/main.h"
#include "ui/touchbar.h"

#include "box2_main.h"
#include "box2_bmp.h"
#include "box2_obj.h"
#include "box2_ui.h"
#include "box2_bgwfrm.h"


//============================================================================================
//	定数定義
//============================================================================================

// カーソル移動データ
typedef struct {
	const CURSORMOVE_DATA * dat;
	const CURSORMOVE_CALLBACK * cb;
}MOVE_DATA_TBL;

#define	TRAYPOKE_SIZE_X		( BOX2OBJ_TRAYPOKE_SX )							// トレイのポケモンのタッチ範囲サイズＸ
#define	TRAYPOKE_SIZE_Y		( 24 )											// トレイのポケモンのタッチ範囲サイズＹ
#define	TRAYPOKE_START_X	( BOX2OBJ_TRAYPOKE_PX - TRAYPOKE_SIZE_X / 2 )	// トレイのポケモンのタッチ開始座標Ｘ
#define	TRAYPOKE_START_Y	( 40 )											// トレイのポケモンのタッチ開始座標Ｙ

#define	TRAYPOKE_PX(a)	( TRAYPOKE_START_X + TRAYPOKE_SIZE_X * a )		// トレイのポケモンのタッチ開始座標Ｘ
#define	TRAYPOKE_PY(a)	( TRAYPOKE_START_Y + TRAYPOKE_SIZE_Y * a )		// トレイのポケモンのタッチ開始座標Ｙ

#define	PICUR_X		( 12 )	// カーソル表示補正Ｘ座標
#define	PICUR_Y		( -4 )	// カーソル表示補正Ｙ座標

// 手持ちフレーム（左）座標データ
#define	PARTY_FRM_UPY		( BOX2BGWFRM_PARTYPOKE_PY*8 )
#define	PARTY_FRM_DPY		( PARTY_FRM_UPY+BOX2BGWFRM_PARTYPOKE_SY*8-1 )
#define	PARTY_FRM_LLX		( BOX2BGWFRM_PARTYPOKE_LX*8 )
#define	PARTY_FRM_LRX		( PARTY_FRM_LLX+BOX2BGWFRM_PARTYPOKE_SX*8-1 )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void CursorObjMove( BOX2_SYS_WORK * syswk, int pos );

static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Dummy( void * work, int now_pos, int old_pos );

//static void PartyOutMainCallBack_On( void * work, int now_pos, int old_pos );
static void PartyOutMainCallBack_Touch( void * work, int now_pos, int old_pos );

//static void PartyOutBoxSelCallBack_On( void * work, int now_pos, int old_pos );
//static void PartyOutBoxSelCallBack_Move( void * work, int now_pos, int old_pos );
//static void PartyOutBoxSelCallBack_Touch( void * work, int now_pos, int old_pos );

//static void PartyInMainCallBack_On( void * work, int now_pos, int old_pos );
static void PartyInMainCallBack_Touch( void * work, int now_pos, int old_pos );

//static void BoxArrangeMainCallBack_On( void * work, int now_pos, int old_pos );
static void BoxArrangeMainCallBack_Touch( void * work, int now_pos, int old_pos );
static void BoxArrangeMainCallBack_Move( void * work, int now_pos, int old_pos );

//static void BoxArrangePokeMoveCallBack_On( void * work, int now_pos, int old_pos );
static void BoxArrangePokeMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void BoxArrangePokeMoveCallBack_Touch( void * work, int now_pos, int old_pos );

//static void BoxArrangePartyPokeMoveCallBack_On( void * work, int now_pos, int old_pos );
static void BoxArrangePartyPokeMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void BoxArrangePartyPokeMoveCallBack_Touch( void * work, int now_pos, int old_pos );

//static void BoxArrangePartyMoveCallBack_On( void * work, int now_pos, int old_pos );
static void BoxArrangePartyMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void BoxArrangePartyMoveCallBack_Touch( void * work, int now_pos, int old_pos );

//static void BoxItemArrangeMainCallBack_On( void * work, int now_pos, int old_pos );
static void BoxItemArrangeMainCallBack_Touch( void * work, int now_pos, int old_pos );
static void BoxItemArrangeMainCallBack_Move( void * work, int now_pos, int old_pos );

//static void BoxItemArrangePartyMoveCallBack_On( void * work, int now_pos, int old_pos );
static void BoxItemArrangePartyMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void BoxItemArrangePartyMoveCallBack_Touch( void * work, int now_pos, int old_pos );

//static void BoxThemaChgCallBack_On( void * work, int now_pos, int old_pos );
static void BoxThemaChgCallBack_Move( void * work, int now_pos, int old_pos );
static void BoxThemaChgCallBack_Touch( void * work, int now_pos, int old_pos );

//static void WallPaperChgCallBack_On( void * work, int now_pos, int old_pos );
static void WallPaperChgCallBack_Move( void * work, int now_pos, int old_pos );
static void WallPaperChgCallBack_Touch( void * work, int now_pos, int old_pos );

//static void BoxJumpCallBack_On( void * work, int now_pos, int old_pos );
static void BoxJumpCallBack_Move( void * work, int now_pos, int old_pos );
static void BoxJumpCallBack_Touch( void * work, int now_pos, int old_pos );

//static void BattleBoxMainCallBack_On( void * work, int now_pos, int old_pos );
static void BattleBoxMainCallBack_Move( void * work, int now_pos, int old_pos );
static void BattleBoxMainCallBack_Touch( void * work, int now_pos, int old_pos );

//static void BattleBoxPartyMainCallBack_On( void * work, int now_pos, int old_pos );
static void BattleBoxPartyMainCallBack_Move( void * work, int now_pos, int old_pos );
static void BattleBoxPartyMainCallBack_Touch( void * work, int now_pos, int old_pos );

//static void SleepMainCallBack_On( void * work, int now_pos, int old_pos );
static void SleepMainCallBack_Move( void * work, int now_pos, int old_pos );
static void SleepMainCallBack_Touch( void * work, int now_pos, int old_pos );


//============================================================================================
//	グローバル変数
//============================================================================================

// あずける・メイン
static const CURSORMOVE_DATA PartyOutMainCursorMoveData[] =
{
	{  40,  52, 0, 0,	 4,  2,  5,  1, {  68-12,  68+12-1, 42-12, 42+12-1 } },		// 00: ポケモン１
	{  80,  60, 0, 0,	 5,  3,  0,  2, {  76-12,  76+12-1, 78-12, 78+12-1 } },		// 01: ポケモン２
	{  40,  84, 0, 0,	 0,  4,  1,  3, { 100-12, 100+12-1, 42-12, 42+12-1 } },		// 02: ポケモン３
	{  80,  92, 0, 0,	 1,  5,  2,  4, { 108-12, 108+12-1, 78-12, 78+12-1 } },		// 03: ポケモン４
	{  40, 116, 0, 0,	 2,  0,  3,  5, { 132-12, 132+12-1, 42-12, 42+12-1 } },		// 04: ポケモン５
	{  80, 124, 0, 0,	 3,  1,  4,  0, { 140-12, 140+12-1, 78-12, 78+12-1 } },		// 05: ポケモン６

	{ 0, 0, 0, 0,	6, 6, 6, 6, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },			// 06: 戻る１
	{ 0, 0, 0, 0,	7, 7, 7, 7, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 07: 戻る２

	{ 212,  40, 0, 0,	12,  9,  8,  8, {  40,  63, 168, 255 } },		// 08: あずける
	{ 212,  64, 0, 0,	 8, 10,  9,  9, {  64,  87, 168, 255 } },		// 09: ようすをみる
	{ 212,  88, 0, 0,	 9, 11, 10, 10, {  88, 111, 168, 255 } },		// 10: マーキング
	{ 212, 112, 0, 0,	10, 12, 11, 11, { 112, 135, 168, 255 } },		// 11: にがす
	{ 212, 136, 0, 0,	11,  8, 12, 12, { 136, 159, 168, 255 } },		// 12: とじる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK PartyOutMainCallBack = {
//	PartyOutMainCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	CursorMoveCallBack_Move,
	PartyOutMainCallBack_Touch
};

// あずける・ボックス選択
static const CURSORMOVE_DATA PartyOutBoxSelCursorMoveData[] =
{
	{ 84, 16, 0, 0,	0, 0, 0, 0, {  17,  39,  26, 141 } },		// 00: トレイ名
	{  0,  0, 0, 0,	1, 1, 1, 1, {  51, 172,   3, 164 } },		// 01: トレイ
	{  0,  0, 0, 0,	2, 2, 2, 2, {  17,  38,   1,  22 } },		// 02: トレイ切り替え矢印・左
	{  0,  0, 0, 0,	3, 3, 3, 3, {  17,  38, 145, 166 } },		// 03: トレイ切り替え矢印・右
	{  0,  0, 0, 0,	4, 4, 4, 4, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 04: 戻る

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK PartyOutBoxSelCallBack = {
//	CursorMoveCallBack_Dummy,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	CursorMoveCallBack_Dummy,
	CursorMoveCallBack_Dummy,
/*
	PartyOutBoxSelCallBack_On,
	CursorMoveCallBack_Off,
	PartyOutBoxSelCallBack_Move,
	PartyOutBoxSelCallBack_Touch,
*/
};

// マーキング
static const CURSORMOVE_DATA MarkingCursorMoveData[] =
{
	{ 204,  42, 0, 0,	0, 2, 0, 1, { 44,  59, 188, 203 } },		// 00: ●
	{ 236,  42, 0, 0,	1, 3, 0, 1, { 44,  59, 220, 235 } },		// 01: ▲
	{ 204,  66, 0, 0,	0, 4, 2, 3, { 68,  83, 188, 203 } },		// 02: ■
	{ 236,  66, 0, 0,	1, 5, 2, 3, { 68,  83, 220, 235 } },		// 03: ハート
	{ 204,  90, 0, 0,	2, 6, 4, 5, { 92, 107, 188, 203 } },		// 04: ★
	{ 236,  90, 0, 0,	3, 6, 4, 5, { 92, 107, 220, 235 } },		// 05: ◆
	{ 212, 112, 0, 0,	CURSORMOVE_RETBIT|4, 7, 6, 6, { 112, 135, 168, 255 } },		// 06:「けってい」
	{ 212, 136, 0, 0,	6, 7, 7, 7, { 136, 159, 168, 255 } },		// 07:「やめる」

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK MarkingCallBack = {
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	CursorMoveCallBack_Move,
	CursorMoveCallBack_Touch
};

// つれていく・メイン
static const CURSORMOVE_DATA PartyInMainCursorMoveData[] =
{
	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  6, 5, 1, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },		// 00: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  7, 0, 2, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  8, 1, 3, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  9, 2, 4, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 10, 3, 5, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 11, 4, 0, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	0, 12, 11,  7, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 06: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	1, 13,  6,  8, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	2, 14,  7,  9, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	3, 15,  8, 10, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	4, 16,  9, 11, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	5, 17, 10,  6, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 6, 18, 17, 13, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 12: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 7, 19, 12, 14, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 8, 20, 13, 15, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 9, 21, 14, 16, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	10, 22, 15, 17, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	11, 23, 16, 12, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	12, 24, 23, 19, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 18: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	13, 25, 18, 20, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	14, 26, 19, 21, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	15, 27, 20, 22, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	16, 28, 21, 23, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	17, 29, 22, 18, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	18, 30, 29, 25, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 24: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	19, 30, 24, 26, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	20, 30, 25, 27, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	21, 30, 26, 28, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	22, 30, 27, 29, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	23, 30, 28, 24, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{  84,  16, 0, 0,	24, CURSORMOVE_RETBIT|0, 30, 30, {  17,  39,  26, 141 } },		// 30: ボックス名
	{   0,   0, 0, 0,	31, 31, 31, 31, {  17,  38,   1,  22 } },						// 31: トレイ切り替え矢印・左
	{   0,   0, 0, 0,	32, 32, 32, 32, {  17,  38, 145, 166 } },						// 32: トレイ切り替え矢印・右

	{ 0, 0, 0, 0,	33, 33, 33, 33, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },			// 33: 戻る１
	{ 0, 0, 0, 0,	34, 34, 34, 34, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 34: 戻る２

	{ 212,  40, 0, 0,	39, 36, 35, 35, {  40,  63, 168, 255 } },		// 35: つれていく
	{ 212,  64, 0, 0,	35, 37, 36, 36, {  64,  87, 168, 255 } },		// 36: ようすをみる
	{ 212,  88, 0, 0,	36, 38, 37, 37, {  88, 111, 168, 255 } },		// 37: マーキング
	{ 212, 112, 0, 0,	37, 39, 38, 38, { 112, 135, 168, 255 } },		// 38: にがす
	{ 212, 136, 0, 0,	38, 35, 39, 39, { 136, 159, 168, 255 } },		// 39: とじる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK PartyInMainCallBack = {
//	PartyInMainCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	CursorMoveCallBack_Move,
	PartyInMainCallBack_Touch,
};

// ボックステーマ変更
static const CURSORMOVE_DATA BoxThemaChgCursorMoveData[] =
{
	{ 212,  64, 0, 0,	3, 1, 0, 0, {  64,  87, 168, 255 } },		// 00: ジャンプする
	{ 212,  88, 0, 0,	0, 2, 1, 1, {  88, 111, 168, 255 } },		// 01: かべがみ
	{ 212, 112, 0, 0,	1, 3, 2, 2, { 112, 135, 168, 255 } },		// 02: なまえ
	{ 212, 136, 0, 0,	2, 0, 3, 3, { 136, 159, 168, 255 } },		// 03: やめる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BoxThemaChgCallBack = {
//	BoxThemaChgCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BoxThemaChgCallBack_Move,
	BoxThemaChgCallBack_Touch
};

// 壁紙変更
static const CURSORMOVE_DATA WallPaperChgCursorMoveData[] =
{
	{ 212,  40, 0, 0,	4, 1, 0, 0, {  40,  63, 168, 255 } },		// 00: メニュー１
	{ 212,  64, 0, 0,	0, 2, 1, 1, {  64,  87, 168, 255 } },		// 01: メニュー２
	{ 212,  88, 0, 0,	1, 3, 2, 2, {  88, 111, 168, 255 } },		// 02: メニュー３
	{ 212, 112, 0, 0,	2, 4, 3, 3, { 112, 135, 168, 255 } },		// 03: メニュー４
	{ 212, 136, 0, 0,	3, 0, 4, 4, { 136, 159, 168, 255 } },		// 04: もどる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK WallPaperChgCallBack = {
//	WallPaperChgCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	WallPaperChgCallBack_Move,
	WallPaperChgCallBack_Touch
};

// ボックス整理・メイン
static const CURSORMOVE_DATA BoxArrangeMainCursorMoveData[] =
{
	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  6, 5, 1, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },		// 00: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  7, 0, 2, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  8, 1, 3, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  9, 2, 4, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 10, 3, 5, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 11, 4, 0, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	0, 12, 11,  7, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 06: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	1, 13,  6,  8, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	2, 14,  7,  9, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	3, 15,  8, 10, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	4, 16,  9, 11, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	5, 17, 10,  6, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 6, 18, 17, 13, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 12: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 7, 19, 12, 14, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 8, 20, 13, 15, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 9, 21, 14, 16, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	10, 22, 15, 17, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	11, 23, 16, 12, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	12, 24, 23, 19, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 18: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	13, 25, 18, 20, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	14, 26, 19, 21, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	15, 27, 20, 22, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	16, 28, 21, 23, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	17, 29, 22, 18, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	18, 33, 29, 25, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 24: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	19, 33, 24, 26, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	20, 33, 25, 27, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	21, 33, 26, 28, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	22, 33, 27, 29, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	23, 33, 28, 24, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{  84,  16, 0, 0,	33, CURSORMOVE_RETBIT|0, 30, 30, {  17,  39,  26, 141 } },		// 30: ボックス名
	{   0,   0, 0, 0,	31, 31, 31, 31, {  17,  38,   1,  22 } },						// 31: トレイ切り替え矢印・左
	{   0,   0, 0, 0,	32, 32, 32, 32, {  17,  38, 145, 166 } },						// 32: トレイ切り替え矢印・右

	{  44, 168, 0, 0,	CURSORMOVE_RETBIT|24, 30, 33, 33, { 168, 191,   0,  95 } },		// 33: 手持ちポケモン

	{ 0, 0, 0, 0,	34, 34, 34, 34, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },			// 34: 戻る１
	{ 0, 0, 0, 0,	35, 35, 35, 35, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 35: 戻る２

	{ 212,  16, 0, 0,	41, 37, 36, 36, {  16,  39, 168, 255 } },	// 36: つかむ
	{ 212,  40, 0, 0,	36, 38, 37, 37, {  40,  63, 168, 255 } },	// 37: ようすをみる
	{ 212,  64, 0, 0,	37, 39, 38, 38, {  64,  87, 168, 255 } },	// 38: もちもの
	{ 212,  88, 0, 0,	38, 40, 39, 39, {  88, 111, 168, 255 } },	// 39: マーキング
	{ 212, 112, 0, 0,	39, 41, 40, 40, { 112, 135, 168, 255 } },	// 40: にがす
	{ 212, 136, 0, 0,	40, 36, 41, 41, { 136, 159, 168, 255 } },	// 41: とじる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BoxArrangeMainCallBack = {
//	BoxArrangeMainCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BoxArrangeMainCallBack_Move,
	BoxArrangeMainCallBack_Touch
};

// ボックス整理・ポケモン移動（ボックス間）
static const CURSORMOVE_DATA BoxArrangePokeMoveCursorMoveData[] =
{
	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  6, 34, 1, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 00: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  7,  0, 2, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  8,  1, 3, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  9,  2, 4, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 10,  3, 5, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 11,  4, CURSORMOVE_RETBIT|34, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	0, 12, 35,  7, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 06: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	1, 13,  6,  8, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	2, 14,  7,  9, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	3, 15,  8, 10, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	4, 16,  9, 11, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	5, 17, 10, CURSORMOVE_RETBIT|35, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 6, 18, 36, 13, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 12: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 7, 19, 12, 14, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 8, 20, 13, 15, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 9, 21, 14, 16, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	10, 22, 15, 17, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	11, 23, 16, CURSORMOVE_RETBIT|36, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	12, 24, 37, 19, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 18: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	13, 25, 18, 20, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	14, 26, 19, 21, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	15, 27, 20, 22, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	16, 28, 21, 23, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	17, 29, 22, CURSORMOVE_RETBIT|37, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	18, 39, 37, 25, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 24: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	19, 39, 24, 26, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	20, 39, 25, 27, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	21, 39, 26, 28, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	22, 39, 27, 29, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	23, 39, 28, CURSORMOVE_RETBIT|37, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ 84, 16, 0, 0,	39, CURSORMOVE_RETBIT|0, 30, 30, { 17, 39, 26, 141 } },		// 30: ボックス名
	{  0,  0, 0, 0,	31, 31, 31, 31, { 17, 38,   1,  22 } },		// 31: ボックス切り替え矢印（左）
	{  0,  0, 0, 0,	32, 32, 32, 32, { 17, 38, 145, 166 } },		// 32: ボックス切り替え矢印（右）

	{   0,   0, 0, 0,	33, 33, CURSORMOVE_RETBIT|33, 33, {   0,  11, 200, 223 } },		// 33: トレイアイコン
	{ 212,  22, 0, 0,	34, 35, CURSORMOVE_RETBIT| 5,  0, {  22,  45, 200, 223 } },		// 34: トレイアイコン
	{ 212,  56, 0, 0,	34, 36, CURSORMOVE_RETBIT|11,  6, {  56,  79, 200, 223 } },		// 35: トレイアイコン
	{ 212,  90, 0, 0,	35, 37, CURSORMOVE_RETBIT|17, 12, {  90, 113, 200, 223 } },		// 36: トレイアイコン
	{ 212, 124, 0, 0,	36, 37, CURSORMOVE_RETBIT|23, 18, { 124, 147, 200, 223 } },		// 37: トレイアイコン
	{   0,   0, 0, 0,	38, 38, CURSORMOVE_RETBIT|38, 38, { 158, 167, 200, 223 } },		// 38: トレイアイコン

	{ 44, 168, 0, 0,	CURSORMOVE_RETBIT|24, 30, 39, 39, { 168, 191, 0, 87 } },			// 39: 手持ちポケモン
	{  0,   0, 0, 0,	40, 40, 40, 40, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_03, TOUCHBAR_ICON_X_03+TOUCHBAR_ICON_WIDTH-1 } },		// 40: ステータス
	{  0,   0, 0, 0,	41, 41, 41, 41, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },		// 41: 戻る

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BoxArrangePokeMoveCallBack = {
//	BoxArrangePokeMoveCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BoxArrangePokeMoveCallBack_Move,
	BoxArrangePokeMoveCallBack_Touch
};

// ボックス整理・手持ちポケモン
static const CURSORMOVE_DATA BoxArrangePartyPokeCursorMoveData[] =
{
	{ 40,  52, 0, 0,	6, 2, 0, 1, {  68-12,  68+12-1, 42-12, 42+12-1 } },	// 00: ポケモン１
	{ 80,  60, 0, 0,	6, 3, 0, 2, {  76-12,  76+12-1, 78-12, 78+12-1 } },	// 01: ポケモン２
	{ 40,  84, 0, 0,	0, 4, 1, 3, { 100-12, 100+12-1, 42-12, 42+12-1 } },	// 02: ポケモン３
	{ 80,  92, 0, 0,	1, 5, 2, 4, { 108-12, 108+12-1, 78-12, 78+12-1 } },	// 03: ポケモン４
	{ 40, 116, 0, 0,	2, 6, 3, 5, { 132-12, 132+12-1, 42-12, 42+12-1 } },	// 04: ポケモン５
	{ 80, 124, 0, 0,	3, 6, 4, 6, { 140-12, 140+12-1, 78-12, 78+12-1 } },	// 05: ポケモン６

	{ 44, 168, 0, 0,	CURSORMOVE_RETBIT|5, 0, 5, 0, { 168, 191, 0, 95 } },		// 06:「ボックスリスト」

	{ 0, 0, 0, 0,	7, 7, 7, 7, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },			// 07: 戻る１
	{ 0, 0, 0, 0,	8, 8, 8, 8, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 08: 戻る２

	{ 212,  16, 0, 0,	14, 10,  9,  9, {  16,  39, 168, 255 } },	// 09: つかむ
	{ 212,  40, 0, 0,	 9, 11, 10, 10, {  40,  63, 168, 255 } },	// 10: ようすをみる
	{ 212,  64, 0, 0,	10, 12, 11, 11, {  64,  87, 168, 255 } },	// 11: もちもの
	{ 212,  88, 0, 0,	11, 13, 12, 12, {  88, 111, 168, 255 } },	// 12: マーキング
	{ 212, 112, 0, 0,	12, 14, 13, 13, { 112, 135, 168, 255 } },	// 13: にがす
	{ 212, 136, 0, 0,	13,  9, 14, 14, { 136, 159, 168, 255 } },	// 14: とじる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BoxArrangePartyPokeCallBack = {
//	BoxArrangePartyMoveCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BoxArrangePartyMoveCallBack_Move,
	BoxArrangePartyMoveCallBack_Touch,
};

// ボックス整理・ポケモン移動（手持ちポケモン）
static const CURSORMOVE_DATA BoxArrangePartyPokeMoveCursorMoveData[] =
{
	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	36,  6, 31, 1, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 00: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	36,  7,  0, 2, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	36,  8,  1, 3, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	36,  9,  2, 4, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	36, 10,  3, 5, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	36, 11,  4, CURSORMOVE_RETBIT|30, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	0, 12, 31,  7, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 06: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	1, 13,  6,  8, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	2, 14,  7,  9, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	3, 15,  8, 10, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	4, 16,  9, 11, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	5, 17, 10, CURSORMOVE_RETBIT|30, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 6, 18, 33, 13, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 12: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 7, 19, 12, 14, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 8, 20, 13, 15, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 9, 21, 14, 16, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	10, 22, 15, 17, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	11, 23, 16, CURSORMOVE_RETBIT|32, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	12, 24, 35, 19, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 18: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	13, 25, 18, 20, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	14, 26, 19, 21, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	15, 27, 20, 22, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	16, 28, 21, 23, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	17, 29, 22, CURSORMOVE_RETBIT|34, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	18, 39, 35, 25, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 24: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	19, 39, 24, 26, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	20, 39, 25, 27, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	21, 39, 26, 28, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	22, 39, 27, 29, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	23, 39, 28, CURSORMOVE_RETBIT|34, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ 192,  52, 0, 0,	34, 32, CURSORMOVE_RETBIT|5, 31, {  68-12,  68+12-1, 194-12, 194+12-1 } },		// 30: 手持ちポケモン
	{ 232,  60, 0, 0,	35, 33, 30, CURSORMOVE_RETBIT|0, {  76-12,  76+12-1, 230-12, 230+12-1 } },
	{ 192,  84, 0, 0,	30, 34, CURSORMOVE_RETBIT|17, 33, { 100-12, 100+12-1, 194-12, 194+12-1 } },
	{ 232,  92, 0, 0,	31, 35, 32, CURSORMOVE_RETBIT|12, { 108-12, 108+12-1, 230-12, 230+12-1 } },
	{ 192, 116, 0, 0,	32, 30, CURSORMOVE_RETBIT|23, 35, { 132-12, 132+12-1, 194-12, 194+12-1 } },
	{ 232, 124, 0, 0,	33, 31, 34, CURSORMOVE_RETBIT|24, { 140-12, 140+12-1, 230-12, 230+12-1 } },

	{ 84, 16, 0, 0,	39, CURSORMOVE_RETBIT|0, 36, 36, { 17, 39, 26, 141 } },		// 36: ボックス名
	{  0,  0, 0, 0,	37, 37, 37, 37, { 17, 38,   1,  22 } },		// 37: ボックス切り替え矢印（左）
	{  0,  0, 0, 0,	38, 38, 38, 38, { 17, 38, 145, 166 } },		// 38: ボックス切り替え矢印（右）

	{ 44, 168, 0, 0,	CURSORMOVE_RETBIT|24, 36, 39, 39, { 168, 191, 0, 87 } },			// 39: ボックスリスト
	{  0,   0, 0, 0,	40, 40, 40, 40, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_03, TOUCHBAR_ICON_X_03+TOUCHBAR_ICON_WIDTH-1 } },		// 40: ステータス
	{  0,   0, 0, 0,	41, 41, 41, 41, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },		// 41: 戻る

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BoxArrangePartyPokeMoveCallBack = {
//	BoxArrangePartyPokeMoveCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BoxArrangePartyPokeMoveCallBack_Move,
	BoxArrangePartyPokeMoveCallBack_Touch
};

// 持ち物整理・メイン
static const CURSORMOVE_DATA BoxItemArrangeMainCursorMoveData[] =
{
	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  6, 5, 1, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },		// 00: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  7, 0, 2, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  8, 1, 3, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  9, 2, 4, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 10, 3, 5, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 11, 4, 0, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	0, 12, 11,  7, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 06: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	1, 13,  6,  8, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	2, 14,  7,  9, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	3, 15,  8, 10, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	4, 16,  9, 11, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	5, 17, 10,  6, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 6, 18, 17, 13, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 12: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 7, 19, 12, 14, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 8, 20, 13, 15, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 9, 21, 14, 16, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	10, 22, 15, 17, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	11, 23, 16, 12, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	12, 24, 23, 19, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 18: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	13, 25, 18, 20, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	14, 26, 19, 21, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	15, 27, 20, 22, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	16, 28, 21, 23, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	17, 29, 22, 18, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	18, CURSORMOVE_RETBIT|33, 29, 25, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 24: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	19, CURSORMOVE_RETBIT|33, 24, 26, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	20, CURSORMOVE_RETBIT|33, 25, 27, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	21, CURSORMOVE_RETBIT|33, 26, 28, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	22, CURSORMOVE_RETBIT|33, 27, 29, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	23, CURSORMOVE_RETBIT|33, 28, 24, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{  84,  16, 0, 0,	33, CURSORMOVE_RETBIT|0, 30, 30, {  17,  39,  26, 141 } },		// 30: ボックス名
	{   0,   0, 0, 0,	31, 31, 31, 31, {  17,  38,   1,  22 } },						// 31: トレイ切り替え矢印・左
	{   0,   0, 0, 0,	32, 32, 32, 32, {  17,  38, 145, 166 } },						// 32: トレイ切り替え矢印・右

	{  44, 168, 0, 0,	CURSORMOVE_RETBIT|24, 30, 33, 33, { 168, 191,   0,  95 } },		// 33: 手持ちポケモン
	{ 0, 0, 0, 0,	34, 34, 34, 34, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },			// 34: 戻る１
	{ 0, 0, 0, 0,	35, 35, 35, 35, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 35: 戻る２

	{ 212,  88, 0, 0,	38, 37, 36, 36, {  88, 111, 168, 255 } },	// 36: いどうする
	{ 212, 112, 0, 0,	36, 38, 37, 37, { 112, 135, 168, 255 } },	// 37: バッグへ or もたせる
	{ 212, 136, 0, 0,	37, 36, 38, 38, { 136, 159, 168, 255 } },	// 38: とじる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BoxItemArrangeMainCallBack = {
//	BoxItemArrangeMainCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BoxItemArrangeMainCallBack_Move,
	BoxItemArrangeMainCallBack_Touch
};

// 持ち物整理・手持ちポケモン
static const CURSORMOVE_DATA BoxItemArrangePartyPokeCursorMoveData[] =
{
	{ 40,  52, 0, 0,	6, 2, 0, 1, {  68-12,  68+12-1, 42-12, 42+12-1 } },		// 00: ポケモン１
	{ 80,  60, 0, 0,	6, 3, 0, 2, {  76-12,  76+12-1, 78-12, 78+12-1 } },		// 01: ポケモン２
	{ 40,  84, 0, 0,	0, 4, 1, 3, { 100-12, 100+12-1, 42-12, 42+12-1 } },		// 02: ポケモン３
	{ 80,  92, 0, 0,	1, 5, 2, 4, { 108-12, 108+12-1, 78-12, 78+12-1 } },		// 03: ポケモン４
	{ 40, 116, 0, 0,	2, 6, 3, 5, { 132-12, 132+12-1, 42-12, 42+12-1 } },		// 04: ポケモン５
	{ 80, 124, 0, 0,	3, 6, 4, 6, { 140-12, 140+12-1, 78-12, 78+12-1 } },		// 05: ポケモン６

	{ 44, 168, 0, 0,	CURSORMOVE_RETBIT|5, 0, 5, 0, { 168, 191, 0, 95 } },	// 06: ボックスリスト

	{ 0, 0, 0, 0,	7, 7, 7, 7, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },			// 07: 戻る１
	{ 0, 0, 0, 0,	8, 8, 8, 8, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 08: 戻る２

	{ 212,  88, 0, 0,	11, 10,  9,  9, {  88, 111, 168, 255 } },		// 09: いどうする
	{ 212, 112, 0, 0,	 9, 11, 10, 10, { 112, 135, 168, 255 } },		// 10: バッグへ or もたせる
	{ 212, 136, 0, 0,	10,  9, 11, 11, { 136, 159, 168, 255 } },		// 11: とじる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BoxItemArrangePartyPokeCallBack = {
//	BoxItemArrangePartyMoveCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BoxItemArrangePartyMoveCallBack_Move,
	BoxItemArrangePartyMoveCallBack_Touch,
};

// ボックスジャンプ
static const CURSORMOVE_DATA BoxJumpCursorMoveData[] =
{
	{   0,   0, 0, 0,	0, 0, 0, 0, {   0,  11, 200, 223 } },		// 00: トレイアイコン
	{ 212,  22, 0, 0,	1, 2, 1, 1, {  22,  45, 200, 223 } },		// 01: トレイアイコン
	{ 212,  56, 0, 0,	1, 3, 2, 2, {  56,  79, 200, 223 } },		// 02: トレイアイコン
	{ 212,  90, 0, 0,	2, 4, 3, 3, {  90, 113, 200, 223 } },		// 03: トレイアイコン
	{ 212, 124, 0, 0,	3, 4, 4, 4, { 124, 147, 200, 223 } },		// 04: トレイアイコン
	{   0,   0, 0, 0,	5, 5, 5, 5, { 158, 167, 200, 223 } },		// 05: トレイアイコン

	{ 0, 0, 0, 0,	6, 6, 6, 6, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },		// 06: 戻る

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BoxJumpCallBack = {
//	BoxJumpCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BoxJumpCallBack_Move,
	BoxJumpCallBack_Touch
};

// バトルボックス・メイン
static const CURSORMOVE_DATA BattleBoxMainCursorMoveData[] =
{
	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  6, 5, 1, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },		// 00: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  7, 0, 2, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  8, 1, 3, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  9, 2, 4, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 10, 3, 5, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 11, 4, 0, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	0, 12, 11,  7, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 06: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	1, 13,  6,  8, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	2, 14,  7,  9, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	3, 15,  8, 10, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	4, 16,  9, 11, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	5, 17, 10,  6, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 6, 18, 17, 13, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 12: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 7, 19, 12, 14, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 8, 20, 13, 15, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 9, 21, 14, 16, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	10, 22, 15, 17, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	11, 23, 16, 12, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	12, 24, 23, 19, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 18: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	13, 25, 18, 20, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	14, 26, 19, 21, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	15, 27, 20, 22, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	16, 28, 21, 23, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	17, 29, 22, 18, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	18, 33, 29, 25, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 24: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	19, 33, 24, 26, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	20, 33, 25, 27, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	21, 33, 26, 28, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	22, 33, 27, 29, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	23, 33, 28, 24, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{  84,  16, 0, 0,	33, CURSORMOVE_RETBIT|0, 30, 30, {  17,  39,  26, 141 } },		// 30: ボックス名
	{   0,   0, 0, 0,	31, 31, 31, 31, {  17,  38,   1,  22 } },						// 31: トレイ切り替え矢印・左
	{   0,   0, 0, 0,	32, 32, 32, 32, {  17,  38, 145, 166 } },						// 32: トレイ切り替え矢印・右

	{  44, 168, 0, 0,	CURSORMOVE_RETBIT|24, 30, 33, 33, { 168, 191,   0,  95 } },		// 33: バトルボックス

	{ 0, 0, 0, 0,	34, 34, 34, 34, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },			// 34: 戻る１
	{ 0, 0, 0, 0,	35, 35, 35, 35, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 35: 戻る２

	{ 212,  64, 0, 0,	39, 37, 36, 36, {  64,  87, 168, 255 } },	// 36: いどうする
	{ 212,  88, 0, 0,	36, 38, 37, 37, {  88, 111, 168, 255 } },	// 37: ようすをみる
	{ 212, 112, 0, 0,	37, 39, 38, 38, { 112, 135, 168, 255 } },	// 38: もちもの
	{ 212, 136, 0, 0,	38, 36, 39, 39, { 136, 159, 168, 255 } },	// 39: やめる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BattleBoxMainCallBack = {
//	BattleBoxMainCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BattleBoxMainCallBack_Move,
	BattleBoxMainCallBack_Touch
};

// バトルボックス・パーティ操作
static const CURSORMOVE_DATA BattleBoxPartyMainCursorMoveData[] =
{
	{ 40,  52, 0, 0,	6, 2, 0, 1, {  68-12,  68+12-1, 42-12, 42+12-1 } },	// 00: ポケモン１
	{ 80,  60, 0, 0,	6, 3, 0, 2, {  76-12,  76+12-1, 78-12, 78+12-1 } },	// 01: ポケモン２
	{ 40,  84, 0, 0,	0, 4, 1, 3, { 100-12, 100+12-1, 42-12, 42+12-1 } },	// 02: ポケモン３
	{ 80,  92, 0, 0,	1, 5, 2, 4, { 108-12, 108+12-1, 78-12, 78+12-1 } },	// 03: ポケモン４
	{ 40, 116, 0, 0,	2, 6, 3, 5, { 132-12, 132+12-1, 42-12, 42+12-1 } },	// 04: ポケモン５
	{ 80, 124, 0, 0,	3, 6, 4, 6, { 140-12, 140+12-1, 78-12, 78+12-1 } },	// 05: ポケモン６

	{ 44, 168, 0, 0,	CURSORMOVE_RETBIT|5, 0, 5, 0, { 168, 191, 0, 95 } },		// 06:「ボックスリスト」

	{ 0, 0, 0, 0,	7, 7, 7, 7, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+TOUCHBAR_ICON_WIDTH-1 } },			// 07: 戻る１
	{ 0, 0, 0, 0,	8, 8, 8, 8, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 08: 戻る２

	{ 212,  64, 0, 0,	12, 10,  9,  9, {  64,  87, 168, 255 } },	// 09: いどうする
	{ 212,  88, 0, 0,	 9, 11, 10, 10, {  88, 111, 168, 255 } },	// 10: ようすをみる
	{ 212, 112, 0, 0,	10, 12, 11, 11, { 112, 135, 168, 255 } },	// 11: もちもの
	{ 212, 136, 0, 0,	11,  9, 12, 12, { 136, 159, 168, 255 } },	// 12: とじる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK BattleBoxPartyMainCallBack = {
//	BattleBoxPartyMainCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	BattleBoxPartyMainCallBack_Move,
	BattleBoxPartyMainCallBack_Touch,
};

// 寝かせる・メイン
static const CURSORMOVE_DATA SleepMainCursorMoveData[] =
{
	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  6, 5, 1, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },		// 00: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  7, 0, 2, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  8, 1, 3, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30,  9, 2, 4, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 10, 3, 5, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(0)+PICUR_Y, 0, 0,	30, 11, 4, 0, { TRAYPOKE_PY(0), TRAYPOKE_PY(1)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	0, 12, 11,  7, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 06: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	1, 13,  6,  8, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	2, 14,  7,  9, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	3, 15,  8, 10, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	4, 16,  9, 11, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(1)+PICUR_Y, 0, 0,	5, 17, 10,  6, { TRAYPOKE_PY(1), TRAYPOKE_PY(2)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 6, 18, 17, 13, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 12: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 7, 19, 12, 14, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 8, 20, 13, 15, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	 9, 21, 14, 16, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	10, 22, 15, 17, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(2)+PICUR_Y, 0, 0,	11, 23, 16, 12, { TRAYPOKE_PY(2), TRAYPOKE_PY(3)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	12, 24, 23, 19, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 18: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	13, 25, 18, 20, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	14, 26, 19, 21, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	15, 27, 20, 22, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	16, 28, 21, 23, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(3)+PICUR_Y, 0, 0,	17, 29, 22, 18, { TRAYPOKE_PY(3), TRAYPOKE_PY(4)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{ TRAYPOKE_PX(0)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	18, 30, 29, 25, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(0),	TRAYPOKE_PX(1)-1 } },	// 24: ポケモン
	{ TRAYPOKE_PX(1)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	19, 30, 24, 26, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(1),	TRAYPOKE_PX(2)-1 } },
	{ TRAYPOKE_PX(2)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	20, 30, 25, 27, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(2),	TRAYPOKE_PX(3)-1 } },
	{ TRAYPOKE_PX(3)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	21, 30, 26, 28, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(3),	TRAYPOKE_PX(4)-1 } },
	{ TRAYPOKE_PX(4)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	22, 30, 27, 29, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(4),	TRAYPOKE_PX(5)-1 } },
	{ TRAYPOKE_PX(5)+PICUR_X,TRAYPOKE_PY(4)+PICUR_Y, 0, 0,	23, 30, 28, 24, { TRAYPOKE_PY(4), TRAYPOKE_PY(5)-1, TRAYPOKE_PX(5),	TRAYPOKE_PX(6)-1 } },

	{  84,  16, 0, 0,	24, CURSORMOVE_RETBIT|0, 30, 30, {  17,  39,  26, 141 } },		// 30: ボックス名
	{   0,   0, 0, 0,	31, 31, 31, 31, {  17,  38,   1,  22 } },						// 31: トレイ切り替え矢印・左
	{   0,   0, 0, 0,	32, 32, 32, 32, {  17,  38, 145, 166 } },						// 32: トレイ切り替え矢印・右

	{ 0, 0, 0, 0,	33, 33, 33, 33, { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1 } },			// 33: 戻る

	{ 212, 112, 0, 0,	35, 35, 34, 34, { 112, 135, 168, 255 } },	// 34: ねかせる
	{ 212, 136, 0, 0,	34, 34, 35, 35, { 136, 159, 168, 255 } },	// 35: とじる

	{ 0, 0, 0, 0,	0, 0, 0, 0,	{ GFL_UI_TP_HIT_END, 0, 0, 0 } }
};
static const CURSORMOVE_CALLBACK SleepMainCallBack = {
//	SleepMainCallBack_On,
	CursorMoveCallBack_On,
	CursorMoveCallBack_Off,
	SleepMainCallBack_Move,
	SleepMainCallBack_Touch
};

// カーソル移動データテーブル
static const MOVE_DATA_TBL MoveDataTable[] =
{
	{	// あずける・メイン
		PartyOutMainCursorMoveData,
		&PartyOutMainCallBack
	},
	{	// あずける・ボックス選択
		PartyOutBoxSelCursorMoveData,
		&PartyOutBoxSelCallBack
	},

	{	// つれていく・メイン
		PartyInMainCursorMoveData,
		&PartyInMainCallBack
	},

	{	// ボックス整理・メイン
		BoxArrangeMainCursorMoveData,
		&BoxArrangeMainCallBack
	},
	{	// ボックス整理・ポケモン移動（ボックス間）
		BoxArrangePokeMoveCursorMoveData,
		&BoxArrangePokeMoveCallBack
	},
	{	// ボックス整理・手持ちポケモン
		BoxArrangePartyPokeCursorMoveData,
		&BoxArrangePartyPokeCallBack
	},
	{	// ボックス整理・ポケモン移動（手持ち）
		BoxArrangePartyPokeMoveCursorMoveData,
		&BoxArrangePartyPokeMoveCallBack
	},

	{	// 持ち物整理・メイン
		BoxItemArrangeMainCursorMoveData,
		&BoxItemArrangeMainCallBack
	},
	{	// 持ち物整理・手持ちポケモン
		BoxItemArrangePartyPokeCursorMoveData,
		&BoxItemArrangePartyPokeCallBack
	},

	{	// マーキング
		MarkingCursorMoveData,
		&MarkingCallBack
	},

	{	// ボックステーマ変更
		BoxThemaChgCursorMoveData,
		&BoxThemaChgCallBack
	},

	{	// 壁紙変更
		WallPaperChgCursorMoveData,
		&WallPaperChgCallBack
	},

	{	// ボックスジャンプ
		BoxJumpCursorMoveData,
		&BoxJumpCallBack
	},

	{	// バトルボックス・メイン
		BattleBoxMainCursorMoveData,
		&BattleBoxMainCallBack
	},
	{	// バトルボックス・パーティ操作
		BattleBoxPartyMainCursorMoveData,
		&BattleBoxPartyMainCallBack
	},

	{	// ねかせる・メイン
		SleepMainCursorMoveData,
		&SleepMainCallBack
	},
};

// タッチチェックの終了データ
static const GFL_UI_TP_HITTBL TpCheckEnd = { GFL_UI_TP_HIT_END, 0, 0, 0 };

// トレイアイコン上下スクロール座標
static const GFL_UI_TP_HITTBL PokeGetTrayScrollHitTbl[] =
{
	{   0,  21, 173, 255 },
	{ 148, 167, 173, 255 },
	{ GFL_UI_TP_HIT_END, 0, 0, 0 }
};

// トレイアイコンスクロール開始座標
static const GFL_UI_TP_HITTBL TrayScrollStartHitTbl[] =
{
	{ 0, 167, 228, 255 },
	{ 0, 167, 168, 195 },
	{ GFL_UI_TP_HIT_END, 0, 0, 0 }
};

// トレイアイコンスクロール継続座標
static const GFL_UI_TP_HITTBL TrayScrollHitTbl[] =
{
	{ 0, 191, 168, 255 },
	{ GFL_UI_TP_HIT_END, 0, 0, 0 }
};

// 手持ちフレーム（左）座標データ
static const GFL_UI_TP_HITTBL PartyLeftHitTbl[] =
{
	{ PARTY_FRM_UPY, PARTY_FRM_DPY, PARTY_FRM_LLX, PARTY_FRM_LRX },
	{ GFL_UI_TP_HIT_END, 0, 0, 0 }
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動ワーク初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2UI_CursorMoveInit( BOX2_SYS_WORK * syswk )
{
	BOOL	flg;
	u32		id;

	// カーソル初期状態
	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		flg = FALSE;
	}else{
		flg = TRUE;
	}

	switch( syswk->dat->callMode ){
	case BOX_MODE_AZUKERU:
		id = BOX2UI_INIT_ID_PTOUT_MAIN;
		break;
	case BOX_MODE_TURETEIKU:
		id = BOX2UI_INIT_ID_PTIN_MAIN;
		break;
	case BOX_MODE_SEIRI:
		id = BOX2UI_INIT_ID_ARRANGE_MAIN;
		break;
	case BOX_MODE_ITEM:
		id = BOX2UI_INIT_ID_ITEM_MAIN;
		break;
	case BOX_MODE_BATTLE:
		id = BOX2UI_INIT_ID_BATTLEBOX_MAIN;
		break;
	case BOX_MODE_SLEEP:
		id = BOX2UI_INIT_ID_SLEEP_MAIN;
		break;
	}

	BOX2UI_CursorMoveExit( syswk );

	syswk->app->cmwk = CURSORMOVE_Create(
							MoveDataTable[id].dat,
							MoveDataTable[id].cb,
							syswk,
							flg,
							syswk->cur_rcv_pos,			// pos
							HEAPID_BOX_APP );

	CURSORMOVE_VanishModeSet( syswk->app->cmwk );

	CursorObjMove( syswk, syswk->cur_rcv_pos );
	BOX2OBJ_SetHandCursorOnOff( syswk, flg );

	syswk->app->old_cur_pos = syswk->cur_rcv_pos;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動ワーク切り替え
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		id			カーソル移動データ番号
 * @param		pos			カーソル初期位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2UI_CursorMoveChange( BOX2_SYS_WORK * syswk, u32 id, u32 pos )
{
	BOOL	flg = CURSORMOVE_CursorOnOffGet( syswk->app->cmwk );

	BOX2UI_CursorMoveExit( syswk );

	syswk->app->cmwk = CURSORMOVE_Create(
							MoveDataTable[id].dat,
							MoveDataTable[id].cb,
							syswk,
							flg,
							pos,					// pos
							HEAPID_BOX_APP );

	CURSORMOVE_VanishModeSet( syswk->app->cmwk );

	CursorObjMove( syswk, pos );
	BOX2OBJ_SetHandCursorOnOff( syswk, flg );

	syswk->app->old_cur_pos = pos;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動ワーク削除
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2UI_CursorMoveExit( BOX2_SYS_WORK * syswk )
{
	if( syswk->app->cmwk != NULL ){
		CURSORMOVE_Exit( syswk->app->cmwk );
		syswk->app->cmwk = NULL;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置にカーソルＯＢＪを配置
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2UI_PutHandCursor( BOX2_SYS_WORK * syswk, int pos )
{
	const CURSORMOVE_DATA * pw;
	u8	x, y;

	pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, pos );

//	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	x = pw->px;
	y = pw->py;
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソルＯＢＪ移動
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CursorObjMove( BOX2_SYS_WORK * syswk, int pos )
{
	BOX2UI_PutHandCursor( syswk, pos );
	BOX2OBJ_MovePokeIconHand( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動汎用コールバック：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

//	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動汎用コールバック：カーソル非表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動汎用コールバック：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );

	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動汎用コールバック：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動汎用コールバック：ダミー処理
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Dummy( void * work, int now_pos, int old_pos )
{
}


//============================================================================================
//	預けるメイン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをあずける」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_PartyOutMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u8	now;

	now = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	if( ret < NELEMS(PartyOutMainCursorMoveData) ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			if( ret >= BOX2UI_PTOUT_MAIN_AZUKERU && ret <= BOX2UI_PTOUT_MAIN_CLOSE ){
				CURSORMOVE_PosSet( syswk->app->cmwk, now );
				CursorObjMove( syswk, now );
				return CURSORMOVE_NONE;
			}
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
				GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
				return BOX2UI_PTOUT_MAIN_RETURN1;
			}
			if( ret == BOX2UI_PTOUT_MAIN_RETURN1 ){
				GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
				return BOX2UI_PTOUT_MAIN_RETURN1;
			}
			if( ret == BOX2UI_PTOUT_MAIN_RETURN2 ){
				CURSORMOVE_PosSet( syswk->app->cmwk, now );
				return BOX2UI_PTOUT_MAIN_RETURN2;
			}
		}else{
			if( ret == BOX2UI_PTOUT_MAIN_RETURN1 || ret == BOX2UI_PTOUT_MAIN_RETURN2 ){
				CURSORMOVE_PosSet( syswk->app->cmwk, now );
				CursorObjMove( syswk, now );
				return CURSORMOVE_NONE;
			}
		}
		return ret;
	}

	if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
		if( ret == CURSORMOVE_CANCEL ){
			return BOX2UI_PTOUT_MAIN_CLOSE;
		}
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをあずける」メイン操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void PartyOutMainCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( old_pos != CURSORMOVE_ONOFF_DIRECT ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			if( syswk->get_pos != BOX2MAIN_GETPOS_NONE &&
					syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				now_pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			}else{
				now_pos = BOX2UI_PTOUT_MAIN_POKE1;
			}
		}else{
			if( now_pos != BOX2UI_PTOUT_MAIN_AZUKERU &&
					now_pos != BOX2UI_PTOUT_MAIN_STATUS &&
					now_pos != BOX2UI_PTOUT_MAIN_MARKING &&
					now_pos != BOX2UI_PTOUT_MAIN_FREE &&
					now_pos != BOX2UI_PTOUT_MAIN_CLOSE ){
				now_pos = BOX2UI_PTOUT_MAIN_AZUKERU;
			}
		}
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをあずける」メイン操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PartyOutMainCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_PTOUT_MAIN_RETURN1 ||		// 06: 戻る１
			now_pos == BOX2UI_PTOUT_MAIN_RETURN2 ){		// 07: 戻る２
	}else{
		CursorObjMove( syswk, now_pos );
	}
}


//============================================================================================
//	預ける・トレイ選択
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをあずける」トレイ選択：指定位置からボックス名への移動処理セット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2UI_PartyOutBoxSelCursorMoveSet( BOX2_SYS_WORK * syswk, u32 pos )
{
	const CURSORMOVE_DATA * opw;
	const CURSORMOVE_DATA * npw;

	opw = CURSORMOVE_GetMoveData( syswk->app->cmwk, pos );
	npw = &PartyOutBoxSelCursorMoveData[ BOX2UI_PTOUT_BOXSEL_NAME ];

	BOX2UI_CursorMoveVFuncWorkSetDirect( syswk->app, npw->px, npw->py, opw->px, opw->py );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをあずける」トレイ選択：ボックス名から指定位置への移動処理セット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2UI_PartyOutBoxSelCursorMovePutSet( BOX2_SYS_WORK * syswk, u32 pos )
{
	const CURSORMOVE_DATA * opw;
	const CURSORMOVE_DATA * npw;

	opw = &PartyOutBoxSelCursorMoveData[ BOX2UI_PTOUT_BOXSEL_NAME ];

	if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		npw = &BoxArrangeMainCursorMoveData[ pos ];
	}else{
		npw = &PartyOutMainCursorMoveData[ pos-BOX2OBJ_POKEICON_TRAY_MAX ];
	}

	BOX2UI_CursorMoveVFuncWorkSetDirect( syswk->app, npw->px, npw->py, opw->px, opw->py );
}


//============================================================================================
//	連れて行くメイン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをつれていく」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_PartyInMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u8	now;

	now = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	if( ret < NELEMS(PartyInMainCursorMoveData) ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			if( ret >= BOX2UI_PTIN_MAIN_TSURETEIKU && ret <= BOX2UI_PTIN_MAIN_CLOSE ){
				CURSORMOVE_PosSet( syswk->app->cmwk, now );
				CursorObjMove( syswk, now );
				return CURSORMOVE_NONE;
			}
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
				GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
				return BOX2UI_PTIN_MAIN_RETURN1;
			}
			if( ret == BOX2UI_PTIN_MAIN_RETURN1 ){
				GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
				return BOX2UI_PTIN_MAIN_RETURN1;
			}
			if( ret == BOX2UI_PTIN_MAIN_RETURN2 ){
				CURSORMOVE_PosSet( syswk->app->cmwk, now );
				return BOX2UI_PTIN_MAIN_RETURN2;
			}
		}else{
			if( ret == BOX2UI_PTIN_MAIN_RETURN1 ||
					ret == BOX2UI_PTIN_MAIN_RETURN2 ){
				CURSORMOVE_PosSet( syswk->app->cmwk, now );
				CursorObjMove( syswk, now );
				return CURSORMOVE_NONE;
			}
		}
		return ret;
	}

	if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == TRUE ){
		if( ret == CURSORMOVE_CANCEL ){
			return BOX2UI_PTIN_MAIN_CLOSE;
		}
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをつれていく」メイン操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void PartyInMainCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( old_pos != CURSORMOVE_ONOFF_DIRECT ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			switch( now_pos ){
			case BOX2UI_PTIN_MAIN_LEFT:
			case BOX2UI_PTIN_MAIN_RIGHT:
				now_pos = BOX2UI_PTIN_MAIN_NAME;
				break;

			case BOX2UI_PTIN_MAIN_NAME:
				break;

			default:
				if( syswk->get_pos != BOX2MAIN_GETPOS_NONE && syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					now_pos = syswk->get_pos;
				}else{
					now_pos = BOX2UI_PTIN_MAIN_POKE1;
				}
			}
		}else{
			if( now_pos != BOX2UI_PTIN_MAIN_TSURETEIKU &&
					now_pos != BOX2UI_PTIN_MAIN_STATUS &&
					now_pos != BOX2UI_PTIN_MAIN_MARKING &&
					now_pos != BOX2UI_PTIN_MAIN_FREE &&
					now_pos != BOX2UI_PTIN_MAIN_CLOSE ){
				now_pos = BOX2UI_PTIN_MAIN_TSURETEIKU;
			}
		}
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをつれていく」メイン操作コールバック関数：タッチ時
 *
 * @param		syswk			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PartyInMainCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_PTIN_MAIN_RETURN1 ||
			now_pos == BOX2UI_PTIN_MAIN_RETURN2 ){
	}else{
		CursorObjMove( syswk, now_pos );
	}
}


//============================================================================================
//	ボックス整理メイン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」メイン操作
 *
 * @param		syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_BoxArrangeMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u8	now;

	now = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	// メニュー非表示
	if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
		if( ret >= BOX2UI_ARRANGE_MAIN_GET && ret <= BOX2UI_ARRANGE_MAIN_CLOSE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
			return BOX2UI_ARRANGE_MAIN_RETURN1;
		}
		if( ret == BOX2UI_ARRANGE_MAIN_RETURN1 ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
			return BOX2UI_ARRANGE_MAIN_RETURN1;
		}
		if( ret == BOX2UI_ARRANGE_MAIN_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			return BOX2UI_ARRANGE_MAIN_RETURN2;
		}
	// メニュー表示中
	}else{
		if( ret == BOX2UI_ARRANGE_MAIN_PARTY ||
				ret == BOX2UI_ARRANGE_MAIN_RETURN1 ||
				ret == BOX2UI_ARRANGE_MAIN_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( ret == CURSORMOVE_CANCEL ){
			return BOX2UI_ARRANGE_MAIN_CLOSE;
		}
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」メイン操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxArrangeMainCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk;
	const CURSORMOVE_DATA * pw;
	
	syswk = work;

	if( old_pos != CURSORMOVE_ONOFF_DIRECT ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			switch( now_pos ){
			case BOX2UI_ARRANGE_MAIN_LEFT:
			case BOX2UI_ARRANGE_MAIN_RIGHT:
				now_pos = BOX2UI_ARRANGE_MAIN_NAME;
				break;

			case BOX2UI_ARRANGE_MAIN_NAME:
			case BOX2UI_ARRANGE_MAIN_PARTY:
			case BOX2UI_ARRANGE_MAIN_RETURN1:
			case BOX2UI_ARRANGE_MAIN_RETURN2:
				break;

			default:
				if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					now_pos = syswk->get_pos;
				}else{
					now_pos = BOX2UI_ARRANGE_MAIN_POKE1;
				}
			}
		}else{
			if( now_pos != BOX2UI_ARRANGE_MAIN_GET ||
				now_pos != BOX2UI_ARRANGE_MAIN_STATUS ||
				now_pos != BOX2UI_ARRANGE_MAIN_ITEM ||
				now_pos != BOX2UI_ARRANGE_MAIN_MARKING ||
				now_pos != BOX2UI_ARRANGE_MAIN_FREE ||
				now_pos != BOX2UI_ARRANGE_MAIN_CLOSE ){

				now_pos = BOX2UI_ARRANGE_MAIN_GET;
			}
		}
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」メイン操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxArrangeMainCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );

	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」メイン操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxArrangeMainCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_ARRANGE_MAIN_RETURN1 ||
			now_pos == BOX2UI_ARRANGE_MAIN_RETURN2 ){
	}else{
		CursorObjMove( syswk, now_pos );
	}
}


//============================================================================================
//	ボックス整理・ポケモン移動
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン取得時のタッチバーボタン設定
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			現在の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetTouchBarIconPokeGet( BOX2_SYS_WORK * syswk, u32 pos )
{
	u8	ret, status;

	ret    = BOX2OBJ_TB_ICON_ON;
	status = BOX2OBJ_TB_ICON_ON;

	if( syswk->poke_get_key == 1 ){
		ret = BOX2OBJ_TB_ICON_PASSIVE;
		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			status = BOX2OBJ_TB_ICON_OFF;
		}
	}else{
		if( syswk->dat->callMode == BOX_MODE_ITEM ){
			status = BOX2OBJ_TB_ICON_OFF;
		}else if( pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			status = BOX2OBJ_TB_ICON_PASSIVE;
		}else if( BOX2MAIN_PokeParaGet(syswk,pos,syswk->tray,ID_PARA_poke_exist,NULL) == 0 ){
			status = BOX2OBJ_TB_ICON_PASSIVE;
		}
	}
	BOX2OBJ_SetTouchBarButton( syswk, ret, BOX2OBJ_TB_ICON_OFF, status );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxArrangePokeMoveCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk;

	syswk = work;

//	CursorObjMove( syswk, now_pos );

	if( now_pos == BOX2UI_ARRANGE_PGT_LEFT ||			// 31: ボックス切り替え矢印（左）
			now_pos == BOX2UI_ARRANGE_PGT_RIGHT ||		// 32: ボックス切り替え矢印（右）
			now_pos == BOX2UI_ARRANGE_PGT_TRAY1 ||		// 33: トレイアイコン
			now_pos == BOX2UI_ARRANGE_PGT_TRAY6 ||		// 38: トレイアイコン
			now_pos == BOX2UI_ARRANGE_PGT_STATUS ||		// 40: ステータス
			now_pos == BOX2UI_ARRANGE_PGT_RET ){			// 41: 戻る
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->app->old_cur_pos );
		now_pos = syswk->app->old_cur_pos;
	}else{
		CursorObjMove( syswk, now_pos );
	}

	SetTouchBarIconPokeGet( syswk, now_pos );

	syswk->app->old_cur_pos = now_pos;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxArrangePokeMoveCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	SetTouchBarIconPokeGet( syswk, now_pos );

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );
	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );

	syswk->app->old_cur_pos = now_pos;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxArrangePokeMoveCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_ARRANGE_PGT_LEFT ||			// 31: ボックス切り替え矢印（左）
			now_pos == BOX2UI_ARRANGE_PGT_RIGHT ||		// 32: ボックス切り替え矢印（右）
			now_pos == BOX2UI_ARRANGE_PGT_TRAY1 ||		// 33: トレイアイコン
			now_pos == BOX2UI_ARRANGE_PGT_TRAY6 ||		// 38: トレイアイコン
			now_pos == BOX2UI_ARRANGE_PGT_STATUS ||		// 40: ステータス
			now_pos == BOX2UI_ARRANGE_PGT_RET ){			// 41: 戻る
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->app->old_cur_pos );
		now_pos = syswk->app->old_cur_pos;
	}else{
		CursorObjMove( syswk, now_pos );
	}

	SetTouchBarIconPokeGet( syswk, now_pos );

	syswk->app->old_cur_pos = now_pos;
}


//============================================================================================
//	ボックス整理・掴む
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動時にアイコンを掴んでいるときの操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_ArrangePokeGetMain( BOX2_SYS_WORK * syswk )
{
	return CURSORMOVE_MainCont( syswk->app->cmwk );
}


//============================================================================================
//	ボックス整理・手持ちメイン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」手持ちポケモン操作
 *
 * @param		syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_BoxArrangePartyMoveMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u8	now;

	now = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == FALSE ){
		if( ret >= BOX2UI_ARRANGE_PARTY_GET && ret <= BOX2UI_ARRANGE_PARTY_CLOSE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
			return BOX2UI_ARRANGE_PARTY_RETURN1;
		}
		if( ret == BOX2UI_ARRANGE_PARTY_RETURN1 ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
			return BOX2UI_ARRANGE_PARTY_RETURN1;
		}
		if( ret == BOX2UI_ARRANGE_PARTY_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			return BOX2UI_ARRANGE_PARTY_RETURN2;
		}
	}else{
		if( ret == BOX2UI_ARRANGE_PARTY_BOXLIST ||
				ret == BOX2UI_ARRANGE_PARTY_RETURN1 ||
				ret == BOX2UI_ARRANGE_PARTY_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}

		if( ret == CURSORMOVE_CANCEL ){
			return BOX2UI_ARRANGE_PARTY_CLOSE;
		}
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」手持ちポケモン操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxArrangePartyMoveCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk;
	const CURSORMOVE_DATA * pw;
	
	syswk = work;

	if( old_pos != CURSORMOVE_ONOFF_DIRECT ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			if( now_pos >= BOX2UI_ARRANGE_PARTY_BOXLIST ){
				if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					now_pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				}else{
					now_pos = BOX2UI_ARRANGE_PARTY_POKE1;
				}
			}
		}else{
			if( now_pos != BOX2UI_ARRANGE_PARTY_GET ||
				now_pos != BOX2UI_ARRANGE_PARTY_STATUS ||
				now_pos != BOX2UI_ARRANGE_PARTY_ITEM ||
				now_pos != BOX2UI_ARRANGE_PARTY_MARKING ||
				now_pos != BOX2UI_ARRANGE_PARTY_FREE ||
				now_pos != BOX2UI_ARRANGE_PARTY_CLOSE ){

				now_pos = BOX2UI_ARRANGE_PARTY_GET;
			}
		}
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」手持ちポケモン操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxArrangePartyMoveCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );

	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」手持ちポケモン操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxArrangePartyMoveCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_ARRANGE_PARTY_RETURN1 ||		// 06: 戻る１
			now_pos == BOX2UI_ARRANGE_PARTY_RETURN2 ){		// 07: 戻る２
	}else{
		CursorObjMove( syswk, now_pos );
	}
}

//============================================================================================
//	ボックス整理・手持ち・掴む
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動時にアイコンを掴んでいるときの操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_ArrangePartyPokeGetMain( BOX2_SYS_WORK * syswk )
{
	return CURSORMOVE_MainCont( syswk->app->cmwk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxArrangePartyPokeMoveCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

//	CursorObjMove( syswk, now_pos );

	if( now_pos == BOX2UI_ARRANGE_PTGT_LEFT ||		// 31: ボックス切り替え矢印（左）
			now_pos == BOX2UI_ARRANGE_PTGT_RIGHT ||		// 32: ボックス切り替え矢印（右）
			now_pos == BOX2UI_ARRANGE_PTGT_STATUS ||	// 40: ステータス
			now_pos == BOX2UI_ARRANGE_PTGT_RET ){			// 41: 戻る
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->app->old_cur_pos );
		now_pos = syswk->app->old_cur_pos;
	}else{
		CursorObjMove( syswk, now_pos );
	}

	SetTouchBarIconPokeGet( syswk, now_pos );

	syswk->app->old_cur_pos = now_pos;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxArrangePartyPokeMoveCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	SetTouchBarIconPokeGet( syswk, now_pos );

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );
	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );

	syswk->app->old_cur_pos = now_pos;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxArrangePartyPokeMoveCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_ARRANGE_PTGT_LEFT ||		// 31: ボックス切り替え矢印（左）
			now_pos == BOX2UI_ARRANGE_PTGT_RIGHT ||		// 32: ボックス切り替え矢印（右）
			now_pos == BOX2UI_ARRANGE_PTGT_STATUS ||	// 40: ステータス
			now_pos == BOX2UI_ARRANGE_PTGT_RET ){			// 41: 戻る
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->app->old_cur_pos );
		now_pos = syswk->app->old_cur_pos;
	}else{
		CursorObjMove( syswk, now_pos );
	}

	SetTouchBarIconPokeGet( syswk, now_pos );

	syswk->app->old_cur_pos = now_pos;
}


//============================================================================================
//	持ち物整理メイン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_BoxItemArrangeMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u8	now;

	now = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
		if( ret == BOX2UI_ITEM_MAIN_MENU1 ||
				ret == BOX2UI_ITEM_MAIN_MENU2 ||
				ret == BOX2UI_ITEM_MAIN_CLOSE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
			return BOX2UI_ITEM_MAIN_RETURN1;
		}
		if( ret == BOX2UI_ITEM_MAIN_RETURN1 ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
			return BOX2UI_ITEM_MAIN_RETURN1;
		}
		if( ret == BOX2UI_ITEM_MAIN_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			return BOX2UI_ITEM_MAIN_RETURN2;
		}
	}else{
		if( ret == BOX2UI_ITEM_MAIN_PARTY ||
				ret == BOX2UI_ITEM_MAIN_RETURN1 ||
				ret == BOX2UI_ITEM_MAIN_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( ret == CURSORMOVE_CANCEL ){
			return BOX2UI_ITEM_MAIN_CLOSE;
		}
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」メイン操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxItemArrangeMainCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk;
	const CURSORMOVE_DATA * pw;
	
	syswk = work;

	if( old_pos != CURSORMOVE_ONOFF_DIRECT ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			switch( now_pos ){
			case BOX2UI_ITEM_MAIN_LEFT:
			case BOX2UI_ITEM_MAIN_RIGHT:
				now_pos = BOX2UI_ITEM_MAIN_NAME;
				break;

			case BOX2UI_ITEM_MAIN_NAME:
			case BOX2UI_ITEM_MAIN_PARTY:
			case BOX2UI_ITEM_MAIN_RETURN1:
			case BOX2UI_ITEM_MAIN_RETURN2:
				break;

			default:
				if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					now_pos = syswk->get_pos;
				}else{
					now_pos = BOX2UI_ITEM_MAIN_POKE1;
				}
			}
		}else{
			if( now_pos != BOX2UI_ITEM_MAIN_MENU1 ||
					now_pos != BOX2UI_ITEM_MAIN_MENU2 ||
					now_pos != BOX2UI_ITEM_MAIN_CLOSE ){
				if( CURSORMOVE_MoveTableBitCheck(syswk->app->cmwk,BOX2UI_ITEM_MAIN_MENU1) == TRUE ){
					now_pos = BOX2UI_ITEM_MAIN_MENU1;
				}else{
					now_pos = BOX2UI_ITEM_MAIN_MENU2;
				}
			}
		}
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」メイン操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxItemArrangeMainCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );

	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」メイン操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxItemArrangeMainCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_ITEM_MAIN_RETURN1 ||
			now_pos == BOX2UI_ITEM_MAIN_RETURN2 ){
	}else{
		CursorObjMove( syswk, now_pos );
	}
}


//============================================================================================
//	持ち物整理・手持ちメイン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」手持ちポケモン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_BoxItemArrangePartyMoveMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u8	now;

	now = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == FALSE ){
		if( ret == BOX2UI_ITEM_PARTY_MENU1 ||
				ret == BOX2UI_ITEM_PARTY_MENU2 ||
				ret == BOX2UI_ITEM_PARTY_CLOSE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
			return BOX2UI_ITEM_PARTY_RETURN1;
		}
		if( ret == BOX2UI_ITEM_PARTY_RETURN1 ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
			return BOX2UI_ITEM_PARTY_RETURN1;
		}
		if( ret == BOX2UI_ITEM_PARTY_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			return BOX2UI_ITEM_PARTY_RETURN2;
		}
	}else{
		if( ret == BOX2UI_ITEM_PARTY_BOXLIST ||
				ret == BOX2UI_ITEM_PARTY_RETURN1 ||
				ret == BOX2UI_ITEM_PARTY_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( ret == CURSORMOVE_CANCEL ){
			return BOX2UI_ITEM_PARTY_CLOSE;
		}
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」手持ちポケモン操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxItemArrangePartyMoveCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( old_pos != CURSORMOVE_ONOFF_DIRECT ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			if( now_pos != BOX2UI_ITEM_PARTY_MENU1 ||
					now_pos != BOX2UI_ITEM_PARTY_MENU2 ||
					now_pos != BOX2UI_ITEM_PARTY_CLOSE ){
				if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					now_pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				}else{
					now_pos = BOX2UI_ITEM_PARTY_POKE1;
				}
			}
		}else{
			if( now_pos != BOX2UI_ITEM_PARTY_MENU1 ||
					now_pos != BOX2UI_ITEM_PARTY_MENU2 ||
					now_pos != BOX2UI_ITEM_PARTY_CLOSE ){
				if( CURSORMOVE_MoveTableBitCheck(syswk->app->cmwk,BOX2UI_ITEM_PARTY_MENU1) == TRUE ){
					now_pos = BOX2UI_ITEM_PARTY_MENU1;
				}else{
					now_pos = BOX2UI_ITEM_PARTY_MENU2;
				}
			}
		}
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」手持ちポケモン操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxItemArrangePartyMoveCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );

	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」手持ちポケモン操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxItemArrangePartyMoveCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_ITEM_PARTY_RETURN1 ||
			now_pos == BOX2UI_ITEM_PARTY_RETURN2 ){
	}else{
		CursorObjMove( syswk, now_pos );
	}
}


//============================================================================================
//	ボックステーマ変更
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックステーマ変更操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxThemaChgCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );
	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックステーマ変更操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxThemaChgCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );
	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックステーマ変更操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxThemaChgCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );
	CursorObjMove( syswk, now_pos );
}


//============================================================================================
//	壁紙変更
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙変更操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
/*
static void WallPaperChgCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );
	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙変更操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
static void WallPaperChgCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );
	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙変更操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void WallPaperChgCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );
	CursorObjMove( syswk, now_pos );
}


//============================================================================================
//	ボックスジャンプ
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックスジャンプコールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BoxJumpCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk;

	syswk = work;

//	CursorObjMove( syswk, now_pos );

	if( now_pos == BOX2UI_BOXJUMP_TRAY1 ||		// 00: トレイアイコン
			now_pos == BOX2UI_BOXJUMP_TRAY6 ||		// 05: トレイアイコン
			now_pos == BOX2UI_BOXJUMP_RET ){			// 06: 戻る
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->app->old_cur_pos );
		now_pos = syswk->app->old_cur_pos;
	}else{
		CursorObjMove( syswk, now_pos );
	}

	syswk->app->old_cur_pos = now_pos;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックスジャンプコールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxJumpCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos >= BOX2UI_BOXJUMP_TRAY2 && now_pos <= BOX2UI_BOXJUMP_TRAY5 ){
		BOX2OBJ_ChangeTrayName( syswk, now_pos-BOX2UI_BOXJUMP_TRAY2, TRUE );
	}else{
		BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
	}

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );
	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );

	syswk->app->old_cur_pos = now_pos;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックスジャンプコールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BoxJumpCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_BOXJUMP_TRAY1 ||		// 00: トレイアイコン
			now_pos == BOX2UI_BOXJUMP_TRAY6 ||		// 05: トレイアイコン
			now_pos == BOX2UI_BOXJUMP_RET ){			// 06: 戻る
		CURSORMOVE_PosSet( syswk->app->cmwk, syswk->app->old_cur_pos );
		now_pos = syswk->app->old_cur_pos;
	}else{
		CursorObjMove( syswk, now_pos );
	}

	syswk->app->old_cur_pos = now_pos;
}


//============================================================================================
//	バトルボックスメイン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_BattleBoxMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u8	now;

	now = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	// メニュー非表示
	if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
		if( ret >= BOX2UI_BATTLEBOX_MAIN_GET && ret <= BOX2UI_BATTLEBOX_MAIN_CLOSE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
			return BOX2UI_BATTLEBOX_MAIN_RETURN1;
		}
		if( ret == BOX2UI_BATTLEBOX_MAIN_RETURN1 ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
			return BOX2UI_BATTLEBOX_MAIN_RETURN1;
		}
		if( ret == BOX2UI_BATTLEBOX_MAIN_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			return BOX2UI_BATTLEBOX_MAIN_RETURN2;
		}
	// メニュー表示中
	}else{
		if( ret == BOX2UI_BATTLEBOX_MAIN_NAME ||
				ret == BOX2UI_BATTLEBOX_MAIN_PARTY ||
				ret == BOX2UI_BATTLEBOX_MAIN_RETURN1 ||
				ret == BOX2UI_BATTLEBOX_MAIN_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( ret == CURSORMOVE_CANCEL ){
			return BOX2UI_BATTLEBOX_MAIN_CLOSE;
		}
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」メイン操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BattleBoxMainCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk;
	const CURSORMOVE_DATA * pw;

	syswk = work;

	if( old_pos != CURSORMOVE_ONOFF_DIRECT ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			switch( now_pos ){
			case BOX2UI_BATTLEBOX_MAIN_LEFT:
			case BOX2UI_BATTLEBOX_MAIN_RIGHT:
				now_pos = BOX2UI_BATTLEBOX_MAIN_NAME;
				break;

			case BOX2UI_BATTLEBOX_MAIN_NAME:
			case BOX2UI_BATTLEBOX_MAIN_PARTY:
			case BOX2UI_BATTLEBOX_MAIN_RETURN1:
			case BOX2UI_BATTLEBOX_MAIN_RETURN2:
				break;

			default:
				if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					now_pos = syswk->get_pos;
				}else{
					now_pos = BOX2UI_BATTLEBOX_MAIN_POKE1;
				}
			}
		}else{
			if( now_pos != BOX2UI_BATTLEBOX_MAIN_GET ||
					now_pos != BOX2UI_BATTLEBOX_MAIN_STATUS ||
					now_pos != BOX2UI_BATTLEBOX_MAIN_ITEM ||
					now_pos != BOX2UI_BATTLEBOX_MAIN_CLOSE ){

				now_pos = BOX2UI_BATTLEBOX_MAIN_GET;
			}
		}
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」メイン操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BattleBoxMainCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );

	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」メイン操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BattleBoxMainCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_BATTLEBOX_MAIN_RETURN1 ||
			now_pos == BOX2UI_BATTLEBOX_MAIN_RETURN2 ){
	}else{
		CursorObjMove( syswk, now_pos );
	}
}


//============================================================================================
//	バトルボックス・パーティメイン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」パーティポケモン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_BattleBoxPartyMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u8	now;

	now = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	if( BOX2BGWFRM_PokeMenuPutCheck( syswk->app->wfrm ) == FALSE ){
		if( ret >= BOX2UI_BATTLEBOX_PARTY_GET && ret <= BOX2UI_BATTLEBOX_PARTY_CLOSE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
			return BOX2UI_BATTLEBOX_PARTY_RETURN1;
		}
		if( ret == BOX2UI_BATTLEBOX_PARTY_RETURN1 ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
			return BOX2UI_BATTLEBOX_PARTY_RETURN1;
		}
		if( ret == BOX2UI_BATTLEBOX_PARTY_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			return BOX2UI_BATTLEBOX_PARTY_RETURN2;
		}
	}else{
		if( ret == BOX2UI_BATTLEBOX_PARTY_BOXLIST ||
				ret == BOX2UI_BATTLEBOX_PARTY_RETURN1 ||
				ret == BOX2UI_BATTLEBOX_PARTY_RETURN2 ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}

		if( ret == CURSORMOVE_CANCEL ){
			return BOX2UI_BATTLEBOX_PARTY_CLOSE;
		}
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」パーティポケモン操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void BattleBoxPartyMainCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( old_pos != CURSORMOVE_ONOFF_DIRECT ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			if( now_pos >= BOX2UI_BATTLEBOX_PARTY_BOXLIST ){
				if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					now_pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				}else{
					now_pos = BOX2UI_BATTLEBOX_PARTY_POKE1;
				}
			}
		}else{
			if( now_pos != BOX2UI_BATTLEBOX_PARTY_GET ||
				now_pos != BOX2UI_BATTLEBOX_PARTY_STATUS ||
				now_pos != BOX2UI_BATTLEBOX_PARTY_ITEM ||
				now_pos != BOX2UI_BATTLEBOX_PARTY_CLOSE ){

				now_pos = BOX2UI_BATTLEBOX_PARTY_GET;
			}
		}
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」パーティポケモン操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BattleBoxPartyMainCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );

	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」パーティポケモン操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BattleBoxPartyMainCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_BATTLEBOX_PARTY_RETURN1 ||
			now_pos == BOX2UI_BATTLEBOX_PARTY_RETURN2 ){
	}else{
		CursorObjMove( syswk, now_pos );
	}
}


//============================================================================================
//	寝かすメイン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		「寝かすポケモンをえらぶ」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_SleepMain( BOX2_SYS_WORK * syswk )
{
	u32	ret;
	u8	now;

	now = CURSORMOVE_PosGet( syswk->app->cmwk );
	ret = CURSORMOVE_MainCont( syswk->app->cmwk );

	// メニュー非表示
	if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
		if( ret >= BOX2UI_SLEEP_MAIN_SET && ret <= BOX2UI_SLEEP_MAIN_CLOSE ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
		if( ret == BOX2UI_SLEEP_MAIN_RETURN ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			return BOX2UI_SLEEP_MAIN_RETURN;
		}
	// メニュー表示中
	}else{
		if( ret == BOX2UI_SLEEP_MAIN_NAME || ret == BOX2UI_SLEEP_MAIN_RETURN ){
			CURSORMOVE_PosSet( syswk->app->cmwk, now );
			CursorObjMove( syswk, now );
			return CURSORMOVE_NONE;
		}
/*
		if( ret == CURSORMOVE_CANCEL ){
			return BOX2UI_SLEEP_MAIN_CLOSE;
		}
*/
	}

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「寝かすポケモンをえらぶ」メイン操作コールバック関数：カーソル表示時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void SleepMainCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( old_pos != CURSORMOVE_ONOFF_DIRECT ){
		if( BOX2BGWFRM_PokeMenuPutCheck(syswk->app->wfrm) == FALSE ){
			switch( now_pos ){
			case BOX2UI_SLEEP_MAIN_LEFT:
			case BOX2UI_SLEEP_MAIN_RIGHT:
				now_pos = BOX2UI_SLEEP_MAIN_NAME;
				break;

			case BOX2UI_SLEEP_MAIN_NAME:
			case BOX2UI_SLEEP_MAIN_RETURN:
				break;

			default:
				if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					now_pos = syswk->get_pos;
				}else{
					now_pos = BOX2UI_SLEEP_MAIN_POKE1;
				}
			}
		}else{
			if( now_pos != BOX2UI_SLEEP_MAIN_SET ||
					now_pos != BOX2UI_SLEEP_MAIN_CLOSE ){
				now_pos = BOX2UI_SLEEP_MAIN_SET;
			}
		}
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	CursorObjMove( syswk, now_pos );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「寝かすポケモンをえらぶ」メイン操作コールバック関数：カーソル移動時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SleepMainCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );

	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「寝かすポケモンをえらぶ」メイン操作コールバック関数：タッチ時
 *
 * @param		work			ボックス画面システムワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SleepMainCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos == BOX2UI_SLEEP_MAIN_RETURN ){
	}else{
		CursorObjMove( syswk, now_pos );
	}
}


//============================================================================================
//	その他
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動をVBLANKで行うためのワーク設定（位置指定）
 *
 * @param		appwk			ボックス画面アプリワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2UI_CursorMoveVFuncWorkSet( BOX2_APP_WORK * appwk, int now_pos, int old_pos )
{
	const CURSORMOVE_DATA * npw;
	const CURSORMOVE_DATA * opw;

	npw = CURSORMOVE_GetMoveData( appwk->cmwk, now_pos );
	opw = CURSORMOVE_GetMoveData( appwk->cmwk, old_pos );

	BOX2UI_CursorMoveVFuncWorkSetDirect( appwk, npw->px, npw->py, opw->px, opw->py );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動をVBLANKで行うためのワーク設定（座標指定）
 *
 * @param		appwk		ボックス画面アプリワーク
 * @param		nx			移動位置Ｘ座標
 * @param		ny			移動位置Ｙ座標
 * @param		ox			前回位置Ｘ座標
 * @param		oy			前回位置Ｙ座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2UI_CursorMoveVFuncWorkSetDirect( BOX2_APP_WORK * appwk, u8 nx, u8 ny, u8 ox, u8 oy )
{
	BOX2MAIN_CURSORMOVE_WORK * cwk = GFL_HEAP_AllocMemory( HEAPID_BOX_APP, sizeof(BOX2MAIN_CURSORMOVE_WORK) );

	cwk->cnt = 4;
	cwk->px  = nx;
	cwk->py  = ny;

	if( nx >= ox ){
		cwk->vx = nx - ox;
		cwk->mx = 0;
	}else{
		cwk->vx = ox - nx;
		cwk->mx = 1;
	}
	if( ny >= oy ){
		cwk->vy = ny - oy;
		cwk->my = 0;
	}else{
		cwk->vy = oy - ny;
		cwk->my = 1;
	}
	cwk->vx = ( ( cwk->vx << 8 ) / cwk->cnt ) >> 8;
	cwk->vy = ( ( cwk->vy << 8 ) / cwk->cnt ) >> 8;

	appwk->vfunk.work = cwk;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチパネルヒットチェック（トリガ）
 *
 * @param		dat			カーソル移動データ
 * @param		max			データ数
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
static u32 HitCheckTrg( const CURSORMOVE_DATA * dat, u32 max )
{
	GFL_UI_TP_HITTBL	rect[2];
	u32	i;

	rect[1] = TpCheckEnd;

	for( i=0; i<max; i++ ){
		rect[0] = dat[i].rect;
		if( GFL_UI_TP_HitTrg( rect ) != GFL_UI_TP_HIT_NONE ){
			return i;
		}
	}

	return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチパネルヒットチェック（ベタ）
 *
 * @param		dat			カーソル移動データ
 * @param		max			データ数
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
static u32 HitCheckCont( const CURSORMOVE_DATA * dat, u32 max )
{
	GFL_UI_TP_HITTBL	rect[2];
	u32	i;

	rect[1] = TpCheckEnd;

	for( i=0; i<max; i++ ){
		rect[0] = dat[i].rect;
		if( GFL_UI_TP_HitCont( rect ) != GFL_UI_TP_HIT_NONE ){
			return i;
		}
	}

	return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチパネルヒットチェック（座標指定）
 *
 * @param		dat			カーソル移動データ
 * @param		max			データ数
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
static u32 HitCheckPos( const CURSORMOVE_DATA * dat, u32 max, u32 x, u32 y )
{
	GFL_UI_TP_HITTBL	rect[2];
	u32	i;

	rect[1] = TpCheckEnd;

	for( i=0; i<max; i++ ){
		rect[0] = dat[i].rect;
		if( GFL_UI_TP_HitSelf( rect, x, y ) != GFL_UI_TP_HIT_NONE ){
			return i;
		}
	}

	return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイのポケモン（トリガ）
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_HitCheckTrgTrayPoke(void)
{
	return HitCheckTrg( BoxArrangeMainCursorMoveData, BOX2OBJ_POKEICON_TRAY_MAX );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイのポケモン（座標指定）
 *
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_HitCheckPosTrayPoke( u32 x, u32 y )
{
	return HitCheckPos( BoxArrangeMainCursorMoveData, BOX2OBJ_POKEICON_TRAY_MAX, x, y );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：手持ちポケモン（左）
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_HitCheckTrgPartyPokeLeft(void)
{
	return HitCheckTrg( BoxArrangePartyPokeCursorMoveData, BOX2OBJ_POKEICON_MINE_MAX );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：手持ちポケモン（右）
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_HitCheckTrgPartyPokeRight(void)
{
	return HitCheckTrg(
					&BoxArrangePartyPokeMoveCursorMoveData[BOX2UI_ARRANGE_PTGT_PARTY_POKE], BOX2OBJ_POKEICON_MINE_MAX );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイ切り替え矢印
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_HitCheckContTrayArrow(void)
{
	return HitCheckCont( &BoxArrangeMainCursorMoveData[BOX2UI_ARRANGE_MAIN_LEFT], 2 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイアイコン上下スクロール
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_HitCheckContTrayIconScroll(void)
{
	return GFL_UI_TP_HitCont( PokeGetTrayScrollHitTbl );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイアイコンスクロール開始
 *
 * @param		x			Ｘ座標格納場所
 * @param		y			Ｙ座標格納場所
 *
 * @retval	"TRUE = タッチあり"
 * @retval	"FALSE = タッチなし"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2UI_HitCheckTrayScrollStart( u32 * x, u32 * y )
{
	if( GFL_UI_TP_HitCont( TrayScrollStartHitTbl ) != GFL_UI_TP_HIT_NONE ){
		GFL_UI_TP_GetPointCont( x, y );
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイアイコンスクロール継続
 *
 * @param		x			Ｘ座標格納場所
 * @param		y			Ｙ座標格納場所
 *
 * @retval	"TRUE = タッチあり"
 * @retval	"FALSE = タッチなし"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2UI_HitCheckTrayScroll( u32 * x, u32 * y )
{
	if( GFL_UI_TP_HitCont( TrayScrollHitTbl ) != GFL_UI_TP_HIT_NONE ){
		GFL_UI_TP_GetPointCont( x, y );
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定座標のトレイアイコンを取得
 *
 * @param		x			Ｘ座標
 * @param		y			Ｙ座標
 *
 * @retval	"BOX2MAIN_GETPOS_NONE = トレイアイコンなし"
 * @retval	"BOX2MAIN_GETPOS_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
u32 BOX2UI_HitCheckTrayIcon( u32 x, u32 y )
{
	u32	ret = HitCheckPos( &BoxArrangePokeMoveCursorMoveData[BOX2UI_ARRANGE_PGT_TRAY2], 4, x, y );

	if( ret != GFL_UI_TP_HIT_NONE ){
		return ( BOX2OBJ_POKEICON_PUT_MAX + ret );
	}
	return BOX2MAIN_GETPOS_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：手持ちフレーム（左）
 *
 * @param		none
 *
 * @retval	"TRUE = タッチあり"
 * @retval	"FALSE = タッチなし"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2UI_HitCheckPartyFrameLeft(void)
{
	if( GFL_UI_TP_HitCont( PartyLeftHitTbl ) != GFL_UI_TP_HIT_NONE ){
		return TRUE;
	}
	return FALSE;
}
















//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
//	旧処理
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンをあずける」預けるトレイ選択コールバック関数：カーソル表示時
 *
 * @param	work		ボックス画面システムワーク
 * @param	now_pos		現在の位置
 * @param	old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void PartyOutBoxSelCallBack_On( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk;
	const CURSORMOVE_DATA * pw;
	u8	x, y;

	syswk = work;

	if( now_pos == BOX2UI_PTOUT_BOXSEL_LEFT || now_pos == BOX2UI_PTOUT_BOXSEL_RIGHT ){
		now_pos = syswk->box_mv_pos % BOX2OBJ_TRAYICON_MAX;
		syswk->app->old_cur_pos = now_pos;
	}

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	pw = CURSORMOVE_GetMoveData( syswk->app->cmwk, now_pos );

	x = pw->px;
	y = pw->py;
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, TRUE );

	if( now_pos >= BOX2UI_PTOUT_BOXSEL_TRAY1 && now_pos <= BOX2UI_PTOUT_BOXSEL_TRAY6 ){
//		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
	}else{
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンをあずける」預けるトレイ選択コールバック関数：カーソル移動時
 *
 * @param	work		ボックス画面システムワーク
 * @param	now_pos		現在の位置
 * @param	old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void PartyOutBoxSelCallBack_Move( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	if( now_pos >= BOX2UI_PTOUT_BOXSEL_TRAY1 && now_pos <= BOX2UI_PTOUT_BOXSEL_TRAY6 ){
//		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_TRAY );
	}else{
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
	}

	//「あずける」からトレイへの移動
	if( old_pos == BOX2UI_PTOUT_BOXSEL_ENTER && now_pos == BOX2UI_PTOUT_BOXSEL_TRAY1 ){
		now_pos = syswk->app->old_cur_pos;
		CURSORMOVE_PosSetEx( syswk->app->cmwk, now_pos, BOX2UI_PTOUT_BOXSEL_ENTER, BOX2UI_PTOUT_BOXSEL_ENTER );
	}

	// 前回位置更新
	if( now_pos >= BOX2UI_PTOUT_BOXSEL_TRAY1 && now_pos <= BOX2UI_PTOUT_BOXSEL_TRAY6 ){
		syswk->app->old_cur_pos = now_pos;
	}

	// トレイ左から右へのループ
	if( now_pos == BOX2UI_PTOUT_BOXSEL_TRAY1 && old_pos == BOX2UI_PTOUT_BOXSEL_TRAY6 ){
		if( syswk->box_mv_pos + 1 >= syswk->trayMax ){
			syswk->box_mv_pos = 0;
		}else{
			syswk->box_mv_pos++;
		}
		BOX2OBJ_TrayIconChange( syswk );
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2OBJ_BoxMoveCursorSet( syswk );
		BOX2BMP_BoxMoveNameWrite( syswk, 0 );
//		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_BOXMV_RA, BOX2OBJ_ANM_R_ARROW_ON );
		BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );
		BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
		return;
	}

	// トレイ右から左へのループ
	if( now_pos == BOX2UI_PTOUT_BOXSEL_TRAY6 && old_pos == BOX2UI_PTOUT_BOXSEL_TRAY1 ){
		if( syswk->box_mv_pos - 1 < 0 ){
			syswk->box_mv_pos = syswk->trayMax - 1;
		}else{
			syswk->box_mv_pos--;
		}
		BOX2OBJ_TrayIconChange( syswk );
		BOX2OBJ_BoxMoveNameSet( syswk );
		BOX2OBJ_BoxMoveCursorSet( syswk );
		BOX2BMP_BoxMoveNameWrite( syswk, 0 );
//		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_BOXMV_LA, BOX2OBJ_ANM_L_ARROW_ON );
		BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );
		BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
		return;
	}

	if( now_pos >= BOX2UI_PTOUT_BOXSEL_TRAY1 && now_pos <= BOX2UI_PTOUT_BOXSEL_TRAY6 ){
		if( old_pos != BOX2UI_PTOUT_BOXSEL_ENTER ){
			syswk->box_mv_pos = BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + now_pos;
			BOX2OBJ_BoxMoveNameSet( syswk );
			BOX2BMP_BoxMoveNameWrite( syswk, 0 );
		}
	}

	BOX2UI_CursorMoveVFuncWorkSet( syswk->app, now_pos, old_pos );
	BOX2MAIN_VFuncReq( syswk->app, BOX2MAIN_VFuncCursorMove );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンをあずける」預けるトレイ選択コールバック関数：タッチ時
 *
 * @param	work		ボックス画面システムワーク
 * @param	now_pos		現在の位置
 * @param	old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void PartyOutBoxSelCallBack_Touch( void * work, int now_pos, int old_pos )
{
	BOX2_SYS_WORK * syswk = work;

	CURSORMOVE_PosSet( syswk->app->cmwk, now_pos );

	if( now_pos != BOX2UI_PTOUT_BOXSEL_LEFT && now_pos != BOX2UI_PTOUT_BOXSEL_RIGHT ){
		CursorObjMove( syswk, now_pos );
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタンチェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"TRUE = 起動可"
 * @retval	"FALSE = 起動不可"
 */
//--------------------------------------------------------------------------------------------
/*
BOOL BOX2UI_YStatusButtonCheck( BOX2_SYS_WORK * syswk )
{
	if( BOX2BGWFRM_YStatusButtonCheck(syswk->app->wfrm) == FALSE ){
		return FALSE;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
		return TRUE;
	}

	if( GFL_UI_TP_HitTrg( YStatusHitTbl ) != GFL_UI_TP_HIT_NONE ){
		return TRUE;
	}

	return FALSE;
}
*/
