
//============================================================================================
/**
 * @file  btlv_mcss.c
 * @brief 戦闘MCSS管理
 * @author  soga
 * @date  2008.11.14
 */
//============================================================================================

#include <gflib.h>

#include "system/gfl_use.h"
#include "system/mcss_tool.h"
#include "sound/pm_voice.h"
#include "sound/pm_wb_voice.h"

#include "btlv_effect.h"

#include "battle/battgra_wb.naix"
#include "pokegra/pokegra_wb.naix"

#include "data/patch.cdat"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================

#define BTLV_MCSS_MAX ( BTLV_MCSS_POS_TOTAL ) //BTLV_MCSSで管理するMCSSのMAX値

#define BTLV_MCSS_DEFAULT_SHIFT ( FX32_SHIFT - 4 )          //ポリゴン１辺の基準の長さにするシフト値
#define BTLV_MCSS_DEFAULT_LINE  ( 1 << BTLV_MCSS_DEFAULT_SHIFT )  //ポリゴン１辺の基準の長さ

#define BTLV_MCSS_ORTHO_SCALE_MINE  ( FX32_ONE * 16 * 2 )
#define BTLV_MCSS_ORTHO_SCALE_ENEMY ( FX32_ONE * 16 * 1 )

#define BTLV_MCSS_STOP_ANIME_TIME ( 64 )

#define BTLV_MCSS_NO_INDEX  ( -1 )

enum{
  REVERSE_FLAG_OFF = 0,
  REVERSE_FLAG_ON,

  ANIME_STOP_FLAG = 0x80000000,
  ANIME_START_FLAG = 0x00000000,

  SEQ_ANIME_STANDBY = 0,
  SEQ_ANIME_STOP,
};

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

struct  _BTLV_MCSS
{ 
  MCSS_WORK*      mcss;
  MCSS_ADD_WORK   maw;
  GFL_TCB*        tcb;
  int             mons_no;
  int             form_no;
  u32             personal_rnd;
  u32             status_flag;
  u16             weight;
  BtlvMcssPos     position;
};

struct _BTLV_MCSS_WORK
{
  GFL_TCBSYS*     tcb_sys;
  MCSS_SYS_WORK*  mcss_sys;
  BTLV_MCSS       btlv_mcss[ BTLV_MCSS_MAX ];

  u32             mcss_pos_3vs3             :1;
  u32             mcss_pos_rotate           :1;
  u32             mcss_proj_mode            :1;
  u32             mcss_tcb_rotation_execute :1;
  u32                                       :28;
  u32             mcss_tcb_move_execute;
  u32             mcss_tcb_scale_execute;
  u32             mcss_tcb_rotate_execute;

  u32             mcss_tcb_blink_execute;
  u32             mcss_tcb_alpha_execute;
  u32             mcss_tcb_mosaic_execute;

  HEAPID          heapID;
};

typedef struct
{
  BTLV_MCSS_WORK*   bmw;
  int               position;
  VecFx32           now_value;
  EFFTOOL_MOVE_WORK emw;
}BTLV_MCSS_TCB_WORK;

typedef struct
{ 
  int seq_no;
  int position;
  int wait;
}BTLV_MCSS_STOP_ANIME;

typedef struct
{ 
  BTLV_MCSS_WORK*   bmw;
  int               seq_no;
  int               side;
  int               dir;
}BTLV_MCSS_ROTATION_WORK;

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================

static  void  BTLV_MCSS_MakeMAW( const POKEMON_PARAM *pp, MCSS_ADD_WORK* maw, int position );
static  void  BTLV_MCSS_MakeMAWTrainer( int tr_type, MCSS_ADD_WORK* maw, int position );
static  void  BTLV_MCSS_SetDefaultScale( BTLV_MCSS_WORK *bmw, int position );

static  void  BTLV_MCSS_TCBInitialize( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *start, VecFx32 *end,
                                       int frame, int wait, int count, GFL_TCB_FUNC *func, int reverse_flag );
