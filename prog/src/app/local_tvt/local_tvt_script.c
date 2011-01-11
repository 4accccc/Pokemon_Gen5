//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_script.h
 *	@brief  ローカルライブキャスタースクリプトモードでの動作（WBまでのローカルライブキャスターの処理）
 *	@author	Toru=Nagihashi
 *	@data		2011.01.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"
#include "arc_def.h"
#include "local_tvt_script.naix"
#include "message.naix"
#include "font/font.naix"

#include "system/ds_system.h"

#include "local_tvt_local_def.h"
#include "local_tvt_chara.h"
#include "local_tvt_script.h"
#include "local_tvt_graphic.h"

#include "test/ariizumi/ari_debug.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#pragma mark [> define
typedef enum
{
  LTS_FADEIN,
  LTS_FADEIN_WAIT,
  LTS_FADEOUT,
  LTS_FADEOUT_WAIT,

  LTS_MAIN,
}LOCAL_TVT_STATE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	スクリプトのデータ
//=====================================
typedef struct
{
  u8 chara[LOCAL_TVT_MEMBER_MAX];
  u8 back[LOCAL_TVT_MEMBER_MAX];
}LOCAL_TVT_SCRIPT_HEADER;

typedef struct
{
  u8  comNo;
  u8  charaNo;
  u16 option;
}LOCAL_TVT_SCRIPT_DATA;


//-------------------------------------
///	メインワーク
//=====================================
struct _LOCAL_TVT_SCRIPT
{
  HEAPID heapId;
  GFL_TCB *vBlankTcb;

  LOCAL_TVT_MEMBER member;

  LOCAL_TVT_CHARA_SYS *chara_sys;
  
  GAMEDATA  *gamedata;
  u16 scriptID;
  u8 state;
  ARCHANDLE *arcHandle;
  
  BOOL isFirstCommand;
  void *scriptRes;
  LOCAL_TVT_SCRIPT_HEADER *scriptHead;
  LOCAL_TVT_SCRIPT_DATA *scriptData;
  GFL_MSGDATA     *talkMsgHandle;
  u8  talkCharaNo;

