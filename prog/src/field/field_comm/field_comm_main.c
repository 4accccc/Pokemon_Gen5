//======================================================================
/**
 * @file  field_comm_main.c
 * @brief フィールド通信の処理の外と中をつなぐ関数とか群
 * @author  ariizumi
 * @data  08/11/11
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "infowin/infowin.h"
#include "gamesystem/gamesystem.h"
#include "test/ariizumi/ari_debug.h"
#include "field_comm_main.h"
#include "field_comm_menu.h"
#include "field_comm_func.h"
#include "field_comm_data.h"
#include "field_comm_sys.h"
#include "test/performance.h"
#include "system/main.h"

#include "msg/msg_d_field.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  enum
//======================================================================

//======================================================================
//  typedef struct
//======================================================================
struct _FIELD_COMM_MAIN
{
  u8  menuSeq_;
  HEAPID  heapID_;
  u8  talkTrgChara_;
  FIELD_COMM_MENU *commMenu_;
#if 0 ////
  FIELD_COMM_FUNC *commFunc_;
#else
 COMM_FIELD_SYS_PTR commField_;
#endif
  u8  commActorIndex_[FIELD_COMM_MEMBER_MAX];
};

//上下左右に対応したグリッドでのオフセット
static const s8 FCM_dirOfsArr[4][2]={{0,-1},{0,1},{-1,0},{1,0}};

//======================================================================
//  proto
//======================================================================

FIELD_COMM_MAIN* FIELD_COMM_MAIN_InitSystem( HEAPID heapID , HEAPID commHeapID );
void FIELD_COMM_MAIN_TermSystem( FIELD_MAIN_WORK *fieldWork, FIELD_COMM_MAIN *commSys );
void  FIELD_COMM_MAIN_UpdateCommSystem( FIELD_MAIN_WORK *fieldWork ,
        GAMESYS_WORK *gameSys , FIELD_PLAYER *pcActor , FIELD_COMM_MAIN *commSys );
static void FIELD_COMM_MAIN_UpdateSelfData( FIELD_MAIN_WORK *fieldWork ,
        GAMESYS_WORK *gameSys , FIELD_PLAYER *pcActor , FIELD_COMM_MAIN *commSys );
static void FIELD_COMM_MAIN_UpdateCharaData( FIELD_MAIN_WORK *fieldWork ,
        GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys );

const BOOL  FIELD_COMM_MAIN_CanTalk( FIELD_COMM_MAIN *commSys );
static  const u8  FIELD_COMM_MAIN_CheckTalkTarget( FIELD_COMM_MAIN *commSys );
void  FIELD_COMM_MAIN_StartTalk( FIELD_COMM_MAIN *commSys );
const BOOL  FIELD_COMM_MAIN_CheckReserveTalk( FIELD_COMM_MAIN *commSys );
//接続開始用メニュー処理
//開始時
void  FIELD_COMM_MAIN_InitStartCommMenu( FIELD_COMM_MAIN *commSys, FLDMSGBG *fldMsgBG );
void  FIELD_COMM_MAIN_TermStartCommMenu( FIELD_COMM_MAIN *commSys );
//橋の時
void  FIELD_COMM_MAIN_InitStartInvasionMenu( FIELD_COMM_MAIN *commSys, FLDMSGBG *fldMsgBG );
void  FIELD_COMM_MAIN_TermStartInvasionMenu( FIELD_COMM_MAIN *commSys );
const BOOL  FIELD_COMM_MAIN_LoopStartInvasionMenu( GAMESYS_WORK *gsys, FIELD_COMM_MAIN *commSys );

void  FIELD_COMM_MAIN_Disconnect( FIELD_MAIN_WORK *fieldWork , FIELD_COMM_MAIN *commSys );
void FIELD_COMM_MAIN_CommFieldSysFree(FIELD_COMM_MAIN *commSys);


//--------------------------------------------------------------
//  フィールド通信システム初期化
//  @param  commHeapID 通信用に常駐するヒープID
//      通信が有効な間中開放されないHeapを指定してください
//--------------------------------------------------------------
FIELD_COMM_MAIN* FIELD_COMM_MAIN_InitSystem( HEAPID heapID , HEAPID commHeapID )
{
  FIELD_COMM_MAIN *commSys;
  commSys = GFL_HEAP_AllocClearMemory( heapID , sizeof(FIELD_COMM_MAIN) );
  commSys->heapID_ = heapID;
////  commSys->commField_->commFunc_ = FIELD_COMM_FUNC_InitSystem( commHeapID );

////  FIELD_COMM_DATA_InitSystem( commHeapID );
#if DEB_ARI
  DEBUG_PerformanceSetActive( FALSE );
#endif
  return commSys;
}

//--------------------------------------------------------------
// フィールド通信システム開放
//--------------------------------------------------------------
void FIELD_COMM_MAIN_TermSystem( FIELD_MAIN_WORK *fieldWork, FIELD_COMM_MAIN *commSys )
{
  if(commSys->commField_ != NULL){
    FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);
    if(FIELD_COMM_FUNC_IsFinishTermCommSystem(commFunc) == TRUE){
      //通信監視ワークが存在しているのに切断されているのは通信エラーが発生して強制切断された場合
      //その為、フィールドの破棄と同時に通信管理ワークの破棄も行う
      //フィールドの通信エラーの仕様によってこれをどうするかは後々決める　※check
      FIELD_COMM_MAIN_CommFieldSysFree(commSys);
      GAMESYSTEM_SetCommFieldWork(FIELDMAP_GetGameSysWork(fieldWork), NULL);
    }
  }

#if 0
  if( isTermAll == TRUE )
  {
////    FIELD_COMM_DATA_TermSystem();
////    FIELD_COMM_FUNC_TermSystem( commSys->commField_->commFunc_ );
  }
#endif

  GFL_HEAP_FreeMemory( commSys );
}

//==================================================================
/**
 * フィールド通信監視ワークポインタのセット
 *
 * @param   commSys       フィールド通信システムワークへのポインタ
 * @param   comm_field    フィールド通信監視ワークポインタ
 */