static  void  TCB_BTLV_MCSS_Move( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Scale( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Rotate( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Blink( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Alpha( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_MoveCircle( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Mosaic( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_StopAnime( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Rotation( GFL_TCB *tcb, void *work );

static  void  BTLV_MCSS_CallBackFunctorFrame( u32 data, fx32 currentFrame );
static  BOOL  BTLV_MCSS_CallBackNodes( u32 data, const NNSG2dMultiCellHierarchyData* pNodeData,
                                       NNSG2dCellAnimation* pCellAnim, u16 nodeIdx );

static  void  BTLV_MCSS_GetDefaultPos( BTLV_MCSS_WORK *bmw, VecFx32 *pos, BtlvMcssPos position );
static  fx32  BTLV_MCSS_GetDefaultScale( BTLV_MCSS_WORK* bmw, BtlvMcssPos position, BTLV_MCSS_PROJECTION proj );

static  BOOL  BTLV_MCSS_MakeBuchi( const MCSS_ADD_WORK* maw, TCB_LOADRESOURCE_WORK* tlw, void* work );

static  int   BTLV_MCSS_GetIndex( BTLV_MCSS_WORK* bmw, BtlvMcssPos position );

#ifdef PM_DEBUG
void  BTLV_MCSS_AddDebug( BTLV_MCSS_WORK *bmw, const MCSS_ADD_DEBUG_WORK *madw, int position );
#endif

//============================================================================================
/**
 *  ポケモンの立ち位置テーブル
 */
//============================================================================================
static  const VecFx32 poke_pos_single_table[]={
  { 0x00000800, 0x00000666, 0x00007000 }, //POS_AA
  { 0x000004cd, 0x00000666, 0xffff6000 }, //POS_BB
};

static  const VecFx32 poke_pos_double_table[]={
  { 0xffffe500, 0x00000666, 0x00007000 }, //POS_A
  { 0x000026cd, 0x00000666, 0xffff5a00 }, //POS_B
  { 0x00002800, 0x00000666, 0x00007000 }, //POS_C
  { 0xffffdccd, 0x00000666, 0xffff4700 }, //POS_D
};

static  const VecFx32 poke_pos_triple_table[]={
  { 0xffffc000, 0x00000666, 0x00007000 }, //POS_A
  { 0x00004800, 0x00000666, 0xffff4000 }, //POS_B
  { 0x00000c00, 0x00000666, 0x00004c00 }, //POS_C
  { 0x00000bcd, 0x00000666, 0xffff7000 }, //POS_D
  { 0x00005300, 0x00000666, 0x00006300 }, //POS_E
  { 0xffffb9cd, 0x00000666, 0xffff4000 }, //POS_F
};

static  const VecFx32 poke_pos_rotate_table[]={
  { 0xffffe500, 0x00000666, 0x00007000 }, //POS_A
  { 0x000026cd, 0x00000666, 0xffff5a00 }, //POS_B
  { 0x00002800, 0x00000666, 0x00007000 }, //POS_C
  { 0xffffdccd, 0x00000666, 0xffff4700 }, //POS_D
  { 0x00000800, 0x00000666, 0x0000a000 }, //POS_E
  { 0x000004cd, 0x00000666, 0xffff3000 }, //POS_F
};

static  const VecFx32 trainer_pos_table[]={
  { FX_F32_TO_FX32( -2.5f + 3.000f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(   7.5f - 0.5f ) },    //POS_TR_AA
  { FX_F32_TO_FX32(  4.5f - 4.200f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( -25.0f ) },           //POS_TR_BB
  { FX_F32_TO_FX32( -3.5f + 3.500f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(   8.5f ) },           //POS_TR_A
  { FX_F32_TO_FX32(  6.0f - 4.200f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(  -9.0f ) },           //POS_TR_B
  { FX_F32_TO_FX32( -0.5f + 3.845f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(   9.0f ) },           //POS_TR_C
  { FX_F32_TO_FX32(  2.0f - 4.964f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( -11.0f ) },           //POS_TR_D
};

//============================================================================================
/**
 *  ポケモンの立ち位置によるスケール補正テーブル
 */
//============================================================================================
static  const fx32 poke_scale_single_table[]={
  0x1030,   //POS_AA
  0x119b,   //POS_BB
};

static  const fx32 poke_scale_double_table[]={
  0x000010f0, //POS_A
  0x00001205, //POS_B
  0x00000f00, //POS_C
  0x00001320, //POS_D
};

static  const fx32 poke_scale_triple_table[]={
  0x00001200, //POS_A
  0x00001200, //POS_B
  0x00001270, //POS_C
  0x000011b0, //POS_D
  0x00000ed4, //POS_E
  0x00001400, //POS_F
};

static  const fx32 poke_scale_rotate_table[]={
  0x000010f0, //POS_A
  0x00001205, //POS_B
  0x00000f00, //POS_C
  0x00001320, //POS_D
  0x00000e00, //POS_E
  0x000010f0, //POS_F
};

static  const fx32 trainer_scale_table[]={
  0x1030,   //POS_TR_AA
  0x119b,   //POS_TR_BB
  0x0f00,   //POS_TR_A
  0x10e0,   //POS_TR_B
  0x0d00,   //POS_TR_C
  0x1320,   //POS_TR_D
};

//============================================================================================
/**
 *  ポケモンの立ち位置による鳴き声パンテーブル
 */
//============================================================================================
static  const int pokevoice_pan_single_table[]={
   20, //POS_AA
  107, //POS_BB
};

static  const int pokevoice_pan_double_table[]={
   12, //POS_A
  115, //POS_B
   28, //POS_C
   99, //POS_D
};

static  const int pokevoice_pan_triple_table[]={
    0, //POS_A
  127, //POS_B
   20, //POS_C
  107, //POS_D
   40, //POS_E
   87, //POS_F
};

static  const int pokevoice_pan_rotate_table[]={
   12, //POS_A
  115, //POS_B
   28, //POS_C
   99, //POS_D
   20, //POS_E
  107, //POS_F
};

//============================================================================================
/**
 * @brief システム初期化
 *
 * @param[in] tcb_sys システム内で使用するTCBSYS構造体へのポインタ
 * @param[in] heapID  ヒープID
 */
//============================================================================================
BTLV_MCSS_WORK  *BTLV_MCSS_Init( BtlRule rule, GFL_TCBSYS *tcb_sys, HEAPID heapID )
{
  BTLV_MCSS_WORK *bmw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_MCSS_WORK ) );

  bmw->mcss_sys = MCSS_Init( BTLV_MCSS_MAX, heapID );
  bmw->tcb_sys  = tcb_sys;

  bmw->mcss_pos_3vs3 = ( rule == BTL_RULE_TRIPLE ) ? 1 : 0;
  bmw->mcss_pos_rotate = ( rule == BTL_RULE_ROTATION ) ? 1 : 0;

  return bmw;
}

//============================================================================================
/**
 * @brief システム終了
 *
 * @param[in] bmw BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_Exit( BTLV_MCSS_WORK *bmw )
{
  MCSS_Exit( bmw->mcss_sys );
  GFL_HEAP_FreeMemory( bmw );
}

//============================================================================================
/**
 * @brief システムメイン
 *
 * @param[in] bmw BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_Main( BTLV_MCSS_WORK *bmw )
{
  MCSS_Main( bmw->mcss_sys );
  //まばたき
  { 
    BtlvMcssPos pos;

    for( pos = BTLV_MCSS_POS_AA ; pos < BTLV_MCSS_POS_MAX ; pos++ )
    { 
      if( ( BTLV_MCSS_CheckExist( bmw, pos ) ) && ( GFL_STD_MtRand( 100 ) == 0 ) )
      { 
        if( ( bmw->mcss_tcb_blink_execute & BTLV_EFFTOOL_Pos2Bit( pos ) ) == 0 )
        { 
          BTLV_MCSS_MoveBlink( bmw, pos, BTLEFF_MEPACHI_MABATAKI, 4, 1 );
        }
      }
    }
  }
}

//============================================================================================
/**
 * @brief 描画
 *
 * @param[in] bmw BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_Draw( BTLV_MCSS_WORK *bmw )
{
  MCSS_Draw( bmw->mcss_sys );
}

//============================================================================================
/**
 * @brief BTLV_MCSS追加
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] pp      POKEMON_PARAM構造体へのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_Add( BTLV_MCSS_WORK *bmw, const POKEMON_PARAM *pp, int position )
{
  VecFx32 pos;
  int index;

  for( index = 0 ; index < BTLV_MCSS_POS_TOTAL ; index++ )
  { 
    if( bmw->btlv_mcss[ index ].mcss == NULL )
    { 
      break;
    }
  }

  GF_ASSERT( position < BTLV_MCSS_POS_TOTAL );
  GF_ASSERT( index < BTLV_MCSS_POS_TOTAL );
  GF_ASSERT_MSG( BTLV_MCSS_GetIndex( bmw, position ) == BTLV_MCSS_NO_INDEX, "pos=%d", position );

  //立ち位置を保存
  bmw->btlv_mcss[ index ].position = position;

  //個性乱数を取得しておく
  bmw->btlv_mcss[ index ].personal_rnd = PP_Get( pp, ID_PARA_personal_rnd, NULL );

  BTLV_MCSS_MakeMAW( pp, &bmw->btlv_mcss[ index ].maw, position );
  BTLV_MCSS_GetDefaultPos( bmw, &pos, position );
  bmw->btlv_mcss[ index ].mcss = MCSS_Add( bmw->mcss_sys, pos.x, pos.y, pos.z, &bmw->btlv_mcss[ index ].maw );

  //ポケモンのナンバー、フォルム、体重データを取得しておく
  { 
    bmw->btlv_mcss[ index ].mons_no = PP_Get( pp, ID_PARA_monsno, NULL );
    bmw->btlv_mcss[ index ].form_no = PP_Get( pp, ID_PARA_form_no, NULL );
    bmw->btlv_mcss[ index ].weight = POKETOOL_GetPersonalParam( bmw->btlv_mcss[ index ].mons_no,
                                                                bmw->btlv_mcss[ index ].form_no, POKEPER_ID_weight );
  }

  BTLV_MCSS_SetDefaultScale( bmw, position );

  MCSS_SetAnimCtrlCallBack( bmw->btlv_mcss[ index ].mcss, position, BTLV_MCSS_CallBackFunctorFrame, 1 );

  MCSS_SetCallBackFunc( bmw->mcss_sys, BTLV_MCSS_MakeBuchi, &bmw->btlv_mcss[ index ].personal_rnd );
}

//============================================================================================
/**
 * @brief BTLV_MCSS追加
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] pp      POKEMON_PARAM構造体へのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_AddTrainer( BTLV_MCSS_WORK *bmw, int tr_type, int position )
{
  MCSS_ADD_WORK maw;
  VecFx32 pos;
  int index;

  for( index = 0 ; index < BTLV_MCSS_POS_TOTAL ; index++ )
  { 
    if( bmw->btlv_mcss[ index ].mcss == NULL )
    { 
      break;
    }
  }

  GF_ASSERT( position < BTLV_MCSS_POS_TOTAL );
  GF_ASSERT( index < BTLV_MCSS_POS_TOTAL );
  GF_ASSERT_MSG( BTLV_MCSS_GetIndex( bmw, position ) == BTLV_MCSS_NO_INDEX, "pos=%d", position );

  BTLV_MCSS_MakeMAWTrainer( tr_type, &maw, position );
  BTLV_MCSS_GetDefaultPos( bmw, &pos, position );
  bmw->btlv_mcss[ index ].mcss = MCSS_Add( bmw->mcss_sys, pos.x, pos.y, pos.z, &maw );

  BTLV_MCSS_SetDefaultScale( bmw, position );
}

//============================================================================================
/**
 * @brief BTLV_MCSS削除
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_Del( BTLV_MCSS_WORK *bmw, int position )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );

  MCSS_Del( bmw->mcss_sys, bmw->btlv_mcss[ index ].mcss );
  bmw->btlv_mcss[ index ].mcss = NULL;
  if( bmw->btlv_mcss[ index ].tcb )
  { 
    GFL_HEAP_FreeMemory( GFL_TCB_GetWork( bmw->btlv_mcss[ index ].tcb ) );
    GFL_TCB_DeleteTask( bmw->btlv_mcss[ index ].tcb );
    bmw->btlv_mcss[ index ].tcb = NULL;
  }
}

//============================================================================================
/**
 * @brief 座標セット
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  座標セットするMCSSの立ち位置
 * @param[in] pos_x     X座標
 * @param[in] pos_y     Y座標
 * @param[in] pos_z     Z座標
 */
//============================================================================================
void  BTLV_MCSS_SetPosition( BTLV_MCSS_WORK *bmw, int position, fx32 pos_x, fx32 pos_y, fx32 pos_z )
{
  VecFx32 pos;
  int index = BTLV_MCSS_GetIndex( bmw, position );

  pos.x = pos_x;
  pos.y = pos_y;
  pos.z = pos_z;

  MCSS_SetPosition( bmw->btlv_mcss[ index ].mcss, &pos );
}

//============================================================================================
/**
 * @brief 正射影描画モードON
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_SetOrthoMode( BTLV_MCSS_WORK *bmw )
{
  int position;

  MCSS_SetOrthoMode( bmw->mcss_sys );

  bmw->mcss_proj_mode = BTLV_MCSS_PROJ_ORTHO;

  for( position = 0 ; position < BTLV_MCSS_POS_TOTAL ; position++ )
  {
    if( BTLV_MCSS_GetIndex( bmw, position ) != BTLV_MCSS_NO_INDEX )
    {
      BTLV_MCSS_SetDefaultScale( bmw, position );
    }
  }
}

//============================================================================================
/**
 * @brief 正射影描画モードOFF
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_ResetOrthoMode( BTLV_MCSS_WORK *bmw )
{
  int position;

  MCSS_ResetOrthoMode( bmw->mcss_sys );

  bmw->mcss_proj_mode = BTLV_MCSS_PROJ_PERSPECTIVE;

  for( position = 0 ; position < BTLV_MCSS_POS_TOTAL ; position++ )
  {
    if( BTLV_MCSS_GetIndex( bmw, position ) != BTLV_MCSS_NO_INDEX )
    {
      BTLV_MCSS_SetDefaultScale( bmw, position );
    }
  }
}

//============================================================================================
/**
 * @brief メパチ処理
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  メパチさせたいポケモンの立ち位置
 * @param[in] flag    メパチフラグ（BTLV_MCSS_MEPACHI_ON、BTLV_MCSS_MEPACHI_OFF）
 */
//============================================================================================
void  BTLV_MCSS_SetMepachiFlag( BTLV_MCSS_WORK *bmw, int position, int flag )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  if( flag == BTLV_MCSS_MEPACHI_FLIP ){
    MCSS_FlipMepachiFlag( bmw->btlv_mcss[ index ].mcss );
  }
  else if( flag == BTLV_MCSS_MEPACHI_ON ){
    MCSS_SetMepachiFlag( bmw->btlv_mcss[ index ].mcss );
  }
  else{
    MCSS_ResetMepachiFlag( bmw->btlv_mcss[ index ].mcss );
  }
}

//============================================================================================
/**
 * @brief アニメストップ処理
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  アニメストップさせたいポケモンの立ち位置
 * @param[in] flag    アニメストップフラグ（BTLV_MCSS_ANM_STOP_ON、BTLV_MCSS_ANM_STOP_OFF）
 */
//============================================================================================
void  BTLV_MCSS_SetAnmStopFlag( BTLV_MCSS_WORK *bmw, int position, int flag )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  if( flag == BTLV_MCSS_ANM_STOP_ON ){
    MCSS_SetAnmStopFlag( bmw->btlv_mcss[ index ].mcss );
  }
  else{
    MCSS_ResetAnmStopFlag( bmw->btlv_mcss[ index ].mcss );
  }
}

//============================================================================================
/**
 * @brief バニッシュフラグ取得
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  フラグ操作させたいポケモンの立ち位置
 */
//============================================================================================
int   BTLV_MCSS_GetVanishFlag( BTLV_MCSS_WORK *bmw, int position )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  return MCSS_GetVanishFlag( bmw->btlv_mcss[ index ].mcss );
}

//============================================================================================
/**
 * @brief バニッシュ処理
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  フラグ操作させたいポケモンの立ち位置
 * @param[in] flag    バニッシュフラグ（BTLV_MCSS_VANISH_ON、BTLV_MCSS_VANISH_OFF）
 */
//============================================================================================
void  BTLV_MCSS_SetVanishFlag( BTLV_MCSS_WORK *bmw, int position, int flag )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  if( flag == BTLV_MCSS_VANISH_FLIP ){
    MCSS_FlipVanishFlag( bmw->btlv_mcss[ index ].mcss );
  }
  else if( flag == BTLV_MCSS_VANISH_ON ){
    MCSS_SetVanishFlag( bmw->btlv_mcss[ index ].mcss );
  }
  else{
    MCSS_ResetVanishFlag( bmw->btlv_mcss[ index ].mcss );
  }
}

//============================================================================================
/**
 * @brief ポケモンの初期立ち位置を取得
 *
 * @param[out]  pos     初期立ち位置を格納するワークへのポインタ
 * @param[in] position  取得するポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_GetPokeDefaultPos( BTLV_MCSS_WORK *bmw, VecFx32 *pos, int position )
{
  BTLV_MCSS_GetDefaultPos( bmw, pos, position );
}

//============================================================================================
/**
 * @brief ポケモンの初期拡縮率を取得
 *
 * @param[in] position  取得するポケモンの立ち位置
 */
//============================================================================================
fx32  BTLV_MCSS_GetPokeDefaultScale( BTLV_MCSS_WORK *bmw, int position )
{
  return BTLV_MCSS_GetPokeDefaultScaleEx( bmw, position, bmw->mcss_proj_mode );
}

//============================================================================================
/**
 * @brief ポケモンの初期拡縮率を取得
 *
 * @param[in] position  取得するポケモンの立ち位置
 * @param[in] proj      取得する射影方法
 */
//============================================================================================
fx32  BTLV_MCSS_GetPokeDefaultScaleEx( BTLV_MCSS_WORK *bmw, int position, BTLV_MCSS_PROJECTION proj )
{
  return BTLV_MCSS_GetDefaultScale( bmw, position, proj );
}

//============================================================================================
/**
 * @brief ポケモンのスケール値を取得
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  取得するポケモンの立ち位置
 * @param[out]  scale   取得したスケール値を格納するワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_GetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  MCSS_GetScale( bmw->btlv_mcss[ index ].mcss, scale );
}

//============================================================================================
/**
 * @brief ポケモンのスケール値を格納
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  格納するポケモンの立ち位置
 * @param[in] scale   格納するスケール値
 */
//============================================================================================
void  BTLV_MCSS_SetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale )
{
  VecFx32 shadow_scale;
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  MCSS_SetScale( bmw->btlv_mcss[ index ].mcss, scale );
}

//============================================================================================
/**
 * @brief ポケモンの影バニッシュフラグを格納
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  格納するポケモンの立ち位置
 * @param[in] flag      格納するスケール値
 */
//============================================================================================
void  BTLV_MCSS_SetShadowVanishFlag( BTLV_MCSS_WORK *bmw, int position, u8 flag )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  MCSS_SetShadowVanishFlag( bmw->btlv_mcss[ index ].mcss, flag );
}

//============================================================================================
/**
 * @brief アニメーションフレームスピードセット
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  セットするMCSSの立ち位置
 * @param[in] speed     セットするスピード
 */
//============================================================================================
void  BTLV_MCSS_SetAnmSpeed( BTLV_MCSS_WORK *bmw, int position, fx32 speed )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  //MCSS_SetAnmSpeed( bmw->btlv_mcss[ index ].mcss, speed );
}

//============================================================================================
/**
 * @brief ポケモン移動
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  移動するポケモンの立ち位置
 * @param[in] type    移動タイプ
 * @param[in] pos     移動タイプにより意味が変化
 *              EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  移動先
 *              EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame   移動フレーム数（目的地まで何フレームで到達するか）
 * @param[in] wait    移動ウエイト
 * @param[in] count   往復カウント（POKEMCSS_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_MCSS_MovePosition( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *pos, int frame, int wait, int count )
{
  VecFx32 start;
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  MCSS_GetPosition( bmw->btlv_mcss[ index ].mcss, &start );
  //移動の補間は相対指定とする
  if( type == EFFTOOL_CALCTYPE_INTERPOLATION )
  {
    //ポケモンの視線の方向で＋－を決定する
    if( position & 1 )
    {
      pos->x = start.x - pos->x;
      pos->y += start.y;
      pos->z += start.z;
    }
    else
    {
      pos->x += start.x;
      pos->y += start.y;
      pos->z += start.z;
    }
  }
  //初期位置移動
  if( type == BTLEFF_POKEMON_MOVE_INIT )
  {
    BTLV_MCSS_GetDefaultPos( bmw, pos, position );
    type = EFFTOOL_CALCTYPE_INTERPOLATION;
  }
  BTLV_MCSS_TCBInitialize( bmw, position, type, &start, pos, frame, wait, count, TCB_BTLV_MCSS_Move, REVERSE_FLAG_ON );
  bmw->mcss_tcb_move_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 * @brief ポケモン拡縮
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  拡縮するポケモンの立ち位置
 * @param[in] type    拡縮タイプ
 * @param[in] scale   拡縮タイプにより意味が変化
 *              EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  最終的なスケール値
 *              EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame   拡縮フレーム数（設定した拡縮値まで何フレームで到達するか）
 * @param[in] wait    拡縮ウエイト
 * @param[in] count   往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_MCSS_MoveScale( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *scale, int frame, int wait, int count )
{
  VecFx32 start;
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  MCSS_GetOfsScale( bmw->btlv_mcss[ index ].mcss, &start );
  BTLV_MCSS_TCBInitialize( bmw, position, type, &start, scale, frame, wait, count, TCB_BTLV_MCSS_Scale, REVERSE_FLAG_OFF );
  bmw->mcss_tcb_scale_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 * @brief ポケモン回転
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  回転するポケモンの立ち位置
 * @param[in] type    回転タイプ
 * @param[in] rotate    回転タイプにより意味が変化
 *              EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  最終的な回転値
 *              EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame   回転フレーム数（設定した回転値まで何フレームで到達するか）
 * @param[in] wait    回転ウエイト
 * @param[in] count   往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_MCSS_MoveRotate( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *rotate, int frame, int wait, int count )
{
  VecFx32 start;
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  MCSS_GetRotate( bmw->btlv_mcss[ index ].mcss, &start );
  BTLV_MCSS_TCBInitialize( bmw, position, type, &start, rotate, frame, wait, count, TCB_BTLV_MCSS_Rotate, REVERSE_FLAG_ON );
  bmw->mcss_tcb_rotate_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 * @brief ポケモンまばたき
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  まばたきるポケモンの立ち位置
 * @param[in] type    まばたきイプ
 * @param[in] wait    まばたきウエイト
 * @param[in] count   まばたきカウント
 */
//============================================================================================
void  BTLV_MCSS_MoveBlink( BTLV_MCSS_WORK *bmw, int position, int type, int wait, int count )
{
  BTLV_MCSS_TCB_WORK  *pmtw = GFL_HEAP_AllocMemory( bmw->heapID, sizeof( BTLV_MCSS_TCB_WORK ) );

  pmtw->bmw           = bmw;
  pmtw->position      = position;
  pmtw->emw.move_type = type;
  pmtw->emw.wait      = 0;
  pmtw->emw.wait_tmp  = wait;
  pmtw->emw.count     = count * 2;  //閉じて開くを1回とカウントするために倍しておく

  switch( type ){
  case BTLEFF_MEPACHI_ON:
    BTLV_MCSS_SetMepachiFlag( bmw, position, BTLV_MCSS_MEPACHI_ON );
    break;
  case BTLEFF_MEPACHI_OFF:
    BTLV_MCSS_SetMepachiFlag( bmw, position, BTLV_MCSS_MEPACHI_OFF );
    break;
  case BTLEFF_MEPACHI_MABATAKI:
    GFL_TCB_AddTask( bmw->tcb_sys, TCB_BTLV_MCSS_Blink, pmtw, 0 );
    bmw->mcss_tcb_blink_execute |= BTLV_EFFTOOL_Pos2Bit( position );
    break;
  }
}

//============================================================================================
/**
 * @brief ポケモンα値
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  α値セットするポケモンの立ち位置
 * @param[in] type      α値タイプ
 * @param[in] alpha     α値タイプにより意味が変化
 *                      EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  最終的なα値
 *                      EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame     フレーム数（設定したα値まで何フレームで到達するか）
 * @param[in] wait      ウエイト
 * @param[in] count     往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_MCSS_MoveAlpha( BTLV_MCSS_WORK *bmw, int position, int type, int alpha, int frame, int wait, int count )
{
  VecFx32 start;
  VecFx32 end;
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  start.x = FX32_CONST( MCSS_GetAlpha( bmw->btlv_mcss[ index ].mcss ) );
  start.y = 0;
  start.z = 0;
  end.x = FX32_CONST( alpha );
  end.y = 0;
  end.z = 0;
  BTLV_MCSS_TCBInitialize( bmw, position, type, &start, &end, frame, wait, count, TCB_BTLV_MCSS_Alpha, REVERSE_FLAG_OFF );
  bmw->mcss_tcb_alpha_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 * @brief ポケモン円運動
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] bmmcp     円運動パラメータ構造体
 */
//============================================================================================
void  BTLV_MCSS_MoveCircle( BTLV_MCSS_WORK *bmw, BTLV_MCSS_MOVE_CIRCLE_PARAM* bmmcp )
{
  BTLV_MCSS_MOVE_CIRCLE_PARAM* bmmcp_p = GFL_HEAP_AllocMemory( bmw->heapID, sizeof( BTLV_MCSS_MOVE_CIRCLE_PARAM ) );

  bmmcp_p->bmw                      = bmw;
  bmmcp_p->position                 = bmmcp->position;
  bmmcp_p->axis                     = bmmcp->axis;
  bmmcp_p->shift                    = bmmcp->shift;
  bmmcp_p->radius_h                 = bmmcp->radius_h;
  bmmcp_p->radius_v                 = bmmcp->radius_v;
  bmmcp_p->frame                    = bmmcp->frame;
  bmmcp_p->rotate_wait              = bmmcp->rotate_wait;
  bmmcp_p->count                    = bmmcp->count;
  bmmcp_p->rotate_after_wait        = bmmcp->rotate_after_wait;
  bmmcp_p->rotate_wait_count        = 0;
  bmmcp_p->rotate_after_wait_count  = 0;

  //ポケモンの向きによって、＋－を決定
  if( bmmcp_p->position & 1 )
  {
    if( ( ( bmmcp_p->axis == BTLEFF_AXIS_X_L ) || ( bmmcp_p->axis == BTLEFF_AXIS_X_R ) ||
          ( bmmcp_p->axis == BTLEFF_AXIS_Z_L ) || ( bmmcp_p->axis == BTLEFF_AXIS_Z_R ) ) &&
        ( ( bmmcp->shift == BTLEFF_SHIFT_V_P ) || ( bmmcp->shift == BTLEFF_SHIFT_V_M ) ) )
    {
      ;
    }
    else
    {
      bmmcp_p->shift ^= 1;
    }
  }
  if( bmmcp_p->axis & 1 )
  {
    bmmcp_p->angle  = 0x10000;
  }
  else
  {
    bmmcp_p->angle  = 0;
  }
  bmmcp_p->speed  = 0x10000 / bmmcp->frame;

  GFL_TCB_AddTask( bmw->tcb_sys, TCB_BTLV_MCSS_MoveCircle, bmmcp_p, 0 );
  bmw->mcss_tcb_move_execute |= BTLV_EFFTOOL_Pos2Bit( bmmcp_p->position );
}

//============================================================================================
/**
 * @brief ポケモンモザイク
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  モザイクするポケモンの立ち位置
 * @param[in] type      モザイクタイプ
 * @param[in] alpha     モザイクタイプにより意味が変化
 *                      EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  最終的なモザイク値
 *                      EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame     フレーム数（設定したモザイク値まで何フレームで到達するか）
 * @param[in] wait      ウエイト
 * @param[in] count     往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_MCSS_MoveMosaic( BTLV_MCSS_WORK *bmw, int position, int type, int mosaic, int frame, int wait, int count )
{
  VecFx32 start;
  VecFx32 end;
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  start.x = FX32_CONST( MCSS_GetMosaic( bmw->btlv_mcss[ index ].mcss ) );
  start.y = 0;
  start.z = 0;
  end.x = FX32_CONST( mosaic );
  end.y = 0;
  end.z = 0;
  BTLV_MCSS_TCBInitialize( bmw, position, type, &start, &end, frame, wait, count, TCB_BTLV_MCSS_Mosaic, REVERSE_FLAG_OFF );
  bmw->mcss_tcb_mosaic_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 * @brief タスクが起動中かチェック
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  ポケモンの立ち位置
 *
 * @retval: TRUE:起動中　FALSE:終了
 */
//============================================================================================
BOOL  BTLV_MCSS_CheckTCBExecute( BTLV_MCSS_WORK *bmw, int position )
{
  BOOL  pal_fade_flag = FALSE;

  if( BTLV_MCSS_CheckExist( bmw, position ) )
  {
    int index = BTLV_MCSS_GetIndex( bmw, position );
    pal_fade_flag = MCSS_CheckExecutePaletteFade( bmw->btlv_mcss[ index ].mcss );
  }

  return ( ( bmw->mcss_tcb_move_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->mcss_tcb_scale_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->mcss_tcb_rotate_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->mcss_tcb_blink_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->mcss_tcb_alpha_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->mcss_tcb_mosaic_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->mcss_tcb_rotation_execute ) ||
       ( pal_fade_flag ) );
}

//============================================================================================
/**
 * @brief タスクが起動中かチェック（すべての立ち位置）
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 *
 * @retval: TRUE:起動中　FALSE:終了
 */
//============================================================================================
BOOL  BTLV_MCSS_CheckTCBExecuteAllPos( BTLV_MCSS_WORK *bmw )
{ 
  BtlvMcssPos pos;

  for( pos = BTLV_MCSS_POS_AA ; pos < BTLV_MCSS_POS_MAX ; pos++ )
  { 
    if( BTLV_MCSS_CheckExist( bmw, pos ) )
    { 
      if( BTLV_MCSS_CheckTCBExecute( bmw, pos ) )
      { 
        return TRUE;
      }
    }
  }
  return FALSE;
}

//============================================================================================
/**
 * @brief 指定された立ち位置に存在するかチェック
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  チェックする立ち位置
 *
 * @retval  TRUE:存在する　FALSE:存在しない
 */
//============================================================================================
BOOL  BTLV_MCSS_CheckExist( BTLV_MCSS_WORK *bmw, int position )
{
  return ( BTLV_MCSS_GetIndex( bmw, position ) != BTLV_MCSS_NO_INDEX );
}

//============================================================================================
/**
 * @brief 指定された立ち位置のMCSSにパレットフェードをセットする
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  MCSSの立ち位置
 * @param[in] start_evy セットするパラメータ（フェードさせる色に対する開始割合16段階）
 * @param[in] end_evy   セットするパラメータ（フェードさせる色に対する終了割合16段階）
 * @param[in] wait      セットするパラメータ（ウェイト）
 * @param[in] rgb       セットするパラメータ（フェードさせる色）
 *
 */
//============================================================================================
void  BTLV_MCSS_SetPaletteFade( BTLV_MCSS_WORK *bmw, int position, u8 start_evy, u8 end_evy, u8 wait, u32 rgb )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  MCSS_SetPaletteFade( bmw->btlv_mcss[ index ].mcss, start_evy, end_evy, wait, rgb );
}

//============================================================================================
/**
 * @brief 指定された立ち位置のMCSSのポケモンNoデータを取得
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  MCSSの立ち位置
 *
 */
//============================================================================================
int  BTLV_MCSS_GetMonsNo( BTLV_MCSS_WORK *bmw, int position )
{ 
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  return bmw->btlv_mcss[ index ].mons_no;
}

//============================================================================================
/**
 * @brief 指定された立ち位置のMCSSのポケモンNoデータを取得
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  MCSSの立ち位置
 *
 */
//============================================================================================
int  BTLV_MCSS_GetFormNo( BTLV_MCSS_WORK *bmw, int position )
{ 
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  return bmw->btlv_mcss[ index ].form_no;
}

//============================================================================================
/**
 * @brief 指定された立ち位置のMCSSの体重データを取得
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  MCSSの立ち位置
 *
 */
//============================================================================================
u16  BTLV_MCSS_GetWeight( BTLV_MCSS_WORK *bmw, int position )
{ 
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  return bmw->btlv_mcss[ index ].weight;
}

//============================================================================================
/**
 * @brief 指定された立ち位置のstatus_flagを取得
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  MCSSの立ち位置
 *
 */
//============================================================================================
u32  BTLV_MCSS_GetStatusFlag( BTLV_MCSS_WORK *bmw, int position )
{ 
  int index;
  if( position == BTLV_MCSS_POS_ERROR )
  { 
    return 0;
  }
  index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  return bmw->btlv_mcss[ index ].status_flag;
}

//============================================================================================
/**
 * @brief 指定された立ち位置のMCSSにみがわりセット
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  MCSSの立ち位置
 * @param[in] sw        みがわりセットorリセット
 * @param[in] flag      BTLV_MCSSで管理しているフラグの立ち下げをどうするか（TRUE:立ち下げする　FALSE:しない）
 *
 */
//============================================================================================
void  BTLV_MCSS_SetMigawari( BTLV_MCSS_WORK* bmw, int position, int sw, BOOL flag )
{ 
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  if( sw == BTLEFF_MIGAWARI_OFF )
  { 
    //再度ぶちをつけなければいけないのでコールバックを設定しておく
    MCSS_SetCallBackFunc( bmw->mcss_sys, BTLV_MCSS_MakeBuchi, &bmw->btlv_mcss[ index ].personal_rnd );
    MCSS_ReloadResource( bmw->mcss_sys, bmw->btlv_mcss[ index ].mcss, &bmw->btlv_mcss[ index ].maw );
    if( flag )
    { 
      bmw->btlv_mcss[ index ].status_flag &= ( BTLV_MCSS_STATUS_FLAG_MIGAWARI ^ 0xffffffff );
    }
  }
  else
  { 
    MCSS_ADD_WORK maw;
    maw.arcID = POKEGRA_GetArcID();
    maw.ncbr = position & 1 ? NARC_pokegra_wb_pfwb_migawari_NCBR : NARC_pokegra_wb_pbwb_migawari_NCBR;
    maw.nclr = NARC_pokegra_wb_pmwb_migawari_NCLR;
    maw.ncer = position & 1 ? NARC_pokegra_wb_pfwb_migawari_NCER : NARC_pokegra_wb_pbwb_migawari_NCER;
    maw.nanr = position & 1 ? NARC_pokegra_wb_pfwb_migawari_NANR : NARC_pokegra_wb_pbwb_migawari_NANR;
    maw.nmcr = position & 1 ? NARC_pokegra_wb_pfwb_migawari_NMCR : NARC_pokegra_wb_pbwb_migawari_NMCR;
    maw.nmar = position & 1 ? NARC_pokegra_wb_pfwb_migawari_NMAR : NARC_pokegra_wb_pbwb_migawari_NMAR;
    maw.ncec = position & 1 ? NARC_pokegra_wb_pfwb_migawari_NCEC : NARC_pokegra_wb_pbwb_migawari_NCEC;
    MCSS_ReloadResource( bmw->mcss_sys, bmw->btlv_mcss[ index ].mcss, &maw );
    if( flag )
    { 
      bmw->btlv_mcss[ index ].status_flag |= BTLV_MCSS_STATUS_FLAG_MIGAWARI;
    }
  }
}

//============================================================================================
/**
 * @brief モザイク
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  セットするMCSSの立ち位置
 * @param[in] speed     セットするスピード
 */
//============================================================================================
void  BTLV_MCSS_SetMosaic( BTLV_MCSS_WORK *bmw, int position, int mosaic )
{
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );
  MCSS_SetMosaic( bmw->mcss_sys, bmw->btlv_mcss[ index ].mcss, mosaic );
}

//============================================================================================
/**
 * @brief MAW構造体をコピーしてリソースをリロード
 *
 * @param[in] bmw BTLV_MCSS管理ワークへのポインタ
 * @param[in] src コピー元MCSSの立ち位置
 * @param[in] dst コピー先MCSSの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_CopyMAW( BTLV_MCSS_WORK *bmw, int src, int dst )
{
  int src_index = BTLV_MCSS_GetIndex( bmw, src );
  int dst_index = BTLV_MCSS_GetIndex( bmw, dst );
  bmw->btlv_mcss[ dst_index ].maw.arcID = bmw->btlv_mcss[ src_index ].maw.arcID;
  bmw->btlv_mcss[ dst_index ].maw.nclr = bmw->btlv_mcss[ src_index ].maw.nclr;
  //正面→背面
  if( src & 1 )
  { 
    bmw->btlv_mcss[ dst_index ].maw.ncbr = bmw->btlv_mcss[ src_index ].maw.ncbr + POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.ncer = bmw->btlv_mcss[ src_index ].maw.ncer + POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.nanr = bmw->btlv_mcss[ src_index ].maw.nanr + POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.nmcr = bmw->btlv_mcss[ src_index ].maw.nmcr + POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.nmar = bmw->btlv_mcss[ src_index ].maw.nmar + POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.ncec = bmw->btlv_mcss[ src_index ].maw.ncec + POKEGRA_MAX;
  }
  //背面→正面
  else
  { 
    bmw->btlv_mcss[ dst_index ].maw.ncbr = bmw->btlv_mcss[ src_index ].maw.ncbr - POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.ncer = bmw->btlv_mcss[ src_index ].maw.ncer - POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.nanr = bmw->btlv_mcss[ src_index ].maw.nanr - POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.nmcr = bmw->btlv_mcss[ src_index ].maw.nmcr - POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.nmar = bmw->btlv_mcss[ src_index ].maw.nmar - POKEGRA_MAX;
    bmw->btlv_mcss[ dst_index ].maw.ncec = bmw->btlv_mcss[ src_index ].maw.ncec - POKEGRA_MAX;
  }
  bmw->btlv_mcss[ dst_index ].personal_rnd = bmw->btlv_mcss[ src_index ].personal_rnd;
  MCSS_ReloadResource( bmw->mcss_sys, bmw->btlv_mcss[ dst_index ].mcss, &bmw->btlv_mcss[ dst_index ].maw );
}

//============================================================================================
/**
 * @brief 指定された立ち位置のポケモンの鳴き声を再生
 *
 * @param[in] bmw BTLV_MCSS管理ワークへのポインタ
 * @param[in] pos 立ち位置
 */
//============================================================================================
void  BTLV_MCSS_PlayVoice( BTLV_MCSS_WORK *bmw, int position )
{ 
  PMV_REF pmvRef;
  int pan;
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  switch( position ){
  case BTLV_MCSS_POS_AA:
  case BTLV_MCSS_POS_BB:
    pan = pokevoice_pan_single_table[ position ];
    break;
  case BTLV_MCSS_POS_A:
  case BTLV_MCSS_POS_B:
  case BTLV_MCSS_POS_C:
  case BTLV_MCSS_POS_D:
    if( bmw->mcss_pos_rotate )
    { 
      pan = pokevoice_pan_rotate_table[ position - BTLV_MCSS_POS_A ];
    }
    else if( bmw->mcss_pos_3vs3 )
    {
      pan = pokevoice_pan_triple_table[ position - BTLV_MCSS_POS_A ];
    }
    else
    {
      pan = pokevoice_pan_double_table[ position - BTLV_MCSS_POS_A ];
    }
    break;
  case BTLV_MCSS_POS_E:
  case BTLV_MCSS_POS_F:
    GF_ASSERT( bmw->mcss_pos_3vs3 == 1 || bmw->mcss_pos_rotate == 1 );
    if( bmw->mcss_pos_rotate )
    { 
      pan = pokevoice_pan_rotate_table[ position - BTLV_MCSS_POS_A ];
    }
    else
    { 
      pan = pokevoice_pan_triple_table[ position - BTLV_MCSS_POS_A ];
    }
    break;
  default:
    //定義されていないポジションが指定されています
    GF_ASSERT( 0 );
    break;
  }
   
  if( ( position & 1 ) == 0 )
  { 
    PMV_MakeRefDataMine( &pmvRef );
  }

	PMVOICE_Play(	bmw->btlv_mcss[ index ].mons_no, bmw->btlv_mcss[ index ].form_no, pan, FALSE, 0, 0, FALSE, (u32)&pmvRef );	
}

//============================================================================================
/**
 * @brief 指定された側のポケモンをローテーション
 *
 * @param[in] bmw   BTLV_MCSS管理ワークへのポインタ
 * @param[in] side  立ち位置側
 * @param[in] dir   ローテーション方向
 */
//============================================================================================
void  BTLV_MCSS_SetRotation( BTLV_MCSS_WORK* bmw, int side, int dir )
{ 
  BTLV_MCSS_ROTATION_WORK* bmrw = GFL_HEAP_AllocMemory( bmw->heapID, sizeof( BTLV_MCSS_ROTATION_WORK ) );

  bmrw->bmw     = bmw;
  bmrw->seq_no  = 0;
  bmrw->side    = side;
  bmrw->dir     = dir;
  bmw->mcss_tcb_rotation_execute = 1;

  GFL_TCB_AddTask( bmw->tcb_sys, TCB_BTLV_MCSS_Rotation, bmrw, 0 );
}

//============================================================================================
/**
 * @brief POKEMON_PARAMからMCSS_ADD_WORKを生成する
 *
 * @param[in]   pp        POKEMON_PARAM構造体へのポインタ
 * @param[out]  maw       MCSS_ADD_WORKワークへのポインタ
 * @param[in]   position  ポケモンの立ち位置
 */
//============================================================================================
static  void  BTLV_MCSS_MakeMAW( const POKEMON_PARAM *pp, MCSS_ADD_WORK *maw, int position )
{
  int dir = ( ( position & 1 ) ) ? MCSS_DIR_FRONT : MCSS_DIR_BACK;

  MCSS_TOOL_MakeMAWPP( pp, maw, dir );
}

//============================================================================================
/**
 * @brief tr_typeからMCSS_ADD_WORKを生成する
 *
 * @param[in]   pp        POKEMON_PARAM構造体へのポインタ
 * @param[out]  maw       MCSS_ADD_WORKワークへのポインタ
 * @param[in]   position  トレーナーの立ち位置
 */
//============================================================================================
static  void  BTLV_MCSS_MakeMAWTrainer( int tr_type, MCSS_ADD_WORK* maw, int position )
{
  int dir = ( ( position & 1 ) ) ? MCSS_DIR_FRONT : MCSS_DIR_BACK;

  MCSS_TOOL_MakeMAWTrainer( tr_type, maw, dir );
}

//============================================================================================
/**
 * @brief ポケモンデフォルトスケールセット
 */
//============================================================================================
static  void  BTLV_MCSS_SetDefaultScale( BTLV_MCSS_WORK *bmw, int position )
{
  VecFx32 scale;
  fx32    def_scale;
  int index = BTLV_MCSS_GetIndex( bmw, position );

  GF_ASSERT( position < BTLV_MCSS_POS_TOTAL );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss );

  def_scale = BTLV_MCSS_GetDefaultScale( bmw, position, bmw->mcss_proj_mode );

  VEC_Set( &scale, def_scale, def_scale, FX32_ONE );

  MCSS_SetScale( bmw->btlv_mcss[ index ].mcss, &scale );

  def_scale = BTLV_MCSS_GetDefaultScale( bmw, position, BTLV_MCSS_PROJ_PERSPECTIVE );

  //VEC_Set( &scale, def_scale, def_scale / 2, FX32_ONE );
  VEC_Set( &scale, def_scale, def_scale, FX32_ONE );

  MCSS_SetShadowScale( bmw->btlv_mcss[ index ].mcss, &scale );
}

//============================================================================================
/**
 * @brief ポケモン操作系タスク初期化処理
 */
//============================================================================================
static  void  BTLV_MCSS_TCBInitialize( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *start, VecFx32 *end, int frame, int wait, int count, GFL_TCB_FUNC *func, int reverse_flag )
{
  BTLV_MCSS_TCB_WORK  *pmtw = GFL_HEAP_AllocMemory( bmw->heapID, sizeof( BTLV_MCSS_TCB_WORK ) );

  pmtw->bmw               = bmw;
  pmtw->position          = position;
  pmtw->emw.move_type     = type;
  pmtw->emw.vec_time      = frame;
  pmtw->emw.vec_time_tmp  = frame;
  pmtw->emw.wait          = 0;
  pmtw->emw.wait_tmp      = wait;
  pmtw->emw.count         = count * 2;
  pmtw->emw.start_value.x = start->x;
  pmtw->emw.start_value.y = start->y;
  pmtw->emw.start_value.z = start->z;
  pmtw->emw.end_value.x   = end->x;
  pmtw->emw.end_value.y   = end->y;
  pmtw->emw.end_value.z   = end->z;
  pmtw->now_value.x       = start->x;
  pmtw->now_value.y       = start->y;
  pmtw->now_value.z       = start->z;

  switch( type ){
  case EFFTOOL_CALCTYPE_DIRECT:               //直接ポジションに移動
    break;
  case EFFTOOL_CALCTYPE_INTERPOLATION:        //移動先までを補間しながら移動
  case EFFTOOL_CALCTYPE_INTERPOLATION_DIRECT: //移動先までを補間しながら移動
    BTLV_EFFTOOL_CalcMoveVector( &pmtw->emw.start_value, end, &pmtw->emw.vector, FX32_CONST( frame ) );
    break;
  case EFFTOOL_CALCTYPE_ROUNDTRIP_LONG:       //指定した区間を往復移動
    pmtw->emw.count         *= 2;
    //fall through
  case EFFTOOL_CALCTYPE_ROUNDTRIP:            //指定した区間を往復移動
    pmtw->emw.vector.x = FX_Div( end->x, FX32_CONST( frame ) );
    pmtw->emw.vector.y = FX_Div( end->y, FX32_CONST( frame ) );
    pmtw->emw.vector.z = FX_Div( end->z, FX32_CONST( frame ) );
    if( ( position & 1 ) && ( reverse_flag ) )
    {
      pmtw->emw.vector.x *= -1;
      pmtw->emw.vector.z *= -1;
    }
    break;
  }
  GFL_TCB_AddTask( bmw->tcb_sys, func, pmtw, 0 );
}

//============================================================================================
/**
 * @brief ポケモン移動タスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Move( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;
  VecFx32 now_pos;
  BOOL  ret;
  int index = BTLV_MCSS_GetIndex( bmw, pmtw->position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  MCSS_GetPosition( bmw->btlv_mcss[ index ].mcss, &now_pos );
  ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_pos );
  MCSS_SetPosition( bmw->btlv_mcss[ index ].mcss, &now_pos );
  if( ret == TRUE ){
    bmw->mcss_tcb_move_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief ポケモン拡縮タスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Scale( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;
  VecFx32 now_scale;
  BOOL  ret;
  int index = BTLV_MCSS_GetIndex( bmw, pmtw->position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  MCSS_GetOfsScale( bmw->btlv_mcss[ index ].mcss, &now_scale );
  ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_scale );
  MCSS_SetOfsScale( bmw->btlv_mcss[ index ].mcss, &now_scale );
  if( ret == TRUE ){
    bmw->mcss_tcb_scale_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief ポケモン回転タスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Rotate( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;
  VecFx32 now_rotate;
  BOOL  ret;
  int index = BTLV_MCSS_GetIndex( bmw, pmtw->position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  MCSS_GetRotate( bmw->btlv_mcss[ index ].mcss, &now_rotate );
  ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_rotate );
  MCSS_SetRotate( bmw->btlv_mcss[ index ].mcss, &now_rotate );
  if( ret == TRUE ){
    bmw->mcss_tcb_rotate_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief ポケモンまばたきタスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Blink( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;

  //立ち位置にポケモンが存在しなかったらフリーする
  if( !BTLV_MCSS_CheckExist( bmw, pmtw->position ) ) 
  { 
    bmw->mcss_tcb_blink_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
    return;
  }

  if( pmtw->emw.wait == 0 ){
    pmtw->emw.wait = pmtw->emw.wait_tmp;
    BTLV_MCSS_SetMepachiFlag( pmtw->bmw, pmtw->position, BTLV_MCSS_MEPACHI_FLIP );
    if( --pmtw->emw.count == 0 ){
      bmw->mcss_tcb_blink_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
      GFL_HEAP_FreeMemory( work );
      GFL_TCB_DeleteTask( tcb );
    }
  }
  else{
    pmtw->emw.wait--;
  }
}

//============================================================================================
/**
 * @brief ポケモンα値タスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Alpha( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;
  BOOL  ret;
  int index = BTLV_MCSS_GetIndex( bmw, pmtw->position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &pmtw->now_value );
  MCSS_SetAlpha( bmw->btlv_mcss[ index ].mcss, pmtw->now_value.x >> FX32_SHIFT );
  if( ret == TRUE ){
    bmw->mcss_tcb_alpha_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief ポケモン円運動タスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_MoveCircle( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_MOVE_CIRCLE_PARAM*  bmmcp = ( BTLV_MCSS_MOVE_CIRCLE_PARAM * )work;
  BTLV_MCSS_WORK *bmw = bmmcp->bmw;
  VecFx32 ofs = { 0, 0, 0 };
  fx32  sin, cos;
  int index = BTLV_MCSS_GetIndex( bmw, bmmcp->position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  if( bmmcp->rotate_after_wait_count ==0 )
  {
    if( bmmcp->rotate_wait_count == bmmcp->rotate_wait )
    {
      bmmcp->rotate_wait_count = 0;
      if( bmmcp->axis & 1 )
      {
        bmmcp->angle -= bmmcp->speed;
      }
      else
      {
        bmmcp->angle += bmmcp->speed;
      }
      if( bmmcp->angle & 0xffff0000 )
      {
        bmmcp->angle &= 0x0000ffff;
        bmmcp->count--;
        bmmcp->rotate_after_wait_count = bmmcp->rotate_after_wait;
      }
      if( bmmcp->count )
      {
        switch( bmmcp->shift ){
        case BTLEFF_SHIFT_H_P:    //シフトＨ＋
          sin = FX_Mul( FX_SinIdx( ( bmmcp->angle + 0x4000 ) & 0xffff ), bmmcp->radius_h ) * -1;
          cos = FX_Mul( FX_CosIdx( ( bmmcp->angle + 0x4000 ) & 0xffff ), bmmcp->radius_v ) * -1;
          sin += bmmcp->radius_h;
          break;
        case BTLEFF_SHIFT_H_M:    //シフトＨ－
          sin = FX_Mul( FX_SinIdx( ( bmmcp->angle + 0x4000 ) & 0xffff ), bmmcp->radius_h ) - bmmcp->radius_h;
          cos = FX_Mul( FX_CosIdx( ( bmmcp->angle + 0x4000 ) & 0xffff ), bmmcp->radius_v );
          break;
        case BTLEFF_SHIFT_V_P:    //シフトＶ＋
          sin = FX_Mul( FX_SinIdx( bmmcp->angle ), bmmcp->radius_h ) * -1;
          cos = FX_Mul( FX_CosIdx( bmmcp->angle ), bmmcp->radius_v ) * -1;
          cos += bmmcp->radius_v;
          break;
        case BTLEFF_SHIFT_V_M:    //シフトＶ－
          sin = FX_Mul( FX_SinIdx( bmmcp->angle ), bmmcp->radius_h );
          cos = FX_Mul( FX_CosIdx( bmmcp->angle ), bmmcp->radius_v ) - bmmcp->radius_v;
          break;
        }
        switch( ( bmmcp->axis & 7 ) >> 1 ){
        default:
        case 0:   //X軸
          ofs.z = sin;
          ofs.y = cos;
          break;
        case 1:   //Y軸
          ofs.x = sin;
          ofs.z = cos;
          break;
        case 2:   //Z軸
          ofs.x = sin;
          ofs.y = cos;
          break;
        }
      }
      MCSS_SetOfsPosition( bmw->btlv_mcss[ index ].mcss, &ofs );
    }
    else
    {
      bmmcp->rotate_wait_count++;
    }
  }
  else
  {
    bmmcp->rotate_after_wait_count--;
  }
  if( bmmcp->count == 0 ){
    bmw->mcss_tcb_move_execute &= ( BTLV_EFFTOOL_Pos2Bit( bmmcp->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief ポケモンモザイクタスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Mosaic( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;
  BOOL  ret;
  int index = BTLV_MCSS_GetIndex( bmw, pmtw->position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &pmtw->now_value );
  MCSS_SetMosaic( bmw->mcss_sys, bmw->btlv_mcss[ index ].mcss, pmtw->now_value.x >> FX32_SHIFT );
  if( ret == TRUE ){
    bmw->mcss_tcb_mosaic_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}


//============================================================================================
/**
 * @brief 指定したフレームで呼ばれるコールバック関数
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_StopAnime( GFL_TCB *tcb, void *work )
{ 
  BTLV_MCSS_WORK *bmw = BTLV_EFFECT_GetMcssWork();
  BTLV_MCSS_STOP_ANIME* bmsa = ( BTLV_MCSS_STOP_ANIME* )work;
  int index = BTLV_MCSS_GetIndex( bmw, bmsa->position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  switch( bmsa->seq_no ){ 
  case 0:
    NNS_G2dStopAnimCtrl( NNS_G2dGetMCAnimAnimCtrl( MCSS_GetAnimCtrl( bmw->btlv_mcss[ index ].mcss ) ) );
    MCSS_SetTraverseMCNodesCallBack( bmw->btlv_mcss[ index ].mcss, ANIME_STOP_FLAG | bmsa->position, BTLV_MCSS_CallBackNodes );
    bmsa->seq_no++;
    bmsa->wait = BTLV_MCSS_STOP_ANIME_TIME;
    break;
  case 1:
    if( ( --bmsa->wait == 0 ) || 
        ( bmw->mcss_tcb_rotate_execute ) ||
        ( BTLV_EFFVM_GetExecuteEffectType( BTLV_EFFECT_GetVMHandle() ) == EXECUTE_EFF_TYPE_WAZA ) )
    { 
      MCSS_SetAnimeIndex( bmw->btlv_mcss[ index ].mcss, SEQ_ANIME_STANDBY );
      NNS_G2dStartAnimCtrl( NNS_G2dGetMCAnimAnimCtrl( MCSS_GetAnimCtrl( bmw->btlv_mcss[ index ].mcss ) ) );
      MCSS_SetTraverseMCNodesCallBack( bmw->btlv_mcss[ index ].mcss, ANIME_START_FLAG | bmsa->position, BTLV_MCSS_CallBackNodes );
      bmsa->seq_no++;
    }
    break;
  case 2:
    MCSS_SetAnimCtrlCallBack( bmw->btlv_mcss[ index ].mcss, bmsa->position, BTLV_MCSS_CallBackFunctorFrame, 1 );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
    bmw->btlv_mcss[ index ].tcb = NULL;
    break;
  }
}

//============================================================================================
/**
 * @brief ローテーション処理
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Rotation( GFL_TCB *tcb, void *work )
{ 
  BTLV_MCSS_ROTATION_WORK* bmrw = ( BTLV_MCSS_ROTATION_WORK* )work;
  int src_pos[ 2 ][ 3 ] = { 
    { BTLV_MCSS_POS_A, BTLV_MCSS_POS_C, BTLV_MCSS_POS_E },
    { BTLV_MCSS_POS_B, BTLV_MCSS_POS_D, BTLV_MCSS_POS_F },
  };
  int dst_pos[ 2 ][ 2 ][ 3 ] = { 
    { 
      { BTLV_MCSS_POS_E, BTLV_MCSS_POS_A, BTLV_MCSS_POS_C },
      { BTLV_MCSS_POS_F, BTLV_MCSS_POS_B, BTLV_MCSS_POS_D },
    },
    { 
      { BTLV_MCSS_POS_C, BTLV_MCSS_POS_E, BTLV_MCSS_POS_A },
      { BTLV_MCSS_POS_D, BTLV_MCSS_POS_F, BTLV_MCSS_POS_B },
    },
  };

  switch( bmrw->seq_no ){ 
  case 0:
    { 
      int i;
      for( i = 0 ; i < 3 ; i++ )
      { 
        if( bmrw->bmw->btlv_mcss[ src_pos[ bmrw->side ][ i ] ].tcb )
        { 
          break;
        }
      }
      if( i == 3 )
      { 
        bmrw->seq_no++;
      }
    }
    break;
  case 1:
    { 
      int i;

      for( i = 0 ; i < 3 ; i++ )
      { 
        VecFx32 end;
        if( BTLV_MCSS_CheckExist( bmrw->bmw, src_pos[ bmrw->side ][ i ] ) )
        { 
          BTLV_MCSS_GetDefaultPos( bmrw->bmw, &end, dst_pos[ bmrw->dir ][ bmrw->side ][ i ] );
          BTLV_MCSS_MovePosition( bmrw->bmw, src_pos[ bmrw->side ][ i ],
                                  EFFTOOL_CALCTYPE_INTERPOLATION_DIRECT, &end, 100, 0, 0 );
        }
      }
    }
    bmrw->seq_no++;
    break;
  case 2:
    if( bmrw->bmw->mcss_tcb_move_execute == 0 )
    { 
      int i;
      int index[ 3 ];

      for( i = 0 ; i < 3 ; i++ )
      { 
        index[ i ] = BTLV_MCSS_GetIndex( bmrw->bmw, src_pos[ bmrw->side ][ i ] );
      }
      for( i = 0 ; i < 3 ; i++ )
      { 
        if( index[ i ] != BTLV_MCSS_NO_INDEX )
        { 
          bmrw->bmw->btlv_mcss[ index[ i ] ].position = dst_pos[ bmrw->dir ][ bmrw->side ][ i ];
        }
      }

      bmrw->bmw->mcss_tcb_rotation_execute = 0;
      GFL_HEAP_FreeMemory( work );
      GFL_TCB_DeleteTask( tcb );
    }
    break;
  }
}

//============================================================================================
/**
 * @brief 指定したフレームで呼ばれるコールバック関数
 */
//============================================================================================
static  void  BTLV_MCSS_CallBackFunctorFrame( u32 data, fx32 currentFrame )
{
  BTLV_MCSS_WORK *bmw = BTLV_EFFECT_GetMcssWork();
  BTLV_MCSS_STOP_ANIME* bmsa;
  int index = BTLV_MCSS_GetIndex( bmw, data );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  if( BTLV_EFFVM_GetExecuteEffectType( BTLV_EFFECT_GetVMHandle() ) == EXECUTE_EFF_TYPE_WAZA )
  { 
    return;
  }

  if( MCSS_GetStopCellAnms( bmw->btlv_mcss[ index ].mcss ) == MCSS_CELL_ANIM_NONSTOP ) 
  { 
    return;
  }

  if( bmw->mcss_tcb_rotation_execute )
  { 
    return;
  }

  bmsa = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bmw->heapID ), sizeof( BTLV_MCSS_STOP_ANIME ) );
  bmsa->position = data;

  if( MCSS_GetMCellAnmNum( bmw->btlv_mcss[ index ].mcss ) > 1 )
  { 
    MCSS_SetAnimeIndex( bmw->btlv_mcss[ index ].mcss, SEQ_ANIME_STOP );
  }

  GF_ASSERT( bmw->btlv_mcss[ index ].tcb == NULL );
  bmw->btlv_mcss[ index ].tcb = GFL_TCB_AddTask( bmw->tcb_sys, TCB_BTLV_MCSS_StopAnime, bmsa, 0 );
}

