//======================================================================
/**
 * @file	mb_util_msg.c
 * @brief	マルチブート・メッセージ表示
 * @author	ariizumi
 * @data	09/11/16
 *
 * モジュール名：MB_MSG
 */
//======================================================================
#include <gflib.h>

#include "arc_def.h"
#include "app/app_keycursor.h"
#include "print/printsys.h"
#include "system/talkmsgwin.h"

#include "multiboot/mb_util_msg.h"
#include "multiboot/mb_local_def.h"

#ifndef MULTI_BOOT_MAKE  //通常時処理
  //talkmsgwin用にfieldを読む
  FS_EXTERN_OVERLAY(fieldmap);
#endif //MULTI_BOOT_MAKE

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _MB_MSG_WORK
{
  HEAPID heapId;
  u8     frame;
  u8     selFrame;
  MB_MSG_WIN_TYPE type;
  
  //メッセージ用
  GFL_TCBLSYS     *tcblSys;
  GFL_BMPWIN      *msgWin;
  GFL_FONT        *fontHandle;
  PRINT_STREAM    *printHandle;
  GFL_MSGDATA     *msgHandle;
  STRBUF          *msgStr;
  WORDSET         *wordSet;
  
  PRINT_QUE *printQue;
  BOOL      isUpdateQue;
  BOOL      isUseCursor;
  
  APP_TASKMENU_WORK *yesNoWork;
  APP_TASKMENU_RES  *takmenures;
  APP_KEYCURSOR_WORK *cursorWork;
  
  TALKMSGWIN_SYS    *talkWinSys;
};


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//  メッセージ系 初期化
//--------------------------------------------------------------
MB_MSG_WORK* MB_MSG_MessageInit( HEAPID heapId , const u8 frame ,const u8 selFrame , const u32 datId , const BOOL useTalkWin )
{
  
  MB_MSG_WORK* msgWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_MSG_WORK ) );
  msgWork->heapId = heapId;
  msgWork->frame = frame;
  msgWork->selFrame = selFrame;
  //メッセージ用処理
  msgWork->msgWin = NULL;
  
  //フォント読み込み
  msgWork->fontHandle = GFL_FONT_Create( ARCID_FONT_MB , FILE_FONT_LARGE_MB , GFL_FONT_LOADTYPE_FILE , FALSE , msgWork->heapId );
  
  //メッセージ
  msgWork->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE_MB , datId , msgWork->heapId );

  BmpWinFrame_GraphicSet( frame , MB_MSG_MSGWIN_CGX , MB_MSG_PLT_MAIN_MSGWIN , 0 , msgWork->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , FILE_FONT_PLT_MB , PALTYPE_MAIN_BG , MB_MSG_PLT_MAIN_FONT*0x20, 16*2, msgWork->heapId );
  GFL_FONTSYS_SetDefaultColor();
  
  msgWork->tcblSys = GFL_TCBL_Init( msgWork->heapId , msgWork->heapId , 3 , 0x100 );
  msgWork->printHandle = NULL;
  msgWork->msgStr = NULL;
  msgWork->wordSet = NULL;
  
  //YesNo用
  msgWork->printQue = PRINTSYS_QUE_Create( msgWork->heapId );
  msgWork->takmenures  = APP_TASKMENU_RES_Create( msgWork->selFrame, MB_MSG_PLT_MAIN_TASKMENU, msgWork->fontHandle, msgWork->printQue, msgWork->heapId );
  msgWork->yesNoWork = NULL;
  
  msgWork->cursorWork = APP_KEYCURSOR_Create( 0x0f , FALSE , TRUE , msgWork->heapId );
  
#ifndef MULTI_BOOT_MAKE  //通常時処理
  //talkmsgwin用にfieldを読む
  GFL_OVERLAY_Load( FS_OVERLAY_ID( fieldmap ) );
  OS_TPrintf("デバッグ起動だとASSERT出ますがSKIPしてください。\n");
  OS_TPrintf("↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓\n");
