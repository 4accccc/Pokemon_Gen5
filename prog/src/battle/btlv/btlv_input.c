//============================================================================================
/**
 * @file  btlv_input.c
 * @brief 戦闘下画面
 * @author  soga
 * @date  2009.06.29
 */
//============================================================================================
#include <gflib.h>

#include "btlv_effect.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "waza_tool/wazadata.h"
#include "system/palanm.h"
#include "system/bmp_oam.h"
#include "infowin/infowin.h"
#include "pokeicon/pokeicon.h"
#include "system/sdkdef.h"

#include "app/app_menu_common.h"

#include "battle/btl_field.h"

#include "btlv_input.h"
#include "data/btlv_input.cdat"

#include "arc_def.h"
#include "battle/battgra_wb.naix"

#include "pm_define.h"

#include "sound/pm_sndsys.h"

#include "message.naix"
#include "msg/msg_btlv_input.h"

#include "data/rotate_sel.cdat"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================

#define BTLV_INPUT_TCB_MAX ( 8 )    //使用するTCBのMAX
#define BTLV_INPUT_BUTTON_MAX ( 6 + 1 )  //押せるボタンのMAX値（3vs3時の対象選択6個＋キャンセル1個）
#define BG_CLEAR_CODE     (0x8000 / 0x20 - 1)   ///BGスクリーンのクリアコード（キャラクタの一番後ろを指定）

#define INFOWIN_PAL_NO  ( 0x0f )  //情報ステータスバーパレット
#define BMPWIN_PAL_NO  ( 0x0d )   //BMPWINパレット

#define WAZATYPEICON_COMP_CHAR  ( 0 )
#define WAZATYPEICON_OAMSIZE    ( 32 * 8 )

#define STANDBY_PAL        ( 0x0003 )
#define STANDBY_FADE        ( 12 )
#define STANDBY_FADE_COLOR  ( 0x0842 )

//PP表示用カラー定義
#define MSGCOLOR_PP_WHITE   ( PRINTSYS_LSB_Make( 1, 2, 0 ) )
#define MSGCOLOR_PP_YELLOW  ( PRINTSYS_LSB_Make( 3, 4, 0 ) )
#define MSGCOLOR_PP_ORANGE  ( PRINTSYS_LSB_Make( 5, 6, 0 ) )
#define MSGCOLOR_PP_RED     ( PRINTSYS_LSB_Make( 7, 8, 0 ) )

//TCB_TransformStandby2Command
#define TTS2C_SCROLL_COUNT ( 8 )
#define TTS2C_START_SCALE ( FX32_ONE * 3 )
#define TTS2C_END_SCALE ( FX32_ONE )
#define TTS2C_START_SCROLL_X ( 0 )
#define TTS2C_START_SCROLL_Y ( 0x1c0 )
#define TTS2C_SCALE_SPEED ( ( FX32_ONE * 2 ) / TTS2C_SCROLL_COUNT )
#define TTS2C_SCROLL_SPEED ( 64 / TTS2C_SCROLL_COUNT )
#define TTS2C_FRAME1_SCROLL_X ( 0 )
#define TTS2C_FRAME1_SCROLL_Y ( 0 )

//TCB_TransformCommand2Waza
#define TTC2W_SCROLL_COUNT ( 8 )
#define TTC2W_START_SCROLL_X ( 256 )
#define TTC2W_START_SCROLL_Y ( 0x1c0 )
#define TTC2W_SCROLL_SPEED ( 64 / TTC2W_SCROLL_COUNT )

//TCB_TransformWaza2Command
#define TTW2C_SCROLL_COUNT ( 8 )
#define TTW2C_START_SCROLL_X ( 0 )
#define TTW2C_START_SCROLL_Y ( 0x1c0 )
#define TTW2C_SCROLL_SPEED ( 64 / TTW2C_SCROLL_COUNT )

//TCB_TransformCommand2Standby
#define TTC2S_SCALE_COUNT ( 8 )
#define TTC2S_SCALE_SPEED ( ( FX32_ONE * 2 ) / TTC2S_SCALE_COUNT )
#define TTC2S_START_SCALE ( FX32_ONE )
#define TTC2S_END_SCALE   ( FX32_ONE * 3 )

#define STANBY_POS_Y  ( 128 + 24 )
#define COMMAND_POS_Y  ( 128 + 40 )

//TCB_SCREEN_ANIME用のスクロール値
enum{
  TSA_SCROLL_X = 0,
  TSA_SCROLL_Y,

  TSA_SCROLL_X0 = 0,
  TSA_SCROLL_Y0 = 0,
  TSA_SCROLL_X1 = 256,
  TSA_SCROLL_Y1 = 0,
  TSA_SCROLL_X2 = 0,
  TSA_SCROLL_Y2 = 192,
  TSA_SCROLL_X3 = 256,
  TSA_SCROLL_Y3 = 192,

  TSA_WAIT = 2,
};

typedef enum
{
  SCREEN_ANIME_DIR_FORWARD = 0,
  SCREEN_ANIME_DIR_BACKWARD,
}SCREEN_ANIME_DIR;

//TCB_BUTTON～系定義
enum{
  BUTTON_ANIME_MAX = BTLV_INPUT_DIR_MAX,     //3対3時の攻撃対象選択がマックスのはず

  BUTTON2_APPEAR_ANIME = 8,
  BUTTON2_VANISH_ANIME = 9,

  BUTTON4_APPEAR_ANIME = 8,
  BUTTON4_VANISH_ANIME = 9,

  BUTTON6_APPEAR_ANIME = 8,
  BUTTON6_VANISH_ANIME = 9,

  //BUTTON_TYPE_YES_NO
  BUTTON2_X1 = 64,
  BUTTON2_Y1 = 56,
  BUTTON2_X2 = 128 + 64,
  BUTTON2_Y2 = 56,

  //BUTTON_TYPE_WAZA & DIR_4
  BUTTON4_X1 = 64,
  BUTTON4_Y1 = 56,
  BUTTON4_X2 = 128 + 64,
  BUTTON4_Y2 = 56,
  BUTTON4_X3 = 64,
  BUTTON4_Y3 = 64 + 40,
  BUTTON4_X4 = 128 + 64,
  BUTTON4_Y4 = 64 + 40,

  //BUTTON_TYPE_DIR_6
  BUTTON6_X1 = 64,
  BUTTON6_Y1 = 56,
  BUTTON6_X2 = 128 + 64,
  BUTTON6_Y2 = 56,
  BUTTON6_X3 = 64,
  BUTTON6_Y3 = 64 + 40,
  BUTTON6_X4 = 128 + 64,
  BUTTON6_Y4 = 64 + 40,
  BUTTON6_X5 = 64,
  BUTTON6_Y5 = 64 + 40,
  BUTTON6_X6 = 128 + 64,
  BUTTON6_Y6 = 64 + 40,
};

typedef enum
{
  BUTTON_TYPE_WAZA = 0,
  BUTTON_TYPE_DIR_4 = 0,
  BUTTON_TYPE_DIR_6 = 1,
  BUTTON_TYPE_YES_NO = 2,
  BUTTON_TYPE_MAX = 3,
}BUTTON_TYPE;

typedef enum
{
  BUTTON_ANIME_TYPE_APPEAR = 0,
  BUTTON_ANIME_TYPE_VANISH,
  BUTTON_ANIME_TYPE_MAX,
}BUTTON_ANIME_TYPE;

//技選択ボタン表示位置定義
enum
{
  BUTTON_UP_Y1 = 8 + 2,
  BUTTON_UP_Y2 = 24 + 2,

  BUTTON_DOWN_Y1 = 56 + 2,
  BUTTON_DOWN_Y2 = 72 + 2,

  WAZANAME_X1 = 64,
  WAZANAME_Y1 = BUTTON_UP_Y1,
  WAZANAME_X2 = 128 + WAZANAME_X1,
  WAZANAME_Y2 = BUTTON_UP_Y1,
  WAZANAME_X3 = WAZANAME_X1,
  WAZANAME_Y3 = BUTTON_DOWN_Y1,
  WAZANAME_X4 = WAZANAME_X2,
  WAZANAME_Y4 = BUTTON_DOWN_Y1,

  PPMSG_X1 = 49,
  PPMSG_Y1 = BUTTON_UP_Y2,
  PPMSG_X2 = 128 + PPMSG_X1,
  PPMSG_Y2 = BUTTON_UP_Y2,
  PPMSG_X3 = PPMSG_X1,
  PPMSG_Y3 = BUTTON_DOWN_Y2,
  PPMSG_X4 = PPMSG_X2,
  PPMSG_Y4 = BUTTON_DOWN_Y2,

  PP_X1 = 90,
  PP_Y1 = BUTTON_UP_Y2,
  PP_X2 = 128 + PP_X1,
  PP_Y2 = BUTTON_UP_Y2,
  PP_X3 = PP_X1,
  PP_Y3 = BUTTON_DOWN_Y2,
  PP_X4 = PP_X2,
  PP_Y4 = BUTTON_DOWN_Y2,
};

//攻撃対象選択ボタン表示位置定義
enum
{
  //4ボタン
  BUTTON4_UP_Y = 8 + 2,

  BUTTON4_DOWN_Y = 64 + 2,

  MONSNAME4_X1 = 64,
  MONSNAME4_Y1 = BUTTON4_UP_Y,
  MONSNAME4_X2 = 128 + MONSNAME4_X1,
  MONSNAME4_Y2 = BUTTON4_UP_Y,
  MONSNAME4_X3 = MONSNAME4_X1,
  MONSNAME4_Y3 = BUTTON4_DOWN_Y,
  MONSNAME4_X4 = MONSNAME4_X2,
  MONSNAME4_Y4 = BUTTON4_DOWN_Y,

  MONSNAME6_X1 = 48,
  MONSNAME6_Y1 = BUTTON4_UP_Y,
  MONSNAME6_X2 = 128,
  MONSNAME6_Y2 = BUTTON4_UP_Y,
  MONSNAME6_X3 = 208,
  MONSNAME6_Y3 = BUTTON4_UP_Y,
  MONSNAME6_X4 = MONSNAME6_X1,
  MONSNAME6_Y4 = BUTTON4_DOWN_Y,
  MONSNAME6_X5 = MONSNAME6_X2,
  MONSNAME6_Y5 = BUTTON4_DOWN_Y,
  MONSNAME6_X6 = MONSNAME6_X3,
  MONSNAME6_Y6 = BUTTON4_DOWN_Y,
};

//技タイプアイコンの表示座標
enum
{
  WAZATYPE_X1 = 32,
  WAZATYPE_Y1 = 64,
  WAZATYPE_X2 = 128 + WAZATYPE_X1,
  WAZATYPE_Y2 = WAZATYPE_Y1,
  WAZATYPE_X3 = WAZATYPE_X1,
  WAZATYPE_Y3 = 112,
  WAZATYPE_X4 = WAZATYPE_X2,
  WAZATYPE_Y4 = WAZATYPE_Y3,

  WAZATYPE_PLTT = 0x90,

};

ALIGN4  static  const GFL_CLACTPOS WazaIconPos[] =
{
  { WAZATYPE_X1, WAZATYPE_Y1 },
  { WAZATYPE_X2, WAZATYPE_Y2 },
  { WAZATYPE_X3, WAZATYPE_Y3 },
  { WAZATYPE_X4, WAZATYPE_Y4 },
};

//ボールゲージ座標定義
enum
{
  BTLV_INPUT_BALLGAUGE_ENEMY_X        = ( 128 + ( 8 * 3 ) - 4 ),
  BTLV_INPUT_BALLGAUGE_ENEMY_Y_CLOSE  = ( 8 * 6 ),                  //コマンド選択時
  BTLV_INPUT_BALLGAUGE_ENEMY_Y_OPEN   = ( 8 * 3 ),                  //コマンド選択時以外
  BTLV_INPUT_BALLGAUGE_MINE_X         = ( 128 - ( 16 * 3 ) + 8 ),
//  BTLV_INPUT_BALLGAUGE_MINE_Y_CLOSE   = ( 8 * 16 ),                 //コマンド選択時
//  BTLV_INPUT_BALLGAUGE_MINE_Y_OPEN    = ( 8 * 17 ),                 //コマンド選択時以外
  BTLV_INPUT_BALLGAUGE_MINE_Y_CLOSE   = ( 8 * 17 ),                 //コマンド選択時
  BTLV_INPUT_BALLGAUGE_MINE_Y_OPEN    = ( 8 * 17 ),                 //コマンド選択時以外

  BALL_GAUGE_MOVE_VALUE               = 4,  //ボールゲージ移動量
};

typedef enum
{
  BALL_GAUGE_TYPE_MINE = 0,
  BALL_GAUGE_TYPE_ENEMY,
  BALL_GAUGE_TYPE_MAX,
}BALL_GAUGE_TYPE;

typedef enum
{
  BALL_GAUGE_MOVE_OPEN = 0,
  BALL_GAUGE_MOVE_CLOSE,
}BALL_GAUGE_MOVE_DIR;

//カーソルOBJ定義
enum
{
  BTLV_INPUT_CURSOR_LU = 0,
  BTLV_INPUT_CURSOR_RU,
  BTLV_INPUT_CURSOR_LD,
  BTLV_INPUT_CURSOR_RD,
  BTLV_INPUT_CURSOR_LD_2,
  BTLV_INPUT_CURSOR_RD_2,
  BTLV_INPUT_CURSOR_MAX,

  BTLV_INPUT_CURSOR_LU_ANIME = 0,
  BTLV_INPUT_CURSOR_LD_ANIME,
  BTLV_INPUT_CURSOR_RU_ANIME,
  BTLV_INPUT_CURSOR_RD_ANIME,
};

//ポケモンアイコン定義
enum
{
  BTLV_INPUT_POKE_ICON_SINGLE_X = 128,
  BTLV_INPUT_POKE_ICON_SINGLE_Y = 112,  //128 + 12,
  BTLV_INPUT_POKE_ICON_DOUBLE_X1 = 128 - 24,
  BTLV_INPUT_POKE_ICON_DOUBLE_Y1 = BTLV_INPUT_POKE_ICON_SINGLE_Y,
  BTLV_INPUT_POKE_ICON_DOUBLE_X2 = 128 + 24,
  BTLV_INPUT_POKE_ICON_DOUBLE_Y2 = BTLV_INPUT_POKE_ICON_SINGLE_Y,
  BTLV_INPUT_POKE_ICON_TRIPLE_X1 = 128 - 40,
  BTLV_INPUT_POKE_ICON_TRIPLE_Y1 = BTLV_INPUT_POKE_ICON_SINGLE_Y,
  BTLV_INPUT_POKE_ICON_TRIPLE_X2 = BTLV_INPUT_POKE_ICON_SINGLE_X,
  BTLV_INPUT_POKE_ICON_TRIPLE_Y2 = BTLV_INPUT_POKE_ICON_SINGLE_Y,
  BTLV_INPUT_POKE_ICON_TRIPLE_X3 = 128 + 40,
  BTLV_INPUT_POKE_ICON_TRIPLE_Y3 = BTLV_INPUT_POKE_ICON_SINGLE_Y,
};

//天候アイコン定義
enum
{
  BTLV_INPUT_WEATHER_ICON_MAX = 2,

  BTLV_INPUT_WEATHER_BASE = 14,   ///< 天候アイコン土台
  BTLV_INPUT_WEATHER_SHINE,       ///< はれ
  BTLV_INPUT_WEATHER_RAIN,        ///< あめ
  BTLV_INPUT_WEATHER_SNOW,        ///< あられ
  BTLV_INPUT_WEATHER_SAND,        ///< すなあらし
  BTLV_INPUT_WEATHER_MIST,        ///< きり

  BTLV_INPUT_WEATHER_X1 = 256 + 48,
  BTLV_INPUT_WEATHER_X2 = 256 - 32,

  BTLV_INPUT_WEATHER_Y = 32,

  BTLV_INPUT_WEATHER_MOVE_SPEED = 8,
};

enum
{
  BTLV_INPUT_FADE_OUT = 1,
  BTLV_INPUT_FADE_IN,
};

enum
{
  BTLV_INPUT_YESNO_MSG_X  = 128,
  BTLV_INPUT_YES_MSG_Y    = 16,
  BTLV_INPUT_NO_MSG_Y     = 64,
};

enum
{
  ROTATE_MONSNAME_X0 = 64,
  ROTATE_MONSNAME_X1 = 192,
  ROTATE_MONSNAME_X2 = 128,
  ROTATE_MONSNAME_Y0 = 12,
  ROTATE_MONSNAME_Y1 = 12,
  ROTATE_MONSNAME_Y2 = 60,

  ROTATE_POKEICON_X0 = 64,
  ROTATE_POKEICON_X1 = 192,
  ROTATE_POKEICON_X2 = 128,
  ROTATE_POKEICON_Y0 = 76,
  ROTATE_POKEICON_Y1 = 76,
  ROTATE_POKEICON_Y2 = 120,

  ROTATE_FRAME = 8,

  ROTATE_SCROLL_COUNT = 8,
  ROTATE_SCROLL_SPEED = 64 / ROTATE_SCROLL_COUNT,
};

