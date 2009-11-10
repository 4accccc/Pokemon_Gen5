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

#include "field_wfbc.h"

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
///	人物情報
//=====================================
struct _FIELD_WFBC_PEOPLE 
{
  u8  status;   // FIELD_WFBC_PEOPLE_STATUS
  u8  pad;
  u16 objcode;  // 見た目
};

//-------------------------------------
///	WFBCワーク
//=====================================
struct _FIELD_WFBC 
{
  FIELD_WFBC_PEOPLE people[ FIELD_WFBC_PEOPLE_MAX ];
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	FIELD_WFBC
//=====================================
static int WFBC_GetClearPeople( const FIELD_WFBC* cp_wk );
static int WFBC_GetPeopleNum( const FIELD_WFBC* cp_wk );

//-------------------------------------
///	FIELD_WFBC_PEOPLE
//=====================================
static void WFBC_PEOPLE_Clear( FIELD_WFBC_PEOPLE* p_people );


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
  int i;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_WFBC) );

  //　人物ワークをクリア
  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    WFBC_PEOPLE_Clear( &p_wk->people[i] );
  }
  
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

  GFL_HEAP_FreeMemory( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  WFBC用高さテーブルインデックスを返す
 *
 *	@param	cp_wk   ワーク
 *
 *	@return 高さテーブルのインデックス
 */
//-----------------------------------------------------------------------------
u32 FIELD_WFBC_GetPeopleNum( const FIELD_WFBC* cp_wk )
{
  GF_ASSERT( cp_wk );

  // 人数を数える
  return WFBC_GetPeopleNum( cp_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物情報の取得
 *
 *	@param	cp_wk   ワーク
 *	@param	index   インデックス
 *
 *	@return 人物情報
 */
//-----------------------------------------------------------------------------
const FIELD_WFBC_PEOPLE* FIELD_WFBC_GetPeople( const FIELD_WFBC* cp_wk, int index )
{
  GF_ASSERT( index < FIELD_WFBC_PEOPLE_MAX );
  GF_ASSERT( cp_wk );
  
  return &cp_wk->people[index];
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物の追加
 *
 *	@param	p_wk    ワーク
 *	@param	objcode オブジェコード
 *
 *	@retval 追加したインデックス
 */
//-----------------------------------------------------------------------------
int FIELD_WFBC_AddPeople( FIELD_WFBC* p_wk, u32 objcode )
{
  FIELD_WFBC_PEOPLE* p_people;
  int index;

  GF_ASSERT( p_wk );

  // 空いているワークを取得
  index = WFBC_GetClearPeople( p_wk );

  p_people          = &p_wk->people[index];
  p_people->status  = FIELD_WFBC_PEOPLE_STATUS_NORMAL;
  p_people->objcode = objcode;

  return index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物の破棄
 *
 *	@param	p_wk    ワーク
 *	@param	index   破棄する人物のインデックス
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_DeletePeople( FIELD_WFBC* p_wk, int index )
{
  GF_ASSERT( p_wk );
  GF_ASSERT( index < FIELD_WFBC_PEOPLE_MAX );
  
  // インデックスのワークをクリーン
  WFBC_PEOPLE_Clear( &p_wk->people[index] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物　見た目の取得
 *
 *	@param	cp_people   ワーク
 *
 *	@return 見た目
 */
//-----------------------------------------------------------------------------
u32 FIELD_WFBC_PEOPLE_GetOBJCode( const FIELD_WFBC_PEOPLE* cp_people )
{
  GF_ASSERT( cp_people );
  return cp_people->objcode;
}





//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  空いている人物ワークを取得する
 *
 *	@param	cp_wk     ワーク
 *
 *	@return 空いているワークのインデックス
 */
//-----------------------------------------------------------------------------
static int WFBC_GetClearPeople( const FIELD_WFBC* cp_wk )
{
  int i;

  GF_ASSERT( cp_wk );

  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    if( cp_wk->people[i].status != FIELD_WFBC_PEOPLE_STATUS_NONE )
    {
      return i;
    }
  }

  GF_ASSERT_MSG( 0, "WFBC People Over\n" );
  return 0; // メモリ破壊などがないように０を返す
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物の数を取得する
 *
 *	@param	cp_wk   ワーク
 *
 *	@return 人物の数
 */
//-----------------------------------------------------------------------------
static int WFBC_GetPeopleNum( const FIELD_WFBC* cp_wk )
{
  int num;
  int i;
  
  GF_ASSERT( cp_wk );

  num = 0;
  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    if( cp_wk->people[i].status != FIELD_WFBC_PEOPLE_STATUS_NONE )
    {
      num ++;
    }
  }

  return num;
}


//----------------------------------------------------------------------------
/**
 *	@brief  人物ワークのクリア
 *
 *	@param	p_people  人物
 */
//-----------------------------------------------------------------------------
static void WFBC_PEOPLE_Clear( FIELD_WFBC_PEOPLE* p_people )
{
  p_people->status  = FIELD_WFBC_PEOPLE_STATUS_NONE;
  p_people->objcode = 0;
}




