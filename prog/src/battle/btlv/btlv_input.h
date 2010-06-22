
//============================================================================================
/**
 * @file  btlv_input.h
 * @brief 戦闘下画面
 * @author  soga
 * @date  2009.06.30
 */
//============================================================================================

#pragma once

#include "..\btl_common.h"
#include "..\btl_client.h"
#include "system/palanm.h"
#include "gamesystem/game_data.h"

#define BTLV_INPUT_POKEMON_MAX ( 3 )            //入力するポケモンMAX
#define BTLV_INPUT_DIR_MAX  ( TEMOTI_POKEMAX )  //対象選択MAX
#define BTLV_INPUT_POKEICON_MAX ( 3 )           //ポケモンアイコンMAX
#define CANCEL_FLAG_MASK    ( 2 )
#define CANCEL_FLAG_SHIFT   ( 1 )

#define BTLV_INPUT_CANCEL_BUTTON      ( 4 )     //キャンセルボタンを押したときに返す値
#define BTLV_INPUT_WARUAGAKI_BUTTON   ( 7 )     //わるあがきボタンを押したときに返す値

//戦闘下画面OBJパレット内訳
/*
0:モンスターボール
1:わるあがきボタン
2:技＆対象選択ボタンアニメ
3:天候アイコンうわもの
4:天候アイコンベース
5:天候アイコンベース
6:天候アイコンベース
7:天候アイコンベース
8:技タイプアイコン
9:技タイプアイコン
A:技タイプアイコン
B:ポケモンアイコン
C:ポケモンアイコン
D:ポケモンアイコン
E:通信アイコン
F:ローカライズ用予約
*/

typedef struct _BTLV_INPUT_WORK BTLV_INPUT_WORK;
typedef void  (BTLV_INPUT_TCB_CALLBACK_FUNC)( GFL_TCB* tcb );

///技選択画面構成用のデータ
typedef struct{
  WazaID      wazano[ PTL_WAZA_MAX ];
  u8          pp[ PTL_WAZA_MAX ];
  u8          ppmax[ PTL_WAZA_MAX ];
  BtlvMcssPos pos;
  BOOL        henshin_flag;
}BTLV_INPUT_WAZA_PARAM;

///攻撃対象選択画面構成用のデータ
typedef struct{
  u8                    sex   :2;
  u8                    exist :1;
  u8                          :5;
  u8                    status;
  u16                   dummy;

  s16                   hp;             //現在のHP
  u16                   hpmax;          //現在のHPMAX
  const POKEMON_PARAM*  pp;
}BTLV_INPUT_DIR_PARAM;

///ポケモン選択画面構成用のSceneワーク
typedef struct{
  BTLV_INPUT_DIR_PARAM  bidp[ BTLV_INPUT_DIR_MAX ];
  u8                    pos;
  u8                    waza_target;  ///<対象選択タイプ
}BTLV_INPUT_SCENE_PARAM;

///YES/NO選択画面構成用のSceneワーク
typedef struct{
  STRBUF*   yes_msg;        //YESに相当するメッセージ
  STRBUF*   no_msg;         //NOに相当するメッセージ
  BOOL      b_cancel_flag;  //Bキャンセル有効か無効か？
}BTLV_INPUT_YESNO_PARAM;

enum
{
  BTLV_INPUT_BR_SEL_REW = 0,  //巻き戻し
  BTLV_INPUT_BR_SEL_FF,       //早送り
  BTLV_INPUT_BR_SEL_STOP,     //停止
};

typedef enum
{
  BTLV_INPUT_BR_STOP_NONE = 0,
  BTLV_INPUT_BR_STOP_KEY,         //キーによる中断
  BTLV_INPUT_BR_STOP_BREAK,       //データ破壊による中断
  BTLV_INPUT_BR_STOP_OVER,        //録画時間オーバーによる中断
  BTLV_INPUT_BR_STOP_SKIP,        //チャプタースキップによる中断
}BTLV_INPUT_BR_STOP_FLAG;

