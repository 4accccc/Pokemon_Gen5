
//============================================================================================
/**
 * @file  btlv_effvm.c
 * @brief 戦闘エフェクトスクリプトコマンド解析
 * @author  soga
 * @date  2008.11.21
 */
//============================================================================================

#include <gflib.h>
#include "system/main.h"
#include "system/vm_cmd.h"
#include "system/mcss.h"
#include "sound/pm_sndsys.h"
#include "item/item.h"

#include "net/dwc_rap.h"

#include "btlv_scu.h"
#include "btlv_effect.h"

#include "btlv_effvm_dat.h"

#include "arc_def.h"
#include "particle/wazaeffect/spa.naix"
#include "wazaeffect/waza_eff_gra.naix"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================

enum{
  EFFVM_PTCNO_NONE = 0xffffffff,    //ptc_noの未格納のときの値
  EFFVM_OBJNO_NONE = 0xffffffff,    //obj_noの未格納のときの値
  TEMP_WORK_SIZE = 16,          //テンポラリワークのサイズ
  BTLV_EFFVM_BG_PAL = 8,        //エフェクトBGのパレット開始位置
  EFFVM_OBJ_MAX = 4,            //エフェクト中に使用できるOBJMAX

  EFFVM_PTCNO_NO_FIND = PARTICLE_GLOBAL_MAX,

  ORTHO_WIDTH = 4,
  ORTHO_HEIGHT = 3,

  EFFVM_CHANGE_VOLUME = ( 127 * EFFVM_VOLUME_DOWN_RATIO / 100 ) << FX32_SHIFT,
  EFFVM_CHANGE_VOLUME_DOWN_FRAME = EFFVM_VOLUME_DOWN_FRAME,
  EFFVM_CHANGE_VOLUME_UP_FRAME = EFFVM_VOLUME_UP_FRAME,
  EFFVM_CHANGE_VOLUME_PV = ( 127 * EFFVM_VOLUME_DOWN_RATIO_PV / 100 ) << FX32_SHIFT,
  EFFVM_CHANGE_VOLUME_DOWN_FRAME_PV = EFFVM_VOLUME_DOWN_FRAME_PV,
  EFFVM_CHANGE_VOLUME_UP_FRAME_PV = EFFVM_VOLUME_UP_FRAME_PV,

  EFFVM_VOICEPLAYER_INDEX_NONE = 0xffffffff,

  POLYID_FIX = 5,   //パーティクルの使用するポリゴンIDの固定値
  POLYID_MIN = 6,   //パーティクルの使用するポリゴンIDのMIN
  POLYID_MAX = 54,  //パーティクルの使用するポリゴンIDのMAX
};

enum{ 
  ORTHO_MODE_OFF = 0,
  ORTHO_MODE_ON,
  ORTHO_MODE_OFFSET,
  ORTHO_MODE_EMITTER_MOVE,
};

#ifdef PM_DEBUG
#ifdef DEBUG_ONLY_FOR_sogabe
//#define DEBUG_OS_PRINT
#endif
#endif

#ifdef PM_DEBUG
#ifdef DEBUG_ONLY_FOR_kageyama_shota
#define KAGEYAMA_DEBUG
vu32  volume_down_ratio = EFFVM_VOLUME_DOWN_RATIO;
vu32  volume_down_frame = EFFVM_CHANGE_VOLUME_DOWN_FRAME;
vu32  volume_up_frame   = EFFVM_CHANGE_VOLUME_UP_FRAME;
vu32  volume_down_ratio_pv = EFFVM_VOLUME_DOWN_RATIO_PV;
vu32  volume_down_frame_pv = EFFVM_CHANGE_VOLUME_DOWN_FRAME_PV;
vu32  volume_up_frame_pv   = EFFVM_CHANGE_VOLUME_UP_FRAME_PV;
#endif
#endif

#ifdef PM_DEBUG
#if defined(DEBUG_ONLY_FOR_kageyama_shota)|\
    defined(DEBUG_ONLY_FOR_adachi_minako)|\
    defined(DEBUG_ONLY_FOR_sato_hitomi)|\
    defined(DEBUG_ONLY_FOR_ichinose)
//#define DEBUG_SE_END_PRINT
#endif
#endif

#ifdef PM_DEBUG
#if defined(DEBUG_ONLY_FOR_yoshida)|\
    defined(DEBUG_ONLY_FOR_sogabe)
//#define CAMERA_POS_PRINT
#endif
#ifdef DEBUG_ONLY_FOR_sogabe
//#define CAMERA_POS_PRINT_FX32
#endif
#endif

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

typedef struct{
  u32               position_reverse_flag :1;     //立ち位置反転フラグ
  u32               camera_ortho_on_flag  :1;     //カメラ移動後、正射影に戻すフラグ
  u32               se_play_wait_flag     :1;     //SE再生ウエイト中
  u32               se_effect_enable_flag :1;     //SEEFFECTフラグ
  u32               set_priority_flag     :1;     //PRIORITY操作されたか？
  u32               set_alpha_flag        :1;     //ALPHA操作されたか？
  u32               execute_effect_type   :1;     //起動しているエフェクトタイプ（0:技エフェクト　1:戦闘エフェクト）
  u32               pause_flag            :1;     //一時停止フラグ
  u32               window_move_flag      :1;     //ウインドウ操作フラグ
  u32               volume_down_flag      :1;     //ボリュームダウン中かどうかフラグ
  u32               volume_already_down   :1;     //すでにボリュームダウンだったフラグ
  u32               nakigoe_wait_flag     :1;     //鳴き声再生ウエイト中
  u32               se_play_flag          :1;     //SEPLAYされたかフラグ
  u32               zoom_out_flag         :1;
  u32               zoom_out_flag_work    :1;     //技シーケンスで使用するZOOM_OUT_FLAG
  u32               camera_move_ignore    :1;
  u32               particle_tex_load     :1;     //パーティクルテクスチャデータロード中フラグ
  u32               zoom_in_migawari      :1;     //カメラズームイン時にみがわり入れ替えを行ったフラグ
  u32               reverse_draw_no_check :1;     //リバースドローチェックをしない
  u32                                     :13;
  u32               sequence_work;                //シーケンスで使用する汎用ワーク

  GFL_TCBSYS*       tcbsys;
  GFL_PTC_PTR       ptc[ PARTICLE_GLOBAL_MAX ];
  int               ptc_no[ PARTICLE_GLOBAL_MAX ];
  int               spr_max[ PARTICLE_GLOBAL_MAX ];
  int               obj[ EFFVM_OBJ_MAX ];
  BtlvMcssPos       attack_pos;
  BtlvMcssPos       defence_pos;
  BtlvMcssPos       push_attack_pos[ BTLV_EFFVM_STACK_SIZE ];     //VMEC_CALLが実行されたときに退避するattack_pos
  BtlvMcssPos       push_defence_pos[ BTLV_EFFVM_STACK_SIZE ];    //VMEC_CALLが実行されたときに退避するdefence_pos
  VM_CODE*          push_sequence[ BTLV_EFFVM_STACK_SIZE ];       //VMEC_CALLが実行されたときに退避するsequence
  u32               push_sequence_work[ BTLV_EFFVM_STACK_SIZE ];  //VMEC_CALLが実行されたときに退避するsequence_work
  int               wait;
  VM_CODE*          sequence;
  VM_CODE*          migawari_sequence;
  int               migawari_table_ofs;
  void*             temp_work[ TEMP_WORK_SIZE ];
  int               temp_work_count;
  HEAPID            heapID;
  VMCMD_RESULT      control_mode;
  int               effect_end_wait_kind;
  int               camera_projection;
  BTLV_EFFVM_PARAM  param;
  WazaID            waza;
  int               ball_mode;
  int               call_count;   //サブルーチンコールした回数
  int               temp_scr_x;
  int               temp_scr_y;
  u32               voiceplayerIndex[ TEMOTI_POKEMAX ];  //鳴き声プレイヤーインデックス（念のため6体分）
  BOOL              push_camera_flag;
  VecFx32           push_camera_pos;
  VecFx32           push_camera_target;
  int               wcs_camera_work;
  int               old_eff_no;
  int               old_from;
#ifdef PM_DEBUG
  const DEBUG_PARTICLE_DATA*  dpd;
  BOOL                        debug_flag;
  BOOL                        debug_return_through;
  ARCDATID                    dat_id[ PARTICLE_GLOBAL_MAX ];
  u32                         dat_id_cnt;
  void*                       unpack_info[ PARTICLE_GLOBAL_MAX ];
  int                         camera_flag;
#endif
}BTLV_EFFVM_WORK;

typedef struct{
  VMHANDLE* vmh;
  int       src;
  int       dst;
  VecFx32   ofs;
  fx32      angle;
  fx32      top;
  int       move_type;
  int       move_frame; //移動するフレーム数
  VecFx32   src_pos;
  VecFx32   dst_pos;
  int       ortho_mode;
  fx32      radius;
  fx32      life;
  fx32      scale;
  fx32      speed;
  fx32      wave;
  BOOL      minus_flag;
}BTLV_EFFVM_EMIT_INIT_WORK;

//エミッタ移動用パラメータ構造体
typedef struct{
  MtxFx43 mtx43;      //移動座標変換用行列
  fx32    radius_x;   //放物線移動時のX方向半径
  fx32    radius_y;   //放物線移動時のY方向半径（０だと直線になるはず）
  fx32    angle;      //放物線現在角度
  fx32    wave_angle;
  fx32    speed;      //スピード
  fx32    wave_speed;
  int     move_frame; //移動するフレーム数
  int     move_type;  //移動タイプ
  int     wait;
  fx32    wait_tmp;
  fx32    wait_tmp_plus;
  int     ortho_mode;
}BTLV_EFFVM_EMITTER_MOVE_WORK;

//エミッタ円移動用パラメータ構造体
typedef struct{
  int       center;       //回転中心
  VecFx32   center_pos;   //回転中心座標
  fx32      radius_h;     //横方向半径
  fx32      radius_v;     //縦方向半径
  int       angle;        //現在角度
  int       speed;        //スピード
  int       frame;        //移動するフレーム数
  int       frame_tmp;
  int       wait;         //1フレーム移動するごとのウエイト
  int       wait_tmp;
  int       count;        //カウント
  int       after_wait;   //1回転ごとのウエイト
  int       after_wait_tmp;
  BOOL      ortho_mode;   //正射影モードかどうか
}BTLV_EFFVM_EMITTER_CIRCLE_MOVE_WORK;

//SE再生用パラメータ構造体
typedef struct
{
  BTLV_EFFVM_WORK*  bevw;
  int               se_no;
  int               player;
  int               pan;
  int               wait;
  int               pitch;
  int               vol;
  int               mod_depth;
  int               mod_speed;
}BTLV_EFFVM_SEPLAY;

//SEエフェクト用パラメータ構造体
typedef struct
{
  BTLV_EFFVM_WORK*  bevw;
  int               player;
  int               type;
  int               param;
  int               start;
  int               end;
  fx32              value;
  fx32              vec_value;
  int               start_wait;
  int               frame;
  int               frame_tmp;
  int               wait;
  int               wait_tmp;
  int               count;
}BTLV_EFFVM_SEEFFECT;

//ボリューム変化
typedef struct
{
  BTLV_EFFVM_WORK*  bevw;
  fx32              start_vol;
  fx32              end_vol;
  fx32              vec_vol;
}BTLV_EFFVM_CHANGE_VOLUME;

//鳴き声再生
typedef struct
{ 
  BTLV_EFFVM_WORK*  bevw;
  BtlvMcssPos       pos;
  int               pitch;
  int               volume;
  int               chorus_vol;
  int               chorus_speed;
  BOOL              play_dir;
  int               wait;
}BTLV_EFFVM_NAKIGOE;

//SPAデータ（SPAに含まれるsprの数を取りたい）
typedef struct
{
  u8  head[ 8 ];    //ヘッダーデータ
  u16 size;         //spaに含まれるsprの数らしい
  u8  data[ 1 ];    //実データ
}SPA_DATA;

//ウインドウ操作
typedef struct
{
  BTLV_EFFVM_WORK*  bevw;
  int               seq_no;
	int               type;
	int               horizon;
	int               vertical;
  int               in_out;
	int               frame;
	int               wait;
	int               wait_tmp;
  BOOL              flag;
}BTLV_EFFVM_WINDOW_MOVE;

//着地演出待ち操作
typedef struct
{
  BTLV_EFFVM_WORK*  bevw;
  int               side;
  int               finish_flag;
}BTLV_EFFVM_LANDING_WAIT;

typedef struct
{ 
  BTLV_EFFVM_WORK*  bevw;
}TCB_NONE_WORK;

typedef struct
{ 
  BTLV_EFFVM_WORK*  bevw;
  GFL_PTC_PTR       psys;
  void*             resource;
}TCB_PARTICLE_LOAD;

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================

VMHANDLE* BTLV_EFFVM_Init( GFL_TCBSYS *tcbsys, HEAPID heapID );
BOOL      BTLV_EFFVM_Main( VMHANDLE *vmh );
void      BTLV_EFFVM_Exit( VMHANDLE *vmh );
void      BTLV_EFFVM_Start( VMHANDLE *vmh, BtlvMcssPos from, BtlvMcssPos to, WazaID waza, BTLV_EFFVM_PARAM* param );
void      BTLV_EFFVM_Stop( VMHANDLE *vmh );
void      BTLV_EFFVM_Restart( VMHANDLE *vmh );

