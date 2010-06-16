//============================================================================
/**
 *  @file   manual_data.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL_DATA
 */
//============================================================================


// デバッグ機能
//#define DEBUG_OPEN_FLAG_ALL_ON    // これが定義されているとき、オープンフラグが全てONになっている
//#define DEBUG_READ_FLAG_ALL_OFF   // これが定義されているとき、リードフラグが全てOFFになっている
//#define DEBUG_READ_FLAG_ALL_ON    // これが定義されているとき、リードフラグが全てONになっている


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_data.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "app/app_taskmenu.h"

#include "../../../../resource/fldmapdata/flagwork/flag_define.h"
#include "savedata/save_control.h"
#include "savedata/c_gear_data.h"
#include "savedata/etc_save.h"
#include "field/eventwork.h"

#include "manual_graphic.h"
#include "manual_def.h"
#include "manual_common.h"
#include "manual_data.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_manual.h"


// リソースで生成したヘッダ
#include "../../../../resource/manual/manual_open_flag.h"


// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// u16の配列添え字

// カテゴリ開始場所ファイル
#define CATE_REF_U16_ARRAY_IDX_TOTAL_CATE_NUM             (0)
#define CATE_REF_U16_ARRAY_IDX_CATE_BYTE_POS_START        (1)

// カテゴリファイル中の1カテゴリデータ
#define CATE_DATA_U16_ARRAY_IDX_CATE_NO                   (0)
#define CATE_DATA_U16_ARRAY_IDX_CATE_GMM_ID               (1)
#define CATE_DATA_U16_ARRAY_IDX_TITLE_NUM                 (2)
#define CATE_DATA_U16_ARRAY_IDX_TITLE_IDX_START           (3)

// タイトル開始場所ファイル
#define TITLE_REF_U16_ARRAY_IDX_TOTAL_TITLE_NUM           (0)
#define TITLE_REF_U16_ARRAY_IDX_TITLE_BYTE_POS_START      (1)

// タイトルファイル中の1タイトルデータ
#define TITLE_DATA_U16_ARRAY_IDX_CATE_NO                  (0)
#define TITLE_DATA_U16_ARRAY_IDX_TITLE_NO                 (1)
#define TITLE_DATA_U16_ARRAY_IDX_SERIAL_NO                (2)
#define TITLE_DATA_U16_ARRAY_IDX_TITLE_GMM_ID             (3)
#define TITLE_DATA_U16_ARRAY_IDX_OPEN_FLAG                (4)
#define TITLE_DATA_U16_ARRAY_IDX_READ_FLAG                (5)
#define TITLE_DATA_U16_ARRAY_IDX_PAGE_NUM                 (6)
#define TITLE_DATA_U16_ARRAY_IDX_PAGE_START               (7)

#define TITLE_DATA_U16_ARRAY_IDX_PAGE_DATA_TOTAL          (2)  // ページのデータの合計はgmmのIDと画像のIDの2つ
#define TITLE_DATA_U16_ARRAY_IDX_PAGE_DATA_GMM_ID         (0)  // ページのデータのうちgmmのIDの位置
#define TITLE_DATA_U16_ARRAY_IDX_PAGE_DATA_IMAGE_ID       (1)  // ページのデータのうち画像のIDの位置


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
struct  _MANUAL_DATA_WORK
{
  u16* title_file;       // タイトルファイル
  u16* title_ref_file;   // タイトル開始場所ファイル
  u16* cate_file;        // カテゴリファイル
  u16* cate_ref_file;    // カテゴリ開始場所ファイル
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// タイトルファイル
static  u16* Manual_Data_TitleGetTitleData( MANUAL_DATA_WORK* work, u16 title_idx );

// タイトル開始場所ファイル
static  u16  Manual_Data_TitleRefGetTitleBytePos( MANUAL_DATA_WORK* work, u16 title_idx );

// カテゴリファイル
static  u16* Manual_Data_CateGetCateData( MANUAL_DATA_WORK* work, u16 cate_idx );

// カテゴリ開始場所ファイル
static  u16  Manual_Data_CateRefGetCateBytePos( MANUAL_DATA_WORK* work, u16 cate_idx );


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//-------------------------------------
/// Load処理
//=====================================
MANUAL_DATA_WORK*  MANUAL_DATA_Load( ARCHANDLE* handle, HEAPID heap_id )
{
  MANUAL_DATA_WORK* work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(MANUAL_DATA_WORK) ); 
  u32 size;
  
