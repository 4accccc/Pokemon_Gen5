//==============================================================================
/**
 * @file	compati_tool.c
 * @brief	相性チェック：ツール類
 * @author	matsuda
 * @date	2009.02.10(火)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include "net\network_define.h"
#include "print\printsys.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "message.naix"
#include "test_graphic\d_taya.naix"
#include "font/font.naix"
#include "system\gfl_use.h"
#include "compati_check.naix"
#include "system/wipe.h"

#include "compati_types.h"
#include "compati_tool.h"


//==============================================================================
//	構造体定義
//==============================================================================
///円の当たり判定
typedef struct{
	s16 top;
	s16 bottom;
	s16 left;
	s16 right;
}CIRCLE_RANGE;


//==============================================================================
//	外部データ
//==============================================================================
#include "compati_circle.dat"


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL _CCTOOL_CircleTrgCheck(const CC_CIRCLE_PACKAGE *ccp, CCT_TOUCH_SYS *cts);
static void _CCTOOL_CircleContCheck(const CC_CIRCLE_PACKAGE *ccp, CCT_TOUCH_SYS *cts);
static void _CCTOOL_CircleLenCalc(CCT_TOUCH_SYS *cts, const CC_CIRCLE_PARAM *circle, u32 tp_x, u32 tp_y);
static void _CCTOOL_CircleRangeGet(const CC_CIRCLE_PARAM *circle, CIRCLE_RANGE *range);
static BOOL _CCTOOL_RangeHitCheck(const CIRCLE_RANGE *range, u32 tp_x, u32 tp_y);


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   使用するサークルデータをランダムで取得する
 *
 * @param   dest		代入先
 */
//--------------------------------------------------------------
void CCTOOL_CircleDataGet(CC_CIRCLE_PACKAGE *dest)
{
	int data_no, rand_range;
	
	rand_range = NELEMS(CompatiCircleData);
	data_no = GFUser_GetPublicRand(rand_range);
	GFL_STD_MemCopy(&CompatiCircleData[data_no], dest, sizeof(CC_CIRCLE_PACKAGE));
}

//--------------------------------------------------------------
/**
 * @brief   円とタッチペンの当たり判定チェック
 *
 * @param   ccp		
 * @param   cts		
 */
//--------------------------------------------------------------
void CCTOOL_CircleTouchCheck(const CC_CIRCLE_PACKAGE *ccp, CCT_TOUCH_SYS *cts)
{
	if(cts->touch_hold == CIRCLE_TOUCH_NULL){
		_CCTOOL_CircleTrgCheck(ccp, cts);
	}
	else{
		_CCTOOL_CircleContCheck(ccp, cts);
	}
}

//--------------------------------------------------------------
/**
 * @brief   円とのトリガーチェック
 *
 * @param   ccp		
 * @param   cts		
 *
 * @retval  TRUE:当たり判定発生。　FALSE:当たりなし
 */
//--------------------------------------------------------------
static BOOL _CCTOOL_CircleTrgCheck(const CC_CIRCLE_PACKAGE *ccp, CCT_TOUCH_SYS *cts)
{
	int i;
	u32 tp_x, tp_y;
	const CC_CIRCLE_PARAM *circle;
	CIRCLE_RANGE range;
	
	if(GFL_UI_TP_GetPointTrg(&tp_x, &tp_y) == FALSE){
		return FALSE;
	}
	
	circle = ccp->data;
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		switch(circle->type){
		case CC_CIRCLE_TYPE_NULL:
			return FALSE;
		default:
			_CCTOOL_CircleRangeGet(circle, &range);
			break;
		}
		
		if(_CCTOOL_RangeHitCheck(&range, tp_x, tp_y) == TRUE){
			cts->touch_hold = CIRCLE_TOUCH_HOLD;
			cts->hold_circle_no = i;
			cts->old_len_x = tp_x;
			cts->old_len_y = tp_y;
			OS_TPrintf("円とヒット no = %d\n", i);
			return TRUE;
		}
		circle++;
	}
	
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   円の掴み状況をチェック
 *
 * @param   ccp		
 * @param   cts		
 */