  u8 scriptIdx;
  u16 waitCnt;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void LOCAL_TVT_VBlankFunc(GFL_TCB *tcb, void *wk );

static void LOCAL_TVT_LoadScript( LOCAL_TVT_SCRIPT *work );
static void LOCAL_TVT_InitGraphic( LOCAL_TVT_SCRIPT *work );
static void LOCAL_TVT_TermGraphic( LOCAL_TVT_SCRIPT *work );
static void LOCAL_TVT_ScriptMain( LOCAL_TVT_SCRIPT *work );

//=============================================================================
/**
 *					データ
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプト動作モードの初期化
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	*gamedata ゲームデータ
 *	@param	scriptID  スクリプトID
 *  @param  chara_sys キャラシステム
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_SCRIPT* LOCAL_TVT_SCRIPT_Init( HEAPID heapID, GAMEDATA *gamedata, u16 scriptID, LOCAL_TVT_CHARA_SYS *chara_sys )
{
  LOCAL_TVT_SCRIPT* work  = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_SCRIPT) );
  work->scriptID  = scriptID;
  work->gamedata  = gamedata;
  work->heapId    = heapID;
  work->chara_sys = chara_sys;

  LOCAL_TVT_InitGraphic( work );

  ARI_TPrintf("[%d]\n",DS_SYSTEM_IsRestrictUGC());
 
  work->arcHandle = GFL_ARC_OpenDataHandle( ARCID_LOCAL_TVT_SCRIPT, work->heapId );

  LOCAL_TVT_LoadScript( work );

  LOCAL_TVT_UTIL_LoadBg( gamedata, work->heapId );


  //スクリプトから使用メッセージデータ作成
  work->talkMsgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_tvt_event_01_dat + (scriptID-1) , work->heapId );

  //スクリプトからキャラ設定
  {
    int i;
    LOCAL_TVT_CHARA_PARAM chara_param[LOCAL_TVT_MEMBER_MAX];
    GFL_STD_MemClear( chara_param, sizeof(LOCAL_TVT_CHARA_PARAM)*LOCAL_TVT_MEMBER_MAX );
    for( i = 0; i < work->member; i++ )
    {
      chara_param[i].charaType  = work->scriptHead->chara[i];
      chara_param[i].bgType     = work->scriptHead->back[i];
    }
    LOCAL_TVT_CHARA_SYS_LoadChara( work->chara_sys, chara_param, work->member );
  }

  work->vBlankTcb = GFUser_VIntr_CreateTCB( LOCAL_TVT_VBlankFunc , work , 8 );
  
  work->state = LTS_FADEIN;

  work->isFirstCommand = TRUE;
  work->scriptIdx = 0;
  
  GFL_NET_ReloadIconTopOrBottom(FALSE , work->heapId );

  return work;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプト動作モードの破棄
 *
 *	@param	LOCAL_TVT_SCRIPT *work ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_SCRIPT_Term( LOCAL_TVT_SCRIPT *work )
{
  u8 i;

  GFL_TCB_DeleteTask( work->vBlankTcb );
  
  GFL_MSG_Delete( work->talkMsgHandle );
  GFL_ARC_CloseDataHandle( work->arcHandle );
  
  LOCAL_TVT_TermGraphic( work );
  
  GFL_HEAP_FreeMemory( work->scriptRes );

  GFL_HEAP_FreeMemory( work );
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプト動作モードの破棄
 *
 *	@param	LOCAL_TVT_SCRIPT *work ワーク
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_SCRIPT_Main( LOCAL_TVT_SCRIPT *work )
{
  u8 i;
  
  switch( work->state )
  {
  case LTS_FADEIN:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = LTS_FADEIN_WAIT;
    break;
  case LTS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      work->state = LTS_MAIN;
    }
    break;
  case LTS_FADEOUT:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = LTS_FADEOUT_WAIT;
    break;
  case LTS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return TRUE;
    }
    break;

  case LTS_MAIN:
    LOCAL_TVT_ScriptMain( work );
    break;
  }

#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START &&
      GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    return TRUE;
  }
#endif

  return FALSE;
}


//--------------------------------------------------------------
//	VBlankTcb
//--------------------------------------------------------------
static void LOCAL_TVT_VBlankFunc(GFL_TCB *tcb, void *wk )
{
  //上画面ノイズ
  GFL_BG_SetScroll( LTVT_FRAME_NOIZE_BG , GFL_BG_SCROLL_X_SET , GFUser_GetPublicRand0(256) );
  GFL_BG_SetScroll( LTVT_FRAME_NOIZE_BG , GFL_BG_SCROLL_Y_SET , GFUser_GetPublicRand0(256) );
}
//--------------------------------------------------------------
//  グラフィック系初期化
//--------------------------------------------------------------
static void LOCAL_TVT_InitGraphic( LOCAL_TVT_SCRIPT *work )
{
  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0 , 
                    GX_BLEND_PLANEMASK_OBJ|
                    GX_BLEND_PLANEMASK_BG3|
                    GX_BLEND_PLANEMASK_BG2|
                    GX_BLEND_PLANEMASK_BG1 , 
                    16 , 4 );
}

static void LOCAL_TVT_TermGraphic( LOCAL_TVT_SCRIPT *work )
{

}

//--------------------------------------------------------------
//  スクリプト読み込み
//--------------------------------------------------------------
static void LOCAL_TVT_LoadScript( LOCAL_TVT_SCRIPT *work )
{
  ARI_TPrintf("Load local tvt script[%d]\n",work->scriptID);
  
  work->scriptRes = GFL_ARCHDL_UTIL_Load( work->arcHandle , 
                                          NARC_local_tvt_script_tvt_script_01_bin + (work->scriptID-1) ,
                                          FALSE , work->heapId );
  work->scriptHead = work->scriptRes;
  work->scriptData = (void*)((u32)work->scriptRes + sizeof(LOCAL_TVT_SCRIPT_HEADER) );

  /*
  {
    u8 i;
    for( i=0;i<4;i++ )
    {
      ARI_TPrintf("[%d][%d:%d]\n",i,work->scriptHead->chara[i],work->scriptHead->back[i]);
    }
  }
  */
  if( work->scriptHead->chara[2] == LTCT_NONE &&
      work->scriptHead->chara[3] == LTCT_NONE )
  {
    work->member = LTM_2_MEMBER;
  }
  else
  {
    work->member = LTM_4_MEMBER;
  }
}

//=============================================================================
/**
 *    スクリプト
 */
//=============================================================================
typedef enum
{
  LCL_FINISH,
  LCL_DISP_MESSAGE,
  LCL_WAIT,

  LCL_MAX,
}LTVT_COMMAND_LIST;

