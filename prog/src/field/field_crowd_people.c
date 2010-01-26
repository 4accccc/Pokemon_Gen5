//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_crowd_people.c
 *	@brief  群集管理処理
 *	@author	tomoya takahshi
 *	@date		2010.01.26
 *
 *	モジュール名：FIELD_CROWD_PEOPLE
 *
 *	C03の路地用　群集処理　　沢山の動作モデルを使用して、表現
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "field/field_const.h"

#include "fieldmap.h"
#include "field_crowd_people.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

// 動作シーケンス
enum
{
  // 通常
  FIELD_CROWD_PEOPLE_SEQ_NORMAL_WALKWAIT,// 動作完了待ち
  FIELD_CROWD_PEOPLE_SEQ_NORMAL_WALK,    // 判断＋指示
  FIELD_CROWD_PEOPLE_SEQ_NORMAL_END,     // 終了

  // 意地悪
  FIELD_CROWD_PEOPLE_SEQ_NASTY_WALKWAIT,// 動作完了待ち
  FIELD_CROWD_PEOPLE_SEQ_NASTY_WALK,    // 判断＋指示
  FIELD_CROWD_PEOPLE_SEQ_NASTY_END,     // 終了
};

// 動作タイプ
enum
{
  FIELD_CROWD_PEOPLE_TYPE_NORMAL,   // 普通の人
  FIELD_CROWD_PEOPLE_TYPE_NASTY,    // いじわる　な人

  FIELD_CROWD_PEOPLE_TYPE_MAX,    //
};

// 人物最大数
#define FIELD_CROWD_PEOPLE_MAX  (20)

// 群集OBJ識別ID
#define FIELD_CROWD_PEOPLE_ID (MMDL_ID_GIMMICK) // 仕掛けようOBJIDを使用させてもらう

// 人物基本値ヘッダー
static const MMDL_HEADER sc_MMDL_HEADER = 
{
  FIELD_CROWD_PEOPLE_ID,  ///<識別ID
  0,                      ///<表示するOBJコード
  MV_DMY,                 ///<動作コード
  0,                      ///<イベントタイプ
  0,                      ///<イベントフラグ
  0,                      ///<イベントID
  0,                      ///<指定方向
  0,                      ///<指定パラメタ 0
  0,                      ///<指定パラメタ 1
  0,                      ///<指定パラメタ 2
	0,			                ///<X方向移動制限
	0,			                ///<Z方向移動制限
	MMDL_HEADER_POSTYPE_GRID,///<ポジションタイプ
};

//-------------------------------------
///	通常動作　定数
//=====================================
#define FIELD_CROWD_PEOPLE_WK_PLAYER_HIT_GRID (2) // 何歩前までチェックするのか？


//-------------------------------------
///	意地悪動作　定数
//=====================================
#define FIELD_CROWD_PEOPLE_WK_PLAYER_HIT_NASTY_GRID (1) // 何歩前までチェックするのか？



//-------------------------------------
///	起動情報
//=====================================
#define FIELD_CROWD_PEOPLE_BOOT_POINT_MAX (2)
#define FIELD_CROWD_PEOPLE_OBJ_CODE_MAX (4)   // 表示OBJ_CODEの最大数


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	起動点情報
//=====================================
typedef struct 
{
  // 起動グリッド範囲
  u16 top;
  u16 bottom;
  u16 left;
  u16 right;

  // 移動方向
  u16 move_dir;

  // 移動距離
  u16 move_grid;

} FIELD_CROWD_PEOPLE_BOOT_POINT;

//-------------------------------------
///	起動情報
//=====================================
typedef struct 
{
  s16 wait;       // 起動ウエイト 0最短 
  u8 point_num;  // 起動ポイントの数
  u8 obj_code_num;// OBJCODEの数
  u16 obj_code_tbl[FIELD_CROWD_PEOPLE_OBJ_CODE_MAX]; // OBJCODEのテーブル
  
  FIELD_CROWD_PEOPLE_BOOT_POINT point[FIELD_CROWD_PEOPLE_BOOT_POINT_MAX];
} FIELD_CROWD_PEOPLE_BOOT_DATA;


//-------------------------------------
///	起動コントロールシステム
//=====================================
typedef struct 
{
  FIELD_CROWD_PEOPLE_BOOT_DATA boot_data;
  s16 wait[ FIELD_CROWD_PEOPLE_BOOT_POINT_MAX ];
} FIELD_CROWD_PEOPLE_BOOL_CONTROL;



