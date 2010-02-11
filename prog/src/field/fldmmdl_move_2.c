//======================================================================
/**
 *
 * @file  fieldobj_move_2.c
 * @brief  フィールド動作モデル　基本動作系その2
 * @author  kagaya
 * @date  05.07.21
 *
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procmove.h"

#include "field_player.h"   //event_trainer_eye.h
#include "field_encount.h"  //event_trainer_eye.h
#include "event_trainer_eye.h"

//======================================================================
//  define
//======================================================================
///トレーナーペア　親移動停止
#define PAIR_TR_OYA_MOVEBIT_STOP \
  (MMDL_MOVEBIT_ATTR_GET_ERROR    | \
   MMDL_MOVEBIT_HEIGHT_GET_ERROR  | \
   MMDL_MOVEBIT_PAUSE_MOVE)

//--------------------------------------------------------------
///  コピー動作番号
//--------------------------------------------------------------
enum
{
  SEQNO_COPYMOVE_FIRST_INIT,
  SEQNO_COPYMOVE_FIRST_WAIT,
  SEQNO_COPYMOVE_INIT,
  SEQNO_COPYMOVE_CMD_SET,
  SEQNO_COPYMOVE_CMD_WAIT,
  SEQNO_COPYMOVE_MAX,
};

//--------------------------------------------------------------
///  壁移動　利き手
//--------------------------------------------------------------
typedef enum
{
  ALONG_L = 0,    ///<左
  ALONG_R,      ///<右
  ALONG_LR,      ///<左右
}ALONG_DIR;

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
//  MV_PAIR_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  u8 seq_no;        ///<動作番号
  u8 jiki_init;      ///<自機情報初期化完了
  s16 jiki_gx;      ///<自機グリッド座標X
  s16 jiki_gz;      ///<自機グリッド座標Z
  u16 jiki_ac;      ///<自機アニメーションコード
}MV_PAIR_WORK;

#define MV_PAIR_WORK_SIZE (sizeof(MV_PAIR_WORK))

//--------------------------------------------------------------
//  MV_TR_PAIR_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  u8 seq_no;        ///<動作番号
  u8 oya_init;      ///<親情報初期化完了
  s16 oya_gx;        ///<親グリッド座標X
  s16 oya_gz;        ///<親グリッド座標Z
  u16 oya_ac;        ///<自機アニメーションコード
  MMDL * oyaobj;    ///<親となるOBJ
}MV_TR_PAIR_WORK;

#define MV_TR_PAIR_WORK_SIZE (sizeof(MV_TR_PAIR_WORK))

//--------------------------------------------------------------
///  MV_HIDE_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  u8 seq_no;
  u8 hide_type;
  u8 pulloff_flag;
  u8 dmy;
  #ifndef MMDL_PL_NULL
  EOA_PTR eoa_hide;
  #endif
}MV_HIDE_WORK;

#define MV_HIDE_WORK_SIZE (sizeof(MV_HIDE_WORK))

//--------------------------------------------------------------
///  MV_COPY_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  u8 seq_no;
  s8 dir_jiki;
  u8 lgrass_on;
  u8 dmy;
}MV_COPY_WORK;

#define MV_COPY_WORK_SIZE (sizeof(MV_COPY_WORK))

//--------------------------------------------------------------
///  MV_ALONGW_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  u32 seq_no;
  ALONG_DIR dir_hand_init;
  ALONG_DIR dir_hand;
}MV_ALONGW_WORK;

#define MV_ALONGW_WORK_SIZE (sizeof(MV_ALONGW_WORK))

//======================================================================
//  プロトタイプ
//======================================================================
int (* const DATA_PairMoveTbl[])( MMDL * mmdl, MV_PAIR_WORK *work );

static int Pair_WorkSetJikiSearch( MMDL * mmdl, MV_PAIR_WORK *work );
static void Pair_WorkInit( MMDL * mmdl, MV_PAIR_WORK *work );
static int Pair_JikiPosUpdateCheck( MMDL * mmdl, MV_PAIR_WORK *work );
static void Pair_JikiPosSet( MMDL * mmdl, MV_PAIR_WORK *work );
static u32 Pair_JikiAcmdCodeGet( MMDL * mmdl );
static int Pair_JikiCheckAcmdSet( MMDL * mmdl );

int (* const DATA_PairTrMoveTbl[])( MMDL * mmdl, MV_TR_PAIR_WORK *work );

static int PairTr_WorkSetOyaSearch( MMDL * mmdl, MV_TR_PAIR_WORK *work );
static void PairTr_WorkInit( MMDL * mmdl, MV_TR_PAIR_WORK *work, MMDL * oyaobj);
static int PairTr_OyaPosUpdateCheck( MMDL * mmdl, MV_TR_PAIR_WORK *work );
static void PairTr_OyaPosSet( MMDL * mmdl, MV_TR_PAIR_WORK *work );
static int PairTr_OyaCheckAcmdSet( MMDL * mmdl, MV_TR_PAIR_WORK *work );

int (* const DATA_HideMoveTbl[])( MMDL * mmdl, MV_HIDE_WORK *work );

//======================================================================
//  MV_PAIR  自機連れ歩き
//======================================================================
//--------------------------------------------------------------
/**
 * MV_PAIR　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MovePair_Init( MMDL * mmdl )
{
  MV_PAIR_WORK *work = MMDL_InitMoveProcWork( mmdl, MV_PAIR_WORK_SIZE );
  Pair_WorkSetJikiSearch( mmdl, work );
  MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
  MMDL_OffMoveBitMove( mmdl );
  MMDL_SetStatusBitFellowHit( mmdl, FALSE );
}

//--------------------------------------------------------------
/**
 * MV_PAIR　動作
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MovePair_Move( MMDL * mmdl )
{
  MV_PAIR_WORK *work = MMDL_GetMoveProcWork( mmdl );
  
  if( Pair_WorkSetJikiSearch(mmdl,work) == FALSE ){
    return;
  }
  
  MMDL_SetStatusBitFellowHit( mmdl, FALSE );
  
  while( DATA_PairMoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * MV_PAIR 削除
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MovePair_Delete( MMDL * mmdl )
{
}

//======================================================================
//  MV_PAIR 動作
//======================================================================
//--------------------------------------------------------------
/**
 * Pair 0
 * @param  mmdl  MMDL *
 * @param  work  MV_PAIR_WORK
 * @retval  int    TRUE=再帰要求
 */