//============================================================================================
/**
 * @brief ノード巡回して呼ばれるコールバック関数
 */
//============================================================================================
static  BOOL  BTLV_MCSS_CallBackNodes( u32 data, const NNSG2dMultiCellHierarchyData* pNodeData,
                                       NNSG2dCellAnimation* pCellAnim, u16 nodeIdx )
{ 
  BTLV_MCSS_WORK *bmw = BTLV_EFFECT_GetMcssWork();
  int position = data & 0x0000ffff;
  int index = BTLV_MCSS_GetIndex( bmw, position );
  GF_ASSERT( bmw->btlv_mcss[ index ].mcss != NULL );

  if( data & ANIME_STOP_FLAG )
  { 
    BOOL  flag = FALSE;
    u8    cnt = MCSS_GetStopCellAnms( bmw->btlv_mcss[ index ].mcss );
    int   i;

    for( i = 0 ; i < cnt ; i++ )
    { 
      if( nodeIdx == MCSS_GetStopNode( bmw->btlv_mcss[ index ].mcss, i ) )
      { 
        flag = TRUE;
      }
    }
    if( flag == FALSE )
    { 
      NNS_G2dStopAnimCtrl( NNS_G2dGetCellAnimAnimCtrl( pCellAnim ) );
    }
    else
    { 
      NNS_G2dStartAnimCtrl( NNS_G2dGetCellAnimAnimCtrl( pCellAnim ) );
    }
  }
  else
  { 
    NNS_G2dStartAnimCtrl( NNS_G2dGetCellAnimAnimCtrl( pCellAnim ) );
  }
  return TRUE;
}

