//==============================================================================
/**
 * @file    union_eff.c
 * @brief   ユニオンルーム：エフェクト類
 * @author  matsuda
 * @date    2010.01.17(日)
 */
//==============================================================================
#include <gflib.h>
#include "union_types.h"
#include "union_local.h"
#include "net_app/union/union_main.h"
#include "field/field_player.h"
#include "field/fieldmap.h"
#include "unionroom.naix"
#include "net_app/union_eff.h"


//==============================================================================
//  定数定義
//==============================================================================
///同時にフォーカス出来る最大数
#define FOCUS_MAX     (4)
///フォーカスエフェクトのアニメリソース数
#define FOCUS_ANM_RESOURCE_MAX    (1)
///フォーカスエフェクトの寿命
#define FOCUS_LIFE    (30 * 3)


//==============================================================================
//  構造体定義
//==============================================================================
//--------------------------------------------------------------
//  フォーカスエフェクト
//--------------------------------------------------------------
///フォーカスエフェクトワーク
typedef struct _UNION_EFF_FOCUS{
  MMDL *mmdl;           ///フォーカス対象の動作モデルへのポインタ
  GFL_G3D_RND *rnder;   //G3Dレンダー
  GFL_G3D_ANM *anm[FOCUS_ANM_RESOURCE_MAX];     //G3Dアニメーションハンドル
  GFL_G3D_OBJ *g3dobj;  //G3Dオブジェクト
  u16 eff_no;           ///<何番目に生成されたエフェクトか
  u16 timer;            ///<生成されてからのフレーム数をカウント
}UNION_EFF_FOCUS;

///フォーカスエフェクト制御ワーク
typedef struct _UNION_EFF_FOCUS_MANAGER{
  UNION_EFF_FOCUS focus[FOCUS_MAX];   ///フォーカスエフェクトワーク
  GFL_G3D_RES *res_nsbmd;
  GFL_G3D_RES *res_anm[FOCUS_ANM_RESOURCE_MAX];
  u16 eff_no_count;                   ///<何個エフェクトを作成してきたかをカウント
  u16 padding;
}UNION_EFF_FOCUS_MANAGER;

//--------------------------------------------------------------
//  エフェクト全体制御
//--------------------------------------------------------------
///ユニオンエフェクト制御システムワーク
struct _UNION_EFF_SYSTEM{
  UNION_EFF_FOCUS_MANAGER focus_man;    ///<フォーカスエフェクト制御ワーク
};


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void UnionEff_Focus_ResourceSet(UNION_EFF_FOCUS_MANAGER *focus_man, HEAPID heap_id);
static void UnionEff_Focus_ResourceDelete(UNION_EFF_FOCUS_MANAGER *focus_man);
static void UnionEff_Focus_Setup(UNION_EFF_FOCUS_MANAGER *focus_man, MMDL *mmdl);
static void UnionEff_Focus_Create(UNION_EFF_FOCUS_MANAGER *focus_man, UNION_EFF_FOCUS *focus, MMDL *mmdl, u16 eff_no);
static void UnionEff_Focus_Delete(UNION_EFF_FOCUS *focus);
static void UnionEff_Focus_Update(UNION_EFF_FOCUS *focus);
static void UnionEff_Focus_Draw( UNION_EFF_FOCUS *focus, GFL_G3D_OBJSTATUS *status );
static void UnionEff_Focus_DeleteAll(UNION_EFF_FOCUS_MANAGER *focus_man);
static void UnionEff_Focus_UpdateAll(UNION_EFF_FOCUS_MANAGER *focus_man);
static void UnionEff_Focus_DrawAll( UNION_EFF_FOCUS_MANAGER *focus_man);
static UNION_EFF_FOCUS * _SearchFocusWork(UNION_EFF_FOCUS_MANAGER *focus_man);



//==============================================================================
//
//  システム系
//
//==============================================================================
//==================================================================
/**
 * ユニオンエフェクト制御システムセットアップ
 *
 * @param   heap_id		ユニオンエフェクト制御システムをAllocするヒープID
 *
 * @retval  UNION_EFF_SYSTEM *		作成されたユニオンエフェクト制御システムへのポインタ
 */
//==================================================================
UNION_EFF_SYSTEM * UNION_EFF_SystemSetup(HEAPID heap_id)
{
  UNION_EFF_SYSTEM *unieff;
  
  unieff = GFL_HEAP_AllocClearMemory(heap_id, sizeof(UNION_EFF_SYSTEM));

  UnionEff_Focus_ResourceSet(&unieff->focus_man, heap_id);

  return unieff;
}

//==================================================================
/**
 * ユニオンエフェクト制御システム削除
 *
 * @param   unieff		ユニオンエフェクト制御システムへのポインタ
 */
//==================================================================
void UNION_EFF_SystemExit(UNION_EFF_SYSTEM *unieff)
{
  if(unieff == NULL){
    return;
  }

  UnionEff_Focus_DeleteAll(&unieff->focus_man);
  UnionEff_Focus_ResourceDelete(&unieff->focus_man);
  
  GFL_HEAP_FreeMemory(unieff);
}