#endif //MULTI_BOOT_MAKE

  //会話Win
  if( useTalkWin == TRUE )
  {
    TALKMSGWIN_SYS_SETUP setup;
    setup.heapID = heapId;
    setup.g3Dcamera = NULL;
    setup.fontHandle = msgWork->fontHandle;
    setup.chrNumOffs = MB_MSG_TALKWIN_CGX;
    setup.ini.frameID = frame;
    setup.ini.winPltID = MB_MSG_PLT_MAIN_TALKWIN;
    setup.ini.fontPltID = MB_MSG_PLT_MAIN_FONT;
    msgWork->talkWinSys = TALKMSGWIN_SystemCreate( &setup );
  }
  else
  {
    msgWork->talkWinSys = NULL;
  }
  
  msgWork->isUpdateQue = FALSE;
  msgWork->isUseCursor = FALSE;
  return msgWork;
}


//--------------------------------------------------------------
//  メッセージ系 開放
//--------------------------------------------------------------
void MB_MSG_MessageTerm( MB_MSG_WORK *msgWork )
{
  if( msgWork->talkWinSys != NULL )
  {
    TALKMSGWIN_SystemDelete( msgWork->talkWinSys );
  }
  if( msgWork->yesNoWork != NULL )
  {
    APP_TASKMENU_CloseMenu( msgWork->yesNoWork );
  }
  APP_KEYCURSOR_Delete( msgWork->cursorWork );
  APP_TASKMENU_RES_Delete( msgWork->takmenures );
  PRINTSYS_QUE_Delete( msgWork->printQue );
  if( msgWork->printHandle != NULL )
  {
    PRINTSYS_PrintStreamDelete( msgWork->printHandle );
  }
  if( msgWork->msgStr != NULL )
  {
    GFL_STR_DeleteBuffer( msgWork->msgStr );
  }
  if( msgWork->wordSet != NULL )
  {
    MB_MSG_MessageDeleteWordset( msgWork );
  }
  GFL_MSG_Delete( msgWork->msgHandle );
  if( msgWork->msgWin != NULL )
  {
    GFL_BMPWIN_Delete( msgWork->msgWin );
  }
  GFL_FONT_Delete( msgWork->fontHandle );
  GFL_TCBL_Exit( msgWork->tcblSys );
  
  GFL_HEAP_FreeMemory( msgWork );

#ifndef MULTI_BOOT_MAKE  //通常時処理
  //talkmsgwin用にfieldを読む
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( fieldmap ) );
#endif //MULTI_BOOT_MAKE
}

//--------------------------------------------------------------
//  メッセージ系 更新
//--------------------------------------------------------------
void MB_MSG_MessageMain( MB_MSG_WORK *msgWork )
{
  GFL_TCBL_Main( msgWork->tcblSys );
  if( msgWork->printHandle != NULL  )
  {
    if( msgWork->isUseCursor == TRUE )
    {
      APP_KEYCURSOR_Main( msgWork->cursorWork , msgWork->printHandle , msgWork->msgWin );
    }
    
    if( PRINTSYS_PrintStreamGetState( msgWork->printHandle ) == PRINTSTREAM_STATE_DONE )
    {
      PRINTSYS_PrintStreamDelete( msgWork->printHandle );
      msgWork->printHandle = NULL;
    }
    else
    if( PRINTSYS_PrintStreamGetState( msgWork->printHandle ) == PRINTSTREAM_STATE_PAUSE )
    {
      if( GFL_UI_TP_GetTrg() == TRUE )
      {
        PRINTSYS_PrintStreamReleasePause( msgWork->printHandle );
      }
    }
    else
    {
      if( GFL_UI_TP_GetCont() == TRUE )
      {
        PRINTSYS_PrintStreamShortWait( msgWork->printHandle , 0 );
      }
    }
  }
  PRINTSYS_QUE_Main( msgWork->printQue );
  if( msgWork->isUpdateQue == TRUE &&
      PRINTSYS_QUE_IsFinished( msgWork->printQue ) == TRUE )
  {
    GFL_BMPWIN_TransVramCharacter( msgWork->msgWin );
    msgWork->isUpdateQue = FALSE;
  }

  if( msgWork->yesNoWork != NULL )
  {
    APP_TASKMENU_UpdateMenu( msgWork->yesNoWork );
  }

}