//エフェクトコマンド
static VMCMD_RESULT VMEC_CAMERA_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_CAMERA_MOVE_COODINATE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_CAMERA_MOVE_ANGLE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_CAMERA_SHAKE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_CAMERA_PROJECTION( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_CAMERA_POS_PUSH( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_PARTICLE_LOAD( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_PARTICLE_PLAY( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_PARTICLE_PLAY_COORDINATE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_PARTICLE_PLAY_ORTHO( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_PARTICLE_PLAY_ALL( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_PARTICLE_DELETE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_EMITTER_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_EMITTER_MOVE_COORDINATE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_EMITTER_MOVE_ORTHO( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_EMITTER_MOVE_ORTHO_COORDINATE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_EMITTER_CIRCLE_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_EMITTER_CIRCLE_MOVE_ORTHO( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_CIRCLE_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_SIN_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_SCALE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_ROTATE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_ALPHA( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_MOSAIC( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_SET_MEPACHI_FLAG( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_SET_ANM_FLAG( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_PAL_FADE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_VANISH( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_SHADOW_VANISH( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_SHADOW_SCALE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_POKEMON_DEL( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_TRAINER_SET( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_TRAINER_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_TRAINER_ANIME_SET( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_TRAINER_DEL( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BG_LOAD( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BG_SCROLL( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BG_RASTER_SCROLL( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BG_PAL_ANM( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BG_PRIORITY( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BG_ALPHA( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BG_PAL_FADE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BG_VISIBLE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_WINDOW_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_OBJ_SET( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_OBJ_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_OBJ_SCALE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_OBJ_ANIME_SET( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_OBJ_PAL_FADE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_OBJ_DEL( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_GAUGE_VANISH( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_SE_PLAY( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_SE_STOP( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_SE_PAN( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_SE_EFFECT( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_EFFECT_END_WAIT( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_WAIT( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_CONTROL_MODE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_IF( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_IF_WORK( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_MCSS_POS_CHECK( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_SET_WORK( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_GET_WORK( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_SET_PARAM( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_MIGAWARI( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_HENSHIN( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_NAKIGOE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BALL_MODE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_BALLOBJ_SET( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_CALL( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_RETURN( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_JUMP( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_PAUSE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_SEQ_JUMP( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_LANDING_WAIT( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT VMEC_REVERSE_DRAW_SET( VMHANDLE *vmh, void *context_work );

static VMCMD_RESULT VMEC_SEQ_END( VMHANDLE *vmh, void *context_work );

//VM_WAIT_FUNC群
static  BOOL  VWF_EFFECT_END_CHECK( VMHANDLE *vmh, void *context_work );
static  BOOL  VWF_WAIT_CHECK( VMHANDLE *vmh, void *context_work );
static  BOOL  VWF_PAUSE_CHECK( VMHANDLE *vmh, void *context_work );
static  BOOL  VWF_PARTICLE_LOAD_WAIT( VMHANDLE *vmh, void *context_work );

//非公開関数群
static  int           EFFVM_GetPokePosition( BTLV_EFFVM_WORK* bevw, int pos_flag, BtlvMcssPos* pos );
static  int           EFFVM_GetEmitterPosition( BTLV_EFFVM_WORK* bevw, int pos_flag, BtlvMcssPos* pos );
static  int           EFFVM_GetPosition( VMHANDLE *vmh, int pos_flag );
static  int           EFFVM_ConvPokePosition( BTLV_EFFVM_WORK* bevw, BtlvMcssPos position );
static  int           EFFVM_ConvPosition( VMHANDLE *vmh, BtlvMcssPos position );
static  BtlRule       EFFVM_GetDirPosition( VMHANDLE* vmh, int* pos );
static  BOOL          EFFVM_RegistPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID, int* ptc_no );
static  void          EFFVM_RegistSprMax( BTLV_EFFVM_WORK *bevw, int ptc_no, void* resource );
static  int           EFFVM_GetPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID );
static  int           EFFVM_GetSprMax( BTLV_EFFVM_WORK *bevw, int ptc_no );
static  void          EFFVM_InitEmitterPos( GFL_EMIT_PTR emit );
static  void          EFFVM_MoveEmitter( GFL_EMIT_PTR emit, unsigned int flag );
static  void          EFFVM_InitEmitterCircleMove( GFL_EMIT_PTR emit );
static  void          EFFVM_CircleMoveEmitter( GFL_EMIT_PTR emit, unsigned int flag );
//static  void          EFFVM_CheckLinesOver( GFL_EMIT_PTR emit, unsigned int flag );
static  void          EFFVM_DeleteEmitter( GFL_PTC_PTR ptc );
static  void          EFFVM_ChangeCameraProjection( BTLV_EFFVM_WORK *bevw );
static  void          EFFVM_SePlay( int se_no, int player, int pan, int pitch, int vol, int mod_depth, int mod_speed );
static  int           EFFVM_GetWork( BTLV_EFFVM_WORK* bevw, int param );
static  void          EFFVM_INIT_EMITTER_POS( BTLV_EFFVM_WORK* bevw, BTLV_EFFVM_EMIT_INIT_WORK* beeiw_src, int ptc_no, int index );
static  VMCMD_RESULT  EFFVM_INIT_EMITTER_CIRCLE_MOVE( VMHANDLE *vmh, void *context_work, BOOL ortho_mode );
static  void          EFFVM_CalcPosOrtho( VecFx32 *pos, VecFx32 *ofs );
static  void          MTX_MultVec44( const VecFx32 *cp_src, const MtxFx44 *cp_m, VecFx32 *p_dst, fx32 *p_w );
static  ARCDATID      EFFVM_ConvDatID( BTLV_EFFVM_WORK* bevw, ARCDATID datID );
static  void          EFFVM_ChangeVolume( BTLV_EFFVM_WORK* bevw, fx32 start_vol, fx32 end_vol, int frame );
static  int           EFFVM_GetVoicePlayerIndex( BTLV_EFFVM_WORK* bevw );
static  void          EFFVM_CheckPokePosition( BTLV_EFFVM_WORK* bevw );
static  void          EFFVM_CheckShadow( BTLV_EFFVM_WORK* bevw );

static  void          if_act( VMHANDLE* vmh, int src, int dst, int cond, int adrs );
static  void          set_mcss_scale_move( fx32 mul_value_m, fx32 mul_value_e, int frame, int wait, int count );
//static  void          check_linesover( GFL_EMIT_PTR emit, u32 flag );
static  BOOL          check_camera_work_effect( int eff_no );

//TCB関数
static  void  TCB_EFFVM_SEPLAY( GFL_TCB* tcb, void* work );
static  void  TCB_EFFVM_SEPLAY_CB( GFL_TCB* tcb );
static  void  TCB_EFFVM_SEEFFECT( GFL_TCB* tcb, void* work );
static  void  TCB_EFFVM_SEEFFECT_CB( GFL_TCB* tcb );
static  void  TCB_EFFVM_NAKIGOE( GFL_TCB* tcb, void* work );
static  void  TCB_EFFVM_NAKIGOE_CB( GFL_TCB* tcb );
static  void  TCB_EFFVM_ChangeVolume( GFL_TCB* tcb, void* work );
static  void  TCB_EFFVM_WINDOW_MOVE( GFL_TCB* tcb, void* work );
static  void  TCB_EFFVM_WINDOW_MOVE_CB( GFL_TCB* tcb );
static  void  TCB_EFFVM_NakigoeEndCheck( GFL_TCB* tcb, void* work );
static  void  TCB_EFFVM_LandingWait( GFL_TCB* tcb, void* work );
static  void  TCB_EFFVM_ParticleLoad( GFL_TCB* tcb, void* work );

#ifdef PM_DEBUG
typedef enum
{
  DPD_TYPE_PARTICLE = 0,
  DPD_TYPE_BG,
}DPD_TYPE;

//デバッグ用関数
static  u32   BTLV_EFFVM_GetDPDNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID, DPD_TYPE type );
#endif

//============================================================================================
/**
 *  データテーブル
 */
//============================================================================================
#define TBL_CNT   ( 4 * 0 )
#define TBL_AA2BB ( 4 * 1 )
#define TBL_BB2AA ( 4 * 2 )
#define TBL_A2B   ( 4 * 3 )
#define TBL_A2C   ( 4 * 4 )
#define TBL_A2D   ( 4 * 5 )
#define TBL_B2A   ( 4 * 6 )
#define TBL_B2C   ( 4 * 7 )
#define TBL_B2D   ( 4 * 8 )
#define TBL_C2A   ( 4 * 9 )
#define TBL_C2B   ( 4 * 10 )
#define TBL_C2D   ( 4 * 11 )
#define TBL_D2A   ( 4 * 12 )
#define TBL_D2B   ( 4 * 13 )
#define TBL_D2C   ( 4 * 14 )
#define TBL_MAX   ( 4 * 14 )
#define TBL_ERROR ( 0xffffffff )

static const int  script_table[ BTLV_MCSS_POS_MAX ][ BTLV_MCSS_POS_MAX ]={
  //  AA          BB          A           B           C           D           E           F
  { TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  },  //AA
  { TBL_BB2AA,  TBL_BB2AA,  TBL_ERROR,  TBL_ERROR,  TBL_ERROR,  TBL_ERROR,  TBL_ERROR,  TBL_ERROR,  },  //BB
  { TBL_ERROR,  TBL_ERROR,  TBL_AA2BB,  TBL_A2B,    TBL_A2C,    TBL_A2D,    TBL_AA2BB,  TBL_AA2BB,  },  //A
  { TBL_ERROR,  TBL_ERROR,  TBL_B2A,    TBL_AA2BB,  TBL_B2C,    TBL_B2D,    TBL_AA2BB,  TBL_AA2BB,  },  //B
  { TBL_ERROR,  TBL_ERROR,  TBL_C2A,    TBL_C2B,    TBL_AA2BB,  TBL_C2D,    TBL_AA2BB,  TBL_AA2BB,  },  //C
  { TBL_ERROR,  TBL_ERROR,  TBL_D2A,    TBL_D2B,    TBL_D2C,    TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  },  //D
  { TBL_ERROR,  TBL_ERROR,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  },  //E
  { TBL_ERROR,  TBL_ERROR,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  TBL_AA2BB,  },  //F
};

//============================================================================================
/**
 *  スクリプトテーブル
 */
//============================================================================================
static const VMCMD_FUNC btlv_effect_command_table[]={
  VMEC_CAMERA_MOVE,
  VMEC_CAMERA_MOVE_COODINATE,
  VMEC_CAMERA_MOVE_ANGLE,
  VMEC_CAMERA_SHAKE,
  VMEC_CAMERA_PROJECTION,
  VMEC_CAMERA_POS_PUSH,
  VMEC_PARTICLE_LOAD,
  VMEC_PARTICLE_PLAY,
  VMEC_PARTICLE_PLAY_COORDINATE,
  VMEC_PARTICLE_PLAY_ORTHO,
  VMEC_PARTICLE_PLAY_ALL,
  VMEC_PARTICLE_DELETE,
  VMEC_EMITTER_MOVE,
  VMEC_EMITTER_MOVE_COORDINATE,
  VMEC_EMITTER_MOVE_ORTHO,
  VMEC_EMITTER_MOVE_ORTHO_COORDINATE,
  VMEC_EMITTER_CIRCLE_MOVE,
  VMEC_EMITTER_CIRCLE_MOVE_ORTHO,
  VMEC_POKEMON_MOVE,
  VMEC_POKEMON_CIRCLE_MOVE,
  VMEC_POKEMON_SIN_MOVE,
  VMEC_POKEMON_SCALE,
  VMEC_POKEMON_ROTATE,
  VMEC_POKEMON_ALPHA,
  VMEC_POKEMON_MOSAIC,
  VMEC_POKEMON_SET_MEPACHI_FLAG,
  VMEC_POKEMON_SET_ANM_FLAG,
  VMEC_POKEMON_PAL_FADE,
  VMEC_POKEMON_VANISH,
  VMEC_POKEMON_SHADOW_VANISH,
  VMEC_POKEMON_SHADOW_SCALE,
  VMEC_POKEMON_DEL,
  VMEC_TRAINER_SET,
  VMEC_TRAINER_MOVE,
  VMEC_TRAINER_ANIME_SET,
  VMEC_TRAINER_DEL,
  VMEC_BG_LOAD,
  VMEC_BG_SCROLL,
  VMEC_BG_RASTER_SCROLL,
  VMEC_BG_PAL_ANM,
  VMEC_BG_PRIORITY,
  VMEC_BG_ALPHA,
  VMEC_BG_PAL_FADE,
  VMEC_BG_VISIBLE,
  VMEC_WINDOW_MOVE,
  VMEC_OBJ_SET,
  VMEC_OBJ_MOVE,
  VMEC_OBJ_SCALE,
  VMEC_OBJ_ANIME_SET,
  VMEC_OBJ_PAL_FADE,
  VMEC_OBJ_DEL,
  VMEC_GAUGE_VANISH,
  VMEC_SE_PLAY,
  VMEC_SE_STOP,
  VMEC_SE_PAN,
  VMEC_SE_EFFECT,
  VMEC_EFFECT_END_WAIT,
  VMEC_WAIT,
  VMEC_CONTROL_MODE,
  VMEC_IF,
  VMEC_IF_WORK,
  VMEC_MCSS_POS_CHECK,
  VMEC_SET_WORK,
  VMEC_GET_WORK,
  VMEC_SET_PARAM,
  VMEC_MIGAWARI,
  VMEC_HENSHIN,
  VMEC_NAKIGOE,
  VMEC_BALL_MODE,
  VMEC_BALLOBJ_SET,
  VMEC_CALL,
  VMEC_RETURN,
  VMEC_JUMP,
  VMEC_PAUSE,
  VMEC_SEQ_JUMP,
  VMEC_LANDING_WAIT,
  VMEC_REVERSE_DRAW_SET,

  VMEC_SEQ_END,
};

//============================================================================================
/**
 *  VMイニシャライザテーブル
 */
//============================================================================================
static  const VM_INITIALIZER  vm_init={
  BTLV_EFFVM_STACK_SIZE,                //u16 stack_size; ///<使用するスタックのサイズ
  BTLV_EFFVM_REG_SIZE,                  //u16 reg_size;   ///<使用するレジスタの数
  btlv_effect_command_table,            //const VMCMD_FUNC * command_table; ///<使用する仮想マシン命令の関数テーブル
  NELEMS( btlv_effect_command_table ),  //const u32 command_max;      ///<使用する仮想マシン命令定義の最大数
};

//============================================================================================
/**
 * @brief VM初期化
 *
 * @param[in] heapID      ヒープID
 */
//============================================================================================
VMHANDLE  *BTLV_EFFVM_Init( GFL_TCBSYS *tcbsys, HEAPID heapID )
{
  int     i;
  VMHANDLE  *vmh;
  BTLV_EFFVM_WORK *bevw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_EFFVM_WORK ) );

  //VMCMD_RESULTはenum定義なので、.sでインクルードできないためdefine定義しているラベルと一致チェックをしておく
  GF_ASSERT( VMCMD_RESULT_SUSPEND == BTLEFF_CONTROL_MODE_SUSPEND );

  bevw->heapID = heapID;
  bevw->tcbsys = tcbsys;
  bevw->control_mode = BTLEFF_CONTROL_MODE_SUSPEND;
  bevw->camera_projection = BTLEFF_CAMERA_PROJECTION_PERSPECTIVE;

  //パーティクルインデックスワーク初期化
  for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ )
  {
    bevw->ptc_no[ i ] = EFFVM_PTCNO_NONE;
  }

  //OBJインデックスワーク初期化
  for( i = 0 ; i < EFFVM_OBJ_MAX ; i++ )
  {
    bevw->obj[ i ] = EFFVM_OBJNO_NONE;
  }

  //voiceplayerIndex初期化
  for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
  {
    bevw->voiceplayerIndex[ i ] = EFFVM_VOICEPLAYER_INDEX_NONE;
  }

  vmh = VM_Create( heapID, &vm_init );
  VM_Init( vmh, bevw );

  return vmh;
}

//============================================================================================
/**
 * @brief VM初期化
 *
 * @param[in] heapID      ヒープID
 */
//============================================================================================
BOOL    BTLV_EFFVM_Main( VMHANDLE *vmh )
{
  BOOL  ret = VM_Control( vmh );
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

  if( bevw->pause_flag )
  {
    return FALSE;
  }
  if( ( ret == FALSE ) && ( bevw->sequence ) )
  {
    //HPゲージ表示
    BTLV_EFFECT_SetGaugeDrawEnable( TRUE, BTLEFF_GAUGE_ALL );

    if( bevw->execute_effect_type == EXECUTE_EFF_TYPE_WAZA )
    {
#ifdef KAGEYAMA_DEBUG
      fx32  start_vol = ( 127 * volume_down_ratio / 100 ) << FX32_SHIFT;
      int   frame = volume_up_frame;
#else
      fx32  start_vol = EFFVM_CHANGE_VOLUME;
      int   frame = EFFVM_CHANGE_VOLUME_UP_FRAME;
#endif
      fx32  end_vol = 127 << FX32_SHIFT;
      EFFVM_ChangeVolume( bevw, start_vol, end_vol, frame );
    }

    GFL_HEAP_FreeMemory( bevw->sequence );
    bevw->sequence = NULL;
  }

  return ret;
}

//============================================================================================
/**
 * @brief VM終了
 *
 * @param[in] vmh 仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void  BTLV_EFFVM_Exit( VMHANDLE *vmh )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

  BTLV_EFFVM_Stop( vmh );
  GFL_HEAP_FreeMemory ( bevw );
  VM_Delete( vmh );
}

//============================================================================================
/**
 * @brief VM起動
 *
 * @param[in] vmh 仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void  BTLV_EFFVM_Start( VMHANDLE *vmh, BtlvMcssPos from, BtlvMcssPos to, WazaID waza, BTLV_EFFVM_PARAM* param )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  int *start_ofs;
  int table_ofs;

  GF_ASSERT_MSG( bevw->sequence == NULL, "すでにエフェクトが起動中です\neffno:%d\n", bevw->waza );
//  GF_ASSERT( ( from < BTLV_MCSS_POS_MAX ) || ( from == BTLV_MCSS_POS_ERROR ) );
//  GF_ASSERT( ( to   < BTLV_MCSS_POS_MAX ) || ( to   == BTLV_MCSS_POS_ERROR ) );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("EFFVM_Start:\nEFFNO:%d\n",waza);
#endif
//  SOGABE_Printf("EFFVM_Start:\nEFFNO:%d\n",waza);

  bevw->sequence = NULL;
  BTLV_EFFECT_FreeTCBGroup( GROUP_EFFVM );

  if( param != NULL )
  {
    bevw->param = *param;
  }
  else
  {
    MI_CpuClear16( &bevw->param, sizeof( BTLV_EFFVM_PARAM ) );
  }

  bevw->attack_pos = from;
  bevw->defence_pos = to;
  bevw->camera_projection = BTLEFF_CAMERA_PROJECTION_PERSPECTIVE;
  bevw->waza = waza;

  if( bevw->waza >= BTLEFF_SINGLE_ENCOUNT_1 )
  { 
    //連続でステータス上昇下降エフェクトを行わないようにする
    if( ( ( bevw->waza == BTLEFF_STATUS_UP ) ||
          ( bevw->waza == BTLEFF_STATUS_DOWN ) ) &&
          ( bevw->old_eff_no == bevw->waza ) &&
          ( bevw->old_from == from ) )
    { 
      return;
    }
  }

  bevw->old_eff_no = bevw->waza;
  bevw->old_from = from;

  //リバース描画OFF
  //カメラワークではOFFらない
  if( check_camera_work_effect( bevw->waza ) == FALSE )
  { 
    BTLV_EFFECT_SetReverseDrawFlag( BTLV_EFFECT_REVERSE_DRAW_OFF );
  }

  if( bevw->waza < BTLEFF_SINGLE_ENCOUNT_1 )
  {
    bevw->sequence = GFL_ARC_LoadDataAlloc( ARCID_WAZAEFF_SEQ, waza, GFL_HEAP_LOWID( bevw->heapID ) );
    //へんしんとオウムがえしとしぜんのちからは、単体のエフェクトはないので、HPゲージ非表示をしない
    if( ( bevw->waza != WAZANO_HENSIN ) && ( bevw->waza != WAZANO_OUMUGAESI ) && ( bevw->waza != WAZANO_SIZENNOTIKARA ) )
    { 
      //HPゲージ非表示
      BTLV_EFFECT_SetGaugeDrawEnable( FALSE, BTLEFF_GAUGE_ALL );
    }
    //BG非表示
    GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_OFF );
    
    bevw->execute_effect_type = EXECUTE_EFF_TYPE_WAZA;
    //BGMのマスターボリュームを下げる
    {
      fx32  start_vol = 127 << FX32_SHIFT;
#ifdef KAGEYAMA_DEBUG
      fx32  end_vol = ( 127 * volume_down_ratio / 100 ) << FX32_SHIFT;
      int   frame = volume_down_frame;
#else
      fx32  end_vol = EFFVM_CHANGE_VOLUME;
      int   frame = EFFVM_CHANGE_VOLUME_DOWN_FRAME;
#endif
      EFFVM_ChangeVolume( bevw, start_vol, end_vol, EFFVM_CHANGE_VOLUME_DOWN_FRAME );
    }
  }
  else
  {
    bevw->sequence = GFL_ARC_LoadDataAlloc( ARCID_BATTLEEFF_SEQ, waza - BTLEFF_SINGLE_ENCOUNT_1, GFL_HEAP_LOWID( bevw->heapID ) );
    bevw->execute_effect_type = EXECUTE_EFF_TYPE_BATTLE;
    if( ( waza >= BTLEFF_STATUS_EFFECT_START ) && ( ( *(u16 *)(REG_BLDALPHA_ADDR) & 0x1f ) == 0 ) )
    { 
      //BG非表示
      GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_OFF );
    }
  }
#if 0
  if( ( from != BTLV_MCSS_POS_ERROR ) && ( to != BTLV_MCSS_POS_ERROR ) )
  {
    table_ofs = script_table[ from ][ to ];
    GF_ASSERT( table_ofs != TBL_ERROR );
    if( table_ofs == TBL_ERROR )
    {
      table_ofs = TBL_AA2BB;
    }
  }
  else
  {
    table_ofs = TBL_AA2BB;
  }
#endif
  table_ofs = TBL_AA2BB;

  if( param != NULL )
  { 
    //シーケンスが持っているテーブルより指定が大きいときは、0にしてふっとびを回避
    if( bevw->sequence[ TBL_CNT ] <= param->turn_count )
    { 
      param->turn_count = 0;
    }
    table_ofs += ( TBL_MAX * param->turn_count );
  }

  { 
    BOOL  migawari_flag = FALSE;

    if( BTLV_MCSS_CheckExist( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos ) )
    { 
      //みらいよちorはめつのねがいの発動ターンではみがわりチェックをしない
      if( ( ( bevw->waza == WAZANO_MIRAIYOTI ) || ( bevw->waza == WAZANO_HAMETUNONEGAI ) ) && 
            ( param->turn_count == 1 ) )
      { 
        ;
      }
      //へんしんとオウムがえしとしぜんのちからは、単体のエフェクトはないので、みがわりチェックをしない
      else if( ( bevw->waza == WAZANO_HENSIN ) ||
               ( bevw->waza == WAZANO_OUMUGAESI ) ||
               ( bevw->waza == WAZANO_SIZENNOTIKARA ) )
      { 
        ;
      }
      else
      { 
        migawari_flag = ( BTLV_MCSS_GetStatusFlag( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos ) &
                          BTLV_MCSS_STATUS_FLAG_MIGAWARI );
      }
    }

    if( bevw->waza == BTLEFF_ZOOM_IN )
    { 
      //連続攻撃でのカメラズームインのあとはリバースドローチェックをしない
      bevw->reverse_draw_no_check = 1;
    }

    if( bevw->waza == BTLEFF_ZOOM_IN_RESET )
    { 
      //連続攻撃でのカメラズームインリセットのあとはリバースドローチェックをする
      bevw->reverse_draw_no_check = 0;
    }

    //みがわりが出ているときに技エフェクトを起動するなら、みがわりを引っ込めるエフェクトを差し込む
    //連続攻撃でのカメラズームインの前に身代わり引っ込めるエフェクトを差し込む
    if( ( bevw->waza == BTLEFF_ZOOM_IN ) && ( migawari_flag ) )
    {
      bevw->migawari_sequence = bevw->sequence;
      bevw->migawari_table_ofs = table_ofs;
      bevw->sequence = GFL_ARC_LoadDataAlloc( ARCID_BATTLEEFF_SEQ, BTLEFF_MIGAWARI_WAZA_BEFORE - BTLEFF_SINGLE_ENCOUNT_1,
                                              GFL_HEAP_LOWID( bevw->heapID ) );
      bevw->execute_effect_type = EXECUTE_EFF_TYPE_BATTLE;
      bevw->zoom_in_migawari = 1;     //カメラズームイン時にみがわり入れ替えを行ったフラグ
      table_ofs = TBL_AA2BB;
    }
    //みがわりが出ているときに技エフェクトを起動するなら、みがわりを引っ込めるエフェクトを差し込む
    else if( ( bevw->execute_effect_type == EXECUTE_EFF_TYPE_WAZA ) &&
        //へんしんは技エフェクトではなくなったので除外
        ( bevw->waza != WAZANO_HENSIN ) &&
        ( bevw->zoom_in_migawari == 0 ) &&
        ( migawari_flag ) )
    {
      bevw->migawari_sequence = bevw->sequence;
      bevw->migawari_table_ofs = table_ofs;
      bevw->sequence = GFL_ARC_LoadDataAlloc( ARCID_BATTLEEFF_SEQ, BTLEFF_MIGAWARI_WAZA_BEFORE - BTLEFF_SINGLE_ENCOUNT_1,
                                              GFL_HEAP_LOWID( bevw->heapID ) );
      bevw->execute_effect_type = EXECUTE_EFF_TYPE_BATTLE;
      table_ofs = TBL_AA2BB;
    }
  }

  start_ofs = (int *)&bevw->sequence[ table_ofs ];

  //汎用ワークを初期化
  bevw->sequence_work = 0;

  //SE再生フラグを初期化
  bevw->se_play_flag = 0;

  //ボールモードを初期化
  bevw->ball_mode = BTLEFF_USE_BALL;

  VM_Start( vmh, &bevw->sequence[ (*start_ofs) ] );

}

//============================================================================================
/**
 * @brief VM強制停止
 *
 * @param[in] vmh 仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void      BTLV_EFFVM_Stop( VMHANDLE *vmh )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

  if( bevw->sequence )
  {
    VMEC_SEQ_END( vmh, VM_GetContext( vmh ) );
    GFL_HEAP_FreeMemory( bevw->sequence );
    bevw->sequence = NULL;
  }
}

//============================================================================================
/**
 * @brief VM再開
 *
 * @param[in] vmh 仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void      BTLV_EFFVM_Restart( VMHANDLE *vmh )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

  GF_ASSERT( bevw->pause_flag );

  bevw->pause_flag = 0;
}

//============================================================================================
/**
 * @brief 起動しているエフェクトNoを取得
 *
 * @param[in] vmh 仮想マシン制御構造体へのポインタ
 */
//============================================================================================
int BTLV_EFFVM_GetExecuteEffectNo( VMHANDLE *vmh )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

  if( bevw->sequence )
  {
    return bevw->waza;
  }

  return 0;
}

//============================================================================================
/**
 * @brief 起動しているエフェクトのタイプを取得
 *
 * @param[in] vmh 仮想マシン制御構造体へのポインタ
 */
//============================================================================================
BTLV_EFFVM_EXECUTE_EFF_TYPE BTLV_EFFVM_GetExecuteEffectType( VMHANDLE *vmh )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  BTLV_EFFVM_EXECUTE_EFF_TYPE type = EXECUTE_EFF_TYPE_NONE;

  if( bevw->sequence )
  {
    type = bevw->execute_effect_type;
  }

  return type;
}

//============================================================================================
/**
 * @brief BTLV_EFFVM_PARAMを初期化
 *
 * @param[out]  param クリアするBTLV_EFFVM_PARAM構造体へのポインタ
 */
//============================================================================================
void  BTLV_EFFVM_ClearParam( BTLV_EFFVM_PARAM* param )
{ 
  param->waza_range     = 0;      ///<技の効果範囲
  param->turn_count     = 0;      ///< ターンによって異なるエフェクトを出す場合のターン指定。（ex.そらをとぶ）
  param->continue_count = 0;      ///< 連続して出すとエフェクトが異なる場合の連続カウンタ（ex. ころがる）
  param->yure_cnt       = 0;      ///<ボールゆれるカウント
  param->get_success    = 0;      ///<捕獲成功かどうか
  param->get_critical   = 0;      ///<クリティカルかどうか
  param->item_no        = 0;      ///<ボールのアイテムナンバー
}

//============================================================================================
/**
 * @brief SE再生（外部からも呼び出せるようにした）
 *
 * @param[in] vmh         仮想マシン制御構造体へのポインタ
 * @param[in] se_no       再生するSEナンバー
 * @param[in] player      再生するPlayerNo
 * @param[in] pan         再生パン
 * @param[in] pitch       再生ピッチ
 * @param[in] vol         再生ボリューム
 * @param[in] mod_depth   再生モジュレーションデプス
 * @param[in] mod_speed   再生モジュレーションスピード
 * @param[in] wait        再生開始までのウエイト
 */
//============================================================================================
void  BTLV_EFFVM_SePlay( VMHANDLE* vmh, int se_no, int player, int pan, int pitch, int vol, int mod_depth, int mod_speed, int wait )
{ 
  BTLV_EFFVM_WORK*  bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

  //SEモードがMUTEならなにもせずにリターン
  if( BTLV_EFFECT_GetSEMode() == BTLV_EFFECT_SE_MODE_MUTE ) { return; }

  //SE再生フラグをセット
  bevw->se_play_flag = 1;

  if( pan == BTLEFF_SEPAN_FLAT )
  {
    pan = 0;
  }
  else if( ( pan == BTLEFF_SEPAN_L ) || ( pan == BTLEFF_SEPAN_R ) )
  { 
    pan = ( pan & 1 ) ? 127 : -128;
  }
  else
  {
    pan = ( EFFVM_GetPosition( vmh, pan ) & 1 ) ? 127 : -128;
  }

  if( mod_depth > 255 ) { mod_depth = 255; }
  if( mod_speed > 255 ) { mod_speed = 255; }

  if( wait == 0 )
  {
    EFFVM_SePlay( se_no, player, pan, pitch, vol, mod_depth, mod_speed );
  }
  else
  {
    BTLV_EFFVM_SEPLAY*  bes = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), sizeof( BTLV_EFFVM_SEPLAY ) );
    bes->bevw       = bevw;
    bes->se_no      = se_no;
    bes->player     = player;
    bes->pan        = pan;
    bes->wait       = wait;
    bes->pitch      = pitch;
    bes->vol        = vol;
    bes->mod_depth  = mod_depth;
    bes->mod_speed  = mod_speed;

    bevw->se_play_wait_flag = 1;

    BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( bevw->tcbsys, TCB_EFFVM_SEPLAY, bes, 0 ), TCB_EFFVM_SEPLAY_CB, GROUP_EFFVM );
  }
}

//============================================================================================
/**
 * @brief SE動的変化（外部からも呼び出せるようにした）
 *
 * @param[in] vmh         仮想マシン制御構造体へのポインタ
 * @param[in] player      変更するPlayerNo
 * @paran[in] type        変化タイプ
 * @param[in] param       変化させるパラメータ
 * @param[in] start       開始値
 * @param[in] end         終了値
 * @param[in] start_wait  開始ウエイト
 * @param[in] frame       startからendまでのフレーム数
 * @param[in] wait        フレーム間のウエイト
 * @param[in] count       往復回数
 */
//============================================================================================
void  BTLV_EFFVM_SeEffect( VMHANDLE* vmh, int player, int type, int param, int start, int end, int start_wait, int frame, int wait, int count )
{ 
  BTLV_EFFVM_WORK*  bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  BTLV_EFFVM_SEEFFECT*  bes = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), sizeof( BTLV_EFFVM_SEEFFECT ) );
  bes->bevw     = bevw;

  bes->player     = player;
  bes->type       = type;
  bes->param      = param;
  bes->start      = start;
  bes->end        = end;
  bes->start_wait = start_wait;
  bes->frame      = frame;
  bes->frame_tmp  = bes->frame;
  bes->wait       = 0;
  bes->wait_tmp   = wait;
  bes->count      = count * 2;

  if( ( bes->type == BTLEFF_SEEFFECT_ROUNDTRIP ) && ( bes->count == 0 ) )
  {
    bes->count = 2;
  }

  bes->value = FX32_CONST( bes->start );
  bes->vec_value = FX_Div( FX32_CONST( bes->end - bes->start ) , FX32_CONST( bes->frame ) );

  BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( bevw->tcbsys, TCB_EFFVM_SEEFFECT, bes, 0 ), TCB_EFFVM_SEEFFECT_CB, GROUP_EFFVM );

  bevw->se_effect_enable_flag = 1;
}

//============================================================================================
/**
 * @brief 鳴き声ストップ
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void  BTLV_EFFVM_StopAllPMVoice( VMHANDLE* vmh )
{ 
  BTLV_EFFVM_WORK*  bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  int i;

  for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
  { 
    if( bevw->voiceplayerIndex[ i ] != EFFVM_VOICEPLAYER_INDEX_NONE )
    { 
      PMVOICE_Stop( bevw->voiceplayerIndex[ i ] );
    }
  }
}

//============================================================================================
/**
 *  @brief  ステータス上昇下降エフェクトの連続起動抑制をクリア
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void  BTLV_EFFVM_ClearOldStatusEffectBuffer( VMHANDLE* vmh )
{ 
  BTLV_EFFVM_WORK*  bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

  bevw->old_eff_no = 0;
  bevw->old_from = 0;
}

//============================================================================================
/**
 * @brief カメラ移動
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_CAMERA_MOVE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)context_work;
  VecFx32   cam_pos,cam_target;
  //カメラタイプ読み込み
  int     cam_type = ( int )VMGetU32( vmh );
  //カメラ移動先位置を読み込み
  int     cam_move_pos = ( int )VMGetU32( vmh );
  int     frame, wait, brake;
  static  VecFx32 cam_pos_table_1vs1[]={
    { 0x00005ca6, 0x00005f33, 0x00013cc3 },       //BTLEFF_CAMERA_POS_AA
    { 0x00006994, 0x00006f33, 0x00006e79 },       //BTLEFF_CAMERA_POS_BB
  };

  
  static  VecFx32 cam_pos_table_2vs2[]={
    { 0x00002b33, 0x00005333, 0x000114cd },       //BTLEFF_CAMERA_POS_A
    { 0x00008994, 0x00006f33, 0x00006e79 },       //BTLEFF_CAMERA_POS_B
    { 0x00006b33, 0x00005333, 0x000114cd },       //BTLEFF_CAMERA_POS_C
    { 0x00004994, 0x00006f33, 0x00006e79 },       //BTLEFF_CAMERA_POS_D
  };

  static  VecFx32 cam_pos_table_3vs3[]={
    { 0x00001b33, 0x00006333, 0x000144cd },       //BTLEFF_CAMERA_POS_A
    { 0x0000b994, 0x00006f33, 0x00006e79 },       //BTLEFF_CAMERA_POS_B
    { 0x00005ca6, 0x00005f33, 0x00013cc3 },       //BTLEFF_CAMERA_POS_C
    { 0x00006994, 0x00006f33, 0x00006e79 },       //BTLEFF_CAMERA_POS_D
    { 0x00006b33, 0x00006333, 0x000144cd },       //BTLEFF_CAMERA_POS_E
    { 0x00002994, 0x00006f33, 0x00006e79 },       //BTLEFF_CAMERA_POS_F
  };

  static  VecFx32 cam_target_table_1vs1[]={
    { 0xfffff173, 0x00001d9a, 0x000027f6 },       //BTLEFF_CAMERA_POS_AA
    { 0xfffffe61, 0x00002d9a, 0xffff59ac },       //BTLEFF_CAMERA_POS_BB
  };

  static  VecFx32 cam_target_table_2vs2[]={
    { 0xffffc000, 0x0000119a, 0x00000000 },       //BTLEFF_CAMERA_POS_A
    { 0x0000219f, 0x00002d9a, 0xffff59ac },       //BTLEFF_CAMERA_POS_B
    { 0x00000000, 0x0000119a, 0x00000000 },       //BTLEFF_CAMERA_POS_C
    { 0xffffde61, 0x00002d9a, 0xffff59ac },       //BTLEFF_CAMERA_POS_D
  };

  static  VecFx32 cam_target_table_3vs3[]={
    { 0xffffb000, 0x0000219a, 0x00003000 },       //BTLEFF_CAMERA_POS_A
    { 0x0000519f, 0x00002d9a, 0xffff59ac },       //BTLEFF_CAMERA_POS_B
    { 0xfffff173, 0x00001d9a, 0x000027f6 },       //BTLEFF_CAMERA_POS_C
    { 0xfffffe61, 0x00002d9a, 0xffff59ac },       //BTLEFF_CAMERA_POS_D
    { 0x00005000, 0x0000219a, 0x00003000 },       //BTLEFF_CAMERA_POS_E
    { 0xffffbe61, 0x00002d9a, 0xffff59ac },       //BTLEFF_CAMERA_POS_F
  };

  static  VecFx32 cam_pos_zoom_out    = { 0x00008b33, 0x00007b33, 0x00017ccd };
  static  VecFx32 cam_target_zoom_out = { 0x00002000, 0x0000399a, 0x00006800 };

  static  VecFx32 cam_pos_zoom_out_rotation    = { 0x00006b33, 0x00007b33, 0x0001eccd };
  static  VecFx32 cam_target_zoom_out_rotation = { 0x00000000, 0x0000399a, 0x0000d800 };

  static  VecFx32 cam_pos_b_ortho     = { 0x00009b33, 0x00006b33, 0x000114cd };
  static  VecFx32 cam_target_b_ortho  = { 0x00003000, 0x0000299a, 0x00000000 };

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_CAMERA_MOVE\n");
  OS_TPrintf("cam_move_pos:%d push_camera_flag:%d\n",cam_move_pos,bevw->push_camera_flag);
#endif DEBUG_OS_PRINT

  //移動フレーム数を読み込み
  frame = ( int )VMGetU32( vmh );
  //移動ウエイトを読み込み
  wait = ( int )VMGetU32( vmh );
  //ブレーキフレーム数を読み込み
  brake = ( int )VMGetU32( vmh );

  //カメラ移動無視フラグが立っていたらなにもせずに終了
  if( bevw->camera_move_ignore )
  { 
    return bevw->control_mode;
  }

  //カメラ位置を退避していないのに、指定されていたら、初期位置にする
  if( ( cam_move_pos == BTLEFF_CAMERA_POS_PUSH ) && ( bevw->push_camera_flag == FALSE ) )
  { 
    cam_move_pos = BTLEFF_CAMERA_POS_INIT;
  }

  //デフォルト位置に動かないなら、投視射影モードにする
  if( ( cam_move_pos != BTLEFF_CAMERA_POS_INIT ) &&
      ( cam_move_pos != BTLEFF_CAMERA_POS_PLURAL_A ) &&
      ( cam_move_pos != BTLEFF_CAMERA_POS_PLURAL_D ) &&
      ( cam_move_pos != BTLEFF_CAMERA_POS_ZOOM_OUT ) &&
      ( cam_move_pos != BTLEFF_CAMERA_POS_INIT_ORTHO ) )
  {
    EFFVM_ChangeCameraProjection( bevw );
  }
  else
  {
    bevw->camera_ortho_on_flag = 1; //カメラ移動後、正射影に戻すフラグ
  }

  switch( cam_move_pos ){
  case BTLEFF_CAMERA_POS_PLURAL_A:
  case BTLEFF_CAMERA_POS_PLURAL_D:
    { 
      BtlRule rule = BTLV_EFFECT_GetBtlRule();
      BtlvMcssPos pos = ( cam_move_pos == BTLEFF_CAMERA_POS_PLURAL_A ) ? bevw->attack_pos : bevw->defence_pos;
      switch( rule ){ 
      case BTL_RULE_SINGLE:
        //1vs1ならカメラ移動なし
        return bevw->control_mode;
        break;
      case BTL_RULE_DOUBLE:
      case BTL_RULE_TRIPLE:
        switch( pos ){ 
        case BTLV_MCSS_POS_A:
          cam_move_pos = BTLEFF_CAMERA_POS_A;
          break;
        case BTLV_MCSS_POS_B:
          cam_move_pos = ( rule == BTL_RULE_DOUBLE ) ? BTLEFF_CAMERA_POS_INIT : BTLEFF_CAMERA_POS_B_ORTHO;
          break;
        case BTLV_MCSS_POS_C:
          cam_move_pos = ( rule == BTL_RULE_DOUBLE ) ? BTLEFF_CAMERA_POS_C : BTLEFF_CAMERA_POS_INIT;
          break;
        case BTLV_MCSS_POS_D:
          cam_move_pos = ( rule == BTL_RULE_DOUBLE ) ? BTLEFF_CAMERA_POS_INIT : BTLEFF_CAMERA_POS_INIT;
          break;
        case BTLV_MCSS_POS_E:
          cam_move_pos = ( rule == BTL_RULE_DOUBLE ) ? BTLEFF_CAMERA_POS_INIT : BTLEFF_CAMERA_POS_E;
          break;
        case BTLV_MCSS_POS_F:
        default:
          return bevw->control_mode;
          break;
        }
        break;
      case BTL_RULE_ROTATION:
        if( ( pos == BTLV_MCSS_POS_C ) ||
            ( pos == BTLV_MCSS_POS_E ) )
        { 
          cam_move_pos = BTLEFF_CAMERA_POS_ZOOM_OUT_ROTATION;
        }
        break;
      }
    }
    break;
  case BTLEFF_CAMERA_POS_ZOOM_OUT:
    { 
      BtlRule rule = BTLV_EFFECT_GetBtlRule();
      if( ( rule == BTL_RULE_SINGLE ) || ( rule == BTL_RULE_ROTATION ) )
      { 
        //1vs1orRotationならカメラ移動なし
        return bevw->control_mode;
      }
    }
    break;
  case BTLEFF_CAMERA_POS_ZOOM_OUT_PERS:
    { 
      BtlRule rule = BTLV_EFFECT_GetBtlRule();
      if( ( rule == BTL_RULE_SINGLE ) || ( rule == BTL_RULE_ROTATION ) )
      { 
        //1vs1orRotationならカメラ移動なし
        BTLV_MCSS_SetOrthoMode( BTLV_EFFECT_GetMcssWork() );
        return bevw->control_mode;
      }
      else
      { 
        cam_move_pos = BTLEFF_CAMERA_POS_ZOOM_OUT;
      }
    }
    break;
  case BTLEFF_CAMERA_POS_ZOOM_OUT_ATTACK:
    { 
      BtlRule rule = BTLV_EFFECT_GetBtlRule();
      if( ( rule == BTL_RULE_SINGLE ) || ( rule == BTL_RULE_ROTATION ) )
      { 
        //1vs1orRotationなら攻撃対象者に移動
        cam_move_pos = bevw->attack_pos;
      }
      else
      { 
        cam_move_pos = ( bevw->attack_pos & 1 ) ? BTLEFF_CAMERA_POS_BB : BTLEFF_CAMERA_POS_ZOOM_OUT;
      }
    }
    break;
  case BTLEFF_CAMERA_POS_INIT_ORTHO:
    cam_move_pos = BTLEFF_CAMERA_POS_INIT;
    break;
  case BTLEFF_CAMERA_POS_ATTACK:
  case BTLEFF_CAMERA_POS_ATTACK_PAIR:
    { 
      BtlRule rule = BTLV_EFFECT_GetBtlRule();
      cam_move_pos = bevw->attack_pos;
      if( ( rule ==  BTL_RULE_ROTATION ) &&
        ( ( cam_move_pos == BTLV_MCSS_POS_C ) || ( cam_move_pos == BTLV_MCSS_POS_E ) ) )
      { 
        cam_move_pos = BTLEFF_CAMERA_POS_ZOOM_OUT_ROTATION;
      }
    }
    break;
  case BTLEFF_CAMERA_POS_DEFENCE:
  case BTLEFF_CAMERA_POS_DEFENCE_PAIR:
    switch( bevw->param.waza_range ){
    case WAZA_TARGET_OTHER_SELECT:        ///< 通常ポケ（１体選択）
    case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方ポケ（１体選択）
    case WAZA_TARGET_FRIEND_SELECT:       ///< 自分以外の味方ポケ（１体選択）
    case WAZA_TARGET_ENEMY_SELECT:        ///< 相手側ポケ（１体選択）
      if( bevw->defence_pos != BTLV_MCSS_POS_ERROR )
      { 
        cam_move_pos = bevw->defence_pos;
      }
      else
      { 
        cam_move_pos = BTLV_MCSS_POS_AA + ( bevw->attack_pos & 1 ) ^ 1;
      }
      break;
    case WAZA_TARGET_ENEMY_RANDOM:        ///< 相手ポケ１体ランダム
    case WAZA_TARGET_UNKNOWN:             ///< ゆびをふるなど特殊型
    case WAZA_TARGET_USER:                ///< 自分のみ
    case WAZA_TARGET_ENEMY_ALL:           ///< 相手側全ポケ
      if( bevw->defence_pos != BTLV_MCSS_POS_ERROR )
      { 
        cam_move_pos = bevw->defence_pos;
        break;
      }
      /*fallthru*/
    case WAZA_TARGET_SIDE_ENEMY:          ///< 敵側陣営
      cam_move_pos = BTLEFF_CAMERA_POS_AA + ( bevw->attack_pos & 1 ) ^ 1;
      break;
    case WAZA_TARGET_FRIEND_ALL:          ///< 味方側全ポケ
    case WAZA_TARGET_SIDE_FRIEND:         ///< 自分側陣営
      cam_move_pos = BTLEFF_CAMERA_POS_AA + ( bevw->attack_pos & 1 );
      break;
    case WAZA_TARGET_OTHER_ALL:           ///< 自分以外の全ポケ
    case WAZA_TARGET_ALL:                 ///< 場に出ている全ポケ
    case WAZA_TARGET_FIELD:               ///< 場全体（天候など）
      { 
        BtlRule rule = BTLV_EFFECT_GetBtlRule();
        if( ( rule == BTL_RULE_SINGLE ) || ( rule == BTL_RULE_ROTATION ) )
        { 
          cam_move_pos = BTLEFF_CAMERA_POS_AA + ( bevw->attack_pos & 1 ) ^ 1;
        }
        else
        { 
          cam_move_pos = BTLEFF_CAMERA_POS_ZOOM_OUT;
        }
      }
      break;
    }
  }

  switch( cam_move_pos ){
  case BTLEFF_CAMERA_POS_AA:
  case BTLEFF_CAMERA_POS_BB:
    cam_pos.x = cam_pos_table_1vs1[ cam_move_pos ].x;
    cam_pos.y = cam_pos_table_1vs1[ cam_move_pos ].y;
    cam_pos.z = cam_pos_table_1vs1[ cam_move_pos ].z;
    cam_target.x = cam_target_table_1vs1[ cam_move_pos ].x;
    cam_target.y = cam_target_table_1vs1[ cam_move_pos ].y;
    cam_target.z = cam_target_table_1vs1[ cam_move_pos ].z;
    break;
  case BTLEFF_CAMERA_POS_A:
  case BTLEFF_CAMERA_POS_B:
  case BTLEFF_CAMERA_POS_C:
  case BTLEFF_CAMERA_POS_D:
    { 
      BtlRule rule = BTLV_EFFECT_GetBtlRule();
      cam_move_pos -= BTLEFF_CAMERA_POS_A;
      if( rule == BTL_RULE_ROTATION )
      { 
        cam_pos.x = cam_pos_table_1vs1[ cam_move_pos ].x;
        cam_pos.y = cam_pos_table_1vs1[ cam_move_pos ].y;
        cam_pos.z = cam_pos_table_1vs1[ cam_move_pos ].z;
        cam_target.x = cam_target_table_1vs1[ cam_move_pos ].x;
        cam_target.y = cam_target_table_1vs1[ cam_move_pos ].y;
        cam_target.z = cam_target_table_1vs1[ cam_move_pos ].z;
      }
      else if( rule == BTL_RULE_DOUBLE )
      { 
        cam_pos.x = cam_pos_table_2vs2[ cam_move_pos ].x;
        cam_pos.y = cam_pos_table_2vs2[ cam_move_pos ].y;
        cam_pos.z = cam_pos_table_2vs2[ cam_move_pos ].z;
        cam_target.x = cam_target_table_2vs2[ cam_move_pos ].x;
        cam_target.y = cam_target_table_2vs2[ cam_move_pos ].y;
        cam_target.z = cam_target_table_2vs2[ cam_move_pos ].z;
      }
      else
      { 
        cam_pos.x = cam_pos_table_3vs3[ cam_move_pos ].x;
        cam_pos.y = cam_pos_table_3vs3[ cam_move_pos ].y;
        cam_pos.z = cam_pos_table_3vs3[ cam_move_pos ].z;
        cam_target.x = cam_target_table_3vs3[ cam_move_pos ].x;
        cam_target.y = cam_target_table_3vs3[ cam_move_pos ].y;
        cam_target.z = cam_target_table_3vs3[ cam_move_pos ].z;
      }
    }
    break;
  case BTLEFF_CAMERA_POS_E:
  case BTLEFF_CAMERA_POS_F:
    cam_move_pos -= BTLEFF_CAMERA_POS_A;
    cam_pos.x = cam_pos_table_3vs3[ cam_move_pos ].x;
    cam_pos.y = cam_pos_table_3vs3[ cam_move_pos ].y;
    cam_pos.z = cam_pos_table_3vs3[ cam_move_pos ].z;
    cam_target.x = cam_target_table_3vs3[ cam_move_pos ].x;
    cam_target.y = cam_target_table_3vs3[ cam_move_pos ].y;
    cam_target.z = cam_target_table_3vs3[ cam_move_pos ].z;
    break;
  case BTLEFF_CAMERA_POS_PUSH:
    cam_pos.x     = bevw->push_camera_pos.x;
    cam_pos.y     = bevw->push_camera_pos.y;
    cam_pos.z     = bevw->push_camera_pos.z;
    cam_target.x  = bevw->push_camera_target.x;
    cam_target.y  = bevw->push_camera_target.y;
    cam_target.z  = bevw->push_camera_target.z;
    break;
  case BTLEFF_CAMERA_POS_ZOOM_OUT:
    cam_pos.x     = cam_pos_zoom_out.x;
    cam_pos.y     = cam_pos_zoom_out.y;
    cam_pos.z     = cam_pos_zoom_out.z;
    cam_target.x  = cam_target_zoom_out.x;
    cam_target.y  = cam_target_zoom_out.y;
    cam_target.z  = cam_target_zoom_out.z;
    if( bevw->camera_ortho_on_flag )
    { 
      set_mcss_scale_move( FX_F32_TO_FX32( 0.65 ), FX_F32_TO_FX32( 0.725 ), frame, wait, 0 );
      bevw->zoom_out_flag = 1;
    }
    break;
  case BTLEFF_CAMERA_POS_ZOOM_OUT_ROTATION:
    cam_pos.x     = cam_pos_zoom_out_rotation.x;
    cam_pos.y     = cam_pos_zoom_out_rotation.y;
    cam_pos.z     = cam_pos_zoom_out_rotation.z;
    cam_target.x  = cam_target_zoom_out_rotation.x;
    cam_target.y  = cam_target_zoom_out_rotation.y;
    cam_target.z  = cam_target_zoom_out_rotation.z;
    if( bevw->camera_ortho_on_flag )
    { 
      set_mcss_scale_move( FX_F32_TO_FX32( 0.65 ), FX_F32_TO_FX32( 0.725 ), frame, wait, 0 );
      bevw->zoom_out_flag = 1;
    }
    break;
  case BTLEFF_CAMERA_POS_B_ORTHO:
    cam_pos.x     = cam_pos_b_ortho.x;
    cam_pos.y     = cam_pos_b_ortho.y;
    cam_pos.z     = cam_pos_b_ortho.z;
    cam_target.x  = cam_target_b_ortho.x;
    cam_target.y  = cam_target_b_ortho.y;
    cam_target.z  = cam_target_b_ortho.z;
    break;
  case BTLEFF_CAMERA_POS_INIT:
  default:
    BTLV_CAMERA_GetDefaultCameraPosition( &cam_pos, &cam_target );
    if( bevw->zoom_out_flag )
    { 
      set_mcss_scale_move( FX_F32_TO_FX32( 1 ), FX_F32_TO_FX32( 1 ), frame, wait, 0 );
      bevw->zoom_out_flag = 0;
    }
    break;
  }

  //カメラタイプから移動先を計算
  switch( cam_type ){
  case BTLEFF_CAMERA_MOVE_DIRECT:   //ダイレクト
    BTLV_CAMERA_MoveCameraPosition( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target );
    break;
  case BTLEFF_CAMERA_MOVE_INTERPOLATION:  //追従
    BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target, frame, wait, brake );
    break;
  }

#ifdef PM_DEBUG
  bevw->camera_flag = 1;
#endif

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief カメラ移動（座標指定）
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_CAMERA_MOVE_COODINATE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)context_work;
  VecFx32   cam_pos,cam_target;
  //カメラタイプ読み込み
  int     cam_type = ( int )VMGetU32( vmh );
  int     frame, wait, brake;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_CAMERA_MOVE_COODINATE\n");
#endif DEBUG_OS_PRINT

  //カメラ移動先位置を読み込み
  cam_pos.x = ( fx32 )VMGetU32( vmh );
  cam_pos.y = ( fx32 )VMGetU32( vmh );
  cam_pos.z = ( fx32 )VMGetU32( vmh );
  cam_target.x = ( fx32 )VMGetU32( vmh );
  cam_target.y = ( fx32 )VMGetU32( vmh );
  cam_target.z = ( fx32 )VMGetU32( vmh );

  //移動フレーム数を読み込み
  frame = ( int )VMGetU32( vmh );
  //移動ウエイトを読み込み
  wait = ( int )VMGetU32( vmh );
  //ブレーキフレーム数を読み込み
  brake = ( int )VMGetU32( vmh );

  //投視射影モードにする
  EFFVM_ChangeCameraProjection( bevw );

  //カメラタイプから移動先を計算
  switch( cam_type ){
  case BTLEFF_CAMERA_MOVE_DIRECT:   //ダイレクト
    BTLV_CAMERA_MoveCameraPosition( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target );
    break;
  case BTLEFF_CAMERA_MOVE_INTERPOLATION:  //追従
    BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target, frame, wait, brake );
    break;
  case BTLEFF_CAMERA_MOVE_INTERPOLATION_RELATIVITY: //追従（相対指定）
    {
      VecFx32 now_pos, now_target;

      BTLV_CAMERA_GetCameraPosition( BTLV_EFFECT_GetCameraWork(), &now_pos, &now_target );
      VEC_Add( &now_pos, &cam_pos, &cam_pos );
      VEC_Add( &now_target, &cam_target, &cam_target );
      BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target, frame, wait, brake );
    }
    break;
  }

#ifdef PM_DEBUG
  bevw->camera_flag = 1;
#endif

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief カメラ移動（角度指定）
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_CAMERA_MOVE_ANGLE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)context_work;
  //カメラタイプ読み込み
  int   cam_type = ( int )VMGetU32( vmh );
  //カメラ角度読み込み
  int   phi = ( int )VMGetU32( vmh );
  int   theta = ( int )VMGetU32( vmh );
  //移動フレーム数を読み込み
  int   frame = ( int )VMGetU32( vmh );
  //移動ウエイトを読み込み
  int   wait = ( int )VMGetU32( vmh );
  //ブレーキフレーム数を読み込み
  int   brake = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_CAMERA_MOVE_ANGLE\n");
#endif DEBUG_OS_PRINT

  //投視射影モードにする
  EFFVM_ChangeCameraProjection( bevw );

  //カメラタイプから移動先を計算
  switch( cam_type ){
  case BTLEFF_CAMERA_MOVE_DIRECT:   //ダイレクト
    BTLV_CAMERA_MoveCameraAngle( BTLV_EFFECT_GetCameraWork(), phi, theta );
    break;
  case BTLEFF_CAMERA_MOVE_INTERPOLATION:  //追従
    {
      VecFx32 cam_pos, cam_target;
      BTLV_CAMERA_GetCameraPositionAngle( BTLV_EFFECT_GetCameraWork(), phi, theta, &cam_pos, &cam_target );
      BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target, frame, wait, brake );
    }
    break;
  }

#ifdef PM_DEBUG
  bevw->camera_flag = 1;
#endif

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief カメラゆれ
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_CAMERA_SHAKE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)context_work;
  int dir     = ( int )VMGetU32( vmh );
  fx32 value  = ( fx32 )VMGetU32( vmh );
  fx32 offset = ( fx32 )VMGetU32( vmh );
  int frame   = ( int )VMGetU32( vmh );
  int wait    = ( int )VMGetU32( vmh );
  int count   = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_CAMERA_SHAKE\n");
#endif DEBUG_OS_PRINT

  BTLV_CAMERA_Shake( BTLV_EFFECT_GetCameraWork(), dir, value, offset, frame, wait, count );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief 射影モード
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_CAMERA_PROJECTION( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)context_work;
  int proj  = ( int )VMGetU32( vmh );
  int pos   = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_CAMERA_PROJECTION\n");
#endif DEBUG_OS_PRINT

  if( pos == BTLEFF_CAMERA_PROJECTION_ALL )
  { 
    //射影モードを読み込み
    bevw->camera_projection = proj;

    EFFVM_ChangeCameraProjection( bevw );
  }
  else
  { 
    BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
    int   pos_cnt = EFFVM_GetPokePosition( bevw, BTLEFF_POKEMON_SIDE_ATTACK, pos );

    if( pos_cnt )
    { 
      int i;

      for( i = 0 ; i < pos_cnt ; i++ )
      {
        if( proj == BTLEFF_CAMERA_PROJECTION_PERSPECTIVE )
        { 
          BTLV_MCSS_ResetOrthoModeByPos( BTLV_EFFECT_GetMcssWork(), pos[ i ] );
        }
        else
        { 
          BTLV_MCSS_SetOrthoModeByPos( BTLV_EFFECT_GetMcssWork(), pos[ i ] );
        }
      }
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief		カメラ現在位置をプッシュ
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_CAMERA_POS_PUSH( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)context_work;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_CAMERA_POS_PUSH\n");
#endif DEBUG_OS_PRINT

  bevw->push_camera_flag = TRUE;
  BTLV_CAMERA_GetCameraPosition( BTLV_EFFECT_GetCameraWork(), &bevw->push_camera_pos, &bevw->push_camera_target );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief パーティクルリソースロード
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_PARTICLE_LOAD( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  void      *heap;
  void      *resource;
  ARCDATID  datID = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_PARTICLE_LOAD:%d\n",datID);
#endif DEBUG_OS_PRINT

  if( EFFVM_RegistPtcNo( bevw, datID, &ptc_no ) == TRUE )
  {
#ifdef PM_DEBUG
    //デバッグ読み込みの場合は専用のバッファからロードする
    if( bevw->debug_flag == TRUE )
    {
      u32   ofs;
      u32*  ofs_p;

      heap = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), PARTICLE_LIB_HEAP_SIZE );
      bevw->ptc[ ptc_no ] = GFL_PTC_CreateEx( heap, PARTICLE_LIB_HEAP_SIZE, FALSE, POLYID_FIX, POLYID_MIN, POLYID_MAX,
                                              GFL_HEAP_LOWID( bevw->heapID ) );
      ofs_p = (u32*)&bevw->dpd->adrs[ 0 ];
      ofs = ofs_p[ BTLV_EFFVM_GetDPDNo( bevw, datID, DPD_TYPE_PARTICLE ) ];
      resource = (void *)&bevw->dpd->adrs[ ofs ];
      GFL_PTC_SetResourceEx( bevw->ptc[ ptc_no ], resource, FALSE, GFUser_VIntr_GetTCBSYS() );
      EFFVM_RegistSprMax( bevw, ptc_no, resource );
//      BTLV_EFFECT_SetFieldAnmStopOnce();
      return bevw->control_mode;
    }
#endif
    heap = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), PARTICLE_LIB_HEAP_SIZE );
    resource = GFL_PTC_LoadArcResource( ARCID_PTC, datID, GFL_HEAP_LOWID( bevw->heapID ) );
    if( ( heap != NULL ) && ( resource != NULL ) )
    { 
      bevw->ptc[ ptc_no ] = GFL_PTC_CreateEx( heap, PARTICLE_LIB_HEAP_SIZE, FALSE, POLYID_FIX, POLYID_MIN, POLYID_MAX,
                                              GFL_HEAP_LOWID( bevw->heapID ) );
      if( bevw->ptc[ ptc_no ] )
      { 
        GFL_PTC_SetResourceSetup( bevw->ptc[ ptc_no ], resource );
        { 
          TCB_PARTICLE_LOAD* tpl = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), sizeof( TCB_PARTICLE_LOAD ) );
          tpl->bevw     = bevw;
          tpl->psys     = bevw->ptc[ ptc_no ];
          tpl->resource = resource;
          bevw->particle_tex_load = 1;
          BTLV_EFFECT_SetTCB( GFUser_VIntr_CreateTCB( TCB_EFFVM_ParticleLoad, tpl, 0 ), NULL, GROUP_EFFVM );
          VMCMD_SetWait( vmh, VWF_PARTICLE_LOAD_WAIT );
        }
        EFFVM_RegistSprMax( bevw, ptc_no, resource );
//      BTLV_EFFECT_SetFieldAnmStopOnce();
      }
    }

    //確保に失敗したときの処理
    if( bevw->ptc[ ptc_no ] == NULL )
    { 
      if( heap != NULL )
      { 
        GFL_HEAP_FreeMemory( heap );
      }
      if( resource != NULL )
      { 
        GFL_HEAP_FreeMemory( resource );
      }
      bevw->ptc_no[ ptc_no ] = EFFVM_PTCNO_NONE;
    }
  }

  return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 * @brief パーティクル再生
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_PARTICLE_PLAY( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw_src = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                    sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );
  int       index   = ( int )VMGetU32( vmh );
  int       dummy;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_PARTICLE_PLAY\n");
#endif DEBUG_OS_PRINT

  beeiw_src->vmh    = vmh;
  beeiw_src->src    = ( int )VMGetU32( vmh );
  beeiw_src->dst    = ( int )VMGetU32( vmh );
  beeiw_src->ofs.x  = 0;
  beeiw_src->ofs.y  = ( fx32 )VMGetU32( vmh );
  beeiw_src->ofs.z  = 0;
  beeiw_src->angle  = ( fx32 )VMGetU32( vmh );
  //ダミーデータがあるので空読み；
  dummy = VMGetU32( vmh );
  beeiw_src->radius = ( fx32 )VMGetU32( vmh );
  beeiw_src->life   = ( fx32 )VMGetU32( vmh );
  beeiw_src->scale  = ( fx32 )VMGetU32( vmh );
  beeiw_src->speed  = ( fx32 )VMGetU32( vmh );

  if( beeiw_src->dst == BTLEFF_PARTICLE_PLAY_SIDE_NONE )
  {
    beeiw_src->dst = beeiw_src->src;
  }

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    EFFVM_INIT_EMITTER_POS( bevw, beeiw_src, ptc_no, index );
  }

  GFL_HEAP_FreeMemory( beeiw_src );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief パーティクル再生（座標指定）
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_PARTICLE_PLAY_COORDINATE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );
  int       index   = ( int )VMGetU32( vmh );
  int       dummy;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_PARTICLE_PLAY_COORDINATE\n");
#endif DEBUG_OS_PRINT

  beeiw->vmh        = vmh;
  beeiw->src        = BTLEFF_PARTICLE_PLAY_SIDE_NONE;
  beeiw->dst        = BTLEFF_PARTICLE_PLAY_SIDE_NONE;
  beeiw->src_pos.x  = ( fx32 )VMGetU32( vmh );
  beeiw->src_pos.y  = ( fx32 )VMGetU32( vmh );
  beeiw->src_pos.z  = ( fx32 )VMGetU32( vmh );
  beeiw->dst_pos.x  = ( fx32 )VMGetU32( vmh );
  beeiw->dst_pos.y  = ( fx32 )VMGetU32( vmh );
  beeiw->dst_pos.z  = ( fx32 )VMGetU32( vmh );
  beeiw->ofs.x      = 0;
  beeiw->ofs.y      = ( fx32 )VMGetU32( vmh );
  beeiw->ofs.z      = 0;
  beeiw->angle      = ( fx32 )VMGetU32( vmh );
  //ダミーデータがあるので空読み；
  dummy = VMGetU32( vmh );
  beeiw->radius     = ( fx32 )VMGetU32( vmh );
  beeiw->life       = ( fx32 )VMGetU32( vmh );
  beeiw->scale      = ( fx32 )VMGetU32( vmh );
  beeiw->speed      = ( fx32 )VMGetU32( vmh );

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    if( GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw ) == PTC_NON_CREATE_EMITTER )
    {
      GFL_HEAP_FreeMemory( beeiw );
    }
  }
  else
  { 
    GFL_HEAP_FreeMemory( beeiw );
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief パーティクル再生（正射影）
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_PARTICLE_PLAY_ORTHO( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw_src = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                    sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );
  int       index   = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_PARTICLE_PLAY\n");
#endif DEBUG_OS_PRINT

  beeiw_src->vmh      = vmh;
  beeiw_src->src      = ( int )VMGetU32( vmh );
  beeiw_src->dst      = ( int )VMGetU32( vmh );
  beeiw_src->ofs.x    = ( fx32 )VMGetU32( vmh );
  beeiw_src->ofs.y    = ( fx32 )VMGetU32( vmh );
  beeiw_src->ofs.z    = ( fx32 )VMGetU32( vmh );
  beeiw_src->radius   = ( fx32 )VMGetU32( vmh );
  beeiw_src->life     = ( fx32 )VMGetU32( vmh );
  beeiw_src->scale    = ( fx32 )VMGetU32( vmh );
  beeiw_src->speed    = ( fx32 )VMGetU32( vmh );
  beeiw_src->ortho_mode = ORTHO_MODE_ON;

  if( beeiw_src->src == BTLEFF_PARTICLE_PLAY_SIDE_ATTACKOFS )
  {
    beeiw_src->src = BTLEFF_PARTICLE_PLAY_SIDE_ATTACK;
    beeiw_src->ortho_mode = ORTHO_MODE_OFFSET;
  }

  if( beeiw_src->dst == BTLEFF_PARTICLE_PLAY_SIDE_NONE )
  {
    beeiw_src->dst = beeiw_src->src;
  }

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    GFL_G3D_PROJECTION  proj;
    VecFx32 Eye    = { 0, 0, 0 };          // Eye position
    VecFx32 vUp    = { 0, FX32_ONE, 0 };  // Up
    VecFx32 at     = { 0, 0, -FX32_ONE }; // Viewpoint

    proj.type = GFL_G3D_PRJORTH;
    proj.param1 = FX32_CONST( ORTHO_HEIGHT );
    proj.param2 = -FX32_CONST( ORTHO_HEIGHT );
    proj.param3 = -FX32_CONST( ORTHO_WIDTH );
    proj.param4 = FX32_CONST( ORTHO_WIDTH );
    proj.near   = FX32_ONE * 1;
    proj.far    = FX32_ONE * 512;
    proj.scaleW = FX32_ONE;

    if( GFL_PTC_GetCameraPtr( bevw->ptc[ ptc_no ] ) == NULL )
    {
      GFL_PTC_PersonalCameraCreate( bevw->ptc[ ptc_no ], &proj, DEFAULT_PERSP_WAY, &Eye, &vUp, &at,
                                    GFL_HEAP_LOWID( bevw->heapID ) );
    }
    EFFVM_INIT_EMITTER_POS( bevw, beeiw_src, ptc_no, index );
  }

  GFL_HEAP_FreeMemory( beeiw_src );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief パーティクル再生すべて
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_PARTICLE_PLAY_ALL( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw_src = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                    sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );
  int       spr_max = EFFVM_GetSprMax( bevw, ptc_no );
  int       param_proj, index;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_PARTICLE_PLAY_ALL\n");
#endif DEBUG_OS_PRINT

  beeiw_src->vmh    = vmh;
  beeiw_src->src    = ( int )VMGetU32( vmh );
  beeiw_src->dst    = ( int )VMGetU32( vmh );
  beeiw_src->ofs.x  = 0;
  beeiw_src->ofs.y  = ( fx32 )VMGetU32( vmh );
  beeiw_src->ofs.z  = 0;
  beeiw_src->angle  = ( fx32 )VMGetU32( vmh );
  param_proj = VMGetU32( vmh );
  beeiw_src->radius = ( fx32 )VMGetU32( vmh );
  beeiw_src->life   = ( fx32 )VMGetU32( vmh );
  beeiw_src->scale  = ( fx32 )VMGetU32( vmh );
  beeiw_src->speed  = ( fx32 )VMGetU32( vmh );

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    if( param_proj )
    {
      GFL_G3D_PROJECTION  proj;
      VecFx32 Eye    = { 0, 0, 0 };          // Eye position
      VecFx32 vUp    = { 0, FX32_ONE, 0 };  // Up
      VecFx32 at     = { 0, 0, -FX32_ONE }; // Viewpoint
  
      proj.type = GFL_G3D_PRJORTH;
      proj.param1 = FX32_CONST( ORTHO_HEIGHT );
      proj.param2 = -FX32_CONST( ORTHO_HEIGHT );
      proj.param3 = -FX32_CONST( ORTHO_WIDTH );
      proj.param4 = FX32_CONST( ORTHO_WIDTH );
      proj.near   = FX32_ONE * 1;
      proj.far    = FX32_ONE * 512;
      proj.scaleW = FX32_ONE;

      if( GFL_PTC_GetCameraPtr( bevw->ptc[ ptc_no ] ) == NULL )
      {
        GFL_PTC_PersonalCameraCreate( bevw->ptc[ ptc_no ], &proj, DEFAULT_PERSP_WAY, &Eye, &vUp, &at,
                                      GFL_HEAP_LOWID( bevw->heapID ) );
      }
      beeiw_src->ortho_mode = ORTHO_MODE_OFFSET;
    }

    if( beeiw_src->dst == BTLEFF_PARTICLE_PLAY_SIDE_NONE )
    {
      beeiw_src->dst = beeiw_src->src;
    }

    for( index = 0 ; index < spr_max ; index++ )
    {
      EFFVM_INIT_EMITTER_POS( bevw, beeiw_src, ptc_no, index );
    }
  }

  GFL_HEAP_FreeMemory( beeiw_src );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief パーティクル削除
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_PARTICLE_DELETE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_PARTICLE_DELETE\n");
#endif DEBUG_OS_PRINT

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    EFFVM_DeleteEmitter( bevw->ptc[ ptc_no ] );
    bevw->ptc[ ptc_no ] = NULL;
    bevw->ptc_no[ ptc_no ] = EFFVM_PTCNO_NONE;
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief エミッタ移動
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_EMITTER_MOVE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );
  int       index   = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_EMITTER_MOVE\n");
#endif DEBUG_OS_PRINT

  beeiw->vmh        = vmh;
  beeiw->move_type  = ( int )VMGetU32( vmh );
  beeiw->src        = ( int )VMGetU32( vmh );
  beeiw->dst        = ( int )VMGetU32( vmh );
  beeiw->ofs.x      = 0;
  beeiw->ofs.y      = ( fx32 )VMGetU32( vmh );
  beeiw->ofs.z      = 0;
  beeiw->move_frame = ( int )VMGetU32( vmh );
  beeiw->top        = ( fx32 )VMGetU32( vmh );
  beeiw->radius     = FX32_ONE;
  beeiw->life       = ( fx32 )VMGetU32( vmh );
  beeiw->scale      = FX32_ONE;
  beeiw->speed      = ( fx32 )VMGetU32( vmh );
  beeiw->wave       = ( fx32 )VMGetU32( vmh );
  if( beeiw->wave == 0 )
  { 
    beeiw->wave = 1;
  }

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    //移動元と移動先が同一のときは、アサートで止める
    GF_ASSERT( beeiw->dst != beeiw->src );

    if( GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw ) == PTC_NON_CREATE_EMITTER )
    {
      GFL_HEAP_FreeMemory( beeiw );
    }
  }
  else
  { 
    GFL_HEAP_FreeMemory( beeiw );
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief エミッタ移動（座標指定）
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_EMITTER_MOVE_COORDINATE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );
  int       index   = ( int )VMGetU32( vmh );
  int       dummy;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_EMITTER_MOVE_COORDINATE\n");
#endif DEBUG_OS_PRINT

  beeiw->vmh        = vmh;
  beeiw->move_type  = ( int )VMGetU32( vmh );
  beeiw->src        = BTLEFF_PARTICLE_PLAY_SIDE_NONE;
  beeiw->src_pos.x  = ( fx32 )VMGetU32( vmh );
  beeiw->src_pos.y  = ( fx32 )VMGetU32( vmh );
  beeiw->src_pos.z  = ( fx32 )VMGetU32( vmh );
  beeiw->dst        = ( int )VMGetU32( vmh );
  beeiw->ofs.x      = 0;
  beeiw->ofs.y      = ( fx32 )VMGetU32( vmh );
  beeiw->ofs.z      = 0;
  beeiw->move_frame = ( int )VMGetU32( vmh );
  beeiw->top        = ( fx32 )VMGetU32( vmh );
  beeiw->radius     = FX32_ONE;
  beeiw->life       = ( fx32 )VMGetU32( vmh );
  beeiw->scale      = FX32_ONE;
  beeiw->speed      = ( fx32 )VMGetU32( vmh );
  beeiw->wave       = ( fx32 )VMGetU32( vmh );
  if( beeiw->wave == 0 )
  { 
    beeiw->wave = 1;
  }

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    //移動元と移動先が同一のときは、アサートで止める
    GF_ASSERT( beeiw->dst != beeiw->src );

    if( GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw ) == PTC_NON_CREATE_EMITTER )
    {
      GFL_HEAP_FreeMemory( beeiw );
    }
  }
  else
  { 
    GFL_HEAP_FreeMemory( beeiw );
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief エミッタ移動（正射影）
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_EMITTER_MOVE_ORTHO( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );
  int       index   = ( int )VMGetU32( vmh );
  int       dummy;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_EMITTER_MOVE_ORTHO\n");
#endif DEBUG_OS_PRINT

  beeiw->vmh        = vmh;
  beeiw->move_type  = ( int )VMGetU32( vmh );
  beeiw->src        = ( int )VMGetU32( vmh );
  beeiw->dst        = ( int )VMGetU32( vmh );
  beeiw->ofs.x      = 0;
  beeiw->ofs.y      = ( fx32 )VMGetU32( vmh );
  beeiw->ofs.z      = 0;
  beeiw->move_frame = ( int )VMGetU32( vmh );
  beeiw->top        = ( fx32 )VMGetU32( vmh );
  beeiw->radius     = FX_F32_TO_FX32( 0.75f );
  beeiw->life       = ( fx32 )VMGetU32( vmh );
  beeiw->scale      = FX_F32_TO_FX32( 0.75f );
  beeiw->speed      = ( fx32 )VMGetU32( vmh );
  beeiw->wave       = ( fx32 )VMGetU32( vmh );
  beeiw->ortho_mode = ORTHO_MODE_EMITTER_MOVE;
  if( beeiw->wave == 0 )
  { 
    beeiw->wave = 1;
  }

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    GFL_G3D_PROJECTION  proj;
    VecFx32 Eye    = { 0, 0, 0 };          // Eye position
    VecFx32 vUp    = { 0, FX32_ONE, 0 };  // Up
    VecFx32 at     = { 0, 0, -FX32_ONE }; // Viewpoint

    proj.type = GFL_G3D_PRJORTH;
    proj.param1 = FX32_CONST( ORTHO_HEIGHT );
    proj.param2 = -FX32_CONST( ORTHO_HEIGHT );
    proj.param3 = -FX32_CONST( ORTHO_WIDTH );
    proj.param4 = FX32_CONST( ORTHO_WIDTH );
    proj.near   = FX32_ONE * 1;
    proj.far    = FX32_ONE * 512;
    proj.scaleW = FX32_ONE;

    if( GFL_PTC_GetCameraPtr( bevw->ptc[ ptc_no ] ) == NULL )
    {
      GFL_PTC_PersonalCameraCreate( bevw->ptc[ ptc_no ], &proj, DEFAULT_PERSP_WAY, &Eye, &vUp, &at,
                                    GFL_HEAP_LOWID( bevw->heapID ) );
    }

    if( GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw ) == PTC_NON_CREATE_EMITTER )
    {
      GFL_HEAP_FreeMemory( beeiw );
    }
  }
  else
  { 
    GFL_HEAP_FreeMemory( beeiw );
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief エミッタ移動（正射影座標指定）
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_EMITTER_MOVE_ORTHO_COORDINATE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );
  int       index   = ( int )VMGetU32( vmh );
  int       dummy;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_EMITTER_MOVE_ORTHO_COORDINATE\n");
#endif DEBUG_OS_PRINT

  beeiw->vmh        = vmh;
  beeiw->move_type  = ( int )VMGetU32( vmh );
  beeiw->src        = BTLEFF_PARTICLE_PLAY_SIDE_NONE;
  beeiw->src_pos.x  = ( fx32 )VMGetU32( vmh );
  beeiw->src_pos.y  = ( fx32 )VMGetU32( vmh );
  beeiw->src_pos.z  = ( fx32 )VMGetU32( vmh );
  beeiw->dst        = ( int )VMGetU32( vmh );
  beeiw->ofs.x      = 0;
  beeiw->ofs.y      = ( fx32 )VMGetU32( vmh );
  beeiw->ofs.z      = 0;
  beeiw->move_frame = ( int )VMGetU32( vmh );
  beeiw->top        = ( fx32 )VMGetU32( vmh );
  beeiw->radius     = FX32_ONE;
  beeiw->life       = ( fx32 )VMGetU32( vmh );
  beeiw->speed      = ( fx32 )VMGetU32( vmh );
  beeiw->scale      = ( fx32 )VMGetU32( vmh );
  beeiw->ortho_mode = ORTHO_MODE_EMITTER_MOVE;

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    GFL_G3D_PROJECTION  proj;
    VecFx32 Eye    = { 0, 0, 0 };          // Eye position
    VecFx32 vUp    = { 0, FX32_ONE, 0 };  // Up
    VecFx32 at     = { 0, 0, -FX32_ONE }; // Viewpoint

    proj.type = GFL_G3D_PRJORTH;
    proj.param1 = FX32_CONST( ORTHO_HEIGHT );
    proj.param2 = -FX32_CONST( ORTHO_HEIGHT );
    proj.param3 = -FX32_CONST( ORTHO_WIDTH );
    proj.param4 = FX32_CONST( ORTHO_WIDTH );
    proj.near   = FX32_ONE * 1;
    proj.far    = FX32_ONE * 512;
    proj.scaleW = FX32_ONE;

    if( GFL_PTC_GetCameraPtr( bevw->ptc[ ptc_no ] ) == NULL )
    {
      GFL_PTC_PersonalCameraCreate( bevw->ptc[ ptc_no ], &proj, DEFAULT_PERSP_WAY, &Eye, &vUp, &at,
                                    GFL_HEAP_LOWID( bevw->heapID ) );
    }

    if( GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw ) == PTC_NON_CREATE_EMITTER )
    {
      GFL_HEAP_FreeMemory( beeiw );
    }
  }
  else
  { 
    GFL_HEAP_FreeMemory( beeiw );
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief エミッタ円移動
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_EMITTER_CIRCLE_MOVE( VMHANDLE *vmh, void *context_work )
{
  return EFFVM_INIT_EMITTER_CIRCLE_MOVE( vmh, context_work, FALSE );
}

//============================================================================================
/**
 * @brief エミッタ円移動（正射影）
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_EMITTER_CIRCLE_MOVE_ORTHO( VMHANDLE *vmh, void *context_work )
{
  return EFFVM_INIT_EMITTER_CIRCLE_MOVE( vmh, context_work, TRUE );
}

//============================================================================================
/**
 * @brief ポケモン移動
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_MOVE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int   type;
  VecFx32 move_pos;
  int   frame;
  int   wait;
  int   count;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_MOVE\n");
#endif DEBUG_OS_PRINT

  type        = ( int )VMGetU32( vmh );
  move_pos.x  = ( fx32 )VMGetU32( vmh );
  move_pos.y  = ( fx32 )VMGetU32( vmh );
  move_pos.z  = 0;
  frame       = ( int )VMGetU32( vmh );
  wait        = ( int )VMGetU32( vmh );
  count       = ( int )VMGetU32( vmh );

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_MovePosition( BTLV_EFFECT_GetMcssWork(), pos[ i ], type, &move_pos, frame, wait, count );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモン円運動
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_CIRCLE_MOVE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_MCSS_MOVE_CIRCLE_PARAM bmmcp;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_CIRCLE_MOVE\n");
#endif DEBUG_OS_PRINT

  bmmcp.axis              = ( int )VMGetU32( vmh );
  bmmcp.shift             = ( int )VMGetU32( vmh );
  bmmcp.radius_h          = ( fx32 )VMGetU32( vmh );
  bmmcp.radius_v          = ( fx32 )VMGetU32( vmh );
  bmmcp.frame             = ( int )VMGetU32( vmh ) >> FX32_SHIFT;
  bmmcp.rotate_wait       = ( int )VMGetU32( vmh ) >> FX32_SHIFT;
  bmmcp.count             = ( int )VMGetU32( vmh ) >> FX32_SHIFT;
  bmmcp.rotate_after_wait = ( int )VMGetU32( vmh );

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    if( bmmcp.count )
    {
      int i;

      for( i = 0 ; i < pos_cnt ; i++ )
      {
        bmmcp.position = pos[ i ];
        BTLV_MCSS_MoveCircle( BTLV_EFFECT_GetMcssWork(), &bmmcp );
      }
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモンSIN運動
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_SIN_MOVE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_MCSS_MOVE_SIN_PARAM bmmsp;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int   dir           = ( int )VMGetU32( vmh );
  fx32  start_angle   = ( fx32 )VMGetU32( vmh );
  fx32  end_angle     = ( fx32 )VMGetU32( vmh );
  fx32  radius        = ( fx32 )VMGetU32( vmh );
  int   frame         = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_SIN_MOVE\n");
#endif DEBUG_OS_PRINT

  bmmsp.dir     = dir;
  bmmsp.angle   = start_angle;
  bmmsp.radius  = radius;
  bmmsp.frame   = frame;
  bmmsp.speed   = FX_Div( ( end_angle - start_angle ), ( frame << FX32_SHIFT ) );

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      bmmsp.position = pos[ i ];
      BTLV_MCSS_MoveSin( BTLV_EFFECT_GetMcssWork(), &bmmsp );
    }
  }

  return bevw->control_mode;
}


//============================================================================================
/**
 * @brief ポケモン拡縮
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_SCALE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int   type;
  VecFx32 scale;
  int   frame;
  int   wait;
  int   count;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_SCALE\n");
#endif DEBUG_OS_PRINT

  type    = ( int )VMGetU32( vmh );
  scale.x = ( fx32 )VMGetU32( vmh );
  scale.y = ( fx32 )VMGetU32( vmh );
  scale.z = FX32_ONE;
  frame   = ( int )VMGetU32( vmh );
  wait    = ( int )VMGetU32( vmh );
  count   = ( int )VMGetU32( vmh );

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_MoveScale( BTLV_EFFECT_GetMcssWork(), pos[ i ], type, &scale, frame, wait, count );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモン回転
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_ROTATE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int   type;
  VecFx32 rotate;
  int   frame;
  int   wait;
  int   count;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_ROTATE\n");
#endif DEBUG_OS_PRINT

  type     = ( int )VMGetU32( vmh );
  rotate.x   = 0;
  rotate.y   = 0;
  rotate.z   = ( fx32 )VMGetU32( vmh );
  frame    = ( int )VMGetU32( vmh );
  wait     = ( int )VMGetU32( vmh );
  count    = ( int )VMGetU32( vmh );

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_MoveRotate( BTLV_EFFECT_GetMcssWork(), pos[ i ], type, &rotate, frame, wait, count );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモンα値
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_ALPHA( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int type;
  int alpha;
  int frame;
  int wait;
  int count;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_ALPHA\n");
#endif DEBUG_OS_PRINT

  type  = ( int )VMGetU32( vmh );
  alpha = ( int )VMGetU32( vmh );
  frame = ( int )VMGetU32( vmh );
  wait  = ( int )VMGetU32( vmh );
  count = ( int )VMGetU32( vmh );

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_MoveAlpha( BTLV_EFFECT_GetMcssWork(), pos[ i ], type, alpha, frame, wait, count );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモンモザイク
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_MOSAIC( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int type;
  int mosaic;
  int frame;
  int wait;
  int count;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_MOSAIC\n");
#endif DEBUG_OS_PRINT

  type    = ( int )VMGetU32( vmh );
  mosaic  = ( int )VMGetU32( vmh );
  frame   = ( int )VMGetU32( vmh );
  wait    = ( int )VMGetU32( vmh );
  count   = ( int )VMGetU32( vmh );

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_MoveMosaic( BTLV_EFFECT_GetMcssWork(), pos[ i ], type, mosaic, frame, wait, count );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモンメパチフラグセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_SET_MEPACHI_FLAG( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int   type;
  int   wait;
  int   count;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_SET_MEPACHI_FLAG\n");
#endif DEBUG_OS_PRINT

  type     = ( int )VMGetU32( vmh );
  wait     = ( int )VMGetU32( vmh );
  count    = ( int )VMGetU32( vmh );

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;
    if( count == 0 )
    {
      count = 1;
    }
    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_MoveBlink( BTLV_EFFECT_GetMcssWork(), pos[ i ], type, wait, count );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモンアニメーションフラグセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_SET_ANM_FLAG( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int flag    = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_SET_ANM_FLAG\n");
#endif DEBUG_OS_PRINT

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_SetAnmStopFlag( BTLV_EFFECT_GetMcssWork(), pos[ i ], flag );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモンパレットフェードセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_PAL_FADE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt         = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int   start_evy       = ( int )VMGetU32( vmh );
  int   end_evy         = ( int )VMGetU32( vmh );
  int   wait            = ( int )VMGetU32( vmh );
  int   rgb             = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_PAL_FADE\n");
#endif DEBUG_OS_PRINT

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_SetPaletteFade( BTLV_EFFECT_GetMcssWork(), pos[ i ], start_evy, end_evy, wait, rgb );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモンバニッシュフラグセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_VANISH( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt   = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int   flag      = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_VANISH\n");
#endif DEBUG_OS_PRINT
  if( flag == BTLV_MCSS_EFFECT_VANISH_POP )
  { 
    BTLV_MCSS_PopVanishFlag( BTLV_EFFECT_GetMcssWork() );
  }
  else if( pos_cnt )
  //立ち位置情報がないときは、コマンド実行しない
  {
    int i;
    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_SetVanishFlag( BTLV_EFFECT_GetMcssWork(), pos[ i ], flag );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモン影バニッシュフラグセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_SHADOW_VANISH( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt   = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int   flag      = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_SHADOW_VANISH\n");
#endif DEBUG_OS_PRINT

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_SetShadowVanishFlag( BTLV_EFFECT_GetMcssWork(), pos[ i ], flag );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモン影拡縮
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_SHADOW_SCALE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int   type;
  VecFx32 scale;
  int   frame;
  int   wait;
  int   count;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_SCALE\n");
#endif DEBUG_OS_PRINT

  type    = ( int )VMGetU32( vmh );
  scale.x = ( fx32 )VMGetU32( vmh );
  scale.y = ( fx32 )VMGetU32( vmh );
  scale.z = FX32_ONE;
  frame   = ( int )VMGetU32( vmh );
  wait    = ( int )VMGetU32( vmh );
  count   = ( int )VMGetU32( vmh );

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_MoveShadowScale( BTLV_EFFECT_GetMcssWork(), pos[ i ], type, &scale, frame, wait, count );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモン削除
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_POKEMON_DEL( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int   pos_cnt = EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_POKEMON_DEL\n");
#endif DEBUG_OS_PRINT

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_EFFECT_DelPokemon( pos[ i ] );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief トレーナーセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_TRAINER_SET( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int   index = ( int )VMGetU32( vmh );
  int   position = EFFVM_GetPosition( vmh, ( int )VMGetU32( vmh ) );
  int   pos_x = ( int )VMGetU32( vmh );
  int   pos_y = ( int )VMGetU32( vmh );
  int   pos_z = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_TRAINER_SET\n");
#endif DEBUG_OS_PRINT

  if( index == BTLEFF_TRTYPE_FROM_WORK )
  {
    index = bevw->sequence_work;
  }
  BTLV_EFFECT_SetTrainer( index, position , pos_x, pos_y, pos_z );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief トレーナー移動
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_TRAINER_MOVE( VMHANDLE *vmh, void *context_work )
{
#if 0
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int           index;
  int           type;
  GFL_CLACTPOS  move_pos;
  int           frame;
  int           wait;
  int           count;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_TRAINER_MOVE\n");
#endif DEBUG_OS_PRINT

  index = BTLV_EFFECT_GetTrainerIndex( ( int )VMGetU32( vmh ) );
  type     = ( int )VMGetU32( vmh );
  move_pos.x = ( int )VMGetU32( vmh );
  move_pos.y = ( int )VMGetU32( vmh );
  frame    = ( int )VMGetU32( vmh );
  wait     = ( int )VMGetU32( vmh );
  count    = ( int )VMGetU32( vmh );

  BTLV_CLACT_MovePosition( BTLV_EFFECT_GetCLWK(), index, type, &move_pos, frame, wait, count );

  return bevw->control_mode;
#else
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int           position = BTLV_EFFECT_GetTrainerIndex( ( int )VMGetU32( vmh ) );
  int           type;
  VecFx32       move_pos;
  int           frame;
  int           wait;
  int           count;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_TRAINER_MOVE\n");
#endif DEBUG_OS_PRINT

  type     = ( int )VMGetU32( vmh );
  move_pos.x = ( int )VMGetU32( vmh );
  move_pos.y = ( int )VMGetU32( vmh );
  move_pos.z = ( int )VMGetU32( vmh );
  frame    = ( int )VMGetU32( vmh );
  wait     = ( int )VMGetU32( vmh );
  count    = ( int )VMGetU32( vmh );

  BTLV_MCSS_MovePosition( BTLV_EFFECT_GetMcssWork(), position, type, &move_pos, frame, wait, count );

  return bevw->control_mode;
#endif
}

//============================================================================================
/**
 * @brief トレーナーアニメセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_TRAINER_ANIME_SET( VMHANDLE *vmh, void *context_work )
{
#if 0
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int index = BTLV_EFFECT_GetTrainerIndex( ( int )VMGetU32( vmh ) );
  int anm_no = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_TRAINER_ANIME_SET\n");
#endif DEBUG_OS_PRINT

  BTLV_CLACT_SetAnime( BTLV_EFFECT_GetCLWK(), index, anm_no );

  return bevw->control_mode;
#else
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int position  = ( int )VMGetU32( vmh );
  int anm_no    = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_TRAINER_ANIME_SET\n");
#endif DEBUG_OS_PRINT

  if( BTLV_MCSS_SetAnime( BTLV_EFFECT_GetMcssWork(), position, anm_no ) == TRUE )
  { 
    BTLV_MCSS_SetAnimeEndCheck( BTLV_EFFECT_GetMcssWork(), position );
  }

  return bevw->control_mode;
#endif
}

//============================================================================================
/**
 * @brief トレーナー削除
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_TRAINER_DEL( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int position = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_TRAINER_DEL\n");
#endif DEBUG_OS_PRINT

  BTLV_EFFECT_DelTrainer( position );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief BGのロード
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BG_LOAD( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  ARCDATID  datID = ( ARCDATID )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_BG_LOAD\n");
#endif DEBUG_OS_PRINT

#ifdef PM_DEBUG
  //デバッグ読み込みの場合は専用のバッファからロードする
  if( bevw->debug_flag == TRUE )
  {
    u32   ofs;
    u32*  ofs_p;
    void* resource;
    int   dpd_no = BTLV_EFFVM_GetDPDNo( bevw, datID, DPD_TYPE_BG );
    NNSG2dCharacterData* charData;
    NNSG2dScreenData* scrnData;
    NNSG2dPaletteData* palData;

    GFL_BG_SetPriority( GFL_BG_FRAME3_M, 1 );

    ofs_p = (u32*)&bevw->dpd->adrs[ 0 ];

    ofs = ofs_p[ dpd_no + 0 ];
    resource = (void *)&bevw->dpd->adrs[ ofs ];

    if( bevw->unpack_info[ dpd_no + 0 ] == NULL )
    {
      NNS_G2dGetUnpackedScreenData( resource, &scrnData );
      bevw->unpack_info[ dpd_no + 0 ] = scrnData;
    }
    else
    {
      scrnData = (NNSG2dScreenData*)bevw->unpack_info[ dpd_no + 0 ];
    }
    GFL_BG_LoadScreen( GFL_BG_FRAME3_M, scrnData->rawData, 0x2000, 0 );

    ofs = ofs_p[ dpd_no + 1 ];
    resource = (void *)&bevw->dpd->adrs[ ofs ];

    if( bevw->unpack_info[ dpd_no + 1 ] == NULL )
    {
      NNS_G2dGetUnpackedBGCharacterData( resource, &charData );
      bevw->unpack_info[ dpd_no + 1 ] = charData;
    }
    else
    {
      charData = (NNSG2dCharacterData*)bevw->unpack_info[ dpd_no + 1 ];
    }
    GFL_BG_LoadCharacter( GFL_BG_FRAME3_M, charData->pRawData, 0x8000, 0 );


    ofs = ofs_p[ dpd_no + 2 ];
    resource = (void *)&bevw->dpd->adrs[ ofs ];
    if( bevw->unpack_info[ dpd_no + 2 ] == NULL )
    {
      NNS_G2dGetUnpackedPaletteData( resource, &palData );
      bevw->unpack_info[ dpd_no + 2 ] = palData;
    }
    else
    {
      palData = (NNSG2dPaletteData*)bevw->unpack_info[ dpd_no + 2 ];
    }
    PaletteWorkSet( BTLV_EFFECT_GetPfd(), palData->pRawData, FADE_MAIN_BG, BTLV_EFFVM_BG_PAL * 16, palData->szByte );

    bevw->temp_scr_x = GFL_BG_GetScrollX( GFL_BG_FRAME3_M );
    bevw->temp_scr_y = GFL_BG_GetScrollY( GFL_BG_FRAME3_M );
    BTLV_BG_SetPosition( BTLV_EFFECT_GetBGWork(), 0, 0 );

    bevw->set_priority_flag = 1;

    return bevw->control_mode;
  }
#endif
  GFL_BG_SetPriority( GFL_BG_FRAME3_M, 1 );

  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_WAZAEFF_GRA, datID + 1, GFL_BG_FRAME3_M, 0, 0, 0, GFL_HEAP_LOWID( bevw->heapID ) );
  GFL_ARC_UTIL_TransVramScreen( ARCID_WAZAEFF_GRA, datID, GFL_BG_FRAME3_M, 0, 0, 0, GFL_HEAP_LOWID( bevw->heapID ) );
  PaletteWorkSetEx_Arc( BTLV_EFFECT_GetPfd(), ARCID_WAZAEFF_GRA, datID + 2, GFL_HEAP_LOWID( bevw->heapID ), FADE_MAIN_BG, 0,
                        BTLV_EFFVM_BG_PAL * 16, 0 );

  bevw->temp_scr_x = GFL_BG_GetScrollX( GFL_BG_FRAME3_M );
  bevw->temp_scr_y = GFL_BG_GetScrollY( GFL_BG_FRAME3_M );
  BTLV_BG_SetPosition( BTLV_EFFECT_GetBGWork(), 0, 0 );

  bevw->set_priority_flag = 1;

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief BGのスクロール
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BG_SCROLL( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int type  = ( int )VMGetU32( vmh );
  int scr_x = ( int )VMGetU32( vmh );
  int scr_y = ( int )VMGetU32( vmh );
  int frame = ( int )VMGetU32( vmh );
  int wait  = ( int )VMGetU32( vmh );
  int count = ( int )VMGetU32( vmh );
  int position = EFFVM_GetPosition( vmh, BTLEFF_POKEMON_SIDE_ATTACK );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_BG_SCROLL\n");
#endif DEBUG_OS_PRINT

  BTLV_BG_MovePosition( BTLV_EFFECT_GetBGWork(), position, type, scr_x, scr_y, frame, wait, count );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief BGのラスタースクロール
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BG_RASTER_SCROLL( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int type    = ( int )VMGetU32( vmh );
  int radius  = ( int )VMGetU32( vmh );
  int line    = ( int )VMGetU32( vmh );
  int wait    = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_BG_RASTER_SCROLL\n");
#endif DEBUG_OS_PRINT

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief BGのパレットアニメ
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BG_PAL_ANM( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int datID     = ( int )VMGetU32( vmh );
  int trans_pal = ( int )VMGetU32( vmh );
  int col_count = ( int )VMGetU32( vmh );
  int offset    = ( int )VMGetU32( vmh );
  int wait      = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_BG_PAL_ANM\n");
#endif DEBUG_OS_PRINT

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief BGのプライオリティ
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BG_PRIORITY( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int pri = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_BG_PRIORITY\n");
#endif DEBUG_OS_PRINT

  GFL_BG_SetPriority( GFL_BG_FRAME3_M, pri );

  bevw->set_priority_flag = 1;

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief BGα値
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BG_ALPHA( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int bg_num  = ( int )VMGetU32( vmh );
  int type    = ( int )VMGetU32( vmh );
  int alpha   = ( int )VMGetU32( vmh );
  int frame   = ( int )VMGetU32( vmh );
  int wait    = ( int )VMGetU32( vmh );
  int count   = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_BG_ALPHA\n");
#endif DEBUG_OS_PRINT


  if( alpha == 31 )
  {
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,
                      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 |
                      GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
                      31, 7 );
  }
  else
  {
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG3,
                      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 |
                      GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
                      31, alpha );
  }


  bevw->set_alpha_flag = 1;

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief BGパレットフェード
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BG_PAL_FADE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int   flag      = ( int )VMGetU32( vmh );
  int   start_evy = ( int )VMGetU32( vmh );
  int   end_evy   = ( int )VMGetU32( vmh );
  int   wait      = ( int )VMGetU32( vmh );
  int   rgb       = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_BG_PAL_FADE\n");
#endif DEBUG_OS_PRINT

  BTLV_EFFECT_SetPaletteFade( flag, start_evy, end_evy, wait, rgb );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief BGの表示/非表示
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BG_VISIBLE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int model = ( int )VMGetU32( vmh );
  int flag = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_BG_VISIBLE\n");
#endif DEBUG_OS_PRINT

  BTLV_EFFECT_SetVanishFlag( model, flag );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief	ウインドウ操作
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_WINDOW_MOVE( VMHANDLE *vmh, void *context_work )
{ 
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_WINDOW_MOVE *bevwm = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                             sizeof( BTLV_EFFVM_WINDOW_MOVE ) );

  bevwm->bevw     = bevw;
	bevwm->type     = ( int )VMGetU32( vmh );
	bevwm->horizon  = ( int )VMGetU32( vmh );
	bevwm->vertical = ( int )VMGetU32( vmh );
	bevwm->in_out   = ( int )VMGetU32( vmh );
	bevwm->frame    = ( int )VMGetU32( vmh );
	bevwm->wait_tmp = ( int )VMGetU32( vmh );
	bevwm->flag     = ( BOOL )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_WINDOW_MOVE\n");
#endif DEBUG_OS_PRINT

  //加減算レジスタを書き換えてしまうので、BGをオフる
  GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_OFF );
  GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_OFF );

  BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( bevw->tcbsys, TCB_EFFVM_WINDOW_MOVE, bevwm, 0 ), TCB_EFFVM_WINDOW_MOVE_CB, GROUP_EFFVM );
  bevw->window_move_flag = 1;

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief OBJのセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_OBJ_SET( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int index     = ( int )VMGetU32( vmh );
  int datID     = ( int )VMGetU32( vmh );
  int position  = EFFVM_GetPosition( vmh, ( int )VMGetU32( vmh ) );
  int ofs_x     = ( int )VMGetU32( vmh );
  int ofs_y     = ( int )VMGetU32( vmh );
  fx32 scalex   = ( fx32 )VMGetU32( vmh );
  fx32 scaley   = ( fx32 )VMGetU32( vmh );
  VecFx32 pos;
  int pos_x,pos_y;

  GF_ASSERT( index < EFFVM_OBJ_MAX );
  GF_ASSERT( bevw->obj[ index ] == EFFVM_OBJNO_NONE );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_OBJ_SET\n");
#endif DEBUG_OS_PRINT

  BTLV_MCSS_GetPokeDefaultPos( BTLV_EFFECT_GetMcssWork(), &pos, position );
  pos.x += ofs_x;
  pos.y += ofs_y;
  NNS_G3dWorldPosToScrPos( &pos, &pos_x, &pos_y );

  bevw->obj[ index ] = BTLV_CLACT_AddAffine( BTLV_EFFECT_GetCLWK(), ARCID_WAZAEFF_GRA, datID, pos_x, pos_y, scalex, scaley );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief OBJ移動
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_OBJ_MOVE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int           index;
  int           type;
  GFL_CLACTPOS  move_pos;
  int           frame;
  int           wait;
  int           count;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_OBJ_MOVE\n");
#endif DEBUG_OS_PRINT

  index       = ( int )VMGetU32( vmh );
  type        = ( int )VMGetU32( vmh );
  move_pos.x  = ( int )VMGetU32( vmh );
  move_pos.y  = ( int )VMGetU32( vmh );
  frame       = ( int )VMGetU32( vmh );
  wait        = ( int )VMGetU32( vmh );
  count       = ( int )VMGetU32( vmh );

  GF_ASSERT( index < EFFVM_OBJ_MAX );
  GF_ASSERT( bevw->obj[ index ] != EFFVM_OBJNO_NONE );

  BTLV_CLACT_MovePosition( BTLV_EFFECT_GetCLWK(), bevw->obj[ index ], type, &move_pos, frame, wait, count );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief OBJ拡縮
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_OBJ_SCALE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int   index;
  int   type;
  VecFx32 scale;
  int   frame;
  int   wait;
  int   count;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_OBJ_SCALE\n");
#endif DEBUG_OS_PRINT

  index   = ( int )VMGetU32( vmh );
  type    = ( int )VMGetU32( vmh );
  scale.x = ( fx32 )VMGetU32( vmh );
  scale.y = ( fx32 )VMGetU32( vmh );
  scale.z = FX32_ONE;
  frame   = ( int )VMGetU32( vmh );
  wait    = ( int )VMGetU32( vmh );
  count   = ( int )VMGetU32( vmh );

  GF_ASSERT( index < EFFVM_OBJ_MAX );
  GF_ASSERT( bevw->obj[ index ] != EFFVM_OBJNO_NONE );

  BTLV_CLACT_MoveScale( BTLV_EFFECT_GetCLWK(), bevw->obj[ index ], type, &scale, frame, wait, count );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief OBJアニメセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_OBJ_ANIME_SET( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int index   = ( int )VMGetU32( vmh );
  int anm_no  = ( int )VMGetU32( vmh );

  GF_ASSERT( index < EFFVM_OBJ_MAX );
  GF_ASSERT( bevw->obj[ index ] != EFFVM_OBJNO_NONE );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_OBJ_ANIME_SET\n");
#endif DEBUG_OS_PRINT

  BTLV_CLACT_SetAnime( BTLV_EFFECT_GetCLWK(), bevw->obj[ index ], anm_no );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief OBJパレットフェード
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_OBJ_PAL_FADE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int   index     = ( int )VMGetU32( vmh );
  int   start_evy = ( int )VMGetU32( vmh );
  int   end_evy   = ( int )VMGetU32( vmh );
  int   wait      = ( int )VMGetU32( vmh );
  int   rgb       = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_OBJ_PAL_FADE\n");
#endif DEBUG_OS_PRINT

  BTLV_CLACT_SetPaletteFade( BTLV_EFFECT_GetCLWK(), bevw->obj[ index ], start_evy, end_evy, wait, rgb );

  return bevw->control_mode;
}


//============================================================================================
/**
 * @brief OBJ削除
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_OBJ_DEL( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int index = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_OBJ_DEL\n");
#endif DEBUG_OS_PRINT

  GF_ASSERT( index < EFFVM_OBJ_MAX );
  GF_ASSERT( bevw->obj[ index ] != EFFVM_OBJNO_NONE );

  BTLV_CLACT_Delete( BTLV_EFFECT_GetCLWK(), bevw->obj[ index ] );
  bevw->obj[ index ] = EFFVM_OBJNO_NONE;

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief	HPゲージ表示／非表示
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_GAUGE_VANISH( VMHANDLE *vmh, void *context_work )
{ 
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int flag = ( int )VMGetU32( vmh );
  int side = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_GAUGE_VANISH\n");
#endif DEBUG_OS_PRINT

  if( ( flag == BTLEFF_GAUGE_DRAW_OFF ) ||
      ( flag == BTLEFF_GAUGE_DRAW_ON ) )
  { 
    switch( side ){ 
    case BTLEFF_GAUGE_ATTACK:
      BTLV_EFFECT_SetGaugeDrawEnableByPos( flag, bevw->attack_pos );
      break;
    case BTLEFF_GAUGE_DEFENCE:
      { 
        BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
        int   pos_cnt = EFFVM_GetPokePosition( bevw, BTLEFF_POKEMON_SIDE_DEFENCE, pos );

        //立ち位置情報がないときは、コマンド実行しない
        if( pos_cnt )
        {
          int i;

          for( i = 0 ; i < pos_cnt ; i++ )
          {
            BTLV_EFFECT_SetGaugeDrawEnableByPos( flag, pos[ pos_cnt ] );
          }
        }
      }
      break;
    default:
      BTLV_EFFECT_SetGaugeDrawEnable( flag, side );
      break;
    }
  }
  else
  { 
    BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
    int pos_cnt = 0;
    int i;
    switch( side ){ 
    case BTLEFF_GAUGE_MINE:
      for( i = BTLV_MCSS_POS_AA ; i < BTLV_MCSS_POS_MAX ; i += 2 )
      { 
        if( BTLV_EFFECT_CheckExist( i ) )
        { 
          pos[ pos_cnt ] = i;
          pos_cnt++;
        }
      }
      break;
    case BTLEFF_GAUGE_ENEMY:
      for( i = BTLV_MCSS_POS_BB ; i < BTLV_MCSS_POS_MAX ; i += 2 )
      { 
        if( BTLV_EFFECT_CheckExist( i ) )
        { 
          pos[ pos_cnt ] = i;
          pos_cnt++;
        }
      }
      break;
    case BTLEFF_GAUGE_ALL:
      for( i = BTLV_MCSS_POS_AA ; i < BTLV_MCSS_POS_MAX ; i++ )
      { 
        if( BTLV_EFFECT_CheckExist( i ) )
        { 
          pos[ pos_cnt ] = i;
          pos_cnt++;
        }
      }
      break;
    case BTLEFF_GAUGE_ATTACK:
      pos_cnt = 1;
      pos[ 0 ] = bevw->attack_pos;
      break;
    case BTLEFF_GAUGE_DEFENCE:
      pos_cnt = EFFVM_GetPokePosition( bevw, BTLEFF_POKEMON_SIDE_DEFENCE, pos );
      break;
    }
    //立ち位置情報がないときは、コマンド実行しない
    if( pos_cnt )
    {
      for( i = 0 ; i < pos_cnt ; i++ )
      {
        GFL_CLACTPOS  ofs = { 0, 0 };
        ofs.x = ( flag == BTLEFF_GAUGE_MOVE_DRAW_OFF ) ? BTLEFF_GAUGE_MOVE_VALUE : -16;
        if( pos[ i ] & 1 )
        { 
          ofs.x *= -1;
        }
        BTLV_GAUGE_SetPos( BTLV_EFFECT_GetGaugeWork(), pos[ i ], &ofs );
      }
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief SE再生
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_SE_PLAY( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int se_no     = ( int )VMGetU32( vmh );
  int player    = ( int )VMGetU32( vmh );
  int pan       = ( int )VMGetU32( vmh );
  int wait      = ( int )VMGetU32( vmh );
  int pitch     = ( int )VMGetU32( vmh );
  int vol       = ( int )VMGetU32( vmh );
  int mod_depth = ( int )VMGetU32( vmh );
  int mod_speed = ( int )VMGetU32( vmh );
  int dummy     = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_SE_PLAY\n");
#endif DEBUG_OS_PRINT

  BTLV_EFFVM_SePlay( vmh, se_no, player, pan, pitch, vol, mod_depth, mod_speed, wait );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief SEストップ
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_SE_STOP( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int player      = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_SE_STOP\n");
#endif DEBUG_OS_PRINT

  if( player == BTLEFF_SESTOP_ALL )
  {
    PMSND_StopSE();
  }
  else
  {
    PMSND_StopSE_byPlayerID( player );
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief SEパン
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_SE_PAN( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_SEEFFECT*  bes = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), sizeof( BTLV_EFFVM_SEEFFECT ) );
  int start, end;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_SE_PAN\n");
#endif DEBUG_OS_PRINT

  bes->bevw     = bevw;

  bes->player     = ( int )VMGetU32( vmh );
  bes->type       = ( int )VMGetU32( vmh );
  bes->param      = BTLEFF_SEEFFECT_PAN;
  start           = EFFVM_GetPosition( vmh, ( int )VMGetU32( vmh ) );
  end             = EFFVM_GetPosition( vmh, ( int )VMGetU32( vmh ) );
  bes->start      = ( start & 1 ) ? 127 : -128;
  bes->end        = ( end & 1 ) ? 127 : -128;
  bes->start_wait = ( int )VMGetU32( vmh );
  bes->frame      = ( int )VMGetU32( vmh );
  bes->frame_tmp  = bes->frame;
  bes->wait       = 0;
  bes->wait_tmp   = ( int )VMGetU32( vmh );
  bes->count      = ( int )VMGetU32( vmh ) * 2;

  if( ( bes->type == BTLEFF_SEPAN_ROUNDTRIP ) && ( bes->count == 0 ) )
  {
    bes->count = 2;
  }

  bes->value = FX32_CONST( bes->start );
  bes->vec_value = FX_Div( FX32_CONST( bes->end - bes->start ) , FX32_CONST( bes->frame ) );

  BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( bevw->tcbsys, TCB_EFFVM_SEEFFECT, bes, 0 ), TCB_EFFVM_SEEFFECT_CB, GROUP_EFFVM );

  bevw->se_effect_enable_flag = 1;

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief SE動的変化
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_SE_EFFECT( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int player      = ( int )VMGetU32( vmh );
  int type        = ( int )VMGetU32( vmh );
  int param       = ( int )VMGetU32( vmh );
  int start       = ( int )VMGetU32( vmh );
  int end         = ( int )VMGetU32( vmh );
  int start_wait  = ( int )VMGetU32( vmh );
  int frame       = ( int )VMGetU32( vmh );
  int wait        = ( int )VMGetU32( vmh );
  int count       = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_SE_EFFECT\n");
#endif DEBUG_OS_PRINT

  BTLV_EFFVM_SeEffect( vmh, player, type, param, start, end, start_wait, frame, wait, count );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief エフェクト終了待ち
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_EFFECT_END_WAIT( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_EFFECT_END_WAIT\n");
#endif DEBUG_OS_PRINT

  bevw->effect_end_wait_kind = ( int )VMGetU32( vmh );

  VMCMD_SetWait( vmh, VWF_EFFECT_END_CHECK );

  //エフェクト待ちは必ずSUSPENDモードに切り替える
  bevw->control_mode = VMCMD_RESULT_SUSPEND;

  return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 * @brief タイマーウエイト
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_WAIT( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_WAIT\n");
#endif DEBUG_OS_PRINT

  bevw->wait = ( int )VMGetU32( vmh );

  VMCMD_SetWait( vmh, VWF_WAIT_CHECK );

  //ウエイトは必ずSUSPENDモードに切り替える
  bevw->control_mode = VMCMD_RESULT_SUSPEND;

  return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 * @brief コントロールモード変更
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_CONTROL_MODE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_CONTROL_MODE\n");
#endif DEBUG_OS_PRINT

  bevw->control_mode = ( VMCMD_RESULT )VMGetU32( vmh );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief 指定されたワークを見て条件分岐
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_IF( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int work  = EFFVM_GetWork( bevw, ( int )VMGetU32( vmh ) );
  int cond  = ( int )VMGetU32( vmh );
  int value = ( int )VMGetU32( vmh );
  int adrs  = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_IF\n");
#endif DEBUG_OS_PRINT

  if_act( vmh, work, value, cond, adrs );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief 指定されたワーク同士を見て条件分岐
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_IF_WORK( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int src   = EFFVM_GetWork( bevw, ( int )VMGetU32( vmh ) );
  int cond  = ( int )VMGetU32( vmh );
  int dst   = EFFVM_GetWork( bevw, ( int )VMGetU32( vmh ) );
  int adrs  = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_IF\n");
#endif DEBUG_OS_PRINT

  if_act( vmh, src, dst, cond, adrs );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief 条件分岐
 */
//============================================================================================
static  void  if_act( VMHANDLE* vmh, int src, int dst, int cond, int adrs )
{ 
  BOOL  flag = FALSE;

  switch( cond ){
  case BTLEFF_COND_EQUAL:       // ==
    if( src == dst )
    {
      flag = TRUE;
    }
    break;
  case BTLEFF_COND_NOT_EQUAL:   // !=
    if( src != dst )
    {
      flag = TRUE;
    }
    break;
  case BTLEFF_COND_MIMAN:       // <
    if( src < dst )
    {
      flag = TRUE;
    }
    break;
  case BTLEFF_COND_KOERU:       // >
    if( src > dst )
    {
      flag = TRUE;
    }
    break;
  case BTLEFF_COND_IKA:         // <=
    if( src <= dst )
    {
      flag = TRUE;
    }
    break;
  case BTLEFF_COND_IJOU:        // >=
    if( src >= dst )
    {
      flag = TRUE;
    }
    break;
  }

  if( flag == TRUE )
  {
    VMCMD_Jump( vmh, vmh->adrs + adrs );
  }
}


//============================================================================================
/**
 * @brief 立ち位置チェック
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_MCSS_POS_CHECK( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int pos =  ( int )VMGetU32( vmh );
  int cond = ( int )VMGetU32( vmh );
  int adrs = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_MCSS_POS_CHECK:\npos:%d cond:%d adrs:%d\n",pos,cond,adrs);
#endif DEBUG_OS_PRINT
  if( BTLV_EFFECT_CheckExist( pos ) == cond )
  {
    VMCMD_Jump( vmh, vmh->adrs + adrs );
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief 汎用ワークに値をセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_SET_WORK( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int value =  ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_SET_WORK:\nvalue:%d\n",value);
#endif DEBUG_OS_PRINT

  bevw->sequence_work = value;

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief 指定されたワークから値をゲット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_GET_WORK( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_GET_WORK:\n");
#endif DEBUG_OS_PRINT

  bevw->sequence_work = EFFVM_GetWork( bevw, ( int )VMGetU32( vmh ) );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief 指定されたワークに値をセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_SET_PARAM( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int work = ( int )VMGetU32( vmh );
  int param = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_SET_WORK:\n");
#endif DEBUG_OS_PRINT

  switch( work ){
  case BTLEFF_WORK_WAZA_RANGE:    ///<技の効果範囲
    bevw->param.waza_range = param;
    break;
  case BTLEFF_WORK_TURN_COUNT:    ///< ターンによって異なるエフェクトを出す場合のターン指定。（ex.そらをとぶ）
    bevw->param.turn_count = param;
    break;
  case BTLEFF_WORK_CONTINUE_COUNT:   ///< 連続して出すとエフェクトが異なる場合の連続カウンタ（ex. ころがる）
    bevw->param.continue_count = param;
    break;
  case BTLEFF_WORK_YURE_CNT:   ///<ボールゆれるカウント
    bevw->param.yure_cnt = param;
    break;
  case BTLEFF_WORK_GET_SUCCESS:   ///<捕獲成功かどうか
    bevw->param.get_success = param;
    break;
  case BTLEFF_WORK_GET_CRITICAL:   ///<クリティカルかどうか
    bevw->param.get_critical = param;
    break;
  case BTLEFF_WORK_ITEM_NO:   ///<ボールのアイテムナンバー
    bevw->param.item_no = param;
    break;
  case BTLEFF_WORK_ZOOM_OUT:
    bevw->zoom_out_flag = param;
    break;
  case BTLEFF_WORK_PUSH_CAMERA_POS:
    bevw->push_camera_flag = param;
    break;
  case BTLEFF_WORK_WCS_CAMERA_WORK:
    bevw->wcs_camera_work = param;
    break;
  case BTLEFF_WORK_CAMERA_MOVE_IGNORE:
    bevw->camera_move_ignore = param;
    break;
  default:
    //未知のパラメータです
    GF_ASSERT( 0 );
    break;
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief みがわり処理
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_MIGAWARI( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int sw =  ( int )VMGetU32( vmh );
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int pos_cnt =  EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int flag =  ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_MIGAWARI:\nsw:%d\n",sw);
#endif DEBUG_OS_PRINT

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    int i;

    for( i = 0 ; i < pos_cnt ; i++ )
    {
      BTLV_MCSS_SetMigawari( BTLV_EFFECT_GetMcssWork(), pos[ i ], sw, flag );
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief へんしん処理
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_HENSHIN( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_HENSHIN:\n");
#endif DEBUG_OS_PRINT

  BTLV_MCSS_CopyMAW( BTLV_EFFECT_GetMcssWork(), bevw->defence_pos, bevw->attack_pos );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ポケモン鳴き声
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_NAKIGOE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
  int pos_cnt       =  EFFVM_GetPokePosition( bevw, ( int )VMGetU32( vmh ), pos );
  int pitch         = ( int )VMGetU32( vmh );
  int volume        = ( int )VMGetU32( vmh );
  int chorus_vol    = ( int )VMGetU32( vmh );
  int chorus_speed  = ( int )VMGetU32( vmh );
  BOOL play_dir     = ( BOOL )VMGetU32( vmh );
  int wait          = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_NAKIGOE:\npos:%d\n",pos_cnt);
#endif DEBUG_OS_PRINT

  //立ち位置情報がないときは、コマンド実行しない
  if( pos_cnt )
  {
    if( wait == 0 )
    { 
      int i;

      for( i = 0 ; i < pos_cnt ; i++ )
      {
        int index = EFFVM_GetVoicePlayerIndex( bevw );
        bevw->voiceplayerIndex[ index ] = BTLV_MCSS_PlayVoice( BTLV_EFFECT_GetMcssWork(), pos[ i ], pitch, volume,
                                                               chorus_vol, chorus_speed, play_dir );
      }
      //BGMのマスターボリュームを下げる
      {
        fx32  start_vol = 127 << FX32_SHIFT;
#ifdef KAGEYAMA_DEBUG
        fx32  end_vol = ( 127 * volume_down_ratio_pv / 100 ) << FX32_SHIFT;
        int   frame = volume_down_frame_pv;
#else
        fx32  end_vol = EFFVM_CHANGE_VOLUME_PV;
        int   frame = EFFVM_CHANGE_VOLUME_DOWN_FRAME_PV;
#endif
        EFFVM_ChangeVolume( bevw, start_vol, end_vol, frame );
      }
      if( bevw->volume_already_down == 0 )
      { 
        TCB_NONE_WORK*  tnw = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), sizeof( TCB_NONE_WORK ) );
        tnw->bevw = bevw;
        BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( bevw->tcbsys, TCB_EFFVM_NakigoeEndCheck, tnw, 0 ), NULL, GROUP_DEFAULT );
      }
    }
    else
    { 
      int i;

      for( i = 0 ; i < pos_cnt ; i++ )
      { 
        BTLV_EFFVM_NAKIGOE*  ben = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), sizeof( BTLV_EFFVM_NAKIGOE ) );
        ben->bevw         = bevw;
        ben->pos          = pos[ i ];
        ben->pitch        = pitch;
        ben->volume       = volume;
        ben->chorus_vol   = chorus_vol;
        ben->chorus_speed = chorus_speed;
        ben->play_dir     = play_dir;
        ben->wait         = wait;

        bevw->nakigoe_wait_flag = 1;

        BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( bevw->tcbsys, TCB_EFFVM_NAKIGOE, ben, 0 ), TCB_EFFVM_NAKIGOE_CB, GROUP_EFFVM );
      }
    }
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ボールモード
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BALL_MODE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

  bevw->ball_mode = ( int )VMGetU32( vmh );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ボールOBJのセット
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_BALLOBJ_SET( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int index     = ( int )VMGetU32( vmh );
  int position  = EFFVM_GetPosition( vmh, ( int )VMGetU32( vmh ) );
  int ofs_x     = ( int )VMGetU32( vmh );
  int ofs_y     = ( int )VMGetU32( vmh );
  fx32 scalex   = ( fx32 )VMGetU32( vmh );
  fx32 scaley   = ( fx32 )VMGetU32( vmh );
  VecFx32 pos;
  int pos_x,pos_y;

  GF_ASSERT( index < EFFVM_OBJ_MAX );
  GF_ASSERT( bevw->obj[ index ] == EFFVM_OBJNO_NONE );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_BALLOBJ_SET\n");
#endif DEBUG_OS_PRINT

  BTLV_MCSS_GetPokeDefaultPos( BTLV_EFFECT_GetMcssWork(), &pos, position );
  pos.x += ofs_x;
  pos.y += ofs_y;
  NNS_G3dWorldPosToScrPos( &pos, &pos_x, &pos_y );

  {
    BALL_ID ballID = ITEM_GetBallID( bevw->param.item_no );
    ARCDATID  ncgrID, nclrID;

    //不正なボールIDはモンスターボールにする
    if( ( ballID == BALLID_NULL ) || ( ballID > BALLID_MAX ) )
    {
      ballID = BALLID_MONSUTAABOORU;
    }
    //ドリームボールはスピードボールにする（間が抜けているので）
    if( ballID == BALLID_DORIIMUBOORU )
    { 
      ballID = BALLID_SUPIIDOBOORU;
    }
    ncgrID = NARC_waza_eff_gra_ball_01_anim_NCER + ballID * 2;
    nclrID = NARC_waza_eff_gra_ball_01_anim_NANR + ballID * 2;
    bevw->obj[ index ] = BTLV_CLACT_AddEx( BTLV_EFFECT_GetCLWK(), ARCID_WAZAEFF_GRA,
                                           ncgrID, nclrID,
                                           NARC_waza_eff_gra_ball_01_anim_NCER, NARC_waza_eff_gra_ball_01_anim_NANR,
                                           pos_x, pos_y, scalex, scaley );
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief サブルーチンコール
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_CALL( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int seq_no = ( int )VMGetU32( vmh );
  int *start_ofs;
  int attack  = ( int )VMGetU32( vmh );
  int defence = ( int )VMGetU32( vmh );

  GF_ASSERT( bevw->call_count < BTLV_EFFVM_STACK_SIZE );
  //技エフェクトのサブルーチンコールは許可しない
  GF_ASSERT( seq_no >= BTLEFF_SINGLE_ENCOUNT_1 );

  bevw->push_attack_pos   [ bevw->call_count ]  = bevw->attack_pos;
  bevw->push_defence_pos  [ bevw->call_count ]  = bevw->defence_pos;
  bevw->push_sequence     [ bevw->call_count ]  = bevw->sequence;
  bevw->push_sequence_work[ bevw->call_count ]  = bevw->sequence_work;

  if( attack != BTLEFF_POKEMON_SIDE_ATTACK )
  {
    bevw->attack_pos  = attack;
  }
  if( defence != BTLEFF_POKEMON_SIDE_DEFENCE )
  {
    bevw->defence_pos  = defence;
  }
  bevw->sequence_work = 0;

  bevw->sequence = GFL_ARC_LoadDataAlloc( ARCID_BATTLEEFF_SEQ, seq_no - BTLEFF_SINGLE_ENCOUNT_1, GFL_HEAP_LOWID( bevw->heapID ) );
  start_ofs = (int *)&bevw->sequence[ TBL_AA2BB ];

  VMCMD_Call( vmh, &bevw->sequence[ (*start_ofs) ] );

  bevw->call_count++;

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief サブルーチンリターン
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_RETURN( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

//デバッグのときはサブルーチンコールがない場合でもアサートで止めずにVMEC_SEQ_END扱いにする
#ifdef PM_DEBUG
  if( bevw->debug_flag == TRUE )
  {
    int i;

    bevw->debug_flag = FALSE;
    //解放されていないパーティクルがあったら解放しておく
    for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ )
    {
      if( bevw->ptc[ i ] )
      {
        EFFVM_DeleteEmitter( bevw->ptc[ i ] );
        bevw->ptc[ i ] = NULL;
      }
      bevw->ptc_no[ i ] = EFFVM_PTCNO_NONE;
    }

    //解放されていないOBJがあったら解放しておく
    for( i = 0 ; i < EFFVM_OBJ_MAX ; i++ )
    {
      if( bevw->obj[ i ] != EFFVM_OBJNO_NONE )
      {
        BTLV_CLACT_Delete( BTLV_EFFECT_GetCLWK(), bevw->obj[ i ] );
        bevw->obj[ i ] = EFFVM_OBJNO_NONE;
      }
    }

    //解放していないテンポラリワークを解放
    for( i = 0 ; i < bevw->temp_work_count ; i++ )
    {
      GFL_HEAP_FreeMemory( bevw->temp_work[ i ] );
    }
    bevw->temp_work_count = 0;

    //仮想マシン停止
    VM_End( vmh );

    return VMCMD_RESULT_SUSPEND;
  }

  if( bevw->debug_return_through )
  {
    bevw->debug_return_through = FALSE;

    //仮想マシン停止
    VM_End( vmh );

    return VMCMD_RESULT_SUSPEND;
  }
#endif

  GF_ASSERT( bevw->call_count );

  bevw->call_count--;

  GFL_HEAP_FreeMemory( bevw->sequence );

  bevw->attack_pos    = bevw->push_attack_pos   [ bevw->call_count ];
  bevw->defence_pos   = bevw->push_defence_pos  [ bevw->call_count ];
  bevw->sequence      = bevw->push_sequence     [ bevw->call_count ];
  bevw->sequence_work = bevw->push_sequence_work[ bevw->call_count ];

  VMCMD_Return( vmh );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief ジャンプ
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_JUMP( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int adrs  = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_JUMP\n");
#endif DEBUG_OS_PRINT

  VMCMD_Jump( vmh, vmh->adrs + adrs );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief シーケンス一時停止
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_PAUSE( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_PAUSE\n");
#endif DEBUG_OS_PRINT

  bevw->pause_flag = 1;

  VMCMD_SetWait( vmh, VWF_PAUSE_CHECK );

  //ウエイトは必ずSUSPENDモードに切り替える
  bevw->control_mode = VMCMD_RESULT_SUSPEND;

  return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 * @brief	シーケンスジャンプ
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_SEQ_JUMP( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int seq_no = ( int )VMGetU32( vmh );
  int *start_ofs;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_SEQ_JUMP\n");
#endif DEBUG_OS_PRINT

#ifdef PM_DEBUG
  //デバッグ起動ではsequenceはNULLなのでスルー
  if( bevw->debug_flag == FALSE )
  { 
    GFL_HEAP_FreeMemory( bevw->sequence );
  }
  //ロムの技エフェクトを起動することになるので、デバッグ起動フラグはオフ
  bevw->debug_flag = FALSE;
#else
  GFL_HEAP_FreeMemory( bevw->sequence );
#endif

  if( seq_no < BTLEFF_SINGLE_ENCOUNT_1 )
  { 
    bevw->sequence = GFL_ARC_LoadDataAlloc( ARCID_WAZAEFF_SEQ, seq_no, GFL_HEAP_LOWID( bevw->heapID ) );
  }
  else
  { 
    seq_no -= BTLEFF_SINGLE_ENCOUNT_1;
    bevw->sequence = GFL_ARC_LoadDataAlloc( ARCID_BATTLEEFF_SEQ, seq_no, GFL_HEAP_LOWID( bevw->heapID ) );
  }
  start_ofs = (int *)&bevw->sequence[ TBL_AA2BB ];
  bevw->sequence_work = 0;

  VMCMD_Jump( vmh, &bevw->sequence[ (*start_ofs) ] );

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief	着地演出待ち
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_LANDING_WAIT( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int side = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_LANDING_WAIT\n");
#endif DEBUG_OS_PRINT

  { 
    BTLV_EFFVM_LANDING_WAIT* belw = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), sizeof( BTLV_EFFVM_LANDING_WAIT ) );
    belw->bevw        = bevw;
    belw->side        = side;
    belw->finish_flag = 0;
    BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( bevw->tcbsys, TCB_EFFVM_LandingWait, belw, 0 ), NULL, GROUP_EFFVM );
  }

  return bevw->control_mode;
}

//============================================================================================
/**
 * @brief	着地演出待ち
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_REVERSE_DRAW_SET( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int flag = ( int )VMGetU32( vmh );

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("REVERSE_DRAW_SET\n");
#endif DEBUG_OS_PRINT

  BTLV_EFFECT_SetReverseDrawFlag( flag );

  return bevw->control_mode;
}


//============================================================================================
/**
 * @brief エフェクトシーケンス終了
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT VMEC_SEQ_END( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  int i;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_SEQ_END\n");
#endif DEBUG_OS_PRINT

  //解放されていないパーティクルがあったら解放しておく
  for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ )
  {
    if( bevw->ptc[ i ] )
    {
      EFFVM_DeleteEmitter( bevw->ptc[ i ] );
      bevw->ptc[ i ] = NULL;
    }
    bevw->ptc_no[ i ] = EFFVM_PTCNO_NONE;
  }

  //解放されていないOBJがあったら解放しておく
  for( i = 0 ; i < EFFVM_OBJ_MAX ; i++ )
  {
    if( bevw->obj[ i ] != EFFVM_OBJNO_NONE )
    {
      BTLV_CLACT_Delete( BTLV_EFFECT_GetCLWK(), bevw->obj[ i ] );
      bevw->obj[ i ] = EFFVM_OBJNO_NONE;
    }
  }

  //解放していないテンポラリワークを解放
  for( i = 0 ; i < bevw->temp_work_count ; i++ )
  {
    GFL_HEAP_FreeMemory( bevw->temp_work[ i ] );
  }
  bevw->temp_work_count = 0;

  BTLV_EFFECT_FreeTCBGroup( GROUP_EFFVM );

  //初期位置にいないポケモンをチェック
  EFFVM_CheckPokePosition( bevw );

  //影を消したままになっているかもしれないので、復活させる
  EFFVM_CheckShadow( bevw );

  //リバース描画ON
  if( !bevw->reverse_draw_no_check )
  { 
    BTLV_EFFECT_SetReverseDrawFlag( BTLV_EFFECT_REVERSE_DRAW_ON );
  }

  //仮想マシン停止
  VM_End( vmh );


  //SUSPENDモードに切り替えておく
  bevw->control_mode = VMCMD_RESULT_SUSPEND;

  //BG周りの設定をデフォルトに戻しておく
  if( bevw->set_priority_flag )
  {
    const BTLV_SCU* scu = BTLV_EFFECT_GetScu();
    if( scu != NULL )
    { 
      BTLV_SCU_RestoreDefaultScreen( scu );
    }
    GFL_BG_SetPriority( GFL_BG_FRAME3_M, 0 );
    bevw->set_priority_flag = 0;
    GFL_BG_SetScroll( GFL_BG_FRAME3_M, GFL_BG_SCROLL_X_SET, bevw->temp_scr_x );
    GFL_BG_SetScroll( GFL_BG_FRAME3_M, GFL_BG_SCROLL_Y_SET, bevw->temp_scr_y );
  }

  if( bevw->set_alpha_flag )
  {

    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,
                      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 |
                      GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
                      31, 7 );

    bevw->set_alpha_flag = 0;
  }

  //SEを強制的にストップ
  if( bevw->se_play_flag )
  { 
    PMSND_StopSE();
  }

  { 
    BOOL  migawari_flag = FALSE;

    if( BTLV_MCSS_CheckExist( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos ) )
    { 
      //みらいよちorはめつのねがいの発動ターンではみがわりチェックをしない
      if( ( ( bevw->waza == WAZANO_MIRAIYOTI ) || ( bevw->waza == WAZANO_HAMETUNONEGAI ) ) && 
            ( bevw->param.turn_count == 1 ) )
      { 
        ;
      }
      //へんしんとオウムがえしとしぜんのちからは、単体のエフェクトはないので、みがわりチェックをしない
      else if( ( bevw->waza == WAZANO_HENSIN ) ||
               ( bevw->waza == WAZANO_OUMUGAESI ) ||
               ( bevw->waza == WAZANO_SIZENNOTIKARA ) )
      { 
        ;
      }
      else
      { 
        migawari_flag = ( BTLV_MCSS_GetStatusFlag( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos ) &
                          BTLV_MCSS_STATUS_FLAG_MIGAWARI );
      }
    }

    //みがわり引っ込むエフェクト起動していたなら、退避していた技エフェクトを起動
    if( bevw->migawari_sequence )
    {
      int* start_ofs;
  
      GFL_HEAP_FreeMemory( bevw->sequence );
      bevw->sequence = bevw->migawari_sequence;
      bevw->migawari_sequence = NULL;
      bevw->execute_effect_type = EXECUTE_EFF_TYPE_WAZA;
  
      start_ofs = (int *)&bevw->sequence[ bevw->migawari_table_ofs ];
  
      //BG非表示
      GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_OFF );
  
      //汎用ワークを初期化
      bevw->sequence_work = 0;
  
      VM_Start( vmh, &bevw->sequence[ (*start_ofs) ] );
      return VMCMD_RESULT_SUSPEND;
    }
    //みがわりが出ているときに技エフェクトを起動していたなら、みがわりを戻すエフェクトを差し込む
    else if( ( bevw->zoom_in_migawari == 1 ) && ( bevw->waza == BTLEFF_ZOOM_IN_RESET ) )
    {
      int* start_ofs;
  
      GFL_HEAP_FreeMemory( bevw->sequence );
      bevw->waza = BTLEFF_MIGAWARI_WAZA_AFTER - BTLEFF_SINGLE_ENCOUNT_1;
      bevw->sequence = GFL_ARC_LoadDataAlloc( ARCID_BATTLEEFF_SEQ, bevw->waza, GFL_HEAP_LOWID( bevw->heapID ) );
      bevw->execute_effect_type = EXECUTE_EFF_TYPE_BATTLE;
  
      start_ofs = (int *)&bevw->sequence[ TBL_AA2BB ];
  
      //汎用ワークを初期化
      bevw->sequence_work = 0;

      bevw->zoom_in_migawari = 0;
  
      VM_Start( vmh, &bevw->sequence[ (*start_ofs) ] );
      return VMCMD_RESULT_SUSPEND;
    }
    //みがわりが出ているときに技エフェクトを起動していたなら、みがわりを戻すエフェクトを差し込む
    else if( ( ( bevw->waza == BTLEFF_POKEMON_VANISH_OFF ) ||
               ( bevw->execute_effect_type == EXECUTE_EFF_TYPE_WAZA ) ) &&
            //へんしんは技エフェクトではなくなったので除外
            ( bevw->waza != WAZANO_HENSIN ) &&
            //バトンタッチは戻すエフェクトにつながるので除外
            ( bevw->waza != WAZANO_BATONTATTI ) &&
            ( bevw->zoom_in_migawari == 0 ) &&
            ( migawari_flag ) )
    {
      int* start_ofs;
  
      //サイドチェンジだとバニッシュされているので、バニッシュをオフる
      if( bevw->waza == WAZANO_SAIDOTHENZI )
      { 
        BTLV_MCSS_SetVanishFlag( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos, BTLV_MCSS_VANISH_OFF );
      }
      GFL_HEAP_FreeMemory( bevw->sequence );
      bevw->waza = BTLEFF_MIGAWARI_WAZA_AFTER - BTLEFF_SINGLE_ENCOUNT_1;
      bevw->sequence = GFL_ARC_LoadDataAlloc( ARCID_BATTLEEFF_SEQ, bevw->waza, GFL_HEAP_LOWID( bevw->heapID ) );
      bevw->execute_effect_type = EXECUTE_EFF_TYPE_BATTLE;
  
      start_ofs = (int *)&bevw->sequence[ TBL_AA2BB ];
  
      //汎用ワークを初期化
      bevw->sequence_work = 0;

      bevw->zoom_in_migawari = 0;
  
      VM_Start( vmh, &bevw->sequence[ (*start_ofs) ] );
      return VMCMD_RESULT_SUSPEND;
    }
  }

  //サブルーチンコールが残っていてはいけない
  GF_ASSERT_MSG( bevw->call_count == 0, "count:%d\n", bevw->call_count );

  GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );
  GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
  GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_ON );

#ifdef PM_DEBUG
  { 
    const BTLV_SCU* scu = BTLV_EFFECT_GetScu();
    if( scu == NULL )
    { 
      GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_OFF );
    }
    //デバッグフラグを落としておく
    bevw->debug_flag = FALSE;
  }
#endif

#ifdef DEBUG_SE_END_PRINT
  { 
    BOOL  se_end_flag = FALSE;

    OS_TPrintf("WazaNo:%d\n",bevw->waza);
    //SE終了チェックをおこなう
    if( bevw->se_play_wait_flag )
    { 
      OS_TPrintf("開始待ちのSEが終了していません\n");
      se_end_flag = TRUE;
    }
    if( bevw->se_effect_enable_flag )
    { 
      OS_TPrintf("動的変化中のSEが終了していません\n");
      se_end_flag = TRUE;
    }
    if( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE1 ) )
    { 
      OS_TPrintf("SE1が終了していません\n");
      se_end_flag = TRUE;
    }
    if( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE2 ) )
    { 
      OS_TPrintf("SE2が終了していません\n");
      se_end_flag = TRUE;
    }
    if( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE3 ) )
    { 
      OS_TPrintf("SE3が終了していません\n");
      se_end_flag = TRUE;
    }
    if( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE_PSG ) )
    { 
      OS_TPrintf("PSGが終了していません\n");
      se_end_flag = TRUE;
    }
    if( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SYS ) )
    { 
      OS_TPrintf("SYSTEMが終了していません\n");
      se_end_flag = TRUE;
    }
    GF_ASSERT( se_end_flag == FALSE );
    if( se_end_flag == TRUE )
    { 
      OS_TPrintf("se_end_flagのアサート抜けました\n\n");
    }
    else
    { 
      OS_TPrintf("正常終了しました\n\n");
    }
  }
#endif

  return VMCMD_RESULT_SUSPEND;
}

//VM_WAIT_FUNC群
//============================================================================================
/**
 * @brief エフェクト終了チェック
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 *
 * @retval  TRUE:エフェクト終了　FALSE:エフェクト動作中
 */
//============================================================================================
static  BOOL  VWF_EFFECT_END_CHECK( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_CAMERA ) )
  {
    //カメラ移動終了？
    if( BTLV_CAMERA_CheckExecute( BTLV_EFFECT_GetCameraWork() ) == TRUE )
    {
      return FALSE;
    }
    if( bevw->camera_ortho_on_flag )
    {
      bevw->camera_ortho_on_flag = 0; //カメラ移動後、正射影に戻すフラグ
      bevw->camera_projection = BTLEFF_CAMERA_PROJECTION_PERSPECTIVE;
      BTLV_MCSS_SetOrthoMode( BTLV_EFFECT_GetMcssWork() );
    }
  }
  //ポケモン移動終了？
  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_POKEMON ) )
  {
    BtlvMcssPos pos;

    for( pos = 0 ; pos < BTLV_MCSS_POS_TOTAL ; pos++ )
    {
      if( BTLV_MCSS_CheckTCBExecute( BTLV_EFFECT_GetMcssWork(), pos ) == TRUE )
      {
        return FALSE;
      }
    }
  }

#if 0
  //トレーナー移動終了？
  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_TRAINER ) )
  {
    int index;

    for( index = 0 ; index < BTLV_CLACT_CLWK_MAX ; index++ )
    {
      if( BTLV_CLACT_CheckTCBExecute( BTLV_EFFECT_GetCLWK(), index ) == TRUE )
      {
        return FALSE;
      }
    }
  }
#endif
  //MCSSアニメ終了？
  if( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ANIME )
  {
    BtlvMcssPos pos;

    for( pos = BTLV_MCSS_POS_TR_AA ; pos < BTLV_MCSS_POS_TOTAL ; pos++ )
    {
      if( BTLV_MCSS_CheckExist( BTLV_EFFECT_GetMcssWork(), pos ) )
      { 
        if( BTLV_MCSS_CheckAnimeExecute( BTLV_EFFECT_GetMcssWork(), pos ) == TRUE )
        {
          return FALSE;
        }
      }
    }
  }

  //BG移動終了？
  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_BG ) )
  {
    if( BTLV_BG_CheckTCBExecute( BTLV_EFFECT_GetBGWork() ) == TRUE )
    {
      return FALSE;
    }
  }

  //パーティクル描画終了？
  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_PARTICLE ) )
  {
    int ptc_no;

    for( ptc_no = 0 ; ptc_no < PARTICLE_GLOBAL_MAX ; ptc_no++ )
    {
      if( bevw->ptc[ ptc_no ] != NULL )
      {
        if( GFL_PTC_GetEmitterNum( bevw->ptc[ ptc_no ] ) != 0 )
        {
          return FALSE;
        }
      }
    }
  }
  //パレットフェード終了？
  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_PALFADE_STAGE ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_PALFADE_3D ) )
  {
    if( BTLV_EFFECT_CheckExecutePaletteFade( BTLEFF_PAL_FADE_STAGE ) )
    {
      return FALSE;
    }
  }
  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_PALFADE_FIELD ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_PALFADE_3D ) )
  {
    if( BTLV_EFFECT_CheckExecutePaletteFade( BTLEFF_PAL_FADE_FIELD ) )
    {
      return FALSE;
    }
  }
  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_PALFADE_EFFECT ) )
  {
    if( BTLV_EFFECT_CheckExecutePaletteFade( BTLEFF_PAL_FADE_EFFECT ) )
    {
      return FALSE;
    }
  }
  //SE再生
