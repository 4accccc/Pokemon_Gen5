//======================================================================
/**
 * @file  event_trainer_eye.c
 * @brief  トレーナー視線イベント
 * @author  kagaya
 * @date  05.10.03
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"

#include "fieldmap.h"
#include "fldmmdl.h"
#include "script.h"
#include "fldeff_gyoe.h"
#include "event_trainer_eye.h"
#include "effect_encount.h"

//SCRID_TRAINER_MOVE_BATTLE 
#include "../../../resource/fldmapdata/script/trainer_def.h"

#include "script_def.h"
//======================================================================
//  define
//======================================================================
#define EYE_MEET_GYOE_END_WAIT (30) ///<!マーク出現後の間　フレーム単位
#define EYE_MEET_MOVE_END_WAIT (8) ///<視線移動後の間　フレーム単位

//--------------------------------------------------------------
///  視線移動イベント処理番号
//--------------------------------------------------------------
enum
{
  SEQNO_TRMOVE_INIT = 0,
  SEQNO_TRMOVE_OBJMOVE_WAIT,     ///<動作停止待ち
  SEQNO_TRMOVE_JIKIMOVE_WAIT,    ///<自機動作終了待ち
  SEQNO_TRMOVE_DIR_CHANGE,       ///<方向変更
  SEQNO_TRMOVE_DIR_CHANGE_WAIT,  ///<方向変更待ち
  SEQNO_TRMOVE_GYOE_SET,         ///<!セット
  SEQNO_TRMOVE_GYOE_WAIT,        ///<!待ち
  SEQNO_TRMOVE_HIDE_PULLOFF_SET, ///<隠れ蓑脱ぎセット
  SEQNO_TRMOVE_HIDE_PULLOFF_WAIT,///<隠れ蓑終了待ち
  SEQNO_TRMOVE_GYOE_END_WAIT,    ///<!終了後待ち
  SEQNO_TRMOVE_MOVE_RANGE_CHECK, ///<移動距離チェック
  SEQNO_TRMOVE_MOVE_START,       ///<移動開始
  SEQNO_TRMOVE_MOVE,             ///<移動中
  SEQNO_TRMOVE_MOVE_END_WAIT,    ///<移動終了後待ち
  SEQNO_TRMOVE_JIKI_TURN_SET,    ///<自機振り向きセット
  SEQNO_TRMOVE_JIKI_TURN,        ///<自機振り向き
  SEQNO_TRMOVE_ACMD_END,         ///<アニメ終了
  SEQNO_TRMOVE_END,              ///<終了
};

//======================================================================
//  strcut
//======================================================================
//--------------------------------------------------------------
///  視線ヒット格納
//--------------------------------------------------------------
typedef struct
{
  int range;
  int dir;
  int scr_id;
  int tr_id;
  int tr_type;
  MMDL *mmdl;
}EYEMEET_HITDATA;

//--------------------------------------------------------------
//  視線ヒット移動処理用ワーク
//--------------------------------------------------------------
typedef struct
{
  int seq_no;        ///<処理番号
  BOOL end_flag;      ///<終了フラグ
   
  int dir;          ///<移動方向
  int range;        ///<移動距離
  int gyoe_type;    ///<!タイプ
  int tr_type;      ///<トレーナータイプ
  int sisen_no;      ///<視線番号
  int count;        ///<移動カウント
   
  FLDEFF_TASK *task_gyoe;   ////<ギョエータスク
  MMDL *mmdl;                ///<移動を行うMMDL*
  FIELDMAP_WORK *fieldMap;  ///<FILEDMAP_WORK*
  FIELD_PLAYER *fieldPlayer; ///<FIELD_PLAYER*
}EV_EYEMEET_MOVE_WORK;

//======================================================================
//  proto
//======================================================================
static BOOL treye_CheckEyeMeet(
    FIELDMAP_WORK *fieldMap, MMDL *non_mmdl, EYEMEET_HITDATA *hit );
static int treye_CheckEyeRange(
    const FIELD_ENCOUNT* enc, const MMDL *mmdl, const MMDL *jiki, u16 *hitDir );

static int treye_CheckEyeLine(
    u16 dir, s16 range, s16 tx, s16 ty, s16 tz, s16 gx, s16 gy, s16 gz );

static GMEVENT_RESULT treyeEvent_EyeMeetMove(
    GMEVENT *ev, int *seq, void *wk );

static u16 tr_GetTrainerEventType( const MMDL *mmdl );
static BOOL tr_HitCheckEyeLine( const MMDL *mmdl, u16 dir, int range, const FIELD_ENCOUNT* enc );
static u16 tr_GetTrainerID( const MMDL *mmdl );
static BOOL tr_CheckEventFlag( FIELDMAP_WORK *fieldMap, const MMDL *mmdl );
static void tr_InitEyeMeetHitData(
    EYEMEET_HITDATA *data, MMDL *mmdl, int range, u16 dir );
static MMDL * tr_CheckPairTrainer( const MMDL *tr_mmdl, u16 tr_id );
static GMEVENT * tr_SetEventScript( FIELDMAP_WORK *fieldMap, MMDL *mmdl );
static void trEventScript_SetTrainerEyeData(
    GMEVENT *event, const EYEMEET_HITDATA *data, int tr_type, int tr_num );

static int (* const data_treye_CheckEyeLineTbl[DIR_MAX4])(
    s16,s16,s16,s16,s16,s16,s16);

//======================================================================
//  トレーナー視線イベント
//======================================================================
//--------------------------------------------------------------
/**
 * トレーナー視線イベントチェック
 * @param fieldMap FIELDMAP_WORK*
 * @param vs2 ダブルバトル可能か TRUE=可能 FALSE=不可
 * @retval GMEVENT NULL=ヒット無し
 */
