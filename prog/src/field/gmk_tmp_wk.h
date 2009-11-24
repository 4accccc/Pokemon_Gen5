//======================================================================
/**
 * @file	gmk_tmp_work.h
 * @brief	ギミックテンポラリワーク
 * @author  Saito
 *
 * @todo  テンポラリワークを使用する際は、このヘッダーファイルを明示的にインクルードしてください
 * @todo  構成はメモリ確保、確保メモリポインタ取得、メモリ解放の3つです
 *
 */
//======================================================================
#pragma once

#include "field/fieldmap_proc.h"    //for FIELDMAP_WORK

extern void *GMK_TMP_WK_AllocWork
      (FIELDMAP_WORK *fieldWork, const u32 inAssignID, const HEAPID inHeapID, const u32 inSize);
extern void GMK_TMP_WK_FreeWork
      (FIELDMAP_WORK *fieldWork, const u32 inAssignID);
extern void *GMK_TMP_WK_GetWork(FIELDMAP_WORK *fieldWork, const u32 inAssignID);



