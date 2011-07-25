//=============================================================================
/**
 *  @file fest_mission_data.h
 *  @brief  フェスミッションアーカイブデータ操作
 *  @author Miyuki Iwasawa
 *  @date   11.02.22
 */
//=============================================================================

#pragma once

#include "msgdata.h"
#include "gamesystem/fmission_data_id.h"
#include "gamesystem/fmission_data_def.h"

///<ミッションアーカイブデータ 目標値と報酬
typedef struct _FMISSION_ARCDATA_LV_PARAM{
  u16 time;   ///<制限時間
  u8  goal;   ///<目標値
  u8  pay;    ///<報酬
}FMISSION_ARCDATA_LV_PARAM;

/*
 * @brief ミッションアーカイブデータフォーマット
 *
 * この構造体の構成を変えた場合、resource/fmission_data/tool/fmission_dat_conv.rbを
 * 修正する必要があります
 */
#if 1
typedef struct _FMISSION_ARCDATA{
  u8  m_type;         ///<ミッションタイプ
  u8  end_type;       ///<クリアタイプ
  u8  location;       ///<範囲
  u8  host;           ///<開催制限
  
  u8  unit;           ///<表示単位
  u8  dummy;          ///<パディング

  u16 param;          ///<汎用パラメータ
  u16 entry_flag_id;  ///<参加条件フラグID
  u16 item;           ///<報酬アイテム

  u8  time_bonus;     ///<ボーナスタイム
  u8  pay_bonus;      ///<報酬計算式タイプ
  u8  m_title_id;     ///<タイトルメッセージID
  u8  m_exp_id;       ///<説明文メッセージID

  FMISSION_ARCDATA_LV_PARAM level[FMISSION_LEVEL_MAX];
}FMISSION_ARCDATA;

#else
typedef struct _FMISSION_ARCDATA{
  //1
  u16   type:4;     //ミッションタイプ 4bit(15種類 FMISSION_TYPE_ID型)
  u16   time:12;    //ミッション時間(秒、最大1時間) 12bit(3600s/4095s)
  u16   param;      //ミッションパラメータ            16bit
  //2
  u32   target:10;    ///<達成目標
  u32   item:10;      ///<報酬のアイテムNo 
  u32   item_num:8;   ///<報酬数の基礎値
  u32   calc_type:3;  ///<報酬計算式タイプ
  u32   end_type:1;   ///<終了条件タイプ

  //3
  u8    bonus;        //スコアボーナス 4bit(max15秒or15パターン 現在はx*10でmax150秒ボーナス)

}FMISSION_ARCDATA;
#endif
///ミッションアーカイブデータマネージャー
typedef struct _FMISSION_DATA_MAN{
  ARCHANDLE* data_h;    ///<ミッションデータアーカイブハンドル
  GFL_MSGDATA* msg;     ///<フェスミッション用メッセージデータ
}FMISSION_DATA_MAN;

//------------------------------------------------------------
/*
 *  @brief  ミッションデータマネージャー初期化
 */
//------------------------------------------------------------
extern FMISSION_DATA_MAN*  FMISSION_DATA_Init( HEAPID heapID );

//------------------------------------------------------------
/*
 *  @brief  ミッションデータマネージャー開放
 */
//------------------------------------------------------------
extern void FMISSION_DATA_Release( FMISSION_DATA_MAN* man );

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFMISSION_ARCDATA型ワークに取得する
 *  データハンドルは GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID ); であらかじめ取得してください
 */
//------------------------------------------------------------
extern void FMISSION_DATA_GetArcParamByHandle( ARCHANDLE* handle, FEST_MISSION_ID id, FMISSION_ARCDATA* fm_arc );

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(ハンドル版)
 *
 *  データハンドルは GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID ); であらかじめ取得してください
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
extern FMISSION_ARCDATA* FMISSION_DATA_AllocArcParamAllByHandle( ARCHANDLE* handle, HEAPID heapID );

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(データマネージャ経由)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
extern FMISSION_ARCDATA* FMISSION_DATA_AllocArcParamAll( FMISSION_DATA_MAN* man, HEAPID heapID );

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(アーカイブをその場で開く)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
extern FMISSION_ARCDATA* FMISSION_DATA_AllocArcParamAllDirect( HEAPID heapID );

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFEST_MISSION型ワークに取得する(ハンドル指定版)
 */
//------------------------------------------------------------
extern void FMISSION_DATA_GetMissionParamByHandle( ARCHANDLE* handle, FEST_MISSION_ID id, FMISSION_LEVEL level, FEST_MISSION* fmwk );

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFEST_MISSION型ワークに取得する(データマネージャ経由)
 */
//------------------------------------------------------------
void FMISSION_DATA_GetMissionParam( FMISSION_DATA_MAN* man, FEST_MISSION_ID id, FMISSION_LEVEL level, FEST_MISSION* fmwk );

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFEST_MISSION型ワークに取得する(アーカイブをその場で開く)
 */
//------------------------------------------------------------
extern void FMISSION_DATA_GetMissionParamDirect( FEST_MISSION_ID id, FMISSION_LEVEL level, FEST_MISSION* fmwk, HEAPID heapID );

//---------------------------------------------------------
/*
 * @brief ミッションパラメータ一括ワードセット
 */
//---------------------------------------------------------
extern void FMISSION_DATA_WordsetParamAll( WORDSET* wset, FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッションタイトル文字列取得
 *
 * ミッションパラメータからミッションタイトル文字列を生成
 */
//---------------------------------------------------------
extern void FMISSION_DATA_GetTitleStr( FMISSION_DATA_MAN* man, STRBUF* str, const FEST_MISSION* fmwk, HEAPID heapID );

//---------------------------------------------------------
/*
 * @brief ミッション説明文文字列取得
 *
 * ミッションパラメータからミッション説明文を生成
 */
//---------------------------------------------------------
extern void FMISSION_DATA_GetExpStr( FMISSION_DATA_MAN* man, STRBUF* str, const FEST_MISSION* fmwk, HEAPID heapID );



