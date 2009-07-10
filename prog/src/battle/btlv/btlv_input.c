//============================================================================================
/**
 * @file	btlv_input.c
 * @brief	戦闘下画面
 * @author	soga
 * @date	2009.06.29
 */
//============================================================================================
#include <gflib.h>

#include "btlv_effect.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/wazatype_icon.h"
#include "waza_tool/waza_tool.h"
#include "system/palanm.h"
#include "system/bmp_oam.h"
#include "infowin/infowin.h"

#include "btlv_input.h"
#include "data/btlv_input.cdat"

#include "arc_def.h"
#include "battle/battgra_wb.naix"

#include "pm_define.h"

#include "sound/pm_sndsys.h"

#include "message.naix"
#include "msg/msg_btlv_input.h"

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================

#define BTLV_INPUT_TCB_MAX ( 8 )    //使用するTCBのMAX
#define BG_CLEAR_CODE     (0x8000 / 0x20 - 1)   ///BGスクリーンのクリアコード（キャラクタの一番後ろを指定）

#define INFOWIN_PAL_NO  ( 0x0f )  //情報ステータスバーパレット

#define WAZATYPEICON_OAMSIZE  ( 32 * 8 )

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
  BUTTON_ANIME_MAX = 6,     //3対3時の攻撃対象選択がマックスのはず

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

///技タイプアイコン：アクターヘッダ
static  const GFL_CLWK_DATA WazaTypeIconObjParam = {
  0, 0,   //x, y
  0, 100, 0,  //アニメ番号、優先順位、BGプライオリティ
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
};

ALIGN4  static  const u16 WazaIconPos[][2] = {  //0:X, 1:Y
  { WAZATYPE_X1, WAZATYPE_Y1 },
  { WAZATYPE_X2, WAZATYPE_Y2 },
  { WAZATYPE_X3, WAZATYPE_Y3 },
  { WAZATYPE_X4, WAZATYPE_Y4 },
};

//ボールゲージ座標定義
enum
{ 
  BTLV_INPUT_BALLGAUGE_ENEMY_X = ( 128 + ( 8 * 3 ) - 4 ),
  BTLV_INPUT_BALLGAUGE_ENEMY_Y = ( 6 * 8 ),
  BTLV_INPUT_BALLGAUGE_MINE_X = ( 128 - ( 16 * 3 ) + 8 ),
  BTLV_INPUT_BALLGAUGE_MINE_Y = ( 16 * 8 ) + 8,
};

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

typedef struct
{ 
  GFL_CLWK*   clwk;
  s16         pos_x;
  s16         pos_y;
}BTLV_INPUT_BALLGAUGE;

struct _BTLV_INPUT_WORK
{
  GFL_TCBSYS*           tcbsys;
  void*                 tcbwork;
  ARCHANDLE*            handle;
  BTLV_INPUT_TYPE       type;
  BTLV_INPUT_SCRTYPE    scr_type;
  u32                   tcb_execute_flag  :1;
  u32                   tcb_execute_count :3;
  u32                                     :28;

  //OBJリソース
  u32                   objcharID;
  u32                   objplttID;
  u32                   objcellID;

  //ボールゲージOBJ
  GFL_CLUNIT*           ballgauge_clunit;
  BTLV_INPUT_BALLGAUGE  ballgauge_mine[ TEMOTI_POKEMAX ];
  BTLV_INPUT_BALLGAUGE  ballgauge_enemy[ TEMOTI_POKEMAX ];

  //技タイプアイコンOBJ
  u32                   wazatype_charID[ PTL_WAZA_MAX ];
  u32                   wazatype_plttID;
  u32                   wazatype_cellID;
  GFL_CLUNIT*           wazatype_clunit;
  GFL_CLWK*             wazatype_wk[ PTL_WAZA_MAX ];

  //フォント
  GFL_FONT*             font;

  //BMP
  GFL_BMPWIN*           bmp_win;
  GFL_BMP_DATA*         bmp_data;

  //メインループTCB
  GFL_TCB*              main_loop;      //scdにメインループが存在しないのでBTLV_EFFECTのTCBを間借りしてメインを回す

	HEAPID                heapID;
};

typedef struct
{ 
  BTLV_INPUT_WORK*  biw;
  int               seq_no;
}TCB_TRANSFORM_WORK;

