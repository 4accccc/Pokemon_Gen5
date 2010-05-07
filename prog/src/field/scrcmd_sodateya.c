//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：育て屋関連
 * @file   scrcmd_sodateya.c
 * @author obata
 * @date   2009.09.24
 *
 */
//////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd.h"
#include "scrcmd_sodateya.h"
#include "app/pokelist.h"
#include "system/main.h"
#include "script_local.h"
#include "app/p_status.h"
#include "event_fieldmap_control.h"
#include "savedata/sodateya_work.h"
#include "sodateya.h"
#include "fieldmap.h"
#include "script_def.h"
#include "poke_tool/poke_tool.h"

FS_EXTERN_OVERLAY(pokelist);


//====================================================================
// ■プロトタイプ宣言
//====================================================================
static GMEVENT* EVENT_SodatePokeSelect( 
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, u16* ret_wk );

static int SodateyaCheck_TakeBackMenu( const POKEMON_PARAM* poke );


//====================================================================
// ■タマゴ関連
//====================================================================

//--------------------------------------------------------------------
/**
 * @brief 育て屋のポケモンに卵が生まれたかどうかのチェック
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdCheckSodateyaHaveEgg( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );

  *ret_wk = (u16)SODATEYA_HaveEgg( sodateya );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋のタマゴを受け取る
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaEgg( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*         gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );

  SODATEYA_TakeBackEgg( sodateya, party ); 
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋のタマゴを削除する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdDeleteSodateyaEgg( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );

  SODATEYA_DeleteEgg( sodateya );
  return VMCMD_RESULT_CONTINUE;
} 


//====================================================================
// ■ポケモン預ける・引き取る
//====================================================================

//--------------------------------------------------------------------
/**
 * @brief ポケモンの選択画面を呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdSodateyaSelectTemotiPokemon( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*         sw = SCRCMD_WORK_GetScriptWork( work );
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA*         gdata = SCRCMD_WORK_GetGameData( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  SAVE_CONTROL_WORK*  scw = GAMEDATA_GetSaveControlWork( gdata );
  SODATEYA_WORK* sodateya = SODATEYA_WORK_GetSodateyaWork( scw );

  // イベント呼び出し
  {
    GMEVENT* event; 
    event = EVENT_SodatePokeSelect( gsys, fieldmap, ret_wk );
    SCRIPT_CallEvent( sw, event );
  }

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋にポケモンを預ける
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdTrustPokeToSodateya( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMEDATA*         gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );

  SODATEYA_TakePokemon( sodateya, party, pos );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋からポケモンを受け取る
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdCheckOutPokeFromSodateya( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMEDATA*         gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );

  SODATEYA_TakeBackPokemon( sodateya, pos, party );
  return VMCMD_RESULT_CONTINUE;
}


//====================================================================
// ■預けポケモン関連
//====================================================================

//--------------------------------------------------------------------
/**
 * @breif 育て屋に預けているポケモンの数を取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaPokemonNum( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );

  *ret_wk = (u16)SODATEYA_GetPokemonNum( sodateya );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @breif 預けポケモン2体の相性をチェックする
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdSodateyaLoveCheck( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );

  *ret_wk = (u16)SODATEYA_GetLoveLevel( sodateya );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋に預けているポケモンのモンスターナンバーを取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaPokeMonsNo( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );
  const POKEMON_PARAM* param = SODATEYA_GetPokemonParam( sodateya, pos );

  *ret_wk = (u16)PP_Get( param, ID_PARA_monsno, NULL );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋に預けているポケモンの形状ナンバーを取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaPokeFormNo( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );
  const POKEMON_PARAM* param = SODATEYA_GetPokemonParam( sodateya, pos );

  *ret_wk = (u16)PP_Get( param, ID_PARA_form_no, NULL );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋に預けているポケモンについて, 性別を取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaPokeSex( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );
  const POKEMON_PARAM* param = SODATEYA_GetPokemonParam( sodateya, pos );

  *ret_wk = (u16)PP_Get( param, ID_PARA_sex, NULL );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋に預けているポケモンについて, 現在のレベルを取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaPokeLv( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );

  *ret_wk = (u16)SODATEYA_GetPokeLv_Current( sodateya, pos );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋に預けているポケモンについて, 成長したレベルを取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaPokeGrowUpLv( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );
  const POKEMON_PARAM* param = SODATEYA_GetPokemonParam( sodateya, pos );

  *ret_wk = (u16)SODATEYA_GetPokeLv_GrowUp( sodateya, pos );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋に預けているポケモンについて, 引き取り料金を取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaPokeTakeBackCharge( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );
  const POKEMON_PARAM* param = SODATEYA_GetPokemonParam( sodateya, pos );

  *ret_wk = (u16)SODATEYA_GetCharge( sodateya, pos );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋ポケモン特殊チェック
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdSodateyaCheck( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*   work     = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*  gsys     = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );

  u16* ret_wk;
  u16 poke_pos, check_id;
  const POKEMON_PARAM* poke;

  // コマンド引数を受け取る
  ret_wk   = SCRCMD_GetVMWork( core, work );      // 第一引数: 結果の格納先
  check_id = SCRCMD_GetVMWorkValue( core, work ); // 第二引数: チェックID
  poke_pos = SCRCMD_GetVMWorkValue( core, work ); // 第三引数: 預けポケモンIndex

  // 引数チェック
  GF_ASSERT( poke_pos < SODATEYA_GetPokemonNum(sodateya) );

  poke = SODATEYA_GetPokemonParam( sodateya, poke_pos );

  // チェックIDごとに判定
  switch( check_id ) {
    case SCR_SODATEYA_CHECK_ID_TAKEBACK_MENU:
      *ret_wk = SodateyaCheck_TakeBackMenu( poke );
      break;
    default:
      GF_ASSERT(0);
      *ret_wk = 0;
      break;
  }

  return VMCMD_RESULT_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief 育て屋特殊チェック ( 引き取りメニューの分類 )
 *
 * @param poke チェック対象のポケモン
 *
 * @return 引き取りメニューの分類
 */
