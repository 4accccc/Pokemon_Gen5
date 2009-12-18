//======================================================================
/**
 * @file  field_player_grid.c
 * @brief グリッド専用 フィールドプレイヤー制御
 * @author kagaya
 */
//======================================================================
#include <gflib.h>

#include "fieldmap.h"
#include "fldmmdl.h"
#include "field_player.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "fieldmap_ctrl.h"
#include "fieldmap_ctrl_grid.h"
#include "field_player_grid.h"
#include "map_attr.h"

#include "fldeff_namipoke.h"

#include "field_sound.h"

#include "fldeff_kemuri.h"

//======================================================================
//  define
//======================================================================
//#define PLAYER_MOVE_TRACK_CHANGE //定義で自機動作時にトラック変更

///自機専用ヒットチェックビットを出す
#define EX_MOVEHITBIT(bit_no) (1<<(MMDL_MOVEHITBIT_BITMAX+bit_no))

///尾瀬ゆらゆら限界フレーム
#define OZE_YURE_MAXFRAME (30*2)

//--------------------------------------------------------------
/// JIKI_MOVEBIT
//--------------------------------------------------------------
typedef enum
{
  ///特に無し
  JIKI_MOVEBIT_NON = (0),
  ///強制移動中
  JIKI_MOVEBIT_FORCE       = (1<<0),
  ///足元無効
  JIKI_MOVEBIT_UNDER_OFF   = (1<<1),
  ///一歩移動
  JIKI_MOVEBIT_STEP        = (1<<2),
  ///尾瀬ゆれ状態
  JIKI_MOVEBIT_OZE_YURE    = (1<<3),
  ///尾瀬落下中
  JIKI_MOVEBIT_OZE_FALLOUT = (1<<4),
  ///最大
  JIKI_MOVEBIT_BITMAX      = (1<<5),
}JIKI_MOVEBIT;

//--------------------------------------------------------------
///  JIKI_MOVEORDER
//--------------------------------------------------------------
typedef enum
{
  //基本移動
  JIKI_MOVEORDER_NON = 0, ///<セット無し
  JIKI_MOVEORDER_STOP,    ///<停止セット
  JIKI_MOVEORDER_WALK,    ///<歩きセット
  JIKI_MOVEORDER_TURN,    ///<振り向きセット
  JIKI_MOVEORDER_HITCH,   ///<障害物ヒットセット
  JIKI_MOVEORDER_JUMP,    ///<ジャンプセット
  
  //通常移動　尾瀬
  JIKI_MOVEORDER_OZE_YURE, ///<尾瀬ゆれ
  JIKI_MOVEORDER_OZE_FALLOUT_JUMP_START, ///<尾瀬落下セット
  JIKI_MOVEORDER_OZE_FALLOUT_JUMP_WAIT, ///<尾瀬落下開始
  JIKI_MOVEORDER_OZE_FALLOUT, ///<尾瀬落下中
}JIKI_MOVEORDER;


//--------------------------------------------------------------
///  JIKI_ACTION
//--------------------------------------------------------------
typedef enum
{
  //基本移動
  JIKI_ACTION_STOP = 0,
  JIKI_ACTION_WALK,
  JIKI_ACTION_TURN,
  JIKI_ACTION_HITCH,
  
  //通常移動　尾瀬
  JIKI_ACTION_OZE_YURE,
  JIKI_ACTION_OZE_FALLOUT_JUMP,
  JIKI_ACTION_OZE_FALLOUT_JUMP_WAIT,
  JIKI_ACTION_OZE_FALLOUT,
}JIKI_ACTION;

//--------------------------------------------------------------
/// JIKI_MOVEHITBIT 
//--------------------------------------------------------------
typedef enum
{
  JIKI_MOVEHITBIT_EXIT  =  EX_MOVEHITBIT(0), ///<出入り口ヒット
  JIKI_MOVEHITBIT_WATER =  EX_MOVEHITBIT(1), ///<水アトリビュートヒット
  JIKI_MOVEHITBIT_JUMP  =  EX_MOVEHITBIT(2), ///<ジャンプアトリビュートヒット
  JIKI_MOVEHITBIT_OZE   =  EX_MOVEHITBIT(3), ///<尾瀬アトリビュートヒット
  
  JIKI_MOVEHITBIT_CHECK_OVER = (MMDL_MOVEHITBIT_BITMAX+4),
  
#if 0
  JIKI_MOVE_HIT_BIT_TAKE_OFF  (1<<4)          ///<ジャンプ台ヒット
  JIKI_MOVE_HIT_BIT_BRIDGEHV  (1<<6)          ///<一本橋ヒット
  JIKI_MOVE_HIT_BIT_JUMP_3G  (1<<7)          ///<3Gジャンプ
#endif
}JIKI_MOVEHITBIT;

//--------------------------------------------------------------
/// UNDER
//--------------------------------------------------------------
typedef enum
{
  UNDER_NONE = 0, //何も無し
  UNDER_ICE, //滑る床
  UNDER_MAX,
}UNDER;

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// FIELD_PLAYER_GRID
//--------------------------------------------------------------
struct _TAG_FIELD_PLAYER_GRID
{
  JIKI_ACTION move_action;
  JIKI_MOVEORDER move_order;
  
  JIKI_MOVEBIT move_bit;
  FIELD_PLAYER_REQBIT req_bit;
  
  FIELD_PLAYER *fld_player;
  FIELDMAP_WORK *fieldWork;
  FLDEFF_TASK *fldeff_joint;
  
  u16 input_key_dir_x; //キー入力横方向
  u16 input_key_dir_z; //キー入力縦方向
  
  u16 oze_yure_frame;
  u16 oze_anime_reset_flag;
};

//======================================================================
//  proto
//======================================================================
static void gjiki_InitMoveStartCommon(
    FIELD_PLAYER_GRID *gjiki, u16 key_prs, JIKI_MOVEORDER set );
static BOOL gjiki_CheckMoveStart( FIELD_PLAYER_GRID *gjiki, u16 dir );
static void gjiki_PlaySE( FIELD_PLAYER_GRID *gjiki, JIKI_MOVEORDER set, u16 dir );

//キー入力処理
static u16 gjiki_GetInputKeyDir(
    FIELD_PLAYER_GRID *gjiki, u16 key_prs );
static void gjiki_SetInputKeyDir( FIELD_PLAYER_GRID *gjiki, u16 key_prs );
static u16 getKeyDirX( u16 key_prs );
static u16 getKeyDirZ( u16 key_prs );

//移動開始状態
static JIKI_MOVEORDER gjiki_GetMoveOrder( FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag );

