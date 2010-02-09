//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_data.h
 *	@brief  sysに置いて、coreでも使用するデータバッファ
 *	@author	Toru=Nagihashi
 *	@date		2009.12.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "poke_tool/pokeparty.h"
#include "buflen.h"
#include "system/pms_data.h"
#include "savedata/mystatus.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
/// 保存データ
//=====================================

//対戦者情報
typedef struct 
{
  u8        mystatus[MYSTATUS_SAVE_SIZE];
  u32       win_cnt;
  u32       lose_cnt;
  u32       btl_cnt;
  u32       rate;
  PMS_DATA  pms;
  u32       wificup_no;
  u8        pokeparty[0]; //後尾にポケパーティのデータがつく
} WIFIBATTLEMATCH_ENEMYDATA;

//対戦者情報のサイズ
#define WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE (sizeof(WIFIBATTLEMATCH_ENEMYDATA) + PokeParty_GetWorkSize())

#include "arc_def.h"
#include "message.naix"
#include "net_app/wifi_country.h"

//----------------------------------------------------------------------------
/**
 *	@brief  名前を不正だったときの名前に変更
 *
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *p_data   データ
 *	@param	heapID                              テンポラリ用ヒープ
 */
//-----------------------------------------------------------------------------
static inline void WIFIBATTLEMATCH_DATA_ModifiName( WIFIBATTLEMATCH_ENEMYDATA *p_data, HEAPID heapID )
{ 
  MYSTATUS  *p_mystatus = (MYSTATUS *)p_data->mystatus;
  STRBUF* p_strbuf;
  GFL_MSGDATA *p_msg;
  p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
      NARC_message_wifi_match_dat, GetHeapLowID(heapID) );
  p_strbuf  = GFL_MSG_CreateString( p_msg, GetHeapLowID(heapID) );

  //規定名に変更
  GFL_STR_GetStringCode( p_strbuf, p_mystatus->name, PERSON_NAME_SIZE+EOM_SIZE );

  GFL_STR_DeleteBuffer( p_strbuf );
  GFL_MSG_Delete( p_msg );
}

//----------------------------------------------------------------------------
/**
 *	@brief  対戦者情報が不正だった場合、無理やり正常値にする
 *
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *p_data ワーク
 */
//-----------------------------------------------------------------------------
static inline u32 WIFIBATTLEMATCH_DATA_ModifiEnemyData( WIFIBATTLEMATCH_ENEMYDATA *p_data, HEAPID heapID )
{
  MYSTATUS  *p_mystatus = (MYSTATUS *)p_data->mystatus;
  u32 dirty = 0;

  //名前が不正
  { 
    int i;
    BOOL is_darty = TRUE;
    //EOMが入っていれば正常とみなす
    //不正名は別なところで対処
    for( i = 0; i < PERSON_NAME_SIZE+EOM_SIZE; i++ )
    { 
      if( p_mystatus->name[i] == GFL_STR_GetEOMCode() )
      { 
        is_darty  = FALSE;
      }
    }

    if( is_darty )
    { 
      dirty++;
      WIFIBATTLEMATCH_DATA_ModifiName( p_data, heapID );
    }
  }

  //性別が不正
  if( p_mystatus->sex != PTL_SEX_MALE && p_mystatus->sex != PTL_SEX_FEMALE )
  { 
    OS_TPrintf( "性別が不正だったので、無理やり書き換えます %d\n", p_mystatus->sex );
    p_mystatus->sex = PTL_SEX_MALE;
    dirty++;
  }

  //トレーナービュー
  if( p_mystatus->trainer_view >= 8 )
  { 
    OS_TPrintf( "見た目が不正だったので、無理やり書き換えます \n" );
    p_mystatus->trainer_view  = 0;
    dirty++;
  }

  //場所
  if( p_mystatus->nation >= WIFI_COUNTRY_GetDataLen() )
  { 
    OS_TPrintf( "国が不正だったので、無理やり書き換えます %d %d\n", p_mystatus->nation, p_mystatus->area );
    p_mystatus->nation  = 0;
    dirty++;
  }

  //エリア
  { 
    if( p_mystatus->area >= WIFI_COUNTRY_CountryCodeToPlaceIndexMax( p_mystatus->nation ) )
    {
      OS_TPrintf( "地域が不正だったので、無理やり書き換えます %d %d\n", p_mystatus->nation, p_mystatus->area );
      p_mystatus->area  = 0;
    dirty++;
    }
  }

  //PMS
  if( !PMSDAT_IsValid( &p_data->pms, GetHeapLowID(heapID) ) )
  { 
    OS_TPrintf( "PMSが不正だったので、無理やり書き換えます\n" );
    PMSDAT_Init( &p_data->pms, PMS_TYPE_BATTLE_READY ); //何挨拶？ @todo
    dirty++;
  }

  return dirty;
}

#include "debug/debug_str_conv.h"
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグプリント表示
 *
 *	@param	const WIFIBATTLEMATCH_ENEMYDATA *cp_data  ワーク
 */
//-----------------------------------------------------------------------------
static inline void WIFIBATTLEMATCH_DATA_DebugPrint( const WIFIBATTLEMATCH_ENEMYDATA *cp_data )
{ 
  const MYSTATUS  *cp_mystatus = (const MYSTATUS *)cp_data->mystatus;

  OS_TFPrintf( 3, "○○プレイヤー情報\n" );
  { 
    char str[PERSON_NAME_SIZE+EOM_SIZE];
    DEB_STR_CONV_StrcodeToSjis( cp_mystatus->name, str, PERSON_NAME_SIZE+EOM_SIZE ); 
    OS_TFPrintf( 3, "name =[%s]\n", str );
  }
  OS_TFPrintf( 3, "name =[%d]\n", cp_mystatus->sex );
  OS_TFPrintf( 3, "view =[%d]\n", cp_mystatus->trainer_view );
  OS_TFPrintf( 3, "nation =[%d]\n", cp_mystatus->nation );
  OS_TFPrintf( 3, "area =[%d]\n", cp_mystatus->area );
  OS_TFPrintf( 3, "win =[%d]\n", cp_data->win_cnt );
  OS_TFPrintf( 3, "lose =[%d]\n", cp_data->lose_cnt );
  OS_TFPrintf( 3, "btl =[%d]\n", cp_data->btl_cnt );
  OS_TFPrintf( 3, "rate =[%d]\n", cp_data->rate );
  OS_TFPrintf( 3, "cup =[%d]\n", cp_data->wificup_no );

  { 
    int i;
    POKEMON_PARAM *p_pp;
    const POKEPARTY *cp_party  = (const POKEPARTY *)cp_data->pokeparty;
    OS_TFPrintf( 3, "poke_cnt =[%d]\n", PokeParty_GetPokeCount( cp_party) );
    for( i = 0; i < PokeParty_GetPokeCount( cp_party); i++ )
    { 
      p_pp  = PokeParty_GetMemberPointer(cp_party, i);
      OS_TFPrintf( 3, "pokemon[%d] \n", i );
      OS_TFPrintf( 3, "monsno =[%d]\n", PP_Get( p_pp, ID_PARA_monsno, NULL ) );
      OS_TFPrintf( 3, "itemno =[%d]\n", PP_Get( p_pp, ID_PARA_item, NULL ) );
    }
  }
} 