static  const GFL_CLACTPOS pokeicon_pos[ BTLV_INPUT_POKEICON_MAX ] =
{
  { ROTATE_POKEICON_X0, ROTATE_POKEICON_Y0 },
  { ROTATE_POKEICON_X1, ROTATE_POKEICON_Y1 },
  { ROTATE_POKEICON_X2, ROTATE_POKEICON_Y2 },
};

static  const int rotate_pos_table[][ BTLV_INPUT_POKEICON_MAX ] = {
  { 0, 1, 2 },
  { 2, 0, 1 },
  { 1, 2, 0 },
};

#define CURSOR_NOMOVE ( 0x0f )    //カーソルが一度も動いていないときのold_cursor_posの数値

static  const GFL_CLWK_DATA obj_param =
{
  0, 0,   //x, y
  0, 0, 0,  //アニメ番号、優先順位、BGプライオリティ
};


//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

typedef struct
{
  GFL_CLWK*   clwk;
  s16         pos_x;
  s16         pos_y;
}BTLV_INPUT_CLWK;

struct _BTLV_INPUT_WORK
{
  const BTL_CLIENT*     clientWork;
  GFL_TCBSYS*           tcbsys;
  void*                 tcbwork;
  ARCHANDLE*            handle;
  BTLV_INPUT_TYPE       type;
  BTLV_INPUT_SCRTYPE    scr_type;
  GFL_MSGDATA*          msg;
  u32                   tcb_execute_flag    :1;
  u32                   tcb_execute_count   :3;
  u32                   center_button_type  :1;
  u32                   cursor_pos          :4;
  u32                   old_cursor_pos      :4;
  u32                   cursor_decide       :1;
  u32                   trainer_flag        :1;
  u32                   fade_flag           :2;
  u32                   rotate_flag         :2;
  u32                   rotate_scr          :2;
  u32                   before_select_dir   :2;
  u32                                       :10;

  //OBJリソース
  u32                   objcharID;
  u32                   objplttID;
  u32                   objcellID;

  //カーソル
  u32                   cur_charID;
  u32                   cur_cellID;

  //ボールゲージOBJ
  GFL_CLUNIT*           ballgauge_clunit;
  BTLV_INPUT_CLWK       ballgauge_mine[ TEMOTI_POKEMAX ];
  BTLV_INPUT_CLWK       ballgauge_enemy[ TEMOTI_POKEMAX ];

  //カーソルOBJ
  GFL_CLUNIT*           cursor_clunit;
  BTLV_INPUT_CLWK       cursor[ BTLV_INPUT_CURSOR_MAX ];

  //ポケモンアイコン
  u32                   pokeicon_charID[ BTLV_INPUT_POKEICON_MAX ];
  u32                   pokeicon_plttID;
  u32                   pokeicon_cellID;
  GFL_CLUNIT*           pokeicon_clunit;
  BTLV_INPUT_CLWK       pokeicon_wk[ BTLV_INPUT_POKEICON_MAX ];

  //技タイプアイコンOBJ
  u32                   wazatype_charID[ PTL_WAZA_MAX ];
  u32                   wazatype_plttID;
  u32                   wazatype_cellID;
  GFL_CLUNIT*           wazatype_clunit;
  GFL_CLWK*             wazatype_wk[ PTL_WAZA_MAX ];

  //天候アイコン
  GFL_CLUNIT*           weather_clunit;
  GFL_CLWK*             weather_wk[ BTLV_INPUT_WEATHER_ICON_MAX ];

  //フォント
  GFL_FONT*             font;

  //BMP
  GFL_BMPWIN*           bmp_win;
  GFL_BMP_DATA*         bmp_data;

  //メインループTCB
  GFL_TCB*              main_loop;      //scdにメインループが存在しないのでBTLV_EFFECTのTCBを間借りしてメインを回す

  HEAPID                heapID;

  u8                    button_exist[ BTLV_INPUT_BUTTON_MAX ];  //押せるボタンかどうかチェック

  u8                    *cursor_mode;

  //ローテーション用POKEMON_PARAM
  const POKEMON_PARAM*  rotate_pp[ BTLV_INPUT_POKEICON_MAX ];
};

typedef struct
{
  BTLV_INPUT_WORK*  biw;
  int               seq_no;
  BtlvMcssPos       pos;
  ARCDATID          datID;
}TCB_TRANSFORM_WORK;

typedef struct
{
  BTLV_INPUT_WORK*  biw;
  fx32              start_scale;
  fx32              end_scale;
  fx32              scale_speed;
  int               pos_y;
}TCB_SCALE_UP;

typedef struct
{
  BTLV_INPUT_WORK*  biw;
  int               scroll_y;
  int               scroll_speed;
  int               scroll_count;
}TCB_SCROLL_UP;

typedef struct
{
  BTLV_INPUT_WORK*  biw;
  int               count;
  SCREEN_ANIME_DIR  dir;
  int               wait;
}TCB_SCREEN_ANIME;

typedef struct
{
  BTLV_INPUT_WORK*  biw;
  GFL_CLUNIT*       clunit;
  GFL_CLWK*         clwk[ BUTTON_ANIME_MAX ];
}TCB_BUTTON_ANIME;

typedef struct
{
  BTLV_INPUT_WORK*  biw;
  int               move_dir;    //移動方向
}TCB_BALL_GAUGE_MOVE;

typedef struct
{
  BTLV_INPUT_WORK*  biw;
}TCB_FADE_ACT;

typedef struct
{
  BTLV_INPUT_WORK*  biw;
}TCB_WEATHER_ICON_MOVE;

typedef struct
{
  BTLV_INPUT_WORK*  biw;
  int               seq_no;
  VecFx32           now_pos[ BTLV_INPUT_POKEICON_MAX ];
  VecFx32           end_pos[ BTLV_INPUT_POKEICON_MAX ];
  VecFx32           vector[ BTLV_INPUT_POKEICON_MAX ];
  int               frame;
}TCB_ROTATE_ACTION;

typedef struct
{
  const GFL_CLACTPOS  pos[ BUTTON_ANIME_MAX ];          //座標
  int                 anm_no[ BUTTON_ANIME_TYPE_MAX ];  //アニメーションナンバー
}BUTTON_ANIME_PARAM;

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  void  BTLV_INPUT_LoadResource( BTLV_INPUT_WORK* biw );
static  void  TCB_TransformStandby2Command( GFL_TCB* tcb, void* work );
static  void  TCB_TransformCommand2Waza( GFL_TCB* tcb, void* work );
static  void  TCB_TransformWaza2Command( GFL_TCB* tcb, void* work );
static  void  TCB_TransformRotate2Command( GFL_TCB* tcb, void* work );
static  void  TCB_TransformWaza2Dir( GFL_TCB* tcb, void* work );
static  void  TCB_TransformDir2Waza( GFL_TCB* tcb, void* work );
static  void  TCB_TransformCommand2Standby( GFL_TCB* tcb, void* work );
static  void  TCB_TransformWaza2Standby( GFL_TCB* tcb, void* work );
static  void  TCB_TransformStandby2YesNo( GFL_TCB* tcb, void* work );
static  void  TCB_TransformStandby2Rotate( GFL_TCB* tcb, void* work );

static  void  SetupScaleChange( BTLV_INPUT_WORK* biw, fx32 start_scale, fx32 end_scale, fx32 scale_speed, int pos_y );
static  void  TCB_ScaleChange( GFL_TCB* tcb, void* work );
static  void  SetupScrollUp( BTLV_INPUT_WORK* biw, int scroll_x, int scroll_y, int scroll_speed, int scroll_count );
static  void  TCB_ScrollUp( GFL_TCB* tcb, void* work );
static  void  SetupScreenAnime( BTLV_INPUT_WORK* biw, int index, SCREEN_ANIME_DIR dir );
static  void  TCB_ScreenAnime( GFL_TCB* tcb, void* work );
static  void  SetupButtonAnime( BTLV_INPUT_WORK* biw, BUTTON_TYPE type, BUTTON_ANIME_TYPE anm_type );
static  void  TCB_ButtonAnime( GFL_TCB* tcb, void* work );
static  void  SetupBallGaugeMove( BTLV_INPUT_WORK* biw, BALL_GAUGE_MOVE_DIR dir );
static  void  TCB_BallGaugeMove( GFL_TCB* tcb, void* work );
static  void  TCB_Fade( GFL_TCB* tcb, void* work );
static  void  TCB_WeatherIconMove( GFL_TCB* tcb, void* work );
static  void  SetupRotateAction( BTLV_INPUT_WORK* biw, int hit );
static  void  TCB_RotateAction( GFL_TCB* tcb, void* work );

static  void  BTLV_INPUT_MainTCB( GFL_TCB* tcb, void* work );
static  void  FontLenGet( const STRBUF *str, GFL_FONT *font, int *ret_dot_len, int *ret_char_len );
static  void  BTLV_INPUT_CreateWazaScreen( BTLV_INPUT_WORK* biw, const BTLV_INPUT_WAZA_PARAM *biwp );
static  void  BTLV_INPUT_CreateDirScreen( BTLV_INPUT_WORK* biw, TCB_TRANSFORM_WORK* ttw, const BTLV_INPUT_SCENE_PARAM *bisp );
static  void  BTLV_INPUT_CreateYesNoScreen( BTLV_INPUT_WORK* biw, const BTLV_INPUT_YESNO_PARAM *biyp );
static  void  BTLV_INPUT_CreateRotateScreen( BTLV_INPUT_WORK* biw );
static  void  BTLV_INPUT_CreateRotatePokeIcon( BTLV_INPUT_WORK* biw );
static  void  BTLV_INPUT_ClearScreen( BTLV_INPUT_WORK* biw );
static  PRINTSYS_LSB  PP_FontColorGet( int pp, int pp_max );
static  void  BTLV_INPUT_CreatePokeIcon( BTLV_INPUT_WORK* biw, BTLV_INPUT_COMMAND_PARAM* bicp );
static  void  BTLV_INPUT_DeletePokeIcon( BTLV_INPUT_WORK* biw );
static  void  BTLV_INPUT_CreateWeatherIcon( BTLV_INPUT_WORK* biw );
static  void  BTLV_INPUT_DeleteWeatherIcon( BTLV_INPUT_WORK* biw );
static  void  BTLV_INPUT_CreateBallGauge( BTLV_INPUT_WORK* biw, const BTLV_INPUT_COMMAND_PARAM *bicp, BALL_GAUGE_TYPE type );
static  void  BTLV_INPUT_DeleteBallGauge( BTLV_INPUT_WORK* biw );
static  void  BTLV_INPUT_CreateCursorOBJ( BTLV_INPUT_WORK* biw );
static  void  BTLV_INPUT_DeleteCursorOBJ( BTLV_INPUT_WORK* biw );
static  int   BTLV_INPUT_CheckKey( BTLV_INPUT_WORK* biw, const GFL_UI_TP_HITTBL* tp_tbl, const BTLV_INPUT_KEYTBL* key_tbl, int hit );
static  void  BTLV_INPUT_PutCursorOBJ( BTLV_INPUT_WORK* biw, const GFL_UI_TP_HITTBL* tp_tbl, const BTLV_INPUT_KEYTBL* key_tbl );
static  inline  void  SePlaySelect( void );
static  inline  void  SePlayRotateSelect( void );
static  inline  void  SePlayRotateDecide( void );
static  inline  void  SePlayRotation( void );

//============================================================================================
/**
 *  @brief  システム初期化
 *
 *  @param[in]  type          インターフェースタイプ
 *  @param[in]  font          使用するフォント
 *  @param[in]  cursor_flag   カーソル表示するかどうかフラグのポインタ（他のアプリとも共用するため）
 *  @param[in]  heapID        ヒープID
 *
 *  @retval システム管理構造体のポインタ
 */
//============================================================================================
BTLV_INPUT_WORK*  BTLV_INPUT_Init( BTLV_INPUT_TYPE type, const BTL_CLIENT* clientWork, GFL_FONT* font, u8* cursor_flag, HEAPID heapID )
{
  BTLV_INPUT_WORK *biw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_INPUT_WORK ) );

  biw->heapID = heapID;
  biw->clientWork = clientWork;

  biw->tcbwork  = GFL_HEAP_AllocClearMemory( biw->heapID, GFL_TCB_CalcSystemWorkSize( BTLV_INPUT_TCB_MAX ) );
  biw->tcbsys   = GFL_TCB_Init( BTLV_INPUT_TCB_MAX, biw->tcbwork );

  biw->font = font;
  biw->type = type;

  biw->cursor_mode = cursor_flag;

  biw->old_cursor_pos = CURSOR_NOMOVE;

  {
    int i;

    biw->objcharID        = GFL_CLGRP_REGISTER_FAILED;
    biw->objplttID        = GFL_CLGRP_REGISTER_FAILED;
    biw->objcellID        = GFL_CLGRP_REGISTER_FAILED;
    biw->cur_charID       = GFL_CLGRP_REGISTER_FAILED;
    biw->cur_cellID       = GFL_CLGRP_REGISTER_FAILED;
    biw->pokeicon_plttID  = GFL_CLGRP_REGISTER_FAILED;
    biw->pokeicon_cellID  = GFL_CLGRP_REGISTER_FAILED;
    biw->wazatype_plttID  = GFL_CLGRP_REGISTER_FAILED;
    biw->wazatype_cellID  = GFL_CLGRP_REGISTER_FAILED;
    for( i = 0 ; i < BTLV_INPUT_POKEICON_MAX ; i++ )
    {
      biw->pokeicon_charID[ i ] = GFL_CLGRP_REGISTER_FAILED;
    }
    for( i = 0 ; i < PTL_WAZA_MAX ; i++ )
    {
      biw->wazatype_charID[ i ] = GFL_CLGRP_REGISTER_FAILED;
    }
  }

  biw->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_btlv_input_dat, biw->heapID );

  BTLV_INPUT_InitBG( biw );

  return biw;
}

//============================================================================================
/**
 *  @brief  システム終了
 *
 *  @param[in]  biw システム管理構造体のポインタ
 */
//============================================================================================
void  BTLV_INPUT_Exit( BTLV_INPUT_WORK* biw )
{
  BTLV_INPUT_ExitBG( biw );

  GFL_MSG_Delete( biw->msg );

  GFL_TCB_Exit( biw->tcbsys );
  GFL_HEAP_FreeMemory( biw->tcbwork );
  GFL_HEAP_FreeMemory( biw );
}

//============================================================================================
/**
 *  @brief  システムメインループ
 *
 *  @param[in]  biw システム管理構造体のポインタ
 */
//============================================================================================
void  BTLV_INPUT_Main( BTLV_INPUT_WORK* biw )
{
//  GFL_TCB_Main( biw->tcbsys );
//  INFOWIN_Update();
}

//============================================================================================
/**
 *  @brief  システムメインループ（TCB版）
 *
 *  @param[in]  biw システム管理構造体のポインタ
 */
//============================================================================================
static  void  BTLV_INPUT_MainTCB( GFL_TCB* tcb, void* work )
{
  BTLV_INPUT_WORK* biw = (BTLV_INPUT_WORK *)work;
  GFL_TCB_Main( biw->tcbsys );
  INFOWIN_Update();
}

//============================================================================================
/**
 *  @brief  BG画面初期化
 *
 *  @param[in]  biw システム管理構造体のポインタ
 */
//============================================================================================
void  BTLV_INPUT_InitBG( BTLV_INPUT_WORK *biw )
{
  //ＢＧモード設定
  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGModeDisp( &sysHeader, GFL_BG_SUB_DISP );
  }
  biw->handle   = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, biw->heapID );
  biw->wazatype_clunit = GFL_CLACT_UNIT_Create( PTL_WAZA_MAX, 0, biw->heapID );
  biw->ballgauge_clunit = GFL_CLACT_UNIT_Create( TEMOTI_POKEMAX * 2, 0, biw->heapID );
  biw->cursor_clunit = GFL_CLACT_UNIT_Create( BTLV_INPUT_CURSOR_MAX, 0, biw->heapID );
  biw->pokeicon_clunit = GFL_CLACT_UNIT_Create( BTLV_INPUT_POKEICON_MAX, 0, biw->heapID );
  biw->weather_clunit = GFL_CLACT_UNIT_Create( BTLV_INPUT_WEATHER_ICON_MAX, 0, biw->heapID );

  BTLV_INPUT_SetFrame();
  BTLV_INPUT_LoadResource( biw );

  BTLV_INPUT_CreateCursorOBJ( biw );

  //ビットマップ初期化
  biw->bmp_win = GFL_BMPWIN_Create( GFL_BG_FRAME2_S, 0, 4, 32, 12, BMPWIN_PAL_NO, GFL_BMP_CHRAREA_GET_B );
  biw->bmp_data = GFL_BMPWIN_GetBmp( biw->bmp_win );
  GFL_BMP_Clear( biw->bmp_data, 0x00 );
  GFL_BMPWIN_TransVramCharacter( biw->bmp_win );
  GFL_BMPWIN_MakeScreen( biw->bmp_win );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );

  //情報ステータスバー初期化
  INFOWIN_Init( GFL_BG_FRAME2_S, INFOWIN_PAL_NO, NULL, biw->heapID );
  //情報ステータスバー表示
  GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_ON );

  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

  //メインループはTCBで行う
  biw->main_loop = GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), BTLV_INPUT_MainTCB, biw, 0 );

  //OBJカーソル表示処理をする
  biw->cursor_decide = 1;

  BTLV_INPUT_CreateScreen( biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
}

