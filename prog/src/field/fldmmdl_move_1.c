//======================================================================
/**
 *
 * @file  fieldobj_move_1.c
 * @brief  フィールド動作モデル　基本動作系その1　サブ動作
 * @author  kagaya
 * @date  05.07.21
 *
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procmove.h"

//======================================================================
//  define
//======================================================================
//#define DEBUG_IDO_ASSERT

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static void MoveSub_InitProcCall( MMDL * mmdl );
static int MoveSub_StartCheckProcCall( MMDL * mmdl );
static int MoveSub_MoveProcCall( MMDL * mmdl );

static int MoveSub_PosUpdateStartCheck( MMDL * mmdl );
static int MoveSub_PosUpdateEndCheck( MMDL * mmdl );

static void SubMoveNon_Init( MMDL * mmdl );
static int SubMoveNon_StartCheck( MMDL * mmdl );
static int SubMoveNon_Move( MMDL * mmdl );

static void SubMoveKyoro_Init( MMDL * mmdl );
static int SubMoveKyoro_StartCheck( MMDL * mmdl );
static int SubMoveKyoro_Move( MMDL * mmdl );

static void SubMoveSpinStop_Init( MMDL * mmdl );
static int SubMoveSpinStop_StartCheck( MMDL * mmdl );
static int SubMoveSpinStop_Move( MMDL * mmdl );

static void (* const DATA_MMdl_EventTypeInitProcTbl[EV_TYPE_MAX])( MMDL * );
static int (* const DATA_MMdl_EventTypeStartCheckProcTbl[EV_TYPE_MAX])( MMDL * );
static int (* const DATA_MMdl_EventTypeMoveProcTbl[EV_TYPE_MAX])( MMDL * );

//======================================================================
//  サブ動作
//======================================================================
//--------------------------------------------------------------
/**
 * サブ動作初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveSubProcInit( MMDL * mmdl )
{
  MoveSub_InitProcCall( mmdl );
}

//--------------------------------------------------------------
/**
 * サブ動作
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=サブ動作による移動アリ
 */
//--------------------------------------------------------------
int MMDL_MoveSub( MMDL * mmdl )
{
  if( MoveSub_StartCheckProcCall(mmdl) == FALSE ){
    return( FALSE );
  }
  
  if( MoveSub_MoveProcCall(mmdl) == FALSE ){
    return( FALSE );
  }
  
  return( TRUE );
}

//======================================================================
//  サブ動作処理　パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 初期化関数呼び出し
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MoveSub_InitProcCall( MMDL * mmdl )
{
  int type = MMDL_GetEventType( mmdl );
  DATA_MMdl_EventTypeInitProcTbl[type]( mmdl );
}

//--------------------------------------------------------------
/**
 * チェック関数呼び出し
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=チェックに引っかかった
 */
//--------------------------------------------------------------
static int MoveSub_StartCheckProcCall( MMDL * mmdl )
{
  int type = MMDL_GetEventType( mmdl );
  return( DATA_MMdl_EventTypeStartCheckProcTbl[type](mmdl) );
}

//--------------------------------------------------------------
/**
 * 動作関数呼び出し
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=動作中
 */
//--------------------------------------------------------------
static int MoveSub_MoveProcCall( MMDL * mmdl )
{
  int type = MMDL_GetEventType( mmdl );
  return( DATA_MMdl_EventTypeMoveProcTbl[type](mmdl) );
}

//======================================================================
//  サブ動作用パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 座標更新開始をチェック
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=更新
 */
