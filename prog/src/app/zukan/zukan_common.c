//============================================================================================
/**
 * @file		zukan_common.c
 * @brief		図鑑画面 共通処理
 * @author	Hiroyuki Nakamura
 * @date		09.12.15
 *
 *	モジュール名：ZKNCOMM
 */
//============================================================================================
#include <gflib.h>

#include "../../field/script_def.h"

#include "system/main.h"
#include "system/wipe.h"
#include "poke_tool/monsnum_def.h"

#include "zukan_common.h"

#include "arc_def.h"          // for ARCID_*
#include "fieldmap/zone_id.h" // for ZONE_ID_*
#include "../../../../resource/fldmapdata/flagwork/flag_define.h" // for SYS_FLAG_ARRIVE_*
#include "../../../../resource/habitat_list/habitat_list_data.h" // for HABITAT_LIST_DATA_MAX
#include "../../../../resource/habitat_list/habitat_list_data.cdat" // for sc_habitat_list_data

#include "savedata/misc.h"

//============================================================================================
//	定数定義
//============================================================================================
#define	BASEBG_SCROLL_WAIT	( 4 )			// ＢＧスクロールウェイト
#define	BASEBG_SCROLL_VAL		( 1 )			// ＢＧスクロール値

//============================================================================================
//	プロトタイプ
//============================================================================================
static void GetCompleteFlag( HABITATLIST_STATUS *p_dest,
    ARC_HABITAT_ZONE_DATA *p_arc_data ,ZUKAN_SAVEDATA *p_save);
static BOOL CheckHabitatZone( ZUKAN_SAVEDATA * p_zkn_sv,
    ARC_HABITAT_MONS_DATA *p_mons_data ,
    u16 ground_type,u16 season_id,BOOL season_enable);
static BOOL CheckLandEnable( ARC_HABITAT_ZONE_DATA *p_arc_data,BOOL *is_vanish );

//--------------------------------------------------------------------------------------------
/**
 * @brief		デフォルトリスト作成
 *
 * @param		sv			図鑑セーブデータ
 * @param		list		リスト作成場所
 * @param		heapID	ヒープＩＤ
 *
 * @return	リストに登録した数
 */