//------------------------------------------------------------------
static int SodateyaCheck_TakeBackMenu( const POKEMON_PARAM* poke )
{
  u32 monsno = PP_Get( poke, ID_PARA_monsno, NULL );
  u32 sex    = PP_Get( poke, ID_PARA_sex, NULL );

  // ニドラン♂ or ニドラン♀
  if( (monsno == MONSNO_NIDORAN_M) || (monsno == MONSNO_NIDORAN_F) ) {
    // ニックネームが無い場合は, 性別なし用のメニューを使用する
    if( PP_Get( poke, ID_PARA_nickname_flag, NULL ) == FALSE ) {
      return SCR_SODATEYA_CHECK_RESULT_TAKEBACK_MENU_UNKNOWN;
    }
  }

  // 性別ごとに分岐
  switch( sex ) {
    default: GF_ASSERT(0);
    case PTL_SEX_SPEC_UNKNOWN: return SCR_SODATEYA_CHECK_RESULT_TAKEBACK_MENU_UNKNOWN;
    case PTL_SEX_SPEC_MALE:    return SCR_SODATEYA_CHECK_RESULT_TAKEBACK_MENU_MALE;
    case PTL_SEX_SPEC_FEMALE:  return SCR_SODATEYA_CHECK_RESULT_TAKEBACK_MENU_FEMALE;
  }
}


//====================================================================
// ■非公開関数
//====================================================================

//------------------------------------------------------------------
/**
 * @brief 育てポケモン選択ワーク
 */
//------------------------------------------------------------------
typedef struct 
{
	GAMESYS_WORK*      gsys;  // ゲームシステム
	FIELDMAP_WORK* fieldmap;  // フィールドマップ
  PLIST_DATA*      plData;  // ポケモンリスト
  PSTATUS_DATA*    psData;  // ポケモンステータス
  u16*            retWork;  // 選択ポケモン番号の格納先ワーク
} POKE_SELECT_WORK;

