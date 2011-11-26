//============================================================================================
/**
 * @file  pokewood_system.c
 * @brief
 * @author  mori GAMEFREAK inc.
 * @date  2011.07.11
 */
//============================================================================================

#include <gflib.h>

#include "pokewood_system.h"

#include "system/main.h"
#include "pokewood_system_local.h"
//#include "wbt_tool.h"

#include "poke_tool/poketype_def.h"
#include "tr_tool/trtype_def.h" //TRTYPE_HERO/TRTYPE_HEROINE

#include "fld_btl_inst_tool.h"
#include "tr_tool/ex_trainer_def.h" //TRID_OFFSET_WBT
#include "battle/pokewood/pokewood_senario.h"
#include "savedata/pokewood_savedata.h"
#include "savedata/medalbox.h"
#include "battle/battle.h"
#include "waza_tool/wazano_def.h"

#include "savedata/pokewood_rec.h"  //POKEWOOD_REC_GetWorkSize

//============================================================================================
//============================================================================================

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
//=============================================================================================
/**
 * @brief	POKEWOOD_SYSTEMワーク確保
 *
 * @param   heapID		
 *
 * @retval  PKWDSYS *		
 */
//=============================================================================================
PKWDSYS * PKWDSYS_Create( HEAPID heapID )
{
  int i;
  PKWDSYS * pkwdsys = GFL_HEAP_AllocClearMemory( heapID, sizeof(PKWDSYS) );

  pkwdsys->heapID = heapID;

  pkwdsys->backup_area = GFL_HEAP_AllocClearMemory( heapID, POKEWOOD_REC_GetWorkSize() );
  return pkwdsys;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
//=============================================================================================
/**
 * @brief	POKEWOOD_SYSTEMワーク解放
 *
 * @param   pkwdsys		
 */
//=============================================================================================
void PKWDSYS_Delete( PKWDSYS * pkwdsys )
{
  GFL_HEAP_FreeMemory( pkwdsys->backup_area );
  GFL_HEAP_FreeMemory( pkwdsys );
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
//=============================================================================================
/**
 * @brief	ポケモンリストからの結果を取得
 *
 * @param   pkwdsys		
 * @param   plist_data		
 */
//=============================================================================================
void PKWDSYS_SetPokelistResult( PKWDSYS * pkwdsys, PLIST_DATA * plist_data )
{
  GFL_STD_MemCopy( plist_data->in_num, pkwdsys->pokelist_select_num, 6 );
  pkwdsys->pokelist_result_select = plist_data->ret_sel;
  pkwdsys->pokelist_return_mode = plist_data->ret_mode;

  if( pkwdsys->pokelist_return_mode != PL_RET_NORMAL ||
      pkwdsys->pokelist_result_select == PL_SEL_POS_EXIT ||
      pkwdsys->pokelist_result_select == PL_SEL_POS_EXIT2 )
  {
    *pkwdsys->scr_answer_work = FALSE;
  }
  else
  {
    *pkwdsys->scr_answer_work = TRUE;
  }
#ifdef  PM_DEBUG
  {
    int i;
    MORI_Printf("Select Mons:");
    for (i = 0; i < 6; i ++ )
    {
      MORI_Printf("%02d ", pkwdsys->pokelist_select_num[i] );
    }
    MORI_Printf("\n ret_sel(%d) ret_mode(%d) script_return(%d)\n",
        pkwdsys->pokelist_result_select, pkwdsys->pokelist_return_mode, *pkwdsys->scr_answer_work );
  }
#endif
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void PKWDSYS_SetupPokeListData( PKWDSYS * pkwdsys, GAMEDATA * gamedata, PLIST_DATA * plist_data )
{
  int type;
  POKEPARTY *party = GAMEDATA_GetMyPokemon( gamedata );

  PLIST_DATA_Init( plist_data, gamedata, PL_MODE_BATTLE_SUBWAY, party );

  // 基本レギュレーテョンデータ読み込み
  PokeRegulation_LoadData( REG_FREE_SINGLE, &pkwdsys->reg );

  // ポケウッドシナリオデータから参戦可能数を書き換える
  {
    POKEWOOD_SENARIO *senario = GFL_ARC_LoadDataAlloc( ARCID_POKEWOOD, pkwdsys->scenario_no, HEAPID_PROC);
    pkwdsys->reg.NUM_LO = senario->pokenum_limit;   // @TODO:上限・下限ともに同数じゃなくなるかも
    pkwdsys->reg.NUM_HI = senario->pokenum_limit;
    GFL_HEAP_FreeMemory(senario);
  }
  
  // 拡張レギュレーション設定
  // 技「へんしん」はボスポケになれるので禁止
  REGULATIONEX_Init( &pkwdsys->reg_ex, POKETYPE_NULL, WAZANO_HENSIN );

  // パラメータに引渡し
  plist_data->reg   = (void *)&pkwdsys->reg;
  plist_data->regEx = (void *)&pkwdsys->reg_ex;
  plist_data->type = PL_TYPE_SINGLE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PKWDSYS_SetupPokeStatusData( PKWDSYS * pkwdsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus )
{
  ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gamedata );
  GFL_STD_MemClear32( pstatus, sizeof(PSTATUS_DATA) );
  pstatus->ppd = GAMEDATA_GetMyPokemon( gamedata );
  pstatus->ppt = PST_PP_TYPE_POKEPARTY;
  pstatus->max = PokeParty_GetPokeCount( pstatus->ppd );
  pstatus->mode = PST_MODE_NORMAL;
  pstatus->page = PPT_INFO;
  pstatus->game_data = gamedata;

  pstatus->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void PKWDSYS_SetScriptAnswerWork( PKWDSYS * pkwdsys, u16 * scr_answer_work )
{
  pkwdsys->scr_answer_work = scr_answer_work;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u16 * PKWDSYS_GetScriptAnswerWork( PKWDSYS * pkwdsys )
{
  GF_ASSERT( pkwdsys->scr_answer_work );
  return pkwdsys->scr_answer_work;
}


//------------------------------------------------------------------
/**
 * @brief ポケウッドに手持ちで参加か設定する
 */
//------------------------------------------------------------------
void PKWDSYS_SetJoinMypoke( PKWDSYS *pkwdsys, u16 flag )
{
  OS_Printf("手持ちポケモンでポケウッド参加？ = %d\n", flag);
  pkwdsys->join_mypoke = flag;
}
//------------------------------------------------------------------
/**
 * @brief ポケウッドに手持ちで参加か取得する
 */
//------------------------------------------------------------------
int PKWDSYS_GetJoinMypoke( PKWDSYS *pkwdsys )
{
  return pkwdsys->join_mypoke;
}

//------------------------------------------------------------------
/**
 * @brief 撮影するシナリオナンバーのセット
 */
//------------------------------------------------------------------
void PKWDSYS_SetScenarioNo( PKWDSYS *pkwdsys, int no )
{
  OS_Printf("選択したシナリオ = %d\n", no);
  pkwdsys->scenario_no = no;
}

//------------------------------------------------------------------
/**
 * @brief 撮影するシナリオナンバーの取得
 */
//------------------------------------------------------------------
u16 PKWDSYS_GetScenarioNo( PKWDSYS *pkwdsys )
{
  return pkwdsys->scenario_no;
}

//------------------------------------------------------------------
/**
 * @brief 録画セーブデータナンバーのセット
 */
//------------------------------------------------------------------
void PKWDSYS_SetPlayNo( PKWDSYS *pkwdsys, int no )
{
  OS_Printf("選択した上映データ = %d\n", no);
  pkwdsys->play_no = no;
}

//------------------------------------------------------------------
/**
 * @brief 録画セーブデータナンバーの取得
 */
//------------------------------------------------------------------
u16 PKWDSYS_GetPlayNo( PKWDSYS *pkwdsys )
{
  return pkwdsys->play_no;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void * PKWDSYS_GetBackupArea( PKWDSYS *pkwdsys )
{
  return pkwdsys->backup_area;
}

//------------------------------------------------------------------
/**
 * @brief 参戦するポケモンパーティを生成する
 * @param pkwdsys     
 * @param gamedata    
 * @param entry_party 書き出すPOKEPARTYポインタ（確保済みであること）
 *
 * @return  POKEPARTY 生成したポケモンパーティ
 *
 * @note    呼び出し元で責任を持ってメモリを開放する必要がある
 */
//------------------------------------------------------------------
void PKWDSYS_CreateBattleParty( PKWDSYS * pkwdsys, GAMEDATA * gamedata, POKEPARTY *entry_party )
{
  POKEPARTY * base_party;
  POKEMON_PARAM * entry_pp;
  u32 cnt, max;
  u8 revise_level;

  PokeParty_Init( entry_party, TEMOTI_POKEMAX );
  base_party = GAMEDATA_GetMyPokemon( gamedata );
  entry_pp = GFL_HEAP_AllocMemoryLo( pkwdsys->heapID, POKETOOL_GetWorkSize() );
  PP_Clear( entry_pp );
  max = pkwdsys->reg.NUM_HI;
  for ( cnt = 0; cnt < max; cnt ++ )
  {
    POKEMON_PARAM * base_pp;
    u32 pos = pkwdsys->pokelist_select_num[ cnt ];
    if ( pos == 0 )
    {
      GF_ASSERT_MSG( 0, "参戦するポケモンが正しくありません!!\n" );
      pos = 1;
    }
    pos -= 1; //1 origin --> 0 origin
    base_pp = PokeParty_GetMemberPointer( base_party, pos );
    POKETOOL_CopyPPtoPP( base_pp, entry_pp );
    PokeParty_Add( entry_party, entry_pp );
  }
  GFL_HEAP_FreeMemory( entry_pp );

}



//=============================================================================================
/**
 * @brief	成功・ストレンジENDの場合はセーブデータに記録する
 *
 * @param   pkwdsys		  PKWDSYS
 * @param   gamedata		GAMEDATA
 * @param   scenario_no	シナリオNO
 * @param   ending		  エンディング値
 * @param   ending		  興行収入
 */
//=============================================================================================
void PKWDSYS_CheckEnding( PKWDSYS *pkwdsys, GAMEDATA * gamedata, int scenario_no, BtlPokewoodResult result)
{
  POKEWOOD_SAVEDATA *ps = GAMEDATA_GetPokewoodSavedata(gamedata);

  // 映画のエンディング３パターン（成功・失敗・ストレンジ）を
  // シナリオ毎のセーブデータに反映させる
  if(result==BTL_POKEWOOD_RESULT_SUCCESS){
    POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_SUCCESS, scenario_no );
  }else if(result==BTL_POKEWOOD_RESULT_STRANGE){
    POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_STRANGE, scenario_no );
  }else if(result==BTL_POKEWOOD_RESULT_BAD){
    POKEWOOD_SAVEDATA_SetPokewoodFlag( ps, PWSV_TYPE_SENARIO_BAD,     scenario_no );
  }
}


#define PW_SERIES_END       (0xff)
//=================================================
// ポケウッド連続シナリオ定義
//=================================================
static const struct PWS_MEDAL_REF{
  u8 tbl[5];
  u8 medal_id;
} series_medal_tbl[]={
  {{  0,             PW_SERIES_END, }, MDLID_PKWD_SERIES0 }, // シリーズ0
  {{  1,  2,         PW_SERIES_END, }, MDLID_PKWD_SERIES1 }, // シリーズ1
  {{  3,  4,  5,  6, PW_SERIES_END, }, MDLID_PKWD_SERIES2 }, // シリーズ2
  {{  7,  8,  9, 10, PW_SERIES_END, }, MDLID_PKWD_SERIES3 }, // シリーズ3
  {{ 11, 12, 13, 14, PW_SERIES_END, }, MDLID_PKWD_SERIES4 }, // シリーズ4
  {{ 15, 16, 17,     PW_SERIES_END, }, MDLID_PKWD_SERIES5 }, // シリーズ5
  {{ 18, 19, 20,     PW_SERIES_END, }, MDLID_PKWD_SERIES6 }, // シリーズ6
  {{ 21, 22, 23,     PW_SERIES_END, }, MDLID_PKWD_SERIES7 }, // シリーズ7
  {{ 24, 25, 26, 27, PW_SERIES_END, }, MDLID_PKWD_SERIES8 }, // シリーズ8
  {{ 28, 29, 30,     PW_SERIES_END, }, MDLID_PKWD_SERIES9 }, // シリーズ9
  {{ 31, 32, 33, 34, PW_SERIES_END, }, MDLID_PKWD_SERIES10 }, // シリーズ10
  {{ 35, 36, 37, 38, PW_SERIES_END, }, MDLID_PKWD_SERIES11 }, // シリーズ11
  {{ 39,             PW_SERIES_END, }, MDLID_PKWD_SERIES12 }, // シリーズ12
};

/*
MDLID_PKWD_SERIES1	// シリーズかんけつ１
MDLID_PKWD_SERIES2	// シリーズかんけつ２
MDLID_PKWD_SERIES3	// シリーズかんけつ３
MDLID_PKWD_SERIES4	// シリーズかんけつ４
MDLID_PKWD_SERIES5	// シリーズかんけつ５
MDLID_PKWD_SERIES6	// シリーズかんけつ６
MDLID_PKWD_SERIES7	// シリーズかんけつ７
MDLID_PKWD_SERIES8	// シリーズかんけつ８
MDLID_PKWD_SERIES9	// シリーズかんけつ９
MDLID_PKWD_SERIES10	// シリーズかんけつ１０
MDLID_PKWD_SERIES11	// シリーズかんけつ１１
MDLID_PKWD_SERIES12	// シリーズかんけつ１２
MDLID_PKWD_SERIES_ALL	// マスターディレクター
MDLID_PKWD_BOXOFFICE	// がっぽりムービー
MDLID_PKWD_GOODEND	// めいさくディレクター
MDLID_PKWD_STRANGE_FIRST	// はじめてのカルト
MDLID_PKWD_STRANGE_ALL	// カルトディレクター
MDLID_PKWD_SERIES0	 // シリーズかんけつ0
*/
//----------------------------------------------------------------------------------
/**
 * @brief	ひとつのポケウッドシリーズの撮影状態をチェック
 *
 * @param   ps		  POKEWOOD_SAVEDATA
 * @param   index		シリーズを指す添字番号
 *
 * @retval  BOOL		TRUE:全て撮影済み FALSE:まだ未撮影がある
 */
//----------------------------------------------------------------------------------
static BOOL _check_pokewoodseries_played( POKEWOOD_SAVEDATA *ps, int index )
{
  int i;
  //シリーズの対象になるシナリオが全て上映済みか？
  for(i=0;series_medal_tbl[index].tbl[i]!=PW_SERIES_END;i++){
    if(POKEWOOD_SAVEDATA_GetPokewoodFlag( ps, PWSV_TYPE_SENARIO_PLAYED, 
                                          series_medal_tbl[index].tbl[i] )==0){
      break;
    }
  }
  // 抜けてきた値が番兵ならシリーズ達成
  if(series_medal_tbl[index].tbl[i]==PW_SERIES_END){
    return TRUE;
  }
  return FALSE;
}


//=============================================================================================
/**
 * @brief	メダルボックス用登録処理
 *
 * @param   gamedata		
 */
//=============================================================================================
void PKWDSYS_CheckPokewoodMedal( GAMEDATA * gamedata )
{
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  MEDALBOX *medalbox     = SaveData_GetMedalBox( sv );
  POKEWOOD_SAVEDATA *ps  = GAMEDATA_GetPokewoodSavedata(gamedata);
  int i;


  // ポケウッドシリーズメダル取得処理
  for(i=0;i<NELEMS(series_medal_tbl);i++){
    if(_check_pokewoodseries_played( ps, i )){
      MEDALBOX_SetTitleMedalAchive( medalbox, series_medal_tbl[i].medal_id );
      OS_Printf("シリーズメダル%dがオープン\n",i);
    }
  }

  // 全ての映画を撮影したかチェック
  if(POKEWOOD_SAVEDATA_GetFlagTotal(ps, PWSV_TYPE_SENARIO_PLAYED)==POKEWOOD_BOXOFFICE_DATA_MAX){
    MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_PKWD_SERIES_ALL );
  }

  // 全ての映画で成功ENDにしたかチェック
  if(POKEWOOD_SAVEDATA_GetFlagTotal(ps, PWSV_TYPE_SENARIO_SUCCESS)==POKEWOOD_BOXOFFICE_DATA_MAX){
    MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_PKWD_GOODEND );
  }

  // ストレンジEND関連
  {
    int strange_count = POKEWOOD_SAVEDATA_GetFlagTotal(ps, PWSV_TYPE_SENARIO_STRANGE);

    // 初めてのストレンジENDチェック
    if(strange_count!=0){
      MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_PKWD_STRANGE_FIRST );
    }

    // 全ての映画でストレンジENDにしたかチェック
    if(strange_count==POKEWOOD_BOXOFFICE_DATA_MAX){
      MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_PKWD_STRANGE_ALL );
    }
  }
  
}

