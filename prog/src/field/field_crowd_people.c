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

#include "arc/arc_def.h"
#include "arc/script_message.naix"

#include "field/field_const.h"
#include "field/field_msgbg.h"

#include "fieldmap.h"
#include "field_crowd_people.h"


#include "fieldmap/field_crowd_people.naix"

#include "msg/script/msg_crowd_people_scr.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

// 動作シーケンス
enum
{
  // 通常
  FIELD_CROWD_PEOPLE_SEQ_NORMAL_WALKWAIT=0,// 動作完了待ち
  FIELD_CROWD_PEOPLE_SEQ_NORMAL_WALK,    // 判断＋指示
  FIELD_CROWD_PEOPLE_SEQ_NORMAL_END,     // 終了

  // 意地悪
  FIELD_CROWD_PEOPLE_SEQ_NASTY_WALKWAIT=0,// 動作完了待ち
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
#define FIELD_CROWD_PEOPLE_MAX  (32)

// 群集OBJ識別ID
#define FIELD_CROWD_PEOPLE_ID (48) // 仕掛けようOBJIDを使用させてもらう

// 人物基本値ヘッダー
static MMDL_HEADER s_MMDL_HEADER = 
{
  FIELD_CROWD_PEOPLE_ID,  ///<識別ID
  BOY1,                      ///<表示するOBJコード
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

// オブセットを動かす数字
#define FIELD_CROWD_PEOPLE_OFFSET_MOVE_MIN  ( -4 )
#define FIELD_CROWD_PEOPLE_OFFSET_MOVE_RND  ( 8 )

//-------------------------------------
///	通常動作　定数
//=====================================
#define FIELD_CROWD_PEOPLE_WK_PLAYER_HIT_GRID (2) // 何歩前までチェックするのか？


//-------------------------------------
///	意地悪動作　定数
//=====================================
#define FIELD_CROWD_PEOPLE_WK_PLAYER_HIT_NASTY_GRID (1) // 何歩前までチェックするのか？

#define FIELD_CROWD_PEOPLE_PUT_PAR (1) // FIELD_CROWD_PEOPLE_PUT_PAR回に一回意地悪がでる



//-------------------------------------
///	起動情報
//=====================================
#define FIELD_CROWD_PEOPLE_BOOT_POINT_MAX (2)
#define FIELD_CROWD_PEOPLE_OBJ_CODE_MAX (4)   // 表示OBJ_CODEの最大数

#define FIELD_CROWD_PEOPLE_OBJ_WALK_FRAME (6) // 歩きのフレーム数


#define FIELD_CROWD_PEOPLE_SCRIPT_MAX   (4) // OBJCODEのスクリプトの数


//-------------------------------------
///	起動情報リストのデータ
//=====================================
typedef u32 BOOT_ZONEID_TYPE;


//-------------------------------------
///	騒音　定数
//=====================================
#define NOISE_TIME_MIN  ( 16)
#define NOISE_TIME_WAIT_NUM  ( 1 )
#define NOISE_MSG_MAX  ( 20 )
#define NOISE_SIZX  ( 8 )
#define NOISE_SIZY  ( 2 )
#define NOISE_WK_WAIT  ( 48)

#define NOISE_WK_MAX  ( 16 )

static const u8 sc_NOISE_X[ NOISE_WK_MAX ] = 
{
  1,
  22,
  2,
  20,
  4,
  21,
  3,
  19,
  1,
  19,
  2,
  22,
  3,
  20,
  4,
  21,
};
static const u8 sc_NOISE_Y[ NOISE_WK_MAX ] = 
{
  6,
  2,
  17,
  14,
  17,
  10,
  14,
  2,
  10,
  17,
  10,
  6,
  2,
  6,
  14,
  6,
};


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------


//-------------------------------------
///	objcode スクリプトテーブル
//=====================================
typedef struct 
{
  u32 objcode;
  u16 script[FIELD_CROWD_PEOPLE_SCRIPT_MAX];
} FIELD_CROWD_PEOPLE_OBJSCRIPT;


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


  FIELD_CROWD_PEOPLE_OBJSCRIPT* p_objcode_script_tbl;
  u32 objcode_num;
} FIELD_CROWD_PEOPLE_BOOL_CONTROL;


//-------------------------------------
///	騒音ワーク
//=====================================
typedef struct {
  u16 flag;
  u8 x;
  u8 y;
  u16 id;
  s16 wait;
} FIELD_CROWD_PEOPLE_NOISE_WK;



//-------------------------------------
///	サブウィンドウ　騒音システム
//=====================================
typedef struct 
{
  s16 wait;
  s16 wait_max;
  u32 add_idx;
  GFL_MSGDATA * p_msgdata;
  FLDMSGBG* p_fmb;

  FIELD_CROWD_PEOPLE_NOISE_WK wk[NOISE_WK_MAX];
} FIELD_CROWD_PEOPLE_NOISE;


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

