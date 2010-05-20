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

#include "script_trainer.h"
//SCRID_TRAINER_MOVE_BATTLE 
#include "../../../resource/fldmapdata/script/trainer_def.h"

#include "trainer_eye_data.h"  //TRAINER_HITDATA

#include "script_def.h"

#include "field_event_check.h"

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
/**
 * @brief 視線ヒット移動制御ワーク
 */
//--------------------------------------------------------------
struct _EV_EYEMEET_MOVE_WORK
{
  int seq_no;        ///<処理番号
  BOOL end_flag;      ///<終了フラグ
   
  TRAINER_HITDATA hitdata;

  int gyoe_type;    ///<!タイプ
  u32 sisen_no;      ///<視線番号
  int count;        ///<移動カウント
   
  FLDEFF_TASK *task_gyoe;   ////<ギョエータスク
  FIELDMAP_WORK *fieldMap;  ///<FILEDMAP_WORK*
  FIELD_PLAYER *fieldPlayer; ///<FIELD_PLAYER*
};

//--------------------------------------------------------------
/**
 * @brief 視線ヒット移動制御イベント用ワーク
 */
//--------------------------------------------------------------
typedef struct {
  EV_EYEMEET_MOVE_WORK * eye0;  ///<トレーナー０用
  EV_EYEMEET_MOVE_WORK * eye1;  ///<トレーナー１用
}TRAINER_MOVE_EVENT_WORK;

//======================================================================
//  proto
//======================================================================
static BOOL treye_CheckEyeMeet(
    FIELDMAP_WORK *fieldMap, MMDL *non_mmdl, TRAINER_HITDATA *hit );
static int treye_CheckEyeRange( const FIELD_ENCOUNT *enc,
    const MMDL *mmdl, const MMDL *jiki, u16 *hitDir );

static int treye_CheckEyeLine( u16 dir, s16 range,
    const MMDL_GRIDPOS *pos0, const MMDL_GRIDPOS *pos1 );

static u16 tr_GetTrainerEventType( const MMDL *mmdl );
static BOOL tr_HitCheckEyeLine( const MMDL *mmdl, u16 dir, int range, const FIELD_ENCOUNT* enc );
static BOOL tr_CheckEventFlag( FIELDMAP_WORK *fieldMap, const MMDL *mmdl );
static void makeHitData( TRAINER_HITDATA *data, MMDL *mmdl, int range, u16 dir );
static MMDL * tr_CheckPairTrainer( const MMDL *tr_mmdl, u16 tr_id );
static GMEVENT * createTrainerScript( FIELDMAP_WORK *fieldMap, MMDL *mmdl );

static int (* const data_treye_CheckEyeLineTbl[DIR_MAX4])(
    s16 range,const MMDL_GRIDPOS *t_pos,const MMDL_GRIDPOS *j_pos );

static GMEVENT_RESULT controlTrainerEyeMoveEvent(
    GMEVENT * ev, int *seq, void * wk );

//======================================================================
//  トレーナー視線イベント
//======================================================================
//--------------------------------------------------------------
/**
 * トレーナー視線イベントチェック
 * @param fieldMap FIELDMAP_WORK*
 * @retval GMEVENT NULL=ヒット無し
 */
