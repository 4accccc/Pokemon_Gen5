//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon_creator.c
 *	@brief  無限ダンジョン：ダンジョン生成用ソース
 *	@author	Masayuki Onoue
 *	@date		2011.10.11
 *	@note   vsプロジェクトでもソースを流用し、テストを行うことを考慮し設計します
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "infinity_dungeon_creator.h"

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

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void RemoveTrRoom( INF_DUNGEON_DATA *p_dungeon_data ,const u8 floor_no ,int rmv_num ,GFL_STD_RandContext *rand_context );
static int GetBossRoomSetArray( ROOM_DATA *p_room_data ,int *p_index_array ,int fs_x ,int fs_y);
static int GetElevatorSetArray( ROOM_DATA *p_room_data ,int *p_index_array ,int fs_x ,int fs_y);
static ROOM_DATA* GetDigPosRoomData( const DIG_POS *p_dig_pos,ROOM_DATA* p_room_data);
static const ROOM_DATA* GetDigPosRoomDataConst( const DIG_POS *p_dig_pos,const ROOM_DATA* p_room_data);
static void MoveDigPos(u8 dir ,INF_DIG_DATA *p_dig_data ,ROOM_DATA *p_room_data);
static int GetCanMove( const DIG_POS *p_dig_pos ,const ROOM_DATA *p_room_data ,int *p_array,int fs_x ,int fs_y ,int mode);
static BOOL CheckCloseAllDoor( const DIG_POS *p_dig_pos ,const ROOM_DATA *p_room_data);
static void pop_dig_pos( INF_DIG_DATA *p_dig_data );
static void push_dig_pos( INF_DIG_DATA *p_dig_data ,const DIG_POS *p_dig_pos);
static BOOL CheckDigEnd( const ROOM_DATA *p_room_data ,int fs_x ,int fs_y);
static u8 GetRoomTypeArray( int area_no ,u8 floor_no ,u8 version ,u8 *p_pos_type_id_array );
static u8 GetDefaultRoomTypeArray( int area_no ,u8 floor_no ,u8 version ,u8 *p_pos_type_id_array );

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン生成処理
 *
 *	@param	*p_dungeon_data 
 *	@param	area_no
 *	@param	seed 
 *	@param	fs_x 
 *	@param	fs_y
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_DCREATOR_SetDungeonData( INF_DUNGEON_DATA *p_dungeon_data ,int area_no, u32 seed ,int fs_x ,int fs_y,u8 top_floor)
{
  int i,j,k;
  DIG_POS elv_pos[INFINITYDUNGEON_FLOOR_MAX];
  DIG_POS boss_pos;
  s32 boss_floor;
  GFL_STD_RandContext rand_context;

  // 乱数を作成
  GFL_STD_RandInit( &rand_context, seed );

  // 部屋タイプを初期化
  {
    for( i = 0; i<INFINITYDUNGEON_FLOOR_MAX; i++ )
    {
      // 部屋初期化
      for( j = 0; j<ROOM_MAX; j++ )
      {
        // 初期化
        p_dungeon_data->room_data[i][j].room_id = j;
        p_dungeon_data->room_data[i][j].room_type = ROOM_TYPE_DEFAULT;
        p_dungeon_data->room_data[i][j].tr_pos_type = INF_ROOM_POS_TYPE_NONE;

        // 扉を閉じる
        for( k = 0 ; k < 4 ; k++ )
        {
          p_dungeon_data->room_data[i][j].can_move[k]  = FALSE;
        }
      }

    }
  }

  // エレベータ設置配列を設定（必ず右上）
  {
    for( i = 0; i<INFINITYDUNGEON_FLOOR_MAX; i++ )
    {
      ROOM_DATA *p_room_data = p_dungeon_data->room_data[i];
      p_room_data[0].room_type    = ROOM_TYPE_ELEVATOR;
      p_room_data[0].tr_pos_type  = INF_ROOM_POS_TYPE_NONE;
      // エレベーター位置を格納（穴掘りスタート位置に使用）
      elv_pos[i].x = 0;
      elv_pos[i].y = 0;
    }
  }

  // ボス部屋の設置
  {
    ROOM_DATA *p_rd;
    ROOM_DATA *p_room_data;
    int floor_no;
    int max;
    int room_index;
    int index_array[ROOM_MAX];
    u8 room_id;

    // 階数を決定
    floor_no = GFL_STD_Rand( &rand_context ,top_floor);
    p_room_data = p_dungeon_data->room_data[floor_no];

    // 場所を決定
    max = GetBossRoomSetArray(p_room_data,index_array,fs_x,fs_y);
    GF_ASSERT_MSG((max > 0),"ボス部屋を置く場所がありません\n");
    room_index = GFL_STD_Rand( &rand_context ,max);

    // 部屋データを設定
    room_id = index_array[room_index];
    p_rd= &p_room_data[room_id];
    p_rd->room_type = ROOM_TYPE_BOSS;
    p_rd->tr_pos_type = INF_ROOM_POS_TYPE_NONE;

    // 位置を保存
    boss_pos.x = room_id % ROOM_MAX_X;
    boss_pos.y = room_id / ROOM_MAX_X;
    boss_floor = floor_no;
  }


  //ランダム
  {
    int floor_no;
    for( floor_no = 0; floor_no < top_floor; floor_no++ )
    {
      ROOM_DATA *p_room_data = p_dungeon_data->room_data[floor_no];
      INF_DIG_DATA dig_data;
      u8   dir_index;
      u8   dir;
      int dir_array[INF_MOVE_MAX];
      dig_data.now_pos = elv_pos[floor_no];
      dig_data.data_index = -1;

      for( i = 0; i< INF_MAX_SEARC_CNT; i++ )
      {
        int move_cnt;
        move_cnt = GetCanMove( &dig_data.now_pos ,p_room_data,dir_array
            ,fs_x,fs_y,INF_CHECKMODE_FULL_CLOSE);
        if( move_cnt > 0 )
        {
          dir_index = GFL_STD_Rand( &rand_context,move_cnt);
          dir = dir_array[dir_index];
          MoveDigPos(dir ,&dig_data ,p_room_data );
        }
        else
        {
          // 一歩戻る
          pop_dig_pos( &dig_data );

          // 終了
          if( CheckDigEnd( p_room_data ,fs_x ,fs_y ) == TRUE )
          {
            break;
          }
        }
      }

#ifdef PM_DEBUG
      if( i == INF_MAX_SEARC_CNT )
      {
        GF_ASSERT_MSG(0,"迷路が最後まで生成できませんでした");
      }
#endif

      // ボス部屋への道を空ける
      if( boss_floor == floor_no )
      {
        dig_data.now_pos= boss_pos;
        MoveDigPos(INF_MOVE_DOWN ,&dig_data ,p_room_data );
      }

      // ランダムオープン
      {
        DIG_POS pos;
        int dir_array[INF_MOVE_MAX];
        int move_cnt;
        u8   dir_index;
        INF_DIG_DATA dig_data;
        ROOM_DATA *p_rd;
        dig_data.data_index = -1;

        for( pos.y = 0; pos.y < fs_y ; pos.y++ )
        {
          for( pos.x = 0; pos.x < fs_x ; pos.x++ )
          {
            dig_data.now_pos = pos;
            p_rd= GetDigPosRoomData( &pos ,p_room_data);

            // エレベータ部屋とボス部屋からは新たな道を作らない
            if( (p_rd->room_type != ROOM_TYPE_ELEVATOR) && (p_rd->room_type != ROOM_TYPE_BOSS) )
            {
              move_cnt = GetCanMove( &pos ,p_room_data,dir_array ,fs_x,fs_y,INF_CHECKMODE_MOVE);
              dir_index = GFL_STD_Rand( &rand_context,move_cnt);
              dir = dir_array[dir_index];
              MoveDigPos(dir ,&dig_data ,p_room_data );
            }
          }
        }
      }
    }
  }
// 部屋タイプ取得部分はVSプロジェクト移植対象外にするか
//#ifndef VS_PROJECT
  {
    for( i = 0; i<top_floor; i++ )
    {
      // 配置する部屋を取得
      int data_index = 0;
      u8 pos_type_id_array[ROOM_MAX];
      u8 room_array_max;
      room_array_max = GetRoomTypeArray( area_no ,i
                                    ,p_dungeon_data->version ,pos_type_id_array );

      // 配置可能な部屋のみをランダムに並び替える
      INF_TOOL_RndArrayU8( pos_type_id_array ,room_array_max ,&rand_context);

      // 実際に部屋を配置
      for( j = 0; j<ROOM_MAX; j++ )
      {
        if( p_dungeon_data->room_data[i][j].room_type == ROOM_TYPE_DEFAULT )
        {
          int x = j % ROOM_MAX_X;
          int y = j / ROOM_MAX_X;
          if( (x < fs_x) && (y < fs_y ) )
          {
            if( (boss_pos.x == x) && ((boss_pos.y+1) == y) && ( i == boss_floor ) )
            {
              // ボス部屋の下の部屋はデフォルト部屋から選ぶ
              u8 idx;
              u8 default_id_array[ROOM_MAX];
              const INF_ROOM_SET_TR_PARAM *p_room_param;
              u8 max = GetDefaultRoomTypeArray( area_no ,i ,p_dungeon_data->version ,default_id_array );

              GF_ASSERT_MSG( max != 0,"通常部屋が設置されていません\n");
              idx = default_id_array[GFL_STD_Rand( &rand_context,max)];
              p_room_param = INF_TOOL_GetRoomTrParam( idx );
              p_dungeon_data->room_data[i][j].tr_pos_type = idx;
              p_dungeon_data->room_data[i][j].room_type = p_room_param->room_type;
            }
            else
            {
              // 通常部屋
              const INF_ROOM_SET_TR_PARAM *p_room_param;
              u8 idx = pos_type_id_array[data_index];
              
              // NONEの場合は配置を行わずテーブルのみを進める
              if( INF_ROOM_POS_TYPE_NONE != idx )
              {
                p_room_param = INF_TOOL_GetRoomTrParam( idx );
                p_dungeon_data->room_data[i][j].tr_pos_type = idx;
                p_dungeon_data->room_data[i][j].room_type = p_room_param->room_type;
                GF_ASSERT_MSG(data_index<room_array_max,"部屋配列が取得できませんでした。エリアデータを確認してください。\n");
              }
              data_index++;
            }
          }
        }
      }
    }
  }
//#endif

}