//==================================================================
void FIELD_COMM_MAIN_CommFieldSysPtrSet( FIELD_COMM_MAIN *commSys, COMM_FIELD_SYS_PTR comm_field)
{
  //commField_がNULL時にNULL代入はあり
  GF_ASSERT((commSys->commField_ == NULL && comm_field == NULL) || (commSys->commField_ != NULL && comm_field == NULL) || (commSys->commField_ == NULL && comm_field != NULL));

  commSys->commField_ = comm_field;
}

//==================================================================
/**
 * フィールド通信監視ワークを生成する(通信開始時に生成すればよい)
 *
 * @param   commSys
 * @param   commHeapID
 *
 * @retval  COMM_FIELD_SYS_PTR    生成したフィールド通信監視ワークへのポインタ
 */
//==================================================================
COMM_FIELD_SYS_PTR FIELD_COMM_MAIN_CommFieldSysAlloc(FIELD_COMM_MAIN *commSys, HEAPID commHeapID)
{
  GF_ASSERT(commSys->commField_ == NULL);

  commSys->commField_ = FIELD_COMM_SYS_Alloc(commHeapID);
  return commSys->commField_;
}

//==================================================================
/**
 * フィールド通信監視ワークを破棄(通信終了時に破棄)
 *
 * @param   commSys   フィールド通信システムワークへのポインタ
 */
//==================================================================
void FIELD_COMM_MAIN_CommFieldSysFree(FIELD_COMM_MAIN *commSys)
{
  GF_ASSERT(commSys->commField_ != NULL);

  FIELD_COMM_SYS_Free(commSys->commField_);
  commSys->commField_ = NULL;
}

//==================================================================
/**
 * 通信継続したまま、マップ切り替えをした場合の初期化処理
 *
 * @param   comm_field
 */
//==================================================================
void FIELD_COMM_MAIN_CommFieldMapInit(COMM_FIELD_SYS_PTR comm_field)
{
  FIELD_COMM_DATA *commData;

  if(comm_field == NULL){
    return;
  }
  commData = FIELD_COMM_SYS_GetCommDataWork(comm_field);
  FIELD_COMM_DATA_DataReset(commData);
}


