//==============================================================================
/**
 * @file    union_subdisp.h
 * @brief   ユニオン下画面のヘッダ
 * @author  matsuda
 * @date    2009.07.27(月)
 */
//==============================================================================
#pragma once


//==============================================================================
//  定数定義
//==============================================================================
///選択しているアピール番号
enum{
  UNISUB_APPEAL_NO_BATTLE,
  UNISUB_APPEAL_NO_TRADE,
  UNISUB_APPEAL_NO_RECORD,
  UNISUB_APPEAL_NO_PICTURE,
  UNISUB_APPEAL_NO_GURUGURU,

  UNISUB_APPEAL_NO_NONE = 0xff,
};

//==============================================================================
//  型定義
//==============================================================================
typedef struct _UNION_SUBDISP * UNION_SUBDISP_PTR;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern UNION_SUBDISP_PTR UNION_SUBDISP_Init(GAMESYS_WORK *gsys);
extern void UNION_SUBDISP_Exit(UNION_SUBDISP_PTR unisub);
extern void UNION_SUBDISP_Update(UNION_SUBDISP_PTR unisub, BOOL bActive);
extern void UNION_SUBDISP_Draw(UNION_SUBDISP_PTR unisub, BOOL bActive);

