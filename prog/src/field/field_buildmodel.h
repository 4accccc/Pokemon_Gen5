//============================================================================================
/**
 * @file	field_buildmodel.h
 * @brief		配置モデルの制御
 * @author	tamada GAMEFREAK inc.
 * @date	2009.04.24
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "field/areadata.h"
#include "field_g3d_mapper.h"

#include "map/dp3format.h"
#include "field_hit_check.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 配置モデルIDの型定義
 */
//------------------------------------------------------------------
typedef u16 BMODEL_ID;
//------------------------------------------------------------------
//  配置モデルマネジャー
//------------------------------------------------------------------
typedef struct _FIELD_BMODEL_MAN FIELD_BMODEL_MAN;

//============================================================================================
//
//  メイン処理関数群
//
//============================================================================================
//------------------------------------------------------------------
//  生成
//------------------------------------------------------------------
extern FIELD_BMODEL_MAN * FIELD_BMODEL_MAN_Create(HEAPID heapID, FLDMAPPER * fldmapper);

//------------------------------------------------------------------
//  削除
//------------------------------------------------------------------
extern void FIELD_BMODEL_MAN_Delete(FIELD_BMODEL_MAN * man);

//------------------------------------------------------------------
//  更新処理
//------------------------------------------------------------------
extern void FIELD_BMODEL_MAN_Main(FIELD_BMODEL_MAN * man);

//------------------------------------------------------------------
//  描画処理
//------------------------------------------------------------------
extern void FIELD_BMODEL_MAN_Draw(FIELD_BMODEL_MAN * man);

//------------------------------------------------------------------
//  マップデータの読み込み処理
//------------------------------------------------------------------
extern void FIELD_BMODEL_MAN_Load(FIELD_BMODEL_MAN * man, u16 zoneid, const AREADATA * areadata);

//------------------------------------------------------------------
/// ヒープ指定の取得
//------------------------------------------------------------------
extern HEAPID FIELD_BMODEL_MAN_GetHeapID(const FIELD_BMODEL_MAN * man);

//============================================================================================
//
//  配置登録関連
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern GFL_G3D_MAP_GLOBALOBJ * FIELD_BMODEL_MAN_GetGlobalObjects(FIELD_BMODEL_MAN * man);

extern int FIELD_BMODEL_MAN_ResistAllMapObjects
(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap, const PositionSt* objStatus, u32 objCount);

extern void FIELD_BMODEL_MAN_ReleaseAllMapObjects
(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap);

extern void FIELD_BMODEL_MAN_ResistMapObject
(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap, const PositionSt* objStatus, u32 objCount);

//============================================================================================
//
//  アニメ制御関連
//
//============================================================================================
//------------------------------------------------------------------
/// アニメリクエスト用定義
//------------------------------------------------------------------
typedef enum {
  BMANM_REQ_START = 0,
  BMANM_REQ_REVERSE_START,
  BMANM_REQ_LOOP,
  BMANM_REQ_STOP,
  BMANM_REQ_END,

  BMANM_REQ_MAX,
}BMANM_REQUEST;

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  //アニメデータの定義順に依存している。
  BMANM_INDEX_DOOR_OPEN = 0,
  BMANM_INDEX_DOOR_CLOSE,

  BMANM_INDEX_MAX = 4,

  BMANM_INDEX_NULL = 0xffff,
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief プログラム指定ID
 * @note
 * コンバータから参照されているので記述ルールには注意。
 */
//------------------------------------------------------------------
typedef enum {
  BM_PROG_ID_NONE = 0,
  BM_PROG_ID_DOOR_NORMAL,
  BM_PROG_ID_DOOR_AUTO,
  BM_PROG_ID_BADGEGATE,
  BM_PROG_ID_SANDSTREAM,
  BM_PROG_ID_PCELEVATOR,
  BM_PROG_ID_PCMACHINE,
  BM_PROG_ID_PC,
  BM_PROG_ID_PCEV_DOOR,
  BM_PROG_ID_PCEV_FLOOR,
  BM_PROG_ID_WARP,
  BM_PROG_ID_EVENT,

  BM_PROG_ID_MAX,
}BM_PROG_ID;
//------------------------------------------------------------------
/**
 * @brief 検索用指定ID
 * @note
 * BM_PROG_IDをさらにグループ化したもの。
 * スクリプトのSCR_BMID_と一対一対応なので注意
 */
//------------------------------------------------------------------
typedef enum {
  BM_SEARCH_ID_NULL = 0,
  BM_SEARCH_ID_DOOR,
  BM_SEARCH_ID_SANDSTREAM,
  BM_SEARCH_ID_PCMACHINE,
  BM_SEARCH_ID_PC,
  BM_SEARCH_ID_PCEV_DOOR,
  BM_SEARCH_ID_PCEV_FLOOR,
  BM_SEARCH_ID_WARP,
  BM_SEARCH_ID_EVENT,

  BM_SEARCH_ID_MAX,
}BM_SEARCH_ID;

//------------------------------------------------------------------
/// 配置モデル実データアクセス用オブジェクト
//------------------------------------------------------------------
typedef struct _G3DMAPOBJST G3DMAPOBJST;

//------------------------------------------------------------------
/// 配置モデル制御データ
//------------------------------------------------------------------
typedef struct _FIELD_BMODEL FIELD_BMODEL;

//============================================================================================
//
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern u16 FIELD_BMODEL_MAN_GetUniqKey( FIELD_BMODEL_MAN * man, const FIELD_BMODEL * bmodel );