typedef void (*LTVT_SCRIPT_INIT_FUNC)( LOCAL_TVT_SCRIPT *work );
typedef const BOOL (*LTVT_SCRIPT_MAIN_FUNC)( LOCAL_TVT_SCRIPT *work );
typedef struct
{
  LTVT_SCRIPT_INIT_FUNC initFunc;
  LTVT_SCRIPT_MAIN_FUNC mainFunc;
}LTVT_SCRIPT_COMMAND_TABLE;

#pragma mark [>script func

static void LTVT_SCRIPT_DispMessageInit( LOCAL_TVT_SCRIPT *work );
static const BOOL LTVT_SCRIPT_DispMessageMain( LOCAL_TVT_SCRIPT *work );
static void LTVT_SCRIPT_WaitInit( LOCAL_TVT_SCRIPT *work );
static const BOOL LTVT_SCRIPT_WaitMain( LOCAL_TVT_SCRIPT *work );



static const LTVT_SCRIPT_COMMAND_TABLE commandTable[LCL_MAX] = 
{
  { NULL , NULL },  //終了コマンド
  { LTVT_SCRIPT_DispMessageInit , LTVT_SCRIPT_DispMessageMain },
  { LTVT_SCRIPT_WaitInit , LTVT_SCRIPT_WaitMain },
};

//--------------------------------------------------------------
//  スクリプトメイン
//--------------------------------------------------------------
static void LOCAL_TVT_ScriptMain( LOCAL_TVT_SCRIPT *work )
{
  const u8 comNo = work->scriptData[work->scriptIdx].comNo;
  if( work->isFirstCommand == TRUE )
  {
    //最初のコマンドの初期化を呼ぶ
    work->isFirstCommand = FALSE;
    commandTable[ comNo ].initFunc(work);
  }
  {
    const BOOL ret = commandTable[comNo].mainFunc(work);
    if( ret == TRUE )
    {
      u8 nextComNo;
      work->scriptIdx++;
      nextComNo = work->scriptData[work->scriptIdx].comNo;
      if( nextComNo != LCL_FINISH )
      {
        commandTable[ nextComNo ].initFunc(work);
      }
      else
      {
        work->state = LTS_FADEOUT;
      }
    }
  }
  
}


//--------------------------------------------------------------
//  コマンド：メッセージ表示
//--------------------------------------------------------------
static void LTVT_SCRIPT_DispMessageInit( LOCAL_TVT_SCRIPT *work )
{
  const u8 comNo = work->scriptData[work->scriptIdx].comNo;
  const u8 charaNo = work->scriptData[work->scriptIdx].charaNo;
  const u8 option = work->scriptData[work->scriptIdx].option;

  LOCAL_TVT_CHARA *chara  = LOCAL_TVT_CHARA_SYS_GetWork( work->chara_sys, charaNo );

  ARI_TPrintf("CommandInit DispMessage[%d][%d]\n",charaNo,option);
  

  work->talkCharaNo = charaNo;
  
  LOCAL_TVT_CHARA_StartTalk( chara, work->talkMsgHandle, option );
}

static const BOOL LTVT_SCRIPT_DispMessageMain( LOCAL_TVT_SCRIPT *work )
{
  LOCAL_TVT_CHARA *chara  = LOCAL_TVT_CHARA_SYS_GetWork( work->chara_sys, work->talkCharaNo );
  return LOCAL_TVT_CHARA_WaitTalk( chara, work->chara_sys);
}

//--------------------------------------------------------------
//  コマンド：ウェイト
//--------------------------------------------------------------
static void LTVT_SCRIPT_WaitInit( LOCAL_TVT_SCRIPT *work )
{
  const u8 comNo = work->scriptData[work->scriptIdx].comNo;
  const u8 charaNo = work->scriptData[work->scriptIdx].charaNo;
  const u8 option = work->scriptData[work->scriptIdx].option;
  ARI_TPrintf("CommandInit Wait[%d][%d]\n",charaNo,option);
  
  work->waitCnt = 0;
}

static const BOOL LTVT_SCRIPT_WaitMain( LOCAL_TVT_SCRIPT *work )
{
  const u8 option = work->scriptData[work->scriptIdx].option;

  work->waitCnt++;
  if( work->waitCnt > option )
  {
    return TRUE;
  }
  return FALSE;
}