//  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
//      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_SEALL ) )
  if( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_SEALL )
  {
    if( ( PMSND_CheckPlaySE() ) || ( bevw->se_play_wait_flag ) || ( bevw->se_effect_enable_flag ) )
    {
      return FALSE;
    }
  }
  if( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_SE1 )
  {
    if( ( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE1 ) ) || ( bevw->se_play_wait_flag ) || ( bevw->se_effect_enable_flag ) )
    {
      return FALSE;
    }
  }
  if( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_SE2 )
  {
    if( ( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE2 ) ) || ( bevw->se_play_wait_flag ) || ( bevw->se_effect_enable_flag ) )
    {
      return FALSE;
    }
  }
  if( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_SE3 )
  {
    if( ( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE3 ) ) || ( bevw->se_play_wait_flag ) || ( bevw->se_effect_enable_flag ) )
    {
      return FALSE;
    }
  }
  if( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_PSG )
  {
    if( ( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE_PSG ) ) || ( bevw->se_play_wait_flag ) || ( bevw->se_effect_enable_flag ) )
    {
      return FALSE;
    }
  }
  if( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_SYSTEM )
  {
    if( ( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SYS ) ) || ( bevw->se_play_wait_flag ) || ( bevw->se_effect_enable_flag ) )
    {
      return FALSE;
    }
  }
  if( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_VOICE )
  {
    int i;
    if( bevw->nakigoe_wait_flag )
    { 
      return FALSE;
    }
    for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
    {
      if( bevw->voiceplayerIndex[ i ] != EFFVM_VOICEPLAYER_INDEX_NONE )
      { 
        if( PMVOICE_CheckPlay( bevw->voiceplayerIndex[ i ] ) )
        { 
          return FALSE;
        }
        else
        { 
          bevw->voiceplayerIndex[ i ] = EFFVM_VOICEPLAYER_INDEX_NONE;
        }
      }
    }
  }
  if( ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_ALL ) ||
      ( bevw->effect_end_wait_kind == BTLEFF_EFFENDWAIT_WINDOW ) )
  { 
    if( bevw->window_move_flag )
    { 
      return FALSE;
    }
  }

