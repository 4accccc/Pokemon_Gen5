//============================================================================================
/**
 * @file  btlv_effect.h
 * @brief 戦闘エフェクト制御
 * @author  soga
 * @date  2008.11.21
 */
//============================================================================================

#pragma once

#include "btlv_stage.h"
#include "btlv_field.h"
#include "btlv_camera.h"
#include "btlv_mcss.h"
#include "btlv_clact.h"
#include "btlv_gauge.h"
#include "btlv_b_gauge.h"
#include "btlv_timer.h"
#include "btlv_bg.h"

#include "btlv_efftool.h"
#include "btlv_effvm.h"

#include "battle/battle.h"

#include "waza_tool/wazadata.h"
#include "system/palanm.h"

#include  "waza_tool/wazano_def.h"

//戦闘エフェクト定義
enum{
  BTLEFF_SINGLE_ENCOUNT_1 = WAZANO_MAX + 1,
  BTLEFF_SINGLE_ENCOUNT_2_MALE,
  BTLEFF_SINGLE_ENCOUNT_2_FEMALE,
  BTLEFF_SINGLE_ENCOUNT_3,
  BTLEFF_CAMERA_WORK,
  BTLEFF_CAMERA_INIT,
  BTLEFF_SINGLE_TRAINER_ENCOUNT_1,
  BTLEFF_SINGLE_TRAINER_ENCOUNT_2,
  BTLEFF_SINGLE_TRAINER_ENCOUNT_3,
  BTLEFF_BALL_THROW,
  BTLEFF_HINSHI,

  //ステータスエフェクト
  BTLEFF_NEMURI,              //ねむり
  BTLEFF_DOKU,                //どく
  BTLEFF_YAKEDO,              //やけど
  BTLEFF_KOORI,               //こおり
  BTLEFF_MAHI,                //まひ
  BTLEFF_KONRAN,              //こんらん
  BTLEFF_MEROMERO,            //メロメロ
  BTLEFF_LVUP,                //レベルアップ
  BTLEFF_USE_ITEM,            //ポケモンにアイテムを使用
  BTLEFF_SOUBI_ITEM,          //装備道具発動
  BTLEFF_RARE,                //レアエフェクト
  BTLEFF_STATUS_UP,           //ステータス上昇
  BTLEFF_STATUS_DOWN,         //ステータス下降
  BTLEFF_HP_RECOVER,          //HP回復
//  BTLEFF_FLAME_OUT,           //みがわり画面外（@todo　未作成）
//  BTLEFF_FLAME_IN,            //みがわり画面内（@todo　未作成）
  BTLEFF_ITEM_ESCAPE,         //エネコのしっぽorピッピ人形
  BTLEFF_WEATHER_KIRI,        //きり（シャチではないかも？）
  BTLEFF_WEATHER_AME,         //あめ
  BTLEFF_WEATHER_ARARE,       //あられ
  BTLEFF_WEATHER_SUNAARASHI,  //すなあらし
  BTLEFF_WEATHER_HARE,        //はれ
//  BTLEFF_MIGAWARI_FALL,       //
//  BTLEFF_MIGAWARI_DEAD,       //
//  BTLEFF_FUN,                 //
//  BTLEFF_EAT,                 //
//  BTLEFF_ANGRY,               //
  BTLEFF_NOROI,               //のろい
//  BTLEFF_AKUMU,              //あくむ
  BTLEFF_YADORIGI,            //やどりぎ
  BTLEFF_SIMETUKERU,          //しめつける
  BTLEFF_MAKITUKU,            //まきつく
  BTLEFF_HONOONOUZU,          //ほのおのうず
  BTLEFF_MAGUMASUTOOMU,       //マグマストーム
  BTLEFF_KARADEHASAMU,        //からではさむ
  BTLEFF_UZUSIO,              //うずしお
  BTLEFF_SUNAZIGOKU,          //すなじごく
  BTLEFF_NEWOHARU,            //ねをはる
};

typedef struct _BTLV_EFFECT_WORK BTLV_EFFECT_WORK;

//OBJパレット使用内訳
enum{
  BTLV_OBJ_PLTT_HP_GAUGE    = 0x20 * 0,   //0:HPゲージ
  BTLV_OBJ_PLTT_TIMER       = 0x20 * 1,   //1:タイマー
  BTLV_OBJ_PLTT_RED_TIMER   = 0x20 * 2,   //2:時間切れ迫ったタイマー
  BTLV_OBJ_PLTT_STATUS_ICON = 0x20 * 3,   //3:状態異常アイコン
  BTLV_OBJ_PLTT_NONE_4      = 0x20 * 4,   //4:空き
  BTLV_OBJ_PLTT_NONE_5      = 0x20 * 5,   //5:空き
  BTLV_OBJ_PLTT_CLACT       = 0x20 * 6,   //6:BTLV_CLACTで使用するセルアクター（10個分）
};