//=============================================================================================
/**
 * @brief	興行成績データの作成
 *
 * @param   pkwdsys		    PKWDSYS
 * @param   bsp		        BATTLE_SETUP_PARAM (撮影バトルの結果が入っている）
 * @param   scenario_no		シナリオＮＯ
 */
//=============================================================================================
void PKWDSYS_MakeBoxofficeRecord( PKWDSYS *pkwdsys, BATTLE_SETUP_PARAM *bsp, int scenario_no )
{
  POKEPARTY *party = bsp->party[0];
  POKEWOOD_BOXOFFICE_RECORD  input;
  int i;

  MI_CpuClear8( &input, sizeof(POKEWOOD_BOXOFFICE_RECORD));

  // 興行収入
  input.profits  = bsp->pokewoodProfit;

  // ポケモン情報格納
  for(i=0;i<PokeParty_GetPokeCount(party);i++){
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(party,i);
    int mons, form, sex, star_point;
    mons = PP_Get( pp, ID_PARA_monsno, NULL );
    form = PP_Get( pp, ID_PARA_form_no, NULL );
    sex  = PP_Get( pp, ID_PARA_sex, NULL );
    input.pokeicon[i] = (u16)POKEICON_PackResourceData( mons, form, sex );
    star_point = PP_Get(pp, ID_PARA_star_point, NULL);
    if(POKETOOL_StarPoint2Rank(star_point) >= PTL_STAR_RANK_2){
      input.pokeicon_star_bit |= (1<<i);
    }
  }
  pkwdsys->BoxofficeData = input;
  pkwdsys->pokewoodResult = bsp->pokewoodResult;

}