//--------------------------------------------------------------
static int PairMove_Init( MMDL * mmdl, MV_PAIR_WORK *work )
{
  MMDL_OffMoveBitMove( mmdl );
  MMDL_OffMoveBitMoveEnd( mmdl );
    
  if( Pair_JikiPosUpdateCheck(mmdl,work) == TRUE ){
    Pair_JikiPosSet( mmdl, work );
    
    if( Pair_JikiCheckAcmdSet(mmdl) == TRUE ){
      MMDL_OnMoveBitMove( mmdl );
      work->seq_no++;
      return( TRUE );
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * Pair 1
 * @param  mmdl  MMDL *
 * @param  work  MV_PAIR_WORK
 * @retval  int    TRUE=再帰要求
 */
//--------------------------------------------------------------
static int PairMove_Move( MMDL * mmdl, MV_PAIR_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == TRUE ){
    MMDL_OffMoveBitMove( mmdl );
    work->seq_no = 0;
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
///  つれあるき動作テーブル
//--------------------------------------------------------------
static int (* const DATA_PairMoveTbl[])( MMDL * mmdl, MV_PAIR_WORK *work ) =
{
  PairMove_Init,
  PairMove_Move,
};

//======================================================================
//  MV_PAIR　パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 自機が存在するかチェック　存在するのであれば情報初期化 
 * @param  mmdl  MMDL *
 * @param  work  MV_PAIR_WORK
 * @retval  int    FALSE=存在していない
 */
//--------------------------------------------------------------
static int Pair_WorkSetJikiSearch( MMDL * mmdl, MV_PAIR_WORK *work )
{
  MMDL *jiki = MMDLSYS_SearchMMdlPlayer( MMDL_GetMMdlSys(mmdl) );
  
  if( jiki == NULL ){
    work->jiki_init = FALSE;
    return( FALSE );
  }
  
  if( work->jiki_init == FALSE ){
    Pair_WorkInit( mmdl, work );
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_PAIR_WORK初期化　自機がいる事前提
 * @param  mmdl  MMDL *  
 * @param  work  MV_PAIR_WORK
 * @retval  nothing
 */
//--------------------------------------------------------------
static void Pair_WorkInit( MMDL * mmdl, MV_PAIR_WORK *work )
{
  MMDL *jiki = MMDLSYS_SearchMMdlPlayer( MMDL_GetMMdlSys(mmdl) );
  work->jiki_init = TRUE;
  work->jiki_gx = MMDL_GetGridPosX( jiki );
  work->jiki_gz = MMDL_GetGridPosZ( jiki );
  work->jiki_ac = ACMD_NOT;
}

//--------------------------------------------------------------
/**
 * 自機座標更新チェック
 * @param  mmdl  MMDL *
 * @param  work  MV_PAIR_WORK
 * @retval  int    TRUE=更新した
 */
static int Pair_JikiPosUpdateCheck( MMDL * mmdl, MV_PAIR_WORK *work )
{
  MMDL *jiki = MMDLSYS_SearchMMdlPlayer( MMDL_GetMMdlSys(mmdl) );
  
  if( jiki != NULL ){
    int gx = MMDL_GetGridPosX( jiki );
    int gz = MMDL_GetGridPosZ( jiki );
    
    if( gx != work->jiki_gx || gz != work->jiki_gz ){
      return( TRUE );
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機座標セット
 * @param  mmdl  MMDL *
 * @param  work  MV_PAIR_WORK
 * @retval  nothing
 */
//--------------------------------------------------------------
static void Pair_JikiPosSet( MMDL * mmdl, MV_PAIR_WORK *work )
{
  MMDL *jiki = MMDLSYS_SearchMMdlPlayer( MMDL_GetMMdlSys(mmdl) );
  work->jiki_gx = MMDL_GetGridPosX( jiki );
  work->jiki_gz = MMDL_GetGridPosZ( jiki );
}

//--------------------------------------------------------------
/**
 * 自機アニメコード取得
 * @param  mmdl  MMDL *
 * @retval  u32    アニメーションコード
 */
//--------------------------------------------------------------
static u32 Pair_JikiAcmdCodeGet( MMDL * mmdl )
{
  u16 code;
  MMDL *jiki = MMDLSYS_SearchMMdlPlayer( MMDL_GetMMdlSys(mmdl) );
  code = MMDL_GetAcmdCode( mmdl );
  
  switch( code ){
  case AC_DASH_U_4F: code = AC_WALK_U_4F; break;
  case AC_DASH_D_4F: code = AC_WALK_D_4F; break;
  case AC_DASH_L_4F: code = AC_WALK_L_4F; break;
  case AC_DASH_R_4F: code = AC_WALK_R_4F; break;
  }
  
  return( code );
}

//--------------------------------------------------------------
/**
 * 自機アニメコード、方向を取得しアニメーションコマンドをセット
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=セットできた FALSE=重なりによりセットできない
 */
//--------------------------------------------------------------
static int Pair_JikiCheckAcmdSet( MMDL * mmdl )
{
  MMDL *jiki = MMDLSYS_SearchMMdlPlayer( MMDL_GetMMdlSys(mmdl) );
  int gx = MMDL_GetGridPosX( mmdl );
  int gz = MMDL_GetGridPosZ( mmdl );
  int jx = MMDL_GetOldGridPosX( jiki );
  int jz = MMDL_GetOldGridPosZ( jiki );
  
  if( gx != jx || gz != jz ){
    u32 code = Pair_JikiAcmdCodeGet( mmdl );
    int dir = MMDL_TOOL_GetRangeDir( gx, gz, jx, jz );
    code = MMDL_ChangeDirAcmdCode( dir, code );
    MMDL_SetLocalAcmd( mmdl, code );
    return( TRUE );
  }
  return( FALSE );
}

//======================================================================
//  MV_TR_PAIR トレーナー連れ歩き
//======================================================================
//--------------------------------------------------------------
/**
 * MV_TR_PAIR　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MovePairTr_Init( MMDL * mmdl )
{
  MV_TR_PAIR_WORK *work = MMDL_InitMoveProcWork( mmdl, MV_TR_PAIR_WORK_SIZE );
  PairTr_WorkSetOyaSearch( mmdl, work );
  MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
  MMDL_OffMoveBitMove( mmdl );
  
#if 0 //当り判定の無効化は無し。
  MMDL_SetStatusBitFellowHit( mmdl, FALSE );
#endif

  work->oya_init = FALSE;
}

//--------------------------------------------------------------
/**
 * MV_TR_PAIR　動作
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MovePairTr_Move( MMDL * mmdl )
{
  MV_TR_PAIR_WORK *work = MMDL_GetMoveProcWork( mmdl );
  
  if( PairTr_WorkSetOyaSearch(mmdl,work) == FALSE ){
    return;
  }

#if 0 //当り判定の無効化は無し。
  MMDL_SetStatusBitFellowHit( mmdl, FALSE );
#endif

  while( DATA_PairTrMoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * MV_TR_PAIR 削除
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MovePairTr_Delete( MMDL * mmdl )
{
}

//--------------------------------------------------------------
/**
 * MV_TR_PAIR　復帰
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MovePairTr_Return( MMDL * mmdl )
{
  MV_TR_PAIR_WORK *work = MMDL_GetMoveProcWork( mmdl );
  work->oya_init = 0;
}

//======================================================================
//  MV_TR_PAIR 動作
//======================================================================
//--------------------------------------------------------------
/**
 * Pair 0
 * @param  mmdl  MMDL *
 * @param  work  MV_TR_PAIR_WORK
 * @retval  int    TRUE=再帰要求
 */
//--------------------------------------------------------------
static int PairTrMove_Init( MMDL * mmdl, MV_TR_PAIR_WORK *work )
{
  MMDL_OffMoveBitMove( mmdl );
  MMDL_OffMoveBitMoveEnd( mmdl );
  
  if( PairTr_OyaPosUpdateCheck(mmdl,work) == TRUE ){
    if( PairTr_OyaCheckAcmdSet(mmdl,work) == TRUE ){
      MMDL_OnMoveBitMove( mmdl );
      work->seq_no++;
      return( TRUE );
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * Pair 1
 * @param  mmdl  MMDL *
 * @param  work  MV_PAIR_WORK
 * @retval  int    TRUE=再帰要求
 */
//--------------------------------------------------------------
static int PairTrMove_Move( MMDL * mmdl, MV_TR_PAIR_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == FALSE ){
    return( FALSE );
  }
  
  MMDL_OffMoveBitMove( mmdl );
  work->seq_no = 0;
  return( FALSE );
}

//--------------------------------------------------------------
///  つれあるき動作テーブル
//--------------------------------------------------------------
static int (* const DATA_PairTrMoveTbl[])( MMDL * mmdl, MV_TR_PAIR_WORK *work ) =
{
  PairTrMove_Init,
  PairTrMove_Move,
};

//======================================================================
//  MV_TR_PAIR　パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 対象がペア動作かどうか。ペア動作であれば相方を探す
 * @param  mmdl  MMDL *
 * @retval  MMDL *　相方のMMDL *、NULL=ペア動作ではない
 */
//--------------------------------------------------------------
MMDL * MMDL_MovePairSearch( MMDL * mmdl )
{
  MMDL * pair;
  u32 no = 0;
  int type = MMDL_GetEventType( mmdl );
  int zone_id = MMDL_GetZoneID( mmdl );
  const MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
  u32 trid = EVENT_GetTrainerEyeTrainerID( mmdl );
  
  switch( type ){
  case EV_TYPE_TRAINER:
  case EV_TYPE_TRAINER_EYEALL:
  case EV_TYPE_ITEM:
  case EV_TYPE_TRAINER_KYORO:
  case EV_TYPE_TRAINER_SPIN_STOP_L:
  case EV_TYPE_TRAINER_SPIN_STOP_R:
  case EV_TYPE_TRAINER_SPIN_MOVE_L:
  case EV_TYPE_TRAINER_SPIN_MOVE_R:
    while( MMDLSYS_SearchUseMMdl(mmdlsys,&pair,&no) == TRUE )
    {
      if( mmdl != pair && MMDL_GetZoneID(pair) == zone_id )
      {
        if( EVENT_GetTrainerEyeTrainerID(pair) == trid )
        {
          return( pair );
        }
      }
    }
  }
  return( NULL );
}

//--------------------------------------------------------------
/**
 * 親が存在するかチェック　存在するのであれば情報初期化 
 * @param  mmdl  MMDL *
 * @param  work  MV_TR_PAIR_WORK
 * @retval  int    FALSE=存在していない
 */
//--------------------------------------------------------------
static int PairTr_WorkSetOyaSearch( MMDL * mmdl, MV_TR_PAIR_WORK *work )
{
  int zone;
  u32 trid,no;
  MMDL *oyaobj;
  const MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
  
  no = 0;
  zone = MMDL_GetZoneID( mmdl );
  trid = EVENT_GetTrainerEyeTrainerID( mmdl );
  
  while( MMDLSYS_SearchUseMMdl(mmdlsys,&oyaobj,&no) == TRUE ){
    if( mmdl != oyaobj && MMDL_GetZoneID(oyaobj) == zone &&
      EVENT_GetTrainerEyeTrainerID(oyaobj) == trid ){
      
      if( work->oya_init == FALSE ){
        PairTr_WorkInit( mmdl, work, oyaobj );
      }
      
      return( TRUE );
    }
  }
  
  work->oya_init = FALSE;
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * MV_TR_PAIR_WORK初期化　親がいる事前提
 * @param  mmdl  MMDL *  
 * @param  work  MV_TR_PAIR_WORK
 * @retval  nothing
 */
//--------------------------------------------------------------
static void PairTr_WorkInit( MMDL * mmdl, MV_TR_PAIR_WORK *work, MMDL * oyaobj )
{
  work->oya_init = TRUE;
  work->oya_gx = MMDL_GetGridPosX( oyaobj );
  work->oya_gz = MMDL_GetGridPosZ( oyaobj );
  work->oya_ac = ACMD_NOT;
  work->oyaobj = oyaobj;
}

//--------------------------------------------------------------
/**
 * 親座標更新チェック
 * @param  mmdl  MMDL *
 * @param  work  MV_PAIR_WORK
 * @retval  int    TRUE=更新した
 */
//--------------------------------------------------------------
static int PairTr_OyaPosUpdateCheck( MMDL * mmdl, MV_TR_PAIR_WORK *work )
{
  MMDL * oyaobj = work->oyaobj;
  int gx = MMDL_GetGridPosX( mmdl );
  int gz = MMDL_GetGridPosZ( mmdl );
  int ngx = MMDL_GetOldGridPosX( oyaobj );
  int ngz = MMDL_GetOldGridPosZ( oyaobj );
  
  if( (gx != ngx || gz != ngz) &&
    (MMDL_CheckMoveBitMove(oyaobj) == TRUE ||
    MMDL_CheckMoveBit(oyaobj,PAIR_TR_OYA_MOVEBIT_STOP) == 0) ){
    return( TRUE );
  }
  
  return( FALSE );
}

#if 0  //相方の高さを考慮していない
static int PairTr_OyaPosUpdateCheck( MMDL * mmdl, MV_TR_PAIR_WORK *work )
{
  MMDL * oyaobj = work->oyaobj;
  int gx = MMDL_GetGridPosX( mmdl );
  int gz = MMDL_GetGridPosZ( mmdl );
  int ngx = MMDL_GetOldGridPosX( oyaobj );
  int ngz = MMDL_GetOldGridPosZ( oyaobj );
  
  if( (gx != ngx || gz != ngz) &&
    MMDL_CheckStatusBit(oyaobj,MMDL_STABIT_ATTR_GET_ERROR) == 0 &&
    MMDL_CheckStatusBit(oyaobj,MMDL_STABIT_PAUSE_MOVE) == 0 ){
    return( TRUE );
  }
  
  return( FALSE );
}
#endif

#if 0
static int PairTr_OyaPosUpdateCheck( MMDL * mmdl, MV_TR_PAIR_WORK *work )
{
  MMDL * oyaobj = work->oyaobj;
  int gx = MMDL_GetGridPosX( oyaobj );
  int gz = MMDL_GetGridPosZ( oyaobj );
  
  if( (gx != work->oya_gx || gz != work->oya_gz) &&
    MMDL_CheckStatusBit(oyaobj,MMDL_STABIT_ATTR_GET_ERROR) == 0 &&
    MMDL_CheckStatusBit(oyaobj,MMDL_STABIT_PAUSE_MOVE) == 0 ){
    return( TRUE );
  }
  
  return( FALSE );
}
#endif

//--------------------------------------------------------------
/**
 * 親座標セット
 * @param  mmdl  MMDL *
 * @param  work  MV_PAIR_WORK
 * @retval  nothing
 */
//--------------------------------------------------------------
static void PairTr_OyaPosSet( MMDL * mmdl, MV_TR_PAIR_WORK *work )
{
  work->oya_gx = MMDL_GetGridPosX( work->oyaobj );
  work->oya_gz = MMDL_GetGridPosZ( work->oyaobj );
}

//--------------------------------------------------------------
/**
 * 親アニメコード、方向を取得しアニメーションコマンドをセット
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=セットできた FALSE=重なりによりセットできない
 */
//--------------------------------------------------------------
static int PairTr_OyaCheckAcmdSet( MMDL * mmdl, MV_TR_PAIR_WORK *work )
{
  int gx = MMDL_GetGridPosX( mmdl );
  int gz = MMDL_GetGridPosZ( mmdl );
  int ngx = MMDL_GetGridPosX( work->oyaobj );
  int ngz = MMDL_GetGridPosZ( work->oyaobj );
  int ogx = MMDL_GetOldGridPosX( work->oyaobj );
  int ogz = MMDL_GetOldGridPosZ( work->oyaobj );
  int dir;
  
  if( gx == ngx && gz == ngz ){
    return( FALSE );
  }
  
  dir = MMDL_TOOL_GetRangeDir( gx, gz, ogx, ogz );
  gx += MMDL_TOOL_GetDirAddValueGridX( dir );
  gz += MMDL_TOOL_GetDirAddValueGridZ( dir );
    
  if( gx != ngx || gz != ngz ){
    u32 code = AC_WALK_U_8F;
    code = MMDL_ChangeDirAcmdCode( dir, code );
    MMDL_SetLocalAcmd( mmdl, code );
    return( TRUE );
  }
  
  return( FALSE );
}

#if 0
static int PairTr_OyaCheckAcmdSet( MMDL * mmdl, MV_TR_PAIR_WORK *work )
{
  int gx = MMDL_GetGridPosX( mmdl );
  int gz = MMDL_GetGridPosZ( mmdl );
  int ngx = MMDL_GetGridPosX( work->oyaobj );
  int ngz = MMDL_GetGridPosZ( work->oyaobj );
  int ogx = MMDL_GetOldGridPosX( work->oyaobj );
  int ogz = MMDL_GetOldGridPosZ( work->oyaobj );
  int sx,sz;
  
  if( gx == ngx && gz == ngz ){
    return( FALSE );
  }
  
  sx = gx - ogx;
  if( sx < 0 ){ sx = -sx; }
  sz = gz - ogz;
  if( sz < 0 ){ sz = -sz; }
  
  if( sx || sz ){
    int dir = MMDL_TOOL_GetRangeDir( gx, gz, ogx, ogz );
    gx += MMDL_TOOL_GetDirAddValueGridX( dir );
    gz += MMDL_TOOL_GetDirAddValueGridZ( dir );
    
    if( gx != ngx && gz != ngz ){
      u32 code = AC_WALK_U_8F;
      code = MMDL_ChangeDirAcmdCode( dir, code );
      MMDL_SetLocalAcmd( mmdl, code );
      return( TRUE );
    }
  }
  
  return( FALSE );
}
#endif

//======================================================================
//  MV_HIDE_SNOW　隠れ蓑　雪
//======================================================================
//--------------------------------------------------------------
/**
 * MV_HIDE 初期化
 * @param  mmdl  MMDL *
 * @param  type  HIDETYPE
 * @retval  nothing
 */
//--------------------------------------------------------------
#ifndef MMDL_PL_NULL
static void MMdl_MoveHide_Init( MMDL * mmdl, HIDETYPE type )
{
  MV_HIDE_WORK *work = MMDL_InitMoveProcWork( mmdl, MV_HIDE_WORK_SIZE );
  work->hide_type = type;
  MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
  MMDL_OffMoveBitMove( mmdl );
  MMDL_OnStatusBit( mmdl, MMDL_STABIT_SHADOW_VANISH );
  
  {                              //高さ落とす
    VecFx32 offs = { 0, NUM_FX32(-32), 0 };
    MMDL_SetVectorDrawOffsetPos( mmdl, &offs );
  }
}
#endif

//--------------------------------------------------------------
/**
 * MV_HIDE_SNOW 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveHideSnow_Init( MMDL * mmdl )
{
#ifndef MMDL_PL_NULL
  MMdl_MoveHide_Init( mmdl, HIDE_SNOW );
#endif
}

//--------------------------------------------------------------
/**
 * MV_HIDE_SAND 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveHideSand_Init( MMDL * mmdl )
{
#ifndef MMDL_PL_NULL
  MMdl_MoveHide_Init( mmdl, HIDE_SAND );
#endif
}

//--------------------------------------------------------------
/**
 * MV_HIDE_GRND 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveHideGround_Init( MMDL * mmdl )
{
#ifndef MMDL_PL_NULL
  MMdl_MoveHide_Init( mmdl, HIDE_GROUND );
#endif
}

//--------------------------------------------------------------
/**
 * MV_HIDE_KUSA 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveHideKusa_Init( MMDL * mmdl )
{
#ifndef MMDL_PL_NULL
  MMdl_MoveHide_Init( mmdl, HIDE_GRASS );
#endif
}

//--------------------------------------------------------------
/**
 * MV_HIDE 動作
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveHide_Move( MMDL * mmdl )
{
  MV_HIDE_WORK *work = MMDL_GetMoveProcWork( mmdl );
  while( DATA_HideMoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * MV_HIDE 削除
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveHide_Delete( MMDL * mmdl )
{
#ifndef MMDL_PL_NULL
  EOA_PTR eoa = MMDL_MoveHideEoaPtrGet( mmdl );
  if( eoa != NULL ){ FE_EoaDelete( eoa ); }
#endif
}

//--------------------------------------------------------------
/**
 * MV_HIDE 復帰
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveHide_Return( MMDL * mmdl )
{
#ifndef MMDL_PL_NULL
  MV_HIDE_WORK *work = MMDL_GetMoveProcWork( mmdl );
  
  work->seq_no = 0;
  
  MMDL_MoveHideEoaPtrSet( mmdl, NULL );
  
  if( work->pulloff_flag == FALSE ){
    VecFx32 offs = { 0, NUM_FX32(-32), 0 };
    MMDL_SetVectorDrawOffsetPos( mmdl, &offs );
    //add pl 
    MMDL_OnStatusBit( mmdl, MMDL_STABIT_SHADOW_VANISH );
  }
#endif
}

//======================================================================
//  MV_HIDE 動作
//======================================================================
//--------------------------------------------------------------
/**
 * hide 0
 * @param  mmdl  MMDL *
 * @param  work  MV_HIDE_WORK
 * @retval  int    TRUE=再帰要求
 */
//--------------------------------------------------------------
static int HideMove_Init( MMDL * mmdl, MV_HIDE_WORK *work )
{
#ifndef MMDL_PL_NULL
  if( work->pulloff_flag == FALSE ){
    EOA_PTR eoa = FE_mmdlHide_Add( mmdl, work->hide_type );
    MMDL_MoveHideEoaPtrSet( mmdl, eoa );
  }
  
  MMDL_OffMoveBitMove( mmdl );
  MMDL_OffMoveBitMoveEnd( mmdl );
  
  work->seq_no++;
#endif
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * hide 1
 * @param  mmdl  MMDL *
 * @param  work  MV_HIDE_WORK
 * @retval  int    TRUE=再帰要求
 */
//--------------------------------------------------------------
static int HideMove_Move( MMDL * mmdl, MV_HIDE_WORK *work )
{
#ifndef MMDL_PL_NULL
  if( work->pulloff_flag == FALSE ){
    EOA_PTR eoa = MMDL_MoveHideEoaPtrGet( mmdl );
  
    if( eoa == NULL ){ 
      if( MMDL_CheckCompletedDrawInit(mmdl) == TRUE ){
        eoa = FE_mmdlHide_Add( mmdl, work->hide_type );
        MMDL_MoveHideEoaPtrSet( mmdl, eoa );
      }
    }
    
    //add pl 常に影を消す
    MMDL_OnStatusBit( mmdl, MMDL_STABIT_SHADOW_VANISH );
  }
#endif
  return( FALSE );
}

//--------------------------------------------------------------
///  MV_HIDE 動作テーブル
//--------------------------------------------------------------
static int (* const DATA_HideMoveTbl[])( MMDL * mmdl, MV_HIDE_WORK *work ) =
{
  HideMove_Init,
  HideMove_Move,
};

//======================================================================
//  MV_HIDE パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * MV_HIDE 隠れ蓑EOA_PTRセット
 * @param  mmdl  MMDL *
 * @param  eoa    EOA_PTR
 * @retval  nothing
 */
//--------------------------------------------------------------
#ifndef MMDL_PL_NULL
void MMDL_MoveHideEoaPtrSet( MMDL * mmdl, EOA_PTR eoa )
{
  MV_HIDE_WORK *work = MMDL_GetMoveProcWork( mmdl );
  work->eoa_hide = eoa;
}
#endif

//--------------------------------------------------------------
/**
 * MV_HIDE 隠れ蓑EOA_PTR取得
 * @param  mmdl  MMDL *
 * @param  eoa    EOA_PTR
 * @retval  nothing
 */
//--------------------------------------------------------------
#ifndef MMDL_PL_NULL
EOA_PTR MMDL_MoveHideEoaPtrGet( MMDL * mmdl )
{
  MV_HIDE_WORK *work = MMDL_GetMoveProcWork( mmdl );
  return( work->eoa_hide );
}
#endif

//--------------------------------------------------------------
/**
 * MV_HIDE 隠れ蓑、脱いだ状態に
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveHidePullOffFlagSet( MMDL * mmdl )
{
  MV_HIDE_WORK *work = MMDL_GetMoveProcWork( mmdl );
  work->pulloff_flag = TRUE;
}

//======================================================================
//  MV_COPYU
//======================================================================
static int (* const DATA_CopyMoveTbl[SEQNO_COPYMOVE_MAX])( MMDL *, MV_COPY_WORK * );

//--------------------------------------------------------------
/**
 * ものまね　ワーク初期化
 * @param  mmdl  MMDL *
 * @param  dir    初期方向
 * @param  lgrass  長い草限定ものまね FALSE=Not
 * @retval
 */
//--------------------------------------------------------------
static void MoveCopy_WorkInit( MMDL * mmdl, int dir, u32 lgrass )
{
  MV_COPY_WORK *work = MMDL_InitMoveProcWork( mmdl, MV_COPY_WORK_SIZE );
  work->dir_jiki = DIR_NOT;
  work->lgrass_on = lgrass;
  MMDL_SetDirDisp( mmdl, DIR_UP );
}

//--------------------------------------------------------------
/**
 * MV_COPYU 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCopyU_Init( MMDL * mmdl )
{
  MoveCopy_WorkInit( mmdl, DIR_UP, FALSE );
}

//--------------------------------------------------------------
/**
 * MV_COPYD 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCopyD_Init( MMDL * mmdl )
{
  MoveCopy_WorkInit( mmdl, DIR_DOWN, FALSE );
}

//--------------------------------------------------------------
/**
 * MV_COPYL 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCopyL_Init( MMDL * mmdl )
{
  MoveCopy_WorkInit( mmdl, DIR_LEFT, FALSE );
}

//--------------------------------------------------------------
/**
 * MV_COPYR 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCopyR_Init( MMDL * mmdl )
{
  MoveCopy_WorkInit( mmdl, DIR_RIGHT, FALSE );
}

//--------------------------------------------------------------
/**
 * MV_COPYLGRASSU 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCopyLGrassU_Init( MMDL * mmdl )
{
  MoveCopy_WorkInit( mmdl, DIR_UP, TRUE );
}

//--------------------------------------------------------------
/**
 * MV_COPYLGRASSD 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCopyLGrassD_Init( MMDL * mmdl )
{
  MoveCopy_WorkInit( mmdl, DIR_DOWN, TRUE );
}

//--------------------------------------------------------------
/**
 * MV_COPYLGRASSL 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCopyLGrassL_Init( MMDL * mmdl )
{
  MoveCopy_WorkInit( mmdl, DIR_LEFT, TRUE );
}

//--------------------------------------------------------------
/**
 * MV_COPYLGRASSR 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCopyLGrassR_Init( MMDL * mmdl )
{
  MoveCopy_WorkInit( mmdl, DIR_RIGHT, TRUE );
}

//--------------------------------------------------------------
/**
 * MV_COPY 動作
 * @param
 * @retval
 */
//--------------------------------------------------------------
void MMDL_MoveCopy_Move( MMDL * mmdl )
{
  MV_COPY_WORK *work = MMDL_GetMoveProcWork( mmdl );
  while( DATA_CopyMoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * MV_COPY 動作　初回方向　その0
 * @param  mmdl  MMDL *
 * @param  work  MV_COPY_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int CopyMove_FirstInit( MMDL * mmdl, MV_COPY_WORK *work )
{
  int ret = MMDL_GetDirDisp( mmdl );
  ret = MMDL_ChangeDirAcmdCode( ret, AC_DIR_U );
  MMDL_SetLocalAcmd( mmdl, ret );
  MMDL_OffMoveBitMove( mmdl );
  MMDL_OffMoveBitMoveEnd( mmdl );
  work->seq_no = SEQNO_COPYMOVE_FIRST_WAIT;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_COPYU 動作　初回方向　その1
 * @param  mmdl  MMDL *
 * @param  work  MV_COPY_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int CopyMove_FirstWait( MMDL * mmdl, MV_COPY_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == TRUE ){
    work->seq_no = SEQNO_COPYMOVE_INIT;
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * MV_COPY 動作 0
 * @param  mmdl  MMDL *
 * @param  work  MV_COPY_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int CopyMove_Init( MMDL * mmdl, MV_COPY_WORK *work )
{
  if( work->dir_jiki == DIR_NOT ){
    MMDL *jiki = MMDLSYS_SearchMMdlPlayer( MMDL_GetMMdlSys(mmdl) );
    if( jiki != NULL ){
      work->dir_jiki = MMDL_GetDirMove( jiki );
    }
  }
  
  MMDL_OffMoveBitMove( mmdl );
  MMDL_OffMoveBitMoveEnd( mmdl );
  work->seq_no = SEQNO_COPYMOVE_CMD_SET;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * ものまね草移動チェック
 * @param  mmdl  MMDL *
 * @param  dir    移動方向
 * @retval  u32    hit bit  
 */
//--------------------------------------------------------------
static u32 CopyMove_LongGrassHitCheck( MMDL * mmdl, int dir )
{
  u32 ret = MMDL_MOVEHITBIT_NON;
  MAPATTR attr = MMDL_GetMapDirAttr( mmdl, dir );
  MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
  
  if( MAPATTR_VALUE_CheckLongGrass(val) == FALSE ){
    ret = MMDL_MOVEHITBIT_ATTR;
  }
  
  ret |= MMDL_HitCheckMoveDir( mmdl, dir );
  return( ret );
}

//--------------------------------------------------------------
/**
 * コピー動作 移動セット
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void CopyMove_MoveSet(
  MMDL * mmdl, int dir, int ac, u32 lgrass_on )
{
  u32 ret;
  
  if( lgrass_on == FALSE ){
    ret = MMDL_HitCheckMoveDir( mmdl, dir );
  }else{
    ret = CopyMove_LongGrassHitCheck( mmdl, dir );
  }
  
  if( ret != MMDL_MOVEHITBIT_NON ){
    ac = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
  }else{
    ac = MMDL_ChangeDirAcmdCode( dir, ac );
    MMDL_OnMoveBitMove( mmdl );
  }
  
  MMDL_SetLocalAcmd( mmdl, ac );
}

//--------------------------------------------------------------
/**
 * MV_COPY 動作 1
 * @param  mmdl  MMDL *
 * @param  work  MV_COPY_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int CopyMove_CmdSet( MMDL * mmdl, MV_COPY_WORK *work )
{
  int ret;
  MMDL *jiki = MMDLSYS_SearchMMdlPlayer( MMDL_GetMMdlSys(mmdl) );
  u16 dir = MMDL_GetDirMove( jiki );
#ifndef MMDL_PL_NULL
  u32 type = Player_AcmdTypeGet( fsys->player );
  
  switch( type ){
  case HEROACTYPE_NON:
  case HEROACTYPE_STOP:
    ret = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
    MMDL_SetLocalAcmd( mmdl, ret );
    break;
  case HEROACTYPE_WALK_32F:
    CopyMove_MoveSet( mmdl, dir, AC_WALK_U_32F, work->lgrass_on );
    break;
  case HEROACTYPE_WALK_16F:
    CopyMove_MoveSet( mmdl, dir, AC_WALK_U_16F, work->lgrass_on );
    break;
  case HEROACTYPE_WALK_8F:
    CopyMove_MoveSet( mmdl, dir, AC_WALK_U_8F, work->lgrass_on );
    break;
  case HEROACTYPE_WALK_4F:
    CopyMove_MoveSet( mmdl, dir, AC_WALK_U_4F, work->lgrass_on );
    break;
  case HEROACTYPE_WALK_2F:
    CopyMove_MoveSet( mmdl, dir, AC_WALK_U_2F, work->lgrass_on );
    break;
  }
#else
  ret = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
  MMDL_SetLocalAcmd( mmdl, ret );
#endif

  work->seq_no = SEQNO_COPYMOVE_CMD_WAIT;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_COPY 動作 2
 * @param  mmdl  MMDL *
 * @param  work  MV_COPY_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int CopyMove_MoveWait( MMDL * mmdl, MV_COPY_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == FALSE ){
    return( FALSE );
  }
  
  MMDL_OffMoveBitMove( mmdl );
  MMDL_OffMoveBitMoveEnd( mmdl );
  work->seq_no = SEQNO_COPYMOVE_INIT;
  return( FALSE );
}

//--------------------------------------------------------------
///  コピー動作テーブル
//--------------------------------------------------------------
static int (* const DATA_CopyMoveTbl[SEQNO_COPYMOVE_MAX])( MMDL *, MV_COPY_WORK * ) =
{
  CopyMove_FirstInit,
  CopyMove_FirstWait,
  CopyMove_Init,
  CopyMove_CmdSet,
  CopyMove_MoveWait,
};

//======================================================================
//  壁沿い移動
//======================================================================
static int AlongWall_WallMove(MMDL * mmdl,MV_ALONGW_WORK *work,int ac);
static int (* const DATA_AlongMoveTbl[3])(MMDL *, MV_ALONGW_WORK *);

//--------------------------------------------------------------
/**
 * 壁沿い移動　ワーク初期化
 * @param  mmdl    MMDL *
 * @param  dir_h_init  初期利き手
 * @param  dir_h    利き手
 * @retval  nothing
 */
//--------------------------------------------------------------
static void AlongWallWorkInit( MMDL * mmdl, ALONG_DIR dir_h_init, ALONG_DIR dir_h )
{
  MV_ALONGW_WORK *work;
  work = MMDL_InitMoveProcWork( mmdl, MV_ALONGW_WORK_SIZE );
  work->dir_hand_init = dir_h_init;
  work->dir_hand = dir_h;
}

//--------------------------------------------------------------
/**
 * 壁沿い移動　左手　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_AlongWallL_Init( MMDL * mmdl )
{
  AlongWallWorkInit( mmdl, ALONG_L, ALONG_L );
}

//--------------------------------------------------------------
/**
 * 壁沿い移動　右手　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_AlongWallR_Init( MMDL * mmdl )
{
  AlongWallWorkInit( mmdl, ALONG_R, ALONG_R );
}

//--------------------------------------------------------------
/**
 * 壁沿い移動　両手左　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_AlongWallLRL_Init( MMDL * mmdl )
{
  AlongWallWorkInit( mmdl, ALONG_LR, ALONG_L );
}

//--------------------------------------------------------------
/**
 * 壁沿い移動　両手右　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_AlongWallLRR_Init( MMDL * mmdl )
{
  AlongWallWorkInit( mmdl, ALONG_LR, ALONG_R );
}

//--------------------------------------------------------------
/**
 * 壁沿い移動　動作
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_AlongWall_Move( MMDL * mmdl )
{
  MV_ALONGW_WORK *work = MMDL_GetMoveProcWork( mmdl );
  while( DATA_AlongMoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * 動作 初期化
 * @param  mmdl  MMDL *
 * @param  work  MV_ALONGW_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int AlongWallMove_Init( MMDL * mmdl, MV_ALONGW_WORK *work )
{
  MMDL_OffMoveBitMove( mmdl );
  work->seq_no++;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 動作 初期化
 * @param  mmdl  MMDL *
 * @param  work  MV_ALONGW_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int AlongWallMove_CmdSet( MMDL * mmdl, MV_ALONGW_WORK *work )
{
  AlongWall_WallMove( mmdl, work, AC_WALK_U_8F );
  work->seq_no++;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 動作 コマンド終了待ち
 * @param  mmdl  MMDL *
 * @param  work  MV_ALONGW_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int AlongWallMove_CmdWait( MMDL * mmdl, MV_ALONGW_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == TRUE ){
    work->seq_no = 0;
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
//  壁伝い移動　動作テーブル
//--------------------------------------------------------------
static int (* const DATA_AlongMoveTbl[3])( MMDL *, MV_ALONGW_WORK * ) =
{
  AlongWallMove_Init,
  AlongWallMove_CmdSet,
  AlongWallMove_CmdWait,
};

//--------------------------------------------------------------
///  手の位置
//--------------------------------------------------------------
static const int DATA_DirHandPosX[DIR_MAX4][2] = 
{
  { -1, 1 },  //up,left,right
  { 1, -1 },  //down,left,right
  { 0, 0 },  //left,left,right
  { 0, 0 },  //right,left,right
};

static const int DATA_DirHandPosZ[DIR_MAX4][2] =
{
  { 0, 0 },  //up,left,right
  { 0, 0 },  //down,left,right
  { 1, -1 },  //left,left,right
  { -1, 1 },  //right,left,right
};

//--------------------------------------------------------------
///  見失った際の判定位置
//--------------------------------------------------------------
static const int DATA_DirHandLostPosX[DIR_MAX4][2] =
{
  { -1, 1 },  //up,left,right
  { 1, -1 },  //down,left,right
  { 1, 1  },  //left,left,right
  { -1, -1 },  //right,left,right
};

static const int DATA_DirHandLostPosZ[DIR_MAX4][2] =
{
  { 1, 1 },  //up,left,right
  { -1, -1 },  //down,left,right
  { 1, -1  },  //left,left,right
  { -1, 1 },  //right,left,right
};

//--------------------------------------------------------------
///  見失った際の方向切り替え
//--------------------------------------------------------------
static const int DATA_DirHandLostDir[DIR_MAX4][2] =
{
  { DIR_LEFT, DIR_RIGHT },  //up,left,right
  { DIR_RIGHT, DIR_LEFT },  //down,left,right
  { DIR_DOWN, DIR_UP  },  //left,left,right
  { DIR_UP, DIR_DOWN },  //right,left,right
};

//--------------------------------------------------------------
///  壁衝突時の方向切り替え
//--------------------------------------------------------------
static const int DATA_DirHandAttrHitDir[DIR_MAX4][2] =
{
  { DIR_RIGHT, DIR_LEFT },  //up,left,right
  { DIR_LEFT, DIR_RIGHT },  //down,left,right
  { DIR_UP, DIR_DOWN  },  //left,left,right
  { DIR_DOWN, DIR_UP },  //right,left,right
};

//--------------------------------------------------------------
///  手のむき反転
//--------------------------------------------------------------
static const ALONG_DIR DATA_DirHandFlip[ALONG_LR] =
{ ALONG_R, ALONG_L };
  
//--------------------------------------------------------------
/**
 * 壁沿い　指定方向の壁を拾う
 * @param  fsys  FIELDSYS_WORK
 * @param  gx    グリッドX
 * @param  gz    グリッドZ
 * @param  dir    向き
 * @param  hdir  利き手
 * @retval  BOOL  TRUE=壁あり
 */
//--------------------------------------------------------------
static BOOL AlongWall_HandHitGet(
  const MMDL *mmdl, const VecFx32 *pos, int dir, ALONG_DIR hdir )
{
  u32 attr;
  BOOL hit = TRUE;
  VecFx32 next = *pos;
  s16 gx = SIZE_GRID_FX32( pos->x );
  s16 gz = SIZE_GRID_FX32( pos->z );
  
  gx += DATA_DirHandPosX[dir][hdir];
  gz += DATA_DirHandPosZ[dir][hdir];
  next.x = GRID_SIZE_FX32( gx );
  next.z = GRID_SIZE_FX32( gz );
  
  if( MMDL_GetMapPosAttr(mmdl,&next,&attr) == TRUE ){
    if( MAPATTR_GetHitchFlag(attr) == FALSE ){
      hit = FALSE;
    }
  }
  
  return( hit );
}

//--------------------------------------------------------------
/**
 * 壁沿い　壁消失時の壁検索
 * @param  fsys  FIELDSYS_WORK
 * @param  gx    グリッドX
 * @param  gz    グリッドZ
 * @param  dir    向き
 * @param  hdir  利き手
 * @retval  BOOL  TRUE=壁あり
 */
//--------------------------------------------------------------
static BOOL AlongWall_HandLostHitGet(
    const MMDL *mmdl, const VecFx32 *pos, int dir, ALONG_DIR hdir )
{
  u32 attr;
  BOOL hit = FALSE;
  VecFx32 next = *pos;
  s16 gx = SIZE_GRID_FX32( pos->x );
  s16 gz = SIZE_GRID_FX32( pos->z );
  gx += DATA_DirHandLostPosX[dir][hdir];
  gz += DATA_DirHandLostPosZ[dir][hdir];
  next.x = GRID_SIZE_FX32( gx );
  next.z = GRID_SIZE_FX32( gz );
  if( MMDL_GetMapPosAttr(mmdl,&next,&attr) == TRUE ){
    if( MAPATTR_GetHitchFlag(attr) == FALSE ){
      hit = FALSE;
    }
  }
  return( hit );
}

//--------------------------------------------------------------
/**
 * 壁沿い移動　手の位置に壁はあるか
 * @param  mmdl  MMDL *
 * @param  dir_hand  ALONG_DIR
 * @retval  int    TRUE=壁アリ
 */
//--------------------------------------------------------------
static int AlongWall_HandWallCheck(
  const MMDL * mmdl, int dir, ALONG_DIR dir_hand )
{
  BOOL hit;
  VecFx32 pos;
  MMDL_GetVectorPos( mmdl, &pos );
  hit = AlongWall_HandHitGet( mmdl, &pos, dir, dir_hand );
  return( hit );
}

//--------------------------------------------------------------
/**
 * 壁沿い移動　壁手探り
 * @param  mmdl  MMDL *
 * @param  dir_hand  ALONG_DIR
 * @retval  int    TRUE=壁アリ
 */
//--------------------------------------------------------------
static int AlongWall_HandLostWallCheck(
  MMDL * mmdl, int dir, ALONG_DIR dir_hand )
{
  BOOL hit;
  VecFx32 pos;
  MMDL_GetVectorPos( mmdl, &pos );
  hit = AlongWall_HandLostHitGet( mmdl, &pos, dir, dir_hand );
  return( hit );
}

//--------------------------------------------------------------
/**
 * 壁移動チェック
 * @param  mmdl  MMDL *
 * @param  dir_hand  ALONG_DIR
 * @retval  int    移動するべき方向
 */
//--------------------------------------------------------------
static int AlongWall_MoveHitCheck(
  MMDL * mmdl, int dir_move, ALONG_DIR dir_hand )
{
  if( AlongWall_HandWallCheck(
        mmdl,dir_move,dir_hand) == FALSE ){  //壁が無い
    if( AlongWall_HandLostWallCheck(mmdl,dir_move,dir_hand) == FALSE ){
      return( DIR_NOT );          //手探りでも壁が無い
    }
    
    //壁発見 方向切り替え
    dir_move = DATA_DirHandLostDir[dir_move][dir_hand];
  }
  
  return( dir_move );
}

//--------------------------------------------------------------
/**
 * 壁伝い移動チェック
 * @param  mmdl  MMDL *
 * @param  dir_move  移動方向 DIR_NOT=移動不可
 * @param  dir_hand  壁側の手の向き
 * @retval  u32    ヒットビット
 */
//--------------------------------------------------------------
static u32 AlongWall_WallMoveCheck(
  MMDL * mmdl, int *dir_move, ALONG_DIR dir_hand )
{
  u32 ret;
  
  *dir_move = AlongWall_MoveHitCheck( mmdl, *dir_move, dir_hand );
  
  if( *dir_move != DIR_NOT ){
    ret = MMDL_HitCheckMoveDir( mmdl, *dir_move );
    return( ret );
  }
  
  return( 0 );
}

//--------------------------------------------------------------
/**
 * 壁伝い移動
 * @param  mmdl  MMDL *
 * @param  work  MV_ALONGW_WORK
 * @retval  int    TRUE=移動した
 */
//--------------------------------------------------------------
static int AlongWall_WallMove(
  MMDL * mmdl, MV_ALONGW_WORK *work, int ac )
{
  u32 ret;
  int dir_hand = work->dir_hand;
  int dir_move = MMDL_GetDirDisp( mmdl );
  
  ret = AlongWall_WallMoveCheck( mmdl, &dir_move, dir_hand );
  
  if( dir_move == DIR_NOT ){          //壁がない
    dir_move = MMDL_GetDirDisp( mmdl );
    ac = MMDL_ChangeDirAcmdCode( dir_move, AC_STAY_WALK_U_16F );
    MMDL_SetLocalAcmd( mmdl, ac );
    return( FALSE );
  }
  
  if( ret == MMDL_MOVEHITBIT_NON ){    //移動可能
    ac = MMDL_ChangeDirAcmdCode( dir_move, ac );
    MMDL_OnMoveBitMove( mmdl );
    MMDL_SetLocalAcmd( mmdl, ac );
    return( TRUE );
  }
  
  //移動制限ヒット&両手利き　反転を試みる
  if( (ret & MMDL_MOVEHITBIT_LIM) && work->dir_hand_init == ALONG_LR ){
    dir_move = MMDL_TOOL_FlipDir( MMDL_GetDirDisp(mmdl) );
    dir_hand = DATA_DirHandFlip[dir_hand];
    work->dir_hand = dir_hand;
    
    ret = AlongWall_WallMoveCheck( mmdl, &dir_move, dir_hand );
    
    if( dir_move == DIR_NOT ){          //壁がない
      dir_move = MMDL_GetDirDisp( mmdl );
      ac = MMDL_ChangeDirAcmdCode( dir_move, AC_STAY_WALK_U_16F );
      MMDL_SetLocalAcmd( mmdl, ac );
      return( FALSE );
    }
    
    if( ret == MMDL_MOVEHITBIT_NON ){    //移動可能
      ac = MMDL_ChangeDirAcmdCode( dir_move, ac );
      MMDL_OnMoveBitMove( mmdl );
      MMDL_SetLocalAcmd( mmdl, ac );
      return( TRUE );
    }
  }
  
  //壁ヒット　移動方向変更
  if( (ret & MMDL_MOVEHITBIT_ATTR) ){
    dir_move = DATA_DirHandAttrHitDir[dir_move][dir_hand];
    
    ret = AlongWall_WallMoveCheck( mmdl, &dir_move, dir_hand );
    
    if( dir_move == DIR_NOT ){          //壁が無い
      dir_move = MMDL_GetDirDisp( mmdl );
      ac = MMDL_ChangeDirAcmdCode( dir_move, AC_STAY_WALK_U_16F );
      MMDL_SetLocalAcmd( mmdl, ac );
      return( FALSE );
    }
    
    if( ret == MMDL_MOVEHITBIT_NON ){    //移動可能
      ac = MMDL_ChangeDirAcmdCode( dir_move, ac );
      MMDL_OnMoveBitMove( mmdl );
      MMDL_SetLocalAcmd( mmdl, ac );
      return( TRUE );
    }
  }
  
  //移動不可
  dir_move = MMDL_GetDirDisp( mmdl );  //向きを戻す
  ac = MMDL_ChangeDirAcmdCode( dir_move, AC_STAY_WALK_U_16F );
  MMDL_SetLocalAcmd( mmdl, ac );
  return( FALSE );
}
