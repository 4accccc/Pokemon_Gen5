//=============================================================================
/**
 *
 *  @file   bag_param.c
 *  @brief
 *  @author   hosaka genya
 *  @data   2009.10.21
 *
 */
//=============================================================================
#include "app/bag.h"
#include "field/field_subscreen.h"

//=============================================================================
/**
 *                定数定義
 */
//=============================================================================

//=============================================================================
/**
 *                構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *              プロトタイプ宣言
 */
//=============================================================================

//=============================================================================
/**
 *                外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *  @brief  バッグパラメータ生成
 *
 *  @param  GAMEDATA* gmData  ゲームデータ
 *  @param  ITEMCHECK_WORK* icwk アイテムチェックワーク
 *  @param  mode バッグ起動モード
 *  @param  heap_id ヒープID
 *
 *  @retval BAG_PARAM* バッグパラメータ(ALLOC済み)
 */
//-----------------------------------------------------------------------------
BAG_PARAM* BAG_CreateParam( GAMEDATA* gmData, const ITEMCHECK_WORK* icwk, BAG_MODE mode, HEAPID heap_id )
{
  SAVE_CONTROL_WORK* saveControl;
  BAG_PARAM * bag;

  saveControl = GAMEDATA_GetSaveControlWork( gmData );

  bag = GFL_HEAP_AllocClearMemory( heap_id, sizeof(BAG_PARAM));

  bag->p_gamedata           = gmData;
  bag->p_config             = SaveData_GetConfig( saveControl );
  bag->p_mystatus           = GAMEDATA_GetMyStatus( gmData );
  bag->p_bagcursor          = GAMEDATA_GetBagCursor( gmData );
  bag->p_myitem             = GAMEDATA_GetMyItem( gmData );
  bag->repository_sort_type = MYITEM_GetRepositorySortType( bag->p_bagcursor );
  
  if( icwk != NULL){
    GFL_STD_MemCopy(icwk , &bag->icwk, sizeof(ITEMCHECK_WORK));
  }

  bag->mode       = mode;
  bag->cycle_flg  = ( PLAYERWORK_GetMoveForm( GAMEDATA_GetMyPlayerWork(gmData) ) == PLAYER_MOVE_FORM_CYCLE );

  // ダウジングを使用中かどうか判定
  if( FIELD_SUBSCREEN_DOWSING == GAMEDATA_GetSubScreenMode(gmData) )
  {
    bag->dowsing_flg = TRUE;
    ONOUE_Printf("ダウジング起動中\n");
  }
  else
  {
    bag->dowsing_flg = FALSE;
    ONOUE_Printf("ダウジング未起動\n");
  }
  
  return bag;
}