//==================================================================
/**
 * ユニオンエフェクト制御システム更新処理
 *
 * @param   unieff		ユニオンエフェクト制御システムへのポインタ
 */
//==================================================================
void UNION_EFF_SystemUpdate(UNION_EFF_SYSTEM *unieff)
{
  if(unieff == NULL){
    return;
  }
  
  UnionEff_Focus_UpdateAll(&unieff->focus_man);
}

//==================================================================
/**
 * ユニオンエフェクト制御システム描画処理
 *
 * @param   unieff		ユニオンエフェクト制御システムへのポインタ
 */
//==================================================================
void UNION_EFF_SystemDraw(UNION_EFF_SYSTEM *unieff)
{
  if(unieff == NULL){
    return;
  }
  
  UnionEff_Focus_DrawAll(&unieff->focus_man);
}




//==============================================================================
//
//  アプリ系
//
//==============================================================================
//==================================================================
/**
 * 対象の動作モデルにフォーカスエフェクトをリクエストする
 *
 * @param   unieff		
 * @param   mmdl		  フォーカスする対象動作モデルへのポインタ
 */
//==================================================================
void UnionEff_App_ReqFocus(UNION_EFF_SYSTEM *unieff, MMDL *mmdl)
{
  if(unieff == NULL){
    return;
  }
  
  UnionEff_Focus_Setup(&unieff->focus_man, mmdl);
}




//==============================================================================
//
//  ローカル
//
//==============================================================================

//--------------------------------------------------------------
/**
 * フォーカスエフェクトのリソース登録
 *
 * @param   focus_man		
 * @param   heap_id
 */
//--------------------------------------------------------------
static void UnionEff_Focus_ResourceSet(UNION_EFF_FOCUS_MANAGER *focus_man, HEAPID heap_id)
{
  ARCHANDLE *handle;
  BOOL ret;
  
  handle = GFL_ARC_OpenDataHandle(ARCID_UNION, heap_id);

  focus_man->res_nsbmd = GFL_G3D_CreateResourceHandle( handle, NARC_unionroom_sentaku_nsbmd );
  focus_man->res_anm[0] = GFL_G3D_CreateResourceHandle( handle, NARC_unionroom_sentaku_nsbta );

  ret = GFL_G3D_TransVramTexture( focus_man->res_nsbmd );
  GF_ASSERT( ret );
  
  GFL_ARC_CloseDataHandle(handle);
}

//--------------------------------------------------------------
/**
 * フォーカスエフェクトのリソース破棄
 *
 * @param   focus_man		
 */
//--------------------------------------------------------------
static void UnionEff_Focus_ResourceDelete(UNION_EFF_FOCUS_MANAGER *focus_man)
{
  int i;
  
  GFL_G3D_FreeVramTexture(focus_man->res_nsbmd);
  
  for(i = 0; i < FOCUS_ANM_RESOURCE_MAX; i++){
    GFL_G3D_DeleteResource(focus_man->res_anm[i]);
    focus_man->res_anm[i] = NULL;
  }
  GFL_G3D_DeleteResource(focus_man->res_nsbmd);
  focus_man->res_nsbmd = NULL;
}

//--------------------------------------------------------------
/**
 * 空き領域を検索してフォーカスエフェクトを作成する
 *
 * @param   focus_man		
 * @param   mmdl		    フォーカス対象の動作モデルへのポインタ
 */
//--------------------------------------------------------------
static void UnionEff_Focus_Setup(UNION_EFF_FOCUS_MANAGER *focus_man, MMDL *mmdl)
{
  UNION_EFF_FOCUS *focus;
  
  GF_ASSERT(focus_man->res_nsbmd != NULL);
  
  focus = _SearchFocusWork(focus_man);
  UnionEff_Focus_Create(focus_man, focus, mmdl, focus_man->eff_no_count);
  focus_man->eff_no_count++;
}

//--------------------------------------------------------------
/**
 * フォーカスエフェクト作成
 *
 * @param   focus		  フォーカスエフェクトワークへのポインタ
 * @param   mmdl		  フォーカス対象の動作モデルへのポインタ
 * @param   eff_no		エフェクト番号
 */
//--------------------------------------------------------------
static void UnionEff_Focus_Create(UNION_EFF_FOCUS_MANAGER *focus_man, UNION_EFF_FOCUS *focus, MMDL *mmdl, u16 eff_no)
{
  int i;

  GFL_STD_MemClear(focus, sizeof(UNION_EFF_FOCUS));

  focus->mmdl = mmdl;
  focus->eff_no = eff_no;
  
  focus->rnder = GFL_G3D_RENDER_Create( focus_man->res_nsbmd, 0, focus_man->res_nsbmd );
  for(i = 0; i < FOCUS_ANM_RESOURCE_MAX; i++){
    focus->anm[i] = GFL_G3D_ANIME_Create( focus->rnder, focus_man->res_anm[i], 0 );
  }
  
  focus->g3dobj = GFL_G3D_OBJECT_Create( focus->rnder, focus->anm, FOCUS_ANM_RESOURCE_MAX );
  
  for(i = 0; i < FOCUS_ANM_RESOURCE_MAX; i++){
    GFL_G3D_OBJECT_EnableAnime( focus->g3dobj, i );
  }
}

