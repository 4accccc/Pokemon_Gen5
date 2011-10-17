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
static u8 GetFloorNum( u8 area_id );
static void RemoveTrRoom( INF_DUNGEON_DATA *p_dungeon_data ,const u8 floor_no ,int rmv_num ,GFL_STD_RandContext *rand_context );
static int GetElevatorSetArray( ROOM_DATA *p_room_data ,int *p_index_array ,int fs_x ,int fs_y);
static ROOM_DATA* GetDigPosRoomData( const DIG_POS *p_dig_pos,ROOM_DATA* p_room_data);
static const ROOM_DATA* GetDigPosRoomDataConst( const DIG_POS *p_dig_pos,const ROOM_DATA* p_room_data);
static void MoveDigPos(u8 dir ,INF_DIG_DATA *p_dig_data ,ROOM_DATA *p_room_data);
static int GetCanMove( const DIG_POS *p_dig_pos ,const ROOM_DATA *p_room_data ,int *p_array,int fs_x ,int fs_y ,int mode);
static BOOL CheckCloseAllDoor( const DIG_POS *p_dig_pos ,const ROOM_DATA *p_room_data);
static void pop_dig_pos( INF_DIG_DATA *p_dig_data );
static void push_dig_pos( INF_DIG_DATA *p_dig_data ,const DIG_POS *p_dig_pos);
static BOOL CheckDigEnd( const ROOM_DATA *p_room_data ,int fs_x ,int fs_y);

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
void INF_DCREATOR_SetDungeonData( INF_DUNGEON_DATA *p_dungeon_data ,int area_no, u32 seed ,int fs_x ,int fs_y)
{
  int i,j,k;
  DIG_POS elv_pos[INFINITYDUNGEON_FLOOR_MAX];
  DIG_POS boss_pos;
  s32 boss_floor;
  GFL_STD_RandContext rand_context;
  const INF_ROOM_SETUP_PARAM *sc_room_setup = INF_TOOL_GetRoomSetupParam(area_no);

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
        p_dungeon_data->room_data[i][j].tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;

        // 扉を閉じる
        for( k = 0 ; k < 4 ; k++ )
        {
          p_dungeon_data->room_data[i][j].can_move[k]  = FALSE;
        }
      }

    }
  }

  // ボス部屋の設置
  {
    ROOM_DATA *p_rd;
    ROOM_DATA *p_room_data;
    int floor_no;
    u8 top_floor = GetFloorNum( area_no );
    floor_no = GFL_STD_Rand( &rand_context ,top_floor);
    p_room_data = p_dungeon_data->room_data[floor_no];
    boss_pos.x = GFL_STD_Rand( &rand_context ,fs_x);
    boss_pos.y = GFL_STD_Rand( &rand_context ,fs_y-1); // 一番下の部屋には置けない
    p_rd= GetDigPosRoomData( &boss_pos ,p_room_data);
    p_rd->room_type = ROOM_TYPE_BOSS;
    p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    boss_floor = floor_no;
  }

  // エレベータ設置配列を取得
  {
    for( i = 0; i<INFINITYDUNGEON_FLOOR_MAX; i++ )
    {
      ROOM_DATA *p_room_data = p_dungeon_data->room_data[i];
      int index_array[ROOM_MAX];
      int room_index;
      int max = GetElevatorSetArray(p_room_data,index_array,fs_x,fs_y);

      #ifndef VS_PROJECT
        GF_ASSERT_MSG((max > 0),"エレベータを置く場所がありません\n");
      #endif
      room_index = GFL_STD_Rand( &rand_context ,max);
      p_room_data[index_array[room_index]].room_type    = ROOM_TYPE_ELEVATOR;
      p_room_data[index_array[room_index]].tr_pos_type  = INF_TR_ROOM_POS_TYPE_NONE;

      // エレベーター位置を格納（穴掘りスタート位置に使用）
      elv_pos[i].x = index_array[room_index]%ROOM_MAX_X;
      elv_pos[i].y = index_array[room_index]/ROOM_MAX_X;

    }
  }

  //ランダム
  {
    int floor_no;
    for( floor_no = 0; floor_no < INFINITYDUNGEON_FLOOR_MAX; floor_no++ )
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

  // 部屋タイプ取得部分はVSプロジェクト移植対象外です
#ifndef VS_PROJECT
  {
    for( i = 0; i<INFINITYDUNGEON_FLOOR_MAX; i++ )
    {
      u8  no_tr_room_data[ROOM_MAX];
      int no_tr_room_cnt = 0;
      GFL_STD_MemFill(no_tr_room_data ,ROOM_MAX ,sizeof(u8)*ROOM_MAX );

      for( j = 0; j<ROOM_MAX; j++ )
      {
        // 部屋タイプを抽選する(ボス部屋、エレベータ部屋など既に配置済みの場所には配置しない)
        if( p_dungeon_data->room_data[i][j].room_type == ROOM_TYPE_DEFAULT )
        {
          int x = j % ROOM_MAX_X;
          int y = j / ROOM_MAX_X;

          if( (x < fs_x) && (y < fs_y ) )
          {
            BOOL is_set_trainer  = FALSE;
            u32 sum_num = 0;
            u32 rnd_num = GFL_STD_Rand( &rand_context ,100 ) + 1; // 抽選値を決定（1~100）
            for( k = 0 ; k < sc_room_setup->member_num; k++ )
            {
              const INF_ROOM_DATA *p_rs_data = &sc_room_setup->room_setup_data[k];
              const u32 dungeon_color = p_dungeon_data->version;
              const u8 room_version = INF_TOOL_GetRoomVersion(p_rs_data->tr_pos_type);

              // バージョンでの制限
              if( (dungeon_color == VERSION_B2) && (room_version == INF_B2_ONLY_DUNGEON) || // ブラック専用
                  (dungeon_color == VERSION_W2) && (room_version == INF_W2_ONLY_DUNGEON) || // ホワイト専用
                  (room_version == INF_MULTI_DUNGEON) )
              {
                sum_num += p_rs_data->probability;
                if( rnd_num <= sum_num )
                {
                  p_dungeon_data->room_data[i][j].tr_pos_type = p_rs_data->tr_pos_type;
                  p_dungeon_data->room_data[i][j].room_type = INF_TOOL_GetRoomType(p_rs_data->tr_pos_type);
                  is_set_trainer = TRUE;
                  break;
                }
              }
            }
            if( is_set_trainer == FALSE )
            {
              // 未配置部屋は配置人数が０のとき用に保存しておく
              no_tr_room_data[no_tr_room_cnt] = j;
              no_tr_room_cnt++;
            }
          }
        }
      }
      
      // 最低5部屋は置く（BOSS+エレベータ+回復×2+通常部屋=5）
      {
        int no_tr_room_index  = 0;
        int tr_room_cnt   = (fs_x*fs_y) - no_tr_room_cnt; // 部屋数 - トレーナーが配置されていない部屋 = トレーナー配置済みの部屋数
        for( j = tr_room_cnt; j < INFINITY_DUNGEON_TR_ROOM_MIN ;j++ )
        {
          u8 room_id = no_tr_room_data[no_tr_room_index];
          p_dungeon_data->room_data[i][room_id].tr_pos_type = INF_ROOM_POS_TYPE_EX_1;
          p_dungeon_data->room_data[i][room_id].room_type = INF_TOOL_GetRoomType(INF_ROOM_POS_TYPE_EX_1);
          no_tr_room_index++;
          // 未配置部屋数以上は配置できない
#ifndef VS_PROJECT
          GF_ASSERT( no_tr_room_index <= no_tr_room_cnt  );
#endif
        }
      }
    }
  }

  // 配置人数が32人以上の場合、ランダムで適当な人数まで減らす
  {
    int floor_no;
    for( floor_no = 0; floor_no < INFINITYDUNGEON_FLOOR_MAX; floor_no++ )
    {
      int member_num = 0;
      for( i = 0; i< ROOM_MAX; i++ )
      {
        u8 pos_type = p_dungeon_data->room_data[floor_no][i].tr_pos_type;
        if( INF_TOOL_GetMemberNum(pos_type) > 0 )
        {
          member_num += INF_TOOL_GetMemberNum(pos_type);
        }
      }

      if( member_num > INFINITY_DUNGEON_TRID_MAX )
      {
        int rmv_num = member_num - INFINITY_DUNGEON_TRID_MAX;
        RemoveTrRoom( p_dungeon_data , floor_no , rmv_num , &rand_context);
      }

#ifdef  PM_DEBUG
      // 再度人数を数える
      member_num = 0;
      for( i = 0; i< ROOM_MAX; i++ )
      {
        u8 pos_type = p_dungeon_data->room_data[floor_no][i].tr_pos_type;
        if( INF_TOOL_GetMemberNum(pos_type) > 0 )
        {
          member_num += INF_TOOL_GetMemberNum(pos_type);
        }
      }
      // 人数チェック
      GF_ASSERT_MSG( member_num > 0 ,"配置人数が０人です\n");
#endif
    }
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定フロアが何回あるかを返す
 *
 *	@param	now_area 
 *
 *	@retval
 *
 *	@note	 無限ダンジョンセーブにも同じ機能があるが、
 *	       この機能のみVSプロジェクトに載せる
 */
//-----------------------------------------------------------------------------
static u8 GetFloorNum( u8 area_id )
{
  // フロアごとのスタート回数
  const u8 floor_num_table[ INFINITY_DUNGEON_AREA_ID_MAX ] =
  {
    INFINITYDUNGEON_FLOOR_AREA1_MAX,
    INFINITYDUNGEON_FLOOR_AREA2_MAX,
    INFINITYDUNGEON_FLOOR_AREA3_MAX,
    INFINITYDUNGEON_FLOOR_AREA4_MAX,
    INFINITYDUNGEON_FLOOR_AREA5_MAX,
    INFINITYDUNGEON_FLOOR_AREA6_MAX,
    INFINITYDUNGEON_FLOOR_AREA7_MAX,
    INFINITYDUNGEON_FLOOR_AREA8_MAX,
    INFINITYDUNGEON_FLOOR_AREA9_MAX,
    INFINITYDUNGEON_FLOOR_AREA10_MAX,
  };

  return floor_num_table[ area_id ];
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
    if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
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
    p_dungeon_data->room_data[floor_no][index_array[i]].tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;

    if( rmv_num <= 0 )
    {
      return;
    }
  }

  #ifndef VS_PROJECT
    GF_ASSERT_MSG(0,"トレーナーが配置されている部屋がありません\n");
  #endif
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