//------------------------------------------------------------------
/**
 * @brief イベント動作関数 (育てポケモン選択イベント)
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_SodatePokeSelect(GMEVENT * event, int * seq, void * work)
{
	POKE_SELECT_WORK * psw = work;
	GAMESYS_WORK *gsys = psw->gsys;

  // シーケンスの定義
  enum {
    SEQ_FADE_OUT,
    SEQ_FIELDMAP_CLOSE,
    SEQ_SELECT_POKEMON,
    SEQ_FIELDMAP_OPEN,
    SEQ_FADE_IN,
    SEQ_END,
  };

	switch( *seq ) {
	case SEQ_FADE_OUT: //// フェードアウト
		GMEVENT_CallEvent(event, EVENT_FieldFadeOut_Black(gsys, psw->fieldmap, FIELD_FADE_WAIT));
    *seq = SEQ_FIELDMAP_CLOSE;
		break;
	case SEQ_FIELDMAP_CLOSE: //// フィールドマップ終了
		GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, psw->fieldmap));
    psw->plData->ret_sel = psw->psData->pos;  // カーソル初期位置セット
    *seq = SEQ_SELECT_POKEMON;
		break;
  case SEQ_SELECT_POKEMON:
		GMEVENT_CallEvent(event, EVENT_PokeSelect(gsys, psw->fieldmap, psw->plData, psw->psData));
    *seq = SEQ_FIELDMAP_OPEN;
    break;
	case SEQ_FIELDMAP_OPEN: //// フィールドマップ復帰
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    *seq = SEQ_FADE_IN;
		break;
	case SEQ_FADE_IN: //// フェードイン
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, psw->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    *seq = SEQ_END;
		break;
	case SEQ_END: //// イベント終了
    *psw->retWork = psw->plData->ret_sel; // 選択結果をワークに格納
    GFL_HEAP_FreeMemory( psw->plData );
    GFL_HEAP_FreeMemory( psw->psData );
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
} 

//------------------------------------------------------------------
/**
 * @brief 育てポケモン選択イベント
 *
 * @param gsys      ゲームシステム
 * @param fieldmap  フィールドマップ
 * @param ret_wk    選択結果の格納先ワーク
 */
//------------------------------------------------------------------
static GMEVENT * EVENT_SodatePokeSelect( 
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, u16* ret_wk )
{
	GMEVENT* event;
	POKE_SELECT_WORK* psw;
  PLIST_DATA* pl_data;
  PSTATUS_DATA* ps_data;
  GAMEDATA*  gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY* party = GAMEDATA_GetMyPokemon( gdata );
  ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gdata );

  // ポケモンリスト生成
  pl_data    = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(PLIST_DATA) );
  pl_data->pp = party;
  pl_data->mode = PL_MODE_SODATEYA;
  pl_data->type = PL_TYPE_SINGLE;
  pl_data->ret_sel = 0;

  // ポケモンステータス生成
  ps_data = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(PSTATUS_DATA) );
  ps_data->ppd = pl_data->pp;
  ps_data->cfg = pl_data->cfg;
  ps_data->game_data = gdata;
  ps_data->ppt = PST_PP_TYPE_POKEPARTY;
  ps_data->max = PokeParty_GetPokeCount( party );
  ps_data->mode = PST_MODE_NORMAL;
  ps_data->page = PPT_INFO;
  ps_data->pos = 0;
  ps_data->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );

  // イベント生成
  event = GMEVENT_Create(gsys, NULL, EVENT_FUNC_SodatePokeSelect, sizeof(POKE_SELECT_WORK));
  psw   = GMEVENT_GetEventWork(event);
	psw->gsys      = gsys;
	psw->fieldmap  = fieldmap;
  psw->plData    = pl_data;
  psw->psData    = ps_data;
  psw->retWork   = ret_wk;
	return event;
} 