//============================================================================================
/**
 * @brief MCSSの立ち位置ごとのデフォルト座標を取得
 *
 * @param[in]   bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[out]  pos       座標を格納するワークへのポインタ
 * @param[in]   position  ポケモンの立ち位置
 */
//============================================================================================
static  void  BTLV_MCSS_GetDefaultPos( BTLV_MCSS_WORK *bmw, VecFx32 *pos, BtlvMcssPos position )
{
  const VecFx32 *pos_table;
  switch( position ){
  case BTLV_MCSS_POS_AA:
  case BTLV_MCSS_POS_BB:
    pos_table = &poke_pos_single_table[ position ];
    break;
  case BTLV_MCSS_POS_A:
  case BTLV_MCSS_POS_B:
  case BTLV_MCSS_POS_C:
  case BTLV_MCSS_POS_D:
    if( bmw->mcss_pos_rotate )
    { 
      pos_table = &poke_pos_rotate_table[ position - BTLV_MCSS_POS_A ];
    }
    else if( bmw->mcss_pos_3vs3 )
    {
      pos_table = &poke_pos_triple_table[ position - BTLV_MCSS_POS_A ];
    }
    else
    {
      pos_table = &poke_pos_double_table[ position - BTLV_MCSS_POS_A ];
    }
    break;
  case BTLV_MCSS_POS_E:
  case BTLV_MCSS_POS_F:
    GF_ASSERT( bmw->mcss_pos_3vs3 == 1 || bmw->mcss_pos_rotate == 1 );
    if( bmw->mcss_pos_rotate )
    { 
      pos_table = &poke_pos_rotate_table[ position - BTLV_MCSS_POS_A ];
    }
    else
    { 
      pos_table = &poke_pos_triple_table[ position - BTLV_MCSS_POS_A ];
    }
    break;
  case BTLV_MCSS_POS_TR_AA:
  case BTLV_MCSS_POS_TR_BB:
  case BTLV_MCSS_POS_TR_A:
  case BTLV_MCSS_POS_TR_B:
  case BTLV_MCSS_POS_TR_C:
  case BTLV_MCSS_POS_TR_D:
    pos_table = &trainer_pos_table[ position - BTLV_MCSS_POS_TR_AA ];
    break;
  default:
    //定義されていないポジションが指定されています
    GF_ASSERT( 0 );
    break;
  }
  pos->x = pos_table->x;
  pos->y = pos_table->y;
  pos->z = pos_table->z;
}