#ifdef CAMERA_POS_PRINT
  if( bevw->camera_flag )
  { 
    VecFx32 pos, tar;

    bevw->camera_flag = 0;
    BTLV_CAMERA_GetCameraPosition( BTLV_EFFECT_GetCameraWork(), &pos, &tar );
#ifdef CAMERA_POS_PRINT_FX32
    OS_Printf("cam_pos_x:0x%08x cam_pos_y:0x%08x cam_pos_z:0x%08x\n",pos.x,pos.y,pos.z);
    OS_Printf("cam_tar_x:0x%08x cam_tar_y:0x%08x cam_tar_z:0x%08x\n",tar.x,tar.y,tar.z);
#endif
    OS_Printf("cam_pos_x:%f cam_pos_y:%f cam_pos_z:%f\n",FX_FX32_TO_F32(pos.x),FX_FX32_TO_F32(pos.y),FX_FX32_TO_F32(pos.z));
    OS_Printf("cam_tar_x:%f cam_tar_y:%f cam_tar_z:%f\n",FX_FX32_TO_F32(tar.x),FX_FX32_TO_F32(tar.y),FX_FX32_TO_F32(tar.z));
  }
#endif

  return TRUE;
}

//============================================================================================
/**
 * @brief タイマーウエイト終了チェック
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 *
 * @retval  TRUE:タイマーウエイト終了　FALSE:タイマーウエイト動作中
 */
