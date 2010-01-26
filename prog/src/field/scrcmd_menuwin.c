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

#include "scrcmd.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd_work.h"
#include "scrcmd_menuwin.h"

#include "fieldmap.h"

#include "arc/script_message.naix"
#include "msg/script/msg_common_scr.h"

//======================================================================
//  define
//======================================================================
#define DEBUG_BWINTYPE TALKMSGWIN_TYPE_NORMAL

// 所持金表示ウィンドウ
#define GOLD_WIN_WIDTH (14) // 幅(キャラクタ単位)
#define GOLD_WIN_HEIGHT (2) // 高さ(キャラクタ単位)
#define GOLD_WIN_KETA   (6) // 所持金数値の桁数

//======================================================================
//  proto
//======================================================================
static void CloseWin(SCRCMD_WORK *work);
static void CloseBalloonWin(VMHANDLE *core, SCRCMD_WORK *work);
static void DeleteBalloonWin(SCRCMD_WORK *work);

static void setBalloonWindow( SCRCMD_WORK *work,
  FLDMSGBG *fmb, FLDTALKMSGWIN_IDX idx,
  const VecFx32 *pos, STRBUF *strBuf, TALKMSGWIN_TYPE type );

static STRBUF * SetExpandWord(
    SCRCMD_WORK *work, SCRIPT_WORK *sc, u32 msg_id );

//======================================================================
//  スクリプトチェック
//======================================================================
//--------------------------------------------------------------
/**
 * スクリプト終了時ウィンドウ終了チェック
 * @param   end_chk     チェック構造体
 * @param   seq     サブシーケンサ
 * @retval  BOOL    TRUEでチェック終了
 */
