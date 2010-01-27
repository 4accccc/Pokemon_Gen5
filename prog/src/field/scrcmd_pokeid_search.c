//======================================================================
/**
 * @file  scrcmd_pokeid_search.c
 * @brief  ポケモン親ID検索
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "savedata/battle_box_save.h"

#include "scrcmd_pokeid_search.h"


#define ID_SEARCH_MAX (50)    //最大検索保持数
#define ONCE_SEARCH_BOX_TRAY_NUM   (2)  //一度に検索するボックス数

typedef struct ARRAY_WORK_tag
{
  int NowEntryNum;
  u32 ID[ID_SEARCH_MAX];
}ARRAY_WORK;

typedef struct SEARCH_EVT_WORK_tag
{
  u32 MyID;
  int FinishBoxNum;
  u16 *RetWork;
  ARRAY_WORK Array;
}SEARCH_EVT_WORK;

static GMEVENT_RESULT SearchEvt( GMEVENT* event, int* seq, void* work );
static int SearchMyParty(const u32 inMyID, GAMEDATA * gdata, ARRAY_WORK *ioArray);
static int SearchBox(const u32 inMyID, GAMEDATA * gdata, const int inBoxIdx, ARRAY_WORK *ioArray);
static int SearchBtlBox(const u32 inMyID, GAMEDATA * gdata, const int inBoxIdx, ARRAY_WORK *ioArray);
static BOOL EntryID( const u32 inMyID, const u32 inID, ARRAY_WORK *ioArray );
static BOOL EntryIDCore(const u32 inID, ARRAY_WORK *ioArray);

//--------------------------------------------------------------
/**
 * 自分のID以外を持つポケモンの親IDの種類数を調べる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPokeID_SearchNum( VMHANDLE *core, void *wk )
{
  GMEVENT * event;
  u16 *ret;
  int size = sizeof(SEARCH_EVT_WORK);
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*    gdata = SCRCMD_WORK_GetGameData( work );

  ret = SCRCMD_GetVMWork( core, work );

  //イベントコール
  event = GMEVENT_Create( gsys, NULL, SearchEvt, size );
  {
    SEARCH_EVT_WORK *evt_work;
    MYSTATUS *status = GAMEDATA_GetMyStatus( gdata );
    evt_work = GMEVENT_GetEventWork( event );
    evt_work->MyID = MyStatus_GetID(status);
    evt_work->Array.NowEntryNum = 0;
    evt_work->FinishBoxNum = 0;
    evt_work->RetWork = ret;
    OS_Printf("MyID = %d\n",evt_work->MyID);
  }

  SCRIPT_CallEvent( sc, event ); 

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 検索イベント
 * @param   event               イベントポインタ
 * @param   seq                 シーケンサ
 * @param   work                ワークポインタ
 * @retval  GMEVENT_RESULT      イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT SearchEvt( GMEVENT* event, int* seq, void* work )  
{
  BOOL finish = FALSE;
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  GAMEDATA * gdata = GAMESYSTEM_GetGameData(gsys);
  SEARCH_EVT_WORK *evt_work = (SEARCH_EVT_WORK *)work;

  switch(*seq){
  case 0:
    //手持ちを調べる
    SearchMyParty(evt_work->MyID, gdata, &evt_work->Array);
    (*seq)++;
    break;
  case 1:
    //ボックスを調べる
    {
      int i;
      int box_idx_base = evt_work->FinishBoxNum;
      for(i=0;i<ONCE_SEARCH_BOX_TRAY_NUM;i++)
      {
        int box_idx;
        box_idx = box_idx_base + i;
        SearchBox(evt_work->MyID, gdata, box_idx, &evt_work->Array);
        evt_work->FinishBoxNum++;
        if (evt_work->FinishBoxNum >= BOX_MAX_TRAY)
        {
          (*seq)++;
          break;
        }
      }
    }
    break;
  case 2:
    //バトルボックスを調べる
    {
      int i;
      for(i=0;i<BATTLE_BOX_PARTY_NUM;i++)
      {
        int box_idx;
        box_idx = i;
        SearchBtlBox(evt_work->MyID, gdata, box_idx, &evt_work->Array);
      }
    }
    finish = TRUE;
  }

  //IDがID_SEARCH_MAX件に到達した場合は、検索途中でも終了とする
  if ( evt_work->Array.NowEntryNum >= ID_SEARCH_MAX) finish = TRUE;

  if ( finish )
  {
    *evt_work->RetWork = evt_work->Array.NowEntryNum;
    OS_Printf("Entry:%d ret:%d\n",evt_work->Array.NowEntryNum, *evt_work->RetWork);
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 手持ち検索
 * @param   inMyID    自分のID
 * @param   gdata     ゲームデータポインタ
 * @param   ioArray   ID格納ワークポインタ
 * @retval  int       格納したIDの数
 */
