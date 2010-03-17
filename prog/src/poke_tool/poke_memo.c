//======================================================================
/**
 * @file	poke_memo.c
 * @brief	トレーナーメモ設定・表示関数
 * @author	ariizumi
 * @data	10/03/13
 *
 * モジュール名：POKE_MEMO
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_memo.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef enum
{
  PMDT_1, //場所１・時間１  捕獲した～
  PMDT_2, //場所２・時間２  生まれた～
}POKE_MEMO_DATA_TYPE;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void POKE_MEMO_ClearPlaceTime( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_DATA_TYPE setType );
static void POKE_MEMO_SetPlaceTime( POKEMON_PASO_PARAM *ppp , const u32 place , const POKE_MEMO_DATA_TYPE setType );
static void POKE_MEMO_CopyPlaceTime( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_DATA_TYPE srcSetType );
static void POKE_MEMO_SetMyStatus( POKEMON_PASO_PARAM *ppp , const MYSTATUS* my , const HEAPID heapId );
static void POKE_MEMO_SetGetLevel( POKEMON_PASO_PARAM *ppp );
static void POKE_MEMO_SetRomVersion( POKEMON_PASO_PARAM *ppp );
static void POKE_MEMO_SetRomVersion( POKEMON_PASO_PARAM *ppp );


#pragma mark [>TrainerMemo
//--------------------------------------------------------------
//	トレーナーメモ設定
//--------------------------------------------------------------
void POKE_MEMO_SetTrainerMemoPP( POKEMON_PARAM *pp , const POKE_MEMO_SET_TYPE type ,
                                  const MYSTATUS* my, const u32 place , const HEAPID heapId )
{
  POKE_MEMO_SetTrainerMemoPPP( PP_GetPPPPointer(pp) , type , my , place , heapId );
}

void POKE_MEMO_SetTrainerMemoPPP( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_SET_TYPE type ,
                                  const MYSTATUS* my, const u32 place , const HEAPID heapId )
{
  const BOOL fastFlg = PPP_FastModeOn(ppp);
  switch( type )
  {
  case POKE_MEMO_SET_CAPTURE:  //捕獲
    POKE_MEMO_ClearPlaceTime( ppp , PMDT_2 );
    POKE_MEMO_SetPlaceTime( ppp , place , PMDT_1 );
    POKE_MEMO_SetMyStatus( ppp , my , heapId );
    POKE_MEMO_SetGetLevel( ppp );
    POKE_MEMO_SetRomVersion( ppp );
    break;

  case POKE_MEMO_GAME_TRADE:   //ゲーム内交換
    POKE_MEMO_SetPlaceTime( ppp , POKE_MEMO_PLACE_GAME_TRADE , PMDT_2 );
    POKE_MEMO_SetGetLevel( ppp );
    POKE_MEMO_SetRomVersion( ppp );
    break;

  case POKE_MEMO_INCUBATION:   //タマゴ孵化
    {
      const BOOL isMatchOya = PPP_IsMatchOya( ppp , my );
      if( isMatchOya == TRUE )
      {
        POKE_MEMO_SetPlaceTime( ppp , place , PMDT_2 );
        POKE_MEMO_SetMyStatus( ppp , my , heapId );
      }
      else
      {
        POKE_MEMO_CopyPlaceTime( ppp , PMDT_2 );
        POKE_MEMO_SetPlaceTime( ppp , place , PMDT_2 );
        POKE_MEMO_SetMyStatus( ppp , my , heapId );
      }
      POKE_MEMO_SetRomVersion( ppp );
    }
    break;

  case POKE_MEMO_POKE_SHIFTER: //ポケシフター(過去作転送
    POKE_MEMO_SetTrainerMemoPokeShifter( ppp );
    break;

  case POKE_MEMO_DISTRIBUTION: //配布
    POKE_MEMO_SetPlaceTime( ppp , place , PMDT_2 );
    POKE_MEMO_SetGetLevel( ppp );
    POKE_MEMO_SetRomVersion( ppp );
    break;
    
  case POKE_MEMO_EGG_FIRST:
    POKE_MEMO_SetPlaceTime( ppp , place , PMDT_1 );
    POKE_MEMO_SetMyStatus( ppp , my , heapId );
    POKE_MEMO_SetRomVersion( ppp );
    break;
    
  case POKE_MEMO_EGG_TRADE:
    POKE_MEMO_SetPlaceTime( ppp , place , PMDT_2 );
    break;
    
  }
  PPP_FastModeOff(ppp,fastFlg);
}

void POKE_MEMO_SetTrainerMemoPokeShifter( POKEMON_PASO_PARAM *ppp )
{
  const u32 monsNo = PPP_Get( ppp, ID_PARA_monsno , NULL );
  const u32 birthPlace = PPP_Get( ppp, ID_PARA_birth_place , NULL );
  const u32 isEvent = PPP_Get( ppp, ID_PARA_event_get_flag , NULL );
  const BOOL isRare = PPP_CheckRare( ppp );
  
  //通常対応
  POKE_MEMO_SetPlaceTime( ppp , POKE_MEMO_PLACE_POKE_SHIFTER , PMDT_2 );

  //2010イベント対応
  if( monsNo == MONSNO_SEREBHI &&
      birthPlace == POKE_MEMO_2010_MOVIE &&
      isEvent == 1 )
  {
    PPP_Put( ppp, ID_PARA_birth_place, POKE_MEMO_PLACE_SEREBIXI_BEFORE );
  }
  else
  if( ( monsNo == MONSNO_RAIKOU || monsNo == MONSNO_ENTEI || monsNo == MONSNO_SUIKUN ) &&
      birthPlace == POKE_MEMO_2010_MOVIE &&
      isRare == TRUE &&
      isEvent == 1 )
  {
    PPP_Put( ppp, ID_PARA_birth_place, POKE_MEMO_PLACE_ENRAISUI_BEFORE );
  }
}

static void POKE_MEMO_ClearPlaceTime( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_DATA_TYPE setType )
{
  if( setType == PMDT_1 )
  {
    PPP_Put( ppp , ID_PARA_get_place , 0 );
    PPP_Put( ppp , ID_PARA_get_year  , 0 );
    PPP_Put( ppp , ID_PARA_get_month , 0 );
    PPP_Put( ppp , ID_PARA_get_day   , 0 );
  }
  else
  {
    PPP_Put( ppp , ID_PARA_birth_place , 0 );
    PPP_Put( ppp , ID_PARA_birth_year  , 0 );
    PPP_Put( ppp , ID_PARA_birth_month , 0 );
    PPP_Put( ppp , ID_PARA_birth_day   , 0 );
  }
}

static void POKE_MEMO_SetPlaceTime( POKEMON_PASO_PARAM *ppp , const u32 place , const POKE_MEMO_DATA_TYPE setType )
{
  RTCDate date;
  GFL_RTC_GetDate( &date );
  
  if( setType == PMDT_1 )
  {
    PPP_Put( ppp , ID_PARA_get_place , place );
    PPP_Put( ppp , ID_PARA_get_year  , date.year );
    PPP_Put( ppp , ID_PARA_get_month , date.month );
    PPP_Put( ppp , ID_PARA_get_day   , date.day );
  }
  else
  {
    PPP_Put( ppp , ID_PARA_birth_place , place );
    PPP_Put( ppp , ID_PARA_birth_year  , date.year );
    PPP_Put( ppp , ID_PARA_birth_month , date.month );
    PPP_Put( ppp , ID_PARA_birth_day   , date.day );
  }
}

static void POKE_MEMO_CopyPlaceTime( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_DATA_TYPE srcSetType )
{
  if( srcSetType == PMDT_1 )
  {
    const u32 place = PPP_Get( ppp , ID_PARA_get_place , NULL );
    const u32 year  = PPP_Get( ppp , ID_PARA_get_year  , NULL );
    const u32 month = PPP_Get( ppp , ID_PARA_get_month , NULL );
    const u32 day   = PPP_Get( ppp , ID_PARA_get_day   , NULL );

    PPP_Put( ppp , ID_PARA_birth_place , place );
    PPP_Put( ppp , ID_PARA_birth_year  , year );
    PPP_Put( ppp , ID_PARA_birth_month , month );
    PPP_Put( ppp , ID_PARA_birth_day   , day );
  }
  else
  {
    const u32 place = PPP_Get( ppp , ID_PARA_birth_place , NULL );
    const u32 year  = PPP_Get( ppp , ID_PARA_birth_year  , NULL );
    const u32 month = PPP_Get( ppp , ID_PARA_birth_month , NULL );
    const u32 day   = PPP_Get( ppp , ID_PARA_birth_day   , NULL );

    PPP_Put( ppp , ID_PARA_get_place , place );
    PPP_Put( ppp , ID_PARA_get_year  , year );
    PPP_Put( ppp , ID_PARA_get_month , month );
    PPP_Put( ppp , ID_PARA_get_day   , day );
  }
}

static void POKE_MEMO_SetMyStatus( POKEMON_PASO_PARAM *ppp , const MYSTATUS* my , const HEAPID heapId )
{
  STRBUF *name  = MyStatus_CreateNameString( my, heapId );
  const u32 id  = MyStatus_GetID( my );
  const u32 sex = MyStatus_GetMySex( my );

  PPP_Put( ppp , ID_PARA_oyaname , (u32)name );
  PPP_Put( ppp , ID_PARA_oyasex , id );
  PPP_Put( ppp , ID_PARA_id_no , sex );
}

static void POKE_MEMO_SetGetLevel( POKEMON_PASO_PARAM *ppp )
{
  const u32 lv  = PPP_Get( ppp , ID_PARA_level , NULL );
  
  PPP_Put( ppp , ID_PARA_get_level , lv );
}

static void POKE_MEMO_SetRomVersion( POKEMON_PASO_PARAM *ppp )
{
  PPP_Put( ppp , ID_PARA_get_cassette , GET_VERSION() );
}

#pragma mark [>CheckEventPoke
const BOOL POKE_MEMO_CheckEventPokePP( POKEMON_PARAM *pp , const POKE_MEMO_EVENT_TYPE type )
{
  return POKE_MEMO_CheckEventPokePPP(PP_GetPPPPointer(pp),type);
}
const BOOL POKE_MEMO_CheckEventPokePPP( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_EVENT_TYPE type )
{
  const BOOL fastFlg = PPP_FastModeOn(ppp);

  const u32 monsNo = PPP_Get( ppp, ID_PARA_monsno , NULL );
  const u32 birthPlace = PPP_Get( ppp, ID_PARA_birth_place , NULL );
  const u32 isEvent = PPP_Get( ppp, ID_PARA_event_get_flag , NULL );
  const BOOL isRare = PPP_CheckRare( ppp );

  PPP_FastModeOff(ppp,fastFlg);

  switch( type )
  {
  case POKE_MEMO_EVENT_2010MOVIE_SEREBIXI_BEF:   
    //2010映画セレビィ(イベント前
    if( monsNo == MONSNO_SEREBHI &&
        birthPlace == POKE_MEMO_PLACE_SEREBIXI_BEFORE &&
        isEvent == 1 )
    {
      return TRUE;
    }
    break;

  case POKE_MEMO_EVENT_2010MOVIE_SEREBIXI_AFT:   
    //2010映画セレビィ(イベント後
    if( monsNo == MONSNO_SEREBHI &&
        birthPlace == POKE_MEMO_PLACE_SEREBIXI_AFTER &&
        isEvent == 1 )
    {
      return TRUE;
    }
    break;

  case POKE_MEMO_EVENT_2010MOVIE_ENRAISUI_BEF:   
    //2010映画エンテイ・ライコウ・スイクン(イベント前
    if( ( monsNo == MONSNO_RAIKOU || monsNo == MONSNO_ENTEI || monsNo == MONSNO_SUIKUN ) &&
        birthPlace == POKE_MEMO_PLACE_ENRAISUI_BEFORE &&
        isRare == TRUE &&
        isEvent == 1 )
    {
      return TRUE;
    }
    break;
  case POKE_MEMO_EVENT_2010MOVIE_ENRAISUI_AFT:
    //2010映画エンテイ・ライコウ・スイクン(イベント後
    if( ( monsNo == MONSNO_RAIKOU || monsNo == MONSNO_ENTEI || monsNo == MONSNO_SUIKUN ) &&
        birthPlace == POKE_MEMO_PLACE_ENRAISUI_AFTER &&
        isRare == TRUE &&
        isEvent == 1 )
    {
      return TRUE;
    }

  case POKE_MEMO_EVENT_DARUTANISU:
    //配布ダルタニス
    if( monsNo == MONSNO_DARUTANISU &&
        isEvent == 1 )
    {
      return TRUE;
    }
    break;

  case POKE_MEMO_EVENT_MERODHIA:
    //配布メロディア
    if( monsNo == MONSNO_MERODHIA &&
        isEvent == 1 )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//イベント後の処理を行う(2010映画配布用
void POKE_MEMO_SetEventPoke_AfterEventPP( POKEMON_PARAM *pp , const POKE_MEMO_EVENT_TYPE type )
{
  POKE_MEMO_SetEventPoke_AfterEventPPP( PP_GetPPPPointer(pp),type);
}

void POKE_MEMO_SetEventPoke_AfterEventPPP( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_EVENT_TYPE type )
{
  const BOOL fastFlg = PPP_FastModeOn(ppp);
  const BOOL flg = POKE_MEMO_CheckEventPokePPP( ppp , type );

  if( flg == TRUE )
  {
    switch(type)
    {
    case POKE_MEMO_EVENT_2010MOVIE_SEREBIXI_BEF:
      //2010映画セレビィ(イベント前
      PPP_Put( ppp, ID_PARA_birth_place, POKE_MEMO_PLACE_SEREBIXI_AFTER );
      break;
    case POKE_MEMO_EVENT_2010MOVIE_ENRAISUI_BEF:
      //2010映画エンテイ・ライコウ・スイクン(イベント前
      PPP_Put( ppp, ID_PARA_birth_place, POKE_MEMO_PLACE_ENRAISUI_AFTER );
      break;
    }
  }

  PPP_FastModeOff(ppp,fastFlg);
}
