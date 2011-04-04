///////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  名前入力イベント
 * @file   event_name_input.h
 * @author obata
 * @date   2009.10.20
 */
///////////////////////////////////////////////////////////////////////////////////////
#pragma once


//-------------------------------------------------------------------------------------
/**
 * @brief 手持ちポケモン名入力画面呼び出しイベント
 * 
 * @param gameSystem
 * @param ret_wk      入力結果を受け取るワーク ( TRUE:確定, FALSE:キャンセル )
 * @param party_index 名前を入力するポケモンのパーティー内インデックス
 * @param fade_flag   アプリ呼び出し前後に画面フェードをよぶかどうか
 *
 * @return イベント
 */
//-------------------------------------------------------------------------------------
extern GMEVENT* EVENT_NameInput_PartyPoke( 
    GAMESYS_WORK* gameSystem, u16* ret_wk, u16 party_index, BOOL fade_flag );


typedef enum
{
  RESORT_NAMEIN_TYPE_CITY_NAME,
  RESORT_NAMEIN_TYPE_AUTHER_NAME,
  RESORT_NAMEIN_TYPE_AUTHER_PHASE,
} RESORT_NAMEIN_TYPE;
extern GMEVENT* EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE type, GAMESYS_WORK* gameSystem, u16* ret_wk );