typedef struct
{ 
  BTLV_INPUT_WORK*  biw;
  fx32              start_scale;
  fx32              end_scale;
  fx32              scale_speed;
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
  const GFL_CLACTPOS  pos[ BUTTON_ANIME_MAX ];          //座標
  int                 anm_no[ BUTTON_ANIME_TYPE_MAX ];  //アニメーションナンバー
}BUTTON_ANIME_PARAM;

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================
static  void  BTLV_INPUT_LoadResource( BTLV_INPUT_WORK* biw );
static  void  TCB_TransformStandby2Command( GFL_TCB* tcb, void* work );
static  void  TCB_TransformCommand2Waza( GFL_TCB* tcb, void* work );
static  void  TCB_TransformWaza2Command( GFL_TCB* tcb, void* work );
static  void  TCB_TransformWaza2Dir( GFL_TCB* tcb, void* work );
static  void  TCB_TransformCommand2Standby( GFL_TCB* tcb, void* work );
static  void  TCB_TransformWaza2Standby( GFL_TCB* tcb, void* work );

static  void  SetupScaleChange( BTLV_INPUT_WORK* biw, fx32 start_scale, fx32 end_scale, fx32 scale_speed );
static  void  TCB_ScaleChange( GFL_TCB* tcb, void* work );
static  void  SetupScrollUp( BTLV_INPUT_WORK* biw, int scroll_x, int scroll_y, int scroll_speed, int scroll_count );
static  void  TCB_ScrollUp( GFL_TCB* tcb, void* work );
static  void  SetupScreenAnime( BTLV_INPUT_WORK* biw, int index, SCREEN_ANIME_DIR dir );
static  void  TCB_ScreenAnime( GFL_TCB* tcb, void* work );
static  void  SetupButtonAnime( BTLV_INPUT_WORK* biw, BUTTON_TYPE type, BUTTON_ANIME_TYPE anm_type );
static  void  TCB_ButtonAnime( GFL_TCB* tcb, void* work );

static  void	BTLV_INPUT_MainTCB( GFL_TCB* tcb, void* work );
static  void  FontLenGet( const STRBUF *str, GFL_FONT *font, int *ret_dot_len, int *ret_char_len );
static  void  BTLV_INPUT_CreateWazaScreen( BTLV_INPUT_WORK* biw, const BTLV_INPUT_WAZA_PARAM *biwp );
static  void  BTLV_INPUT_ClearWazaScreen( BTLV_INPUT_WORK* biw );
static  PRINTSYS_LSB  PP_FontColorGet( int pp, int pp_max );
static  void  BTLV_INPUT_CreateBallGauge( BTLV_INPUT_WORK* biw, const BTLV_INPUT_DIR_PARAM *bidp, int type );
static  void  BTLV_INPUT_DeleteBallGauge( BTLV_INPUT_WORK* biw );

//============================================================================================
/**
 *  @brief  システム初期化
 *
 *  @param[in]  type    インターフェースタイプ
 *  @param[in]  font    使用するフォント
 *  @param[in]  heapID  ヒープID
 *
 *  @retval システム管理構造体のポインタ
 */
//============================================================================================
BTLV_INPUT_WORK*  BTLV_INPUT_Init( BTLV_INPUT_TYPE type, GFL_FONT* font, HEAPID heapID )
{
	BTLV_INPUT_WORK *biw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_INPUT_WORK ) );

	biw->heapID = heapID;

  biw->handle   = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, biw->heapID );
  biw->tcbwork  = GFL_HEAP_AllocClearMemory( biw->heapID, GFL_TCB_CalcSystemWorkSize( BTLV_INPUT_TCB_MAX ) );
  biw->tcbsys   = GFL_TCB_Init( BTLV_INPUT_TCB_MAX, biw->tcbwork );

  biw->font = font;

  biw->wazatype_clunit = GFL_CLACT_UNIT_Create( PTL_WAZA_MAX, 0, biw->heapID );
  biw->ballgauge_clunit = GFL_CLACT_UNIT_Create( TEMOTI_POKEMAX * 2, 0, biw->heapID );

  BTLV_INPUT_SetFrame();
  BTLV_INPUT_LoadResource( biw );

  //ビットマップ初期化
  biw->bmp_win = GFL_BMPWIN_Create( GFL_BG_FRAME0_S, 32, 4, 32, 12, 0, GFL_BMP_CHRAREA_GET_B );
  biw->bmp_data = GFL_BMPWIN_GetBmp( biw->bmp_win );
  GFL_BMP_Clear( biw->bmp_data, 0x00 );
  GFL_BMPWIN_MakeScreen( biw->bmp_win );
  GFL_BMPWIN_TransVramCharacter( biw->bmp_win );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );

  //情報ステータスバー初期化
  INFOWIN_Init( GFL_BG_FRAME2_S, INFOWIN_PAL_NO, NULL, biw->heapID );

  //メインループはTCBで行う
  biw->main_loop = GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), BTLV_INPUT_MainTCB, biw, 0 );

  BTLV_INPUT_CreateScreen( biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );

	return biw;
}

