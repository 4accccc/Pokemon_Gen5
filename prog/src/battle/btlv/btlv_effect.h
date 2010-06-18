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
#include "btlv_scu_def.h"

#include "btlv_efftool.h"
#include "btlv_effvm.h"

#include "battle/battle.h"
#include "battle/app/b_bag.h"

#include "waza_tool/wazadata.h"
#include "system/palanm.h"

#include  "waza_tool/wazano_def.h"

//戦闘エフェクト定義
enum{
  BTLEFF_SINGLE_ENCOUNT_1 = WAZANO_MAX + 1,   //561
  BTLEFF_SINGLE_ENCOUNT_2_SOLO,
  BTLEFF_SINGLE_ENCOUNT_2_TAG,
  BTLEFF_SINGLE_ENCOUNT_3,
  BTLEFF_CAMERA_WORK,
  BTLEFF_CAMERA_INIT,
  BTLEFF_SINGLE_TRAINER_ENCOUNT_1,
  BTLEFF_SINGLE_TRAINER_ENCOUNT_2,
  BTLEFF_SINGLE_TRAINER_ENCOUNT_3,
  BTLEFF_BALL_THROW,                          //570
  BTLEFF_HINSHI,
  BTLEFF_SINGLE_ENCOUNT_2_ARARAGI,
  BTLEFF_BALL_THROW_TRAINER,
  BTLEFF_PDC_ENCOUNT,
  BTLEFF_1vs1_POS_AA_FOCUS,
  BTLEFF_2vs2_POS_A_FOCUS,
  BTLEFF_2vs2_POS_C_FOCUS,
  BTLEFF_3vs3_POS_A_FOCUS,
  BTLEFF_3vs3_POS_E_FOCUS,
  BTLEFF_3vs3_CAMERA_ZOOMOUT,                 //580
  BTLEFF_ROTATION_POS_A_FOCUS,
  BTLEFF_ROTATION_POS_A_TO_C,
  BTLEFF_ROTATION_POS_A_TO_E,
  BTLEFF_ROTATION_POS_C_TO_A,
  BTLEFF_ROTATION_POS_E_TO_A,
  BTLEFF_WCS_CAMERA_WORK_E_M,
  BTLEFF_WCS_CAMERA_WORK_M_E,
  BTLEFF_CAMERA_WORK_ROTATE_R_L,
  BTLEFF_CAMERA_WORK_ROTATE_L_R,
  BTLEFF_CAMERA_WORK_UPDOWN_E_M,              //590
  BTLEFF_CAMERA_WORK_UPDOWN_M_E,
  BTLEFF_CAMERA_WORK_TRIANGLE,
  BTLEFF_CAMERA_WORK_WCS_INIT,
  BTLEFF_CAMERA_WORK_INIT,
  BTLEFF_ZOOM_IN_ORTHO, 
  BTLEFF_CAMERA_INIT_ORTHO, 