  work->title_file     = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_manual_manual_data_title_dat,     FALSE, heap_id, &size );
  work->title_ref_file = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_manual_manual_data_title_ref_dat, FALSE, heap_id, &size );
  work->cate_file      = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_manual_manual_data_cate_dat,      FALSE, heap_id, &size );
  work->cate_ref_file  = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_manual_manual_data_cate_ref_dat,  FALSE, heap_id, &size );

  return work;
}

//-------------------------------------
/// Unload処理
//=====================================
void               MANUAL_DATA_Unload( MANUAL_DATA_WORK* work )
{
  GFL_HEAP_FreeMemory( work->cate_ref_file );
  GFL_HEAP_FreeMemory( work->cate_file );
  GFL_HEAP_FreeMemory( work->title_ref_file );
  GFL_HEAP_FreeMemory( work->title_file );

  GFL_HEAP_FreeMemory( work );
}

//-------------------------------------
/// タイトルファイル
//=====================================
u16  MANUAL_DATA_TitleGetTitleGmmId( MANUAL_DATA_WORK* work, u16 title_idx )
                                          // title_idxは全タイトルにおける出席番号
                                          // title_idxタイトルのgmmのIDを戻り値として返す
{
  u16* title_data = Manual_Data_TitleGetTitleData( work, title_idx );
  u16  title_gmm_id = title_data[TITLE_DATA_U16_ARRAY_IDX_TITLE_GMM_ID];
  return title_gmm_id;
}
u16  MANUAL_DATA_TitleGetOpenFlag( MANUAL_DATA_WORK* work, u16 title_idx )
{
  u16* title_data = Manual_Data_TitleGetTitleData( work, title_idx );
  u16  open_flag = title_data[TITLE_DATA_U16_ARRAY_IDX_OPEN_FLAG];
  return open_flag;
}
u16  MANUAL_DATA_TitleGetReadFlag( MANUAL_DATA_WORK* work, u16 title_idx )
{
  u16* title_data = Manual_Data_TitleGetTitleData( work, title_idx );
  u16  read_flag = title_data[TITLE_DATA_U16_ARRAY_IDX_READ_FLAG];
  return read_flag;
}
u16  MANUAL_DATA_TitleGetPageNum( MANUAL_DATA_WORK* work, u16 title_idx )
{
  u16* title_data = Manual_Data_TitleGetTitleData( work, title_idx );
  u16  page_num = title_data[TITLE_DATA_U16_ARRAY_IDX_PAGE_NUM];
  return page_num;
}
u16  MANUAL_DATA_TitleGetPageGmmId( MANUAL_DATA_WORK* work, u16 title_idx, u16 page_order )
                                          // title_idxは全タイトルにおける出席番号
                                          // title_idxタイトル内におけるpage_order番目のページの、gmmのIDを戻り値として返す
{
  u16* title_data = Manual_Data_TitleGetTitleData( work, title_idx );
  u16  page_gmm_id = title_data[TITLE_DATA_U16_ARRAY_IDX_PAGE_START + TITLE_DATA_U16_ARRAY_IDX_PAGE_DATA_TOTAL * page_order + TITLE_DATA_U16_ARRAY_IDX_PAGE_DATA_GMM_ID];
  return page_gmm_id;
}
u16  MANUAL_DATA_TitleGetPageImageId( MANUAL_DATA_WORK* work, u16 title_idx, u16 page_order )
                                          // title_idxは全タイトルにおける出席番号
                                          // title_idxタイトル内におけるpage_order番目のページの、画像のIDを戻り値として返す
{
  u16* title_data = Manual_Data_TitleGetTitleData( work, title_idx );
  u16  page_image_id = title_data[TITLE_DATA_U16_ARRAY_IDX_PAGE_START + TITLE_DATA_U16_ARRAY_IDX_PAGE_DATA_TOTAL * page_order + TITLE_DATA_U16_ARRAY_IDX_PAGE_DATA_IMAGE_ID];
  return page_image_id;
}

