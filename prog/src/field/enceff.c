//======================================================================
/*
 * @file	enceff.c
 * @brief	エンカウントエフェクト
 * @author saito
 */
//======================================================================
#include "enceff.h"
#include "gamesystem/gamesystem.h"

#include "fieldmap.h"

#include "system/main.h" //for HEAPID_FIELDMAP

#define OVERLAY_STACK_MAX (3)
#define OVERLAY_NONE  (-1)

typedef GMEVENT* (*CREATE_FUNC)(GAMESYS_WORK *, FIELDMAP_WORK *, const BOOL);
typedef void (*DRAW_FUNC)(ENCEFF_CNT_PTR);

FS_EXTERN_OVERLAY(enceff_ci);
FS_EXTERN_OVERLAY(enceff_mdl);
FS_EXTERN_OVERLAY(enceff_prg);
FS_EXTERN_OVERLAY(enceff_wav);
FS_EXTERN_OVERLAY(enceff_pnl);
FS_EXTERN_OVERLAY(enceff_pnl1);
FS_EXTERN_OVERLAY(enceff_pnl2);
FS_EXTERN_OVERLAY(enceff_pnl3);

typedef struct ENCEFF_CNT_tag
{
  void *Work;
  DRAW_FUNC DrawFunc;
  FIELDMAP_WORK *FieldMapWork;
  int OverlayStack[OVERLAY_STACK_MAX];
  int OverlayStackNum;
  void *UserWorkPtr;
}ENCEFF_CNT;

typedef struct {
  CREATE_FUNC CreateFunc;
  DRAW_FUNC DrawFunc;
  FSOverlayID OverlayID;
  BOOL IsFadeWhite;
  u32 UseMemSize;  //エフェクトが使用するフィールドヒープ　リソースに依存するので、更新がかかった場合は再調査が必要です
}ENCEFF_TBL;

