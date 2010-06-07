//==============================================================================
/**
 * @file    union_eff.h
 * @brief   ユニオンルーム：エフェクト類のヘッダ
 * @author  matsuda
 * @date    2010.01.17(日)
 */
//==============================================================================
#pragma once

#include "field/fldmmdl.h"


//==============================================================================
//  型定義
//==============================================================================
///ユニオンエフェクト制御システム構造体
typedef struct _UNION_EFF_SYSTEM UNION_EFF_SYSTEM;

///友達マークの種類
typedef enum{
  FRIENDMARK_TYPE_FRIEND,         ///<友達
  FRIENDMARK_TYPE_ACQUAINTANCE,   ///<知り合い
}FRIENDMARK_TYPE;


//==============================================================================
//  外部関数宣言
//==============================================================================
//--------------------------------------------------------------
//  システム系
//--------------------------------------------------------------
extern UNION_EFF_SYSTEM * UNION_EFF_SystemSetup(HEAPID heap_id);
extern void UNION_EFF_SystemExit(UNION_EFF_SYSTEM *unieff);
extern void UNION_EFF_SystemUpdate(UNION_EFF_SYSTEM *unieff);
extern void UNION_EFF_SystemDraw(UNION_EFF_SYSTEM *unieff);

//--------------------------------------------------------------
//  アプリ系
//--------------------------------------------------------------
extern void UnionEff_App_ReqFocus(UNION_EFF_SYSTEM *unieff, MMDL *mmdl);
extern void UnionEff_App_ReqFriendMark(UNION_EFF_SYSTEM *unieff, MMDL *mmdl, FRIENDMARK_TYPE mark_type);
extern void UnionEff_App_SearchDeleteFriendMark( UNION_EFF_SYSTEM *unieff, const MMDL *mmdl );
