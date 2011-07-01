//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_wfbc.c
 *	@brief  White Forest  Black City 
 *	@author	tomoya takahashi
 *	@date		2009.11.09
 *
 *	モジュール名：FIELD_WFBC
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "arc/arc_def.h"
#include "arc/field_wfbc_data.naix"
#include "arc/fieldmap/eventdata.naix"

#include "savedata/mystatus.h"

#include "field/field_const.h"

#include "field_wfbc.h"
#include "land_data_patch.h"

#include "eventdata_local.h"



//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	WFBCワーク
//=====================================
struct _FIELD_WFBC 
{
  u8    type;     // WF / BC FIELD_WFBC_CORE_TYPE
  u8    mapmode;  // マップモードMAPMODE

  // mapmodeがMAPMODE_NORMALなら、即時に変更がセーブデータに設定される。
  FIELD_WFBC_CORE* p_core;  
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	FIELD_WFBC
//=====================================
static void WFBC_InitSystem( FIELD_WFBC* p_wk, HEAPID heapID );
static void WFBC_ExitSystem( FIELD_WFBC* p_wk );

static void WFBC_Clear( FIELD_WFBC* p_wk );

static void WFBC_SetCore( FIELD_WFBC* p_wk, FIELD_WFBC_CORE* p_buff, MAPMODE mapmode );
static void WFBC_GetCore( const FIELD_WFBC* cp_wk, FIELD_WFBC_CORE* p_buff );

static void WFBC_ReadCore( FIELD_WFBC* p_wk );
static void WFBC_UpdateCore( FIELD_WFBC* p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  WFBC用ワークの生成
 *
 *	@param	heapID  ヒープID
 *
 *	@return WFBCワーク
 */
//-----------------------------------------------------------------------------
FIELD_WFBC* FIELD_WFBC_Create( HEAPID heapID )
{
  FIELD_WFBC* p_wk;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_WFBC) );

