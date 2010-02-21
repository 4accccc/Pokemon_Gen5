//======================================================================
/**
 * @file  p_status.c
 * @brief ポケモンステータス
 * @author  ariizumi
 * @data  09/07/01
 *
 * モジュール名：POKE_STATUS
 */
//======================================================================
#pragma once

#include "gamesystem/game_data.h"
#include "poke_tool/pokeparty.h"
#include "savedata/config.h"


//-----------------------------------------------
//以下GSより移植(09/07/01)
//enumにtypedefで型追加
//-----------------------------------------------
typedef struct {
  void * ppd;   // ポケモンデータ
  CONFIG * cfg; // コンフィグデータ
  GAMEDATA *game_data; //ゲームデータ

  //game_dataから取得するように変更
  //const STRCODE * player_name;  // 見ている人の名前
  //u32 player_id;          // 見ている人のID
  //u8  player_sex;         // 見ている人の性別

  u8  ppt;    // パラメータタイプ
  u8  mode;   // モード
  u8  max;    // 最大数
  u8  pos;    // 何番目のデータか

  u8  page;     //初期表示ページ  //PSTATUS_PAGE_TYPE //技モードでは無効
  u8  ret_sel;  // 選択された技位置
  u8  ret_mode;
  u16 waza;     //技追加時は技番号。0で選択モード

  u32       zukan_mode; // 図鑑ナンバー表示モード

  //void      *poruto;  // 与えるポルトのデータ ( _PURUTO_DATA )

  //PERAPVOICE    *perap;   // ぺラップの鳴き声データ

//  BOOL      ev_contest; // コンテスト用
//  BOOL      pokethlon;  // ポケスロンパフォーマンス画面表示かどうか
  BOOL      waza_chg; // 技を入れ替えたかどうか TRUE = 入れ替えた, FALSE = それ以外

  BOOL      canExitButton;  //X戻りが使えるか？
  BOOL      isExitRequest;  //時間切れ時、終了リクエストを出す
  

}PSTATUS_DATA;

// モード
enum {
  PST_MODE_NORMAL = 0,  // 通常
  PST_MODE_NO_WAZACHG,  // 技入れ替え不可
  PST_MODE_WAZAADD,   // 技覚え/技忘れ
  PST_MODE_PORUTO,    // ポルト使用       （使わない）
  PST_MODE_CONDITION,   // コンディション変更デモ （使わない）
};

// パラメータタイプ
enum {
  PST_PP_TYPE_POKEPARAM = 0,  // POKEMON_PARAM
  PST_PP_TYPE_POKEPARTY,      // POKEPARTY
  PST_PP_TYPE_POKEPASO,       // POKEMON_PASO_PARAM
#if PM_DEBUG
  PST_PP_TYPE_DEBUG,
#endif
};

// 戻り
enum {
  PST_RET_DECIDE = 0,
  PST_RET_CANCEL,
  PST_RET_EXIT,
  
};


typedef enum
{
  PPT_INFO,     //ポケモン情報
  PPT_SKILL,    //技
  PPT_RIBBON,   //リボン
  
  PPT_SKILL_ADD,    //技系

  PPT_MAX,
}PSTATUS_PAGE_TYPE;

//-----------------------------------------------
//以上GSより移植(09/07/01)
//-----------------------------------------------

extern GFL_PROC_DATA PokeStatus_ProcData;
