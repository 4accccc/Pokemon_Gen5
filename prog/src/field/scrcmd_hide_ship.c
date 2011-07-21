//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_hide_ship.c
 *	@brief  アジト船スクリプトコマンド
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "gflib.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

// app
#include "app/name_input.h"

// script
#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

// field
#include "fieldmap.h"
#include "event_fieldmap_control.h"

// arc
#include "arc/message.naix" 
#include "msg/msg_hide_ship_password.h"

// mine
#include "scrcmd_hide_ship.h"
#include "field_gimmick_hide_ship_engine.h"

// debug
#include "debug/debug_str_conv.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define PASSWORD_NUM    ( msg_hide_ship_password_max )     // パスワード答えの数

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	パスワード入力イベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK* gsys;
  GAMEDATA*     gdata;
  MISC          *p_misc;
  MYSTATUS      *p_mystatus;
  FIELDMAP_WORK *p_fieldmap;

  u16           *ret_wk;   ///<入力を確定したかどうかを返すワーク
  NAMEIN_PARAM  *p_namein_param;
 
  HEAPID        heapID;
  
} PASSWORD_INPUT_EVENT_WORK;

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// パスワード入力イベント
static GMEVENT* PASSWORD_INPUT_CreateEvent( GAMESYS_WORK* gsys, u16* ret_wk );
// パスワード入力イベント関数
static GMEVENT_RESULT PasswordInputEvent(GMEVENT * event, int * seq, void *work);
// 入力処理
static void result_proc( PASSWORD_INPUT_EVENT_WORK *p_wk, NAMEIN_PARAM* param );

// トレーナーIDからパスワードIDを求める
static u8 password_id_get(const MYSTATUS * p_mystatus);

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン押下
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdHideShip_Button( VMHANDLE *core, void *wk )
{
  int id;
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // ボタンID
  id  = VMGetU16( core );

  event = HIDE_SHIP_Button( gsys, id );

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゲートの状態セット
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdHideShip_GateSet( VMHANDLE *core, void *wk )
{
  int id;
  int state;
  BOOL  flag;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // ゲートID
  id  = VMGetU16( core );
  // 解除しているか
  state  = SCRCMD_GetVMWorkValue( core, work );
  if( state == 1 ){ flag = FALSE; }   // 解除した
  else            { flag = TRUE;  }   // 解除していない

  HIDE_SHIP_GateSet( gsys, id, flag );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゲート解除アニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdHideShip_GateOff( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // ゲートID
  id  = VMGetU16( core );

  HIDE_SHIP_GateOff( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  パスワード入力
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdHideShip_PasswordInput( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // 戻り値
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  // パスワード入力イベント
  event = PASSWORD_INPUT_CreateEvent( gsys, ret_wk );

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  パスワードID取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdHideShip_PasswordIdGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA    *gdata = GAMESYSTEM_GetGameData( gsys );
  MYSTATUS    *mystatus = GAMEDATA_GetMyStatus( gdata );

  // 戻り値
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = password_id_get( mystatus );

  return VMCMD_RESULT_CONTINUE;
}


//=============================================================================
/**
 *					static関数
*/
//=============================================================================

// パスワード入力イベント
static GMEVENT* PASSWORD_INPUT_CreateEvent( GAMESYS_WORK* gsys, u16* ret_wk )
{
  GMEVENT* event = GMEVENT_Create(gsys, NULL, PasswordInputEvent, sizeof(PASSWORD_INPUT_EVENT_WORK));
	PASSWORD_INPUT_EVENT_WORK *p_wk = GMEVENT_GetEventWork( event );

  p_wk->gsys        = gsys;
  p_wk->gdata       = GAMESYSTEM_GetGameData( gsys );
  p_wk->p_fieldmap  = GAMESYSTEM_GetFieldMapWork( gsys );
  p_wk->ret_wk      = ret_wk;
  p_wk->heapID      = HEAPID_PROC;

  p_wk->p_misc      = SaveData_GetMisc( GAMEDATA_GetSaveControlWork( p_wk->gdata ) );
  p_wk->p_mystatus  = GAMEDATA_GetMyStatus( p_wk->gdata );

  // 名前入力パラメータ初期化
  {
    p_wk->p_namein_param = NAMEIN_AllocParam( 
        HEAPID_PROC,
        NAMEIN_FREE_PASSWORD,
        0, 0,
        NAMEIN_FREE_WORD_LENGTH,
        NULL, p_wk->p_misc );
  }

  return event;
}

// パスワード入力イベント関数
static GMEVENT_RESULT PasswordInputEvent(GMEVENT * event, int * seq, void *work)
{
	PASSWORD_INPUT_EVENT_WORK *p_wk = work;

  switch( *seq ){
  case 0:
    {
      GMEVENT* child = 
        EVENT_FieldSubProc( p_wk->gsys, p_wk->p_fieldmap,
          FS_OVERLAY_ID(namein), &NameInputProcData, p_wk->p_namein_param );
      GMEVENT_CallEvent( event, child );
    }
    (*seq)++;
    break;
  case 1:
    // 結果処理
    result_proc( p_wk, p_wk->p_namein_param );

    //後始末
    NAMEIN_FreeParam( p_wk->p_namein_param );
		return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  結果処理
 */
//-----------------------------------------------------------------------------
static void result_proc( PASSWORD_INPUT_EVENT_WORK *p_wk, NAMEIN_PARAM* param )
{
  if( p_wk->ret_wk == NULL )
  {
    GF_ASSERT(0);
    return;
  }

  if( NAMEIN_IsCancel( param ) == TRUE ){  //キャンセルされた
    *(p_wk->ret_wk) = 0;
    return;
  }

  // 答え比較
  {
    STRBUF      *p_pass_str;  // パスワード文字
    GFL_MSGDATA *p_msg_data;
    u8          password_id;

    // メッセージデータ ハンドルオープン
    p_msg_data = GFL_MSG_Create( 
        GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_hide_ship_password_dat, p_wk->heapID ); 

    // パスワードの答え取得
    password_id = password_id_get( p_wk->p_mystatus );
    p_pass_str = GFL_MSG_CreateString( p_msg_data, password_id );

#ifdef PM_DEBUG
    {
      const STRCODE *cp_strcode = GFL_STR_GetStringCodePointer(param->strbuf);
      char str[NAMEIN_FREE_WORD_LENGTH] = {0};

      DEB_STR_CONV_StrcodeToSjis( cp_strcode, str, NAMEIN_FREE_WORD_LENGTH );

      ICHI_Printf("入力した文字 %s\n", str );
      ICHI_Printf("パスワードID %d \n", password_id );
    }
#endif

    // 文字列比較
    if( GFL_STR_CompareBuffer( param->strbuf, p_pass_str ) == TRUE )
    {
      *(p_wk->ret_wk) = 1;
      OS_Printf("正解!");
    }
    else
    {
      *(p_wk->ret_wk) = 0;
      OS_Printf("不正解!");
    }

    // 破棄
    GFL_STR_DeleteBuffer( p_pass_str );
    GFL_MSG_Delete( p_msg_data );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分のIDからパスワードIDを求める
 *
 *	@param	MYSTATUS * p_mystatus
 *
 *	@return パスワードID
 */
//-----------------------------------------------------------------------------
static u8 password_id_get(const MYSTATUS * p_mystatus)
{
  u32 id = MyStatus_GetID( p_mystatus );
  u8  password_id;

  password_id = (id&0xFF) % PASSWORD_NUM;

  return password_id;
}