//============================================================================================
/**
 *  @brief  BG画面終了
 *
 *  @param[in]  biw システム管理構造体のポインタ
 */
//============================================================================================
void  BTLV_INPUT_ExitBG( BTLV_INPUT_WORK *biw )
{
  BTLV_INPUT_DeleteBallGauge( biw );
  BTLV_INPUT_DeleteCursorOBJ( biw );
  BTLV_INPUT_DeletePokeIcon( biw );
  BTLV_INPUT_DeleteWeatherIcon( biw );

  if( biw->objcharID != GFL_CLGRP_REGISTER_FAILED )
  {
    GFL_CLGRP_CGR_Release( biw->objcharID );
    biw->objcharID = GFL_CLGRP_REGISTER_FAILED;
  }
  if( biw->objplttID != GFL_CLGRP_REGISTER_FAILED )
  {
    GFL_CLGRP_PLTT_Release( biw->objplttID );
    biw->objplttID = GFL_CLGRP_REGISTER_FAILED;
  }
  if( biw->objcellID != GFL_CLGRP_REGISTER_FAILED )
  {
    GFL_CLGRP_CELLANIM_Release( biw->objcellID );
    biw->objcellID = GFL_CLGRP_REGISTER_FAILED;
  }
  if( biw->cur_charID != GFL_CLGRP_REGISTER_FAILED )
  {
    GFL_CLGRP_CGR_Release( biw->cur_charID );
    biw->cur_charID = GFL_CLGRP_REGISTER_FAILED;
  }
  if( biw->cur_cellID != GFL_CLGRP_REGISTER_FAILED )
  {
    GFL_CLGRP_CELLANIM_Release( biw->cur_cellID );
    biw->cur_cellID = GFL_CLGRP_REGISTER_FAILED;
  }

  {
    int i;

    for( i = 0 ; i < PTL_WAZA_MAX ; i++ )
    {
      if( biw->wazatype_charID[ i ] != GFL_CLGRP_REGISTER_FAILED )
      {
        GFL_CLGRP_CGR_Release( biw->wazatype_charID[ i ] );
        biw->wazatype_charID[ i ] = GFL_CLGRP_REGISTER_FAILED;
      }
    }
  }

  if( biw->wazatype_cellID != GFL_CLGRP_REGISTER_FAILED )
  {
    GFL_CLGRP_CELLANIM_Release( biw->wazatype_cellID );
    biw->wazatype_cellID = GFL_CLGRP_REGISTER_FAILED;
  }
  if( biw->wazatype_plttID  != GFL_CLGRP_REGISTER_FAILED )
  {
    GFL_CLGRP_PLTT_Release( biw->wazatype_plttID );
    biw->wazatype_plttID = GFL_CLGRP_REGISTER_FAILED;
  }

  GFL_CLACT_UNIT_Delete( biw->wazatype_clunit );
  GFL_CLACT_UNIT_Delete( biw->ballgauge_clunit );
  GFL_CLACT_UNIT_Delete( biw->cursor_clunit );
  GFL_CLACT_UNIT_Delete( biw->pokeicon_clunit );
  GFL_CLACT_UNIT_Delete( biw->weather_clunit );

  GFL_BMPWIN_Delete( biw->bmp_win );

  INFOWIN_Exit();

  BTLV_INPUT_FreeFrame();

  GFL_TCB_DeleteTask( biw->main_loop );

  GFL_ARC_CloseDataHandle( biw->handle );

  biw->scr_type = BTLV_INPUT_SCRTYPE_STANDBY;
}

//============================================================================================
/**
 *  @brief  下画面BGフレーム設定
 */
//============================================================================================
void BTLV_INPUT_SetFrame( void )
{
  int i;

  for( i = 0 ; i < NELEMS( bibf ) ; i++ )
  {
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S + i, &bibf[ i ].bgcnt_table, bibf[ i ].bgcnt_mode );
    GFL_BG_ClearScreenCode( GFL_BG_FRAME0_S + i, BG_CLEAR_CODE );
    GFL_BG_SetScroll( GFL_BG_FRAME0_S + i, GFL_BG_SCROLL_X_SET, 0 );
    GFL_BG_SetScroll( GFL_BG_FRAME0_S + i, GFL_BG_SCROLL_Y_SET, 0 );
  }
}

//============================================================================================
/**
 *  @brief  下画面BGフレーム設定解放
 *
 *  @param[in]  biw   システム管理構造体のポインタ
 */
//============================================================================================
void BTLV_INPUT_FreeFrame( void )
{
  int i;

  for( i = 0 ; i < NELEMS( bibf ) ; i++ ){
    GFL_BG_SetVisible( GFL_BG_FRAME0_S + i, VISIBLE_OFF );
    GFL_BG_FreeBGControl( GFL_BG_FRAME0_S + i );
  }
}

//============================================================================================
/**
 *  @brief  下画面フェードアウトリクエスト
 *
 *  @param[in]  biw   システム管理構造体のポインタ
 */
//============================================================================================
void BTLV_INPUT_SetFadeOut( BTLV_INPUT_WORK* biw )
{
  TCB_FADE_ACT* tfa = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_FADE_ACT ) );

  PaletteFadeReq( BTLV_EFFECT_GetPfd(), PF_BIT_SUB_ALL, 0xffff, 1, 0, 16, 0, biw->tcbsys );
  biw->fade_flag = BTLV_INPUT_FADE_OUT;

  tfa->biw           = biw;
  GFL_TCB_AddTask( biw->tcbsys, TCB_Fade, tfa, 0 );
}

//============================================================================================
/**
 *  @brief  下画面フェードインリクエスト
 */
//============================================================================================
void BTLV_INPUT_SetFadeIn( BTLV_INPUT_WORK* biw )
{
  TCB_FADE_ACT* tfa = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_FADE_ACT ) );

  BTLV_INPUT_InitBG( biw );
  //PaletteFadeReq( BTLV_EFFECT_GetPfd(), PF_BIT_SUB_ALL, 0xffff, 1, 16, 0, 0, biw->tcbsys );
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0 );
  biw->fade_flag = BTLV_INPUT_FADE_IN;

  tfa->biw           = biw;
  GFL_TCB_AddTask( biw->tcbsys, TCB_Fade, tfa, 0 );
}

//============================================================================================
/**
 *  @brief  下画面フェードチェック
 *
 *  @param[in]  biw   システム管理構造体のポインタ
 *
 *  @retval TRUE:フェード中 FALSE:フェード終了
 */
//============================================================================================
BOOL BTLV_INPUT_CheckFadeExecute( BTLV_INPUT_WORK* biw )
{
  return ( biw->fade_flag != 0 );
}

//============================================================================================
/**
 *  @brief  下画面生成
 *
 *  @param[in]  biw   システム管理構造体のポインタ
 *  @param[in]  type  生成するスクリーンタイプ
 *  @param[in]  param 生成に必要なパラメータ構造体のポインタ
 */
//============================================================================================
void BTLV_INPUT_CreateScreen( BTLV_INPUT_WORK* biw, BTLV_INPUT_SCRTYPE type, void* param )
{
  BTLV_INPUT_ClearScreen( biw );

  switch( type ){
  case BTLV_INPUT_SCRTYPE_STANDBY:
    if( biw->scr_type == BTLV_INPUT_SCRTYPE_STANDBY )
    {
      MtxFx22 mtx;

      MTX_Scale22( &mtx, FX32_ONE * 3, FX32_ONE * 3 );
      GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, 128, &mtx, 256, 256 );
      GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, 128 + 24, &mtx, 256, 256 );

      GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
      PaletteFadeReq( BTLV_EFFECT_GetPfd(), PF_BIT_SUB_BG, STANDBY_PAL, 1, STANDBY_FADE, STANDBY_FADE, STANDBY_FADE_COLOR, biw->tcbsys );
    }
    else
    {
      TCB_TRANSFORM_WORK* ttw = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_TRANSFORM_WORK ) );
      biw->tcb_execute_flag = 1;
      ttw->biw = biw;

      BTLV_INPUT_DeleteBallGauge( biw );
      BTLV_INPUT_DeletePokeIcon( biw );
      BTLV_INPUT_DeleteWeatherIcon( biw );

      if( biw->scr_type == BTLV_INPUT_SCRTYPE_COMMAND )
      {
        GFL_TCB_AddTask( biw->tcbsys, TCB_TransformCommand2Standby, ttw, 1 );
      }
      else
      {
        GFL_TCB_AddTask( biw->tcbsys, TCB_TransformWaza2Standby, ttw, 1 );
      }
      PaletteFadeReq( BTLV_EFFECT_GetPfd(), PF_BIT_SUB_BG, STANDBY_PAL, 1, 0, STANDBY_FADE, STANDBY_FADE_COLOR, biw->tcbsys );
    }
    break;
  case BTLV_INPUT_SCRTYPE_COMMAND:
    {
      BTLV_INPUT_COMMAND_PARAM* bicp = ( BTLV_INPUT_COMMAND_PARAM * )param;
      int i;
      TCB_TRANSFORM_WORK* ttw = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_TRANSFORM_WORK ) );
      biw->tcb_execute_flag = 1;
      biw->center_button_type = bicp->center_button_type;
      ttw->biw = biw;
      ttw->pos = bicp->pos;

      for( i = 0 ; i < 4 ; i++ )
      {
        biw->button_exist[ i ] = TRUE;  //押せるボタンかどうかチェック
      }

      BTLV_INPUT_DeletePokeIcon( biw );
      BTLV_INPUT_CreatePokeIcon( biw, bicp );
      BTLV_INPUT_CreateWeatherIcon( biw );

      if( biw->scr_type == BTLV_INPUT_SCRTYPE_WAZA )
      {
        GFL_TCB_AddTask( biw->tcbsys, TCB_TransformWaza2Command, ttw, 1 );
      }
      else
      {
        BTLV_INPUT_CreateBallGauge( biw, bicp, BALL_GAUGE_TYPE_MINE );
        if( bicp->trainer_flag )
        {
          BTLV_INPUT_CreateBallGauge( biw, bicp, BALL_GAUGE_TYPE_ENEMY );
        }
        biw->trainer_flag = bicp->trainer_flag;
        if( biw->scr_type == BTLV_INPUT_SCRTYPE_ROTATE )
        {
          GFL_TCB_AddTask( biw->tcbsys, TCB_TransformRotate2Command, ttw, 1 );
        }
        else
        {
          GFL_TCB_AddTask( biw->tcbsys, TCB_TransformStandby2Command, ttw, 1 );
        }
      }
    }
    break;
  case BTLV_INPUT_SCRTYPE_WAZA:
    {
      TCB_TRANSFORM_WORK* ttw = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_TRANSFORM_WORK ) );
      BTLV_INPUT_WAZA_PARAM* biwp = ( BTLV_INPUT_WAZA_PARAM * )param;

      BTLV_INPUT_CreateWazaScreen( biw, ( const BTLV_INPUT_WAZA_PARAM * )param );
      biw->tcb_execute_flag = 1;
      ttw->biw = biw;
      ttw->pos = biwp->pos;

      if( biw->scr_type == BTLV_INPUT_SCRTYPE_DIR )
      {
        GFL_TCB_AddTask( biw->tcbsys, TCB_TransformDir2Waza, ttw, 1 );
      }
      else
      {
        BTLV_INPUT_DeletePokeIcon( biw );
        BTLV_INPUT_DeleteWeatherIcon( biw );
        GFL_TCB_AddTask( biw->tcbsys, TCB_TransformCommand2Waza, ttw, 1 );
      }
    }
    break;
  case BTLV_INPUT_SCRTYPE_DIR:
    {
      TCB_TRANSFORM_WORK* ttw = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_TRANSFORM_WORK ) );
      BTLV_INPUT_SCENE_PARAM* bisp = ( BTLV_INPUT_SCENE_PARAM * )param;

      BTLV_INPUT_CreateDirScreen( biw, ttw, ( const BTLV_INPUT_SCENE_PARAM * )param );
      biw->tcb_execute_flag = 1;
      ttw->biw = biw;
      ttw->pos = bisp->pos;
      GFL_TCB_AddTask( biw->tcbsys, TCB_TransformWaza2Dir, ttw, 1 );
    }
    break;
  case BTLV_INPUT_SCRTYPE_YES_NO:
    {
      TCB_TRANSFORM_WORK* ttw = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_TRANSFORM_WORK ) );
      BTLV_INPUT_YESNO_PARAM* biyp = ( BTLV_INPUT_YESNO_PARAM * )param;

      BTLV_INPUT_CreateYesNoScreen( biw, ( const BTLV_INPUT_YESNO_PARAM * )param );
      biw->tcb_execute_flag = 1;
      ttw->biw = biw;
      GFL_TCB_AddTask( biw->tcbsys, TCB_TransformStandby2YesNo, ttw, 1 );
    }
    break;
  case BTLV_INPUT_SCRTYPE_ROTATE:
    {
      TCB_TRANSFORM_WORK* ttw = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_TRANSFORM_WORK ) );
      BTLV_INPUT_ROTATE_PARAM* birp = ( BTLV_INPUT_ROTATE_PARAM * )param;
      int i;

      biw->before_select_dir = birp->before_select_dir;
      biw->rotate_flag = 0;
      biw->rotate_scr = bird[ biw->before_select_dir ].init_scr;

      for( i = 0 ; i < BTLV_INPUT_POKEICON_MAX ; i++ )
      {
        biw->rotate_pp[ i ] = birp->pp[ i ];
        biw->button_exist[ i ] = button_exist_table[ biw->rotate_scr ][ i ];
      }

      BTLV_INPUT_CreateRotateScreen( biw );
      BTLV_INPUT_CreateRotatePokeIcon( biw );
      biw->tcb_execute_flag = 1;
      ttw->biw = biw;
      GFL_TCB_AddTask( biw->tcbsys, TCB_TransformStandby2Rotate, ttw, 1 );
    }
    break;
  default:
    //ありえないSCRTYPEが指定されている
    GF_ASSERT( 0 );
    break;
  }
  biw->scr_type = type;
}

//============================================================================================
/**
 *  @brief  入力チェック
 *
 *  @param[in]  tp_tbl  タッチパネルテーブル
 *  @param[in]  key_tbl キー操作テーブル
 */
//============================================================================================
int BTLV_INPUT_CheckInput( BTLV_INPUT_WORK* biw, const GFL_UI_TP_HITTBL* tp_tbl, const BTLV_INPUT_KEYTBL* key_tbl )
{
  int hit;

  //下画面変形中は入力を無視
  if( biw->tcb_execute_flag )
  {
    return  GFL_UI_TP_HIT_NONE;
  }

  //ローテーション選択は、別判定をする
  if( biw->scr_type == BTLV_INPUT_SCRTYPE_ROTATE )
  {
    hit = GFL_UI_TP_HitTrg( RotateTouchData );
    hit = BTLV_INPUT_CheckKey( biw, RotateTouchData, RotateKeyData[ biw->rotate_scr ], hit );
    if( hit != GFL_UI_TP_HIT_NONE )
    {
      if( biw->button_exist[ hit ] == FALSE )
      {
        hit = GFL_UI_TP_HIT_NONE;
      }
      else
      {
        //けっていを選択
        if( hit == 2 )
        {
          SePlayRotateDecide();
          hit = biw->rotate_flag + 1;
        }
        else
        {
          SePlayRotateSelect();
          SetupRotateAction( biw, hit );
          hit = GFL_UI_TP_HIT_NONE;
        }
      }
    }
    return hit;
  }

  GF_ASSERT( tp_tbl != NULL );
  GF_ASSERT( key_tbl != NULL );

  hit = GFL_UI_TP_HitTrg( tp_tbl );
  hit = BTLV_INPUT_CheckKey( biw, tp_tbl, key_tbl, hit );

  if( hit != GFL_UI_TP_HIT_NONE )
  {
    if( biw->button_exist[ hit ] == FALSE )
    {
      hit = GFL_UI_TP_HIT_NONE;
    }
  }
  return hit;
}

//============================================================================================
/**
 *  @brief  下画面リソースロード
 *
 *  @param[in]  biw  下画面管理構造体
 */
