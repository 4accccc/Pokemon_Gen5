//======================================================================
/**
 * @file	scrcmd_menuwin.c
 * @brief	スクリプトコマンド：メニュー、システムウィンドウ関連
 * @date  2009.09.13
 * @author	tamada GAMEFREAK inc.
 *
 * 2009.09.13 scrcmd.cより分離させた
 */
//======================================================================

#include <wchar.h>
#include <gflib.h>

#include "system/vm_cmd.h"
#include "system/time_icon.h"

#include "pm_version.h"

#include "scrcmd.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd_work.h"
#include "scrcmd_menuwin.h"

#include "fieldmap.h"

#include "arc/script_message.naix"    //NARC_script_message_common_scr_dat
#include "msg/script/msg_common_scr.h"  // msg_yen_01

#include "font/font.naix"

#include "sound/pm_sndsys.h"

//======================================================================
//  define
//======================================================================
#define DEBUG_BWINTYPE TALKMSGWIN_TYPE_NORMAL

// 所持金表示ウィンドウ
#define GOLD_WIN_WIDTH  (9) // 幅(キャラクタ単位)
#define GOLD_WIN_HEIGHT (2) // 高さ(キャラクタ単位)
#define GOLD_WIN_KETA   (7) // 所持金数値の桁数

//======================================================================
//  proto
//======================================================================
static void sysWin_Close(SCRCMD_WORK *work);

static void balloonWin_Close(VMHANDLE *core, SCRCMD_WORK *work);

static void DeleteBalloonWin(SCRCMD_WORK *work);

static void setBalloonWindow( SCRCMD_WORK *work,
  FLDMSGBG *fmb, FLDTALKMSGWIN_IDX idx,
  const VecFx32 *pos, STRBUF *strBuf,
  TALKMSGWIN_TYPE type, TAIL_SETPAT tail );

static STRBUF * SetExpandWord(
    SCRCMD_WORK *work, SCRIPT_WORK *sc, u32 msg_id );

//======================================================================
//  はい、いいえ　処理
//======================================================================
//--------------------------------------------------------------
/// はい・いいえウィンドウ制御イベント用ワーク定義
//--------------------------------------------------------------
typedef struct {
  SCRCMD_WORK * scrcmd_work;
  u16 * ret_wk;
  FLDMENUFUNC * mw;
}YESNO_EV_WORK;

//--------------------------------------------------------------
/// はい・いいえウィンドウ制御イベント
//--------------------------------------------------------------
static GMEVENT_RESULT YesNoSelectEvent( GMEVENT * event, int * seq, void * work )
{
  YESNO_EV_WORK * yew = work;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( yew->scrcmd_work );

  switch (*seq)
  {
  case 0:
    SCRCMD_WORK_SetWaitCount( yew->scrcmd_work, SELECT_WAIT_VALUE );
    (*seq) ++;
    /* FALL THROUGH */

  case 1:
    //一定時間ウェイト後、選択可能にする
    if ( SCRCMD_WORK_WaitCountDown( yew->scrcmd_work ) == TRUE )
    {
      GAMESYS_WORK * gsys = SCRCMD_WORK_GetGameSysWork( yew->scrcmd_work );
      FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
      FLDMSGBG * msgBG = FIELDMAP_GetFldMsgBG( fieldmap );
      FLDMENUFUNC *mw = FLDMENUFUNC_AddYesNoMenu( msgBG, 0 );
      yew->mw = mw;

      (*seq) ++;
    }
    break;

  case 2:
    {
      FLDMENUFUNC_YESNO ret;
      ret = FLDMENUFUNC_ProcYesNoMenu( yew->mw );
      switch ( ret )
      {
      case FLDMENUFUNC_YESNO_NULL:
        break;
      case FLDMENUFUNC_YESNO_YES:
        *yew->ret_wk = SCR_YES;
        (*seq) ++;
        break;
      default:
        *yew->ret_wk = SCR_NO;
        (*seq) ++;
        break;
      }
    }
    break;

  case 3:
    FLDMENUFUNC_DeleteMenu( yew->mw );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/// はい・いいえウィンドウ制御イベントの生成
//--------------------------------------------------------------
static GMEVENT * createYesNoEvent( GAMESYS_WORK * gsys, SCRCMD_WORK * work, u16 * ret_wk )
{
  GMEVENT * event;
  YESNO_EV_WORK * yew;
  event = GMEVENT_Create( gsys, NULL, YesNoSelectEvent, sizeof(YESNO_EV_WORK) );
  yew = GMEVENT_GetEventWork( event );
  yew->scrcmd_work = work;
  yew->ret_wk = ret_wk;
  return event;
}

//--------------------------------------------------------------
/**
 * 「はい・いいえ」処理
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdYesNoWin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );

  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK * gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT * event;
  event = createYesNoEvent( gsys, work, ret_wk );
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//  メニュー
//======================================================================

//--------------------------------------------------------------
///  BMPMENU制御イベント用ワーク
//--------------------------------------------------------------
typedef struct {
  SCRCMD_WORK * scrcmd_work;  ///< スクリプト個別ワークへのポインタ
  BOOL breakable_flag;        ///< Ｘボタン有効かどうか
}BMPMENU_EVENT_WORK;

//--------------------------------------------------------------
///  BMPMENU制御イベント本体
//--------------------------------------------------------------
static GMEVENT_RESULT bmpMenuEvent( GMEVENT * event, int *seq, void * work )
{
  BMPMENU_EVENT_WORK * bew = work;
  BOOL result;

  switch ( *seq )
  {
  case 0:
    SCRCMD_WORK_SetWaitCount( bew->scrcmd_work, SELECT_WAIT_VALUE );
    (*seq) ++;
    /* FALL THROUGH */

  case 1:
    if ( SCRCMD_WORK_WaitCountDown( bew->scrcmd_work ) == TRUE )
    {
      SCRCMD_WORK_StartMenu( bew->scrcmd_work );
      (*seq) ++;
    }
    break;

  case 2:
    if ( bew->breakable_flag ) {
      result = SCRCMD_WORK_ProcMenu_Breakable( bew->scrcmd_work );
    } else {
      result = SCRCMD_WORK_ProcMenu( bew->scrcmd_work );
    }
    if ( result == TRUE ){
      return GMEVENT_RES_FINISH;
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
///  BMPMENU制御イベント生成
//--------------------------------------------------------------
static GMEVENT * createBmpMenuEvent( GAMESYS_WORK * gsys, SCRCMD_WORK * work, BOOL breakable_flag )
{
  GMEVENT * event;
  BMPMENU_EVENT_WORK * bew;
  event = GMEVENT_Create( gsys, NULL, bmpMenuEvent, sizeof(BMPMENU_EVENT_WORK) );
  bew = GMEVENT_GetEventWork( event );
  bew->scrcmd_work = work;
  bew->breakable_flag = breakable_flag;
  return event;
}

//--------------------------------------------------------------
/**
 * BMPメニュー初期化
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBmpMenuInit( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  u8 x = VMGetU8(core);
  u8 y = VMGetU8(core);
  u16 cursor = SCRCMD_GetVMWorkValue( core, work );
  u8 cancel = VMGetU8(core);
  u16 wk_id = VMGetU16( core );
  u16 *ret = SCRIPT_GetEventWork( sc, gdata, wk_id );
  WORDSET *wordset = SCRIPT_GetWordSet( sc );
  
  SCRCMD_WORK_InitMenuWork( work,
      x, y, cursor, cancel, SCR_MENU_JUST_L,
      ret, wordset, NULL );
  
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BMPメニュー初期化(読み込んでいるgmmファイルを使用する) 
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBmpMenuInitEx( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  u8 x = VMGetU8(core);
  u8 y = VMGetU8(core);
  u16 cursor = SCRCMD_GetVMWorkValue( core, work );
  u8 cancel = VMGetU8(core);
  u16 wk_id = VMGetU16( core );
  u16 *ret = SCRIPT_GetEventWork( sc, gdata, wk_id );
  WORDSET *wordset = SCRIPT_GetWordSet( sc );
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
  
  GF_ASSERT( wordset != NULL );
  GF_ASSERT( msgData != NULL );

  SCRCMD_WORK_InitMenuWork( work,
      x, y, cursor, cancel, SCR_MENU_JUST_L,
      ret, wordset, msgData );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BMPメニュー初期化(読み込んでいるgmmファイルを使用する) 右詰
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBmpMenuInitExRight( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  u8 x = VMGetU8(core);
  u8 y = VMGetU8(core);
  u16 cursor = SCRCMD_GetVMWorkValue( core, work );
  u8 cancel = VMGetU8(core);
  u16 wk_id = VMGetU16( core );
  u16 *ret = SCRIPT_GetEventWork( sc, gdata, wk_id );
  WORDSET *wordset = SCRIPT_GetWordSet( sc );
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
  
  GF_ASSERT( wordset != NULL );
  GF_ASSERT( msgData != NULL );

  SCRCMD_WORK_InitMenuWork( work,
      x, y, cursor, cancel, SCR_MENU_JUST_R,
      ret, wordset, msgData );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BMPメニュー	リスト作成
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBmpMenuMakeList( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 msg_id	= VMGetU16( core );
	u16 ex_msg_id	= VMGetU16( core );
	u16 param	= SCRCMD_GetVMWorkValue( core, work );
  STRBUF *msgbuf  = SCRIPT_GetMsgBuffer( sc );
  STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );
  SCRCMD_WORK_AddMenuList( work, msg_id, ex_msg_id, param, msgbuf, tmpbuf );
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * BMPメニュー	開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBmpMenuStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK * sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK * gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT * event = createBmpMenuEvent( gsys, work, FALSE );
  SCRIPT_CallEvent( sc, event );

	return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BMPメニュー	開始(xボタンによる中断あり)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBmpMenuStart_Breakable( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK * sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK * gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT * event = createBmpMenuEvent( gsys, work, FALSE );
  SCRIPT_CallEvent( sc, event );

	return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//  フィールド　システムウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * システムウィンドウ表示追加
 * @param work SCRCMD_WORK
 * @param u8 updown SCRCMD_MSGWIN_UP等
 * @retval nothing
 */
//--------------------------------------------------------------
static void sysWin_AddWindow( SCRCMD_WORK *work, u8 up_down )
{
  FLDSYSWIN_STREAM *sysWin;
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_WIN_OPEN) ) //メニューオープン済み
  {
    sysWin = SCRCMD_WORK_GetMsgWinPtr( work );
    FLDSYSWIN_STREAM_ClearMessage( sysWin );
  }
  else
  {
    u16 y = 1; //up
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
    GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
    
    if( up_down == SCRCMD_MSGWIN_UP ){
      y = 1;
    }else if( up_down == SCRCMD_MSGWIN_DOWN ){
      y = 19;
    }else if( up_down == SCRCMD_MSGWIN_DEFAULT ){ //前回の位置を継続
      up_down = SCRCMD_WORK_GetBeforeWindowPosType( work );
       
      switch( up_down ){
      case SCRCMD_MSGWIN_UPLEFT:
      case SCRCMD_MSGWIN_UPRIGHT: 
      case SCRCMD_MSGWIN_UP:
        up_down = SCRCMD_MSGWIN_UP;
        break;
      case SCRCMD_MSGWIN_DOWNLEFT:
      case SCRCMD_MSGWIN_DOWNRIGHT:
      case SCRCMD_MSGWIN_DOWN:
        y = 19;
        up_down = SCRCMD_MSGWIN_DOWN;
        break;
      case SCRCMD_MSGWIN_MAX: //初回　前回記録無し
        y = 19;
        up_down = SCRCMD_MSGWIN_DOWN;
        break;
      default:
        GF_ASSERT( 0 && "SYSWIN:ERROR NONE BEFORE" );
        y = 19;
        up_down = SCRCMD_MSGWIN_DOWN;
      }
    }else{ //エラー
      GF_ASSERT( 0 && "SYSWIN:ERROR SET POS" );
      y = 19;
      up_down = SCRCMD_MSGWIN_DOWN;
    }
    
	  sysWin = FLDSYSWIN_STREAM_Add( fparam->msgBG, msgData, y );
    SCRCMD_WORK_SetBeforeWindowPosType( work, up_down );
    
    SCRCMD_WORK_SetMsgWinPtr( work, sysWin );
    SCREND_CHK_SetBitOn(SCREND_CHK_WIN_OPEN);
  }
}

//--------------------------------------------------------------
/**
 * システムウィンドウ終了
 * @param   work    スクリプトコマンドワークポインタ
 * @retval  none
 */
//--------------------------------------------------------------
static void sysWin_Close( SCRCMD_WORK *work )
{
  FLDSYSWIN_STREAM *sysWin;
  TIMEICON_WORK* timeIcon;
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_WIN_OPEN) )
  {
    sysWin = (FLDSYSWIN_STREAM*)SCRCMD_WORK_GetMsgWinPtr( work );
    FLDSYSWIN_STREAM_Delete( sysWin );

    // タイムウエイトがあったら破棄
    {
      timeIcon = SCRCMD_WORK_GetTimeIconPtr( work );
      if( timeIcon ){
        TIMEICON_Exit( timeIcon );
        SCRCMD_WORK_SetTimeIconPtr( work, NULL );
      }
    }
    //NULLセット
    SCRCMD_WORK_SetMsgWinPtr( work, NULL );
    SCREND_CHK_SetBitOff(SCREND_CHK_WIN_OPEN);
  }
  else
  {
    #ifdef SCR_ASSERT_ON
    GF_ASSERT_MSG( 0, "SYSWIN:ERROR とじるウィンドウがありません" );
    #else
    OS_Printf( "SYSWIN:ERROR とじるウィンドウがありません\n" );
    #endif
  }
}


