//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_wfbc_data.c
 *	@brief  White Forest  Black City 基本データ
 *	@author	tomoya takahashi
 *	@date		2009.11.10
 *
 *	モジュール名：FIELD_WFBC_CORE
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "system/gfl_use.h"

#include "field/field_wfbc_data.h"

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

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static u32 WFBC_CORE_GetNotUseNpcID( const FIELD_WFBC_CORE* cp_wk, int idx );
static BOOL WFBC_CORE_IsUseNpc( const FIELD_WFBC_CORE* cp_wk, int npc_id );





//-------------------------------------
///	FIELD_WFBC_CORE用関数
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  WFBCワークのクリア
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_Crear( FIELD_WFBC_CORE* p_wk )
{
  int i;

  GF_ASSERT( p_wk );
  
  p_wk->data_in = FALSE;
  p_wk->type    = FIELD_WFBC_CORE_TYPE_MAX;

  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    FIELD_WFBC_CORE_PEOPLE_Crear( &p_wk->people[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  街の情報を設定する
 *
 *	@param	p_wk          ワーク
 *	@param  cp_mystatus   まいステータス
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_SetUp( FIELD_WFBC_CORE* p_wk )
{
  int i;
  int npc_idx;
  GF_ASSERT( p_wk );
  
  p_wk->data_in = TRUE;
#if PM_VERSION == VERSION_BLACK
  p_wk->type    = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
#else   // PM_VERSION == VERSION_BLACK
  p_wk->type    = FIELD_WFBC_CORE_TYPE_WHITE_FOREST; 
#endif  // PM_VERSION == VERSION_BLACK

  // 人を設定する
  for( i=0; i<FIELD_WFBC_INIT_PEOPLE_NUM; i++ )
  {
    npc_idx = GFUser_GetPublicRand(FIELD_WFBC_PEOPLE_MAX - i);
    if( 0 )
    {
      p_wk->people[i].data_in   = TRUE;
      p_wk->people[i].npc_id    = i;
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  WFBCデータの整合性チェック
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval TRUE    正常
 *	@retval FALSE   不正
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_CORE_IsConfomity( const FIELD_WFBC_CORE* cp_wk )
{
  int i;

  GF_ASSERT( cp_wk );
  
  if( (cp_wk->data_in == TRUE) && (cp_wk->type >= FIELD_WFBC_CORE_TYPE_MAX) )
  {
    return FALSE;
  }
  
  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    if( FIELD_WFBC_CORE_PEOPLE_IsConfomity( &cp_wk->people[i] ) == FALSE )
    {
      return FALSE;
    }
  }

  // OK
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ調整  不正データの場合は、正常な情報に書き換えます。
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_Management( FIELD_WFBC_CORE* p_wk )
{
  int i;

  GF_ASSERT( p_wk );
  
  // 一応データがない状態も管理しておく
  if( (p_wk->data_in == FALSE) && (p_wk->type != FIELD_WFBC_CORE_TYPE_MAX) )
  {
    p_wk->type = FIELD_WFBC_CORE_TYPE_MAX;
  }

  // 街タイプの調整
  if( (p_wk->data_in == TRUE) && (p_wk->type >= FIELD_WFBC_CORE_TYPE_MAX) )
  {
    // @TODO 自分か通信相手のROMコードに合わせるべき
    p_wk->type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
  }

  // 人物情報の調整
  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    FIELD_WFBC_CORE_PEOPLE_Management( &p_wk->people[i] );
  }
}

// データの有無   不正データの場合、FALSEを返します。
//----------------------------------------------------------------------------
/**
 *	@brief  データの有無チェック  不正データの場合、FALSEを返します。
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval TRUE  データあり
 *	@retval FALSE データなし
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_CORE_IsInData( const FIELD_WFBC_CORE* cp_wk )
{
  GF_ASSERT( cp_wk );
  
  if( !cp_wk->data_in )
  {
    return FALSE;
  }

  if( FIELD_WFBC_CORE_IsConfomity( cp_wk ) == FALSE )
  {
    return FALSE;
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人の数を返す
 *
 *	@param	cp_wk   ワーク
 *
 *	@return 人の数
 */
//-----------------------------------------------------------------------------
u32 FIELD_WFBC_CORE_GetPeopleNum( const FIELD_WFBC_CORE* cp_wk )
{
  int i;
  int num;
  GF_ASSERT( cp_wk );


  num = 0;

  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    if( FIELD_WFBC_CORE_PEOPLE_IsInData( &cp_wk->people[i] ) )
    {
      num ++;
    }
  }
  return num;
}


//-------------------------------------
///	FIELD_WFBC_CORE_PEOPLE用関数
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  人物ワークのクリア
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_PEOPLE_Crear( FIELD_WFBC_CORE_PEOPLE* p_wk )
{
  GF_ASSERT( p_wk );

  p_wk->data_in = FALSE;
  p_wk->npc_id  = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  情報の整合性チェック
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval TRUE    正常
 *	@retval FALSE   不正
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_CORE_PEOPLE_IsConfomity( const FIELD_WFBC_CORE_PEOPLE* cp_wk )
{
  GF_ASSERT( cp_wk );

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ調整  不正データの場合は、正常な情報に書き換えます。
 *
 *	@param	p_wk  人物ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_PEOPLE_Management( FIELD_WFBC_CORE_PEOPLE* p_wk )
{
  GF_ASSERT( p_wk );

  //
}

//----------------------------------------------------------------------------
/**
 *	@brief  データの有無   不正データの場合、FALSEを返します。
 *
 *	@param	cp_wk   人物ワーク
 *
 *	@retval TRUE    データあり
 *	@retval FALSE   データなし
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_CORE_PEOPLE_IsInData( const FIELD_WFBC_CORE_PEOPLE* cp_wk )
{
  GF_ASSERT( cp_wk );
  
  if( !cp_wk->data_in )
  {
    return FALSE;
  }
  if( FIELD_WFBC_CORE_PEOPLE_IsConfomity( cp_wk ) == FALSE )
  {
    return FALSE;
  }

  return TRUE;
}






//-----------------------------------------------------------------------------
/**
 *        private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  まだ、人物データとして保持していない人物のIDX番目を取得する
 *
 *	@param	cp_wk     ワーク
 *	@param	idx       インデックス
 */ 
//-----------------------------------------------------------------------------
static u32 WFBC_CORE_GetNotUseNpcID( const FIELD_WFBC_CORE* cp_wk, int idx )
{
#if 0
  int i;

  for(  )
  {
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brieif NPC_IDがワーク内の人物にいるのかチェック
 *
 *	@param	cp_wk     ワーク
 *	@param	npc_id    NPCID
 *
 *	@retval TRUE  いる
 *	@retval FALSE いない
 */
//-----------------------------------------------------------------------------
static BOOL WFBC_CORE_IsUseNpc( const FIELD_WFBC_CORE* cp_wk, int npc_id )
{
#if 0
  int i;

  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
  }
#endif
}