//============================================================================================
static  void  BTLV_INPUT_LoadResource( BTLV_INPUT_WORK* biw )
{
  GFL_ARCHDL_UTIL_TransVramBgCharacter( biw->handle, NARC_battgra_wb_battle_w_bg_NCGR,
                                        GFL_BG_FRAME0_S, 0, 0, FALSE, biw->heapID );

  GFL_ARCHDL_UTIL_TransVramBgCharacter( biw->handle, NARC_battgra_wb_battle_w_bg3_NCGR,
                                        GFL_BG_FRAME3_S, 0, 0x8000, FALSE, biw->heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( biw->handle, NARC_battgra_wb_battle_w_bg3_NSCR,
                                   GFL_BG_FRAME3_S, 0, 0, FALSE, biw->heapID );
  PaletteWorkSet_ArcHandle( BTLV_EFFECT_GetPfd(), biw->handle, NARC_battgra_wb_battle_w_bg_NCLR,
                            biw->heapID, FADE_SUB_BG, 0x1e0, 0 );

  biw->objcharID = GFL_CLGRP_CGR_Register( biw->handle, NARC_battgra_wb_battle_w_obj_NCGR, FALSE,
                                           CLSYS_DRAW_SUB, biw->heapID );
  biw->objcellID = GFL_CLGRP_CELLANIM_Register( biw->handle, NARC_battgra_wb_battle_w_obj_NCER,
                                                NARC_battgra_wb_battle_w_obj_NANR, biw->heapID );
  biw->objplttID = GFL_CLGRP_PLTT_Register( biw->handle, NARC_battgra_wb_battle_w_obj_NCLR, CLSYS_DRAW_SUB, 0, biw->heapID );
  PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_SUB_OBJ,
                           GFL_CLGRP_PLTT_GetAddr( biw->objplttID, CLSYS_DRAW_SUB ) / 2, 0x20 * 8 );
  biw->cur_charID = GFL_CLGRP_CGR_Register( biw->handle, NARC_battgra_wb_battle_w_cursor_NCGR, FALSE,
                                           CLSYS_DRAW_SUB, biw->heapID );
  biw->cur_cellID = GFL_CLGRP_CELLANIM_Register( biw->handle, NARC_battgra_wb_battle_w_cursor_NCER,
                                                NARC_battgra_wb_battle_w_cursor_NANR, biw->heapID );

  {
    ARCHANDLE*  hdl;
    int         i;

    //技タイプアイコン
    hdl = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), biw->heapID );
    biw->wazatype_cellID = GFL_CLGRP_CELLANIM_Register( hdl,
                                                        APP_COMMON_GetPokeTypeCellArcIdx( APP_COMMON_MAPPING_32K ),
                                                        APP_COMMON_GetPokeTypeAnimeArcIdx( APP_COMMON_MAPPING_32K ),
                                                        biw->heapID );
    biw->wazatype_plttID = GFL_CLGRP_PLTT_Register( hdl,
                                                    APP_COMMON_GetPokeTypePltArcIdx(), CLSYS_DRAW_SUB,
                                                    0x20 * 8, biw->heapID );
    PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_SUB_OBJ,
                             GFL_CLGRP_PLTT_GetAddr( biw->wazatype_plttID, CLSYS_DRAW_SUB ) / 2, 0x20 * 3 );
    for( i = 0; i < PTL_WAZA_MAX ; i++ ){
      biw->wazatype_charID[ i ] = GFL_CLGRP_CGR_Register( hdl, APP_COMMON_GetPokeTypeCharArcIdx( POKETYPE_NORMAL ), FALSE,
                                                          CLSYS_DRAW_SUB, biw->heapID );
    }
    GFL_ARC_CloseDataHandle( hdl );
  }
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（待機→コマンド選択）
 */
//============================================================================================
static  void  TCB_TransformStandby2Command( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){
  case 0:
    if( ttw->biw->center_button_type == BTLV_INPUT_CENTER_BUTTON_ESCAPE )
    {
      GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0a_NSCR,
                                       GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
    }
    else
    {
      if( ( ttw->biw->type == BTLV_INPUT_TYPE_TRIPLE ) && ( ttw->pos != BTLV_MCSS_POS_C ) )
      {
        GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0e_NSCR,
                                         GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
      }
      else
      {
        GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0d_NSCR,
                                         GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
      }
    }
    GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg1a_NSCR,
                                     GFL_BG_FRAME1_S, 0, 0, FALSE, ttw->biw->heapID );
    PMSND_PlaySE( SEQ_SE_OPEN2 );
    GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, TTS2C_FRAME1_SCROLL_X );
    GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, TTS2C_FRAME1_SCROLL_Y );
    SetupScaleChange( ttw->biw, TTS2C_START_SCALE, TTS2C_END_SCALE, -TTS2C_SCALE_SPEED, STANBY_POS_Y );
    SetupScrollUp( ttw->biw, TTS2C_START_SCROLL_X, TTS2C_START_SCROLL_Y, TTS2C_SCROLL_SPEED, TTS2C_SCROLL_COUNT );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
    PaletteFadeReq( BTLV_EFFECT_GetPfd(), PF_BIT_SUB_BG, STANDBY_PAL, 1, STANDBY_FADE, 0, STANDBY_FADE_COLOR, ttw->biw->tcbsys );
    ttw->seq_no++;
    break;
  case 1:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（コマンド選択→技選択）
 */
//============================================================================================
static  void  TCB_TransformCommand2Waza( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){
  case 0:
    if( ( ttw->biw->type == BTLV_INPUT_TYPE_TRIPLE ) && ( ttw->pos != BTLV_MCSS_POS_C ) )
    {
     GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0b_NSCR,
                                      GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
    }
    else
    {
     GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0a_NSCR,
                                      GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
    }
    GFL_BMPWIN_MakeScreen( ttw->biw->bmp_win );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
    SetupScrollUp( ttw->biw, TTC2W_START_SCROLL_X, TTC2W_START_SCROLL_Y, TTC2W_SCROLL_SPEED, TTC2W_SCROLL_COUNT );
    SetupScreenAnime( ttw->biw, 0, SCREEN_ANIME_DIR_FORWARD );
    SetupButtonAnime( ttw->biw, BUTTON_TYPE_WAZA, BUTTON_ANIME_TYPE_APPEAR );
    SetupBallGaugeMove( ttw->biw, BALL_GAUGE_MOVE_OPEN );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
    ttw->seq_no++;
    break;
  case 1:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      GFL_BMPWIN_TransVramCharacter( ttw->biw->bmp_win );
      GFL_CLACT_UNIT_SetDrawEnable( ttw->biw->wazatype_clunit, TRUE );
      GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, TSA_SCROLL_X3 );
      GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, TSA_SCROLL_Y3 );
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（技選択→コマンド選択）
 */
//============================================================================================
static  void  TCB_TransformWaza2Command( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){
  case 0:
    if( ttw->biw->center_button_type == BTLV_INPUT_CENTER_BUTTON_ESCAPE )
    {
      GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0a_NSCR,
                                       GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
    }
    else
    {
      if( ( ttw->biw->type == BTLV_INPUT_TYPE_TRIPLE ) && ( ttw->pos != BTLV_MCSS_POS_C ) )
      {
        GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0e_NSCR,
                                         GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
      }
      else
      {
        GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0d_NSCR,
                                         GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
      }
    }
    SetupScrollUp( ttw->biw, TTW2C_START_SCROLL_X, TTW2C_START_SCROLL_Y, TTW2C_SCROLL_SPEED, TTW2C_SCROLL_COUNT );
    SetupScreenAnime( ttw->biw, 0, SCREEN_ANIME_DIR_BACKWARD );
    SetupButtonAnime( ttw->biw, BUTTON_TYPE_WAZA, BUTTON_ANIME_TYPE_VANISH );
    SetupBallGaugeMove( ttw->biw, BALL_GAUGE_MOVE_CLOSE );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
    ttw->seq_no++;
    break;
  case 1:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（ローテーション選択→コマンド選択）
 */
//============================================================================================
static  void  TCB_TransformRotate2Command( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){
  case 0:
    if( ttw->biw->center_button_type == BTLV_INPUT_CENTER_BUTTON_ESCAPE )
    {
      GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0a_NSCR,
                                       GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
    }
    else
    {
      if( ( ttw->biw->type == BTLV_INPUT_TYPE_TRIPLE ) && ( ttw->pos != BTLV_MCSS_POS_C ) )
      {
        GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0e_NSCR,
                                         GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
      }
      else
      {
        GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0d_NSCR,
                                         GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
      }
    }
    GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg1a_NSCR,
                                     GFL_BG_FRAME1_S, 0, 0, FALSE, ttw->biw->heapID );
    SetupScrollUp( ttw->biw, TTW2C_START_SCROLL_X, TTW2C_START_SCROLL_Y, TTW2C_SCROLL_SPEED, TTW2C_SCROLL_COUNT );
    SetupScreenAnime( ttw->biw, 0, SCREEN_ANIME_DIR_BACKWARD );
    SetupButtonAnime( ttw->biw, BUTTON_TYPE_WAZA, BUTTON_ANIME_TYPE_VANISH );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
    ttw->seq_no++;
    break;
  case 1:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（技選択→攻撃対象選択）
 */
//============================================================================================
static  void  TCB_TransformWaza2Dir( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  //現状、どんな演出になるか決まっていないので、表示だけしてタスク終了
  /*
  switch( ttw->seq_no ){
  case 0:
    SetupScrollUp( ttw->biw, TTW2C_START_SCROLL_X, TTW2C_START_SCROLL_Y, TTW2C_SCROLL_SPEED, TTW2C_SCROLL_COUNT );
    SetupScreenAnime( ttw->biw, 0, SCREEN_ANIME_DIR_BACKWARD );
    SetupButtonAnime( ttw->biw, BUTTON_TYPE_WAZA, BUTTON_ANIME_TYPE_VANISH );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
    ttw->seq_no++;
    break;
  case 1:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
  */
  GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0a_NSCR,
                                   GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, ttw->datID,
                                   GFL_BG_FRAME0_S, 0x0400 + 0x40, 0x40 * 14, FALSE, ttw->biw->heapID );
  GFL_BMPWIN_MakeScreen( ttw->biw->bmp_win );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
  if( ttw->biw->type == BTLV_INPUT_TYPE_TRIPLE )
  {
    GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg1c_NSCR,
                                     GFL_BG_FRAME1_S, 0, 0, FALSE, ttw->biw->heapID );
  }
  else
  {
    GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg1b_NSCR,
                                     GFL_BG_FRAME1_S, 0, 0, FALSE, ttw->biw->heapID );
  }
  GFL_BMPWIN_TransVramCharacter( ttw->biw->bmp_win );
  ttw->biw->tcb_execute_flag = 0;
  GFL_HEAP_FreeMemory( ttw );
  GFL_TCB_DeleteTask( tcb );
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（攻撃対象選択→技選択）
 */
//============================================================================================
static  void  TCB_TransformDir2Waza( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  //現状、どんな演出になるか決まっていないので、表示だけしてタスク終了
  /*
  switch( ttw->seq_no ){
  case 0:
    SetupScrollUp( ttw->biw, TTW2C_START_SCROLL_X, TTW2C_START_SCROLL_Y, TTW2C_SCROLL_SPEED, TTW2C_SCROLL_COUNT );
    SetupScreenAnime( ttw->biw, 0, SCREEN_ANIME_DIR_BACKWARD );
    SetupButtonAnime( ttw->biw, BUTTON_TYPE_WAZA, BUTTON_ANIME_TYPE_VANISH );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
    ttw->seq_no++;
    break;
  case 1:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
  */
  if( ( ttw->biw->type == BTLV_INPUT_TYPE_TRIPLE ) && ( ttw->pos != BTLV_MCSS_POS_C ) )
  {
    GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0b_NSCR,
                                     GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
  }
  else
  {
    GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg0a_NSCR,
                                     GFL_BG_FRAME0_S, 0, 0, FALSE, ttw->biw->heapID );
  }
  GFL_BMPWIN_MakeScreen( ttw->biw->bmp_win );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
  GFL_ARCHDL_UTIL_TransVramScreen( ttw->biw->handle, NARC_battgra_wb_battle_w_bg1a_NSCR,
                                   GFL_BG_FRAME1_S, 0, 0, FALSE, ttw->biw->heapID );
  GFL_BMPWIN_TransVramCharacter( ttw->biw->bmp_win );
  GFL_CLACT_UNIT_SetDrawEnable( ttw->biw->wazatype_clunit, TRUE );
  ttw->biw->tcb_execute_flag = 0;
  GFL_HEAP_FreeMemory( ttw );
  GFL_TCB_DeleteTask( tcb );
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（コマンド選択→スタンバイ）
 */
//============================================================================================
static  void  TCB_TransformCommand2Standby( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){
  case 0:
    SetupScaleChange( ttw->biw, TTC2S_START_SCALE, TTC2S_END_SCALE, TTC2S_SCALE_SPEED, COMMAND_POS_Y );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
    ttw->seq_no++;
    break;
  case 1:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（技選択→スタンバイ）
 */
//============================================================================================
static  void  TCB_TransformWaza2Standby( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){
  case 0:
    SetupScreenAnime( ttw->biw, 0, SCREEN_ANIME_DIR_BACKWARD );
    SetupButtonAnime( ttw->biw, BUTTON_TYPE_WAZA, BUTTON_ANIME_TYPE_VANISH );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
    ttw->seq_no++;
    break;
  case 1:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      SetupScaleChange( ttw->biw, TTC2S_START_SCALE, TTC2S_END_SCALE, TTC2S_SCALE_SPEED, COMMAND_POS_Y );
      GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
      ttw->seq_no++;
    }
    break;
  case 2:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（スタンバイ→2択）
 */
//============================================================================================
static  void  TCB_TransformStandby2YesNo( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){
  case 0:
    PMSND_PlaySE( SEQ_SE_OPEN2 );
    GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, TTS2C_FRAME1_SCROLL_X );
    GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, TTS2C_FRAME1_SCROLL_Y );
    SetupScaleChange( ttw->biw, TTS2C_START_SCALE, TTS2C_END_SCALE, -TTS2C_SCALE_SPEED, STANBY_POS_Y );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
    PaletteFadeReq( BTLV_EFFECT_GetPfd(), PF_BIT_SUB_BG, STANDBY_PAL, 1, STANDBY_FADE, 0, STANDBY_FADE_COLOR, ttw->biw->tcbsys );
    ttw->seq_no++;
    break;
  case 1:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      SetupScreenAnime( ttw->biw, 0, SCREEN_ANIME_DIR_FORWARD );
      GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
      ttw->seq_no++;
    }
    break;
  case 2:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      GFL_BMPWIN_MakeScreen( ttw->biw->bmp_win );
      GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
      GFL_BMPWIN_TransVramCharacter( ttw->biw->bmp_win );
      GFL_BG_SetScroll( GFL_BG_FRAME0_S, GFL_BG_SCROLL_X_SET, TSA_SCROLL_X3 );
      GFL_BG_SetScroll( GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, TSA_SCROLL_Y0 );
      GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, TSA_SCROLL_X3 );
      GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, TSA_SCROLL_Y3 );
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//============================================================================================
/**
 *  @brief  下画面変形タスク（スタンバイ→ローテーション）
 */
//============================================================================================
static  void  TCB_TransformStandby2Rotate( GFL_TCB* tcb, void* work )
{
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){
  case 0:
    PMSND_PlaySE( SEQ_SE_OPEN2 );
    GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, TTS2C_FRAME1_SCROLL_X );
    GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, TTS2C_FRAME1_SCROLL_Y );
    SetupScaleChange( ttw->biw, TTS2C_START_SCALE, TTS2C_END_SCALE, -TTS2C_SCALE_SPEED, STANBY_POS_Y );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
    PaletteFadeReq( BTLV_EFFECT_GetPfd(), PF_BIT_SUB_BG, STANDBY_PAL, 1, STANDBY_FADE, 0, STANDBY_FADE_COLOR, ttw->biw->tcbsys );
    ttw->seq_no++;
    break;
  case 1:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      SetupScreenAnime( ttw->biw, 0, SCREEN_ANIME_DIR_FORWARD );
      SetupScrollUp( ttw->biw,
                     rotate_scroll_table[ ttw->biw->rotate_scr ][ 0 ], rotate_scroll_table[ ttw->biw->rotate_scr ][ 1 ],
                     ROTATE_SCROLL_SPEED, ROTATE_SCROLL_COUNT );
      GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
      ttw->seq_no++;
    }
    break;
  case 2:
  default:
    if( ttw->biw->tcb_execute_count == 0 )
    {
      int i;

      for( i = 0 ; i < BTLV_INPUT_POKEICON_MAX ; i++ )
      {
        if( ttw->biw->pokeicon_wk[ i ].clwk )
        {
          GFL_CLACT_WK_SetDrawEnable( ttw->biw->pokeicon_wk[ i ].clwk, TRUE );
        }
      }
      GFL_BMPWIN_MakeScreen( ttw->biw->bmp_win );
      GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
      GFL_BMPWIN_TransVramCharacter( ttw->biw->bmp_win );
      GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, TSA_SCROLL_X3 );
      GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, TSA_SCROLL_Y3 );
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//============================================================================================
/**
 *  @brief  スケール変更処理セットアップ
 *
 *  @param[in]  biw           システム管理構造体のポインタ
 *  @param[in]  start_scale   スケール初期値
 *  @param[in]  end_scale     最終的なスケール値
 *  @param[in]  scale_speed   スケール変更速度
 *  @param[in]  pos_y         スクロールY初期値
 */
//============================================================================================
static  void  SetupScaleChange( BTLV_INPUT_WORK* biw, fx32 start_scale, fx32 end_scale, fx32 scale_speed, int pos_y )
{
  TCB_SCALE_UP* tsu = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_SCALE_UP ) );

  tsu->biw          = biw;
  tsu->start_scale  = start_scale;
  tsu->end_scale    = end_scale;
  tsu->scale_speed  = scale_speed;
  tsu->pos_y        = pos_y;

  GFL_TCB_AddTask( biw->tcbsys, TCB_ScaleChange, tsu, 0 );

  biw->tcb_execute_count++;
}