//--------------------------------------------------------------
static int SearchMyParty(const u32 inMyID, GAMEDATA * gdata, ARRAY_WORK *ioArray)
{
  int count;
  int i, num;
  POKEPARTY *party =  GAMEDATA_GetMyPokemon( gdata );
  num = PokeParty_GetPokeCount( party );
  //カウンタークリア
  count = 0;
  for (i=0;i<num;i++)
  {
    POKEMON_PARAM *param = PokeParty_GetMemberPointer( party, i );
    //タマゴは対象外
    if ( !PP_Get( param, ID_PARA_tamago_flag, NULL ) )
    {
      u32 id;
      //ID取得
      id = PP_Get( param, ID_PARA_id_no, NULL );
      if ( EntryID( inMyID, id, ioArray ) ) count++;
    }
  }
  OS_Printf("MyParty_DifID: %d\n",count);
  return count;
}

//--------------------------------------------------------------
/**
 * ボックス検索
 * @param   inMyID    自分のID
 * @param   gdata     ゲームデータポインタ
 * @param   inBoxIdx  検索ボックスインデックス
 * @param   ioArray   ID格納ワークポインタ
 * @retval  int       格納したIDの数
 */
//--------------------------------------------------------------
static int SearchBox(const u32 inMyID, GAMEDATA * gdata, const int inBoxIdx, ARRAY_WORK *ioArray)
{
  int i;
  int count;
  BOX_MANAGER *box = GAMEDATA_GetBoxManager(gdata);
  
  //カウンタークリア
  count = 0;
  for(i=0;i<BOX_MAX_POS;i++)
  {
    if ( BOXDAT_PokeParaGet( box, inBoxIdx, i, ID_PARA_poke_exist, NULL ) )
    {
      u32 id;
      id = BOXDAT_PokeParaGet( box, inBoxIdx, i, ID_PARA_id_no, NULL );
      if ( EntryID( inMyID, id, ioArray ) ) count++;
    }
  }
  OS_Printf("Box_DifID: %d\n",count);
  return count;
}

//--------------------------------------------------------------
/**
 * バトルボックス検索
 * @param   inMyID    自分のID
 * @param   gdata     ゲームデータポインタ
 * @param   inBoxIdx  ボックスインデックス
 * @param   ioArray   ID格納ワークポインタ
 * @retval  int       格納したIDの数
 */
//--------------------------------------------------------------
static int SearchBtlBox(const u32 inMyID, GAMEDATA * gdata, const int inBoxIdx, ARRAY_WORK *ioArray)
{
  int i;
  int count;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( gdata );
  BATTLE_BOX_SAVE   *btl_box = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
  
  //カウンタークリア
  count = 0;
  for(i=0;i<BATTLE_BOX_PARTY_MEMBER;i++)
  {
    POKEMON_PASO_PARAM* ppp = BATTLE_BOX_SAVE_GetPPP( btl_box , inBoxIdx, i );
    if ( PPP_Get( ppp, ID_PARA_poke_exist, NULL ) )
    {
      u32 id;
      id = PPP_Get( ppp, ID_PARA_id_no, NULL );
      if ( EntryID( inMyID, id, ioArray ) ) count++;
    }
  }
  OS_Printf("BtlBox_DifID: %d\n",count);
  return count;
}

//--------------------------------------------------------------
/**
 * IDのエントリ関数
 * @param   inMyID    自分のID
 * @param   inID      登録対象ID
 * @param   ioArray   ID格納ワークポインタ
 * @retval  BOOL      TRUEで登録成功
 */
//--------------------------------------------------------------
static BOOL EntryID( const u32 inMyID, const u32 inID, ARRAY_WORK *ioArray )
{
  BOOL rc = FALSE;
  if (inMyID != inID)   //自分と同じIDの場合は処理しない
  {
    rc = EntryIDCore(inID, ioArray);
  }
  return rc;
}

//--------------------------------------------------------------
/**
 * IDのエントリ関数
 * @param   inID      登録対象ID
 * @param   ioArray   ID格納ワークポインタ
 * @retval  BOOL      TRUEで登録成功
 */
//--------------------------------------------------------------
static BOOL EntryIDCore(const u32 inID, ARRAY_WORK *ioArray)
{
  int i;
  //既にエントリがいっぱいの場合は登録せずに終了
  if ( ioArray->NowEntryNum >= ID_SEARCH_MAX ) return FALSE; //もういっぱい
  
  for (i=0;i<ioArray->NowEntryNum;i++)
  {
    if (ioArray->ID[i] == inID) return FALSE;   //既にある
  }

  OS_Printf("entry_idx %d ID = %d\n", i, inID);
  ioArray->ID[i] = inID;
  ioArray->NowEntryNum++;
  return TRUE;
}


