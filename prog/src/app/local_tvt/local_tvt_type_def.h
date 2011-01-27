//======================================================================
/**
 * @file	local_tvt_type_def.h
 * @brief	非通信テレビトランシーバー タイプ定義(コンバータで読み込む)
 * @author	ariizumi
 * @data	09/12/12
 *
 * モジュール名：LOCAL_TVT
 */
//======================================================================
#pragma once

//-------------------------------------
///	キャラクター定義
//=====================================
typedef enum
{
  LTCT_PLAYER_M,  //主人公だった場合分岐させる
  LTCT_RIVAL,
  LTCT_CHELEN,
  LTCT_SUPPORT,
  LTCT_DOCTOR_D,
  LTCT_MOTHER,
  LTCT_SECRET_M,  //隠しキャラ男
  LTCT_SECRET_F,  //隠しキャラ女
  LTCT_SOUNDONLY, //音声のみ

  LTCT_PLAYER_F,  //主人公だった場合分岐させる
  
  LTCT_MAX,
  
  LTCT_NONE = 0xFF,
}LOCAL_TVT_CHARA_TYPE;

//-------------------------------------
///	天候定義
//=====================================
typedef enum
{
  LOCAL_TVT_WEATHER_RAIN, //雨
  LOCAL_TVT_WEATHER_SAND, //砂嵐
  LOCAL_TVT_WEATHER_SNOW, //雪
  LOCAL_TVT_WEATHER_SUNNY,//晴れ（なし）
  LOCAL_TVT_WEATHER_MAX = LOCAL_TVT_WEATHER_SUNNY,
}LOCAL_TVT_WEATHER;

//-------------------------------------
///	背景定義
//=====================================
typedef enum
{
  LOCAL_TVT_BACK_ROOM,    //部屋
  LOCAL_TVT_BACK_DUNGEON, //洞窟
  LOCAL_TVT_BACK_CITY,    //街
  LOCAL_TVT_BACK_PLAIN,   //平野
  LOCAL_TVT_BACK_MAUNTAIN,//山
  LOCAL_TVT_BACK_SEA,     //空（海）
  LOCAL_TVT_BACK_DESERT,  //砂漠

  LOCAL_TVT_BACK_MAX,
}LOCAL_TVT_BACK;

//-------------------------------------
///	タイムゾーン定義
//=====================================
typedef enum
{
  LTTZ_NOON,    //朝
  LTTZ_EVENING, //昼
  LTTZ_NIGHT,   //夜
  
  LTTZ_MAX,
}LOCAL_TVT_TIME_ZONE;