extern FIELD_BMODEL * FIELD_BMODEL_MAN_GetBModelByUniqKey(
    FIELD_BMODEL_MAN * man, u16 uniq_key );
//------------------------------------------------------------------
//  配置モデル：生成
//------------------------------------------------------------------
extern FIELD_BMODEL * FIELD_BMODEL_Create(FIELD_BMODEL_MAN * man, G3DMAPOBJST * obj);
extern FIELD_BMODEL * FIELD_BMODEL_Create_Direct(
    FIELD_BMODEL_MAN * man, BMODEL_ID bmodel_id, const GFL_G3D_OBJSTATUS* status );
extern FIELD_BMODEL * FIELD_BMODEL_Create_Search(
    FIELD_BMODEL_MAN * bmodel_man, BM_SEARCH_ID id, const VecFx32 * pos );

//------------------------------------------------------------------
//  配置モデル：削除
//------------------------------------------------------------------
extern void FIELD_BMODEL_Delete(FIELD_BMODEL * bmodel);

//------------------------------------------------------------------
//  配置モデル：制御
//------------------------------------------------------------------
extern void FIELD_BMODEL_SetAnime(FIELD_BMODEL * bmodel, u32 idx, BMANM_REQUEST req);
extern void FIELD_BMODEL_SetCurrentAnime( FIELD_BMODEL * bmodel, BMANM_REQUEST req );
inline static void FIELD_BMODEL_StartAnime( FIELD_BMODEL * bmodel, u32 idx )
{
  FIELD_BMODEL_SetAnime( bmodel, idx, BMANM_REQ_START );
}
inline static void FIELD_BMODEL_StartCurrentAnime( FIELD_BMODEL * bmodel )
{
  FIELD_BMODEL_SetCurrentAnime( bmodel, BMANM_REQ_START );
}
inline static void FIELD_BMODEL_StopAnime( FIELD_BMODEL * bmodel, u32 idx )
{
  FIELD_BMODEL_SetAnime( bmodel, idx, BMANM_REQ_STOP );
}
inline static void FIELD_BMODEL_StopCurrentAnime( FIELD_BMODEL * bmodel )
{
  FIELD_BMODEL_SetCurrentAnime( bmodel, BMANM_REQ_STOP );
}

extern BOOL FIELD_BMODEL_WaitAnime( FIELD_BMODEL * bmodel, u32 idx );
extern BOOL FIELD_BMODEL_WaitCurrentAnime( FIELD_BMODEL * bmodel );

extern BOOL FIELD_BMODEL_GetAnimeStatus( const FIELD_BMODEL * bmodel, u32 idx);
extern BOOL FIELD_BMODEL_GetCurrentAnimeStatus( const FIELD_BMODEL * bmodel );
extern BM_PROG_ID FIELD_BMODEL_GetProgID(const FIELD_BMODEL * bmodel);
extern const G3DMAPOBJST * FIELD_BMODEL_GetG3DMAPOBJST( const FIELD_BMODEL * bmodel );

extern BOOL FIELD_BMODEL_GetSENo(const FIELD_BMODEL * bmodel, u32 anm_idx, u16 * se_no);
extern BOOL FIELD_BMODEL_GetCurrentSENo( const FIELD_BMODEL * bmodel, u16 * se_no );
extern BOOL FIELD_BMODEL_CheckCurrentSE( const FIELD_BMODEL * bmodel );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//  配置モデル実データ：検索関数
//------------------------------------------------------------------
extern G3DMAPOBJST ** FIELD_BMODEL_MAN_CreateObjStatusList
( FIELD_BMODEL_MAN* man, const FLDHIT_RECT * rect, BM_SEARCH_ID search, u32 * num );

extern G3DMAPOBJST * FIELD_BMODEL_MAN_SearchObjStatusRect(
    FIELD_BMODEL_MAN * bmodel_man, BM_SEARCH_ID id, const FLDHIT_RECT * rect );
extern G3DMAPOBJST * FIELD_BMODEL_MAN_SearchObjStatusPos(
    FIELD_BMODEL_MAN * bmodel_man, BM_SEARCH_ID id, const VecFx32 * pos );

//------------------------------------------------------------------
//  配置モデル実データ：アクセス関数
//------------------------------------------------------------------
extern BM_SEARCH_ID G3DMAPOBJST_getSearchID(
    const FIELD_BMODEL_MAN * man, const G3DMAPOBJST * obj );
extern void G3DMAPOBJST_changeViewFlag(G3DMAPOBJST * obj, BOOL flag);
extern void G3DMAPOBJST_setAnime(
    FIELD_BMODEL_MAN * man, G3DMAPOBJST * obj, u32 anm_idx, BMANM_REQUEST req );
extern void G3DMAPOBJST_getPos(G3DMAPOBJST * obj, VecFx32 * dst);

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//  ↓このヘッダで宣言しているが実態はfield_g3d_mapper.cにある
//------------------------------------------------------------------
extern FIELD_BMODEL_MAN * FLDMAPPER_GetBuildModelManager( FLDMAPPER* g3Dmapper);


#ifdef PM_DEBUG

#define BMODEL_DEBUG_RESOURCE_MEMORY_SIZE // ONでメモリサイズを計算

// 情報を表示
extern void FIELD_BMODEL_DEBUG_ScriptPrintInfo( const FIELD_BMODEL * bmodel );

#endif


//============================================================================================
//  DEBUG リソースメモリ使用量の検査
//============================================================================================
#ifdef BMODEL_DEBUG_RESOURCE_MEMORY_SIZE

extern u32 FIELD_BMODEL_MAN_GetUseResourceMemorySize(void);

#endif