//--------------------------------------------
/**
 *  ターン種類指定
 */
//--------------------------------------------
typedef enum {
  BTLV_WAZAEFF_TURN_DEFAULT = 0,
  BTLV_WAZAEFF_TURN_TAME,
}BtlvWazaEffect_TurnType;

//--------------------------------------------
/**
 *  ワザエフェクト呼び出しパラメータ
 */
//--------------------------------------------
typedef struct {
  WazaID       waza;            ///< ワザナンバー
  BtlvMcssPos  from;            ///< ワザを出すポケ位置
  BtlvMcssPos  to;              ///< ワザを受けるポケ位置（不要の場合 BTLV_MCSS_POS_ERROR を指定）
  u8           turn_count;      ///< ターンによって異なるエフェクトを出す場合のターン指定。（ex.そらをとぶ）
  u8           continue_count;  ///< 連続して出すとエフェクトが異なる場合の連続カウンタ（ex. ころがる）
}BTLV_WAZAEFFECT_PARAM;

extern  void              BTLV_EFFECT_Init( BtlRule rule, const BTL_FIELD_SITUATION *bfs, GFL_FONT* fontHandle, HEAPID heapID );
extern  void              BTLV_EFFECT_Exit( void );
extern  void              BTLV_EFFECT_Main( void );
extern  void              BTLV_EFFECT_Add( int eff_no );
extern  void              BTLV_EFFECT_AddByPos( BtlvMcssPos pos, int eff_no );
extern  void              BTLV_EFFECT_AddByDir( BtlvMcssPos from, BtlvMcssPos to, int eff_no );
extern  void              BTLV_EFFECT_AddWazaEffect( const BTLV_WAZAEFFECT_PARAM* param );
extern  void              BTLV_EFFECT_Stop( void );

extern  void              BTLV_EFFECT_Damage( BtlvMcssPos target, WazaID waza );
extern  void              BTLV_EFFECT_Hinshi( BtlvMcssPos target );

extern  BOOL              BTLV_EFFECT_CheckExecute( void );
extern  void              BTLV_EFFECT_SetPokemon( const POKEMON_PARAM *pp, int position );
extern  void              BTLV_EFFECT_DelPokemon( int position );
extern  BOOL              BTLV_EFFECT_CheckExist( int position );
extern  void              BTLV_EFFECT_SetTrainer( int trtype, int position, int pos_x, int pos_y, int pos_z );
extern  void              BTLV_EFFECT_DelTrainer( int position );
extern  void              BTLV_EFFECT_SetGauge( const BTL_POKEPARAM* bpp, int position );
extern  void              BTLV_EFFECT_DelGauge( int position );
extern  void              BTLV_EFFECT_CalcGaugeHP( int position, int value );
extern  void              BTLV_EFFECT_CalcGaugeEXP( int position, int value );
extern  void              BTLV_EFFECT_CalcGaugeEXPLevelUp( int position, const BTL_POKEPARAM* bpp );
extern  void              BTLV_EFFECT_BallThrow( int position, u16 item_no, u8 yure_cnt, BOOL f_success );
extern  BOOL              BTLV_EFFECT_CheckExecuteGauge( void );
extern  void              BTLV_EFFECT_SetGaugeDrawEnable( BOOL on_off );
extern  void              BTLV_EFFECT_SetGaugeStatus( PokeSick sick,  BtlvMcssPos pos );
extern  void              BTLV_EFFECT_SetBallGauge( const BTLV_BALL_GAUGE_PARAM* bbgp );
extern  void              BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE type );
extern  BOOL              BTLV_EFFECT_CheckExecuteBallGauge( BTLV_BALL_GAUGE_TYPE type );
extern  void              BTLV_EFFECT_SetPaletteFade( int model, u8 start_evy, u8 end_evy, u8 wait, u16 rgb );
extern  BOOL              BTLV_EFFECT_CheckExecutePaletteFade( int model );
extern  void              BTLV_EFFECT_SetVanishFlag( int model, int flag );
extern  int               BTLV_EFFECT_GetTrainerIndex( int position );
extern  BTLV_CAMERA_WORK* BTLV_EFFECT_GetCameraWork( void );
extern  BTLV_MCSS_WORK*   BTLV_EFFECT_GetMcssWork( void );
extern  VMHANDLE*         BTLV_EFFECT_GetVMHandle( void );
extern  GFL_TCBSYS*       BTLV_EFFECT_GetTCBSYS( void );
extern  PALETTE_FADE_PTR  BTLV_EFFECT_GetPfd( void );
extern  BTLV_CLACT_WORK*  BTLV_EFFECT_GetCLWK( void );
extern  BTLV_BG_WORK*     BTLV_EFFECT_GetBGWork( void );

#ifdef PM_DEBUG
extern  void        BTLV_EFFECT_SetPokemonDebug( const MCSS_ADD_DEBUG_WORK *madw, int position );
#endif

