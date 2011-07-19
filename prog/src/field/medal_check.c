//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medal_check.c
 *	@brief  メダル達成関数
 *	@author	Masayuki Onoue
 *	@date		2011.02.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
// ライブラリ
#include  <gflib.h>

//システム
#include "gamesystem/game_data.h"

#include "savedata/record.h"
#include "system/medaldata.h" // fro MEDALDATA
#include "arc_def.h"          // for ARCID_MEDALDATA
#include "savedata\medalbox.h"
#include "../../../resource/medal/data/mdlrec_id.h" // for MDLRECID_START etc
#include "field/medal_check.h"
#include "script_def.h" // SCR_MEDALCATEGORY_～

#include "fldmmdl.h"  // for MMDL_GetFallRockCnt
#include "poke_tool/poketype_def.h" // for POKETYPE_MAX
#include "medal_achieve_data.naix"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"                          // MDL_FLAG_ANIMALTRAIL_～

// セーブデータ
#include "savedata/musical_save.h"
#include "savedata/zukan_savedata.h"
#include "savedata/sodateya_work.h"
#include "savedata/bsubway_savedata.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define   HINT_LEVEL_MAX  (6) // ヒントレベルの種類数

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//-------------------------------------
///	タイプデータアーカイブ構造体
//=====================================
typedef struct {
  u16 zenkoku_no;
  u8  type1;
  u8  type2;
  u8  is_check;
  u8  padding[3];
}TYPE_TABLE_DATA;

//-------------------------------------
///	データチェック用ワーク
//=====================================
typedef struct {
  BOOL  is_set_zukandata;
  BOOL  zukan_type_flag[POKETYPE_MAX];  // タイプごとのコンプリートフラグ
  BOOL  is_set_medal_data;    // メダルデータを調べ済み
  BOOL  is_set_my_party;      // 手持ちデータを調べ済み
  BOOL  is_set_boxdata;       // ボックスデータを調べ済み
  BOOL  is_set_box_save_data; // ボックスデータを調べ済み
  BOOL  is_set_sodateya;
  BOOL  is_get_pokerus;
  BOOL  is_get_n_poke;
  BOOL  is_get_rare;
  u32   medal_get_count[ SCR_MEDALCATEGORY_MAX ];
  u32   box_total;
}MEDALREC_DATA;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void MEDAL_CheckMedalRecAchieve( MedalID medal_id ,s32 cmp_id, u32 target_count, GAMEDATA* p_gamedata, MEDALBOX *p_medal_box ,MEDALREC_DATA *p_recdata, HEAPID heapID );
static BOOL CheckZukanData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID );
static BOOL BreakTypeCmpFlag( BOOL *p_out ,u32 type1 ,u32 type2 );
static BOOL CheckBoxData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID );
static BOOL CheckPartyData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID );
static BOOL CheckSodateyaData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID );
static void CheckPPFlagCheck( MEDALREC_DATA *p_recdata, const POKEMON_PARAM *p_pp );
static BOOL CheckTypeComplete( u8 type_id ,MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID );
static BOOL cmp_target( u32 cnt ,u32 target );
static BOOL CheckeMedalData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID );

// 判定関数群
static BOOL MDLREC_CHECK_CompNormal( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompHonoo( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompMizu( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompDenki( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompKusa( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompKoori( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompKakutou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompDoku( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompJime( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompHikou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompEsper( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompMushi( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompIwa( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompGhost( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompDragon( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompAku( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompHagane( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_CompMusicalGoods( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_BoxPokeCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_BsubwaySingleRenshou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_BsubwayDoubleRenshou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_BsubwayMultiRenshou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_FriendCodeCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_FieldMedalCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_BattleMedalCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_AmusementMedalCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_ChallengeMedalCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_n_poke( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_rare_poke( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_pokerus_poke( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_fall_rock( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_SuretigaiCount( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_BattleCount( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_TradeCount( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );
static BOOL MDLREC_CHECK_AnimalTrailCheck( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID );

// 判定用関数テーブル読み込み
typedef BOOL (*MDLACHIEVEPROC)( MEDALREC_DATA* ,GAMEDATA* ,u32 ,HEAPID );
#include "../../../resource/medal/achieve_table/mdlachieve_proc_table.cdat"


//----------------------------------------------------------------------------
/**
 *	@brief  タイトルメダル、ヒントメダルの達成状態更新関数
 *
 *	@param	p_gamedata
 *	@param	*p_sv 
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDAL_AchiveCheck( GAMEDATA* p_gamedata, SAVE_CONTROL_WORK *p_sv , HEAPID heapID )
{
  int i;
  MedalID     medal_id;
  MEDALBOX    *p_medal_box;
  u8          medal_state;
  ARCHANDLE   *p_handle;
  MEDALDATA   medal_data[MDLID_MAX];
  RECORD      *p_rec;
  u8          open_hint_level = 0;
  u8          title_open_cnt[HINT_LEVEL_MAX];
  u8          title_open_max[HINT_LEVEL_MAX];
  MEDALREC_DATA medalrec_data;

#if 1
#ifdef PM_DEBUG
  // 生成にかかる micro second を表示
  OSTick debug_start_tick = OS_GetTick(); 
  OSTick debug_end_tick;
#endif 
#endif 

  GF_ASSERT_HEAVY( p_sv );
 
  // クリア
  GFL_STD_MemClear( &medalrec_data, sizeof(medalrec_data) );

  // タイプ別コンプリートフラグを初期化
  for( i = 0; i < POKETYPE_MAX; i++ )
  {
    medalrec_data.zukan_type_flag[i] = TRUE;
  }

  // 獲得条件アーカイブデータハンドル取得
  p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , heapID );

  // 達成条件データを取得
  GFL_ARC_LoadDataOfsByHandle( p_handle , 0 , 0 , sizeof(MEDALDATA)*MDLID_MAX ,medal_data );

  // セーブデータの取得
  p_medal_box = SaveData_GetMedalBox( p_sv );
  p_rec = SaveData_GetRecord(p_sv);

  // 初期化
  for( i = 0; i < HINT_LEVEL_MAX ; i++)
  {
    title_open_cnt[i] = 0;
    title_open_max[i]  = 0;
  }


  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );

    // タイトル達成処理
    if( medal_state < MEDAL_STATE_TITLE_ACHIVE )
    {
      if( medal_data[medal_id].cmp_id < MDLRECID_START )
      {
        // レコードデータとの比較
        u32 rec_data = RECORD_Get( p_rec , medal_data[medal_id].cmp_id );

        // タイトル達成を行っていない場合はまず調べる
        if( medal_state < MEDAL_STATE_TITLE_ACHIVE )
        {
          if( medal_data[medal_id].target_count <= rec_data )
          {
            MEDALBOX_SetTitleMedalAchive( p_medal_box , medal_id );
          }
        }
      }
      else
      {
        // レコード以外の条件を判定する
        MEDAL_CheckMedalRecAchieve( medal_id ,medal_data[medal_id].cmp_id ,medal_data[medal_id].target_count ,p_gamedata ,p_medal_box ,&medalrec_data ,heapID );
      }
    }

    // タイトル獲得数とヒントレベル毎の獲得最大数を取得
    // (最初からとヒントレベル式のものを数える)
    medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );
    if( (medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_FIRST) || 
        (medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_LEVEL) )
    {
      if( (medal_state == MEDAL_STATE_TITLE_GET) ||
          (medal_state == MEDAL_STATE_TITLE_ACHIVE))
      {
        title_open_cnt[medal_data[medal_id].hint_level]++;
      }
      title_open_max[ medal_data[medal_id].hint_level ]++;
    }
  }

  // ヒントレベルごとのタイトル取得数が過半数以上を取得している場合は、そのレベル＋１のヒントレベルまで全て開放する
  for( i = HINT_LEVEL_MAX-1 ; i >= 0; i-- )
  {
    if( (title_open_cnt[i]) >= (title_open_max[ i ]/2) )
    {
      for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
      {
        medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );

        // ヒントレベルで開放するものだけ開放
        if( medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_LEVEL )
        {
          if( medal_data[medal_id].hint_level<= i+1 )
          {
            if( medal_state < MEDAL_STATE_HINT_ACHIVE )
            {
              MEDALBOX_SetHintMedalAchive( p_medal_box , medal_id );
            }
          }
        }
      }
      break;
    }
  }

  {
    int hint_level_max = -1;

    // ヒントが開放されているもので一番ヒントレベルが高いものを調べる(ヒントレベル式のもの限定)
    for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
    {
      medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );

      // ヒントレベルで開放するものだけ開放
      if( medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_LEVEL )
      {
        if( (medal_state == MEDAL_STATE_TITLE_GET) ||
            (medal_state == MEDAL_STATE_TITLE_ACHIVE))
        {
          if( hint_level_max < medal_data[medal_id].hint_level )
          {
            hint_level_max = medal_data[medal_id].hint_level;
          }
        }
      }
    }

    // 一番ヒントレベル未満のヒントは全て開放
    if( hint_level_max >= 0 )
    {
      for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
      {
        medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );

        // ヒントレベルで開放するものだけ開放
        if( medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_LEVEL )
        {
          // 取得している最大未満は全て開放
          if( medal_data[medal_id].hint_level < hint_level_max )
          {
            if( medal_state < MEDAL_STATE_HINT_ACHIVE )
            {
              MEDALBOX_SetHintMedalAchive( p_medal_box , medal_id );
            }
          }
        }

      }
    }
  }

  // アーカイブクローズ
  GFL_ARC_CloseDataHandle(p_handle);

  
#if 1
#ifdef PM_DEBUG 
  debug_end_tick = OS_GetTick();
  debug_end_tick -= debug_start_tick;
  OS_Printf( "achieve_check micro second:%d\n",OS_TicksToMicroSeconds( debug_end_tick ) );
#endif
#endif
 
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒント取得条件が最初からのメダルをヒント取得状態にする
 *
 *	@param	*p_sv 
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDAL_InitHintMedal( SAVE_CONTROL_WORK *p_sv , HEAPID heapID )
{
  MedalID     medal_id;
  MEDALBOX    *p_medal_box;
  ARCHANDLE   *p_handle;
  MEDALDATA   medal_data[MDLID_MAX];
  RTCDate     rtc;
  u8          medal_state;

  GF_ASSERT_HEAVY( p_sv );

  // 獲得条件アーカイブデータハンドル取得
  p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , heapID );

  // 達成条件データを取得
  GFL_ARC_LoadDataOfsByHandle( p_handle , 0 , 0 , sizeof(MEDALDATA)*MDLID_MAX ,medal_data );

  // セーブデータの取得
  p_medal_box = SaveData_GetMedalBox( p_sv );
  
  // 現在の時間を取得する
  GFL_RTC_GetDate(&rtc);

  // 最初から持っているヒントメダルを初期化する
  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );
    if( medal_state < MEDAL_STATE_HINT_ACHIVE )
    {
      if( medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_FIRST )
      {
        MEDALBOX_SetHintMedalAchive( p_medal_box , medal_id );
        MEDALBOX_SetHintMedalGetFlg( p_medal_box , medal_id ,rtc.year ,rtc.month ,rtc.day);
      }
    }
  }

  // アーカイブクローズ
  GFL_ARC_CloseDataHandle(p_handle);
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルレコードID個別の
 *
 *	@param	medal_id 
 *	@param	cmp_id
 *	@param	p_gamedata
 *	@param	*p_medal_box 
 *	@param	*p_recdata
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MEDAL_CheckMedalRecAchieve( MedalID medal_id ,s32 cmp_id, u32 target_count, GAMEDATA* p_gamedata, MEDALBOX *p_medal_box ,MEDALREC_DATA *p_recdata, HEAPID heapID )
{
  BOOL is_achieve = FALSE;
  s32  proc_id = cmp_id - MDLRECID_START;

#if 1
#ifdef PM_DEBUG
  // 生成にかかる micro second を表示
  OSTick debug_start_tick = OS_GetTick(); 
  OSTick debug_end_tick;
#endif 
#endif 

  // 関数テーブルから比較用関数を取得し、調査する
  if( sc_achieve_proc[ proc_id ] != NULL )
  {
    is_achieve = sc_achieve_proc[ proc_id ]( p_recdata, p_gamedata ,target_count ,heapID);
  }

  // 達成状態を設定する
  if( is_achieve == TRUE )
  {
    MEDALBOX_SetTitleMedalAchive( p_medal_box , medal_id );
  }

   
#if 1
#ifdef PM_DEBUG 
  debug_end_tick = OS_GetTick();
  debug_end_tick -= debug_start_tick;
  OS_Printf( "[%2u]ac ms:%d\n",proc_id,OS_TicksToMicroSeconds( debug_end_tick ) );
#endif
#endif
 
}

//----------------------------------------------------------------------------
/**
 *	@brief  図鑑関連フラグを調査する
 *
 *	@param	*p_recdata
 *	@param	p_gamedata
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckZukanData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID )
{
#if 0
#ifdef PM_DEBUG
  // 生成にかかる micro second を表示
  OSTick debug_start_tick = OS_GetTick(); 
  OSTick debug_end_tick;
#endif
#endif

  // 無かった場合は生成する
  if( p_recdata->is_set_zukandata == FALSE )
  {
    u16 local_mons_no;
    u16 local_mons_max;
    BOOL is_get;
    ZUKAN_SAVEDATA* p_zukan_savedata = GAMEDATA_GetZukanSave( p_gamedata );
    TYPE_TABLE_DATA type_data[ZUKANNO_ISSHU_END+1]; // 0オリジンなので+1

    // タイプテーブル取得
    GFL_ARC_LoadData(type_data,ARCID_MEDALACHIEVEDATA,NARC_medal_achieve_data_achieve_data_bin);

    for( local_mons_no = 0; local_mons_no <= ZUKANNO_ISSHU_END; local_mons_no++ )
    {
      if( type_data[local_mons_no].is_check == TRUE )
      {
        BOOL is_get = ZUKANSAVE_GetPokeGetFlag( p_zukan_savedata , type_data[local_mons_no].zenkoku_no );  // 捕獲チェック
        u32 form_no;
        u32 type1,type2;

        // タイプフラグを落とす処理
        if( is_get == FALSE )
        {
          if( BreakTypeCmpFlag( p_recdata->zukan_type_flag ,type_data[local_mons_no].type1 ,type_data[local_mons_no].type2 ) )
          {
            break;
          }
        }
      }
    }
    p_recdata->is_set_zukandata = TRUE;
  }

#if 0
#ifdef PM_DEBUG 
  debug_end_tick = OS_GetTick();
  debug_end_tick -= debug_start_tick;
  OS_Printf( "zukan micro second:%d\n",OS_TicksToMicroSeconds( debug_end_tick ) );
#endif
#endif
  
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイプコンプリートのフラグを落とす
 *
 *	@param	*p_out 
 *	@param	type1 
 *	@param	type2 
 *
 *	@retval 全てのフラグがオフになったらTRUE
 */
//-----------------------------------------------------------------------------
static BOOL BreakTypeCmpFlag( BOOL *p_out ,u32 type1 ,u32 type2 )
{
  BOOL is_flag_break = FALSE;

  // 同タイプの場合は1回
  if( type1 == type2 )
  {
    if( p_out[ type1 ] == TRUE )
    {
      p_out[ type1 ] = FALSE;
      is_flag_break = TRUE;
    }
  }
  else
  {
    // 別タイプを持っている場合は、2つに
    if( p_out[ type1 ] == TRUE )
    {
      p_out[ type1 ] = FALSE;
      is_flag_break = TRUE;
    }
    if( p_out[ type2 ] == TRUE )
    {
      p_out[ type2 ] = FALSE;
      is_flag_break = TRUE;
    }
  }

  // フラグを落とした場合は、全てのフラグが落ちているかをチェック
  if( is_flag_break )
  {
    int i;
    for( i = 0; i < POKETYPE_MAX; i++ )
    {
      if( p_out[i] == TRUE )
      {
        return FALSE;
      }
    }
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボックスデータ関連フラグチェック
 *
 *	@param	*p_recdata
 *	@param	p_gamedata
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckBoxData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID )
{
#if 0 
#ifdef PM_DEBUG
  // 生成にかかる micro second を表示
  OSTick debug_start_tick = OS_GetTick(); 
  OSTick debug_end_tick;
#endif
#endif

  if( p_recdata->is_set_boxdata == FALSE )
  {
    u32 tray_num;
    u32 pos;
    BOX_MANAGER *p_box = GAMEDATA_GetBoxManager(p_gamedata);
    for( tray_num = 0; tray_num < BOX_MAX_TRAY; tray_num++)
    {
      for( pos = 0; pos < BOX_MAX_POS; pos++)
      {
        if( BOXDAT_PokeParaGet( p_box ,tray_num ,pos ,ID_PARA_poke_exist, NULL ) )
        {
          BOOL flag;
          POKEMON_PASO_PARAM *p_ppp;
          p_ppp = BOXDAT_GetPokeDataAddress( p_box ,tray_num ,pos );

          // ボックスポケモンパラメータ構造体へのアクセスを高速化モードへ
          flag = PPP_FastModeOn( p_ppp );

          // ポケルスチェック
          if( p_recdata->is_get_pokerus == FALSE )
          {
            if( PPP_Get( p_ppp, ID_PARA_pokerus, NULL ) != 0 )
            {
              p_recdata->is_get_pokerus = TRUE;
            }
          }

          // Nポケ
          if( p_recdata->is_get_n_poke == FALSE )
          {
            p_recdata->is_get_n_poke = PPP_Get( p_ppp, ID_PARA_n_poke_flag, NULL );
          }

          // レア
          if( p_recdata->is_get_rare == FALSE ){
            p_recdata->is_get_rare = PPP_CheckRare( p_ppp );
          }
          
          // ボックスポケモンパラメータ構造体へのアクセスを高速化モード終了
          PPP_FastModeOff( p_ppp, flag );
        }
      }
    }
    p_recdata->is_set_boxdata = TRUE;
  }

#if 0 
#ifdef PM_DEBUG 
  debug_end_tick = OS_GetTick();
  debug_end_tick -= debug_start_tick;
  OS_Printf( "box micro second:%d\n",OS_TicksToMicroSeconds( debug_end_tick ) );
#endif
#endif
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  手持ちポケモンチェック
 *
 *	@param	*p_recdata
 *	@param	p_gamedata
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckPartyData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID )
{
  int i;

  // 手持ちパーティの取得
  POKEPARTY *p_party  = GAMEDATA_GetMyPokemon( p_gamedata );
  int max = PokeParty_GetPokeCount( p_party );  // 全ポケモン数

  // 一度のみ生成
  if( p_recdata->is_set_my_party == FALSE )
  {
    // 手持ちを調べる
    for( i = 0; i < max; i++ )
    {
      BOOL flag;
      POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer( p_party ,i );
      // ボックスポケモンパラメータ構造体へのアクセスを高速化モードへ
      flag = PP_FastModeOn( p_pp );
      CheckPPFlagCheck( p_recdata ,p_pp );
      // ボックスポケモンパラメータ構造体へのアクセスを高速化モード終了
      PP_FastModeOff( p_pp, flag );
    }
    p_recdata->is_set_my_party = TRUE;
  }

  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  飼育ポケモンチェック
 *
 *	@param	*p_recdata
 *	@param	p_gamedata
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckSodateyaData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID )
{ 
  int i;
  SAVE_CONTROL_WORK* p_svc = GAMEDATA_GetSaveControlWork( p_gamedata );
  SODATEYA_WORK *p_sodateya_sv = SODATEYA_WORK_GetSodateyaWork( p_svc );
 
  if( p_recdata->is_set_sodateya == FALSE )
  {
    // 育てやチェック
    for( i = 0; i < SODATEYA_POKE_MAX; i++ )
    {
      if( SODATEYA_WORK_IsValidPokemon( p_sodateya_sv ,i ) )
      {
        const POKEMON_PARAM *p_pp = SODATEYA_WORK_GetPokemon( p_sodateya_sv ,i );
        CheckPPFlagCheck( p_recdata ,p_pp );
      }
    }
    p_recdata->is_set_sodateya = TRUE;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンの状態フラグを調べる
 *
 *	@param	*p_recdata
 *	@param	p_pp 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CheckPPFlagCheck( MEDALREC_DATA *p_recdata, const POKEMON_PARAM *p_pp )
{
  if( !PP_Get( p_pp, ID_PARA_tamago_flag, NULL ) )
  {
    // ポケルスチェック
    if( p_recdata->is_get_pokerus == FALSE )
    {
      if( PP_Get( p_pp, ID_PARA_pokerus, NULL ) != 0 )
      {
        p_recdata->is_get_pokerus = TRUE;
      }
    }

    // Nポケ
    if( p_recdata->is_get_n_poke == FALSE )
    {
      p_recdata->is_get_n_poke = PP_Get( p_pp, ID_PARA_n_poke_flag, NULL );
    }

    // レア
    if( p_recdata->is_get_rare == FALSE ){
      p_recdata->is_get_rare = PP_CheckRare( p_pp );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイプコンプリートフラグチェック
 *
 *	@param	type_id 
 *	@param	*p_recdata
 *	@param	p_gamedata
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckTypeComplete( u8 type_id ,MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID )
{
  if( CheckZukanData( p_recdata , p_gamedata ,heapID ) )
  {
    return p_recdata->zukan_type_flag[ POKETYPE_NORMAL ];
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  目標数に達しているかしらべる
 *
 *	@param	cnt 
 *	@param	target 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL cmp_target( u32 cnt ,u32 target )
{
  if( cnt >= target )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルのグラフィックタイプからカテゴリを取得する
 *
 *	@param	graphic_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 MEDAL_GetMedalCategoryIndex( u8 graphic_type )
{
  // カテゴリ分けテーブル
  int category_index[] = {
    SCR_MEDALCATEGORY_ADVENTURE,SCR_MEDALCATEGORY_ADVENTURE,
    SCR_MEDALCATEGORY_ADVENTURE,SCR_MEDALCATEGORY_ADVENTURE,
    SCR_MEDALCATEGORY_ADVENTURE,  // 5
    SCR_MEDALCATEGORY_BATTLE,SCR_MEDALCATEGORY_BATTLE,
    SCR_MEDALCATEGORY_BATTLE,SCR_MEDALCATEGORY_BATTLE,
    SCR_MEDALCATEGORY_BATTLE,     // 5
    SCR_MEDALCATEGORY_AMUSEMENT,SCR_MEDALCATEGORY_AMUSEMENT,
    SCR_MEDALCATEGORY_AMUSEMENT,SCR_MEDALCATEGORY_AMUSEMENT,
    SCR_MEDALCATEGORY_AMUSEMENT,  // 5
    SCR_MEDALCATEGORY_CHALLENGE,SCR_MEDALCATEGORY_CHALLENGE,
    SCR_MEDALCATEGORY_CHALLENGE,SCR_MEDALCATEGORY_CHALLENGE,
    SCR_MEDALCATEGORY_CHALLENGE,  // 5
    SCR_MEDALCATEGORY_SPECIAL,SCR_MEDALCATEGORY_SPECIAL,
    SCR_MEDALCATEGORY_SPECIAL,SCR_MEDALCATEGORY_SPECIAL,
    SCR_MEDALCATEGORY_SPECIAL,SCR_MEDALCATEGORY_SPECIAL,
    SCR_MEDALCATEGORY_SPECIAL,    // 7
  };

  return category_index[ graphic_type ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルデータチェック
 *
 *	@param	*p_recdata
 *	@param	p_gamedata
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckeMedalData( MEDALREC_DATA *p_recdata, GAMEDATA* p_gamedata, HEAPID heapID )
{
  SAVE_CONTROL_WORK* p_svc = GAMEDATA_GetSaveControlWork( p_gamedata );
  MEDALBOX *p_medal_box = SaveData_GetMedalBox( p_svc );

#if 0
#ifdef PM_DEBUG
  // 生成にかかる micro second を表示
  OSTick debug_start_tick = OS_GetTick(); 
  OSTick debug_end_tick;
#endif
#endif

  if( p_recdata->is_set_medal_data == FALSE )
  {
    u32 id;
    u32 data_cnt = GFL_ARC_GetDataFileCnt( ARCID_MEDALDATA );
    MEDALDATA *p_medal_data = GFL_ARC_LoadDataAllocOfs( ARCID_MEDALDATA ,0 ,heapID ,0,data_cnt*sizeof(MEDALDATA));
  
    // データの初期化
    GFL_STD_MemClear(p_recdata->medal_get_count,sizeof(p_recdata->medal_get_count) );
    for ( id = 0; id < MDLID_MAX; id ++ )
    {
      u32 category_index;
      category_index = MEDAL_GetMedalCategoryIndex(p_medal_data[id].graphic_type);
      if( MEDALBOX_GetMedalState( p_medal_box, id ) == MEDAL_STATE_TITLE_GET )
      {
        p_recdata->medal_get_count[category_index]++;
        
      }
    }
    GFL_HEAP_FreeMemory( p_medal_data );
    p_recdata->is_set_medal_data = TRUE;
  }
  
#if 0
#ifdef PM_DEBUG 
  debug_end_tick = OS_GetTick();
  debug_end_tick -= debug_start_tick;
  OS_Printf( "medal micro second:%d\n",OS_TicksToMicroSeconds( debug_end_tick ) );
#endif
#endif
  
  return TRUE;
}


//=============================================================================
//
//					メダルごとの達成チェック関数
//
//=============================================================================

// ノーマルタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompNormal( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_NORMAL ,p_recdata , p_gamedata ,heapID);
}
// ほのおタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompHonoo( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_HONOO ,p_recdata , p_gamedata ,heapID);
}
// みずタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompMizu( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_MIZU ,p_recdata , p_gamedata ,heapID);
}
// でんきタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompDenki( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_DENKI ,p_recdata , p_gamedata ,heapID);
}
// くさタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompKusa( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_KUSA ,p_recdata , p_gamedata ,heapID);
}
// こおりタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompKoori( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_KOORI ,p_recdata , p_gamedata ,heapID);
}
// かくとうタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompKakutou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_KAKUTOU ,p_recdata , p_gamedata ,heapID);
}
// どくタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompDoku( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_DOKU ,p_recdata , p_gamedata ,heapID);
}
// じめんタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompJime( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_JIMEN ,p_recdata , p_gamedata ,heapID);
}
// ひこうタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompHikou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_HIKOU ,p_recdata , p_gamedata ,heapID);
}
// エスパータイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompEsper( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_ESPER ,p_recdata , p_gamedata ,heapID);
}
// むしタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompMushi( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_MUSHI ,p_recdata , p_gamedata ,heapID);
}
// いわタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompIwa( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_IWA ,p_recdata , p_gamedata ,heapID);
}
// ゴーストタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompGhost( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_GHOST ,p_recdata , p_gamedata ,heapID);
}
// ドラゴンタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompDragon( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_DRAGON ,p_recdata , p_gamedata ,heapID);
}
// あくタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompAku( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_AKU ,p_recdata , p_gamedata ,heapID);
}
// はがねタイプポケモンコンプリート
static BOOL MDLREC_CHECK_CompHagane( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  return CheckTypeComplete( POKETYPE_HAGANE ,p_recdata , p_gamedata ,heapID);
}
// ミュージカルグッズコンプリート
static BOOL MDLREC_CHECK_CompMusicalGoods( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  MUSICAL_SAVE *p_sv = GAMEDATA_GetMusicalSavePtr( p_gamedata);
  return MUSICAL_SAVE_IsCompleteItem( p_sv );
}
// ボックス内ポケモン数
static BOOL MDLREC_CHECK_BoxPokeCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  u32 box_cnt;
  if( p_recdata->is_set_box_save_data == FALSE )
  {
    BOX_MANAGER *p_box = GAMEDATA_GetBoxManager(p_gamedata);
    p_recdata->box_total = BOXDAT_GetPokeExistCountTotal( p_box );
    p_recdata->is_set_box_save_data = TRUE;
  }
  return cmp_target( p_recdata->box_total , target_cnt );
}
// バトルサブウェイシングル連勝
static BOOL MDLREC_CHECK_BsubwaySingleRenshou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  u32 renshou_cnt;
  BSUBWAY_SCOREDATA* score = GAMEDATA_GetBSubwayScoreData( p_gamedata );
  renshou_cnt = BSUBWAY_SCOREDATA_GetRenshou( score, BSWAY_PLAYMODE_SINGLE );
  return cmp_target( renshou_cnt , target_cnt );
}
// バトルサブウェイダブル連勝
static BOOL MDLREC_CHECK_BsubwayDoubleRenshou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  u32 renshou_cnt;
  BSUBWAY_SCOREDATA* score = GAMEDATA_GetBSubwayScoreData( p_gamedata );
  renshou_cnt = BSUBWAY_SCOREDATA_GetRenshou( score, BSWAY_PLAYMODE_DOUBLE );
  return cmp_target( renshou_cnt , target_cnt );
}
// バトルサブウェイマルチ連勝
static BOOL MDLREC_CHECK_BsubwayMultiRenshou( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  u32 renshou_cnt;
  BSUBWAY_SCOREDATA* score = GAMEDATA_GetBSubwayScoreData( p_gamedata );
  renshou_cnt = BSUBWAY_SCOREDATA_GetRenshou( score, BSWAY_PLAYMODE_MULTI );
  return cmp_target( renshou_cnt , target_cnt );
}
// 友達コード登録数
static BOOL MDLREC_CHECK_FriendCodeCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  s32 i;
  u32 friend_data_num = 0;
  WIFI_LIST* p_list = GAMEDATA_GetWiFiList( p_gamedata );
  for(i = 0; i < WIFILIST_FRIEND_MAX;i++){
    if( WifiList_IsFriendData( p_list, i ) ){
      u32 sex = WifiList_GetFriendInfo( p_list, i, WIFILIST_FRIEND_SEX );
      if( sex != PM_NEUTRAL )
      {
        friend_data_num++;
      }
    }
  }
  ONOUE_Printf("友達登録数=%u\n",friend_data_num);
  return cmp_target( friend_data_num , target_cnt );
}
// ぼうけんメダル獲得数
static BOOL MDLREC_CHECK_FieldMedalCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  if( CheckeMedalData( p_recdata , p_gamedata ,heapID ) )
  {
    return cmp_target( p_recdata->medal_get_count[ SCR_MEDALCATEGORY_ADVENTURE ] , target_cnt );
  }
  GF_ASSERT(0); // メダルデータ生成エラー
  return FALSE;
}
// せんとうメダル獲得数
static BOOL MDLREC_CHECK_BattleMedalCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  if( CheckeMedalData( p_recdata , p_gamedata ,heapID ) )
  {
    return cmp_target( p_recdata->medal_get_count[ SCR_MEDALCATEGORY_BATTLE ] , target_cnt );
  }
  GF_ASSERT(0); // メダルデータ生成エラー
  return FALSE;
}
// おたのしみメダル獲得数
static BOOL MDLREC_CHECK_AmusementMedalCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  if( CheckeMedalData( p_recdata , p_gamedata ,heapID ) )
  {
    return cmp_target( p_recdata->medal_get_count[ SCR_MEDALCATEGORY_AMUSEMENT ] , target_cnt );
  }
  GF_ASSERT(0); // メダルデータ生成エラー
  return FALSE;
}
// ちょうせんメダル獲得数
static BOOL MDLREC_CHECK_ChallengeMedalCnt( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  if( CheckeMedalData( p_recdata , p_gamedata ,heapID ) )
  {
    return cmp_target( p_recdata->medal_get_count[ SCR_MEDALCATEGORY_CHALLENGE ] , target_cnt );
  }
  GF_ASSERT(0); // メダルデータ生成エラー
  return FALSE;
}
// Nポケモンチェック
static BOOL MDLREC_CHECK_n_poke( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  BOOL is_achieve = FALSE;
  if( CheckBoxData( p_recdata , p_gamedata ,heapID ) &&
      CheckPartyData( p_recdata , p_gamedata ,heapID ) &&
      CheckSodateyaData( p_recdata , p_gamedata ,heapID ))
  {
    is_achieve = p_recdata->is_get_n_poke;
  }
  return is_achieve;
}
// 色違いポケモンチェック
static BOOL MDLREC_CHECK_rare_poke( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  BOOL is_achieve = FALSE;
  if( CheckBoxData( p_recdata , p_gamedata ,heapID ) &&
      CheckPartyData( p_recdata , p_gamedata ,heapID ) &&
      CheckSodateyaData( p_recdata , p_gamedata ,heapID ))
  {
    is_achieve = p_recdata->is_get_rare;
  }
  return is_achieve;
}
// ポケルスポケモンチェック
static BOOL MDLREC_CHECK_pokerus_poke( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  BOOL is_achieve = FALSE;
  if( CheckBoxData( p_recdata , p_gamedata ,heapID ) &&
      CheckPartyData( p_recdata , p_gamedata ,heapID ) &&
      CheckSodateyaData( p_recdata , p_gamedata ,heapID ))
  {
    is_achieve = p_recdata->is_get_pokerus;
  }
  return is_achieve;
}

// 岩チェック
static BOOL MDLREC_CHECK_fall_rock( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  u32 fall_rock_cnt = 0;
  MMDLSYS *p_mmdlsys = GAMEDATA_GetMMdlSys( p_gamedata );
  fall_rock_cnt = MMDL_GetFallRockCnt( p_mmdlsys );
  ONOUE_Printf("落とした岩の数：%d\n",fall_rock_cnt);
  return cmp_target( fall_rock_cnt , target_cnt );
}

// すれ違い人数チェック
static BOOL MDLREC_CHECK_SuretigaiCount( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  u32 suretigai_cnt = 0;
  SAVE_CONTROL_WORK* p_svc = GAMEDATA_GetSaveControlWork( p_gamedata );
  const MISC *p_misc = SaveData_GetMiscConst( p_svc );
  suretigai_cnt = MISC_CrossComm_GetSuretigaiCount( p_misc );
  ONOUE_Printf("すれ違い人数：%d\n",suretigai_cnt);
  return cmp_target( suretigai_cnt , target_cnt );
}

// 対戦数チェック
static BOOL MDLREC_CHECK_BattleCount( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  u32 rec_sum = 0;
  SAVE_CONTROL_WORK* p_svc = GAMEDATA_GetSaveControlWork( p_gamedata );
  RECORD      *p_rec = SaveData_GetRecord(p_svc);
  rec_sum += RECORD_Get( p_rec , RECID_COMM_BATTLE );
  rec_sum += RECORD_Get( p_rec , RECID_WIFI_BATTLE );
  rec_sum += RECORD_Get( p_rec , RECID_IRC_BATTLE );
  return cmp_target( rec_sum , target_cnt );
}

// 通信交換数チェック
static BOOL MDLREC_CHECK_TradeCount( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  u32 rec_sum = 0;
  SAVE_CONTROL_WORK* p_svc = GAMEDATA_GetSaveControlWork( p_gamedata );
  RECORD      *p_rec = SaveData_GetRecord(p_svc);
  rec_sum += RECORD_Get( p_rec , RECID_COMM_TRADE );
  rec_sum += RECORD_Get( p_rec , RECID_WIFI_TRADE );
  rec_sum += RECORD_Get( p_rec , RECID_IRC_TRADE );
  return cmp_target( rec_sum , target_cnt );
}

// 獣道チェック
static BOOL MDLREC_CHECK_AnimalTrailCheck( MEDALREC_DATA* p_recdata, GAMEDATA* p_gamedata, u32 target_cnt , HEAPID heapID )
{
  s32 i;
  u32 animal_trail_cnt = 0;
  const s32 at_flag_tbl[] =
  {
    MDL_FLAG_ANIMALTRAIL_01,
    MDL_FLAG_ANIMALTRAIL_02,
    MDL_FLAG_ANIMALTRAIL_03,
    MDL_FLAG_ANIMALTRAIL_04,
    MDL_FLAG_ANIMALTRAIL_05,
    MDL_FLAG_ANIMALTRAIL_06,
    MDL_FLAG_ANIMALTRAIL_07,
    MDL_FLAG_ANIMALTRAIL_08,
    MDL_FLAG_ANIMALTRAIL_09,
    MDL_FLAG_ANIMALTRAIL_10,
    MDL_FLAG_ANIMALTRAIL_11,
    MDL_FLAG_ANIMALTRAIL_12,
    MDL_FLAG_ANIMALTRAIL_13,
    MDL_FLAG_ANIMALTRAIL_14,
    MDL_FLAG_ANIMALTRAIL_15,
    MDL_FLAG_ANIMALTRAIL_16,
    MDL_FLAG_ANIMALTRAIL_17,
    MDL_FLAG_ANIMALTRAIL_18,
    MDL_FLAG_ANIMALTRAIL_19,
    MDL_FLAG_ANIMALTRAIL_20,
  };
  s32 table_cnt = NELEMS(at_flag_tbl);
  EVENTWORK* p_event_wk = GAMEDATA_GetEventWork( p_gamedata );

  for( i = 0;  i < table_cnt ; i++ )
  {
    BOOL is_set = EVENTWORK_CheckEventFlag( p_event_wk ,at_flag_tbl[i]);
    if( is_set == TRUE )
    {
      animal_trail_cnt++;
    }
  }
  ONOUE_Printf("獣道到達数%u\n",animal_trail_cnt);
  return cmp_target( animal_trail_cnt , target_cnt );
}