//--------------------------------------------------------------
//  ウィンドウの作成(各種
//--------------------------------------------------------------
void MB_MSG_MessageCreateWindow( MB_MSG_WORK *msgWork , MB_MSG_WIN_TYPE type )
{
  if( msgWork->type != type )
  {
    if( msgWork->msgWin != NULL )
    {
      GFL_BMPWIN_Delete( msgWork->msgWin );
      msgWork->msgWin = NULL;
    }
    //メッセージ用処理
    switch( type )
    {
    case MMWT_NORMAL:  //画面中央
      msgWork->msgWin = GFL_BMPWIN_Create( msgWork->frame , 
                                        MB_MSG_MSGWIN_LEFT , 
                                        MB_MSG_MSGWIN_TOP ,
                                        MB_MSG_MSGWIN_WIDTH , 
                                        MB_MSG_MSGWIN_HEIGHT , 
                                        MB_MSG_PLT_MAIN_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
      break;
    case MMWT_1LINE:   //画面下１行
      msgWork->msgWin = GFL_BMPWIN_Create( msgWork->frame , 
                                        MB_MSG_MSGWIN_SEL_LEFT , 
                                        MB_MSG_MSGWIN_SEL_TOP ,
                                        MB_MSG_MSGWIN_SEL_WIDTH , 
                                        MB_MSG_MSGWIN_SEL_HEIGHT , 
                                        MB_MSG_PLT_MAIN_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
      break;
    case MMWT_2LINE:   //画面下２行
    case MMWT_2LINE_TALK:   //画面下２行(会話WIN
      msgWork->msgWin = GFL_BMPWIN_Create( msgWork->frame , 
                                        MB_MSG_MSGWIN_SEL_L_LEFT , 
                                        MB_MSG_MSGWIN_SEL_L_TOP ,
                                        MB_MSG_MSGWIN_SEL_L_WIDTH , 
                                        MB_MSG_MSGWIN_SEL_L_HEIGHT , 
                                        MB_MSG_PLT_MAIN_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
      break;
    }
    msgWork->type = type;
  }
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( msgWork->msgWin ) , 0x0 );
  GFL_BMPWIN_TransVramCharacter( msgWork->msgWin );
  GFL_BMPWIN_MakeScreen( msgWork->msgWin );
  GFL_BG_LoadScreenReq(msgWork->frame);
}


//--------------------------------------------------------------------------
//  メッセージ表示
//  子機はMsgSpeedはセーブから取れないので、親からもらった物を変数で渡す
//--------------------------------------------------------------------------
void MB_MSG_MessageDisp( MB_MSG_WORK *msgWork , const u16 msgId , const int msgSpeed )
{
  if( msgWork->printHandle != NULL )
  {
    MB_TPrintf( "Message is not finish!!\n" );
    PRINTSYS_PrintStreamDelete( msgWork->printHandle );
    msgWork->printHandle = NULL;
  }
  
  //一応デフォは消しておく
  msgWork->isUseCursor = FALSE;
  
  {
    if( msgWork->msgStr != NULL )
    {
      GFL_STR_DeleteBuffer( msgWork->msgStr );
      msgWork->msgStr = NULL;
    }
    
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( msgWork->msgWin ) , 0xf );
    msgWork->msgStr = GFL_MSG_CreateString( msgWork->msgHandle , msgId );
    
    if( msgWork->wordSet != NULL )
    {
      STRBUF *msgWorkStr = GFL_STR_CreateBuffer( 128 , msgWork->heapId );
      WORDSET_ExpandStr( msgWork->wordSet , msgWorkStr , msgWork->msgStr );
      GFL_STR_DeleteBuffer( msgWork->msgStr );
      msgWork->msgStr = msgWorkStr;
    }
    
    msgWork->printHandle = PRINTSYS_PrintStream( msgWork->msgWin , 0,0, msgWork->msgStr ,msgWork->fontHandle ,
                        msgSpeed , msgWork->tcblSys , 2 , msgWork->heapId , 0xf );
  }
  if( msgWork->type == MMWT_2LINE_TALK )
  {
    TALKMSGWIN_WriteBmpWindow( msgWork->talkWinSys , msgWork->msgWin , TALKMSGWIN_TYPE_NORMAL );
  }
  else
  {
    BmpWinFrame_Write( msgWork->msgWin , WINDOW_TRANS_ON_V , MB_MSG_MSGWIN_CGX , MB_MSG_PLT_MAIN_MSGWIN );
  }
}


//--------------------------------------------------------------------------
//  メッセージ表示
//--------------------------------------------------------------------------
void MB_MSG_MessageDispNoWait( MB_MSG_WORK *msgWork , const u16 msgId )
{
  if( msgWork->printHandle != NULL )
  {
    MB_TPrintf( "Message is not finish!!\n" );
    PRINTSYS_PrintStreamDelete( msgWork->printHandle );
    msgWork->printHandle = NULL;
  }

  {
    if( msgWork->msgStr != NULL )
    {
      GFL_STR_DeleteBuffer( msgWork->msgStr );
      msgWork->msgStr = NULL;
    }
    
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( msgWork->msgWin ) , 0xf );
    msgWork->msgStr = GFL_MSG_CreateString( msgWork->msgHandle , msgId );
    
    if( msgWork->wordSet != NULL )
    {
      STRBUF *msgWorkStr = GFL_STR_CreateBuffer( 128 , msgWork->heapId );
      WORDSET_ExpandStr( msgWork->wordSet , msgWorkStr , msgWork->msgStr );
      GFL_STR_DeleteBuffer( msgWork->msgStr );
      msgWork->msgStr = msgWorkStr;
    }
    PRINTSYS_PrintQue( msgWork->printQue , GFL_BMPWIN_GetBmp( msgWork->msgWin ) , 
            0 , 0 , msgWork->msgStr , msgWork->fontHandle );
    msgWork->isUpdateQue = TRUE;

  }
  if( msgWork->type == MMWT_2LINE_TALK )
  {
    TALKMSGWIN_WriteBmpWindow( msgWork->talkWinSys , msgWork->msgWin , TALKMSGWIN_TYPE_NORMAL );
    GFL_BG_LoadScreenReq(msgWork->frame);
    }
  else
  {
    BmpWinFrame_Write( msgWork->msgWin , WINDOW_TRANS_ON_V , MB_MSG_MSGWIN_CGX , MB_MSG_PLT_MAIN_MSGWIN );
  }
}

