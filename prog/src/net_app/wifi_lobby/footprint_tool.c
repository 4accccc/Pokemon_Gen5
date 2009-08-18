//==============================================================================
/**
 * @file	footprint_tool.c
 * @brief	足跡ボードのツール類
 * @author	matsuda
 * @date	2008.01.22(火)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/bmp_menulist.h"
#include "print\printsys.h"
#include "savedata\system_data.h"
#include "system/bmp_menu.h"
#include "system/wipe.h"
#include "print/wordset.h"
#include "message.naix"
//#include "system/fontproc.h"
//#include "gflib/strbuf_family.h"
#include "msg/msg_wflby_footprint.h"
#include "savedata/config.h"
//#include  "communication/wm_icon.h"
//#include "system/fontoam.h"
//#include "system/d3dobj.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
//#include "gflib/strbuf_family.h"

#include "net_app/wifi_lobby/wflby_system_def.h"
#include "net_app/footprint_main.h"
#include "footprint_common.h"
#include "footprint_board.naix"
#include "footprint_stamp.h"
#include "footprint_comm.h"
#include "footprint_tool.h"
#include "footprint_snd_def.h"
#include "arc_def.h"
#include "poke_tool/monsno_def.h"


//==============================================================================
//	データ
//==============================================================================
#include "footprint_control.dat"
#include "footprint_foot.dat"

///インクパレットタッチパネル領域設定
static const GFL_UI_TP_HITTBL InkPaletteTouchData[] = {
	//TOP BOTTOM LEFT RIGHT
	{160, 192, 32*0, 32*1},
	{160, 192, 32*1, 32*2},
	{160, 192, 32*2, 32*3},
	{160, 192, 32*3, 32*4},
	{160, 192, 32*4, 32*5},
	{160, 192, 32*5, 32*6},
	{160, 192, 32*6, 255},		//やめる
	{ GFL_UI_TP_HIT_END, 0, 0, 0 }
};


//--------------------------------------------------------------
/**
 * @brief   トレーナー名を描画
 *
 * @param   msgman			メッセージマネージャへのポインタ
 * @param   wordset			ワードセットへのポインタ
 * @param   win[]			BMPWINへのポインタ
 * @param   wflby_sys		ワイファイロビーシステムへのポインタ
 * @param   user_index		サブチャンネルのユーザーID
 */
//--------------------------------------------------------------
void FootPrintTool_NameDraw(GFL_MSGDATA *msgman, WORDSET *wordset, GFL_BMPWIN *win[], PRINT_UTIL print_util[], WFLBY_SYSTEM *wflby_sys, s32 user_id, PRINT_QUE *printQue, GFL_FONT *font_handle)
{
	const WFLBY_USER_PROFILE* wup;
	MYSTATUS *my_status;
	STRBUF *message_src;
	STRBUF *expand_src;
	STRBUF *name_src;
	int expand_len = 64;	//ワードセット展開バッファのサイズ。適当に大きく
	int sex;
	u32 user_index, main_idx;
	PRINTSYS_LSB print_color;
	
	//サブチャンネルのユーザーIDからメインのユーザーIndexを取得する(道のり)
	user_index = OLDDWC_LOBBY_SUBCHAN_GetUserIDIdx(user_id);
	if(user_index == OLDDWC_LOBBY_USERIDTBL_IDX_NONE){
		return;
	}
	main_idx = OLDDWC_LOBBY_GetUserIDIdx(user_id);
	if(main_idx == OLDDWC_LOBBY_USERIDTBL_IDX_NONE){
		return;
	}
	
	OS_TPrintf("user_index = %d, main_idx = %d, user_id = %d\n", user_index, main_idx, user_id);
	//メインのユーザーIndexからプロフィールを取得
	wup = WFLBY_SYSTEM_GetUserProfile(wflby_sys, main_idx);
	if(wup == NULL){
		return;
	}
	
	if(user_index >= FOOTPRINT_BMPWIN_NAME_MAX){
		GF_ASSERT(0);
		return;
	}
	
	my_status = MyStatus_AllocWork(HEAPID_FOOTPRINT);
	WFLBY_SYSTEM_GetProfileMyStatus(wup, my_status, HEAPID_FOOTPRINT);
	name_src = MyStatus_CreateNameString(my_status, HEAPID_FOOTPRINT);
	sex = MyStatus_GetMySex(my_status);
	
	WORDSET_RegisterWord(wordset, 0, name_src, sex, TRUE, PM_LANG);
	message_src = GFL_MSG_CreateString(msgman, msg_footprint_name);
	expand_src = GFL_STR_CreateBuffer(expand_len, HEAPID_FOOTPRINT);
	WORDSET_ExpandStr(wordset, expand_src, message_src);

	GFL_BMP_Clear(GFL_BMPWIN_GetBmp(win[user_index]), 0x00);
	if(WFLBY_SYSTEM_GetUserVipFlag(wflby_sys, main_idx) == TRUE){
		print_color = GF_PRINTCOLOR_MAKE(7,8,0);
	}
	else{
		print_color = GF_PRINTCOLOR_MAKE(1,2,0);
	}
#if WB_FIX
	PRINT_UTIL_PrintColor(/*引数内はまだ未移植*/&win[user_index], NET_FONT_SYSTEM, expand_src, 0, 0, 
		MSG_ALLPUT, print_color, NULL);
#else
	PRINT_UTIL_PrintColor(
	  &print_util[user_index], printQue, 0, 0, expand_src, font_handle, print_color);
#endif
#if WB_FIX
	GF_BGL_BmpWinOn(&win[user_index]);
#else
  GFL_BMPWIN_MakeTransWindow(win[user_index]);
#endif

	GFL_STR_DeleteBuffer(name_src);
	GFL_STR_DeleteBuffer(message_src);
	GFL_STR_DeleteBuffer(expand_src);
	
	GFL_HEAP_FreeMemory(my_status);
}

