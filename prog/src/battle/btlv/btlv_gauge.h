
//============================================================================================
/**
 * @file  btlv_gauge.h
 * @brief 戦闘ゲージ
 * @author  soga
 * @date  2009.07.15
 */
//============================================================================================

#pragma once

#include "print/gf_font.h"

#include "../btl_common.h"
#include "../btl_main.h"
#include "../btl_pokeparam.h"

///1キャラの中に入っているゲージのドット数
#define BTLV_GAUGE_DOTTO      (8)

///HPバーのキャラ数
#define BTLV_GAUGE_HP_CHARMAX   (6)
///EXPバーのキャラ数
#define BTLV_GAUGE_EXP_CHARMAX  (10)
///HPゲージのドット数
#define BTLV_GAUGE_HP_DOTTOMAX  ( BTLV_GAUGE_DOTTO * BTLV_GAUGE_HP_CHARMAX )

typedef enum
{
  BTLV_GAUGE_TYPE_1vs1 = 0,
  BTLV_GAUGE_TYPE_2vs2,
  BTLV_GAUGE_TYPE_3vs3,
  BTLV_GAUGE_TYPE_ROTATE,
  BTLV_GAUGE_TYPE_SAFARI,
}BTLV_GAUGE_TYPE;

typedef struct _BTLV_GAUGE_WORK BTLV_GAUGE_WORK;
typedef struct _BTLV_GAUGE_CLWK BTLV_GAUGE_CLWK;

extern  BTLV_GAUGE_WORK*  BTLV_GAUGE_Init( GFL_FONT* fontHandle, HEAPID heapID );
extern  void              BTLV_GAUGE_Exit( BTLV_GAUGE_WORK *bgw );
extern  void              BTLV_GAUGE_Main( BTLV_GAUGE_WORK *bgw );
extern  void              BTLV_GAUGE_Add( BTLV_GAUGE_WORK *bgw, const BTL_MAIN_MODULE* wk, const BTL_POKEPARAM* bpp,
                                          BTLV_GAUGE_TYPE type, BtlvMcssPos pos );
extern  void              BTLV_GAUGE_AddPP( BTLV_GAUGE_WORK *bgw, const ZUKAN_SAVEDATA* zs, const POKEMON_PARAM* pp,
                                            BTLV_GAUGE_TYPE type, BtlvMcssPos pos );
extern  void              BTLV_GAUGE_Del( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos );
extern  void              BTLV_GAUGE_SetPos( BTLV_GAUGE_WORK* bgw, BtlvMcssPos pos, GFL_CLACTPOS* ofs );
extern  void              BTLV_GAUGE_CalcHP( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos, int damage );
extern  void              BTLV_GAUGE_CalcHPAtOnce( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos, int value );
extern  void              BTLV_GAUGE_CalcEXP( BTLV_GAUGE_WORK *bgw, BtlvMcssPos pos, int add_exp );
extern  void              BTLV_GAUGE_CalcEXPLevelUp( BTLV_GAUGE_WORK *bgw, const BTL_POKEPARAM* bpp, BtlvMcssPos pos );
extern  BOOL              BTLV_GAUGE_CheckExecute( BTLV_GAUGE_WORK *bgw );
extern  void              BTLV_GAUGE_SetDrawEnable( BTLV_GAUGE_WORK* bgw, BOOL on_off, int side );
extern  void              BTLV_GAUGE_SetDrawEnableByPos( BTLV_GAUGE_WORK* bgw, BOOL on_off, BtlvMcssPos pos );
extern  BOOL              BTLV_GAUGE_CheckExist( BTLV_GAUGE_WORK* bgw, BtlvMcssPos pos );
extern  BOOL              BTLV_GAUGE_CheckAppearFlag( BTLV_GAUGE_WORK* bgw, BtlvMcssPos pos );
extern  void              BTLV_GAUGE_SetStatus( BTLV_GAUGE_WORK* bgw, PokeSick sick, BtlvMcssPos pos );
extern  void              BTLV_GAUGE_RequestYure( BTLV_GAUGE_WORK* bgw, BtlvMcssPos pos );
extern  int               BTLV_GAUGE_GetPinchBGMFlag( BTLV_GAUGE_WORK* bgw );
extern  void              BTLV_GAUGE_SetPinchBGMFlag( BTLV_GAUGE_WORK* bgw, BOOL value );
extern  void              BTLV_GAUGE_SetNowBGMNo( BTLV_GAUGE_WORK* bgw, int bgm_no );
extern  void              BTLV_GAUGE_SetTrainerBGMChangeFlag( BTLV_GAUGE_WORK* bgw, int value );
extern  void              BTLV_GAUGE_SetPinchBGMNoCheck( BTLV_GAUGE_WORK* bgw, BOOL value );
extern  BOOL              BTLV_GAUGE_GetGaugeStatus( BTLV_GAUGE_WORK* bgw, BtlvMcssPos pos, int* color, int* sick_anm );

