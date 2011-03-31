//====================================================================================
/**
 *  @file   fmission_log_local.h
 *  @brief  フェスミッションログ　管理(NOTWIFI)
 *  @author Miyuki Iwasawa
 *  @date   11.03.09
 */
//====================================================================================

#pragma once

typedef enum{
  FMISSION_LOG_SET_NORMAL,  ///<通常モード
  FMISSION_LOG_SET_NEW,     ///<Newエントリーモード
  FMISSION_LOG_SET_MAP,     ///<マップチェンジモード
  FMISSION_LOG_SET_MAX,
}FMISSION_LOG_SET_MODE;

///ミッションログデータ
struct _FMISSION_LOG{
  STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];  //7+EOM 16byte
  STRCODE str[GAMEBEACON_FREEWORD_MESSAGE_LEN];  //8+EOM 18byte

  u16     tr_id;      //トレーナーID(下位2byte)
  u8      mine_f:1;   //プレイヤーフラグ
  u8      sex:1;      //性別
  u8      tr_view:6;  //ユニオン見た目
  u8      action;     //ログアクションNO
  u16     param;      //ログアクションパラメータ
};

///ミッションログデータテーブル
struct _FMISSION_LOG_TBL{
  struct  _FMISSION_LOG log[FMISSION_LOG_MAX];
  
  u8  ring_top;   //リングバッファ制御用 リストのトップindexを保持
  u8  entry_num;  //リング登録数
  u8  view_top;   //描画トップ
};

//------------------------------------------------------------------
/*
 *  ログクリア
 */
//------------------------------------------------------------------
extern void FMISSION_LOG_Clear( FMISSION_LOG* log );

//------------------------------------------------------------------
/*
 *  ログテーブルクリア
 */
//------------------------------------------------------------------
extern void FMISSION_LOG_TBL_Clear( FMISSION_LOG_TBL* tbl );

//------------------------------------------------------------------
/*
 *  ログスタック データセット
 *
 *  @param  tbl   FMISSION_LOG_TBL型構造体へのポインタ
 *  @param  info  セットするビーコン情報
 *  @param  new_f 新規エントリーならTRUE
 *  @return データをセットした配列の参照index
 *
 *  データがいっぱいの場合、先頭のデータをところてん式に押し出して末尾にセット
 */
//------------------------------------------------------------------
extern int FMISSION_LOG_TBL_SetLog( FMISSION_LOG_TBL* tbl, const FMISSION_LOG* p_log );


//------------------------------------------------------------------
/*
 *  ビーコンからミッションログパラメータを取得してスタックにセット
 *
 *  @param  tbl     FMISSION_LOG_TBL型構造体へのポインタ
 *  @param  info    セットするビーコン情報
 *  @param  mode    ログのセットモード enum FMISSION_LOG_SET_MODE
 *  @param  mine_f  自分の行動ログならTRUE
 *
 *  データがいっぱいの場合、先頭のデータをところてん式に押し出して末尾にセット
 */
//------------------------------------------------------------------
extern void FMISSION_LOG_TBL_SetLogFromBeacon( FMISSION_LOG_TBL* tbl, const GAMEBEACON_INFO *info, FMISSION_LOG_SET_MODE mode, BOOL mine_f );


