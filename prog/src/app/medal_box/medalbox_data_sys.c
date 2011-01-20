//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medalbox_data_sys.c
 *	@brief  メダルボックス用データ操作
 *	@author	Masayuki Onoue
 *	@data		2011.01.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "medalbox_data_sys.h"

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

//----------------------------------------------------------------------------
/**
 *	@brief  メダルデータをセーブデータ、アーカイブから取得
 *
 *	@param	p_my_medal 
 *	@param	p_gamedata 
 *	@param	heapID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALDATASYS_InitMedalData( MY_MEDALDATA* p_my_medal , GAMEDATA* p_gamedata , HEAPID heapID)
{
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  MEDALBOX* p_medal_box = SaveData_GetMedalBox( p_sv );
  
  // メダルの数だけメモリ確保
  {
    p_my_medal->medal_cnt = MEDALBOX_GetMedalCount( p_medal_box ,TRUE);

#ifdef PM_DEBUG
    // メダル取得数が0で呼ばれることは無い
    if( p_my_medal->medal_cnt <= 0 )
    {
      p_my_medal->medal_cnt = 1;
    }
#endif

    GF_ASSERT_MSG( p_my_medal->medal_cnt > 0 ,"メダル取得数が0以下です。");

    p_my_medal->p_medal_st	= GFL_HEAP_AllocMemory( heapID, sizeof(MY_MEDAL_ST) * p_my_medal->medal_cnt );
    GF_ASSERT( p_my_medal->p_medal_st );
  }

  // 取得済みデータをコピーする
  {
    MedalID medal_id;
    u16 count = 0;
    for( medal_id = 0 ; medal_id < MDLID_MAX ; medal_id++ )
    {
      MY_MEDAL_ST* p_st = &p_my_medal->p_medal_st[count];
      p_st->medal_id    = medal_id;
      p_st->medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );
      p_st->new_state   = MEDALBOX_GetNewMedalFlg( p_medal_box , medal_id );
      MEDALBOX_GetDate( p_medal_box , medal_id , &p_st->year , &p_st->month , &p_st->day );
    }
  }

  ONOUE_Printf( "メダル所持数：%u\n",p_my_medal->medal_cnt );
}

//----------------------------------------------------------------------------
/**
 *	@brief  確保したメダルデータを開放する
 *
 *	@param	p_my_medal 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALDATASYS_DeleteMedalData( MY_MEDALDATA* p_my_medal )
{
  // 確保したメダルデータを開放する
  GFL_HEAP_FreeMemory( p_my_medal->p_medal_st );
}