//--------------------------------------------------------------
/**
 * フォーカスエフェクト削除
 *
 * @param   focus		
 */
//--------------------------------------------------------------
static void UnionEff_Focus_Delete(UNION_EFF_FOCUS *focus)
{
  int i;
  
  for(i = 0; i < FOCUS_ANM_RESOURCE_MAX; i++){
    GFL_G3D_ANIME_Delete( focus->anm[i] );
  }
  GFL_G3D_OBJECT_Delete( focus->g3dobj );
	GFL_G3D_RENDER_Delete( focus->rnder );

  GFL_STD_MemClear(focus, sizeof(UNION_EFF_FOCUS));
}

//--------------------------------------------------------------
/**
 * フォーカスエフェクト更新
 *
 * @param   focus		
 */
//--------------------------------------------------------------
static void UnionEff_Focus_Update(UNION_EFF_FOCUS *focus)
{
  GFL_G3D_OBJECT_LoopAnimeFrame(focus->g3dobj, 0, FX32_ONE);
  focus->timer++;
  if(focus->timer > FOCUS_LIFE){
    UnionEff_Focus_Delete(focus);
  }
}

//--------------------------------------------------------------
/**
 * フォーカスエフェクト描画
 *
 * @param   unieff		
 * @param   status		
 */
//--------------------------------------------------------------
static void UnionEff_Focus_Draw( UNION_EFF_FOCUS *focus, GFL_G3D_OBJSTATUS *status )
{
  MMDL_GetVectorPos( focus->mmdl, &status->trans );
  GFL_G3D_DRAW_DrawObjectCullingON( focus->g3dobj, status );
}

//--------------------------------------------------------------
/**
 * 全フォーカスエフェクト削除
 *
 * @param   focus_man		
 */
//--------------------------------------------------------------
static void UnionEff_Focus_DeleteAll(UNION_EFF_FOCUS_MANAGER *focus_man)
{
  int i;
  
  for(i = 0; i < FOCUS_MAX; i++){
    if(focus_man->focus[i].g3dobj != NULL){
      UnionEff_Focus_Delete(&focus_man->focus[i]);
    }
  }
}

//--------------------------------------------------------------
/**
 * 全フォーカスエフェクト更新
 *
 * @param   focus_man		
 */
//--------------------------------------------------------------
static void UnionEff_Focus_UpdateAll(UNION_EFF_FOCUS_MANAGER *focus_man)
{
  int i;
  
  for(i = 0; i < FOCUS_MAX; i++){
    if(focus_man->focus[i].g3dobj != NULL){
      if(MMDL_CheckStatusBitUse(focus_man->focus[i].mmdl) == TRUE){
        UnionEff_Focus_Update(&focus_man->focus[i]);
      }
      else{
        UnionEff_Focus_Delete(&focus_man->focus[i]);
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 全フォーカスエフェクト描画
 *
 * @param   focus_man		
 */
//--------------------------------------------------------------
static void UnionEff_Focus_DrawAll( UNION_EFF_FOCUS_MANAGER *focus_man)
{
  int i;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
  
  MTX_Identity33( &status.rotate );
  
  for(i = 0; i < FOCUS_MAX; i++){
    if(focus_man->focus[i].g3dobj != NULL 
        && MMDL_CheckStatusBitUse(focus_man->focus[i].mmdl) == TRUE){
      UnionEff_Focus_Draw(&focus_man->focus[i], &status);
    }
  }
}

//--------------------------------------------------------------
/**
 * フォーカスエフェクトワークの空き領域を検索
 *
 * @param   focus_man		
 *
 * @retval  UNION_EFF_FOCUS *		空いていたフォーカスエフェクトワークへのポインタ
 *
 * 空きが無い場合は最も古いエフェクトワークを削除して、そこのワークポインタを返します
 */
//--------------------------------------------------------------
static UNION_EFF_FOCUS * _SearchFocusWork(UNION_EFF_FOCUS_MANAGER *focus_man)
{
  int i, old_no = 0;
  
  for(i = 0; i < FOCUS_MAX; i++){
    if(focus_man->focus[i].g3dobj == NULL){
      return &focus_man->focus[i];
    }
    if(focus_man->focus[i].eff_no < focus_man->focus[old_no].eff_no){
      old_no = i;
    }
  }
  
  //空きが無い場合は最も古いフォーカスエフェクトを削除して、そこを返す
  UnionEff_Focus_Delete(&focus_man->focus[old_no]);
  return &focus_man->focus[old_no];
}

