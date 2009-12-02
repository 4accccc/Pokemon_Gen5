//======================================================================
/**
 * @file  field_comm_menu.c
 * @brief フィールド通信　メニュー部分
 * @author  ariizumi
 * @data  08/11/11
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "system/bmp_menu.h"
#include "system/bmp_winframe.h"

#include "test/ariizumi/ari_debug.h"
#include "field_comm_main.h"
#include "field_comm_menu.h"
#include "field/field_comm/field_comm_sys.h"

#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_d_field.h"
#include "debug/debug_str_conv.h"

#define NULL_OLD_FLDMSG (0) //FLDMSGBG置き換え前

//======================================================================
//  define
//======================================================================
#define F_COMM_MENU_PALETTE (14)
#define F_COMM_FONT_PALETTE (15)

//======================================================================
//  enum
//======================================================================
//BGエリア
#define F_COMM_BG_AREA (GFL_BMP_CHRAREA_GET_B)

//はい・いいえウィンドウサイズ
#define F_COMM_YNWIN_TOP  (13)
#define F_COMM_YNWIN_LEFT (25)
#define F_COMM_YNWIN_WIDTH  ( 6)
#define F_COMM_YNWIN_HEIGHT ( 4)

//はい・いいえウィンドウサイズ
#define F_COMM_ACTLIST_TOP  (17-(FCAL_MAX*2))
#define F_COMM_ACTLIST_LEFT (21)
#define F_COMM_ACTLIST_WIDTH  ( 10)
#define F_COMM_ACTLIST_HEIGHT (FCAL_MAX*2)

//メッセージウィンドウサイズ
#define F_COMM_MSGWIN_TOP   (19)
#define F_COMM_MSGWIN_LEFT    ( 1)
#define F_COMM_MSGWIN_WIDTH   (30)
#define F_COMM_MSGWIN_HEIGHT  ( 4)

//======================================================================
//  typedef struct
//======================================================================
struct _FIELD_COMM_MENU
{
  HEAPID heapID_;
  FLDMSGBG *fldMsgBG_;

  GFL_MSGDATA *msgData_;

  //選択肢用(はい・いいえ と 会話後兼用
  FLDMENUFUNC *ynMenuFunc_;

  //メッセージウィンドウ用
  BOOL    isInitMsgWin_;
  FLDMSGWIN *msgWin_;

  //デバッグウィンドウ(Msg
#if DEB_ARI
  BOOL      isOpenDebugWin_;
#endif
};

//メニュー最大数
#define D_YESNO_MENULIST_MAX (2)

//======================================================================
//  proto
//======================================================================
FIELD_COMM_MENU * FIELD_COMM_MENU_InitCommMenu( HEAPID heapID, FLDMSGBG *fldMsgBG );
void  FIELD_COMM_MENU_TermCommMenu( FIELD_COMM_MENU *commMenu );
void  FIELD_COMM_MENU_InitBG_MsgPlane( FIELD_COMM_MENU *commMenu );
void  FIELD_COMM_MENU_TermBG_MsgPlane( FIELD_COMM_MENU *commMenu );
void  FIELD_COMM_MENU_InitBG_MenuPlane( FIELD_COMM_MENU *commMenu );
void  FIELD_COMM_MENU_TermBG_MenuPlane( FIELD_COMM_MENU *commMenu );

void  FIELD_COMM_MENU_OpenYesNoMenu( u8 bgPlane , FIELD_COMM_MENU *commMenu );
const u8 FIELD_COMM_MENU_UpdateYesNoMenu( FIELD_COMM_MENU *commMenu );
void  FIELD_COMM_MENU_CloseYesNoMenu( FIELD_COMM_MENU *commMenu );

void  FIELD_COMM_MENU_OpenActionList( u8 bgPlane , FIELD_COMM_MENU *commMenu );
const F_COMM_ACTION_LIST FIELD_COMM_MENU_UpdateActionList( FIELD_COMM_MENU *commMenu );
void  FIELD_COMM_MENU_CloseActionList( FIELD_COMM_MENU *commMenu );

void  FIELD_COMM_MENU_OpenMessageWindow( u8 bgPlane , FIELD_COMM_MENU *commMenu );
void  FIELD_COMM_MENU_CloseMessageWindow( FIELD_COMM_MENU *commMenu );
void  FIELD_COMM_MENU_SetMessage( u16 msgID , FIELD_COMM_MENU *commMenu );
void  FIELD_COMM_MENU_UpdateMessageWindow( FIELD_COMM_MENU *commMenu );

#if DEB_ARI
void  FIELD_COMM_MENU_SwitchDebugWindow( u8 bgPlane, FLDMSGBG *fldMsgBG );
void  FIELD_COMM_MENU_UpdateDebugWindow( FIELD_COMM_MAIN *commSys );
FIELD_COMM_MENU *debCommMenu = NULL;
#endif

//--------------------------------------------------------------
//  通信用メニュー作業領域　初期化
//--------------------------------------------------------------
FIELD_COMM_MENU* FIELD_COMM_MENU_InitCommMenu( HEAPID heapID, FLDMSGBG *fldMsgBG )
{
  FIELD_COMM_MENU *commMenu;
  ARCHANDLE *arcHandle;

  commMenu = GFL_HEAP_AllocClearMemory( heapID , sizeof(FIELD_COMM_MENU) );
  commMenu->heapID_ = heapID;
  commMenu->isInitMsgWin_ = FALSE;
  commMenu->fldMsgBG_ = fldMsgBG;

  commMenu->msgData_ = FLDMSGBG_CreateMSGDATA( fldMsgBG, NARC_message_d_field_dat );

#if DEB_ARI
  commMenu->isOpenDebugWin_ = FALSE;
#endif
  return commMenu;
}


//--------------------------------------------------------------
//  通信用メニュー作業領域　開放
//--------------------------------------------------------------
void  FIELD_COMM_MENU_TermCommMenu( FIELD_COMM_MENU *commMenu )
{
  GFL_MSG_Delete( commMenu->msgData_ );
  GFL_HEAP_FreeMemory( commMenu );
  commMenu = NULL;
}

//--------------------------------------------------------------
//  通信用メニューMSG面BG初期化
//--------------------------------------------------------------
void FIELD_COMM_MENU_InitBG_MsgPlane( FIELD_COMM_MENU *commMenu )
{
}

void  FIELD_COMM_MENU_TermBG_MsgPlane( FIELD_COMM_MENU *commMenu )
{
}

//--------------------------------------------------------------
//  通信用メニューMENU面BG初期化
//--------------------------------------------------------------
void  FIELD_COMM_MENU_InitBG_MenuPlane( FIELD_COMM_MENU *commMenu )
{
}

void  FIELD_COMM_MENU_TermBG_MenuPlane( FIELD_COMM_MENU *commMenu )
{
}

//--------------------------------------------------------------
//  はい・いいえ汎用　開く
//--------------------------------------------------------------
static const FLDMENUFUNC_HEADER YesNoMenuHeader =
{
  1,    //リスト項目数
  9,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  2,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

void FIELD_COMM_MENU_OpenYesNoMenu( u8 bgPlane , FIELD_COMM_MENU *commMenu )
{
  { //menu create
    u32 i;
    const u16 lmax = D_YESNO_MENULIST_MAX;
    FLDMENUFUNC_HEADER head = YesNoMenuHeader;
    FLDMENUFUNC_LISTDATA *ynMenuList;

    ynMenuList = FLDMENUFUNC_CreateListData( lmax, commMenu->heapID_ );

    FLDMENUFUNC_AddArcStringListData(
      ynMenuList, commMenu->msgData_,
      DEBUG_FIELD_C_CHOICE02, YNR_YES, commMenu->heapID_ );
    FLDMENUFUNC_AddArcStringListData(
      ynMenuList, commMenu->msgData_,
      DEBUG_FIELD_C_CHOICE03, YNR_NO, commMenu->heapID_ );

    FLDMENUFUNC_InputHeaderListSize( &head, lmax,
      F_COMM_YNWIN_LEFT, F_COMM_YNWIN_TOP,
      F_COMM_YNWIN_WIDTH, F_COMM_YNWIN_HEIGHT );

    commMenu->ynMenuFunc_ = FLDMENUFUNC_AddMenu( commMenu->fldMsgBG_, &head, ynMenuList );
  }
}

//--------------------------------------------------------------
//  はい・いいえ汎用閉じる
//--------------------------------------------------------------
void  FIELD_COMM_MENU_CloseYesNoMenu( FIELD_COMM_MENU *commMenu )
{
  FLDMENUFUNC_DeleteMenu( commMenu->ynMenuFunc_ );
}

//--------------------------------------------------------------
//  はい・いいえ汎用　更新
//--------------------------------------------------------------
const u8  FIELD_COMM_MENU_UpdateYesNoMenu( FIELD_COMM_MENU *commMenu )
{
  const u32 ret = FLDMENUFUNC_ProcMenu( commMenu->ynMenuFunc_ );

  switch( ret )
  {
  case FLDMENUFUNC_NULL:
    break;
  case FLDMENUFUNC_CANCEL:
    return YNR_NO;
    break;
  default:
    switch( ret )
    {
    case YNR_YES:
      return YNR_YES;
      break;
    case YNR_NO:
      return YNR_NO;
      break;
    default:
      GF_ASSERT("Invalid return value!\n");
      break;
    }
  }

  return YNR_WAIT;
}

//--------------------------------------------------------------
//  接続後行動選択　開く
//--------------------------------------------------------------
void  FIELD_COMM_MENU_OpenActionList( u8 bgPlane , FIELD_COMM_MENU *commMenu )
{
  { //menu create
    const u16 lmax = FCAL_MAX - 1;
    FLDMENUFUNC_HEADER head = YesNoMenuHeader;
    FLDMENUFUNC_LISTDATA *MenuList;
    static const FLDMENUFUNC_LIST listMsgIDArr[FCAL_MAX]=
    {
      { DEBUG_FIELD_C_CHOICE04, (void*)FCAL_TRAINERCARD },  //トレーナーカード
//      { DEBUG_FIELD_C_CHOICE05, (void*)FCAL_BATTLE },   //バトル
      { DEBUG_FIELD_C_CHOICE06, (void*)FCAL_END },      //終了
    };

    MenuList = FLDMENUFUNC_CreateMakeListData(
      listMsgIDArr, FCAL_MAX - 1, commMenu->msgData_, commMenu->heapID_ );

    FLDMENUFUNC_InputHeaderListSize( &head, lmax,
      F_COMM_ACTLIST_LEFT, F_COMM_ACTLIST_TOP + 2,
      F_COMM_ACTLIST_WIDTH, F_COMM_ACTLIST_HEIGHT - 2 );

    commMenu->ynMenuFunc_ = FLDMENUFUNC_AddMenu( commMenu->fldMsgBG_, &head, MenuList );
  }
}

//--------------------------------------------------------------
//  接続後行動選択 閉じる
//--------------------------------------------------------------
void  FIELD_COMM_MENU_CloseActionList( FIELD_COMM_MENU *commMenu )
{
  FLDMENUFUNC_DeleteMenu( commMenu->ynMenuFunc_ );
}

//--------------------------------------------------------------
//  接続後行動選択　更新
//--------------------------------------------------------------
const F_COMM_ACTION_LIST FIELD_COMM_MENU_UpdateActionList( FIELD_COMM_MENU *commMenu )
{
  const u32 ret = FLDMENUFUNC_ProcMenu( commMenu->ynMenuFunc_ );

  switch( ret )
  {
  case FLDMENUFUNC_NULL:
    break;
  case FLDMENUFUNC_CANCEL:
    return FCAL_END;
    break;
  default:
    {
      GF_ASSERT_MSG( ret<FCAL_MAX , "Invalid return value!\n");
      return ret;
    }
  }

  return FCAL_SELECT;
}

//--------------------------------------------------------------
//  メッセージウィンドウ　開く
//    初期メッセージの設定は各自で。
//--------------------------------------------------------------
void  FIELD_COMM_MENU_OpenMessageWindow( u8 bgPlane , FIELD_COMM_MENU *commMenu )
{
#if DEB_ARI
  if( debCommMenu != NULL )
  {
    FIELD_COMM_MENU_SwitchDebugWindow( bgPlane, commMenu->fldMsgBG_ );
  }
#endif //DEB_ARI

  { //window frame
    commMenu->msgWin_ = FLDMSGWIN_AddTalkWin( commMenu->fldMsgBG_, commMenu->msgData_ );
  }

  commMenu->isInitMsgWin_ = TRUE;
}

//--------------------------------------------------------------
//  メッセージウィンドウ　閉じる
//--------------------------------------------------------------
void  FIELD_COMM_MENU_CloseMessageWindow( FIELD_COMM_MENU *commMenu )
{
  FLDMSGBG_AllPrint( commMenu->fldMsgBG_ );
  FLDMSGWIN_Delete( commMenu->msgWin_ );
  commMenu->isInitMsgWin_ = FALSE;
}

//--------------------------------------------------------------
//  メッセージウィンドウ　メッセージ設定
//--------------------------------------------------------------
void  FIELD_COMM_MENU_SetMessage( u16 msgID , FIELD_COMM_MENU *commMenu )
{
  FLDMSGBG_AllPrint( commMenu->fldMsgBG_ );
  FLDMSGWIN_ClearWindow( commMenu->msgWin_ ); //前回文字列の消去
  FLDMSGWIN_Print( commMenu->msgWin_, 0, 0, msgID );
}

//--------------------------------------------------------------
//  メッセージウィンドウ　更新
//--------------------------------------------------------------
void  FIELD_COMM_MENU_UpdateMessageWindow( FIELD_COMM_MENU *commMenu )
{
  if( commMenu->isInitMsgWin_ == TRUE )
  {
  }
}

//--------------------------------------------------------------
//  デバッグ用メッセージウィンドウ
//--------------------------------------------------------------
#if DEB_ARI
#include "field_comm_main.h"
#include "field_comm_func.h"
#include "field_comm_data.h"
#include "gamesystem/playerwork.h"
#define F_COMM_DEBWIN_TOP   (F_COMM_MSGWIN_TOP)
#define F_COMM_DEBWIN_LEFT    (F_COMM_MSGWIN_LEFT)
#define F_COMM_DEBWIN_WIDTH   (F_COMM_MSGWIN_WIDTH)
#define F_COMM_DEBWIN_HEIGHT  (F_COMM_MSGWIN_HEIGHT)

void  FIELD_COMM_MENU_SwitchDebugWindow( u8 bgPlane, FLDMSGBG *fldMsgBG )
{
  FIELD_COMM_MENU *commMenu = debCommMenu;

  if( debCommMenu == NULL )
  {
    commMenu = FIELD_COMM_MENU_InitCommMenu( GFL_HEAPID_APP, fldMsgBG );
    commMenu->isOpenDebugWin_ = TRUE;
    FIELD_COMM_MENU_OpenMessageWindow( bgPlane, commMenu );
  }
  else
  {
    FLDMSGBG_AllPrint( fldMsgBG );
    commMenu->isOpenDebugWin_ = FALSE;
    FIELD_COMM_MENU_CloseMessageWindow( commMenu );
    FIELD_COMM_MENU_TermCommMenu( commMenu );
    commMenu = NULL;
  }
  debCommMenu = commMenu;
}

void  FIELD_COMM_MENU_UpdateDebugWindow( FIELD_COMM_MAIN *commSys )
{
  FIELD_COMM_MENU *commMenu = debCommMenu;
  COMM_FIELD_SYS_PTR commField = FIELD_COMM_MAIN_GetCommFieldSysPtr( commSys );
  FIELD_COMM_DATA* commData = FIELD_COMM_SYS_GetCommDataWork( commField );

  if( commMenu == NULL )
    return;

  if( commMenu->isOpenDebugWin_ == TRUE )
  {
    //文字列バッファ
    STRBUF *strTemp;
    u16 codeTemp[128];
    char  strBase[128];
    int codeTempLen;
    u8 i;

    //if( GFL_UI_KEY_GetTrg() ==PAD_BUTTON_Y )
    if( FLDMSGBG_CheckFinishPrint(commMenu->fldMsgBG_) )
    {
      //前回文字列の消去
      FLDMSGWIN_ClearWindow( commMenu->msgWin_ );

      for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
      {
        int px,pz;
        PLAYER_WORK *plWork = NULL;
        strTemp = GFL_STR_CreateBuffer( 128, commMenu->heapID_ );

        if( i == FIELD_COMM_FUNC_GetSelfIndex(NULL) )
        {
          plWork = FIELD_COMM_DATA_GetSelfData_PlayerWork(commData);
          FIELD_COMM_DATA_GetGridPos_AfterMove(commData, FCD_SELF_INDEX,&px,&pz);
        }
        if( FIELD_COMM_DATA_GetCharaData_IsExist(commData, i) == TRUE )
        {
          plWork = FIELD_COMM_DATA_GetCharaData_PlayerWork(commData, i);
          FIELD_COMM_DATA_GetGridPos_AfterMove(commData, i,&px,&pz);
        }
        if( plWork != NULL )
        {
          codeTempLen = 128;
          sprintf(strBase,"P%d:%3d(%2d):%3d(%2d):%d  \0",i+1,(int)F32_CONST(plWork->position.x),px,(int)F32_CONST(plWork->position.z),pz,(int)F32_CONST(plWork->position.y));
          //sprintf(strBase,"P%d:%3d:%3d  \0",i+1,plWork->position.x,plWork->position.z);
          //STD_ConvertStringSjisToUnicode( codeTemp ,&codeTempLen, strBase , NULL , NULL);
          GFL_STR_SetStringCodeOrderLength( strTemp , codeTemp , codeTempLen);
          FLDMSGWIN_PrintStrBuf( commMenu->msgWin_, (i%2)*128, (i/2)*16, (void*)strTemp );
        }
        GFL_STR_DeleteBuffer( strTemp );
      }
    }
  }
  debCommMenu = commMenu;
}

#endif  //DEB_ARI