//=============================================================================================
/**
 * @brief	ポケウッド撮影結果を記録に反映させる
 *
 * @param   bsp		
 */
//=============================================================================================
void PKWDSYS_BoxofficeRecordSet( GAMEDATA * gamedata, PKWDSYS *pkwdsys )
{
  POKEWOOD_SAVEDATA *ps  = GAMEDATA_GetPokewoodSavedata(gamedata);
  const POKEWOOD_BOXOFFICE_RECORD *compare = POKEWOOD_SAVEDATA_GetBoxOfficeData( ps, pkwdsys->scenario_no );
  
  OS_Printf("興行収入比較 シナリオ=%d\n", pkwdsys->scenario_no);
  
  // 興行収入で比較して代入
  if(pkwdsys->BoxofficeData.profits >= compare->profits){
    OS_Printf("興行収入データ更新: シナリオ%d\n", pkwdsys->scenario_no);
    POKEWOOD_SAVEDATA_SetBoxOfficeData( ps, pkwdsys->scenario_no, &pkwdsys->BoxofficeData );
  }
}

//=============================================================================================
/**
 * @brief	レコードデータへの登録
 *
 * @param   gamedata	GAMEDATA
 * @param   pkwdsys		POKEWOOD_SYSTEM
 */
//=============================================================================================
void PKWDSYS_RecordSet( GAMEDATA * gamedata, PKWDSYS *pkwdsys )
{
  RECORD      *record = SaveData_GetRecord(GAMEDATA_GetSaveControlWork(gamedata));

  RECORD_SetIfLarge( record, RECID_POKEWOOD_HIGH, pkwdsys->BoxofficeData.profits);  // 最高
  RECORD_Add( record, RECID_POKEWOOD_TOTAL, pkwdsys->BoxofficeData.profits); // 累計
  
}