//※ enceffno_def.h と並びを同じにすること
static const ENCEFF_TBL EncEffTbl[] = {
  {ENCEFF_MDL_Create1, ENCEFF_MDL_Draw, FS_OVERLAY_ID(enceff_mdl), TRUE, 0x13010},     //野生　通常
  {ENCEFF_MDL_Create2, ENCEFF_MDL_Draw, FS_OVERLAY_ID(enceff_mdl), TRUE, 0x131a4},     //野生　強
  {ENCEFF_WAV_Create, ENCEFF_WAV_Draw, FS_OVERLAY_ID(enceff_wav), TRUE, 0},      //野生　水上 釣り
  {ENCEFF_CI_CreateCave, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0x1e04},     //野生　屋内
  {ENCEFF_CI_CreateDesert, NULL, FS_OVERLAY_ID(enceff_ci), TRUE, 0x14bc8},     //野生　砂地
  {ENCEFF_MDL_Create3, ENCEFF_MDL_Draw, FS_OVERLAY_ID(enceff_mdl), TRUE, 0x12fbc},     //トレーナー　通常
  {ENCEFF_MDL_Create4, ENCEFF_MDL_Draw, FS_OVERLAY_ID(enceff_mdl), TRUE, 0x13b20},      //トレーナー　水上
  {ENCEFF_PNL1_Create, ENCEFF_PNL_Draw, FS_OVERLAY_ID(enceff_pnl1), FALSE, 0},    //トレーナー　屋内
  {ENCEFF_PNL3_Create, ENCEFF_PNL_Draw, FS_OVERLAY_ID(enceff_pnl3), TRUE, 0},    //トレーナー　砂地

  {ENCEFF_PNL2_Create, ENCEFF_PNL_Draw, FS_OVERLAY_ID(enceff_pnl2), TRUE, 0},    //バトルサブウェイ

  {ENCEFF_CI_CreateRival, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //ライバル
  {ENCEFF_CI_CreateSupport, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //サポーター
  {ENCEFF_CI_CreateGym01A, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C1ジムリーダーＡ
  {ENCEFF_CI_CreateGym01B, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C1ジムリーダーＢ
  {ENCEFF_CI_CreateGym01C, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C1ジムリーダーＣ
  {ENCEFF_CI_CreateGym02, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C2ジムリーダー
  {ENCEFF_CI_CreateGym03, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C3ジムリーダー
  {ENCEFF_CI_CreateGym04, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C4ジムリーダー
  {ENCEFF_CI_CreateGym05, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C5ジムリーダー
  {ENCEFF_CI_CreateGym06, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C6ジムリーダー
  {ENCEFF_CI_CreateGym07, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C7ジムリーダー
  {ENCEFF_CI_CreateGym08A, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C8ジムリーダーＡ
  {ENCEFF_CI_CreateGym08B, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xe570},    //C8ジムリーダーＢ
  {ENCEFF_CI_CreateBigFour1, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0x12a68},    //四天王1
  {ENCEFF_CI_CreateBigFour2, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0x12a68},    //四天王2
  {ENCEFF_CI_CreateBigFour3, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0x12a68},    //四天王3
  {ENCEFF_CI_CreateBigFour4, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0x12a68},    //四天王4
  {ENCEFF_CI_CreateChamp, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0x12a68},    //チャンプ
  {ENCEFF_CI_CreateBoss, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xd530},    //N
  {ENCEFF_CI_CreateSage, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xd530},    //ゲーツィス
  {ENCEFF_CI_CreatePlasma, NULL, FS_OVERLAY_ID(enceff_ci), FALSE, 0xcf20},    //プラズマ団

  {ENCEFF_MDL_Create5, ENCEFF_MDL_Draw, FS_OVERLAY_ID(enceff_mdl), TRUE, 0x132e8},    //エフェクト
  {ENCEFF_CI_CreatePackage, NULL, FS_OVERLAY_ID(enceff_ci), TRUE, 0x80e0},    //パッケージポケ
  {ENCEFF_CI_CreateMovePoke, NULL, FS_OVERLAY_ID(enceff_ci), TRUE, 0x619c},    //移動ポケ
  {ENCEFF_CI_CreateThree, NULL, FS_OVERLAY_ID(enceff_ci), TRUE, 0xac88},    //三銃士ポケ
  {ENCEFF_MDL_Create6, ENCEFF_MDL_Draw, FS_OVERLAY_ID(enceff_mdl), TRUE, 0x12dc0},    //ゾロアーク

  {ENCEFF_WAV_Create2, ENCEFF_WAV_Draw, FS_OVERLAY_ID(enceff_wav), FALSE, 0},    //パレスワープ（エンカウントには使用しない）

};


static void LoadOverlay(ENCEFF_CNT_PTR ptr, const inFSOverlayID);


//--------------------------------------------------------------
/**
 * @brief エンカウントエフェクトコントローラ作成
 * @param inHeapID    ヒープＩＤ
 * @retval  ptr   コントローラポインタ
 */
//--------------------------------------------------------------
ENCEFF_CNT_PTR ENCEFF_CreateCntPtr(const HEAPID inHeapID, FIELDMAP_WORK * fieldmap)
{
  ENCEFF_CNT_PTR ptr;
  ptr = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(ENCEFF_CNT));
  ptr->FieldMapWork = fieldmap;
  ptr->OverlayStackNum = 0;
  return ptr;
}

//--------------------------------------------------------------
/**
 * @brief   エンカウントエフェクトコントローラ破棄
 * @param ptr     コントローラポインタ
 * @retval  none
 */
//--------------------------------------------------------------
void ENCEFF_DeleteCntPtr(ENCEFF_CNT_PTR ptr)
{
  ENCEFF_FreeUserWork(ptr);
  //オーバーレイスタックがある場合はここでアンロード
  ENCEFF_UnloadEffOverlay(ptr);
  
  GFL_HEAP_FreeMemory( ptr );
}

//--------------------------------------------------------------
/**
 * @brief   エンカウントエフェクトセット
 * @param   ptr       コントローラポインタ
 * @param   event     イベントポインタ
 * @param   inID      エフェクトＩＤ
 * @retval  none
 */
//--------------------------------------------------------------
void ENCEFF_SetEncEff(ENCEFF_CNT_PTR ptr, GMEVENT * event, const ENCEFF_ID inID)
{
  GMEVENT *call_event;
  FIELDMAP_WORK * fieldmap = ptr->FieldMapWork;
  GAMESYS_WORK * gsys = FIELDMAP_GetGameSysWork( ptr->FieldMapWork );

  int no;
  no = inID;
  if (no >= ENCEFFID_MAX)
  {
    GF_ASSERT_MSG(0,"ID=%d",no);
    no = 0;
  }
#ifdef PM_DEBUG
#ifdef DEBUG_ONLY_FOR_saitou
//  no = ENCEFFID_MAX-1;
#endif
#endif

  //ヒープが確保できるかをチェック GFI関数は基本的に直接使用禁止ですが、今回は特別に使用します。
  if ( GFI_HEAP_GetHeapAllocatableSize(HEAPID_FIELDMAP) < EncEffTbl[no].UseMemSize )
  {
    GF_ASSERT_MSG(0,"EFF_CAN_NOT_ALLOC No:%d  Allocatable=%x total=%x",
        no, GFI_HEAP_GetHeapAllocatableSize(HEAPID_FIELDMAP),GFL_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP));
    //フィールドヒープを使用しないエフェクトに変更
    no = ENCEFFID_TR_INNER;
  }
  
  //オーバーレイロード
  if (EncEffTbl[no].OverlayID != OVERLAY_NONE)
  {
    LoadOverlay(ptr, EncEffTbl[no].OverlayID);
  }

  call_event = EncEffTbl[no].CreateFunc(gsys, fieldmap, EncEffTbl[no].IsFadeWhite);
  //イベントコール
  GMEVENT_CallEvent( event, call_event );
  //ワークポインタセット
  ptr->Work = GMEVENT_GetEventWork( call_event );
  //描画関数セット
  ptr->DrawFunc = EncEffTbl[no].DrawFunc;
}

//--------------------------------------------------------------
/**
 * @brief   ワークポインタを返す
 * @param ptr   コントローラポインタ
 * @retval  void*     ワークポインタ
 */
//--------------------------------------------------------------
void *ENCEFF_GetWork(ENCEFF_CNT_PTR ptr)
{
  if (ptr->Work == NULL) GF_ASSERT(0);

  return ptr->Work;
}

//--------------------------------------------------------------
/**
 * @brief   描画関数
 * @param   ptr     コントローラポインタ
 * @retval  none
 */
//--------------------------------------------------------------
void ENCEFF_Draw(ENCEFF_CNT_PTR ptr)
{
  if (ptr->DrawFunc != NULL)
  {
    ptr->DrawFunc(ptr);
  }
}

//--------------------------------------------------------------
/**
 * @brief   ユーザーワークポインタをセット
 * @param ptr   コントローラポインタ
 * @retval  void*     ワークポインタ
 */
//--------------------------------------------------------------
void *ENCEFF_AllocUserWork(ENCEFF_CNT_PTR ptr, const int size, const HEAPID inHeapID)
{
  if (ptr->UserWorkPtr != NULL)
  {
    GF_ASSERT(0);
  }

  NOZOMU_Printf("user_work_size = %x\n",size); 
  
  ptr->UserWorkPtr = GFL_HEAP_AllocClearMemory(inHeapID, size);
  return ptr->UserWorkPtr;
}

//--------------------------------------------------------------
/**
 * @brief   ユーザーワーク解放
 * @param ptr   コントローラポインタ
 * @retval  none
 */
//--------------------------------------------------------------
void ENCEFF_FreeUserWork(ENCEFF_CNT_PTR ptr)
{
  if (ptr->UserWorkPtr != NULL)
  {
    GFL_HEAP_FreeMemory( ptr->UserWorkPtr );
    ptr->UserWorkPtr = NULL;
  }
}


//--------------------------------------------------------------
/**
 * @brief   ユーザーワークポインタを取得
 * @param ptr   コントローラポインタ
 * @retval  void*     ワークポインタ
 */
//--------------------------------------------------------------
void *ENCEFF_GetUserWorkPtr(ENCEFF_CNT_PTR ptr)
{
  return ptr->UserWorkPtr;
}

//--------------------------------------------------------------------------------------------
/**
 * オーバーレイをロードしてスタックする
 *
 * @param   ptr     コントローラポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void LoadOverlay(ENCEFF_CNT_PTR ptr, const inFSOverlayID)
{
  GFL_OVERLAY_Load( inFSOverlayID );
  ptr->OverlayStack[ ptr->OverlayStackNum++ ] = inFSOverlayID;
  GF_ASSERT( ptr->OverlayStackNum <= OVERLAY_STACK_MAX );
}

//--------------------------------------------------------------------------------------------
/**
 * パネルエフェクト用オーバーレイロード
 *
 * @param   ptr     コントローラポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ENCEFF_LoadPanelEffOverlay(ENCEFF_CNT_PTR ptr)
{
  //プログラム制御エフェクトオーバーレイロード
  LoadOverlay(ptr, FS_OVERLAY_ID(enceff_prg));
  //パネルエフェクトオーバーレイロード
  LoadOverlay(ptr, FS_OVERLAY_ID(enceff_pnl));
}

//--------------------------------------------------------------------------------------------
/**
 * 頂点エフェクト用オーバーレイロード
 *
 * @param   ptr     コントローラポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ENCEFF_LoadVertexEffOverlay(ENCEFF_CNT_PTR ptr)
{
  //プログラム制御エフェクトオーバーレイロード
  LoadOverlay(ptr, FS_OVERLAY_ID(enceff_prg));
}

//--------------------------------------------------------------------------------------------
/**
 * エフェクト用オーバーレイアンロード
 *
 * @param   
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ENCEFF_UnloadEffOverlay(ENCEFF_CNT_PTR ptr)
{
  if (ptr->OverlayStackNum>0)
  {
    int idx;
    while(ptr->OverlayStackNum>0)
    {
      idx = ptr->OverlayStackNum-1;
      //アンロード
      if ( ptr->OverlayStack[ idx ] != NULL )
      {
        GFL_OVERLAY_Unload( ptr->OverlayStack[ idx ] );
      }
      else GF_ASSERT_MSG(0,"Can not unload idx=%d\n", idx);

      ptr->OverlayStackNum--;
    }
  }
}


