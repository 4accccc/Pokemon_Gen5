////////////////////////////////////////////////////////////////////////////
/**
 * @file   place_name_letter.h
 * @brief  地名表示の文字オブジェクト
 * @author obata_toshihiro
 * @date   2010.03.28
 */
////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "print/gf_font.h"  // for GFL_FONT
#include "system/bmp_oam.h" // for BmpOam_xxxx
//=========================================================================
// ■地名表示オブジェクト（上部、下部のオブジェクト）
//=========================================================================
typedef struct _PN_PLATE PN_PLATE;

//=========================================================================
// ■初期化パラメータ
//=========================================================================
typedef struct 
{
  BMPOAM_SYS_PTR  bmpOamSys;
  u16 fontPlttRegIdx;
} PN_PLATE_TOP_INIT_PARAM;

typedef struct 
{
  int dummy;
} PN_PLATE_BOTTOM_INIT_PARAM;
//=========================================================================
// ■設定パラメータ
//=========================================================================
typedef struct 
{
  GFL_FONT      *font;            //フォント
  u16           zoneID;           //ゾーンID
  const STRBUF  *placeName;       //表示する地名
} PN_PLATE_TOP_SETUP_PARAM;

typedef struct 
{
  u32           seasonID;         //季節
} PN_PLATE_BOTTOM_SETUP_PARAM;
//=========================================================================
// ■定数
//=========================================================================
typedef enum
{
  PN_PLATE_TYPE_TOP,    //上部のオブジェクト（道路情報）
  PN_PLATE_TYPE_BOTTOM, //下部のオブジェクト（季節情報）
  PN_PLATE_TYPE_MAX,
}PN_PLATE_TYPE;

//=========================================================================
// ■public functions
//=========================================================================
extern PN_PLATE* PN_PLATE_Create( HEAPID heapID, PN_PLATE_TYPE type, const void *init_param, GFL_CLUNIT *unit, ARCHANDLE* arcHandle, u16 surface, u16 backobjPlttRegIdx, u16 backobjCgrRegIdx, u16 backobjCellRegIdx );
extern void PN_PLATE_Delete( PN_PLATE *plate );
extern void PN_PLATE_SetupReq( PN_PLATE *plate, const void *setup_param );
extern void PN_PLATE_Main( PN_PLATE *plate );
extern void PN_PLATE_EnterReq( PN_PLATE *plate, s16 start, s16 end );
extern void PN_PLATE_OutReq( PN_PLATE *plate, s16 start, s16 end );
extern void PN_PLATE_HideReq( PN_PLATE *plate );
extern BOOL PN_PLATE_IsEnd( const PN_PLATE *plate );