//--------------------------------------------------------------------------
//  メッセージ消去
//--------------------------------------------------------------------------
void MB_MSG_MessageHide( MB_MSG_WORK *msgWork )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( msgWork->msgWin ) , 0 );
  if( msgWork->type == MMWT_2LINE_TALK )
  {
    TALKMSGWIN_ClearBmpWindow( msgWork->talkWinSys , msgWork->msgWin );
    GFL_BG_LoadScreenReq(msgWork->frame);
  }
  else
  {
    BmpWinFrame_Clear( msgWork->msgWin , WINDOW_TRANS_ON_V );
  }
}

//--------------------------------------------------------------------------
//  メッセージ WORDSET 作成
//--------------------------------------------------------------------------
void MB_MSG_MessageCreateWordset( MB_MSG_WORK *msgWork )
{
  if( msgWork->wordSet == NULL )
  {
    msgWork->wordSet = WORDSET_Create( msgWork->heapId );
  }
}

//--------------------------------------------------------------------------
//  メッセージ WORDSET 削除
//--------------------------------------------------------------------------
void MB_MSG_MessageDeleteWordset( MB_MSG_WORK *msgWork )
{
  if( msgWork->wordSet != NULL )
  {
    WORDSET_Delete( msgWork->wordSet );
    msgWork->wordSet = NULL;
  }
}

//--------------------------------------------------------------------------
//  メッセージ WORDSET 名前設定
//--------------------------------------------------------------------------
void MB_MSG_MessageWordsetName( MB_MSG_WORK *msgWork , const u32 bufId , MYSTATUS *myStatus)
{
  WORDSET_RegisterPlayerName( msgWork->wordSet , bufId , myStatus );
}