//============================================================================================
/**
 *  @brief  システム終了
 *
 *  @param[in]  biw システム管理構造体のポインタ
 */
//============================================================================================
void	BTLV_INPUT_Exit( BTLV_INPUT_WORK* biw )
{
  GFL_CLGRP_CGR_Release( biw->objcharID );
  GFL_CLGRP_CELLANIM_Release( biw->objcellID );
  GFL_CLGRP_PLTT_Release( biw->objplttID );

  { 
    int i;

    for( i = 0 ; i < PTL_WAZA_MAX ; i++ )
    { 
      GFL_CLGRP_CGR_Release( biw->wazatype_charID[ i ] );
    }
  }
  GFL_CLGRP_CELLANIM_Release( biw->wazatype_cellID );
  GFL_CLGRP_PLTT_Release( biw->wazatype_plttID );

  GFL_CLACT_UNIT_Delete( biw->wazatype_clunit );
  GFL_CLACT_UNIT_Delete( biw->ballgauge_clunit );

  GFL_BMPWIN_Delete( biw->bmp_win );

  INFOWIN_Exit();

  GFL_TCB_Exit( biw->tcbsys );
  GFL_TCB_DeleteTask( biw->main_loop );

  GFL_HEAP_FreeMemory( biw->tcbwork );

  GFL_ARC_CloseDataHandle( biw->handle );

	GFL_HEAP_FreeMemory( biw );
}

//============================================================================================
/**
 *  @brief  システムメインループ
 *
 *  @param[in]  biw システム管理構造体のポインタ
 */
//============================================================================================
void	BTLV_INPUT_Main( BTLV_INPUT_WORK* biw )
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
static  void	BTLV_INPUT_MainTCB( GFL_TCB* tcb, void* work )
{
  BTLV_INPUT_WORK* biw = (BTLV_INPUT_WORK *)work;
  GFL_TCB_Main( biw->tcbsys );
  INFOWIN_Update();
}

//============================================================================================
/**
 *	@brief  下画面BGフレーム設定
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
 *	@brief  下画面BGフレーム設定解放
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
 *	@brief  下画面生成
 *
 *  @param[in]  biw   システム管理構造体のポインタ
 *	@param[in]  type  生成するスクリーンタイプ
 *	@param[in]  param 生成に必要なパラメータ構造体のポインタ
 */
//============================================================================================
void BTLV_INPUT_CreateScreen( BTLV_INPUT_WORK* biw, BTLV_INPUT_SCRTYPE type, void* param )
{
  BTLV_INPUT_ClearWazaScreen( biw );

  switch( type ){ 
  case BTLV_INPUT_SCRTYPE_STANDBY:
    if( biw->scr_type == BTLV_INPUT_SCRTYPE_STANDBY )
    { 
      MtxFx22 mtx;

      MTX_Scale22( &mtx, FX32_ONE * 3, FX32_ONE * 3 );
      GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, 128, &mtx, 256, 256 );
      GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, 128 + 40, &mtx, 256, 256 );

      GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
    }
    else
    { 
      TCB_TRANSFORM_WORK* ttw = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_TRANSFORM_WORK ) );
      biw->tcb_execute_flag = 1;
      ttw->biw = biw;

      BTLV_INPUT_DeleteBallGauge( biw );

      if( biw->scr_type == BTLV_INPUT_SCRTYPE_COMMAND )
      { 
        GFL_TCB_AddTask( biw->tcbsys, TCB_TransformCommand2Standby, ttw, 1 );
      }
      else
      {
        GFL_TCB_AddTask( biw->tcbsys, TCB_TransformWaza2Standby, ttw, 1 );
      }
    }
    break;
  case BTLV_INPUT_SCRTYPE_COMMAND:
    { 
      TCB_TRANSFORM_WORK* ttw = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_TRANSFORM_WORK ) );
      biw->tcb_execute_flag = 1;
      ttw->biw = biw;

      if( biw->scr_type == BTLV_INPUT_SCRTYPE_WAZA )
      { 
        GFL_TCB_AddTask( biw->tcbsys, TCB_TransformWaza2Command, ttw, 1 );
      }
      else
      {
        BTLV_INPUT_CreateBallGauge( biw, ( BTLV_INPUT_DIR_PARAM * )param, 0 );
        GFL_TCB_AddTask( biw->tcbsys, TCB_TransformStandby2Command, ttw, 1 );
      }
    }
    break;
  case BTLV_INPUT_SCRTYPE_WAZA:
    { 
      TCB_TRANSFORM_WORK* ttw = GFL_HEAP_AllocClearMemory( biw->heapID, sizeof( TCB_TRANSFORM_WORK ) );

      BTLV_INPUT_CreateWazaScreen( biw, ( const BTLV_INPUT_WAZA_PARAM * )param );
      biw->tcb_execute_flag = 1;
      ttw->biw = biw;
      GFL_TCB_AddTask( biw->tcbsys, TCB_TransformCommand2Waza, ttw, 1 );
    }
    break;
  case BTLV_INPUT_SCRTYPE_DIR:
    break;
  case BTLV_INPUT_SCRTYPE_YES_NO:
    break;
  case BTLV_INPUT_SCRTYPE_ROTATE:
    break;
  }
  biw->scr_type = type;
}