//--------------------------------------------------------------
// フィールド通信システム更新
//  @param  FIELD_MAIN_WORK フィールドワーク
//  @param  GAMESYS_WORK  ゲームシステムワーク(PLAYER_WORK取得用
//  @param  PC_ACTCONT    プレイヤーアクター(プレイヤー数値取得用
//  @param  FIELD_COMM_MAIN 通信ワーク
//  自分のキャラの数値を取得して通信用に保存
//  他キャラの情報を取得し、通信から設定
//--------------------------------------------------------------
void  FIELD_COMM_MAIN_UpdateCommSystem( FIELD_MAIN_WORK *fieldWork ,
        GAMESYS_WORK *gameSys , FIELD_PLAYER *pcActor , FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_FUNC *commFunc;

  if(commSys->commField_ == NULL){
    return;
  }

  commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);
  if( FIELD_COMM_FUNC_IsFinishInitCommSystem( commFunc ) == TRUE )
  {
    u8 i;
    FIELD_COMM_FUNC_UpdateSystem( commFunc );
    if( FIELD_COMM_FUNC_GetMemberNum( commFunc ) > 1 )
    //if( FIELD_COMM_FUNC_GetCommMode( commFunc ) == FIELD_COMM_MODE_CONNECT )
    {
      FIELD_COMM_MAIN_UpdateSelfData( fieldWork , gameSys , pcActor , commSys );
      FIELD_COMM_MAIN_UpdateCharaData( fieldWork , gameSys , commSys );
    }
  }
#if DEB_ARI
  if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_L )
    FIELD_COMM_MENU_SwitchDebugWindow( FCM_BGPLANE_MSG_WINDOW, FIELDMAP_GetFldMsgBG(fieldWork) );
  FIELD_COMM_MENU_UpdateDebugWindow( commSys );
#endif  //DEB_ARI
}

//--------------------------------------------------------------
// 自分ののキャラの更新
//--------------------------------------------------------------
static void FIELD_COMM_MAIN_UpdateSelfData( FIELD_MAIN_WORK *fieldWork ,
        GAMESYS_WORK *gameSys , FIELD_PLAYER *pcActor , FIELD_COMM_MAIN *commSys )
{
  ZONEID zoneID;
  VecFx32 pos;
  u16 dir;
  PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gameSys );
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);
  FIELD_COMM_DATA *commData = FIELD_COMM_SYS_GetCommDataWork(commSys->commField_);

  //自キャラ座標を更新
  zoneID = PLAYERWORK_getZoneID( plWork );
  FIELD_PLAYER_GetPos( pcActor, &pos );
  dir = FIELD_PLAYER_GetDir( pcActor );
  //dir = FieldMainGrid_GetPlayerDir( fieldWork );
  FIELD_COMM_DATA_SetSelfData_Pos( commData, &zoneID , &pos , &dir );
  FIELD_COMM_FUNC_Send_SelfData( commFunc, commData );
}

//--------------------------------------------------------------
// 他のキャラの更新
//--------------------------------------------------------------
static void FIELD_COMM_MAIN_UpdateCharaData( FIELD_MAIN_WORK *fieldWork ,
        GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);
  FIELD_COMM_DATA *commData = FIELD_COMM_SYS_GetCommDataWork(commSys->commField_);
  u8 i;
  //届いたデータのチェック
  for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
  {
#if DEB_ARI&0
    if( FIELD_COMM_DATA_GetCharaData_IsValid(commData, i) == TRUE )
#else
    if( i != FIELD_COMM_FUNC_GetSelfIndex(commFunc) &&
      FIELD_COMM_DATA_GetCharaData_IsValid(commData, i) == TRUE )
#endif
    {
      //有効なデータが入っている
      switch( FIELD_COMM_DATA_GetCharaData_State( commData, i ) )
      {
      case FCCS_NONE:
      case FCCS_CONNECT:
        //詳細データが無いので、データをリクエスト
        FIELD_COMM_FUNC_Send_RequestData( i , FCRT_PROFILE , commFunc );
        FIELD_COMM_DATA_SetCharaData_State( commData, i , FCCS_REQ_DATA );
        break;
      case FCCS_REQ_DATA:
        //データ受信中なので待ち
        break;
      case FCCS_EXIST_DATA:
        FIELD_COMM_DATA_SetCharaData_State( commData, i , FCCS_FIELD );
        //break through
      case FCCS_FIELD:
        {
          GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
          PLAYER_WORK *setPlWork = GAMEDATA_GetPlayerWork( gameData , i+1 );  //0には自分が入っているから
          PLAYER_WORK *charaWork = FIELD_COMM_DATA_GetCharaData_PlayerWork(commData, i);
          GFL_STD_MemCopy( (void*)charaWork , (void*)setPlWork , sizeof(PLAYER_WORK) );
          if( FIELD_COMM_DATA_GetCharaData_IsExist(commData, i) == FALSE )
          {
            //未初期化なキャラなので、初期化する
            FieldMain_AddCommActor( fieldWork , setPlWork );
            FIELD_COMM_DATA_SetCharaData_IsExist(commData, i,TRUE);
          }
          FIELD_COMM_DATA_SetCharaData_IsValid(commData, i,FALSE);
        }
      }
    }
  }
}

