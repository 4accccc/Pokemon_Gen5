//============================================================================
/**
 *
 *	@file		fes_rom_sys_local.h
 *	@brief  フェス配信ロム システムプロセスローカルヘッダ
 *	@author	Miyuki Iwasawa
 *	@data		2012.03.29
 */
//============================================================================
#pragma once
//	lib
#include <gflib.h>

//system
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_beacon.h"
#include "sound/pm_sndsys.h"

///ミッションセーブデータ数
#define FESROM_MISSION_DATA_MAX (10)

///セーブデータ有効を示すマジックキー
#define FES_ROM_SAVE_MAGICKEY    (0xaefb3235)

///トレーナーID
#define FES_ROM_TRAINER_ID       (0x0CA9)

#define FES_ROM_TIME_UNIT_MIN   (30)  //30分単位
#define FES_ROM_TIME_UNIT_HOUR  (60/FES_ROM_TIME_UNIT_MIN)  //1時間は2単位

//フェスロムアプリの呼び出しモード定義
typedef enum{
 FESROM_APP_MODE_BASE,
 FESROM_APP_MODE_MEDIT,
 FESROM_APP_MODE_SEND,
 FESROM_APP_MODE_MAX,
}FESROM_APP_MODE;

typedef enum{
 FESROM_APP_RET_NONE,
 FESROM_APP_RET_EDIT_NAME,
 FESROM_APP_RET_EDIT_SELFMSG, 
}FESROM_APP_RET_MODE;

//--------------------------------------------------------------
//  ミッション詳細セーブ
//--------------------------------------------------------------
typedef struct _FESROM_MISSION_PARAM{
  u8      idx;  ///<データindex
  u8      live_f; ///<データが有効かどうか？

  int     m_id;
  int     m_t_bonus;
  int     m_base_param;
  int     m_ex_param;
  int     m_type_code;
  int     m_level;
//  int     m_goal;
  int     m_time;
  int     m_version;
//  int     m_unit;

  int     m_item;
  int     m_pay;
  
  int     m_title;
  int     m_exp;
  
  int     m_sp_item;
  int     m_sp_pay;
  int     m_pattern[FMISSION_MAP_ATTR_MAX]; //出現パターン
}FESROM_MISSION_PARAM;

typedef struct _FESROM_MISSION_SEND{
  int     id;      ///<ミッションID(0は無効値)
  int     time;    ///<配信時間(1-60分)
  int     blank;   ///<配信間隔(1-60分)
  int     odds;    ///<ランダム配信時の%指定
}FESROM_MISSION_SEND;

///送信ミッションパラメータ
typedef struct _FESROM_MISSION{
  FESROM_MISSION_PARAM  param;
  FESROM_MISSION_SEND   send;
}FESROM_MISSION;

#define BUFLEN_FESROM_SELFMSG (GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN+EOM_SIZE)

//--------------------------------------------------------------
//  セーブ
//--------------------------------------------------------------
///セーブデータ
typedef struct _FES_ROM_SAVE{
  u32 magic_key;  ///<FES_ROM_SAVE_MAGICKEY

  int misson_num; ///<有効ミッションデータ数
  FESROM_MISSION  mission[FESROM_MISSION_DATA_MAX];

  //プレイヤーネーム
  STRCODE name[SAVELEN_PLAYER_NAME + EOM_SIZE];
  //あいさつ文
  STRCODE selfmsg[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN + EOM_SIZE];

  int tr_view;
  int start_time;
  int end_time;
  int beacon_interval;
  int mode;
}FES_ROM_SAVE;

///セーブデータ参照用ワーク(STRCODEなどを展開したコピー)
typedef struct _FES_ROM_SAVE_WORK{
  FES_ROM_SAVE  save; //セーブデータコピー

  STRBUF* str_name;
  STRBUF* str_selfmsg;

}FES_ROM_SAVE_WK;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;
  HEAPID tmpHeapID;
  GFL_PROCSYS*          localProc;
 

  int                   seq;
	int										sub_seq;
  int                   eff_task_ct;
  BOOL                  data_error_f;
  BOOL                  data_enable_f;
  BOOL                  save_enable_f;
  BOOL                  command_f;  //隠しコマンドが有効か？
  FESROM_APP_RET_MODE   ret_mode;
  int                   init_pos;

  GFL_TCBLSYS*  pTcbSys;

  void*                 proc_wk;
  STRBUF*               str_tmp;

  FES_ROM_SAVE_WK       save_wk; ///<セーブワーク編集データ
  FES_ROM_SAVE          save;    ///<セーブオリジナル
} FES_ROM_SYS_WORK;

//==================================================================
/**
 * 現在の設定でセーブを実行する
 *
 * @param   control		
 */
//==================================================================
extern void FesRomSys_SaveWrite( FES_ROM_SYS_WORK* wk );

//==================================================================
/**
 * セーブデータがあればロードを行う
 *
 * @param   control		
 *
 * @retval  BOOL		TRUE:ロード成功　FALSE：データ無し
 */
//==================================================================
extern BOOL FesRomSys_SaveLoad(  FES_ROM_SYS_WORK* wk );

//==================================================================
/**
 * セーブデータ初期化
 *
 * @param   control		
 */
//==================================================================
extern void FesRomSys_SaveInit( FES_ROM_SAVE* sv );

extern void FesRomSys_MissionSaveInit( FES_ROM_SAVE* sv, int idx );