//----------------------------------------------------------------------------
/**
 *	@brief  指定階に配置する部屋テーブルを取得する
 *
 *	@param	area_no 
 *	@param	floor_no 
 *	@param	version 
 *	@param	*p_pos_type_id_array 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetRoomTypeArray( int area_no ,u8 floor_no ,u8 version ,u8 *p_pos_type_id_array )
{
  int i = 0;
  int data_index = 0;
  const u8 *p_room_table = INF_TOOL_GetFloorData( area_no ,floor_no ,version );
  // クリア
  GFL_STD_MemFill(p_pos_type_id_array ,INF_ROOM_POS_TYPE_NONE ,sizeof(u8)*ROOM_MAX );

  // 使用可能な部屋を取得（NONEも含める）
  for( i = 0; i < INF_ROOM_POS_TYPE_MAX; i++)
  {
    u8 room_num = p_room_table[i];
    while( room_num > 0 )
    {
      p_pos_type_id_array[data_index] = i;
      data_index++;
      room_num--;
    }
  }
  return data_index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  デフォルト部屋の配列を取得する
 *
 *	@param	area_no 
 *	@param	floor_no 
 *	@param	version 
 *	@param	*p_pos_type_id_array 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetDefaultRoomTypeArray( int area_no ,u8 floor_no ,u8 version ,u8 *p_pos_type_id_array )
{
  int i = 0;
  int data_index = 0;
  const u8 *p_room_table = INF_TOOL_GetFloorData( area_no ,floor_no ,version );

  // クリア
  GFL_STD_MemFill(p_pos_type_id_array ,INF_ROOM_POS_TYPE_NONE ,sizeof(u8)*ROOM_MAX );

  // 使用可能な部屋を取得(NONEは含めない)
  for( i = 0; i < INF_ROOM_POS_TYPE_NONE; i++)
  {
    u8 room_num = p_room_table[i];
    if( room_num > 0 )
    {
      const INF_ROOM_SET_TR_PARAM *p_room_param = INF_TOOL_GetRoomTrParam( i );
      if( p_room_param->room_type == ROOM_TYPE_DEFAULT )
      {
        p_pos_type_id_array[data_index] = i;
        data_index++;
      }
    }
  }
  return data_index;

}

//----------------------------------------------------------------------------
/**
 *	@brief  指定数トレーナーが減るように部屋タイプを入れ替える
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RemoveTrRoom( INF_DUNGEON_DATA *p_dungeon_data ,const u8 floor_no ,int rmv_num ,GFL_STD_RandContext *rand_context )
{
  u8  pos_type;
  int cnt = 0;
  int i;
  u32 index_array[ROOM_MAX];

  GFL_STD_MemFill(index_array, ROOM_MAX, sizeof(int)*ROOM_MAX);
  for( i = 0; i< ROOM_MAX; i++ )
  {
    u8 pos_type = p_dungeon_data->room_data[floor_no][i].tr_pos_type;
    if( pos_type != INF_ROOM_POS_TYPE_NONE )
    {
      index_array[cnt] = i;
      cnt++;
    }
  }

  // ランダム抽選
  INF_TOOL_RndArray( index_array ,cnt ,rand_context);

  // 部屋を削ってゆく
  for( i = 0; i< cnt; i++ )
  {
    pos_type = p_dungeon_data->room_data[floor_no][index_array[i]].tr_pos_type;
    rmv_num -= INF_TOOL_GetMemberNum(pos_type);
    p_dungeon_data->room_data[floor_no][index_array[i]].tr_pos_type = INF_ROOM_POS_TYPE_NONE;

    if( rmv_num <= 0 )
    {
      return;
    }
  }

    GF_ASSERT_MSG(0,"トレーナーが配置されている部屋がありません\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボス部屋の設置可能配列を取得する
 *
 *	@param	*p_room_data 
 *	@param	*p_index_array 
 *	@param	fs_x 
 *	@param	fs_y
 *
 *	@retval 配列に格納した数
 */
