//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_utilproc.h
 *	@brief  wifibattlematch_sysで色々な処理をさせないために、PROC化したもの
 *	@author	Toru=Nagihashi
 *	@date		2010.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "net_app/wifibattlematch.h"
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					リスト後プロセス
 *					    タイミングをとってパーティを送るだけ
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	リスト後プロセスの戻り値
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_LISTAFTER_RESULT_SUCCESS,           //正常に終了
  WIFIBATTLEMATCH_LISTAFTER_RESULT_ERROR_NEXT_LOGIN,  //エラー検知(WIFIのみ)
  WIFIBATTLEMATCH_LISTAFTER_RESULT_ERROR_RETURN_LIVE, //エラー検知(LIVEのみ)
} WIFIBATTLEMATCH_LISTAFTER_RESULT;

//-------------------------------------
///	通信タイプ
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_LISTAFTER_NETTYPE_WIFI,
  WIFIBATTLEMATCH_LISTAFTER_NETTYPE_IRC,
} WIFIBATTLEMATCH_LISTAFTER_NETTYPE;


//-------------------------------------
///	リスト後プロセスの引数
//=====================================
typedef struct
{
  WIFIBATTLEMATCH_PARAM             *p_param;     //[in ]外部引数
  WIFIBATTLEMATCH_LISTAFTER_NETTYPE type;         //[in ]通信タイプ
  WIFIBATTLEMATCH_LISTAFTER_RESULT  result;       //[out]戻り値

  //以下、[in ]常駐データ置き場
  POKEPARTY                   *p_player_btl_party; //自分で決めたパーティ
  POKEPARTY                   *p_enemy_btl_party; //相手の決めたパーティ

} WIFIBATTLEMATCH_LISTAFTER_PARAM;

//-------------------------------------
///	リスト後プロセスの外部参照
//=====================================
//FS_EXTERN_OVERLAY( wifibattlematch_core );  //core.hで呼ばれている
//リスト後の処理
extern const GFL_PROC_DATA	WifiBattleMatch_ListAfter_ProcData;