//--------------------------------------------------------------
/**
 * システムウィンドウメッセージ表示　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL sysWinMsgWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  FLDSYSWIN_STREAM *sysWin;
  sysWin = (FLDSYSWIN_STREAM*)SCRCMD_WORK_GetMsgWinPtr( work );
  
  if( FLDSYSWIN_STREAM_Print(sysWin) == TRUE ){
    return( 1 );
  }
  
  return( 0 );
}

//--------------------------------------------------------------
/**
 * 登録された単語を使って文字列展開　システムウィンドウメッセージ表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSysWinMsg( VMHANDLE *core, void *wk )
{
  STRBUF *msgbuf;
  FLDSYSWIN_STREAM *sysWin;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u16 up_down = VMGetU16( core );

  sysWin_AddWindow( work, up_down );
  msgbuf = SetExpandWord( work, sc, msg_id );
  sysWin = (FLDSYSWIN_STREAM*)SCRCMD_WORK_GetMsgWinPtr( work );
  FLDSYSWIN_STREAM_PrintStrBufStart( sysWin, 0, 0, msgbuf );
  
  VMCMD_SetWait( core, sysWinMsgWait );
  return VMCMD_RESULT_SUSPEND;
}

#if 0
static VMCMD_RESULT EvCmdTalkMsg( VMHANDLE *core, void *wk )
{
  u16 dir;
  VecFx32 pos;
  const VecFx32 *pos_p;
  WORDSET *wordset;
  STRBUF *msgbuf;
  STRBUF *tmpbuf;
  MMDL *fmmdl;
  FLDTALKMSGWIN *tmsg;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u8 msg_id = VMGetU8(core);
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  
  fmmdl = MMDLSYS_SearchOBJID( SCRCMD_WORK_GetMMdlSys(work), 0xff );
  MMDL_GetVectorPos( fmmdl, &pos );
  dir = MMDL_GetDirDisp( fmmdl );
  MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
  
  if( dir == DIR_UP ){        //下から
    pos.x += FX32_ONE*8;
    pos.z -= FX32_ONE*8;
  }else if( dir == DIR_DOWN ){ //上から
    pos.x += FX32_ONE*8;
    pos.z -= FX32_ONE*8;
  }else if( dir == DIR_LEFT ){ //右から
    pos.x += -FX32_ONE*8;
    pos.z += FX32_ONE*16;
  }else{                       //左から
    pos.x += FX32_ONE*8;
    pos.z += FX32_ONE*16;
  }
  
  SCRCMD_WORK_SetTalkMsgWinTailPos( work, &pos );
  pos_p = SCRCMD_WORK_GetTalkMsgWinTailPos( work );
  
  {
    wordset = SCRIPT_GetWordSet( sc );
    msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
    tmpbuf = SCRIPT_GetMsgTempBuffer( sc );
    KAGAYA_Printf( "EvCmdTalkMsg MSG ID = %d\n", msg_id );
    GFL_MSG_GetString( msgData, msg_id, tmpbuf );
    WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
  }
  
  {
    FLDTALKMSGWIN_IDX idx = FLDTALKMSGWIN_IDX_LOWER;
    
    if( dir == DIR_UP ){
      idx = FLDTALKMSGWIN_IDX_UPPER;
    }
    
    tmsg = FLDTALKMSGWIN_AddStrBuf( fparam->msgBG, idx, pos_p, msgbuf );
  }
  
  SCRCMD_WORK_setBalloonWindow( work, (FLDSYSWIN_STREAM*)tmsg );
  PMSND_PlaySystemSE( SEQ_SE_MESSAGE );

  VMCMD_SetWait( core, TalkMsgWait );
  return VMCMD_RESULT_SUSPEND;
}
#endif

//--------------------------------------------------------------
/**
 * 登録された単語を使って文字列展開　システムウィンドウメッセージ全表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSysWinMsgAllPut( VMHANDLE *core, void *wk )
{
  FLDSYSWIN_STREAM *sysWin;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  STRBUF *msgbuf = SCRIPT_GetMsgBuffer( sc );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u16 up_down = VMGetU16( core );
  
  sysWin_AddWindow( work, up_down );
  
  {
    GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );
    GFL_MSG_GetString( msgData, msg_id, tmpbuf );
    WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
  }
  
  sysWin = (FLDSYSWIN_STREAM*)SCRCMD_WORK_GetMsgWinPtr( work );
  FLDSYSWIN_STREAM_ClearMessage( sysWin );
  FLDSYSWIN_STREAM_AllPrintStrBuf( sysWin, 0, 0, msgbuf );
  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  システムウィンドウにタイムアイコンを設定する。
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdSysWinTimeIcon( VMHANDLE *core, void *wk )
{
  FLDSYSWIN_STREAM *sysWin;
  SCRCMD_WORK *work = wk;
  TIMEICON_WORK* timeIcon;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID heapID = FIELDMAP_GetHeapID( fieldmap );
  GFL_BMPWIN* bmpwin;
  u16 flag = SCRCMD_GetVMWorkValue( core, work );


  if( SCREND_CHK_CheckBit(SCREND_CHK_WIN_OPEN) == FALSE ) //システムウィンドウがない。
  {
    // タイムアイコンの設定先がない
    GF_ASSERT(0); 
    return VMCMD_RESULT_CONTINUE;
  }

  // OFF処理
  timeIcon = (TIMEICON_WORK*)SCRCMD_WORK_GetTimeIconPtr( work );
  if( timeIcon ){
    // 破棄
    TIMEICON_Exit( timeIcon );
    SCRCMD_WORK_SetTimeIconPtr( work, NULL );
  }

  // ON処理
  if( flag == SCR_SYSWIN_TIMEICON_ON )
  {
    // 設定
    sysWin = (FLDSYSWIN_STREAM*)SCRCMD_WORK_GetMsgWinPtr( work );
    bmpwin = FLDSYSWIN_STREAM_GetBmpWin( sysWin );

    timeIcon = TIMEICON_Create( GFUser_VIntr_GetTCBSYS(), bmpwin, 15, TIMEICON_DEFAULT_WAIT, heapID );

    SCRCMD_WORK_SetTimeIconPtr( work, timeIcon );
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * システムウィンドウ表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
#if 0
VMCMD_RESULT EvCmdTalkWinOpen( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 up_down = VMGetU16( core );
  
  sysWin_AddWindow( work, up_down ); //<<内部で既開チェックをしています
  return VMCMD_RESULT_CONTINUE;
}
#endif

//--------------------------------------------------------------
/**
 * システムウィンドウをとじる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSysWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  sysWin_Close(work);   //<<内部で既閉チェックをしています
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * スクリプト終了時システムウィンドウ終了チェック
 * @param   end_chk     チェック構造体
 * @param   seq     サブシーケンサ
 * @retval  BOOL    TRUEでチェック終了
 */