//--------------------------------------------------------------
GMEVENT * EVENT_CheckTrainerEye( FIELDMAP_WORK *fieldMap, BOOL vs2 )
{
  EYEMEET_HITDATA hit0;
  GMEVENT *event = NULL;
  
  if( treye_CheckEyeMeet(fieldMap,NULL,&hit0) == TRUE )
  {
    MMDL *mmdl;
    EYEMEET_HITDATA hit1;
    
    if( hit0.tr_type == SCR_EYE_TR_TYPE_SINGLE ) //シングル
    {
      event = tr_SetEventScript( fieldMap, hit0.mmdl ); //スクリプト起動

      if( vs2 == FALSE || //シングル戦チェック
          treye_CheckEyeMeet(fieldMap,hit0.mmdl,&hit1) == FALSE )
      { //スクリプト用イベントデータセット シングル
        trEventScript_SetTrainerEyeData(
            event, &hit0, SCR_EYE_TR_TYPE_SINGLE, SCR_EYE_TR_0 );
        KAGAYA_Printf( "TRAINER EYE HIT SINGLE\n" );
      }
      else
      { //スクリプト用イベントデータセット タッグ
        trEventScript_SetTrainerEyeData(
            event, &hit0, SCR_EYE_TR_TYPE_TAG, SCR_EYE_TR_0 );
        trEventScript_SetTrainerEyeData(
            event, &hit1, SCR_EYE_TR_TYPE_TAG, SCR_EYE_TR_1 );
        KAGAYA_Printf( "TRAINER EYE HIT TAG DOUBLE\n" );
      }
    }
    else if( hit0.tr_type == SCR_EYE_TR_TYPE_DOUBLE )
    {
      if( vs2 == TRUE ) //ダブル可能
      {
        event = tr_SetEventScript( fieldMap, hit0.mmdl ); //スクリプト起動
        
        mmdl = tr_CheckPairTrainer( hit0.mmdl, hit0.tr_id );
        tr_InitEyeMeetHitData( &hit1, mmdl, hit0.range, hit0.dir );
        
        //スクリプト用イベントデータセット ダブル
        trEventScript_SetTrainerEyeData(
            event, &hit0, SCR_EYE_TR_TYPE_DOUBLE, SCR_EYE_TR_0 );
        trEventScript_SetTrainerEyeData(
            event, &hit1, SCR_EYE_TR_TYPE_DOUBLE, SCR_EYE_TR_1 );
        KAGAYA_Printf( "TRAINER EYE HIT DOUBLE\n" );
      }
    }
    else //対応していないトレーナータイプ
    {
      GF_ASSERT( 0 && "TRAINER EYE TYPE ERROR" );
    }
  }
  
  return( event );
}

//--------------------------------------------------------------
/**
 * トレーナー視線範囲チェック　グローバル　引数指定
 * @param  mmdl FIELD_OBJ_PTR
 * @param  jiki    PLAYER_STATE_PTR
 * @param  eye_dir    視線方向
 * @param  eye_range  視線距離
 * @param enc FIELD_ENCOUNT NULL=エフェクトエンカウントチェック無し
 * @retval  int 視線ヒット時の自機までのグリッド距離。EYE_CHECK_NOHIT=エラー
 */
