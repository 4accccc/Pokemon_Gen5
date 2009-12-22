
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

#define BTLV_INPUT_DIR_MAX  ( TEMOTI_POKEMAX )  //対象選択MAX
#define BTLV_INPUT_POKEICON_MAX ( 3 )           //ポケモンアイコンMAX

//戦闘下画面OBJパレット内訳
/*
0:モンスターボール
1:
2:技＆対象選択ボタンアニメ
3:天候アイコンうわもの
4:天候アイコンベース
5:天候アイコンベース
6:天候アイコンベース
7:天候アイコンベース
8:技タイプアイコン
9:技タイプアイコン
A:技タイプアイコン
B:空き
C:ポケモンアイコン
D:ポケモンアイコン
E:ポケモンアイコン
F:ローカライズ用予約
*/

typedef struct _BTLV_INPUT_WORK BTLV_INPUT_WORK;

///技選択画面構成用のデータ
typedef struct{
  u16         wazano[ PTL_WAZA_MAX ];
  u16         pp[ PTL_WAZA_MAX ];
  u16         ppmax[ PTL_WAZA_MAX ];
  BtlvMcssPos pos;
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
  STRBUF*   yes_msg;    //YESに相当するメッセージ
  STRBUF*   no_msg;     //NOに相当するメッセージ
}BTLV_INPUT_YESNO_PARAM;

typedef enum
{
  BTLV_INPUT_CENTER_BUTTON_ESCAPE = FALSE,
  BTLV_INPUT_CENTER_BUTTON_MODORU = TRUE,
}BTLV_INPUT_CENTER_BUTTON_TYPE;

///コマンド選択画面構成用のワーク
typedef struct{
  BTLV_INPUT_DIR_PARAM          bidp[ 2 ][ BTLV_INPUT_DIR_MAX ];
  BTLV_INPUT_CENTER_BUTTON_TYPE center_button_type;
  BOOL                          trainer_flag;                    //対戦相手がトレーナーかどうか
  int                           mons_no[ BTLV_INPUT_POKEICON_MAX ];   //繰り出しているポケモンナンバー
  int                           form_no[ BTLV_INPUT_POKEICON_MAX ];   //繰り出しているポケモンのフォルムナンバー
  BtlvMcssPos                   pos;
}BTLV_INPUT_COMMAND_PARAM;

///ローテーション画面構成用のワーク
typedef struct{
  const POKEMON_PARAM*  pp[ BTL_ROTATE_NUM ];
  BtlRotateDir          before_select_dir;                  //1つ前に選択した方向
}BTLV_INPUT_ROTATE_PARAM;

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
  s8 cur_pos[ 6 ];  //カーソルポジション
  s8 up;            //上キーを押したときの移動先
  s8 down;          //下キーを押したときの移動先
  s8 left;          //左キーを押したときの移動先
  s8 right;         //右キーを押したときの移動先
  s8 a_button;      //Aボタンを押したときのどこを押したことにするか
  s8 b_button;      //キャンセル可能かどうか
}BTLV_INPUT_KEYTBL;

#define BTLV_INPUT_NOMOVE ( -128 )

extern  BTLV_INPUT_WORK*  BTLV_INPUT_Init( BTLV_INPUT_TYPE type, const BTL_CLIENT* client, GFL_FONT* font, u8* cursor_flag, HEAPID heapID );
extern  void              BTLV_INPUT_Exit( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_Main( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_InitBG( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_ExitBG( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_SetFrame( void );
extern  void              BTLV_INPUT_FreeFrame( void );
extern  void              BTLV_INPUT_SetFadeOut( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_SetFadeIn( BTLV_INPUT_WORK* biw );
extern  BOOL              BTLV_INPUT_CheckFadeExecute( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_CreateScreen( BTLV_INPUT_WORK* biw, BTLV_INPUT_SCRTYPE type, void* param );
extern  int               BTLV_INPUT_CheckInput( BTLV_INPUT_WORK* biw, const GFL_UI_TP_HITTBL* tp_tbl,
                                                 const BTLV_INPUT_KEYTBL* key_tbl );