//バトルレコーダー画面構成用のSceneワーク
typedef struct{
  int                       play_chapter;   //上画面で再生中のチャプタ
  int                       view_chapter;   //下画面に表示するチャプタ
  int                       max_chapter;    //総チャプタ数
  BTLV_INPUT_BR_STOP_FLAG   stop_flag;      //再生停止フラグ
}BTLV_INPUT_BATTLE_RECORDER_PARAM;

typedef enum
{
  BTLV_INPUT_CENTER_BUTTON_ESCAPE = FALSE,
  BTLV_INPUT_CENTER_BUTTON_MODORU = TRUE,
}BTLV_INPUT_CENTER_BUTTON_TYPE;

///コマンド選択画面構成用のワーク
typedef struct{
  BTLV_INPUT_DIR_PARAM          bidp[ 2 ][ BTLV_INPUT_DIR_MAX ];
  BTLV_INPUT_CENTER_BUTTON_TYPE center_button_type;
  BOOL                          trainer_flag;                         //対戦相手がトレーナーかどうか
  int                           mons_no[ BTLV_INPUT_POKEICON_MAX ];   //繰り出しているポケモンナンバー
  int                           form_no[ BTLV_INPUT_POKEICON_MAX ];   //繰り出しているポケモンのフォルムナンバー
  int                           sex[ BTLV_INPUT_POKEICON_MAX ];       //繰り出しているポケモンの性別
  BtlvMcssPos                   pos;
  BtlBagMode                    bagMode;                              //バッグモード
  u8                            shooterEnergy;                        //シューターエネルギー
}BTLV_INPUT_COMMAND_PARAM;

///ローテーション画面構成用のワーク
#ifdef ROTATION_NEW_SYSTEM
typedef struct{
  const BTL_POKEPARAM*  bpp[ BTL_ROTATE_NUM ];
  BOOL  waza_exist[ BTL_ROTATE_NUM ][PTL_WAZA_MAX ];
}BTLV_INPUT_ROTATE_PARAM;
#else
typedef struct{
  const POKEMON_PARAM*  pp[ BTL_ROTATE_NUM ];
  BtlRotateDir          before_select_dir;                  //1つ前に選択した方向
}BTLV_INPUT_ROTATE_PARAM;
#endif

typedef enum
{
  BTLV_INPUT_TYPE_SINGLE = 0,
  BTLV_INPUT_TYPE_DOUBLE,
  BTLV_INPUT_TYPE_TRIPLE,
  BTLV_INPUT_TYPE_ROTATION,
}BTLV_INPUT_TYPE;

typedef enum
{
  BTLV_INPUT_SCRTYPE_STANDBY = 0,
  BTLV_INPUT_SCRTYPE_COMMAND,
  BTLV_INPUT_SCRTYPE_WAZA,
  BTLV_INPUT_SCRTYPE_DIR,
  BTLV_INPUT_SCRTYPE_YES_NO,
  BTLV_INPUT_SCRTYPE_ROTATE,
  BTLV_INPUT_SCRTYPE_BATTLE_RECORDER,
  BTLV_INPUT_SCRTYPE_PDC,

  BTLV_INPUT_SCRTYPE_MAX,
}BTLV_INPUT_SCRTYPE;

//SCRTYPE_COMMANDのstatusパラメータの意味
enum
{
  BTLV_INPUT_STATUS_NONE = 0, ///<ポケモンがいない
  BTLV_INPUT_STATUS_ALIVE,    ///<ポケモンがいる（いきてる）
  BTLV_INPUT_STATUS_DEAD,     ///<ポケモンがいる（気絶）
  BTLV_INPUT_STATUS_NG,       ///<ポケモンがいる（ステータス異常）
};