//--------------------------------------------------------------
int EVENT_CheckTrainerEyeRange(
    const MMDL *mmdl, u16 eye_dir, int eye_range, const FIELD_ENCOUNT *enc )
{
  int ret;
  MMDL_GRIDPOS pt,pj;
  const MMDL *jiki;
  const MMDLSYS *mmdlsys;
  
  mmdlsys = MMDL_GetMMdlSys( mmdl );
  jiki = MMDLSYS_SearchOBJID( mmdlsys, MMDL_ID_PLAYER );
  
  if( jiki == NULL ){
    GF_ASSERT( 0 );
    return( EYE_CHECK_NOHIT );
  }
  
  MMDL_GetGridPos( mmdl, &pj );
  MMDL_GetGridPos( mmdl, &pt );
  
  ret = treye_CheckEyeLine( eye_dir, eye_range, 
      pt.gx, pt.gy, pt.gz, pj.gx, pj.gy, pj.gz );
  
  if( ret != EYE_CHECK_NOHIT ){
    if( tr_HitCheckEyeLine(mmdl,eye_dir,ret,enc) == TRUE ){
      ret = EYE_CHECK_NOHIT;
    }
  }
  
  return( ret );
}

//======================================================================
//  視線チェック
//======================================================================
//--------------------------------------------------------------
/**
 * トレーナー視線チェック
 * @param fieldMap FIELDMAP_WORK
 * @param non_mmdl 視線チェックの際に無視するMMDL NULL=無し
 * @param hit 視線ヒット時の情報格納先
 * @retval BOOL TRUE=ヒット
 */