  FIELD_CROWD_PEOPLE_NOISE noise;
} FIELD_CROWD_PEOPLE;




//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------





static void FIELD_CROWD_PEOPLE_TASK_Create( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void FIELD_CROWD_PEOPLE_TASK_Delete( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void FIELD_CROWD_PEOPLE_TASK_Update( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );



// 
static FIELD_CROWD_PEOPLE_WK* FIELD_CROWD_PEOPLE_GetClearWk( FIELD_CROWD_PEOPLE* p_sys );
static const FIELD_CROWD_PEOPLE_WK* FIELD_CROWD_PEOPLE_Search( const FIELD_CROWD_PEOPLE* cp_sys, u16 gx, u16 gz, u16 move_dir, u16 type );

//-------------------------------------
///	ワーク管理
//=====================================
static void FIELD_CROWD_PEOPLE_WK_Init( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, int zone_id, const FIELD_CROWD_PEOPLE_BOOL_CONTROL* cp_data, u32 idx );
static void FIELD_CROWD_PEOPLE_WK_Exit( FIELD_CROWD_PEOPLE_WK* p_wk );
static void FIELD_CROWD_PEOPLE_WK_Main( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player );
static void FIELD_CROWD_PEOPLE_WK_SetUp( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player, u16 type, u16 gx, u16 gz, u16 move_dir, u16 move_grid );
static void FIELD_CROWD_PEOPLE_WK_ClearMove( FIELD_CROWD_PEOPLE_WK* p_wk );


// 起動管理
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_Init( FIELD_CROWD_PEOPLE_BOOL_CONTROL* p_wk, int zone_id, HEAPID heapID );
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_Exit( FIELD_CROWD_PEOPLE_BOOL_CONTROL* p_wk );
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_Main( FIELD_CROWD_PEOPLE_BOOL_CONTROL* p_wk, FIELD_CROWD_PEOPLE* p_sysdata );
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_SetUpPeople( const FIELD_CROWD_PEOPLE* cp_sysdata, const FIELD_CROWD_PEOPLE_BOOT_DATA* cp_data, const FIELD_CROWD_PEOPLE_BOOT_POINT* cp_point, FIELD_CROWD_PEOPLE_WK* p_people, u16 add_grid );
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_StartUp( const FIELD_CROWD_PEOPLE_BOOL_CONTROL* cp_wk, FIELD_CROWD_PEOPLE* p_sysdata );

// タイプごとの動き
static void FIELD_CROWD_PEOPLE_WK_SetUpNormal( FIELD_CROWD_PEOPLE_WK* p_wk );
static void FIELD_CROWD_PEOPLE_WK_SetUpNasty( FIELD_CROWD_PEOPLE_WK* p_wk );
static void FIELD_CROWD_PEOPLE_WK_MainNormal( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player );
static void FIELD_CROWD_PEOPLE_WK_MainNasty( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player );


// 引数歩前に自機がいないかチェック
static BOOL FIELD_CROWD_PEOPLE_WK_IsFrontPlayer( const FIELD_CROWD_PEOPLE_WK* cp_wk, const FIELD_PLAYER* cp_player, u16 grid );
static BOOL FIELD_CROWD_PEOPLE_WK_TOOL_IsGridHit( s16 mygx, s16 mygz, s16 pl_gx, s16 pl_gz, u16 grid, u16 move_dir );



// NOISE管理
static void FIELD_CROWD_PEOPLE_NOISE_Init( FIELD_CROWD_PEOPLE_NOISE* p_wk, FLDMSGBG* p_fmb, const FIELD_CROWD_PEOPLE_BOOL_CONTROL* cp_boot, HEAPID heapID );
static void FIELD_CROWD_PEOPLE_NOISE_Exit( FIELD_CROWD_PEOPLE_NOISE* p_wk );
static void FIELD_CROWD_PEOPLE_NOISE_Main( FIELD_CROWD_PEOPLE_NOISE* p_wk, const FIELD_CROWD_PEOPLE_BOOL_CONTROL* cp_boot, const FIELDMAP_WORK* cp_fieldmap );

static void FIELD_CROWD_PEOPLE_NOISEWK_Init( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk, u16 id );
static void FIELD_CROWD_PEOPLE_NOISEWK_Start( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk, FLDMSGBG* p_fmb, GFL_MSGDATA* p_msgdata );
static void FIELD_CROWD_PEOPLE_NOISEWK_Main( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk, FLDMSGBG* p_fmb );
static void FIELD_CROWD_PEOPLE_NOISEWK_Clear( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk );
static void FIELD_CROWD_PEOPLE_NOISEWK_Off( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk, FLDMSGBG* p_fmb );



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
static void FIELD_CROWD_PEOPLE_TASK_Create( FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work ) {
  FIELD_CROWD_PEOPLE* p_wk = p_work;
  int i;
  int zone_id;
  HEAPID heapID;
  FLDMSGBG* p_fmb = FIELDMAP_GetFldMsgBG( p_fieldmap );

  // 管理システムワーク初期化
  p_wk->p_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );
  p_wk->p_fos     = FIELDMAP_GetMMdlSys( p_fieldmap );
  zone_id = FIELDMAP_GetZoneID( p_fieldmap );
  heapID = FIELDMAP_GetHeapID( p_fieldmap );

  // ゾーンから起動情報を読み込み
  FIELD_CROWD_PEOPLE_BOOT_CONTROL_Init( &p_wk->boot_control, zone_id, heapID );

  for( i=0; i<FIELD_CROWD_PEOPLE_MAX; i++ )
  {
    FIELD_CROWD_PEOPLE_WK_Init( &p_wk->people_wk[i], p_wk->p_fos, zone_id, &p_wk->boot_control, i ); 
  }

  // 全員を配置
  FIELD_CROWD_PEOPLE_BOOT_CONTROL_StartUp( &p_wk->boot_control, p_wk );


  // 騒音システム初期化
  FIELD_CROWD_PEOPLE_NOISE_Init( &p_wk->noise, p_fmb, &p_wk->boot_control, heapID );
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

  // ゾーンから起動情報を破棄
  FIELD_CROWD_PEOPLE_BOOT_CONTROL_Exit( &p_wk->boot_control );

  // 騒音システム破棄
  FIELD_CROWD_PEOPLE_NOISE_Exit( &p_wk->noise );

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
  FIELD_CROWD_PEOPLE_BOOT_CONTROL_Main( &p_wk->boot_control, p_wk ); 

  
  // 管理システムワーク更新
  for( i=0; i<FIELD_CROWD_PEOPLE_MAX; i++ )
  {
    FIELD_CROWD_PEOPLE_WK_Main( &p_wk->people_wk[i], p_wk->p_fos, p_wk->p_player ); 
  }

  // 騒音システム更新
  FIELD_CROWD_PEOPLE_NOISE_Main( &p_wk->noise, &p_wk->boot_control, p_fieldmap );
}






//----------------------------------------------------------------------------
/**
 *	@brief  空いているワークを取得
 *
 *	@param	p_sys 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static FIELD_CROWD_PEOPLE_WK* FIELD_CROWD_PEOPLE_GetClearWk( FIELD_CROWD_PEOPLE* p_sys )
{
  int i;

  for( i=0; i<FIELD_CROWD_PEOPLE_MAX; i++ )
  {
    if( p_sys->people_wk[i].flag == FALSE )
    {
      return &p_sys->people_wk[i];
    }
  }

  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  gx gzからmove_dirの方向で　type動作しているオブジェを検索
 *
 *	@param	cp_sys       システムワーク
 *	@param	gx          グリッドX
 *	@param	gz          グリッドZ
 *	@param	move_dir    動作方向
 *	@param	type        タイプ 
 *
 *	@return 人物ワーク
 */
//-----------------------------------------------------------------------------
static const FIELD_CROWD_PEOPLE_WK* FIELD_CROWD_PEOPLE_Search( const FIELD_CROWD_PEOPLE* cp_sys, u16 gx, u16 gz, u16 move_dir, u16 type )
{
  int i;
  s16 move_x;
  s16 move_z;
  s16 dif_x;
  s16 dif_z;
  s16 mdl_gx, mdl_gz;



  switch( move_dir )
  {
  case DIR_UP:
    move_x = 0;
    move_z = -1;
    break;
  case DIR_DOWN:
    move_x = 0;
    move_z = 1;
    break;
  case DIR_RIGHT:
    move_x = 1;
    move_z = 0;
    break;
  case DIR_LEFT:
    move_x = -1;
    move_z = 0;
    break;
  }
  

  for( i=0; i<FIELD_CROWD_PEOPLE_MAX; i++ )
  {
    if( cp_sys->people_wk[i].flag )
    {
      if(cp_sys->people_wk[i].type == FIELD_CROWD_PEOPLE_TYPE_NASTY)
      {
        // グリッド方向にいるか？
        mdl_gx = MMDL_GetGridPosX( cp_sys->people_wk[i].p_mmdl );
        mdl_gz = MMDL_GetGridPosZ( cp_sys->people_wk[i].p_mmdl );

        dif_x = mdl_gx - gx;
        dif_z = mdl_gz - gz;

        if( dif_x != 0 ){
          dif_x /= MATH_ABS( dif_x );
        }
        if( dif_z != 0 ){
          dif_z /= MATH_ABS( dif_z );
        }

        // 移動方向が一致したら、OK
        if( (dif_x == move_x) || (dif_z == move_z) )
        {
          return &cp_sys->people_wk[i];
        }
      }
    }
  }

  return NULL;
}



//----------------------------------------------------------------------------
/**
 *	@brief  人物ワーク  初期化
 *
 *	@param	p_wk      ワーク
 *	@param	p_fos     動作モデルシステム
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_Init( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, int zone_id, const FIELD_CROWD_PEOPLE_BOOL_CONTROL* cp_data, u32 idx )
{
  u16 obj_code;
  u32 rand = GFUser_GetPublicRand(0);
  int i;


  // 見た目決定
  // テーブルからランダムで。
  obj_code = cp_data->boot_data.obj_code_tbl[ rand % cp_data->boot_data.obj_code_num ];

  s_MMDL_HEADER.obj_code = obj_code;
  s_MMDL_HEADER.id = FIELD_CROWD_PEOPLE_ID + idx;

  for( i=0; i<cp_data->objcode_num; i++ )
  {
    if( cp_data->p_objcode_script_tbl[i].objcode == obj_code )
    {
      s_MMDL_HEADER.event_id = cp_data->p_objcode_script_tbl[i].script[ rand % FIELD_CROWD_PEOPLE_SCRIPT_MAX ];
      break;
    }
  }
  // OBJCODEのスクリプトを設定したかチェック
  GF_ASSERT( i < cp_data->objcode_num );
  
  p_wk->p_mmdl    = MMDLSYS_AddMMdl( p_fos, &s_MMDL_HEADER, zone_id ); 
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
    offset.x = (FIELD_CROWD_PEOPLE_OFFSET_MOVE_MIN<<FX32_SHIFT) + GFUser_GetPublicRand(FIELD_CROWD_PEOPLE_OFFSET_MOVE_RND<<FX32_SHIFT);
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

  // 座標を表示
#ifdef PM_DEBUG
  /*
  {
    VecFx32 pos;
    MMDL_GetVectorPos( p_wk->p_mmdl, &pos );
    TOMOYA_Printf( "mdl pos x[%d] y[%d] z[%d]\n", FX_Whole(pos.x), FX_Whole(pos.y), FX_Whole(pos.z) );
  }
  */
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物ワーク　動作セットアップ
 *
 *	@param	p_wk        ワーク
 *	@param	p_fos       動作モデルシステム
 *	@param	cp_player   主人公ワーク
 *	@param  type        動作タイプ
 *	@param	gx          開始グリッド位置
 *	@param	gz          開始グリッド位置
 *	@param	move_dir    動作方向
 *	@param  move_grid   動作グリッド距離
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_WK_SetUp( FIELD_CROWD_PEOPLE_WK* p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player, u16 type, u16 gx, u16 gz, u16 move_dir, u16 move_grid )
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
    MMDL_HEADER header = s_MMDL_HEADER;
    header.obj_code = 10;
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
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_Init( FIELD_CROWD_PEOPLE_BOOL_CONTROL* p_wk, int zone_id, HEAPID heapID )
{
  int i;
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_FIELD_CROWD_PEOPLE, GFL_HEAP_LOWID(heapID) );

  {
    BOOT_ZONEID_TYPE* p_list;
    u32 list_num;
    u32 list_idx;
    u32 size;

    // 起動IDXを求める
    p_list = GFL_ARC_LoadDataAllocByHandle( p_handle, NARC_field_crowd_people_list_data_dat, GFL_HEAP_LOWID(heapID) );
    size = GFL_ARC_GetDataSizeByHandle( p_handle, NARC_field_crowd_people_list_data_dat );
    list_num = size / sizeof(BOOT_ZONEID_TYPE);

    for( i=0; i<list_num; i++ )
    {
      if( p_list[i] == zone_id )
      {
        break;
      }
    }
    GF_ASSERT_MSG( i < list_num, "zoneに群集情報がありません。\n" );
    list_idx = i;

    TOMOYA_Printf( "crowd people list_idx %d\n", list_idx );
    
    // 起動ポイント情報を読み込み
    GFL_ARC_LoadDataOfsByHandle( p_handle, list_idx, 0, sizeof(FIELD_CROWD_PEOPLE_BOOT_DATA), &p_wk->boot_data );

    GFL_HEAP_FreeMemory( p_list );
  }

  // スクリプト情報読み込み
  {
    u32 size;
    size = GFL_ARC_GetDataSizeByHandle( p_handle, NARC_field_crowd_people_script_dat );


    p_wk->p_objcode_script_tbl = GFL_ARC_LoadDataAllocByHandle( p_handle, NARC_field_crowd_people_script_dat, heapID );
    p_wk->objcode_num = size / sizeof(FIELD_CROWD_PEOPLE_OBJSCRIPT);
  }

  GFL_ARC_CloseDataHandle( p_handle );

  for( i=0; i<FIELD_CROWD_PEOPLE_BOOT_POINT_MAX; i++ )
  {
    p_wk->wait[i] = GFUser_GetPublicRand(p_wk->boot_data.wait); // 初期ウエイト値
  }

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
  GFL_HEAP_FreeMemory( p_wk->p_objcode_script_tbl );
  p_wk->p_objcode_script_tbl = NULL;
}