//-------------------------------------
/// タイトル開始場所ファイル
//=====================================
u16  MANUAL_DATA_TitleRefGetTotalTitleNum( MANUAL_DATA_WORK* work )
{
  u16 total_title_num = work->title_ref_file[TITLE_REF_U16_ARRAY_IDX_TOTAL_TITLE_NUM];
  return total_title_num;
}

//-------------------------------------
/// カテゴリファイル
//=====================================
u16  MANUAL_DATA_CateGetCateGmmId( MANUAL_DATA_WORK* work, u16 cate_idx )
                                          // cate_idxは全カテゴリにおける出席番号
                                          // cate_idxカテゴリのgmmのIDを戻り値として返す
{
  u16* cate_data = Manual_Data_CateGetCateData( work, cate_idx );
  u16  cate_gmm_id = cate_data[CATE_DATA_U16_ARRAY_IDX_CATE_GMM_ID];
  return cate_gmm_id;
}
u16  MANUAL_DATA_CateGetTitleNum( MANUAL_DATA_WORK* work, u16 cate_idx )
                                          // cate_idxは全カテゴリにおける出席番号
                                          // cate_idxカテゴリ内におけるタイトルの個数を戻り値として返す
{
  u16* cate_data = Manual_Data_CateGetCateData( work, cate_idx );
  u16  title_num = cate_data[CATE_DATA_U16_ARRAY_IDX_TITLE_NUM];
  return title_num;
}
u16  MANUAL_DATA_CateGetTitleIdx( MANUAL_DATA_WORK* work, u16 cate_idx, u16 title_order )
                                          // cate_idxは全カテゴリにおける出席番号
                                          // cate_idxカテゴリ内におけるtitle_order番目のタイトルの、全タイトルにおける出席番号を戻り値として返す
{
  u16* cate_data = Manual_Data_CateGetCateData( work, cate_idx );
  u16  title_idx = cate_data[CATE_DATA_U16_ARRAY_IDX_TITLE_IDX_START + title_order];
  return title_idx;
}

//-------------------------------------
/// カテゴリ開始場所ファイル
//=====================================
u16  MANUAL_DATA_CateRefGetTotalCateNum( MANUAL_DATA_WORK* work )
{
  u16 total_cate_num = work->cate_ref_file[CATE_REF_U16_ARRAY_IDX_TOTAL_CATE_NUM];
  return total_cate_num;
}

//-------------------------------------
/// オープンフラグ
//=====================================
BOOL MANUAL_DATA_OpenFlagIsOpen( MANUAL_DATA_WORK* work, u16 open_flag, GAMEDATA* gamedata )
{
#ifdef DEBUG_OPEN_FLAG_ALL_ON
  return TRUE;
#else

  BOOL is_open = FALSE;
  switch( open_flag )
  {
  case MANUAL_OPEN_FLAG_START:
    {
      is_open = TRUE;
    }
    break;
  case MANUAL_OPEN_FLAG_CGERA:
    {
      if( CGEAR_SV_GetCGearONOFF( CGEAR_SV_GetCGearSaveData(GAMEDATA_GetSaveControlWork(gamedata)) ) )
      {
        is_open = TRUE;
      }
    }
    break;
  case MANUAL_OPEN_FLAG_NCLEAR:
    {
      if( EVENTWORK_CheckEventFlag(GAMEDATA_GetEventWork(gamedata), SYS_FLAG_GAME_CLEAR) )
      {
        is_open = TRUE;
      }
    }
    break;
  }
  return is_open;

#endif
}