  //ステータスエフェクト
  BTLEFF_STATUS_EFFECT_START,
  BTLEFF_NEMURI = BTLEFF_STATUS_EFFECT_START, //ねむり 
  BTLEFF_DOKU,                                //どく
  BTLEFF_YAKEDO,                              //やけど
  BTLEFF_KOORI,                               //こおり  600
  BTLEFF_MAHI,                                //まひ
  BTLEFF_KONRAN,                              //こんらん
  BTLEFF_MEROMERO,                            //メロメロ
  BTLEFF_LVUP,                                //レベルアップ
  BTLEFF_USE_ITEM,                            //ポケモンにアイテムを使用
  BTLEFF_SOUBI_ITEM,                          //装備道具発動 10
  BTLEFF_RARE,                                //レアエフェクト
  BTLEFF_STATUS_UP,                           //ステータス上昇
  BTLEFF_STATUS_DOWN,                         //ステータス下降
  BTLEFF_HP_RECOVER,                          //HP回復  610
  BTLEFF_MIGAWARI_WAZA_BEFORE,                //みがわり時に技を繰り出す前エフェクト
  BTLEFF_MIGAWARI_WAZA_AFTER,                 //みがわり時に技を繰り出した後エフェクト
  BTLEFF_ITEM_ESCAPE,                         //エネコのしっぽorピッピ人形
  BTLEFF_WEATHER_KIRI,                        //きり（シャチではないかも？）
  BTLEFF_WEATHER_AME,                         //あめ
  BTLEFF_WEATHER_ARARE,                       //あられ  20
  BTLEFF_WEATHER_SUNAARASHI,                  //すなあらし
  BTLEFF_WEATHER_HARE,                        //はれ
  BTLEFF_POKEMON_MODOSU,                      //ポケモン引っ込めるエフェクト
  BTLEFF_POKEMON_KURIDASU,                    //ポケモン繰り出しエフェクト  620
  BTLEFF_MIGAWARI_FALL,                       //みがわり落ちてくる
  BTLEFF_MIGAWARI_DEAD,                       //みがわり消滅
  BTLEFF_TRAINER_IN,                          //トレーナーフレームイン
  BTLEFF_TRAINER_OUT,                         //トレーナーフレームアウト
  BTLEFF_KIAIPUNCH_TAME,                      //きあいパンチのタメエフェクト
  BTLEFF_NOROI,                               //のろい  30
  BTLEFF_AKUMU,                               //あくむ
  BTLEFF_YADORIGI,                            //やどりぎ
  BTLEFF_SIMETUKERU,                          //しめつける
  BTLEFF_MAKITUKU,                            //まきつく
  BTLEFF_HONOONOUZU,                          //ほのおのうず
  BTLEFF_MAGUMASUTOOMU,                       //マグマストーム
  BTLEFF_KARADEHASAMU,                        //からではさむ  630
  BTLEFF_UZUSIO,                              //うずしお
  BTLEFF_SUNAZIGOKU,                          //すなじごく
  BTLEFF_NEWOHARU,                            //ねをはる  40
  BTLEFF_ZOOM_IN,                             //カメラズームイン  634
  BTLEFF_IYASINONEGAI_KAIHUKU,                //いやしのねがい回復エフェクト
  BTLEFF_MIKADUKINOMAI_KAIHUKU,               //みかづきのまい回復エフェクト
  BTLEFF_RESET_MOVE,                          //リセットムーブエフェクト
  BTLEFF_SHOOTER_EFFECT,                      //シューターエフェクト
  BTLEFF_TONBOGAERI_RETURN,                   //とんぼがえり戻りエフェクト
  BTLEFF_VOLTCHANGE_RETURN,                   //ボルトチェンジ戻りエフェクト
  BTLEFF_POKEMON_VANISH_ON,                   //ポケモンバニッシュON
  BTLEFF_POKEMON_VANISH_OFF,                  //ポケモンバニッシュOFF
  BTLEFF_RESHUFFLE_FOCUS_DEFAULT,             //入れ替えカメラフォーカスデフォルト 50
  BTLEFF_RESHUFFLE_FOCUS_2vs2_POS_A,          //入れ替えカメラフォーカス2vs2POS_A
  BTLEFF_RESHUFFLE_FOCUS_3vs3_POS_A,          //入れ替えカメラフォーカス3vs3POS_A
  BTLEFF_RESHUFFLE_FOCUS_3vs3_POS_E,          //入れ替えカメラフォーカス3vs3POS_E
  BTLEFF_TOMOENAGE_RETURN,                    //ともえなげ戻りエフェクト
  BTLEFF_DRAGONTAIL_RETURN,                   //ドラゴンテール戻りエフェクト
  BTLEFF_RAINBOW,                             //虹エフェクト
  BTLEFF_BURNING,                             //火の海エフェクト
  BTLEFF_MOOR,                                //湿地エフェクト
  BTLEFF_DAMAGE,                              //ダメージエフェクト

