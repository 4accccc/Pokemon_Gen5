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
#include "mdlsort_def.cdat"
#include "arc_def.h"              // ARC_ID

//=============================================================================
/**
 *					関数プロトタイプ
*/
//=============================================================================
static MY_MEDAL_ST* GetMyMedalST( MY_MEDALDATA *p_my_medal , u8 page_num , u8 num );
static void ResetSortList( MY_MEDALDATA *p_my_medal );

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

  // ソートリストのリセット
  ResetSortList( p_my_medal );

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

  // イチオシメダル番号の取得
  p_my_medal->push_medalid = MEDALBOX_GetPushMedalID( p_medal_box );
  
  ONOUE_Printf( "イチオシメダル番号 ：%u\n",p_my_medal->push_medalid );
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
 *	@brief  メダルデータをセーブする
 *
 *	@param	p_my_medal 
 *	@param	p_gamedata 
 *	@param	heapID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALDATASYS_SaveMedalData( MY_MEDALDATA* p_my_medal , GAMEDATA* p_gamedata , HEAPID heapID)
{
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  MEDALBOX* p_medal_box = SaveData_GetMedalBox( p_sv );

  // イチオシメダルのセーブ
  MEDALBOX_SetPushMedalID( p_medal_box , p_my_medal->push_medalid );

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

//----------------------------------------------------------------------------
/**
 *	@brief  一押しメダル番号を設定する
 *
 *	@param	p_my_medal 
 *	@param	meda_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALDATASYS_SetPushMedalID( MY_MEDALDATA* p_my_medal , MedalID meda_id )
{
  p_my_medal->push_medalid = meda_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルをソートする
 *
 *	@param	p_my_medal 
 *	@param	sort_mode 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALDATASYS_SortMedal( MY_MEDALDATA* p_my_medal , SORT_MODE sort_mode )
{
  // フィルタータイプをセットする
  p_my_medal->sort_mode = sort_mode;

  // ソートリストのリセット
  ResetSortList( p_my_medal );

  // フィルターをリセットする
  {
    int i;
    int n;
    int count = 0;

        
    // タイプ毎のソート
    switch( p_my_medal->sort_mode )
    {
    // 古い順
    case SORT_MODE_DEFAULT:
      {
        // ソートメダルIDをリセット
        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {
          p_my_medal->sort_medal_id[i] = i;
        }

        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {

          for( n = i+1 ; n < p_my_medal->medal_cnt ; n++ )
          {
            MY_MEDAL_ST *p_st_i = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[i]];
            MY_MEDAL_ST *p_st_n = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[n]];
            u32 ymd_i = 0xfffffff;
            u32 ymd_n = 0xfffffff;
            ymd_i = (p_st_i->year * 10000) + (p_st_i->month + 100 ) + (p_st_n->day);
            ymd_n = (p_st_n->year * 10000) + (p_st_n->month + 100 ) + (p_st_n->day);

            GF_ASSERT(p_st_i);
            GF_ASSERT(p_st_n);
            GF_ASSERT( n != p_my_medal->medal_cnt);

            // 古い物
            if( ymd_i > ymd_n )
            {
              MedalID tmp = p_my_medal->sort_medal_id[i];
              p_my_medal->sort_medal_id[i] = p_my_medal->sort_medal_id[n];
              p_my_medal->sort_medal_id[n] = tmp;
            }
          }
        }
      }
      break;
    // タイトルだけ
    case SORT_MODE_TITLE_ONLY:
      {
        // タイトルメダルだけでリセット
        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {
          MY_MEDAL_ST *p_st = &p_my_medal->p_medal_st[i];
          if( p_st->medal_state == MEDAL_STATE_TITLE_GET )
          {
            p_my_medal->sort_medal_id[count] = i;
            count++;
          }
        }
        for( i = 0 ; i < count - 1 ; i++ )
        {
          for( n = i+1 ; n < count ; n++ )
          {
            MY_MEDAL_ST *p_st_i = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[i]];
            MY_MEDAL_ST *p_st_n = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[n]];
            u32 ymd_i = 0xfffffff;
            u32 ymd_n = 0xfffffff;
            ymd_i = (p_st_i->year * 10000) + (p_st_i->month + 100 ) + (p_st_n->day);
            ymd_n = (p_st_n->year * 10000) + (p_st_n->month + 100 ) + (p_st_n->day);

            GF_ASSERT(p_st_i);
            GF_ASSERT(p_st_n);
            GF_ASSERT( n != p_my_medal->medal_cnt);

            GF_ASSERT( count > n );

            // 古い物
            if( ymd_i > ymd_n )
            {
              MedalID tmp = p_my_medal->sort_medal_id[i];
              p_my_medal->sort_medal_id[i] = p_my_medal->sort_medal_id[n];
              p_my_medal->sort_medal_id[n] = tmp;
            }
          }
        }
      }
      break;
    // 新しい順
    case SORT_MODE_NEW:
      {
        // ソートメダルIDをリセット
        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {
          p_my_medal->sort_medal_id[i] = i;
        }

        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {

          for( n = i+1 ; n < p_my_medal->medal_cnt ; n++ )
          {
            MY_MEDAL_ST *p_st_i = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[i]];
            MY_MEDAL_ST *p_st_n = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[n]];
            u32 ymd_i = 0xfffffff;
            u32 ymd_n = 0xfffffff;
            ymd_i = (p_st_i->year * 10000) + (p_st_i->month + 100 ) + (p_st_n->day);
            ymd_n = (p_st_n->year * 10000) + (p_st_n->month + 100 ) + (p_st_n->day);

            GF_ASSERT(p_st_i);
            GF_ASSERT(p_st_n);
            GF_ASSERT( n != p_my_medal->medal_cnt);

            // 新しい物
            if( ymd_i < ymd_n )
            {
              MedalID tmp = p_my_medal->sort_medal_id[i];
              p_my_medal->sort_medal_id[i] = p_my_medal->sort_medal_id[n];
              p_my_medal->sort_medal_id[n] = tmp;
            }
          }
        }
      }
      break;
    // ヒントで（タイプ順)
    case SORT_MODE_HINT_ONLY:
      {
        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {
          MY_MEDAL_ST *p_st = &p_my_medal->p_medal_st[i];

          if( p_st->medal_state == MEDAL_STATE_HINT_GET )
          {
            p_my_medal->sort_medal_id[count] = i;
            count++;
          }
        }
      }
      break;
    // タイプ順
    case SORT_MODE_TYPE:
      {
       // ソートメダルIDをリセット
        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {
          p_my_medal->sort_medal_id[i] = i;
        }

        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {

          for( n = i+1 ; n < p_my_medal->medal_cnt ; n++ )
          {
            MY_MEDAL_ST *p_st_i = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[i]];
            MY_MEDAL_ST *p_st_n = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[n]];
            u32 id_i = 0;
            u32 id_n = 0;
            id_i = p_st_i->medal_id;
            id_n = p_st_n->medal_id;

            GF_ASSERT(p_st_i);
            GF_ASSERT(p_st_n);
            GF_ASSERT( n != p_my_medal->medal_cnt);

            if( p_st_i->medal_state == MEDAL_STATE_HINT_GET )
            {
              id_i +=0xfffffff;
            }
            if( p_st_n->medal_state == MEDAL_STATE_HINT_GET )
            {
              id_n +=0xfffffff;
            }
            if( id_i > id_n )
            {
              MedalID tmp = p_my_medal->sort_medal_id[i];
              p_my_medal->sort_medal_id[i] = p_my_medal->sort_medal_id[n];
              p_my_medal->sort_medal_id[n] = tmp;
            }
          }
        }
      }
      break;
     // 名前順
    case SORT_MODE_NAME:
      {
        // ソートメダルIDをリセット
        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {
          p_my_medal->sort_medal_id[i] = i;
        }

        // 名前順ソートデータを取得
        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {
          MY_MEDAL_ST *p_st = &p_my_medal->p_medal_st[i];
          int name_index;

          // 名前順データを取得する
          for( name_index = 0 ; name_index < MDLID_MAX ; name_index++ )
          {
            if( MedalSortList[name_index] == p_st->medal_id )
            {
              p_st->name_sort_id = name_index;
              break;
            }
          }
        }

        for( i = 0 ; i < p_my_medal->medal_cnt ; i++ )
        {

          for( n = i+1 ; n < p_my_medal->medal_cnt ; n++ )
          {
            BOOL name_i_get = FALSE;
            BOOL name_n_get = FALSE;
            MY_MEDAL_ST *p_st_i = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[i]];
            MY_MEDAL_ST *p_st_n = &p_my_medal->p_medal_st[p_my_medal->sort_medal_id[n]];
            u32 name_i;
            u32 name_n;

            GF_ASSERT(p_st_i);
            GF_ASSERT(p_st_n);
            GF_ASSERT( n != p_my_medal->medal_cnt);

            name_i = p_st_i->name_sort_id;
            name_n = p_st_n->name_sort_id;

            // ヒントメダルは後ろに
            if( p_st_i->medal_state == MEDAL_STATE_HINT_GET )
            {
              name_i +=0xfffffff;
            }
            if( p_st_n->medal_state == MEDAL_STATE_HINT_GET )
            {
              name_n +=0xfffffff;
            }

            // 新しい物
            if( name_i > name_n )
            {
              MedalID tmp = p_my_medal->sort_medal_id[i];
              p_my_medal->sort_medal_id[i] = p_my_medal->sort_medal_id[n];
              p_my_medal->sort_medal_id[n] = tmp;
            }
          }
        }
      }
      break;

    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ソートリストをresetする
 *
 *	@param	*p_my_medal 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ResetSortList( MY_MEDALDATA *p_my_medal )
{
  // ソートリストの初期化
  {
    MedalID medal_id;
    for( medal_id = 0 ; medal_id < MDLID_MAX ; medal_id++ )
    {
      p_my_medal->sort_medal_id[medal_id] = MDLID_MAX;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定したメダルIDのメダルがソート後何番目に位置しているかを調べる
 *
 *	@param	*p_my_medal 
 *	@param	medal_id 
 *
 *	@retval メダルのソート後の位置
 */
//-----------------------------------------------------------------------------
u8 MEDALDATASYS_GetMedalSortPos( MY_MEDALDATA *p_my_medal , MedalID medal_id )
{
  int i,n;
  int count = 0;
  for( i = 0 ; i < p_my_medal->page_max ; i++ )
  {
    for( n = 0 ; n < PAGE_MEDAL_MAX ; n++)
    {
      MY_MEDAL_ST *p_st = GetMyMedalST( p_my_medal , i , n );
      if( p_st != NULL )
      {
        if( p_st->medal_id == medal_id )
        {
          return count;
        }
        count++;
      }
    }
  }
  return MDLID_MAX;
}