//-------------------------------------
///	管理１オブジェ
//=====================================
typedef struct 
{
  u8 flag;
  u8 seq;
  u8 type;
  u8 move_dir;
  u32 move_grid;
  MMDL* p_mmdl;
} FIELD_CROWD_PEOPLE_WK;


//-------------------------------------
///	管理ワーク
//=====================================
typedef struct 
{
  FIELD_PLAYER* p_player;
  MMDLSYS* p_fos;
  FIELD_CROWD_PEOPLE_WK people_wk[FIELD_CROWD_PEOPLE_MAX];

  FIELD_CROWD_PEOPLE_BOOL_CONTROL boot_control;
} FIELD_CROWD_PEOPLE;




//-------------------------------------
///	ダミー起動情報
#include "arc/fieldmap/fldmmdl_objcode.h"
//=====================================
static const FIELD_CROWD_PEOPLE_BOOT_DATA sc_FIELD_CROWD_PEOPLE_BOOT_DATA = 
{
  30,
  2,
  3,
  { BUSINESSMAN, OL, MAID },

  {
    {
      // top bottom left right
      0, 0, 16, 19,
      // move_dir
      DIR_DOWN,
      // move_grid
      63
    },
    {
      // top bottom left right
      63, 63, 16, 19,
      // move_dir
      DIR_UP,
      // move_grid
      63
    },
  },
  
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------





static void FIELD_CROWD_PEOPLE_TASK_Create( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void FIELD_CROWD_PEOPLE_TASK_Delete( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void FIELD_CROWD_PEOPLE_TASK_Update( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );


//-------------------------------------
///	ワーク管理
//=====================================
static void FIELD_CROWD_PEOPLE_WK_Init( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, int zone_id );
static void FIELD_CROWD_PEOPLE_WK_Exit( FIELD_CROWD_PEOPLE_WK* p_wk );
static void FIELD_CROWD_PEOPLE_WK_Main( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player );
static void FIELD_CROWD_PEOPLE_WK_SetUp( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player, u16 obj_code, u16 type, u16 gx, u16 gz, u16 move_dir, u16 move_grid );
static void FIELD_CROWD_PEOPLE_WK_ClearMove( FIELD_CROWD_PEOPLE_WK* p_wk );


// 起動管理
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_Init( FIELD_CROWD_PEOPLE_BOOL_CONTROL* p_wk, int zone_id );
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_Exit( FIELD_CROWD_PEOPLE_BOOL_CONTROL* p_wk );

// タイプごとの動き
static void FIELD_CROWD_PEOPLE_WK_SetUpNormal( FIELD_CROWD_PEOPLE_WK* p_wk );
static void FIELD_CROWD_PEOPLE_WK_SetUpNasty( FIELD_CROWD_PEOPLE_WK* p_wk );
static void FIELD_CROWD_PEOPLE_WK_MainNormal( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player );
static void FIELD_CROWD_PEOPLE_WK_MainNasty( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player );


// 引数歩前に自機がいないかチェック
static BOOL FIELD_CROWD_PEOPLE_WK_IsFrontPlayer( const FIELD_CROWD_PEOPLE_WK* cp_wk, const FIELD_PLAYER* cp_player, u16 grid );


static const FLDMAPFUNC_DATA sc_FLDMAPFUNC_DATA = 
{
  0,
  sizeof(FIELD_CROWD_PEOPLE),
  FIELD_CROWD_PEOPLE_TASK_Create,
  FIELD_CROWD_PEOPLE_TASK_Delete,
  FIELD_CROWD_PEOPLE_TASK_Update,
  NULL,
};



//----------------------------------------------------------------------------
/**
 *	@brief  群集管理タスク生成
 *
 *	@param	sys   システム
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
FLDMAPFUNC_WORK * FIELD_CROWD_PEOPLE_Create( FLDMAPFUNC_SYS * sys )
{
  return FLDMAPFUNC_Create( sys, &sc_FLDMAPFUNC_DATA );
}



//----------------------------------------------------------------------------
/**
 *	@brief  群集ワーク生成
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_TASK_Create( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  FIELD_CROWD_PEOPLE* p_wk = p_work;
  int i;
  int zone_id;

  // 管理システムワーク初期化
  p_wk->p_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );
  p_wk->p_fos     = FIELDMAP_GetMMdlSys( p_fieldmap );
  zone_id = FIELDMAP_GetZoneID( p_fieldmap );

  for( i=0; i<FIELD_CROWD_PEOPLE_MAX; i++ )
  {
    FIELD_CROWD_PEOPLE_WK_Init( &p_wk->people_wk[i], p_wk->p_fos, zone_id ); 
  }

  // ゾーンから起動情報を読み込み
  {
    //GFL_STD_MemCopy( &sc_FIELD_CROWD_PEOPLE_BOOT_DATA, &p_wk->boot_data, sizeof(FIELD_CROWD_PEOPLE_BOOT_DATA) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  群集ワーク破棄
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_TASK_Delete( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  FIELD_CROWD_PEOPLE* p_wk = p_work;
  int i;
  
  // 管理システムワーク破棄
  for( i=0; i<FIELD_CROWD_PEOPLE_MAX; i++ )
  {
    FIELD_CROWD_PEOPLE_WK_Exit( &p_wk->people_wk[i] ); 
  }

  p_wk->p_player  = NULL;
  p_wk->p_fos     = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  群集　更新
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_TASK_Update( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  FIELD_CROWD_PEOPLE* p_wk = p_work;
  int i;

  // 起動チェック
  

  
  // 管理システムワーク更新
  for( i=0; i<FIELD_CROWD_PEOPLE_MAX; i++ )
  {
    FIELD_CROWD_PEOPLE_WK_Main( &p_wk->people_wk[i], p_wk->p_fos, p_wk->p_player ); 
  }
}







//----------------------------------------------------------------------------
/**
 *	@brief  人物ワーク  初期化
 *
 *	@param	p_wk      ワーク
 *	@param	p_fos     動作モデルシステム
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_Init( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, int zone_id )
{
  p_wk->p_mmdl    = MMDLSYS_AddMMdl( p_fos, &sc_MMDL_HEADER, zone_id ); 
  p_wk->flag      = 0;
  p_wk->seq       = 0;
  p_wk->type      = 0;
  p_wk->move_dir  = 0;

  // Vanish
  MMDL_SetStatusBitVanish( p_wk->p_mmdl, TRUE );

  // 描画オフセット設定
  {
    VecFx32 offset = {0,0,0};

    // ハングリッド　＋　－
    offset.x = -((FIELD_CONST_GRID_SIZE/2)<<FX32_SHIFT) + GFUser_GetPublicRand(FIELD_CONST_GRID_SIZE<<FX32_SHIFT);
    MMDL_SetVectorDrawOffsetPos( p_wk->p_mmdl, &offset );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物ワーク　破棄
 *
 *	@param	p_wk      ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_Exit( FIELD_CROWD_PEOPLE_WK* p_wk )
{
  // オブジェを破棄
  MMDL_Delete( p_wk->p_mmdl );
  p_wk->p_mmdl = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物ワーク　メイン
 *
 *	@param	p_wk        ワーク
 *	@param  p_fos       動作システムワーク
 *	@param	cp_player   プレイヤーシステム
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_Main( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player )
{
  static void (* const cpFunc[FIELD_CROWD_PEOPLE_TYPE_MAX])( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player ) = 
  {
    FIELD_CROWD_PEOPLE_WK_MainNormal,
    FIELD_CROWD_PEOPLE_WK_MainNasty,
  };
  
  if( p_wk->flag == FALSE ){
    return ;
  }
  
  GF_ASSERT( p_wk->type < FIELD_CROWD_PEOPLE_TYPE_MAX );
  
  // タイプごとの動き
  cpFunc[ p_wk->type ]( p_wk, p_fos, cp_player );
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物ワーク　動作セットアップ
 *
 *	@param	p_wk        ワーク
 *	@param	p_fos       動作モデルシステム
 *	@param	cp_player   主人公ワーク
 *	@param  obj_code    見た目
 *	@param  type        動作タイプ
 *	@param	gx          開始グリッド位置
 *	@param	gz          開始グリッド位置
 *	@param	move_dir    動作方向
 *	@param  move_grid   動作グリッド距離
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_SetUp( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player, u16 obj_code, u16 type, u16 gx, u16 gz, u16 move_dir, u16 move_grid )
{
  static void (* const cpFunc[FIELD_CROWD_PEOPLE_TYPE_MAX])( FIELD_CROWD_PEOPLE_WK* p_wk ) = 
  {
    FIELD_CROWD_PEOPLE_WK_SetUpNormal,
    FIELD_CROWD_PEOPLE_WK_SetUpNasty,
  };

  GF_ASSERT( p_wk->flag == FALSE );

  // 動作開始
  p_wk->flag = TRUE; 

  // 表示ON
  MMDL_SetStatusBitVanish( p_wk->p_mmdl, FALSE );

  // 人物の情報を大きく変更
  {
    MMDL_HEADER header = sc_MMDL_HEADER;
    header.obj_code = obj_code;
    header.dir      = move_dir;
    MMDLHEADER_SetGridPos( &header, gx, gz, 0 );
    MMDL_ChangeMoveParam( p_wk->p_mmdl, &header );
  }
  p_wk->type = type;
  p_wk->move_dir = move_dir;
  p_wk->seq = 0;
  p_wk->move_grid = move_grid;

  // 内部ワーク情報の設定
  cpFunc[ type ]( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brieif 動作情報のクリア
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_ClearMove( FIELD_CROWD_PEOPLE_WK* p_wk )
{
  p_wk->flag = FALSE;
  p_wk->type = 0;
  p_wk->move_dir = 0;
  p_wk->seq = 0;

  // 表示OFF
  MMDL_SetStatusBitVanish( p_wk->p_mmdl, TRUE );
}


//----------------------------------------------------------------------------
/**
 *	@brief  人物　起動　管理処理　初期化
 *
 *	@param	p_wk        ワーク
 *	@param	zone_id     ゾーンID
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_Init( FIELD_CROWD_PEOPLE_BOOL_CONTROL* p_wk, int zone_id )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物　起動　管理処理　破棄
 *
 *	@param	p_wk        ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_Exit( FIELD_CROWD_PEOPLE_BOOL_CONTROL* p_wk )
{
}





//----------------------------------------------------------------------------
/**
 *	@brief  通常動作のセットアップ
 *
 *	@param	p_wk  ワーク
 */ 
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_SetUpNormal( FIELD_CROWD_PEOPLE_WK* p_wk )
{
  MMDL_SetStatusBitHeightGetOFF( p_wk->p_mmdl, TRUE );  // 高さ取得OFF
  MMDL_SetStatusBitAttrGetOFF( p_wk->p_mmdl, TRUE );    // アトリビュート取得OFF
  MMDL_SetStatusBitFellowHit( p_wk->p_mmdl, FALSE );    // 他のオブジェとの判定 OFF
}

//----------------------------------------------------------------------------
/**
 *	@brief  意地悪動作のセットアップ
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_SetUpNasty( FIELD_CROWD_PEOPLE_WK* p_wk )
{
  MMDL_SetStatusBitHeightGetOFF( p_wk->p_mmdl, TRUE );  // 高さ取得OFF
  MMDL_SetStatusBitAttrGetOFF( p_wk->p_mmdl, TRUE );    // アトリビュート取得OFF
  MMDL_SetStatusBitFellowHit( p_wk->p_mmdl, TRUE );    // 他のオブジェとの判定 ON
}

//----------------------------------------------------------------------------
/**
 *	@brief  通常オブジェ動作
 *
 *	@param	p_wk        ワーク
 *	@param	p_fos       動作オブジェシステム
 *	@param	cp_player   主人公ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_MainNormal( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player )
{
  switch( p_wk->seq )
  {
  case FIELD_CROWD_PEOPLE_SEQ_NORMAL_WALKWAIT:// 動作完了待ち
    if( MMDL_CheckEndAcmd( p_wk->p_mmdl ) )
    {
      p_wk->seq ++;
    }
  case FIELD_CROWD_PEOPLE_SEQ_NORMAL_WALK:    // 判断＋指示

    // 終了処理
    if( p_wk->move_grid <= 0 )
    {
      // 終了
      p_wk->seq = FIELD_CROWD_PEOPLE_SEQ_NORMAL_END;
      break;
    }

    // 主人公とぶつかるかチェック
    if( FIELD_CROWD_PEOPLE_WK_IsFrontPlayer( p_wk, cp_player, FIELD_CROWD_PEOPLE_WK_PLAYER_HIT_GRID ) )
    {
      u16 player_dir = FIELD_PLAYER_GetDir( cp_player );
      u16 move_dir;
   
      // よける
      // よける方向を決定
      if( (player_dir == p_wk->move_dir) || (MMDL_TOOL_FlipDir(player_dir) == p_wk->move_dir) )
      {
        // ランダムで９０度先のどちらかの方向にする
        if( GFUser_GetPublicRand(2) == 0 )
        {
          move_dir = player_dir + 2;
        }
        else
        {
          move_dir = MMDL_TOOL_FlipDir(player_dir + 2);
        }
      }
      else
      {
        // プレイヤーの反対方向によける
        move_dir = MMDL_TOOL_FlipDir(player_dir);
      }

      // よけまーす。
      MMDL_SetAcmd( p_wk->p_mmdl, AC_WALK_U_6F + move_dir );
    }
    else
    {
      // そのまま進む
      MMDL_SetAcmd( p_wk->p_mmdl, AC_WALK_U_6F + p_wk->move_dir );
      p_wk->move_grid --; // 移動距離減算
    }
    p_wk->seq = FIELD_CROWD_PEOPLE_SEQ_NORMAL_WALKWAIT;
    break;

  case FIELD_CROWD_PEOPLE_SEQ_NORMAL_END:     // 終了
    // 情報のクリア
    FIELD_CROWD_PEOPLE_WK_ClearMove( p_wk );
    break;

  default:
    GF_ASSERT(0);
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  意地悪オブジェ動作  直進するのみです。
 *
 *	@param	p_wk        ワーク
 *	@param	p_fos       動作オブジェシステム
 *	@param	cp_player   主人公ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_MainNasty( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player )
{
  switch( p_wk->seq )
  {
  case FIELD_CROWD_PEOPLE_SEQ_NASTY_WALKWAIT:// 動作完了待ち
    if( MMDL_CheckEndAcmd( p_wk->p_mmdl ) )
    {
      p_wk->seq ++;
    }
  case FIELD_CROWD_PEOPLE_SEQ_NASTY_WALK:    // 判断＋指示

    // 終了処理
    if( p_wk->move_grid <= 0 )
    {
      // 終了
      p_wk->seq = FIELD_CROWD_PEOPLE_SEQ_NASTY_END;
      break;
    }

    // 主人公とぶつかるかチェック
    if( FIELD_CROWD_PEOPLE_WK_IsFrontPlayer( p_wk, cp_player, FIELD_CROWD_PEOPLE_WK_PLAYER_HIT_NASTY_GRID ) )
    {
      // ぶつかるなら何もしない
      break;
    }
    else
    {
      // そのまま進む
      MMDL_SetAcmd( p_wk->p_mmdl, AC_WALK_U_6F + p_wk->move_dir );
      p_wk->move_grid --; // 移動距離減算
    }
    p_wk->seq = FIELD_CROWD_PEOPLE_SEQ_NASTY_WALKWAIT;
    break;

  case FIELD_CROWD_PEOPLE_SEQ_NASTY_END:     // 終了
    // 情報のクリア
    FIELD_CROWD_PEOPLE_WK_ClearMove( p_wk );
    break;

  default:
    GF_ASSERT(0);
    break;
  }
}




//----------------------------------------------------------------------------
/**
 *	@brief  grid歩前までに自機がいないかチェック
 *
 *	@param	cp_wk         ワーク
 *	@param	cp_player     プレイヤー
 *	@param  grid          グリッド
 *
 *	@retval TRUE  いる
 *	@retval FALSE いない
 */
//-----------------------------------------------------------------------------
static BOOL FIELD_CROWD_PEOPLE_WK_IsFrontPlayer( const FIELD_CROWD_PEOPLE_WK* cp_wk, const FIELD_PLAYER* cp_player, u16 grid )
{
  s16 player_gx, player_gz;
  s16 my_gx, my_gz;
  s32 grid_dif;
  const MMDL* cp_playermmdl = FIELD_PLAYER_GetMMdl( cp_player );
  
  player_gx = MMDL_GetGridPosX( cp_playermmdl );
  player_gz = MMDL_GetGridPosZ( cp_playermmdl );
  my_gx = MMDL_GetGridPosX( cp_wk->p_mmdl );
  my_gz = MMDL_GetGridPosZ( cp_wk->p_mmdl );
  
  switch( cp_wk->move_dir )
  {
  case DIR_UP:
    grid_dif = my_gz - player_gz;
    break;
  case DIR_DOWN:
    grid_dif = player_gz - my_gz;
    break;
  case DIR_LEFT:
    grid_dif = my_gx - player_gx;
    break;
  case DIR_RIGHT:
    grid_dif = player_gx - my_gx;
    break;

  default:
    GF_ASSERT(0);
    break;
  }

  if( grid_dif <= grid )
  {
    return TRUE;
  }
  return FALSE;
}