//エンディングに対応したポケモン用取得スターポイント
static int star_point_tbl_poke[]={
  -250,  // BAD END
  150,  // GOOD END
  250,  // STRANGE END
};

//エンディングに対応したプレイヤー用取得スターポイント
static int star_point_table_player[]={
  1,  // BAD END
  2,  // GOOD END
  4,  // STRANGE END
};


//=============================================================================================
/**
 * @brief	プレイヤーのスターポイント加算処理
 *
 * @param   gamedata	GAMEDATA
 * @param   pkwdsys		POKEWOOD_SYSTEM
 * @param   result		ポケウッドの上映結果
 * @param   play_no		録画データNO
 */
//=============================================================================================
void PKWDSYS_CalcStarPoint( GAMEDATA * gamedata, PKWDSYS *pkwdsys, BtlPokewoodResult result, int play_no )
{
  POKEWOOD_SAVEDATA *ps  = GAMEDATA_GetPokewoodSavedata(gamedata);
  GF_ASSERT( result<BTL_POKEWOOD_RESULT_MAX );
  
  if(POKEWOOD_SAVEDATA_GetMovieShowed( ps, play_no )==FALSE){
    POKEWOOD_SAVEDATA_AddStarPoint( ps, star_point_table_player[result] );
  }

}

//=============================================================================================
/**
 * @brief	ポケモンにスターポイントを加算する
 *
 * @param   gamedata	GAMEDATA
 * @param   pkwdsys		POKEWOOD_SYSTEM
 */
