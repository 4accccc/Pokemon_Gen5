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

extern void FIELD_BMODEL_MAN_ResistAllMapObjects
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
typedef enum {
  BMANM_REQ_START = 0,
  BMANM_REQ_REVERSE_START,
  BMANM_REQ_STOP,
  BMANM_REQ_END,
}BMANM_REQUEST;


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/// プログラム指定ID
//------------------------------------------------------------------
typedef enum {
  BM_PROG_ID_NONE = 0,
  BM_PROG_ID_DOOR_NORMAL,
  BM_PROG_ID_DOOR_AUTO,
  BM_PROG_ID_BADGEGATE,
  BM_PROG_ID_SANDSTREAM,
  BM_PROG_ID_PCELEVATOR,

  BM_PROG_ID_MAX,
}BM_PROG_ID;
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
  BM_SEARCH_ID_NULL = 0,
  BM_SEARCH_ID_DOOR,
  BM_SEARCH_ID_SANDSTREAM,

  BM_SEARCH_ID_MAX,
}BM_SEARCH_ID;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _G3DMAPOBJST G3DMAPOBJST;
typedef struct _FIELD_BMODEL FIELD_BMODEL;

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_BMODEL_MAN_EntryBuildModel(FIELD_BMODEL_MAN * man, FIELD_BMODEL * bmodel);
extern void FIELD_BMODEL_MAN_releaseBuildModel(FIELD_BMODEL_MAN * man, FIELD_BMODEL * bmodel);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern FIELD_BMODEL * FIELD_BMODEL_Create(FIELD_BMODEL_MAN * man, const G3DMAPOBJST * obj);
extern FIELD_BMODEL * FIELD_BMODEL_Create_Direct(
    FIELD_BMODEL_MAN * man, BMODEL_ID bmodel_id, const GFL_G3D_OBJSTATUS* status );
extern void FIELD_BMODEL_Delete(FIELD_BMODEL * bmodel);
extern void FIELD_BMODEL_SetAnime(FIELD_BMODEL * bmodel, u32 idx, BMANM_REQUEST req);
extern BOOL FIELD_BMODEL_GetAnimeStatus( const FIELD_BMODEL * bmodel, u32 idx);
extern BM_PROG_ID FIELD_BMODEL_GetProgID(const FIELD_BMODEL * bmodel);


//------------------------------------------------------------------
//------------------------------------------------------------------

extern G3DMAPOBJST ** FIELD_BMODEL_MAN_CreateObjStatusList
( FIELD_BMODEL_MAN* man, const FLDHIT_RECT * rect, BM_SEARCH_ID search, u32 * num );

extern void G3DMAPOBJST_changeViewFlag(G3DMAPOBJST * obj, BOOL flag);
extern void G3DMAPOBJST_setAnime( FIELD_BMODEL_MAN * man, G3DMAPOBJST * obj, u32 anm_idx, BMANM_REQUEST req );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//  ↓このヘッダで宣言しているが実態はfield_g3d_mapper.cにある
//------------------------------------------------------------------
extern FIELD_BMODEL_MAN * FLDMAPPER_GetBuildModelManager( FLDMAPPER* g3Dmapper);


#ifdef PM_DEBUG

#define BMODEL_DEBUG_RESOURCE_MEMORY_SIZE // ONでメモリサイズを計算

#endif


//============================================================================================
//  DEBUG リソースメモリ使用量の検査
//============================================================================================
#ifdef BMODEL_DEBUG_RESOURCE_MEMORY_SIZE

extern u32 FIELD_BMODEL_MAN_GetUseResourceMemorySize(void);

#endif