//--------------------------------------------------------------
GMEVENT * EVENT_CheckTrainerEye( FIELDMAP_WORK *fieldMap )
{
  TRAINER_HITDATA hit0;
  GMEVENT *event = NULL;
  
  if( treye_CheckEyeMeet(fieldMap,NULL,&hit0) == TRUE )
  {
    TRAINER_HITDATA hit1;
    BtlRule btl_rule = SCRIPT_GetTrainerBtlRule( hit0.tr_id );
    u32 enable_member = FIELD_EVENT_CountBattleMember( FIELDMAP_GetGameSysWork(fieldMap) );
    
    if( btl_rule == BTL_RULE_SINGLE ) //シングル
    {
      if( enable_member < 2 || //シングル戦チェック
          treye_CheckEyeMeet(fieldMap,hit0.mmdl,&hit1) == FALSE )
      { //スクリプト用イベントデータセット シングル
        event = createTrainerScript( fieldMap, hit0.mmdl ); //スクリプト起動
        hit0.move_type = SCR_EYE_TR_TYPE_SINGLE;
        SCRIPT_TRAINER_SetHitData( event, SCR_EYE_TR_0, &hit0 );
      }
      else
      { //スクリプト用イベントデータセット タッグ
        event = createTrainerScript( fieldMap, hit0.mmdl ); //スクリプト起動
        hit0.move_type = SCR_EYE_TR_TYPE_TAG;
        SCRIPT_TRAINER_SetHitData( event, SCR_EYE_TR_0, &hit0 );
        hit1.move_type = SCR_EYE_TR_TYPE_TAG;
        SCRIPT_TRAINER_SetHitData( event, SCR_EYE_TR_1, &hit1 );
      }
    }
    else if( btl_rule == BTL_RULE_DOUBLE )
    {
      if( enable_member >= 2 ) //ダブル可能
      { //スクリプト用イベントデータセット ダブル
        MMDL *mmdl = tr_CheckPairTrainer( hit0.mmdl, hit0.tr_id );
        makeHitData( &hit1, mmdl, hit0.range, hit0.dir );
        
        event = createTrainerScript( fieldMap, hit0.mmdl ); //スクリプト起動
        hit0.move_type = SCR_EYE_TR_TYPE_DOUBLE;
        SCRIPT_TRAINER_SetHitData( event, SCR_EYE_TR_0, &hit0 );
        hit1.move_type = SCR_EYE_TR_TYPE_DOUBLE;
        SCRIPT_TRAINER_SetHitData( event, SCR_EYE_TR_1, &hit1 );
      }
    }
    else if ( btl_rule == BTL_RULE_TRIPLE || btl_rule == BTL_RULE_ROTATION )
    {
      if (enable_member >= 3 )  //3vs3可能
      { //スクリプト用イベントデータセット　トリプルorローテーション
        event = createTrainerScript( fieldMap, hit0.mmdl ); //スクリプト起動
        hit0.move_type = SCR_EYE_TR_TYPE_SINGLE;
        SCRIPT_TRAINER_SetHitData( event, SCR_EYE_TR_0, &hit0 );
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
int TRAINER_MMDL_CheckEyeRange(
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
  
  MMDL_GetGridPos( jiki, &pj );
  MMDL_GetGridPos( mmdl, &pt );
  
  ret = treye_CheckEyeLine( eye_dir, eye_range, &pt, &pj );
  
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
    FIELDMAP_WORK *fieldMap, MMDL *non_mmdl, TRAINER_HITDATA *hit )
{
  u16 dir;
  int range;
  const FIELD_ENCOUNT *enc = FIELDMAP_GetEncount( fieldMap );
  MMDL *jiki = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer(fieldMap) );
  MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldMap );
  MMDL *mmdl = NULL;
  u32 no = 0;
  
  while( MMDLSYS_SearchUseMMdl( mmdlsys, &mmdl, &no ) == TRUE )
  {
    if( mmdl != jiki )
    {
      if( non_mmdl == NULL || (non_mmdl != mmdl) )
      {
        range = treye_CheckEyeRange( enc, mmdl, jiki, &dir );
      
        if( range != EYE_CHECK_NOHIT )
        {
          if( tr_CheckEventFlag(fieldMap,mmdl) == FALSE )
          {
            makeHitData( hit, mmdl, range, dir );
            return( TRUE );
          }
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
static int treye_CheckEyeRange( const FIELD_ENCOUNT *enc,
    const MMDL *mmdl, const MMDL *jiki, u16 *hitDir )
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
      ret = treye_CheckEyeLine( dir, range, &pt, &pj );
      
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
        ret = treye_CheckEyeLine( dir, range, &pt, &pj );
        
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
static int treye_CheckEyeLine( u16 dir, s16 range,
    const MMDL_GRIDPOS *t_pos, const MMDL_GRIDPOS *j_pos )
{
  int hit;
  GF_ASSERT( dir < DIR_MAX4 );
  hit = data_treye_CheckEyeLineTbl[dir]( range, t_pos, j_pos );
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
static int treye_CheckEyeLineUp( s16 range,
    const MMDL_GRIDPOS *t_pos, const MMDL_GRIDPOS *j_pos )
{
  if( t_pos->gx == j_pos->gx ){
    if( t_pos->gy == j_pos->gy ){
      if( j_pos->gz < t_pos->gz && j_pos->gz >= (t_pos->gz-range) ){
        return( t_pos->gz - j_pos->gz );
      }
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
static int treye_CheckEyeLineDown( s16 range, 
    const MMDL_GRIDPOS *t_pos, const MMDL_GRIDPOS *j_pos )
{
  if( t_pos->gx == j_pos->gx ){
    if( t_pos->gy == j_pos->gy ){
      if( j_pos->gz > t_pos->gz && j_pos->gz <= (t_pos->gz+range) ){
        return( j_pos->gz - t_pos->gz );
      }
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
static int treye_CheckEyeLineLeft( s16 range, 
    const MMDL_GRIDPOS *t_pos, const MMDL_GRIDPOS *j_pos )
{
  if( t_pos->gz == j_pos->gz ){
    if( t_pos->gy == j_pos->gy ){
      if( j_pos->gx < t_pos->gx && j_pos->gx >= (t_pos->gx-range) ){
        return( t_pos->gx - j_pos->gx );
      }
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
static int treye_CheckEyeLineRight( s16 range, 
    const MMDL_GRIDPOS *t_pos, const MMDL_GRIDPOS *j_pos )
{
  if( t_pos->gz == j_pos->gz ){
    if( t_pos->gy == j_pos->gy ){
      if( j_pos->gx > t_pos->gx && j_pos->gx <= (t_pos->gx+range) ){
        return( j_pos->gx - t_pos->gx );
      }
    }
  }
  return( EYE_CHECK_NOHIT );
}


//======================================================================
//
//
//  視線ヒットによる移動イベント
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 視線ヒット移動制御イベントの生成
 * @param gsys
 * @param eye0  EV_EYEMEET_MOVE_WORK
 * @param eye1  EV_EYEMEET_MOVE_WORK
 * @return  GMEVENT 生成したイベント
 */
//--------------------------------------------------------------
GMEVENT * EVENT_TrainerEyeMoveControl(
    GAMESYS_WORK * gsys, EV_EYEMEET_MOVE_WORK * eye0, EV_EYEMEET_MOVE_WORK * eye1 )
{
  GMEVENT * event;
  TRAINER_MOVE_EVENT_WORK * tmew;
  event = GMEVENT_Create( gsys, NULL, controlTrainerEyeMoveEvent, sizeof(TRAINER_MOVE_EVENT_WORK) );
  tmew = GMEVENT_GetEventWork( event );
  tmew->eye0 = eye0;
  tmew->eye1 = eye1;

  return event;
}
//--------------------------------------------------------------
/**
 * 視線ヒット移動制御用ワークの生成
 * @param   heapID
 * @param   fieldMap
 * @param   hitdata
 * @param   work_pos 視線ヒットワーク要素数 0=視線主 1=ペア
 * @retval  GMEVENT 移動処理を行うGMEVENT
 */
//--------------------------------------------------------------
EV_EYEMEET_MOVE_WORK * TRAINER_EYEMOVE_Create( HEAPID heapID,
    FIELDMAP_WORK *fieldMap, const TRAINER_HITDATA * hitdata, u32 work_pos )
{
  EV_EYEMEET_MOVE_WORK * work;
  work = GFL_HEAP_AllocClearMemory( heapID, sizeof(EV_EYEMEET_MOVE_WORK) );
  
  work->hitdata = *hitdata;
  work->gyoe_type = 0;      //現在未使用
  work->sisen_no = work_pos;
  work->fieldMap = fieldMap;
  work->fieldPlayer = FIELDMAP_GetFieldPlayer( fieldMap );
  
  return  work;
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
  if( MMDL_CheckMoveBitMove(work->hitdata.mmdl) == TRUE ){ //動作中
    MMDL_OffMoveBitMoveProcPause( work->hitdata.mmdl ); //動作ポーズ解除
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
  if( MMDL_CheckMoveBitMove(work->hitdata.mmdl) == TRUE ){
    return( FALSE ); //動作中
  }
  
  MMDL_SetDirDisp( work->hitdata.mmdl, work->hitdata.dir );
  MMDL_OnMoveBitMoveProcPause( work->hitdata.mmdl );
  
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
    u32 code = MMDL_GetMoveCode( work->hitdata.mmdl );
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
  
  if( MMDL_CheckPossibleAcmd(work->hitdata.mmdl) == TRUE ){
    u16 code;
    GF_ASSERT( work->hitdata.dir < DIR_MAX4 );
    code = MMDL_ChangeDirAcmdCode( work->hitdata.dir, AC_DIR_U );
    MMDL_SetAcmd( work->hitdata.mmdl, code );
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
  if( MMDL_CheckEndAcmd(work->hitdata.mmdl) == FALSE ){
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
      fectrl, work->hitdata.mmdl, FLDEFF_GYOETYPE_GYOE, FALSE ); //SEは鳴らさない
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
  MMDL_SetAcmd( work->hitdata.mmdl, AC_HIDE_PULLOFF );
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
  if( MMDL_CheckEndAcmd(work->hitdata.mmdl) == TRUE ){
    work->seq_no = SEQNO_TRMOVE_GYOE_END_WAIT;
  }
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
  if( work->hitdata.range <= 1 ){                  //自機目の前 移動する必要なし
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
  if( MMDL_CheckPossibleAcmd(work->hitdata.mmdl) == TRUE ){
    u16 code = MMDL_ChangeDirAcmdCode( work->hitdata.dir, AC_WALK_U_8F );
    MMDL_SetAcmd( work->hitdata.mmdl, code );
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
  if( MMDL_CheckEndAcmd(work->hitdata.mmdl) == FALSE ){
    return( FALSE );
  }
  
  work->hitdata.range--;                      //一歩減らす
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
      MMDL_GetGridPosX(work->hitdata.mmdl), MMDL_GetGridPosZ(work->hitdata.mmdl) );
  
  if( MMDL_GetDirDisp(j_mmdl) != turn_dir &&
    (work->sisen_no == 0 || work->hitdata.move_type == SCR_EYE_TR_TYPE_TAG) )
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
  MMDL_EndAcmd( work->hitdata.mmdl );

  //戦闘終了後からは移動しないように、動作コードをMV_DMYにする
  MMDL_ChangeMoveCode( work->hitdata.mmdl, MV_DMY );
 
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
static BOOL trainerMoveTask( EV_EYEMEET_MOVE_WORK * work )
{
  while( data_EyeMeetMoveFuncTbl[work->seq_no](work) == TRUE ){};
  
  return( work->end_flag == TRUE );
}

//--------------------------------------------------------------
/**
 * @brief 視線ヒット移動制御イベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT controlTrainerEyeMoveEvent( GMEVENT * ev, int *seq, void * wk )
{
  TRAINER_MOVE_EVENT_WORK * tmew = wk;

  if ( tmew->eye0 && trainerMoveTask( tmew->eye0 ) == TRUE )
  {
    GFL_HEAP_FreeMemory( tmew->eye0 );
    tmew->eye0 = NULL;
  }

  if ( tmew->eye1 && trainerMoveTask( tmew->eye1 ) == TRUE )
  {
    GFL_HEAP_FreeMemory( tmew->eye1 );
    tmew->eye1 = NULL;
  }
  if ( tmew->eye0 == NULL && tmew->eye1 == NULL )
  {
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
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
  return( MMDL_TOOL_GetTrainerEventType(type) );
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
  if( range ) //レンジ0=目の前
  {
    int i = 0;
    u32 hit;
    s16 dx,dz;
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
 * @brief   動作モデルからトレーナーID取得
 * @param   mmdl MMDL*
 * @retval  u16 トレーナーID
 */
//--------------------------------------------------------------
u16 TRAINER_MMDL_GetTrainerID( const MMDL *mmdl )
{
  u16 scr_id = MMDL_GetEventID( mmdl );
  return( SCRIPT_GetTrainerID_ByScriptID(scr_id) );
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
  u16 id = TRAINER_MMDL_GetTrainerID( mmdl );
  return( SCRIPT_CheckEventFlagTrainer(evwork,id) );
}

//--------------------------------------------------------------
/**
 * TRAINER_HITDATA初期化
 * @param data TRAINER_HITDATA
 * @param mmdl MMDL
 * @param range 視線距離
 * @param dir 移動方向
 * @retval nothing
 */
//--------------------------------------------------------------
static void makeHitData(
    TRAINER_HITDATA *data, MMDL *mmdl, int range, u16 dir )
{
  data->range = range;
  data->dir = dir;
  data->scr_id = MMDL_GetEventID( mmdl );
  data->tr_id = TRAINER_MMDL_GetTrainerID( mmdl );
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
        if( tr_id == TRAINER_MMDL_GetTrainerID(mmdl) ){
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
static GMEVENT * createTrainerScript( FIELDMAP_WORK *fieldMap, MMDL *mmdl )
{
  GMEVENT *event;
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldMap );
  event = SCRIPT_SetEventScript(
      gsys, SCRID_TRAINER_MOVE_BATTLE, mmdl, GFL_HEAPID_APP );
  return( event );
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
/// 視線チェックテーブル
//--------------------------------------------------------------
static int (* const data_treye_CheckEyeLineTbl[DIR_MAX4])(
    s16 range,const MMDL_GRIDPOS *t_pos, const MMDL_GRIDPOS *j_pos ) =
{
  treye_CheckEyeLineUp,
  treye_CheckEyeLineDown,
  treye_CheckEyeLineLeft,
  treye_CheckEyeLineRight,
};