//-----------------------------------------------------------------------------
static int GetBossRoomSetArray( ROOM_DATA *p_room_data ,int *p_index_array ,int fs_x ,int fs_y)
{
  DIG_POS pos;
  int cnt = 0;
  int x,y;
  ROOM_DATA *p_rd;
  GFL_STD_MemFill(p_index_array, ROOM_MAX, sizeof(int)*ROOM_MAX);

  // ボス部屋は一番下の列に出ない
  for( y = 0; y<fs_y-1; y++ )
  {
    for( x = 0; x<fs_x; x++ )
    {
      // 現在地がエレベータ部屋
      {
        pos.x = x;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_ELEVATOR )
        {
          continue;
        }
      }

      // 設置可能位置を保存する
      {
        p_index_array[cnt] = x + y*ROOM_MAX_X;
        cnt++;
      }
    }
  }
  return cnt;

}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベータの設置可能配列を取得する
 *
 *	@param	*p_room_data 
 *	@param	*p_index_array 
 *	@param	fs_x 
 *	@param	fs_y
 *
 *	@retval 配列に格納した数
 */
//-----------------------------------------------------------------------------
static int GetElevatorSetArray( ROOM_DATA *p_room_data ,int *p_index_array ,int fs_x ,int fs_y)
{
  DIG_POS pos;
  int cnt = 0;
  int x,y;
  ROOM_DATA *p_rd;
  GFL_STD_MemFill(p_index_array, ROOM_MAX, sizeof(int)*ROOM_MAX);

  // ボス部屋を調べ、配置出る場所リストを作成する
  for( y = 0; y<fs_y; y++ )
  {
    for( x = 0; x<fs_x; x++ )
    {
      // 現在地がボス部屋
      {
        pos.x = x;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }

      // 上
      if( y - 1 >= 0 )
      {
        pos.x = x;
        pos.y = y-1;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }

      // 下
      if( y + 1  < fs_y )
      {
        pos.x = x;
        pos.y = y+1;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }

      // 左
      if( x - 1 >= 0 )
      {
        pos.x = x-1;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }
      else
      {
        // 左端で2列目のときは、右上にボス部屋があると生成不能になるのでチェック
        if( y == 1 ){
          pos.x = x+1;
          pos.y = y-1;
          p_rd= GetDigPosRoomData( &pos ,p_room_data);
          if( p_rd->room_type == ROOM_TYPE_BOSS )
          {
            continue;
          }
        }
      }

      // 右
      if( x + 1 < fs_x  )
      {
        pos.x = x+1;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }
      else
      {
        // 左端で2列目のときは、右上にボス部屋があると生成不能になるのでチェック
        if( y == 1 ){
          pos.x = x-1;
          pos.y = y-1;
          p_rd= GetDigPosRoomData( &pos ,p_room_data);
          if( p_rd->room_type == ROOM_TYPE_BOSS )
          {
            continue;
          }
        }
      }

      // 設置可能位置を保存する
      {
        p_index_array[cnt] = x + y*ROOM_MAX_X;
        cnt++;
      }
    }
  }
  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定位置の部屋データを取得する
 *
 *	@param	*p_dig_pos
 *	@param	p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static ROOM_DATA* GetDigPosRoomData( const DIG_POS *p_dig_pos,ROOM_DATA* p_room_data)
{
  return (&p_room_data[p_dig_pos->x+p_dig_pos->y*ROOM_MAX_X]);
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定位置の部屋データを取得する(const)
 *
 *	@param	DIG_POS *p_dig_pos
 *	@param	ROOM_DATA* p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const ROOM_DATA* GetDigPosRoomDataConst( const DIG_POS *p_dig_pos,const ROOM_DATA* p_room_data)
{
  return (&p_room_data[p_dig_pos->x+p_dig_pos->y*ROOM_MAX_X]);
}

//----------------------------------------------------------------------------
/**
 *	@brief  穴を掘る
 *
 *	@param	dir 
 *	@param	*p_dig_data 
 *	@param	*p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MoveDigPos(u8 dir ,INF_DIG_DATA *p_dig_data ,ROOM_DATA *p_room_data)
{
  ROOM_DATA *p_now_room_data;
  ROOM_DATA *p_next_room_data;

  // 指定の方向に進む
  switch( dir )
  {
  case INF_MOVE_UP:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_UP] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.y -= 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_DOWN] = TRUE;
    break;
  case INF_MOVE_DOWN:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_DOWN] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.y += 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_UP] = TRUE;
    break;
  case INF_MOVE_LEFT:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_LEFT] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.x -= 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_RIGHT] = TRUE;
    break;
  case INF_MOVE_RIGHT:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_RIGHT] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.x += 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_LEFT] = TRUE;
    break;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  移動可能な方向があるか調べる
 *
 *	@param	*p_dig_pos 
 *	@param	*p_room_data
 *	@param	*p_array      移動可能先格納変数
 *	@param	mode          
 *
 *	@retval 移動可能な方向数
 */
//-----------------------------------------------------------------------------
static int GetCanMove( const DIG_POS *p_dig_pos ,const ROOM_DATA *p_room_data ,int *p_array,int fs_x ,int fs_y ,int mode)
{
  int ret_cnt = 0;
  const ROOM_DATA *p_rd;

  // 移動方向のクリア
  GFL_STD_MemFill(p_array, INF_MOVE_MAX, sizeof(int)*INF_MOVE_MAX);

  // 4方向の部屋で全てドアが閉まっている部屋を調べる
  {
    DIG_POS pos = *p_dig_pos;
    pos.x+=1;
    if( pos.x < fs_x )
    {
      // 移動方向の部屋がボス部屋の場合は掘り進まない
      p_rd= GetDigPosRoomDataConst( &pos ,p_room_data);
      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if( mode == INF_CHECKMODE_FULL_CLOSE )
        {
          // 全てのドアが閉まっているかチェックする
          if( CheckCloseAllDoor( &pos ,p_room_data ) )
          {
            p_array[ret_cnt] = INF_MOVE_RIGHT;
            ret_cnt++;
          }
        }
        else
        {
          p_array[ret_cnt] = INF_MOVE_RIGHT;
          ret_cnt++;
        }
      }
    }
  }
  {
    DIG_POS pos = *p_dig_pos;
    pos.x-=1;
    if( pos.x >= 0 )
    {
      p_rd= GetDigPosRoomDataConst( &pos ,p_room_data);
      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if( mode == INF_CHECKMODE_FULL_CLOSE )
        {
          if( CheckCloseAllDoor( &pos ,p_room_data ) )
          {
            p_array[ret_cnt] = INF_MOVE_LEFT;
            ret_cnt++;
          }
        }
        else
        {
            p_array[ret_cnt] = INF_MOVE_LEFT;
            ret_cnt++;
        }
      }
    }
  }
  {
    DIG_POS pos = *p_dig_pos;
    pos.y+=1;
    if( pos.y < fs_y )
    {
      p_rd= GetDigPosRoomDataConst( &pos ,p_room_data);

      // 移動方向がボス部屋、エレベータ部屋の場合はしたには掘り進まない
      if( (p_rd->room_type != ROOM_TYPE_BOSS) && (p_rd->room_type != ROOM_TYPE_ELEVATOR) )
      {
        if( mode == INF_CHECKMODE_FULL_CLOSE )
        {
          if( CheckCloseAllDoor( &pos ,p_room_data ) )
          {
            p_array[ret_cnt] = INF_MOVE_DOWN;
            ret_cnt++;
          }
        }
        else
        {
          p_array[ret_cnt] = INF_MOVE_DOWN;
          ret_cnt++;
        }
      }
    }
  }
  {
    DIG_POS pos = *p_dig_pos;
    p_rd= GetDigPosRoomDataConst( &pos ,p_room_data);
    if( p_rd->room_type != ROOM_TYPE_ELEVATOR )   // エレベータ部屋は上に移動することを禁止
    {
      pos.y-=1;
      p_rd= GetDigPosRoomDataConst( &pos ,p_room_data);
      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if( pos.y >= 0 )
        {
          if( mode == INF_CHECKMODE_FULL_CLOSE )
          {

            if( CheckCloseAllDoor( &pos ,p_room_data ) )
            {
              p_array[ret_cnt] = INF_MOVE_UP;
              ret_cnt++;
            }
          }
          else
          {
            p_array[ret_cnt] = INF_MOVE_UP;
            ret_cnt++;
          }
        }
      }
    }
  }

  return ret_cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  部屋の扉が全て閉じているかを調べる
 *
 *	@param	*p_dig_pos 
 *	@param	*p_room_data
 *
 *	@retval TRUE=全て閉じている
 */
