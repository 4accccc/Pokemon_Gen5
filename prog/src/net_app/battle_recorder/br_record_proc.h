//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_record_proc.h
 *	@brief	レコードプロセス
 *	@author	Toru=Nagihashi
 *	@date		2009.11.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "br_proc_sys.h"
#include "br_res.h"
#include "br_fade.h"
#include "savedata/gds_profile.h"
#include "savedata/battle_rec.h"
#include "br_inner.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	レコードモード
//=====================================
typedef enum
{
	BR_RECODE_PROC_MY,
	BR_RECODE_PROC_OTHER_00,
	BR_RECODE_PROC_OTHER_01,
	BR_RECODE_PROC_OTHER_02,
	BR_RECODE_PROC_DOWNLOAD_RANK,
	BR_RECODE_PROC_DOWNLOAD_NUMBER,
} BR_RECODE_PROC_MODE;

//-------------------------------------
///	終了モード
//=====================================
typedef enum
{
  BR_RECORD_RETURN_FINISH,
  BR_RECORD_RETURN_BTLREC,
} BR_RECORD_RETURN;

//=============================================================================
/**
 *					構造体
*/
//=============================================================================
//-------------------------------------
///	レコードプロセス引数
//=====================================
typedef struct 
{
  BR_RECORD_RETURN    ret;          //[out]終了モード
	BR_RECODE_PROC_MODE	mode;					//[in ]起動モード
  BR_FADE_WORK        *p_fade;      //[in ]フェード
	BR_RES_WORK					*p_res;				//[in ]リソース管理
	BR_PROC_SYS					*p_procsys;		//[in ]プロセス管理
	GFL_CLUNIT					*p_unit;			//[in ]ユニット
  u64                 video_number; //[in ]ビデオナンバー（modeがBR_RECODE_PROC_DOWNLOAD_NUMBERのときのみ受け取る）
  BR_NET_WORK         *p_net;       //[in ]通信管理
  GAMEDATA            *p_gamedata;  //[in ]ゲームデータ
  BR_RECORD_DATA      *p_record;    //[in ]録画常駐データ
  BR_OUTLINE_DATA     *p_outline;   //[in ]アウトライン常駐データ
  BOOL                is_btl_return;//[in ]戦闘からの戻り
} BR_RECORD_PROC_PARAM;


//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//-------------------------------------
///	レコードプロセスデータ
//=====================================
extern const GFL_PROC_DATA	BR_RECORD_ProcData;
