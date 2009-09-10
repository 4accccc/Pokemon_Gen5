//======================================================================
/**
 * @file  field_comm_debug.c
 * @brief フィールド通信デバッグ　子メニュー
 * @author  ariizumi
 * @data  08/11/21
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "field/field_msgbg.h"

#include "field_comm_debug.h"
#include "field_comm_event.h"
#include "field_comm_main.h"

#include "arc_def.h"
#include "message.naix"
#include "msg/msg_d_field.h"

#define NULL_OLD_FLDMSG (0) //FLDMSGBG置き換え前

//======================================================================
//  define
//======================================================================

//======================================================================
//  enum
//======================================================================

//======================================================================
//  typedef struct
//======================================================================
typedef const BOOL(*FIELD_COMM_DEBUG_CALLBACK)(FIELD_COMM_DEBUG_WORK* work);
typedef const BOOL(*FIELD_COMM_DEBUG_SUBPROC)(FIELD_COMM_DEBUG_WORK* work);

struct _FIELD_COMM_DEBUG_WORK
{
  u8  subSeq_;
  HEAPID heapID_;
  GAMESYS_WORK  *gameSys_;
  FIELD_MAIN_WORK *fieldWork_;
  GMEVENT *event_;
  FLDMENUFUNC *menuFunc_;

  FIELD_COMM_DEBUG_CALLBACK callback_;
  FIELD_COMM_DEBUG_SUBPROC subProc_;
};

//======================================================================
//  proto
//======================================================================
const int FIELD_COMM_DEBUG_GetWorkSize(void);
void  FIELD_COMM_DEBUG_InitWork( const HEAPID heapID , GAMESYS_WORK *gameSys , FIELD_MAIN_WORK *fieldWork , GMEVENT *event , FIELD_COMM_DEBUG_WORK *commDeb );
GMEVENT_RESULT FIELD_COMM_DEBUG_CommDebugMenu( GMEVENT *event , int *seq , void *work );

static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_StartComm( FIELD_COMM_DEBUG_WORK *work );
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_StartInvasion(FIELD_COMM_DEBUG_WORK *work  );
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_EndComm(FIELD_COMM_DEBUG_WORK *work  );
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_StartComm_Palace( FIELD_COMM_DEBUG_WORK *work );
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_StartInvasion_Palace( FIELD_COMM_DEBUG_WORK *work );
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_ChangePartTest(FIELD_COMM_DEBUG_WORK *work );
static  const BOOL  FIELD_COMM_DEBUG_SubProc_DisconnectWait(FIELD_COMM_DEBUG_WORK *work );
static  const BOOL  FIELD_COMM_DEBUG_SubProc_ChangePartTest(FIELD_COMM_DEBUG_WORK *work );
//--------------------------------------------------------------
//  ワークサイズ取得
//--------------------------------------------------------------
const int FIELD_COMM_DEBUG_GetWorkSize(void)
{
  return sizeof( FIELD_COMM_DEBUG_WORK );
}
//--------------------------------------------------------------
//  ワーク初期化
//--------------------------------------------------------------
void  FIELD_COMM_DEBUG_InitWork( const HEAPID heapID , GAMESYS_WORK *gameSys , FIELD_MAIN_WORK *fieldWork , GMEVENT *event , FIELD_COMM_DEBUG_WORK *commDeb )
{
  commDeb->heapID_ = heapID;
  commDeb->gameSys_ = gameSys;
  commDeb->fieldWork_ = fieldWork;
  commDeb->event_ = event;
}

//--------------------------------------------------------------
//  イベントメイン
//--------------------------------------------------------------
/// メニューヘッダー
static const FLDMENUFUNC_HEADER FieldCommDebugMenuHeader =
{
  1,    //リスト項目数
  12,   //表示最大項目数
  0,    //ラベル表示Ｘ座標
  11,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

GMEVENT_RESULT FIELD_COMM_DEBUG_CommDebugMenu( GMEVENT *event , int *seq , void *work )
{
  FIELD_COMM_DEBUG_WORK *commDeb = work;
  switch( *seq )
  {
  case 0:
    //メニューの生成
    {
      static const u8 itemNum = 5;
      static const FLDMENUFUNC_LIST itemMenuList[itemNum] =
      {
        {DEBUG_FIELD_C_CHOICE00,FIELD_COMM_DEBUG_MenuCallback_StartComm},
        {DEBUG_FIELD_C_CHOICE01,FIELD_COMM_DEBUG_MenuCallback_StartInvasion},
        {DEBUG_FIELD_C_CHOICE07,FIELD_COMM_DEBUG_MenuCallback_EndComm},
        {DEBUG_FIELD_C_CHOICE08,FIELD_COMM_DEBUG_MenuCallback_StartComm_Palace},
        {DEBUG_FIELD_C_CHOICE09,FIELD_COMM_DEBUG_MenuCallback_StartInvasion_Palace},
      };
      FLDMENUFUNC_HEADER head = FieldCommDebugMenuHeader;
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG( commDeb->fieldWork_ );
      GFL_MSGDATA *msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_d_field_dat );
      FLDMENUFUNC_LISTDATA *listdata;

      listdata = FLDMENUFUNC_CreateMakeListData(
        itemMenuList, itemNum, msgData, commDeb->heapID_ );
      FLDMENUFUNC_InputHeaderListSize( &head, itemNum, 1, 1, 11, 16 );
      commDeb->menuFunc_ = FLDMENUFUNC_AddMenu( msgBG, &head, listdata );
      commDeb->subProc_ = NULL;
      GFL_MSG_Delete( msgData );
    }
    *seq += 1;
    break;
  case 1:
    {
      const u32 ret = FLDMENUFUNC_ProcMenu( commDeb->menuFunc_ );
      switch( ret )
      {
      case FLDMENUFUNC_NULL:
        break;

      case FLDMENUFUNC_CANCEL:
        FLDMENUFUNC_DeleteMenu( commDeb->menuFunc_ );
        return GMEVENT_RES_FINISH;
        break;

      default:
        commDeb->callback_ = (FIELD_COMM_DEBUG_CALLBACK)ret;
        *seq += 1;
        break;
      }
    }
    break;
  case 2:
    FLDMENUFUNC_DeleteMenu( commDeb->menuFunc_ );
    *seq += 1;
    if( commDeb->callback_(commDeb) == TRUE )
      return GMEVENT_RES_CONTINUE;
    else
      return GMEVENT_RES_FINISH;
    break;
  case 3:
    //SubProcが設定されていればこっちにくる
    GF_ASSERT( commDeb->subProc_ != NULL );
    if( commDeb->subProc_(commDeb) == TRUE )
    {
      return GMEVENT_RES_FINISH;
    }
    break;
  }
  //return GMEVENT_RES_FINISH;
  return GMEVENT_RES_CONTINUE;
}

#if NULL_OLD_FLDMSG
GMEVENT_RESULT FIELD_COMM_DEBUG_CommDebugMenu( GMEVENT *event , int *seq , void *work )
{
  FIELD_COMM_DEBUG_WORK *commDeb = work;
  switch( *seq )
  {
  case 0:
    //メニューの生成
    {
      static const u8 itemNum = 3;
      static const u16 itemMsgList[itemNum] =
      {
        DEBUG_FIELD_C_CHOICE00 ,
        DEBUG_FIELD_C_CHOICE01 ,
        DEBUG_FIELD_C_CHOICE07 ,
      };
      static const void* itemCallProc[itemNum] =
      {
        FIELD_COMM_DEBUG_MenuCallback_StartComm,
        FIELD_COMM_DEBUG_MenuCallback_StartInvasion,
        FIELD_COMM_DEBUG_MenuCallback_EndComm,
      };
      u8 i;
      GFL_MSGDATA *msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL ,
          ARCID_MESSAGE , NARC_message_d_field_dat , commDeb->heapID_ );
      BMPMENULIST_HEADER menuHeader = FieldCommDebugMenuHeader;

      commDeb->debMenuWork_ = GFL_HEAP_AllocClearMemory( commDeb->heapID_ , DebugMenu_GetWorkSize() );
      commDeb->menuListData_ = BmpMenuWork_ListCreate( itemNum, commDeb->heapID_ );
      commDeb->subProc_ = NULL;

      for( i=0;i<itemNum;i++ )
      {
        BmpMenuWork_ListAddArchiveString(
            commDeb->menuListData_ , msgData ,
            itemMsgList[i] , (u32)itemCallProc[i] , commDeb->heapID_ );
      }


      DebugMenu_InitCommonMenu(
        commDeb->debMenuWork_,
        &menuHeader, commDeb->menuListData_, NULL, itemNum,
        11, 16, NARC_message_d_field_dat, commDeb->heapID_ );

      GFL_MSG_Delete( msgData );
    }
    *seq += 1;
    break;
  case 1:
    {
      const u32 ret = DebugMenu_ProcCommonMenu( commDeb->debMenuWork_ );
      switch( ret )
      {
      case BMPMENU_NULL:
        break;

      case BMPMENU_CANCEL:
        DebugMenu_DeleteCommonMenu( commDeb->debMenuWork_ );
        GFL_HEAP_FreeMemory( commDeb->debMenuWork_ );
        return GMEVENT_RES_FINISH;
        break;

      default:
        commDeb->callback_ = (FIELD_COMM_DEBUG_CALLBACK)ret;
        *seq += 1;
        break;
      }
    }
    break;
  case 2:
    BmpMenuWork_ListDelete( commDeb->menuListData_ );
    DebugMenu_DeleteCommonMenu( commDeb->debMenuWork_ );
    GFL_HEAP_FreeMemory( commDeb->debMenuWork_ );
    *seq += 1;
    if( commDeb->callback_(commDeb) == TRUE )
      return GMEVENT_RES_CONTINUE;
    else
      return GMEVENT_RES_FINISH;
    break;
  case 3:
    //SubProcが設定されていればこっちにくる
    GF_ASSERT( commDeb->subProc_ != NULL );
    if( commDeb->subProc_(commDeb) == TRUE )
    {
      return GMEVENT_RES_FINISH;
    }
    break;
  }
  //return GMEVENT_RES_FINISH;
  return GMEVENT_RES_CONTINUE;
}
#endif

//--------------------------------------------------------------
//  通信開始
//--------------------------------------------------------------
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_StartComm( FIELD_COMM_DEBUG_WORK *work )
{
  GMEVENT *event = work->event_;
  FIELD_COMM_MAIN *commSys = FIELDMAP_GetCommSys(work->fieldWork_);
  FIELD_MAIN_WORK *fieldWork = work->fieldWork_;
  FIELD_COMM_EVENT *eveWork;

  GMEVENT_Change( event , FIELD_COMM_EVENT_StartCommNormal , FIELD_COMM_EVENT_GetWorkSize() );

  eveWork = GMEVENT_GetEventWork(event);
  FIELD_COMM_EVENT_SetWorkData( commSys , fieldWork , eveWork );
  return TRUE;
}
//--------------------------------------------------------------
//  侵入開始
//--------------------------------------------------------------
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_StartInvasion( FIELD_COMM_DEBUG_WORK *work )
{
  GMEVENT *event = work->event_;
  FIELD_COMM_MAIN *commSys = FIELDMAP_GetCommSys(work->fieldWork_);
  FIELD_MAIN_WORK *fieldWork = work->fieldWork_;
  FIELD_COMM_EVENT *eveWork;

  GMEVENT_Change( event , FIELD_COMM_EVENT_StartCommInvasion , FIELD_COMM_EVENT_GetWorkSize() );

  eveWork = GMEVENT_GetEventWork(event);
  FIELD_COMM_EVENT_SetWorkData( commSys , fieldWork , eveWork );
  return TRUE;
}
//--------------------------------------------------------------
//  通信終了
//--------------------------------------------------------------
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_EndComm( FIELD_COMM_DEBUG_WORK *work )
{
  GMEVENT *event = work->event_;
  FIELD_COMM_MAIN *commSys = FIELDMAP_GetCommSys(work->fieldWork_);
  FIELD_MAIN_WORK *fieldWork = work->fieldWork_;
  FIELD_COMM_EVENT *eveWork;

  FIELD_COMM_MAIN_Disconnect( fieldWork , commSys );
  work->subProc_ = FIELD_COMM_DEBUG_SubProc_DisconnectWait;

  return TRUE;
}
//--------------------------------------------------------------
//  通信開始
//--------------------------------------------------------------
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_StartComm_Palace( FIELD_COMM_DEBUG_WORK *work )
{
  BOOL ret;
  FIELD_COMM_MAIN *commSys = FIELDMAP_GetCommSys(work->fieldWork_);
  
  ret = FIELD_COMM_DEBUG_MenuCallback_StartComm(work);
  FIELD_COMM_MAIN_SetCommType(commSys, FIELD_COMM_TYPE_PALACE);
  return ret;
}
//--------------------------------------------------------------
//  侵入開始
//--------------------------------------------------------------
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_StartInvasion_Palace( FIELD_COMM_DEBUG_WORK *work )
{
  BOOL ret;
  FIELD_COMM_MAIN *commSys = FIELDMAP_GetCommSys(work->fieldWork_);
  
  ret = FIELD_COMM_DEBUG_MenuCallback_StartInvasion(work);
  FIELD_COMM_MAIN_SetCommType(commSys, FIELD_COMM_TYPE_PALACE);
  return ret;
}

//--------------------------------------------------------------
//  パート切り替えテスト
//--------------------------------------------------------------
extern const GFL_PROC_DATA DebugAriizumiMainProcData;
//extern const GFL_PROC_DATA DebugSogabeMainProcData;
static  const BOOL  FIELD_COMM_DEBUG_MenuCallback_ChangePartTest(FIELD_COMM_DEBUG_WORK *work )
{
  work->subProc_ = FIELD_COMM_DEBUG_SubProc_ChangePartTest;
  work->subSeq_ = 0;
  return TRUE;

}

//--------------------------------------------------------------
/**
 * 通信切断の終了待ち
 *
 * @param   work
 *
 * @retval  const BOOL
 */