//--------------------------------------------------------------
static int MoveSub_PosUpdateStartCheck( MMDL * mmdl )
{
  int now = MMDL_GetGridPosX( mmdl );
  int old = MMDL_GetOldGridPosX( mmdl );
  
  if( now != old ){
    return( TRUE );
  }
  
  now = MMDL_GetGridPosZ( mmdl );
  old = MMDL_GetOldGridPosZ( mmdl );
  
  if( now != old ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 座標更新終了をチェック
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=終了
 */
//--------------------------------------------------------------
static int MoveSub_PosUpdateEndCheck( MMDL * mmdl )
{
  int now = MMDL_GetGridPosX( mmdl );
  int old = MMDL_GetOldGridPosX( mmdl );
  
  if( now != old ){
    return( FALSE );
  }
  
  now = MMDL_GetGridPosZ( mmdl );
  old = MMDL_GetOldGridPosZ( mmdl );
  
  if( now != old ){
    return( FALSE );
  }
  
  return( TRUE );
}

//======================================================================
//  サブ動作　無し
//======================================================================
//--------------------------------------------------------------
/**
 * サブ動作初期化　無し
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void SubMoveNon_Init( MMDL * mmdl )
{
}

//--------------------------------------------------------------
/**
 * サブ動作スタートチェック　無し
 * @param  mmdl  MMDL *
 * @retval  int    FALSE
 */
//--------------------------------------------------------------
static int SubMoveNon_StartCheck( MMDL * mmdl )
{
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * サブ動作　無し
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=動作中
 */
//--------------------------------------------------------------
static int SubMoveNon_Move( MMDL * mmdl )
{
  return( FALSE );
}

//======================================================================
//  サブ動作　止まってきょろきょろ
//======================================================================
//--------------------------------------------------------------
///  SUBWORK_KYORO構造体
//--------------------------------------------------------------
typedef struct
{
  u8 check_seq_no;
  u8 move_seq_no;
  s8 walk_count;
  s8 max_count;
  
  s8 origin_dir;
  s8 dir_type;
  s8 dir_no;
  s8 dir_count;
  s8 wait;
}SUBWORK_KYORO;

#define SUBWORK_KYORO_SIZE (sizeof(SUBWORK_KYORO))

//--------------------------------------------------------------
///  きょろきょろ向き
//--------------------------------------------------------------
enum
{
  DIR_H_TYPE = 0,    ///<横方向きょろきょろ
  DIR_V_TYPE,      ///<縦方向きょろきょろ
};

#define KYORO_WAIT_FRAME (8)    ///<きょろきょろの合間
#define KYORO_WAIT_FRAME_DASH (4)    ///<きょろきょろの合間 DASH反応版
#define KYORO_COUNT_MAX (4)      ///<きょろきょろ回数最大

//--------------------------------------------------------------
/**
 * 止まってきょろきょろ　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void SubMoveKyoro_Init( MMDL * mmdl )
{
  SUBWORK_KYORO *work;
  
  work = MMDL_InitMoveSubProcWork( mmdl, SUBWORK_KYORO_SIZE );
  work->max_count = MMDL_GetParam( mmdl, MMDL_PARAM_1 );
}

//--------------------------------------------------------------
/**
 * 止まってきょろきょろ　スタートチェック
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=動作アリ
 */
//--------------------------------------------------------------
static int SubMoveKyoro_StartCheck( MMDL * mmdl )
{
  SUBWORK_KYORO *work;
  
  work = MMDL_GetMoveSubProcWork( mmdl );
  
  switch( work->check_seq_no ){
  case 0:                            //移動開始監視
    if( MoveSub_PosUpdateStartCheck(mmdl) == TRUE ){
      work->check_seq_no++;
    }
    
    break;
  case 1:                            //移動終了監視
    if( MoveSub_PosUpdateEndCheck(mmdl) == FALSE ){
      break;
    }
    
    work->walk_count++;
    
    if( work->walk_count < work->max_count ){
      work->check_seq_no = 0;
      break;
    }
    
    work->check_seq_no++;
  case 2:                            //移動完了監視
    if( MMDL_CheckMoveBitMove(mmdl) == TRUE ){
      #ifdef DEBUG_IDO_ASSERT
      GF_ASSERT( MoveSub_PosUpdateStartCheck(mmdl) == TRUE &&
          "SubMoveKyoro_StartCheck()ERROR" );
      #endif
      break;
    }
    
    work->check_seq_no++;
    work->walk_count = 0;
    work->move_seq_no = 0;
  case 3:                            //サブ動作中
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 止まってきょろきょろ　動作
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=動作中
 */
//--------------------------------------------------------------
static int SubMoveKyoro_Move( MMDL * mmdl )
{
  SUBWORK_KYORO *work;
  
  work = MMDL_GetMoveSubProcWork( mmdl );
  
  switch( work->move_seq_no ){
  case 0:                            //初期化
    {
      int type_tbl[4] = {DIR_H_TYPE,DIR_H_TYPE,DIR_V_TYPE,DIR_V_TYPE};
      int dir = MMDL_GetDirDisp( mmdl );
      work->origin_dir = dir;
      work->dir_type = type_tbl[dir];
      work->move_seq_no++;
    }
  case 1:                            //方向セット  
    {
      int dir_tbl[2][2] = { {DIR_LEFT,DIR_RIGHT},{DIR_UP,DIR_DOWN} };
      int dir = dir_tbl[work->dir_type][work->dir_no];
      int code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
      MMDL_SetLocalAcmd( mmdl, code );
      work->move_seq_no++;
    }
  case 2:
    {
      if( MMDL_ActionLocalAcmd(mmdl) == FALSE ){
        return( TRUE );
      }
      
      work->move_seq_no++;
    }
  case 3:
    {
      u8 wait_max = KYORO_WAIT_FRAME;
      u16 ev_type = MMDL_GetEventType( mmdl );
      
      if( ev_type == EV_TYPE_TRAINER_DASH_ACCEL_KYORO ){
        if( MMDL_CheckPlayerDispSizeRect(mmdl) == TRUE ){
          wait_max = KYORO_WAIT_FRAME_DASH;
        }
      }
      
      work->wait++;
      
     
      if( work->wait < wait_max ){
        return( TRUE );
      }
      
      work->wait = 0;
      work->dir_count++;
      
      if( work->dir_count < KYORO_COUNT_MAX ){
        work->dir_no = (work->dir_no + 1) & 0x01;
        work->move_seq_no = 1;
        return( TRUE );
      }
      
      MMDL_SetDirDisp( mmdl, work->origin_dir );
      work->move_seq_no++;
      work->dir_count = 0;
      work->check_seq_no = 0;
    }
  }
  
  return( FALSE );
}

//======================================================================
//  サブ動作　止まってクルクル
//======================================================================
//--------------------------------------------------------------
///  SUBWORK_SPIN_STOP構造体
//--------------------------------------------------------------
typedef struct
{
  u8 check_seq_no;
  u8 move_seq_no;
  s8 walk_count;
  s8 max_count;
  
  s8 origin_dir;
  s8 dir_type;
  s8 dir_no;
  s8 dir_count;
  s8 wait;
}SUBWORK_SPIN_STOP;

#define SUBWORK_SPIN_STOP_SIZE (sizeof(SUBWORK_SPIN_STOP))

//--------------------------------------------------------------
///  回転方向
//--------------------------------------------------------------
enum
{
  SPIN_STOP_L_TYPE = 0,    ///<左回転
  SPIN_STOP_R_TYPE,      ///<右回転
};

#define SPIN_STOP_WAIT_FRAME (8)  ///<回転の間
#define SPIN_STOP_WAIT_FRAME_DASH (4) ///<回転の間 ダッシュ反応版
#define SPIN_STOP_COUNT_MAX (4)    ///<回転最大数

//--------------------------------------------------------------
/**
 * 止まってクルクル　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void SubMoveSpinStop_Init( MMDL * mmdl )
{
  int type;
  SUBWORK_SPIN_STOP *work;
  
  work = MMDL_InitMoveSubProcWork( mmdl, SUBWORK_SPIN_STOP_SIZE );
  work->max_count = MMDL_GetParam( mmdl, MMDL_PARAM_1 );
  
  type = MMDL_GetEventType( mmdl );
  
  if( type == EV_TYPE_TRAINER_SPIN_STOP_L ||
      type == EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_L ){
    type = SPIN_STOP_L_TYPE;
  }else{
    type = SPIN_STOP_R_TYPE;
  }
  
  work->dir_type = type;
}

//--------------------------------------------------------------
/**
 * 止まってクルクル　スタートチェック
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=動作アリ
 */
//--------------------------------------------------------------
static int SubMoveSpinStop_StartCheck( MMDL * mmdl )
{
  SUBWORK_SPIN_STOP *work;
  
  work = MMDL_GetMoveSubProcWork( mmdl );
  
  switch( work->check_seq_no ){
  case 0:                            //移動開始監視
    if( MoveSub_PosUpdateStartCheck(mmdl) == TRUE ){
      work->check_seq_no++;
    }
    
    break;
  case 1:                            //移動終了監視
    if( MoveSub_PosUpdateEndCheck(mmdl) == FALSE ){
      break;
    }
    
    work->walk_count++;
    
    if( work->walk_count < work->max_count ){
      work->check_seq_no = 0;
      break;
    }
    
    work->check_seq_no++;
  case 2:                            //移動完了監視
    if( MMDL_CheckMoveBitMove(mmdl) == TRUE ){
      #ifdef DEBUG_IDO_ASSERT
      GF_ASSERT( MoveSub_PosUpdateStartCheck(mmdl) == TRUE &&
        "SubMoveKyoro_StartCheck()対象の移動方法が異常" );
      #endif
      break;
    }
    
    work->check_seq_no++;
    work->walk_count = 0;
    work->move_seq_no = 0;
  case 3:                            //サブ動作中
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 止まってクルクル　動作
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=動作中
 */
//--------------------------------------------------------------
static int SubMoveSpinStop_Move( MMDL * mmdl )
{
  SUBWORK_KYORO *work;
  int spin_tbl[2][4] =
  { {DIR_UP,DIR_LEFT,DIR_DOWN,DIR_RIGHT},{DIR_UP,DIR_RIGHT,DIR_DOWN,DIR_LEFT} };
  
  work = MMDL_GetMoveSubProcWork( mmdl );
  
  switch( work->move_seq_no ){
  case 0:                            //初期化
    {
      int i,dir = MMDL_GetDirDisp( mmdl );
      
      for( i = 0; (i < DIR_MAX4 && dir != spin_tbl[work->dir_type][i]); i++ ){}
      GF_ASSERT( i < DIR_MAX4 && "SubMoveKyoro_Move()方向異常" );
      
      work->origin_dir = dir;
      work->dir_no = (i + 1) % SPIN_STOP_COUNT_MAX;    //+1=1つ先
      work->move_seq_no++;
    }
  case 1:                            //方向セット  
    {
      int dir = spin_tbl[work->dir_type][work->dir_no];
      int code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
      MMDL_SetLocalAcmd( mmdl, code );
      work->move_seq_no++;
    }
  case 2:
    {
      if( MMDL_ActionLocalAcmd(mmdl) == FALSE ){
        return( TRUE );
      }
      
      work->move_seq_no++;
    }
  case 3:
    {
      u8 wait_max = SPIN_STOP_WAIT_FRAME;
      u16 ev_type = MMDL_GetEventType( mmdl );
      
      if( ev_type == EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_L ||
          ev_type == EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_R ){
        if( MMDL_CheckPlayerDispSizeRect(mmdl) == TRUE ){
          wait_max = SPIN_STOP_WAIT_FRAME_DASH;
        }
      }
      
      work->wait++;
      
      if( work->wait < wait_max ){
        return( TRUE );
      }
      
      work->wait = 0;
      work->dir_count++;
      
      if( work->dir_count < SPIN_STOP_COUNT_MAX ){
        work->dir_no = (work->dir_no + 1) % SPIN_STOP_COUNT_MAX;
        work->move_seq_no = 1;
        return( TRUE );
      }
      
      MMDL_SetDirDisp( mmdl, work->origin_dir );
      work->move_seq_no++;
      work->dir_count = 0;
      work->check_seq_no = 0;
    }
  }
  
  return( FALSE );
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
//  イベントタイプ別初期化関数 EV_TYPE_NORMAL等の値に一致
//--------------------------------------------------------------
static void (* const DATA_MMdl_EventTypeInitProcTbl[EV_TYPE_MAX])( MMDL * ) =
{
  SubMoveNon_Init, //EV_TYPE_NORMAL
  SubMoveNon_Init, //EV_TYPE_TRAINER
  SubMoveNon_Init, //EV_TYPE_TRAINER_EYEALL
  SubMoveNon_Init, //EV_TYPE_ITEM
  SubMoveKyoro_Init, //EV_TYPE_TRAINER_KYORO
  SubMoveSpinStop_Init, //EV_TYPE_TRAINER_SPIN_STOP_L
  SubMoveSpinStop_Init, //EV_TYPE_TRAINER_SPIN_STOP_R
  SubMoveNon_Init, //EV_TYPE_TRAINER_SPIN_MOVE_L
  SubMoveNon_Init, //EV_TYPE_TRAINER_SPIN_MOVE_R
  SubMoveNon_Init, //EV_TYPE_MSG
  SubMoveNon_Init, //EV_TYPE_TRAINER_ESCAPE
  SubMoveNon_Init, //EV_TYPE_TRAINER_DASH_ALTER
  SubMoveNon_Init, //EV_TYPE_TRAINER_DASH_REACT
  SubMoveNon_Init, //EV_TYPE_TRAINER_DASH_ACCEL
  SubMoveKyoro_Init, //EV_TYPE_TRAINER_DASH_ACCEL_KYORO
  SubMoveSpinStop_Init, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_L
  SubMoveSpinStop_Init, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_R
  SubMoveNon_Init, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_L
  SubMoveNon_Init, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_R
};

//--------------------------------------------------------------
//  イベントタイプ別スタートチェック関数　EV_TYPE_NORMAL等の値に一致
//--------------------------------------------------------------
static int (* const DATA_MMdl_EventTypeStartCheckProcTbl[EV_TYPE_MAX])( MMDL * ) =
{
  SubMoveNon_StartCheck, //EV_TYPE_NORMAL
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER_EYEALL
  SubMoveNon_StartCheck, //EV_TYPE_ITEM
  SubMoveKyoro_StartCheck, //EV_TYPE_TRAINER_KYORO
  SubMoveSpinStop_StartCheck, //EV_TYPE_TRAINER_SPIN_STOP_L
  SubMoveSpinStop_StartCheck, //EV_TYPE_TRAINER_SPIN_STOP_R
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER_SPIN_MOVE_L
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER_SPIN_MOVE_R
  SubMoveNon_StartCheck, //EV_TYPE_MSG
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER_ESCAPE
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER_DASH_ALTER
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER_DASH_REACT
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER_DASH_ACCEL
  SubMoveKyoro_StartCheck, //EV_TYPE_TRAINER_DASH_ACCEL_KYORO
  SubMoveSpinStop_StartCheck, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_L
  SubMoveSpinStop_StartCheck, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_R
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_L
  SubMoveNon_StartCheck, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_R
};

//--------------------------------------------------------------
//  イベントタイプ別動作関数 EV_TYPE_NORMAL等の値に一致
//--------------------------------------------------------------
static int (* const DATA_MMdl_EventTypeMoveProcTbl[EV_TYPE_MAX])( MMDL * ) =
{
  SubMoveNon_Move, //EV_TYPE_NORMAL
  SubMoveNon_Move, //EV_TYPE_TRAINER
  SubMoveNon_Move, //EV_TYPE_TRAINER_EYEALL
  SubMoveNon_Move, //EV_TYPE_ITEM
  SubMoveKyoro_Move, //EV_TYPE_TRAINER_KYORO
  SubMoveSpinStop_Move, //EV_TYPE_TRAINER_SPIN_STOP_L
  SubMoveSpinStop_Move, //EV_TYPE_TRAINER_SPIN_STOP_R
  SubMoveNon_Move, //EV_TYPE_TRAINER_SPIN_MOVE_L
  SubMoveNon_Move, //EV_TYPE_TRAINER_SPIN_MOVE_R
  SubMoveNon_Move, //EV_TYPE_MSG
  SubMoveNon_Move, //EV_TYPE_TRAINER_ESCAPE
  SubMoveNon_Move, //EV_TYPE_TRAINER_DASH_ALTER
  SubMoveNon_Move, //EV_TYPE_TRAINER_DASH_REACT
  SubMoveNon_Move, //EV_TYPE_TRAINER_DASH_ACCEL
  SubMoveKyoro_Move, //EV_TYPE_TRAINER_DASH_ACCEL_KYORO
  SubMoveSpinStop_Move, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_L
  SubMoveSpinStop_Move, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_R
  SubMoveNon_Move, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_L
  SubMoveNon_Move, //EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_R
};