static JIKI_MOVEORDER gjiki_GetMoveOrder_Normal(
    FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjiki_CheckMoveOrder_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjiki_CheckMoveOrder_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjiki_CheckMoveOrder_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjiki_CheckMoveOrder_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static JIKI_MOVEORDER gjiki_GetMoveOrder_Oze( FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_Yure(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_WaitFallOutJump(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_FallOut(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static JIKI_MOVEORDER gjiki_GetMoveOrder_Cycle( FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiCycle_CheckMoveOrder_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiCycle_CheckMoveOrder_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiCycle_CheckMoveOrder_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiCycle_CheckMoveOrder_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static JIKI_MOVEORDER gjiki_GetMoveOrder_Swim( FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

//移動セット
static void gjiki_SetMove( FIELD_PLAYER_GRID *gjiki, JIKI_MOVEORDER set,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjiki_SetMove_Normal( FIELD_PLAYER_GRID *gjiki, JIKI_MOVEORDER set,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Non(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Jump(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiOze_SetMove_Yure(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiOze_SetMove_FallOutJumpStart(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiOze_SetMove_FallOutJumpWait(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiOze_SetMove_FallOut(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjiki_SetMove_Cycle( FIELD_PLAYER_GRID *gjiki,
    JIKI_MOVEORDER set,
    u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Non(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Jump(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjiki_SetMove_Swim( FIELD_PLAYER_GRID *gjiki,
    JIKI_MOVEORDER set,
    u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjikiSwim_SetMove_Non(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiSwim_SetMove_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiSwim_SetMove_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiSwim_SetMove_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiSwim_SetMove_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

//リクエスト
static void (* const data_gjikiRequestProcTbl[
    FIELD_PLAYER_REQBIT_MAX])( FIELD_PLAYER_GRID *gjiki );

//ヒットチェック
static u32 gjiki_HitCheckMove(
    FIELD_PLAYER_GRID *gjiki, MMDL *mmdl, u16 dir, MAPATTR *attr );

//サウンド
#if 0
static void gjiki_Sound_MoveStop( void );
static void gjiki_Sound_Move( void );
#endif

//自機動作ビット
static void gjiki_OnMoveBit(
    FIELD_PLAYER_GRID *gjiki, JIKI_MOVEBIT bit );
static void gjiki_OffMoveBit(
    FIELD_PLAYER_GRID *gjiki, JIKI_MOVEBIT bit );
static BOOL gjiki_CheckMoveBit(
    FIELD_PLAYER_GRID *gjiki, JIKI_MOVEBIT bit );
static void gjiki_OnMoveBitForce( FIELD_PLAYER_GRID *gjiki );
static void gjiki_OffMoveBitForce( FIELD_PLAYER_GRID *gjiki );
static BOOL gjiki_CheckMoveBitForce( FIELD_PLAYER_GRID *gjiki );
static void gjiki_OnMoveBitUnderOff( FIELD_PLAYER_GRID *gjiki );
static void gjiki_OffMoveBitUnderOff( FIELD_PLAYER_GRID *gjiki );
static void gjiki_OnMoveBitStep( FIELD_PLAYER_GRID *gjiki );
static void gjiki_OffMoveBitStep( FIELD_PLAYER_GRID *gjiki );
static void gjiki_SetMoveBitOzeYure( FIELD_PLAYER_GRID *gjiki, BOOL on );
static BOOL gjiki_CheckMoveBitOzeYure( FIELD_PLAYER_GRID *gjiki );
static void gjiki_SetMoveBitOzeFallOut( FIELD_PLAYER_GRID *gjiki, BOOL on );
static BOOL gjiki_CheckMoveBitOzeFallOut( FIELD_PLAYER_GRID *gjiki );

static UNDER gjiki_CheckUnder( FIELD_PLAYER_GRID *gjiki, u16 dir );

static BOOL gjiki_ControlUnder(
    FIELD_PLAYER_GRID *gjiki, u16 dir, BOOL debug );

//尾瀬
//static void oze_InitParam( FIELD_PLAYER_GRID *gjiki );
static BOOL oze_IsOzeAttr( const MAPATTR_VALUE val );
static BOOL oze_CheckAttrOzeMove( FIELD_PLAYER_GRID *gjiki, u16 dir );
static BOOL oze_CheckAttrUnderOze( FIELD_PLAYER_GRID *gjiki );
static u16 oze_CheckAttrOzeFallOut( FIELD_PLAYER_GRID *gjiki );
static u32 oze_HitCheckMove(
    FIELD_PLAYER_GRID *gjiki, MMDL *mmdl, u16 dir, MAPATTR *attr );
static BOOL oze_CheckFallOutEnd( FIELD_PLAYER_GRID *gjiki );
static void oze_StartYureJiki( FIELD_PLAYER_GRID *gjiki );
static void oze_EndYureJiki( FIELD_PLAYER_GRID *gjiki );

//parts
static BOOL gjiki_GetAttr( FIELD_PLAYER_GRID *gjiki, u16 dir, MAPATTR *attr );

//======================================================================
//  グリッド移動 フィールドプレイヤー制御
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　生成
 * @param  fld_player FIELD_PLAYER
 * @param heapID HEAPID
 * @retval FIELD_PLAYER_GRID
 */
//--------------------------------------------------------------
FIELD_PLAYER_GRID * FIELD_PLAYER_GRID_Init(
    FIELD_PLAYER *fld_player, HEAPID heapID )
{
  FIELD_PLAYER_GRID *gjiki;
  
  gjiki = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_PLAYER_GRID) );
  gjiki->fld_player = fld_player;
  gjiki->fieldWork = FIELD_PLAYER_GetFieldMapWork( fld_player );
  gjiki->input_key_dir_x = DIR_NOT;
  gjiki->input_key_dir_z = DIR_NOT;

  gjiki_OnMoveBitUnderOff( gjiki );
  
  //復帰
  {
    PLAYER_MOVE_FORM form;
    form = FIELD_PLAYER_GetMoveForm( gjiki->fld_player );
    
    switch( form ){
    case PLAYER_MOVE_FORM_SWIM:
      FIELD_PLAYER_GRID_SetRequest(
          gjiki, FIELD_PLAYER_REQBIT_SWIM );
      FIELD_PLAYER_GRID_UpdateRequest( gjiki );
      break;
    }
  }
  
  { //専用のヒットチェック用ビットが32bit以内か
    GF_ASSERT( JIKI_MOVEHITBIT_CHECK_OVER < 32 );
  }
  
//SetGridPlayerActTrans( gjiki->pActCont, &gjiki->vec_pos );
  return( gjiki );
}

//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　削除
 * @param  gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_Delete( FIELD_PLAYER_GRID *gjiki )
{
  GFL_HEAP_FreeMemory( gjiki );
}

//======================================================================
//  グリッド移動 フィールドプレイヤー制御　移動
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動 フィールドプレイヤー制御　移動
 * @param  gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_Move(
    FIELD_PLAYER_GRID *gjiki, int key_trg, int key_cont )
{
  u16 dir;
  JIKI_MOVEORDER order;
  BOOL debug_flag = FALSE;
  
  dir = gjiki_GetInputKeyDir( gjiki, key_cont );
  
  if( gjiki_CheckMoveStart(gjiki,dir) == FALSE ){
    return;
  }
  
#ifdef PM_DEBUG
  if( key_cont & PAD_BUTTON_R ){
    debug_flag = TRUE;
  }
#endif
  
  FIELD_PLAYER_GRID_UpdateRequest( gjiki );
  
  if( gjiki_ControlUnder(gjiki,dir,debug_flag) == TRUE ){
    return;
  }
  
  order = gjiki_GetMoveOrder( gjiki, key_trg, key_cont, dir, debug_flag );
  
  gjiki_InitMoveStartCommon( gjiki, key_cont, order );
  gjiki_SetMove( gjiki, order, key_trg, key_cont, dir, debug_flag );
  gjiki_PlaySE( gjiki, order, dir );
}

//--------------------------------------------------------------
/**
 * 移動開始時に行う初期化処理
 * @param gjiki FIELD_PLAYER_GRID
 * @param key_prs キー入力情報
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_InitMoveStartCommon(
    FIELD_PLAYER_GRID *gjiki, u16 key_prs, JIKI_MOVEORDER set )
{
  gjiki->move_order = set;
  gjiki_SetInputKeyDir( gjiki, key_prs );
  gjiki_OffMoveBitStep( gjiki );
  
  if( set == JIKI_MOVEORDER_WALK ){
    gjiki_OffMoveBitUnderOff( gjiki );
  }
}

//--------------------------------------------------------------
/**
 * オーダーチェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL gjiki_CheckMoveStart( FIELD_PLAYER_GRID *gjiki, u16 dir )
{
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  
  if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
    return( TRUE ); //セット可能
  }
  
  if( dir != DIR_NOT ){
    if( gjiki->move_action == JIKI_ACTION_HITCH ){
      if( dir != MMDL_GetDirDisp(mmdl) ){
        return( TRUE );
      }
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 移動開始時に鳴らすSE
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void gjiki_PlaySE( FIELD_PLAYER_GRID *gjiki,
    JIKI_MOVEORDER set, u16 dir )
{
  if( set == JIKI_MOVEORDER_WALK ){
    VecFx32 pos;
    MAPATTR attr;
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
    
    MMDL_GetVectorPos( mmdl, &pos );
    MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
    
    if( MMDL_GetMapPosAttr(mmdl,&pos,&attr) == TRUE ){
      int se;
      MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
      MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
      
      if( (flag & MAPATTR_FLAGBIT_GRASS) ){
        se = SEQ_SE_FLD_09;
        if( MAPATTR_VALUE_CheckLongGrass(val) == TRUE ){
          se = SEQ_SE_FLD_08;
        }
        PMSND_PlaySE( se );
      }else{
        if( MAPATTR_VALUE_CheckShoal(val) == TRUE ||
            MAPATTR_VALUE_CheckPool(val) == TRUE  ||
            MAPATTR_VALUE_CheckMarsh(val) == TRUE ){
          PMSND_PlaySE( SEQ_SE_FLD_13 );
        }
      }
    }
  }
}

//======================================================================
//  キー入力処理
//======================================================================
//--------------------------------------------------------------
/**
 * キー入力方向取得
 * @param gjiki
 * @param key_prs キー入力情報
 * @retval u16 キー入力方向
 */
//--------------------------------------------------------------
static u16 gjiki_GetInputKeyDir( FIELD_PLAYER_GRID *gjiki, u16 key_prs )
{
  int key_dir_x = getKeyDirX( key_prs );
  int key_dir_z = getKeyDirZ( key_prs );
  
  if( key_dir_x == DIR_NOT ){   //一方向押しの場合はZ優先
    return( key_dir_z );
  }
  
  if( key_dir_z == DIR_NOT ){   //Zキー無し Xキー返し
    return( key_dir_x );
  }
  
  { //斜め押し
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
    u16 move_dir = MMDL_GetDirMove( mmdl );
    u16 input_x = gjiki->input_key_dir_x;
    u16 input_z = gjiki->input_key_dir_z;
    
    //移動方向と一致しない方向をZ優先で返す
    if( move_dir != DIR_NOT ){
      //過去に押した方向を継続
      if( key_dir_x == input_x && key_dir_z == input_z ){
        return( move_dir ); //移動中と一致する方向を返す
      }
      
      if( key_dir_z != input_z ){  //新規斜め押しはZ優先で返す
        return( key_dir_z );
      }
      
      return( key_dir_x );
    }
    
    return( key_dir_z ); //Z優先で返す
  }
  
  return( DIR_NOT );
}

//--------------------------------------------------------------
/**
 * キー入力情報セット
 * @param gjiki FIELD_PLAYER_GRID
 * @param key_prs キー入力情報
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetInputKeyDir( FIELD_PLAYER_GRID *gjiki, u16 key_prs )
{
  gjiki->input_key_dir_x = getKeyDirX( key_prs );
  gjiki->input_key_dir_z = getKeyDirZ( key_prs );
}

//--------------------------------------------------------------
/**
 * キープレスから押されているX方向を取得
 * @param key_prs キー入力情報
 * @retval u16 押されている方向 DIR_UP等
 */
//--------------------------------------------------------------
static u16 getKeyDirX( u16 key_prs )
{
  if( (key_prs & PAD_KEY_LEFT) ){
    return( DIR_LEFT );
  }
  
  if( (key_prs & PAD_KEY_RIGHT) ){
    return( DIR_RIGHT );
  }
  
  return( DIR_NOT );
}

//--------------------------------------------------------------
/**
 * キープレスから押されているZ方向を取得
 * @param key_prs キー入力情報
 * @retval u16 押されている方向 DIR_UP等
 */
//--------------------------------------------------------------
static u16 getKeyDirZ( u16 key_prs )
{
  if( (key_prs & PAD_KEY_UP) ){
    return( DIR_UP );
  }
  
  if( (key_prs & PAD_KEY_DOWN) ){
    return( DIR_DOWN );
  }
  
  return( DIR_NOT );
}

//======================================================================
// キー入力による自機移動オーダーを取得
//======================================================================
//--------------------------------------------------------------
/**
 * キー入力による自機移動オーダーを取得
 * @param gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir キー入力方向
 * @param debug_flag デバッグ移動フラグ TRUE=デバッグ移動
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjiki_GetMoveOrder( FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag )
{
  JIKI_MOVEORDER set;
  PLAYER_MOVE_FORM form;
  
  set = JIKI_MOVEORDER_NON;
  form = FIELD_PLAYER_GetMoveForm( gjiki->fld_player );
  
  switch( form ){
  case PLAYER_MOVE_FORM_NORMAL:
    set = gjiki_GetMoveOrder_Normal(
        gjiki, key_trg, key_cont, dir, debug_flag );
    break;
  case PLAYER_MOVE_FORM_CYCLE:
    set = gjiki_GetMoveOrder_Cycle(
        gjiki, key_trg, key_cont, dir, debug_flag );
    break;
  case PLAYER_MOVE_FORM_SWIM:
    set = gjiki_GetMoveOrder_Swim(
        gjiki, key_trg, key_cont, dir, debug_flag );
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  return( set );
}

//--------------------------------------------------------------
/**
 * キー入力による自機移動オーダーを取得
 * @param gjiki FIELD_PLAYER_GRID
 * @param dir キー入力方向
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
PLAYER_MOVE_VALUE FIELD_PLAYER_GRID_GetMoveValue(
    FIELD_PLAYER_GRID *gjiki, u16 dir )
{
  PLAYER_MOVE_VALUE val = PLAYER_MOVE_VALUE_STOP;
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  u16 m_dir = MMDL_GetDirDisp( mmdl );
  
  if( dir == DIR_NOT ){
    return( PLAYER_MOVE_VALUE_STOP );
  }
  
  if( dir != m_dir && gjiki->move_action != JIKI_ACTION_WALK ){
    return( PLAYER_MOVE_VALUE_TURN );
  }
  
  return( PLAYER_MOVE_VALUE_WALK );
}

//======================================================================
// キー入力による自機移動オーダーを取得　通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * キー入力による自機移動オーダーを取得 通常移動
 * @param gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir キー入力方向
 * @param debug_flag デバッグ移動フラグ TRUE=デバッグ移動
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjiki_GetMoveOrder_Normal( FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag )
{
  JIKI_MOVEORDER set;
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  
  set = JIKI_MOVEORDER_NON;
  
  if( oze_CheckAttrUnderOze(gjiki) == TRUE ||
      gjiki_CheckMoveBitOzeFallOut(gjiki) == TRUE ){
    set = gjiki_GetMoveOrder_Oze(
        gjiki, key_trg, key_cont, dir, debug_flag );
    return( set );
  }
  
  switch( gjiki->move_action ){
  case JIKI_ACTION_STOP:
    set = gjiki_CheckMoveOrder_Stop(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_WALK:
    set = gjiki_CheckMoveOrder_Walk(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_TURN:
    set = gjiki_CheckMoveOrder_Turn(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_HITCH:
    set = gjiki_CheckMoveOrder_Hitch(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  return( set );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　停止中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjiki_CheckMoveOrder_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
    if( dir != DIR_NOT ){
      u16 old_dir;
      old_dir = MMDL_GetDirDisp( mmdl );
      
      if( dir != old_dir && debug_flag == FALSE ){
        return( JIKI_MOVEORDER_TURN );
      }
      
      return( gjiki_CheckMoveOrder_Walk(
        gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
    }
    
    return( JIKI_MOVEORDER_STOP );
  }
  
  return( JIKI_MOVEORDER_NON );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　移動中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjiki_CheckMoveOrder_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT )
  {
    return( gjiki_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  {
    MAPATTR attr;
    u32 hit = gjiki_HitCheckMove( gjiki, mmdl, dir, &attr );
    
    if( debug_flag == TRUE )
    {
      if( hit != MMDL_MOVEHITBIT_NON &&
          !(hit & MMDL_MOVEHITBIT_OUTRANGE) )
      {
        hit = MMDL_MOVEHITBIT_NON;
      }
    }
    
    if( attr != MAPATTR_ERROR )
    {
      MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
      MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
      
      if( (hit & MMDL_MOVEHITBIT_ATTR) )
      {
        u16 attr_dir = DIR_NOT;
        
        if( MAPATTR_VALUE_CheckJumpUp(val) ){
          attr_dir = DIR_UP;
        }else if( MAPATTR_VALUE_CheckJumpDown(val) ){
          attr_dir = DIR_DOWN;
        }else if( MAPATTR_VALUE_CheckumpLeft(val) ){
          attr_dir = DIR_LEFT;
        }else if( MAPATTR_VALUE_CheckJumpRight(val) ){
          attr_dir = DIR_RIGHT;
        }
        
        if( attr_dir != DIR_NOT && attr_dir == dir ){ //ジャンプ方向一致
          return( JIKI_MOVEORDER_JUMP );
        }
      }
      
      if( hit == MMDL_MOVEHITBIT_NON )
      {
        if( (flag & MAPATTR_FLAGBIT_WATER) == 0 ){ //水地形
          return( JIKI_MOVEORDER_WALK );
        }
        
        if( debug_flag == TRUE ){ //デバッグ移動
          return( JIKI_MOVEORDER_WALK );
        }
      }
    }
  }
  
  return( JIKI_MOVEORDER_HITCH );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　振り向き中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjiki_CheckMoveOrder_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT ){
    return( gjiki_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  return( gjiki_CheckMoveOrder_Walk(
    gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　障害物ヒット中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjiki_CheckMoveOrder_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( mmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( mmdl );
    return( gjiki_CheckMoveOrder_Walk(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT ){
    return( gjiki_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  return( gjiki_CheckMoveOrder_Walk(
    gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
// キー入力による自機移動オーダーを取得　尾瀬
//======================================================================
//--------------------------------------------------------------
/**
 * キー入力による自機移動オーダーを取得 尾瀬
 * @param gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir キー入力方向
 * @param debug_flag デバッグ移動フラグ TRUE=デバッグ移動
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjiki_GetMoveOrder_Oze( FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag )
{
  JIKI_MOVEORDER set;
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  
  set = JIKI_MOVEORDER_NON;
  
  switch( gjiki->move_action ){
  case JIKI_ACTION_STOP:
    set = gjikiOze_CheckMoveOrder_Stop(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_WALK:
    set = gjikiOze_CheckMoveOrder_Walk(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_TURN:
    set = gjiki_CheckMoveOrder_Turn( //流用
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_HITCH:
    set = gjiki_CheckMoveOrder_Hitch( //流用
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_OZE_YURE:
    set = gjikiOze_CheckMoveOrder_Yure(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_OZE_FALLOUT_JUMP:
  case JIKI_ACTION_OZE_FALLOUT_JUMP_WAIT:
    set = gjikiOze_CheckMoveOrder_WaitFallOutJump(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_OZE_FALLOUT:
    set = gjikiOze_CheckMoveOrder_FallOut(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  return( set );
}

//--------------------------------------------------------------
/**
 * 尾瀬　オーダーチェック　停止中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
    if( dir != DIR_NOT ){
      u16 old_dir;
      old_dir = MMDL_GetDirDisp( mmdl );
      
      if( dir != old_dir && debug_flag == FALSE ){
        return( JIKI_MOVEORDER_TURN );
      }
      
      return( gjikiOze_CheckMoveOrder_Walk(
        gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
    }
    
    return( gjikiOze_CheckMoveOrder_Yure(
          gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  return( JIKI_MOVEORDER_NON );
}

//--------------------------------------------------------------
/**
 * 尾瀬　オーダーチェック　移動中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT )
  {
    return( gjikiOze_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  {
    MAPATTR attr;
    u32 hit = oze_HitCheckMove( gjiki, mmdl, dir, &attr );
    
    if( attr != MAPATTR_ERROR )
    {
      if( (hit & JIKI_MOVEHITBIT_OZE) ) //移動先尾瀬アトリビュート
      {
        hit &= ~JIKI_MOVEHITBIT_OZE;
        
        if( hit == MMDL_MOVEHITBIT_NON ) //障害物なし
        {
          return( JIKI_MOVEORDER_WALK );
        }
      }
      else //尾瀬ではない
      {
        if( MAPATTR_GetHitchFlag(attr) == FALSE ) //移動可能
        {
          gjiki_SetMoveBitOzeFallOut( gjiki, TRUE ); //落下フラグON
          return( JIKI_MOVEORDER_OZE_FALLOUT_JUMP_START ); //落下
        }
      }
    }
  }
  
  return( JIKI_MOVEORDER_HITCH );
}

//--------------------------------------------------------------
/**
 * 尾瀬　オーダーチェック　揺れ中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_Yure(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( dir == DIR_NOT ) //何も押していない
  {
    if( gjiki_CheckMoveBitOzeYure(gjiki) == FALSE ){
      oze_StartYureJiki( gjiki );
    }
    
    if( gjiki->oze_anime_reset_flag == TRUE ){
      if( MMDL_DrawYureHero_SetAnimeFrame(mmdl,gjiki->oze_yure_frame) ){
        gjiki->oze_anime_reset_flag = FALSE;
      }
    }

    gjiki->oze_yure_frame++;
    
    if( gjiki->oze_yure_frame == OZE_YURE_MAXFRAME )
    { //落下開始
      oze_EndYureJiki( gjiki );
      gjiki_SetMoveBitOzeFallOut( gjiki, TRUE );
      return( JIKI_MOVEORDER_OZE_FALLOUT_JUMP_START );
    }
  }
  else //何か押した
  { 
    oze_EndYureJiki( gjiki );
    return( gjikiOze_CheckMoveOrder_Walk(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  return( JIKI_MOVEORDER_OZE_YURE );
}

//--------------------------------------------------------------
/**
 * 尾瀬　オーダーチェック　落下開始　ジャンプ終了
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 * @attention ここに来るという事は落下移動中という事である
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_WaitFallOutJump(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ) //尾瀬落下開始アニメ終了待ち
  {
    return( JIKI_MOVEORDER_OZE_FALLOUT );
  }
  
  return( JIKI_MOVEORDER_OZE_FALLOUT_JUMP_WAIT );
}

//--------------------------------------------------------------
/**
 * 尾瀬　オーダーチェック　落下中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 * @attention ここに来るという事は落下移動中という事である
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiOze_CheckMoveOrder_FallOut(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( oze_CheckFallOutEnd(gjiki) == TRUE ){
    VecFx32 pos;
    //ステータスを元に戻す
    //OBJコードも元に戻す必要アリ
    MMDL_SetStatusBitHeightGetOFF( mmdl, FALSE ); //高さ取得開始
	  MMDL_OnStatusBit( mmdl, //ジャンプ動作終了をセット
        MMDL_STABIT_MOVE_END|MMDL_STABIT_JUMP_END );
    MMDL_UpdateCurrentHeight( mmdl );
    MMDL_GetVectorPos( mmdl, &pos );
    MMDL_SetGridPosY( mmdl, SIZE_GRID_FX32(pos.y) );
    MMDL_UpdateCurrentMapAttr( mmdl );
    MMDL_UpdateGridPosCurrent( mmdl );
    gjiki_SetMoveBitOzeFallOut( gjiki, FALSE );
    return( JIKI_MOVEORDER_STOP );
  }
  
  return( JIKI_MOVEORDER_OZE_FALLOUT );
}

//======================================================================
// キー入力による自機状態変化を取得　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * キー入力による自機移動オーダーを取得　自転車移動
 * @param gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir キー入力方向
 * @param debug_flag デバッグ移動フラグ TRUE=デバッグ移動
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjiki_GetMoveOrder_Cycle( FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag )
{
  JIKI_MOVEORDER set;
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
   
  set = JIKI_MOVEORDER_NON;
  switch( gjiki->move_action ){
  case JIKI_ACTION_STOP:
    set = gjikiCycle_CheckMoveOrder_Stop(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_WALK:
    set = gjikiCycle_CheckMoveOrder_Walk(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_TURN:
    set = gjikiCycle_CheckMoveOrder_Turn(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_HITCH:
    set = gjikiCycle_CheckMoveOrder_Hitch(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  return( set );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　停止中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiCycle_CheckMoveOrder_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
    if( dir != DIR_NOT ){
      u16 old_dir;
      old_dir = MMDL_GetDirDisp( mmdl );
      
      if( dir != old_dir && debug_flag == FALSE ){
        return( JIKI_MOVEORDER_TURN );
      }
      
      return( gjikiCycle_CheckMoveOrder_Walk(
        gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
    }
    return( JIKI_MOVEORDER_STOP );
  }
  
  return( JIKI_MOVEORDER_NON );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　移動中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiCycle_CheckMoveOrder_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT )
  {
    return( gjikiCycle_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  {
    u32 hit = MMDL_HitCheckMoveDir( mmdl, dir );
    
    if( debug_flag == TRUE )
    {
      if( hit != MMDL_MOVEHITBIT_NON &&
          !(hit&MMDL_MOVEHITBIT_OUTRANGE) )
      {
        hit = MMDL_MOVEHITBIT_NON;
      }
    }
    
    if( hit == MMDL_MOVEHITBIT_NON )
    {
      return( JIKI_MOVEORDER_WALK );
    }
    
    if( (hit & MMDL_MOVEHITBIT_ATTR) )
    {
      BOOL ret;
      u32 attr;
      VecFx32 pos;
      
      MMDL_GetVectorPos( mmdl, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      ret = MMDL_GetMapPosAttr( mmdl, &pos, &attr );
      
      if( ret == TRUE )
      {
        u16 attr_dir = DIR_NOT;
        MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
        MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
        
        if( MAPATTR_VALUE_CheckJumpUp(val) ){
          attr_dir = DIR_UP;
        }else if( MAPATTR_VALUE_CheckJumpDown(val) ){
          attr_dir = DIR_DOWN;
        }else if( MAPATTR_VALUE_CheckumpLeft(val) ){
          attr_dir = DIR_LEFT;
        }else if( MAPATTR_VALUE_CheckJumpRight(val) ){
          attr_dir = DIR_RIGHT;
        }
        
        if( attr_dir != DIR_NOT && attr_dir == dir ) //ジャンプ方向一致
        {
          return( JIKI_MOVEORDER_JUMP );
        }
      }
    }
  }
  
  return( JIKI_MOVEORDER_HITCH );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　振り向き中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiCycle_CheckMoveOrder_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT ){
    return( gjikiCycle_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  return( gjikiCycle_CheckMoveOrder_Walk(
    gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　障害物ヒット中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiCycle_CheckMoveOrder_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( mmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( mmdl );
    return( gjikiCycle_CheckMoveOrder_Walk(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT ){
    return( gjikiCycle_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  return( gjikiCycle_CheckMoveOrder_Walk(
    gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
// キー入力による自機状態変化を取得　波乗り移動
//======================================================================
//--------------------------------------------------------------
/**
 * キー入力による自機移動オーダーを取得　自転車移動
 * @param gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir キー入力方向
 * @param debug_flag デバッグ移動フラグ TRUE=デバッグ移動
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjiki_GetMoveOrder_Swim( FIELD_PLAYER_GRID *gjiki,
    int key_trg, int key_cont, u16 dir, BOOL debug_flag )
{
  JIKI_MOVEORDER set;
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
   
  set = JIKI_MOVEORDER_NON;
  switch( gjiki->move_action ){
  case JIKI_ACTION_STOP:
    set = gjikiSwim_CheckMoveOrder_Stop(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_WALK:
    set = gjikiSwim_CheckMoveOrder_Walk(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_TURN:
    set = gjikiSwim_CheckMoveOrder_Turn(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_ACTION_HITCH:
    set = gjikiSwim_CheckMoveOrder_Hitch(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  return( set );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　停止中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
    if( dir != DIR_NOT ){
      u16 old_dir;
      old_dir = MMDL_GetDirDisp( mmdl );
      
      if( dir != old_dir && debug_flag == FALSE ){
        return( JIKI_MOVEORDER_TURN );
      }
      
      return( gjikiSwim_CheckMoveOrder_Walk(
        gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
    }
    
    return( JIKI_MOVEORDER_STOP );
  }
  
  return( JIKI_MOVEORDER_NON );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　移動中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT )
  {
    return( gjikiSwim_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  {
    MAPATTR attr;
    u32 hit = gjiki_HitCheckMove( gjiki, mmdl, dir, &attr );
    
    if( debug_flag == TRUE )
    {
      if( hit != MMDL_MOVEHITBIT_NON &&
          !(hit & MMDL_MOVEHITBIT_OUTRANGE) )
      {
        hit = MMDL_MOVEHITBIT_NON;
      }
    }
    
    if( attr != MAPATTR_ERROR )
    {
      MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
      MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
      
      if( hit == MMDL_MOVEHITBIT_NON ){
        if( (flag & MAPATTR_FLAGBIT_WATER) ){
          return( JIKI_MOVEORDER_WALK );
        }
        
        if( debug_flag == TRUE ){
          return( JIKI_MOVEORDER_WALK );
        }
      }
    }
  }
  
  return( JIKI_MOVEORDER_HITCH );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　振り向き中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT ){
    return( gjikiSwim_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  return( gjikiSwim_CheckMoveOrder_Walk(
    gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * オーダーチェック　障害物ヒット中
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval JIKI_MOVEORDER
 */
//--------------------------------------------------------------
static JIKI_MOVEORDER gjikiSwim_CheckMoveOrder_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( mmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( mmdl );
    return( gjikiSwim_CheckMoveOrder_Walk(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
    return( JIKI_MOVEORDER_NON );
  }
  
  if( dir == DIR_NOT ){
    return( gjikiSwim_CheckMoveOrder_Stop(
      gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
  return( gjikiSwim_CheckMoveOrder_Walk(
    gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
//  自機移動セット
//======================================================================
//--------------------------------------------------------------
/**
 * 自機移動セット
 * @param gjiki FIELD_PLAYER_GRID
 * @param set JIKI_MOVEORDER
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove( FIELD_PLAYER_GRID *gjiki, JIKI_MOVEORDER set,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  PLAYER_MOVE_FORM form;
  
  form = FIELD_PLAYER_GetMoveForm( gjiki->fld_player );
  
  switch( form ){
  case PLAYER_MOVE_FORM_NORMAL:
    gjiki_SetMove_Normal( gjiki, set, key_trg, key_cont, dir, debug_flag );
    break;
  case PLAYER_MOVE_FORM_CYCLE:
    gjiki_SetMove_Cycle( gjiki, set, key_trg, key_cont, dir, debug_flag );
    break;
  case PLAYER_MOVE_FORM_SWIM:
    gjiki_SetMove_Swim( gjiki, set, key_trg, key_cont, dir, debug_flag );
    break;
  default:
    GF_ASSERT( 0 );
  }
}

//======================================================================
//  自機移動セット  通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * 自機移動セット　通常移動
 * @param gjiki FIELD_PLAYER_GRID
 * @param set JIKI_MOVEORDER
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Normal( FIELD_PLAYER_GRID *gjiki, JIKI_MOVEORDER set,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  
  switch( set ){
  case JIKI_MOVEORDER_NON:
    gjiki_SetMove_Non(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_STOP:
    gjiki_SetMove_Stop(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_WALK:
    gjiki_SetMove_Walk(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_TURN:
    gjiki_SetMove_Turn(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_HITCH:
    gjiki_SetMove_Hitch(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_JUMP:
    gjiki_SetMove_Jump(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_OZE_YURE:
    gjikiOze_SetMove_Yure(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_OZE_FALLOUT_JUMP_START:
    gjikiOze_SetMove_FallOutJumpStart(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_OZE_FALLOUT_JUMP_WAIT:
    gjikiOze_SetMove_FallOutJumpWait(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_OZE_FALLOUT:
    gjikiOze_SetMove_FallOut(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  default:
    GF_ASSERT( 0 );
  }
}

//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Non(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  if( dir == DIR_NOT ){
    dir = MMDL_GetDirDisp( mmdl );
  }
  
  code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_STOP;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackStill();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  
  if( debug_flag == TRUE ){
    code = AC_WALK_U_2F;
  }else if( key_cont & PAD_BUTTON_B ){
    code = AC_DASH_U_4F;
    
    if( FIELD_PLAYER_CheckIllegalOBJCode(gjiki->fld_player) == FALSE ){
      code = AC_WALK_U_4F;
    }
  }else{
    code = AC_WALK_U_8F;
  }
  
  code = MMDL_ChangeDirAcmdCode( dir, code );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_WALK;

  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackAction();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_TURN;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_TURN );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackStill();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_HITCH;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackStill();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット ジャンプ
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Jump(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  
  code = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_WALK;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
  PMSND_PlaySE( SEQ_SE_DANSA );
  
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackAction();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット　尾瀬　ゆれ
 * @param gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiOze_SetMove_Yure(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  //特になにもしない？
  gjiki->move_action = JIKI_ACTION_OZE_YURE;
}

//--------------------------------------------------------------
/**
 * 移動セット　尾瀬　落下ジャンプセット
 * @param gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiOze_SetMove_FallOutJumpStart(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;

  if( dir == DIR_NOT ){ //揺れ限界により落ちる
    dir = oze_CheckAttrOzeFallOut( gjiki );
  }
  
  code = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_1G_8F );
  MMDL_SetAcmd( mmdl, code );
  MMDL_SetStatusBitHeightGetOFF( mmdl, TRUE ); //高さ取得を禁止に
  MMDL_OnStatusBit( mmdl, MMDL_STABIT_SHADOW_VANISH );

  gjiki->move_action = JIKI_ACTION_OZE_FALLOUT_JUMP;
}

//--------------------------------------------------------------
/**
 * 移動セット　尾瀬　落下ジャンプ終了待ち
 * @param gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiOze_SetMove_FallOutJumpWait(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  gjiki->move_action = JIKI_ACTION_OZE_FALLOUT_JUMP_WAIT;
}

//--------------------------------------------------------------
/**
 * 移動セット　尾瀬　落下
 * @param gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiOze_SetMove_FallOut(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  fx32 height;
  VecFx32 pos;
  
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  MMDL_GetVectorPos( mmdl, &pos );
  MMDL_GetMapPosHeight( mmdl, &pos, &height );
  
  if( pos.y > height ){
    pos.y += NUM_FX32( -8 );
  
    if( pos.y < height ){
      pos.y = height;
      MMDL_OffStatusBit( mmdl, MMDL_STABIT_SHADOW_VANISH );
      MMDL_SetStatusBitHeightGetOFF( mmdl, FALSE ); //高さ取得禁止解除
    }
  
    MMDL_SetVectorPos( mmdl, &pos );
    gjiki->move_action = JIKI_ACTION_OZE_FALLOUT;
  }
}

//======================================================================
//  自機移動セット  自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * 自機移動セット　自転車移動
 * @param gjiki FIELD_PLAYER_GRID
 * @param set JIKI_MOVEORDER
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Cycle( FIELD_PLAYER_GRID *gjiki,
    JIKI_MOVEORDER set,
    u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );

  switch( set ){
  case JIKI_MOVEORDER_NON:
    gjikiCycle_SetMove_Non(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_STOP:
    gjikiCycle_SetMove_Stop(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_WALK:
    gjikiCycle_SetMove_Walk(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_TURN:
    gjikiCycle_SetMove_Turn(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_HITCH:
    gjikiCycle_SetMove_Hitch(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_JUMP:
    gjikiCycle_SetMove_Jump(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  default:
    GF_ASSERT( 0 );
  }
}

//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Non(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  if( dir == DIR_NOT ){
    dir = MMDL_GetDirDisp( mmdl );
  }
  
  code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_STOP;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackStill();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  
  if( debug_flag == TRUE ){
    code = AC_WALK_U_2F;
  }else{
    MAPATTR attr;
    code = AC_WALK_U_2F;
    
    if( gjiki_GetAttr(gjiki,DIR_NOT,&attr) == TRUE ){ //深い砂漠なら速度低下
      MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
      
      if( MAPATTR_VALUE_CheckDesertDeep(val) == TRUE ){
        FLDEFF_CTRL *fectrl;
        code = AC_WALK_U_16F; //専用
        
        fectrl = FIELDMAP_GetFldEffCtrl( gjiki->fieldWork );
        FLDEFF_KEMURI_SetMMdl( mmdl, fectrl ); //煙出す
      }
    }
  }
  
  code = MMDL_ChangeDirAcmdCode( dir, code );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_WALK;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackAction();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_TURN;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_TURN );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackStill();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_HITCH;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackStill();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット ジャンプ
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Jump(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  
  code = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_WALK;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
  PMSND_PlaySE( SEQ_SE_DANSA );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackAction();
#endif
}

//======================================================================
//  自機移動セット  波乗り移動
//======================================================================
//--------------------------------------------------------------
/**
 * 自機移動セット　波乗り移動
 * @param gjiki FIELD_PLAYER_GRID
 * @param set JIKI_MOVEORDER
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Swim(
    FIELD_PLAYER_GRID *gjiki, JIKI_MOVEORDER set,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );

  switch( set ){
  case JIKI_MOVEORDER_NON:
    gjikiSwim_SetMove_Non(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_STOP:
    gjikiSwim_SetMove_Stop(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_WALK:
    gjikiSwim_SetMove_Walk(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_TURN:
    gjikiSwim_SetMove_Turn(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  case JIKI_MOVEORDER_HITCH:
    gjikiSwim_SetMove_Hitch(
      gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
  default:
    GF_ASSERT( 0 );
    break;
  }
}

//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Non(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Stop(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  if( dir == DIR_NOT ){
    dir = MMDL_GetDirDisp( mmdl );
  }
  
  code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_STOP;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );

#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackStill();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Walk(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  
  if( debug_flag == TRUE ){
    code = AC_WALK_U_2F;
  }else{
    code = AC_WALK_U_4F;
  }
  
  code = MMDL_ChangeDirAcmdCode( dir, code );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_WALK;

  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackAction();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Turn(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_TURN;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_TURN );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackStill();
#endif
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param  gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Hitch(
  FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
  u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 code;
  
  GF_ASSERT( dir != DIR_NOT );
  code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
  
  MMDL_SetAcmd( mmdl, code );
  gjiki->move_action = JIKI_ACTION_HITCH;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
#ifdef PLAYER_MOVE_TRACK_CHANGE
  FIELD_SOUND_ChangeBGMTrackStill();
#endif
}

//======================================================================
//  リクエスト
//======================================================================
//--------------------------------------------------------------
/**
 * 自機の形態に合わせてBGMを再生
 * @param gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_PlayBGM( FIELD_PLAYER_GRID *gjiki )
{
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( gjiki->fieldWork );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gdata );
  PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( gjiki->fld_player );
  u32 zone_id = FIELDMAP_GetZoneID( gjiki->fieldWork );
  u32 no = FIELD_SOUND_GetFieldBGMNo( gdata, form, zone_id );
  FIELD_SOUND_PlayNextBGM( fsnd, no );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト 
 * @param gjiki FIELD_PLAYER_GRID
 * @param reqbit FIELD_PLAYER_REQBIT
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_SetRequest(
  FIELD_PLAYER_GRID *gjiki, FIELD_PLAYER_REQBIT req_bit )
{
  gjiki->req_bit = req_bit;
}

//--------------------------------------------------------------
/**
 * リクエストを更新
 * @param gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_UpdateRequest( FIELD_PLAYER_GRID *gjiki )
{
  int i = 0;
  FIELD_PLAYER_REQBIT req_bit = gjiki->req_bit;
  
  while( i < FIELD_PLAYER_REQBIT_MAX ){
    if( (req_bit&0x01) ){
      data_gjikiRequestProcTbl[i]( gjiki );
    }
    req_bit >>= 1;
    i++;
  }
   
  gjiki->req_bit = 0;
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　通常移動に変更
 * @param gjiki FIELD_PLAYER_GIRD
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetNormal( FIELD_PLAYER_GRID *gjiki )
{
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_NORMAL );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }

  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_NORMAL );
  gjiki_PlayBGM( gjiki );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　自転車移動に変更
 * @param gjiki FIELD_PLAYER_GIRD
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetCycle( FIELD_PLAYER_GRID *gjiki )
{
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_CYCLE );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
   
  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_CYCLE );
  gjiki_PlayBGM( gjiki );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　波乗り移動に変更
 * @param gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetSwim( FIELD_PLAYER_GRID *gjiki )
{
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_SWIM );

  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  
  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_SWIM );
  gjiki_PlayBGM( gjiki );
  
  if( gjiki->fldeff_joint == NULL ){ //波乗りポケモン
    u16 dir;
    VecFx32 pos;
    FLDEFF_CTRL *fectrl;
    
    fectrl = FIELDMAP_GetFldEffCtrl( gjiki->fieldWork );
    dir = MMDL_GetDirDisp( mmdl );
    MMDL_GetVectorPos( mmdl, &pos );
    
    gjiki->fldeff_joint = FLDEFF_NAMIPOKE_SetMMdl(
        fectrl, dir, &pos, mmdl, NAMIPOKE_JOINT_ON );
  }
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　動作形態にあわせた表示にする
 * @param gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetMoveFormToDrawForm( FIELD_PLAYER_GRID *gjiki )
{
  FIELD_PLAYER_ResetMoveForm( gjiki->fld_player );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　アイテムゲット自機に変更
 * @param gjiki FIELD_PLYER
 * @retval  nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetItemGetHero( FIELD_PLAYER_GRID *gjiki )
{
  FIELD_PLAYER_ChangeDrawForm( gjiki->fld_player, PLAYER_DRAW_FORM_ITEMGET );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　PC預け自機に変更
 * @param gjiki FIELD_PLYER
 * @retval  nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetReportHero( FIELD_PLAYER_GRID *gjiki )
{
  FIELD_PLAYER_ChangeDrawForm( gjiki->fld_player, PLAYER_DRAW_FORM_SAVEHERO );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　PC預け自機に変更
 * @param gjiki FIELD_PLYER
 * @retval  nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetPCAzukeHero( FIELD_PLAYER_GRID *gjiki )
{
  FIELD_PLAYER_ChangeDrawForm( gjiki->fld_player, PLAYER_DRAW_FORM_PCHERO );
}

//--------------------------------------------------------------
/// 自機リクエスト処理テーブル
//--------------------------------------------------------------
static void (* const data_gjikiRequestProcTbl[FIELD_PLAYER_REQBIT_MAX])( FIELD_PLAYER_GRID *gjiki ) =
{
  gjikiReq_SetNormal, //FIELD_PLAYER_REQBIT_NORMAL
  gjikiReq_SetCycle, //FIELD_PLAYER_REQBIT_CYCLE
  gjikiReq_SetSwim, //FIELD_PLAYER_REQBIT_SWIM
  gjikiReq_SetMoveFormToDrawForm,
  gjikiReq_SetItemGetHero,
  gjikiReq_SetReportHero,
  gjikiReq_SetPCAzukeHero,
};

//======================================================================
//  移動チェック
//======================================================================
//--------------------------------------------------------------
/**
 * 自機移動チェック
 * @param gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL
 * @param dir 移動方向
 * @param attr 移動先のアトリビュートを格納
 * @retval u32 ヒットビット。MMDL_MOVEHITBIT_NON等
 */
//--------------------------------------------------------------
static u32 gjiki_HitCheckMove(
    FIELD_PLAYER_GRID *gjiki, MMDL *mmdl, u16 dir, MAPATTR *attr )
{
  u32 hit;
  VecFx32 pos;
  s16 gx = MMDL_GetGridPosX( mmdl );
  s16 gy = MMDL_GetHeightGrid( mmdl );
  s16 gz = MMDL_GetGridPosZ( mmdl );
  JIKI_MOVEHITBIT ret = MMDL_MOVEHITBIT_NON;
  
  gx += MMDL_TOOL_GetDirAddValueGridX( dir );
  gz += MMDL_TOOL_GetDirAddValueGridZ( dir );
  
  hit = MMDL_HitCheckMoveCurrent( mmdl, gx, gy, gz, dir );
  
  MMDL_GetVectorPos( mmdl, &pos );
  MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
  
  if( MMDL_GetMapPosAttr(mmdl,&pos,attr) == FALSE )
  {
    *attr = MAPATTR_ERROR;
  }
  
  if( (hit&MMDL_MOVEHITBIT_ATTR) ) //一部無効とする判定をチェック
  {
    MAPATTR_VALUE attr_val;
    attr_val = MAPATTR_GetAttrValue( *attr );
    
    if( MAPATTR_VALUE_CheckKairikiAna(attr_val) == TRUE )
    {
      const MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
      
      KAGAYA_Printf( "穴チェックきました\n" );
      
      if( MMDLSYS_ROCKPOS_CheckRockFalled(mmdlsys,&pos) == TRUE )
      {
        hit &= ~MMDL_MOVEHITBIT_ATTR;
      }
    }
  }
  
  return( hit );
}

//--------------------------------------------------------------
/**
 * 自機オーダーチェック
 * @param gjiki FIELD_PLAYER_GRID
 * @param dir 移動方向。DIR_UP等
 * @retval BOOL TRUE=移動可能 FALSE=移動不可
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_GRID_CheckStartMove( FIELD_PLAYER_GRID *gjiki, u16 dir )
{
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  
  if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
    return( TRUE );
  }
  
  if( dir == DIR_NOT ){
    return( FALSE );
  }
  
#if 0  
  if( Jiki_WallHitAcmdCodeCheck(code) == TRUE ){ //壁ヒットはキャンセル可能
  }
#endif
  return( FALSE );
}

//======================================================================
//  サウンド
//======================================================================
#if 0
//--------------------------------------------------------------
/**
 * サウンドテスト　自機停止
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_Sound_MoveStop( void )
{
  //サウンドテスト（停止）
  u16 trackBit = 0xfcff; // track 9,10 OFF
  PMSND_ChangeBGMtrack(trackBit);
}

//--------------------------------------------------------------
/**
 * サウンドテスト　自機移動
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_Sound_Move( void )
{
  //サウンドテスト（移動）
  u16 trackBit = 0xffff; // 全track ON
  PMSND_ChangeBGMtrack(trackBit);
}
#endif

//======================================================================
//  自機動作ビット
//======================================================================
//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT ON
 * @param gjiki FIELD_PLAYER_GRID
 * @param bit JIKI_MOVEBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_OnMoveBit(
    FIELD_PLAYER_GRID *gjiki, JIKI_MOVEBIT bit )
{
  GF_ASSERT( bit < JIKI_MOVEBIT_BITMAX );
  gjiki->move_bit |= bit;
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT OFF
 * @param gjiki FIELD_PLAYER_GRID
 * @param bit JIKI_MOVEBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_OffMoveBit(
    FIELD_PLAYER_GRID *gjiki, JIKI_MOVEBIT bit )
{
  GF_ASSERT( bit < JIKI_MOVEBIT_BITMAX );
  gjiki->move_bit &= ~bit;
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT Check
 * @param gjiki FIELD_PLAYER_GRID
 * @param bit JIKI_MOVEBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static BOOL gjiki_CheckMoveBit(
    FIELD_PLAYER_GRID *gjiki, JIKI_MOVEBIT bit )
{
  GF_ASSERT( bit < JIKI_MOVEBIT_BITMAX );
  if( (gjiki->move_bit & bit) ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 強制移動ビットON
 * @param gjiki FIELD_PLAYER_GRID
 * @param bit JIKI_MOVEBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_OnMoveBitForce( FIELD_PLAYER_GRID *gjiki )
{
  gjiki_OnMoveBit( gjiki, JIKI_MOVEBIT_FORCE );
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 強制移動ビットOFF
 * @param gjiki FIELD_PLAYER_GRID
 * @param bit JIKI_MOVEBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_OffMoveBitForce( FIELD_PLAYER_GRID *gjiki )
{
  gjiki_OffMoveBit( gjiki, JIKI_MOVEBIT_FORCE );
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 強制移動ビットチェック
 * @param gjiki FIELD_PLAYER_GRID
 * @retval BOOL 
 */
//--------------------------------------------------------------
static BOOL gjiki_CheckMoveBitForce( FIELD_PLAYER_GRID *gjiki )
{
  if( gjiki_CheckMoveBit(gjiki,JIKI_MOVEBIT_FORCE) ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 足元無効ON
 * @param gjiki FIELD_PLAYER_GRID
 * @param bit JIKI_MOVEBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_OnMoveBitUnderOff( FIELD_PLAYER_GRID *gjiki )
{
  gjiki_OnMoveBit( gjiki, JIKI_MOVEBIT_UNDER_OFF );
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 足元無効OFF
 * @param gjiki FIELD_PLAYER_GRID
 * @param bit JIKI_MOVEBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_OffMoveBitUnderOff( FIELD_PLAYER_GRID *gjiki )
{
  gjiki_OffMoveBit( gjiki, JIKI_MOVEBIT_UNDER_OFF );
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 一歩移動 ON
 * @param gjiki FIELD_PLAYER_GRID
 * @param bit JIKI_MOVEBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_OnMoveBitStep( FIELD_PLAYER_GRID *gjiki )
{
  gjiki_OnMoveBit( gjiki, JIKI_MOVEBIT_STEP );
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 一歩移動 OFF
 * @param gjiki FIELD_PLAYER_GRID
 * @param bit JIKI_MOVEBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_OffMoveBitStep( FIELD_PLAYER_GRID *gjiki )
{
  gjiki_OffMoveBit( gjiki, JIKI_MOVEBIT_STEP );
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 尾瀬ゆれ状態 セット
 * @param gjiki FIELD_PLAYER_GRID
 * @param on TRUE=尾瀬状態 FALSE=尾瀬フラグクリア
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMoveBitOzeYure(
    FIELD_PLAYER_GRID *gjiki, BOOL on )
{
  if( on == TRUE ){
    gjiki_OnMoveBit( gjiki, JIKI_MOVEBIT_OZE_YURE );
  }else{
    gjiki_OffMoveBit( gjiki, JIKI_MOVEBIT_OZE_YURE );
  }
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 尾瀬ゆれ状態 チェック
 * @param gjiki FIELD_PLAYER_GRID
 * @param on TRUE=尾瀬状態 FALSE=尾瀬フラグクリア
 * @retval nothing
 */
//--------------------------------------------------------------
static BOOL gjiki_CheckMoveBitOzeYure( FIELD_PLAYER_GRID *gjiki )
{
  if( gjiki_CheckMoveBit(gjiki,JIKI_MOVEBIT_OZE_YURE) ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 尾瀬落下 セット
 * @param gjiki FIELD_PLAYER_GRID
 * @param on TRUE=尾瀬状態 FALSE=尾瀬フラグクリア
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMoveBitOzeFallOut( FIELD_PLAYER_GRID *gjiki, BOOL on )
{
  if( on == TRUE ){
    gjiki_OnMoveBit( gjiki, JIKI_MOVEBIT_OZE_FALLOUT );
  }else{
    gjiki_OffMoveBit( gjiki, JIKI_MOVEBIT_OZE_FALLOUT );
  }
}

//--------------------------------------------------------------
/**
 * JIKI_MOVEBIT 尾瀬状態 セット
 * @param gjiki FIELD_PLAYER_GRID
 * @param on TRUE=尾瀬状態 FALSE=尾瀬フラグクリア
 * @retval nothing
 */
//--------------------------------------------------------------
static BOOL gjiki_CheckMoveBitOzeFallOut( FIELD_PLAYER_GRID *gjiki )
{
  if( gjiki_CheckMoveBit(gjiki,JIKI_MOVEBIT_OZE_FALLOUT) ){
    return( TRUE );
  }
  return( FALSE );
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * 自機を強制停止させる。グリッド専用
 * @param fld_player FIELD_PLAYER
 * @retval nothing
 * @note 自機が強制停止出来ない場合はそのまま
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_ForceStop( FIELD_PLAYER *fld_player )
{
  FIELDMAP_WORK *fieldWork = FIELD_PLAYER_GetFieldMapWork( fld_player );
  FIELD_PLAYER_GRID *gjiki = FIELDMAP_GetPlayerGrid( fieldWork );

  if( gjiki->move_action == JIKI_ACTION_HITCH ){
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
    u16 dir = MMDL_GetDirDisp( mmdl );
    MMDL_FreeAcmd( mmdl );
    MMDL_SetDirDisp( mmdl, dir );
    MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
    FIELD_PLAYER_SetMoveValue( fld_player, PLAYER_MOVE_VALUE_STOP );
  }
}

//--------------------------------------------------------------
/**
 * 自機　動作停止
 * @param fld_player
 * @retval BOOL TRUE=停止完了。FALSE=移動中につき停止出来ない。
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_GRID_SetMoveStop( FIELD_PLAYER_GRID *gjiki )
{
  switch( gjiki->move_action ){
  case JIKI_ACTION_STOP:
    break;
  case JIKI_ACTION_WALK:
    return( FALSE );
  case JIKI_ACTION_TURN:
  case JIKI_ACTION_HITCH:
    {
      MMDL *mmdl = FIELD_PLAYER_GRID_GetMMdl( gjiki );
      u16 dir = MMDL_GetDirDisp( mmdl );
      MMDL_FreeAcmd( mmdl );
      MMDL_SetDirDisp( mmdl, dir );
      MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
      FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
      break;
    }
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 自機に波乗りポケモンのタスクポインタをセット
 * @param gjiki FIELD_PLAYER_GRID
 * @param task セットするFLDEFF_TASK
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_SetEffectTaskWork(
    FIELD_PLAYER_GRID *gjiki, FLDEFF_TASK *task )
{
  gjiki->fldeff_joint = task;
}

//--------------------------------------------------------------
/**
 * 自機が持っているエフェクトタスクのポインタを取得
 * @param   gjiki FIELD_PLAYER_GRID
 * @retval FLDEFF_TASK*
 */
//--------------------------------------------------------------
FLDEFF_TASK * FIELD_PLAYER_GRID_GetEffectTaskWork(
    FIELD_PLAYER_GRID *gjiki )
{
  return( gjiki->fldeff_joint );
}

//--------------------------------------------------------------
/**
 * 自機を波乗り状態にする
 * @param gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetNaminori( FIELD_PLAYER_GRID *gjiki )
{
  int sex;
  u16 code;
  MMDL *mmdl;
   
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_SWIM );

  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  
  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_SWIM );
  gjiki_PlayBGM( gjiki );
}

//--------------------------------------------------------------
/**
 * 自機波乗り状態を終了する
 * @param gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetNaminoriEnd( FIELD_PLAYER_GRID *gjiki )
{
  VecFx32 offs = {0,0,0};
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_NORMAL );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  
  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_NORMAL );
  gjiki_PlayBGM( gjiki );
  
  if( gjiki->fldeff_joint != NULL ){
    FLDEFF_TASK_CallDelete( gjiki->fldeff_joint );
    gjiki->fldeff_joint = NULL;
  }
  
  MMDL_SetVectorOuterDrawOffsetPos( mmdl, &offs );
}

//--------------------------------------------------------------
/**
 * 自機　入力キーから移動する方向を返す
 * @param gjiki FIELD_PLAYER_GRID
 * @param key キー情報
 * @retval u16 移動方向 DIR_UP等
 */
//--------------------------------------------------------------
u16 FIELD_PLAYER_GRID_GetKeyDir( FIELD_PLAYER_GRID *gjiki, int key )
{
  u16 dir = gjiki_GetInputKeyDir( gjiki, key );
  return( dir );
}

//--------------------------------------------------------------
/**
 * 自機足元をチェックする
 * @param
 * @retval
 */
//--------------------------------------------------------------
static UNDER gjiki_CheckUnder( FIELD_PLAYER_GRID *gjiki, u16 dir )
{
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  MAPATTR attr = MMDL_GetMapAttr( mmdl );
  
  if( gjiki_CheckMoveBit(gjiki,JIKI_MOVEBIT_UNDER_OFF) == FALSE ){
    int i = 0;
    MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
    
    if( MAPATTR_VALUE_CheckIce(val) == TRUE ){
      return( UNDER_ICE );
    }
  }
  
  return( UNDER_NONE );
}

//--------------------------------------------------------------
/**
 * 自機足元が強制移動かどうか
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_GRID_CheckUnderForceMove( FIELD_PLAYER *fld_player )
{
  FIELDMAP_WORK *fieldmap = FIELD_PLAYER_GetFieldMapWork( fld_player );
  FIELD_PLAYER_GRID *gjiki = FIELDMAP_GetPlayerGrid( fieldmap );
  UNDER under = gjiki_CheckUnder( gjiki, DIR_NOT );
  
  if( under != UNDER_NONE ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機が尾瀬落下中かどうか
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_GRID_CheckOzeFallOut( FIELD_PLAYER *fld_player )
{
  FIELDMAP_WORK *fieldmap = FIELD_PLAYER_GetFieldMapWork( fld_player );
  FIELD_PLAYER_GRID *gjiki = FIELDMAP_GetPlayerGrid( fieldmap );
  
  if( gjiki_CheckMoveBitOzeFallOut(gjiki) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//======================================================================
//  自機足元処理
//======================================================================
//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 *
 */
//--------------------------------------------------------------
static void gjiki_ClearUnderForceMove( FIELD_PLAYER_GRID *gjiki )
{
  if( gjiki_CheckMoveBitForce(gjiki) == TRUE ){
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
    MMDL_OffStatusBit( mmdl, MMDL_STABIT_PAUSE_DIR|MMDL_STABIT_PAUSE_ANM );
    gjiki_OffMoveBitForce( gjiki );
  }
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL gjikiUnder_None( FIELD_PLAYER_GRID *gjiki, u16 dir )
{
  gjiki_ClearUnderForceMove( gjiki );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL gjikiUnder_Ice( FIELD_PLAYER_GRID *gjiki, u16 dir )
{
  MAPATTR attr;
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  u16 jiki_dir = MMDL_GetDirMove( mmdl );
  u32 hit = gjiki_HitCheckMove( gjiki, mmdl, jiki_dir, &attr );
	
  if( hit != MMDL_MOVEHITBIT_NON ){ //障害物ヒット
    gjiki_ClearUnderForceMove( gjiki );
    gjiki_OnMoveBitUnderOff( gjiki );
    FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
    return( FALSE );
  }
  
  gjiki_OnMoveBitForce( gjiki );
  
  {
	  u16 code = MMDL_ChangeDirAcmdCode( jiki_dir, AC_WALK_U_4F );
    
    MMDL_SetAcmd( mmdl, code );
    MMDL_OnStatusBit( mmdl, MMDL_STABIT_PAUSE_DIR|MMDL_STABIT_PAUSE_ANM );
    
    gjiki->move_action = JIKI_ACTION_WALK;
    FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
	  return( TRUE );
  }
}

static BOOL (* const data_MoveUnderFuncTbl[UNDER_MAX])(FIELD_PLAYER_GRID*,u16) =
{
  gjikiUnder_None,
  gjikiUnder_Ice,
};

//--------------------------------------------------------------
/**
 * 自機足元処理
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL gjiki_ControlUnder(
    FIELD_PLAYER_GRID *gjiki, u16 dir, BOOL debug )
{
  if( debug == FALSE ){
    UNDER under = gjiki_CheckUnder( gjiki, dir );
    
    if( data_MoveUnderFuncTbl[under](gjiki,dir) == TRUE ){
      return( TRUE );
    }
  }
  
  gjiki_OffMoveBitForce( gjiki );
  return( FALSE );
}

//======================================================================
//  尾瀬
//======================================================================
#if 0
//--------------------------------------------------------------
/**
 * 尾瀬状態にする
 * @param gjiki FIELD_PLAYER_GRID
 * @param init_frame TRUE=ゆれフレーム初期化
 * @retval nothing
 */
//--------------------------------------------------------------
static void oze_InitParam( FIELD_PLAYER_GRID *gjiki )
{
}
#endif

//--------------------------------------------------------------
/**
 * 尾瀬系アトリビュートか
 * @param val MAPATTR_VALUE
 * @retval BOOL TRUE=尾瀬系
 */
//--------------------------------------------------------------
static BOOL oze_IsOzeAttr( const MAPATTR_VALUE val )
{
  if( MAPATTR_VALUE_CheckOze01(val) == TRUE ||
      MAPATTR_VALUE_CheckOzeStairs(val) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 尾瀬アトリビュートか　尾瀬階段は考慮しない
 * @param val MAPATTR_VALUE
 * @retval BOOL TRUE=尾瀬
 */
//--------------------------------------------------------------
static BOOL oze_IsOnlyOzeAttr( const MAPATTR_VALUE val )
{
  if( MAPATTR_VALUE_CheckOze01(val) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 尾瀬　自機の移動先が移動可能かどうか
 * @param gjiki FIELD_PLAYER_GRID
 * @param dir 移動方向
 * @retval BOOL TRUE=移動可能
 */
//--------------------------------------------------------------
static BOOL oze_CheckAttrOzeMove( FIELD_PLAYER_GRID *gjiki, u16 dir )
{
  BOOL ret;
  MAPATTR attr;
  
  GF_ASSERT( dir < DIR_MAX4 );
  ret = gjiki_GetAttr( gjiki, dir, &attr );
  
  if( ret == TRUE ){
    MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
    
    if( oze_IsOzeAttr(val) == TRUE ){
      return( TRUE );
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 尾瀬　自機の足場が尾瀬かどうか
 * @param gjiki FIELD_PLAYER_GRID
 * @retval BOOL TRUE=尾瀬
 */
//--------------------------------------------------------------
static BOOL oze_CheckAttrUnderOze( FIELD_PLAYER_GRID *gjiki )
{
  BOOL ret;
  MAPATTR attr;
  MAPATTR_VALUE val;

  gjiki_GetAttr( gjiki, DIR_NOT, &attr );
  val = MAPATTR_GetAttrValue( attr );
  
  if( oze_IsOnlyOzeAttr(val) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 尾瀬　尾瀬アトリビュート以外で落下可能な方向を探す。
 * @param gjiki FIELD_PLAYER_GRID
 * @retval u16 落下可能な方向 DIR_NOT=なし
 */
//--------------------------------------------------------------
static u16 oze_CheckAttrOzeFallOut( FIELD_PLAYER_GRID *gjiki )
{
  u16 i,dir;
  BOOL ret;
  MAPATTR attr;
  MAPATTR_VALUE val;
  MAPATTR_FLAGBIT flag;
  //時計回りで各方向をチェック
  u16 next_dir[DIR_MAX4] = { DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN };
  
  dir = FIELD_PLAYER_GetDir( gjiki->fld_player );
  
  for( i = 0; i < DIR_MAX4; i++, dir = next_dir[dir] ){
    ret = gjiki_GetAttr( gjiki, dir, &attr );
    
    if( ret == TRUE ){
      val = MAPATTR_GetAttrValue( attr );
      
      if( oze_IsOzeAttr(val) == FALSE ){
        if( MAPATTR_GetHitchFlag(attr) == FALSE ){
          return( dir );
        }
      }
    }
  }
  
  GF_ASSERT( 0 );
  return( DIR_NOT );
}

//--------------------------------------------------------------
/**
 * 尾瀬　移動ヒットチェック
 * @param gjiki
 * @retval
 */
//--------------------------------------------------------------
static u32 oze_HitCheckMove(
    FIELD_PLAYER_GRID *gjiki, MMDL *mmdl, u16 dir, MAPATTR *attr )
{
  u32 hit = gjiki_HitCheckMove( gjiki, mmdl, dir, attr );
  MAPATTR_VALUE val = MAPATTR_GetAttrValue( *attr );
  
  if( oze_IsOzeAttr(val) == TRUE ){
    hit |= JIKI_MOVEHITBIT_OZE;
  }
  
  return( hit );
}

//--------------------------------------------------------------
/**
 * 尾瀬　落下による着地チェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL oze_CheckFallOutEnd( FIELD_PLAYER_GRID *gjiki )
{
  fx32 height;
  VecFx32 pos;
  MMDL *mmdl;
  
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  MMDL_GetVectorPos( mmdl, &pos );
  MMDL_GetMapPosHeight( mmdl, &pos, &height );
  
  if( pos.y > height ){
    return( FALSE );
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 尾瀬ゆれ状態に
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void oze_StartYureJiki( FIELD_PLAYER_GRID *gjiki )
{
  gjiki_SetMoveBitOzeYure( gjiki, TRUE );
  FIELD_PLAYER_ChangeDrawForm(
    gjiki->fld_player, PLAYER_DRAW_FORM_YURE );
}

//--------------------------------------------------------------
/**
 * 尾瀬ゆれ状態から元に戻す
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void oze_EndYureJiki( FIELD_PLAYER_GRID *gjiki )
{
  gjiki->oze_yure_frame = 0;
  gjiki->oze_anime_reset_flag = FALSE;
  gjiki_SetMoveBitOzeFallOut( gjiki, FALSE );
      
  gjiki_SetMoveBitOzeYure( gjiki, FALSE );
  FIELD_PLAYER_ResetMoveForm( gjiki->fld_player );
}

//======================================================================
//  FIELD_PLAYER_GRID参照
//======================================================================
//--------------------------------------------------------------
/**
 * FIELD_PLAYER_GRID FIELD_PLAYER取得
 * @param gjiki FIELD_PLAYER_GRID
 * @retval FIELD_PLAYER*
 */
//--------------------------------------------------------------
FIELD_PLAYER * FIELD_PLAYER_GRID_GetFieldPlayer( FIELD_PLAYER_GRID *gjiki )
{
  return( gjiki->fld_player );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER_GRID 使用している動作モデル取得
 * @param gjiki FIELD_PLAYER_GRID
 * @retval MMDL*
 */
//--------------------------------------------------------------
MMDL * FIELD_PLAYER_GRID_GetMMdl( FIELD_PLAYER_GRID *gjiki )
{
  return( FIELD_PLAYER_GetMMdl(gjiki->fld_player) );
}


//--------------------------------------------------------------
/**
 * 自機が特殊表示の場合は元に戻す。
 * @param fld_player
 * @retval nothing
 * @note 今の所、該当箇所は尾瀬ゆれ状態のみ
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_CheckSpecialDrawForm(
    FIELD_PLAYER *fld_player, BOOL menu_open_flag )
{
  FIELDMAP_WORK *fieldWork = FIELD_PLAYER_GetFieldMapWork( fld_player );
  FIELD_PLAYER_GRID *gjiki = FIELDMAP_GetPlayerGrid( fieldWork );
  
  if( gjiki_CheckMoveBitOzeYure(gjiki) == TRUE ){
    u32 oze_yure_frame = gjiki->oze_yure_frame;
    oze_EndYureJiki( gjiki );
    //メニューオープン時は揺れフレームを維持
    //オープン後、バッグ等、画面遷移の場合は自機ワークはクリアされる
    if( menu_open_flag == TRUE ){
      gjiki->oze_yure_frame = oze_yure_frame;
      gjiki->oze_anime_reset_flag = TRUE;
    }
  }
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * 自機の指定方向先のアトリビュートを取得する
 * @param gjiki FIELD_PLAYER_GRID
 * @param dir 取得先 DIR_NOT=その場チェック
 * @pparam attr MNAPATTR格納先
 * @retval BOOL FALSE=アトリビュートが存在しない
 */
//--------------------------------------------------------------
static BOOL gjiki_GetAttr(
    FIELD_PLAYER_GRID *gjiki, u16 dir, MAPATTR *attr )
{
  BOOL ret;
  MMDL *mmdl;
  VecFx32 pos;
  
  FIELD_PLAYER_GetPos( gjiki->fld_player, &pos );
  
  if( dir != DIR_NOT ){
    MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
  }
  
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  ret = MMDL_GetMapPosAttr( mmdl, &pos, attr );
  return( ret );
}
