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
  const u16 useSkillBit = FIELD_SKILL_CHECK_GetUseSkillBit( gameData );
  MYITEM_PTR myItem = GAMEDATA_GetMyItem( gameData );
  switch( wazaNo )
  {
  case WAZANO_IAIGIRI:
    if( MYITEM_CheckItem( myItem , ITEM_WAZAMASIN94 , 1 , heapId ) == FALSE )
    {
      return FSCR_NO_MACHINE;
    }
    break;
  case WAZANO_KAIRIKI:
    if( MYITEM_CheckItem( myItem , ITEM_WAZAMASIN95 , 1 , heapId ) == FALSE )
    {
      return FSCR_NO_MACHINE;
    }
    if( useSkillBit & FSCB_KAIRIKI )
    {
      return FSCR_USE_SKILL;
    }
    break;
  case WAZANO_NAMINORI:
    if( MYITEM_CheckItem( myItem , ITEM_WAZAMASIN96 , 1 , heapId ) == FALSE )
    {
      return FSCR_NO_MACHINE;
    }
    if( useSkillBit & FSCB_NAMINORI )
    {
      return FSCR_USE_SKILL;
    }
    break;
  case WAZANO_SORAWOTOBU:
    if( MYITEM_CheckItem( myItem , ITEM_WAZAMASIN97 , 1 , heapId ) == FALSE )
    {
      return FSCR_NO_MACHINE;
    }
    break;
  case WAZANO_DAIBINGU:
    if( MYITEM_CheckItem( myItem , ITEM_WAZAMASIN98 , 1 , heapId ) == FALSE )
    {
      return FSCR_NO_MACHINE;
    }
    if( useSkillBit & FSCB_DIVING )
    {
      return FSCR_USE_SKILL;
    }
    break;
  case WAZANO_TAKINOBORI:
    if( MYITEM_CheckItem( myItem , ITEM_WAZAMASIN99 , 1 , heapId ) == FALSE )
    {
      return FSCR_NO_MACHINE;
    }
    break;
  case WAZANO_IWAKUDAKI:
    if( MYITEM_CheckItem( myItem , ITEM_WAZAMASIN100 , 1 , heapId ) == FALSE )
    {
      return FSCR_NO_MACHINE;
    }
    break;
  case WAZANO_HURASSYU:
    if( useSkillBit & FSCB_FLASH )
    {
      return FSCR_USE_SKILL;
    }
    break;
  }
  
  return FSCR_OK;
}

//--------------------------------------------------------------
//	交換できるか？
//--------------------------------------------------------------
const BOOL FIELD_SKILL_CHECK_CanTradePoke( POKEMON_PASO_PARAM *ppp , HEAPID heapId )
{
  u8 i;
  for( i=0;i<4;i++ )
  {
    const u32 wazaNo = PPP_Get( ppp , ID_PARA_waza1+i , NULL );
    if( wazaNo == WAZANO_IAIGIRI ||
        wazaNo == WAZANO_KAIRIKI ||
        wazaNo == WAZANO_NAMINORI ||
        wazaNo == WAZANO_SORAWOTOBU ||
        wazaNo == WAZANO_DAIBINGU ||
        wazaNo == WAZANO_TAKINOBORI ||
        wazaNo == WAZANO_IWAKUDAKI ||
        wazaNo == WAZANO_HURASSYU )
    {
      return FALSE;
    }
  }
  return TRUE;
}
