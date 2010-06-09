#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『MMDL LIST』
 * @file   event_debug_menu_mmdl_list.c
 * @author obata
 * @date   2010.05.06
 *
 * ※event_debug_menu.c より移動
 */
///////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "field/fieldmap_proc.h"
#include "field/field_msgbg.h"
#include "debug/debug_str_conv.h"
#include "event_debug_local.h"

#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"

#include "field/fldmmdl.h"
#include "event_debug_menu_mmdl_list.h"

#include "fieldmap.h"
#include "field_player.h"

//======================================================================
//  デバッグメニュー　動作モデル一覧
//======================================================================
//--------------------------------------------------------------
/// DEBUG_MMDL_LIST_EVENT_WORK 動作モデルリスト処理用ワーク
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
  MMDLSYS *fldmmdlsys;

  u16 obj_code;
  u8 padding[2];
  MMDL *fmmdl;
}DEBUG_MMDLLIST_EVENT_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMMdlListEvent(
    GMEVENT *event, int *seq, void *wk );
static u16 * DEBUG_GetOBJCodeStrBuf( HEAPID heapID, u16 code );
static void DEBUG_SetMenuWorkMMdlList(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );

/// 動作モデルリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_MMdlList =
{
  1,    //リスト項目数
  9,   //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
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

static const DEBUG_MENU_INITIALIZER DebugMMdlListData = {
  NARC_debug_message_d_field_dat,
  OBJCODETOTAL,
  NULL,
  &DATA_DebugMenuList_MMdlList,
  1, 1, 11, 16,
  DEBUG_SetMenuWorkMMdlList,
  NULL,
};


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * イベント：MMDL LIST
 * @param gsys
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @return GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu_MmdlList( GAMESYS_WORK *gsys, void* wk )
{
  DEBUG_MENU_EVENT_WORK* debug_work = wk;
  GMEVENT*               event      = debug_work->gmEvent;
  HEAPID                 heapID     = debug_work->heapID;
  FIELDMAP_WORK*         fieldWork  = debug_work->fieldWork;
  DEBUG_MMDLLIST_EVENT_WORK *work;

  event = GMEVENT_Create( gsys, NULL,
    debugMenuMMdlListEvent, sizeof(DEBUG_MMDLLIST_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MMDLLIST_EVENT_WORK) );

  work->gmSys     = gsys;
  work->gmEvent   = event;
  work->heapID    = heapID;
  work->fieldWork = fieldWork;

  {
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    work->fldmmdlsys = GAMEDATA_GetMMdlSys( gdata );
  }

  return event;
}

//--------------------------------------------------------------
/**
 * イベント：動作モデル一覧
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMMdlListEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MMDLLIST_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugMMdlListData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      if( ret == FLDMENUFUNC_CANCEL || ret == NONDRAW ){ //キャンセル
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        return( GMEVENT_RES_FINISH );
      }

      work->obj_code = ret;

      {
        MMDL *jiki;
        MMDL_HEADER head = {
          0,  ///<識別ID
          0,  ///<表示するOBJコード
          MV_DMY, ///<動作コード
          0,  ///<イベントタイプ
          0,  ///<イベントフラグ
          0,  ///<イベントID
          DIR_DOWN,  ///<指定方向
          0,  ///<指定パラメタ 0
          0,  ///<指定パラメタ 1
          0,  ///<指定パラメタ 2
          4,  ///<X方向移動制限
          4,  ///<Z方向移動制限
          0,  ///<グリッドX
          0,  ///<グリッドZ
          0,  ///<Y値 fx32型
        };

        jiki = MMDLSYS_SearchOBJID(
          work->fldmmdlsys, MMDL_ID_PLAYER );

        head.id = 250;

        MMDLHEADER_SetGridPos( &head,
            MMDL_GetGridPosX( jiki ) + 2,
            MMDL_GetGridPosZ( jiki ),
            MMDL_GetVectorPosY( jiki ) );

        head.obj_code = work->obj_code;

        work->fmmdl = MMDLSYS_AddMMdl(
          work->fldmmdlsys, &head, 0 );
      }

      (*seq)++;
      break;
    case 2:
      {
        MMDL *mmdl = work->fmmdl;
        int key_cont = GFL_UI_KEY_GetCont();
        u16 code = MMDL_GetOBJCode( mmdl );
        u16 ac = ACMD_NOT;
        
        if( MMDL_CheckMoveBitAcmd(mmdl) ){
          if( MMDL_CheckEndAcmd(mmdl) == TRUE ){
            MMDL_EndAcmd( mmdl );
          }
        }
        
        if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
          if( (key_cont & PAD_BUTTON_B) ){
            MMDL_Delete( work->fmmdl );
            (*seq) = 1;
            return GMEVENT_RES_CONTINUE;
          }

          if( code == SHIN_A || code == MU_A ){
            if( key_cont & PAD_KEY_UP ){
              ac = AC_SHIN_MU_FLY_L;
            }else if( key_cont & PAD_KEY_DOWN ){
              ac = AC_DIR_D;
            }else if( key_cont & PAD_KEY_LEFT ){
              ac = AC_SHIN_MU_GUTARI;
            }else if( key_cont & PAD_KEY_RIGHT ){
              ac = AC_SHIN_MU_FLY_UPPER;
            }

            if( ac != ACMD_NOT ){
              VecFx32 offs = {0,0,0};
              MMDL_SetVectorDrawOffsetPos( mmdl, &offs );
            }
          }else if( code == SHIN_B || code == MU_B ){
            if( key_cont & PAD_KEY_UP ){
              ac = AC_DIR_U;
            }else if( key_cont & PAD_KEY_DOWN ){
              ac = AC_DIR_D;
            }else if( key_cont & PAD_KEY_LEFT ){
              ac = AC_SHIN_MU_HOERU;
            }else if( key_cont & PAD_KEY_RIGHT ){
              ac = AC_SHIN_MU_TURN;
            }
          }else if( code == SHIN_C || code == MU_C ){
            if( key_cont & PAD_KEY_UP ){
              ac = AC_SHIN_MU_C_FLY;
            }
          }else{
            u16 ac_act_u = ACMD_NOT;
            u16 ac_act_d = ACMD_NOT;
            u16 ac_act_l = ACMD_NOT;
            u16 ac_act_r = ACMD_NOT;
            
            const OBJCODE_PARAM *param = MMDL_GetOBJCodeParam( mmdl );

            if( key_cont & PAD_BUTTON_A ){
              switch( param->anm_id ){
              case MMDL_BLACT_ANMTBLNO_PCWOMAN:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_PC_BOW;
                break;
              case MMDL_BLACT_ANMTBLNO_RIVEL:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_RIVEL_ACT0;
                break;
              case MMDL_BLACT_ANMTBLNO_SUPPORT:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_SUPPORT_ACT0;
                break;
              case MMDL_BLACT_ANMTBLNO_BOSS_N:
                ac_act_u = AC_N_ACT0;
                ac_act_d = AC_N_ACT1;
                ac_act_l = ac_act_r = AC_N_ACT2;
                break;
              case MMDL_BLACT_ANMTBLNO_SAGE1:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_SAGE1_ACT0;
                break;
              case MMDL_BLACT_ANMTBLNO_NINJA:
                ac_act_u = ac_act_d = AC_NINJA_ACT0;
                ac_act_l = ac_act_r = AC_NINJA_ACT1;
                break;
              case MMDL_BLACT_ANMTBLNO_CHAMPION:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_CHAMPION_ACT0;
                break;
              }
            }
            
            if( key_cont & PAD_KEY_UP ){
              if( ac_act_u != ACMD_NOT ){
                ac = ac_act_u;
              }else{
                ac = AC_STAY_WALK_U_32F;
              }
            }else if( key_cont & PAD_KEY_DOWN ){
              if( ac_act_d != ACMD_NOT ){
                ac = ac_act_d;
              }else{
                ac = AC_STAY_WALK_D_32F;
              }
            }else if( key_cont & PAD_KEY_LEFT ){
              if( ac_act_l != ACMD_NOT ){
                ac = ac_act_l;
              }else{
                ac = AC_STAY_WALK_L_32F;
              }
            }else if( key_cont & PAD_KEY_RIGHT ){
              if( ac_act_r != ACMD_NOT ){
                ac = ac_act_r;
              }else{
                ac = AC_STAY_WALK_R_32F;
              }
            }
          }

          if( ac != ACMD_NOT ){
            MMDL_SetAcmd( mmdl, ac );
            (*seq)++;
          }
        }
      }
      break;
    case 3:
      if( MMDL_EndAcmd( work->fmmdl )){
        (*seq) = 2;
      }
      break;
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * 動作モデルOBJコード->STRCODE
 * @param heapID  文字列バッファ確保用ヒープID
 * @param code  文字列を取得したいOBJコード
 * @retval  u16*  文字列が格納されたu16*(開放が必要
 */
//--------------------------------------------------------------
static u16 * DEBUG_GetOBJCodeStrBuf( HEAPID heapID, u16 code )
{
  int i;
  u16 utf16,utf16_eom;
  u16 *pStrBuf;
  u8 *name8;

  pStrBuf = GFL_HEAP_AllocClearMemory( heapID,
      sizeof(u16)*DEBUG_OBJCODE_STR_LENGTH );
  name8 = DEBUG_MMDL_GetOBJCodeString( code, heapID );
  //DEBUG_ConvertAsciiToUTF16( name8, DEBUG_OBJCODE_STR_LENGTH, pStrBuf );
  DEB_STR_CONV_SjisToStrcode( (const char *)name8, pStrBuf, DEBUG_OBJCODE_STR_LENGTH );
  GFL_HEAP_FreeMemory( name8 );

  return( pStrBuf );
}

//--------------------------------------------------------------
/**
 * 動作モデルリスト用BMP_MENULIST_DATAセット
 * @param list  セット先BMP_MENULIST_DATA
 * @param heapID  文字列バッファ確保用HEAPID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkMMdlList(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  u16 *str;
  int id,max;
  STRBUF *strBuf = GFL_STR_CreateBuffer( DEBUG_OBJCODE_STR_LENGTH, heapID );

  for( id = OBJCODESTART_BBD; id < OBJCODEEND_BBD; id++ ){
    GFL_STR_ClearBuffer( strBuf );
    str = DEBUG_GetOBJCodeStrBuf( heapID, id );
    GFL_STR_SetStringCode( strBuf, str );
    GFL_HEAP_FreeMemory( str );
    FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
  }

  for( id = OBJCODESTART_TPOKE; id < OBJCODEEND_TPOKE; id++ ){
    GFL_STR_ClearBuffer( strBuf );
    str = DEBUG_GetOBJCodeStrBuf( heapID, id );
    GFL_STR_SetStringCode( strBuf, str );
    GFL_HEAP_FreeMemory( str );
    FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
  }

  for( id = OBJCODESTART_MDL; id < OBJCODEEND_MDL; id++ ){
    GFL_STR_ClearBuffer( strBuf );
    str = DEBUG_GetOBJCodeStrBuf( heapID, id );
    GFL_STR_SetStringCode( strBuf, str );
    GFL_HEAP_FreeMemory( str );
    FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
  }

  GFL_HEAP_FreeMemory( strBuf );
}

//======================================================================
//======================================================================
#include "item/itemsym.h" 
#include "field_comm/mission_ng_check.cdat"

static GMEVENT_RESULT debugMenuDisguiseListEvent(
    GMEVENT *event, int *seq, void *wk );

static void DEBUG_SetMenuWorkDisguiseList(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );

static const DEBUG_MENU_INITIALIZER DebugDisguiseListData = {
  NARC_debug_message_d_field_dat,
  NELEMS(IntrudeOBJID_List),
  NULL,
  &DATA_DebugMenuList_MMdlList,
  1, 1, 11, 16,
  DEBUG_SetMenuWorkDisguiseList,
  NULL,
};

//--------------------------------------------------------------
/**
 * イベント：MMDL LIST
 * @param gsys
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @return GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu_DisguiseList( GAMESYS_WORK *gsys, void* wk )
{
  DEBUG_MENU_EVENT_WORK* debug_work = wk;
  GMEVENT*               event      = debug_work->gmEvent;
  HEAPID                 heapID     = debug_work->heapID;
  FIELDMAP_WORK*         fieldWork  = debug_work->fieldWork;
  DEBUG_MMDLLIST_EVENT_WORK *work;

  event = GMEVENT_Create( gsys, NULL,
    debugMenuDisguiseListEvent, sizeof(DEBUG_MMDLLIST_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MMDLLIST_EVENT_WORK) );

  work->gmSys     = gsys;
  work->gmEvent   = event;
  work->heapID    = heapID;
  work->fieldWork = fieldWork;

  {
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    work->fldmmdlsys = GAMEDATA_GetMMdlSys( gdata );
  }

  return event;
}

//--------------------------------------------------------------
/**
 * イベント：動作モデル一覧
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuDisguiseListEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MMDLLIST_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugDisguiseListData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      if( ret != FLDMENUFUNC_CANCEL ){
        //選択した
        work->obj_code = ret;
      } else {
        //キャンセル
        MYSTATUS * myst = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(work->gmSys));
        if ( MyStatus_GetMySex(myst) == PM_MALE ) {
          work->obj_code = HERO;
        } else {
          work->obj_code = HEROINE;
        }
      }

      {
        const VecFx32 offset_pos = {0,0,0};
        FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( work->fieldWork );

        FIELD_PLAYER_ChangeOBJCode( fld_player, work->obj_code );
        //連れ歩きポケモンはdraw_offsetをずらしているので変身後は初期化する
        MMDL_SetVectorDrawOffsetPos( FIELD_PLAYER_GetMMdl(fld_player), &offset_pos );
        if( work->obj_code == HERO || work->obj_code == HEROINE ) {
          FIELD_PLAYER_ClearOBJCodeFix( fld_player );
        }
      }
      (*seq) ++;
      break;

    case 2:
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        return( GMEVENT_RES_FINISH );
      break;
    }
  }

  return( GMEVENT_RES_CONTINUE );
}


//--------------------------------------------------------------
/**
 * 動作モデルリスト用BMP_MENULIST_DATAセット
 * @param list  セット先BMP_MENULIST_DATA
 * @param heapID  文字列バッファ確保用HEAPID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkDisguiseList(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  u16 *str;
  int i, id,max;
  STRBUF *strBuf = GFL_STR_CreateBuffer( DEBUG_OBJCODE_STR_LENGTH, heapID );

  for( i = 0; i < NELEMS(IntrudeOBJID_List); i++ ){
    id = IntrudeOBJID_List[i];
    GFL_STR_ClearBuffer( strBuf );
    str = DEBUG_GetOBJCodeStrBuf( heapID, id );
    GFL_STR_SetStringCode( strBuf, str );
    GFL_HEAP_FreeMemory( str );
    FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
  }

  GFL_HEAP_FreeMemory( strBuf );
}


#endif // PM_DEBUG 