  BTLEFF_STATUS_EFFECT_END,
};

typedef struct _BTLV_EFFECT_SETUP_PARAM BTLV_EFFECT_SETUP_PARAM;
typedef struct _BTLV_EFFECT_WORK        BTLV_EFFECT_WORK;

typedef void  (BTLV_EFFECT_TCB_CALLBACK_FUNC)( GFL_TCB* tcb );

#define BTLV_EFFECT_GAUGE_YURE_STOP ( BTLV_MCSS_POS_MAX )

//OBJパレット使用内訳
enum{
  BTLV_OBJ_PLTT_HP_GAUGE    = 0x20 * 0,   //0-5:HPゲージ（6本分）
  BTLV_OBJ_PLTT_BALL_GAUGE  = 0x20 * 6,   //6:ボールゲージ
  BTLV_OBJ_PLTT_TIMER       = 0x20 * 7,   //7:タイマー
  BTLV_OBJ_PLTT_STATUS_ICON = 0x20 * 8,   //8:状態異常アイコン
  BTLV_OBJ_PLTT_CLACT       = 0x20 * 9,   //9:BTLV_CLACTで使用するセルアクター（7個分）
};

//BTLV_EFFECT_SetTCBで使用するグループ指定
typedef enum{ 
  GROUP_DEFAULT = 0,
  GROUP_EFFVM,
  GROUP_MCSS,
  GROUP_CLACT,
}BTLV_EFFECT_TCB_GROUP;

typedef enum{ 
  BTLV_EFFECT_CWE_NONE = 0,
  BTLV_EFFECT_CWE_NORMAL,         //通常動作
  BTLV_EFFECT_CWE_NO_STOP,        //入力があってもCAMERA_INITを呼ばない
}BTLV_EFFECT_CWE;

typedef enum{ 
  BTLV_EFFECT_SE_MODE_PLAY = 0, //SE再生する
  BTLV_EFFECT_SE_MODE_MUTE,     //SE再生しない
}BTLV_EFFECT_SE_MODE;

typedef enum{ 
  BTLV_EFFECT_REVERSE_DRAW_OFF  = BTLV_MCSS_REVERSE_DRAW_OFF,
  BTLV_EFFECT_REVERSE_DRAW_ON   = BTLV_MCSS_REVERSE_DRAW_ON,
}BTLV_EFFECT_REVERSE_DRAW;

//--------------------------------------------
/**
 *  ターン種類指定
 */
