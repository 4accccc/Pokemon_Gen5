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
 *  ミッションアーカイブデータアーカイブは、フィールドメインループのバックグラウンド処理中で
 *  参照したい常駐データと、イベント発生時やアプリ中のみでだけ参照したい非常駐データで
 *  ファイルを分けています。
 */

/*
 * @brief ミッションアーカイブ 常駐データフォーマット
 *
 * この構造体の構成を変えた場合、resource/fmission_data/tool/fmission_dat_conv.rbを
 * 修正する必要があります
 */
#if 0
typedef struct _FMISSION_ARC_COMMON{
  u8   m_type;         ///<ミッションタイプ FMISSION_TYPE_ID
  u8   unit:4;         ///<表示単位 FMISSION_UNIT_ID
  u8   host:4;         ///<開催制限 FMISSION_HOST_TYPE
  u8   m_title_id;     ///<タイトルメッセージID
  u8   m_exp_id;       ///<説明文メッセージID
  u16  entry_flag_id;  ///<参加条件フラグID
  u16  clear_flag_id;  ///<ミッションクリア時に立てるフラグID
}FMISSION_ARC_COMMON;
#else
typedef struct _FMISSION_ARC_COMMON{
  u8   m_type;         ///<ミッションタイプ FMISSION_TYPE_ID
  u8   host;           ///<開催制限 FMISSION_HOST_TYPE
  u16  clear_flag_id;  ///<ミッションクリア時に立てるフラグID
}FMISSION_ARC_COMMON;
#endif
/*
 * @brief ミッションアーカイブ 非常駐データフォーマット
 *
 * この構造体の構成を変えた場合、resource/fmission_data/tool/fmission_dat_conv.rbを
 * 修正する必要があります
 */
typedef struct _FMISSION_ARC_PARAM{
  u8  end_type;       ///<クリアタイプ FMISSION_END_TYPE
  u8  location;       ///<範囲 FMISSION_LOCATION
  u8  time_bonus;     ///<ボーナスタイム FMISSION_TBONUS
  u8  pay_bonus;      ///<報酬計算式タイプ FMISSION_END_TYPE
  u8  sp_pay;         ///<ダブルスコアのスペシャルボーナス数
  u8  unit;           ///<表示単位 FMISSION_UNIT_ID
  u8  m_title_id;     ///<タイトルメッセージID
  u8  m_exp_id;       ///<説明文メッセージID

  u16  sp_item;        ///<ダブルスコアのスペシャルボーナスアイテム
  u16  base_param;     ///<汎用パラメータベース
  u16  ex_param;       ///<汎用パラメータ拡張
  u16  item;           ///<報酬アイテム
  u16  open_flag_id;   ///<ミッションの開放条件フラグID
  u16  entry_flag_id;  ///<参加条件フラグID
  u32  pattern;        ///<出現パターン(3bit*6パターン)
  FMISSION_ARCDATA_LV_PARAM level[FMISSION_LEVEL_MAX];
}FMISSION_ARC_PARAM;

/*
 * @brief ミッションアーカイブデータフォーマット
 *
 * この構造体の構成を変えた場合、resource/fmission_data/tool/fmission_dat_conv.rbを
 * 修正する必要があります
 */
typedef struct _FMISSION_ARCDATA{
  FMISSION_ARC_COMMON common;  ///<コモンパラメータ
  FMISSION_ARC_PARAM  param;    ///<非常駐パラメータ
}FMISSION_ARCDATA;

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
 *  @brief  フェスミッション常駐データをアロケートしたメモリに取得する
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
extern FMISSION_ARC_COMMON* FMISSION_DATA_AllocArcCommonAllByHandle( ARCHANDLE* handle, HEAPID heapID );

//------------------------------------------------------------
/*
 *  @brief  フェスミッション常駐データをアロケートしたメモリに取得する(アーカイブをその場で開く)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
extern FMISSION_ARC_COMMON* FMISSION_DATA_AllocArcCommonAllDirect( HEAPID heapID );

//------------------------------------------------------------
/*
 *  @brief  指定ミッションのパラメータをFMISSION_ARCDATA型ワークに取得する
 *  データハンドルは GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID ); であらかじめ取得してください
 */
//------------------------------------------------------------
extern void FMISSION_DATA_GetArcByHandle( ARCHANDLE* handle, FEST_MISSION_ID id, FMISSION_ARCDATA* fm_arc );

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(ハンドル版)
 *
 *  データハンドルは GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, heapID ); であらかじめ取得してください
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
extern FMISSION_ARCDATA* FMISSION_DATA_AllocArcAllByHandle( ARCHANDLE* handle, HEAPID heapID );

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(データマネージャ経由)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
extern FMISSION_ARCDATA* FMISSION_DATA_AllocArcAll( FMISSION_DATA_MAN* man, HEAPID heapID );

//------------------------------------------------------------
/*
 *  @brief  フェスミッションデータをアロケートしたメモリに取得する(アーカイブをその場で開く)
 *
 *  使用後はGFL_HEAP_FreeMemory()で開放してください
 */
//------------------------------------------------------------
extern FMISSION_ARCDATA* FMISSION_DATA_AllocArcAllDirect( HEAPID heapID );

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