//============================================================================================
/**
 *	@brief  入力チェック
 *
 *  @param[in]  tp_tbl  タッチパネルテーブル
 */
//============================================================================================
int BTLV_INPUT_CheckInput( BTLV_INPUT_WORK* biw, const GFL_UI_TP_HITTBL* tp_tbl )
{ 
  //下画面変形中は入力を無視
  if( biw->tcb_execute_flag )
  { 
    return  GFL_UI_TP_HIT_NONE;
  }

  //本来はここでキー入力の方も処理したい

  return GFL_UI_TP_HitTrg( tp_tbl );
}

//============================================================================================
/**
 *	@brief  下画面リソースロード
 *
 *	@param[in]  biw  下画面管理構造体
 */
//============================================================================================
static  void  BTLV_INPUT_LoadResource( BTLV_INPUT_WORK* biw )
{ 
//	NARC_battgra_wb_battle_w_bg1b_NSCR = 18,
  GFL_ARCHDL_UTIL_TransVramBgCharacter( biw->handle, NARC_battgra_wb_battle_w_bg_NCGR,
                                        GFL_BG_FRAME0_S, 0, 0, FALSE, biw->heapID );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( biw->handle, NARC_battgra_wb_battle_w_bg3_NCGR,
                                        GFL_BG_FRAME3_S, 0, 0x8000, FALSE, biw->heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( biw->handle, NARC_battgra_wb_battle_w_bg0_NSCR,
                                   GFL_BG_FRAME0_S, 0, 0, FALSE, biw->heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( biw->handle, NARC_battgra_wb_battle_w_bg1a_NSCR,
                                   GFL_BG_FRAME1_S, 0, 0, FALSE, biw->heapID );
//  GFL_ARCHDL_UTIL_TransVramScreen( biw->handle, NARC_battgra_wb_battle_w_bg2_NSCR,
 //                                  GFL_BG_FRAME2_S, 0, 0, FALSE, biw->heapID );
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
                           GFL_CLGRP_PLTT_GetAddr( biw->objplttID, CLSYS_DRAW_SUB ) / 2, 0x20 * 3 );

  {
    ARCHANDLE*  hdl;
    int         i;

    hdl = GFL_ARC_OpenDataHandle( WazaTypeIcon_ArcIDGet(), biw->heapID );
    biw->wazatype_cellID = GFL_CLGRP_CELLANIM_Register( hdl, WazaTypeIcon_CellIDGet(), WazaTypeIcon_CellAnmIDGet(),
                                                        biw->heapID );
    biw->wazatype_plttID = GFL_CLGRP_PLTT_Register( hdl, WazaTypeIcon_PlttIDGet(), CLSYS_DRAW_SUB, 0x20 * 3, biw->heapID );
    PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_SUB_OBJ,
                             GFL_CLGRP_PLTT_GetAddr( biw->wazatype_plttID, CLSYS_DRAW_SUB ) / 2, 0x20 * 3 );
    for( i = 0; i < PTL_WAZA_MAX ; i++ ){
      biw->wazatype_charID[ i ] = GFL_CLGRP_CGR_Register( hdl, WazaTypeIcon_CgrIDGet( POKETYPE_NORMAL ), FALSE,
                                                          CLSYS_DRAW_SUB, biw->heapID );
    }
    GFL_ARC_CloseDataHandle( hdl );
  }
}