//--------------------------------------------------------------
// 会話ができるか？(会話ターゲットが居るか？
// 内部で会話ターゲットを確定してしまい、そのまま保持しておく
// 現状netID順で近いほうを優先するので・・・
//--------------------------------------------------------------
const BOOL  FIELD_COMM_MAIN_CanTalk( FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_FUNC *commFunc;
  FIELD_COMM_DATA *commData;

  if(commSys->commField_ == NULL){
    return FALSE;
  }

  commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);
  commData = FIELD_COMM_SYS_GetCommDataWork(commSys->commField_);

  if( FIELD_COMM_FUNC_GetCommMode(commFunc) == FIELD_COMM_MODE_CONNECT)
  {
    if( FIELD_COMM_DATA_GetTalkState( commData, FCD_SELF_INDEX ) == FCTS_NONE )
    {
      commSys->talkTrgChara_ = FIELD_COMM_MAIN_CheckTalkTarget( commSys );
      if( commSys->talkTrgChara_ < FIELD_COMM_MEMBER_MAX )
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}
static  const u8 FIELD_COMM_MAIN_CheckTalkTarget( FIELD_COMM_MAIN *commSys )
{
  u8 i;
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);
  FIELD_COMM_DATA *commData = FIELD_COMM_SYS_GetCommDataWork(commSys->commField_);
  const PLAYER_WORK *plWork = FIELD_COMM_DATA_GetCharaData_PlayerWork( commData, FCD_SELF_INDEX );
  const u16 selfDir = plWork->direction;
  int selfX,selfZ;
  FIELD_COMM_DATA_GetGridPos_AfterMove( commData, FCD_SELF_INDEX,&selfX,&selfZ );
  selfX += FCM_dirOfsArr[selfDir][0];
  selfZ += FCM_dirOfsArr[selfDir][1];
  for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
  {
    if( i != FIELD_COMM_FUNC_GetSelfIndex(commFunc) )
    {
      int px,pz;
      FIELD_COMM_DATA_GetGridPos_AfterMove( commData, i,&px,&pz );
      if( px == selfX && pz == selfZ )
      {
        return i;
      }
    }
  }
  return 0xFF;
}

