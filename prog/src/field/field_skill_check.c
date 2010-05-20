//======================================================================
/**
 * @file	field_skill_check.c
 * @brief	フィールド技の状態チェック(技忘れよう
 * @author	ariizumi
 * @data	10/03/25
 *
 * モジュール名：FIELD_SKILL_CHECK
 */
//======================================================================
#include <gflib.h>
#include "field/field_skill_check.h"
#include "eventwork_def.h"
#include "field/field_status_local.h"   //FIELD_STATUS

#include "waza_tool/wazano_def.h"
#include "item/itemsym.h"
#include "item/item.h"

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

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	今使っている技ビットの取得
//--------------------------------------------------------------
const u16 FIELD_SKILL_CHECK_GetUseSkillBit( GAMEDATA *gameData )
{
  u16 bit = 0;

  //かいりき
  {
    EVENTWORK *ev = GAMEDATA_GetEventWork(gameData);
    if( EVENTWORK_CheckEventFlag(ev,SYS_FLAG_KAIRIKI) == TRUE )
    {
      bit += FSCB_KAIRIKI;
    }
  }
  //波乗り＆ダイビング
  {
    const PLAYER_WORK *playerWork = GAMEDATA_GetMyPlayerWork(gameData);
    const PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( playerWork );
    if( form == PLAYER_MOVE_FORM_SWIM )
    {
      bit += FSCB_NAMINORI;
    }
    if( form == PLAYER_MOVE_FORM_DIVING )
    {
      bit += FSCB_DIVING;
    }
  }
  //フラッシュ
  {
    FIELD_STATUS *fldStatus = GAMEDATA_GetFieldStatus( gameData );
    if( FIELD_STATUS_IsFieldSkillFlash( fldStatus ) == TRUE )
    {
      bit += FSCB_FLASH;
    }
  }

  return bit;
}

//--------------------------------------------------------------
//	指定技が忘れられるか？
//--------------------------------------------------------------
const FIELD_SKILL_CHECK_RET FIELD_SKILL_CHECK_CheckForgetSkill( GAMEDATA *gameData , const u16 wazaNo , HEAPID heapId )
{
  return (FSCR_OK+ITEM_CheckHidenWaza( wazaNo ));
}

//--------------------------------------------------------------
//	交換できるか？
//--------------------------------------------------------------
const BOOL FIELD_SKILL_CHECK_CanTradePoke( POKEMON_PASO_PARAM *ppp , HEAPID heapId )
{
  int i;
  for( i=0;i<4;i++ )
  {
    const u32 wazaNo = PPP_Get( ppp , ID_PARA_waza1+i , NULL );
    if( ITEM_CheckHidenWaza( wazaNo )){
      return FALSE;
    }
  }
  return TRUE;
}