//============================================================================================
/**
 * @brief MCSSの立ち位置ごとのデフォルトスケールを取得
 *
 * @param[in]   bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in]   position  MCSSの立ち位置
 * @param[in]   proj      射影モード
 */
//============================================================================================
static  fx32  BTLV_MCSS_GetDefaultScale( BTLV_MCSS_WORK* bmw, BtlvMcssPos position, BTLV_MCSS_PROJECTION proj )
{
  fx32 scale;

  if( proj == BTLV_MCSS_PROJ_PERSPECTIVE )
  {
    switch( position ){
    case BTLV_MCSS_POS_AA:
    case BTLV_MCSS_POS_BB:
      scale = poke_scale_single_table[ position ];
      break;
    case BTLV_MCSS_POS_A:
    case BTLV_MCSS_POS_B:
    case BTLV_MCSS_POS_C:
    case BTLV_MCSS_POS_D:
      if( bmw->mcss_pos_rotate )
      { 
        scale = poke_scale_rotate_table[ position - BTLV_MCSS_POS_A ];
      }
      else if( bmw->mcss_pos_3vs3 )
      {
        scale = poke_scale_triple_table[ position - BTLV_MCSS_POS_A ];
      }
      else
      {
        scale = poke_scale_double_table[ position - BTLV_MCSS_POS_A ];
      }
      break;
    case BTLV_MCSS_POS_E:
    case BTLV_MCSS_POS_F:
      GF_ASSERT( bmw->mcss_pos_3vs3 == 1 || bmw->mcss_pos_rotate == 1 );
      if( bmw->mcss_pos_rotate )
      { 
        scale = poke_scale_rotate_table[ position - BTLV_MCSS_POS_A ];
      }
      else
      { 
        scale = poke_scale_triple_table[ position - BTLV_MCSS_POS_A ];
      }
      break;
    case BTLV_MCSS_POS_TR_AA:
    case BTLV_MCSS_POS_TR_BB:
    case BTLV_MCSS_POS_TR_A:
    case BTLV_MCSS_POS_TR_B:
    case BTLV_MCSS_POS_TR_C:
    case BTLV_MCSS_POS_TR_D:
      scale = trainer_scale_table[ position - BTLV_MCSS_POS_TR_AA ];
      break;
    default:
      //定義されていないポジションが指定されています
      GF_ASSERT( 0 );
      break;
    }
  }
  else
  {
    if( position & 1 )
    {
      scale = BTLV_MCSS_ORTHO_SCALE_ENEMY;
    }
    else
    {
      scale = BTLV_MCSS_ORTHO_SCALE_MINE;
    }
  }
  return scale;
}

