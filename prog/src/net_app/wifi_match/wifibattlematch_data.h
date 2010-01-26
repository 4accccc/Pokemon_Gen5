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
  STRCODE   name[PERSON_NAME_SIZE+EOM_SIZE];
  u8        sex;
  u8        trainer_view;
  u8        nation;
  u8        area;
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
 *	@brief  対戦者情報が不正だった場合、無理やり正常値にする
 *
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *p_data ワーク
 */
//-----------------------------------------------------------------------------
static inline u32 WIFIBATTLEMATCH_DATA_ModifiEnemyData( WIFIBATTLEMATCH_ENEMYDATA *p_data, HEAPID heapID )
{
  u32 dirty = 0;

  //名前が不正
  { 
    int i;
    BOOL is_darty = TRUE;
    //EOMが入っていれば正常とみなす
    for( i = 0; i < PERSON_NAME_SIZE+EOM_SIZE; i++ )
    { 
      if( p_data->name[i] == GFL_STR_GetEOMCode() )
      { 
        is_darty  = FALSE;
        dirty++;
      }
    }

    if( is_darty )
    { 
      STRBUF* p_strbuf;
      GFL_MSGDATA *p_msg;
      p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
												NARC_message_wifi_match_dat, GetHeapLowID(heapID) );
      p_strbuf  = GFL_MSG_CreateString( p_msg, GetHeapLowID(heapID) );

      //規定名に変更
      OS_TPrintf( "対戦者名が不正だったので、無理やり書き換えます\n" );
      GFL_STR_GetStringCode( p_strbuf, p_data->name, PERSON_NAME_SIZE+EOM_SIZE );

      GFL_STR_DeleteBuffer( p_strbuf );
      GFL_MSG_Delete( p_msg );
    }
  }

  //性別が不正
  if( p_data->sex != PTL_SEX_MALE && p_data->sex != PTL_SEX_FEMALE )
  { 
    OS_TPrintf( "性別が不正だったので、無理やり書き換えます %d\n", p_data->sex );
    p_data->sex = PTL_SEX_MALE;
    dirty++;
  }

  //トレーナービュー
  //@todo
  if( p_data->trainer_view == 0xFFFF )
  { 
    OS_TPrintf( "見た目が不正だったので、無理やり書き換えます \n" );
    p_data->trainer_view  = 0;
    dirty++;
  }

  //場所
  if( p_data->nation >= WIFI_COUNTRY_GetDataLen() )
  { 
    OS_TPrintf( "国が不正だったので、無理やり書き換えます %d %d\n", p_data->nation, p_data->area );
    p_data->nation  = 0;
    dirty++;
  }

  //エリア
  { 
    if( p_data->area >= WIFI_COUNTRY_CountryCodeToPlaceIndexMax( p_data->nation ) )
    {
      OS_TPrintf( "地域が不正だったので、無理やり書き換えます %d %d\n", p_data->nation, p_data->area );
      p_data->area  = 0;
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

static inline void WIFIBATTLEMATCH_DATA_ModifiName( WIFIBATTLEMATCH_ENEMYDATA *p_data, HEAPID heapID )
{ 
  STRBUF* p_strbuf;
  GFL_MSGDATA *p_msg;
  p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
      NARC_message_wifi_match_dat, GetHeapLowID(heapID) );
  p_strbuf  = GFL_MSG_CreateString( p_msg, GetHeapLowID(heapID) );

  //規定名に変更
  GFL_STR_GetStringCode( p_strbuf, p_data->name, PERSON_NAME_SIZE+EOM_SIZE );

  GFL_STR_DeleteBuffer( p_strbuf );
  GFL_MSG_Delete( p_msg );
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
  OS_TFPrintf( 3, "○○プレイヤー情報\n" );
  { 
    char str[PERSON_NAME_SIZE+EOM_SIZE];
    DEB_STR_CONV_StrcodeToSjis( cp_data->name, str, PERSON_NAME_SIZE+EOM_SIZE ); 
    OS_TFPrintf( 3, "name =[%s]\n", str );
  }
  OS_TFPrintf( 3, "name =[%d]\n", cp_data->sex );
  OS_TFPrintf( 3, "view =[%d]\n", cp_data->trainer_view );
  OS_TFPrintf( 3, "nation =[%d]\n", cp_data->nation );
  OS_TFPrintf( 3, "area =[%d]\n", cp_data->area );
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