//--------------------------------------------------------------
static BOOL treye_CheckEyeMeet(
    FIELDMAP_WORK *fieldMap, MMDL *non_mmdl, EYEMEET_HITDATA *hit )
{
  u16 dir;
  u32 no=0;
  int range=EYE_CHECK_NOHIT;
  MMDL *mmdl = NULL;
  MMDL *jiki = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer(fieldMap) );
  MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldMap );
  const FIELD_ENCOUNT * enc = FIELDMAP_GetEncount( fieldMap );

  while( MMDLSYS_SearchUseMMdl(mmdlsys,&mmdl,&no) == TRUE )
  {
    if( non_mmdl == NULL || (non_mmdl != mmdl) )
    {
      range = treye_CheckEyeRange( enc, mmdl, jiki, &dir );
      
      if( range != EYE_CHECK_NOHIT )
      {
        if( tr_CheckEventFlag(fieldMap,mmdl) == FALSE )
        {
#ifdef DEBUG_ONLY_FOR_kagaya
          KAGAYA_Printf( "視線ヒット ヒットしたトレーナーID =%d\n",
              tr_GetTrainerID(mmdl) );
#endif
          tr_InitEyeMeetHitData( hit, mmdl, range, dir );
          return( TRUE );
        }
      }
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * トレーナー視線範囲チェック
 * @param mmdl MMDL*
 * @param jiki 自機MMDL
 * @param hitDir 
 * @retval int 視線ヒット時の自機までのグリッド数。EYE_CHECK_NOHIT=外れ。
 */
//--------------------------------------------------------------
static int treye_CheckEyeRange(
    const FIELD_ENCOUNT* enc, const MMDL *mmdl, const MMDL *jiki, u16 *hitDir )
{
  u16 type;
  
  type = tr_GetTrainerEventType( mmdl );
  
  if( type == EV_TYPE_TRAINER || type == EV_TYPE_TRAINER_EYEALL )
  {
    int ret;
    u16 dir;
    MMDL_GRIDPOS pt,pj;
    s16 range = MMDL_GetParam( mmdl, MMDL_PARAM_0 );

    MMDL_GetGridPos( mmdl, &pt);
    MMDL_GetGridPos( jiki, &pj);
    
    if( type == EV_TYPE_TRAINER )
    {
      dir = MMDL_GetDirDisp( mmdl );
      ret = treye_CheckEyeLine( dir, range, pt.gx, pt.gy, pt.gz, pj.gx, pj.gy, pj.gz );
      
      if( ret != EYE_CHECK_NOHIT )
      {
        if( tr_HitCheckEyeLine(mmdl,dir,ret,enc) == FALSE ){
          *hitDir = dir;
          return( ret );
        }
      }
    }
    else //EV_TYPE_TRAINER_EYEALL
    {
      dir = DIR_UP;
      
      do{
        ret = treye_CheckEyeLine( dir, range, pt.gx, pt.gy, pt.gz, pj.gx, pj.gy, pj.gz );
        
        if( ret != EYE_CHECK_NOHIT )
        {
          if( tr_HitCheckEyeLine(mmdl,dir,ret,enc) == FALSE ){
            *hitDir = dir;
            return( ret );
          }
        }
        
        dir++;
      }while( dir < DIR_MAX4 );
    }
  }

  return( EYE_CHECK_NOHIT );
}

//--------------------------------------------------------------
/**
 * トレーナー視線チェック
 * @param mmdl MMDL
 * @param dir 視線方向 DIR_UP等
 * @param range 視線距離
 * @param x 自機グリッドX座標
 * @param y 自機グリッドY座標
 * @param z 自機グリッドZ座標
 * @retval int 自機へのグリッド距離。EYE_CHECK_NOHIT=外れ
 */
//--------------------------------------------------------------
static int treye_CheckEyeLine(
    u16 dir, s16 range, s16 tx, s16 ty, s16 tz, s16 gx, s16 gy, s16 gz )
{
  int hit;
  GF_ASSERT( dir < DIR_MAX4 );
  hit = data_treye_CheckEyeLineTbl[dir]( range, tx, ty, tz, gx, gy, gz );
  return( hit );
}

//--------------------------------------------------------------
/**
 * トレーナー視線チェック 上
 * @param range 視線距離
 * @param tx トレーナーグリッドX座標
 * @param ty トレーナーグリッドY座標
 * @param tz トレーナーグリッドZ座標
 * @param jx 自機グリッドX座標
 * @param jy 自機グリッドY座標
 * @param jz 自機グリッドZ座標
 * @retval int 自機へのグリッド距離。EYE_CHECK_NOHIT=外れ
 */
//--------------------------------------------------------------
static int treye_CheckEyeLineUp(
    s16 range, s16 tx, s16 ty, s16 tz, s16 jx, s16 jy, s16 jz )
{
  if( tx == jx && ty == jy ){
    if( jz < tz && jz >= (tz-range) ){
      return( tz-jz );
    }
  }
  return( EYE_CHECK_NOHIT );
}

//--------------------------------------------------------------
/**
 * トレーナー視線チェック 下
 * @param range 視線距離
 * @param tx トレーナーグリッドX座標
 * @param ty トレーナーグリッドY座標
 * @param tz トレーナーグリッドZ座標
 * @param jx 自機グリッドX座標
 * @param jy 自機グリッドY座標
 * @param jz 自機グリッドZ座標
 * @retval int 自機へのグリッド距離。EYE_CHECK_NOHIT=外れ
 */
//--------------------------------------------------------------
static int treye_CheckEyeLineDown(
    s16 range, s16 tx, s16 ty, s16 tz, s16 jx, s16 jy, s16 jz )
{
  if( tx == jx && ty == jy ){
    if( jz > tz && jz <= (tz+range) ){
      return( jz - tz );
    }
  }
  return( EYE_CHECK_NOHIT );
}

//--------------------------------------------------------------
/**
 * トレーナー視線チェック 左
 * @param range 視線距離
 * @param tx トレーナーグリッドX座標
 * @param ty トレーナーグリッドY座標
 * @param tz トレーナーグリッドZ座標
 * @param jx 自機グリッドX座標
 * @param jy 自機グリッドY座標
 * @param jz 自機グリッドZ座標
 * @retval int 自機へのグリッド距離。EYE_CHECK_NOHIT=外れ
 */
//--------------------------------------------------------------
static int treye_CheckEyeLineLeft(
    s16 range, s16 tx, s16 ty, s16 tz, s16 jx, s16 jy, s16 jz )
{
  if( tz == jz && ty == jy ){
    if( jx < tx && jx >= (tx-range) ){
      return( tx - jx );
    }
  }
  return( EYE_CHECK_NOHIT );
}

//--------------------------------------------------------------
/**
 * トレーナー視線チェック 右
 * @param range 視線距離
 * @param tx トレーナーグリッドX座標
 * @param ty トレーナーグリッドY座標
 * @param tz トレーナーグリッドZ座標
 * @param jx 自機グリッドX座標
 * @param jy 自機グリッドY座標
 * @param jz 自機グリッドZ座標
 * @retval int 自機へのグリッド距離。EYE_CHECK_NOHIT=外れ
 */
//--------------------------------------------------------------
static int treye_CheckEyeLineRight(
    s16 range, s16 tx, s16 ty, s16 tz, s16 jx, s16 jy, s16 jz )
{
  if( tz == jz ){
    if( jx > tx && jx <= (tx+range) ){
      return( jx - tx );
    }
  }
  return( EYE_CHECK_NOHIT );
}

//======================================================================
//  視線ヒットによる移動イベント
//======================================================================
//--------------------------------------------------------------
/**
 * 視線移動イベントセット
 * @param  fsys  FIELDSYS_WORK
 * @param  fldobj  FIELD_OBJ_PTR
 * @param  jiki  PLAYER_STATE_PTR
 * @param  dir    移動方向
 * @param  range  移動距離
 * @param  gyoe  !マーク制御 現状未使用
 * @param  tr_type  トレーナータイプ
 * @param  work_pos 視線ヒットワーク要素数 0=視線主 1=ペア
 * @retval  GMEVENT 移動処理を行うGMEVENT
 */
//--------------------------------------------------------------
GMEVENT * EVENT_SetTrainerEyeMove( FIELDMAP_WORK *fieldMap,
    MMDL *mmdl, FIELD_PLAYER *jiki,
    int dir, int range, int gyoe, int tr_type, int work_pos )
{
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldMap );
  GMEVENT *ev = GMEVENT_Create( gsys, NULL,
      treyeEvent_EyeMeetMove, sizeof(EV_EYEMEET_MOVE_WORK) );
  EV_EYEMEET_MOVE_WORK *work = GMEVENT_GetEventWork( ev );
  MI_CpuClear8( work, sizeof(EV_EYEMEET_MOVE_WORK) );
  
  work->dir = dir;
  work->range = range;
  work->gyoe_type = gyoe;
  work->tr_type = tr_type;
  work->sisen_no = work_pos;
  work->mmdl = mmdl;
  work->fieldMap = fieldMap;
  work->fieldPlayer = FIELDMAP_GetFieldPlayer( fieldMap );
  
  KAGAYA_Printf( "OBJ ID %dの視線移動をセット\n", MMDL_GetOBJID(mmdl) );
  return( ev );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　初期化
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_Init( EV_EYEMEET_MOVE_WORK *work )
{
  if( MMDL_CheckMoveBitMove(work->mmdl) == TRUE ){ //動作中
    MMDL_OffMoveBitMoveProcPause( work->mmdl ); //動作ポーズ解除
  }
  
  work->seq_no = SEQNO_TRMOVE_OBJMOVE_WAIT;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　OBJ動作終了待ち
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_OBJMoveWait( EV_EYEMEET_MOVE_WORK *work )
{
  if( MMDL_CheckMoveBitMove(work->mmdl) == TRUE ){
    return( FALSE ); //動作中
  }
  
  MMDL_SetDirDisp( work->mmdl, work->dir );
  MMDL_OnMoveBitMoveProcPause( work->mmdl );
  
  work->seq_no = SEQNO_TRMOVE_JIKIMOVE_WAIT;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　自機移動終了待ち
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_JikiMoveWait( EV_EYEMEET_MOVE_WORK *work )
{
  MMDL *j_mmdl = FIELD_PLAYER_GetMMdl( work->fieldPlayer );
  
  if( MMDL_CheckEndAcmd(j_mmdl) == FALSE ){
    return( FALSE );
  }
  
  work->seq_no = SEQNO_TRMOVE_DIR_CHANGE;

  {
    u32 code = MMDL_GetMoveCode( work->mmdl );
    KAGAYA_Printf( "トレーナー動作コード=0x%x\n", code );
    
    switch( code )
    {
    case MV_HIDE_SNOW:
    case MV_HIDE_SAND:
    case MV_HIDE_GRND:
    case MV_HIDE_KUSA:
      work->seq_no = SEQNO_TRMOVE_HIDE_PULLOFF_SET;
    }
  }
   
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　移動先へ向きを変える 
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_DirChange( EV_EYEMEET_MOVE_WORK *work )
{
  
  if( MMDL_CheckPossibleAcmd(work->mmdl) == TRUE ){
    u16 code;
    GF_ASSERT( work->dir < DIR_MAX4 );
    code = MMDL_ChangeDirAcmdCode( work->dir, AC_DIR_U );
    MMDL_SetAcmd( work->mmdl, code );
    work->seq_no = SEQNO_TRMOVE_DIR_CHANGE_WAIT;
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　移動先へ向きを変える　待ち
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_DirChangeWait( EV_EYEMEET_MOVE_WORK *work )
{
  if( MMDL_CheckEndAcmd(work->mmdl) == FALSE ){
    return( FALSE );
  }
  
  work->seq_no = SEQNO_TRMOVE_GYOE_SET;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　びっくりマーク出現
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_GyoeSet( EV_EYEMEET_MOVE_WORK *work )
{
  FLDEFF_CTRL *fectrl =  FIELDMAP_GetFldEffCtrl( work->fieldMap );
  work->task_gyoe = FLDEFF_GYOE_SetMMdl(
      fectrl, work->mmdl, FLDEFF_GYOETYPE_GYOE, FALSE ); //SEは鳴らさない
  work->seq_no = SEQNO_TRMOVE_GYOE_WAIT;
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　びっくりマーク表示終了待ち
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_GyoeWait( EV_EYEMEET_MOVE_WORK *work )
{
  if( FLDEFF_GYOE_CheckEnd(work->task_gyoe) == TRUE ){
    FLDEFF_TASK_CallDelete( work->task_gyoe );
    work->seq_no = SEQNO_TRMOVE_GYOE_END_WAIT;
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　隠れ蓑脱ぎ
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_HidePullOFFSet( EV_EYEMEET_MOVE_WORK *work )
{
#if 0 //pl null
  FieldOBJ_AcmdSet( work->fldobj, AC_HIDE_PULLOFF );
#else //wb kari
#endif
  work->seq_no = SEQNO_TRMOVE_HIDE_PULLOFF_WAIT;
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　隠れ蓑脱ぎ終了待ち
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_HidePullOFFWait( EV_EYEMEET_MOVE_WORK *work )
{
#if 0 //pl null
  if( FieldOBJ_AcmdEndCheck(work->fldobj) == TRUE ){
    work->seq_no = SEQNO_TRMOVE_GYOE_END_WAIT;
  }
#else //wb kari
  work->seq_no = SEQNO_TRMOVE_GYOE_END_WAIT;
#endif
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　びっくりマーク表示終了後の間
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_GyoeEndWait( EV_EYEMEET_MOVE_WORK *work )
{
  work->count++;
  
  if( work->count >= EYE_MEET_GYOE_END_WAIT ){
    work->count = 0;
    work->seq_no = SEQNO_TRMOVE_MOVE_RANGE_CHECK;
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　移動距離チェック
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_MoveRangeCheck( EV_EYEMEET_MOVE_WORK *work )
{
  if( work->range <= 1 ){                  //自機目の前 移動する必要なし
    work->seq_no = SEQNO_TRMOVE_MOVE_END_WAIT;
    return( TRUE );  
  }

  work->seq_no = SEQNO_TRMOVE_MOVE_START;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　移動開始
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_MoveStart( EV_EYEMEET_MOVE_WORK *work )
{
  if( MMDL_CheckPossibleAcmd(work->mmdl) == TRUE ){
    u16 code = MMDL_ChangeDirAcmdCode( work->dir, AC_WALK_U_8F );
    MMDL_SetAcmd( work->mmdl, code );
    work->seq_no = SEQNO_TRMOVE_MOVE;
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　移動中
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_Move( EV_EYEMEET_MOVE_WORK *work )
{
  if( MMDL_CheckEndAcmd(work->mmdl) == FALSE ){
    return( FALSE );
  }
  
  work->range--;                      //一歩減らす
  work->seq_no = SEQNO_TRMOVE_MOVE_RANGE_CHECK;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　移動終了後、自機の方向へ向かせる
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_MoveEndWait( EV_EYEMEET_MOVE_WORK *work )
{
  work->count++;
  
  if( work->count < EYE_MEET_MOVE_END_WAIT ){
    return( FALSE );
  }
  
  work->count = 0;
  work->seq_no = SEQNO_TRMOVE_JIKI_TURN_SET;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　自機を振り向かせる
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_JikiTurnSet( EV_EYEMEET_MOVE_WORK *work )
{
  u16 turn_dir;
  MMDL *j_mmdl;
  
  j_mmdl = FIELD_PLAYER_GetMMdl( work->fieldPlayer );
  turn_dir = MMDL_TOOL_GetRangeDir(
      MMDL_GetGridPosX(j_mmdl), MMDL_GetGridPosZ(j_mmdl),
      MMDL_GetGridPosX(work->mmdl), MMDL_GetGridPosZ(work->mmdl) );
  
  if( MMDL_GetDirDisp(j_mmdl) != turn_dir &&
    (work->sisen_no == 0 || work->tr_type == SCR_EYE_TR_TYPE_TAG) )
  {
    if( MMDL_CheckPossibleAcmd(j_mmdl) == TRUE )
    {
      u16 code = MMDL_ChangeDirAcmdCode( turn_dir, AC_DIR_U );
//    MMDL_OnStatusBit( j_mmdl, MMDL_STABIT_PAUSE_DIR );
      MMDL_SetAcmd( j_mmdl, code );
      work->seq_no = SEQNO_TRMOVE_JIKI_TURN;
    }
  }
  else
  {
    work->seq_no = SEQNO_TRMOVE_ACMD_END;
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　自機振り向き終了待ち
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_JikiTurn( EV_EYEMEET_MOVE_WORK *work )
{
  MMDL *j_mmdl = FIELD_PLAYER_GetMMdl( work->fieldPlayer );
  
  if( MMDL_CheckEndAcmd(j_mmdl) == FALSE ){
    return( FALSE );
  }
  
  MMDL_EndAcmd( j_mmdl );
  work->seq_no = SEQNO_TRMOVE_ACMD_END;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　アニメ終了
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_AcmdEnd( EV_EYEMEET_MOVE_WORK *work )
{
  MMDL_EndAcmd( work->mmdl );

#if 0 //pl null ゴーストジム用特殊処理
  if( GYM_GimmickCodeCheck(work->fsys, FLD_GIMMICK_GHOST_GYM) == FALSE 
      || GYM_PlGhostGymTrainerMoveCodeChange(work->fsys,work->fldobj) == FALSE ){
    FieldOBJ_MoveCodeChange( work->fldobj, MV_DMY );
  }
#endif
  
  work->seq_no = SEQNO_TRMOVE_END;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 視線移動イベント　終了
 * @param  work  EV_EYEMEET_MOVE_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int eyeMeetMove_End( EV_EYEMEET_MOVE_WORK *work )
{
  work->end_flag = TRUE;
  return( FALSE );
}

//--------------------------------------------------------------
///  視線移動イベント処理テーブル
//--------------------------------------------------------------
static int (* const data_EyeMeetMoveFuncTbl[])( EV_EYEMEET_MOVE_WORK* ) =
{
  eyeMeetMove_Init,
  eyeMeetMove_OBJMoveWait,
  eyeMeetMove_JikiMoveWait,
  eyeMeetMove_DirChange,
  eyeMeetMove_DirChangeWait,
  eyeMeetMove_GyoeSet,
  eyeMeetMove_GyoeWait,
  eyeMeetMove_HidePullOFFSet,
  eyeMeetMove_HidePullOFFWait,
  eyeMeetMove_GyoeEndWait,
  eyeMeetMove_MoveRangeCheck,
  eyeMeetMove_MoveStart,
  eyeMeetMove_Move,
  eyeMeetMove_MoveEndWait,
  eyeMeetMove_JikiTurnSet,
  eyeMeetMove_JikiTurn,
  eyeMeetMove_AcmdEnd,
  eyeMeetMove_End,
};

//--------------------------------------------------------------
/**
 * 視線移動処理　TCB
 * @param  tcb    TCB_PTR
 * @param  wk    TCB work
 * @retval  nothing
 */
//--------------------------------------------------------------
static GMEVENT_RESULT treyeEvent_EyeMeetMove(
    GMEVENT *ev, int *seq, void *wk )
{
  EV_EYEMEET_MOVE_WORK *work = wk;
  
  while( data_EyeMeetMoveFuncTbl[work->seq_no](work) == TRUE ){};
  
  if( work->end_flag == TRUE ){
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 対象トレーナータイプを取得 
 * @param mmdl トレーナーMMDL
 * @retval u16 EV_TYPE_TRAINER
 */
//--------------------------------------------------------------
static u16 tr_GetTrainerEventType( const MMDL *mmdl )
{
  u16 type = MMDL_GetEventType( mmdl );
  
  switch( type ){
  case EV_TYPE_TRAINER_KYORO:
  case EV_TYPE_TRAINER_SPIN_STOP_L:
  case EV_TYPE_TRAINER_SPIN_STOP_R:
  case EV_TYPE_TRAINER_SPIN_MOVE_L:
  case EV_TYPE_TRAINER_SPIN_MOVE_R:
    type = EV_TYPE_TRAINER;
    break;
  }
  
  return( type );
}

//--------------------------------------------------------------
/**
 * 視線範囲移動判定
 * @param  mmdl MMDL
 * @param  dir      移動方向
 * @param  range    移動距離
 * @retval  int     TRUE=移動不可
 */
//--------------------------------------------------------------
static BOOL tr_HitCheckEyeLine( const MMDL *mmdl, u16 dir, int range, const FIELD_ENCOUNT* enc )
{
  s16 gx,gy,gz;
  s16 dx,dz;
  u32 ret;
  
  if( range ) //レンジ0=目の前
  {
    int i = 0;
    u32 hit;
    MMDL_GRIDPOS pos;
    MMDL_GetGridPos( mmdl, &pos );
    
    dx = MMDL_TOOL_GetDirAddValueGridX(dir);
    dz = MMDL_TOOL_GetDirAddValueGridZ(dir);
    pos.gx += dx;
    pos.gz += dz;
    
    for( ; i < (range-1); i++ ){
      hit = MMDL_HitCheckMoveCurrent( mmdl, pos.gx, pos.gy, pos.gz, dir );
      hit &= ~MMDL_MOVEHITBIT_LIM; //移動制限を無視する
      
      if( hit ){ //移動制限以外でヒット
        return( TRUE );
      }
      
      if( enc != NULL ){ //エフェクトエンカウント座標チェック
        if( EFFECT_ENC_CheckEffectPos( enc, &pos )){
          return( TRUE );
        }
      }
      
      pos.gx += dx;
      pos.gz += dz;
    }
    
    hit = MMDL_HitCheckMoveCurrent( //最後
        mmdl, pos.gx, pos.gy, pos.gz, dir );
    
    hit &= ~MMDL_MOVEHITBIT_LIM; //移動制限を無視する
    
    if( hit != MMDL_MOVEHITBIT_OBJ ){ //OBJ衝突(自機)のみ
      return( TRUE );
    }
    
    if( enc != NULL ){ //エフェクトエンカウント座標チェック
      if( EFFECT_ENC_CheckEffectPos(enc,&pos) ){
        return( TRUE );
      }
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 動作モデルからトレーナーID取得
 * @param mmdl MMDL*
 * @retval u16 トレーナーID
 */
//--------------------------------------------------------------
static u16 tr_GetTrainerID( const MMDL *mmdl )
{
  u16 scr_id = MMDL_GetEventID( mmdl );
  return( SCRIPT_GetTrainerID_ByScriptID(scr_id) );
}

//--------------------------------------------------------------
/**
 * 動作モデルからトレーナーID取得　グローバル
 * @param mmdl MMDL*
 * @retval u16 トレーナーID
 */
//--------------------------------------------------------------
u16 EVENT_GetTrainerEyeTrainerID( const MMDL *mmdl )
{
  return( tr_GetTrainerID(mmdl) );
}

//--------------------------------------------------------------
/**
 * トレーナーイベントフラグチェック
 * @param fieldMap FIELDMAP_WORK
 * @param mmdl トレーナーMMDL
 * @retval BOOL TRUE=フラグON FALSE=フラグOFF
 */
//--------------------------------------------------------------
static BOOL tr_CheckEventFlag( FIELDMAP_WORK *fieldMap, const MMDL *mmdl )
{
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldMap );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
  u16 id = tr_GetTrainerID( mmdl );
  return( SCRIPT_CheckEventFlagTrainer(evwork,id) );
}

//--------------------------------------------------------------
/**
 * EYEMEET_HITDATA初期化
 * @param data EYEMEET_HITDATA
 * @param mmdl MMDL
 * @param range 視線距離
 * @param dir 移動方向
 * @retval nothing
 */
//--------------------------------------------------------------
static void tr_InitEyeMeetHitData(
    EYEMEET_HITDATA *data, MMDL *mmdl, int range, u16 dir )
{
  data->range = range;
  data->dir = dir;
  data->scr_id = MMDL_GetEventID( mmdl );
  data->tr_id = tr_GetTrainerID( mmdl );
  data->tr_type = SCRIPT_CheckTrainer2vs2Type( data->tr_id );
  data->mmdl = mmdl;
}

//--------------------------------------------------------------
/**
 * 同一のトレーナーIDを持つ相方を探す
 * @param mmdl 相方を探しているMMDL*
 * @param tr_id トレーナーID
 * @retval MMDL 相方MMDL* 見つからない場合はASSERT
 */
//--------------------------------------------------------------
static MMDL * tr_CheckPairTrainer( const MMDL *tr_mmdl, u16 tr_id )
{
  u16 type;
  MMDL *mmdl;
  u32 no = 0;
  const MMDLSYS *mmdlsys = MMDL_GetMMdlSys( tr_mmdl );
  
  while( MMDLSYS_SearchUseMMdl(mmdlsys,&mmdl,&no) == TRUE ){
    if( mmdl != tr_mmdl ){
      type = tr_GetTrainerEventType( mmdl );

      if( type == EV_TYPE_TRAINER || type == EV_TYPE_TRAINER_EYEALL ){
        if( tr_id == tr_GetTrainerID(mmdl) ){
          return( mmdl );
        }
      }
    }
  }
  
  GF_ASSERT( 0 && "TRAINER EYE NON PAIR" );
  return( NULL );
}

//--------------------------------------------------------------
/**
 * トレーナー視線イベントスクリプトセット
 * @param fieldMap FIELDMAP_WORK*
 * @param mmdl トレーナーMMDL*
 * @param id スクリプトID
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
static GMEVENT * tr_SetEventScript( FIELDMAP_WORK *fieldMap, MMDL *mmdl )
{
  GMEVENT *event;
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldMap );
  HEAPID heapID = FIELDMAP_GetHeapID( fieldMap );
  event = SCRIPT_SetEventScript(
      gsys, SCRID_TRAINER_MOVE_BATTLE, mmdl, GFL_HEAPID_APP );
  return( event );
}

//--------------------------------------------------------------
/**
 * 視線情報をスクリプトイベントへセット
 * @param event 起動したGMEVENT*
 * @param data EYEMEET_HITDATA
 * @param tr_type トレーナー視線タイプ。SCR_EYE_TR_TYPE_SINGLE等
 * @param tr_num トレーナー番号。SCR_EYE_TR_0等
 * @retval nothing
 */
//--------------------------------------------------------------
static void trEventScript_SetTrainerEyeData(
    GMEVENT *event, const EYEMEET_HITDATA *data, int tr_type, int tr_num )
{
  SCRIPT_SetTrainerEyeData( event, data->mmdl,
    data->range, data->dir, data->scr_id, data->tr_id,
    tr_type, tr_num );
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
/// 視線チェックテーブル
//--------------------------------------------------------------
static int (* const data_treye_CheckEyeLineTbl[DIR_MAX4])(
    s16,s16,s16,s16,s16,s16,s16) =
{
  treye_CheckEyeLineUp,
  treye_CheckEyeLineDown,
  treye_CheckEyeLineLeft,
  treye_CheckEyeLineRight,
};