//=============================================================================================
void PKWDSYS_CalcMyPokeStarPoint( GAMEDATA * gamedata, PKWDSYS *pkwdsys )
{
  int max,cnt;
  POKEPARTY  *myparty = GAMEDATA_GetMyPokemon( gamedata );

  OS_Printf("手持ちポケモン? %d\n ", pkwdsys->join_mypoke);

  // 手持ちポケモンじゃなかったら終了
  if(pkwdsys->join_mypoke==FALSE){
    return;
  }
  GF_ASSERT( pkwdsys->pokewoodResult < BTL_POKEWOOD_RESULT_MAX );
  
  
  // 参加したポケモンにスターポイントを加算する
  max = pkwdsys->reg.NUM_HI;
  for ( cnt = 0; cnt < max; cnt ++ )
  {
    POKEMON_PARAM * pp;
    u32 pos = pkwdsys->pokelist_select_num[ cnt ];
    int star_point;
    pp = PokeParty_GetMemberPointer( myparty, pos-1 );
    star_point = PP_Get( pp, ID_PARA_star_point, NULL );
    star_point += star_point_tbl_poke[pkwdsys->pokewoodResult];
//    star_point = MATH_CLAMP(star_point, 0, 255 );
    if(star_point<0){   star_point = 0;}
    if(star_point>255){ star_point = 255;}
    PP_Put( pp, ID_PARA_star_point, star_point );
    OS_Printf("スターポイント加算 %d, result=%d, star_point=%d\n ", 
      pkwdsys->pokelist_select_num[ cnt ], pkwdsys->pokewoodResult, star_point);
  }
}

// シナリオ毎に使用するポスター定義
#include "../app/pkwd_announce/pkwd_movie_poster.cdat"

//=============================================================================================
/**
 * @brief	上映決定時の看板を設定する
 *
 * @param   gamedata	GAMEDATA
 * @param   pkwdsys		PKWDSYS
 * @param   rec_pos		外部セーブ番号
 */
//=============================================================================================
void PKWDSYS_SetKanban( GAMEDATA * gamedata, PKWDSYS *pkwdsys, int rec_pos )
{
  POKEWOOD_SAVEDATA *ps  = GAMEDATA_GetPokewoodSavedata(gamedata);

  if(pkwdsys->scenario_no<POKEWOOD_PLAY_MAX){
    // 看板の扱いは0=無しなので、＋１して渡す
    POKEWOOD_SAVEDATA_SetKanbanNo( ps, rec_pos, scenario_logo_tbl[pkwdsys->scenario_no]+1 );
  }
}