//--------------------------------------------------------------
BOOL SCREND_CheckEndWin(SCREND_CHECK *end_check , int *seq)
{
  sysWin_Close(end_check->ScrCmdWk);
  return  TRUE;
}

//======================================================================
//  所持金ウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * 所持金ウィンドウを表示する
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//-------------------------------------------------------------- 
VMCMD_RESULT EvCmdGoldWinOpen( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = wk;
  u16                   x = SCRCMD_GetVMWorkValue( core, work );  // スクリプト第1引数
  u16                   y = SCRCMD_GetVMWorkValue( core, work );  // スクリプト第2引数
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  PLAYER_WORK*     player = GAMEDATA_GetMyPlayerWork( gdata ); 
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGBG*        msg_bg = FIELDMAP_GetFldMsgBG( fieldmap );
  FLDMSGWIN*      msg_win = FIELDMAP_GetGoldMsgWin( fieldmap );
  SCRIPT_WORK*         sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*        wordset = SCRIPT_GetWordSet( sc );
  STRBUF*         tempbuf = SCRIPT_GetMsgTempBuffer( sc );
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_MSGDATA*   msg_data = GFL_MSG_Create( 
      GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_common_scr_dat, heap_id );
  STRBUF*          strbuf = GFL_STR_CreateBuffer( 128, heap_id );
  u32                gold = MISC_GetGold( GAMEDATA_GetMiscWork(gdata) );

  // 表示中のウィンドウを削除
  if( msg_win != NULL )
  {
    FLDMSGWIN_Delete( msg_win );
  }
  
  { //所持金ウィンドウは基本右詰に変更
    x -= GOLD_WIN_WIDTH;
  }

  // ウィンドウを表示
  msg_win = FLDMSGWIN_Add( msg_bg, msg_data, x, y, GOLD_WIN_WIDTH, GOLD_WIN_HEIGHT );
  FIELDMAP_SetGoldMsgWin( fieldmap, msg_win );

  // 文字列を作成
  WORDSET_RegisterNumber( 
      wordset, 2, gold, GOLD_WIN_KETA, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  GFL_MSG_GetString( msg_data, msg_yen_01, tempbuf );
  WORDSET_ExpandStr( wordset, strbuf, tempbuf );
  FLDMSGWIN_PrintStrBuf( msg_win, 0, 0, strbuf );
  GFL_STR_DeleteBuffer( strbuf );
  GFL_MSG_Delete( msg_data );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 所持金ウィンドウを更新する
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//-------------------------------------------------------------- 
VMCMD_RESULT EvCmdGoldWinUpdate( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work = wk;
  GAMESYS_WORK*  gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gdata = GAMESYSTEM_GetGameData( gsys );
  PLAYER_WORK*   player = GAMEDATA_GetMyPlayerWork( gdata ); 
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGWIN*     msg_win = FIELDMAP_GetGoldMsgWin( fieldmap );
  SCRIPT_WORK*   script = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*       wordset = SCRIPT_GetWordSet( script );
  STRBUF*        tempbuf = SCRIPT_GetMsgTempBuffer( script );
  HEAPID         heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_MSGDATA*   msg_data = GFL_MSG_Create( 
      GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_common_scr_dat, heap_id );
  STRBUF*        strbuf = GFL_STR_CreateBuffer( 128, heap_id );
  u32            gold = MISC_GetGold( GAMEDATA_GetMiscWork(gdata) );

  GF_ASSERT( msg_win );
  
  // 文字列を作成
  WORDSET_RegisterNumber( 
      wordset, 2, gold, GOLD_WIN_KETA, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  GFL_MSG_GetString( msg_data, msg_yen_01, tempbuf );
  WORDSET_ExpandStr( wordset, strbuf, tempbuf );

  // 表示を更新
  FLDMSGWIN_ClearWindow( msg_win );
  FLDMSGWIN_PrintStrBuf( msg_win, 0, 0, strbuf );

  GFL_STR_DeleteBuffer( strbuf );
  GFL_MSG_Delete( msg_data );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 所持金ウィンドウを消去する
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//-------------------------------------------------------------- 
VMCMD_RESULT EvCmdGoldWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGWIN*      msg_win = FIELDMAP_GetGoldMsgWin( fieldmap );

  if( msg_win )
  {
    FLDMSGWIN_Delete( msg_win );
    FIELDMAP_SetGoldMsgWin( fieldmap, NULL );
  }
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  吹き出しウィンドウ
//======================================================================
//--------------------------------------------------------------
//  吹き出しウィンドウ　補正座標
//--------------------------------------------------------------
#define OFFS_0_0_X (NUM_FX32(0))
#define OFFS_0_0_Y (NUM_FX32(16))
#define OFFS_0_0_Z (NUM_FX32(-8))

#define OFFS_0_1_X (NUM_FX32(-8))
#define OFFS_0_1_Y (NUM_FX32(12))
#define OFFS_0_1_Z (NUM_FX32(-8))

#define OFFS_0_2_X (NUM_FX32(8))
#define OFFS_0_2_Y (NUM_FX32(12))
#define OFFS_0_2_Z (NUM_FX32(-8))

#define OFFS_0_3_X (NUM_FX32(0))
#define OFFS_0_3_Y (NUM_FX32(16))
#define OFFS_0_3_Z (NUM_FX32(-8))

#define OFFS_1_0_X (NUM_FX32(0))
#define OFFS_1_0_Y (NUM_FX32(12))
#define OFFS_1_0_Z (NUM_FX32(-8))

#define OFFS_1_1_X (NUM_FX32(-8))
#define OFFS_1_1_Y (NUM_FX32(12))
#define OFFS_1_1_Z (NUM_FX32(-8))

#define OFFS_1_2_X (NUM_FX32(8))
#define OFFS_1_2_Y (NUM_FX32(12))
#define OFFS_1_2_Z (NUM_FX32(-8))

#define OFFS_1_3_X (NUM_FX32(0))
#define OFFS_1_3_Y (NUM_FX32(12))
#define OFFS_1_3_Z (NUM_FX32(8))

static const VecFx32 data_balloonWinOffsetTbl[2][4] =
{
  { //y0
    {OFFS_0_0_X,OFFS_0_0_Y,OFFS_0_0_Z},
    {OFFS_0_1_X,OFFS_0_1_Y,OFFS_0_1_Z},
    {OFFS_0_2_X,OFFS_0_2_Y,OFFS_0_2_Z},
    {OFFS_0_3_X,OFFS_0_3_Y,OFFS_0_3_Z},
  },
  { //y1
    {OFFS_1_0_X,OFFS_1_0_Y,OFFS_1_0_Z},
    {OFFS_1_1_X,OFFS_1_1_Y,OFFS_1_1_Z},
    {OFFS_1_2_X,OFFS_1_2_Y,OFFS_1_2_Z},
    {OFFS_1_3_X,OFFS_1_3_Y,OFFS_1_3_Z},
  },
};

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ　画面位置から調節位置を取得
 * @param pos 指定座標
 * @param offs 座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
static void balloonWin_GetDispOffsetPos(
    const VecFx32 *pos, VecFx32 *offs,
    const GFL_G3D_CAMERA *cp_g3Dcamera,
    const FIELD_CAMERA *fld_camera )
{
  u32 x,y;
  VecFx32 target_pos = *pos;
  
  {
    VecFx32 camera_way;
    VecFx32 camera_way_xz;
    VecFx32 camera_side;
    VecFx32 camera_up;
    VecFx32 camera_pos;
    VecFx32 camera_target;
    static const VecFx32 up_way = { 0,FX32_ONE,0 };
    
	  GFL_G3D_CAMERA_Switching( cp_g3Dcamera );
    GFL_G3D_CAMERA_GetTarget( cp_g3Dcamera, &camera_target );
    GFL_G3D_CAMERA_GetPos( cp_g3Dcamera, &camera_pos );
    
    VEC_Subtract( &camera_target, &camera_pos, &camera_way );
    //XZ平面横方向取得
    camera_way_xz = camera_way;
    camera_way_xz.y = 0;
    VEC_Normalize( &camera_way_xz, &camera_way_xz );
    
    // 横方向を外積で求める
    VEC_CrossProduct( &camera_way_xz, &up_way, &camera_side );
    // 横とカメラ方向から上を求める
    VEC_CrossProduct( &camera_way, &camera_side, &camera_up );
    VEC_Normalize( &camera_up, &camera_up );
    
    target_pos.x += camera_up.x;
    target_pos.y += camera_up.y;
    target_pos.z += camera_up.z;
  }
  
  NNS_G3dWorldPosToScrPos( &target_pos, (int*)&x, (int*)&y );
  
  OS_Printf( "BALLOONWIN TARGET pos %xH,%xH,%xH : x=%d,y=%d\n",
      pos->x, pos->y, pos->z, x, y );
#if 0  //old
  x >>= 3; // chara size
  x >>= 3; // table size
  y >>= 3; // chara size
  y /= 12; // table size
#else
  x >>= 3; //chara size

  if( x < 10 ){   // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    x = 0;
  }else if( x >= 10 && x <= 15 ){ //10,11,12,13,14,15
    x = 1;
  }else if( x >= 16 && x < 22 ){  //16,17,18,19,20,21
    x = 2;
  }else{          //22,23,24,25,26,27,28,29,30,31
    x = 3;
  }
  
  y >>= 3; // chara size
  y /= 12; // table size
#endif
  
  if( x < 4 && y < 2 ){
    *offs = data_balloonWinOffsetTbl[y][x];
    OS_Printf( "BALLOONWIN TYPE AUTO TABLE POS %d,%d\n", y, x );
  }else{
    VecFx32 error = {0,0,0};
    *offs = error;
    OS_Printf( "BALLOONWIN OUTSIDE SCREEN\n" );
  }
} 

static void balloonWin_SetCameraOffsetPos(
    VecFx32 *offs,
    const GFL_G3D_CAMERA *g3Dcamera )
{
  VecFx32 camPos,camUp,target;
  MtxFx43 mtx43;
  MtxFx33 mtx33; 
  
  GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
  GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
  GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );
  
  MTX_LookAt( &camPos, &camUp, &target, &mtx43 ); //カメラ行列取得
  MTX_Copy43To33( &mtx43, &mtx33 ); //回転行列である3x3部分を取得
  MTX_Inverse33( &mtx33, &mtx33 ); //反転し逆行列にする
  MTX_MultVec33( offs, &mtx33, offs ); //逆行列で回転を無効化し平行化
}

#if 0 //カメラピッチから補正座標
static void balloonWin_SetCameraOffsetPos(
    VecFx32 *offs, const FIELD_CAMERA *fld_camera )
{
  {
    #define PITCH_MAX (0x4000)
    #define PITCH_AVE (0x25d8) ///<標準とする値 t01を見本にしてみた
    #define PITCH_AVE_PER (PITCH_AVE/100)
    #define PITCH_OFFS_X_SPOINT (NUM_FX32(0))
    #define PITCH_OFFS_X_PER (PITCH_OFFS_X_SPOINT/100)
    #define PITCH_OFFS_X(p) \
      (PITCH_OFFS_X_SPOINT-(((p)/PITCH_AVE_PER)*PITCH_OFFS_X_PER))
    #define PITCH_OFFS_Y_SPOINT (NUM_FX32(0))
    #define PITCH_OFFS_Y_PER (PITCH_OFFS_Y_SPOINT/100)
    #define PITCH_OFFS_Y(p) \
      (PITCH_OFFS_Y_SPOINT-(((p)/PITCH_AVE_PER)*PITCH_OFFS_Y_PER))
    #define PITCH_OFFS_Z_SPOINT (NUM_FX32(12))
    #define PITCH_OFFS_Z_PER (PITCH_OFFS_Z_SPOINT/100)
    #define PITCH_OFFS_Z(p) \
      (PITCH_OFFS_Z_SPOINT-(((p)/PITCH_AVE_PER)*PITCH_OFFS_Z_PER))
    
    u16 pitch = FIELD_CAMERA_GetAnglePitch( fld_camera );
    
    if( pitch < PITCH_MAX ){ //補正対象内
      VecFx32 offs_pitch = {0,0,0};
      offs_pitch.x = PITCH_OFFS_X( pitch );
      offs_pitch.y = PITCH_OFFS_Y( pitch );
      offs_pitch.z = PITCH_OFFS_Z( pitch );
      offs->x += offs_pitch.x;
      offs->y += offs_pitch.y;
      offs->z += offs_pitch.z;
      KAGAYA_Printf( "BALLOON PITCH %d(%xH) OFFSET x=%d(%xH), y=%d(%xH), z=%d(%xH)\n",
          pitch, pitch,
          offs_pitch.x, offs_pitch.x,
          offs_pitch.y, offs_pitch.y,
          offs_pitch.z, offs_pitch.z );
    }
  }
}
#endif

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ　指定座標から調節位置を取得
 * @param pos 指定座標
 * @param offs 座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
static void balloonWin_GetOffsetPos(
    const VecFx32 *pos, VecFx32 *offs,
    const GFL_G3D_CAMERA *cp_g3Dcamera,
    const FIELD_CAMERA *fld_camera,
    u8 pos_type )
{
  int x = 0, y = 0;
  
  switch( pos_type ){
  case SCRCMD_MSGWIN_UPLEFT: //ウィンドウ上　吹き出し向き左
    x = 1;
    break;
  case SCRCMD_MSGWIN_DOWNLEFT: //ウィンドウ下　吹き出し向き左
    x = 1;
    y = 1;
    break;
  case SCRCMD_MSGWIN_UPRIGHT: //ウィンドウ上　吹き出し向き右
    x = 2;
    break;
  case SCRCMD_MSGWIN_DOWNRIGHT: //ウィンドウ下　吹き出し向き右
    x = 2;
    y = 1;
    break;
  case SCRCMD_MSGWIN_UP: //ウィンドウ上　吹き出し位置自動
  case SCRCMD_MSGWIN_DOWN: //ウィンドウ下　吹き出し位置自動
  case SCRCMD_MSGWIN_DEFAULT: //自機の位置から自動割り当て
    balloonWin_GetDispOffsetPos( pos, offs, cp_g3Dcamera, fld_camera );
    balloonWin_SetCameraOffsetPos( offs, cp_g3Dcamera );
    return;
  }
  
  *offs = data_balloonWinOffsetTbl[y][x];
  balloonWin_SetCameraOffsetPos( offs, cp_g3Dcamera );
  
#ifdef PM_DEBUG  
  switch( pos_type ){
  case SCRCMD_MSGWIN_UPLEFT:
    OS_Printf( "BALLOONWIN TYPE UL TABLE POS %d,%d\n", y, x );
    break;
  case SCRCMD_MSGWIN_DOWNLEFT:
    OS_Printf( "BALLOONWIN TYPE DL TABLE POS %d,%d\n", y, x );
    break;
  case SCRCMD_MSGWIN_UPRIGHT:
    OS_Printf( "BALLOONWIN TYPE UR TABLE POS %d,%d\n", y, x );
    break;
  case SCRCMD_MSGWIN_DOWNRIGHT:
    OS_Printf( "BALLOONWIN TYPE DR TABLE POS %d,%d\n", y, x );
    break;
  }
#endif
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ　指定位置から調節位置を取得。
 * 外部版。scrcmd_menuwinと同等の位置が欲しい場合に使用する。
 * @param
 * @retval
 */
//--------------------------------------------------------------
void EvCmdBalloonWin_GetOffsetPos(
    const VecFx32 *pos, VecFx32 *offs,
    const GFL_G3D_CAMERA *cp_g3Dcamera,
    const FIELD_CAMERA *fld_camera,
    u8 pos_type )
{
  balloonWin_GetOffsetPos( pos, offs, cp_g3Dcamera, fld_camera, pos_type );
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ　指定位置タイプから
 * FLDTALKMSGWIN_IDX,TAIL_SETPATを取得。
 * @param pos_type
 * @param pOutTail
 * @param pOutTail
 * @retval nothing
 */
//--------------------------------------------------------------
void EvCmdBalloonWin_GetWinType( u16 pos_type,
    FLDTALKMSGWIN_IDX *pOutWin, TAIL_SETPAT *pOutTail )
{
  *pOutWin = FLDTALKMSGWIN_IDX_UPPER;
  *pOutTail = TAIL_SETPAT_NONE;
    
  switch( pos_type ){
  case SCRCMD_MSGWIN_UPLEFT: //ウィンドウ上　吹き出し向き左
    *pOutTail = TAIL_SETPAT_FIX_DL;
    break;
  case SCRCMD_MSGWIN_DOWNLEFT: //ウィンドウ下　吹き出し向き左
    *pOutWin = FLDTALKMSGWIN_IDX_LOWER;
    *pOutTail = TAIL_SETPAT_FIX_UL;
    break;
  case SCRCMD_MSGWIN_UPRIGHT: //ウィンドウ上　吹き出し向き右
    *pOutTail = TAIL_SETPAT_FIX_DR;
    break;
  case SCRCMD_MSGWIN_DOWNRIGHT: //ウィンドウ下　吹き出し向き右
    *pOutWin = FLDTALKMSGWIN_IDX_LOWER;
    *pOutTail = TAIL_SETPAT_FIX_UR;
    break;
  case SCRCMD_MSGWIN_UP: //ウィンドウ上　吹き出し位置自動
    break;
  case SCRCMD_MSGWIN_DOWN: //ウィンドウ下　吹き出し位置自動
    *pOutWin = FLDTALKMSGWIN_IDX_LOWER;
    break;
  case SCRCMD_MSGWIN_DEFAULT:
    GF_ASSERT( 0 && "SYSWIN:ERROR BEFORE POS" );
    *pOutWin = FLDTALKMSGWIN_IDX_LOWER;
    *pOutTail = SCRCMD_MSGWIN_DOWN;
    break;
  }
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ　自機の位置からSCRCMD_MSGWIN_UP,DOWNを取得
 * @param pos 自機と比べる座標
 * @retval u8 SCRCMD_MSGWIN_UP,SCRCMD_MSGWIN_DOWN
 */
//--------------------------------------------------------------
u8 EvCmdBalloonWin_GetWinTypeDefault(
    const MMDL *jiki, const VecFx32 *pos )
{
  VecFx32 jiki_pos;
  u8 type = SCRCMD_MSGWIN_DOWN;
        
  MMDL_GetVectorPos( jiki, &jiki_pos ); //自機の位置から割り当て
  
  if( pos->z < jiki_pos.z ){
    type = SCRCMD_MSGWIN_UP;
  }
  
  return type;
}

//--------------------------------------------------------------
/**
 * バルーンウィンドウクローズ待ち
 * @param core  VMHANDLE*
 * @param wk
 * @retval BOOL
 */
//--------------------------------------------------------------
static BOOL EvWaitballoonWin_Close( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  FLDTALKMSGWIN *tmsg;
  tmsg = (FLDTALKMSGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );

  if( FLDTALKMSGWIN_WaitClose(tmsg) == TRUE ){
     //NULLセット
    SCRCMD_WORK_SetMsgWinPtr( work, NULL );
    SCREND_CHK_SetBitOff(SCREND_CHK_BALLON_WIN_OPEN);
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 *  バルーンウィンドウとじる
 * @param   work    スクリプトコマンドワークポインタ
 * @retval  none
 * @note 終了待ちのWait処理をVMHANDLEにセットしています。
 */
//--------------------------------------------------------------
static void balloonWin_Close(VMHANDLE *core, SCRCMD_WORK *work)
{
  if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) ){
    FLDTALKMSGWIN *tmsg;
    tmsg = (FLDTALKMSGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
    FLDTALKMSGWIN_StartClose( tmsg );
    VMCMD_SetWait( core, EvWaitballoonWin_Close );
  }else{
    #ifdef SCR_ASSERT_ON    
    GF_ASSERT_MSG( 0, "BALLOONWIN:ERROR とじるウィンドウがありません");
    #else
    OS_Printf( "BALLOONWIN:ERROR とじるウィンドウがありません\n" );
    #endif
  }
}

//--------------------------------------------------------------
/**
 *  バルーンウィンドウ削除
 * @param   work    スクリプトコマンドワークポインタ
 * @retval  none
 */
//--------------------------------------------------------------
static void DeleteBalloonWin(SCRCMD_WORK *work)
{
  if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) ){
    FLDTALKMSGWIN *tmsg;
    tmsg = (FLDTALKMSGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
    FLDTALKMSGWIN_Delete( tmsg );
    //NULLセット
    SCRCMD_WORK_SetMsgWinPtr( work, NULL );
    SCREND_CHK_SetBitOff(SCREND_CHK_BALLON_WIN_OPEN);
  }else{
    #ifdef SCR_ASSERT_ON    
    GF_ASSERT_MSG( 0, "BALLOONWIN:ERROR とじるウィンドウがありません");
    #else
    OS_Printf( "BALLOONWIN:ERROR とじるウィンドウがありません\n" );
    #endif
  }
}

//--------------------------------------------------------------
/**
 * 吹きだしメッセージ作成共通部分
 * @param   work
 * @param   fmb
 * @param   idx
 * @param   pos
 * @param   strBuf
 * @return  none
 */
//--------------------------------------------------------------
static void setBalloonWindow( SCRCMD_WORK *work,
  FLDMSGBG *fmb, FLDTALKMSGWIN_IDX idx,
  const VecFx32 *pos, STRBUF *strBuf,
  TALKMSGWIN_TYPE type, TAIL_SETPAT tail )
{
  FLDTALKMSGWIN *tmsg;
  
  tmsg = FLDTALKMSGWIN_AddStrBuf(
      fmb, idx, pos, strBuf, type, tail );
  SCRCMD_WORK_SetMsgWinPtr( work, tmsg );
  //ウィンドウ作成フラグオン
  SCREND_CHK_SetBitOn(SCREND_CHK_BALLON_WIN_OPEN);
}


//--------------------------------------------------------------
/**
 * 座標更新
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void balloonWin_UpdatePos( SCRCMD_WORK *work, BOOL init_flag )
{
  u16 dir;
  MMDL *npc;
  VecFx32 npc_pos;
  MMDLSYS *mmdlsys;
  SCRCMD_BALLOONWIN_WORK *bwin_work;
  
  mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  bwin_work = SCRCMD_WORK_GetBalloonWinWork( work );
  npc = MMDLSYS_SearchOBJID( mmdlsys, bwin_work->obj_id );
  
  if( npc == NULL ){
    GF_ASSERT( 0 && "BALLOONWIN:ERROR TARGET LOST" );
    return;
  }
  
  dir = MMDL_GetDirDisp( npc );
  MMDL_GetVectorPos( npc, &npc_pos );
  
  if( init_flag == TRUE ||
      npc_pos.x != bwin_work->tail_pos_org.x ||
      npc_pos.y != bwin_work->tail_pos_org.y ||
      npc_pos.z != bwin_work->tail_pos_org.z ){
    bwin_work->tail_pos_org = npc_pos;
    
    {
      GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
      FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
      FIELD_CAMERA *p_camera = FIELDMAP_GetFieldCamera( fieldmap );
      const GFL_G3D_CAMERA *cp_g3Dcamera =
        FIELD_CAMERA_GetCameraPtr( p_camera ); //g3Dcamera Lib ハンドル
      u8 pos_type = SCRCMD_WORK_GetWindowPosType( work );
      balloonWin_GetOffsetPos( &bwin_work->tail_pos_org,
          &bwin_work->tail_offs, cp_g3Dcamera, p_camera, pos_type );
    }
  }
  
  bwin_work->tail_pos.x = bwin_work->tail_pos_org.x + bwin_work->tail_offs.x;
  bwin_work->tail_pos.y = bwin_work->tail_pos_org.y + bwin_work->tail_offs.y;
  bwin_work->tail_pos.z = bwin_work->tail_pos_org.z + bwin_work->tail_offs.z;
}

#if 0
#define U_DEF_Y (NUM_FX32(16))
#define U_DEF_Z (NUM_FX32(-3))
#define D_DEF_Y (NUM_FX32(16))
#define D_DEF_Z (NUM_FX32(1))

//--------------------------------------------------------------
//  吹き出しウィンドウ　補正座標
//--------------------------------------------------------------
static const VecFx32 data_balloonWinOffset_Up[DIR_MAX4] =
{
  {NUM_FX32(5),U_DEF_Y,U_DEF_Z}, //fix
  {NUM_FX32(5),U_DEF_Y,U_DEF_Z}, //fix
  {NUM_FX32(-2),U_DEF_Y,U_DEF_Z},
  {NUM_FX32(2),U_DEF_Y,U_DEF_Z},
//  {NUM_FX32(5),U_DEF_Y,U_DEF_Z}, //fix
//  {NUM_FX32(5),U_DEF_Y,U_DEF_Z}, //fix
//  {NUM_FX32(-4),U_DEF_Y,U_DEF_Z},
//  {NUM_FX32(4),U_DEF_Y,U_DEF_Z},
};

static const VecFx32 data_balloonWinOffset_Down[DIR_MAX4] =
{
  {NUM_FX32(5),NUM_FX32(16),D_DEF_Z},
  {NUM_FX32(5),D_DEF_Y,D_DEF_Z},
  {NUM_FX32(-2),D_DEF_Y,D_DEF_Z},
  {NUM_FX32(2),D_DEF_Y,D_DEF_Z},
//  {NUM_FX32(5),D_DEF_Y,D_DEF_Z},
//  {NUM_FX32(5),D_DEF_Y,D_DEF_Z},
//  {NUM_FX32(-4),D_DEF_Y,D_DEF_Z},
//  {NUM_FX32(4),D_DEF_Y,D_DEF_Z},
};


static void balloonWin_UpdatePos( SCRCMD_WORK *work )
{
  u16 dir;
  MMDL *npc;
  MMDLSYS *mmdlsys;
  const VecFx32 *offs;
  SCRCMD_BALLOONWIN_WORK *bwin_work;

  mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  bwin_work = SCRCMD_WORK_GetBalloonWinWork( work );
  npc = MMDLSYS_SearchOBJID( mmdlsys, bwin_work->obj_id );
  
  if( npc == NULL ){
    OS_Printf( "BALLOON WINDOW TARGET LOST\n" );
    GF_ASSERT( 0 );
    return;
  }
  
  dir = MMDL_GetDirDisp( npc );
  MMDL_GetVectorPos( npc, &bwin_work->tail_pos );

  offs = &data_balloonWinOffset_Up[dir];
  
  if( bwin_work->win_idx == FLDTALKMSGWIN_IDX_LOWER ){
    offs = &data_balloonWinOffset_Down[dir];
  }
   
  bwin_work->tail_pos.x += offs->x;
  bwin_work->tail_pos.y += offs->y;
  bwin_work->tail_pos.z += offs->z;
}
#endif

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ作成
 * @param work  SCRCMD_WORK
 * @param objID 吹き出しを出すOBJID
 * @param arcID 表示するメッセージのアーカイブ指定ID
 * @param msgID 表示するメッセージID
 * @param idx  FLDTALKMSGWIN_IDX 
 * @retval BOOL TRUE=表示 FALSE=エラー
 * @note arcID=0x400の場合、デフォルトのメッセージアーカイブを使用する
 */
//--------------------------------------------------------------
static BOOL balloonWin_SetWrite( SCRCMD_WORK *work,
    u16 objID, u16 arcID, u16 msgID, u16 sc_pos_type,
    TALKMSGWIN_TYPE type )
{
  STRBUF *msgbuf;
  SCRCMD_BALLOONWIN_WORK *bwin_work;
  u16 pos_type = sc_pos_type;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *jiki = MMDLSYS_SearchOBJID( mmdlsys, MMDL_ID_PLAYER );
  MMDL *npc = MMDLSYS_SearchOBJID( mmdlsys, objID );
  
  if( npc == NULL ){
    OS_Printf("スクリプトエラー 吹き出し対象のOBJが存在しません\n");
    return( FALSE );
  }
  
  msgbuf = SCRIPT_GetMsgBuffer( sc );

  { //メッセージ初期化
    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );

    if (arcID == 0x400){
      GFL_MSG_GetString( msgData, msgID, tmpbuf );
    }else{
      msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
          arcID, SCRCMD_WORK_GetHeapID( work ) );
      GFL_MSG_GetString( msgData, msgID, tmpbuf );
      GFL_MSG_Delete( msgData );
    }

    WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
  }

  bwin_work = SCRCMD_WORK_GetBalloonWinWork( work );
  
  //既にビットが立っていたら新しく作らない
  if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) )
  {
    FLDTALKMSGWIN *tmsg = SCRCMD_WORK_GetMsgWinPtr( work );
    GF_ASSERT( tmsg != NULL );
    FLDTALKMSGWIN_ResetMessageStrBuf( tmsg, msgbuf );
  }
  else //初期化
  { 
    FLDTALKMSGWIN_IDX idx;
    TAIL_SETPAT tail;
    
    MI_CpuClear8( bwin_work, sizeof(SCRCMD_BALLOONWIN_WORK) );
    
    bwin_work->obj_id = objID;
    
    if( pos_type == SCRCMD_MSGWIN_DEFAULT ){ //過去or自機の位置から割り当て
      u16 before_pos = SCRCMD_WORK_GetBeforeWindowPosType( work );
#if 0 //old      
      if( before_pos == SCRCMD_MSGWIN_MAX ){ //初回である
        VecFx32 pos,jiki_pos;
        
        MMDL_GetVectorPos( npc, &pos );
        MMDL_GetVectorPos( jiki, &jiki_pos ); //時期の位置から割り当て
        
        pos_type = SCRCMD_MSGWIN_DOWN;
        
        if( pos.z < jiki_pos.z ){
          pos_type = SCRCMD_MSGWIN_UP;
        }
      }else{ //履歴あり
        pos_type = before_pos;
      }
#else //吹き出しデフォルトは常に自機の位置から割り当てになった
      {
        VecFx32 pos;
        MMDL_GetVectorPos( npc, &pos );
        pos_type = EvCmdBalloonWin_GetWinTypeDefault( jiki, &pos );
      }
#endif
    }
    
    EvCmdBalloonWin_GetWinType( pos_type, &idx, &tail );
    
    bwin_work->win_idx = idx;
    SCRCMD_WORK_SetBeforeWindowPosType( work, pos_type );
    SCRCMD_WORK_SetWindowPosType( work, sc_pos_type );

    balloonWin_UpdatePos( work, TRUE );
    
    setBalloonWindow( work,
        fparam->msgBG, idx, &bwin_work->tail_pos, msgbuf, type, tail );
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ　表示完了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL BallonWinMsgWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  FLDTALKMSGWIN *tmsg;
  tmsg = (FLDTALKMSGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
  
  balloonWin_UpdatePos( work, FALSE );
  
  if( FLDTALKMSGWIN_Print(tmsg) == TRUE ){
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ 描画 OBJID指定有り
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBalloonWinWrite( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 arc_id = SCRCMD_GetVMWorkValue( core, work );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u16 obj_id = SCRCMD_GetVMWorkValue( core, work );
  u16 pos_type = SCRCMD_GetVMWorkValue( core, work );
  TALKMSGWIN_TYPE type = SCRCMD_GetVMWorkValue( core, work );
  
  OS_Printf( "吹き出しウィンドウ OBJID =%d\n", obj_id );
  
  if( balloonWin_SetWrite(work,obj_id,arc_id,msg_id,pos_type,type) == TRUE ){
    VMCMD_SetWait( core, BallonWinMsgWait );
    return VMCMD_RESULT_SUSPEND;
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ 描画 話し掛けOBJ対象
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBalloonWinTalkWrite( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDL *mmdl = SCRIPT_GetTargetObj( sc );
  u16 arc_id = SCRCMD_GetVMWorkValue( core, work );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u16 pos_type = SCRCMD_GetVMWorkValue( core, work );
  TALKMSGWIN_TYPE type = SCRCMD_GetVMWorkValue( core, work );

  if( mmdl == NULL ){
    OS_Printf( "スクリプトエラー 話し掛け対象のOBJが居ません\n" );
    return VMCMD_RESULT_SUSPEND;
  }

  {
    u8 obj_id = MMDL_GetOBJID( mmdl );
    
    if( balloonWin_SetWrite(work,obj_id,arc_id,msg_id,pos_type,type) == TRUE ){
      VMCMD_SetWait( core, BallonWinMsgWait );
      return VMCMD_RESULT_SUSPEND;
    }
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ 描画 男女別メッセージ表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBalloonWinWriteMF( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 arc_id = SCRCMD_GetVMWorkValue( core, work );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u16 msg_id_f = SCRCMD_GetVMWorkValue( core, work );
  u16 obj_id = SCRCMD_GetVMWorkValue( core, work );
  u16 pos_type = SCRCMD_GetVMWorkValue( core, work );
  TALKMSGWIN_TYPE type = SCRCMD_GetVMWorkValue( core, work );

  {
    GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
    PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
    u32 sex = MyStatus_GetMySex( &player->mystatus );
    
    if( sex != PM_MALE ){
      msg_id = msg_id_f;
    }
  }
  
  if( balloonWin_SetWrite(work,obj_id,arc_id,msg_id,pos_type,type) == TRUE ){
    VMCMD_SetWait( core, BallonWinMsgWait );
    return VMCMD_RESULT_SUSPEND;
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ 描画 バージョン別メッセージ表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBalloonWinWriteWB( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 arc_id = SCRCMD_GetVMWorkValue( core, work );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u16 msg_id_b = SCRCMD_GetVMWorkValue( core, work );
  u16 obj_id = SCRCMD_GetVMWorkValue( core, work );
  u16 pos_type = SCRCMD_GetVMWorkValue( core, work );
  TALKMSGWIN_TYPE type = SCRCMD_GetVMWorkValue( core, work );
  
  if( GET_VERSION() == VERSION_BLACK ){
    msg_id = msg_id_b;
  }
  
  if( balloonWin_SetWrite(work,obj_id,arc_id,msg_id,pos_type,type) == TRUE ){
    VMCMD_SetWait( core, BallonWinMsgWait );
    return VMCMD_RESULT_SUSPEND;
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウをとじる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBalloonWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  balloonWin_Close(core,work);       //<<内部で既閉チェックをしています
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * スクリプト終了時吹きだしウィンドウ終了チェック
 * @param   end_chk     チェック構造体
 * @param   seq     サブシーケンサ
 * @retval  BOOL    TRUEでチェック終了
 */
//--------------------------------------------------------------
BOOL SCREND_CheckEndBallonWin(SCREND_CHECK *end_check , int *seq)
{
  DeleteBalloonWin(end_check->ScrCmdWk);
  return  TRUE;
}

//======================================================================
//  トレーナーメッセージ
//======================================================================
//--------------------------------------------------------------
/**
 * トレーナー会話呼び出し
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerMessageSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  
  u16 tr_id = SCRCMD_GetVMWorkValue( core, work );
  u16 kind_id = SCRCMD_GetVMWorkValue( core, work );
  u16 obj_id  = SCRCMD_GetVMWorkValue( core, work );
  
  STRBUF *msgbuf = SCRIPT_GetMsgBuffer( sc );
  
  KAGAYA_Printf( "TR ID =%d, KIND ID =%d\n", tr_id, kind_id );
  
  //吹きだしウィンドウ既開チェック開いていたら処理を行わない
  if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) )
  {
    #ifdef SCR_ASSERT_ON     
    GF_ASSERT_MSG( 0,"TR_MSG_ERROR:既にウィンドウあり\n");
    #else
    OS_Printf( "TR_MSG_ERROR:既にウィンドウありn");
    #endif
    return VMCMD_RESULT_SUSPEND;
  }
  
  TT_TrainerMessageGet(
      tr_id, kind_id, msgbuf, SCRCMD_WORK_GetHeapID(work) );
  
  { //トレーナーOBJを探す
    MMDLSYS * mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
    if ( MMDLSYS_SearchOBJID( mmdlsys, obj_id ) == NULL ) {
      //対象トレーナーOBJが見つからない
      GF_ASSERT( 0 );
      obj_id = MMDL_ID_PLAYER;
    }
  }
    
  {
    SCRCMD_BALLOONWIN_WORK *bwin_work;
    FIELD_PLAYER *fld_player;
    u16 dir;
    
    bwin_work = SCRCMD_WORK_GetBalloonWinWork( work );
    MI_CpuClear8( bwin_work, sizeof(SCRCMD_BALLOONWIN_WORK) );

    bwin_work->obj_id = obj_id;
    
    fld_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
    dir = MMDL_GetDirDisp( FIELD_PLAYER_GetMMdl( fld_player ) );

    {
      u8 pos_type = SCRCMD_MSGWIN_UPLEFT;
      FLDTALKMSGWIN_IDX idx = FLDTALKMSGWIN_IDX_LOWER;
      
      if( dir == DIR_UP ){
        pos_type = SCRCMD_MSGWIN_DOWNLEFT;
        idx = FLDTALKMSGWIN_IDX_UPPER;
      }
      
      bwin_work->win_idx = idx;
      balloonWin_UpdatePos( work, TRUE );
      
      setBalloonWindow( work, fparam->msgBG,
          idx, &bwin_work->tail_pos, msgbuf,
          TALKMSGWIN_TYPE_NORMAL, TAIL_SETPAT_NONE );
      
      SCRCMD_WORK_SetBeforeWindowPosType( work, pos_type );

      VMCMD_SetWait( core, BallonWinMsgWait );
      return VMCMD_RESULT_SUSPEND;
    }
  }
}

//======================================================================
//  プレーンウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * プレーンウィンドウメッセージ表示　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL PlainWinMsgWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  FLDPLAINMSGWIN *plnWin;
  plnWin = (FLDPLAINMSGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
  
  if( FLDPLAINMSGWIN_PrintStream(plnWin) == TRUE ){
    return( 1 );
  }
  
  return( 0 );
}

//--------------------------------------------------------------
/**
 * プレーンウィンドウ　作成　メイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
static VMCMD_RESULT addPlainWin(
    VMHANDLE *core, void *wk, TALKMSGWIN_TYPE type )
{
  STRBUF *msgbuf;
  FLDPLAINMSGWIN *win;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u8 up_down = VMGetU8( core );
  u16 x = 1;
  u16 y = 1;
  u16 sx = 30;
  u16 sy = 4;
  
  msgbuf = SetExpandWord( work, sc, msg_id );
  
  if( up_down != SCRCMD_MSGWIN_UP ){ //down
    y = 19;
  }
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_PLAINWIN_OPEN) ){ //オープン済み
    win = SCRCMD_WORK_GetMsgWinPtr( work );
    FLDPLAINMSGWIN_ClearMessage( win );
  }else{
    win = FLDPLAINMSGWIN_Add( fparam->msgBG, NULL, type, x, y, sx, sy );
    SCRCMD_WORK_SetMsgWinPtr( work, win );
  }
  
  FLDPLAINMSGWIN_PrintStreamStartStrBuf( win,
      PLAINMSGWIN_WRITE_START_X, PLAINMSGWIN_WRITE_START_Y, msgbuf );
  SCREND_CHK_SetBitOn(SCREND_CHK_PLAINWIN_OPEN);
  
  VMCMD_SetWait( core, PlainWinMsgWait );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * プレーンウィンドウ　とじる 
 * @param work SCRCMD_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void plainWin_Close( SCRCMD_WORK *work )
{
  if( SCREND_CHK_CheckBit(SCREND_CHK_PLAINWIN_OPEN) ){
    FLDPLAINMSGWIN *win = (FLDPLAINMSGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
    FLDPLAINMSGWIN_Delete( win );
    //NULLセット
    SCRCMD_WORK_SetMsgWinPtr( work, NULL );
    SCREND_CHK_SetBitOff(SCREND_CHK_PLAINWIN_OPEN);
  }else{
    GF_ASSERT( 0 && "PLAINWIN:ERROR とじるウィンドウがありません" );
  }
}


//--------------------------------------------------------------
/**
 * プレーンウィンドウ　作成
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlainWinMsg( VMHANDLE *core, void *wk )
{
  return( addPlainWin(core,wk,TALKMSGWIN_TYPE_NORMAL) );
}

//--------------------------------------------------------------
/**
 * プレーンウィンドウ　ギザギザ　作成
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlainGizaWinMsg( VMHANDLE *core, void *wk )
{
  return( addPlainWin(core,wk,TALKMSGWIN_TYPE_GIZA) );
}

//--------------------------------------------------------------
/**
 * プレーンウィンドウ　とじる
 * @param core  仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlainWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  plainWin_Close( work );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * プレーンウィンドウ　終了チェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL SCREND_CheckEndPlainWin( SCREND_CHECK *end_check, int *seq )
{
  plainWin_Close( end_check->ScrCmdWk );
  return( TRUE );
}

//======================================================================
//  サブウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * サブウィンドウ　作成
 * @param
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSubWinMsg( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  u16 msg_id = VMGetU16( core );
  u16 x = SCRCMD_GetVMWorkValue( core, work );
  u16 y = SCRCMD_GetVMWorkValue( core, work );
  u16 win_id = VMGetU16( core );
  u16 sx;
  u8 cx;
  u8 cy = 2;
  
  {
    HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
    STRBUF *msgBuf = GFL_STR_CreateBuffer( FLDMSGBG_STRLEN_SUBWIN, heapID );
    
    GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );
    
    GFL_MSG_GetString( msgData, msg_id, tmpbuf );
    WORDSET_ExpandStr( wordset, msgBuf, tmpbuf );

#if 0    
    sx = GFL_STR_GetBufferLength( msgBuf );
    sx = sx * 12;
    cx = sx / 8;
    if( (sx&0x07) ){ cx++; }
#else
    FLDSPWIN_GetNeedWindowCharaSize( fparam->msgBG, msgBuf, &cx, &cy );
#endif
    
#if 1
    if( (x + cx) > 32 ){
      OS_Printf( "SUBWIN ERROR: X(%d),WIDTH(%d) OVER 32", x, cx );
      GF_ASSERT( 0 );
      x = 4;
      cx = 4;
    }
    
    if( (y + cy) > 24 ){
      OS_Printf( "SUBWIN ERROR: Y(%d),HEIGHT(%d) OVER 24", y, cy );
      GF_ASSERT( 0 );
      y = 4;
      cy = 4;
    }
#endif
    
    FLDSUBMSGWIN_AddStrBuf( fparam->msgBG, msgBuf, win_id, x, y, cx, cy );
    GFL_STR_DeleteBuffer( msgBuf );
  }
  
  SCREND_CHK_SetBitOn( SCREND_CHK_SUBWIN_OPEN );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * サブウィンドウ　とじる
 * @param
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSubWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  u16 win_id = VMGetU16( core );
  FLDSUBMSGWIN_Delete( fparam->msgBG, win_id );
  
  if( FLDSUBMSGWIN_CheckExistWindow(fparam->msgBG) == FALSE ){
    SCREND_CHK_SetBitOff( SCREND_CHK_SUBWIN_OPEN );
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * サブウィンドウ　全てとじる
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void CloseSubWin( SCRCMD_WORK *work )
{
  if( SCREND_CHK_CheckBit(SCREND_CHK_SUBWIN_OPEN) ){
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
    FLDSUBMSGWIN_DeleteAll( fparam->msgBG );
    SCREND_CHK_SetBitOff(SCREND_CHK_SUBWIN_OPEN);
  }else{
    GF_ASSERT( 0 );
  }
}

//--------------------------------------------------------------
/**
 * サブウィンドウ　終了チェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL SCREND_CheckEndSubWin( SCREND_CHECK *end_check, int *seq )
{
  CloseSubWin( end_check->ScrCmdWk );
  return( TRUE );
}

//======================================================================
//  BGウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * BGウィンドウメッセージ表示　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL BGWinMsgWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  FLDBGWIN *bgWin;
  STRBUF *msgBuf;
  
  msgBuf = SCRIPT_GetMsgBuffer( sc );
  bgWin = (FLDBGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
  
  if( FLDBGWIN_PrintStrBuf(bgWin,msgBuf) == TRUE ){
    return( 1 );
  }
  
  return( 0 );
}

//--------------------------------------------------------------
/**
 * BGウィンドウとじる
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void bgWin_Close( SCRCMD_WORK *work )
{
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );

  if( SCREND_CHK_CheckBit(SCREND_CHK_BGWIN_OPEN) ){
    FLDBGWIN *bgWin;
    GFL_FONT* fontHandle;
    
    bgWin = (FLDBGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
    FLDBGWIN_Delete( bgWin );

    // 海底神殿でフォントを読み込んでいる可能性がある。
    fontHandle = SCRIPT_GetFontHandle( sc );
    SCRIPT_SetFontHandle( sc, NULL );
    if( fontHandle ){
      GFL_FONT_Delete( fontHandle );
    }
    //NULLセット
    SCRCMD_WORK_SetMsgWinPtr( work, NULL ); 
    SCREND_CHK_SetBitOff(SCREND_CHK_BGWIN_OPEN);
  }else{
    GF_ASSERT( 0 );
  }
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　作成
 * @param
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBGWinMsg( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 msg_id = VMGetU16( core );
  u16 type = VMGetU16( core );
  
  {
    SCRIPT_FLDPARAM *fparam;
    GFL_MSGDATA *msgData;
    FLDBGWIN *bgWin;
    
    fparam = SCRIPT_GetFieldParam( sc );
    msgData = SCRCMD_WORK_GetMsgData( work );
    
	  bgWin = FLDBGWIN_Add( fparam->msgBG, type );
    SCRCMD_WORK_SetMsgWinPtr( work, bgWin );
    
    {
      STRBUF *msgbuf = SCRIPT_GetMsgBuffer( sc );
      WORDSET *wordset = SCRIPT_GetWordSet( sc );
      STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );
      GFL_MSG_GetString( msgData, msg_id, tmpbuf );
      WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
      
      VMCMD_SetWait( core, BGWinMsgWait );
    }
  }
  
  SCREND_CHK_SetBitOn( SCREND_CHK_BGWIN_OPEN);
  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  海底神殿　暗号BGメッセージ　表示
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdBGWinSeaTempleMsg( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  u16 msg_id = VMGetU16( core );
  u16 type = VMGetU16( core );
  HEAPID heapID = FIELDMAP_GetHeapID( fparam->fieldMap );
   
  { 
    SCRIPT_FLDPARAM *fparam;
    GFL_MSGDATA *msgData;
    FLDBGWIN *bgWin;
    GFL_FONT* fontHandle;
    
    fparam = SCRIPT_GetFieldParam( sc );
    msgData = SCRCMD_WORK_GetMsgData( work );

    fontHandle = GFL_FONT_Create(
      ARCID_FONT,
      NARC_font_shinden_gftr, //新フォントID
      GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
    SCRIPT_SetFontHandle( sc, fontHandle );
    
	  bgWin = FLDBGWIN_AddEx( fparam->msgBG, type, fontHandle );
    SCRCMD_WORK_SetMsgWinPtr( work, bgWin );
    
    {
      STRBUF *msgbuf = SCRIPT_GetMsgBuffer( sc );
      WORDSET *wordset = SCRIPT_GetWordSet( sc );
      STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );
      GFL_MSG_GetString( msgData, msg_id, tmpbuf );
      WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
      
      VMCMD_SetWait( core, BGWinMsgWait );
    }
  }
  
  SCREND_CHK_SetBitOn( SCREND_CHK_BGWIN_OPEN);
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　とじる
 * @param
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBGWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  bgWin_Close( work );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　終了チェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL SCREND_CheckEndBGWin( SCREND_CHECK *end_check, int *seq )
{
  bgWin_Close( end_check->ScrCmdWk );
  return( TRUE );
}

//======================================================================
//  特殊ウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * 特殊ウィンドウ　とじる
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void spWin_Close( SCRCMD_WORK *work )
{
  if( SCREND_CHK_CheckBit(SCREND_CHK_SPWIN_OPEN) ){
    FLDSPWIN *spWin = SCRCMD_WORK_GetMsgWinPtr( work );
    FLDSPWIN_Delete( spWin );
    //NULLセット
    SCRCMD_WORK_SetMsgWinPtr( work, NULL );
    SCREND_CHK_SetBitOff( SCREND_CHK_SPWIN_OPEN );
  }else{
    GF_ASSERT( 0 );
  }
}

//--------------------------------------------------------------
/**
 * 特殊ウィンドウ　ウェイト部分
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL SpWinMsgWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  FLDSPWIN *spWin = SCRCMD_WORK_GetMsgWinPtr( work );
  
  if( FLDSPWIN_Print(spWin) == TRUE ){
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 特殊ウィンドウ　作成
 * @param
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSpWinMsg( VMHANDLE *core, void *wk )
{
  FLDSPWIN *spWin;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 msg_id = VMGetU16( core );
  u8 x = VMGetU8( core );
  u8 y = VMGetU8( core );
  u16 type = VMGetU16( core );
  STRBUF *msgbuf = SetExpandWord( work, sc, msg_id );
  
  {
#if 0
    u32 len;
    SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
    GFL_FONT *font = FLDMSGBG_GetFontHandle( fparam->msgBG );
    u32 w = PRINTSYS_GetStrWidth( msgbuf, font, 0 ) + 8;
    u32 h = PRINTSYS_GetStrHeight( msgbuf, font ) + 8;
    u32 c = w;
    w >>= 3;
    if( (c&0x07) ){
      w++;
    }
    c = h;
    h >>= 3;
    if( (c&0x07) ){
      h++;
    }
#else
    SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
    u16 w = FLDSPWIN_GetNeedWindowWidthCharaSize( fparam->msgBG, msgbuf, 4 );
    u16 h = FLDSPWIN_GetNeedWindowHeightCharaSize( fparam->msgBG, msgbuf, 4 );
#endif
    spWin = FLDSPWIN_Add( fparam->msgBG, type, x, y, w, h );
    SCRCMD_WORK_SetMsgWinPtr( work, spWin );
  }
  
  FLDSPWIN_PrintStrBufStart( spWin, 4, 4, msgbuf );
  VMCMD_SetWait( core, SpWinMsgWait );
  
  SCREND_CHK_SetBitOn( SCREND_CHK_SPWIN_OPEN );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 特殊ウィンドウとじる
 * @param
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSpWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  spWin_Close( work );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 特殊ウィンドウ　終了チェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL SCREND_CheckEndSpWin( SCREND_CHECK *end_check, int *seq )
{
  spWin_Close( end_check->ScrCmdWk );
  return( TRUE );
}

//======================================================================
//  その他メッセージウィンドウ系コマンド
//======================================================================
//--------------------------------------------------------------
/**
 * メッセージウィンドウ共通　メッセージウィンドウとじる
 * @param 
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMsgWinClose( VMHANDLE *core, void *wk )
{
  BOOL close;
  SCRCMD_WORK *work = wk;

  close = FALSE;
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_WIN_OPEN) ){
    sysWin_Close( work );
    close = TRUE;
  }
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_PLAINWIN_OPEN) ){
    plainWin_Close( work );
    close = TRUE;
  }
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_BGWIN_OPEN) ){
    bgWin_Close( work );
    close = TRUE;
  }
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_SPWIN_OPEN) ){
    spWin_Close( work );
    close = TRUE;
  }

  if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) ){
    balloonWin_Close( core, work );
    close = TRUE;
  }
  
  GF_ASSERT_MSG( close, "MSGWIN_CLOSE:とじるウィンドウがありません" );
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//  キー待ちカーソル
//======================================================================
//--------------------------------------------------------------
/**
 * キー待ちカーソル表示　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
static BOOL EvWaitKeyWaitMsgCursor( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  int trg = GFL_UI_KEY_GetTrg();
  void *win = SCRCMD_WORK_GetMsgWinPtr( work );
  
  if( trg & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
    PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
    return( TRUE );
  }
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_WIN_OPEN) ){
    FLDSYSWIN_WriteKeyWaitCursor( (FLDSYSWIN_STREAM*)win );
  }else if( SCREND_CHK_CheckBit(SCREND_CHK_PLAINWIN_OPEN) ){
    FLDPLAINMSGWIN_WriteKeyWaitCursor( (FLDPLAINMSGWIN*)win );
  }else if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) ){
    FLDTALKMSGWIN_WriteKeyWaitCursor( (FLDTALKMSGWIN*)win );
  }else{
    GF_ASSERT( 0 && "KEYWAIT_MARK:ERROR NONE WINDOW" );
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * キー待ちカーソル表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdKeyWaitMsgCursor( VMHANDLE *core, void *wk )
{
  VMCMD_SetWait( core, EvWaitKeyWaitMsgCursor );
  return VMCMD_RESULT_SUSPEND;
}


//----------------------------------------------------------------------------
/**
 *	@brief  メッセージwindow　自動キー送り　＋　メッセージスピード一定　設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWinMsgSetAutoPrintFlag( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  u16 flag = VMGetU16( core );

  if( flag ){
    FLDMSGBG_SetAutoPrintFlag( fparam->msgBG, TRUE );
    SCREND_CHK_SetBitOn( SCREND_CHK_MSGWIN_AUTO_PRINT );
    
    TOMOYA_Printf( "ON\n" );
  }else{
    FLDMSGBG_SetAutoPrintFlag( fparam->msgBG, FALSE );
    SCREND_CHK_SetBitOff( SCREND_CHK_MSGWIN_AUTO_PRINT );

    TOMOYA_Printf( "OFF\n" );
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージwindow　自動キー送り　＋　メッセージスピード一定　OFF　チェック
 */
//-----------------------------------------------------------------------------
BOOL SCREND_CheckEndWinMsgAutoPrintFlag( SCREND_CHECK *end_check, int *seq )
{
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( end_check->ScrWk );
  if( SCREND_CHK_CheckBit( SCREND_CHK_MSGWIN_AUTO_PRINT ) ){
    FLDMSGBG_SetAutoPrintFlag( fparam->msgBG, FALSE );
    SCREND_CHK_SetBitOff( SCREND_CHK_MSGWIN_AUTO_PRINT );
  }
  return TRUE;
}




//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * スクリプトワーク　WORDSETによるSTRBUF作成
 * @param
 * @retval
 */
//--------------------------------------------------------------
static STRBUF * SetExpandWord( SCRCMD_WORK *work, SCRIPT_WORK *sc, u32 msg_id )
{
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
  STRBUF *msgbuf = SCRIPT_GetMsgBuffer( sc );
  WORDSET *wordset = SCRIPT_GetWordSet( sc );
  STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );
  
  GFL_MSG_GetString( msgData, msg_id, tmpbuf );
  WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
  return( msgbuf );
}