//--------------------------------------------------------------
// 会話開始処理
//--------------------------------------------------------------
const BOOL  FIELD_COMM_MAIN_CheckReserveTalk( FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_FUNC *commFunc;
  FIELD_COMM_DATA *commData;

  if(commSys->commField_ == NULL){
    return FALSE;
  }

  commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);
  commData = FIELD_COMM_SYS_GetCommDataWork(commSys->commField_);

  if( FIELD_COMM_DATA_GetTalkState( commData, FCD_SELF_INDEX ) == FCTS_RESERVE_TALK )
  {
    int selfX,selfZ;
    u8  postID,postX,postZ;
    const BOOL isMove = FIELD_COMM_DATA_GetGridPos_AfterMove( commData, FCD_SELF_INDEX ,&selfX,&selfZ);
    FIELD_COMM_FUNC_GetTalkParterData_ID( &postID , commFunc );
    FIELD_COMM_FUNC_GetTalkParterData_Pos( &postX , &postZ , commFunc );
    if( selfX == postX && selfZ == postZ )
    {
      if( isMove == FALSE )
      {
        return TRUE;
      }
      else
      {
        //停止するまで待つため
        return FALSE;
      }
    }
    else
    {
      //すでに通り過ぎてしまっているためキャンセル処理
      FIELD_COMM_DATA_SetTalkState( commData, FCD_SELF_INDEX , FCTS_NONE );
      FIELD_COMM_FUNC_Send_CommonFlg( commFunc, FCCF_TALK_UNPOSSIBLE , 0 , postID );
      return FALSE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
// 会話開始処理
//--------------------------------------------------------------
void  FIELD_COMM_MAIN_StartTalk( FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);
  FIELD_COMM_DATA *commData = FIELD_COMM_SYS_GetCommDataWork(commSys->commField_);
  const PLAYER_WORK *plWork = FIELD_COMM_DATA_GetCharaData_PlayerWork( commData, FCD_SELF_INDEX );
  const u16 selfDir = plWork->direction;
  int selfX,selfZ;
  u16 sendValue;
  FIELD_COMM_DATA_GetGridPos_AfterMove( commData, FCD_SELF_INDEX,&selfX,&selfZ );
  selfX += FCM_dirOfsArr[selfDir][0];
  selfZ += FCM_dirOfsArr[selfDir][1];
  sendValue = selfX + (selfZ<<8);
  FIELD_COMM_FUNC_InitCommData_StartTalk( commFunc );
  FIELD_COMM_FUNC_Send_CommonFlg( commFunc, FCCF_TALK_REQUEST , sendValue , commSys->talkTrgChara_ );
  FIELD_COMM_DATA_SetTalkState( commData, FCD_SELF_INDEX , FCTS_WAIT_TALK );
}
//--------------------------------------------------------------
// 会話(話しかけられる側)開始処理
//--------------------------------------------------------------
void  FIELD_COMM_MAIN_StartTalkPartner( FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);
  FIELD_COMM_DATA *commData = FIELD_COMM_SYS_GetCommDataWork(commSys->commField_);
  u8  postID;
  FIELD_COMM_FUNC_InitCommData_StartTalk( commFunc );
  FIELD_COMM_FUNC_GetTalkParterData_ID( &postID , commFunc );
  FIELD_COMM_FUNC_Send_CommonFlg( commFunc, FCCF_TALK_ACCEPT , 0 , postID );
  FIELD_COMM_DATA_SetTalkState( commData, FCD_SELF_INDEX , FCTS_REPLY_TALK );
}

//--------------------------------------------------------------
// 通信開始メニュー初期化
//--------------------------------------------------------------
void  FIELD_COMM_MAIN_InitStartCommMenu( FIELD_COMM_MAIN *commSys, FLDMSGBG *fldMsgBG )
{
  commSys->commMenu_ = FIELD_COMM_MENU_InitCommMenu( commSys->heapID_, fldMsgBG );
  FIELD_COMM_MENU_InitBG_MsgPlane( commSys->commMenu_ );
  FIELD_COMM_MENU_InitBG_MenuPlane( commSys->commMenu_ );
  FIELD_COMM_MENU_OpenMessageWindow( FCM_BGPLANE_MSG_WINDOW , commSys->commMenu_ );
  FIELD_COMM_MENU_OpenYesNoMenu( FCM_BGPLANE_YESNO_WINDOW , commSys->commMenu_ );
  FIELD_COMM_MENU_SetMessage( DEBUG_FIELD_C_STR00 , commSys->commMenu_ );
  commSys->menuSeq_ = 0;
}

//--------------------------------------------------------------
//  通信開始メニュー 開放
//--------------------------------------------------------------
void  FIELD_COMM_MAIN_TermStartCommMenu( FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_MENU_CloseMessageWindow( commSys->commMenu_ );
  FIELD_COMM_MENU_TermCommMenu( commSys->commMenu_ );
  FIELD_COMM_MENU_TermBG_MsgPlane( commSys->commMenu_ );
  FIELD_COMM_MENU_TermBG_MenuPlane( commSys->commMenu_ );
}