//============================================================================================
/**
 * @brief キャラデータにぶちをつける
 *
 * @param[in] maw  MCSS_ADD_WORK構造体へのポインタ
 * @param[in] tlw  リソースデータワーク構造体
 * @param[in] work 個性乱数が入ってくる
 *
 * @retval  TRUE:コールバックを解除する
 */
//============================================================================================
static  BOOL  BTLV_MCSS_MakeBuchi( const MCSS_ADD_WORK* maw, TCB_LOADRESOURCE_WORK* tlw, void* work )
{ 
  //パッチールにぶちをつける
  if( maw->ncec == NARC_pokegra_wb_pfwb_327_NCEC )
  { 
	  const	PATTIIRU_BUCHI_DATA	*pbd;
	  int i, j;
	  u8	setx, sety, cnt;
	  int	pos[ 2 ];
    u32 *rnd = (u32*)work;
    u8  *buf = tlw->pCharData->pRawData;

	  //1枚目
	  for( i = 0 ; i < 4 ; i++ )
    {
		  pbd = pbd_table[ i ];
		  cnt=0;
		  while( pbd[ cnt ].posx != 0xff )
      {
			  setx = pbd[ cnt ].posx +   ( ( (*rnd) & 0x0f ) - 8 );
			  sety = pbd[ cnt ].posy + ( ( ( (*rnd) & 0xf0 ) >> 4 ) - 8 );
			  pos[ 0 ] = setx / 2 + sety * 128;
			  pos[ 1 ] = setx / 2 + ( sety + 40 ) * 128;
        for( j = 0 ; j < 2 ; j++ )
        { 
			    if( setx & 1)
          {
				    if( ( ( buf[ pos[ j ] ] & 0xf0 ) >= 0x10 ) && ( ( buf[ pos[ j ] ] & 0xf0 ) <= 0x30) )
            {
					    buf[ pos[ j ] ] += 0x50;
				    }
			    }
			    else
          {
				    if( ( ( buf[ pos[ j ] ] & 0x0f ) >= 0x01 ) && ( ( buf[ pos[ j ] ] & 0x0f ) <= 0x03 ) )
            {
					    buf[ pos[ j ] ] += 0x05;
				    }
			    }
        }
			  cnt++;
		  }
		  (*rnd) = (*rnd) >> 8;
	  }
  }

  return TRUE;
}