//--------------------------------------------------------------
static  const BOOL  FIELD_COMM_DEBUG_SubProc_DisconnectWait(FIELD_COMM_DEBUG_WORK *work )
{
  FIELD_COMM_MAIN *commSys = FIELDMAP_GetCommSys(work->fieldWork_);
  FIELD_MAIN_WORK *fieldWork = work->fieldWork_;

  if(FIELD_COMM_MAIN_DisconnectWait( fieldWork , commSys ) == TRUE){
    OS_TPrintf("FIELD_COMM_DEBUG_SubProc_DisconnectWait : 切断完了\n");
    return TRUE;
  }
  return FALSE;
}


//--------------------------------------------------------------
//  パート切り替えテスト
//  フィールドが終了するのを待ってパート変更を発行する
//  event_mapchange参照
//--------------------------------------------------------------
static  const BOOL  FIELD_COMM_DEBUG_SubProc_ChangePartTest(FIELD_COMM_DEBUG_WORK *work )
{
  switch( work->subSeq_ )
  {
  case 0:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB ,
                   0 ,16 ,ARI_FADE_SPD );
    work->subSeq_++;
    break;
  case 1:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      FIELDMAP_Close(work->fieldWork_);
      work->subSeq_++;
    }
    break;

  case 2:
    if( GAMESYSTEM_IsProcExists(work->gameSys_) == GFL_PROC_MAIN_NULL )
    {
      work->subSeq_++;
    }
    break;

  case 3:
    GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugAriizumiMainProcData, NULL);
    //GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugSogabeMainProcData, NULL);
    work->subSeq_++;
    //ここでフィールドのProcを抜ける
    break;
  case 4:
    //復帰後にここに来る
    GAMESYSTEM_CallFieldProc(work->gameSys_);
    work->subSeq_++;
    break;
  case 5:
    //フィールドマップを開始待ち
    if(GAMESYSTEM_IsProcExists(work->gameSys_) != GFL_PROC_MAIN_NULL )
    {
      //この時点ではまだフィールドの初期化は完全ではない
      if( FIELDMAP_IsReady( work->fieldWork_ ) == TRUE )
      {
        FIELDMAP_ForceUpdate( work->fieldWork_ );
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB ,
                     16 ,0 ,ARI_FADE_SPD );
        work->subSeq_++;
      }
    }
    break;
  case 6:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}