//--------------------------------------------------------------
// 通信開始メニュー更新
//--------------------------------------------------------------
const BOOL  FIELD_COMM_MAIN_LoopStartCommMenu( FIELD_COMM_MAIN *commSys, GAMESYS_WORK *gsys )
{
  FIELD_COMM_MENU_UpdateMessageWindow( commSys->commMenu_ );
  switch( commSys->menuSeq_ )
  {
  case 0:
    {
      const u8 ret = FIELD_COMM_MENU_UpdateYesNoMenu( commSys->commMenu_ );
      if( ret == YNR_YES ){
        FIELD_COMM_MENU_CloseYesNoMenu( commSys->commMenu_ );
        commSys->menuSeq_++;
      }
      else if( ret == YNR_NO ){
        FIELD_COMM_MENU_CloseYesNoMenu( commSys->commMenu_ );
        return (TRUE);
      }
    }
    break;
  case 1:
    if( INFOWIN_IsInitComm() == TRUE )
    {
      INFOWIN_ExitComm();
      commSys->menuSeq_++;
    }
    else
    {
      commSys->menuSeq_ = 3;
    }
    break;
  case 2:
    if( GFL_NET_IsExit() == TRUE )
    {
      commSys->menuSeq_++;
    }
    break;
  case 3:
    //未初期化のときだけ初期化する
////    if( FIELD_COMM_FUNC_IsFinishInitCommSystem( commSys->commField_->commFunc_ ) == FALSE ){
    if(commSys->commField_ == NULL){
      COMM_FIELD_SYS_PTR comm_field;

      comm_field = FIELD_COMM_MAIN_CommFieldSysAlloc(commSys, GFL_HEAP_LOWID(GFL_HEAPID_APP));
      GAMESYSTEM_SetCommFieldWork(gsys, comm_field);
      FIELD_COMM_FUNC_InitCommSystem( commSys->commField_ );
    }
    commSys->menuSeq_++;
    break;
  case 4:
    {
      FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);

      if( FIELD_COMM_FUNC_IsFinishInitCommSystem( commFunc ) == TRUE ){
        FIELD_COMM_FUNC_StartCommWait( commFunc );
        commSys->menuSeq_++;
        return (TRUE);
      }
    }
    break;
  }
  return (FALSE);
}

//--------------------------------------------------------------
// 侵入開始メニュー 初期化
//--------------------------------------------------------------
void  FIELD_COMM_MAIN_InitStartInvasionMenu( FIELD_COMM_MAIN *commSys, FLDMSGBG *fldMsgBG )
{
  commSys->commMenu_ = FIELD_COMM_MENU_InitCommMenu( commSys->heapID_, fldMsgBG );
  FIELD_COMM_MENU_InitBG_MsgPlane( commSys->commMenu_ );
  FIELD_COMM_MENU_InitBG_MenuPlane( commSys->commMenu_ );
  FIELD_COMM_MENU_OpenMessageWindow( FCM_BGPLANE_MSG_WINDOW , commSys->commMenu_ );
  FIELD_COMM_MENU_OpenYesNoMenu( FCM_BGPLANE_YESNO_WINDOW , commSys->commMenu_ );
  FIELD_COMM_MENU_SetMessage( DEBUG_FIELD_C_STR01 , commSys->commMenu_ );
  commSys->menuSeq_ = 0;
}

//--------------------------------------------------------------
// 侵入開始メニュー開放
//--------------------------------------------------------------
void  FIELD_COMM_MAIN_TermStartInvasionMenu( FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_MENU_CloseMessageWindow( commSys->commMenu_ );
  FIELD_COMM_MENU_TermCommMenu( commSys->commMenu_ );
  FIELD_COMM_MENU_TermBG_MsgPlane( commSys->commMenu_ );
  FIELD_COMM_MENU_TermBG_MenuPlane( commSys->commMenu_ );
}

