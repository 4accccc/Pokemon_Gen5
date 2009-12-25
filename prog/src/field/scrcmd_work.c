//======================================================================
/**
 * @file	scrcmd_work.c
 * @brief	スクリプトコマンド用ワーク
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "fieldmap.h"
#include "scrcmd_work.h"
#include "arc_def.h"
#include "script_local.h"
#include "script_def.h"

#include "script_message.naix"

//======================================================================
//	define
//======================================================================
#define SCRCMD_ACMD_MAX (8)

#define SCRCMD_MENU_LIST_MAX (16)
#define EV_WIN_B_CANCEL (0xfffe)
#define EV_WIN_X_BREAK  (0xfffd)

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
/// SCRCMD_MENU_WORK メニュー関連用ワーク
//--------------------------------------------------------------
typedef struct
{
  u16 x;
  u16 y;
  u16 cursor;
  u16 cancel;
  u16 *ret;
  
  WORDSET *wordset;

  BOOL free_msg;
  GFL_MSGDATA *msgData;
  
  FLDMENUFUNC_LISTDATA *listData;
	FLDMENUFUNC *menuFunc;

  STRBUF*   exMsgBuf[SCRCMD_MENU_LIST_MAX];
}SCRCMD_MENU_WORK;

//--------------------------------------------------------------
///	SCRCMD_WORK
//--------------------------------------------------------------
struct _TAG_SCRCMD_WORK
{
	HEAPID heapID;
  HEAPID temp_heapID;
	SCRCMD_WORK_HEADER head;
	
	GFL_MSGDATA *msgData;
  
  GFL_TCB *tcb_callproc;
	GFL_TCB *tcb_anm_tbl[SCRCMD_ACMD_MAX];
  
  SCRCMD_MENU_WORK menuWork;
  SCRCMD_BALLOONWIN_WORK balloonWinWork;
};

//======================================================================
//	proto
//======================================================================
static void	BmpMenu_CallbackFunc(BMPMENULIST_WORK * wk,u32 param,u8 mode);

//======================================================================
//	SCRCMD_WORK 初期化、削除
//======================================================================
//--------------------------------------------------------------
/**
 * SCRCMD_WORK 作成
 * @param	head	SCRCMD_WORK_HEADER
 * @param	heapID	HEAPID
 * @retval	SCRCMD_WORK
 */