  WFBC_Clear( p_wk );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WFBC用ワークの破棄
 *
 *	@param	p_wk  ワーク
 */ 
//-----------------------------------------------------------------------------
void FIELD_WFBC_Delete( FIELD_WFBC* p_wk )
{
  GF_ASSERT( p_wk );

  WFBC_ExitSystem( p_wk );

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  WFBCCOREデータを取得する
 *
 *	@param	cp_wk   ワーク
 *	@param	p_buff  格納バッファ
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_GetCore( const FIELD_WFBC* cp_wk, FIELD_WFBC_CORE* p_buff )
{
  WFBC_GetCore( cp_wk, p_buff );
}

//----------------------------------------------------------------------------
/**
 *	@brief  WFBCCOREデータを最新にする。
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_ReadCore( FIELD_WFBC* p_wk )
{
  WFBC_ReadCore( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  WFBCの街タイプを取得
 *
 *	@param	cp_wk   ワーク
 *
 *	@return 街タイプ
 */
//-----------------------------------------------------------------------------
FIELD_WFBC_CORE_TYPE FIELD_WFBC_GetType( const FIELD_WFBC* cp_wk )
{
  GF_ASSERT( cp_wk );
  return cp_wk->type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WFBCのマップモードの取得
 *
 *	@param	cp_wk   ワーク
 *
 *	@return マップモード
 */
//-----------------------------------------------------------------------------
MAPMODE FIELD_WFBC_GetMapMode( const FIELD_WFBC* cp_wk )
{
  GF_ASSERT( cp_wk );
  return cp_wk->mapmode;
}

//----------------------------------------------------------------------------
/**
 *	@brief  街情報を設定
 *
 *	@param	p_wk        ワーク
 *	@param	cp_core     コア情報
 *	@param  mapmode     マップモード
 *	@param  heapID      ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_SetUp( FIELD_WFBC* p_wk, FIELD_WFBC_CORE* p_core, MAPMODE mapmode, HEAPID heapID )
{
  // 内部ワークの初期か
  WFBC_InitSystem( p_wk, heapID );
  
  WFBC_SetCore( p_wk, p_core, mapmode );
}

//----------------------------------------------------------------------------
/**
 *	@brief  街情報をクリア
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_Clear( FIELD_WFBC* p_wk )
{
  WFBC_Clear( p_wk );
  WFBC_ExitSystem( p_wk );
}

//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  必要システムの初期化
 *
 *	@param	p_wk        ワーク
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
static void WFBC_InitSystem( FIELD_WFBC* p_wk, HEAPID heapID )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  必要システムの破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void WFBC_ExitSystem( FIELD_WFBC* p_wk )
{
}


//----------------------------------------------------------------------------
/**
 *	@brief  WFBCワークをクリア
 */
//-----------------------------------------------------------------------------
static void WFBC_Clear( FIELD_WFBC* p_wk )
{
  int i;

  p_wk->type = 0;
  p_wk->mapmode = 0;
  p_wk->p_core = NULL;
}


//----------------------------------------------------------------------------
/**
 *	@brief  CORE情報を設定する
 *
 *	@param	p_wk        ワーク
 *	@param	cp_buff     バッファ
 */
//-----------------------------------------------------------------------------
static void WFBC_SetCore( FIELD_WFBC* p_wk, FIELD_WFBC_CORE* p_buff, MAPMODE mapmode )
{
  BOOL result;

  GF_ASSERT( p_wk );
  GF_ASSERT( p_buff );

  result = FIELD_WFBC_CORE_IsInData( p_buff );
  GF_ASSERT( result );
  
  // 現状方WFBC情報をクリーンアップ
  WFBC_Clear( p_wk );

  // 情報を設定
  p_wk->type    = p_buff->type;
  p_wk->mapmode = mapmode;

  // コア情報保存
  if( mapmode == MAPMODE_NORMAL )
  {
    p_wk->p_core = p_buff;
  }
  else
  {
    p_wk->p_core = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  CORE情報を取得する
 *
 *	@param	cp_wk     ワーク
 *	@param	p_buff    データ格納先
 */
//-----------------------------------------------------------------------------
static void WFBC_GetCore( const FIELD_WFBC* cp_wk, FIELD_WFBC_CORE* p_buff )
{
  int i;

  GF_ASSERT( cp_wk );
  GF_ASSERT( p_buff );

  // 情報を格納
  p_buff->data_in = TRUE;
  p_buff->type    = cp_wk->type;

}

//----------------------------------------------------------------------------
/**
 *	@brief  現状の人物情報を巻き戻す。
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void WFBC_ReadCore( FIELD_WFBC* p_wk )
{
  int i;
  
  if( (p_wk->mapmode == MAPMODE_NORMAL) && (p_wk->p_core != NULL) )
  {
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップモードが通常時のセーブデータ更新処理
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void WFBC_UpdateCore( FIELD_WFBC* p_wk )
{
  if( (p_wk->mapmode == MAPMODE_NORMAL) && (p_wk->p_core != NULL) )
  {
    // 情報の格納
    FIELD_WFBC_GetCore( p_wk, p_wk->p_core );
  }
}


//デバッグ操作
#ifdef PM_DEBUG

#include "gamesystem/game_data.h"
#include "poke_tool/pokeparty.h"
#include "msg/msg_place_name.h"  // for MAPNAME_xxxx
#include "system/main.h"  // 


static void DEBWIN_Update_CityType( void* userWork , DEBUGWIN_ITEM* item );
static void DEBWIN_Draw_CityType( void* userWork , DEBUGWIN_ITEM* item );


void FIELD_FUNC_RANDOM_GENERATE_InitDebug( HEAPID heapId, void* p_gdata )
{
  
  DEBUGWIN_AddGroupToTop( 10 , "RandomMap" , heapId );
  DEBUGWIN_AddItemToGroupEx( DEBWIN_Update_CityType ,DEBWIN_Draw_CityType , 
                             p_gdata , 10 , heapId );

}

void FIELD_FUNC_RANDOM_GENERATE_TermDebug( void )
{
  DEBUGWIN_RemoveGroup( 10 );
}

static void DEBWIN_Update_CityType( void* userWork , DEBUGWIN_ITEM* item )
{
  GAMEDATA* p_gdata = userWork;
  FIELD_WFBC_CORE* p_wk = GAMEDATA_GetMyWFBCCoreData( p_gdata );
  u8 type = p_wk->type;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT || 
      GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT  ||
      GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A )
  {

    if( type == FIELD_WFBC_CORE_TYPE_BLACK_CITY )
    {
      type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
    }
    else
    {
      type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
    }
    p_wk->type = type;
    DEBUGWIN_RefreshScreen();

    FIELD_WFBC_CORE_SetUpZoneData( p_wk );
  }
}

static void DEBWIN_Draw_CityType( void* userWork , DEBUGWIN_ITEM* item )
{
  GAMEDATA* p_gdata = userWork;
  FIELD_WFBC_CORE* p_wk = GAMEDATA_GetMyWFBCCoreData( p_gdata );
  u8 type = p_wk->type;

  if( type == FIELD_WFBC_CORE_TYPE_BLACK_CITY )
  {
    DEBUGWIN_ITEM_SetName( item , "Type[BLACK CITY]" );
  }
  else
  {
    DEBUGWIN_ITEM_SetName( item , "Type[WHITE FOREST]" );
  }
}



#endif //PM_DEBUG