typedef struct
{
  const GFL_UI_TP_HITTBL* hit_tbl;
  const BOOL*             cancel_flag;
  const int*              button_pltt;
}BTLV_INPUT_HITTBL;

typedef struct
{
  s8 cur_pos[ 6 ];  //カーソルポジション
  s8 up;            //上キーを押したときの移動先
  s8 down;          //下キーを押したときの移動先
  s8 left;          //左キーを押したときの移動先
  s8 right;         //右キーを押したときの移動先
  s8 a_button;      //Aボタンを押したときのどこを押したことにするか
  s8 b_button;      //キャンセル可能かどうか
}BTLV_INPUT_KEYTBL;

#define BTLV_INPUT_NOMOVE   ( -128 )
#define BTLV_INPUT_MOVETBL  ( 0x40 )    //カーソル移動の位置をテーブルで列記するフラグ

typedef u32 BTLV_INPUT_ROTATE_RESULT; //上位16bit:BtlRotateDir  下位16bit:選択した技の位置orキャンセル

//BtlRotateDir取得
static inline u16 BTLV_INPUT_GetBtlRotateDir( BTLV_INPUT_ROTATE_RESULT result )
{
  return ( result & 0xffff0000 ) >> 16;
}

//選択した技の位置orキャンセル取得
static inline u16 BTLV_INPUT_GetSelectWaza( BTLV_INPUT_ROTATE_RESULT result )
{
  return ( result & 0xffff );
}

#define BTLV_INPUT_WAZAINFO_MASK  (0x8000)  // ワザ説明モードへの遷移を示すマスク用Bit

static inline int BTLV_INPUT_SetWazaInfoModeMask( int hit_pos )
{
  return hit_pos | BTLV_INPUT_WAZAINFO_MASK;
}
static inline BOOL BTLV_INPUT_CheckWazaInfoModeMask( int* hit_pos )
{
  BOOL fWazaInfoMode = ((*hit_pos) & BTLV_INPUT_WAZAINFO_MASK) != 0;
  (*hit_pos) &= (~(BTLV_INPUT_WAZAINFO_MASK));
  return fWazaInfoMode;
}

extern  BTLV_INPUT_WORK*  BTLV_INPUT_Init( GAMEDATA* gameData, BTLV_INPUT_TYPE type, BtlCompetitor comp,
                                           PALETTE_FADE_PTR pfd, GFL_FONT* font, u8* cursor_flag, BtlBagMode bagMode,
                                           HEAPID heapID );
extern  BTLV_INPUT_WORK*  BTLV_INPUT_InitEx( GAMEDATA* gameData, BTLV_INPUT_TYPE type, PALETTE_FADE_PTR pfd,
                                             GFL_FONT* font, u8* cursor_flag, HEAPID heapID );
extern  void              BTLV_INPUT_Exit( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_Main( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_InitBG( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_ExitBG( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_SetFrame( void );
extern  void              BTLV_INPUT_FreeFrame( void );
extern  void              BTLV_INPUT_SetFadeOut( BTLV_INPUT_WORK* biw, BOOL start );
extern  void              BTLV_INPUT_SetFadeIn( BTLV_INPUT_WORK* biw );
extern  BOOL              BTLV_INPUT_CheckFadeExecute( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_CreateScreen( BTLV_INPUT_WORK* biw, BTLV_INPUT_SCRTYPE type, void* param );
extern  int               BTLV_INPUT_CheckInput( BTLV_INPUT_WORK* biw, const BTLV_INPUT_HITTBL* tp_tbl,
                                                 const BTLV_INPUT_KEYTBL* key_tbl );
extern  BOOL              BTLV_INPUT_CheckInputDemo( BTLV_INPUT_WORK* biw );
extern  BOOL              BTLV_INPUT_CheckInputRotate( BTLV_INPUT_WORK* biw, BtlRotateDir* dir, int* select );
extern  BOOL              BTLV_INPUT_CheckExecute( BTLV_INPUT_WORK* biw );