//============================================================================================
static  BOOL  VWF_WAIT_CHECK( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

  bevw->wait--;

  return ( bevw->wait <= 0 );
}

//============================================================================================
/**
 * @brief 一時停止終了チェック
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 *
 * @retval  TRUE:一時停止終了　FALSE:一時停止動作中
 */
//============================================================================================
static  BOOL  VWF_PAUSE_CHECK( VMHANDLE *vmh, void *context_work )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

  return ( bevw->pause_flag == 0 );
}

//============================================================================================
/**
 * @brief パーティクルテクスチャロード終了チェック
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 *
 * @retval  TRUE:一時停止終了　FALSE:一時停止動作中
 */
//============================================================================================
static  BOOL  VWF_PARTICLE_LOAD_WAIT( VMHANDLE *vmh, void *context_work )
{ 
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;

  return ( bevw->particle_tex_load == 0 );
}

//非公開関数群
//============================================================================================
/**
 * @brief 立ち位置情報の取得（ポケモン操作関連専用）
 *
 * @param[in]   bevw          エフェクト管理構造体
 * @param[in]   pos_flag      取得したいポジションフラグ
 * @param[out]  pos           取得したポジションの格納ワーク
 *
 * @retval  取得したポジション数
 */
//============================================================================================
static  int   EFFVM_GetPokePosition( BTLV_EFFVM_WORK* bevw, int pos_flag, BtlvMcssPos* pos )
{
  int pos_cnt = 1;

  switch( pos_flag ){
  case BTLEFF_POKEMON_POS_AA:
  case BTLEFF_POKEMON_POS_BB:
  case BTLEFF_POKEMON_POS_A:
  case BTLEFF_POKEMON_POS_B:
  case BTLEFF_POKEMON_POS_C:
  case BTLEFF_POKEMON_POS_D:
  case BTLEFF_POKEMON_POS_E:
  case BTLEFF_POKEMON_POS_F:
  case BTLEFF_TRAINER_POS_AA:
  case BTLEFF_TRAINER_POS_BB:
  case BTLEFF_TRAINER_POS_A:
  case BTLEFF_TRAINER_POS_B:
  case BTLEFF_TRAINER_POS_C:
  case BTLEFF_TRAINER_POS_D:
    pos[ 0 ] = pos_flag;
    break;
  case BTLEFF_POKEMON_SIDE_ATTACK:    //攻撃側
    pos[ 0 ] = bevw->attack_pos;
    if( pos[ 0 ] == BTLV_MCSS_POS_ERROR )
    {
      if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_AA ) == TRUE )
      {
        pos[ 0 ] = BTLV_MCSS_POS_AA;
      }
      else if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_A ) == TRUE )
      {
        pos[ 0 ] = BTLV_MCSS_POS_A;
      }
      else if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_C ) == TRUE )
      {
        pos[ 0 ] = BTLV_MCSS_POS_C;
      }
      else if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_C ) == TRUE )
      {
        pos[ 0 ] = BTLV_MCSS_POS_E;
      }
    }
    break;
  case BTLEFF_POKEMON_SIDE_ATTACK_PAIR: //攻撃側ペア
    if( bevw->attack_pos > BTLV_MCSS_POS_BB ){
      pos[ 0 ] = bevw->attack_pos ^ BTLV_MCSS_POS_PAIR_BIT;
    }
    else{
      pos[ 0 ] = BTLV_MCSS_POS_ERROR;
    }
    break;
  case BTLEFF_POKEMON_SIDE_DEFENCE:   //防御側
    pos[ 0 ] = bevw->defence_pos;
    if( pos[ 0 ] == BTLV_MCSS_POS_ERROR )
    {
      BtlRule rule = BTLV_EFFECT_GetBtlRule();
      pos_cnt = 0;
      if( ( rule == BTL_RULE_SINGLE ) || ( rule == BTL_RULE_ROTATION ) )
      {
        switch( bevw->param.waza_range ){
        case WAZA_TARGET_ALL:                 ///< 場に出ている全ポケ
        case WAZA_TARGET_FIELD:               ///< 場全体（天候など）
          pos_cnt = 1;
          pos[ 0 ] = bevw->attack_pos;
          if( BTLV_EFFECT_CheckExist( bevw->attack_pos ^ 1 ) )
          { 
            pos[ 1 ] = bevw->attack_pos ^ 1;
            pos_cnt++;
          }
          break;
        default:
          pos_cnt = 0;
          if( BTLV_EFFECT_CheckExist( bevw->attack_pos ^ 1 ) )
          { 
            pos[ 0 ] = bevw->attack_pos ^ 1;
            pos_cnt++;
          }
          break;
        }
      }
      else
      { 
        switch( bevw->param.waza_range ){
        case WAZA_TARGET_OTHER_SELECT:        ///< 通常ポケ（１体選択）
        case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方ポケ（１体選択）
        case WAZA_TARGET_FRIEND_SELECT:       ///< 自分以外の味方ポケ（１体選択）
        case WAZA_TARGET_ENEMY_SELECT:        ///< 相手側ポケ（１体選択）
        //case WAZA_TARGET_LONG_SELECT:         ///<対象選択表示用に定義
          //本来この選択範囲でBTLV_MCSS_POS_ERRORになることはないはず
          GF_ASSERT( 0 );
          break;
        case WAZA_TARGET_OTHER_ALL:           ///< 自分以外の全ポケ
          if( ( rule == BTL_RULE_TRIPLE ) &&
              ( bevw->attack_pos != BTLV_MCSS_POS_C ) &&
              ( bevw->attack_pos != BTLV_MCSS_POS_D ) )
          {
            BtlvMcssPos check_pos[ 4 ][ 3 ] = {
              //BTLV_MCSS_POS_A
              {
                BTLV_MCSS_POS_C,
                BTLV_MCSS_POS_D,
                BTLV_MCSS_POS_F,
              },
              //BTLV_MCSS_POS_B
              {
                BTLV_MCSS_POS_C,
                BTLV_MCSS_POS_D,
                BTLV_MCSS_POS_E,
              },
              //BTLV_MCSS_POS_E
              {
                BTLV_MCSS_POS_B,
                BTLV_MCSS_POS_C,
                BTLV_MCSS_POS_D,
              },
              //BTLV_MCSS_POS_F
              {
                BTLV_MCSS_POS_A,
                BTLV_MCSS_POS_C,
                BTLV_MCSS_POS_D,
              },
            };
            int src = ( ( bevw->attack_pos > BTLV_MCSS_POS_B ) ? 2 : 0 ) + ( bevw->attack_pos & 1 );
            int i;
  
            for( i = 0 ; i < 3 ; i++ )
            {
              if( BTLV_EFFECT_CheckExist( check_pos[ src ][ i ] ) )
              {
                pos[ pos_cnt ] = check_pos[ src ][ i ];
                pos_cnt++;
              }
            }
          }
          else
          {
            BtlvMcssPos check_pos;
  
            for( check_pos = BTLV_MCSS_POS_A ; check_pos <= BTLV_MCSS_POS_F ; check_pos++ )
            {
              if( bevw->attack_pos != check_pos )
              {
                if( BTLV_EFFECT_CheckExist( check_pos ) )
                {
                  pos[ pos_cnt ] = check_pos;
                  pos_cnt++;
                }
              }
            }
          }
          break;
        case WAZA_TARGET_UNKNOWN:             ///< ゆびをふるなど特殊型
        case WAZA_TARGET_USER:                ///< 自分のみ
        case WAZA_TARGET_ENEMY_RANDOM:        ///< 相手ポケ１体ランダム
          //エフェクトによって相手側を指示する意味で防御側をつかっているので、
          //全ポケ指示とする
        case WAZA_TARGET_ENEMY_ALL:           ///< 相手側全ポケ
          {
            BtlvMcssPos check_pos;
            BtlvMcssPos start_pos = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_A : BTLV_MCSS_POS_B;
            BtlvMcssPos end_pos = ( rule == BTL_RULE_TRIPLE ) ?  BTLV_MCSS_POS_F : BTLV_MCSS_POS_D;
  
            if( rule == BTL_RULE_TRIPLE )
            {
              if( bevw->attack_pos <= BTLV_MCSS_POS_B )
              {
                start_pos += 2;
              }
              if( bevw->attack_pos >= BTLV_MCSS_POS_E )
              {
                end_pos -= 2;
              }
            }
  
            for( check_pos = start_pos ; check_pos <= end_pos ; check_pos += 2 )
            {
              if( BTLV_EFFECT_CheckExist( check_pos ) )
              {
                pos[ pos_cnt ] = check_pos;
                pos_cnt++;
              }
            }
          }
          break;
        case WAZA_TARGET_SIDE_ENEMY:          ///< 敵側陣営
          {
            BtlvMcssPos check_pos;
            BtlvMcssPos start_pos = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_A : BTLV_MCSS_POS_B;
  
            for( check_pos = start_pos ; check_pos <= BTLV_MCSS_POS_F ; check_pos += 2 )
            {
              if( BTLV_EFFECT_CheckExist( check_pos ) )
              {
                pos[ pos_cnt ] = check_pos;
                pos_cnt++;
              }
            }
          }
          break;
        case WAZA_TARGET_FRIEND_ALL:          ///< 味方側全ポケ
        case WAZA_TARGET_SIDE_FRIEND:         ///< 自分側陣営
          {
            BtlvMcssPos check_pos;
            BtlvMcssPos start_pos = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_B : BTLV_MCSS_POS_A;
  
            for( check_pos = start_pos ; check_pos <= BTLV_MCSS_POS_F ; check_pos += 2 )
            {
              if( BTLV_EFFECT_CheckExist( check_pos ) )
              {
                pos[ pos_cnt ] = check_pos;
                pos_cnt++;
              }
            }
          }
          break;
        case WAZA_TARGET_ALL:                 ///< 場に出ている全ポケ
        case WAZA_TARGET_FIELD:               ///< 場全体（天候など）
          {
            BtlvMcssPos check_pos;
  
            for( check_pos = BTLV_MCSS_POS_A ; check_pos <= BTLV_MCSS_POS_F ; check_pos++ )
            {
              if( BTLV_EFFECT_CheckExist( check_pos ) )
              {
                pos[ pos_cnt ] = check_pos;
                pos_cnt++;
              }
            }
          }
          break;
        }
      }
    }
    break;
  case BTLEFF_POKEMON_SIDE_DEFENCE_PAIR:  //防御側ペア
    if( bevw->defence_pos > BTLV_MCSS_POS_BB ){
      pos[ 0 ] = bevw->defence_pos ^ BTLV_MCSS_POS_PAIR_BIT;
    }
    else{
      pos[ 0 ] = BTLV_MCSS_POS_ERROR;
    }
    break;
  case BTLEFF_POKEMON_ALL:
    {
      BtlvMcssPos check_pos;

      pos_cnt = 0;
      for( check_pos = BTLV_MCSS_POS_AA ; check_pos <= BTLV_MCSS_POS_F ; check_pos++ )
      {
        if( BTLV_EFFECT_CheckExist( check_pos ) )
        {
          pos[ pos_cnt ] = check_pos;
          pos_cnt++;
        }
      }
    }
    break;
  case BTLEFF_POKEMON_SIDE_MINE:
  case BTLEFF_POKEMON_SIDE_ENEMY:
    {
      BtlvMcssPos check_pos;
      BtlvMcssPos start_pos = ( pos_flag == BTLEFF_POKEMON_SIDE_MINE ) ? BTLV_MCSS_POS_AA : BTLV_MCSS_POS_BB;

      pos_cnt = 0;
      for( check_pos = start_pos ; check_pos <= BTLV_MCSS_POS_F ; check_pos += 2 )
      {
        if( BTLV_EFFECT_CheckExist( check_pos ) )
        {
          pos[ pos_cnt ] = check_pos;
          pos_cnt++;
        }
      }
    }
    break;
  default:
    OS_TPrintf("定義されていないフラグが指定されています\n");
    GF_ASSERT( 0 );
    break;
  }

  if( ( pos[ 0 ] != BTLV_MCSS_POS_ERROR ) && ( pos[ 0 ] < BTLV_MCSS_POS_MAX ) )
  {
    if( BTLV_EFFECT_CheckExist( pos[ 0 ] ) == TRUE )
    {
      pos[ 0 ] = EFFVM_ConvPokePosition( bevw, pos[ 0 ] );
    }
    else
    {
      pos[ 0 ] = BTLV_MCSS_POS_ERROR;
      pos_cnt = 0;
    }
  }

  return pos_cnt;
}