//--------------------------------------------------------------
/**
 * @brief   参加者名を消去する
 *
 * @param   win[]			BMPWINへのポインタ
 * @param   user_index		ユーザーindex
 */
//--------------------------------------------------------------
void FootPrintTool_NameErase(GFL_BMPWIN *win[], u32 user_index)
{
	if(user_index >= FOOTPRINT_BMPWIN_NAME_MAX){
		GF_ASSERT(0);
		return;
	}
	GFL_BMPWIN_ClearTransWindow(win[user_index]);
}

//--------------------------------------------------------------
/**
 * @brief   親のIDから、スタンプの色を取得する
 *
 * @param   board_type	ボードの種類(FOOTPRINT_BOARD_TYPE_???)
 * @param   oya_id		親のID
 *
 * @retval  カラーコード
 */
//--------------------------------------------------------------
u16 FootprintTool_StampColorGet(int board_type, u32 oya_id)
{
	return StampColorTbl[oya_id % NELEMS(StampColorTbl)][board_type];
}

//--------------------------------------------------------------
/**
 * @brief   インクパレットがタッチされているかの更新処理
 *
 * @param   my_stamp_array[]		自分のスタンプパラメータへのポインタ
 *
 * @retval  0～5:タッチされたインクパレット or FOOT_TOUCH_RET_EXIT or FOOT_TOUCH_RET_NULL
 */
//--------------------------------------------------------------
int FootprintTool_InkPalTouchUpdate(STAMP_PARAM *my_stamp_array, int now_select_no)
{
	int hit;
	int ret = FOOT_TOUCH_RET_NULL;
	
	hit = GFL_UI_TP_HitTrg(InkPaletteTouchData);
	if(hit < 6){		//インクパレットを押した
		if(now_select_no != hit 
				&& my_stamp_array[hit].monsno != 0 && my_stamp_array[hit].monsno <= MONSNO_END){
			ret = hit;
		}
	}
	else if(hit == 6){	//「やめる」ボタンを押した
		ret = FOOT_TOUCH_RET_EXIT;
	}
	else{
		return FOOT_TOUCH_RET_NULL;
	}
	
	return ret;
}

//--------------------------------------------------------------
/**
 * @brief   足跡が有るポケモンかチェックする
 *
 * @param   monsno		ポケモン番号
 * @param   form_no		フォルム番号
 * @param   arceus_flg	アルセウスフラグ(TRUE:公開OK)
 *
 * @retval  TRUE:足跡がある
 * @retval  FALSE:足跡がない
 */
//--------------------------------------------------------------
BOOL FootprintTool_FootDispCheck(int monsno, int form_no, BOOL arceus_flg)
{
	if(monsno == MONSNO_GIRATHINA && form_no > 0){
		return FALSE;	//フォルムチェンジしたギラティナだけ個別に判定
	}
	if(monsno == MONSNO_ARUSEUSU && arceus_flg == FALSE){
		return FALSE;	//公開OKになっていない時はアルセウスは足跡無し扱いで判定
	}
	
	return PokeFootConvTbl[monsno].disp;
}

//--------------------------------------------------------------
/**
 * @brief   足跡が有るポケモンかチェックする
 *
 * @param   monsno		ポケモン番号
 * @param   form_no		フォルム番号
 *
 * @retval  TRUE:足跡がある
 * @retval  FALSE:足跡がない
 */
//--------------------------------------------------------------
BOOL FootprintTool_FootHitSizeGet(int monsno, int form_no)
{
	if(monsno == MONSNO_GIRATHINA && form_no > 0){
		return 2;	//フォルムチェンジしたギラティナだけ個別に判定
	}
	
	return PokeFootConvTbl[monsno].hit_size;
}