//--------------------------------------------------------------
BOOL SCREND_CheckEndWin(SCREND_CHECK *end_check , int *seq)
{
  CloseWin(end_check->ScrCmdWk);
  return  TRUE;
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
//  はい、いいえ　処理
//======================================================================
//--------------------------------------------------------------
/**
 * 「はい・いいえ」処理 ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvYesNoWinSelect( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  FLDMENUFUNC **mw = SCRIPT_GetMemberWork( sc, ID_EVSCR_MENUWORK );
  u16 *ret_wk = SCRIPT_GetEventWork( sc, gdata, core->vm_register[0] );
  FLDMENUFUNC_YESNO ret = FLDMENUFUNC_ProcYesNoMenu( *mw );
  
  if( ret == FLDMENUFUNC_YESNO_NULL ){
    return FALSE;
  }
  
  if( ret == FLDMENUFUNC_YESNO_YES ){
    *ret_wk = 0;
  }else{
    *ret_wk = 1;
  }
  
  FLDMENUFUNC_DeleteMenu( *mw );
  return TRUE;
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
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMENUFUNC **mw  = SCRIPT_GetMemberWork( sc, ID_EVSCR_MENUWORK );
  u16 wk_id      = VMGetU16( core );
  
  *mw = FLDMENUFUNC_AddYesNoMenu( fparam->msgBG, 0 );
  core->vm_register[0] = wk_id;
   
  VMCMD_SetWait( core, EvYesNoWinSelect );
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//  メニュー
//======================================================================
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
  u8 cursor = VMGetU8(core);
  u8 cancel = VMGetU8(core);
  u16 wk_id = VMGetU16( core );
  u16 *ret = SCRIPT_GetEventWork( sc, gdata, wk_id );
  WORDSET *wordset = SCRIPT_GetWordSet( sc );
  
  SCRCMD_WORK_InitMenuWork( work,
      x, y, cursor, cancel, ret,
      wordset, NULL );
  
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
  u8 cursor = VMGetU8(core);
  u8 cancel = VMGetU8(core);
  u16 wk_id = VMGetU16( core );
  u16 *ret = SCRIPT_GetEventWork( sc, gdata, wk_id );
  WORDSET *wordset = SCRIPT_GetWordSet( sc );
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );

  SCRCMD_WORK_InitMenuWork( work,
      x, y, cursor, cancel, ret,
      wordset, msgData );
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
	u16 param	= VMGetU16( core );
  STRBUF **msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
  SCRCMD_WORK_AddMenuList( work, msg_id, ex_msg_id, param, *msgbuf, *tmpbuf );
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * BMPメニュー	ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvSelWinWait( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  if( SCRCMD_WORK_ProcMenu(work) == TRUE ){
    return( TRUE );
  }
  return( FALSE );
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
  SCRCMD_WORK_StartMenu( work );
	VMCMD_SetWait( core, EvSelWinWait );
	return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BMPメニュー	ウェイト部分(xボタンによる中断あり)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvSelWinWait_Breakable( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  if( SCRCMD_WORK_ProcMenu_Breakable(work) == TRUE ){
    return( TRUE );
  }
  return( FALSE );
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
  SCRCMD_WORK_StartMenu( work );
	VMCMD_SetWait( core, EvSelWinWait_Breakable );
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
static void sysWin_AddWindow( SCRCMD_WORK *work, u8 updown )
{
  FLDSYSWIN_STREAM *sysWin;
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_WIN_OPEN) ) //メニューオープン済み
  {
    sysWin = SCRCMD_WORK_GetMsgWinPtr( work );
    FLDSYSWIN_STREAM_ClearMessage( sysWin );
  }
  else
  {
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
    GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );

    if( updown == SCRCMD_MSGWIN_UP ){
	    sysWin = FLDSYSWIN_STREAM_Add( fparam->msgBG, msgData, 1 );
    }else{ //DOWN
	    sysWin = FLDSYSWIN_STREAM_Add( fparam->msgBG, msgData, 19 );
    }
    
    SCRCMD_WORK_SetMsgWinPtr( work, sysWin );
    SCREND_CHK_SetBitOn(SCREND_CHK_WIN_OPEN);
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
  STRBUF **msgbuf;
  STRBUF **tmpbuf;
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
    tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
    KAGAYA_Printf( "EvCmdTalkMsg MSG ID = %d\n", msg_id );
    GFL_MSG_GetString( msgData, msg_id, *tmpbuf );
    WORDSET_ExpandStr( wordset, *msgbuf, *tmpbuf );
  }
  
  {
    FLDTALKMSGWIN_IDX idx = FLDTALKMSGWIN_IDX_LOWER;
    
    if( dir == DIR_UP ){
      idx = FLDTALKMSGWIN_IDX_UPPER;
    }
    
    tmsg = FLDTALKMSGWIN_AddStrBuf( fparam->msgBG, idx, pos_p, *msgbuf );
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
  STRBUF **msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u16 up_down = VMGetU16( core );
  
  sysWin_AddWindow( work, up_down );
  
  {
    GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
    GFL_MSG_GetString( msgData, msg_id, *tmpbuf );
    WORDSET_ExpandStr( wordset, *msgbuf, *tmpbuf );
  }
  
  sysWin = (FLDSYSWIN_STREAM*)SCRCMD_WORK_GetMsgWinPtr( work );
  FLDSYSWIN_STREAM_ClearMessage( sysWin );
  FLDSYSWIN_STREAM_AllPrintStrBuf( sysWin, 0, 0, *msgbuf );
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
 * システムウィンドウを閉じる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSysWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  CloseWin(work);   //<<内部で既閉チェックをしています
  return VMCMD_RESULT_CONTINUE;
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
  STRBUF**        tempbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
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

  // ウィンドウを表示
  msg_win = FLDMSGWIN_Add( msg_bg, msg_data, x, y, GOLD_WIN_WIDTH, GOLD_WIN_HEIGHT );
  FIELDMAP_SetGoldMsgWin( fieldmap, msg_win );

  // 文字列を作成
  WORDSET_RegisterNumber( 
      wordset, 2, gold, GOLD_WIN_KETA, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  GFL_MSG_GetString( msg_data, msg_yen_01, *tempbuf );
  WORDSET_ExpandStr( wordset, strbuf, *tempbuf );
  FLDMSGWIN_PrintStrBuf( msg_win, 0, 0, strbuf );
  GFL_STR_DeleteBuffer( strbuf );
  GFL_MSG_Delete( msg_data );

  OBATA_Printf( "EvCmdGoldWinOpen\n" );
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
  // 再表示する
  return EvCmdGoldWinOpen( core, wk );
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
  OBATA_Printf( "EvCmdGoldWinClose\n" );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  吹き出しウィンドウ
//======================================================================
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

//--------------------------------------------------------------
/**
 * 座標更新
 * @param
 * @retval
 */
//--------------------------------------------------------------
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
  
  if( bwin_work->up_down == FLDTALKMSGWIN_IDX_LOWER ){
    offs = &data_balloonWinOffset_Down[dir];
  }
   
  bwin_work->tail_pos.x += offs->x;
  bwin_work->tail_pos.y += offs->y;
  bwin_work->tail_pos.z += offs->z;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウ描画
 * @param work  SCRCMD_WORK
 * @param objID 吹き出しを出すOBJID
 * @param arcID 表示するメッセージのアーカイブ指定ID
 * @param msgID 表示するメッセージID
 * @param idx  FLDTALKMSGWIN_IDX 
 * @retval BOOL TRUE=表示 FALSE=エラー
 * @note arcID=0x400の場合、デフォルトのメッセージアーカイブを使用する
 */
//--------------------------------------------------------------
static BOOL balloonWin_Write( SCRCMD_WORK *work,
    u16 objID, u16 arcID, u16 msgID, FLDTALKMSGWIN_IDX idx,
    TALKMSGWIN_TYPE type )
{
  STRBUF **msgbuf;
  SCRCMD_BALLOONWIN_WORK *bwin_work;
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
  
  //既にビットが立っていたら新しく作らない
  if ( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) ){
#ifdef SCR_ASSERT_ON     
    GF_ASSERT_MSG(0,"既にバルーンウィンドウがある\n");
#else
    OS_Printf("====既にバルーンウィンドウがある====\n");
#endif
    return ( FALSE );
  }
  
  bwin_work = SCRCMD_WORK_GetBalloonWinWork( work );
  bwin_work->obj_id = objID;

  msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  
  {
    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );

    if (arcID == 0x400){
      GFL_MSG_GetString( msgData, msgID, *tmpbuf );
    }else{
      msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
          arcID, SCRCMD_WORK_GetHeapID( work ) );
      GFL_MSG_GetString( msgData, msgID, *tmpbuf );
      GFL_MSG_Delete( msgData );
    }

    WORDSET_ExpandStr( wordset, *msgbuf, *tmpbuf );
  }
  
  if( idx == FLDTALKMSGWIN_IDX_AUTO ){
    idx = FLDTALKMSGWIN_IDX_LOWER;
    
    {
      VecFx32 pos,jiki_pos;
      MMDL_GetVectorPos( npc, &pos );
      MMDL_GetVectorPos( jiki, &jiki_pos );
      
      if( pos.z < jiki_pos.z ){
        idx = FLDTALKMSGWIN_IDX_UPPER;
      }
    }
  }
  
  bwin_work->up_down = idx;
  balloonWin_UpdatePos( work );
  
  setBalloonWindow( work,
      fparam->msgBG, idx, &bwin_work->tail_pos, *msgbuf, type );
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
  
  balloonWin_UpdatePos( work );
  
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
  u16 win_idx = SCRCMD_GetVMWorkValue( core, work );
  TALKMSGWIN_TYPE type = SCRCMD_GetVMWorkValue( core, work );
  
  KAGAYA_Printf( "吹き出しウィンドウ OBJID =%d\n", obj_id );
  
  if( balloonWin_Write(work,obj_id,arc_id,msg_id,win_idx,type) == TRUE ){
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
  MMDL **mmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  u16 arc_id = SCRCMD_GetVMWorkValue( core, work );
  u16 msg_id = SCRCMD_GetVMWorkValue( core, work );
  u16 win_idx = SCRCMD_GetVMWorkValue( core, work );
  TALKMSGWIN_TYPE type = SCRCMD_GetVMWorkValue( core, work );

  if( *mmdl == NULL ){
    OS_Printf( "スクリプトエラー 話し掛け対象のOBJが居ません\n" );
    return VMCMD_RESULT_SUSPEND;
  }

  {
    u8 obj_id = MMDL_GetOBJID( *mmdl );
    
    if( balloonWin_Write(work,obj_id,arc_id,msg_id,win_idx,type) == TRUE ){
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
  u16 win_idx = SCRCMD_GetVMWorkValue( core, work );
  TALKMSGWIN_TYPE type = SCRCMD_GetVMWorkValue( core, work );

  {
    GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
    PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
    u32 sex = MyStatus_GetMySex( &player->mystatus );
    
    if( sex != PM_MALE ){
      msg_id = msg_id_f;
    }
  }
  
  if( balloonWin_Write(work,obj_id,arc_id,msg_id,win_idx,type) == TRUE ){
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
  u16 win_idx = SCRCMD_GetVMWorkValue( core, work );
  TALKMSGWIN_TYPE type = SCRCMD_GetVMWorkValue( core, work );

  if( PM_VERSION == VERSION_BLACK ){
    msg_id = msg_id_b;
  }
  
  if( balloonWin_Write(work,obj_id,arc_id,msg_id,win_idx,type) == TRUE ){
    VMCMD_SetWait( core, BallonWinMsgWait );
    return VMCMD_RESULT_SUSPEND;
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 吹き出しウィンドウを閉じる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBalloonWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  CloseBalloonWin(core,work);       //<<内部で既閉チェックをしています
  return VMCMD_RESULT_SUSPEND;
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
  
  WORDSET *wordset = SCRIPT_GetWordSet( sc );
  STRBUF **msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
  GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
  u8 *msg_index      = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGINDEX );
  
  KAGAYA_Printf( "TR ID =%d, KIND ID =%d\n", tr_id, kind_id );

  //吹きだしウィンドウ既開チェック開いていたら処理を行わない
  if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) )
  {
    #ifdef SCR_ASSERT_ON     
    GF_ASSERT_MSG(0,"TR_MSG_ERROR::既にバルーンウィンドウがある\n");
    #else
    OS_Printf("===TR_MSG_ERROR::既にバルーンウィンドウがある===\n");
    #endif
    return VMCMD_RESULT_SUSPEND;
  }
  
  TT_TrainerMessageGet(
      tr_id, kind_id, *msgbuf, SCRCMD_WORK_GetHeapID(work) );
  
  { //トレーナーOBJを探す
    u16 dir;
    s16 gx,gy,gz;
    MMDL *jiki,*ncp;
    MMDLSYS *mmdlsys;
    FIELD_PLAYER *fld_player;
    SCRCMD_BALLOONWIN_WORK *bwin_work;
    
    bwin_work = SCRCMD_WORK_GetBalloonWinWork( work );
    
    mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
    fld_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
    jiki = FIELD_PLAYER_GetMMdl( fld_player );
    dir = MMDL_GetDirDisp( jiki );

    FIELD_PLAYER_GetFrontGridPos( fld_player, &gx, &gy, &gz );
    ncp = MMDLSYS_SearchGridPos( mmdlsys, gx, gz, FALSE );
    
    if( ncp == NULL ){ //不明
      GF_ASSERT( 0 );
      ncp = jiki;
    }
    
    bwin_work->obj_id = MMDL_GetOBJID( ncp );
    
    {
      FLDTALKMSGWIN_IDX idx = FLDTALKMSGWIN_IDX_LOWER;
      
      if( dir == DIR_UP ){
        idx = FLDTALKMSGWIN_IDX_UPPER;
      }
      
      bwin_work->up_down = idx;
      balloonWin_UpdatePos( work );
      
      setBalloonWindow( work, fparam->msgBG,
          idx, &bwin_work->tail_pos, *msgbuf, TALKMSGWIN_TYPE_NORMAL );
      
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
 * 
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void ClosePlainWin( SCRCMD_WORK *work )
{
  if( SCREND_CHK_CheckBit(SCREND_CHK_PLAINWIN_OPEN) ){
    FLDPLAINMSGWIN *win = (FLDPLAINMSGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
    FLDPLAINMSGWIN_Delete( win );
    SCREND_CHK_SetBitOff(SCREND_CHK_PLAINWIN_OPEN);
  }else{
    GF_ASSERT( 0 );
  }
}

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
  
  if( FLDPLAINMSGWIN_Print(plnWin) == TRUE ){
    return( 1 );
  }
  
  return( 0 );
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
  STRBUF *msgbuf;
  FLDPLAINMSGWIN *win;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  u16 msg_id = VMGetU16( core );
  u8 up_down = VMGetU8( core );
  
  msgbuf = SetExpandWord( work, sc, msg_id );
  win = FLDPLAINMSGWIN_AddStrBuf( fparam->msgBG, up_down, msgbuf );
  SCRCMD_WORK_SetMsgWinPtr( work, win );
  
  SCREND_CHK_SetBitOn(SCREND_CHK_PLAINWIN_OPEN);
  VMCMD_SetWait( core, PlainWinMsgWait );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * プレーンウィンドウ　閉じる
 * @param core  仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlainWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  ClosePlainWin( work );
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
  ClosePlainWin( end_check->ScrCmdWk );
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
  u8 x = VMGetU8( core );
  u8 y = VMGetU8( core );
  u16 win_id = VMGetU16( core );
  u16 sx,cx;
  u8 cy = 2;
  
  {
    HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
    STRBUF *msgBuf = GFL_STR_CreateBuffer( FLDMSGBG_STRLEN_SUBWIN, heapID );
    
    GFL_MSGDATA *msgData = SCRCMD_WORK_GetMsgData( work );
    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
    
    GFL_MSG_GetString( msgData, msg_id, *tmpbuf );
    WORDSET_ExpandStr( wordset, msgBuf, *tmpbuf );
    
    sx = GFL_STR_GetBufferLength( msgBuf );
    sx = sx * 12;
    cx = sx / 8;
    if( (sx&0x07) ){ cx++; }
    
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
  
  SCREND_CHK_SetBitOn( SCREND_CHK_WIN_OPEN );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * サブウィンドウ　閉じる
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
    SCREND_CHK_SetBitOff( SCREND_CHK_WIN_OPEN );
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * サブウィンドウ　全て閉じる
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
  STRBUF **msgBuf;
  
  msgBuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  bgWin = (FLDBGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
  
  if( FLDBGWIN_PrintStrBuf(bgWin,*msgBuf) == TRUE ){
    return( 1 );
  }
  
  return( 0 );
}

//--------------------------------------------------------------
/**
 * BGウィンドウ閉じる
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void CloseBGWin( SCRCMD_WORK *work )
{
  if( SCREND_CHK_CheckBit(SCREND_CHK_BGWIN_OPEN) ){
    FLDBGWIN *bgWin;
    bgWin = (FLDBGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
    FLDBGWIN_Delete( bgWin );
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
  
  { //仮
    SCRIPT_FLDPARAM *fparam;
    GFL_MSGDATA *msgData;
    FLDBGWIN *bgWin;
    
    fparam = SCRIPT_GetFieldParam( sc );
    msgData = SCRCMD_WORK_GetMsgData( work );
    
	  bgWin = FLDBGWIN_Add( fparam->msgBG, type );
    SCRCMD_WORK_SetMsgWinPtr( work, bgWin );
    
    {
      STRBUF **msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
      WORDSET *wordset = SCRIPT_GetWordSet( sc );
      STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
      GFL_MSG_GetString( msgData, msg_id, *tmpbuf );
      WORDSET_ExpandStr( wordset, *msgbuf, *tmpbuf );
      
      VMCMD_SetWait( core, BGWinMsgWait );
    }
  }
  
  SCREND_CHK_SetBitOn( SCREND_CHK_BGWIN_OPEN);
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BGウィンドウ　閉じる
 * @param
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBGWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  CloseBGWin( work );
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
  CloseBGWin( end_check->ScrCmdWk );
  return( TRUE );
}

//======================================================================
//  特殊ウィンドウ
//======================================================================
//--------------------------------------------------------------
/**
 * 特殊ウィンドウ　閉じる
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void CloseSpWin( SCRCMD_WORK *work )
{
  if( SCREND_CHK_CheckBit(SCREND_CHK_SPWIN_OPEN) ){
    FLDSPWIN *spWin = SCRCMD_WORK_GetMsgWinPtr( work );
    FLDSPWIN_Delete( spWin );
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
 * 特殊ウィンドウ閉じる
 * @param
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSpWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  CloseSpWin( work );
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
  CloseSpWin( end_check->ScrCmdWk );
  return( TRUE );
}

//======================================================================
//  その他メッセージウィンドウ系コマンド
//======================================================================
//--------------------------------------------------------------
/**
 * メッセージウィンドウ共通　メッセージウィンドウ閉じる
 * @param 
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMsgWinClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_WIN_OPEN) ){
    CloseWin( work );
  }
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_PLAINWIN_OPEN) ){
    ClosePlainWin( work );
  }
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_BGWIN_OPEN) ){
    CloseBGWin( work );
  }
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_SPWIN_OPEN) ){
    CloseSpWin( work );
  }

  if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) ){
   CloseBalloonWin( core, work );
  }
 
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 *  ウィンドウ終了
 * @param   work    スクリプトコマンドワークポインタ
 * @retval  none
 */
//--------------------------------------------------------------
static void CloseWin( SCRCMD_WORK *work )
{
  FLDSYSWIN_STREAM *sysWin;
  
  if( SCREND_CHK_CheckBit(SCREND_CHK_WIN_OPEN) )
  {
    sysWin = (FLDSYSWIN_STREAM*)SCRCMD_WORK_GetMsgWinPtr( work );
    FLDSYSWIN_STREAM_Delete( sysWin );
    SCREND_CHK_SetBitOff(SCREND_CHK_WIN_OPEN);
  }
  else
  {
    #ifdef SCR_ASSERT_ON
    GF_ASSERT_MSG( 0,
      "TALK_CLOSE_ERROR::存在しないトークウィンドウを閉じようとしている");
    #else
    OS_Printf(
      "TALK_CLOSE_ERROR::存在しないトークウィンドウを閉じようとしている\n");
    #endif
  }
}

//--------------------------------------------------------------
/**
 * バルーンウィンドウクローズ待ち
 * @param core  VMHANDLE*
 * @param wk
 * @retval BOOL
 */
//--------------------------------------------------------------
static BOOL EvWaitCloseBalloonWin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  FLDTALKMSGWIN *tmsg;
  tmsg = (FLDTALKMSGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );

  if( FLDTALKMSGWIN_WaitClose(tmsg) == TRUE ){
    SCREND_CHK_SetBitOff(SCREND_CHK_BALLON_WIN_OPEN);
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 *  バルーンウィンドウ閉じる
 * @param   work    スクリプトコマンドワークポインタ
 * @retval  none
 * @note 終了待ちのWait処理をVMHANDLEにセットしています。
 */
//--------------------------------------------------------------
static void CloseBalloonWin(VMHANDLE *core, SCRCMD_WORK *work)
{
  if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) ){
    FLDTALKMSGWIN *tmsg;
    tmsg = (FLDTALKMSGWIN*)SCRCMD_WORK_GetMsgWinPtr( work );
    FLDTALKMSGWIN_StartClose( tmsg );
    VMCMD_SetWait( core, EvWaitCloseBalloonWin );
  }else{
    #ifdef SCR_ASSERT_ON    
    GF_ASSERT_MSG(0,"バルーンウィンドウはありません");
    #else
    OS_Printf("==========バルーンウィンドウはありません============\n");
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
    SCREND_CHK_SetBitOff(SCREND_CHK_BALLON_WIN_OPEN);
  }else{
    #ifdef SCR_ASSERT_ON    
    GF_ASSERT_MSG(0,"バルーンウィンドウはありません");
    #else
    OS_Printf("==========バルーンウィンドウはありません============\n");
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
  const VecFx32 *pos, STRBUF *strBuf, TALKMSGWIN_TYPE type )
{
  FLDTALKMSGWIN *tmsg;
  
  tmsg = FLDTALKMSGWIN_AddStrBuf( fmb, idx, pos, strBuf, type );
  SCRCMD_WORK_SetMsgWinPtr( work, tmsg );
  //ウィンドウ作成フラグオン
  SCREND_CHK_SetBitOn(SCREND_CHK_BALLON_WIN_OPEN);
}

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
  STRBUF **msgbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_MSGBUF );
  WORDSET *wordset = SCRIPT_GetWordSet( sc );
  STRBUF **tmpbuf = SCRIPT_GetMemberWork( sc, ID_EVSCR_TMPBUF );
  
  GFL_MSG_GetString( msgData, msg_id, *tmpbuf );
  WORDSET_ExpandStr( wordset, *msgbuf, *tmpbuf );
  return( *msgbuf );
}