//--------------------------------------------------------------------------
//  選択肢表示
//--------------------------------------------------------------------------
void MB_MSG_DispYesNo( MB_MSG_WORK *msgWork )
{
  APP_TASKMENU_ITEMWORK itemWork[2];
  APP_TASKMENU_INITWORK initWork;
  
  itemWork[0].str = GFL_MSG_CreateString( msgWork->msgHandle , MSGID_YES_MB );
  itemWork[1].str = GFL_MSG_CreateString( msgWork->msgHandle , MSGID_NO_MB );
  itemWork[0].msgColor = MB_MSG_YESNO_COLOR;
  itemWork[1].msgColor = MB_MSG_YESNO_COLOR;
  itemWork[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  itemWork[1].type = APP_TASKMENU_WIN_TYPE_NORMAL;

  initWork.heapId = msgWork->heapId;
  initWork.itemNum = 2;
  initWork.itemWork = itemWork;
  initWork.posType = ATPT_LEFT_UP;
  initWork.charPosX = MB_MSG_YESNO_X;
  initWork.charPosY = MB_MSG_YESNO_Y;
  initWork.w = APP_TASKMENU_PLATE_WIDTH_YN_WIN;
  initWork.h = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;

  msgWork->yesNoWork = APP_TASKMENU_OpenMenu( &initWork, msgWork->takmenures );
  
  GFL_STR_DeleteBuffer( itemWork[0].str );
  GFL_STR_DeleteBuffer( itemWork[1].str );
}
void MB_MSG_DispYesNo_Select( MB_MSG_WORK *msgWork )
{
  APP_TASKMENU_ITEMWORK itemWork[2];
  APP_TASKMENU_INITWORK initWork;
  
  itemWork[0].str = GFL_MSG_CreateString( msgWork->msgHandle , MSG_MB_CHILD_SYS_01 );
  itemWork[1].str = GFL_MSG_CreateString( msgWork->msgHandle , MSG_MB_CHILD_SYS_02 );
  itemWork[0].msgColor = MB_MSG_YESNO_COLOR;
  itemWork[1].msgColor = MB_MSG_YESNO_COLOR;
  itemWork[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  itemWork[1].type = APP_TASKMENU_WIN_TYPE_NORMAL;

  initWork.heapId = msgWork->heapId;
  initWork.itemNum = 2;
  initWork.itemWork = itemWork;
  initWork.posType = ATPT_LEFT_UP;
  initWork.charPosX = MB_MSG_YESNO_SEL_X;
  initWork.charPosY = MB_MSG_YESNO_SEL_Y;
  initWork.w = APP_TASKMENU_PLATE_WIDTH_YN_WIN;
  initWork.h = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;

  msgWork->yesNoWork = APP_TASKMENU_OpenMenu( &initWork, msgWork->takmenures );
  
  GFL_STR_DeleteBuffer( itemWork[0].str );
  GFL_STR_DeleteBuffer( itemWork[1].str );
}
//--------------------------------------------------------------------------
//  YesNo消去
//--------------------------------------------------------------------------
void MB_MSG_ClearYesNo( MB_MSG_WORK *msgWork )
{
  APP_TASKMENU_CloseMenu( msgWork->yesNoWork );
  msgWork->yesNoWork = NULL;
}

//--------------------------------------------------------------------------
//  YesNo更新
//--------------------------------------------------------------------------
const MB_MSG_YESNO_RET MB_MSG_UpdateYesNo( MB_MSG_WORK *msgWork )
{
  if( APP_TASKMENU_IsFinish( msgWork->yesNoWork ) == TRUE )
  {
    return MMYR_RET1 + APP_TASKMENU_GetCursorPos( msgWork->yesNoWork );
  }
  
  return MMYR_NONE;
}

//--------------------------------------------------------------------------
//  MSGDATA取得
//--------------------------------------------------------------------------
GFL_MSGDATA* MB_MSG_GetMsgHandle( MB_MSG_WORK *msgWork )
{
  return msgWork->msgHandle;
}

//--------------------------------------------------------------------------
//  WORDSET取得
//--------------------------------------------------------------------------
WORDSET* MB_MSG_GetWordSet( MB_MSG_WORK *msgWork )
{
  return msgWork->wordSet;
}

//--------------------------------------------------------------------------
//  GFL_FONT取得
//--------------------------------------------------------------------------
GFL_FONT* MB_MSG_GetFont( MB_MSG_WORK *msgWork )
{
  return msgWork->fontHandle;
}
//--------------------------------------------------------------------------
//  PRINT_QUE取得
//--------------------------------------------------------------------------
PRINT_QUE* MB_MSG_GetPrintQue( MB_MSG_WORK *msgWork )
{
  return msgWork->printQue;
}
//--------------------------------------------------------------------------
//  終了チェック
//--------------------------------------------------------------------------
const BOOL MB_MSG_CheckPrintQueIsFinish( MB_MSG_WORK *msgWork )
{
  return PRINTSYS_QUE_IsFinished( msgWork->printQue );
}
const BOOL MB_MSG_CheckPrintStreamIsFinish( MB_MSG_WORK *msgWork )
{
  if( msgWork->printHandle == NULL )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------
//  メッセージ待ちカーソルの表示
//--------------------------------------------------------------------------
void MB_MSG_SetDispKeyCursor( MB_MSG_WORK *msgWork , const BOOL flg )
{
  msgWork->isUseCursor = flg;
}

