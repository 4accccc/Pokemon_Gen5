//======================================================================
/**
 *
 * @file  fieldobj_movedata.c
 * @brief  フィールド動作モデル動作
 * @author  kagaya
 * @date  05.07.20
 *
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procmove.h"
#include "fldmmdl_procacmd.h"

#include "field_comm_actor.h" //MV_COMM_ACTOR

//==============================================================================
//  extern　動作関数
//==============================================================================

//==============================================================================
//  extern  アニメーションコマンド
//==============================================================================
//==============================================================================
//  フィールド動作モデル 動作関数定義
//==============================================================================
//--------------------------------------------------------------
///  ダミー
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Dummy =
{
  MV_DMY,                          ///<動作コード
  MMDL_MoveInitProcDummy,                ///<初期化関数
  MMDL_MoveProcDummy,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_PLAYER  
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Player =
{
  MV_PLAYER,                        ///<動作コード
  MMDL_MoveInitProcDummy,                ///<初期化関数
  MMDL_MoveProcDummy,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_DIR_RND
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_DirRnd =
{
  MV_DIR_RND,                        ///<動作コード
  MMDL_MoveDirRnd_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                ///<動作関数
  MMDL_MoveDirRnd_Delete,                ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Rnd =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRnd_Init,                  ///<初期化関数
  MMDL_MvRnd_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_V
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndV =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRndV_Init,                  ///<初期化関数
  MMDL_MvRnd_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_H
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndH =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRndH_Init,                  ///<初期化関数
  MMDL_MvRnd_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_H_LIM
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndHLim =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRndHLim_Init,                ///<初期化関数
  MMDL_MvRnd_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_UL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndUL =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndUL_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_UR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndUR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndUR_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_DL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndDL =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndDL_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_DR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndDR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndDR_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_UDL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndUDL =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndUDL_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_UDR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndUDR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndUDR_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_ULR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndULR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndULR_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_DLR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndDLR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndDLR_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_UD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndUD =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndUD_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RND_LR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RndLR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDirRndLR_Init,                ///<初期化関数
  MMDL_MoveDirRnd_Move,                  ///<動作関数
  MMDL_MoveDirRnd_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_UP
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Up =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveUp_Init,                  ///<初期化関数
  MMDL_MoveDir_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_DOWN
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Down =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveDown_Init,                  ///<初期化関数
  MMDL_MoveDir_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_LEFT
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Left =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveLeft_Init,                  ///<初期化関数
  MMDL_MoveDir_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RIGHT
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Right =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRight_Init,                ///<初期化関数
  MMDL_MoveDir_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_SPIN_L
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_SpinLeft =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveSpinLeft_Init,                ///<初期化関数
  MMDL_MoveSpin_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_SPIN_R
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_SpinRight =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveSpinRight_Init,              ///<初期化関数
  MMDL_MoveSpin_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_REWAR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Rewar =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRewar_Init,                ///<初期化関数
  MMDL_MoveRewar_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RT2
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Route2 =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRoute2_Init,                ///<初期化関数
  MMDL_MoveRoute2_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTURLD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteURLD =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteURLD_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTRLDU
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteRLDU =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteRLDU_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTDURL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteDURL =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteDURL_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTLDUR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteLDUR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteLDUR_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTULRD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteULRD =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteULRD_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTLRDU
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteLRDU =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteLRDU_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTDULR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteDULR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteDULR_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTRDUL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteRDUL =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteRDUL_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTLUDR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteLUDR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteLUDR_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTUDRL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteUDRL =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteUDRL_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTRLUD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteRLUD =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteRLUD_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTDRLU
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteDRLU =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteDRLU_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTRUDL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteRUDL =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteRUDL_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTUDLR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteUDLR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteUDLR_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTLRUD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteLRUD =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteLRUD_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTDLRU
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteDLRU =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteDLRU_Init,              ///<初期化関数
  MMDL_MoveRoute3_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTUL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteUL =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteUL_Init,                ///<初期化関数
  MMDL_MoveRoute4_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTDR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteDR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteDR_Init,                ///<初期化関数
  MMDL_MoveRoute4_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTLD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteLD =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteLD_Init,                ///<初期化関数
  MMDL_MoveRoute4_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTRU
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteRU =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteRU_Init,                ///<初期化関数
  MMDL_MoveRoute4_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTUR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteUR =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteUR_Init,                ///<初期化関数
  MMDL_MoveRoute4_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTDL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteDL =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteDL_Init,                ///<初期化関数
  MMDL_MoveRoute4_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTLU
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteLU =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteLU_Init,                ///<初期化関数
  MMDL_MoveRoute4_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_RTRD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RouteRD =
{
  MV_RND,                          ///<動作コード
  MMDL_MoveRouteRD_Init,                ///<初期化関数
  MMDL_MoveRoute4_Move,                ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

#if 0
//--------------------------------------------------------------
///  MV_SEED
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Seed =
{
  MV_SEED,                        ///<動作コード
  MMDL_MoveSeed_Init,                  ///<初期化関数
  MMDL_MoveSeed_Move,                  ///<動作関数
  MMDL_MoveSeed_Delete,                ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};
#endif

//--------------------------------------------------------------
///  MV_PAIR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_Pair =
{
  MV_RND,                          ///<動作コード
  MMDL_MovePair_Init,                  ///<初期化関数
  MMDL_MovePair_Move,                  ///<動作関数
  MMDL_MovePair_Delete,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_TR_PAIR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_PairTr =
{
  MV_RND,                          ///<動作コード
  MMDL_MovePairTr_Init,                ///<初期化関数
  MMDL_MovePairTr_Move,                ///<動作関数
  MMDL_MovePairTr_Delete,                ///<削除関数
  MMDL_MovePairTr_Return,                ///<復帰関数
};

//--------------------------------------------------------------
///  MV_HIDE_SNOW
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_HideSnow =
{
  MV_HIDE_SNOW,                        ///<動作コード
  MMDL_MoveHideSnow_Init,                ///<初期化関数
  MMDL_MoveHide_Move,                ///<動作関数
  MMDL_MoveHide_Delete,                ///<削除関数
  MMDL_MoveHide_Return,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_HIDE_SAND
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_HideSand =
{
  MV_HIDE_SAND,                        ///<動作コード
  MMDL_MoveHideSand_Init,                ///<初期化関数
  MMDL_MoveHide_Move,                ///<動作関数
  MMDL_MoveHide_Delete,                ///<削除関数
  MMDL_MoveHide_Return,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_HIDE_GRND
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_HideGround =
{
  MV_HIDE_GRND,                        ///<動作コード
  MMDL_MoveHideGround_Init,                ///<初期化関数
  MMDL_MoveHide_Move,                ///<動作関数
  MMDL_MoveHide_Delete,                ///<削除関数
  MMDL_MoveHide_Return,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_HIDE_KUSA
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_HideKusa =
{
  MV_HIDE_KUSA,                        ///<動作コード
  MMDL_MoveHideKusa_Init,                ///<初期化関数
  MMDL_MoveHide_Move,                ///<動作関数
  MMDL_MoveHide_Delete,                ///<削除関数
  MMDL_MoveHide_Return,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_COPYU
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_CopyU =
{
  MV_COPYU,                        ///<動作コード
  MMDL_MoveCopyU_Init,                ///<初期化関数
  MMDL_MoveCopy_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_COPYD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_CopyD =
{
  MV_COPYD,                        ///<動作コード
  MMDL_MoveCopyD_Init,                ///<初期化関数
  MMDL_MoveCopy_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_COPYL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_CopyL =
{
  MV_COPYL,                        ///<動作コード
  MMDL_MoveCopyL_Init,                ///<初期化関数
  MMDL_MoveCopy_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_COPYR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_CopyR =
{
  MV_COPYR,                        ///<動作コード
  MMDL_MoveCopyR_Init,                ///<初期化関数
  MMDL_MoveCopy_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_COPYLGRASSU
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_CopyLGrassU =
{
  MV_COPYLGRASSU,                      ///<動作コード
  MMDL_MoveCopyLGrassU_Init,                ///<初期化関数
  MMDL_MoveCopy_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_COPYLGRASSD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_CopyLGrassD =
{
  MV_COPYLGRASSD,                        ///<動作コード
  MMDL_MoveCopyLGrassD_Init,                ///<初期化関数
  MMDL_MoveCopy_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_COPYLGRASSL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_CopyLGrassL =
{
  MV_COPYLGRASSL,                        ///<動作コード
  MMDL_MoveCopyLGrassL_Init,                ///<初期化関数
  MMDL_MoveCopy_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_COPYLGRASSR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_CopyLGrassR =
{
  MV_COPYLGRASSR,                        ///<動作コード
  MMDL_MoveCopyLGrassR_Init,                ///<初期化関数
  MMDL_MoveCopy_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_ALONGW_L
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_AlongWallL =
{
  MV_ALONGW_L,                        ///<動作コード
  MMDL_AlongWallL_Init,                ///<初期化関数
  MMDL_AlongWall_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_ALONGW_R
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_AlongWallR =
{
  MV_ALONGW_R,                        ///<動作コード
  MMDL_AlongWallR_Init,                ///<初期化関数
  MMDL_AlongWall_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_ALONGW_LRL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_AlongWallLRL =
{
  MV_ALONGW_LRL,                        ///<動作コード
  MMDL_AlongWallLRL_Init,                ///<初期化関数
  MMDL_AlongWall_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
///  MV_ALONGW_LRR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_AlongWallLRR =
{
  MV_ALONGW_LRR,                        ///<動作コード
  MMDL_AlongWallLRR_Init,                ///<初期化関数
  MMDL_AlongWall_Move,                  ///<動作関数
  MMDL_MoveDeleteProcDummy,              ///<削除関数
  MMDL_MoveReturnProcDummy,              ///<復帰関数
};

//--------------------------------------------------------------
/// MV_COMM_ACTOR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_CommActor =
{
  MV_COMM_ACTOR,                        ///<動作コード
  MMDL_MoveCommActor_Init,            ///<初期化関数
  MMDL_MoveCommActor_Move,            ///<動作関数
  MMDL_MoveCommActor_Delete,          ///<削除関数
  MMDL_MoveReturnProcDummy,          ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DMY
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDmy =
{
  MV_RAIL_DMY,                  ///<動作コード
  MMDL_RailDmy_Init,            ///<初期化関数
  MMDL_RailDmy_Move,            ///<動作関数
  MMDL_RailDmy_Delete,          ///<削除関数
  MMDL_RailDmy_Return,          ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_ALL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndALL =
{
  MV_RAIL_DIR_RND_ALL,          ///<動作コード
  MMDL_RailDirRnd_ALL_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_UL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndUL =
{
  MV_RAIL_DIR_RND_UL,          ///<動作コード
  MMDL_RailDirRnd_UL_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_UR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndUR =
{
  MV_RAIL_DIR_RND_UR,          ///<動作コード
  MMDL_RailDirRnd_UR_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_DL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndDL =
{
  MV_RAIL_DIR_RND_DL,          ///<動作コード
  MMDL_RailDirRnd_DL_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_DR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndDR =
{
  MV_RAIL_DIR_RND_DR,          ///<動作コード
  MMDL_RailDirRnd_DR_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_UDL
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndUDL =
{
  MV_RAIL_DIR_RND_UDL,          ///<動作コード
  MMDL_RailDirRnd_UDL_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_UDR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndUDR =
{
  MV_RAIL_DIR_RND_UDR,          ///<動作コード
  MMDL_RailDirRnd_UDR_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_ULR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndULR =
{
  MV_RAIL_DIR_RND_ULR,          ///<動作コード
  MMDL_RailDirRnd_ULR_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_DLR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndDLR =
{
  MV_RAIL_DIR_RND_DLR,          ///<動作コード
  MMDL_RailDirRnd_DLR_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_UD
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndUD =
{
  MV_RAIL_DIR_RND_UD,          ///<動作コード
  MMDL_RailDirRnd_UD_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_DIR_RND_LR
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailDirRndLR =
{
  MV_RAIL_DIR_RND_LR,          ///<動作コード
  MMDL_RailDirRnd_LR_Init,      ///<初期化関数
  MMDL_RailDirRnd_Move,          ///<動作関数
  MMDL_RailDirRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_RND
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailRnd =
{
  MV_RAIL_RND,          ///<動作コード
  MMDL_RailRnd_ALL_Init,      ///<初期化関数
  MMDL_RailRnd_Move,          ///<動作関数
  MMDL_RailRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_RND_V
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailRndV =
{
  MV_RAIL_RND_V,          ///<動作コード
  MMDL_RailRnd_V_Init,      ///<初期化関数
  MMDL_RailRnd_Move,          ///<動作関数
  MMDL_RailRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_RND_H
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailRndH =
{
  MV_RAIL_RND_H,          ///<動作コード
  MMDL_RailRnd_V_Init,      ///<初期化関数
  MMDL_RailRnd_Move,          ///<動作関数
  MMDL_RailRnd_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};

//--------------------------------------------------------------
/// MV_RAIL_RT2
//--------------------------------------------------------------
static const MMDL_MOVE_PROC_LIST DATA_FieldOBJMove_RailRoot2 =
{
  MV_RAIL_RT2,            ///<動作コード
  MMDL_RailRoot2_Init,      ///<初期化関数
  MMDL_RailRoot2_Move,          ///<動作関数
  MMDL_RailRoot2_Delete,        ///<削除関数
  MMDL_RailDmy_Return,      ///<復帰関数
};




//==============================================================================
//  フィールド動作モデル 動作関数まとめ
//==============================================================================
//--------------------------------------------------------------
///  動作関数まとめ
///  並びはfieldobj_code.h MV_DMY等のコードと一致
//--------------------------------------------------------------
const MMDL_MOVE_PROC_LIST * const DATA_FieldOBJMoveProcListTbl[MV_CODE_MAX] =
{
  &DATA_FieldOBJMove_Dummy,                //MV_DMY
  &DATA_FieldOBJMove_Player,                //MV_PLAYER
  &DATA_FieldOBJMove_DirRnd,                //MV_DIR_RND
  &DATA_FieldOBJMove_Rnd,                  //MV_RND
  &DATA_FieldOBJMove_RndV,                //MV_RND_V
  &DATA_FieldOBJMove_RndH,                //MV_RND_H
  &DATA_FieldOBJMove_RndUL,                //MV_RND_UL
  &DATA_FieldOBJMove_RndUR,                //MV_RND_UR
  &DATA_FieldOBJMove_RndDL,                //MV_RND_DL
  &DATA_FieldOBJMove_RndDR,                //MV_RND_DR
  &DATA_FieldOBJMove_RndUDL,                //MV_RND_UDL
  &DATA_FieldOBJMove_RndUDR,                //MV_RND_UDR
  &DATA_FieldOBJMove_RndULR,                //MV_RND_ULR
  &DATA_FieldOBJMove_RndDLR,                //MV_RND_DLR
  &DATA_FieldOBJMove_Up,                  //MV_UP
  &DATA_FieldOBJMove_Down,                //MV_DOWN
  &DATA_FieldOBJMove_Left,                //MV_LEFT
  &DATA_FieldOBJMove_Right,                //MV_RIGHT
  &DATA_FieldOBJMove_SpinLeft,              //MV_SPIN_L
  &DATA_FieldOBJMove_SpinRight,              //MV_SPIN_R
  &DATA_FieldOBJMove_Route2,                //MV_RT2
  &DATA_FieldOBJMove_RouteURLD,              //MV_RTURLD
  &DATA_FieldOBJMove_RouteRLDU,              //MV_RTRLDU
  &DATA_FieldOBJMove_RouteDURL,              //MV_RTDURL
  &DATA_FieldOBJMove_RouteLDUR,              //MV_RTLDUR
  &DATA_FieldOBJMove_RouteULRD,              //MV_RTULRD
  &DATA_FieldOBJMove_RouteLRDU,              //MV_RTLRDU
  &DATA_FieldOBJMove_RouteDULR,              //MV_RTDULR
  &DATA_FieldOBJMove_RouteRDUL,              //MV_RTRDUL
  &DATA_FieldOBJMove_RouteLUDR,              //MV_RTLUDR
  &DATA_FieldOBJMove_RouteUDRL,              //MV_RTUDRL
  &DATA_FieldOBJMove_RouteRLUD,              //MV_RTRLUD
  &DATA_FieldOBJMove_RouteDRLU,              //MV_RTDRLU
  &DATA_FieldOBJMove_RouteRUDL,              //MV_RTRUDL
  &DATA_FieldOBJMove_RouteUDLR,              //MV_RTUDLR
  &DATA_FieldOBJMove_RouteLRUD,              //MV_RTLRUD
  &DATA_FieldOBJMove_RouteDLRU,              //MV_RTDLRU
  &DATA_FieldOBJMove_RouteUL,                //MV_UL
  &DATA_FieldOBJMove_RouteDR,                //MV_DR
  &DATA_FieldOBJMove_RouteLD,                //MV_LD
  &DATA_FieldOBJMove_RouteRU,                //MV_RU
  &DATA_FieldOBJMove_RouteUR,                //MV_UR
  &DATA_FieldOBJMove_RouteDL,                //MV_DL
  &DATA_FieldOBJMove_RouteLU,                //MV_LU
  &DATA_FieldOBJMove_RouteRD,                //MV_RD
  &DATA_FieldOBJMove_RndUD,                //MV_RND_UD
  &DATA_FieldOBJMove_RndLR,                //MV_RND_LR
#if 0
  &DATA_FieldOBJMove_Seed,                //MV_SEED
#else
  NULL,
#endif
  &DATA_FieldOBJMove_Pair,                //MV_PAIR
  &DATA_FieldOBJMove_Rewar,                //MV_REWAR
  &DATA_FieldOBJMove_PairTr,                //MV_TR_PAIR
  &DATA_FieldOBJMove_HideSnow,              //MV_HIDE_SNOW
  &DATA_FieldOBJMove_HideSand,              //MV_HIDE_SAND
  &DATA_FieldOBJMove_HideGround,              //MV_HIDE_GRND
  &DATA_FieldOBJMove_HideKusa,              //MV_HIDE_KUSA
  &DATA_FieldOBJMove_CopyU,                //MV_COPY_U
  &DATA_FieldOBJMove_CopyD,                //MV_COPY_D
  &DATA_FieldOBJMove_CopyL,                //MV_COPY_L
  &DATA_FieldOBJMove_CopyR,                //MV_COPYLGRASS_R
  &DATA_FieldOBJMove_CopyLGrassU,              //MV_COPYLGRASS_U
  &DATA_FieldOBJMove_CopyLGrassD,              //MV_COPYLGRASS_D
  &DATA_FieldOBJMove_CopyLGrassL,              //MV_COPYLGRASS_L
  &DATA_FieldOBJMove_CopyLGrassR,              //MV_COPYLGRASS_R
  &DATA_FieldOBJMove_AlongWallL,              //MV_ALONGW_L
  &DATA_FieldOBJMove_AlongWallR,              //MV_ALONGW_R
  &DATA_FieldOBJMove_AlongWallLRL,            //MV_ALONGW_LRL
  &DATA_FieldOBJMove_AlongWallLRR,            //MV_ALONGW_LRR
  &DATA_FieldOBJMove_RndHLim,                //MV_RND_H_LIM
  &DATA_FieldOBJMove_CommActor,           //MV_COMM_ACTOR
  &DATA_FieldOBJMove_RailDmy,             //MV_RAIL_DMY
  &DATA_FieldOBJMove_RailDirRndALL,       //MV_RAIL_DIR_RND_ALL
  &DATA_FieldOBJMove_RailDirRndUL,       //MV_RAIL_DIR_RND_UL
  &DATA_FieldOBJMove_RailDirRndUR,       //MV_RAIL_DIR_RND_UR
  &DATA_FieldOBJMove_RailDirRndDL,       //MV_RAIL_DIR_RND_DL
  &DATA_FieldOBJMove_RailDirRndDR,       //MV_RAIL_DIR_RND_DR
  &DATA_FieldOBJMove_RailDirRndUDL,       //MV_RAIL_DIR_RND_UDL
  &DATA_FieldOBJMove_RailDirRndUDR,       //MV_RAIL_DIR_RND_UDR
  &DATA_FieldOBJMove_RailDirRndULR,       //MV_RAIL_DIR_RND_ULR
  &DATA_FieldOBJMove_RailDirRndDLR,       //MV_RAIL_DIR_RND_DLR
  &DATA_FieldOBJMove_RailDirRndUD,       //MV_RAIL_DIR_RND_UD
  &DATA_FieldOBJMove_RailDirRndLR,       //MV_RAIL_DIR_RND_LR
  &DATA_FieldOBJMove_RailRnd,             //MV_RAIL_RND   
  &DATA_FieldOBJMove_RailRndV,            //MV_RAIL_RND_V
  &DATA_FieldOBJMove_RailRndH,            //MV_RAIL_RND_H
  &DATA_FieldOBJMove_RailRoot2,           ///<ルート2分岐
};

//==============================================================================
//  フィールド動作モデル アニメーションコマンド
//==============================================================================
//--------------------------------------------------------------
///  アニメーションコマンドテーブル。
///  並びはfieldobj_code.h AC_DIR_U等のコードと一致。
//--------------------------------------------------------------
int (* const * const DATA_AcmdActionTbl[ACMD_MAX])( MMDL * ) =
{
  DATA_AC_DirU_Tbl,                    //AC_DIR_U
  DATA_AC_DirD_Tbl,                    //AC_DIR_D
  DATA_AC_DirL_Tbl,                    //AC_DIR_L
  DATA_AC_DirR_Tbl,                    //AC_DIR_R
  DATA_AC_WalkU_32F_Tbl,                  //AC_WALK_U_32F
  DATA_AC_WalkD_32F_Tbl,                  //AC_WALK_D_32F
  DATA_AC_WalkL_32F_Tbl,                  //AC_WALK_L_32F
  DATA_AC_WalkR_32F_Tbl,                  //AC_WALK_R_32F
  DATA_AC_WalkU_16F_Tbl,                  //AC_WALK_U_16F
  DATA_AC_WalkD_16F_Tbl,                  //AC_WALK_D_16F
  DATA_AC_WalkL_16F_Tbl,                  //AC_WALK_L_16F
  DATA_AC_WalkR_16F_Tbl,                  //AC_WALK_R_16F
  DATA_AC_WalkU_8F_Tbl,                  //AC_WALK_U_8F
  DATA_AC_WalkD_8F_Tbl,                  //AC_WALK_D_8F
  DATA_AC_WalkL_8F_Tbl,                  //AC_WALK_L_8F
  DATA_AC_WalkR_8F_Tbl,                  //AC_WALK_R_8F
  DATA_AC_WalkU_4F_Tbl,                  //AC_WALK_U_4F
  DATA_AC_WalkD_4F_Tbl,                  //AC_WALK_D_4F
  DATA_AC_WalkL_4F_Tbl,                  //AC_WALK_L_4F
  DATA_AC_WalkR_4F_Tbl,                  //AC_WALK_R_4F
  DATA_AC_WalkU_2F_Tbl,                  //AC_WALK_U_2F
  DATA_AC_WalkD_2F_Tbl,                  //AC_WALK_D_2F
  DATA_AC_WalkL_2F_Tbl,                  //AC_WALK_L_2F
  DATA_AC_WalkR_2F_Tbl,                  //AC_WALK_R_2F
  DATA_AC_StayWalkU_32F_Tbl,                //AC_STAY_WALK_U_32F
  DATA_AC_StayWalkD_32F_Tbl,                //AC_STAY_WALK_D_32F
  DATA_AC_StayWalkL_32F_Tbl,                //AC_STAY_WALK_L_32F
  DATA_AC_StayWalkR_32F_Tbl,                //AC_STAY_WALK_R_32F
  DATA_AC_StayWalkU_16F_Tbl,                //AC_STAY_WALK_U_16F
  DATA_AC_StayWalkD_16F_Tbl,                //AC_STAY_WALK_D_16F
  DATA_AC_StayWalkL_16F_Tbl,                //AC_STAY_WALK_L_16F
  DATA_AC_StayWalkR_16F_Tbl,                //AC_STAY_WALK_R_16F
  DATA_AC_StayWalkU_8F_Tbl,                //AC_STAY_WALK_U_8F
  DATA_AC_StayWalkD_8F_Tbl,                //AC_STAY_WALK_D_8F
  DATA_AC_StayWalkL_8F_Tbl,                //AC_STAY_WALK_L_8F
  DATA_AC_StayWalkR_8F_Tbl,                //AC_STAY_WALK_R_8F
  DATA_AC_StayWalkU_4F_Tbl,                //AC_STAY_WALK_U_4F
  DATA_AC_StayWalkD_4F_Tbl,                //AC_STAY_WALK_D_4F
  DATA_AC_StayWalkL_4F_Tbl,                //AC_STAY_WALK_L_4F
  DATA_AC_StayWalkR_4F_Tbl,                //AC_STAY_WALK_R_4F
  DATA_AC_StayWalkU_2F_Tbl,                //AC_STAY_WALK_U_2F
  DATA_AC_StayWalkD_2F_Tbl,                //AC_STAY_WALK_D_2F
  DATA_AC_StayWalkL_2F_Tbl,                //AC_STAY_WALK_L_2F
  DATA_AC_StayWalkR_2F_Tbl,                //AC_STAY_WALK_R_2F
  DATA_AC_StayJumpU_16F_Tbl,                //AC_STAY_JUMP_U_16F
  DATA_AC_StayJumpD_16F_Tbl,                //AC_STAY_JUMP_D_16F
  DATA_AC_StayJumpL_16F_Tbl,                //AC_STAY_JUMP_L_16F
  DATA_AC_StayJumpR_16F_Tbl,                //AC_STAY_JUMP_R_16F
  DATA_AC_StayJumpU_8F_Tbl,                //AC_STAY_JUMP_U_8F
  DATA_AC_StayJumpD_8F_Tbl,                //AC_STAY_JUMP_D_8F
  DATA_AC_StayJumpL_8F_Tbl,                //AC_STAY_JUMP_L_8F
  DATA_AC_StayJumpR_8F_Tbl,                //AC_STAY_JUMP_R_8F
  DATA_AC_JumpU_1G_8F_Tbl,                //AC_JUMP_U_1G_8F
  DATA_AC_JumpD_1G_8F_Tbl,                //AC_JUMP_D_1G_8F
  DATA_AC_JumpL_1G_8F_Tbl,                //AC_JUMP_L_1G_8F
  DATA_AC_JumpR_1G_8F_Tbl,                //AC_JUMP_R_1G_8F
  DATA_AC_JumpU_2G_16F_Tbl,                //AC_JUMP_U_2G_16F
  DATA_AC_JumpD_2G_16F_Tbl,                //AC_JUMP_D_2G_16F
  DATA_AC_JumpL_2G_16F_Tbl,                //AC_JUMP_L_2G_16F
  DATA_AC_JumpR_2G_16F_Tbl,                //AC_JUMP_R_2G_16F
  DATA_AC_Wait_1F_Tbl,                  //AC_WAIT_1F
  DATA_AC_Wait_2F_Tbl,                  //AC_WAIT_2F
  DATA_AC_Wait_4F_Tbl,                  //AC_WAIT_4F
  DATA_AC_Wait_8F_Tbl,                  //AC_WAIT_8F
  DATA_AC_Wait_15F_Tbl,                  //AC_WAIT_15F
  DATA_AC_Wait_16F_Tbl,                  //AC_WAIT_16F
  DATA_AC_Wait_32F_Tbl,                  //AC_WAIT_32F
  DATA_AC_WarpUp_Tbl,                    //AC_WARP_UP
  DATA_AC_WarpDown_Tbl,                  //AC_WARP_DOWN
  DATA_AC_VanishON_Tbl,                  //AC_VANISH_ON
  DATA_AC_VanishOFF_Tbl,                  //AC_VANISH_OFF
  DATA_AC_DirPauseON_Tbl,                  //AC_DIR_PAUSE_ON
  DATA_AC_DirPauseOFF_Tbl,                //AC_DIR_PAUSE_OFF
  DATA_AC_AnmPauseON_Tbl,                  //AC_ANM_PAUSE_ON
  DATA_AC_AnmPauseOFF_Tbl,                //AC_ANM_PAUSE_OFF
  DATA_AC_MarkGyoe_Tbl,                  //AC_MARK_GYOE
  DATA_AC_WalkU6F_Tbl,                  //AC_WALK_U_6F
  DATA_AC_WalkD6F_Tbl,                  //AC_WALK_D_6F
  DATA_AC_WalkL6F_Tbl,                  //AC_WALK_L_6F
  DATA_AC_WalkR6F_Tbl,                  //AC_WALK_R_6F
  DATA_AC_WalkU3F_Tbl,                  //AC_WALK_U_3F
  DATA_AC_WalkD3F_Tbl,                  //AC_WALK_D_3F
  DATA_AC_WalkL3F_Tbl,                  //AC_WALK_L_3F
  DATA_AC_WalkR3F_Tbl,                  //AC_WALK_R_3F
  DATA_AC_WalkU_1F_Tbl,                  //AC_WALK_U_1F
  DATA_AC_WalkD_1F_Tbl,                  //AC_WALK_D_1F
  DATA_AC_WalkL_1F_Tbl,                  //AC_WALK_L_1F
  DATA_AC_WalkR_1F_Tbl,                  //AC_WALK_R_1F
  DATA_AC_DashU_4F_Tbl,                  //AC_DASH_U_4F
  DATA_AC_DashD_4F_Tbl,                  //AC_DASH_D_4F
  DATA_AC_DashL_4F_Tbl,                  //AC_DASH_L_4F
  DATA_AC_DashR_4F_Tbl,                  //AC_DASH_R_4F
  DATA_AC_JumpHiL_1G_16F_Tbl,                //AC_JUMPHI_L_1G_16F
  DATA_AC_JumpHiR_1G_16F_Tbl,                //AC_JUMPHI_R_1G_16F
  DATA_AC_JumpHiL_3G_32F_Tbl,                //AC_JUMPHI_L_3G_32F
  DATA_AC_JumpHiR_3G_32F_Tbl,                //AC_JUMPHI_R_3G_32F
  DATA_AC_WalkU7F_Tbl,                  //AC_WALK_U_7F
  DATA_AC_WalkD7F_Tbl,                  //AC_WALK_D_7F
  DATA_AC_WalkL7F_Tbl,                  //AC_WALK_L_7F
  DATA_AC_WalkR7F_Tbl,                  //AC_WALK_R_7F
  DATA_AC_PcBow_Tbl,                    //AC_PC_BOW
  DATA_AC_HidePullOFF_Tbl,                //AC_HIDE_PULLOFF
  DATA_AC_HeroBanzai_Tbl,                  //AC_HERO_BANZAI
  DATA_AC_MarkSaisen_Tbl,                  //AC_MARK_SAISEN
  DATA_AC_HeroBanzaiUke_Tbl,                //AC_HERO_BANZAI_UKE
  DATA_AC_WalkGLU8F_Tbl,                  //AC_WALKGL_U_8F
  DATA_AC_WalkGLD8F_Tbl,                  //AC_WALKGL_D_8F
  DATA_AC_WalkGLL8F_Tbl,                  //AC_WALKGL_L_8F
  DATA_AC_WalkGLR8F_Tbl,                  //AC_WALKGL_R_8F
  
  DATA_AC_WalkGRU8F_Tbl,                  //AC_WALKGL_U_8F
  DATA_AC_WalkGRD8F_Tbl,                  //AC_WALKGL_D_8F
  DATA_AC_WalkGRL8F_Tbl,                  //AC_WALKGL_L_8F
  DATA_AC_WalkGRR8F_Tbl,                  //AC_WALKGL_R_8F
  
  DATA_AC_WalkGUU8F_Tbl,                  //AC_WALKGU_U_8F
  DATA_AC_WalkGUD8F_Tbl,                  //AC_WALKGU_D_8F
  DATA_AC_WalkGUL8F_Tbl,                  //AC_WALKGU_L_8F
  DATA_AC_WalkGUR8F_Tbl,                  //AC_WALKGU_R_8F
  
  DATA_AC_JumpU_3G_24F_Tbl,                //AC_JUMP_U_3G_24F
  DATA_AC_JumpD_3G_24F_Tbl,                //AC_JUMP_D_3G_24F
  DATA_AC_JumpL_3G_24F_Tbl,                //AC_JUMP_L_3G_24F
  DATA_AC_JumpR_3G_24F_Tbl,                //AC_JUMP_R_3G_24F
  
  DATA_AC_DashGLU4F_Tbl,                  //AC_DASHGL_U_8F
  DATA_AC_DashGLD4F_Tbl,                  //AC_DASHGL_D_8F
  DATA_AC_DashGLL4F_Tbl,                  //AC_DASHGL_L_8F
  DATA_AC_DashGLR4F_Tbl,                  //AC_DASHGL_R_8F
  
  DATA_AC_DashGRU4F_Tbl,                  //AC_DASHGL_U_8F
  DATA_AC_DashGRD4F_Tbl,                  //AC_DASHGL_D_8F
  DATA_AC_DashGRL4F_Tbl,                  //AC_DASHGL_L_8F
  DATA_AC_DashGRR4F_Tbl,                  //AC_DASHGL_R_8F
  
  DATA_AC_DashGUU4F_Tbl,                  //AC_DASHGU_U_8F
  DATA_AC_DashGUD4F_Tbl,                  //AC_DASHGU_D_8F
  DATA_AC_DashGUL4F_Tbl,                  //AC_DASHGU_L_8F
  DATA_AC_DashGUR4F_Tbl,                  //AC_DASHGU_R_8F
  
  DATA_AC_JumpGLU1G_8F_Tbl,              //AC_JUMPGL_U_1G_8F
  DATA_AC_JumpGLD1G_8F_Tbl,
  DATA_AC_JumpGLL1G_8F_Tbl,
  DATA_AC_JumpGLR1G_8F_Tbl,
  
  DATA_AC_JumpGRU1G_8F_Tbl,
  DATA_AC_JumpGRD1G_8F_Tbl,
  DATA_AC_JumpGRL1G_8F_Tbl,
  DATA_AC_JumpGRR1G_8F_Tbl,
  
  DATA_AC_JumpGUU1G_8F_Tbl,
  DATA_AC_JumpGUD1G_8F_Tbl,
  DATA_AC_JumpGUL1G_8F_Tbl,
  DATA_AC_JumpGUR1G_8F_Tbl,              //AC_JUMPGU_R_1G_8F
  
  DATA_AC_WalkGUU4F_Tbl,                  //AC_WALKGU_U_4F
  DATA_AC_WalkGUD4F_Tbl,                  //AC_WALKGU_D_4F
  DATA_AC_WalkGUL4F_Tbl,                  //AC_WALKGU_L_4F
  DATA_AC_WalkGUR4F_Tbl,                  //AC_WALKGU_R_4F
  
  DATA_AC_WalkGUU2F_Tbl,                  //AC_WALKGU_U_2F
  DATA_AC_WalkGUD2F_Tbl,                  //AC_WALKGU_D_2F
  DATA_AC_WalkGUL2F_Tbl,                  //AC_WALKGU_L_2F
  DATA_AC_WalkGUR2F_Tbl,                  //AC_WALKGU_R_2F
  
  DATA_AC_MarkGyoeTWait_Tbl,              //AC_MARK_GYOE_TWAIT
  
  DATA_AC_HeroItemGet_Tbl,              // AC_HERO_ITEMGET
  
  DATA_AC_SandWalkU_16F_Tbl,                  //AC_SANDWALK_U_16F
  DATA_AC_SandWalkD_16F_Tbl,                  //AC_SANDWALK_D_16F
  DATA_AC_SandWalkL_16F_Tbl,                  //AC_SANDWALK_L_16F
  DATA_AC_SandWalkR_16F_Tbl,                  //AC_SANDWALK_R_16F
  
  DATA_AC_MarkHatena_Tbl,             //AC_MARK_HATENA
  DATA_AC_MarkOnpu_Tbl,               //AC_MARK_ONPU
  DATA_AC_MarkTenTen_Tbl,             //AC_MARK_TENTEN
};

//--------------------------------------------------------------
///  レール動作用  アニメーションコマンドテーブル。
///  並びはfieldobj_code.h AC_DIR_U等のコードと一致。
//--------------------------------------------------------------
int (* const * const DATA_RailAcmdActionTbl[ACMD_MAX])( MMDL * ) =
{
  DATA_AC_RailDirU_Tbl,                    //AC_DIR_U
  DATA_AC_RailDirD_Tbl,                //AC_DIR_D
  DATA_AC_RailDirL_Tbl,                //AC_DIR_L
  DATA_AC_RailDirR_Tbl,                //AC_DIR_R
  DATA_AC_RailDummy,                  //AC_WALK_U_32F
  DATA_AC_RailDummy,                  //AC_WALK_D_32F
  DATA_AC_RailDummy,                  //AC_WALK_L_32F
  DATA_AC_RailDummy,                  //AC_WALK_R_32F
  DATA_AC_RailWalkU_16F_Tbl,          //AC_WALK_U_16F
  DATA_AC_RailWalkD_16F_Tbl,          //AC_WALK_D_16F
  DATA_AC_RailWalkL_16F_Tbl,          //AC_WALK_L_16F
  DATA_AC_RailWalkR_16F_Tbl,          //AC_WALK_R_16F
  DATA_AC_RailWalkU_8F_Tbl,            //AC_WALK_U_8F
  DATA_AC_RailWalkD_8F_Tbl,            //AC_WALK_D_8F
  DATA_AC_RailWalkL_8F_Tbl,            //AC_WALK_L_8F
  DATA_AC_RailWalkR_8F_Tbl,            //AC_WALK_R_8F
  DATA_AC_RailWalkU_4F_Tbl,            //AC_WALK_U_4F
  DATA_AC_RailWalkD_4F_Tbl,            //AC_WALK_D_4F
  DATA_AC_RailWalkL_4F_Tbl,            //AC_WALK_L_4F
  DATA_AC_RailWalkR_4F_Tbl,            //AC_WALK_R_4F
  DATA_AC_RailWalkU_2F_Tbl,            //AC_WALK_U_2F
  DATA_AC_RailWalkD_2F_Tbl,            //AC_WALK_D_2F
  DATA_AC_RailWalkL_2F_Tbl,            //AC_WALK_L_2F
  DATA_AC_RailWalkR_2F_Tbl,            //AC_WALK_R_2F
  DATA_AC_RailDummy,                //AC_STAY_WALK_U_32F
  DATA_AC_RailDummy,                //AC_STAY_WALK_D_32F
  DATA_AC_RailDummy,                //AC_STAY_WALK_L_32F
  DATA_AC_RailDummy,                //AC_STAY_WALK_R_32F
  DATA_AC_RailStayWalkU_16F_Tbl,                //AC_STAY_WALK_U_16F
  DATA_AC_RailStayWalkD_16F_Tbl,                //AC_STAY_WALK_D_16F
  DATA_AC_RailStayWalkL_16F_Tbl,                //AC_STAY_WALK_L_16F
  DATA_AC_RailStayWalkR_16F_Tbl,                //AC_STAY_WALK_R_16F
  DATA_AC_RailStayWalkU_8F_Tbl,                //AC_STAY_WALK_U_8F
  DATA_AC_RailStayWalkD_8F_Tbl,                //AC_STAY_WALK_D_8F
  DATA_AC_RailStayWalkL_8F_Tbl,                //AC_STAY_WALK_L_8F
  DATA_AC_RailStayWalkR_8F_Tbl,                //AC_STAY_WALK_R_8F
  DATA_AC_RailDummy,                //AC_STAY_WALK_U_4F
  DATA_AC_RailDummy,                //AC_STAY_WALK_D_4F
  DATA_AC_RailDummy,                //AC_STAY_WALK_L_4F
  DATA_AC_RailDummy,                //AC_STAY_WALK_R_4F
  DATA_AC_RailStayWalkU_2F_Tbl,                //AC_STAY_WALK_U_2F
  DATA_AC_RailStayWalkD_2F_Tbl,                //AC_STAY_WALK_D_2F
  DATA_AC_RailStayWalkL_2F_Tbl,                //AC_STAY_WALK_L_2F
  DATA_AC_RailStayWalkR_2F_Tbl,                //AC_STAY_WALK_R_2F
  DATA_AC_RailDummy,                //AC_STAY_JUMP_U_16F
  DATA_AC_RailDummy,                //AC_STAY_JUMP_D_16F
  DATA_AC_RailDummy,                //AC_STAY_JUMP_L_16F
  DATA_AC_RailDummy,                //AC_STAY_JUMP_R_16F
  DATA_AC_RailDummy,                //AC_STAY_JUMP_U_8F
  DATA_AC_RailDummy,                //AC_STAY_JUMP_D_8F
  DATA_AC_RailDummy,                //AC_STAY_JUMP_L_8F
  DATA_AC_RailDummy,                //AC_STAY_JUMP_R_8F
  DATA_AC_RailJumpU_1G_8F_Tbl,                //AC_JUMP_U_1G_8F
  DATA_AC_RailJumpD_1G_8F_Tbl,                //AC_JUMP_D_1G_8F
  DATA_AC_RailJumpL_1G_8F_Tbl,                //AC_JUMP_L_1G_8F
  DATA_AC_RailJumpR_1G_8F_Tbl,                //AC_JUMP_R_1G_8F
  DATA_AC_RailJumpU_2G_16F_Tbl,                //AC_JUMP_U_2G_16F
  DATA_AC_RailJumpD_2G_16F_Tbl,                //AC_JUMP_D_2G_16F
  DATA_AC_RailJumpL_2G_16F_Tbl,                //AC_JUMP_L_2G_16F
  DATA_AC_RailJumpR_2G_16F_Tbl,                //AC_JUMP_R_2G_16F
  DATA_AC_RailDummy,                  //AC_WAIT_1F
  DATA_AC_RailDummy,                  //AC_WAIT_2F
  DATA_AC_RailDummy,                  //AC_WAIT_4F
  DATA_AC_RailDummy,                  //AC_WAIT_8F
  DATA_AC_RailDummy,                  //AC_WAIT_15F
  DATA_AC_RailDummy,                  //AC_WAIT_16F
  DATA_AC_RailDummy,                  //AC_WAIT_32F
  DATA_AC_RailDummy,                    //AC_WARP_UP
  DATA_AC_RailDummy,                  //AC_WARP_DOWN
  DATA_AC_RailDummy,                  //AC_VANISH_ON
  DATA_AC_RailDummy,                  //AC_VANISH_OFF
  DATA_AC_RailDummy,                  //AC_DIR_PAUSE_ON
  DATA_AC_RailDummy,                //AC_DIR_PAUSE_OFF
  DATA_AC_RailDummy,                  //AC_ANM_PAUSE_ON
  DATA_AC_RailDummy,                //AC_ANM_PAUSE_OFF
  DATA_AC_RailDummy,                  //AC_MARK_GYOE
  DATA_AC_RailDummy,                  //AC_WALK_U_6F
  DATA_AC_RailDummy,                  //AC_WALK_D_6F
  DATA_AC_RailDummy,                  //AC_WALK_L_6F
  DATA_AC_RailDummy,                  //AC_WALK_R_6F
  DATA_AC_RailDummy,                  //AC_WALK_U_3F
  DATA_AC_RailDummy,                  //AC_WALK_D_3F
  DATA_AC_RailDummy,                  //AC_WALK_L_3F
  DATA_AC_RailDummy,                  //AC_WALK_R_3F
  DATA_AC_RailDummy,                  //AC_WALK_U_1F
  DATA_AC_RailDummy,                  //AC_WALK_D_1F
  DATA_AC_RailDummy,                  //AC_WALK_L_1F
  DATA_AC_RailDummy,                  //AC_WALK_R_1F
  DATA_AC_RailDashU_4F_Tbl,                  //AC_DASH_U_4F
  DATA_AC_RailDashD_4F_Tbl,                  //AC_DASH_D_4F
  DATA_AC_RailDashL_4F_Tbl,                  //AC_DASH_L_4F
  DATA_AC_RailDashR_4F_Tbl,                  //AC_DASH_R_4F
  DATA_AC_RailDummy,                //AC_JUMPHI_L_1G_16F
  DATA_AC_RailDummy,                //AC_JUMPHI_R_1G_16F
  DATA_AC_RailDummy,                //AC_JUMPHI_L_3G_32F
  DATA_AC_RailDummy,                //AC_JUMPHI_R_3G_32F
  DATA_AC_RailDummy,                  //AC_WALK_U_7F
  DATA_AC_RailDummy,                  //AC_WALK_D_7F
  DATA_AC_RailDummy,                  //AC_WALK_L_7F
  DATA_AC_RailDummy,                  //AC_WALK_R_7F
  DATA_AC_RailDummy,                    //AC_PC_BOW
  DATA_AC_RailDummy,                //AC_HIDE_PULLOFF
  DATA_AC_RailDummy,                  //AC_HERO_BANZAI
  DATA_AC_RailDummy,                  //AC_MARK_SAISEN
  DATA_AC_RailDummy,                //AC_HERO_BANZAI_UKE
  DATA_AC_RailDummy,                  //AC_WALKGL_U_8F
  DATA_AC_RailDummy,                  //AC_WALKGL_D_8F
  DATA_AC_RailDummy,                  //AC_WALKGL_L_8F
  DATA_AC_RailDummy,                  //AC_WALKGL_R_8F
  DATA_AC_RailDummy,                  //AC_WALKGL_U_8F
  DATA_AC_RailDummy,                  //AC_WALKGL_D_8F
  DATA_AC_RailDummy,                  //AC_WALKGL_L_8F
  DATA_AC_RailDummy,                  //AC_WALKGL_R_8F
  DATA_AC_RailDummy,                  //AC_WALKGU_U_8F
  DATA_AC_RailDummy,                  //AC_WALKGU_D_8F
  DATA_AC_RailDummy,                  //AC_WALKGU_L_8F
  DATA_AC_RailDummy,                  //AC_WALKGU_R_8F
  DATA_AC_RailDummy,                //AC_JUMP_U_3G_24F
  DATA_AC_RailDummy,                //AC_JUMP_D_3G_24F
  DATA_AC_RailDummy,                //AC_JUMP_L_3G_24F
  DATA_AC_RailDummy,                //AC_JUMP_R_3G_24F
  DATA_AC_RailDummy,                  //AC_DASHGL_U_8F
  DATA_AC_RailDummy,                  //AC_DASHGL_D_8F
  DATA_AC_RailDummy,                  //AC_DASHGL_L_8F
  DATA_AC_RailDummy,                  //AC_DASHGL_R_8F
  DATA_AC_RailDummy,                  //AC_DASHGL_U_8F
  DATA_AC_RailDummy,                  //AC_DASHGL_D_8F
  DATA_AC_RailDummy,                  //AC_DASHGL_L_8F
  DATA_AC_RailDummy,                  //AC_DASHGL_R_8F
  DATA_AC_RailDummy,                  //AC_DASHGU_U_8F
  DATA_AC_RailDummy,                  //AC_DASHGU_D_8F
  DATA_AC_RailDummy,                  //AC_DASHGU_L_8F
  DATA_AC_RailDummy,                  //AC_DASHGU_R_8F
  DATA_AC_RailDummy,              //AC_JUMPGL_U_1G_8F
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,
  DATA_AC_RailDummy,              //AC_JUMPGU_R_1G_8F
  DATA_AC_RailDummy,                  //AC_WALKGU_U_4F
  DATA_AC_RailDummy,                  //AC_WALKGU_D_4F
  DATA_AC_RailDummy,                  //AC_WALKGU_L_4F
  DATA_AC_RailDummy,                  //AC_WALKGU_R_4F
  DATA_AC_RailDummy,                  //AC_WALKGU_U_2F
  DATA_AC_RailDummy,                  //AC_WALKGU_D_2F
  DATA_AC_RailDummy,                  //AC_WALKGU_L_2F
  DATA_AC_RailDummy,                  //AC_WALKGU_R_2F
  DATA_AC_RailDummy,              //AC_MARK_GYOE_TWAIT
  DATA_AC_HeroItemGet_Tbl,              // AC_HERO_ITEMGET
};

//==============================================================================
//  フィールド動作モデル アニメーションコマンドコード　方向まとめ
//==============================================================================
//--------------------------------------------------------------
///  コード別対応方行テーブル作成。
/// 並びはDIR_UP,DIR_DOWN,DIR_LEFT,DIR_RIGHT
//--------------------------------------------------------------
static const int DATA_CodeTbl_AC_DIR_U[] =
{AC_DIR_U,AC_DIR_D,AC_DIR_L,AC_DIR_R};

static const int DATA_CodeTbl_AC_WALK_U_32F[] =
{AC_WALK_U_32F,AC_WALK_D_32F,AC_WALK_L_32F,AC_WALK_R_32F };

static const int DATA_CodeTbl_AC_WALK_U_16F[] =
{AC_WALK_U_16F,AC_WALK_D_16F,AC_WALK_L_16F,AC_WALK_R_16F };

static const int DATA_CodeTbl_AC_WALK_U_8F[] =
{AC_WALK_U_8F,AC_WALK_D_8F,AC_WALK_L_8F,AC_WALK_R_8F };

static const int DATA_CodeTbl_AC_WALK_U_4F[] =
{AC_WALK_U_4F,AC_WALK_D_4F,AC_WALK_L_4F,AC_WALK_R_4F };

static const int DATA_CodeTbl_AC_WALK_U_2F[] =
{AC_WALK_U_2F,AC_WALK_D_2F,AC_WALK_L_2F,AC_WALK_R_2F };

static const int DATA_CodeTbl_AC_STAY_WALK_U_32F[] =
{AC_STAY_WALK_U_32F,AC_STAY_WALK_D_32F,AC_STAY_WALK_L_32F,AC_STAY_WALK_R_32F };

static const int DATA_CodeTbl_AC_STAY_WALK_U_16F[] =
{AC_STAY_WALK_U_16F,AC_STAY_WALK_D_16F,AC_STAY_WALK_L_16F,AC_STAY_WALK_R_16F };

static const int DATA_CodeTbl_AC_STAY_WALK_U_8F[] =
{AC_STAY_WALK_U_8F,AC_STAY_WALK_D_8F,AC_STAY_WALK_L_8F,AC_STAY_WALK_R_8F };

static const int DATA_CodeTbl_AC_STAY_WALK_U_4F[] =
{AC_STAY_WALK_U_4F,AC_STAY_WALK_D_4F,AC_STAY_WALK_L_4F,AC_STAY_WALK_R_4F };

static const int DATA_CodeTbl_AC_STAY_WALK_U_2F[] =
{AC_STAY_WALK_U_2F,AC_STAY_WALK_D_2F,AC_STAY_WALK_L_2F,AC_STAY_WALK_R_2F };

static const int DATA_CodeTbl_AC_STAY_JUMP_U_16F[] =
{AC_STAY_JUMP_U_16F,AC_STAY_JUMP_D_16F,AC_STAY_JUMP_L_16F,AC_STAY_JUMP_R_16F };

static const int DATA_CodeTbl_AC_STAY_JUMP_U_8F[] =
{AC_STAY_JUMP_U_8F,AC_STAY_JUMP_D_8F,AC_STAY_JUMP_L_8F,AC_STAY_JUMP_R_8F };

static const int DATA_CodeTbl_AC_JUMP_U_1G_8F[] =
{AC_JUMP_U_1G_8F,AC_JUMP_D_1G_8F,AC_JUMP_L_1G_8F,AC_JUMP_R_1G_8F };

static const int DATA_CodeTbl_AC_JUMP_U_2G_16F[] =
{AC_JUMP_U_2G_16F,AC_JUMP_D_2G_16F,AC_JUMP_L_2G_16F,AC_JUMP_R_2G_16F };

static const int DATA_CodeTbl_AC_WALK_U_6F[] =
{AC_WALK_U_6F,AC_WALK_D_6F,AC_WALK_L_6F,AC_WALK_R_6F };

static const int DATA_CodeTbl_AC_WALK_U_3F[] =
{AC_WALK_U_3F,AC_WALK_D_3F,AC_WALK_L_3F,AC_WALK_R_3F };

static const int DATA_CodeTbl_AC_WALK_U_1F[] =
{AC_WALK_U_1F,AC_WALK_D_1F,AC_WALK_L_1F,AC_WALK_R_1F };

static const int DATA_CodeTbl_AC_DASH_U_4F[] =
{AC_DASH_U_4F,AC_DASH_D_4F,AC_DASH_L_4F,AC_DASH_R_4F };

static const int DATA_CodeTbl_AC_JUMPHI_L_1G_16F[] =
{AC_JUMPHI_L_1G_16F,AC_JUMPHI_R_1G_16F,AC_JUMPHI_L_1G_16F,AC_JUMPHI_R_1G_16F};

static const int DATA_CodeTbl_AC_JUMPHI_L_3G_32F[] =
{AC_JUMPHI_L_3G_32F,AC_JUMPHI_R_3G_32F,AC_JUMPHI_L_3G_32F,AC_JUMPHI_R_3G_32F};

static const int DATA_CodeTbl_AC_WALK_U_7F[] =
{AC_WALK_U_7F,AC_WALK_D_7F,AC_WALK_L_7F,AC_WALK_R_7F };

static const int DATA_CodeTbl_AC_JUMP_U_3G_24F[] =
{AC_JUMP_U_3G_24F,AC_JUMP_D_3G_24F,AC_JUMP_L_3G_24F,AC_JUMP_R_3G_24F };

static const int DATA_CodeTbl_AC_WALKGL_U_8F[] =
{AC_WALKGL_U_8F,AC_WALKGL_D_8F,AC_WALKGL_L_8F,AC_WALKGL_R_8F};
static const int DATA_CodeTbl_AC_WALKGR_U_8F[] =
{AC_WALKGR_U_8F,AC_WALKGR_D_8F,AC_WALKGR_L_8F,AC_WALKGR_R_8F};
static const int DATA_CodeTbl_AC_WALKGU_U_8F[] =
{AC_WALKGU_U_8F,AC_WALKGU_D_8F,AC_WALKGU_L_8F,AC_WALKGU_R_8F};
static const int DATA_CodeTbl_AC_WALKGU_U_4F[] =
{AC_WALKGU_U_4F,AC_WALKGU_D_4F,AC_WALKGU_L_4F,AC_WALKGU_R_4F};
static const int DATA_CodeTbl_AC_WALKGU_U_2F[] =
{AC_WALKGU_U_2F,AC_WALKGU_D_2F,AC_WALKGU_L_2F,AC_WALKGU_R_2F};

static const int DATA_CodeTbl_AC_DASHGL_U_4F[] =
{AC_DASHGL_U_4F,AC_DASHGL_D_4F,AC_DASHGL_L_4F,AC_DASHGL_R_4F};
static const int DATA_CodeTbl_AC_DASHGR_U_4F[] =
{AC_DASHGR_U_4F,AC_DASHGR_D_4F,AC_DASHGR_L_4F,AC_DASHGR_R_4F};
static const int DATA_CodeTbl_AC_DASHGU_U_4F[] =
{AC_DASHGU_U_4F,AC_DASHGU_D_4F,AC_DASHGU_L_4F,AC_DASHGU_R_4F};

static const int DATA_CodeTbl_AC_JUMPGL_U_1G_8F[] =
{AC_JUMPGL_U_1G_8F,AC_JUMPGL_D_1G_8F,AC_JUMPGL_L_1G_8F,AC_JUMPGL_R_1G_8F};
static const int DATA_CodeTbl_AC_JUMPGR_U_1G_8F[] =
{AC_JUMPGR_U_1G_8F,AC_JUMPGR_D_1G_8F,AC_JUMPGR_L_1G_8F,AC_JUMPGR_R_1G_8F};
static const int DATA_CodeTbl_AC_JUMPGU_U_1G_8F[] =
{AC_JUMPGU_U_1G_8F,AC_JUMPGU_D_1G_8F,AC_JUMPGU_L_1G_8F,AC_JUMPGU_R_1G_8F};

static const int DATA_CodeTbl_AC_SANDWALK_U_16F[] =
{AC_SANDWALK_U_16F,AC_SANDWALK_D_16F,AC_SANDWALK_L_16F,AC_SANDWALK_R_16F};

//--------------------------------------------------------------
///  MMDL_ChangeDirAcmdCode()で対応するコード。
/// 並びは不順。
//--------------------------------------------------------------
const int * const DATA_AcmdCodeDirChangeTbl[] =
{
  DATA_CodeTbl_AC_DIR_U,
  DATA_CodeTbl_AC_WALK_U_32F,
  DATA_CodeTbl_AC_WALK_U_16F,
  DATA_CodeTbl_AC_WALK_U_8F,
  DATA_CodeTbl_AC_WALK_U_4F,
  DATA_CodeTbl_AC_WALK_U_2F,
  DATA_CodeTbl_AC_STAY_WALK_U_32F,
  DATA_CodeTbl_AC_STAY_WALK_U_16F,
  DATA_CodeTbl_AC_STAY_WALK_U_8F,
  DATA_CodeTbl_AC_STAY_WALK_U_4F,
  DATA_CodeTbl_AC_STAY_WALK_U_2F,
  DATA_CodeTbl_AC_STAY_JUMP_U_16F,
  DATA_CodeTbl_AC_STAY_JUMP_U_8F,
  DATA_CodeTbl_AC_JUMP_U_1G_8F,
  DATA_CodeTbl_AC_JUMP_U_2G_16F,
  DATA_CodeTbl_AC_WALK_U_6F,
  DATA_CodeTbl_AC_WALK_U_3F,
  DATA_CodeTbl_AC_WALK_U_1F,
  DATA_CodeTbl_AC_DASH_U_4F,
  DATA_CodeTbl_AC_JUMPHI_L_1G_16F,
  DATA_CodeTbl_AC_JUMPHI_L_3G_32F,
  DATA_CodeTbl_AC_WALK_U_7F,
  DATA_CodeTbl_AC_WALKGL_U_8F,
  DATA_CodeTbl_AC_WALKGR_U_8F,
  DATA_CodeTbl_AC_JUMP_U_3G_24F,
  DATA_CodeTbl_AC_WALKGU_U_8F,
  DATA_CodeTbl_AC_DASHGL_U_4F,
  DATA_CodeTbl_AC_DASHGR_U_4F,
  DATA_CodeTbl_AC_DASHGU_U_4F,
  DATA_CodeTbl_AC_JUMPGL_U_1G_8F,
  DATA_CodeTbl_AC_JUMPGR_U_1G_8F,
  DATA_CodeTbl_AC_JUMPGU_U_1G_8F,
  DATA_CodeTbl_AC_WALKGU_U_4F,
  DATA_CodeTbl_AC_WALKGU_U_2F,
  DATA_CodeTbl_AC_SANDWALK_U_16F,
  
  NULL,  //終端識別
};

