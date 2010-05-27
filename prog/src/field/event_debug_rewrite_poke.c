#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『ポケモン書き換え』
 * @file   event_debug_rewrite_poke.c
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
#include "fieldmap.h"

#include "message.naix"
#include "font/font.naix"
#include "msg/msg_d_field.h"

#include "event_fieldmap_control.h"
#include "event_debug_rewrite_poke.h"


//======================================================================
//  デバッグメニュー ポケモン作成
//======================================================================
#include "debug/debug_makepoke.h"
FS_EXTERN_OVERLAY(debug_makepoke);

//-------------------------------------
/// デバッグポケモン作成用ワーク
//=====================================
typedef struct
{
  HEAPID heapID;
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK *p_field;
  PROCPARAM_DEBUG_MAKEPOKE p_mp_work;
  POKEMON_PARAM *pp;
  POKEMON_PARAM *p_src_pp;
} DEBUG_REWRITEPOKE_EVENT_WORK;
static GMEVENT_RESULT debugMenuReWritePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//--------------------------------------------------------------
/**
 * イベント：ポケモン作成
 * @param gsys
 * @param wk
 * @return GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu_ReWritePoke( GAMESYS_WORK *gsys, void* wk )
{ 
  DEBUG_MENU_EVENT_WORK* debug_work = wk;
  FIELDMAP_WORK *p_field  = debug_work->fieldWork;
  HEAPID heapID = HEAPID_PROC;
  DEBUG_REWRITEPOKE_EVENT_WORK *p_ev_wk;
  GMEVENT       *p_event;

  //イヴェント
  p_event = GMEVENT_Create( gsys, debug_work->gmEvent, 
      debugMenuReWritePoke, sizeof(DEBUG_REWRITEPOKE_EVENT_WORK) );

  p_ev_wk = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_ev_wk, sizeof(DEBUG_REWRITEPOKE_EVENT_WORK) );

  //ワーク設定
  p_ev_wk->p_gamesys  = gsys;
  p_ev_wk->p_event    = p_event;
  p_ev_wk->p_field    = p_field;
  p_ev_wk->heapID     = heapID;
  p_ev_wk->p_src_pp   = PP_Create( 1, 1, 1, heapID );
  {
    POKEPARTY *p_party  = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(gsys) );
    p_ev_wk->pp =  PokeParty_GetMemberPointer( p_party, 0 );

    POKETOOL_CopyPPtoPP( p_ev_wk->pp, p_ev_wk->p_src_pp );
  }
  p_ev_wk->p_mp_work.dst  = p_ev_wk->pp;
  p_ev_wk->p_mp_work.oyaStatus = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(gsys) );
  p_ev_wk->p_mp_work.mode = DMP_MODE_MAKE;

  return p_event;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグポケモン書き換えイベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuReWritePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
  };

  DEBUG_REWRITEPOKE_EVENT_WORK *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(debug_makepoke), &ProcData_DebugMakePoke, &p_wk->p_mp_work ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    if( p_wk->p_mp_work.ret_code == DMP_RET_COPY )
    {
      GAMEDATA *gmData = GAMESYSTEM_GetGameData(p_wk->p_gamesys);
      POKEPARTY *party = GAMEDATA_GetMyPokemon(gmData);
      BOX_MANAGER* boxman = GAMEDATA_GetBoxManager( gmData );

      //手持ちに空きがあれば入れる
      if( PokeParty_GetPokeCount( party ) < 6 )
      {
        PokeParty_Add( party , p_wk->pp );
      }else{
        BOXDAT_PutPokemon( boxman, PP_GetPPPPointer( p_wk->pp ));
      }
    }

#if 0 //別にこれは要らないのでは？
    {
      //書き換えないデータ
      static const u16 sc_id_tbl[]  =
      {
       ID_PARA_pref_code,
       ID_PARA_oyasex,
       ID_PARA_get_ground_id,
       ID_PARA_style,                    //かっこよさ
       ID_PARA_beautiful,                //うつくしさ
       ID_PARA_cute,                     //かわいさ
       ID_PARA_clever,                   //かしこさ
       ID_PARA_strong,                   //たくましさ
       ID_PARA_fur,                      //毛艶
       ID_PARA_sinou_champ_ribbon,       //シンオウチャンプリボン
       ID_PARA_sinou_battle_tower_ttwin_first,     //シンオウバトルタワータワータイクーン勝利1回目
       ID_PARA_sinou_battle_tower_ttwin_second,    //シンオウバトルタワータワータイクーン勝利2回目
       ID_PARA_sinou_battle_tower_2vs2_win50,      //シンオウバトルタワータワーダブル50連勝
       ID_PARA_sinou_battle_tower_aimulti_win50,   //シンオウバトルタワータワーAIマルチ50連勝
       ID_PARA_sinou_battle_tower_siomulti_win50,  //シンオウバトルタワータワー通信マルチ50連勝
       ID_PARA_sinou_battle_tower_wifi_rank5,      //シンオウバトルタワーWifiランク５入り
       ID_PARA_sinou_syakki_ribbon,        //シンオウしゃっきリボン
       ID_PARA_sinou_dokki_ribbon,         //シンオウどっきリボン
       ID_PARA_sinou_syonbo_ribbon,        //シンオウしょんぼリボン
       ID_PARA_sinou_ukka_ribbon,          //シンオウうっかリボン
       ID_PARA_sinou_sukki_ribbon,         //シンオウすっきリボン
       ID_PARA_sinou_gussu_ribbon,         //シンオウぐっすリボン
       ID_PARA_sinou_nikko_ribbon,         //シンオウにっこリボン
       ID_PARA_sinou_gorgeous_ribbon,      //シンオウゴージャスリボン
       ID_PARA_sinou_royal_ribbon,         //シンオウロイヤルリボン
       ID_PARA_sinou_gorgeousroyal_ribbon, //シンオウゴージャスロイヤルリボン
       ID_PARA_sinou_ashiato_ribbon,       //シンオウあしあとリボン
       ID_PARA_sinou_record_ribbon,        //シンオウレコードリボン
       ID_PARA_sinou_history_ribbon,       //シンオウヒストリーリボン
       ID_PARA_sinou_legend_ribbon,        //シンオウレジェンドリボン
       ID_PARA_sinou_red_ribbon,           //シンオウレッドリボン
       ID_PARA_sinou_green_ribbon,         //シンオウグリーンリボン
       ID_PARA_sinou_blue_ribbon,          //シンオウブルーリボン
       ID_PARA_sinou_festival_ribbon,      //シンオウフェスティバルリボン
       ID_PARA_sinou_carnival_ribbon,      //シンオウカーニバルリボン
       ID_PARA_sinou_classic_ribbon,       //シンオウクラシックリボン
       ID_PARA_sinou_premiere_ribbon,      //シンオウプレミアリボン
       ID_PARA_stylemedal_normal,          //かっこよさ勲章(ノーマル)AGBコンテスト
       ID_PARA_stylemedal_super,         //かっこよさ勲章(スーパー)AGBコンテスト
       ID_PARA_stylemedal_hyper,         //かっこよさ勲章(ハイパー)AGBコンテスト
       ID_PARA_stylemedal_master,          //かっこよさ勲章(マスター)AGBコンテスト
       ID_PARA_beautifulmedal_normal,        //うつくしさ勲章(ノーマル)AGBコンテスト
       ID_PARA_beautifulmedal_super,       //うつくしさ勲章(スーパー)AGBコンテスト
       ID_PARA_beautifulmedal_hyper,       //うつくしさ勲章(ハイパー)AGBコンテスト
       ID_PARA_beautifulmedal_master,        //うつくしさ勲章(マスター)AGBコンテスト
       ID_PARA_cutemedal_normal,         //かわいさ勲章(ノーマル)AGBコンテスト
       ID_PARA_cutemedal_super,          //かわいさ勲章(スーパー)AGBコンテスト
       ID_PARA_cutemedal_hyper,          //かわいさ勲章(ハイパー)AGBコンテスト
       ID_PARA_cutemedal_master,         //かわいさ勲章(マスター)AGBコンテスト
       ID_PARA_clevermedal_normal,         //かしこさ勲章(ノーマル)AGBコンテスト
       ID_PARA_clevermedal_super,          //かしこさ勲章(スーパー)AGBコンテスト
       ID_PARA_clevermedal_hyper,          //かしこさ勲章(ハイパー)AGBコンテスト
       ID_PARA_clevermedal_master,         //かしこさ勲章(マスター)AGBコンテスト
       ID_PARA_strongmedal_normal,         //たくましさ勲章(ノーマル)AGBコンテスト
       ID_PARA_strongmedal_super,          //たくましさ勲章(スーパー)AGBコンテスト
       ID_PARA_strongmedal_hyper,          //たくましさ勲章(ハイパー)AGBコンテスト
       ID_PARA_strongmedal_master,         //たくましさ勲章(マスター)AGBコンテスト
       ID_PARA_champ_ribbon,           //チャンプリボン
       ID_PARA_winning_ribbon,           //ウィニングリボン
       ID_PARA_victory_ribbon,           //ビクトリーリボン
       ID_PARA_bromide_ribbon,           //ブロマイドリボン
       ID_PARA_ganba_ribbon,           //がんばリボン
       ID_PARA_marine_ribbon,            //マリンリボン
       ID_PARA_land_ribbon,            //ランドリボン
       ID_PARA_sky_ribbon,             //スカイリボン
       ID_PARA_country_ribbon,           //カントリーリボン
       ID_PARA_national_ribbon,          //ナショナルリボン
       ID_PARA_earth_ribbon,           //アースリボン
      };
      int i;
      int id;
      //書き換えて欲しくないデータを入れる
      for( i = 0; i < NELEMS(sc_id_tbl); i++ )
      {
        id  = sc_id_tbl[i];
        PP_Put( p_wk->pp, id, PP_Get( p_wk->p_src_pp, id, NULL ) );
      }
    }
#endif
    GFL_HEAP_FreeMemory( p_wk->p_src_pp );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

#endif // PM_DEBUG 