//-------------------------------------
/// リードフラグ
//=====================================
BOOL MANUAL_DATA_ReadFlagIsRead( MANUAL_DATA_WORK* work, u16 read_flag, GAMEDATA* gamedata )
{
#if defined DEBUG_READ_FLAG_ALL_OFF
  return FALSE;
#elif defined DEBUG_READ_FLAG_ALL_ON
  return TRUE;
#else

  ETC_SAVE_WORK* etc_sv = SaveData_GetEtc(GAMEDATA_GetSaveControlWork(gamedata));
  BOOL is_read = EtcSave_GetManualFlag( etc_sv, read_flag );
  return is_read;

#endif
}
void MANUAL_DATA_ReadFlagSetRead( MANUAL_DATA_WORK* work, u16 read_flag, GAMEDATA* gamedata )
{
  ETC_SAVE_WORK* etc_sv = SaveData_GetEtc(GAMEDATA_GetSaveControlWork(gamedata));
  EtcSave_SetManualFlag( etc_sv, read_flag );
}

//-------------------------------------
/// 画像のID
//=====================================
BOOL MANUAL_DATA_ImageIdIsValid( MANUAL_DATA_WORK* work, u16 image_id )  // 画像のIDが有効(画像あり)か無効(画像なし)かを返す(有効のときTRUE)
{
  if( image_id != MANUAL_DATA_IMAGE_NONE ) return TRUE;
  return FALSE;
}
u16  MANUAL_DATA_ImageIdGetNoImage( MANUAL_DATA_WORK* work )  // 画像なしのときに表示する画像のIDを得る
{
  return MANUAL_DATA_NO_IMAGE_ID;
}


//------------------------------------------------------------------
/**
 *  @brief           
 *
 *  @param[in]       
 *  @param[in,out]       
 *
 *  @retval          
 */
//------------------------------------------------------------------

//-------------------------------------
/// 
//=====================================


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================

//-------------------------------------
/// タイトルファイル
//=====================================
static  u16* Manual_Data_TitleGetTitleData( MANUAL_DATA_WORK* work, u16 title_idx )
{
  u16  title_byte_pos = Manual_Data_TitleRefGetTitleBytePos( work, title_idx );
  u8*  title_file_byte = (u8*)(work->title_file);
  u16* title_data = (u16*)( &(title_file_byte[title_byte_pos]) );
  return title_data;
}

//-------------------------------------
/// タイトル開始場所ファイル
//=====================================
static  u16  Manual_Data_TitleRefGetTitleBytePos( MANUAL_DATA_WORK* work, u16 title_idx )
{
  u16* title_byte_pos = &(work->title_ref_file[TITLE_REF_U16_ARRAY_IDX_TITLE_BYTE_POS_START]);
  return title_byte_pos[title_idx];
}

//-------------------------------------
/// カテゴリファイル
//=====================================
static  u16* Manual_Data_CateGetCateData( MANUAL_DATA_WORK* work, u16 cate_idx )
{
  u16  cate_byte_pos = Manual_Data_CateRefGetCateBytePos( work, cate_idx );
  u8*  cate_file_byte = (u8*)(work->cate_file);
  u16* cate_data = (u16*)( &(cate_file_byte[cate_byte_pos]) );
  return cate_data;
}

//-------------------------------------
/// カテゴリ開始場所ファイル
//=====================================
static  u16  Manual_Data_CateRefGetCateBytePos( MANUAL_DATA_WORK* work, u16 cate_idx )
{
  u16* cate_byte_pos = &(work->cate_ref_file[CATE_REF_U16_ARRAY_IDX_CATE_BYTE_POS_START]);
  return cate_byte_pos[cate_idx];
}