//--------------------------------------------------------------
static void _CCTOOL_CircleContCheck(const CC_CIRCLE_PACKAGE *ccp, CCT_TOUCH_SYS *cts)
{
	u32 tp_x, tp_y;
	CIRCLE_RANGE range;
	BOOL hit_ret;
	
	if(GFL_UI_TP_GetPointCont(&tp_x, &tp_y) == FALSE){
		cts->touch_hold = CIRCLE_TOUCH_NULL;
		OS_TPrintf("円との掴み解除\n");
		return;
	}
	
	_CCTOOL_CircleRangeGet(&ccp->data[cts->hold_circle_no], &range);
	if(_CCTOOL_RangeHitCheck(&range, tp_x, tp_y) == TRUE){
		cts->touch_hold = CIRCLE_TOUCH_HOLD;
		_CCTOOL_CircleLenCalc(cts, &ccp->data[cts->hold_circle_no], tp_x, tp_y);
		cts->old_len_x = tp_x;
		cts->old_len_y = tp_y;
	}
	else{
		cts->touch_hold = CIRCLE_TOUCH_OUTSIDE;
		cts->old_len_x = -1;
		cts->old_len_y = -1;
	}
}

//--------------------------------------------------------------
/**
 * @brief   円の距離を算出
 *
 * @param   cts		
 * @param   circle		
 * @param   tp_x		
 * @param   tp_y		
 */
//--------------------------------------------------------------
static void _CCTOOL_CircleLenCalc(CCT_TOUCH_SYS *cts, const CC_CIRCLE_PARAM *circle, u32 tp_x, u32 tp_y)
{
	int x_offset, y_offset;
	u32 len;
	
	if(cts->old_len_x == -1 || cts->old_len_y == -1){
		return;	//前フレームが範囲外だったので今回は距離測定なし
	}
	
	if(cts->total_len > circle->point_limit * CC_POINT_DOT){
		OS_TPrintf("リミットに達しています limit = %d\n", circle->point_limit);
		return;	//この円では得点のリミットに達している
	}
	
	x_offset = tp_x - cts->old_len_x;
	y_offset = tp_y - cts->old_len_y;
	x_offset = MATH_ABS(x_offset);
	y_offset = MATH_ABS(y_offset);
	//タッチパネルの精度の問題なのか、ずっと同じ所を指しているつもりでも
	//1がちょこちょこ入ってくるので1は0と同じで無視にする
	if(x_offset > 1 || y_offset > 1){
		u32 temp;
		temp = (x_offset * x_offset) + (y_offset * y_offset);
		CP_SetSqrt32(temp << 4);
		temp = CP_GetSqrtResult32();
		len = (temp >> 2);
//		OS_TPrintf("直線距離 = %d (x_offs=%d, y_offs=%d)\n", len, x_offset, y_offset);
		cts->total_len += len;
//		OS_TPrintf("total_len = %d\n", cts->total_len);
	}
}

//--------------------------------------------------------------
/**
 * @brief   円データからヒット範囲を取得する
 *
 * @param   circle		円データ
 * @param   range		ヒット範囲代入先
 */
//--------------------------------------------------------------
static void _CCTOOL_CircleRangeGet(const CC_CIRCLE_PARAM *circle, CIRCLE_RANGE *range)
{
	s16 half;
	
	switch(circle->type){
	case CC_CIRCLE_TYPE_XL:
		half = 40;
		break;
	case CC_CIRCLE_TYPE_L:
		half = 32;
		break;
	case CC_CIRCLE_TYPE_M:
		half = 24;
		break;
	case CC_CIRCLE_TYPE_S:
		half = 16;
		break;
	case CC_CIRCLE_TYPE_NULL:
	default:
		GF_ASSERT(0);
		return;
	}
	range->top = circle->y - half;
	range->bottom = circle->y + half;
	range->left = circle->x - half;
	range->right = circle->x + half;
}

//--------------------------------------------------------------
/**
 * @brief   ヒット範囲の中にタッチペン座標が収まっているか調べる
 *
 * @param   range		ヒット範囲
 * @param   tp_x		タッチペン座標X
 * @param   tp_y		タッチペン座標Y
 *
 * @retval  TRUE:収まっている。　FALSE:範囲外
 */
//--------------------------------------------------------------
static BOOL _CCTOOL_RangeHitCheck(const CIRCLE_RANGE *range, u32 tp_x, u32 tp_y)
{
	if(tp_x >= range->left && tp_x <= range->right && tp_y >= range->top && tp_y <= range->bottom){
		return TRUE;
	}
	return FALSE;
}