//--------------------------------------------------------------
SCRCMD_WORK * SCRCMD_WORK_Create(
	const SCRCMD_WORK_HEADER *head, HEAPID heapID, HEAPID temp_heapID )
{
	SCRCMD_WORK *work;
	work = GFL_HEAP_AllocClearMemoryLo( heapID, sizeof(SCRCMD_WORK) );
	work->heapID = heapID;
  work->temp_heapID = temp_heapID;
	work->head = *head;
	return( work );
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK 削除
 * @param	work	SCRCMD_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void SCRCMD_WORK_Delete( SCRCMD_WORK *work )
{
	if( work->msgData != NULL ){
		GFL_MSG_Delete( work->msgData );
	}
	
	GFL_HEAP_FreeMemory( work );
}

//======================================================================
//	SCRCMD_WORK 参照、取得
//======================================================================
//--------------------------------------------------------------
/**
 * SCRCMD_WORK HEAPID取得
 * @param	work SCRCMD_WORK
 * @retval	GAMESYS_WORK
 */
//--------------------------------------------------------------
HEAPID SCRCMD_WORK_GetHeapID( SCRCMD_WORK *work )
{
	return( work->heapID );
}

#if 0
//--------------------------------------------------------------
/**
 * SCRCMD_WORK テンポラリ用HEAPID取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
HEAPID SCRCMD_WORK_GetTempHeapID( SCRCMD_WORK *work )
{
  return( work->temp_heapID );
}
#endif

//--------------------------------------------------------------
//--------------------------------------------------------------
u16 SCRCMD_WORK_GetZoneID( SCRCMD_WORK *work )
{
  return( work->head.zone_id );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
u16 SCRCMD_WORK_GetScriptID( SCRCMD_WORK *work )
{
  return( work->head.script_id );
}
//--------------------------------------------------------------
/**
 * SCRCMD_WORK GAMESYS_WORK取得
 * @param	work	SCRCMD_WORK
 * @retval	GAMESYS_WORK
 */
//--------------------------------------------------------------
GAMESYS_WORK * SCRCMD_WORK_GetGameSysWork( SCRCMD_WORK *work )
{
	return( work->head.gsys );
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK GAMEDATA取得
 * @param	work	SCRCMD_WORK
 * @retval	GAMESYSDATA
 */
//--------------------------------------------------------------
GAMEDATA * SCRCMD_WORK_GetGameData( SCRCMD_WORK *work )
{
	return( work->head.gdata );
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK MMDLSYS取得
 * @param	work	SCRCMD_WORK
 * @retval	MMDLSYS
 */
//--------------------------------------------------------------
MMDLSYS * SCRCMD_WORK_GetMMdlSys( SCRCMD_WORK *work )
{
	return( work->head.mmdlsys );
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK SCRIPT取得
 * @param	work	SCRCMD_WORK
 * @retval	SCRIPT
 */
//--------------------------------------------------------------
SCRIPT_WORK * SCRCMD_WORK_GetScriptWork( SCRCMD_WORK *work )
{
	return( work->head.script );
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK FLDMSGBG取得
 * @param	work	SCRCMD_WORK
 * @retval	FLDMSGBG
 */
//--------------------------------------------------------------
FLDMSGBG * SCRCMD_WORK_GetFldMsgBG( SCRCMD_WORK *work )
{
	return( work->head.fldMsgBG );
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK GFL_MSGDATAセット
 * @param	work SCRCMD_WORK
 * @retval	GFL_MSGDATA*
 */
//--------------------------------------------------------------
void SCRCMD_WORK_SetMsgData( SCRCMD_WORK *work, GFL_MSGDATA *msgData )
{
	work->msgData = msgData;
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK GFL_MSGDATA取得
 * @param	work SCRCMD_WORK
 * @retval	GFL_MSGDATA*
 */
//--------------------------------------------------------------
GFL_MSGDATA * SCRCMD_WORK_GetMsgData( SCRCMD_WORK *work )
{
	return( work->msgData );
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK メッセージウィンドウポインタセット
 * @param	work SCRCMD_WORK
 * @param	msgWin メッセージウィンドウポインタ
 * @retval	nothing
 */
//--------------------------------------------------------------
void SCRCMD_WORK_SetMsgWinPtr( SCRCMD_WORK *work, void *msgWin )
{
#if 0
	work->msgWin = msgWin;
#else
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  OS_Printf( "SC = 0x%x WORK = 0x%x\n", (u32)sc, (u32)work );
  SCRIPT_SetMsgWinPointer( sc, msgWin );
#endif
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK FLDMSGWIN_STREAM取得
 * @param	work SCRCMD_WORK
 * @retval void* メッセージウィンドウポインタ
 */
//--------------------------------------------------------------
void * SCRCMD_WORK_GetMsgWinPtr( SCRCMD_WORK *work )
{
#if 0
	return( work->msgWin );
#else
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  return SCRIPT_GetMsgWinPointer( sc  );
#endif
}


//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
SCRCMD_BALLOONWIN_WORK * SCRCMD_WORK_GetBalloonWinWork( SCRCMD_WORK *work )
{
  return( &work->balloonWinWork );
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 *
 */
//--------------------------------------------------------------
void SCRCMD_WORK_SetCallProcTCB( SCRCMD_WORK *work, GFL_TCB *tcb )
{
  work->tcb_callproc = tcb;
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 *
 */
//--------------------------------------------------------------
GFL_TCB * SCRCMD_WORK_GetCallProcTCB( SCRCMD_WORK *work )
{
  return( work->tcb_callproc );
}

//======================================================================
//  動作モデルアニメーション  
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデルアニメ用TCBをセット
 * @param work SCRCMD_WORK
 * @param tcb アニメーションコマンド用TCB
 * @retval nothing
 */
//--------------------------------------------------------------
void SCRCMD_WORK_SetMMdlAnmTCB( SCRCMD_WORK *work, GFL_TCB *tcb )
{
	int i;
	for( i = 0; i < SCRCMD_ACMD_MAX; i++ ){
		if( work->tcb_anm_tbl[i] == NULL ){
			work->tcb_anm_tbl[i] = tcb;
			return;
		}
	}
	GF_ASSERT( 0 );
}

//--------------------------------------------------------------
/**
 * 動作モデルアニメ用TCB動作チェック
 * @param work SCRCMD_WORK
 * @retval BOOL TRUE=動作中。FALSE=動作終了
 */
//--------------------------------------------------------------
BOOL SCRCMD_WORK_CheckMMdlAnmTCB( SCRCMD_WORK *work )
{
	BOOL flag = FALSE;
	int i;
	for( i = 0; i < SCRCMD_ACMD_MAX; i++ ){
		if( work->tcb_anm_tbl[i] != NULL ){
			if( MMDL_CheckEndAcmdList(work->tcb_anm_tbl[i]) == TRUE ){
				MMDL_EndAcmdList( work->tcb_anm_tbl[i] );
				work->tcb_anm_tbl[i] = NULL;
			}else{
				flag = TRUE;
			}
		}
	}
	return( flag );
}

//======================================================================
//	SCRCMD_WORK メッセージデータ
//======================================================================
//--------------------------------------------------------------
/**
 * SCRCMD_WORK MSGDATA初期化
 * @param	work	SCRCMD_WORK
 * @param	datID	アーカイブデータインデックス
 * @retval	nothing
 */
//--------------------------------------------------------------
void SCRCMD_WORK_CreateMsgData( SCRCMD_WORK *work, u32 datID )
{
#if 1
	GFL_MSGDATA *msgData = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, datID, work->heapID );
#else //テンポラリ用HEAPIDに変更。
	GFL_MSGDATA *msgData = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, datID, work->temp_heapID );
#endif
	SCRCMD_WORK_SetMsgData( work, msgData );
}

//======================================================================
//  SCRCMD_WORK メニュー関連
//======================================================================
//--------------------------------------------------------------
/**
 * SCRCMD_WORK メニュー　関連用のワーク初期化
 * @param 
 * @retval
 */
//--------------------------------------------------------------
void SCRCMD_WORK_InitMenuWork( SCRCMD_WORK *work,
  u16 x, u16 y, u16 cursor, u16 cancel, u16 *ret,
  WORDSET *wordset, GFL_MSGDATA *msgData )
{
  SCRCMD_MENU_WORK *menuWork = &work->menuWork;
	MI_CpuClear8( menuWork, sizeof(SCRCMD_MENU_WORK) );
  
  menuWork->x = x;
  menuWork->y = y;
  menuWork->cursor = cursor;
  menuWork->cancel = cancel;
  menuWork->ret = ret;
  menuWork->wordset = wordset;
  menuWork->msgData = msgData;
 
  MI_CpuClear32( menuWork->exMsgBuf, 4*SCRCMD_MENU_LIST_MAX );

  if( menuWork->msgData == NULL ){ //メニュー用共通メッセージ
    menuWork->free_msg = TRUE;
    menuWork->msgData = GFL_MSG_Create(
        GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
        NARC_script_message_ev_win_dat, work->heapID );
  }
  
  menuWork->listData = FLDMENUFUNC_CreateListData(
      SCRCMD_MENU_LIST_MAX, work->heapID );
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK メニュー　リスト追加
 * @param
 * @retval
 */
//--------------------------------------------------------------
void SCRCMD_WORK_AddMenuList(
    SCRCMD_WORK *work, u32 msg_id, u32 ex_msg_id, u32 param,
    STRBUF *msgbuf, STRBUF *tmpbuf )
{
  SCRCMD_MENU_WORK *menuWork = &work->menuWork;
  
  if(ex_msg_id != SCR_BMPMENU_EXMSG_NULL){
    u32  num = BmpMenuWork_GetListMax( (BMP_MENULIST_DATA*)menuWork->listData );
    if( num >= SCRCMD_MENU_LIST_MAX ){
      GF_ASSERT(0);
      return;
    }
    GFL_MSG_GetString( menuWork->msgData, ex_msg_id, tmpbuf );
    WORDSET_ExpandStr( menuWork->wordset, msgbuf, tmpbuf );
    menuWork->exMsgBuf[num] = GFL_STR_CreateCopyBuffer( msgbuf, work->heapID );
  }
  GFL_MSG_GetString( menuWork->msgData, msg_id, tmpbuf );
  WORDSET_ExpandStr( menuWork->wordset, msgbuf, tmpbuf );

  FLDMENUFUNC_AddStringListData(
      menuWork->listData, msgbuf, param, work->heapID );
}

//--------------------------------------------------------------
//  メニューヘッダー
//--------------------------------------------------------------
static const FLDMENUFUNC_HEADER data_MenuHeader =
{
	1,		//リスト項目数
	6,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_LRKEY,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	0,		//表示座標X キャラ単位
	0,		//表示座標Y キャラ単位
	0,		//表示サイズX キャラ単位
	0,		//表示サイズY キャラ単位
};

//--------------------------------------------------------------
/**
 * SCRCMD_WORK メニュー　開始
 * @param
 * @retval
 */
//--------------------------------------------------------------
void SCRCMD_WORK_StartMenu( SCRCMD_WORK *work )
{
  u32 sx,sy,count;
  SCRCMD_MENU_WORK *menuWork = &work->menuWork;
  FLDMENUFUNC_HEADER menuH = data_MenuHeader;

  sx = FLDMENUFUNC_GetListMenuWidth(
      menuWork->listData, menuH.font_size_x, menuH.msg_spc );

  //項目数が表示最大数を超えているかをチェックして、ウィンドウの縦幅を決定する
  {
    u32 len;
    u32 max = FLDMENUFUNC_GetListMax( menuWork->listData );
    if (max > data_MenuHeader.line) len = data_MenuHeader.line;
    else len = max;

    sy = FLDMENUFUNC_GetListMenuLen( len, menuH.font_size_y, menuH.line_spc );
  }
/**
  sy = FLDMENUFUNC_GetListMenuHeight(
      menuWork->listData, menuH.font_size_y, menuH.line_spc );
*/
  count = FLDMENUFUNC_GetListMax( menuWork->listData );
  FLDMENUFUNC_InputHeaderListSize(
      &menuH, count, menuWork->x, menuWork->y, sx, sy );

	menuWork->menuFunc = FLDMENUFUNC_AddEventMenuList(
      work->head.fldMsgBG, &menuH, menuWork->listData,
      BmpMenu_CallbackFunc, work,
      0, 0, menuWork->cancel );
}

//--------------------------------------------------------------
/**
 * @brief メニュー　消去
 * @param
 */
//--------------------------------------------------------------
static void CleanUpMenuWork( SCRCMD_MENU_WORK* menuWork )
{
  FLDMENUFUNC_DeleteMenu( menuWork->menuFunc );
  
  if( menuWork->free_msg == TRUE ){
    GFL_MSG_Delete( menuWork->msgData );
  }
  {
    int i;

    for(i = 0;i < SCRCMD_MENU_LIST_MAX;i++){
      if( menuWork->exMsgBuf[i] != NULL ){
        GFL_STR_DeleteBuffer( menuWork->exMsgBuf[i] );
        menuWork->exMsgBuf[i] = NULL;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK メニュー　メニュー処理
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL SCRCMD_WORK_ProcMenu( SCRCMD_WORK *work )
{
  u32 ret;
  SCRCMD_MENU_WORK *menuWork = &work->menuWork;
  
  ret = FLDMENUFUNC_ProcMenu( menuWork->menuFunc );

	if( ret == FLDMENUFUNC_NULL ){	//操作無し
    return( FALSE );
  }

  CleanUpMenuWork( menuWork );
  
  if( ret != FLDMENUFUNC_CANCEL ){	//決定
    *(menuWork->ret) = ret;
  }else{
#if 0
    if( menuWork->cancel == FALSE ){ //Bキャンセル有効ではない
      return( FALSE );
    }
#endif
    *(menuWork->ret) = EV_WIN_B_CANCEL;
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * SCRCMD_WORK メニュー　メニュー処理(xボタンによる中断あり)
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL SCRCMD_WORK_ProcMenu_Breakable( SCRCMD_WORK *work )
{
  u32 ret;
  SCRCMD_MENU_WORK *menuWork = &work->menuWork;

  // 通常のメニュー処理
  ret = SCRCMD_WORK_ProcMenu( work );

  // 選択されなかった場合 ==> xボタン判定
  if( ret == FALSE )
  {
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      CleanUpMenuWork( menuWork );
      *(menuWork->ret) = EV_WIN_X_BREAK;
      ret = TRUE;
    }
  }
  return ret;
}

/*
 *  @brief  メニューコールバック
 */
static void	BmpMenu_CallbackFunc(BMPMENULIST_WORK * wk,u32 param,u8 mode)
{
  u16 cursor = 0;
  SCRCMD_WORK* work = (SCRCMD_WORK*)BmpMenuList_GetWorkPtr(wk);
  SCRCMD_MENU_WORK* menu = &work->menuWork;

  BmpMenuList_DirectPosGet( wk, &cursor );

  if( menu->exMsgBuf[ cursor ] == NULL ){
    return;
  }
  if( SCREND_CHK_CheckBit(SCREND_CHK_WIN_OPEN) ) //システムウィンドウ
  {
    FLDSYSWIN_STREAM *sysWin;
    sysWin = SCRCMD_WORK_GetMsgWinPtr( work );
    FLDSYSWIN_STREAM_ClearMessage( sysWin );
    FLDSYSWIN_STREAM_AllPrintStrBuf( sysWin, 0, 0, menu->exMsgBuf[ cursor ] );
  }
  else if( SCREND_CHK_CheckBit(SCREND_CHK_BALLON_WIN_OPEN) ) //バルーンウィンドウ
  {
  }
  else if( SCREND_CHK_CheckBit(SCREND_CHK_PLAINWIN_OPEN) ) //プレーンウィンドウ
  {
  
  }else{
    GF_ASSERT(0);
  }
}