//--------------------------------------------
typedef enum {
  BTLV_WAZAEFF_INDEX_DEFAULT = 0,

  // 溜めワザ全般（そらをとぶなど）
  BTLV_WAZAEFF_TAME_START = 0,      ///< 溜め開始
  BTLV_WAZAEFF_TAME_RELEASE,        ///< 溜め解放

  // 時間差ワザ全般（みらいよち、はめつのねがい等）
  BTLV_WAZAEFF_DELAY_START = 0,     ///< ワザ撃ち
  BTLV_WAZAEFF_DELAY_ATTACK,        ///< 攻撃実行

  // どろぼう
  BTLV_WAZAEFF_DOROBOU_NORMAL = 0,  ///< ヒットのみ
  BTLV_WAZAEFF_DOROBOU_STEAL,       ///< ヒット＆盗み成功

  // のろい
  BTLV_WAZAEFF_NOROI_NORMAL = 0,    ///< ゴースト以外
  BTLV_WAZAEFF_NOROI_GHOST,         ///< ゴースト

  // プレゼント
  BTLV_WAZAEFF_PRESENT_DAMAGE = 0,  ///< ダメージ
  BTLV_WAZAEFF_PRESENT_RECOVER,     ///< HP回復

  // かわらわり
  BTLV_WAZAEFF_KAWARAWARI_DEFAULT = 0, ///< 通常
  BTLV_WAZAEFF_KAWARAWARI_BREAK,       ///< リフレクター等を破壊

  // ウェザーボール
  BTLV_WAZAEFF_WEATHERBALL_NORMAL = 0,  ///< ノーマル
  BTLV_WAZAEFF_WEATHERBALL_SHINE,       ///< にほんばれ
  BTLV_WAZAEFF_WEATHERBALL_SNOW,        ///< あられ
  BTLV_WAZAEFF_WEATHERBALL_SAND,        ///< すなあらし
  BTLV_WAZAEFF_WEATHERBALL_RAIN,        ///< あまごい

  // テクノバスター
  BTLV_WAZAEFF_TECKNOBASTER_NORMAL = 0, ///< ノーマル
  BTLV_WAZAEFF_TECKNOBASTER_MIZU,       ///< 水
  BTLV_WAZAEFF_TECKNOBASTER_DENKI,      ///< 電気
  BTLV_WAZAEFF_TECKNOBASTER_HONOO,      ///< 炎
  BTLV_WAZAEFF_TECKNOBASTER_KOORI,      ///< こおり

  // フレイムソウル・サンダーソウル
  BTLV_WAZAEFF_LINKWAZA_NORMAL = 0,     ///< 通常
  BTLV_WAZAEFF_LINKWAZA_LINK,           ///< リンクした（続けて出した）

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
  WazaTarget   waza_range;
}BTLV_WAZAEFFECT_PARAM;


typedef enum{
  BTLV_EFFECT_ROTATE_DIR_LEFT = 0,
  BTLV_EFFECT_ROTATE_DIR_RIGHT,
}BTLV_EFFECT_ROTATE_DIR;

extern  BTLV_EFFECT_SETUP_PARAM*  BTLV_EFFECT_MakeSetUpParam( BtlRule rule, const BTL_FIELD_SITUATION* bfs, BOOL multi,
                                                              u16* tr_type, const BTL_MAIN_MODULE* mainModule,
                                                              const BTLV_SCU* scu, HEAPID heapID );

extern  BTLV_EFFECT_SETUP_PARAM*  BTLV_EFFECT_MakeSetUpParamBtl( const BTL_MAIN_MODULE* mainModule, const BTLV_SCU* viewSCU, HEAPID heapID );

extern  void              BTLV_EFFECT_Init( BTLV_EFFECT_SETUP_PARAM* besp, GFL_FONT* fontHandle, HEAPID heapID );
extern  void              BTLV_EFFECT_Exit( void );
extern  void              BTLV_EFFECT_Main( void );
extern  void              BTLV_EFFECT_Add( int eff_no );
extern  void              BTLV_EFFECT_AddByPos( BtlvMcssPos pos, int eff_no );
extern  void              BTLV_EFFECT_AddByDir( BtlvMcssPos from, BtlvMcssPos to, int eff_no );
extern  void              BTLV_EFFECT_AddWazaEffect( const BTLV_WAZAEFFECT_PARAM* param );
extern  void              BTLV_EFFECT_Stop( void );
extern  void              BTLV_EFFECT_Restart( void );

extern  void              BTLV_EFFECT_Damage( BtlvMcssPos target, WazaID waza );
extern  void              BTLV_EFFECT_Hinshi( BtlvMcssPos target );

