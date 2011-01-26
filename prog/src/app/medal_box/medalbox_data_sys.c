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
#include "arc_def.h"              // ARC_ID

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define LOW_PAGE_LIMIT (10)  // 最低ページ枚数

//=============================================================================
/**
 *					関数プロトタイプ
*/
//=============================================================================
static MY_MEDAL_ST* GetMyMedalST( MY_MEDALDATA *p_my_medal , u8 page_num , u8 num );

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
    GFL_STD_MemClear( p_my_medal->p_medal_st , sizeof(MY_MEDAL_ST) * p_my_medal->medal_cnt );
    GF_ASSERT( p_my_medal->p_medal_st );
  }

  // ソートリストの初期化
  {
    MedalID medal_id;
    for( medal_id = 0 ; medal_id < MDLID_MAX ; medal_id++ )
    {
      p_my_medal->sort_medal_id[medal_id] = MDLID_MAX;
    }
  }

  // 取得済みデータをコピーする(同時にタイプ順ソートメダルIDも作成)
  {
    MedalID medal_id;
    MEDALDATA arc_data;
    u16 count = 0;
		ARCHANDLE	*p_handle	= GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , heapID );

    for( medal_id = 0 ; medal_id < MDLID_MAX ; medal_id++ )
    {
      u8 state = MEDALBOX_GetMedalState( p_medal_box , medal_id );
      if( (state == MEDAL_STATE_HINT_GET ) || (state == MEDAL_STATE_TITLE_GET ) )
      {
        MY_MEDAL_ST* p_st = &p_my_medal->p_medal_st[count];
        p_st->medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );
        p_st->medal_state = state;
        p_st->medal_id    = medal_id;
        p_st->new_state   = MEDALBOX_GetNewMedalFlg( p_medal_box , medal_id );
        MEDALBOX_GetDate( p_medal_box , medal_id , &p_st->year , &p_st->month , &p_st->day );
        GFL_ARC_LoadDataByHandle( p_handle , medal_id , &arc_data);
        p_st->graphic_type = arc_data.graphic_type;
        p_my_medal->sort_medal_id[count] = count;
        count++;
      }
    }
    GFL_ARC_CloseDataHandle( p_handle );
  }


  // ページの最大数取得
  p_my_medal->page_max = p_my_medal->medal_cnt/PAGE_MEDAL_MAX;

  // 端数がある場合はページを増やす
  if( p_my_medal->medal_cnt%PAGE_MEDAL_MAX != 0 )
  {
    p_my_medal->page_max++;
  }

  // 最低ページ数を切っていた場合はページを増やす
  if( p_my_medal->page_max < LOW_PAGE_LIMIT )
  {
    p_my_medal->page_max = LOW_PAGE_LIMIT;
  }

  ONOUE_Printf( "メダル所持数 ：%u\n",p_my_medal->medal_cnt );
  ONOUE_Printf( "ページ枚数   ：%u\n",p_my_medal->page_max );
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

//----------------------------------------------------------------------------
/**
 *	@brief  ページデータを取得する
 *
 *	@param	*p_my_medal 
 *	@param	p_page 
 *	@param	page_num    ページ番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALDATASYS_GetPageMedal( MY_MEDALDATA *p_my_medal , PAGEDATA* p_page , u16 page_num )
{
  int i;
  
  // ページ番号を保存
  p_page->page_num = page_num;

  // ページ内メダル数を初期化
  p_page->medal_max = 0;

  // メダルの取得
  for( i = 0; i< PAGE_MEDAL_MAX ; i++ )
  {
    p_page->p_medal_st[i] =GetMyMedalST( p_my_medal , page_num , i );

    if( p_page->p_medal_st[i] != NULL )
    {
      p_page->medal_max++;
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  ソート順でメダルSTを取得する
 *
 *	@param	*p_my_medal メダルデータ
 *	@param	page_num    ページ番号
 *	@param	num         ページ内での番号(0~4)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static MY_MEDAL_ST* GetMyMedalST( MY_MEDALDATA *p_my_medal , u8 page_num , u8 num )
{
  GF_ASSERT( num < 5 );

  if( ((page_num*5) + num) >= MDLID_MAX )
  {
    return NULL;
  }

  // 取得していない場合はNULLを返す
  if( p_my_medal->sort_medal_id[ (page_num*5) + num ] == MDLID_MAX )
  {
    return NULL;
  }
  return &(p_my_medal->p_medal_st[p_my_medal->sort_medal_id[ num + (page_num*5) ]]);
}