//============================================================================================
/**
 * @brief 立ち位置情報の取得（エミッタ操作関連専用）
 *
 * @param[in]   bevw          エフェクト管理構造体
 * @param[in]   pos_flag      取得したいポジションフラグ
 * @param[out]  pos           取得したポジションの格納ワーク
 *
 * @retval  取得したポジション数
 */
//============================================================================================
static  int   EFFVM_GetEmitterPosition( BTLV_EFFVM_WORK* bevw, int pos_flag, BtlvMcssPos* pos )
{
  int pos_cnt = 1;

  { 
    pos[ 0 ] = bevw->defence_pos;
    if( pos[ 0 ] == BTLV_MCSS_POS_ERROR )
    {
      BtlRule rule = BTLV_EFFECT_GetBtlRule();
      pos_cnt = 0;
      if( ( rule == BTL_RULE_SINGLE ) || ( rule == BTL_RULE_ROTATION ) )
      {
        switch( bevw->param.waza_range ){
        case WAZA_TARGET_ALL:                 ///< 場に出ている全ポケ
        case WAZA_TARGET_FIELD:               ///< 場全体（天候など）
          pos_cnt = 1;
          pos[ 0 ] = bevw->attack_pos;
          if( BTLV_EFFECT_CheckExist( bevw->attack_pos ^ 1 ) )
          { 
            pos[ 1 ] = bevw->attack_pos ^ 1;
            pos_cnt++;
          }
          break;
        default:
          pos_cnt = 0;
          if( BTLV_EFFECT_CheckExist( bevw->attack_pos ^ 1 ) )
          { 
            pos[ 0 ] = bevw->attack_pos ^ 1;
            pos_cnt++;
          }
          break;
        }
      }
      else
      { 
        switch( bevw->param.waza_range ){
        case WAZA_TARGET_OTHER_SELECT:        ///< 通常ポケ（１体選択）
        case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方ポケ（１体選択）
        case WAZA_TARGET_FRIEND_SELECT:       ///< 自分以外の味方ポケ（１体選択）
        case WAZA_TARGET_ENEMY_SELECT:        ///< 相手側ポケ（１体選択）
        //case WAZA_TARGET_LONG_SELECT:         ///<対象選択表示用に定義
          //本来この選択範囲でBTLV_MCSS_POS_ERRORになることはないはず
          GF_ASSERT( 0 );
          break;
        case WAZA_TARGET_OTHER_ALL:           ///< 自分以外の全ポケ
          if( ( rule == BTL_RULE_TRIPLE ) &&
              ( bevw->attack_pos != BTLV_MCSS_POS_C ) &&
              ( bevw->attack_pos != BTLV_MCSS_POS_D ) )
          {
            BtlvMcssPos check_pos[ 4 ][ 3 ] = {
              //BTLV_MCSS_POS_A
              {
                BTLV_MCSS_POS_C,
                BTLV_MCSS_POS_D,
                BTLV_MCSS_POS_F,
              },
              //BTLV_MCSS_POS_B
              {
                BTLV_MCSS_POS_C,
                BTLV_MCSS_POS_D,
                BTLV_MCSS_POS_E,
              },
              //BTLV_MCSS_POS_E
              {
                BTLV_MCSS_POS_B,
                BTLV_MCSS_POS_C,
                BTLV_MCSS_POS_D,
              },
              //BTLV_MCSS_POS_F
              {
                BTLV_MCSS_POS_A,
                BTLV_MCSS_POS_C,
                BTLV_MCSS_POS_D,
              },
            };
            int src = ( ( bevw->attack_pos > BTLV_MCSS_POS_B ) ? 2 : 0 ) + ( bevw->attack_pos & 1 );
            int i;
  
            for( i = 0 ; i < 3 ; i++ )
            {
              if( BTLV_EFFECT_CheckExist( check_pos[ src ][ i ] ) )
              {
                if( !BTLV_MCSS_GetVanishFlag( BTLV_EFFECT_GetMcssWork(), check_pos[ src ][ i ] ) )
                { 
                  pos[ pos_cnt ] = check_pos[ src ][ i ];
                  pos_cnt++;
                }
              }
            }
          }
          else
          {
            BtlvMcssPos check_pos;
  
            for( check_pos = BTLV_MCSS_POS_A ; check_pos <= BTLV_MCSS_POS_F ; check_pos++ )
            {
              if( bevw->attack_pos != check_pos )
              {
                if( BTLV_EFFECT_CheckExist( check_pos ) )
                {
                  if( !BTLV_MCSS_GetVanishFlag( BTLV_EFFECT_GetMcssWork(), check_pos ) )
                  { 
                    pos[ pos_cnt ] = check_pos;
                    pos_cnt++;
                  }
                }
              }
            }
          }
          break;
        case WAZA_TARGET_UNKNOWN:             ///< ゆびをふるなど特殊型
        case WAZA_TARGET_USER:                ///< 自分のみ
        case WAZA_TARGET_ENEMY_RANDOM:        ///< 相手ポケ１体ランダム
          //エフェクトによって相手側を指示する意味で防御側をつかっているので、
          //全ポケ指示とする
        case WAZA_TARGET_ENEMY_ALL:           ///< 相手側全ポケ
          {
#if 0
            BtlvMcssPos check_pos;
            BtlvMcssPos start_pos = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_A : BTLV_MCSS_POS_B;
            BtlvMcssPos end_pos = ( rule == BTL_RULE_TRIPLE ) ?  BTLV_MCSS_POS_F : BTLV_MCSS_POS_D;
  
            if( rule == BTL_RULE_TRIPLE )
            {
              if( bevw->attack_pos <= BTLV_MCSS_POS_B )
              {
                start_pos += 2;
              }
              if( bevw->attack_pos >= BTLV_MCSS_POS_E )
              {
                end_pos -= 2;
              }
            }
  
            for( check_pos = start_pos ; check_pos <= end_pos ; check_pos += 2 )
            {
              if( BTLV_EFFECT_CheckExist( check_pos ) )
              {
                if( !BTLV_MCSS_GetVanishFlag( BTLV_EFFECT_GetMcssWork(), check_pos ) )
                { 
                  pos[ pos_cnt ] = check_pos;
                  pos_cnt++;
                }
              }
            }
#endif
            if( rule == BTL_RULE_DOUBLE )
            { 
              pos[ 0 ] = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_AA : BTLV_MCSS_POS_BB;
              pos_cnt = 1;
            }
            else
            { 
              if( ( bevw->attack_pos != BTLV_MCSS_POS_C ) && ( bevw->attack_pos != BTLV_MCSS_POS_D ) )
              { 
                static  BtlvMcssPos pos_tbl[] = { 
                  NULL, NULL,
                  BTLV_MCSS_POS_D_DOUBLE, BTLV_MCSS_POS_C_DOUBLE,
                  NULL, NULL,
                  BTLV_MCSS_POS_B_DOUBLE, BTLV_MCSS_POS_A_DOUBLE,
                };
                pos[ 0 ] = pos_tbl[ bevw->attack_pos ]; 
                pos_cnt = 1;
              }
              else
              { 
                pos[ 0 ] = bevw->attack_pos ^ 1;
                pos_cnt = 1;
              }
            }
          }
          break;
        case WAZA_TARGET_SIDE_ENEMY:          ///< 敵側陣営
          {
#if 0
            BtlvMcssPos check_pos;
            BtlvMcssPos start_pos = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_A : BTLV_MCSS_POS_B;
  
            for( check_pos = start_pos ; check_pos <= BTLV_MCSS_POS_F ; check_pos += 2 )
            {
              if( BTLV_EFFECT_CheckExist( check_pos ) )
              {
                if( !BTLV_MCSS_GetVanishFlag( BTLV_EFFECT_GetMcssWork(), check_pos ) )
                { 
                  pos[ pos_cnt ] = check_pos;
                  pos_cnt++;
                }
              }
            }
#endif
            if( rule == BTL_RULE_DOUBLE )
            { 
              pos[ 0 ] = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_AA : BTLV_MCSS_POS_BB;
              pos_cnt = 1;
            }
            else
            { 
              if( ( bevw->attack_pos != BTLV_MCSS_POS_C ) && ( bevw->attack_pos != BTLV_MCSS_POS_D ) )
              { 
                static  BtlvMcssPos pos_tbl[] = { 
                  NULL, NULL,
                  BTLV_MCSS_POS_D_DOUBLE, BTLV_MCSS_POS_C_DOUBLE,
                  NULL, NULL,
                  BTLV_MCSS_POS_B_DOUBLE, BTLV_MCSS_POS_A_DOUBLE,
                };
                pos[ 0 ] = pos_tbl[ bevw->attack_pos ]; 
                pos_cnt = 1;
              }
              else
              { 
                pos[ 0 ] = bevw->attack_pos ^ 1;
                pos_cnt = 1;
              }
            }
          }
          break;
        case WAZA_TARGET_FRIEND_ALL:          ///< 味方側全ポケ
        case WAZA_TARGET_SIDE_FRIEND:         ///< 自分側陣営
          {
            BtlvMcssPos check_pos;
            BtlvMcssPos start_pos = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_B : BTLV_MCSS_POS_A;
  
            for( check_pos = start_pos ; check_pos <= BTLV_MCSS_POS_F ; check_pos += 2 )
            {
              if( BTLV_EFFECT_CheckExist( check_pos ) )
              {
                if( !BTLV_MCSS_GetVanishFlag( BTLV_EFFECT_GetMcssWork(), check_pos ) )
                { 
                  pos[ pos_cnt ] = check_pos;
                  pos_cnt++;
                }
              }
            }
          }
          break;
        case WAZA_TARGET_ALL:                 ///< 場に出ている全ポケ
        case WAZA_TARGET_FIELD:               ///< 場全体（天候など）
          {
            BtlvMcssPos check_pos;
  
            for( check_pos = BTLV_MCSS_POS_A ; check_pos <= BTLV_MCSS_POS_F ; check_pos++ )
            {
              if( BTLV_EFFECT_CheckExist( check_pos ) )
              {
                if( !BTLV_MCSS_GetVanishFlag( BTLV_EFFECT_GetMcssWork(), check_pos ) )
                { 
                  pos[ pos_cnt ] = check_pos;
                  pos_cnt++;
                }
              }
            }
          }
          break;
        }
      }
    }
  }

  return pos_cnt;
}

//============================================================================================
/**
 * @brief 立ち位置情報の取得
 *
 * @param[in] vmh     仮想マシン制御構造体へのポインタ
 * @param[in] pos_flag  取得したいポジションフラグ
 *
 * @retval  適切な立ち位置
 */
//============================================================================================
static  int   EFFVM_GetPosition( VMHANDLE *vmh, int pos_flag )
{
  int position;
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

  switch( pos_flag ){
  case BTLEFF_POKEMON_POS_AA:
  case BTLEFF_POKEMON_POS_BB:
  case BTLEFF_POKEMON_POS_A:
  case BTLEFF_POKEMON_POS_B:
  case BTLEFF_POKEMON_POS_C:
  case BTLEFF_POKEMON_POS_D:
  case BTLEFF_POKEMON_POS_E:
  case BTLEFF_POKEMON_POS_F:
  case BTLEFF_TRAINER_POS_AA:
  case BTLEFF_TRAINER_POS_BB:
  case BTLEFF_TRAINER_POS_A:
  case BTLEFF_TRAINER_POS_B:
  case BTLEFF_TRAINER_POS_C:
  case BTLEFF_TRAINER_POS_D:
    position = pos_flag;
    break;
  case BTLEFF_POKEMON_SIDE_ATTACK:    //攻撃側
    position = bevw->attack_pos;
    if( position == BTLV_MCSS_POS_ERROR )
    {
      if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_A ) == TRUE )
      {
        return BTLV_MCSS_POS_A;
      }
      else if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_C ) == TRUE )
      {
        return BTLV_MCSS_POS_C;
      }
      else if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_E ) == TRUE )
      {
        return BTLV_MCSS_POS_E;
      }
      else
      {
        return BTLV_MCSS_POS_AA;
      }
    }
    break;
  case BTLEFF_POKEMON_SIDE_ATTACK_PAIR: //攻撃側ペア
    if( bevw->attack_pos > BTLV_MCSS_POS_BB ){
      position = bevw->attack_pos ^ BTLV_MCSS_POS_PAIR_BIT;
    }
    else{
      position = BTLV_MCSS_POS_ERROR;
    }
    break;
  case BTLEFF_POKEMON_SIDE_DEFENCE:   //防御側
    position = bevw->defence_pos;
    if( position == BTLV_MCSS_POS_ERROR )
    {
      if( bevw->attack_pos & 1 )
      {
        if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_A ) == TRUE )
        {
          return BTLV_MCSS_POS_A;
        }
        else if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_C ) == TRUE )
        {
          return BTLV_MCSS_POS_C;
        }
        else if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_E ) == TRUE )
        {
          return BTLV_MCSS_POS_E;
        }
        else
        {
          return BTLV_MCSS_POS_AA;
        }
      }
      else
      {
        if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_B ) == TRUE )
        {
          return BTLV_MCSS_POS_B;
        }
        else if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_D ) == TRUE )
        {
          return BTLV_MCSS_POS_D;
        }
        else if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_F ) == TRUE )
        {
          return BTLV_MCSS_POS_F;
        }
        else
        {
          return BTLV_MCSS_POS_BB;
        }
      }
    }
    break;
  case BTLEFF_POKEMON_SIDE_DEFENCE_PAIR:  //防御側ペア
    if( bevw->defence_pos > BTLV_MCSS_POS_BB ){
      position = bevw->defence_pos ^ BTLV_MCSS_POS_PAIR_BIT;
    }
    else{
      position = BTLV_MCSS_POS_ERROR;
    }
    break;
  default:
    OS_TPrintf("定義されていないフラグが指定されています\n");
    GF_ASSERT( 0 );
    break;
  }

  position = EFFVM_ConvPosition( vmh, position );
#if 0
  if( ( position != BTLV_MCSS_POS_ERROR ) && ( position < BTLV_MCSS_POS_MAX ) )
  {
    if( BTLV_EFFECT_CheckExist( position ) == TRUE )
    {
      position = EFFVM_ConvPosition( vmh, position );
    }
    else
    {
      position = BTLV_MCSS_POS_ERROR;
    }
  }
#endif

  return position;
}

//============================================================================================
/**
 * @brief 立ち位置情報の変換（反転フラグを見て適切な立ち位置を返す：ポケモン専用）
 *
 * @param[in] bevw
 * @param[in] position
 *
 * @retval  適切な立ち位置
 */
//============================================================================================
static  int   EFFVM_ConvPokePosition( BTLV_EFFVM_WORK* bevw, BtlvMcssPos position )
{
  if( BTLV_EFFECT_CheckExist( position ) == TRUE )
  {
    if( bevw->position_reverse_flag )
    {
      position ^= 1;
    }
  }
  else
  {
    position = BTLV_MCSS_POS_ERROR;
  }

  return position;
}

//============================================================================================
/**
 * @brief 立ち位置情報の変換（反転フラグを見て適切な立ち位置を返す）
 *
 * @param[in] vmh       仮想マシン制御構造体へのポインタ
 *
 * @retval  適切な立ち位置
 */
//============================================================================================
static  int   EFFVM_ConvPosition( VMHANDLE *vmh, BtlvMcssPos position )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

  if( bevw->position_reverse_flag )
  {
    position ^= 1;
  }

  return position;
}

//============================================================================================
/**
 * @brief 方向位置情報の取得（エミッタ操作関連専用）
 *
 * @param[in]   bevw      エフェクト管理構造体
 * @param[out]  pos       取得したポジションの格納ワーク
 *
 * @retval  取得したを取得するための戦闘ルール
 */
//============================================================================================
static  BtlRule EFFVM_GetDirPosition( VMHANDLE* vmh, int* pos )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  BtlRule rule = BTLV_EFFECT_GetBtlRule();

  *pos = bevw->defence_pos;

  if( *pos == BTLV_MCSS_POS_ERROR )
  {
    if( ( rule == BTL_RULE_SINGLE ) ||
        ( rule == BTL_RULE_ROTATION ) ||
        ( bevw->execute_effect_type == EXECUTE_EFF_TYPE_BATTLE ) )
    {
      *pos = bevw->attack_pos ^ 1;
    }
    else
    { 
      switch( bevw->param.waza_range ){
      case WAZA_TARGET_OTHER_SELECT:        ///< 通常ポケ（１体選択）
      case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方ポケ（１体選択）
      case WAZA_TARGET_FRIEND_SELECT:       ///< 自分以外の味方ポケ（１体選択）
      case WAZA_TARGET_ENEMY_SELECT:        ///< 相手側ポケ（１体選択）
      //case WAZA_TARGET_LONG_SELECT:         ///<対象選択表示用に定義
        //本来この選択範囲でBTLV_MCSS_POS_ERRORになることはないはず
        GF_ASSERT( 0 );
        break;
      case WAZA_TARGET_OTHER_ALL:           ///< 自分以外の全ポケ
      case WAZA_TARGET_ENEMY_ALL:           ///< 相手側全ポケ
      case WAZA_TARGET_ALL:                 ///< 場に出ている全ポケ
      case WAZA_TARGET_FIELD:               ///< 場全体（天候など）
        if( rule == BTL_RULE_DOUBLE )
        { 
          rule = BTL_RULE_SINGLE;
          *pos = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_AA : BTLV_MCSS_POS_BB;
        }
        else
        { 
          if( ( bevw->attack_pos != BTLV_MCSS_POS_C ) && ( bevw->attack_pos != BTLV_MCSS_POS_D ) )
          { 
            static  BtlvMcssPos pos_tbl[] = { 
              NULL, NULL,
              BTLV_MCSS_POS_D, BTLV_MCSS_POS_C,
              NULL, NULL,
              BTLV_MCSS_POS_B, BTLV_MCSS_POS_A,
            };
            rule = BTL_RULE_DOUBLE;
            *pos = pos_tbl[ bevw->attack_pos ]; 
          }
          else
          { 
            *pos = bevw->attack_pos ^ 1;
          }
        }
        break;
      case WAZA_TARGET_UNKNOWN:             ///< ゆびをふるなど特殊型
      case WAZA_TARGET_USER:                ///< 自分のみ
      case WAZA_TARGET_ENEMY_RANDOM:        ///< 相手ポケ１体ランダム
        *pos = bevw->attack_pos ^ 1;
        break;
      case WAZA_TARGET_SIDE_ENEMY:          ///< 敵側陣営
        rule = BTL_RULE_SINGLE;
        *pos = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_AA : BTLV_MCSS_POS_BB;
        break;
      case WAZA_TARGET_FRIEND_ALL:          ///< 味方側全ポケ
      case WAZA_TARGET_SIDE_FRIEND:         ///< 自分側陣営
        rule = BTL_RULE_SINGLE;
        *pos = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_BB : BTLV_MCSS_POS_AA;
        break;
      }
    }
  }
  return rule;
}

//============================================================================================
/**
 * @brief パーティクルのdatIDを登録
 *
 * @param[in]   bevw    エフェクト仮想マシンのワーク構造体へのポインタ
 * @param[in]   datID   アーカイブdatID
 * @param[out]  ptc_no  datIDを登録したptc_no
 *
 * @retval  TRUE：登録した  FALSE:すでに登録していた
 */
//============================================================================================
static  BOOL  EFFVM_RegistPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID, int *ptc_no )
{
  int i;
  BOOL  ret = TRUE;

  for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ )
  {
    if( bevw->ptc_no[ i ] == datID )
    {
      ret = FALSE;
      break;
    }
  }

  if( i == PARTICLE_GLOBAL_MAX )
  {
    for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ )
    {
      if( bevw->ptc_no[ i ] == EFFVM_PTCNO_NONE )
      {
        bevw->ptc_no[ i ] = datID;
        break;
      }
    }
  }

  GF_ASSERT( i != PARTICLE_GLOBAL_MAX );

  *ptc_no = i;

  return ret;
}

//============================================================================================
/**
 * @brief パーティクルに含まれるsprのmaxを登録
 *
 * @param[in]   bevw      エフェクト仮想マシンのワーク構造体へのポインタ
 * @param[out]  ptc_no    datIDを登録したptc_no
 * @param[out]  resource  spaリソース
 */
//============================================================================================
static  void  EFFVM_RegistSprMax( BTLV_EFFVM_WORK *bevw, int ptc_no, void* resource )
{
  SPA_DATA* sd = ( SPA_DATA* )resource;

  bevw->spr_max[ ptc_no ] = sd->size;
}

//============================================================================================
/**
 * @brief パーティクルのdatIDからptc配列の添え字Noを取得
 *
 * @param[in] bevw  エフェクト仮想マシンのワーク構造体へのポインタ
 * @param[in] datID アーカイブdatID
 *
 * @retval  ptc配列の添え字No
 */
//============================================================================================
static  int EFFVM_GetPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID )
{
  int i;

  for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ )
  {
    if( bevw->ptc_no[ i ] == datID )
    {
      break;
    }
  }

  return i;
}

//============================================================================================
/**
 * @brief パーティクルに含まれるsprのmaxを取得
 *
 * @param[in] bevw    エフェクト仮想マシンのワーク構造体へのポインタ
 * @param[in] ptc_no  取得するパーティクルデータのインデックス
 *
 * @retval  sprのmax
 */
//============================================================================================
static  int EFFVM_GetSprMax( BTLV_EFFVM_WORK *bevw, int ptc_no )
{
  return bevw->spr_max[ ptc_no ];
}

//============================================================================================
/**
 * @brief エミッタ生成時に呼ばれるエミッタ初期化用コールバック関数（立ち位置から計算）
 *
 * @param[in] emit  エミッタワーク構造体へのポインタ
 */
//============================================================================================
static  void  EFFVM_InitEmitterPos( GFL_EMIT_PTR emit )
{
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw = ( BTLV_EFFVM_EMIT_INIT_WORK* )GFL_PTC_GetTempPtr();
  VecFx32 src,dst,src2;
  BOOL  minus_flag = beeiw->minus_flag;

  switch( beeiw->src ){
  case BTLEFF_PARTICLE_PLAY_POS_AA:
  case BTLEFF_PARTICLE_PLAY_POS_BB:
  case BTLEFF_PARTICLE_PLAY_POS_A:
  case BTLEFF_PARTICLE_PLAY_POS_B:
  case BTLEFF_PARTICLE_PLAY_POS_C:
  case BTLEFF_PARTICLE_PLAY_POS_D:
  case BTLEFF_PARTICLE_PLAY_POS_E:
  case BTLEFF_PARTICLE_PLAY_POS_F:
    beeiw->src = EFFVM_ConvPosition( beeiw->vmh, beeiw->src );
    break;
  case BTLEFF_PARTICLE_PLAY_SIDE_ATTACK:
  case BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE:
    beeiw->src = EFFVM_GetPosition( beeiw->vmh, beeiw->src - BTLEFF_PARTICLE_PLAY_SIDE_ATTACK + BTLEFF_POKEMON_SIDE_ATTACK);
    GF_ASSERT( beeiw->src != BTLV_MCSS_POS_ERROR );
    break;
  case BTLEFF_PARTICLE_PLAY_SIDE_NONE:
    src.x = beeiw->src_pos.x;
    src.y = beeiw->src_pos.y;
    src.z = beeiw->src_pos.z;
    break;
  case BTLEFF_PARTICLE_PLAY_POS_A_DOUBLE:
  case BTLEFF_PARTICLE_PLAY_POS_B_DOUBLE:
  case BTLEFF_PARTICLE_PLAY_POS_C_DOUBLE:
  case BTLEFF_PARTICLE_PLAY_POS_D_DOUBLE:
    BTLV_MCSS_GetPokeDefaultPosByRule( BTLV_EFFECT_GetMcssWork(), &src,
                                       beeiw->src - BTLEFF_PARTICLE_PLAY_POS_A_DOUBLE + BTLEFF_PARTICLE_PLAY_POS_A,
                                       BTL_RULE_DOUBLE );
    beeiw->src = BTLEFF_CAMERA_POS_NONE;
    break;
  }

  if( ( beeiw->src != BTLEFF_CAMERA_POS_NONE ) && ( beeiw->src != BTLEFF_PARTICLE_PLAY_SIDE_NONE ) )
  {
    BTLV_MCSS_GetPokeDefaultPos( BTLV_EFFECT_GetMcssWork(), &src, beeiw->src );
  }

  switch( beeiw->dst ){
  case BTLEFF_PARTICLE_PLAY_POS_AA:
  case BTLEFF_PARTICLE_PLAY_POS_BB:
  case BTLEFF_PARTICLE_PLAY_POS_A:
  case BTLEFF_PARTICLE_PLAY_POS_B:
  case BTLEFF_PARTICLE_PLAY_POS_C:
  case BTLEFF_PARTICLE_PLAY_POS_D:
  case BTLEFF_PARTICLE_PLAY_POS_E:
  case BTLEFF_PARTICLE_PLAY_POS_F:
    beeiw->dst = EFFVM_ConvPosition( beeiw->vmh, beeiw->dst );
    break;
  case BTLEFF_PARTICLE_PLAY_SIDE_ATTACK:
    beeiw->dst = EFFVM_GetPosition( beeiw->vmh, beeiw->dst - BTLEFF_PARTICLE_PLAY_SIDE_ATTACK + BTLEFF_POKEMON_SIDE_ATTACK );
    GF_ASSERT( beeiw->dst != BTLV_MCSS_POS_ERROR );
    break;
  case BTLEFF_PARTICLE_PLAY_SIDE_ATTACK_MINUS:
    minus_flag = TRUE;
    beeiw->dst = EFFVM_GetPosition( beeiw->vmh, beeiw->dst - BTLEFF_PARTICLE_PLAY_SIDE_ATTACK + BTLEFF_POKEMON_SIDE_ATTACK - 1 );
    GF_ASSERT( beeiw->dst != BTLV_MCSS_POS_ERROR );
    break;
  case BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE_MINUS:
    minus_flag = TRUE;
    /*fallthru*/
  case BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE:
    { 
      BtlRule rule = EFFVM_GetDirPosition( beeiw->vmh, &beeiw->dst );
      BTLV_MCSS_GetPokeDefaultPosByRule( BTLV_EFFECT_GetMcssWork(), &dst, beeiw->dst, rule );
      beeiw->dst = BTLEFF_CAMERA_POS_NONE;
    }
    break;
  case BTLEFF_PARTICLE_PLAY_SIDE_NONE:
    dst.x = beeiw->dst_pos.x;
    dst.y = beeiw->dst_pos.y;
    dst.z = beeiw->dst_pos.z;
    break;
  case BTLEFF_PARTICLE_PLAY_POS_A_DOUBLE:
  case BTLEFF_PARTICLE_PLAY_POS_B_DOUBLE:
  case BTLEFF_PARTICLE_PLAY_POS_C_DOUBLE:
  case BTLEFF_PARTICLE_PLAY_POS_D_DOUBLE:
    BTLV_MCSS_GetPokeDefaultPosByRule( BTLV_EFFECT_GetMcssWork(), &dst,
                                       beeiw->dst - BTLEFF_PARTICLE_PLAY_POS_A_DOUBLE + BTLEFF_PARTICLE_PLAY_POS_A,
                                       BTL_RULE_DOUBLE );
    beeiw->dst = BTLEFF_CAMERA_POS_NONE;
    break;
  }

  if( ( beeiw->dst != BTLEFF_CAMERA_POS_NONE ) && ( beeiw->dst != BTLEFF_PARTICLE_PLAY_SIDE_NONE ) )
  {
    BTLV_MCSS_GetPokeDefaultPos( BTLV_EFFECT_GetMcssWork(), &dst, beeiw->dst );
  }

  //ポケモンとパーティクルのプライオリティがおかしいのでZのオフセット処理をする
  { 
    int cells_src = 0;
    int cells_dst = 0;
    BtlRule rule = BTLV_EFFECT_GetBtlRule();
    //まずは一律でMCSS_DEFAULT_Z分手前に
    src.z += MCSS_DEFAULT_Z * 5;
    dst.z += MCSS_DEFAULT_Z * 5;
    //立ち位置による補正はなし
#if 0
    //戦闘ルールと立ち位置によって手前に描画しているので、その分オフセットを取る
    switch( rule ){ 
    //補正はいらない
    case BTL_RULE_SINGLE:
    case BTL_RULE_ROTATION:
      break;
    case BTL_RULE_DOUBLE:
      switch( beeiw->src ){ 
      case BTLV_MCSS_POS_B:
        cells_src = BTLV_MCSS_GetCells( BTLV_EFFECT_GetMcssWork(), beeiw->src ) + 2;
        break;
      case BTLV_MCSS_POS_C:
        cells_src = BTLV_MCSS_GetCells( BTLV_EFFECT_GetMcssWork(), beeiw->src );
        break;
      }
      switch( beeiw->dst ){ 
      case BTLV_MCSS_POS_B:
        cells_dst = BTLV_MCSS_GetCells( BTLV_EFFECT_GetMcssWork(), beeiw->dst ) + 2;
        break;
      case BTLV_MCSS_POS_C:
        cells_dst = BTLV_MCSS_GetCells( BTLV_EFFECT_GetMcssWork(), beeiw->dst );
        break;
      }
      break;
    case BTL_RULE_TRIPLE:
      switch( beeiw->src ){ 
      case BTLV_MCSS_POS_A:
      case BTLV_MCSS_POS_E:
        cells_src = BTLV_MCSS_GetCells( BTLV_EFFECT_GetMcssWork(), beeiw->src );
        break;
      case BTLV_MCSS_POS_D:
        cells_src = BTLV_MCSS_GetCells( BTLV_EFFECT_GetMcssWork(), beeiw->src ) + 2;
        break;
      }
      switch( beeiw->dst ){ 
      case BTLV_MCSS_POS_A:
      case BTLV_MCSS_POS_E:
        cells_dst = BTLV_MCSS_GetCells( BTLV_EFFECT_GetMcssWork(), beeiw->dst );
        break;
      case BTLV_MCSS_POS_D:
        cells_dst = BTLV_MCSS_GetCells( BTLV_EFFECT_GetMcssWork(), beeiw->dst ) + 2;
        break;
      }
      break;
    }
#endif
    //src.x += MCSS_DEFAULT_Z	* ( cells_src / 2 );
    src.z += MCSS_DEFAULT_Z	* cells_src;
    //dst.x += MCSS_DEFAULT_Z	* ( cells_dst / 2 );
    dst.z += MCSS_DEFAULT_Z	* cells_dst;
  }

  if( ( beeiw->ortho_mode == ORTHO_MODE_ON ) ||
      ( beeiw->ortho_mode == ORTHO_MODE_OFFSET ) )
  {
    EFFVM_CalcPosOrtho( &src, &beeiw->ofs );
    EFFVM_CalcPosOrtho( &dst, &beeiw->ofs );
  }
  else if( beeiw->ortho_mode == ORTHO_MODE_EMITTER_MOVE )
  { 
    src2.x = src.x;
    src2.y = src.y;
    src2.z = src.z;
    src.y += beeiw->ofs.y;
    EFFVM_CalcPosOrtho( &src2, &beeiw->ofs );
  }
  else
  {
    src.y += beeiw->ofs.y;
  }

  dst.y += beeiw->ofs.y;

  if( beeiw->move_type )
  {
    BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( beeiw->vmh );
    BTLV_EFFVM_EMITTER_MOVE_WORK  *beemw;
    VecFx32 rot_axis,line_vec,std_vec;
    VecFx32 line_vec_xz, line_vec_y;
    MtxFx43 rot_xz, rot_y;
    u16     angle, angle_xz, angle_y;

    bevw->temp_work[ bevw->temp_work_count ] = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                     sizeof( BTLV_EFFVM_EMITTER_MOVE_WORK ) );
    beemw = ( BTLV_EFFVM_EMITTER_MOVE_WORK *)bevw->temp_work[ bevw->temp_work_count ];
    //エミッタにテンポラリワークを設定
    GFL_PTC_SetUserData( emit, bevw->temp_work[ bevw->temp_work_count ] );
    //エミッタにコールバック関数を設定
    GFL_PTC_SetCallbackFunc( emit, &EFFVM_MoveEmitter );
    bevw->temp_work_count++;

    //サイズオーバーのアサート
    GF_ASSERT( bevw->temp_work_count < TEMP_WORK_SIZE );

    beemw->angle = 0;
    beemw->wait = 0;
    beemw->wait_tmp = 0;
    beemw->wait_tmp_plus = 0;
    beemw->move_type = beeiw->move_type;
    beemw->move_frame = beeiw->move_frame >> FX32_SHIFT;
    beemw->ortho_mode = beeiw->ortho_mode;
    if( beeiw->move_type == BTLEFF_EMITTER_MOVE_CURVE_HALF )
    {
      beemw->speed = FX_Div( ( 0x6000 << FX32_SHIFT ), beeiw->move_frame );
    }
    else if( ( beeiw->move_type == BTLEFF_EMITTER_MOVE_WAVE_H ) ||
             ( beeiw->move_type == BTLEFF_EMITTER_MOVE_WAVE_V ) )
    { 
      beemw->speed = FX_Div( ( 0x8000 << FX32_SHIFT ), beeiw->move_frame );
      beemw->wave_speed = FX_Div( ( 0x10000 << FX32_SHIFT ), beeiw->move_frame );
      beemw->wave_speed = FX_Mul( beemw->wave_speed, beeiw->wave );
    }
    else
    {
      beemw->speed = FX_Div( ( 0x8000 << FX32_SHIFT ), beeiw->move_frame );
    }
    if( beemw->speed == 0 )
    { 
      beemw->speed = 1;
    }
    beemw->radius_x = VEC_Distance( &src, &dst ) / 2;
    if( ( beeiw->move_type == BTLEFF_EMITTER_MOVE_STRAIGHT ) ||
        ( beeiw->move_type == BTLEFF_EMITTER_MOVE_OFFSET ) )
    {
      beemw->radius_y = 0;
    }
    else
    {
      beemw->radius_y = beeiw->top;
    }

    if( beeiw->move_type == BTLEFF_EMITTER_MOVE_OFFSET )
    {
      fx32  work;

      work  = src.x;
      src.x = dst.x;
      dst.x = work;
      work  = src.y;
      src.y = dst.y;
      dst.y = work;
      work  = src.z;
      src.z = dst.z;
      dst.z = work;
      dst.x += src.x;
      dst.y += src.y;
      dst.z += src.z;
    }

    line_vec.x = dst.x - src.x;
    line_vec.y = dst.y - src.y;
    line_vec.z = dst.z - src.z;
    VEC_Normalize( &line_vec, &line_vec );

    std_vec.x = FX32_ONE;
    std_vec.y = 0;
    std_vec.z = 0;

#if 0
    line_vec_xz.x = line_vec.x;
    line_vec_xz.y = 0;
    line_vec_xz.z = line_vec.z;

    VEC_Normalize( &line_vec_xz, &line_vec_xz );

    line_vec_y.x = FX32_ONE;
    line_vec_y.y = line_vec.y;
    line_vec_y.z = 0;
    VEC_Normalize( &line_vec_y, &line_vec_y );

    angle_xz = FX_AcosIdx( VEC_DotProduct( &std_vec, &line_vec_xz ) );
    angle_y = FX_AcosIdx( VEC_DotProduct( &std_vec, &line_vec_y ) );

    MTX_RotY43( &rot_xz, FX_SinIdx( angle_xz ), FX_CosIdx( angle_xz ) );
    MTX_RotZ43( &rot_y, FX_SinIdx( angle_y ), FX_CosIdx( angle_y ) );
    MTX_Concat43( &rot_xz, &rot_y, &beemw->mtx43 );

#else
    //内積を求めて角度を出す
    angle = FX_AcosIdx( VEC_DotProduct( &std_vec, &line_vec ) );

    //外積を求めて回転軸を出す
    VEC_CrossProduct( &std_vec, &line_vec, &rot_axis );
    VEC_Normalize( &rot_axis, &rot_axis );

    //回転行列を生成する
    MTX_RotAxis43( &beemw->mtx43, &rot_axis, FX_SinIdx( angle ), FX_CosIdx( angle ) );
#endif

    //平行移動行列を生成
    beemw->mtx43._30 = src.x;
    beemw->mtx43._31 = src.y;
    beemw->mtx43._32 = src.z;
  }