//============================================================================================
/**
 *	@brief  下画面変形タスク（待機→コマンド選択）
 */
//============================================================================================
static  void  TCB_TransformStandby2Command( GFL_TCB* tcb, void* work )
{ 
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){ 
  case 0:
    PMSND_PlaySE( SEQ_SE_OPEN2 );
    GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, TTS2C_FRAME1_SCROLL_X );
    GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, TTS2C_FRAME1_SCROLL_Y );
    SetupScaleChange( ttw->biw, TTS2C_START_SCALE, TTS2C_END_SCALE, -TTS2C_SCALE_SPEED );
    SetupScrollUp( ttw->biw, TTS2C_START_SCROLL_X, TTS2C_START_SCROLL_Y, TTS2C_SCROLL_SPEED, TTS2C_SCROLL_COUNT );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
    ttw->seq_no++;
    break;
  case 1:
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
 *	@brief  下画面変形タスク（コマンド選択→技選択）
 */
//============================================================================================
static  void  TCB_TransformCommand2Waza( GFL_TCB* tcb, void* work )
{ 
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){ 
  case 0:
    SetupScrollUp( ttw->biw, TTC2W_START_SCROLL_X, TTC2W_START_SCROLL_Y, TTC2W_SCROLL_SPEED, TTC2W_SCROLL_COUNT );
    SetupScreenAnime( ttw->biw, 0, SCREEN_ANIME_DIR_FORWARD );
    SetupButtonAnime( ttw->biw, BUTTON_TYPE_WAZA, BUTTON_ANIME_TYPE_APPEAR );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
    ttw->seq_no++;
    break;
  case 1:
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
 *	@brief  下画面変形タスク（技選択→コマンド選択）
 */
//============================================================================================
static  void  TCB_TransformWaza2Command( GFL_TCB* tcb, void* work )
{ 
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

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
    if( ttw->biw->tcb_execute_count == 0 )
    { 
      ttw->biw->tcb_execute_flag = 0;
      GFL_HEAP_FreeMemory( ttw );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

static  void  TCB_TransformWaza2Dir( GFL_TCB* tcb, void* work )
{ 

}

//============================================================================================
/**
 *	@brief  下画面変形タスク（コマンド選択→スタンバイ）
 */
//============================================================================================
static  void  TCB_TransformCommand2Standby( GFL_TCB* tcb, void* work )
{ 
  TCB_TRANSFORM_WORK* ttw = (TCB_TRANSFORM_WORK *)work;

  switch( ttw->seq_no ){ 
  case 0:
    SetupScaleChange( ttw->biw, TTC2S_START_SCALE, TTC2S_END_SCALE, TTC2S_SCALE_SPEED );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
    ttw->seq_no++;
    break;
  case 1:
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
 *	@brief  下画面変形タスク（技選択→スタンバイ）
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
      SetupScaleChange( ttw->biw, TTC2S_START_SCALE, TTC2S_END_SCALE, TTC2S_SCALE_SPEED );
      GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
      ttw->seq_no++;
    }
    break;
  case 2:
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
 *	@brief  スケール変更処理セットアップ
 *
 *  @param[in]  biw           システム管理構造体のポインタ
 *  @param[in]  start_scale   スケール初期値
 *  @param[in]  end_scale     最終的なスケール値
 *  @param[in]  scale_speed   スケール変更速度
 */
//============================================================================================
static  void  SetupScaleChange( BTLV_INPUT_WORK* biw, fx32 start_scale, fx32 end_scale, fx32 scale_speed )
{ 
  TCB_SCALE_UP* tsu = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_SCALE_UP ) );

  tsu->biw          = biw;
  tsu->start_scale  = start_scale;
  tsu->end_scale    = end_scale;
  tsu->scale_speed  = scale_speed;

  GFL_TCB_AddTask( biw->tcbsys, TCB_ScaleChange, tsu, 0 );

  biw->tcb_execute_count++;
}

//============================================================================================
/**
 *	@brief  スケール変更処理タスク
 */
//============================================================================================
static  void  TCB_ScaleChange( GFL_TCB* tcb, void* work )
{ 
  TCB_SCALE_UP* tsu = ( TCB_SCALE_UP * )work;
  MtxFx22 mtx;

  tsu->start_scale += tsu->scale_speed;

  MTX_Scale22( &mtx, tsu->start_scale, tsu->start_scale );
  GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, 128, &mtx, 256, 256 );
  GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, 128 + 40, &mtx, 256, 256 );
  if( tsu->start_scale == tsu->end_scale )
  { 
    tsu->biw->tcb_execute_count--;
    GFL_HEAP_FreeMemory( tsu );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 *	@brief  スクリーンスクロール処理セットアップ
 *
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
 *	@brief  スクリーンスクロール処理タスク
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
 *	@brief  スクリーンアニメ処理セットアップ
 *
 *	@param[in]  index 読み込むスクリーンリソースインデックス
 *	@param[in]  dir   アニメーションの向き
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
 *	@brief  スクリーンアニメ処理タスク
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
 *	@brief  ボタンアニメ処理セットアップ
 *
 *	@param[in]  type      ボタンタイプ
 *	@param[in]  anm_type  ボタンアニメタイプ
 */
//============================================================================================
static  void  SetupButtonAnime( BTLV_INPUT_WORK* biw, BUTTON_TYPE type, BUTTON_ANIME_TYPE anm_type )
{ 
  TCB_BUTTON_ANIME* tba = GFL_HEAP_AllocMemory( biw->heapID, sizeof( TCB_BUTTON_ANIME ) );
  static const GFL_CLWK_DATA button = {
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
 *	@brief  ボタンアニメ処理タスク
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
  STRBUF *wazaname_p;
  STRBUF *wazaname_src;
  STRBUF *pp_p;
  STRBUF *pp_src;
  STRBUF *ppmsg_src;
  WORDSET *wordset;
  PRINTSYS_LSB color;
  GFL_MSGDATA *msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_btlv_input_dat, biw->heapID );
  static  const int wazaname_pos[ PTL_WAZA_MAX ][ 2 ] = 
  { 
    { WAZANAME_X1, WAZANAME_Y1 },
    { WAZANAME_X2, WAZANAME_Y2 },
    { WAZANAME_X3, WAZANAME_Y3 },
    { WAZANAME_X4, WAZANAME_Y4 },
  };
  static  const int ppmsg_pos[ PTL_WAZA_MAX ][ 2 ] = 
  { 
    { PPMSG_X1, PPMSG_Y1 },
    { PPMSG_X2, PPMSG_Y2 },
    { PPMSG_X3, PPMSG_Y3 },
    { PPMSG_X4, PPMSG_Y4 },
  };
  static  const int pp_pos[ PTL_WAZA_MAX ][ 2 ] = 
  { 
    { PP_X1, PP_Y1 },
    { PP_X2, PP_Y2 },
    { PP_X3, PP_Y3 },
    { PP_X4, PP_Y4 },
  };

  wazaname_p = GFL_STR_CreateBuffer( BUFLEN_WAZA_NAME, biw->heapID );
  wazaname_src = GFL_MSG_CreateString( msg, BI_WazaNameMsg );
  ppmsg_src = GFL_MSG_CreateString( msg, BI_PPMsg );
  wordset = WORDSET_Create( biw->heapID );
  pp_p = GFL_STR_CreateBuffer( BUFLEN_BI_WAZAPP, biw->heapID );
  pp_src = GFL_MSG_CreateString( msg,  BI_PPNowMaxMsg );

  GFL_CLACT_UNIT_SetDrawEnable( biw->wazatype_clunit, FALSE );

  for(i = 0; i < PTL_WAZA_MAX; i++){
    if( biwp->wazano[ i ] ){
      //技タイプアイコン
      { 
        void *arc_data;
        void *obj_vram;
        NNSG2dImageProxy image;
        NNSG2dCharacterData *char_data;
        int waza_type;
        GFL_CLWK_DATA obj_param = WazaTypeIconObjParam;

        waza_type = WT_WazaDataParaGet( biwp->wazano[ i ], ID_WTD_wazatype );

        obj_param.pos_x = WazaIconPos[ i ][ 0 ];
        obj_param.pos_y = WazaIconPos[ i ][ 1 ];
        biw->wazatype_wk[ i ] = GFL_CLACT_WK_Create( biw->wazatype_clunit, biw->wazatype_charID[ i ],
                                                     biw->wazatype_plttID, biw->wazatype_cellID,
                                                     &obj_param, CLSYS_DEFREND_SUB, biw->heapID );

        arc_data = GFL_ARC_UTIL_LoadOBJCharacter( WazaTypeIcon_ArcIDGet(),
                                                  WazaTypeIcon_CgrIDGet( waza_type ), WAZATYPEICON_COMP_CHAR,
                                                  &char_data, biw->heapID );

        GFL_CLACT_WK_SetPlttOffs( biw->wazatype_wk[ i ], WazaTypeIcon_PlttOffsetGet( waza_type ), CLWK_PLTTOFFS_MODE_PLTT_TOP );
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
    }
  }

  WORDSET_Delete( wordset );
  GFL_STR_DeleteBuffer( wazaname_p );
  GFL_STR_DeleteBuffer(ppmsg_src);
  GFL_STR_DeleteBuffer(wazaname_src);
  GFL_STR_DeleteBuffer(pp_src);
  GFL_STR_DeleteBuffer(pp_p);

  GFL_MSG_Delete( msg );
}

//--------------------------------------------------------------
/**
 * @brief   技選択画面クリア
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_ClearWazaScreen( BTLV_INPUT_WORK* biw )
{ 
  int i;

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
  if(pp == 0){
    return MSGCOLOR_PP_RED;
  }
  if(pp_max == pp){
    return MSGCOLOR_PP_WHITE;
  }
  if(pp_max <= 2){
    if(pp == 1){
      return MSGCOLOR_PP_ORANGE;
    }
  }
  else if(pp_max <= 7){
    switch(pp){
    case 1:
      return MSGCOLOR_PP_ORANGE;
    case 2:
      return MSGCOLOR_PP_YELLOW;
    }
  }
  else{
    if(pp <= pp_max / 4){
      return MSGCOLOR_PP_ORANGE;
    }
    if(pp <= pp_max / 2){
      return MSGCOLOR_PP_YELLOW;
    }
  }
  return MSGCOLOR_PP_WHITE;
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
static  void  BTLV_INPUT_CreateBallGauge( BTLV_INPUT_WORK* biw, const BTLV_INPUT_DIR_PARAM *bidp, int type )
{ 
  BTLV_INPUT_BALLGAUGE* bib;
  int                   i;
  GFL_CLACTPOS          pos;
  int                   pos_ofs;
  int                   anm_ofs;
  static const GFL_CLWK_DATA ballgauge = {
    0, 0,     //x, y
    0, 0, 0,  //アニメ番号、優先順位、BGプライオリティ
  };

  if( type )
  { 
    bib = biw->ballgauge_enemy;
    pos.x = BTLV_INPUT_BALLGAUGE_ENEMY_X;
    pos.y = BTLV_INPUT_BALLGAUGE_ENEMY_Y;
    pos_ofs = -8;
    anm_ofs = 4;
  }
  else
  { 
    bib = biw->ballgauge_mine;
    pos.x = BTLV_INPUT_BALLGAUGE_MINE_X;
    pos.y = BTLV_INPUT_BALLGAUGE_MINE_Y;
    pos_ofs = 16;
    anm_ofs = 0;
  }

  for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
  { 
    GF_ASSERT( bib[ i ].clwk == NULL );
    bib[ i ].clwk = GFL_CLACT_WK_Create( biw->ballgauge_clunit, biw->objcharID, biw->objplttID, biw->objcellID,
                                          &ballgauge, CLSYS_DEFREND_SUB, biw->heapID );
    GFL_CLACT_WK_SetPos( bib[ i ].clwk, &pos, CLSYS_DEFREND_SUB );
    GFL_CLACT_WK_SetAutoAnmFlag( bib[ i ].clwk, TRUE );
    GFL_CLACT_WK_SetAnmSeq( bib[ i ].clwk, bidp[ i ].status + anm_ofs );
    pos.x += pos_ofs;
  }
}

//--------------------------------------------------------------
/**
 * @brief   ボールゲージ削除
 *
 * @param[in] biw   システム管理構造体のポインタ
 * @param[in] bidp  ボールゲージ生成用パラメータのポインタ
 * @param[in] type  ボールゲージタイプ
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_DeleteBallGauge( BTLV_INPUT_WORK* biw )
{ 
  BTLV_INPUT_BALLGAUGE* bib;
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