//============================================================================================
/**
 *  @brief  スケール変更処理タスク
 */
//============================================================================================
static  void  TCB_ScaleChange( GFL_TCB* tcb, void* work )
{
  TCB_SCALE_UP* tsu = ( TCB_SCALE_UP * )work;
  MtxFx22 mtx;

  tsu->start_scale += tsu->scale_speed;
  if( tsu->scale_speed > 0 )
  { 
    tsu->pos_y -= 2;
  }
  else
  { 
    tsu->pos_y += 2;
  }

  MTX_Scale22( &mtx, tsu->start_scale, tsu->start_scale );
  GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, 128, &mtx, 256, 256 );
  GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, tsu->pos_y, &mtx, 256, 256 );
  if( tsu->start_scale == tsu->end_scale )
  {
    tsu->biw->tcb_execute_count--;
    GFL_HEAP_FreeMemory( tsu );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 *  @brief  スクリーンスクロール処理セットアップ
 *
 *  @param[in]  biw           システム管理構造体のポインタ
 *  @param[in]  scroll_x      初期X座標
 *  @param[in]  scroll_y      初期Y座標
 *  @param[in]  scroll_speed  スクロールスピード
 *  @param[in]  scroll_count  スクロールカウント
 */
//============================================================================================
static  void  SetupScrollUp( BTLV_INPUT_WORK* biw, int scroll_x, int scroll_y, int scroll_speed, int scroll_count )
{
  TCB_SCROLL_UP* tsu = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_SCROLL_UP ) );

  GFL_BG_SetScroll( GFL_BG_FRAME0_S, GFL_BG_SCROLL_X_SET, scroll_x );
  GFL_BG_SetScroll( GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, scroll_y );

  tsu->biw          = biw;
  tsu->scroll_y     = scroll_y;
  tsu->scroll_speed = scroll_speed;
  tsu->scroll_count = scroll_count;

  GFL_TCB_AddTask( biw->tcbsys, TCB_ScrollUp, tsu, 0 );

  biw->tcb_execute_count++;
}

//============================================================================================
/**
 *  @brief  スクリーンスクロール処理タスク
 */
//============================================================================================
static  void  TCB_ScrollUp( GFL_TCB* tcb, void* work )
{
  TCB_SCROLL_UP* tsu = ( TCB_SCROLL_UP * )work;

  tsu->scroll_y += tsu->scroll_speed;

  GFL_BG_SetScroll( GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, tsu->scroll_y );

  if( --tsu->scroll_count == 0 )
  {
    tsu->biw->tcb_execute_count--;
    GFL_HEAP_FreeMemory( tsu );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 *  @brief  スクリーンアニメ処理セットアップ
 *
 *  @param[in]  biw   システム管理構造体のポインタ
 *  @param[in]  index 読み込むスクリーンリソースインデックス
 *  @param[in]  dir   アニメーションの向き
 */
//============================================================================================
static  void  SetupScreenAnime( BTLV_INPUT_WORK* biw, int index, SCREEN_ANIME_DIR dir )
{
  TCB_SCREEN_ANIME* tsa = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_SCREEN_ANIME ) );

  tsa->biw    = biw;
  tsa->count  = 0;
  tsa->dir    = dir;
  tsa->wait   = TSA_WAIT;

  //indexから表示するスクリーンを読み替える予定

  GFL_TCB_AddTask( biw->tcbsys, TCB_ScreenAnime, tsa, 0 );

  biw->tcb_execute_count++;
}

//============================================================================================
/**
 *  @brief  スクリーンアニメ処理タスク
 */
//============================================================================================
static  void  TCB_ScreenAnime( GFL_TCB* tcb, void* work )
{
  TCB_SCREEN_ANIME* tsa = ( TCB_SCREEN_ANIME * )work;
  static  const int TSA_scroll_table[ 2 ][ 2 ][ 2 ] = {
    {
      { TSA_SCROLL_X1, TSA_SCROLL_Y1 },
      { TSA_SCROLL_X2, TSA_SCROLL_Y2 },
    },
    {
      { TSA_SCROLL_X1, TSA_SCROLL_Y1 },
      { TSA_SCROLL_X0, TSA_SCROLL_Y0 },
    }
  };

  if( --tsa->wait )
  {
    return;
  }

  tsa->wait = TSA_WAIT;

  GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, TSA_scroll_table[ tsa->dir ][ tsa->count ][ TSA_SCROLL_X ] );
  GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, TSA_scroll_table[ tsa->dir ][ tsa->count++ ][ TSA_SCROLL_Y ] );

  if( tsa->count >= 2 )
  {
    tsa->biw->tcb_execute_count--;
    GFL_HEAP_FreeMemory( tsa );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 *  @brief  ボタンアニメ処理セットアップ
 *
 *  @param[in]  biw       システム管理構造体のポインタ
 *  @param[in]  type      ボタンタイプ
 *  @param[in]  anm_type  ボタンアニメタイプ
 */
//============================================================================================
static  void  SetupButtonAnime( BTLV_INPUT_WORK* biw, BUTTON_TYPE type, BUTTON_ANIME_TYPE anm_type )
{
  TCB_BUTTON_ANIME* tba = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_BUTTON_ANIME ) );
  GFL_CLWK_DATA button = {
    0, 0,     //x, y
    0, 0, 2,  //アニメ番号、優先順位、BGプライオリティ
  };
  static  const BUTTON_ANIME_PARAM bap[ BUTTON_TYPE_MAX ] = {
    {
      //BUTTON_TYPE_WAZA & BUTTON_TYPE_DIR_4
      {
        { BUTTON4_X1, BUTTON4_Y1 },
        { BUTTON4_X2, BUTTON4_Y2 },
        { BUTTON4_X3, BUTTON4_Y3 },
        { BUTTON4_X4, BUTTON4_Y4 },
        { -1, -1 },
        { -1, -1 },
      },
      { BUTTON4_APPEAR_ANIME, BUTTON4_VANISH_ANIME },
    },
    {
      //BUTTON_TYPE_DIR_6
      {
        { BUTTON6_X1, BUTTON6_Y1 },
        { BUTTON6_X2, BUTTON6_Y2 },
        { BUTTON6_X3, BUTTON6_Y3 },
        { BUTTON6_X4, BUTTON6_Y4 },
        { BUTTON6_X5, BUTTON6_Y5 },
        { BUTTON6_X6, BUTTON6_Y6 },
      },
      { BUTTON6_APPEAR_ANIME, BUTTON6_VANISH_ANIME },
    },
    {
      //BUTTON_TYPE_YES_NO
      {
        { BUTTON2_X1, BUTTON2_Y1 },
        { BUTTON2_X2, BUTTON2_Y2 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
        { -1, -1 },
      },
      { BUTTON2_APPEAR_ANIME, BUTTON2_VANISH_ANIME },
    },
  };
  int i;

  tba->biw = biw;
  tba->clunit = GFL_CLACT_UNIT_Create( BUTTON_ANIME_MAX, 0, biw->heapID );

  for( i = 0 ; i < BUTTON_ANIME_MAX ; i++ )
  {
    if( bap[ type ].pos[ i ].x == -1 )
    {
      tba->clwk[ i ] = NULL;
      continue;
    }
    tba->clwk[ i ] = GFL_CLACT_WK_Create( tba->clunit, biw->objcharID, biw->objplttID, biw->objcellID,
                                          &button, CLSYS_DEFREND_SUB, biw->heapID );
    GFL_CLACT_WK_SetPos( tba->clwk[ i ], &bap[ type ].pos[ i ], CLSYS_DEFREND_SUB );
    GFL_CLACT_WK_SetAffineParam( tba->clwk[ i ], CLSYS_AFFINETYPE_DOUBLE );
    GFL_CLACT_WK_SetAutoAnmFlag( tba->clwk[ i ], TRUE );
    GFL_CLACT_WK_SetAnmSeq( tba->clwk[ i ], bap[ type ].anm_no[ anm_type ] );
  }

  GFL_TCB_AddTask( biw->tcbsys, TCB_ButtonAnime, tba, 0 );

  biw->tcb_execute_count++;
}

//============================================================================================
/**
 *  @brief  ボタンアニメ処理タスク
 */
//============================================================================================
static  void  TCB_ButtonAnime( GFL_TCB* tcb, void* work )
{
  TCB_BUTTON_ANIME* tba = ( TCB_BUTTON_ANIME * )work;
  int i;

  for( i = 0 ; i < BUTTON_ANIME_MAX ; i++ )
  {
    if( tba->clwk[ i ] )
    {
      if( GFL_CLACT_WK_CheckAnmActive( tba->clwk[ i ] ) )
      {
        return;
      }
    }
  }
  for( i = 0 ; i < PTL_WAZA_MAX ; i++ )
  {
    if( tba->clwk[ i ] )
    {
      GFL_CLACT_WK_Remove( tba->clwk[ i ] );
    }
  }
  GFL_CLACT_UNIT_Delete( tba->clunit );
  tba->biw->tcb_execute_count--;
  GFL_HEAP_FreeMemory( tba );
  GFL_TCB_DeleteTask( tcb );
}

//============================================================================================
/**
 *  @brief  ボールゲージ移動処理セットアップ
 *
 *  @param[in]  biw  システム管理構造体のポインタ
 *  @param[in]  dir  移動方向
 */
//============================================================================================
static  void  SetupBallGaugeMove( BTLV_INPUT_WORK* biw, BALL_GAUGE_MOVE_DIR dir )
{
  TCB_BALL_GAUGE_MOVE* tbgm = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_BALL_GAUGE_MOVE ) );

  tbgm->biw           = biw;
  tbgm->move_dir      = dir;

  GFL_TCB_AddTask( biw->tcbsys, TCB_BallGaugeMove, tbgm, 0 );
}

//============================================================================================
/**
 *  @brief  ボールゲージ移動処理タスク
 */
