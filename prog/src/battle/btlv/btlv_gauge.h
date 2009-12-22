
//============================================================================================
/**
 * @file  btlv_gauge.h
 * @brief êÌì¨ÉQÅ[ÉW
 * @author  soga
 * @date  2009.07.15
 */
//============================================================================================

#pragma once

#include "print/gf_font.h"

#include "../btl_common.h"
#include "../btl_main.h"
#include "../btl_pokeparam.h"


typedef enum
{
  BTLV_GAUGE_TYPE_1vs1 = 0,
  BTLV_GAUGE_TYPE_2vs2,
  BTLV_GAUGE_TYPE_3vs3,
  BTLV_GAUGE_TYPE_SAFARI,
}BTLV_GAUGE_TYPE;

typedef struct _BTLV_GAUGE_WORK BTLV_GAUGE_WORK;
typedef struct _BTLV_GAUGE_CLWK BTLV_GAUGE_CLWK;

extern  BTLV_GAUGE_WORK*  BTLV_GAUGE_Init( GFL_FONT* fontHandle, HEAPID heapID );
extern  void              BTLV_GAUGE_Exit( BTLV_GAUGE_WORK *bgw );
extern  void              BTLV_GAUGE_Main( BTLV_GAUGE_WORK *bgw );
extern  void              BTLV_GAUGE_Add( BTLV_GAUGE_WORK *bgw, const BTL_MAIN_MODULE* wk, const BTL_POKEPARAM* bpp,
                                          BTLV_GAUGE_TYPE type, BtlvMcssPos pos );
extern  void              BTLV_GAUGE_Del( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos );
extern  void              BTLV_GAUGE_SetPos( BTLV_GAUGE_WORK* bgw, BtlvMcssPos pos, int pos_x, int pos_y );
extern  void              BTLV_GAUGE_CalcHP( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos, int damage );
extern  void              BTLV_GAUGE_CalcEXP( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos, int add_exp );
extern  void              BTLV_GAUGE_CalcEXPLevelUp( BTLV_GAUGE_WORK *bgw, const BTL_POKEPARAM* bpp, BtlvMcssPos pos );
extern  BOOL              BTLV_GAUGE_CheckExecute( BTLV_GAUGE_WORK *bgw );
extern  void              BTLV_GAUGE_SetDrawEnable( BTLV_GAUGE_WORK* bgw, BOOL on_off );
extern  void              BTLV_GAUGE_SetStatus( BTLV_GAUGE_WORK* bgw, PokeSick sick, BtlvMcssPos pos );