//----------------------------------------------------------------------------
/**
 *	@brief  人物　起動　メイン処理
 *
 *	@param	p_wk        ワーク
 *	@param	p_sysdata   システムデータ
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_Main( FIELD_CROWD_PEOPLE_BOOL_CONTROL* p_wk, FIELD_CROWD_PEOPLE* p_sysdata )
{
  int i;
  
  for( i=0; i<p_wk->boot_data.point_num; i++ )
  {
    // ウエイト時間がいったら、人を出す
    if( p_wk->wait[i] >= p_wk->boot_data.wait )
    {
      FIELD_CROWD_PEOPLE_WK* p_people = FIELD_CROWD_PEOPLE_GetClearWk( p_sysdata );
      if( p_people )
      {
        // そのポイントから人を出す。
        FIELD_CROWD_PEOPLE_BOOT_CONTROL_SetUpPeople( p_sysdata, &p_wk->boot_data, &p_wk->boot_data.point[i], p_people, 0 ); 
        p_wk->wait[i] = 0;
      }
      else
      {
        TOMOYA_Printf( "CROWD people people none\n" );
      }
    }
    else
    {
      p_wk->wait[i] ++;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動管理処理　人物情報のセットアップ
 *
 *	@param	cp_data       起動データ
 *	@param	cp_point      起動ポイントデータ
 *	@param	p_people      セットアップ人物
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_SetUpPeople( const FIELD_CROWD_PEOPLE* cp_sysdata, const FIELD_CROWD_PEOPLE_BOOT_DATA* cp_data, const FIELD_CROWD_PEOPLE_BOOT_POINT* cp_point, FIELD_CROWD_PEOPLE_WK* p_people, u16 add_grid )
{
  u16 gx, gz;
  u16 type = FIELD_CROWD_PEOPLE_TYPE_NORMAL;
  u32 rand = GFUser_GetPublicRand( 0 );
  s32 move_grid;
  
  // 起動グリッド決定
  if( (cp_point->move_dir == DIR_UP) || (cp_point->move_dir == DIR_DOWN) )
  {
    gx = cp_point->left + (rand % ((cp_point->right - cp_point->left)+1));
    if(cp_point->move_dir == DIR_UP){
      gz = cp_point->top - add_grid;
    }else{
      gz = cp_point->top + add_grid;
    }
  }
  else
  {
    if(cp_point->move_dir == DIR_LEFT){
      gx = cp_point->left - add_grid;
    }else{
      gx = cp_point->left + add_grid;
    }
    gz = cp_point->top + (rand % ((cp_point->bottom - cp_point->top)+1));
  }

  // 動作タイプ決定
  // そのグリッド上に話せる人がいなければ、話せる意地悪な人を出す。
  if( FIELD_CROWD_PEOPLE_Search( cp_sysdata, gx, gz, cp_point->move_dir, FIELD_CROWD_PEOPLE_TYPE_NASTY ) == NULL )
  {
    if( (rand % FIELD_CROWD_PEOPLE_PUT_PAR) == 0 )
    {
      type = FIELD_CROWD_PEOPLE_TYPE_NASTY;
    }
  }

  move_grid = cp_point->move_grid - add_grid;
  if( move_grid <= 0 ){
    move_grid = 1;
  }

  // 設定
  FIELD_CROWD_PEOPLE_WK_SetUp( p_people, cp_sysdata->p_fos, cp_sysdata->p_player, type, gx, gz, cp_point->move_dir, cp_point->move_grid - add_grid );
}


//----------------------------------------------------------------------------
/**
 *	@brief  開始人物配置
 *
 *	@param	p_wk        ワーク
 *	@param	p_sysdata   システムデータ
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_BOOT_CONTROL_StartUp( const FIELD_CROWD_PEOPLE_BOOL_CONTROL* cp_wk, FIELD_CROWD_PEOPLE* p_sysdata )
{
  int i, j;
  int total_time;
  int total_people_num;
  int time;

  for( i=0; i<cp_wk->boot_data.point_num; i++ )
  {
    // 全部の位置を歩くために必要な全体タイム
    total_time = cp_wk->boot_data.point[i].move_grid * FIELD_CROWD_PEOPLE_OBJ_WALK_FRAME;

    // waitで割ると、何人分かチェック
    total_people_num = total_time / cp_wk->boot_data.wait;

    // その人数セットアップ
    time = 0;
    for( j=0; j<total_people_num; j++ )
    {
      FIELD_CROWD_PEOPLE_WK* p_people = FIELD_CROWD_PEOPLE_GetClearWk( p_sysdata );

      if( p_people ){
        FIELD_CROWD_PEOPLE_BOOT_CONTROL_SetUpPeople( p_sysdata, &cp_wk->boot_data, &cp_wk->boot_data.point[i], p_people, time / FIELD_CROWD_PEOPLE_OBJ_WALK_FRAME ); 
        time += cp_wk->boot_data.wait;
      }
    }
  }
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
    else
    {
      break;
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
    else
    {
      break;
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
  const MMDL* cp_playermmdl = FIELD_PLAYER_GetMMdl( cp_player );
  
  player_gx = MMDL_GetGridPosX( cp_playermmdl );
  player_gz = MMDL_GetGridPosZ( cp_playermmdl );
  my_gx = MMDL_GetGridPosX( cp_wk->p_mmdl );
  my_gz = MMDL_GetGridPosZ( cp_wk->p_mmdl );
  
  if( FIELD_CROWD_PEOPLE_WK_TOOL_IsGridHit( my_gx, my_gz, player_gx, player_gz, grid, cp_wk->move_dir ) )
  {
    return TRUE;
  }

  player_gx = MMDL_GetOldGridPosX( cp_playermmdl );
  player_gz = MMDL_GetOldGridPosZ( cp_playermmdl );
  
  return FIELD_CROWD_PEOPLE_WK_TOOL_IsGridHit( my_gx, my_gz, player_gx, player_gz, grid, cp_wk->move_dir );
}

//----------------------------------------------------------------------------
/**
 *	@brief  grid歩前にいるのかのグリッド単位のチェック
 *
 *	@param	mygx
 *	@param	mygz
 *	@param	pl_gx
 *	@param	pl_gz
 *	@param	grid 
 *
 *	@retval TRUE  いる
 *	@retval FALSE いない
 */