//============================================================================================
/**
 * @brief 立ち位置のインデックスを取得
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
static  int   BTLV_MCSS_GetIndex( BTLV_MCSS_WORK* bmw, BtlvMcssPos position )
{ 
  int i;

  for( i = 0 ; i < BTLV_MCSS_POS_TOTAL ; i++ )
  { 
    if( bmw->btlv_mcss[ i ].mcss )
    { 
      if( bmw->btlv_mcss[ i ].position == position )
      { 
        return i;
      }
    }
  }
  return BTLV_MCSS_NO_INDEX;
}

#ifdef PM_DEBUG
//============================================================================================
/**
 * @brief BTLV_MCSS追加（デバッグ用）
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] madw    MCSS_ADD_DEBUG_WORK構造体へのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_AddDebug( BTLV_MCSS_WORK *bmw, const MCSS_ADD_DEBUG_WORK *madw, int position )
{
  VecFx32 pos;

  GF_ASSERT( position < BTLV_MCSS_POS_TOTAL );
  if( bmw->btlv_mcss[ position ].mcss ){
    BTLV_MCSS_Del( bmw, position );
  }

  BTLV_MCSS_GetDefaultPos( bmw, &pos, position );
  bmw->btlv_mcss[ position ].mcss = MCSS_AddDebug( bmw->mcss_sys, pos.x, pos.y, pos.z, madw );

  BTLV_MCSS_SetDefaultScale( bmw, position );

  MCSS_SetAnimCtrlCallBack( bmw->btlv_mcss[ position ].mcss, position, BTLV_MCSS_CallBackFunctorFrame, 1 );
}

//============================================================================================
/**
 * @brief mcss_pos_3vs3に値をセット（デバッグ用）
 *
 * @param[in] bmw   BTLV_MCSS管理ワークへのポインタ
 * @param[in] flag  セットする値
 */
//============================================================================================
void  BTLV_MCSS_SetMcss3vs3( BTLV_MCSS_WORK *bmw, int flag )
{
  bmw->mcss_pos_3vs3 = flag;
}

//============================================================================================
/**
 * @brief mcss_pos_rotateに値をセット（デバッグ用）
 *
 * @param[in] bmw   BTLV_MCSS管理ワークへのポインタ
 * @param[in] flag  セットする値
 */
//============================================================================================
void  BTLV_MCSS_SetMcssRotate( BTLV_MCSS_WORK *bmw, int flag )
{
  bmw->mcss_pos_rotate = flag;
}

//============================================================================================
/**
 * @brief 現在実行中のタスクを強制終了（デバッグ用）
 *
 * @param[in] bmw   BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_ClearExecute( BTLV_MCSS_WORK *bmw )
{

}

#endif