//-----------------------------------------------------------------------------
static BOOL CheckCloseAllDoor( const DIG_POS *p_dig_pos ,const ROOM_DATA *p_room_data)
{
  int i;
  const ROOM_DATA *p_rd = GetDigPosRoomDataConst( p_dig_pos ,p_room_data);
  for( i = 0; i < 4 ;i++ )
  {
    if( p_rd->can_move[i] == TRUE )
    {
      return FALSE;
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  位置データをプッシュ
 *
 *	@param	*p_dig_data 
 *	@param	*p_dig_pos
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void push_dig_pos( INF_DIG_DATA *p_dig_data ,const DIG_POS *p_dig_pos)
{
  p_dig_data->data_index++;
  p_dig_data->dig_pos_list[p_dig_data->data_index] = *p_dig_pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief  位置データをプッシュ
 *
 *	@param	*p_dig_data 
 *	@param	*p_dig_pos
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pop_dig_pos( INF_DIG_DATA *p_dig_data )
{
  if( p_dig_data->data_index >= 0 )
  {
    p_dig_data->now_pos = p_dig_data->dig_pos_list[p_dig_data->data_index];
    p_dig_data->dig_pos_list[p_dig_data->data_index].x = 0;
    p_dig_data->dig_pos_list[p_dig_data->data_index].y = 0;
    p_dig_data->data_index--;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  全て掘り終わったかチェック
 *
 *	@param	*p_room_data 
 *	@param	fs_x 
 *	@param	fs_y
 *
 *	@retval TRUE=掘り終わり
 */
//-----------------------------------------------------------------------------
static BOOL CheckDigEnd( const ROOM_DATA *p_room_data ,int fs_x ,int fs_y)
{
  int x,y;
  const ROOM_DATA *p_rd;
  DIG_POS pos;
  for( y=0; y < fs_y; y++ )
  {
    for( x=0; x < fs_x; x++ )
    {
      pos.x = x;
      pos.y = y;
      p_rd= GetDigPosRoomDataConst( &pos ,p_room_data);

      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if(CheckCloseAllDoor( &pos ,p_room_data ) == TRUE)
        {
          return FALSE;
        }
      }
    }
  }
  return TRUE;
}



//----------------------------------------------------------------------------
/**
 *  トレーナーデーター生成ルーチン
 */
//-----------------------------------------------------------------------------
static int GetSetTrainerIDTable( const INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,u32 *p_trid_array ,GFL_STD_RandContext *rand_context);
static s32 GetRecoverTrainerSetRoomIdArray( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no ,int *room_id_array ,u32 *member_num );
static void SetRecoverTrainer( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no ,u32 area_no,const int *room_id_array ,int cnt,const u32 *member_num , GFL_STD_RandContext *rand_context ,u32 *recover_index );
static s32 GetGateTrainerSetTrIndexArray( INF_DUNGEON_DATA  *p_dungeon_data ,s32 floor_no ,s32* id_pos_array, s32* room_id_array );
static u32 GetRecoverTrainerID( const INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,GFL_STD_RandContext *rand_context );

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーデータ
 *
 *	@param	*p_dungeon_data フロアデータ
 *	@param	area_no         エリア番号
 *	@param	seed            生成用シード
 *	@param	top_floor       フロア数
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_DCREATOR_SetTrainerData( INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no , u32 seed,u8 top_floor )
{
  int i,n;
  int tr_cnt = 0;
  u32 trid_array[INF_DUNGEON_TR_DATA_MAX];
  GFL_STD_RandContext rand_context;
  
  // 乱数を作成
  GFL_STD_RandInit( &rand_context, seed );
 
  // メモリをクリーン
  GFL_STD_MemFill( trid_array ,INFTRID_DUMMY , sizeof(u32)*INF_DUNGEON_TR_DATA_MAX);
 
  // トレーナーIDテーブル取得
  tr_cnt = GetSetTrainerIDTable( p_dungeon_data , area_no ,trid_array, &rand_context  );

  // トレーナーIDのランダムデータを生成
  for( n = 0; n<top_floor; n++)
  {
    // データ初期化
    GFL_STD_MemFill(p_dungeon_data->tr_data[n].tr_id, INFTRID_DUMMY, sizeof(u16)*INFINITY_DUNGEON_TRID_MAX );
    
    // トレーナーデータ配列をランダム化
    INF_TOOL_RndArray( trid_array ,tr_cnt ,&rand_context);

    // 必要な数だけトレーナーIDを保存
    for( i = 0; i<INFINITY_DUNGEON_TRID_MAX; i++)
    {
      p_dungeon_data->tr_data[n].tr_id[i] = trid_array[i];
    }

    // 回復トレーナー部屋IDリストを初期化
    p_dungeon_data->recover_room_id[n] = ROOM_MAX;
  }

  // 回復トレーナー配置位置を決める
  {
    s32 cnt = 0;
    int room_id_array[ROOM_MAX];
    u32 member_num[ROOM_MAX];
    u32 recover_index = 0;

    // 全フロアに配置する
    for( n = 0; n<top_floor; n++)
    {
      GFL_STD_MemFill(room_id_array, 0, sizeof(room_id_array) );
      GFL_STD_MemFill(member_num, 0, sizeof(member_num) );

      // 設置可能な部屋IDをリストにして返す
      cnt = GetRecoverTrainerSetRoomIdArray( p_dungeon_data ,n ,room_id_array, member_num );

      // 配置不能な場合対処
      GF_ASSERT_MSG(cnt!=0,"回復トレーナーが置ける場所がありません\n");
      if( cnt <= 0 ){ return; }

      SetRecoverTrainer( p_dungeon_data ,n,area_no ,room_id_array ,cnt ,member_num ,&rand_context ,&recover_index);
    }

    GF_ASSERT_MSG(recover_index==top_floor,"回復トレーナーが正しく設置できていません\n");
  }

  // ゲートレーナーを配置する
  {
    s32 id_pos_array[INFINITY_DUNGEON_TRID_MAX];
    s32 room_id_array[INFINITY_DUNGEON_TRID_MAX];

    s32 data_cnt = 0;
    u8 index_rand;
    
    // 配置階を決める
    p_dungeon_data->gate_trainer_floor = GFL_STD_Rand( &rand_context ,top_floor );

    // 設置可能な部屋IDをリストにして返す
    data_cnt = GetGateTrainerSetTrIndexArray( p_dungeon_data ,p_dungeon_data->gate_trainer_floor ,id_pos_array ,room_id_array );
    
    // 配置不能な場合対処
    GF_ASSERT_MSG(data_cnt!=0,"ゲートトレーナーが置ける場所がありません\n");
    if( data_cnt <= 0 ){ return; }
    
    // 配列から実際のトレーナー位置をひとつに決定
    index_rand = GFL_STD_Rand( &rand_context ,data_cnt );
    p_dungeon_data->gate_trainer_room_id = room_id_array[ index_rand ];
    p_dungeon_data->gate_trainer_id_pos = id_pos_array[ index_rand ];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ブラック用トレーナーIDテーブルを取得
 *
 *	@param	*p_trid_array 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int GetSetTrainerIDTable( const INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,u32 *p_trid_array ,GFL_STD_RandContext *rand_context)
{
  int i;
  u16 tr_id = 0;
  int cnt = 0;
  u8 version = INF_TOOL_DUNGEON_GetDungeonColor( p_dungeon_data );
  u32 trid_array[INF_DUNGEON_TR_DATA_MAX];
  u32 tmp_trid_array[INF_DUNGEON_TR_DATA_MAX];
  u32 add_data_num = 0;

  // メモリをクリーン
  GFL_STD_MemFill( trid_array ,0 , sizeof(trid_array));

  // 使用できるトレーナーデータを決定
  for( i = 0 ; i < INF_TR_AREA_DATA_MAX; i++ )
  {
    const INF_TR_AREA_DATA* sc_tr_area_data = INF_TOOL_GetTrainerParam( i );
    GF_ASSERT_MSG(cnt<INF_DUNGEON_TR_DATA_MAX,"無限ダンジョンのトレーナー抽選に使用するトレーナー数が多すぎます(%d<%d)\n",cnt,INF_DUNGEON_TR_DATA_MAX);

    if( (area_no >= sc_tr_area_data->start_area ) &&
        (area_no <= sc_tr_area_data->end_area ) )
    {
      // バージョン指定なし=0
      if( (sc_tr_area_data->version == version) ||
          (sc_tr_area_data->version == 0) )
      {
        trid_array[add_data_num] = i;
        add_data_num++;
      }
    }
  }

  // トレーナータイプごとに抽選データを取得する
  {
    u8 tr_type;
    const u8 *p_trtype_cnt = INF_TOOL_GetTrTypeCntTable(area_no,version); 

    for( tr_type = 0; tr_type < INF_TR_TYPE_MAX; tr_type++ )
    {
      // 配置人数が0の場合のみテーブルに追加する作業を行う
      if( p_trtype_cnt[tr_type] > 0 )
      {
        int add_index = 0;
        int add_cnt = p_trtype_cnt[tr_type];
        int tmp_data_cnt = 0;
        GFL_STD_MemFill( tmp_trid_array ,0 , sizeof(tmp_trid_array));

        // タイプを絞り込んだテーブルを作成
        for( i = 0; i < add_data_num; i++ )
        {
          if( INF_TOOL_GetTrainerType( trid_array[i] ) == tr_type )
          {
            // トレーナーIDを一時保存
            tmp_trid_array[tmp_data_cnt] = trid_array[i];
            tmp_data_cnt++;
          }
        }
        // テーブル内をランダムにする
        INF_TOOL_RndArray( tmp_trid_array, tmp_data_cnt, rand_context );

        // テーブルに追加
        while( add_cnt > 0 )
        {
          p_trid_array[cnt] = tmp_trid_array[add_index];
          add_cnt--;    // 追加する数
          add_index++;  // 追加した位置
          cnt++;        // 出力トレーナー数
          GF_ASSERT_MSG(add_index<=tmp_data_cnt,"トレーナータイプ数の指定が間違っています[タイプインデックス->%dのトレーナーが足りない]\n",tr_type);
        }
      }
    }
  }
  
  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナーの設置可能な部屋IDを返す
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_no 
 *	@param	*room_id_array 
 *	@param	*member_num 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s32 GetRecoverTrainerSetRoomIdArray( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no ,int *room_id_array ,u32 *member_num )
{
  int i;
  s32 cnt = 0;
  for( i = 0; i<ROOM_MAX; i++)
  {
    u8 pos_type = p_dungeon_data->room_data[floor_no][i].tr_pos_type;
    if( INF_TOOL_GetMemberNum(pos_type) > 0 )
    {
      room_id_array[cnt] = i;
      member_num[cnt] = INF_TOOL_GetMemberNum(pos_type);
      cnt++;
    }
  }

  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナー設置
 *
 *	@param	*p_dungeon_data ダンジョンデータ
 *	@param	floor_no        設置フロア
 *	@param	*room_id_array  配置可能部屋リスト
 *	@param	cnt             配置可能部屋数
 *	@param	*member_num     トレーナー数配列
 *	@param	*rand_context   
 *	@param	*recover_index  回復トレーナーインデックス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetRecoverTrainer( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no ,u32 area_no,const int *room_id_array ,int cnt,const u32 *member_num , GFL_STD_RandContext *rand_context ,u32 *recover_index )
{
  int id_pos = 0;
  u16 tr_id;
  u32 room_id_index;
  u32 member_index;

  room_id_index = GFL_STD_Rand( rand_context ,cnt);
  member_index = GFL_STD_Rand( rand_context ,member_num[room_id_index]);
  id_pos = INF_TOOL_DUNGEON_GetRoomTrainerIDPos( p_dungeon_data ,floor_no ,room_id_array[room_id_index] ,member_index);

  // エリア番号から、適切な回復トレーナーを抽選
  tr_id = GetRecoverTrainerID( p_dungeon_data ,area_no ,rand_context);

  p_dungeon_data->tr_data[floor_no].tr_id[id_pos] = tr_id;
  p_dungeon_data->recover_data[(*recover_index)] = tr_id;
  (*recover_index)++;
  p_dungeon_data->recover_room_id[floor_no] = room_id_array[room_id_index];
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートトレーナーに上書きできるトレーナーリストを作成する（トレーナーデータインデックス配列で返す）
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_no 
 *	@param	id_pos_array 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s32 GetGateTrainerSetTrIndexArray( INF_DUNGEON_DATA  *p_dungeon_data ,s32 floor_no ,s32* id_pos_array, s32* room_id_array )
{
  s32 i;
  s32 room_id;
  s32 cnt = 0;
  s32 boss_room_id = -1;

  for( room_id = 0; room_id<ROOM_MAX; room_id++)
  {
    u8 room_type = p_dungeon_data->room_data[floor_no][room_id].room_type;
    u8 pos_type = p_dungeon_data->room_data[floor_no][room_id].tr_pos_type;
    
    // ボス部屋には置けない、またボス部屋の位置も覚えておく
    if( room_type ==  ROOM_TYPE_BOSS){ 
      boss_room_id = room_id;
      continue;
    }

    // 通常部屋以外には配置しない
    if( room_type !=  ROOM_TYPE_DEFAULT){ continue; }

    // ボス部屋が見つかったときは、ボス部屋の下の部屋には置かない
    if( ( boss_room_id != -1 ) &&
        boss_room_id == (room_id - ROOM_MAX_X) ){ continue; }

    // トレーナーIDを保存する
    {
      u8 member_num = INF_TOOL_GetMemberNum(pos_type);

      // トレーナーインデックスを取得
      while( member_num > 0 )
      {
        BOOL is_recover = FALSE;
        s32 id_pos;
        u16 tr_id;
        member_num--;
        id_pos = INF_TOOL_DUNGEON_GetRoomTrainerIDPos( p_dungeon_data ,floor_no ,room_id ,member_num );
        tr_id = INF_TOOL_DUNGEON_GetTrIDByIDPos( p_dungeon_data,floor_no ,id_pos );

        // 回復トレーナー以外が配置対象
        for( i = 0; i < INFINITY_DUNGEON_RECOVERTR_MAX; i++ )
        {
          if( p_dungeon_data->recover_data[i] == tr_id)
          {
            is_recover = TRUE;
            break;
          }
        }

        // 回復トレーナーでない場合は保存
        if( is_recover == FALSE )
        {
          id_pos_array[ cnt ] = id_pos;
          room_id_array[ cnt ] = room_id;
          cnt ++;
          GF_ASSERT_MSG(cnt<INFINITY_DUNGEON_TRID_MAX,"配置人数が多すぎます(%d<%d)\n",cnt,INFINITY_DUNGEON_TRID_MAX);
        }
      }
    }
  }

  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エリアごとの回復トレーナーID取得
 *
 *	@param	*p_dungeon_data 
 *	@param	*rand_context 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetRecoverTrainerID( const INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,GFL_STD_RandContext *rand_context )
{
  const u32 dungeon_color = INF_TOOL_DUNGEON_GetDungeonColor( p_dungeon_data );
  u32 start_trid = INF_TRID_START_RECOVERY;
  u32 trid_ofs   = GFL_STD_Rand( rand_context ,INF_TRID_RECOVERY_AREA_NUM);

  // 現在のエリアによって分岐
  switch( area_no )
  {
  case INFINITY_DUNGEON_AREA_ID_1:
  case INFINITY_DUNGEON_AREA_ID_2:
  case INFINITY_DUNGEON_AREA_ID_3:
  case INFINITY_DUNGEON_AREA_ID_4:
  case INFINITY_DUNGEON_AREA_ID_5:
    {
      if( dungeon_color == VERSION_B2 )
      {
        start_trid = INF_TRID_RECOVERY_START_AREA1_5_B;
      }
      else
      {
        start_trid = INF_TRID_RECOVERY_START_AREA1_5_W;
      }
    }
    break;
  case INFINITY_DUNGEON_AREA_ID_6:
  case INFINITY_DUNGEON_AREA_ID_7:
  case INFINITY_DUNGEON_AREA_ID_8:
  case INFINITY_DUNGEON_AREA_ID_9:
  case INFINITY_DUNGEON_AREA_ID_10:
    {
      if( dungeon_color == VERSION_B2 )
      {
        start_trid = INF_TRID_RECOVERY_START_AREA6_10_B;
      }
      else
      {
        start_trid = INF_TRID_RECOVERY_START_AREA6_10_W;
      }
    }
    break;
  default:
    GF_ASSERT(0);
    start_trid = INF_TRID_RECOVERY_START_AREA1_5_B;
    break;
  }

  // 回復トレーナーID
  start_trid += trid_ofs;

  return start_trid;
}