//-----------------------------------------------------------------------------
static BOOL FIELD_CROWD_PEOPLE_WK_TOOL_IsGridHit( s16 mygx, s16 mygz, s16 pl_gx, s16 pl_gz, u16 grid, u16 move_dir )
{
  s32 grid_dif;
  s32 grid_dif_2;

  switch( move_dir )
  {
  case DIR_UP:
    grid_dif = mygz - pl_gz;
    grid_dif_2 = mygx - pl_gx;
    break;
  case DIR_DOWN:
    grid_dif = pl_gz - mygz;
    grid_dif_2 = mygx - pl_gx;
    break;
  case DIR_LEFT:
    grid_dif = mygx - pl_gx;
    grid_dif_2 = mygz - pl_gz;
    break;
  case DIR_RIGHT:
    grid_dif = pl_gx - mygx;
    grid_dif_2 = mygz - pl_gz;
    break;

  default:
    GF_ASSERT(0);
    break;
  }

  if( ((grid_dif <= grid) && (grid_dif >= 0)) && (grid_dif_2 == 0) )
  {
    return TRUE;
  }
  return FALSE;
}




//----------------------------------------------------------------------------
/**
 *	@brief  騒音システム  初期化
 *
 *	@param	p_wk      ワーク
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_NOISE_Init( FIELD_CROWD_PEOPLE_NOISE* p_wk, FLDMSGBG* p_fmb, const FIELD_CROWD_PEOPLE_BOOL_CONTROL* cp_boot, HEAPID heapID )
{
  int i;
  u32 rand = GFUser_GetPublicRand(0);
  
  p_wk->wait = 0;
  p_wk->wait_max = NOISE_TIME_MIN + ( rand % (cp_boot->boot_data.wait*NOISE_TIME_WAIT_NUM) );
  p_wk->p_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_crowd_people_scr_dat, heapID );
  p_wk->p_fmb = p_fmb;

  p_wk->add_idx = rand % NOISE_WK_MAX;

  for( i=0; i<NOISE_WK_MAX; i++ )
  {
    FIELD_CROWD_PEOPLE_NOISEWK_Init( &p_wk->wk[i], i );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  騒音システム　破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_NOISE_Exit( FIELD_CROWD_PEOPLE_NOISE* p_wk )
{
  int i;
  // 全OFF
  for( i=0; i<NOISE_WK_MAX; i++ )
  {
    FIELD_CROWD_PEOPLE_NOISEWK_Off( &p_wk->wk[i], p_wk->p_fmb );
  }
  p_wk->p_fmb = NULL;
  
  GFL_MSG_Delete( p_wk->p_msgdata );
  p_wk->p_msgdata = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  騒音システム　メイン
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_NOISE_Main( FIELD_CROWD_PEOPLE_NOISE* p_wk, const FIELD_CROWD_PEOPLE_BOOL_CONTROL* cp_boot, const FIELDMAP_WORK* cp_fieldmap )
{
  int i;
  u32 rand = GFUser_GetPublicRand(0);

  if( FIELDMAP_CheckDoEvent( cp_fieldmap ) == FALSE )
  {
    p_wk->wait ++;

    // ウエイトたったら、ランダムメッセージ表示
    if( p_wk->wait >= p_wk->wait_max )
    {
      // 表示
      FIELD_CROWD_PEOPLE_NOISEWK_Start( &p_wk->wk[ p_wk->add_idx ], p_wk->p_fmb, p_wk->p_msgdata );
      p_wk->add_idx = (p_wk->add_idx + 1) % NOISE_WK_MAX;

      // ウエイト初期化
      p_wk->wait = 0;
      p_wk->wait_max = NOISE_TIME_MIN + (rand%(cp_boot->boot_data.wait*NOISE_TIME_WAIT_NUM));
    }

    // メッセージ表示
    for( i=0; i<NOISE_WK_MAX; i++ )
    {
      FIELD_CROWD_PEOPLE_NOISEWK_Main( &p_wk->wk[i], p_wk->p_fmb );
    }
  }
  else
  {
    // 全OFF
    for( i=0; i<NOISE_WK_MAX; i++ )
    {
      FIELD_CROWD_PEOPLE_NOISEWK_Off( &p_wk->wk[i], p_wk->p_fmb );
    }
  }

}


//----------------------------------------------------------------------------
/**
 *	@brief  ワークの初期化
 *
 *	@param	p_wk  ワーク
 *	@param	id    ID
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_NOISEWK_Init( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk, u16 id )
{
  p_wk->flag = FALSE;

  // 各種情報は今決る
  p_wk->x     = sc_NOISE_X[id];
  p_wk->y     = sc_NOISE_Y[id];
  p_wk->id    = id;
  p_wk->wait  = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ開始
 *
 *	@param	p_wk
 *	@param	p_fmb 
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_NOISEWK_Start( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk, FLDMSGBG* p_fmb, GFL_MSGDATA* p_msgdata )
{
  u32 rand = GFUser_GetPublicRand( 0 );
  p_wk->flag = TRUE;
  p_wk->wait = 0;
  
  FLDSUBMSGWIN_Add( p_fmb, p_msgdata, 
      CROWD_NOISE_01 + (rand%NOISE_MSG_MAX), p_wk->id,
      p_wk->x, p_wk->y, NOISE_SIZX, NOISE_SIZY );
}


//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_NOISEWK_Main( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk, FLDMSGBG* p_fmb )
{
  if( p_wk->flag == FALSE )
  {
    return ;
  }
  
  // waitまって消える
  p_wk->wait ++;
  if( p_wk->wait >= NOISE_WK_MAX )
  {
    FLDSUBMSGWIN_Delete( p_fmb, p_wk->id );
    FIELD_CROWD_PEOPLE_NOISEWK_Clear( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワークOFF処理
 */ 
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_NOISEWK_Clear( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk )
{
  p_wk->wait = 0;
  p_wk->flag = FALSE;

  //その他のデータは次も使います
}

//----------------------------------------------------------------------------
/**
 *	@brief  OFFにする
 *
 *	@param	p_wk
 *	@param	p_fmb 
 */
//-----------------------------------------------------------------------------
static void FIELD_CROWD_PEOPLE_NOISEWK_Off( FIELD_CROWD_PEOPLE_NOISE_WK* p_wk, FLDMSGBG* p_fmb )
{
  if( p_wk->flag == FALSE )
  {
    return ;
  }
  
  FLDSUBMSGWIN_Delete( p_fmb, p_wk->id );
  FIELD_CROWD_PEOPLE_NOISEWK_Clear( p_wk );
}