extern  BOOL              BTLV_EFFECT_CheckExecute( void );
extern  void              BTLV_EFFECT_SetPokemon( const POKEMON_PARAM *pp, int position );
extern  void              BTLV_EFFECT_DelPokemon( int position );
extern  BOOL              BTLV_EFFECT_CheckExist( int position );
extern  void              BTLV_EFFECT_SetTrainer( int trtype, int position, int pos_x, int pos_y, int pos_z );
extern  void              BTLV_EFFECT_DelTrainer( int position );
extern  void              BTLV_EFFECT_SetGauge( const BTL_MAIN_MODULE* wk, const BTL_POKEPARAM* bpp, int position );
extern  void              BTLV_EFFECT_SetGaugePP( const ZUKAN_SAVEDATA* zs, const POKEMON_PARAM* pp, int position );
extern  void              BTLV_EFFECT_DelGauge( int position );
extern  void              BTLV_EFFECT_CalcGaugeHP( int position, int value );
extern  void              BTLV_EFFECT_CalcGaugeHPAtOnce( int position, int value );
extern  void              BTLV_EFFECT_CalcGaugeEXP( int position, int value );
extern  void              BTLV_EFFECT_CalcGaugeEXPLevelUp( int position, const BTL_POKEPARAM* bpp );
extern  void              BTLV_EFFECT_BallThrow( int position, u16 item_no, u8 yure_cnt, BOOL f_success, BOOL f_critical );
extern  void              BTLV_EFFECT_BallThrowTrainer( int vpos, u16 item_no );
extern  void              BTLV_EFFECT_Henge( const POKEMON_PARAM* pp, BtlvMcssPos vpos );
extern  void              BTLV_EFFECT_HengeShortCut( const POKEMON_PARAM* pp, BtlvMcssPos vpos );
extern  void              BTLV_EFFECT_PokemonVanishOn( BtlvMcssPos vpos );
extern  void              BTLV_EFFECT_PokemonVanishOff( BtlvMcssPos vpos );
extern  void              BTLV_EFFECT_CreateMigawari( BtlvMcssPos vpos );
extern  void              BTLV_EFFECT_DeleteMigawari( BtlvMcssPos vpos );
extern  BOOL              BTLV_EFFECT_CheckExecuteGauge( void );
extern  void              BTLV_EFFECT_SetGaugeDrawEnable( BOOL on_off, int side );
extern  void              BTLV_EFFECT_SetGaugeStatus( PokeSick sick,  BtlvMcssPos pos );
extern  void              BTLV_EFFECT_SetGaugeYure( BtlvMcssPos pos );
extern  BOOL              BTLV_EFFECT_CheckExistGauge( BtlvMcssPos pos );
extern  BOOL              BTLV_EFFECT_GetGaugeStatus( BtlvMcssPos pos, int* color, int* sick_anm );
extern  void              BTLV_EFFECT_SetBallGauge( const BTLV_BALL_GAUGE_PARAM* bbgp );
extern  void              BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE type );
extern  BOOL              BTLV_EFFECT_CheckExecuteBallGauge( BTLV_BALL_GAUGE_TYPE type );
extern  void              BTLV_EFFECT_SetPaletteFade( int model, u8 start_evy, u8 end_evy, u8 wait, u16 rgb );
extern  BOOL              BTLV_EFFECT_CheckExecutePaletteFade( int model );
extern  void              BTLV_EFFECT_SetVanishFlag( int model, int flag );
extern  void              BTLV_EFFECT_SetRotateEffect( BtlRotateDir dir, int side );
extern  void              BTLV_EFFECT_SetSideChange( BtlvMcssPos pos1, BtlvMcssPos pos2 );
extern  int               BTLV_EFFECT_GetTrainerIndex( int position );
extern  void              BTLV_EFFECT_CreateTimer( int game_time, int command_time );
extern  void              BTLV_EFFECT_DrawEnableTimer( BTLV_TIMER_TYPE type, BOOL enable, BOOL init );
extern  BOOL              BTLV_EFFECT_IsZero( BTLV_TIMER_TYPE type );
extern  BTLV_MCSS_VANISH_FLAG BTLV_EFFECT_GetMcssVanishFlag( BtlvMcssPos position );
extern  void              BTLV_EFFECT_SetMcssVanishFlag( BtlvMcssPos position, BTLV_MCSS_VANISH_FLAG flag );
extern  void              BTLV_EFFECT_SetCameraFocus( BtlvMcssPos position, int move_type, int frame, int wait, int brake );
extern  void              BTLV_EFFECT_GetCameraFocus( BtlvMcssPos position, VecFx32* pos, VecFx32* target );
extern  BTLV_EFFECT_WORK* BTLV_EFFECT_GetEffectWork( void );
extern  BTLV_CAMERA_WORK* BTLV_EFFECT_GetCameraWork( void );
extern  BTLV_MCSS_WORK*   BTLV_EFFECT_GetMcssWork( void );
extern  BTLV_STAGE_WORK*  BTLV_EFFECT_GetStageWork( void );
extern  BTLV_GAUGE_WORK*  BTLV_EFFECT_GetGaugeWork( void );
extern  VMHANDLE*         BTLV_EFFECT_GetVMHandle( void );
extern  GFL_TCBSYS*       BTLV_EFFECT_GetTCBSYS( void );
extern  PALETTE_FADE_PTR  BTLV_EFFECT_GetPfd( void );
extern  BTLV_CLACT_WORK*  BTLV_EFFECT_GetCLWK( void );
extern  BTLV_BG_WORK*     BTLV_EFFECT_GetBGWork( void );
extern  BTLV_TIMER_WORK*  BTLV_EFFECT_GetTimerWork( void );
extern  BtlRule           BTLV_EFFECT_GetBtlRule( void );
extern  BOOL              BTLV_EFFECT_GetMulti( void );
extern  int               BTLV_EFFECT_GetTrType( int pos );
extern  const BTL_MAIN_MODULE* BTLV_EFFECT_GetMainModule( void );
extern  const BTLV_SCU*   BTLV_EFFECT_GetScu( void );
extern  int               BTLV_EFFECT_GetPinchBGMFlag( void );
extern  void              BTLV_EFFECT_SetTrainerBGMChangeFlag( int bgm_no );
extern  BOOL              BTLV_EFFECT_GetTrainerBGMChangeFlag( void );
extern  void              BTLV_EFFECT_SetBGMNoCheckPush( int bgm_no );
extern  void              BTLV_EFFECT_SetBagMode( BtlBagMode bagMode );
extern  BtlBagMode        BTLV_EFFECT_GetBagMode( void );
extern  void              BTLV_EFFECT_SetSEMode( BTLV_EFFECT_SE_MODE mode );
extern  BTLV_EFFECT_SE_MODE  BTLV_EFFECT_GetSEMode( void );
extern  BOOL              BTLV_EFFECT_CheckShooterEnable( void );
extern  BOOL              BTLV_EFFECT_CheckItemEnable( void );
extern  void              BTLV_EFFECT_SetReverseDrawFlag( BTLV_EFFECT_REVERSE_DRAW draw_flag );

extern  void              BTLV_EFFECT_SetTCB( GFL_TCB* tcb, BTLV_EFFECT_TCB_CALLBACK_FUNC* callback_func,
                                              BTLV_EFFECT_TCB_GROUP group );
extern  int               BTLV_EFFECT_SearchTCBIndex( GFL_TCB* tcb );
extern  void              BTLV_EFFECT_FreeTCB( GFL_TCB* tcb );
extern  void              BTLV_EFFECT_FreeTCBGroup( BTLV_EFFECT_TCB_GROUP group );
extern  void              BTLV_EFFECT_SetCameraWorkExecute( BTLV_EFFECT_CWE cwe );
extern  void              BTLV_EFFECT_SetCameraWorkStop( void );
extern  void              BTLV_EFFECT_SetCameraWorkSwitch( BTLV_EFFECT_CWE type );

#ifdef PM_DEBUG
extern  void        BTLV_EFFECT_SetPokemonDebug( const MCSS_ADD_DEBUG_WORK *madw, int position );
extern  void        BTLV_EFFECT_SetBtlRule( BtlRule rule );
#endif