#if 0
  else
  { 
    //エミッタにラインズオーバー監視コールバックをセット
    GFL_PTC_SetCallbackFunc( emit, &EFFVM_CheckLinesOver );
  }
#endif

  //srcとdstが一緒のときは、方向なし
  if( ( ( beeiw->src != BTLEFF_PARTICLE_PLAY_SIDE_NONE ) && ( beeiw->dst != BTLEFF_PARTICLE_PLAY_SIDE_NONE ) ) &&
        ( beeiw->src != beeiw->dst ) )
  {
    MtxFx43 mtx43;
    VecFx32 src_vec, dst_vec, rot_axis;
    VecFx16 dir;
    u16     angle;
    u16     spin_axis = SPL_FLD_SPIN_AXIS_TYPE_Z;

    dst.x -= src.x;
    dst.y -= src.y;
    dst.z -= src.z;

    //マグネットとコンバージェンスの座標にdstを代入
    GFL_PTC_SetEmitterMagnetPos( emit, &dst );
    GFL_PTC_SetEmitterConvergencePos( emit, &dst );

    dst_vec.x = dst.x;
    dst_vec.y = 0;
    dst_vec.z = dst.z;

    GFL_PTC_GetEmitterAxis( emit, &dir );
    VEC_Set( &src_vec, dir.x, 0, dir.z );

    VEC_Normalize( &src_vec, &src_vec );
    VEC_Normalize( &dst_vec, &dst_vec );

    //内積を求めて角度を出す
    angle = FX_AcosIdx( VEC_DotProduct( &src_vec, &dst_vec ) );

    //外積を求めて回転軸を出す
    VEC_CrossProduct( &src_vec, &dst_vec, &rot_axis );
    VEC_Normalize( &rot_axis, &rot_axis );

    //回転行列を生成する
    MTX_RotAxis43( &mtx43, &rot_axis, FX_SinIdx( angle ), FX_CosIdx( angle ) );

    VEC_Set( &src_vec, dir.x, dir.y, dir.z );
    MTX_MultVec43( &src_vec, &mtx43, &src_vec );
    VEC_Normalize( &src_vec, &src_vec );
    if( minus_flag == TRUE )
    {
      src_vec.x *= -1;
      src_vec.z *= -1;
    }
    VEC_Fx16Set( &dir, src_vec.x, src_vec.y, src_vec.z );

    if( angle < 0x2000 || ( angle > 0x6000 && angle < 0xa000 ) || angle > 0xe000 )
    {
      spin_axis = SPL_FLD_SPIN_AXIS_TYPE_X;
    }
    GFL_PTC_SetEmitterSpinAxisType( emit, &spin_axis );

    GFL_PTC_SetEmitterAxis( emit, &dir );
  }
//  else
  {
    //正射影ではZで拡縮しないので、向こう側の再生時小さくする補正を入れる
    if( ( ( beeiw->ortho_mode ) && ( beeiw->src & 1 ) ) ||
          ( beeiw->ortho_mode == ORTHO_MODE_OFF ) ||
          ( beeiw->ortho_mode == ORTHO_MODE_OFFSET ) ||
          ( beeiw->ortho_mode == ORTHO_MODE_EMITTER_MOVE ) )
    {
      fx32  radius  = GFL_PTC_GetEmitterRadius( emit );
      fx32  life    = ( GFL_PTC_GetEmitterParticleLife( emit ) << FX32_SHIFT );
      fx32  scale   = GFL_PTC_GetEmitterBaseScale( emit );
      fx32  length  = GFL_PTC_GetEmitterLength( emit );

      if( beeiw->radius )
      {
        GFL_PTC_SetEmitterRadius( emit, FX_Mul( radius, beeiw->radius ) );
        GFL_PTC_SetEmitterLength( emit, FX_Mul( length, beeiw->radius ) );
      }
      if( beeiw->life )
      {
        u16 ptcl_life = FX_Mul( life, beeiw->life ) >> FX32_SHIFT;
        GFL_PTC_SetEmitterParticleLife( emit, ptcl_life );
      }
      if( beeiw->scale )
      {
        GFL_PTC_SetEmitterBaseScale( emit, (fx16)FX_Mul( scale, beeiw->scale ) );
      }
      if( beeiw->speed )
      {
        VecFx32  vel32;
        fx16  vel;

        GFL_PTC_GetEmitterVelocity( emit, &vel32 );
        vel32.x = FX_Mul( vel32.x, beeiw->speed );
        vel32.y = FX_Mul( vel32.y, beeiw->speed );
        vel32.z = FX_Mul( vel32.z, beeiw->speed );
        GFL_PTC_SetEmitterVelocity( emit, &vel32 );
        vel = GFL_PTC_GetEmitterInitVelocityPos( emit );
        GFL_PTC_SetEmitterInitVelocityPos( emit, FX_Mul( vel, beeiw->speed ) );
        vel = GFL_PTC_GetEmitterInitVelocityAxis( emit );
        GFL_PTC_SetEmitterInitVelocityAxis( emit, FX_Mul( vel, beeiw->speed ) );
      }
    }
  }

  if( beeiw->ortho_mode == ORTHO_MODE_EMITTER_MOVE )
  { 
    GFL_PTC_SetEmitterPosition( emit, &src2 );
  }
  else
  { 
    GFL_PTC_SetEmitterPosition( emit, &src );
  }

  GFL_HEAP_FreeMemory( beeiw );
}

//============================================================================================
/**
 * @brief エミッタ移動用コールバック関数
 *
 * @param[in] emit  エミッタワーク構造体へのポインタ
 * @param[in] flag  コールバックが呼ばれたタイミングを示すフラグ
 *                  SPL_EMITTER_CALLBACK_FRONT:エミッタ計算を行う直前
 *                  SPL_EMITTER_CALLBACK_BACK:エミッタ計算を行ったあと
 */
//============================================================================================
static  void  EFFVM_MoveEmitter( GFL_EMIT_PTR emit, unsigned int flag )
{
  BTLV_EFFVM_EMITTER_MOVE_WORK *beemw = ( BTLV_EFFVM_EMITTER_MOVE_WORK *)GFL_PTC_GetUserData( emit );
  VecFx32 emit_pos;
  int     angle;
  int     wave_angle;

  //check_linesover( emit, flag );

  if( ( beemw->move_frame == 0 ) || ( flag == SPL_EMITTER_CALLBACK_FRONT ) )
  {
    return;
  }

  if( beemw->move_type == BTLEFF_EMITTER_MOVE_CURVE_HALF )
  {
    if( beemw->wait )
    {
      beemw->wait--;
      return;
    }
    else
    {
      beemw->wait_tmp_plus += 0x0c;
      beemw->wait_tmp += beemw->wait_tmp_plus;
      beemw->wait = ( beemw->wait_tmp >> FX32_SHIFT );
    }
  }

  beemw->move_frame--;
  beemw->angle += beemw->speed;
  beemw->wave_angle += beemw->wave_speed;

  angle = ( ( beemw->angle >> FX32_SHIFT ) + 0xc000 ) & 0xffff;
  wave_angle = ( ( beemw->wave_angle >> FX32_SHIFT ) + 0xc000 ) & 0xffff;

  emit_pos.x = FX_SinIdx( angle );
  emit_pos.x = FX_Mul( emit_pos.x, beemw->radius_x );
  emit_pos.x += beemw->radius_x;

  if( beemw->move_type == BTLEFF_EMITTER_MOVE_WAVE_H )
  { 
    emit_pos.z = FX_CosIdx( wave_angle );
    emit_pos.z = FX_Mul( emit_pos.z, beemw->radius_y );

    emit_pos.y = 0;
  }
  else if( beemw->move_type == BTLEFF_EMITTER_MOVE_WAVE_V )
  { 
    emit_pos.y = FX_CosIdx( wave_angle );
    emit_pos.y = FX_Mul( emit_pos.y, beemw->radius_y );

    emit_pos.z = 0;
  }
  else
  { 
    emit_pos.y = FX_CosIdx( angle );
    emit_pos.y = FX_Mul( emit_pos.y, beemw->radius_y );

    emit_pos.z = 0;
  }

  MTX_MultVec43( &emit_pos, &beemw->mtx43, &emit_pos );

  //OS_Printf( "pos_x:%x pos_y:%x pos_z:%x\n", emit_pos.x, emit_pos.y, emit_pos.z );
  
  if( beemw->ortho_mode )
  {
    EFFVM_CalcPosOrtho( &emit_pos, NULL );
  }

  GFL_PTC_SetEmitterPosition( emit, &emit_pos );
}

//============================================================================================
/**
 * @brief エミッタ生成時に呼ばれるエミッタ初期化用コールバック関数（エミッタ円移動）
 *
 * @param[in] emit  エミッタワーク構造体へのポインタ
 */
//============================================================================================
static  void  EFFVM_InitEmitterCircleMove( GFL_EMIT_PTR emit )
{
  BTLV_EFFVM_EMITTER_CIRCLE_MOVE_WORK *beecmw = ( BTLV_EFFVM_EMITTER_CIRCLE_MOVE_WORK* )GFL_PTC_GetTempPtr();

  if( ( beecmw->ortho_mode ) && ( beecmw->center_pos.z < 0 ) )
  {
    GFL_PTC_SetEmitterBaseScale( emit, FX16_HALF );
  }

  //エミッタにテンポラリワークを設定
  GFL_PTC_SetUserData( emit, beecmw );
  //エミッタにコールバック関数を設定
  GFL_PTC_SetCallbackFunc( emit, &EFFVM_CircleMoveEmitter );
}

//============================================================================================
/**
 * @brief エミッタ円移動用コールバック関数
 *
 * @param[in] emit  エミッタワーク構造体へのポインタ
 * @param[in] flag  コールバックが呼ばれたタイミングを示すフラグ
 *                  SPL_EMITTER_CALLBACK_FRONT:エミッタ計算を行う直前
 *                  SPL_EMITTER_CALLBACK_BACK:エミッタ計算を行ったあと
 */
//============================================================================================
static  void  EFFVM_CircleMoveEmitter( GFL_EMIT_PTR emit, unsigned int flag )
{
  BTLV_EFFVM_EMITTER_CIRCLE_MOVE_WORK *beecmw = ( BTLV_EFFVM_EMITTER_CIRCLE_MOVE_WORK *)GFL_PTC_GetUserData( emit );
  VecFx32 emit_pos;

  //check_linesover( emit, flag );

  if( flag == SPL_EMITTER_CALLBACK_FRONT )
  {
    return;
  }

  if( beecmw->after_wait == 0 )
  {
    if( beecmw->wait == 0 )
    {
      beecmw->wait = beecmw->wait_tmp;
      if( beecmw->center & 1 )
      {
        beecmw->angle -= beecmw->speed;
      }
      else
      {
        beecmw->angle += beecmw->speed;
      }

      beecmw->angle &= 0xffff;

      emit_pos.x = FX_SinIdx( beecmw->angle );
      emit_pos.x = FX_Mul( emit_pos.x, beecmw->radius_h );
      emit_pos.x += beecmw->center_pos.x;

      emit_pos.y = beecmw->center_pos.y;

      emit_pos.z = FX_CosIdx( beecmw->angle );
      emit_pos.z = FX_Mul( emit_pos.z, beecmw->radius_v );
      emit_pos.z += beecmw->center_pos.z;

      if( beecmw->ortho_mode )
      {
        EFFVM_CalcPosOrtho( &emit_pos, NULL );
      }

      GFL_PTC_SetEmitterPosition( emit, &emit_pos );

      if( beecmw->frame )
      {
        beecmw->frame--;
      }
      else
      {
        beecmw->frame = beecmw->frame_tmp;
        beecmw->after_wait = beecmw->after_wait_tmp;
      }
    }
    else
    {
      beecmw->wait--;
    }
  }
  else
  {
    beecmw->after_wait--;
  }
}

#if 0
//============================================================================================
/**
 * @brief エミッタ円移動用コールバック関数
 *
 * @param[in] emit  エミッタワーク構造体へのポインタ
 * @param[in] flag  コールバックが呼ばれたタイミングを示すフラグ
 *                  SPL_EMITTER_CALLBACK_FRONT:エミッタ計算を行う直前
 *                  SPL_EMITTER_CALLBACK_BACK:エミッタ計算を行ったあと
 */
//============================================================================================
static  void  EFFVM_CheckLinesOver( GFL_EMIT_PTR emit, unsigned int flag )
{ 
  check_linesover( emit, flag );
}
vu32  lines_over_count = 10;
static  void  check_linesover( GFL_EMIT_PTR emit, u32 flag )
{ 
  if( flag != SPL_EMITTER_CALLBACK_FRONT ) return;

#if 0
  if( G3X_IsLineBufferUnderflow() != 0 )
  { 
    SOGABE_Printf("lines over!\n");
    //SPL_StopEmission( emit );
    { 
      SPLParticle* ptcl = SPL_GetHeadParticle( emit );
      if( ptcl )
      { 
        do
        { 
          SPL_TerminateParticleLife( ptcl );
        }while( ( ptcl = SPL_GetNextParticle( ptcl ) ) != NULL );
      }
    }
    G3X_ResetLineBufferUnderflow();
  }
#else
  if( G3X_GetRenderedLineCount() < lines_over_count )
  { 
    SOGABE_Printf("lines over?\n");
    //SPL_StopEmission( emit );
    { 
      SPLParticle* ptcl = SPL_GetHeadParticle( emit );
      if( ptcl )
      { 
        ptcl = SPL_GetNextParticle( ptcl );
        if( ptcl )
        { 
          SPL_TerminateParticleLife( ptcl );
        }
      }
    }
  }
#endif
  else
  { 
    SPL_RestartEmission( emit );
  }
}
#endif

//============================================================================================
/**
 * @brief エミッタ削除
 *
 * @param[in] ptc   削除するエミッタのGFL_PTC_PTR構造体
 */
//============================================================================================
static  void  EFFVM_DeleteEmitter( GFL_PTC_PTR ptc )
{
  void  *heap;

  heap = GFL_PTC_GetHeapPtr( ptc );
  GFL_HEAP_FreeMemory( heap );
  GFL_PTC_Delete( ptc );
}

//============================================================================================
/**
 * @brief カメラの射影モードを変更
 *
 * @param[in] bevw  エフェクト仮想マシンのワーク構造体へのポインタ
 */
//============================================================================================
static  void  EFFVM_ChangeCameraProjection( BTLV_EFFVM_WORK *bevw )
{
  if( bevw->camera_projection == BTLEFF_CAMERA_PROJECTION_PERSPECTIVE )
  {
    BTLV_MCSS_ResetOrthoMode( BTLV_EFFECT_GetMcssWork() );
  }
  else
  {
    BTLV_MCSS_SetOrthoMode( BTLV_EFFECT_GetMcssWork() );
  }
}

//============================================================================================
/**
 * @brief SE再生
 *
 * @param[in] se_no       再生するSEナンバー
 * @param[in] player      再生するPlayerNo
 * @param[in] pan         再生パン
 * @param[in] pitch       再生ピッチ
 * @param[in] vol         再生ボリューム
 * @param[in] mod_depth   再生モジュレーションデプス
 * @param[in] mod_speed   再生モジュレーションスピード
 */
//============================================================================================
static  void  EFFVM_SePlay( int se_no, int player, int pan, int pitch, int vol, int mod_depth, int mod_speed )
{
  if( player == BTLEFF_SEPLAY_DEFAULT )
  {
    PMSND_PlaySE( se_no );
    player = PMSND_GetSE_DefaultPlayerID( se_no );
  }
  else
  {
    PMSND_PlaySE_byPlayerID( se_no, player );
  }
  NNS_SndPlayerSetVolume( PMSND_GetSE_SndHandle( player ), vol );
  PMSND_SetStatusSE_byPlayerID( player, PMSND_NOEFFECT, PMSND_NOEFFECT, pan );
  PMSND_SetStatusSE_byPlayerID( player, PMSND_NOEFFECT, pitch, PMSND_NOEFFECT );
  if( mod_depth != 0 )
  {
    NNS_SndPlayerSetTrackModDepth( PMSND_GetSE_SndHandle( player ), 0xffff, mod_depth );
    NNS_SndPlayerSetTrackModSpeed( PMSND_GetSE_SndHandle( player ), 0xffff, mod_speed );
  }
}

//============================================================================================
/**
 * @brief 指定されたパラメータのワークを返す
 *
 * @param[in] bevw  エフェクト仮想マシンのワーク構造体へのポインタ
 * @param[in] param 取得したいパラメータ
 */
//============================================================================================
static  int  EFFVM_GetWork( BTLV_EFFVM_WORK* bevw, int param )
{
  int ret;

  switch( param ){
  case BTLEFF_WORK_POS_AA_WEIGHT: ///<POS_AAの体重
  case BTLEFF_WORK_POS_BB_WEIGHT: ///<POS_BBの体重
  case BTLEFF_WORK_POS_A_WEIGHT:  ///<POS_Aの体重
  case BTLEFF_WORK_POS_B_WEIGHT:  ///<POS_Bの体重
  case BTLEFF_WORK_POS_C_WEIGHT:  ///<POS_Cの体重
  case BTLEFF_WORK_POS_D_WEIGHT:  ///<POS_Dの体重
  case BTLEFF_WORK_POS_E_WEIGHT:  ///<POS_Eの体重
  case BTLEFF_WORK_POS_F_WEIGHT:  ///<POS_Fの体重
    ret = BTLV_MCSS_GetWeight( BTLV_EFFECT_GetMcssWork(), param );
    break;
  case BTLEFF_WORK_ATTACK_WEIGHT:  ///<攻撃側の体重
    ret = BTLV_MCSS_GetWeight( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos );
    break;
  case BTLEFF_WORK_WAZA_RANGE:    ///<技の効果範囲
    ret = bevw->param.waza_range;
    break;
  case BTLEFF_WORK_TURN_COUNT:    ///< ターンによって異なるエフェクトを出す場合のターン指定。（ex.そらをとぶ）
    ret = bevw->param.turn_count;
    break;
  case BTLEFF_WORK_CONTINUE_COUNT:   ///< 連続して出すとエフェクトが異なる場合の連続カウンタ（ex. ころがる）
    ret = bevw->param.continue_count;
    break;
  case BTLEFF_WORK_YURE_CNT:   ///<ボールゆれるカウント
    ret = bevw->param.yure_cnt;
    break;
  case BTLEFF_WORK_GET_SUCCESS:   ///<捕獲成功かどうか
    ret = bevw->param.get_success;
    break;
  case BTLEFF_WORK_GET_CRITICAL:   ///<クリティカルかどうか
    ret = bevw->param.get_critical;
    break;
  case BTLEFF_WORK_ITEM_NO:   ///<ボールのアイテムナンバー
    ret = bevw->param.item_no;
    break;
  case BTLEFF_WORK_SEQUENCE_WORK:   ///<汎用ワーク
    ret = bevw->sequence_work;
    break;
  case BTLEFF_WORK_ATTACK_POKEMON:
    ret = bevw->attack_pos;
    break;
  case BTLEFF_WORK_ATTACK_POKEMON_VANISH:
    ret = BTLV_MCSS_GetVanishFlag( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos );
    break;
  case BTLEFF_WORK_ATTACK_POKEMON_DIR:
    ret = bevw->attack_pos & 1;
    break;
  case BTLEFF_WORK_POS_AA_RARE:
  case BTLEFF_WORK_POS_BB_RARE:
  case BTLEFF_WORK_POS_A_RARE:
  case BTLEFF_WORK_POS_B_RARE:
  case BTLEFF_WORK_POS_C_RARE:
  case BTLEFF_WORK_POS_D_RARE:
  case BTLEFF_WORK_POS_E_RARE:
  case BTLEFF_WORK_POS_F_RARE:
    ret = ( ( BTLV_MCSS_GetStatusFlag( BTLV_EFFECT_GetMcssWork(), param - BTLEFF_WORK_POS_AA_RARE ) & BTLV_MCSS_STATUS_FLAG_RARE ) != 0 );
    break;
  case BTLEFF_WORK_ATTACK_RARE:
    ret = ( ( BTLV_MCSS_GetStatusFlag( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos ) & BTLV_MCSS_STATUS_FLAG_RARE ) != 0 );
    break;
  case BTLEFF_WORK_POS_AA_JUMP:
  case BTLEFF_WORK_POS_BB_JUMP:
  case BTLEFF_WORK_POS_A_JUMP:
  case BTLEFF_WORK_POS_B_JUMP:
  case BTLEFF_WORK_POS_C_JUMP:
  case BTLEFF_WORK_POS_D_JUMP:
  case BTLEFF_WORK_POS_E_JUMP:
  case BTLEFF_WORK_POS_F_JUMP:
    ret = BTLV_MCSS_GetNoJump( BTLV_EFFECT_GetMcssWork(), param - BTLEFF_WORK_POS_AA_JUMP );
    break;
  case BTLEFF_WORK_ATTACK_JUMP:
    ret = BTLV_MCSS_GetNoJump( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos );
    break;
  case BTLEFF_WORK_MULTI:
    ret = BTLV_EFFECT_GetMulti();
    break;
  case BTLEFF_WORK_RULE:
    ret = BTLV_EFFECT_GetBtlRule();
    break;
  case BTLEFF_WORK_TRTYPE_A:
  case BTLEFF_WORK_TRTYPE_B:
  case BTLEFF_WORK_TRTYPE_C:
  case BTLEFF_WORK_TRTYPE_D:
    ret = BTLV_EFFECT_GetTrType( param - BTLEFF_WORK_TRTYPE_A );
    break;
  case BTLEFF_WORK_POS_AA_FLY:
  case BTLEFF_WORK_POS_BB_FLY:
  case BTLEFF_WORK_POS_A_FLY:
  case BTLEFF_WORK_POS_B_FLY:
  case BTLEFF_WORK_POS_C_FLY:
  case BTLEFF_WORK_POS_D_FLY:
  case BTLEFF_WORK_POS_E_FLY:
  case BTLEFF_WORK_POS_F_FLY:
    ret = BTLV_MCSS_GetFlyFlag( BTLV_EFFECT_GetMcssWork(), param - BTLEFF_WORK_POS_AA_FLY );
    break;
  case BTLEFF_WORK_ATTACK_FLY:
    ret = BTLV_MCSS_GetFlyFlag( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos );
    break;
  case BTLEFF_WORK_ZOOM_OUT:
    ret = bevw->zoom_out_flag;
    break;
  case BTLEFF_WORK_PUSH_CAMERA_POS:
    ret = bevw->push_camera_flag;
    break;
  case BTLEFF_WORK_WCS_CAMERA_WORK:
    ret = bevw->wcs_camera_work;
    break;
  case BTLEFF_WORK_CAMERA_MOVE_IGNORE:
    ret = bevw->camera_move_ignore;
    break;
  case BTLEFF_WORK_DEFENCE_POKEMON:
    ret = bevw->defence_pos;
    break;
  default:
    //未知のパラメータです
    GF_ASSERT( 0 );
    break;
  }
  return ret;
}

//============================================================================================
/**
 * @brief 初期値ありのエミッタ生成（防御側指定を考慮した処理）
 *
 * @param[in] bevw
 * @param[in] beeiw_src
 * @param[in] ptc_no
 * @param[in] index
 */
//============================================================================================
static void EFFVM_INIT_EMITTER_POS( BTLV_EFFVM_WORK* bevw, BTLV_EFFVM_EMIT_INIT_WORK* beeiw_src, int ptc_no, int index )
{
  if( ( beeiw_src->dst == beeiw_src->src ) &&
      ( ( beeiw_src->dst == BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE ) ||
        ( beeiw_src->dst == BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE_MINUS ) ) )
  {
    if( beeiw_src->dst == BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE_MINUS )
    {
      beeiw_src->minus_flag = TRUE;
    }
    {
      BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
      int   pos_cnt = EFFVM_GetEmitterPosition( bevw, BTLEFF_POKEMON_SIDE_DEFENCE, pos );
      if( pos_cnt )
      {
        int i;

        for( i = 0 ; i < pos_cnt ; i++ )
        {
          BTLV_EFFVM_EMIT_INIT_WORK *beeiw = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                        sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );

          *beeiw = *beeiw_src;
          beeiw->src = pos[ i ];
          beeiw->dst = pos[ i ];

          if( GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw ) == PTC_NON_CREATE_EMITTER )
          {
            GFL_HEAP_FreeMemory( beeiw );
          }
        }
      }
    }
  }
  else if( ( beeiw_src->dst == beeiw_src->src ) &&
           ( bevw->param.waza_range == WAZA_TARGET_SIDE_FRIEND ) &&
           ( bevw->execute_effect_type == EXECUTE_EFF_TYPE_WAZA ) &&
           ( bevw->waza != WAZANO_OIKAZE ) &&
         ( ( beeiw_src->dst == BTLEFF_PARTICLE_PLAY_SIDE_ATTACK ) ||
           ( beeiw_src->dst == BTLEFF_PARTICLE_PLAY_SIDE_ATTACK_MINUS ) ) )
  { 
    if( beeiw_src->dst == BTLEFF_PARTICLE_PLAY_SIDE_DEFENCE_MINUS )
    {
      beeiw_src->minus_flag = TRUE;
    }
    {
      BtlvMcssPos pos[ BTLV_MCSS_POS_MAX ];
      int   pos_cnt = 0;
      BtlRule rule = BTLV_EFFECT_GetBtlRule();

      switch( rule ){ 
      case BTL_RULE_SINGLE:
      case BTL_RULE_ROTATION:
      default:
        pos[ 0 ] = bevw->attack_pos;
        pos_cnt = 1;
        break;
      case BTL_RULE_DOUBLE:
        pos[ 0 ] = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_BB : BTLV_MCSS_POS_AA;
        pos_cnt = 1;
        break;
      case BTL_RULE_TRIPLE:
        pos[ 0 ] = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_B_DOUBLE : BTLV_MCSS_POS_A_DOUBLE;
        pos[ 1 ] = ( bevw->attack_pos & 1 ) ? BTLV_MCSS_POS_D_DOUBLE : BTLV_MCSS_POS_C_DOUBLE;
        pos_cnt = 2;
        break;
      }
  
      if( pos_cnt )
      {
        int i;

        for( i = 0 ; i < pos_cnt ; i++ )
        {
          BTLV_EFFVM_EMIT_INIT_WORK *beeiw = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                        sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );

          *beeiw = *beeiw_src;
          beeiw->src = pos[ i ];
          beeiw->dst = pos[ i ];

          if( GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw ) == PTC_NON_CREATE_EMITTER )
          {
            GFL_HEAP_FreeMemory( beeiw );
          }
        }
      }
    }
  }
  else
  {
    BTLV_EFFVM_EMIT_INIT_WORK *beeiw = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                  sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );

    *beeiw = *beeiw_src;

    if( GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw ) == PTC_NON_CREATE_EMITTER )
    {
      GFL_HEAP_FreeMemory( beeiw );
    }
  }
}

//============================================================================================
/**
 * @brief エミッタ円運動初期化
 *
 * @param[in] vmh           仮想マシン制御構造体へのポインタ
 * @param[in] context_work  コンテキストワークへのポインタ
 * @param[in] ortho_mode    正射影モードかどうか(FALSE:透視射影 TRUE:正射影）
 */
//============================================================================================
static VMCMD_RESULT EFFVM_INIT_EMITTER_CIRCLE_MOVE( VMHANDLE *vmh, void *context_work, BOOL ortho_mode )
{
  BTLV_EFFVM_WORK *bevw = ( BTLV_EFFVM_WORK* )context_work;
  BTLV_EFFVM_EMITTER_CIRCLE_MOVE_WORK *beecmw;
  ARCDATID  datID   = EFFVM_ConvDatID( bevw, ( ARCDATID )VMGetU32( vmh ) );
  int       ptc_no  = EFFVM_GetPtcNo( bevw, datID );
  int       index   = ( int )VMGetU32( vmh );
  int       position = 0;
  fx32      offset_y;

#ifdef DEBUG_OS_PRINT
  OS_TPrintf("VMEC_EMITTER_CIRCLE_MOVE\n");
#endif DEBUG_OS_PRINT

  bevw->temp_work[ bevw->temp_work_count ] = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                   sizeof( BTLV_EFFVM_EMITTER_CIRCLE_MOVE_WORK ) );
  beecmw = ( BTLV_EFFVM_EMITTER_CIRCLE_MOVE_WORK *)bevw->temp_work[ bevw->temp_work_count ];
  bevw->temp_work_count++;

  beecmw->center          = ( int )VMGetU32( vmh );
  beecmw->radius_h        = ( fx32 )VMGetU32( vmh );
  beecmw->radius_v        = ( fx32 )VMGetU32( vmh );
  offset_y                = ( fx32 )VMGetU32( vmh );
  beecmw->angle           = 0;
  beecmw->frame           = ( int )VMGetU32( vmh );
  beecmw->frame_tmp       = beecmw->frame;
  beecmw->wait            = 0;
  beecmw->wait_tmp        = ( int )VMGetU32( vmh );
  beecmw->count           = ( int )VMGetU32( vmh );
  beecmw->after_wait      = 0;
  beecmw->after_wait_tmp  = ( int )VMGetU32( vmh );

  beecmw->speed = 0x10000 / beecmw->frame;

  beecmw->ortho_mode = ortho_mode;

  if( ptc_no != EFFVM_PTCNO_NO_FIND )
  {
    if( ( ortho_mode == TRUE ) && ( GFL_PTC_GetCameraPtr( bevw->ptc[ ptc_no ] ) == NULL ) )
    {
      GFL_G3D_PROJECTION  proj;
      VecFx32 Eye    = { 0, 0, 0 };          // Eye position
      VecFx32 vUp    = { 0, FX32_ONE, 0 };  // Up
      VecFx32 at     = { 0, 0, -FX32_ONE }; // Viewpoint

      proj.type = GFL_G3D_PRJORTH;
      proj.param1 = FX32_CONST( ORTHO_HEIGHT );
      proj.param2 = -FX32_CONST( ORTHO_HEIGHT );
      proj.param3 = -FX32_CONST( ORTHO_WIDTH );
      proj.param4 = FX32_CONST( ORTHO_WIDTH );
      proj.near   = FX32_ONE * 1;
      proj.far    = FX32_ONE * 512;
      proj.scaleW = FX32_ONE;

      GFL_PTC_PersonalCameraCreate( bevw->ptc[ ptc_no ], &proj, DEFAULT_PERSP_WAY, &Eye, &vUp, &at,
                                    GFL_HEAP_LOWID( bevw->heapID ) );
    }


    //サイズオーバーのアサート
    GF_ASSERT( bevw->temp_work_count < TEMP_WORK_SIZE );

    switch( beecmw->center ){
    case BTLEFF_EMITTER_CIRCLE_MOVE_ATTACK_L:
    case BTLEFF_EMITTER_CIRCLE_MOVE_ATTACK_R:
      position = EFFVM_GetPosition( vmh, BTLEFF_POKEMON_SIDE_ATTACK );
      BTLV_MCSS_GetPokeDefaultPos( BTLV_EFFECT_GetMcssWork(), &beecmw->center_pos, position );
      break;
    case BTLEFF_EMITTER_CIRCLE_MOVE_DEFENCE_L:
    case BTLEFF_EMITTER_CIRCLE_MOVE_DEFENCE_R:
      position = EFFVM_GetPosition( vmh, BTLEFF_POKEMON_SIDE_DEFENCE );
      BTLV_MCSS_GetPokeDefaultPos( BTLV_EFFECT_GetMcssWork(), &beecmw->center_pos, position );
      break;
    case BTLEFF_EMITTER_CIRCLE_MOVE_CENTER_L:
    case BTLEFF_EMITTER_CIRCLE_MOVE_CENTER_R:
      beecmw->center_pos.x = 0;
      beecmw->center_pos.y = 0;
      beecmw->center_pos.z = 0;
      break;
    }

    if( position & 1 )
    {
      beecmw->angle = 0x8000;
    }

    beecmw->center_pos.y += offset_y;

    if( GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterCircleMove, beecmw ) == PTC_NON_CREATE_EMITTER )
    {
      GFL_HEAP_FreeMemory( beecmw );
    }
  }

  return bevw->control_mode;
}