//--------------------------------------------------------------------------------------------
u16 ZKNCOMM_MakeDefaultList( ZUKAN_SAVEDATA * sv, u16 ** list, HEAPID heapID )
{
	u16 * buf;
	u16	siz, max;
	u32	i;

	if( ZUKANSAVE_GetZukanMode( sv ) == TRUE ){
		siz = ZUKAN_GetNumberRow( ZKNCOMM_LIST_SORT_MODE_ZENKOKU, heapID, &buf );
	}else{
		siz = ZUKAN_GetNumberRow( ZKNCOMM_LIST_SORT_MODE_LOCAL, heapID, &buf );
	}
	max = 0;

	for( i=0; i<siz; i++ ){
		if( ZUKANSAVE_GetPokeGetFlag( sv, buf[i] ) == TRUE ||
				ZUKANSAVE_GetPokeSeeFlag( sv, buf[i] ) == TRUE ){
			max = i+1;
		}
	}

	*list = buf;

	return max;
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定ゾーングループのリストを作成する
 *
 *	@param	* sv
 *	@param	zone_group_id
 *	@param	** list
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 ZKNCOMM_MakeHabitatList( ZUKAN_SAVEDATA * p_zkn_sv,HABITATLIST_SAVE  *p_habitat_sv, u16 ** list, HEAPID heapID )
{
	u32	i;
	u16 * buf;
	u16	max = 0;
  s32 arc_idx = -1;
  s32 data_cnt = 0;
  u16 zone_group_id = HABITATLIST_SAVE_GetDefaultZoneGroupID( p_habitat_sv );
      
  u8 ground_type   = HABITATLIST_SAVE_GetDefaultGround( p_habitat_sv );
  u8 season_id     = HABITATLIST_SAVE_GetDefaultSeason( p_habitat_sv );

  // アーカイブハンドル取得
  ARC_HABITAT_ZONE_DATA arc_data;
  ARCHANDLE	*p_handle	= 
    GFL_ARC_OpenDataHandle( ARCID_HABITAT_LIST_DATA , heapID );

  // アーカイブIDを検索
  for( i = 0; i < HABITAT_LIST_DATA_MAX ; i++ )
  {
    const HABITAT_LIST_DATA *sc_habitat = &sc_habitat_list_data[i];
    if( sc_habitat_list_data[i].zone_group_id == zone_group_id )
    {
      arc_idx = sc_habitat_list_data[i].arc_idx;
      break;
    }
  }

  // 検索した結果発見できなかった場合
  if( arc_idx < 0 )
  {
    arc_idx = 0;
    GF_ASSERT(0);
  }
  
  // アーカイブデータ取得
  GFL_STD_MemClear( &arc_data ,sizeof(ARC_HABITAT_ZONE_DATA) );
  GFL_ARC_LoadDataByHandle( p_handle , arc_idx , &arc_data);
  for( i = 0; i < arc_data.data_num ; i++ )
  {
    BOOL flg = CheckHabitatZone( 
        p_zkn_sv,&arc_data.mons_data[i],
        ground_type,season_id,arc_data.season_enable );
    if( flg == TRUE )
    {
      max++;
    }
  }
 
  ONOUE_Printf("max:%d\n",max);
  GF_ASSERT( max > 0 );

  // リスト長のデータを作成
  buf = GFL_HEAP_AllocClearMemory( heapID ,sizeof(u16)*max );

  // リストに載せるデータを保存
  for( i = 0; i < arc_data.data_num ; i++ )
  {
    BOOL flg = CheckHabitatZone( 
        p_zkn_sv,&arc_data.mons_data[i],
        ground_type,season_id,arc_data.season_enable );
    if( flg == TRUE )
    {
      buf[data_cnt] = arc_data.mons_data[i].monsno;
      data_cnt++;
      ONOUE_Printf("monsno:%d\n",buf[data_cnt]);
    }
  }

	*list = buf;
  
  // アーカイブハンドルのクローズ
  GFL_ARC_CloseDataHandle( p_handle );

	return max;
}

//----------------------------------------------------------------------------
/**
 *	@brief  生息地リストに乗っているモンスターかチェック
 *
 *	@param	* p_zkn_sv
 *	@param	*p_mons_data 
 *	@param	ground_type
 *	@param	season_id
 *	@param	season_enable 季節が存在するかどうか(TRUE=存在する)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckHabitatZone( 
    ZUKAN_SAVEDATA * p_zkn_sv,
    ARC_HABITAT_MONS_DATA *p_mons_data ,
    u16 ground_type,
    u16 season_id,
    BOOL season_enable)
{
  ;
  if(((ZUKANSAVE_GetPokeGetFlag( p_zkn_sv,p_mons_data->monsno )==TRUE) ||	      
        (ZUKANSAVE_GetPokeSeeFlag(p_zkn_sv, p_mons_data->monsno)==TRUE)) )
  {
    // 季節変化有りマップ
    if( season_enable == TRUE )
    {
      // 季節変化あり
      if( p_mons_data->is_appear[season_id][ground_type] == TRUE )
      {
        return TRUE;
      }
    }
    // 季節変化なしマップ
    else
    {
      if( p_mons_data->is_appear[PMSEASON_SPRING][ground_type] == TRUE )
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		デフォルトポケモンを登録されているものにする
 *
 * @param		sv		図鑑セーブデータ
 * @param		list	リストデータ
 * @param		max		リスト項目数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_InitDefaultMons( ZUKAN_SAVEDATA * sv, u16 * list, u16 max )
{
	u16	def_mons, init_mons;
	u16	i;

	init_mons = 0xffff;
	def_mons  = ZUKANSAVE_GetDefaultMons( sv );

	for( i=0; i<max; i++ ){
		if( ZUKANSAVE_GetPokeSeeFlag( sv, list[i] ) == TRUE ){
			if( list[i] == def_mons ){
				return;
			}
			if( init_mons == 0xffff ){
				init_mons = list[i];
			}
		}
	}

	if( init_mons != 0xffff ){
		ZUKANSAVE_SetDefaultMons( sv, init_mons );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ビクティチェック
 *
 * @param		sv		図鑑セーブデータ
 *
 * @retval	"TRUE = 表示する"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZKNCOMM_CheckLocalListNumberZero( ZUKAN_SAVEDATA * sv )
{
	// 表示モードが全国
	if( ZUKANSAVE_GetZukanMode( sv ) == ZUKAN_MODE_ZENKOKU ){
		return TRUE;
	}

	// 見た・捕獲した場合
	if( ZUKANSAVE_GetPokeGetFlag( sv, MONSNO_657 ) == TRUE ||
			ZUKANSAVE_GetPokeSeeFlag( sv, MONSNO_657 ) == TRUE ){
		return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ソートデータリセット
 *
 * @param		sv		図鑑セーブデータ
 * @param		sort	ソートデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_ResetSortData( const ZUKAN_SAVEDATA * sv, ZKNCOMM_LIST_SORT * sort )
{
	if( ZUKANSAVE_GetZukanMode( sv ) == ZUKAN_MODE_ZENKOKU ){
		sort->mode = ZKNCOMM_LIST_SORT_MODE_ZENKOKU;
	}else{
		sort->mode = ZKNCOMM_LIST_SORT_MODE_LOCAL;
	}
	sort->row   = ZKNCOMM_LIST_SORT_ROW_NUMBER;
	sort->name  = ZKNCOMM_LIST_SORT_NONE;
	sort->type1 = ZKNCOMM_LIST_SORT_NONE;
	sort->type2 = ZKNCOMM_LIST_SORT_NONE;
	sort->color = ZKNCOMM_LIST_SORT_NONE;
	sort->form  = ZKNCOMM_LIST_SORT_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードインセット
 *
 * @param		heapID		ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_SetFadeIn( HEAPID heapID )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウトセット
 *
 * @param		heapID		ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_SetFadeOut( HEAPID heapID )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		背景スクロール
 *
 * @param		mainBG		メイン画面のＢＧフレーム
 * @param		subBG			サブ画面のＢＧフレーム
 * @param		cnt				カウンタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_ScrollBaseBG( u8 mainBG, u8 subBG, u32 * cnt )
{
	*cnt += 1;
	if( *cnt == BASEBG_SCROLL_WAIT ){
		GFL_BG_SetScrollReq( mainBG, GFL_BG_SCROLL_X_INC, BASEBG_SCROLL_VAL );
		GFL_BG_SetScrollReq( subBG, GFL_BG_SCROLL_X_INC, BASEBG_SCROLL_VAL );
		*cnt = 0;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター追加
 *
 * @param		unit		GFL_CLUNIT
 * @param		prm			セルアクターデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	GFL_CLWK
 */
//--------------------------------------------------------------------------------------------
GFL_CLWK * ZKNCOMM_CreateClact( GFL_CLUNIT * unit, const ZKNCOMM_CLWK_DATA * prm, HEAPID heapID )
{
	return GFL_CLACT_WK_Create( unit, prm->chrRes, prm->palRes, prm->celRes, &prm->dat, prm->disp, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  生息地リスト用ゾーンリストデータを作成する
 *
 *	@param	* p_zone_info
 *	@param	single_zone_group_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void ZKNCOMM_MakeHabitatZoneListData( HABITATLIST_SAVE *p_sv,ZUKANHABITAT_ZONE_DATA * p_zone_info,BOOL is_set_single_zone )
{
  ZUKAN_SAVEDATA *p_save;
  s32 i;
  EVENTWORK *p_event_wk = GAMEDATA_GetEventWork( p_zone_info->p_gdata );
  BOOL is_vanish[HABITAT_GROUND_MAX]; //リストの表示状態

  // アーカイブハンドル取得
  ARCHANDLE	*p_handle	= 
      GFL_ARC_OpenDataHandle( ARCID_HABITAT_LIST_DATA , HEAPID_ZUKAN_SYS );

  // リストの表示状態取得
  is_vanish[HABITAT_GROUND_LAND] =  TRUE; // 地上は常にOK
  is_vanish[HABITAT_GROUND_WATERSIDE] =  // 波乗りが使える
    MISC_GetBadgeFlag( GAMEDATA_GetMiscWork(p_zone_info->p_gdata), BADGE_ID_05 );
  is_vanish[HABITAT_GROUND_FISHING] =  // 釣竿入手
    EVENTWORK_CheckEventFlag( p_event_wk ,SYS_FLAG_SUGOI_TSURI_GET);

  // セーブデータ取得
  p_save = GAMEDATA_GetZukanSave( p_zone_info->p_gdata );

  if( is_set_single_zone == FALSE )
  {
    // リストの長さをまず取得
    for( i = 0; i < HABITAT_LIST_DATA_MAX ; i++ )
    {
      const HABITAT_LIST_DATA *sc_habitat = &sc_habitat_list_data[i];

      BOOL is_set = EVENTWORK_CheckEventFlag( p_event_wk ,sc_habitat->sys_flag_id);

      if( is_set == TRUE )
      {
        ARC_HABITAT_ZONE_DATA arc_data;
        GFL_STD_MemClear( &arc_data ,sizeof(ARC_HABITAT_ZONE_DATA) );

        // アーカイブデータからデータを格納する
        GFL_ARC_LoadDataByHandle( p_handle , sc_habitat->arc_idx , &arc_data);

        // 地形が存在するかチェック
        if( CheckLandEnable( &arc_data ,is_vanish) )
        {
          // データ数を設定
          p_zone_info->zone_cnt ++;
        }
      }
    }
  }
  else
  {
    p_zone_info->zone_cnt = 1;
  }

  // データ数チェック
  if( p_zone_info->zone_cnt <= 0 )
  {
    // 生息地が１つもない時は、実行できない
    GF_ASSERT(0);
    return;
  }

  // データ格納先を確保
  GFL_STD_MemClear( p_zone_info->zone_list_data ,sizeof(p_zone_info->zone_list_data) );
  ONOUE_Printf("ゾーン数：%u\n",p_zone_info->zone_cnt);
  GF_ASSERT( p_zone_info->zone_cnt  <= HABITAT_LIST_DATA_MAX);
 
  // データ取得
  {
    s32 data_cnt = 0;

    for( i = 0; i < HABITAT_LIST_DATA_MAX ; i++ )
    {
      s32 n;
      const HABITAT_LIST_DATA *sc_habitat = &sc_habitat_list_data[i];
      BOOL is_set = EVENTWORK_CheckEventFlag( p_event_wk ,sc_habitat->sys_flag_id);

      // 指定ゾーンの時は、到着フラグとセーブデータに保存されているゾーングループを比較する
      if( is_set_single_zone == TRUE )
      {
        u16 default_zone_group_id = HABITATLIST_SAVE_GetDefaultZoneGroupID( p_sv );
        if( sc_habitat->zone_group_id != default_zone_group_id )
        {
          is_set = FALSE;
        }
        else
        {
          // 指定して作成する場合は、必ず到着フラグがたっている必要がある
          GF_ASSERT( is_set == TRUE);
        }
      }

      // 確保先にデータをセットする
      if( is_set == TRUE )
      {
        ARC_HABITAT_ZONE_DATA arc_data;
        GFL_STD_MemClear( &arc_data ,sizeof(ARC_HABITAT_ZONE_DATA) );

        // アーカイブデータからデータを格納する
        GFL_ARC_LoadDataByHandle( p_handle , sc_habitat->arc_idx , &arc_data);

        // 地形が存在するかチェック
        if( CheckLandEnable( &arc_data ,is_vanish) )
        {

          HABITAT_ZONE_POKE_DATA *p_poke_data;
          HABITAT_ZONE_DATA *p_zone_data = &p_zone_info->zone_list_data[data_cnt];


          // データ初期化
          GFL_STD_MemClear( p_zone_data ,sizeof(p_zone_data) );

          // 最大値チェック
          GF_ASSERT( arc_data.data_num <= HABITAT_ZONE_POKE_LIST_MAX );

          // ゾーンごとのデータ
          p_zone_data->zone_group = sc_habitat->zone_group_id;
          p_zone_data->season_enable = arc_data.season_enable;
          p_zone_data->info_ground_type = arc_data.ground_type;

          //        ONOUE_Printf("ゾーンID:%u\n",p_zone_data->zone_group);
          //        ONOUE_Printf("季節フラグ:%u\n",p_zone_data->season_enable);
          //        ONOUE_Printf("モンスター種類:%u\n",arc_data.data_num);
          //        ONOUE_Printf("地形タイプ:%u\n",arc_data.ground_type);

          // 地形コンプリート状態を設定する
          GetCompleteFlag( p_zone_data->is_complete ,&arc_data ,p_save );

          // ポケモンデータを設定
          for( n = 0; n < arc_data.data_num; n ++ )
          {
            u8 season_id,ground_id;
            p_poke_data = &p_zone_data->poke_data[n];
            p_poke_data->monsno = arc_data.mons_data[n].monsno;
            
            for( season_id = 0 ; season_id < PMSEASON_TOTAL; season_id ++ )
            {
              for( ground_id = 0 ; ground_id < HABITAT_GROUND_MAX; ground_id ++ )
              {
                p_poke_data->is_appear[season_id][ground_id] = 
                  (arc_data.mons_data[n].is_appear[season_id][ground_id] & is_vanish[ground_id]);
              }
            }

            // ゲット
            if( ZUKANSAVE_GetPokeGetFlag( p_save ,p_poke_data->monsno))
            {
              p_poke_data->status = HABITAT_POKE_STATUS_GET;
            }
            // 見た
            else if( ZUKANSAVE_GetPokeSeeFlag( p_save ,p_poke_data->monsno) )
            {
              p_poke_data->status = HABITAT_POKE_STATUS_LOOK;
            }
            // 知らない
            else
            {
              p_poke_data->status = HABITAT_POKE_STATUS_UNKNOWN;
            }
            //ONOUE_Printf("-----------------\n");
          }
          data_cnt++;
        }
      }
    }
    GFL_ARC_CloseDataHandle( p_handle );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  生息地リスト用リストデータ作成
 *
 *	@param	* p_zone_list 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void ZKNCOMM_MakeHabitatListData( HABITATLIST_SAVE *p_sv,ZUKANHABITAT_LIST_DATA * p_zone_list )
{
  s32 i;
  u8 season_id,ground_id;
  ZUKAN_SAVEDATA *p_save;
	BOOL	is_vanish[HABITAT_GROUND_MAX];

  // イベントワーク取得
  EVENTWORK *p_event_wk = GAMEDATA_GetEventWork( p_zone_list->p_gdata );
  
  // ミスクセーブワーク
  MISC* p_misc = GAMEDATA_GetMiscWork(p_zone_list->p_gdata);
  
  // アーカイブハンドル
  ARCHANDLE	*p_handle	= 
    GFL_ARC_OpenDataHandle( ARCID_HABITAT_LIST_DATA , HEAPID_ZUKAN_SYS );

  // セーブデータ取得
	SAVE_CONTROL_WORK *p_svcont =
                GAMEDATA_GetSaveControlWork( p_zone_list->p_gdata );
  HABITATLIST_SAVE *p_habitat_sv = SaveData_GetHabitatListSave( p_svcont );
  p_save = GAMEDATA_GetZukanSave( p_zone_list->p_gdata );

  // リスト状態取得
  is_vanish[HABITAT_GROUND_LAND] = TRUE; // 常にTRUE
  is_vanish[HABITAT_GROUND_WATERSIDE] =  // 波乗りが使える
    MISC_GetBadgeFlag( p_misc, BADGE_ID_05 );
  is_vanish[HABITAT_GROUND_FISHING] =  // 釣竿入手
    EVENTWORK_CheckEventFlag( p_event_wk ,SYS_FLAG_SUGOI_TSURI_GET);

  // リストの長さをまず取得
  for( i = 0; i < HABITAT_LIST_DATA_MAX ; i++ )
  {
    const HABITAT_LIST_DATA *sc_habitat = &sc_habitat_list_data[i];
    BOOL is_set = EVENTWORK_CheckEventFlag( p_event_wk ,sc_habitat->sys_flag_id);
    if( is_set == TRUE )
    {
      ARC_HABITAT_ZONE_DATA arc_data;
      GFL_STD_MemClear( &arc_data ,sizeof(ARC_HABITAT_ZONE_DATA) );

      // アーカイブデータからデータを格納する
      GFL_ARC_LoadDataByHandle( p_handle , sc_habitat->arc_idx , &arc_data);

      // 地形が存在するかチェック
      if( CheckLandEnable( &arc_data ,is_vanish) )
      {
        p_zone_list->list_size ++;
      }
    }
  }

  // リストデータ生成
  GFL_STD_MemClear( p_zone_list->list_data ,sizeof(p_zone_list->list_data) );
  GF_ASSERT( p_zone_list->list_size  <= HABITAT_LIST_DATA_MAX);

  {
    s32 i,n;
    s32 data_cnt = 0;
    ZUKANHABITAT_LIST_LISTDATA *p_ld;

    for( i = 0; i < HABITAT_LIST_DATA_MAX ; i++ )
    {
      const HABITAT_LIST_DATA *sc_habitat = &sc_habitat_list_data[i];
      BOOL is_set = EVENTWORK_CheckEventFlag( p_event_wk ,sc_habitat->sys_flag_id);

      if( is_set == TRUE )
      {
        ARC_HABITAT_ZONE_DATA arc_data;
        GFL_STD_MemClear( &arc_data ,sizeof(ARC_HABITAT_ZONE_DATA) );

        // アーカイブデータからデータを格納する
        GFL_ARC_LoadDataByHandle( p_handle , sc_habitat->arc_idx , &arc_data);

        // 地形チェック
        if( CheckLandEnable( &arc_data ,is_vanish) )
        {
          p_ld = &p_zone_list->list_data[data_cnt];

          // ゾーングループ
          p_ld->zone_group = sc_habitat->zone_group_id;

          // 地形有無
          p_ld->is_exist_landtype[HABITAT_GROUND_LAND]
            = arc_data.land_enable[HABITAT_GROUND_LAND];
          p_ld->is_exist_landtype[HABITAT_GROUND_WATERSIDE]
            = arc_data.land_enable[HABITAT_GROUND_WATERSIDE];
          p_ld->is_exist_landtype[HABITAT_GROUND_FISHING]
            = arc_data.land_enable[HABITAT_GROUND_FISHING];
  ONOUE_Printf("%d %d %d\n",p_ld->is_exist_landtype[ HABITAT_GROUND_LAND ],p_ld->is_exist_landtype[ HABITAT_GROUND_WATERSIDE ],p_ld->is_exist_landtype[ HABITAT_GROUND_FISHING ]);


          // リストの表示状態設定
          p_ld->is_vanish[HABITAT_GROUND_LAND] 
            = is_vanish[HABITAT_GROUND_LAND];
          p_ld->is_vanish[HABITAT_GROUND_WATERSIDE] 
            = is_vanish[HABITAT_GROUND_WATERSIDE];
          p_ld->is_vanish[HABITAT_GROUND_FISHING] 
            = is_vanish[HABITAT_GROUND_FISHING];

#ifdef PM_DEBUG
          {
            s32 hoge = 0;
            for( hoge = 0; hoge < HABITAT_GROUND_MAX ; hoge++ )
            {
              hoge += p_ld->is_exist_landtype[hoge] & p_ld->is_vanish[hoge];
            }
            GF_ASSERT(hoge);
          }
#endif
          // コンプリート状態
          {
            // 地形コンプリート状態を設定する
            GetCompleteFlag( p_ld->is_complete ,&arc_data ,p_save );

            // リスト状態(セーブと異なる場合は、びっくりマークに変更)
            for( ground_id = 0 ; ground_id < HABITAT_GROUND_MAX; ground_id++ )
            {
              HABITATLIST_STATUS list_status;
              list_status = HABITATLIST_SAVE_GetZoneStatus( p_habitat_sv,i,ground_id );

              if( p_ld->is_exist_landtype[ground_id] == TRUE )
              {
                switch( p_ld->is_complete[ground_id] )
                {
                case HABITAT_ZONE_STATUS_GET:
                  if( list_status == HABITAT_ZONE_STATUS_GET )
                  {
                    p_ld->list_status[ground_id] = HABITAT_LIST_STATUS_GET_CMP;
                  }
                  else
                  {
                    p_ld->list_status[ground_id] = HABITAT_LIST_STATUS_NOTICE;
                  }
                  break;
                case HABITAT_ZONE_STATUS_LOOK:
                  if( list_status == HABITAT_ZONE_STATUS_LOOK )
                  {
                    p_ld->list_status[ground_id] = HABITAT_LIST_STATUS_LOOK_CMP;
                  }
                  else
                  {
                    p_ld->list_status[ground_id] = HABITAT_LIST_STATUS_NOTICE;
                  }
                  break;
                case HABITAT_ZONE_STATUS_NONE:
                  p_ld->list_status[ground_id] = HABITAT_LIST_STATUS_NONE;
                  break;
                }
              }
              else
              {
                // 登録ポケモンなしの場合
                p_ld->list_status[ground_id] = HABITAT_LIST_STATUS_NONE;
              }
            }
          }
          data_cnt++;
        }
      }
    }
  }
  GFL_ARC_CloseDataHandle( p_handle );
}

//----------------------------------------------------------------------------
/**
 *	@brief  モンスターのコンプリート状態を取得する
 *
 *	@param	*p_dest
 *	@param	*p_arc_data 
 *	@param	*p_save
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetCompleteFlag( HABITATLIST_STATUS *p_dest,
    ARC_HABITAT_ZONE_DATA *p_arc_data ,ZUKAN_SAVEDATA *p_save)
{  
  s32 n;
  BOOL flg;
  u8 season_id,ground_id;
  u8 is_land_appear[HABITAT_GROUND_MAX];  // その地形にポケモンが出現するか
  p_dest[HABITAT_GROUND_LAND]      = HABITAT_ZONE_STATUS_GET;
  p_dest[HABITAT_GROUND_WATERSIDE] = HABITAT_ZONE_STATUS_GET;
  p_dest[HABITAT_GROUND_FISHING]   = HABITAT_ZONE_STATUS_GET;

  // 地形ごとに出現するかどうかをチェックする
  for( n = 0; n < p_arc_data->data_num ; n++ )
  {
    // 初期化
    GFL_STD_MemClear( is_land_appear ,sizeof(is_land_appear) );

    for( ground_id = 0 ; ground_id < HABITAT_GROUND_MAX; ground_id++ )
    {
      for( season_id = 0 ; season_id < PMSEASON_TOTAL; season_id++ )
      {
        is_land_appear[ground_id] |=
          p_arc_data->mons_data[n].is_appear[season_id][ground_id];
      }
    }

    // その地形に出現する場合は、捕獲状況をチェックする
    for( ground_id = 0 ; ground_id < HABITAT_GROUND_MAX; ground_id++ )
    {
      if( is_land_appear[ground_id] >= 1 )
      {
        // 捕獲チェック
        if( p_dest[ground_id] == HABITAT_ZONE_STATUS_GET )
        {
          flg=ZUKANSAVE_GetPokeGetFlag(p_save,p_arc_data->mons_data[n].monsno);
          if(!flg)
          {
            // 見たコンプリートに格下げ
            p_dest[ground_id] = HABITAT_ZONE_STATUS_LOOK;
          }
        }
        // 見たチェック
        if( p_dest[ground_id] == HABITAT_ZONE_STATUS_LOOK )
        {
          if(!ZUKANSAVE_GetPokeSeeFlag(p_save,p_arc_data->mons_data[n].monsno))
          {
            // 未コンプリートに格下げ
            p_dest[ground_id] = HABITAT_ZONE_STATUS_NONE;
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  地形が存在するかチェック
 *
 *	@param	*p_arc_data
 *	@param	*is_vanish 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckLandEnable( ARC_HABITAT_ZONE_DATA *p_arc_data,BOOL *is_vanish )
{
  s32 i;
  // 存在する地形を検索する
  for( i = 0; i < HABITAT_GROUND_MAX; i++ )
  {
    if( (p_arc_data->land_enable[i] == TRUE ) && 
        ( is_vanish[i] == TRUE ))
    {
      return TRUE;
    }
  }

  return FALSE;
}

