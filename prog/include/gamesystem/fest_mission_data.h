//=============================================================================
/**
 *  @file fest_mission_data.h
 *  @brief  フェスミッションアーカイブデータ操作
 *  @author Miyuki Iwasawa
 *  @date   11.02.22
 */
//=============================================================================

#pragma once

//@todo コンバータ作成時にアーカイブとして外部化する
typedef enum{
  FMISSION_ID_POKE_SEARCH_TABUNNE,
  FMISSION_ID_MAX,
  FMISSION_ID_END = FMISSION_ID_MAX-1,
}FEST_MISSION_ID;

///ミッションアーカイブデータフォーマット
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

///ミッションアーカイブデータマネージャー
typedef struct _FMISSION_DATA_MAN{
  HEAPID  heapID;
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
 *  @brief  指定ミッションのパラメータをFEST_MISSION型ワークに取得する
 */
//------------------------------------------------------------
extern void FMISSION_DATA_GetMissionParam( FMISSION_DATA_MAN* man, FEST_MISSION_ID id, FEST_MISSION* fmwk );