//----------------------------------------------------------------------------
/**
 *  @brief  透視射影での座標から正射影での座標に変換する
 *
 *  @param[in/out]  pos 座標変換を行う座標
 *  @param[in]      ofs オフセット座標（NULLの場合はオフセット計算をしない）
 */
//-----------------------------------------------------------------------------
static  void  EFFVM_CalcPosOrtho( VecFx32 *pos, VecFx32 *ofs )
{
  MtxFx44 mtx;
  fx32  w;

  MTX_Copy43To44( NNS_G3dGlbGetCameraMtx(), &mtx );
  MTX_Concat44( &mtx, NNS_G3dGlbGetProjectionMtx(), &mtx );

  if( ofs )
  {
    pos->x += ofs->x;
    pos->y += ofs->y;
    pos->z += ofs->z;
  }

  MTX_MultVec44( pos, &mtx, pos, &w );

  pos->x = FX_Mul( FX_Div( pos->x, w ), ORTHO_WIDTH * FX32_ONE );
  pos->y = FX_Mul( FX_Div( pos->y, w ), ORTHO_HEIGHT * FX32_ONE );
  pos->z = -pos->z;
}

//神王蟲からいただき
//----------------------------------------------------------------------------
/**
 *  @brief  4x4行列に座標を掛け合わせる
 *      Vecをx,y,z,1として計算し、計算後のVecとwを返します。
 *
 *  @param  *cp_src Vector座標
 *  @param  *cp_m 4*4行列
 *  @param  *p_dst  Vecror計算結果
 *  @param  *p_w  4つ目の要素の値
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static  void MTX_MultVec44( const VecFx32 *cp_src, const MtxFx44 *cp_m, VecFx32 *p_dst, fx32 *p_w )
{

  fx32 x = cp_src->x;
    fx32 y = cp_src->y;
    fx32 z = cp_src->z;
  fx32 w = FX32_ONE;

  GF_ASSERT( cp_src );
  GF_ASSERT( cp_m );
  GF_ASSERT( p_dst );
  GF_ASSERT( p_w );

    p_dst->x = (fx32)(((fx64)x * cp_m->_00 + (fx64)y * cp_m->_10 + (fx64)z * cp_m->_20) >> FX32_SHIFT);
    p_dst->x += cp_m->_30;  //  W=1なので足すだけ

    p_dst->y = (fx32)(((fx64)x * cp_m->_01 + (fx64)y * cp_m->_11 + (fx64)z * cp_m->_21) >> FX32_SHIFT);
    p_dst->y += cp_m->_31;//  W=1なので足すだけ

    p_dst->z = (fx32)(((fx64)x * cp_m->_02 + (fx64)y * cp_m->_12 + (fx64)z * cp_m->_22) >> FX32_SHIFT);
    p_dst->z += cp_m->_32;//  W=1なので足すだけ

  *p_w  = (fx32)(((fx64)x * cp_m->_03 + (fx64)y * cp_m->_13 + (fx64)z * cp_m->_23) >> FX32_SHIFT);
    *p_w  += cp_m->_33;// W=1なので足すだけ
}

//----------------------------------------------------------------------------
/**
 *  @brief  ボールのリソースを参照しているなら、ball_modeによりオフセット計算する
 *
 *  @param[in]  bevw  システム管理構造体
 *  @param[in]  datID リソースID
 *
 *  @retval ARCDATID  オフセット計算をしたARCDATID
 */
//-----------------------------------------------------------------------------
static  ARCDATID  EFFVM_ConvDatID( BTLV_EFFVM_WORK* bevw, ARCDATID datID )
{
  ARCDATID  ofs = 0;
#ifdef PM_DEBUG
  //デバッグ読み込みの場合はそのまま返す
  if( bevw->debug_flag == TRUE )
  {
    return datID;
  }
#endif

  if( bevw->ball_mode == BTLEFF_CAPTURE_BALL_ATTACK )
  {
    ofs = BTLV_MCSS_GetCaptureBall( BTLV_EFFECT_GetMcssWork(), bevw->attack_pos );
  }
  else if( bevw->ball_mode != BTLEFF_USE_BALL )
  {
    ofs = BTLV_MCSS_GetCaptureBall( BTLV_EFFECT_GetMcssWork(), bevw->ball_mode );
  }
  else
  {
    ofs = ITEM_GetBallID( bevw->param.item_no );
  }

  //不正なボールIDはモンスターボールにする
  if( ( ofs == BALLID_NULL ) || ( ofs > BALLID_MAX ) )
  {
    ofs = BALLID_MONSUTAABOORU;
  }

  ofs -= 1;

  switch( datID ){
  case NARC_spa_be_ball_001_1_spa:
    datID += ofs;
    break;
  case NARC_spa_be_capture_01_spa:
    if( ( ofs + 1 ) == BALLID_DORIIMUBOORU )
    { 
      //捕獲用ボールは、プレシャスボールまでなのでドリームボールはスピードボールのIDに変換
      ofs = BALLID_SUPIIDOBOORU - 1;
    }
    else if( ( ofs + 1 ) >= BALLID_PURESYASUBOORU )
    { 
      //捕獲用ボールは、プレシャスボールまでなのでそれ以降はモンスターボールのIDに変換
      ofs = BALLID_MONSUTAABOORU - 1;
    }
    datID += ofs;
    break;
  case NARC_spa_be_m_ball_01_close_spa:
  case NARC_spa_be_m_ball_01_fix_spa:
  case NARC_spa_be_m_ball_01_kaiten_spa:
  case NARC_spa_be_m_ball_01_open1_spa:
    datID += ofs * 4;
    break;
  default:
    break;
  }

  return datID;
}

//----------------------------------------------------------------------------
/**
 *  @brief  メインボリューム操作
 *
 *  @param[in]  bevw      システム管理構造体
 *  @param[in]  start_vol 開始ボリューム
 *  @param[in]  end_vol   終了ボリューム
 *  @param[in]  frame     操作フレーム数
 *
 *  @retval ARCDATID  オフセット計算をしたARCDATID
 */
//-----------------------------------------------------------------------------
static  void  EFFVM_ChangeVolume( BTLV_EFFVM_WORK* bevw, fx32 start_vol, fx32 end_vol, int frame )
{
  BTLV_EFFVM_CHANGE_VOLUME* becv;

  if( start_vol > end_vol )
  { 
    //すでにボリュームダウンしているなら、なにもしない
    if( bevw->volume_down_flag )
    { 
      bevw->volume_already_down = 1;
      return;
    }
    bevw->volume_down_flag = 1;
  }
  else
  { 
    bevw->volume_down_flag = 0;
  }

  becv = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), sizeof( BTLV_EFFVM_CHANGE_VOLUME ) );

  becv->start_vol = start_vol;
  becv->end_vol = end_vol;
  becv->bevw = bevw;
  BTLV_EFFTOOL_CalcMove( becv->start_vol, becv->end_vol, &becv->vec_vol, FX32_CONST( frame ) );
  BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( bevw->tcbsys, TCB_EFFVM_ChangeVolume, becv, 0 ), NULL, GROUP_EFFVM );
}

//----------------------------------------------------------------------------
/**
 *  @brief  voicePlayIndexの空いているINDEXを返す
 *
 *  @param[in]  bevw      システム管理構造体
 *
 *  @retval ARCDATID  オフセット計算をしたARCDATID
 */
//-----------------------------------------------------------------------------
static  int EFFVM_GetVoicePlayerIndex( BTLV_EFFVM_WORK* bevw )
{ 
  int i;

  for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
  { 
    if( bevw->voiceplayerIndex[ i ] == EFFVM_VOICEPLAYER_INDEX_NONE )
    { 
      break;
    }
  }

  //マックスオーバー
  GF_ASSERT( i != TEMOTI_POKEMAX );
  if( i == TEMOTI_POKEMAX )
  { 
    i = 0;
  }

  return i;
}

//----------------------------------------------------------------------------
/**
 *  @brief  エフェクト終了後に初期位置にいないポケモンを検索して
 *          いた場合は、バニッシュして初期位置に戻す
 *
 *  @param[in]  bevw      システム管理構造体
 */
//-----------------------------------------------------------------------------
static  void  EFFVM_CheckPokePosition( BTLV_EFFVM_WORK* bevw )
{ 
  BtlvMcssPos pos;

  for( pos = 0 ; pos < BTLV_MCSS_POS_MAX ; pos++ )
  {
    if( BTLV_MCSS_CheckExist( BTLV_EFFECT_GetMcssWork(), pos ) == TRUE )
    {
      BTLV_MCSS_CheckPositionSetInitPos( BTLV_EFFECT_GetMcssWork(), pos );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  パーティクルのちらつきを軽減するために影を消しているのの復活忘れを防ぐために終了時に復活させる
 *
 *  @param[in]  bevw      システム管理構造体
 */
//-----------------------------------------------------------------------------
static  void  EFFVM_CheckShadow( BTLV_EFFVM_WORK* bevw )
{ 
  BtlvMcssPos pos;

  for( pos = 0 ; pos < BTLV_MCSS_POS_MAX ; pos++ )
  {
    if( BTLV_MCSS_CheckExist( BTLV_EFFECT_GetMcssWork(), pos ) == TRUE )
    {
      BTLV_MCSS_SetShadowVanishFlag( BTLV_EFFECT_GetMcssWork(), pos, FALSE );
    }
  }
}

//TCB関数
//============================================================================================
/**
 * @brief SE再生
 */
//============================================================================================
static  void  TCB_EFFVM_SEPLAY( GFL_TCB* tcb, void* work )
{
  BTLV_EFFVM_SEPLAY*  bes = ( BTLV_EFFVM_SEPLAY* )work;

  if( --bes->wait == 0 )
  {
    EFFVM_SePlay( bes->se_no, bes->player, bes->pan, bes->pitch, bes->vol, bes->mod_depth, bes->mod_speed );
    BTLV_EFFECT_FreeTCB( tcb );
  }
}

static  void  TCB_EFFVM_SEPLAY_CB( GFL_TCB* tcb )
{ 
  BTLV_EFFVM_SEPLAY*  bes = ( BTLV_EFFVM_SEPLAY* )GFL_TCB_GetWork( tcb );
  bes->bevw->se_play_wait_flag = 0;
}

//============================================================================================
/**
 * @brief SEエフェクト
 */
//============================================================================================
static  void  TCB_EFFVM_SEEFFECT( GFL_TCB* tcb, void* work )
{
  BTLV_EFFVM_SEEFFECT*  bes = ( BTLV_EFFVM_SEEFFECT* )work;
  BOOL  flag = FALSE;

  if( bes->start_wait )
  {
    bes->start_wait--;
    return;
  }

  if( bes->wait == 0 )
  {
    int value;

    bes->wait = bes->wait_tmp;
    bes->value += bes->vec_value;
    value = bes->value >> FX32_SHIFT;

    if( bes->count & 1 )
    {
      if( bes->vec_value < 0 )
      {
        if( value < bes->start )
        {
          value = bes->start;
        }
      }
      else
      {
        if( value > bes->start )
        {
          value = bes->start;
        }
      }
    }
    else
    {
      if( bes->vec_value < 0 )
      {
        if( value < bes->end )
        {
          value = bes->end;
        }
      }
      else
      {
        if( value > bes->end )
        {
          value = bes->end;
        }
      }
    }
    bes->value = FX32_CONST( value );

    switch( bes->param )
    {
    case BTLEFF_SEEFFECT_PITCH:
      PMSND_SetStatusSE_byPlayerID( bes->player, PMSND_NOEFFECT, value, PMSND_NOEFFECT );
      break;
    case BTLEFF_SEEFFECT_VOLUME:
      NNS_SndPlayerSetVolume( PMSND_GetSE_SndHandle( bes->player ), value );
      break;
    case BTLEFF_SEEFFECT_PAN:
      PMSND_SetStatusSE_byPlayerID( bes->player, PMSND_NOEFFECT, PMSND_NOEFFECT, value );
      break;
    }
    if( bes->frame == 0 )
    {
      bes->frame = bes->frame_tmp;
      switch( bes->type ) {
      case BTLEFF_SEEFFECT_INTERPOLATION:
        flag = TRUE;
        break;
      case BTLEFF_SEEFFECT_ROUNDTRIP:
        if( --bes->count )
        {
          bes->vec_value *= -1;
        }
        else
        {
          flag = TRUE;
        }
        break;
      }
    }
    else
    {
      bes->frame--;
    }
  }
  else
  {
    bes->wait--;
  }

  if( flag == TRUE )
  {
    BTLV_EFFECT_FreeTCB( tcb );
  }
}

static  void  TCB_EFFVM_SEEFFECT_CB( GFL_TCB* tcb )
{ 
  BTLV_EFFVM_SEEFFECT*  bes = ( BTLV_EFFVM_SEEFFECT* )GFL_TCB_GetWork( tcb );
  bes->bevw->se_effect_enable_flag = 0;
}

//============================================================================================
/**
 * @brief 鳴き声再生
 */
//============================================================================================
static  void  TCB_EFFVM_NAKIGOE( GFL_TCB* tcb, void* work )
{
  BTLV_EFFVM_NAKIGOE* ben = ( BTLV_EFFVM_NAKIGOE* )work;

  if( --ben->wait == 0 )
  {
    int index = EFFVM_GetVoicePlayerIndex( ben->bevw );
    ben->bevw->voiceplayerIndex[ index ] = BTLV_MCSS_PlayVoice( BTLV_EFFECT_GetMcssWork(), ben->pos, ben->pitch, ben->volume,
                                                                ben->chorus_vol, ben->chorus_speed, ben->play_dir );
    //BGMのマスターボリュームを下げる
    {
      fx32  start_vol = 127 << FX32_SHIFT;
#ifdef KAGEYAMA_DEBUG
      fx32  end_vol = ( 127 * volume_down_ratio_pv / 100 ) << FX32_SHIFT;
      int   frame = volume_down_frame_pv;
#else
      fx32  end_vol = EFFVM_CHANGE_VOLUME_PV;
      int   frame = EFFVM_CHANGE_VOLUME_DOWN_FRAME_PV;
#endif
      EFFVM_ChangeVolume( ben->bevw, start_vol, end_vol, frame );
    }
    if( ben->bevw->volume_already_down == 0 )
    { 
      TCB_NONE_WORK*  tnw = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( ben->bevw->heapID ), sizeof( TCB_NONE_WORK ) );
      tnw->bevw = ben->bevw;
      BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( ben->bevw->tcbsys, TCB_EFFVM_NakigoeEndCheck, tnw, 0 ), NULL, GROUP_DEFAULT );
    }
    BTLV_EFFECT_FreeTCB( tcb );
  }
}

static  void  TCB_EFFVM_NAKIGOE_CB( GFL_TCB* tcb )
{ 
  BTLV_EFFVM_NAKIGOE* ben = ( BTLV_EFFVM_NAKIGOE* )GFL_TCB_GetWork( tcb );
  ben->bevw->nakigoe_wait_flag = 0;
}

//============================================================================================
/**
 * @brief ボリューム変化
 */
//============================================================================================
static  void  TCB_EFFVM_ChangeVolume( GFL_TCB* tcb, void* work )
{
  BTLV_EFFVM_CHANGE_VOLUME* becv = ( BTLV_EFFVM_CHANGE_VOLUME* )work;
  BOOL  ret = TRUE;

  BTLV_EFFTOOL_CheckMove( &becv->start_vol, &becv->vec_vol, &becv->end_vol, &ret );
  //PMSND_ChangeBGMVolume( 0xffff, becv->start_vol >> FX32_SHIFT );
  GFL_NET_DWC_PMSND_ChangeBGMVolume( 0xffff, becv->start_vol >> FX32_SHIFT );

  if( ret )
  {
    BTLV_EFFECT_FreeTCB( tcb );
  }
}

//============================================================================================
/**
 * @brief ウインドウ操作
 */
//============================================================================================
static  void  TCB_EFFVM_WINDOW_MOVE( GFL_TCB* tcb, void* work )
{ 
  BTLV_EFFVM_WINDOW_MOVE* bevwm = ( BTLV_EFFVM_WINDOW_MOVE* )work;

  switch( bevwm->seq_no ){ 
  case 0:

    G2_SetWnd0InsidePlane( bevwm->in_out & 0x00ff, FALSE );
    G2_SetWnd1InsidePlane( bevwm->in_out & 0x00ff, FALSE );
    G2_SetWndOutsidePlane( ( bevwm->in_out & 0xff00 ) >> 8, FALSE );
    GX_SetVisibleWnd( GX_WNDMASK_W0 | GX_WNDMASK_W1 );
    G2_SetWnd1Position( 0, 0, 0, 0 );
    bevwm->seq_no++;
    /* fallthru */
  case 1:
    if( bevwm->wait == 0 )
    { 
      bevwm->wait = bevwm->wait_tmp;
      switch( bevwm->type ){ 
      case BTLEFF_WINDOW_VERTICAL_OPEN:
        bevwm->vertical -= 0x100;
        bevwm->vertical++;
        break;
      case BTLEFF_WINDOW_VERTICAL_CLOSE:
        bevwm->vertical += 0x100;
        bevwm->vertical--;
        break;
      case BTLEFF_WINDOW_HORIZON_OPEN:
        bevwm->horizon -= 0x100;
        bevwm->horizon++;
        break;
      case BTLEFF_WINDOW_HORIZON_CLOSE:
        bevwm->horizon += 0x100;
        bevwm->horizon--;
        break;
      }
      G2_SetWnd0Position( ( bevwm->horizon & 0xff00 ) >> 8,
                          ( bevwm->vertical & 0xff00 ) >> 8,
                          ( bevwm->horizon & 0x00ff ),
                          ( bevwm->vertical & 0x00ff ) );
      if( ( bevwm->horizon & 0xff ) == 0xff )
      { 
        G2_SetWnd1Position( 1,
                            ( bevwm->vertical & 0xff00 ) >> 8,
                            0,
                            ( bevwm->vertical & 0x00ff ) );
      }
      else
      { 
        G2_SetWnd1Position( 0, 0, 0, 0 );
      }
      if( --bevwm->frame == 0 )
      {
        BTLV_EFFECT_FreeTCB( tcb );
      }
    }
    else
    {
      bevwm->wait--;
    }
    break;
  }
}

static  void  TCB_EFFVM_WINDOW_MOVE_CB( GFL_TCB* tcb )
{ 
  BTLV_EFFVM_WINDOW_MOVE* bevwm = ( BTLV_EFFVM_WINDOW_MOVE* )GFL_TCB_GetWork( tcb );
  if( bevwm->flag == FALSE )
  {  
    GX_SetVisibleWnd( GX_WNDMASK_NONE );
  }
  bevwm->bevw->window_move_flag = 0;
}

//============================================================================================
/**
 * @brief 鳴き声終了チェック
 */
//============================================================================================
static  void  TCB_EFFVM_NakigoeEndCheck( GFL_TCB* tcb, void* work )
{ 
  TCB_NONE_WORK*  tnw = ( TCB_NONE_WORK* )work;
  int i;
  for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
  {
    if( tnw->bevw->voiceplayerIndex[ i ] != EFFVM_VOICEPLAYER_INDEX_NONE )
    { 
      if( PMVOICE_CheckPlay( tnw->bevw->voiceplayerIndex[ i ] ) )
      { 
        return;
      }
      else
      { 
        tnw->bevw->voiceplayerIndex[ i ] = EFFVM_VOICEPLAYER_INDEX_NONE;
      }
    }
  }
  {
#ifdef KAGEYAMA_DEBUG
    fx32  start_vol = ( 127 * volume_down_ratio_pv / 100 ) << FX32_SHIFT;
    int   frame = volume_up_frame_pv;
#else
    fx32  start_vol = EFFVM_CHANGE_VOLUME_PV;
    int   frame = EFFVM_CHANGE_VOLUME_UP_FRAME_PV;
#endif
    fx32  end_vol = 127 << FX32_SHIFT;
    EFFVM_ChangeVolume( tnw->bevw, start_vol, end_vol, frame );
  }
  tnw->bevw->volume_already_down = 0;
  BTLV_EFFECT_FreeTCB( tcb );
}

//============================================================================================
/**
 * @brief 着地演出を待ってアニメスタートと影バニッシュオフする
 */
//============================================================================================
static  void  TCB_EFFVM_LandingWait( GFL_TCB* tcb, void* work )
{ 
  BTLV_EFFVM_LANDING_WAIT*  belw = ( BTLV_EFFVM_LANDING_WAIT* )work;
  BtlvMcssPos pos;
  BtlvMcssPos start = ( belw->side ) ? BTLV_MCSS_POS_BB : BTLV_MCSS_POS_AA;
  int finish_flag = ( belw->side ) ? 0xaa : 0x55;

  for( pos = start ; pos <= BTLV_MCSS_POS_F ; pos += 2 )
  { 
    if( BTLV_EFFECT_CheckExist( pos ) )
    { 
      if( ( !BTLV_MCSS_CheckTCBExecute( BTLV_EFFECT_GetMcssWork(), pos ) ) &&
        ( ( belw->finish_flag & BTLV_EFFTOOL_Pos2Bit( pos ) ) == 0 ) )
      { 
        BTLV_MCSS_SetAnmStopFlag( BTLV_EFFECT_GetMcssWork(), pos, FALSE );
        BTLV_MCSS_SetShadowVanishFlag( BTLV_EFFECT_GetMcssWork(), pos, FALSE );
        belw->finish_flag |= BTLV_EFFTOOL_Pos2Bit( pos );
      }
    }
    else
    {
      belw->finish_flag |= BTLV_EFFTOOL_Pos2Bit( pos );
    }
  }
  if( finish_flag == belw->finish_flag )
  { 
    BTLV_EFFECT_FreeTCB( tcb );
  }
}

//============================================================================================
/**
 * @brief パーティクルテクスチャロード
 */
//============================================================================================
static  void  TCB_EFFVM_ParticleLoad( GFL_TCB* tcb, void* work )
{ 
  TCB_PARTICLE_LOAD*  tpl = ( TCB_PARTICLE_LOAD* )work;
  u16 *v_count = (u16 *)REG_VCOUNT_ADDR;
  //VCountを確認してちらつきを防ぐ
  if( ( *v_count < MCSS_VCOUNT_BORDER_LOW ) ||
      ( *v_count > MCSS_VCOUNT_BORDER_HIGH ) )
  { 
    return;
  }
  { 
#ifdef PM_DEBUG
    u16 before = *v_count;
#endif
    GFL_PTC_LoadTex( tpl->psys );
    SOGABE_Printf("particle before:%d after:%d\n",before,*v_count);
  }
  tpl->bevw->particle_tex_load = 0;
  BTLV_EFFECT_FreeTCB( tcb );
}

//============================================================================================
/**
 * @brief カメラ移動にあわせてポケモンを拡縮
 */
//============================================================================================
static  void  set_mcss_scale_move( fx32 mul_value_m, fx32 mul_value_e, int frame, int wait, int count )
{ 
  BtlvMcssPos pos;

  BTLV_MCSS_SetScaleOffset( BTLV_EFFECT_GetMcssWork(), mul_value_m, mul_value_e );

  for( pos = BTLV_MCSS_POS_AA ; pos < BTLV_MCSS_POS_MAX ; pos++ )
  {
    if( BTLV_MCSS_CheckExist( BTLV_EFFECT_GetMcssWork(), pos ) )
    { 
      VecFx32 scale;
      fx32 def_scale = BTLV_MCSS_GetPokeDefaultScaleEx( BTLV_EFFECT_GetMcssWork(), pos, BTLV_MCSS_PROJ_ORTHO );
      if( pos & 1 )
      { 
        def_scale = FX_Mul( def_scale, mul_value_e );
      }
      else
      { 
        def_scale = FX_Mul( def_scale, mul_value_m );
      }
      VEC_Set( &scale, def_scale, def_scale, FX32_ONE );
      BTLV_MCSS_MoveDefaultScale( BTLV_EFFECT_GetMcssWork(), pos, EFFTOOL_CALCTYPE_INTERPOLATION, &scale, frame, wait, count );
    }
  }
}

//============================================================================================
/**
 * @brief カメラワークエフェクトかチェック
 */
//============================================================================================
static  BOOL  check_camera_work_effect( int eff_no )
{ 
  static  const int camera_work_eff_table[] = { 
    BTLEFF_CAMERA_WORK,
    BTLEFF_CAMERA_INIT,
    BTLEFF_WCS_CAMERA_WORK_E_M,
    BTLEFF_WCS_CAMERA_WORK_M_E,
    BTLEFF_CAMERA_WORK_ROTATE_R_L,
    BTLEFF_CAMERA_WORK_ROTATE_L_R,
    BTLEFF_CAMERA_WORK_UPDOWN_E_M,
    BTLEFF_CAMERA_WORK_UPDOWN_M_E,
    BTLEFF_CAMERA_WORK_TRIANGLE,
    BTLEFF_CAMERA_WORK_WCS_INIT,
    BTLEFF_CAMERA_WORK_INIT,
  };
  int i;

  for( i = 0 ; i < NELEMS( camera_work_eff_table ) ; i++ )
  { 
    if( camera_work_eff_table[ i ] == eff_no )
    { 
      return TRUE;
    }
  }
  return FALSE;
}

#ifdef PM_DEBUG

//デバッグ用関数
//============================================================================================
/**
 * @brief VM起動（デバッグ用アドレス指定バージョン）
 *
 * @param[in] vmh 仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void  BTLV_EFFVM_StartDebug( VMHANDLE *vmh, BtlvMcssPos from, BtlvMcssPos to, const VM_CODE *start, const DEBUG_PARTICLE_DATA *dpd, BTLV_EFFVM_PARAM* param, BTLV_EFFVM_EXECUTE_EFF_TYPE type )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  int *start_ofs;
  int table_ofs;
  int i;

  BTLV_EFFECT_FreeTCBGroup( GROUP_EFFVM );
  
  //リバース描画OFF
  BTLV_EFFECT_SetReverseDrawFlag( BTLV_EFFECT_REVERSE_DRAW_OFF );

  if( bevw->pause_flag )
  {
    BTLV_EFFVM_Restart( vmh );
    return;
  }

  if( param != NULL )
  {
    bevw->param = *param;
  }

  bevw->dat_id_cnt = 0;
  for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ )
  {
    bevw->dat_id[ i ] = 0xffffffff;
  }

  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0,
                    GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 |
                    GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
                    0, 0 );

  bevw->execute_effect_type = type;

  //BGMのマスターボリュームを下げる
  {
    fx32  start_vol = 127 << FX32_ONE;
#ifdef KAGEYAMA_DEBUG
    fx32  end_vol = ( 127 * volume_down_ratio / 100 ) << FX32_SHIFT;
    int   frame = volume_down_frame;
#else
    fx32  end_vol = EFFVM_CHANGE_VOLUME;
    int   frame = EFFVM_CHANGE_VOLUME_DOWN_FRAME;
#endif
    EFFVM_ChangeVolume( bevw, start_vol, end_vol, frame );
  }

  if( ( from != BTLV_MCSS_POS_ERROR ) && ( to != BTLV_MCSS_POS_ERROR ) )
  {
    table_ofs = script_table[ from ][ to ];
    GF_ASSERT( table_ofs != TBL_ERROR );
    if( table_ofs == TBL_ERROR )
    {
      table_ofs = TBL_AA2BB;
    }
  }
  else
  {
    table_ofs = TBL_AA2BB;
  }
  start_ofs = (int *)&start[ table_ofs ] ;

  bevw->attack_pos = from;
  bevw->defence_pos = to;
  bevw->dpd = dpd;
  bevw->debug_flag = TRUE;
  bevw->camera_projection = BTLEFF_CAMERA_PROJECTION_PERSPECTIVE;
  bevw->ball_mode = BTLEFF_USE_BALL;
  VM_Start( vmh, &start[ start_ofs[ 0 ] ] );
}

//============================================================================================
/**
 * @brief VM起動（デバッグ用コールリターンコマンドスルーバージョン）
 *
 * @param[in] vmh 仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void  BTLV_EFFVM_StartThrough( VMHANDLE *vmh, BtlvMcssPos from, BtlvMcssPos to, WazaID waza, BTLV_EFFVM_PARAM* param )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  bevw->debug_return_through = TRUE;
  BTLV_EFFVM_Start( vmh, from, to, waza, param );
}

//============================================================================================
/**
 * @brief パーティクル再生（デバッグ用）
 *
 * @param[in] vmh   仮想マシン制御構造体へのポインタ
 * @param[in] ptc   GFL_PTC_PTR
 * @param[in] index 再生するエミッタ番号
 * @param[in] src   攻撃側
 * @param[in] dst   防御側
 * @param[in] ofs_y エミッタの高さ調整値
 * @param[in] angle エミッタのパーティクル射出角度
 *
 */
//============================================================================================
void  BTLV_EFFVM_DebugParticlePlay( VMHANDLE *vmh, GFL_PTC_PTR ptc, int index, int src, int dst, fx32 ofs_y, fx32 angle )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  BTLV_EFFVM_EMIT_INIT_WORK *beeiw = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID( bevw->heapID ),
                                                                sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );

  beeiw->vmh = vmh;
  beeiw->src = src;
  beeiw->dst = dst;
  beeiw->ofs.x = 0;
  beeiw->ofs.y = ofs_y;
  beeiw->ofs.z = 0;
  beeiw->angle = angle;

  if( GFL_PTC_CreateEmitterCallback( ptc, index, &EFFVM_InitEmitterPos, beeiw ) == PTC_NON_CREATE_EMITTER )
  {
    GFL_HEAP_FreeMemory( beeiw );
  }
}

//============================================================================================
/**
 * @brief SE動的変化（デバッグ用）
 *
 * @param[in] vmh         仮想マシン制御構造体へのポインタ
 * @param[in] player      変化させるPlayerNo
 * @param[in] type        変化タイプ
 * @param[in] param       変化させるパラメータ
 * @param[in] start       開始値
 * @param[in] end         終了値
 * @param[in] start_wait  開始ウエイト
 * @param[in] frame       開始値から終了値に変化するまでのフレーム数
 * @param[in] wait        1フレームごとのウエイト
 * @param[in] count       変化タイプが往復だったときの往復回数
 *
 */
//============================================================================================
void  BTLV_EFFVM_DebugSeEffect( VMHANDLE *vmh, int player, int type, int param, int start, int end, int start_wait, int frame, int wait, int count )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  BTLV_EFFVM_SEEFFECT*  bes = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID( bevw->heapID ), sizeof( BTLV_EFFVM_SEEFFECT ) );

  bes->bevw     = bevw;

  bes->player     = player;
  bes->type       = type;
  bes->param      = param;
  bes->start      = start;
  bes->end        = end;
  bes->start_wait = start_wait;
  bes->frame      = frame;
  bes->frame_tmp  = bes->frame;
  bes->wait       = 0;
  bes->wait_tmp   = wait;
  bes->count      = count * 2;

  if( ( bes->type == BTLEFF_SEEFFECT_ROUNDTRIP ) && ( bes->count == 0 ) )
  {
    bes->count = 2;
  }

  bes->value = FX32_CONST( bes->start );
  bes->vec_value = FX_Div( FX32_CONST( bes->end - bes->start ) , FX32_CONST( bes->frame ) );

  BTLV_EFFECT_SetTCB( GFL_TCB_AddTask( bevw->tcbsys, TCB_EFFVM_SEEFFECT, bes, 0 ), TCB_EFFVM_SEEFFECT_CB, GROUP_EFFVM );
}

//============================================================================================
/**
 * @brief DEBUG_PARTICLE_DATAの配列の添え字をARCDATIDから取得する
 *
 * @param[in] bevw  エフェクト管理構造体へのポインタ
 * @param[in] datID 取得するARCDATID
 * @param[in] type  読み込むARCDATIDのタイプ（タイプによって、読み込むファイル数が違う）
 */
//============================================================================================
static  u32   BTLV_EFFVM_GetDPDNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID, DPD_TYPE type )
{
  int i;

  for( i = 0 ; i < bevw->dat_id_cnt ; i++ )
  {
    if( bevw->dat_id[ i ] == datID )
    {
      break;
    }
  }
  if( i == bevw->dat_id_cnt )
  {
    GF_ASSERT( bevw->dat_id_cnt < PARTICLE_GLOBAL_MAX );
    switch( type ){
    case DPD_TYPE_PARTICLE:
      bevw->dat_id[ i ] = datID;
      bevw->dat_id_cnt++;
      break;
    case DPD_TYPE_BG:
      {
        int j;
        for( j = 0 ; j < 3 ; j++ )
        {
          bevw->dat_id[ i + j ] = datID;
          bevw->dat_id_cnt++;
        }
      }
      break;
    }
  }
  return i;
}

//============================================================================================
/**
 * @brief リソースアンパック情報クリア
 *        NCGR系はアンパックを1回しかしてはいけないので、アンパック情報を保持している
 *        リソースを読み込んだタイミングでアンパック情報をクリアする
 *
 * @param[in] vmh 仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void  BTLV_EFFVM_ClearUnpackInfo( VMHANDLE *vmh )
{
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  int i;

  for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ )
  {
    bevw->unpack_info[ i ] = NULL;
  }
}

//============================================================================================
/**
 * @brief 技エフェクトの発動者を取得
 *
 * @param[in] bevw  エフェクト管理構造体へのポインタ
 */
//============================================================================================
BtlvMcssPos BTLV_EFFVM_GetAttackPos( VMHANDLE* vmh )
{ 
  BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
  return bevw->attack_pos;
}

#endif PM_DEBUG