//--------------------------------------------------------------
// 侵入開始メニュー更新
//--------------------------------------------------------------
const BOOL  FIELD_COMM_MAIN_LoopStartInvasionMenu( GAMESYS_WORK *gsys, FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_MENU_UpdateMessageWindow( commSys->commMenu_ );
  switch( commSys->menuSeq_ )
  {
  case 0:
    {
      const u8 ret = FIELD_COMM_MENU_UpdateYesNoMenu( commSys->commMenu_ );
      if( ret == YNR_YES ){
        FIELD_COMM_MENU_CloseYesNoMenu( commSys->commMenu_ );
        commSys->menuSeq_++;
      }
      else if( ret == YNR_NO ){
        FIELD_COMM_MENU_CloseYesNoMenu( commSys->commMenu_ );
        return (TRUE);
      }
    }
    break;
  case 1:
    if( INFOWIN_IsInitComm() == TRUE )
    {
      INFOWIN_ExitComm();
      commSys->menuSeq_++;
    }
    else
    {
      commSys->menuSeq_ = 3;
    }
    break;
  case 2:
    if( GFL_NET_IsExit() == TRUE )
    {
      commSys->menuSeq_++;
    }
    break;
  case 3:
    //未初期化のときだけ初期化する
////    if( FIELD_COMM_FUNC_IsFinishInitCommSystem( commSys->commField_->commFunc_ ) == FALSE ){
    if(commSys->commField_ == NULL){
      COMM_FIELD_SYS_PTR comm_field;

      comm_field = FIELD_COMM_MAIN_CommFieldSysAlloc(commSys, GFL_HEAP_LOWID(GFL_HEAPID_APP));
      GAMESYSTEM_SetCommFieldWork(gsys, comm_field);
      FIELD_COMM_FUNC_InitCommSystem( commSys->commField_ );
    }
    commSys->menuSeq_++;
    break;
  case 4:
    {
    FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);

      if( FIELD_COMM_FUNC_IsFinishInitCommSystem( commFunc ) == TRUE ){
        FIELD_COMM_FUNC_StartCommSearch( commFunc );
        commSys->menuSeq_++;
        return (TRUE);
      }
    }
    break;
  }
  return (FALSE);
}

//==================================================================
/**
 * 通信切断リクエスト
 *
 * @param   fieldWork
 * @param   commSys
 *
 * ※この関数呼出し後、必ずFIELD_COMM_MAIN_DisconnectWait関数で切断完了の確認待ちをする事
 */
//==================================================================
void  FIELD_COMM_MAIN_Disconnect( FIELD_MAIN_WORK *fieldWork , FIELD_COMM_MAIN *commSys )
{
  //FieldMain_CommActorFreeAll( fieldWork );
  FIELD_COMM_FUNC_TermCommSystem();
}

//==================================================================
/**
 * 通信切断終了待ち
 *
 * @param   fieldWork
 * @param   commSys
 *
 * @retval  BOOL    TRUE:切断完了。　FALSE:切断待ち
 */
//==================================================================
BOOL FIELD_COMM_MAIN_DisconnectWait( FIELD_MAIN_WORK *fieldWork, FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_FUNC *commFunc = FIELD_COMM_SYS_GetCommFuncWork(commSys->commField_);

  if(FIELD_COMM_FUNC_IsFinishTermCommSystem(commFunc) == TRUE){
    FIELD_COMM_MAIN_CommFieldSysFree(commSys);
    GAMESYSTEM_SetCommFieldWork(FIELDMAP_GetGameSysWork(fieldWork), NULL);
    return TRUE;
  }
  return FALSE;
}

//======================================================================
//  以下 field_comm_event 用。extern定義も該当ソースに書く
//======================================================================
const int FIELD_COMM_MAIN_GetWorkSize(void)
{
  return sizeof(FIELD_COMM_MAIN);
}

COMM_FIELD_SYS_PTR FIELD_COMM_MAIN_GetCommFieldSysPtr( FIELD_COMM_MAIN *commSys )
{
  return commSys->commField_;
}

FIELD_COMM_MENU** FIELD_COMM_MAIN_GetCommMenuWork( FIELD_COMM_MAIN *commSys )
{
  return &commSys->commMenu_;
}
const HEAPID FIELD_COMM_MAIN_GetHeapID( FIELD_COMM_MAIN *commSys )
{
  return commSys->heapID_;
}