//============================================================================================
static  void  TCB_BallGaugeMove( GFL_TCB* tcb, void* work )
{
  TCB_BALL_GAUGE_MOVE*  tbgm = ( TCB_BALL_GAUGE_MOVE * )work;
  BTLV_INPUT_CLWK*      bib;
  GFL_CLACTPOS          pos;
  int                   i, type;
  BOOL                  flag = FALSE;

  static  const int ball_gauge_move_param[ 2 ][ 2 ] = {
    //MINE
    { BTLV_INPUT_BALLGAUGE_MINE_Y_OPEN, BTLV_INPUT_BALLGAUGE_MINE_Y_CLOSE },
    //ENEMY
    { BTLV_INPUT_BALLGAUGE_ENEMY_Y_OPEN, BTLV_INPUT_BALLGAUGE_ENEMY_Y_CLOSE },
  };

  for( type = BALL_GAUGE_TYPE_MINE ; type < BALL_GAUGE_TYPE_MAX ; type++ )
  {
    int move_pos = ball_gauge_move_param[ type ][ tbgm->move_dir ];

    if( type == BALL_GAUGE_TYPE_MINE )
    {
      bib = tbgm->biw->ballgauge_mine;
    }
    else
    {
      if( tbgm->biw->trainer_flag == 0 )
      {
        break;
      }
      bib = tbgm->biw->ballgauge_enemy;
    }
    for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
    {
      GF_ASSERT( bib[ i ].clwk != NULL );
      GFL_CLACT_WK_GetPos( bib[ i ].clwk, &pos, CLSYS_DEFREND_SUB );
      if( pos.y > move_pos )
      {
        pos.y -= BALL_GAUGE_MOVE_VALUE;
        flag = TRUE;
      }
      else if( pos.y < move_pos )
      {
        pos.y += BALL_GAUGE_MOVE_VALUE;
        flag = TRUE;
      }
      GFL_CLACT_WK_SetPos( bib[ i ].clwk, &pos, CLSYS_DEFREND_SUB );
    }
  }
  if( flag == FALSE )
  {
    GFL_HEAP_FreeMemory( tbgm );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 *  @brief  画面フェード
 */
//============================================================================================
static  void  TCB_Fade( GFL_TCB* tcb, void* work )
{
  TCB_FADE_ACT* tfa = ( TCB_FADE_ACT* )work;

  if( ( PaletteFadeCheck( BTLV_EFFECT_GetPfd() ) == FALSE ) && ( GFL_FADE_CheckFade() == FALSE ) )
  {
    if( tfa->biw->fade_flag == BTLV_INPUT_FADE_OUT )
    {
      BTLV_INPUT_ExitBG( tfa->biw );
    }
    tfa->biw->fade_flag = 0;
    GFL_HEAP_FreeMemory( tfa );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 *  @brief  天候アイコン移動
 */
//============================================================================================
static  void  TCB_WeatherIconMove( GFL_TCB* tcb, void* work )
{
  TCB_WEATHER_ICON_MOVE*  twim = ( TCB_WEATHER_ICON_MOVE* )work;
  GFL_CLACTPOS  pos;

  GFL_CLACT_WK_GetPos( twim->biw->weather_wk[ 0 ], &pos, CLSYS_DEFREND_SUB );
  pos.x -= BTLV_INPUT_WEATHER_MOVE_SPEED;
  GFL_CLACT_WK_SetPos( twim->biw->weather_wk[ 0 ], &pos, CLSYS_DEFREND_SUB );
  GFL_CLACT_WK_GetPos( twim->biw->weather_wk[ 1 ], &pos, CLSYS_DEFREND_SUB );
  pos.x -= BTLV_INPUT_WEATHER_MOVE_SPEED;
  GFL_CLACT_WK_SetPos( twim->biw->weather_wk[ 1 ], &pos, CLSYS_DEFREND_SUB );

  if( pos.x == BTLV_INPUT_WEATHER_X2 )
  {
    GFL_HEAP_FreeMemory( twim );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 *  @brief  ローテーションセットアップ
 *
 *  @param[in]  biw  システム管理構造体のポインタ
 *  @param[in]  hit  どっち回転か？（0：左回り　1:右回り）
 */
//============================================================================================
static  void  SetupRotateAction( BTLV_INPUT_WORK* biw, int hit )
{
  int i, j;
  int old_rotate_pos = biw->rotate_flag;
  TCB_ROTATE_ACTION*  tra = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_ROTATE_ACTION ) );

  BTLV_INPUT_ClearScreen( biw );

  if( biw->rotate_flag )
  {
    biw->rotate_flag = 0;
    biw->rotate_scr = bird[ biw->before_select_dir ].init_scr;
    if( ( hit == 0 ) && ( ( biw->rotate_scr == ROTATE_SCR_C ) || ( biw->rotate_scr == ROTATE_SCR_C_ALL ) ) )
    {
      biw->cursor_pos = 1;
    }
  }
  else
  {
    if( hit )
    {
      biw->rotate_flag = 1;
      biw->rotate_scr = bird[ biw->before_select_dir ].right_scr;
    }
    else
    {
      biw->rotate_flag = 2;
      biw->rotate_scr = bird[ biw->before_select_dir ].left_scr;
    }
  }

  SetupScrollUp( biw, rotate_scroll_table[ biw->rotate_scr ][ 0 ], rotate_scroll_table[ biw->rotate_scr ][ 1 ],
                 ROTATE_SCROLL_SPEED, ROTATE_SCROLL_COUNT );

  for( i = 0 ; i < BTLV_INPUT_POKEICON_MAX ; i++ )
  {
    int now_pos = rotate_pos_table[ old_rotate_pos ][ i ];
    int end_pos = rotate_pos_table[ biw->rotate_flag ][ i ];

    tra->now_pos[ now_pos ].x = pokeicon_pos[ i ].x << FX32_SHIFT;
    tra->now_pos[ now_pos ].y = pokeicon_pos[ i ].y << FX32_SHIFT;
    tra->now_pos[ now_pos ].z = 0;
    tra->end_pos[ end_pos ].x = pokeicon_pos[ i ].x << FX32_SHIFT;
    tra->end_pos[ end_pos ].y = pokeicon_pos[ i ].y << FX32_SHIFT;
    tra->end_pos[ end_pos ].z = 0;
  }
  for( i = 0 ; i < BTLV_INPUT_POKEICON_MAX ; i++ )
  {
    BTLV_EFFTOOL_CalcMoveVector( &tra->now_pos[ i ], &tra->end_pos[ i ], &tra->vector[ i ], ROTATE_FRAME << FX32_SHIFT );
  }

  tra->biw = biw;
  tra->frame = ROTATE_FRAME;

  GFL_TCB_AddTask( biw->tcbsys, TCB_RotateAction, tra, 0 );
  biw->tcb_execute_flag = 1;
}

//============================================================================================
/**
 *  @brief  ローテーション
 */
//============================================================================================
static  void  TCB_RotateAction( GFL_TCB* tcb, void* work )
{
  TCB_ROTATE_ACTION*  tra = ( TCB_ROTATE_ACTION* )work;

  switch( tra->seq_no ){
  case 0:
    {
      int           i;
      GFL_CLACTPOS  pos;

      for( i = 0 ; i < BTLV_INPUT_POKEICON_MAX ; i++ )
      {
        if( tra->biw->pokeicon_wk[ i ].clwk )
        {
          BTLV_EFFTOOL_CheckVecMove( &tra->now_pos[ i ], &tra->vector[ i ], &tra->end_pos[ i ], NULL );
          pos.x = tra->now_pos[ i ].x >> FX32_SHIFT;
          pos.y = tra->now_pos[ i ].y >> FX32_SHIFT;
          GFL_CLACT_WK_SetPos( tra->biw->pokeicon_wk[ i ].clwk, &pos, CLSYS_DEFREND_SUB );
        }
      }
      tra->frame--;
      if( tra->frame == 0 )
      {
        tra->seq_no++;
      }
    }
    break;
  default:
  case 1:
    if( tra->biw->tcb_execute_flag == 1 )
    {
      int i;

      for( i = 0 ; i < BTLV_INPUT_POKEICON_MAX ; i++ )
      {
        tra->biw->button_exist[ i ] = button_exist_table[ tra->biw->rotate_scr ][ i ];
      }
      BTLV_INPUT_CreateRotateScreen( tra->biw );
      GFL_BMPWIN_MakeScreen( tra->biw->bmp_win );
      GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
      GFL_BMPWIN_TransVramCharacter( tra->biw->bmp_win );
      tra->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( tra );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief 文字列の長さを取得する
 *
 * @param[in] str           文字列へのポインタ
 * @param[in] font          フォントタイプ
 * @param[in] ret_dot_len   ドット幅代入先
 * @param[in] ret_char_len  キャラ幅代入先
 */
//--------------------------------------------------------------
static void FontLenGet( const STRBUF *str, GFL_FONT *font, int *ret_dot_len, int *ret_char_len )
{
  int dot_len, char_len;

  //文字列のドット幅から、使用するキャラ数を算出する
  dot_len = PRINTSYS_GetStrWidth( str, font, 0 );
  char_len = dot_len / 8;
  if( FX_ModS32( dot_len, 8 ) != 0 ){
    char_len++;
  }

  *ret_dot_len = dot_len;
  *ret_char_len = char_len;
}

//--------------------------------------------------------------
/**
 * @brief   技選択画面BG生成
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreateWazaScreen( BTLV_INPUT_WORK* biw, const BTLV_INPUT_WAZA_PARAM *biwp )
{
  int i;
  int dot_len, char_len;
  ARCDATID  datID;
  STRBUF *wazaname_p;
  STRBUF *wazaname_src;
  STRBUF *pp_p;
  STRBUF *pp_src;
  STRBUF *ppmsg_src;
  WORDSET *wordset;
  PRINTSYS_LSB color;
  static const int wazaname_pos[ PTL_WAZA_MAX ][ 2 ] =
  {
    { WAZANAME_X1, WAZANAME_Y1 },
    { WAZANAME_X2, WAZANAME_Y2 },
    { WAZANAME_X3, WAZANAME_Y3 },
    { WAZANAME_X4, WAZANAME_Y4 },
  };
  int ppmsg_pos[ PTL_WAZA_MAX ][ 2 ] =
  {
    { PPMSG_X1, PPMSG_Y1 },
    { PPMSG_X2, PPMSG_Y2 },
    { PPMSG_X3, PPMSG_Y3 },
    { PPMSG_X4, PPMSG_Y4 },
  };
  int pp_pos[ PTL_WAZA_MAX ][ 2 ] =
  {
    { PP_X1, PP_Y1 },
    { PP_X2, PP_Y2 },
    { PP_X3, PP_Y3 },
    { PP_X4, PP_Y4 },
  };

  wazaname_p = GFL_STR_CreateBuffer( BUFLEN_WAZA_NAME, biw->heapID );
  wazaname_src = GFL_MSG_CreateString( biw->msg, BI_WazaNameMsg );
  ppmsg_src = GFL_MSG_CreateString( biw->msg, BI_PPMsg );
  wordset = WORDSET_Create( biw->heapID );
  pp_p = GFL_STR_CreateBuffer( BUFLEN_BI_WAZAPP, biw->heapID );
  pp_src = GFL_MSG_CreateString( biw->msg,  BI_PPNowMaxMsg );

  GFL_CLACT_UNIT_SetDrawEnable( biw->wazatype_clunit, FALSE );

  for(i = 0; i < PTL_WAZA_MAX; i++)
  {
    if( biwp->wazano[ i ] )
    {
      //技タイプアイコン
      {
        void *arc_data;
        void *obj_vram;
        NNSG2dImageProxy image;
        NNSG2dCharacterData *char_data;
        int waza_type;
        static  const ARCDATID  waza_type_pltt[]=
        {
          NARC_battgra_wb_waza_w_01_NCLR,
          NARC_battgra_wb_waza_w_07_NCLR,
          NARC_battgra_wb_waza_w_10_NCLR,
          NARC_battgra_wb_waza_w_08_NCLR,
          NARC_battgra_wb_waza_w_09_NCLR,
          NARC_battgra_wb_waza_w_13_NCLR,
          NARC_battgra_wb_waza_w_12_NCLR,
          NARC_battgra_wb_waza_w_14_NCLR,
          NARC_battgra_wb_waza_w_16_NCLR,
          NARC_battgra_wb_waza_w_02_NCLR,
          NARC_battgra_wb_waza_w_03_NCLR,
          NARC_battgra_wb_waza_w_05_NCLR,
          NARC_battgra_wb_waza_w_04_NCLR,
          NARC_battgra_wb_waza_w_06_NCLR,
          NARC_battgra_wb_waza_w_15_NCLR,
          NARC_battgra_wb_waza_w_11_NCLR,
          NARC_battgra_wb_waza_w_17_NCLR,
        };
        waza_type = WAZADATA_GetParam( biwp->wazano[ i ], WAZAPARAM_TYPE );
        datID = waza_type_pltt[ waza_type ];

        biw->wazatype_wk[ i ] = GFL_CLACT_WK_Create( biw->wazatype_clunit, biw->wazatype_charID[ i ],
                                                     biw->wazatype_plttID, biw->wazatype_cellID,
                                                     &obj_param, CLSYS_DEFREND_SUB, biw->heapID );
        GFL_CLACT_WK_SetPos( biw->wazatype_wk[ i ], &WazaIconPos[ i ], CLSYS_DEFREND_SUB );

        arc_data = GFL_ARC_UTIL_LoadOBJCharacter( APP_COMMON_GetArcId(),
                                                  APP_COMMON_GetPokeTypeCharArcIdx( waza_type ), WAZATYPEICON_COMP_CHAR,
                                                  &char_data, biw->heapID );

        GFL_CLACT_WK_SetPlttOffs( biw->wazatype_wk[ i ], APP_COMMON_GetPokeTypePltOffset( waza_type ), CLWK_PLTTOFFS_MODE_PLTT_TOP );
        obj_vram = G2S_GetOBJCharPtr();
        GFL_CLACT_WK_GetImgProxy( biw->wazatype_wk[ i ],  &image );
        MI_CpuCopy16( char_data->pRawData,
                      ( void * )( ( u32 )obj_vram + image.vramLocation.baseAddrOfVram[ NNS_G2D_VRAM_TYPE_2DSUB ] ),
                      WAZATYPEICON_OAMSIZE );

        GFL_HEAP_FreeMemory( arc_data );
      }

      //BMPWIN：技名
      WORDSET_RegisterWazaName( wordset, 0, biwp->wazano[ i ] );
      WORDSET_ExpandStr( wordset, wazaname_p, wazaname_src );
      FontLenGet( wazaname_p, biw->font, &dot_len, &char_len );
      PRINTSYS_Print( biw->bmp_data, wazaname_pos[ i ][ 0 ] - ( dot_len / 2 ), wazaname_pos[ i ][ 1 ], wazaname_p, biw->font );

      WORDSET_RegisterNumber(wordset, 0, biwp->pp[ i ],     2, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
      WORDSET_RegisterNumber(wordset, 1, biwp->ppmax[ i ],  2, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
      WORDSET_ExpandStr(wordset, pp_p, pp_src);
      color = PP_FontColorGet( biwp->pp[ i ], biwp->ppmax[ i ] );

      {
        u8 letter, shadow, back;

        GFL_FONTSYS_GetColor( &letter, &shadow, &back );
        GFL_FONTSYS_SetColor( PRINTSYS_LSB_GetL( color ), PRINTSYS_LSB_GetS( color ), PRINTSYS_LSB_GetB( color ) );

        //BMPWIN：PP
        PRINTSYS_Print( biw->bmp_data, ppmsg_pos[ i ][ 0 ], ppmsg_pos[ i ][ 1 ], ppmsg_src, biw->font );
        FontLenGet( pp_p, biw->font, &dot_len, &char_len );
        PRINTSYS_Print( biw->bmp_data, pp_pos[ i ][ 0 ] - ( dot_len / 2 ), pp_pos[ i ][ 1 ], pp_p, biw->font );
        GFL_FONTSYS_SetColor( letter, shadow, back );
      }
      biw->button_exist[ i ] = ( biwp->pp[ i ] != 0 );  //押せるボタンかどうかチェック
    }
    else
    {
      datID = NARC_battgra_wb_waza_w_00_NCLR;
    }
    {
      NNSG2dPaletteData* palData;
      void* dat = GFL_ARC_UTIL_LoadPalette( ARCID_BATTGRA, datID, &palData, biw->heapID );

      PaletteWorkSet( BTLV_EFFECT_GetPfd(), palData->pRawData, FADE_SUB_BG, WAZATYPE_PLTT + i * 0x10, 0x20 );

      GFL_HEAP_FreeMemory( dat );
    }

  }
  biw->button_exist[ i ] = TRUE;  //押せるボタンかどうかチェック
  //3vs3のときは移動ボタンも存在するので、2個分押せるボタンにする
  if( biw->type == BTLV_INPUT_TYPE_TRIPLE )
  {
    biw->button_exist[ i + 1 ] = TRUE;  //押せるボタンかどうかチェック
  }

  WORDSET_Delete( wordset );
  GFL_STR_DeleteBuffer( wazaname_p );
  GFL_STR_DeleteBuffer( ppmsg_src );
  GFL_STR_DeleteBuffer( wazaname_src );
  GFL_STR_DeleteBuffer( pp_src );
  GFL_STR_DeleteBuffer( pp_p );
}

//--------------------------------------------------------------
/**
 * @brief   攻撃対象選択画面BG生成
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreateDirScreen( BTLV_INPUT_WORK* biw, TCB_TRANSFORM_WORK* ttw, const BTLV_INPUT_SCENE_PARAM *bisp )
{
  int i;
  int dot_len, char_len;
  STRBUF *monsname_p;
  STRBUF *monsname_src;
  WORDSET *wordset;
  static  const int monsname_pos[ 2 ][ BTLV_INPUT_DIR_MAX ][ 2 ] =
  {
    {
      { MONSNAME4_X3, MONSNAME4_Y3 },
      { MONSNAME4_X2, MONSNAME4_Y2 },
      { MONSNAME4_X4, MONSNAME4_Y4 },
      { MONSNAME4_X1, MONSNAME4_Y1 },
      { NULL, NULL },
      { NULL, NULL },
    },
    {
      { MONSNAME6_X4, MONSNAME6_Y4 },
      { MONSNAME6_X3, MONSNAME6_Y3 },
      { MONSNAME6_X5, MONSNAME6_Y5 },
      { MONSNAME6_X2, MONSNAME6_Y2 },
      { MONSNAME6_X6, MONSNAME6_Y6 },
      { MONSNAME6_X1, MONSNAME6_Y1 },
    },
  };
  int max = ( biw->type == BTLV_INPUT_TYPE_TRIPLE ) ? BTLV_INPUT_DIR_MAX : 4;
  int type = ( biw->type == BTLV_INPUT_TYPE_TRIPLE ) ? BUTTON_TYPE_DIR_6 : BUTTON_TYPE_DIR_4;

  monsname_p = GFL_STR_CreateBuffer( BUFLEN_POKEMON_NAME, GFL_HEAP_LOWID(biw->heapID) );
  wordset = WORDSET_Create( GFL_HEAP_LOWID(biw->heapID) );

  for( i = 0 ; i < max ; i++){
    if( bisp->bidp[ i ].hp ){
      //BMPWIN：ポケモン名
      monsname_src = GFL_MSG_CreateString( biw->msg, BI_TargetPokemonMaleMsg + bisp->bidp[ i ].sex );
      WORDSET_RegisterPokeNickName( wordset, 0, bisp->bidp[ i ].pp );
      WORDSET_ExpandStr( wordset, monsname_p, monsname_src );
      FontLenGet( monsname_p, biw->font, &dot_len, &char_len );
      PRINTSYS_Print( biw->bmp_data, monsname_pos[ type ][ i ][ 0 ] - ( dot_len / 2 ),
                      monsname_pos[ type ][ i ][ 1 ], monsname_p, biw->font );
      GFL_STR_DeleteBuffer( monsname_src );
    }
    biw->button_exist[ i ] = bisp->bidp[ i ].exist;  //押せるボタンかどうかチェック
  }
  biw->button_exist[ i ] = TRUE;  //押せるボタンかどうかチェック

  WORDSET_Delete( wordset );
  GFL_STR_DeleteBuffer( monsname_p );

  //選択枠表示
  {
    int       pos = ( bisp->pos - BTLV_MCSS_POS_A ) / 2;
    static  const ARCDATID  datID[ 2 ][ 3 ][ 14 ] = {
      //BUTTON_TYPE_DIR_4
      {
        //BTLV_MCSS_POS_A
        {
          NARC_battgra_wb_btl_sel_normal_l_NSCR,  ///< 自分以外１体（選択）
          NARC_battgra_wb_btl_sel_nomine_l_NSCR,  ///< 自分以外全て
          NARC_battgra_wb_btl_sel_enemy_sel_NSCR, ///< 相手側１体（選択）
          NARC_battgra_wb_btl_sel_enemy_NSCR,     ///< 相手側全て
          NARC_battgra_wb_btl_sel_mine_l_NSCR,    ///< 相手側１体ランダム
          NARC_battgra_wb_btl_sel_mine_sel_NSCR,  ///< 自分を含む味方１体（選択）
          NARC_battgra_wb_btl_sel_mine_r_NSCR,    ///< 自分以外の味方１体（選択）
          NARC_battgra_wb_btl_sel_mine_l_NSCR,    ///< 自分のみ
          NARC_battgra_wb_btl_sel_all_NSCR,       ///< 場に出ているポケモン全て
          NARC_battgra_wb_btl_sel_mine_NSCR,      ///< 自分側陣営
          NARC_battgra_wb_btl_sel_enemy_NSCR,     ///< 敵側陣営
          NARC_battgra_wb_btl_sel_all_NSCR,       ///< 場全体（天候など）
          NARC_battgra_wb_btl_sel_mine_l_NSCR,    ///< ゆびをふるなど特殊型
          NARC_battgra_wb_btl_sel_normal_l_NSCR,  ///< 自分以外１体（遠距離）
        },
        //BTLV_MCSS_POS_C
        {
          NARC_battgra_wb_btl_sel_normal_r_NSCR,  ///< 自分以外１体（選択）
          NARC_battgra_wb_btl_sel_nomine_r_NSCR,  ///< 自分以外全て
          NARC_battgra_wb_btl_sel_enemy_sel_NSCR, ///< 相手側１体（選択）
          NARC_battgra_wb_btl_sel_enemy_NSCR,     ///< 相手側全て
          NARC_battgra_wb_btl_sel_mine_r_NSCR,    ///< 相手側１体ランダム
          NARC_battgra_wb_btl_sel_mine_sel_NSCR,  ///< 自分を含む味方１体（選択）
          NARC_battgra_wb_btl_sel_mine_l_NSCR,    ///< 自分以外の味方１体（選択）
          NARC_battgra_wb_btl_sel_mine_r_NSCR,    ///< 自分のみ
          NARC_battgra_wb_btl_sel_all_NSCR,       ///< 場に出ているポケモン全て
          NARC_battgra_wb_btl_sel_mine_NSCR,      ///< 自分側陣営
          NARC_battgra_wb_btl_sel_enemy_NSCR,     ///< 敵側陣営
          NARC_battgra_wb_btl_sel_all_NSCR,       ///< 場全体（天候など）
          NARC_battgra_wb_btl_sel_mine_r_NSCR,    ///< ゆびをふるなど特殊型
          NARC_battgra_wb_btl_sel_normal_r_NSCR,  ///< 自分以外１体（遠距離）
        },
        //BTLV_MCSS_POS_E
        {
          0xffffffff,                             ///< 自分以外１体（選択）
          0xffffffff,                             ///< 自分以外全て
          0xffffffff,                             ///< 相手側１体（選択）
          0xffffffff,                             ///< 相手側全て
          0xffffffff,                             ///< 相手側１体ランダム
          0xffffffff,                             ///< 自分を含む味方１体（選択）
          0xffffffff,                             ///< 自分以外の味方１体（選択）
          0xffffffff,                             ///< 自分のみ
          0xffffffff,                             ///< 場に出ているポケモン全て
          0xffffffff,                             ///< 自分側陣営
          0xffffffff,                             ///< 敵側陣営
          0xffffffff,                             ///< 場全体（天候など）
          0xffffffff,                             ///< ゆびをふるなど特殊型
          0xffffffff,                             ///< 自分以外１体（遠距離）
        },
      },
      //BUTTON_TYPE_DIR_6
      {
        //BTLV_MCSS_POS_A
        {
          NARC_battgra_wb_btl_sel3_normal_l_NSCR,     ///< 自分以外１体（選択）
          NARC_battgra_wb_btl_sel3_nomine_l_NSCR,     ///< 自分以外全て
          NARC_battgra_wb_btl_sel3_enemy_sel_l_NSCR,  ///< 相手側１体（選択）
          NARC_battgra_wb_btl_sel3_enemy_l_NSCR,      ///< 相手側全て
          NARC_battgra_wb_btl_sel3_mine_l_NSCR,       ///< 相手側１体ランダム
          NARC_battgra_wb_btl_sel3_mine_sel_NSCR,     ///< 自分を含む味方１体（選択）
          NARC_battgra_wb_btl_sel3_pair_l_NSCR,       ///< 自分以外の味方１体（選択）
          NARC_battgra_wb_btl_sel3_mine_l_NSCR,       ///< 自分のみ
          NARC_battgra_wb_btl_sel3_all_NSCR,          ///< 場に出ているポケモン全て
          NARC_battgra_wb_btl_sel3_mine_NSCR,         ///< 自分側陣営
          NARC_battgra_wb_btl_sel3_enemy_c_NSCR,      ///< 敵側陣営
          NARC_battgra_wb_btl_sel3_all_NSCR,          ///< 場全体（天候など）
          NARC_battgra_wb_btl_sel3_mine_l_NSCR,       ///< ゆびをふるなど特殊型
          NARC_battgra_wb_btl_sel3_long_l_NSCR,       ///< 自分以外１体（遠距離）
        },
        //BTLV_MCSS_POS_C
        {
          NARC_battgra_wb_btl_sel3_normal_c_NSCR,     ///< 自分以外１体（選択）
          NARC_battgra_wb_btl_sel3_nomine_c_NSCR,     ///< 自分以外全て
          NARC_battgra_wb_btl_sel3_enemy_sel_c_NSCR,  ///< 相手側１体（選択）
          NARC_battgra_wb_btl_sel3_enemy_c_NSCR,      ///< 相手側全て
          NARC_battgra_wb_btl_sel3_mine_c_NSCR,       ///< 相手側１体ランダム
          NARC_battgra_wb_btl_sel3_mine_sel_NSCR,     ///< 自分を含む味方１体（選択）
          NARC_battgra_wb_btl_sel3_pair_c_NSCR,       ///< 自分以外の味方１体（選択）
          NARC_battgra_wb_btl_sel3_mine_c_NSCR,       ///< 自分のみ
          NARC_battgra_wb_btl_sel3_all_NSCR,          ///< 場に出ているポケモン全て
          NARC_battgra_wb_btl_sel3_mine_NSCR,         ///< 自分側陣営
          NARC_battgra_wb_btl_sel3_enemy_c_NSCR,      ///< 敵側陣営
          NARC_battgra_wb_btl_sel3_all_NSCR,          ///< 場全体（天候など）
          NARC_battgra_wb_btl_sel3_mine_c_NSCR,       ///< ゆびをふるなど特殊型
          NARC_battgra_wb_btl_sel3_normal_c_NSCR,     ///< 自分以外１体（遠距離）
        },
        //BTLV_MCSS_POS_E
        {
          NARC_battgra_wb_btl_sel3_normal_r_NSCR,     ///< 自分以外１体（選択）
          NARC_battgra_wb_btl_sel3_nomine_r_NSCR,     ///< 自分以外全て
          NARC_battgra_wb_btl_sel3_enemy_sel_r_NSCR,  ///< 相手側１体（選択）
          NARC_battgra_wb_btl_sel3_enemy_r_NSCR,      ///< 相手側全て
          NARC_battgra_wb_btl_sel3_mine_r_NSCR,       ///< 相手側１体ランダム
          NARC_battgra_wb_btl_sel3_mine_sel_NSCR,     ///< 自分を含む味方１体（選択）
          NARC_battgra_wb_btl_sel3_pair_r_NSCR,       ///< 自分以外の味方１体（選択）
          NARC_battgra_wb_btl_sel3_mine_r_NSCR,       ///< 自分のみ
          NARC_battgra_wb_btl_sel3_all_NSCR,          ///< 場に出ているポケモン全て
          NARC_battgra_wb_btl_sel3_mine_NSCR,         ///< 自分側陣営
          NARC_battgra_wb_btl_sel3_enemy_c_NSCR,      ///< 敵側陣営
          NARC_battgra_wb_btl_sel3_all_NSCR,          ///< 場全体（天候など）
          NARC_battgra_wb_btl_sel3_mine_r_NSCR,       ///< ゆびをふるなど特殊型
          NARC_battgra_wb_btl_sel3_long_r_NSCR,       ///< 自分以外１体（遠距離）
        },
      },
    };
    ttw->datID = datID[ type ][ pos ][ bisp->waza_target ];
  }
}

//--------------------------------------------------------------
/**
 * @brief   YES/NO選択画面BG生成
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreateYesNoScreen( BTLV_INPUT_WORK* biw, const BTLV_INPUT_YESNO_PARAM *biyp )
{
  STRBUF*       src;
  int           dot_len, char_len;

  FontLenGet( biyp->yes_msg, biw->font, &dot_len, &char_len );
  PRINTSYS_Print( biw->bmp_data, BTLV_INPUT_YESNO_MSG_X - ( dot_len / 2 ), BTLV_INPUT_YES_MSG_Y, biyp->yes_msg, biw->font );

  FontLenGet( biyp->no_msg, biw->font, &dot_len, &char_len );
  PRINTSYS_Print( biw->bmp_data, BTLV_INPUT_YESNO_MSG_X - ( dot_len / 2 ), BTLV_INPUT_NO_MSG_Y, biyp->no_msg, biw->font );

  //2個分押せる状態にする
  biw->button_exist[ 0 ] = TRUE;  //押せるボタンかどうかチェック
  biw->button_exist[ 1 ] = TRUE;  //押せるボタンかどうかチェック

  GFL_ARCHDL_UTIL_TransVramScreen( biw->handle, NARC_battgra_wb_battle_w_bg1d_NSCR,
                                   GFL_BG_FRAME1_S, 0, 0, FALSE, biw->heapID );
}

//--------------------------------------------------------------
/**
 * @brief   ROTATE画面BG生成
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreateRotateScreen( BTLV_INPUT_WORK* biw )
{
  STRBUF *monsname_p;
  STRBUF *monsname_src;
  WORDSET *wordset;
  static  const int monsname_pos[ BTLV_INPUT_POKEICON_MAX ][ 2 ] =
  {
    { ROTATE_MONSNAME_X0, ROTATE_MONSNAME_Y0 },
    { ROTATE_MONSNAME_X1, ROTATE_MONSNAME_Y1 },
    { ROTATE_MONSNAME_X2, ROTATE_MONSNAME_Y2 },
  };
  int dot_len, char_len;
  int i;

  monsname_p = GFL_STR_CreateBuffer( BUFLEN_POKEMON_NAME, GFL_HEAP_LOWID( biw->heapID ) );
  wordset = WORDSET_Create( GFL_HEAP_LOWID( biw->heapID ) );

  for( i = 0 ; i < BTLV_INPUT_POKEICON_MAX ; i++ )
  {
    int pos = rotate_pos_table[ biw->rotate_flag ][ i ];
    if( biw->rotate_pp[ pos ] )
    {
      int sex = PP_Get( biw->rotate_pp[ pos ], ID_PARA_sex, NULL );
      //BMPWIN：ポケモン名
      monsname_src = GFL_MSG_CreateString( biw->msg, BI_TargetPokemonMaleMsg + sex );
      WORDSET_RegisterPokeNickName( wordset, 0, biw->rotate_pp[ pos ] );
      WORDSET_ExpandStr( wordset, monsname_p, monsname_src );
      FontLenGet( monsname_p, biw->font, &dot_len, &char_len );
      PRINTSYS_Print( biw->bmp_data, monsname_pos[ i ][ 0 ] - ( dot_len / 2 ), monsname_pos[ i ][ 1 ], monsname_p, biw->font );
      GFL_STR_DeleteBuffer( monsname_src );
    }
  }

  WORDSET_Delete( wordset );
  GFL_STR_DeleteBuffer( monsname_p );

  GFL_ARCHDL_UTIL_TransVramScreen( biw->handle, NARC_battgra_wb_battle_w_bg0c_NSCR,
                                   GFL_BG_FRAME0_S, 0, 0, FALSE, biw->heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( biw->handle, NARC_battgra_wb_battle_w_bg1e_NSCR,
                                   GFL_BG_FRAME1_S, 0, 0, FALSE, biw->heapID );
}

//--------------------------------------------------------------
/**
 * @brief   ROTATE画面PokeIcon生成
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreateRotatePokeIcon( BTLV_INPUT_WORK* biw )
{
  int i;
  ARCHANDLE*  hdl = GFL_ARC_OpenDataHandle( ARCID_POKEICON, biw->heapID );

  biw->pokeicon_cellID = GFL_CLGRP_CELLANIM_Register( hdl, POKEICON_GetCellSubArcIndex(), POKEICON_GetAnmSubArcIndex(),
                                                        biw->heapID );
  biw->pokeicon_plttID = GFL_CLGRP_PLTT_RegisterComp( hdl, POKEICON_GetPalArcIndex(), CLSYS_DRAW_SUB,
                                                        0x20 * 12, biw->heapID );
  PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_SUB_OBJ,
                           GFL_CLGRP_PLTT_GetAddr( biw->pokeicon_plttID, CLSYS_DRAW_SUB ) / 2, 0x20 * 3 );

  for( i = 0 ; i < BTLV_INPUT_POKEICON_MAX ; i++ )
  {
    if( biw->rotate_pp[ i ] )
    {
      int mons_no = PP_Get( biw->rotate_pp[ i ], ID_PARA_monsno, NULL );
      int form_no = PP_Get( biw->rotate_pp[ i ], ID_PARA_form_no, NULL );
      biw->pokeicon_charID[ i ] = GFL_CLGRP_CGR_Register( hdl,
                                                          POKEICON_GetCgxArcIndexByMonsNumber( mons_no,
                                                                                               form_no,
                                                                                               FALSE ),
                                                          FALSE, CLSYS_DRAW_SUB, biw->heapID );
      biw->pokeicon_wk[ i ].clwk = GFL_CLACT_WK_Create( biw->pokeicon_clunit, biw->pokeicon_charID[ i ],
                                                        biw->pokeicon_plttID, biw->pokeicon_cellID,
                                                        &obj_param, CLSYS_DEFREND_SUB, biw->heapID );
      GFL_CLACT_WK_SetPos( biw->pokeicon_wk[ i ].clwk, &pokeicon_pos[ i ], CLSYS_DEFREND_SUB );
      GFL_CLACT_WK_SetAutoAnmFlag( biw->pokeicon_wk[ i ].clwk, TRUE );
      GFL_CLACT_WK_SetAnmSeq( biw->pokeicon_wk[ i ].clwk, 1 );
      GFL_CLACT_WK_SetPlttOffs( biw->pokeicon_wk[ i ].clwk,
                                POKEICON_GetPalNum( mons_no, form_no, FALSE ), CLWK_PLTTOFFS_MODE_OAM_COLOR );
      GFL_CLACT_WK_SetDrawEnable( biw->pokeicon_wk[ i ].clwk, FALSE );
    }
  }
  GFL_ARC_CloseDataHandle( hdl );
}

//--------------------------------------------------------------
/**
 * @brief   画面クリア（画面推移時に消さなければならないスクリーンを消す）
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_ClearScreen( BTLV_INPUT_WORK* biw )
{
  int i;

  //技選択
  GFL_BMP_Clear( biw->bmp_data, 0x00 );
  GFL_BMPWIN_TransVramCharacter( biw->bmp_win );

  for( i = 0 ; i < PTL_WAZA_MAX ; i++ )
  {
    if( biw->wazatype_wk[ i ] )
    {
      GFL_CLACT_WK_Remove( biw->wazatype_wk[ i ] );
      biw->wazatype_wk[ i ] = NULL;
    }
  }

  //攻撃対象選択
  GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0, 32, 2, 32, 32, 0 );
}

//--------------------------------------------------------------
/**
 * @brief   PPの表示色を取得する
 *
 * @param   pp      現在のPP
 * @param   pp_max    最大PP
 *
 * @retval  カラー
 */
//--------------------------------------------------------------
static PRINTSYS_LSB PP_FontColorGet(int pp, int pp_max)
{
  if( pp == 0 ){
    return MSGCOLOR_PP_RED;
  }
  if( pp_max == pp ){
    return MSGCOLOR_PP_WHITE;
  }
  if( pp_max <= 2 ){
    if( pp == 1 ){
      return MSGCOLOR_PP_ORANGE;
    }
  }
  else if( pp_max <= 7 ){
    switch( pp ){
    case 1:
      return MSGCOLOR_PP_ORANGE;
    case 2:
      return MSGCOLOR_PP_YELLOW;
    }
  }
  else{
    if( pp <= ( pp_max / 4 ) ){
      return MSGCOLOR_PP_ORANGE;
    }
    if( pp <= ( pp_max / 2 ) ){
      return MSGCOLOR_PP_YELLOW;
    }
  }
  return MSGCOLOR_PP_WHITE;
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコン生成
 *
 * @param[in] biw   システム管理構造体のポインタ
 * @param[in] bicp  ポケモンアイコン生成用パラメータのポインタ
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreatePokeIcon( BTLV_INPUT_WORK* biw, BTLV_INPUT_COMMAND_PARAM* bicp )
{
    ARCHANDLE*  hdl;
    int         i;
    int         max = ( biw->type == BTLV_INPUT_TYPE_ROTATION ) ? 2 : biw->type + 1;
    int         mask = 0;

    //シングルでは出さないように仕様変更（後でまた出すことになると困るので、処理自体は残しておく）
    if( biw->type == BTLV_INPUT_TYPE_SINGLE )
    { 
      return;
    }

    hdl = GFL_ARC_OpenDataHandle( ARCID_POKEICON, biw->heapID );

    biw->pokeicon_cellID = GFL_CLGRP_CELLANIM_Register( hdl, POKEICON_GetCellSubArcIndex(), POKEICON_GetAnmSubArcIndex(),
                                                        biw->heapID );
    biw->pokeicon_plttID = GFL_CLGRP_PLTT_RegisterComp( hdl, POKEICON_GetPalArcIndex(), CLSYS_DRAW_SUB,
                                                        0x20 * 12, biw->heapID );
    PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_SUB_OBJ,
                             GFL_CLGRP_PLTT_GetAddr( biw->pokeicon_plttID, CLSYS_DRAW_SUB ) / 2, 0x20 * 3 );

    //ウインドウマスクでアイコンを暗くする実験
    if( max > 1 )
    {
      GXS_SetVisibleWnd( GX_WNDMASK_W0 | GX_WNDMASK_W1 );
      G2S_SetWnd0InsidePlane( GX_WND_PLANEMASK_ALL, TRUE );
      G2S_SetWnd1InsidePlane( GX_WND_PLANEMASK_ALL, TRUE );
      G2S_SetWndOutsidePlane( GX_WND_PLANEMASK_ALL, FALSE );
      G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_OBJ, -8 );
    }

    for( i = 0; i < max ; i++ ){
      if( bicp->mons_no[ i ] )
      {
        static  const GFL_CLACTPOS pos[ 4 ][ 3 ] = {
          //SINGLE
          {
            { BTLV_INPUT_POKE_ICON_SINGLE_X, BTLV_INPUT_POKE_ICON_SINGLE_Y },
            { -1, -1 },
            { -1, -1 },
          },
          //DOUBLE
          {
            { BTLV_INPUT_POKE_ICON_DOUBLE_X1, BTLV_INPUT_POKE_ICON_DOUBLE_Y1 },
            { BTLV_INPUT_POKE_ICON_DOUBLE_X2, BTLV_INPUT_POKE_ICON_DOUBLE_Y2 },
            { -1, -1 },
          },
          //TRIPLE
          {
            { BTLV_INPUT_POKE_ICON_TRIPLE_X1, BTLV_INPUT_POKE_ICON_TRIPLE_Y1 },
            { BTLV_INPUT_POKE_ICON_TRIPLE_X2, BTLV_INPUT_POKE_ICON_TRIPLE_Y2 },
            { BTLV_INPUT_POKE_ICON_TRIPLE_X3, BTLV_INPUT_POKE_ICON_TRIPLE_Y3 },
          },
          //ROTATION
          {
            { BTLV_INPUT_POKE_ICON_DOUBLE_X1, BTLV_INPUT_POKE_ICON_DOUBLE_Y1 },
            { BTLV_INPUT_POKE_ICON_DOUBLE_X2, BTLV_INPUT_POKE_ICON_DOUBLE_Y2 },
            { -1, -1 },
          },
        };

        biw->pokeicon_charID[ i ] = GFL_CLGRP_CGR_Register( hdl,
                                                            POKEICON_GetCgxArcIndexByMonsNumber( bicp->mons_no[ i ],
                                                                                                 bicp->form_no[ i ],
                                                                                                 FALSE ),
                                                            FALSE, CLSYS_DRAW_SUB, biw->heapID );
        biw->pokeicon_wk[ i ].clwk = GFL_CLACT_WK_Create( biw->pokeicon_clunit, biw->pokeicon_charID[ i ],
                                                          biw->pokeicon_plttID, biw->pokeicon_cellID,
                                                          &obj_param, CLSYS_DEFREND_SUB, biw->heapID );
        GFL_CLACT_WK_SetPos( biw->pokeicon_wk[ i ].clwk, &pos[ biw->type ][ i ], CLSYS_DEFREND_SUB );
        GFL_CLACT_WK_SetAutoAnmFlag( biw->pokeicon_wk[ i ].clwk, TRUE );
        if( bicp->pos == i )
        {
          GFL_CLACT_WK_SetAnmSeq( biw->pokeicon_wk[ i ].clwk, 1 );
        }
        else
        {
          if( mask == 0 )
          {
            G2S_SetWnd0Position( pos[ biw->type ][ i ].x - 16, pos[ biw->type ][ i ].y - 16,
                                 pos[ biw->type ][ i ].x + 16, pos[ biw->type ][ i ].y + 16 );
            mask++;
          }
          else
          {
            G2S_SetWnd1Position( pos[ biw->type ][ i ].x - 16, pos[ biw->type ][ i ].y - 16,
                                 pos[ biw->type ][ i ].x + 16, pos[ biw->type ][ i ].y + 16 );
          }
        }
        GFL_CLACT_WK_SetPlttOffs( biw->pokeicon_wk[ i ].clwk,
                                  POKEICON_GetPalNum( bicp->mons_no[ i ], bicp->form_no[ i ], FALSE ),
                                  CLWK_PLTTOFFS_MODE_OAM_COLOR );
      }
    }
    GFL_ARC_CloseDataHandle( hdl );
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコン削除
 *
 * @param[in] biw   システム管理構造体のポインタ
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_DeletePokeIcon( BTLV_INPUT_WORK* biw )
{
  int i;

  for( i = 0 ; i < 3 ; i++ )
  {
    if( biw->pokeicon_wk[ i ].clwk )
    {
      GFL_CLACT_WK_Remove( biw->pokeicon_wk[ i ].clwk );
      GFL_CLGRP_CGR_Release( biw->pokeicon_charID[ i ] );
      biw->pokeicon_wk[ i ].clwk = NULL;
      biw->pokeicon_charID[ i ] = GFL_CLGRP_REGISTER_FAILED;
    }
  }

  if( biw->pokeicon_cellID != GFL_CLGRP_REGISTER_FAILED )
  {
    GFL_CLGRP_CELLANIM_Release( biw->pokeicon_cellID );
    biw->pokeicon_cellID = GFL_CLGRP_REGISTER_FAILED;
  }
  if( biw->pokeicon_plttID != GFL_CLGRP_REGISTER_FAILED )
  {
    GFL_CLGRP_PLTT_Release( biw->pokeicon_plttID );
    biw->pokeicon_plttID = GFL_CLGRP_REGISTER_FAILED;
  }

  GXS_SetVisibleWnd( GX_WNDMASK_NONE );
  G2S_SetWnd0InsidePlane( GX_WND_PLANEMASK_NONE, FALSE );
  G2S_SetWnd1InsidePlane( GX_WND_PLANEMASK_NONE, FALSE );
  G2S_SetWndOutsidePlane( GX_WND_PLANEMASK_NONE, FALSE );
  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_NONE, 0 );

}

//--------------------------------------------------------------
/**
 * @brief   天候アイコン生成
 *
 * @param[in] biw   システム管理構造体のポインタ
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreateWeatherIcon( BTLV_INPUT_WORK* biw )
{
  BtlWeather  btl_weather = BTL_FIELD_GetWeather();
  int pal[] = { 0, 1, 3, 2, 1 };
  int ofs_x[] = {   5, 0,  1,  1,   1 };
  int ofs_y[] = { -12, 0, -8, -8, -12 };
  GFL_CLACTPOS  weather;

  if( btl_weather == BTL_WEATHER_NONE )
  {
    return;
  }

  biw->weather_wk[ 0 ] = GFL_CLACT_WK_Create( biw->weather_clunit, biw->objcharID, biw->objplttID, biw->objcellID,
                                              &obj_param, CLSYS_DEFREND_SUB, biw->heapID );
  weather.x = BTLV_INPUT_WEATHER_X1 + ofs_x[ btl_weather - 1 ];
  weather.y = BTLV_INPUT_WEATHER_Y + ofs_y[ btl_weather - 1 ];
  GFL_CLACT_WK_SetPos( biw->weather_wk[ 0 ], &weather, CLSYS_DEFREND_SUB );
  GFL_CLACT_WK_SetAutoAnmFlag( biw->weather_wk[ 0 ], TRUE );
  GFL_CLACT_WK_SetAnmSeq( biw->weather_wk[ 0 ], BTLV_INPUT_WEATHER_BASE + btl_weather );

  biw->weather_wk[ 1 ] = GFL_CLACT_WK_Create( biw->weather_clunit, biw->objcharID, biw->objplttID, biw->objcellID,
                                              &obj_param, CLSYS_DEFREND_SUB, biw->heapID );
  weather.x = BTLV_INPUT_WEATHER_X1;
  weather.y = BTLV_INPUT_WEATHER_Y;
  GFL_CLACT_WK_SetPos( biw->weather_wk[ 1 ], &weather, CLSYS_DEFREND_SUB );
  GFL_CLACT_WK_SetAutoAnmFlag( biw->weather_wk[ 1 ], TRUE );
  GFL_CLACT_WK_SetAnmSeq( biw->weather_wk[ 1 ], BTLV_INPUT_WEATHER_BASE );
  GFL_CLACT_WK_SetPlttOffs( biw->weather_wk[ 1 ], pal[ btl_weather - 1 ], CLWK_PLTTOFFS_MODE_OAM_COLOR );

  //GFL_CLACT_WK_SetDrawEnable( biw->weather_wk[ 1 ], FALSE );

  {
    TCB_WEATHER_ICON_MOVE*  twim = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_WEATHER_ICON_MOVE ) );
    twim->biw = biw;

    GFL_TCB_AddTask( biw->tcbsys, TCB_WeatherIconMove, twim, 0 );
  }
}

//--------------------------------------------------------------
/**
 * @brief   天候アイコン削除
 *
 * @param[in] biw   システム管理構造体のポインタ
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_DeleteWeatherIcon( BTLV_INPUT_WORK* biw )
{
  int i;

  for( i = 0 ; i < BTLV_INPUT_WEATHER_ICON_MAX ; i++ )
  {
    if( biw->weather_wk[ i ] )
    {
      GFL_CLACT_WK_Remove( biw->weather_wk[ i ] );
      biw->weather_wk[ i ] = NULL;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   ボールゲージ生成
 *
 * @param[in] biw   システム管理構造体のポインタ
 * @param[in] bidp  ボールゲージ生成用パラメータのポインタ
 * @param[in] type  ボールゲージタイプ
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreateBallGauge( BTLV_INPUT_WORK* biw, const BTLV_INPUT_COMMAND_PARAM *bicp, BALL_GAUGE_TYPE type )
{
  BTLV_INPUT_CLWK*  bib;
  int               i;
  GFL_CLACTPOS      pos;
  int               pos_ofs;
  int               anm_ofs;

  if( type == BALL_GAUGE_TYPE_ENEMY )
  {
    bib = biw->ballgauge_enemy;
    pos.x = BTLV_INPUT_BALLGAUGE_ENEMY_X;
    pos.y = BTLV_INPUT_BALLGAUGE_ENEMY_Y_CLOSE;
    pos_ofs = -8;
    anm_ofs = 4;
  }
  else
  {
    bib = biw->ballgauge_mine;
    pos.x = BTLV_INPUT_BALLGAUGE_MINE_X;
    pos.y = BTLV_INPUT_BALLGAUGE_MINE_Y_CLOSE;
    pos_ofs = 16;
    anm_ofs = 0;
  }

  for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
  {
    GF_ASSERT( bib[ i ].clwk == NULL )
    bib[ i ].clwk = GFL_CLACT_WK_Create( biw->ballgauge_clunit, biw->objcharID, biw->objplttID, biw->objcellID,
                                          &obj_param, CLSYS_DEFREND_SUB, biw->heapID );
    GFL_CLACT_WK_SetPos( bib[ i ].clwk, &pos, CLSYS_DEFREND_SUB );
    GFL_CLACT_WK_SetAutoAnmFlag( bib[ i ].clwk, TRUE );
    GFL_CLACT_WK_SetAnmSeq( bib[ i ].clwk, bicp->bidp[ type ][ i ].status + anm_ofs );
    pos.x += pos_ofs;
  }
}

//--------------------------------------------------------------
/**
 * @brief   ボールゲージ削除
 *
 * @param[in] biw   システム管理構造体のポインタ
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_DeleteBallGauge( BTLV_INPUT_WORK* biw )
{
  BTLV_INPUT_CLWK* bib;
  int                   i;
  int                   type;

  for( type = 0 ; type < 2 ; type++ )
  {
    if( type )
    {
      bib = biw->ballgauge_enemy;
    }
    else
    {
      bib = biw->ballgauge_mine;
    }

    for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
    {
      if( bib[ i ].clwk )
      {
        GFL_CLACT_WK_Remove( bib[ i ].clwk );
        bib[ i ].clwk = NULL;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   カーソルOBJ生成
 *
 * @param[in] biw   システム管理構造体のポインタ
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreateCursorOBJ( BTLV_INPUT_WORK* biw )
{
  int           i;
  int           anm_no[] = {
    BTLV_INPUT_CURSOR_LU_ANIME,
    BTLV_INPUT_CURSOR_RU_ANIME,
    BTLV_INPUT_CURSOR_LD_ANIME,
    BTLV_INPUT_CURSOR_RD_ANIME,
    BTLV_INPUT_CURSOR_LD_ANIME,
    BTLV_INPUT_CURSOR_RD_ANIME,
  };

  for( i = 0 ; i < BTLV_INPUT_CURSOR_MAX ; i++ )
  {
    GF_ASSERT( biw->cursor[ i ].clwk == NULL )
    biw->cursor[ i ].clwk = GFL_CLACT_WK_Create( biw->cursor_clunit, biw->cur_charID, biw->objplttID, biw->cur_cellID,
                                                 &obj_param, CLSYS_DEFREND_SUB, biw->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( biw->cursor[ i ].clwk, TRUE );
    GFL_CLACT_WK_SetAnmSeq( biw->cursor[ i ].clwk, anm_no[ i ] );
    GFL_CLACT_WK_SetDrawEnable( biw->cursor[ i ].clwk, FALSE );
  }
}

//--------------------------------------------------------------
/**
 * @brief   カーソルOBJ削除
 *
 * @param[in] biw   システム管理構造体のポインタ
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_DeleteCursorOBJ( BTLV_INPUT_WORK* biw )
{
  int i;

  for( i = 0 ; i < BTLV_INPUT_CURSOR_MAX ; i++ )
  {
    if( biw->cursor[ i ].clwk )
    {
      GFL_CLACT_WK_Remove( biw->cursor[ i ].clwk );
      biw->cursor[ i ].clwk = NULL;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   キー操作チェック
 *
 * @param[in] biw     システム管理構造体のポインタ
 * @param[in] tp_tbl  タッチパネルテーブル（カーソルの表示位置決定に使用）
 * @param[in] key_tbl キー操作テーブル
 * @param[in] hit     キータッチがあったかどうか？
 */
//--------------------------------------------------------------
static  int   BTLV_INPUT_CheckKey( BTLV_INPUT_WORK* biw, const GFL_UI_TP_HITTBL* tp_tbl, const BTLV_INPUT_KEYTBL* key_tbl, int hit )
{
  int trg = GFL_UI_KEY_GetTrg();
  BOOL decide_flag = FALSE;

  if( biw->cursor_decide )
  {
    biw->cursor_decide = 0;
    BTLV_INPUT_PutCursorOBJ( biw, tp_tbl, key_tbl );
    GFL_CLACT_UNIT_SetDrawEnable( biw->cursor_clunit, TRUE );
  }

  if( hit != GFL_UI_TP_HIT_NONE )
  {
    *(biw->cursor_mode) = 0;
    biw->cursor_pos = 0;
    biw->old_cursor_pos = CURSOR_NOMOVE;
    BTLV_INPUT_PutCursorOBJ( biw, tp_tbl, key_tbl );
    return hit;
  }

  if( trg )
  {
    if( *(biw->cursor_mode) )
    {
      s8 move_pos = BTLV_INPUT_NOMOVE;
      const BTLV_INPUT_KEYTBL* tbl = &key_tbl[ biw->cursor_pos ];

      switch( trg ){
      case PAD_KEY_UP:
        move_pos = tbl->up;
        break;
      case PAD_KEY_DOWN:
        move_pos = tbl->down;
        break;
      case PAD_KEY_LEFT:
        move_pos = tbl->left;
        break;
      case PAD_KEY_RIGHT:
        move_pos = tbl->right;
        break;
      case PAD_BUTTON_B:
        move_pos = tbl->b_button;
        break;
      }
      if( trg == PAD_BUTTON_A )
      {
        hit = tbl->a_button;
        decide_flag = TRUE;
      }
      else
      {
        if( move_pos != BTLV_INPUT_NOMOVE )
        {
          SePlaySelect();
          if( move_pos < 0 )
          {
            if( ( -move_pos != biw->old_cursor_pos ) && ( biw->old_cursor_pos != CURSOR_NOMOVE ) )
            {
              move_pos = biw->old_cursor_pos;
            }
            else
            {
              move_pos *= -1;
            }
          }
          biw->old_cursor_pos = biw->cursor_pos;
          biw->cursor_pos = move_pos;
          if( trg == PAD_BUTTON_B )
          {
            hit = move_pos;
            decide_flag = TRUE;
          }
        }
      }
    }
    else
    {
      *(biw->cursor_mode) = 1;
    }
    BTLV_INPUT_PutCursorOBJ( biw, tp_tbl, key_tbl );
  }

  if( decide_flag == TRUE )
  {
    biw->cursor_pos = 0;
    biw->old_cursor_pos = CURSOR_NOMOVE;
    biw->cursor_decide = 1;
    GFL_CLACT_UNIT_SetDrawEnable( biw->cursor_clunit, FALSE );
  }

  return hit;
}

//--------------------------------------------------------------
/**
 * @brief   カーソルOBJ表示
 *
 * @param[in] biw     システム管理構造体のポインタ
 * @param[in] tp_tbl  タッチパネルテーブル（カーソルの表示位置決定に使用）
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_PutCursorOBJ( BTLV_INPUT_WORK* biw, const GFL_UI_TP_HITTBL* tp_tbl, const BTLV_INPUT_KEYTBL* key_tbl )
{
  int i;

  for( i = BTLV_INPUT_CURSOR_LU ; i < BTLV_INPUT_CURSOR_MAX ; i++ )
  {
    const BTLV_INPUT_KEYTBL*  key = &key_tbl[ biw->cursor_pos ];

    if( key->cur_pos[ i ] >= 0 )
    {
      const GFL_UI_TP_HITTBL*  hit = &tp_tbl[ key->cur_pos[ i ] ];
      GFL_CLACTPOS pos;

      switch( i ){
      case BTLV_INPUT_CURSOR_LU:
        pos.x = hit->rect.left;
        pos.y = hit->rect.top;
        break;
      case BTLV_INPUT_CURSOR_RU:
        pos.x = hit->rect.right;
        pos.y = hit->rect.top;
        break;
      case BTLV_INPUT_CURSOR_LD:
        pos.x = hit->rect.left;
        pos.y = hit->rect.bottom;
        break;
      case BTLV_INPUT_CURSOR_RD:
        pos.x = hit->rect.right;
        pos.y = hit->rect.bottom;
        break;
      case BTLV_INPUT_CURSOR_LD_2:
        pos.x = hit->rect.left;
        pos.y = hit->rect.bottom;
        break;
      case BTLV_INPUT_CURSOR_RD_2:
        pos.x = hit->rect.right;
        pos.y = hit->rect.bottom;
        break;
      }
      GFL_CLACT_WK_SetDrawEnable( biw->cursor[ i ].clwk, *(biw->cursor_mode) );
      GFL_CLACT_WK_SetPos( biw->cursor[ i ].clwk, &pos, CLSYS_DEFREND_SUB );
    }
    else
    {
      GFL_CLACT_WK_SetDrawEnable( biw->cursor[ i ].clwk, FALSE );
    }
  }
}

//=============================================================================================
//  決定音再生
//=============================================================================================
static  inline  void  SePlaySelect( void )
{
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//=============================================================================================
//  ローテーション選択音
//=============================================================================================
static  inline  void  SePlayRotateSelect( void )
{ 
  PMSND_PlaySE( SEQ_SE_ROTATION_S );
}

//=============================================================================================
//  ローテーション決定音
//=============================================================================================
static  inline  void  SePlayRotateDecide( void )
{ 
  PMSND_PlaySE( SEQ_SE_DECIDE2 );
}

//=============================================================================================
//  ローテーション音
//=============================================================================================
static  inline  void  SePlayRotation( void )
{ 
  PMSND_PlaySE( SEQ_SE_ROTATION_B );
}
