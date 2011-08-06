//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon.c
 *	@brief  無限ダンジョン
 *	@author	Masayuki Onoue
 *	@date		2011.02.25
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "fldmmdl.h"
#include "infinity_dungeon.h"
#include "infinity_dungeon_local_def.h"
#include "system/gfl_use.h"
#include "scrcmd_work.h"
#include "fieldmap/zone_id.h"
#include "event_mapchange.h"
#include "event_field_fade.h"
#include "field/field_const.h"
#include "infinity_dungeon_warp_dir.h"
#include "savedata/infinity_dungeon_save.h"
#include "../../../resource/fldmapdata/build_model/output/buildmodel_indoor.naix"
#include "../../../resource/fldmapdata/script/infinity_dungeon_scr_def.h"
#include "../../../resource/fldmapdata/script/trainer_def.h"
#include "field/infinity_dungeon_def.h"
#include "fld_btl_inst_tool.h"
#include "event_battle.h"
#include "system/main.h"  //HEAPID_FIELDMAP
#include "message.naix"
#include "tr_tool/ex_trainer_def.h"
#include "msg/script/msg_infinity_dungeon_scr.h"
#include "infinity_dungeon_gimmick.h"
#include "gmk_tmp_wk.h"
#include "arc/fieldmap/fldmmdl_objcode.h"   //for OBJCODE
#include "../../../resource/fldmapdata/eventdata/define/total_header.h"

// グローバル変数
#ifdef PM_DEBUG
extern int  g_debug_inf_tr_pos_type;
extern BOOL g_debug_inf_data_change;
extern u32  g_debug_inf_master_seed;
extern BOOL g_debug_inf_is_presentation;
#endif

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//-------------------------------------
///	部屋位置データ
//=====================================
typedef struct {
  int x;
  int y;
}DIG_POS;

//-------------------------------------
///	迷路生成用ワーク
//=====================================
typedef struct {
  int data_index;
  DIG_POS now_pos;
  DIG_POS dig_pos_list[INF_MAX_SEARC_CNT];
}INF_DIG_DATA;

//-------------------------------------
// グリッド座標データ
//=====================================
typedef struct {
  s32 x;
  s32 y;
  s32 z;
}GRID_DATA;

//-------------------------------------
///	ルームデータ
//=====================================
typedef struct{
  u8  tr_pos_type;
  u8 room_type;
  BOOL can_move[4];
  u8  room_id;
}ROOM_DATA;

//-------------------------------------
///	ダンジョン内トレーナーデータ
//=====================================
typedef struct{
  u16 tr_id[INFINITY_DUNGEON_TRID_MAX]; // 全トレーナーランダム配列
}DUNGEON_TRAINER;

//-------------------------------------
///	無限ダンジョン用シード
//=====================================
typedef struct 
{
  s32 master_seed;
  s32 tr_data_seed;
  s32 dungeon_data_seed;
  u16 tr_seed[INFINITYDUNGEON_FLOOR_MAX][INFINITY_DUNGEON_TRID_MAX]; // トレーナーシード
}INF_DUNGEON_SEED;

//-------------------------------------
///	無限ダンジョンデータ
//=====================================
struct _INF_DUNGEON_DATA{
  INF_DUNGEON_SEED  seed;
  ROOM_DATA         room_data[INFINITYDUNGEON_FLOOR_MAX][ROOM_MAX];
  DUNGEON_TRAINER   tr_data[INFINITYDUNGEON_FLOOR_MAX];
  u16               recover_data[INFINITY_DUNGEON_RECOVERTR_MAX];
  u16               recover_room_id[INFINITYDUNGEON_FLOOR_MAX];
  u8                version;
};

//-------------------------------------
///	レベルデータ
//=====================================
typedef struct {
  u16 level_max;
  u16 level_min;
}INF_LEVEL_TABLE_PARAM;

//-------------------------------------
///	ヒントメッセージ取得用ワーク
//=====================================
typedef struct{
  INF_DUNGEON_DATA      *p_dungeon_data;
  u8                    talk_type;  // 会話タイプ
  INFINITYDUNGEON_SAVE  *p_save;
}TRHINT_WK;

//=============================================================================
/**
 *					データ定義
*/
//=============================================================================

//-------------------------------------
// ゾーンテーブル(ブラック)
//=====================================
static const u16 sc_black_zone_table[] = {
  ZONE_ID_D31R0401, // エレベーター
  ZONE_ID_D31R0601, // ボス
  ZONE_ID_D31R0901, // フラッシュ
  ZONE_ID_D31R0701, // 波乗り
  ZONE_ID_D31R1201, // デフォルト
  ZONE_ID_D31R0201, // 受付
};

//-------------------------------------
// ゾーンテーブル(ホワイト)
//=====================================
static const u16 sc_white_zone_table[] = {
  ZONE_ID_D31R0301, // エレベーター
  ZONE_ID_D31R0501, // ボス
  ZONE_ID_D31R0801, // かいりき
  ZONE_ID_D31R1001, // いあいぎり
  ZONE_ID_D31R1101, // デフォルト
  ZONE_ID_D31R0101, // 受付
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(ブラック)
//=====================================
static const GRID_DATA sc_black_elevator_pos = {
  6 , 0 , 0 ,
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(ホワイト)
//=====================================
static const GRID_DATA sc_white_elevator_pos = {
  6 , 0 , 0 ,
};

//-------------------------------------
// エレベータ部屋のエレベータ位置(受付)
//=====================================
static const GRID_DATA sc_reception_elevator_pos[] = {
  7 , 0 , 2 , // 左
  11, 0 , 2 , // 右
};

//-------------------------------------
// マップタイプごとの移動先テーブル(ブラック)
//=====================================
static const GRID_DATA sc_black_warp_pos_table[ROOM_TYPE_MAX][WARP_DIR_MAX] = {
  // エレベーター部屋
  {
    { 6 , 0 , 13}, // 下
    { 6 , 0 , 1}, // 上
    { 12 , 0 , 7}, // 右
    { 0 , 0 , 7}, // 左
  },
  // ボス部屋
  {
    { 6 , 0 , 14},// 下
    { 6 , 0 , 14},// 上
    { 6 , 0 , 14},// 右
    { 6 , 0 , 14},// 左
  },
  // 特殊部屋１
  {
    { 6 , 0 , 13}, // 下
    { 6 , 0 , 1}, // 上
    { 12 , 0 , 7}, // 右
    { 0 , 0 , 7}, // 左
  },
  // 特殊部屋２
  {
    { 6 , 0 , 13}, // 下
    { 6 , 0 , 1}, // 上
    { 12 , 0 , 7}, // 右
    { 0 , 0 , 7}, // 左
  },
  // 通常部屋
  {
     { 6 , 0 , 13}, // 下
    { 6 , 0 , 1}, // 上
    { 12 , 0 , 7}, // 右
    { 0 , 0 , 7}, // 左
  },
};

//-------------------------------------
// マップタイプごとの移動先テーブル(ホワイト)
//=====================================
static const GRID_DATA sc_white_warp_pos_table[ROOM_TYPE_MAX][WARP_DIR_MAX] = {
  // エレベーター部屋
  {
    { 8 , 0 , 17}, // 下
    { 8 , 0 , 1}, // 上
    { 16 , 0 , 9}, // 右
    { 0 , 0 , 9}, // 左
  },
  // ボス部屋
  {
    { 8 , 0 , 18}, // 下
    { 8 , 0 , 18}, // 上
    { 8 , 0 , 18}, // 右
    { 8 , 0 , 18}, // 左
  },
  // 特殊部屋１
  {
    { 8 , 0 , 17}, // 下
    { 8 , 0 , 1}, // 上
    { 16 , 0 , 9}, // 右
    { 0 , 0 , 9}, // 左
  },
  // 特殊部屋２
  {
    { 8 , 0 , 17}, // 下
    { 8 , 0 , 1}, // 上
    { 16 , 0 , 9}, // 右
    { 0 , 0 , 9}, // 左
  },
  // 通常部屋
  {
    { 8 , 0 , 17}, // 下
    { 8 , 0 , 1}, // 上
    { 16 , 0 , 9}, // 右
    { 0 , 0 , 9}, // 左
  },
};

//-------------------------------------
// 配置モデル配置位置(ブラック)
//=====================================
static const GRID_DATA sc_black_bmodel_pos_table[ROOM_TYPE_MAX][WARP_DIR_MAX] = {
  // エレベーター部屋
  {
    { 6 , 0 , 2}, // 上
    { 6 , 0 , 13}, // 下
    { 1 , 0 , 7}, // 左
    { 11 , 0 , 7}, // 右
  },
  // ボス部屋
  {
    { 6 , 0 , 14},// 上
    { 6 , 0 , 14},// 下
    { 6 , 0 , 14},// 左
    { 6 , 0 , 14},// 右
  },
  // 特殊部屋１
  {
    { 6 , 0 , 2}, // 上
    { 6 , 0 , 13}, // 下
    { 1 , 0 , 7}, // 左
    { 11 , 0 , 7}, // 右
  },
  // 特殊部屋２
  {
    { 6 , 0 , 2}, // 上
    { 6 , 0 , 13}, // 下
    { 1 , 0 , 7}, // 左
    { 11 , 0 , 7}, // 右
  },
  // 通常部屋
  {
    { 6 , 0 , 2}, // 上
    { 6 , 0 , 13}, // 下
    { 1 , 0 , 7}, // 左
    { 11 , 0 , 7}, // 右
  },
};
//-------------------------------------
// 配置モデル配置位置(ホワイト)
//=====================================
static const GRID_DATA sc_white_bmodel_pos_table[ROOM_TYPE_MAX][WARP_DIR_MAX] = {
  // エレベーター部屋
  {
    { 8 , 0 , 1}, // 上
    { 8 , 0 , 17}, // 下
    { 0 , 0 , 9}, // 左
    { 16 , 0 , 9}, // 右
  },
  // ボス部屋
  {
    { 8 , 0 , 18}, // 上
    { 8 , 0 , 18}, // 下
    { 8 , 0 , 18}, // 左
    { 8 , 0 , 18}, // 右
  },
  // 特殊部屋１
  {
    { 8 , 0 , 1}, // 上
    { 8 , 0 , 17}, // 下
    { 0 , 0 , 9}, // 左
    { 16 , 0 , 9}, // 右
  },
  // 特殊部屋２
  {
    { 8 , 0 , 1}, // 上
    { 8 , 0 , 17}, // 下
    { 0 , 0 , 9}, // 左
    { 16 , 0 , 9}, // 右
  },
  // 通常部屋
  {
    { 8 , 0 , 1}, // 上
    { 8 , 0 , 17}, // 下
    { 0 , 0 , 9}, // 左
    { 16 , 0 , 9}, // 右
  },
};

//-------------------------------------
// エリアごとのエリアテーブル
//=====================================
static const INF_LEVEL_TABLE_PARAM sc_level_table[] = {
  { 40 , 45 },   // エリア 1
  { 45 , 50 },   // エリア 2
  { 50 , 55 },   // エリア 3
  { 55 , 60 },   // エリア 4
  { 60 , 65 },   // エリア 5
  { 65 , 70 },   // エリア 6
  { 70 , 75 },   // エリア 7
  { 75 , 80 },   // エリア 8
  { 80 , 85 },   // エリア 9
  { 85 , 90 },   // エリア 10
};

//-------------------------------------
// エリアごとのエリアテーブル
//=====================================
static const u8 sc_pow_rnd_table[] = {
  3*1, // エリア 1
  3*2, // エリア 2
  3*3, // エリア 3
  3*4, // エリア 4
  3*5, // エリア 5
  3*6, // エリア 6
  3*7, // エリア 7
  3*8, // エリア 8
  3*9, // エリア 9
  31, // エリア 10
};

//-------------------------------------
// トレーナータイプからヒントメッセージタイプに変換
//=====================================
static const struct 
{
  s32 obj_code;
  u8  hint_type;
  u8  talk_type;
} sc_talk_pattern_table[INFINITY_DUNGEON_TALK_PATTERN_MAX] =
{
  {MIDDLEMAN1,  INFTR_HINT_TYPE_UP,           INFTR_TALK_TYPE_MIDDLE_M  },
  {MIDDLEWOMAN1,INFTR_HINT_TYPE_DOWN,         INFTR_TALK_TYPE_MIDDLE_W  },
  {BAKER,       INFTR_HINT_TYPE_NONE,         INFTR_TALK_TYPE_MIDDLE_W  },
  {WOMAN3,      INFTR_HINT_TYPE_NONE,         INFTR_TALK_TYPE_YOUNG_W   },
  {FIGHTERM,    INFTR_HINT_TYPE_UP,           INFTR_TALK_TYPE_YOUNG_M   },
  {FIGHTERW,    INFTR_HINT_TYPE_DOWN,         INFTR_TALK_TYPE_YOUNG_W   },
  {BADMAN,      INFTR_HINT_TYPE_BOSS_FLOOR,   INFTR_TALK_TYPE_YOUNG_M   },
  {POLICEMAN,   INFTR_HINT_TYPE_BOSS_DIR,     INFTR_TALK_TYPE_MIDDLE_M  },
  {MOUNTMAN,    INFTR_HINT_TYPE_UP,           INFTR_TALK_TYPE_MIDDLE_M  },
  {CLEANINGM,   INFTR_HINT_TYPE_DOWN,         INFTR_TALK_TYPE_YOUNG_M   }, // 10

  {WORKMAN,     INFTR_HINT_TYPE_BOSS_FLOOR,   INFTR_TALK_TYPE_OLD_M     },
  {WORKMAN2,    INFTR_HINT_TYPE_BOSS_DIR,     INFTR_TALK_TYPE_YOUNG_M   },
  {ESPM,        INFTR_HINT_TYPE_UP,           INFTR_TALK_TYPE_YOUNG_M   },
  {ESPW,        INFTR_HINT_TYPE_DOWN,         INFTR_TALK_TYPE_GIRL      },
  {ASSISTANTM,  INFTR_HINT_TYPE_BOSS_FLOOR,   INFTR_TALK_TYPE_YOUNG_M   },
  {ASSISTANTW,  INFTR_HINT_TYPE_BOSS_DIR,     INFTR_TALK_TYPE_YOUNG_W   },
  {BOY3,        INFTR_HINT_TYPE_DOWN,         INFTR_TALK_TYPE_BOY       },
  {GIRL3,       INFTR_HINT_TYPE_UP,           INFTR_TALK_TYPE_GIRL      },
  {WAITER,      INFTR_HINT_TYPE_BOSS_DIR,     INFTR_TALK_TYPE_YOUNG_M   },
  {WAITRESS,    INFTR_HINT_TYPE_BOSS_FLOOR,   INFTR_TALK_TYPE_YOUNG_W   }, // 20

  {BOY2,        INFTR_HINT_TYPE_NONE,         INFTR_TALK_TYPE_BOY       },
  {GIRL2,       INFTR_HINT_TYPE_NONE,         INFTR_TALK_TYPE_GIRL      },
  {BACKPACKERM, INFTR_HINT_TYPE_UP,           INFTR_TALK_TYPE_YOUNG_M   },
  {BACKPACKERW, INFTR_HINT_TYPE_DOWN,         INFTR_TALK_TYPE_YOUNG_W   },
  {BOY4,        INFTR_HINT_TYPE_MEDIC_DIR,    INFTR_TALK_TYPE_BOY       },
  {GIRL4,       INFTR_HINT_TYPE_BOSS_MONSTER, INFTR_TALK_TYPE_GIRL      },
  {LADY,        INFTR_HINT_TYPE_BOSS_MONSTER, INFTR_TALK_TYPE_OLD_W     },
  {GENTLEMAN,   INFTR_HINT_TYPE_MEDIC_DIR,    INFTR_TALK_TYPE_OLD_M     },
  {BREEDERM,    INFTR_HINT_TYPE_MEDIC_DIR,    INFTR_TALK_TYPE_YOUNG_M   },
  {BREEDERW,    INFTR_HINT_TYPE_BOSS_MONSTER, INFTR_TALK_TYPE_YOUNG_W   }, // 30
};

//-------------------------------------
// 各エリアのフロアサイズ
//=====================================
static const struct 
{
  int fs_x;
  int fs_y;
}sc_floor_size[INFINITY_DUNGEON_AREA_ID_MAX] =
{
  { 3 ,3 }, // エリア 1
  { 3 ,3 }, // エリア 2
  { 3 ,3 }, // エリア 3
  { 3 ,3 }, // エリア 4
  { 3 ,3 }, // エリア 5
  { 4 ,4 }, // エリア 6
  { 4 ,4 }, // エリア 7
  { 4 ,4 }, // エリア 8
  { 4 ,4 }, // エリア 9
  { 4 ,4 }, // エリア 10
};

// 配置データ
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_move_type.cdat"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_pos_type.cdat"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_room_setup_data.cdat"

// トレーナーデータ
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tr_area_data.cdat"

// チュートリアルマップデータ
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tutorial_data.cdat"

//-------------------------------------
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  VecFx32 warp_pos;       // 遷移先でのマップ位置
  s32     zone_id;        // 遷移先ゾーンID
}NEXT_MAP_DATA;

//-------------------------------------
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  GAMESYS_WORK          *p_gsys;
  GAMEDATA              *p_gdata;
  FIELDMAP_WORK         *p_field_work;
  INFINITYDUNGEON_SAVE  *p_save;
  FLDMAPPER             *p_g3d_mapper;
  FIELD_BMODEL_MAN      *p_bmodel_man;
  MMDLSYS               *p_mmdlsys;
  INF_DUNGEON_DATA      *p_dungeon_data;
  HEAPID                heapID;

  // 移動タイプ
  u8                    map_change_type;

  // 移動データ
  u16                   warp_dir;
  NEXT_MAP_DATA         next_map_data;
  const u16             *p_zone_data;

  // フロア
  s32 now_room;
  s32 now_floor;
  s32 next_room;
  s32 next_floor;

  // エリア(フロアから算出)
  u16 now_area;

}MAP_CHAGE_EVENT_WORK;

// ヒントタイプごとのプロセステーブル
typedef u32 (*HINTMSGPROC)(TRHINT_WK*);

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

// 共通
static void InitWorkData( MAP_CHAGE_EVENT_WORK *p_wk ,GAMESYS_WORK *p_gsys);
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk );
static void SetReceptionWarpData( MAP_CHAGE_EVENT_WORK *p_wk );
static void GetFloorData( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data );
static void SetDungeonSeed( INF_DUNGEON_SEED *p_seed ,u32 master_seed );
static u8 GetTrainerTalkType( u16 obj_code );
static u8 GetTrainerHintType( u16 obj_code);
static u8 GetMemberNum( u8 pos_type );
static u8 GetRoomType( u8 pos_type );
static u8 GetRoomVersion( u8 pos_type );
static int GetSetTrainerIDTable( INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,u32 *p_trid_array ,GFL_STD_RandContext *rand_context);
static u8 GetDungeonFloorFromArea( u8 now_floor ,u8 now_area );

// ゾーン依存
static void SetNextZoneData( MAP_CHAGE_EVENT_WORK *p_wk);
static const u16* GetZoneIdTable( MAP_CHAGE_EVENT_WORK *p_wk );
static u16 GetFieldDir( u16 warp_dir );

// 部屋データ
static ROOM_DATA* GetNextRoomData( MAP_CHAGE_EVENT_WORK *p_wk );
static ROOM_DATA* GetNowRoomData( MAP_CHAGE_EVENT_WORK *p_wk );

// ダンジョン生成
static void SetDungeonData( INF_DUNGEON_DATA *p_dungeon_data ,int area_no, u32 seed ,int fs_x ,int fs_y);
static void SetBUildModel( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data);
static const GRID_DATA* GetBmodelPos( MAP_CHAGE_EVENT_WORK *p_wk ,int room_type ,int warp_dir  );
static void AddDummyObj( MAP_CHAGE_EVENT_WORK *p_wk , const GRID_DATA *p_grid_data);
static void SetDummyObj( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data);
static void SetRecoverTrainer( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no ,
    int *room_id_array ,int cnt,u32 *member_num , GFL_STD_RandContext *rand_context ,u32 *recover_index );
static BOOL SetTutorialMap( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data );
static void SetTutorialTrainerData( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data );

// グリッド
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z );

// マップワープイベント
static void SetNextZoneID( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetNextPos( MAP_CHAGE_EVENT_WORK *p_wk );
static const GRID_DATA* GetWarpPosTable( MAP_CHAGE_EVENT_WORK *p_wk ,int room_type ,int warp_dir  );

// フロア移動
static const GRID_DATA* GetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk );
static const GRID_DATA* GetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk ,int elevator_type );
static void SetNextFloor( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk );

// ダンジョン内トレーナー
static int GetRoomTrainerIDPos( INF_DUNGEON_DATA *p_dungeon_data ,u32 floor_id ,u32 room_id,u8 room_tr_no);
static void SetTrainerData( INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no , u32 seed );
static void SetTrainer( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data);
static u32 GetTrSeed( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u32 tr_id_pos );

// フロア生成 
static BOOL CheckCloseAllDoor(DIG_POS *p_dig_pos ,ROOM_DATA *p_room_data);
static int CheckCanMove(DIG_POS *p_dig_pos ,ROOM_DATA *p_room_data ,int *p_array,int fs_x ,int fs_y ,int mode);
static void MoveDigPos(u8 dir ,INF_DIG_DATA *p_dig_data ,ROOM_DATA *p_room_data);
static ROOM_DATA* GetDigPosRoomData( DIG_POS *p_dig_pos,ROOM_DATA* p_room_data);
static void push_dig_pos( INF_DIG_DATA *p_dig_data ,DIG_POS *p_dig_pos);
static void pop_dig_pos( INF_DIG_DATA *p_dig_data );
static void RndArray( u32 *p_array , int size ,GFL_STD_RandContext *rand_context );
static BOOL CheckDigEnd( ROOM_DATA *p_room_data ,int fs_x ,int fs_y);
static void RemoveTrRoom( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_no ,int rmv_num ,GFL_STD_RandContext *rand_context );
static BOOL IsMoveElevator( MAP_CHAGE_EVENT_WORK *p_wk );

// ヒントメッセージ
static u32 gethint_is_up( TRHINT_WK *p_wk );
static u32 gethint_is_down( TRHINT_WK *p_wk );
static u32 gethint_boss_floor( TRHINT_WK *p_wk );
static u32 gethint_boss_dir( TRHINT_WK *p_wk );
static u32 gethint_medic_dir( TRHINT_WK *p_wk );
static u32 gethint_boss_monster( TRHINT_WK *p_wk );
static u32 gethint_no_hint( TRHINT_WK *p_wk );
static u8  GetPosDirID( u8 x1, u8 y1, u8 x2 ,u8 y2 );
static u32 INFINITY_DUNGEON_GetBossRoom( INF_DUNGEON_DATA *p_dungeon_data );

#ifdef PM_DEBUG
static void INF_DEBUG_OutPutFloorData( ROOM_DATA *p_room_data ,INFINITYDUNGEON_SAVE *p_save );
static void INF_DEBUG_OverwritRoomData( ROOM_DATA *p_room_data );
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンデータを生成する
 *
 *	@param	*p_wk
 *	@param	*p_save 
 *	@param	heapID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_MakeDungeonData( INF_DUNGEON_DATA *p_wk,GAMEDATA *p_gdata, HEAPID heapID )
{
  u32 master_seed;
  INFINITYDUNGEON_SAVE  *p_inf_save;
  SAVE_CONTROL_WORK     *p_save;
  FIELD_WFBC_CORE_TYPE wfbc_type;
 
  // 無限ダンジョンのデータを取得
  p_save = GAMEDATA_GetSaveControlWork( p_gdata );
  p_inf_save  = SaveData_GetINFINITYDUNGEON( p_save );
  
  // WFBCの状態を調べる
  wfbc_type = GAMEDATA_GetMyWFBCType( p_gdata );

  // 初期化
  GFL_STD_MemFill(p_wk, 0, sizeof(INF_DUNGEON_DATA) );

  // WFBCの状態からバージョンデータを決定する
  if( wfbc_type == FIELD_WFBC_CORE_TYPE_BLACK_CITY )
  {
    p_wk->version = VERSION_B2;
  }
  else
  {
    p_wk->version = VERSION_W2;
  }

  // シードを生成
  master_seed = INFINITYDUNGEON_SAVE_GetRandomSeed( p_inf_save );
  SetDungeonSeed(&p_wk->seed ,master_seed);

  // フロアデータの生成
  GetFloorData( p_inf_save , p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンデータを再計算する
 *
 *	@param	*p_wk
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_ReCalcDungeonData( INF_DUNGEON_DATA *p_wk,INFINITYDUNGEON_SAVE *p_save )
{
  u32 master_seed;

  // シードを生成
  master_seed = INFINITYDUNGEON_SAVE_GetRandomSeed( p_save );
  SetDungeonSeed(&p_wk->seed ,master_seed);

  // フロアデータの生成
  GetFloorData( p_save , p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンのデータサイズを取得
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INF_GetDungeonDataSize()
{
  return sizeof(INF_DUNGEON_DATA);
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のマップ位置から次のマップを導き、ワープさせる
 *
 *	@param	*p_scr_work
 *	@param	*p_gsys
 *	@param	warp_dir 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_WarpNextMap( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys,INF_DUNGEON_DATA *p_dungeon_data, u16 warp_dir )
{
	GMEVENT *p_event = NULL;
  MAP_CHAGE_EVENT_WORK *p_wk;
 	
  p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
  SCRIPT_CallEvent( p_scr_work, p_event );
	p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_dungeon_data = p_dungeon_data;
  p_wk->warp_dir = warp_dir;
  p_wk->map_change_type = MAP_CHAGE_TYPE_ROOM;
}

//----------------------------------------------------------------------------
/**
 *	@brief  次のフロアにワープ
 *
 *	@param	*p_scr_work
 *	@param	*p_gsys
 *	@param	floor_no 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
extern void INFINITY_DUNGEON_WarpNextFloor( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys ,INF_DUNGEON_DATA *p_dungeon_data ,int floor_no )
{
	GMEVENT *p_event = NULL;
  MAP_CHAGE_EVENT_WORK  *p_wk;
  SAVE_CONTROL_WORK     *p_save_data;
  GAMEDATA              *p_gdata;
  INFINITYDUNGEON_SAVE  *p_save;

  // セーブデータ取得
  p_gdata       = GAMESYSTEM_GetGameData( p_gsys );
  p_save_data   = GAMEDATA_GetSaveControlWork( p_gdata );
  p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
  
  // フロアIDを保存（これを行わないと、エリア番号を取得できない）
  INFINITYDUNGEON_SAVE_SetFloorID( p_save , floor_no );

  // イベント呼び出し
  {
    u8 area_id = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );
    p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
    SCRIPT_CallEvent( p_scr_work, p_event );
    p_wk = GMEVENT_GetEventWork( p_event );
    p_wk->next_floor = GetDungeonFloorFromArea( floor_no ,area_id );
    p_wk->map_change_type = MAP_CHAGE_TYPE_FOOR;
    p_wk->p_dungeon_data = p_dungeon_data;
    p_wk->warp_dir = WARP_DIR_DOWN;
  }
  
  // データを生成するためにフロアを設定する
  {
    // 再計算
    INF_ReCalcDungeonData(p_dungeon_data,p_save); 
  }

  //OS_TFPrintf(1,"p_wk->next_floor:%u\n",p_wk->next_floor)
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  階数からエリア内の階数を取得する
 *
 *	@param	now_floor 
 *	@param	now_area 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetDungeonFloorFromArea( u8 now_floor ,u8 now_area )
{
  u8 start_floor = INFINITYDUNGEON_SAVE_GetAreaStartFloor( now_area );

  return now_floor - start_floor;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベントデータから必要データを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void InitWorkData( MAP_CHAGE_EVENT_WORK *p_wk ,GAMESYS_WORK *p_gsys)
{
  SAVE_CONTROL_WORK* p_save_data;
  p_wk->heapID        = HEAPID_FIELDMAP;
  p_wk->p_gsys        = p_gsys;
  p_wk->p_field_work  = GAMESYSTEM_GetFieldMapWork( p_wk->p_gsys );
  p_wk->p_gdata       = GAMESYSTEM_GetGameData( p_wk->p_gsys );
  p_save_data         = GAMEDATA_GetSaveControlWork( p_wk->p_gdata );
  p_wk->p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
  p_wk->p_zone_data   = GetZoneIdTable( p_wk );
  p_wk->now_room      = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );
  p_wk->now_floor     = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  p_wk->p_g3d_mapper  = FIELDMAP_GetFieldG3Dmapper( p_wk->p_field_work );
  p_wk->p_bmodel_man  = FLDMAPPER_GetBuildModelManager( p_wk->p_g3d_mapper );
  p_wk->p_mmdlsys     = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
  p_wk->now_area      = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_wk->p_save );

  //OS_TFPrintf(1,"p_wk->now_floor:%u\n",p_wk->now_floor);
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップチェンジイベント
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk )
{
  MAP_CHAGE_EVENT_WORK *p_wk = wk;
  ROOM_DATA *p_next_room;

  switch( *seq )
  {
  case 0:
    {
      // データ取得
      GAMESYS_WORK *p_gsys  = GMEVENT_GetGameSysWork( event );
      InitWorkData( p_wk ,p_gsys );

 
      // エレベーター移動の場合と、部屋移動でフェード方法を変える
      if( IsMoveElevator( p_wk ) )
      {
        // フェードアウト
        MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( p_wk->p_field_work ) );
        //GMEVENT_CallEvent( event, EVENT_FieldFadeOut_Black( p_wk->p_gsys, p_wk->p_field_work ,FIELD_FADE_WAIT ) );

        // 遅いフェードアウト
        GFL_FADE_SetMasterBrightReq(
            GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FIELD_FADE_SLOW_WAIT );

      }
      else
      {
        // フェードアウト
        MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( p_wk->p_field_work ) );
        GMEVENT_CallEvent( event, EVENT_FieldFadeOut_PlayerDir( p_wk->p_gsys, p_wk->p_field_work ) );
      }

      // 無限ダンジョン関連の岩データ位置をリセットする
      {
        MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
        MMDLSYS_ROCKPOS_ResetFallParam( mmdlsys ,ZONE_ID_D31R0801 ,D31R0801_ROOCK_01 );
        MMDLSYS_ROCKPOS_ResetFallParam( mmdlsys ,ZONE_ID_D31R0801 ,D31R0801_ROOCK_02 );
        MMDLSYS_ROCKPOS_ResetFallParam( mmdlsys ,ZONE_ID_D31R0801 ,D31R0801_ROOCK_03 );
        MMDLSYS_ROCKPOS_ResetFallParam( mmdlsys ,ZONE_ID_D31R0801 ,D31R0801_ROOCK_04 );
      }
      

      (*seq)++;
      break;
    }
  case 1:
    {
      // フェードチェック
      if( !GFL_FADE_CheckFade() )
      {

        // マップ移動
        NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
        SetNextZoneData( p_wk );
        GMEVENT_CallEvent(event,
            EVENT_ChangeMapPosNoFade(p_wk->p_gsys, p_wk->p_field_work, p_next_map_data->zone_id, &p_next_map_data->warp_pos,GetFieldDir( p_wk->warp_dir ) ));
        (*seq)++;
      }
      break;
    }
  case 2:
    {
      // フェードイン
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
      if ( MMDL_BLACTCONT_IsThereReserve( mmdlsys ) == TRUE )
      {
        break;
      }
 
      // ギミックワークのポインタを再取得
      p_wk->p_dungeon_data  = INFINITY_DUNGEON_DataGet( p_wk->p_field_work );

#ifdef PM_DEBUG      
      {
        u8 now_floor;
        now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
        INF_DEBUG_OutPutFloorData(p_wk->p_dungeon_data->room_data[now_floor] ,p_wk->p_save );
      }
#endif

      // 受付の場合は配置モデルを置かない
      if( p_wk->warp_dir != WARP_INFORMATION )
      {
        // 次の部屋データを取得する
        p_next_room = GetNextRoomData( p_wk );

        if( (p_next_room->room_type != ROOM_TYPE_BOSS) )
        {
          // 配置モデルを配置
          SetBUildModel(p_wk,p_next_room);

          // 封鎖用OBJ配置
          SetDummyObj(p_wk,p_next_room);

          // トレーナー配置
          SetTrainer(p_wk,p_next_room);
        }
      }
 
      // エレベーター移動の場合と、部屋移動でフェード方法を変える
      if( IsMoveElevator( p_wk ) )
      {
        GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black( p_wk->p_gsys, p_wk->p_field_work ,FIELD_FADE_WAIT ) );
      }
      else
      {
        GMEVENT_CallEvent( event, EVENT_FieldFadeIn_PlayerDir( p_wk->p_gsys, p_wk->p_field_work ) );
      }

      (*seq)++;
      break;
    }
  case 3:
    {
      // 動作モデル移動開始
      MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(p_wk->p_field_work));
      
      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン用シードを生成する
 *
 *	@param	*p_seed 
 *	@param	master_seed 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDungeonSeed( INF_DUNGEON_SEED *p_seed ,u32 master_seed )
{
  int i,n;
  GFL_STD_RandContext rand_context;
 
  // シード生成用シード
#ifdef PM_DEBUG
  if( g_debug_inf_master_seed == 0 )
  {
    p_seed->master_seed = master_seed;
  }
  else
  {
    p_seed->master_seed = g_debug_inf_master_seed;
  }
#else
  p_seed->master_seed = master_seed;
#endif

  // 乱数を作成
  GFL_STD_RandInit( &rand_context, p_seed->master_seed );

  // 子シードを生成
  p_seed->tr_data_seed = GFL_STD_Rand( &rand_context, GFL_STD_RAND_MAX);
  p_seed->dungeon_data_seed = GFL_STD_Rand( &rand_context, GFL_STD_RAND_MAX);

  // トレーナーシード
  for( i = 0; i < INFINITYDUNGEON_FLOOR_MAX ;i++ )
  {
    for( n = 0; n < INFINITY_DUNGEON_TRID_MAX ;n++ )
    {
      p_seed->tr_seed[i][n] = GFL_STD_Rand( &rand_context, GFL_STD_RAND_MAX);
    }
  }

  //OS_TFPrintf(1,"シード番号:%u\n",p_seed->master_seed);
}

//----------------------------------------------------------------------------
/**
 *	@brief  次のゾーンデータを設定
 *
 *	@param	*p_wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextZoneData( MAP_CHAGE_EVENT_WORK *p_wk)
{
  if( p_wk->map_change_type == MAP_CHAGE_TYPE_ROOM )
  {
    if( p_wk->warp_dir != WARP_INFORMATION )
    {
      // 次のゾーンID設定
      SetNextZoneID( p_wk );

      // 次の場所設定
      SetNextPos( p_wk );
    }
    else
    {
      // 受付データを設定する
      SetReceptionWarpData( p_wk );

      // 受け付けエレベータの場所をセット
      SetReceptionElevatorPos( p_wk );
    }
  }
  else if( p_wk->map_change_type == MAP_CHAGE_TYPE_FOOR )
  {
    // 次のフロア設定
    SetNextFloor( p_wk );

    // 次の場所設定
    SetElevatorPos( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  次ルームデータ取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static ROOM_DATA* GetNextRoomData( MAP_CHAGE_EVENT_WORK *p_wk )
{
  return &p_wk->p_dungeon_data->room_data[ p_wk->next_floor ][p_wk->next_room];
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動前ルームデータ取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static ROOM_DATA* GetNowRoomData( MAP_CHAGE_EVENT_WORK *p_wk )
{
  return &p_wk->p_dungeon_data->room_data[ p_wk->now_floor ][p_wk->now_room];
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワープ後のゾーンIDを取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextZoneID( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data  = &p_wk->next_map_data;
  ROOM_DATA     *p_next_room_data;
  u16 warp_dir    = p_wk->warp_dir;
  u32 room_type;
  p_wk->next_room   = p_wk->now_room;
  p_wk->next_floor  = p_wk->now_floor;  // フロアは移動しない

  switch( warp_dir )
  {
  case WARP_DIR_UP:
    p_wk->next_room -= ROOM_MAX_X;
    break;
  case WARP_DIR_DOWN:
    p_wk->next_room += ROOM_MAX_X;
    break;
  case WARP_DIR_LEFT:
    p_wk->next_room --;
    break;
  case WARP_DIR_RIGHT:
    p_wk->next_room ++;
    break;
  defautl:
    GF_ASSERT(0);
    break;
  }

  // 範囲外の場合はとりあえず今の部屋に飛ぶ
  if( p_wk->next_room < 0 )
  {
    p_wk->next_room = p_wk->now_room;
  }
  else if( p_wk->next_room >= ROOM_MAX )
  {
    p_wk->next_room = p_wk->now_room;
  }

  // 次の部屋データを取得する
  p_next_room_data = GetNextRoomData( p_wk );

  // ループタイプに合わせたゾーンを設定する
  room_type = p_next_room_data->room_type;
  p_next_map_data->zone_id = p_wk->p_zone_data[ room_type ];
  INFINITYDUNGEON_SAVE_SetRoomID( p_wk->p_save , p_wk->next_room );
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定フロアのエレベーター部屋に移動
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextFloor( MAP_CHAGE_EVENT_WORK *p_wk )
{
  int i;
  NEXT_MAP_DATA *p_next_map_data  = &p_wk->next_map_data;
  ROOM_DATA     *p_next_room_data;

  // 現在の部屋位置を初期化
  p_wk->next_room = 0;

  // エレベータ部屋を探す
  for( i = 0 ; i < ROOM_MAX ; i++ )
  {
    // 部屋タイプの取得
    ROOM_DATA  *p_room_data;
    p_room_data = GetNextRoomData( p_wk );

    // エレベータ部屋が見つかるまでつづける
    if( p_room_data->room_type == ROOM_TYPE_ELEVATOR )
    {
      break;
    }

    // 次の部屋へ
    p_wk->next_room++;
  }

  // 次の部屋データを取得する
  p_next_room_data = GetNextRoomData( p_wk );

  // ループタイプに合わせたゾーンを設定する
  p_next_map_data->zone_id = p_wk->p_zone_data[ p_next_room_data->room_type ];
  INFINITYDUNGEON_SAVE_SetRoomID( p_wk->p_save , p_wk->next_room );

  // 次のフロアを設定する
  p_wk->now_floor = p_wk->next_floor;
  
  // エレベータ部屋が無かったら警告
  GF_ASSERT( p_next_room_data->room_type == ROOM_TYPE_ELEVATOR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動方向に対応した場所を設定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
  const GRID_DATA   *p_pos;
  ROOM_DATA         *p_room_data;
  u32               room_type;
  
  // 部屋タイプの取得
  p_room_data = GetNextRoomData( p_wk );
  room_type = p_room_data->room_type;

  p_pos = GetWarpPosTable( p_wk ,room_type ,p_wk->warp_dir  );

  SetGridPos( &p_next_map_data->warp_pos ,p_pos->x , p_pos->y , p_pos->z);
}

//----------------------------------------------------------------------------
/**
 *	@brief  受付に移動するためのデータを設定する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetReceptionWarpData( MAP_CHAGE_EVENT_WORK *p_wk )
{
  // ループタイプに合わせたゾーンを設定する
  NEXT_MAP_DATA *p_next_map_data  = &p_wk->next_map_data;
  p_next_map_data->zone_id = p_wk->p_zone_data[ ROOM_TYPE_RECEPTION ];
  SetElevatorPos( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンごとのエレベーターの位置を設定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;

  const GRID_DATA *p_pos = GetElevatorPos( p_wk );

  SetGridPos( &p_next_map_data->warp_pos ,p_pos->x , p_pos->y , p_pos->z);
}

//----------------------------------------------------------------------------
/**
 *	@brief  受付用エレベータの場所をセット
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;

  const GRID_DATA *p_pos;

  if( p_wk->now_area <= INF_LEFT_ELEVATOR_AREA )
  {
    p_pos = GetReceptionElevatorPos( p_wk ,RECEPTION_ELEVATOR_L );
  }
  else
  {
    p_pos = GetReceptionElevatorPos( p_wk ,RECEPTION_ELEVATOR_R );
  }

  SetGridPos( &p_next_map_data->warp_pos ,p_pos->x , p_pos->y , p_pos->z);
}

//----------------------------------------------------------------------------
/**
 *	@brief  グリッド座標をFx32に変換してセットする
 *
 *	@param	*vec 
 *	@param	x     Xグリッド座標
 *	@param	y     Yグリッド座標
 *	@param	z     Xグリッド座標
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z )
{
  vec->x = (x * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->y = (y * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->z = (z * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のダンジョンバージョンを返す
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITY_DUNGEON_GetDungeonColor( INF_DUNGEON_DATA *p_wk )
{
  return p_wk->version;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のバージョンに対応したゾーンIDテーブルを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const u16* GetZoneIdTable( MAP_CHAGE_EVENT_WORK *p_wk )
{
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data );

  if( version == VERSION_B2 )
  {
    return sc_black_zone_table;
  }
  else
  {
    return sc_white_zone_table;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  バージョンごとのエレベータ位置を取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const GRID_DATA* GetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data );

  if( version == VERSION_B2 )
  {
    return &sc_black_elevator_pos;
  }
  else
  {
    return &sc_white_elevator_pos;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  受付のエレベータの場所を取得
 *
 *	@param	*p_wk 
 *	@param	elevator_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const GRID_DATA* GetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk ,int elevator_type )
{
  return &sc_reception_elevator_pos[elevator_type];
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のバージョンに対応したワープ後場所テーブルを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const GRID_DATA* GetWarpPosTable( MAP_CHAGE_EVENT_WORK *p_wk ,int room_type ,int warp_dir  )
{
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data );

  if( version == VERSION_B2 )
  {
    return &sc_black_warp_pos_table[room_type][warp_dir];
  }
  else
  {
    return &sc_white_warp_pos_table[room_type][warp_dir];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナー配置パターンから配置人数を取得する
 *
 *	@param	pos_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetMemberNum( u8 pos_type )
{
  if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
  {
    return sc_room_tr_param[pos_type].member_num;
  }
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナー配置パターンから部屋タイプを取得
 *
 *	@param	pos_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetRoomType( u8 pos_type )
{
  if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
  {
    return sc_room_tr_param[pos_type].room_type;
  }
  return ROOM_TYPE_DEFAULT;
}

//----------------------------------------------------------------------------
/**
 *	@brief  部屋ごとのバージョン条件を取得
 *
 *	@param	pos_type 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetRoomVersion( u8 pos_type )
{
  if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
  {
    return sc_room_tr_param[pos_type].version;
  }
  return INF_MULTI_DUNGEON;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のバージョンに対応した配置モデル配置位置取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const GRID_DATA* GetBmodelPos( MAP_CHAGE_EVENT_WORK *p_wk ,int room_type ,int warp_dir  )
{
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data );

  if( version == VERSION_B2 )
  {
    return &sc_black_bmodel_pos_table[room_type][warp_dir];
  }
  else
  {
    return &sc_white_bmodel_pos_table[room_type][warp_dir];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定位置の部屋データを取得する
 *
 *	@param	*p_dig_pos
 *	@param	p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static ROOM_DATA* GetDigPosRoomData( DIG_POS *p_dig_pos,ROOM_DATA* p_room_data)
{
  return (&p_room_data[p_dig_pos->x+p_dig_pos->y*ROOM_MAX_Y]);
}

//----------------------------------------------------------------------------
/**
 *	@brief  穴を掘る
 *
 *	@param	dir 
 *	@param	*p_dig_data 
 *	@param	*p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MoveDigPos(u8 dir ,INF_DIG_DATA *p_dig_data ,ROOM_DATA *p_room_data)
{
  ROOM_DATA *p_now_room_data;
  ROOM_DATA *p_next_room_data;

  // 指定の方向に進む
  switch( dir )
  {
  case INF_MOVE_UP:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_UP] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.y -= 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_DOWN] = TRUE;
    break;
  case INF_MOVE_DOWN:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_DOWN] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.y += 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_UP] = TRUE;
    break;
  case INF_MOVE_LEFT:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_LEFT] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.x -= 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_RIGHT] = TRUE;
    break;
  case INF_MOVE_RIGHT:
    p_now_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_now_room_data->can_move[INF_MOVE_RIGHT] = TRUE;
    push_dig_pos(p_dig_data,&p_dig_data->now_pos); // データを保存
    p_dig_data->now_pos.x += 1;
    p_next_room_data = GetDigPosRoomData( &p_dig_data->now_pos ,p_room_data);
    p_next_room_data->can_move[INF_MOVE_LEFT] = TRUE;
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  位置データをプッシュ
 *
 *	@param	*p_dig_data 
 *	@param	*p_dig_pos
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void push_dig_pos( INF_DIG_DATA *p_dig_data ,DIG_POS *p_dig_pos)
{
  p_dig_data->data_index++;
  p_dig_data->dig_pos_list[p_dig_data->data_index] = *p_dig_pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief  位置データをプッシュ
 *
 *	@param	*p_dig_data 
 *	@param	*p_dig_pos
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pop_dig_pos( INF_DIG_DATA *p_dig_data )
{
  if( p_dig_data->data_index >= 0 )
  {
    p_dig_data->now_pos = p_dig_data->dig_pos_list[p_dig_data->data_index];
    p_dig_data->dig_pos_list[p_dig_data->data_index].x = 0;
    p_dig_data->dig_pos_list[p_dig_data->data_index].y = 0;
    p_dig_data->data_index--;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  部屋の扉が全て閉じているかを調べる
 *
 *	@param	*p_dig_pos 
 *	@param	*p_room_data
 *
 *	@retval TRUE=全て閉じている
 */
//-----------------------------------------------------------------------------
static BOOL CheckCloseAllDoor(DIG_POS *p_dig_pos ,ROOM_DATA *p_room_data)
{
  int i;
  ROOM_DATA *p_rd = GetDigPosRoomData( p_dig_pos ,p_room_data);
  for( i = 0; i < 4 ;i++ )
  {
    if( p_rd->can_move[i] == TRUE )
    {
      return FALSE;
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動可能な方向があるか調べる
 *
 *	@param	*p_dig_pos 
 *	@param	*p_room_data
 *	@param	*p_array      移動可能先格納変数
 *	@param	mode          
 *
 *	@retval 移動可能な方向数
 */
//-----------------------------------------------------------------------------
static int CheckCanMove(DIG_POS *p_dig_pos ,ROOM_DATA *p_room_data ,int *p_array,int fs_x ,int fs_y ,int mode)
{
  int i;
  int ret_cnt = 0;
  ROOM_DATA *p_rd;

  // 移動方向のクリア
  GFL_STD_MemFill(p_array, INF_MOVE_MAX, sizeof(int)*INF_MOVE_MAX);

  // 4方向の部屋で全てドアが閉まっている部屋を調べる
  {
    DIG_POS pos = *p_dig_pos;
    pos.x+=1;
    if( pos.x < fs_x )
    {
      // 移動方向の部屋がボス部屋の場合は掘り進まない
      p_rd= GetDigPosRoomData( &pos ,p_room_data);
      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if( mode == INF_CHECKMODE_FULL_CLOSE )
        {
          // 全てのドアが閉まっているかチェックする
          if( CheckCloseAllDoor( &pos ,p_room_data ) )
          {
            p_array[ret_cnt] = INF_MOVE_RIGHT;
            ret_cnt++;
          }
        }
        else
        {
          p_array[ret_cnt] = INF_MOVE_RIGHT;
          ret_cnt++;
        }
      }
    }
  }
  {
    DIG_POS pos = *p_dig_pos;
    pos.x-=1;
    if( pos.x >= 0 )
    {
      p_rd= GetDigPosRoomData( &pos ,p_room_data);
      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if( mode == INF_CHECKMODE_FULL_CLOSE )
        {
          if( CheckCloseAllDoor( &pos ,p_room_data ) )
          {
            p_array[ret_cnt] = INF_MOVE_LEFT;
            ret_cnt++;
          }
        }
        else
        {
            p_array[ret_cnt] = INF_MOVE_LEFT;
            ret_cnt++;
        }
      }
    }
  }
  {
    DIG_POS pos = *p_dig_pos;
    pos.y+=1;
    if( pos.y < fs_y )
    {
      p_rd= GetDigPosRoomData( &pos ,p_room_data);

      // 移動方向がボス部屋、エレベータ部屋の場合はしたには掘り進まない
      if( (p_rd->room_type != ROOM_TYPE_BOSS) && (p_rd->room_type != ROOM_TYPE_ELEVATOR) )
      {
        if( mode == INF_CHECKMODE_FULL_CLOSE )
        {
          if( CheckCloseAllDoor( &pos ,p_room_data ) )
          {
            p_array[ret_cnt] = INF_MOVE_DOWN;
            ret_cnt++;
          }
        }
        else
        {
          p_array[ret_cnt] = INF_MOVE_DOWN;
          ret_cnt++;
        }
      }
    }
  }
  {
    DIG_POS pos = *p_dig_pos;
    p_rd= GetDigPosRoomData( &pos ,p_room_data);
    if( p_rd->room_type != ROOM_TYPE_ELEVATOR )   // エレベータ部屋は上に移動することを禁止
    {
      pos.y-=1;
      p_rd= GetDigPosRoomData( &pos ,p_room_data);
      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if( pos.y >= 0 )
        {
          if( mode == INF_CHECKMODE_FULL_CLOSE )
          {

            if( CheckCloseAllDoor( &pos ,p_room_data ) )
            {
              p_array[ret_cnt] = INF_MOVE_UP;
              ret_cnt++;
            }
          }
          else
          {
            p_array[ret_cnt] = INF_MOVE_UP;
            ret_cnt++;
          }
        }
      }
    }
  }

  return ret_cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  配列内のデータをランダムにならびかえる
 *
 *	@param	*p_array 
 *	@param	size 
 *	@param	context 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RndArray( u32 *p_array , int size ,GFL_STD_RandContext *rand_context )
{
  int i;
  u32 tmp;
  for( i = 0; i < size ;i++ )
  {
    u32 rnd = GFL_STD_Rand( rand_context,size);
    tmp = p_array[rnd];
    p_array[rnd] = p_array[i];
    p_array[i] = tmp;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベータの設置可能配列を取得する
 *
 *	@param	*p_room_data 
 *	@param	*p_index_array 
 *	@param	fs_x 
 *	@param	fs_y
 *
 *	@retval 配列に格納した数
 */
//-----------------------------------------------------------------------------
static int GetElevatorSetArray( ROOM_DATA *p_room_data ,int *p_index_array ,int fs_x ,int fs_y)
{
  DIG_POS pos;
  int cnt = 0;
  int x,y;
  ROOM_DATA *p_rd;
  GFL_STD_MemFill(p_index_array, ROOM_MAX, sizeof(int)*ROOM_MAX);

  // ボス部屋を調べ、配置出る場所リストを作成する
  for( y = 0; y<fs_y; y++ )
  {
    for( x = 0; x<fs_x; x++ )
    {
      // 現在地がボス部屋
      {
        pos.x = x;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }

      // 上
      if( y - 1 >= 0 )
      {
        pos.x = x;
        pos.y = y-1;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }

      // 下
      if( y + 1  < fs_y )
      {
        pos.x = x;
        pos.y = y+1;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }

      // 左
      if( x - 1 >= 0 )
      {
        pos.x = x-1;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }
      else
      {
        // 左端で2列目のときは、右上にボス部屋があると生成不能になるのでチェック
        if( y == 1 ){
          pos.x = x+1;
          pos.y = y-1;
          p_rd= GetDigPosRoomData( &pos ,p_room_data);
          if( p_rd->room_type == ROOM_TYPE_BOSS )
          {
            continue;
          }
        }
      }

      // 右
      if( x + 1 < fs_x  )
      {
        pos.x = x+1;
        pos.y = y;
        p_rd= GetDigPosRoomData( &pos ,p_room_data);
        if( p_rd->room_type == ROOM_TYPE_BOSS )
        {
          continue;
        }
      }
      else
      {
        // 左端で2列目のときは、右上にボス部屋があると生成不能になるのでチェック
        if( y == 1 ){
          pos.x = x-1;
          pos.y = y-1;
          p_rd= GetDigPosRoomData( &pos ,p_room_data);
          if( p_rd->room_type == ROOM_TYPE_BOSS )
          {
            continue;
          }
        }
      }

      // 設置可能位置を保存する
      {
        p_index_array[cnt] = x + y*ROOM_MAX_Y;
        cnt++;
      }
    }
  }
  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  全て掘り終わったかチェック
 *
 *	@param	*p_room_data 
 *	@param	fs_x 
 *	@param	fs_y
 *
 *	@retval TRUE=掘り終わり
 */
//-----------------------------------------------------------------------------
static BOOL CheckDigEnd( ROOM_DATA *p_room_data ,int fs_x ,int fs_y)
{
  int x,y;
  ROOM_DATA *p_rd;
  DIG_POS pos;
  for( y=0; y < fs_y; y++ )
  {
    for( x=0; x < fs_x; x++ )
    {
      pos.x = x;
      pos.y = y;
      p_rd= GetDigPosRoomData( &pos ,p_room_data);

      if( p_rd->room_type != ROOM_TYPE_BOSS )
      {
        if(CheckCloseAllDoor( &pos ,p_room_data ) == TRUE)
        {
          return FALSE;
        }
      }
    }
  }
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンデータをセットする
 *
 *	@param	*p_dungeon_data 
 *	@param	seed 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDungeonData( INF_DUNGEON_DATA *p_dungeon_data ,int area_no, u32 seed ,int fs_x ,int fs_y)
{
  int i,j,k;
  DIG_POS elv_pos[INFINITYDUNGEON_FLOOR_MAX];
  DIG_POS boss_pos;
  s32 boss_floor;
  GFL_STD_RandContext rand_context;
  const INF_ROOM_SETUP_PARAM *sc_room_setup = &sc_room_setup_param[area_no];
 
  // 乱数を作成
  GFL_STD_RandInit( &rand_context, seed );
    
//  ONOUE_Printf("---------------------\n");
//  ONOUE_Printf("seed = %d\n",seed);
//  ONOUE_Printf("area_no = %d\n",area_no);
//  ONOUE_Printf("fs_x = %d\n",fs_x);
//  ONOUE_Printf("fs_y = %d\n",fs_y);
 
  // 部屋タイプを初期化
  {
    for( i = 0; i<INFINITYDUNGEON_FLOOR_MAX; i++ )
    {
      u8  no_tr_room_data[ROOM_MAX];
      int no_tr_room_cnt = 0;
      GFL_STD_MemFill(no_tr_room_data ,ROOM_MAX ,sizeof(u8)*ROOM_MAX );

      // 部屋抽選
      for( j = 0; j<ROOM_MAX; j++ )
      {
        // 初期化
        p_dungeon_data->room_data[i][j].room_id = j;
        p_dungeon_data->room_data[i][j].room_type = ROOM_TYPE_DEFAULT;
        p_dungeon_data->room_data[i][j].tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;

        // 扉を閉じる
        for( k = 0 ; k < 4 ; k++ )
        {
          p_dungeon_data->room_data[i][j].can_move[k]  = FALSE;
        }

        // 部屋タイプを抽選する
        {
          int x = j % ROOM_MAX_X;
          int y = j / ROOM_MAX_Y;

          if( (x < fs_x) && (y < fs_y ) )
          {
            BOOL is_set_trainer  = FALSE;
            for( k = 0 ; k < sc_room_setup->member_num; k++ )
            {
              const INF_ROOM_DATA *p_rs_data = &sc_room_setup->room_setup_data[k];
              const u32 dungeon_color = INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data );
              const u8 room_version = GetRoomVersion(p_rs_data->tr_pos_type);
              
              // バージョンでの制限
              if( (dungeon_color == VERSION_B2) && (room_version == INF_B2_ONLY_DUNGEON) || // ブラック専用
                  (dungeon_color == VERSION_W2) && (room_version == INF_W2_ONLY_DUNGEON) || // ホワイト専用
                  (room_version == INF_MULTI_DUNGEON) )
              {
                // 確率での抽選
                if( GFL_STD_Rand( &rand_context ,100 ) < p_rs_data->probability)
                {
                  p_dungeon_data->room_data[i][j].tr_pos_type = p_rs_data->tr_pos_type;
                  p_dungeon_data->room_data[i][j].room_type = GetRoomType(p_rs_data->tr_pos_type);
                  is_set_trainer = TRUE;
                  break;
                }
              }
            }
            if( is_set_trainer == FALSE )
            {
              // 未配置部屋は配置人数が０のとき用に保存しておく
              no_tr_room_data[no_tr_room_cnt] = j;
              no_tr_room_cnt++;
            }
          }
        }
      }

      // 最低5部屋は置く（BOSS+エレベータ+回復×2+通常部屋=5）
      {
        int no_tr_room_index  = 0;
        int tr_room_cnt   = (fs_x*fs_y) - no_tr_room_cnt; // 部屋数 - トレーナーが配置されていない部屋 = トレーナー配置済みの部屋数
        for( j = tr_room_cnt; j < INFINITY_DUNGEON_TR_ROOM_MIN ;j++ )
        {
          u8 room_id = no_tr_room_data[no_tr_room_index];
          p_dungeon_data->room_data[i][room_id].tr_pos_type = INF_ROOM_POS_TYPE_EX_1;
          p_dungeon_data->room_data[i][room_id].room_type = GetRoomType(INF_ROOM_POS_TYPE_EX_1);
          no_tr_room_index++;
          GF_ASSERT( no_tr_room_index <= no_tr_room_cnt  ); // 未配置部屋数以上は配置できない
        }
        OS_Printf("%d部屋追加しました 。現在 %d部屋\n",no_tr_room_index,tr_room_cnt+no_tr_room_index);
      }
    }
  }

  // 配置人数が32人以上の場合、ランダムで適当な人数まで減らす
  {
    int floor_no;
    for( floor_no = 0; floor_no < INFINITYDUNGEON_FLOOR_MAX; floor_no++ )
    {
      int member_num = 0;
      for( i = 0; i< ROOM_MAX; i++ )
      {
        u8 pos_type = p_dungeon_data->room_data[floor_no][i].tr_pos_type;
        if( GetMemberNum(pos_type) > 0 )
        {
          member_num += GetMemberNum(pos_type);
        }
      }

      if( member_num > INFINITY_DUNGEON_TRID_MAX )
      {
        int rmv_num = member_num -= INFINITY_DUNGEON_TRID_MAX;
        RemoveTrRoom( p_dungeon_data , floor_no , rmv_num , &rand_context);
      }

#ifdef  PM_DEBUG
      if( member_num == 0 )
      {
        GF_ASSERT_MSG(0,"配置人数が０人です\n");
      }
#endif
      
    }
  }

  // ボス部屋の設置
  {
    ROOM_DATA *p_rd;
    ROOM_DATA *p_room_data;
    int floor_no;
    u8 top_floor = INFINITYDUNGEON_SAVE_GetFloorNum( area_no );
    floor_no = GFL_STD_Rand( &rand_context ,top_floor);
    p_room_data = p_dungeon_data->room_data[floor_no];
    boss_pos.x = GFL_STD_Rand( &rand_context ,fs_x);
    boss_pos.y = GFL_STD_Rand( &rand_context ,fs_y-1); // 一番下の部屋には置けない
    p_rd= GetDigPosRoomData( &boss_pos ,p_room_data);
    p_rd->room_type = ROOM_TYPE_BOSS;
    p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    boss_floor = floor_no;
    //OS_TFPrintf(1,"ボス部屋:%d階 room_id = %u \n",boss_floor+1,p_rd->room_id);
  }

  // エレベータ設置配列を取得
  {
    for( i = 0; i<INFINITYDUNGEON_FLOOR_MAX; i++ )
    {
      ROOM_DATA *p_room_data = p_dungeon_data->room_data[i];
      int index_array[ROOM_MAX];
      u32 rnd;
      int room_index;
      int max = GetElevatorSetArray(p_room_data,index_array,fs_x,fs_y);
      
      GF_ASSERT_MSG((max > 0),"エレベータを置く場所がありません\n");
      room_index = GFL_STD_Rand( &rand_context ,max);
      p_room_data[index_array[room_index]].room_type    = ROOM_TYPE_ELEVATOR;
      p_room_data[index_array[room_index]].tr_pos_type  = INF_TR_ROOM_POS_TYPE_NONE;

      // エレベーター位置を格納（穴掘りスタート位置に使用）
      elv_pos[i].x = index_array[room_index]%ROOM_MAX_X;
      elv_pos[i].y = index_array[room_index]/ROOM_MAX_Y;
    
      //OS_TFPrintf(2,"エレベータ:no=%d\n",index_array[room_index]);
    }
  }

  //ランダム
  {
    int floor_no;
    for( floor_no = 0; floor_no < INFINITYDUNGEON_FLOOR_MAX; floor_no++ )
    {
      ROOM_DATA *p_room_data = p_dungeon_data->room_data[floor_no];
      INF_DIG_DATA dig_data;
      BOOL flg;
      u8   dir_index;
      u8   dir;
      int dir_array[INF_MOVE_MAX];
      dig_data.now_pos = elv_pos[floor_no];
      dig_data.data_index = -1;

      //OS_TFPrintf(2,"スタート地点:x=%d,y=%d\n",dig_data.now_pos.x,dig_data.now_pos.y);


      for( i = 0; i< INF_MAX_SEARC_CNT; i++ )
      {
        int move_cnt;
        move_cnt = CheckCanMove( &dig_data.now_pos ,p_room_data,dir_array
            ,fs_x,fs_y,INF_CHECKMODE_FULL_CLOSE);
        //OS_TFPrintf(2,"dig_pos:x=%d,y=%d\n",dig_data.now_pos.x,dig_data.now_pos.y);
        if( move_cnt > 0 )
        {
          dir_index = GFL_STD_Rand( &rand_context,move_cnt);
          dir = dir_array[dir_index];
          MoveDigPos(dir ,&dig_data ,p_room_data );
        }
        else
        {
          // 一歩戻る
          //OS_TFPrintf(2,"pop\n");
          pop_dig_pos( &dig_data );

          // 終了
          if( CheckDigEnd( p_room_data ,fs_x ,fs_y ) == TRUE )
          {
            //OS_TFPrintf(2,"終了\n");
            break;
          }
        }
      }

#ifdef PM_DEBUG
      if( i == INF_MAX_SEARC_CNT )
      {
        GF_ASSERT_MSG(0,"迷路が最後まで生成できませんでした");
      }
#endif

      // ボス部屋への道を空ける
      if( boss_floor == floor_no )
      {
        dig_data.now_pos= boss_pos;
        MoveDigPos(INF_MOVE_DOWN ,&dig_data ,p_room_data );
      }

      // ランダムオープン
      {
        DIG_POS pos;
        int dir_array[INF_MOVE_MAX];
        int move_cnt;
        u8   dir_index;
        INF_DIG_DATA dig_data;
        ROOM_DATA *p_rd;
        dig_data.data_index = -1;

        for( pos.y = 0; pos.y < fs_y ; pos.y++ )
        {
          for( pos.x = 0; pos.x < fs_x ; pos.x++ )
          {
            dig_data.now_pos = pos;
            p_rd= GetDigPosRoomData( &pos ,p_room_data);

            // エレベータ部屋とボス部屋からは新たな道を作らない
            if( (p_rd->room_type != ROOM_TYPE_ELEVATOR) && (p_rd->room_type != ROOM_TYPE_BOSS) )
            {
              move_cnt = CheckCanMove( &pos ,p_room_data,dir_array ,fs_x,fs_y,INF_CHECKMODE_MOVE);
              dir_index = GFL_STD_Rand( &rand_context,move_cnt);
              dir = dir_array[dir_index];
              MoveDigPos(dir ,&dig_data ,p_room_data );
            }
          }
        }
      }

#ifdef PM_DEBUG
      if( g_debug_inf_data_change == TRUE )
      {
        // 部屋タイプを上書き
        INF_DEBUG_OverwritRoomData(p_dungeon_data->room_data[floor_no]);
      }
#endif
    }
  }

#ifdef PM_DEBUG
  // プレゼン用マップ差し替え
  if( g_debug_inf_is_presentation == TRUE )
  {
    ROOM_DATA *p_rd;

    // 部屋タイプを初期化
    {
      const INF_ROOM_SETUP_PARAM *sc_room_setup = &sc_room_setup_param[area_no];
      for( i = 0; i<INFINITYDUNGEON_FLOOR_MAX; i++ )
      {
        for( j = 0; j<ROOM_MAX; j++ )
        {
          // 初期化
          p_dungeon_data->room_data[i][j].room_id = j;
          p_dungeon_data->room_data[i][j].room_type = ROOM_TYPE_DEFAULT;
          p_dungeon_data->room_data[i][j].tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;

          // 扉を閉じる
          for( k = 0 ; k < 4 ; k++ )
          {
            p_dungeon_data->room_data[i][j].can_move[k]  = FALSE;
          }
        }
      }
    }

    // データ差し替え(１階)
    {
      // エレベータ部屋
      p_rd = &p_dungeon_data->room_data[0][1];
      p_rd->room_type = ROOM_TYPE_ELEVATOR;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = TRUE;
      p_rd->can_move[INF_MOVE_LEFT] = FALSE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;
    }
    {
      u8  tr_pos_type = INF_ROOM_POS_TYPE_K_1;
      p_rd = &p_dungeon_data->room_data[0][2];
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = TRUE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;

      p_rd->tr_pos_type = tr_pos_type;
      p_rd->room_type = GetRoomType(tr_pos_type);
    }
    {
      p_rd = &p_dungeon_data->room_data[0][3];
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= FALSE;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    }
    {
      u8  tr_pos_type = INF_ROOM_POS_TYPE_E_1;
      p_rd = &p_dungeon_data->room_data[0][6];
      p_rd->can_move[INF_MOVE_UP]   = TRUE;
      p_rd->can_move[INF_MOVE_DOWN] = TRUE;
      p_rd->can_move[INF_MOVE_LEFT] = FALSE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;

      p_rd->tr_pos_type = tr_pos_type;
      p_rd->room_type = GetRoomType(tr_pos_type);
    }
    {
      p_rd = &p_dungeon_data->room_data[0][7];
      p_rd->can_move[INF_MOVE_UP]   = TRUE;
      p_rd->can_move[INF_MOVE_DOWN] = TRUE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= FALSE;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    }
    {
      u8  tr_pos_type = INF_ROOM_POS_TYPE_J_1;
      p_rd = &p_dungeon_data->room_data[0][10];
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = FALSE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;

      p_rd->tr_pos_type = tr_pos_type;
      p_rd->room_type = GetRoomType(tr_pos_type);
    }
    {
      u8  tr_pos_type = INF_ROOM_POS_TYPE_G_1;
      p_rd = &p_dungeon_data->room_data[0][11];
      p_rd->can_move[INF_MOVE_UP]   = TRUE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;

      p_rd->tr_pos_type = tr_pos_type;
      p_rd->room_type = GetRoomType(tr_pos_type);
    }
    {
      u8  tr_pos_type = INF_ROOM_POS_TYPE_F_1;
      p_rd = &p_dungeon_data->room_data[0][12];
      p_rd->can_move[INF_MOVE_UP]   = TRUE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;

      p_rd->tr_pos_type = tr_pos_type;
      p_rd->room_type = GetRoomType(tr_pos_type);
    }
    {
      p_rd = &p_dungeon_data->room_data[0][13];
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= FALSE;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    }

    
    // データ差し替え(2階)
    {
      // エレベータ部屋
      p_rd = &p_dungeon_data->room_data[1][7];
      p_rd->room_type = ROOM_TYPE_ELEVATOR;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = TRUE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= FALSE;
    }
    {
      u8  tr_pos_type = INF_ROOM_POS_TYPE_Q_1;
      p_rd = &p_dungeon_data->room_data[1][5];
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = TRUE;
      p_rd->can_move[INF_MOVE_LEFT] = FALSE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;

      p_rd->tr_pos_type = tr_pos_type;
      p_rd->room_type = GetRoomType(tr_pos_type);
    }
    {
      p_rd = &p_dungeon_data->room_data[1][6];
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = TRUE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    }
    {
      u8  tr_pos_type = INF_ROOM_POS_TYPE_E_1;
      p_rd = &p_dungeon_data->room_data[1][10];
      p_rd->can_move[INF_MOVE_UP]   = TRUE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = FALSE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;

      p_rd->tr_pos_type = tr_pos_type;
      p_rd->room_type = GetRoomType(tr_pos_type);
    }
    {
      u8  tr_pos_type = INF_ROOM_POS_TYPE_S_1;
      p_rd = &p_dungeon_data->room_data[1][11];
      p_rd->can_move[INF_MOVE_UP]   = TRUE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;

      p_rd->tr_pos_type = tr_pos_type;
      p_rd->room_type = GetRoomType(tr_pos_type);
    }
    {
      p_rd = &p_dungeon_data->room_data[1][12];
      p_rd->can_move[INF_MOVE_UP]   = TRUE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= FALSE;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    }

    // データ差し替え(3階)
    {
      // エレベータ部屋
      p_rd = &p_dungeon_data->room_data[2][11];
      p_rd->room_type = ROOM_TYPE_ELEVATOR;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = TRUE;
      p_rd->can_move[INF_MOVE_LEFT] = FALSE;
      p_rd->can_move[INF_MOVE_RIGHT]= FALSE;
    }
    {
      // ボス部屋
      p_rd = &p_dungeon_data->room_data[2][13];
      p_rd->room_type = ROOM_TYPE_BOSS;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = TRUE;
      p_rd->can_move[INF_MOVE_LEFT] = FALSE;
      p_rd->can_move[INF_MOVE_RIGHT]= FALSE;
    }
    {
      p_rd = &p_dungeon_data->room_data[2][15];
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = FALSE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    }
    {
      p_rd = &p_dungeon_data->room_data[2][16];
      p_rd->can_move[INF_MOVE_UP]   = TRUE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    }
    {
      u8  tr_pos_type = INF_ROOM_POS_TYPE_F_1;
      p_rd = &p_dungeon_data->room_data[2][17];
      p_rd->can_move[INF_MOVE_UP]   = FALSE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= TRUE;

      p_rd->tr_pos_type = tr_pos_type;
      p_rd->room_type = GetRoomType(tr_pos_type);
    }
    {
      p_rd = &p_dungeon_data->room_data[2][18];
      p_rd->can_move[INF_MOVE_UP]   = TRUE;
      p_rd->can_move[INF_MOVE_DOWN] = FALSE;
      p_rd->can_move[INF_MOVE_LEFT] = TRUE;
      p_rd->can_move[INF_MOVE_RIGHT]= FALSE;
      p_rd->tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;
    }

    // 4階
    p_dungeon_data->room_data[3][0].room_type = ROOM_TYPE_ELEVATOR;
    p_dungeon_data->room_data[3][1].tr_pos_type = INF_ROOM_POS_TYPE_E_1;

    // 5階
    p_dungeon_data->room_data[4][0].room_type = ROOM_TYPE_ELEVATOR;
    p_dungeon_data->room_data[4][1].tr_pos_type = INF_ROOM_POS_TYPE_E_1;

    // 6階
    p_dungeon_data->room_data[5][0].room_type = ROOM_TYPE_ELEVATOR;
    p_dungeon_data->room_data[5][1].tr_pos_type = INF_ROOM_POS_TYPE_E_1;

    // 7階
    p_dungeon_data->room_data[6][0].room_type = ROOM_TYPE_ELEVATOR;
    p_dungeon_data->room_data[6][1].tr_pos_type = INF_ROOM_POS_TYPE_E_1;

    // 8階
    p_dungeon_data->room_data[7][0].room_type = ROOM_TYPE_ELEVATOR;
    p_dungeon_data->room_data[7][1].tr_pos_type = INF_ROOM_POS_TYPE_E_1;

    // 9階
    p_dungeon_data->room_data[8][0].room_type = ROOM_TYPE_ELEVATOR;
    p_dungeon_data->room_data[8][1].tr_pos_type = INF_ROOM_POS_TYPE_E_1;

    // 10階
    p_dungeon_data->room_data[9][0].room_type = ROOM_TYPE_ELEVATOR;
    p_dungeon_data->room_data[9][1].tr_pos_type = INF_ROOM_POS_TYPE_E_1;

 }
#endif

}

//----------------------------------------------------------------------------
/**
 *	@brief  指定数トレーナーが減るように部屋タイプを入れ替える
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RemoveTrRoom( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_no ,int rmv_num ,GFL_STD_RandContext *rand_context )
{
  u8  pos_type;
  int cnt = 0;
  int i;
  u32 index_array[ROOM_MAX];

  GFL_STD_MemFill(index_array, ROOM_MAX, sizeof(int)*ROOM_MAX);
  for( i = 0; i< ROOM_MAX; i++ )
  {
    u8 pos_type = p_dungeon_data->room_data[floor_no][i].tr_pos_type;
    if( pos_type != INF_TR_ROOM_POS_TYPE_NONE )
    {
      index_array[cnt] = i;
      cnt++;
    }
  }

  // ランダム抽選
  RndArray( index_array ,cnt ,rand_context);

  // 部屋を削ってゆく
  for( i = 0; i< cnt; i++ )
  {
    pos_type = p_dungeon_data->room_data[floor_no][index_array[i]].tr_pos_type;
    rmv_num -= GetMemberNum(pos_type);
    p_dungeon_data->room_data[floor_no][index_array[i]].tr_pos_type = INF_TR_ROOM_POS_TYPE_NONE;

    if( rmv_num <= 0 )
    {
      OS_Printf("%d部屋削りました\n",i+1);
      return;
    }
  }

  GF_ASSERT_MSG(0,"トレーナーが配置されている部屋がありません\n");
}


//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーデータ
 *
 *	@param	*p_dungeon_data フロアデータ
 *	@param	area_no          生成用シード
 *	@param	seed          生成用シード
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTrainerData( INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no , u32 seed )
{
  int i,n;
  int tr_cnt = 0;
  u32 trid_array[INF_DUNGEON_TR_DATA_MAX];
  GFL_STD_RandContext rand_context;
  
  // 乱数を作成
  GFL_STD_RandInit( &rand_context, seed );
  
  // トレーナーIDテーブル取得
  tr_cnt = GetSetTrainerIDTable( p_dungeon_data , area_no ,trid_array, &rand_context  );

  // トレーナーIDのランダムデータを生成
  for( n = 0; n<INFINITYDUNGEON_FLOOR_MAX; n++)
  {
    // トレーナーデータ配列をランダム化
    RndArray( trid_array ,tr_cnt ,&rand_context);

    // 必要な数だけトレーナーIDを保存
    for( i = 0; i<INFINITY_DUNGEON_TRID_MAX; i++)
    {
      p_dungeon_data->tr_data[n].tr_id[i] = trid_array[i];
    }

    // 回復トレーナー部屋IDリストを初期化
    p_dungeon_data->recover_room_id[n] = ROOM_MAX;
  }

  // 回復トレーナー配置位置を決める
  {
    int room_id_array[ROOM_MAX];
    u32 member_num[ROOM_MAX];
    u32 recover_index = 0;

    for( n = 0; n<INFINITYDUNGEON_FLOOR_MAX; n++)
    {
      int cnt = 0;
      for( i = 0; i<ROOM_MAX; i++)
      {
        u8 pos_type = p_dungeon_data->room_data[n][i].tr_pos_type;
        if( GetMemberNum(pos_type) > 0 )
        {
          room_id_array[cnt] = i;
          member_num[cnt] = GetMemberNum(pos_type);
          cnt++;
        }
      }

      // 配置不能な場合対処
      {
        GF_ASSERT_MSG(cnt!=0,"回復トレーナーが置ける場所がありません\n");
        if( cnt <= 0 )
        {
          return;
        }
      }

#ifdef PM_DEBUG
      if( (g_debug_inf_is_presentation == FALSE) )
      {
        // 通常配置
        SetRecoverTrainer( p_dungeon_data ,n ,room_id_array ,cnt ,member_num ,&rand_context ,&recover_index);
      }
      else
      {
        if( (n == 0 ) || (n == 1 ) || (n == 2 ) ) 
        {
          // プレゼンテーション固定
          u16 tr_id;
          int id_pos = 0;

          switch(n)
          {
          case 0:
          case 2:
            id_pos = GetRoomTrainerIDPos( p_dungeon_data ,n ,24 ,0);
            OS_Printf("回復トレーナー部屋NO：24（固定）\n");
            break;
          case 1:
            id_pos = GetRoomTrainerIDPos( p_dungeon_data ,n ,10 ,0);
            OS_Printf("回復トレーナー部屋NO：10（固定）\n");
            break;
          }
          tr_id = INF_TRID_START_RECOVERY + GFL_STD_Rand( &rand_context ,INF_TRID_CNT_RECOVERY);
          p_dungeon_data->tr_data[n].tr_id[id_pos] = tr_id;
          p_dungeon_data->recover_data[recover_index] = tr_id;
          recover_index++;
        }
        else
        {
          SetRecoverTrainer( p_dungeon_data ,n ,room_id_array ,cnt ,member_num ,&rand_context ,&recover_index);
        }
      }

#else
      SetRecoverTrainer( p_dungeon_data ,n ,room_id_array ,cnt ,member_num ,&rand_context ,&recover_index);
#endif
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナー設置
 *
 *	@param	*p_dungeon_data ダンジョンデータ
 *	@param	floor_no        設置フロア
 *	@param	*room_id_array  配置可能部屋リスト
 *	@param	cnt             配置可能部屋数
 *	@param	*member_num     トレーナー数配列
 *	@param	*rand_context   
 *	@param	*recover_index  回復トレーナーインデックス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetRecoverTrainer( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no 
    ,int *room_id_array ,int cnt,u32 *member_num , GFL_STD_RandContext *rand_context ,u32 *recover_index )
{
  int id_pos = 0;
  u16 tr_id;
  u32 room_id_index;
  u32 member_index;

  room_id_index = GFL_STD_Rand( rand_context ,cnt);
  member_index = GFL_STD_Rand( rand_context ,member_num[room_id_index]);
  id_pos = GetRoomTrainerIDPos( p_dungeon_data ,floor_no ,room_id_array[room_id_index] ,member_index);

  tr_id = INF_TRID_START_RECOVERY + GFL_STD_Rand( rand_context ,INF_TRID_CNT_RECOVERY);
  p_dungeon_data->tr_data[floor_no].tr_id[id_pos] = tr_id;
  p_dungeon_data->recover_data[(*recover_index)] = tr_id;
  (*recover_index)++;
  p_dungeon_data->recover_room_id[floor_no] = room_id_array[room_id_index];
  OS_Printf("回復トレーナー部屋NO：%u\n",room_id_array[room_id_index]);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ブラック用トレーナーIDテーブルを取得
 *
 *	@param	*p_trid_array 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int GetSetTrainerIDTable( INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,u32 *p_trid_array ,GFL_STD_RandContext *rand_context)
{
  int i;
  u16 tr_id = 0;
  int cnt = 0;
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data );

  // 通常トレーナー
  for( i = 0 ; i < INF_TR_AREA_DATA_MAX; i++ )
  {
    GF_ASSERT_MSG(cnt<INF_DUNGEON_TR_DATA_MAX,"無限ダンジョンのトレーナー抽選に使用するトレーナー数が多すぎます(%d<%d)\n",cnt,INF_DUNGEON_TR_DATA_MAX);

    if( (area_no >= sc_tr_area_data[i].start_area ) &&
        (area_no <= sc_tr_area_data[i].end_area ) )
    {
      // バージョン指定なし=0
      if( (sc_tr_area_data[i].version == version) ||
          (sc_tr_area_data[i].version == 0) )
      {
        // 出現率計算
        if( GFL_STD_Rand( rand_context ,100 ) < sc_tr_area_data[i].set_lv )
        {
          p_trid_array[cnt] = i;
          cnt++;
        }
      }
    }
  }
  // お金トレーナー
  for( i = 0; i < INF_TRID_CNT_MONEY; i++ )
  {
    p_trid_array[cnt] = INF_TRID_START_MONEY+i;
    cnt++;
  }
  // 1階に32人まで出現する可能性がある
  GF_ASSERT_MSG(cnt>=32,"エリア%u-抽選に使用するトレーナーが少なすぎます(%d<=%d)\n",area_no,cnt,32);

  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  配置モデルで通行禁止の場所をふさぐ
 *
 *	@param	*p_wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetBUildModel( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data)
{
  //-------------------------------------
  // 封鎖用モデルOBJ(黒)
  //=====================================
  static const BMODEL_ID sc_black_bmodel_table[] = {
    NARC_output_buildmodel_indoor_mugenobj01_nsbmd, // 上
    NARC_output_buildmodel_indoor_mugenobj05_nsbmd, // 下
    NARC_output_buildmodel_indoor_mugenobj04_nsbmd, // 右
    NARC_output_buildmodel_indoor_mugenobj06_nsbmd, // 左
  };
  //-------------------------------------
  // 封鎖用モデルOBJ(白)
  //=====================================
  static const BMODEL_ID sc_white_bmodel_table[] = {
    NARC_output_buildmodel_indoor_mugenobj02_nsbmd, // 上
    NARC_output_buildmodel_indoor_mugenobj08_nsbmd, // 下
    NARC_output_buildmodel_indoor_mugenobj09_nsbmd, // 右
    NARC_output_buildmodel_indoor_mugenobj07_nsbmd, // 左
  };

  GFL_G3D_OBJSTATUS status;
  const BMODEL_ID *p_bmodel_id_table;
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data );

  // 表示座標を初期化
  status.trans.x = 0;
  status.trans.y = 0;
  status.trans.z = 0;
  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );

  // モデルテーブル差し替え
  if( version == VERSION_B2 )
  {
    p_bmodel_id_table = sc_black_bmodel_table;
  }
  else
  {
    p_bmodel_id_table = sc_white_bmodel_table;
  }
  
  // 配置
  if( p_room_data->room_type != ROOM_TYPE_BOSS )
  {
    // 上
    if( p_room_data->room_type != ROOM_TYPE_ELEVATOR )
    {
      if( !p_room_data->can_move[INF_MOVE_UP] )
      {
        FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_UP], &status );
      }
    }

    // 下
    if( !p_room_data->can_move[INF_MOVE_DOWN] )
    {
      FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_DOWN], &status );
    }

    // 左
    if( !p_room_data->can_move[INF_MOVE_LEFT]  )
    {
      FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_LEFT], &status );
    }

    // 右
    if( !p_room_data->can_move[INF_MOVE_RIGHT] )
    {
      FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_RIGHT], &status );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダミーオブジェクトを追加する
 *
 *	@param	*p_wk 
 *	@param	p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDummyObj( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data)
{
  // ボス部屋は封鎖しない
  if( p_room_data->room_type != ROOM_TYPE_BOSS )
  {
    // 上(エレベータ部屋には上封鎖オブジェクトを置くことがない)
    if( p_room_data->room_type != ROOM_TYPE_ELEVATOR )
    {
      if( !p_room_data->can_move[INF_MOVE_UP] )
      {
        // ダミーオブジェクトで封鎖
        const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_UP );
        GRID_DATA gd = *p_grid_data;
        AddDummyObj( p_wk , &gd );  // 中
        gd.x = (*p_grid_data).x + 1;
        AddDummyObj( p_wk , &gd );  // 左
        gd.x = (*p_grid_data).x - 1;
        AddDummyObj( p_wk , &gd );  // 右
      }
    }

    // 下
    if( !p_room_data->can_move[INF_MOVE_DOWN] )
    {
      // ダミーオブジェクトで封鎖
      const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_DOWN );
      AddDummyObj( p_wk , p_grid_data );
    }

    // 左
    if( !p_room_data->can_move[INF_MOVE_LEFT] )
    {
      // ダミーオブジェクトで封鎖
      const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_LEFT );
      GRID_DATA gd = *p_grid_data;
      AddDummyObj( p_wk , &gd );  // 中
      gd.z = (*p_grid_data).z - 1;
      AddDummyObj( p_wk , &gd );  // 上
      gd.z = (*p_grid_data).z + 1;
      AddDummyObj( p_wk , &gd );  // 下
    }

    // 右
    if( !p_room_data->can_move[INF_MOVE_RIGHT] )
    {
      // ダミーオブジェクトで封鎖
      const GRID_DATA *p_grid_data = GetBmodelPos( p_wk ,p_room_data->room_type ,WARP_DIR_RIGHT );
      GRID_DATA gd = *p_grid_data;
      AddDummyObj( p_wk , &gd );  // 中
      gd.z = (*p_grid_data).z - 1;
      AddDummyObj( p_wk , &gd );  // 上
      gd.z = (*p_grid_data).z + 1;
      AddDummyObj( p_wk , &gd );  // 下
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダミーオブジェクトの追加
 *
 *	@param	*p_wk 
 *	@param	x 
 *	@param	y
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void AddDummyObj( MAP_CHAGE_EVENT_WORK *p_wk , const GRID_DATA *p_grid_data)
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
  MMDLSYS_AddMMdlParam( p_wk->p_mmdlsys, p_grid_data->x, p_grid_data->z, DIR_UP,
      INFINITY_DUNGEON_BLOCK_OBJ_ID, NONDRAW, MV_DMY, p_next_map_data->zone_id );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内復帰処理
 *
 *	@param	*p_gsys 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_Recovery( GAMESYS_WORK *p_gsys ,INF_DUNGEON_DATA  *p_dungeon_data)
{
  MAP_CHAGE_EVENT_WORK  *p_wk;
  ROOM_DATA             *p_room_data;

  // ワーク確保
  p_wk = GFL_HEAP_AllocMemoryLo( HEAPID_FIELDMAP, sizeof( MAP_CHAGE_EVENT_WORK ) );

  // ダンジョンデータの設定
  p_wk->p_dungeon_data = p_dungeon_data;

  // ワーク初期化
  InitWorkData( p_wk ,p_gsys );

  // 配置モデルを配置
  p_room_data = GetNowRoomData( p_wk );
  SetBUildModel(p_wk,p_room_data);
 
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールド指定用方向定義に変換する
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u16 GetFieldDir( u16 warp_dir )
{
  // 変換
  switch( warp_dir )
  {
  case WARP_DIR_UP:
    return DIR_UP;
  case WARP_DIR_DOWN:
    return DIR_DOWN;
  case WARP_DIR_LEFT:
    return DIR_LEFT;
  case WARP_DIR_RIGHT:
    return DIR_RIGHT;
  case WARP_INFORMATION:
    return WARP_DIR_DOWN;
  default:
    return WARP_DIR_DOWN;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーを配置する
 *
 *	@param	*p_wk 
 *	@param	p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTrainer( MAP_CHAGE_EVENT_WORK *p_wk , ROOM_DATA* p_room_data)
{
  int i;
  const INF_ROOM_SET_TR_PARAM *p_tr_room;

  if( GetMemberNum(p_room_data->tr_pos_type) > 0 )
  {
    p_tr_room = &sc_room_tr_param[p_room_data->tr_pos_type];

    for( i= 0; i<GetMemberNum(p_room_data->tr_pos_type); i++ )
    {
      const INF_ROOM_TR_DATA *p_md = &p_tr_room->tr_move_data[i];
      NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
      const INF_MMDL_PARAM *p_mmdlp = &sc_mmdl_param[p_md->move_type];
      u16 tr_id;
      u8  id_pos;

      id_pos = GetRoomTrainerIDPos( p_wk->p_dungeon_data,p_wk->now_floor, p_room_data->room_id ,i );
      tr_id = INFINITY_DUNGEON_GetTrIDByIDPos( p_wk->p_dungeon_data,p_wk->now_floor ,id_pos );
      if( tr_id != INF_DUNGEON_TR_ID_MAX )
      {
        u16 obj_code = FBI_TOOL_GetOBJCode( ARCID_INFINITY_DUGENON_TR , tr_id ,p_wk->heapID);
        MMDL *mmdl = MMDLSYS_AddMMdlParam( p_wk->p_mmdlsys,p_md->g_x, p_md->g_y, p_mmdlp->direction,
            id_pos , obj_code, p_mmdlp->move_code,
            p_next_map_data->zone_id );
        MMDL_SetEventType( mmdl , p_mmdlp->ev_type);
        MMDL_SetParam( mmdl, p_mmdlp->param0 , MMDL_PARAM_0 );
        MMDL_SetParam( mmdl, p_mmdlp->param1 , MMDL_PARAM_1 );
        MMDL_SetMoveLimitX( mmdl, p_mmdlp->lim_x );
        MMDL_SetMoveLimitZ( mmdl, p_mmdlp->lim_z );
        MMDL_SetEventID( mmdl, SCRID_INFINITY_DUNGEON_TALK_BATTLE_TRAINER );
        OS_Printf("配置TRID=%u\n",tr_id);
      }
    }
  }
  
#ifdef PM_DEBUG
  if( g_debug_inf_data_change != TRUE )
  {
    OS_Printf("トレーナー配置パターン番号：%d\n",p_room_data->tr_pos_type);
  }
  else
  {
    OS_Printf("上書き->レーナー配置パターン番号：%d\n",g_debug_inf_tr_pos_type);
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定部屋IDのトレーナーID格納位置取得
 *
 *	@param	*p_dungeon_data フロアデータ
 *	@param	floor_id      フロアID
 *	@param	room_id       部屋ID
 *	@param	room_tr_no    部屋内の相対ID（0～4）
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int GetRoomTrainerIDPos( INF_DUNGEON_DATA *p_dungeon_data ,u32 floor_id ,u32 room_id,u8 room_tr_no)
{
  int i;
  u32 tr_num = 0;

  for( i = 0; i<ROOM_MAX; i++ )
  {
    u32 tr_pos_type = p_dungeon_data->room_data[floor_id][i].tr_pos_type;
    
    // 指定の部屋まで移動した場合は終了する
    if( room_id <= i )
    {
      break;
    }

    // 人数加算
    if( GetMemberNum( tr_pos_type ) > 0 )
    {
      tr_num+=GetMemberNum( tr_pos_type );
    }
  }

  return tr_num+room_tr_no;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内トレーナー戦闘イベント
 *
 *	@param	*gsys
 *	@param	*fieldmap 
 *	@param	obj_id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT * INFINITY_DUNGEON_EVENT_TrainerBattle(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap ,u8 obj_id)
{
  GMEVENT   *event;
  GAMEDATA  *p_gdata;
  SAVE_CONTROL_WORK     *p_save_data;
  INFINITYDUNGEON_SAVE  *p_save;
  u32 area_no;

  BSUBWAY_TRAINER_ROM_DATA  *trd;
  BSUBWAY_PARTNER_DATA tr_data;
  HEAPID heapID = HEAPID_FIELDMAP;
  u16 tr_id;
  GFL_STD_RandContext rand_context;

  INF_DUNGEON_DATA *p_dungeon_data;
  u32 tr_seed;
  u32 floor_id;
  u8  id_pos = obj_id;

  // セーブデータの取得
  p_gdata       = GAMESYSTEM_GetGameData( gsys );
  p_save_data         = GAMEDATA_GetSaveControlWork( p_gdata );
  p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
   
  // 現在のエリア取得
  area_no = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );

  // トレーナーシード
  p_dungeon_data  = INFINITY_DUNGEON_DataGet(fieldmap);
  floor_id        = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
  tr_seed         = GetTrSeed( p_dungeon_data ,floor_id ,id_pos );

  // トレーナーID
  tr_id = INFINITY_DUNGEON_GetTrIDByIDPos( p_dungeon_data ,floor_id ,id_pos );

  // 乱数を作成
  GFL_STD_RandInit( &rand_context, tr_seed );
  
  //トレーナーデータセット
  trd = FBI_TOOL_AllocTrainerRomData(&tr_data ,ARCID_INFINITY_DUGENON_TR ,tr_id,NARC_message_infinity_trname_dat ,heapID);

  //ポケモンデータをセット
  FBI_TOOL_SetBSWayPokemonParam(
      trd, tr_id , tr_data.btpwd,2,
      ARCID_INFINITY_DUGENON_PD,NULL, NULL, NULL, &rand_context, sc_pow_rnd_table[area_no], heapID);

  GFL_HEAP_FreeMemory( trd );

  {
    // 手持ちパーティー生成
    BATTLE_SETUP_PARAM *dst;
    int i;
    POKEPARTY *party;
	  POKEMON_PARAM * pp;
    const POKEPARTY *myparty;
    u8 member_num;

    myparty = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData( gsys ) ); 
    member_num = PokeParty_GetPokeCount(myparty);
    party = PokeParty_AllocPartyWork( GFL_HEAP_LOWID(heapID) );
	  PokeParty_Init( party, member_num );
    for( i=0; i<member_num; i++ ){
      pp = PokeParty_GetMemberPointer( myparty, i );
      PokeParty_Add( party, pp );
    }

    // バトルパラメータの作成
    dst = FBI_TOOL_INFINITY_DUNGEON_CreateBattleParam(
        gsys, party, 
        member_num , 0 , 2,
        &tr_data,
        NULL ,TRID_OFFSET_INFINITY_DUNGEON ,
        sc_level_table[area_no].level_max ,
        sc_level_table[area_no].level_min , 0 , &rand_context);

    GFL_HEAP_FreeMemory(party);

    event = EVENT_InfinityDungeonTrainerBattle( gsys, fieldmap, dst );
  }
  return( event );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンのフロアデータを取得
 *
 *	@param	*p_save       無限ダンジョンセーブ
 *	@param	*p_dungeon_data フロアデータセット先
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetFloorData( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data )
{
  int i;
  u32 area_no = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );
  INF_DUNGEON_SEED *p_seed = &p_dungeon_data->seed;
  BOOL is_tutorial;

  // チュートリアルMAPをセットする
  is_tutorial = SetTutorialMap( p_save ,p_dungeon_data );

  // チュートリアルMAPをセットしなかったときは、ダンジョンを自動生成
  if( is_tutorial == FALSE )
  {
    // ダンジョンデータを生成(エリアに対応したサイズで生成する)
    SetDungeonData( p_dungeon_data ,area_no ,p_seed->dungeon_data_seed ,sc_floor_size[area_no].fs_x ,sc_floor_size[area_no].fs_y);

    // トレーナーデータをセット
    SetTrainerData( p_dungeon_data ,area_no ,p_seed->tr_data_seed );
  }
  else
  {
    // チュートリアル用トレーナーデータをセットする
    SetTutorialTrainerData( p_save ,p_dungeon_data );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  チュートリアルMAPをセットする
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SetTutorialMap( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data )
{
  u8 dungeon_status;
  FIELD_WFBC_CORE_TYPE wfbc_type;

  // バージョンからWFBCの状態を取得
  if( p_dungeon_data->version == VERSION_B2 )
  {
    wfbc_type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
  }
  else
  {
    wfbc_type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
  }

  // 自分と同じバージョンの時のみチュートリアルマップにさしかえる
  if ( GET_VERSION() == p_dungeon_data->version )
  {
    // ダンジョンの状態を取得
    dungeon_status = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_save ,wfbc_type );

    // チュートリアル用マップに差替え(エリア2クリアまでさしかえ)
    if( (dungeon_status < INF_TUTORIAL_DATA_CNT) )
    {
      u8 tutorial_data_index = dungeon_status;
      const ROOM_DATA *p_w_tutorialmap_table[] =
      {
        sc_tutorial_rooo_data_w1,
        sc_tutorial_rooo_data_w2,
      };
      const ROOM_DATA *p_b_tutorialmap_table[] =
      {
        sc_tutorial_rooo_data_b1,
        sc_tutorial_rooo_data_b2,
      };

      // データをクリア
      GFL_STD_MemFill(p_dungeon_data->room_data, 0, sizeof(p_dungeon_data->room_data));

      // ダンジョンの状態を設定
      if( p_dungeon_data->version == VERSION_B2 )
      {
        GFL_STD_MemCopy( p_b_tutorialmap_table[tutorial_data_index], p_dungeon_data->room_data, sizeof(sc_tutorial_rooo_data_b1));
      }
      else
      {
        GFL_STD_MemCopy( p_w_tutorialmap_table[tutorial_data_index], p_dungeon_data->room_data, sizeof(sc_tutorial_rooo_data_b1));
      }
      return TRUE;
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  チュートリアルトレーナーをセットする
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTutorialTrainerData( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data )
{
  u8 dungeon_status;
  FIELD_WFBC_CORE_TYPE wfbc_type;

  // バージョンからWFBCの状態を取得
  if( p_dungeon_data->version == VERSION_B2 )
  {
    wfbc_type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
  }
  else
  {
    wfbc_type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
  }

  // ダンジョンの状態を取得
  dungeon_status = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_save ,wfbc_type );

  // チュートリアル用トレーナーをセットする
  {
    u8 tutorial_data_index = dungeon_status;

    // トレーナーデータ配列一覧
    const u16 *p_w_tutorialtr_table[][2] =
    {
      {
        sc_tutorial_tr_data_w1,
        sc_tutorial_rectr_data_w1,
      },
      {
        sc_tutorial_tr_data_w2,
        sc_tutorial_rectr_data_w2,
      },
    };
    const u16 *p_b_tutorialtr_table[][2] =
    {
      {
        sc_tutorial_tr_data_b1,
        sc_tutorial_rectr_data_b1,
      },
      {
        sc_tutorial_tr_data_b2,
        sc_tutorial_rectr_data_b2,
      },
    };
    
    // データをクリア
    GFL_STD_MemFill(p_dungeon_data->tr_data, 0, sizeof(p_dungeon_data->tr_data));
    GFL_STD_MemFill(p_dungeon_data->recover_data, 0, sizeof(p_dungeon_data->recover_data));

    // ダンジョンの通常トレーナーを設定
    if( p_dungeon_data->version == VERSION_B2 )
    {
      GFL_STD_MemCopy( p_b_tutorialtr_table[tutorial_data_index][0], &p_dungeon_data->tr_data[0], sizeof(DUNGEON_TRAINER));
      GFL_STD_MemCopy16( p_b_tutorialtr_table[tutorial_data_index][1], p_dungeon_data->recover_data, sizeof(p_dungeon_data->recover_data));
    }
    else
    {
      GFL_STD_MemCopy( p_w_tutorialtr_table[tutorial_data_index][0], &p_dungeon_data->tr_data[0], sizeof(DUNGEON_TRAINER));
      GFL_STD_MemCopy16( p_w_tutorialtr_table[tutorial_data_index][1], p_dungeon_data->recover_data, sizeof(p_dungeon_data->recover_data));
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボス部屋の存在するフロアを返す
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetBossFloor( INF_DUNGEON_DATA *p_dungeon_data )
{
  s32 boss_room;
  s32 boss_floor;

  // ボスの存在するフロアを検索する
  for( boss_floor = 0; boss_floor<INFINITYDUNGEON_FLOOR_MAX; boss_floor++ )
  {
    for( boss_room = 0; boss_room<ROOM_MAX; boss_room++ )
    {
      if( p_dungeon_data->room_data[boss_floor][boss_room].room_type == ROOM_TYPE_BOSS )
      {
        return boss_floor;
      }
    }
  }

  GF_ASSERT_MSG(0,"ボス部屋が見つかりません");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボス部屋の存在する部屋IDを返す
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 INFINITY_DUNGEON_GetBossRoom( INF_DUNGEON_DATA *p_dungeon_data )
{
  s32 boss_room;
  s32 boss_floor;

  // ボスの存在するフロアを検索する
  for( boss_floor = 0; boss_floor<INFINITYDUNGEON_FLOOR_MAX; boss_floor++ )
  {
    for( boss_room = 0; boss_room<ROOM_MAX; boss_room++ )
    {
      if( p_dungeon_data->room_data[boss_floor][boss_room].room_type == ROOM_TYPE_BOSS )
      {
        return boss_room;
      }
    }
  }

  GF_ASSERT_MSG(0,"ボス部屋が見つかりません");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  表示コードに対応した会話パターンを返す
 *
 *	@param	obj_code
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetTrainerHintType( u16 obj_code)
{
  int i;
  for( i = 0; i < INFINITY_DUNGEON_TALK_PATTERN_MAX; i++ )
  {
    if( sc_talk_pattern_table[i].obj_code == obj_code )
    {
      return sc_talk_pattern_table[i].hint_type;
    }
  }

  GF_ASSERT_MSG(0,"ヒントタイプの登録されていない表示コードです\n");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  表示コードに対応した会話パターンを返す
 *
 *	@param	obj_code
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetTrainerTalkType( u16 obj_code)
{
  int i;
  for( i = 0; i < INFINITY_DUNGEON_TALK_PATTERN_MAX; i++ )
  {
    if( sc_talk_pattern_table[i].obj_code == obj_code )
    {
      return sc_talk_pattern_table[i].talk_type;
    }
  }

  GF_ASSERT_MSG(0,"会話パターンの登録されていない表示コードです\n");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージを取得する
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *	@param	obj_code
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetHintMessage( INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data ,u16 obj_code)
{
  u8    talk_type;
  u8    hint_type;
  int   sub_str_num = 0;
  u32   ret_msg_id;
  BOOL  is_get_msg_id = FALSE;
  u8    now_floor;
  TRHINT_WK trhint_wk;

  // ヒントメッセージ取得プログラム
  HINTMSGPROC hint_msg_proc_table[ INFTR_HINT_TYPE_MAX ] =
  {
    gethint_is_up,
    gethint_is_down,
    gethint_boss_floor,
    gethint_boss_dir,
    gethint_boss_monster,
    gethint_medic_dir,
    gethint_no_hint,
  };

  // OBJコードから会話パターンを選択
  talk_type = GetTrainerTalkType( obj_code );
  hint_type = GetTrainerHintType( obj_code );

  // ヒント取得ワークにパラメータを設定する
  {
    trhint_wk.p_dungeon_data = p_dungeon_data;
    trhint_wk.talk_type = talk_type;
    trhint_wk.p_save = p_save;
  }

  // ヒントメッセージ取得
  return hint_msg_proc_table[hint_type]( &trhint_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：上にいるかどうか
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_is_up( TRHINT_WK *p_wk )
{
  // 現在のフロア
  u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  // ボス部屋の存在するフロア
  u8 boss_floor = INFINITY_DUNGEON_GetBossFloor( p_wk->p_dungeon_data );

  // メッセージを取得
  u32 msg_id = msg_hint_updown_unknown_01;

  ONOUE_Printf("現在のフロア：%u,ボスのフロア%u\n",now_floor ,boss_floor );
  ONOUE_Printf("ヒント：上にいるか->");
  if( now_floor < boss_floor )
  {
    ONOUE_Printf("上にいる\n");
    msg_id = msg_hint_up_01;
  }
  else
  {
    ONOUE_Printf("しらない\n");
  }

  return msg_id + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：下にいるかどうか
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_is_down( TRHINT_WK *p_wk )
{
  // 現在のフロア
  u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  // ボス部屋の存在するフロア
  u8 boss_floor = INFINITY_DUNGEON_GetBossFloor( p_wk->p_dungeon_data );

  // メッセージを取得
  u32 msg_id = msg_hint_updown_unknown_01;

  ONOUE_Printf("現在のフロア：%u,ボスのフロア%u\n",now_floor ,boss_floor );
  ONOUE_Printf("ヒント：下にいるか->");
  if( now_floor > boss_floor )
  {
    msg_id = msg_hint_down_01;
    ONOUE_Printf("下にいる\n");
  }
  else
  {
    ONOUE_Printf("しらない\n");
  }

  return msg_id + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ボスが何階にいるか
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_boss_floor( TRHINT_WK *p_wk )
{
  ONOUE_Printf("ヒント：どのフロアにいるか\n");
  return msg_hint_boss_floor_01 + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ボスの方向
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_boss_dir( TRHINT_WK *p_wk )
{
  // 現在のフロア
  u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  // ボス部屋の存在するフロア
  u8 boss_floor = INFINITY_DUNGEON_GetBossFloor( p_wk->p_dungeon_data );
  
  // ボスのいる部屋ID
  u8 boss_room = INFINITY_DUNGEON_GetBossRoom( p_wk->p_dungeon_data );
  // 主人公のいる部屋ID
  u8 now_room = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );

  // メッセージを取得
  u32 msg_id = msg_hint_dir_unknown_01;

  ONOUE_Printf("現在のフロア：%u,ボスのフロア%u\n",now_floor ,boss_floor );
  ONOUE_Printf("現在の部屋番号：%u,ボスの部屋番号%u\n",now_floor ,boss_room );
  if( now_floor == boss_floor )
  {
    u8 dir;
    u8 x1 = 0;
    u8 y1 = 0;
    u8 x2 = 0;
    u8 y2 = 0;
    if( now_room != 0 )
    {
      x1  = now_room % ROOM_MAX_X;
      y1  = now_room / ROOM_MAX_Y;
    }
    if( boss_room != 0 )
    {
      x2  = boss_room % ROOM_MAX_X;
      y2  = boss_room / ROOM_MAX_Y;
    }
    dir = GetPosDirID( x1, y1, x2, y2 );
    switch( dir )
    {
    case INFTR_DIR_U:  // 上
      msg_id = msg_hint_boss_dir_u_01;
      break;
    case INFTR_DIR_D:  // 下
      msg_id = msg_hint_boss_dir_d_01;
      break;
    case INFTR_DIR_L:  // 左
      msg_id = msg_hint_boss_dir_l_01;
      break;
    case INFTR_DIR_R:  // 右
      msg_id = msg_hint_boss_dir_r_01;
      break;
    case INFTR_DIR_LU:  // 上左
      msg_id = msg_hint_boss_dir_lu_01;
      break;
    case INFTR_DIR_RU:  // 上右
      msg_id = msg_hint_boss_dir_ru_01;
      break;
    case INFTR_DIR_LD:  // 下左
      msg_id = msg_hint_boss_dir_ld_01;
      break;
    case INFTR_DIR_RD:  // 下右
      msg_id = msg_hint_boss_dir_rd_01;
      break;
    default:
      GF_ASSERT(0);
      msg_id = msg_hint_boss_dir_u_01;
      break;
    }
  }

  return msg_id + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  2つの位置より、方向を導いて返す
 *
 *	@param	x1  基準X
 *	@param	y1  基準Y
 *	@param	x2  目標X
 *	@param	y2  目標Y
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetPosDirID( u8 x1, u8 y1, u8 x2 ,u8 y2 )
{
  if( x1 == x2 )
  {
    // 上下
    if( y1 > y2 )
    {
      return INFTR_DIR_U;
    }
    else
    {
      return INFTR_DIR_D;
    }
  }
  else if( y1 == y2 )
  {
    // 左右
    if( x1 < x2 )
    {
      return INFTR_DIR_R;
    }
    else
    {
      return INFTR_DIR_L;
    }
  }
  else
  {
    if( y1 > y2 )
    {
      // 上＋○
      if( x1 < x2 )
      {
        return INFTR_DIR_RU;
      }
      else
      {
        return INFTR_DIR_LU;
      }
    }
    else
    {
      // 下＋○
      if( x1 < x2 )
      {
        return INFTR_DIR_RD;
      }
      else
      {
        return INFTR_DIR_LD;
      }
    }
  }

  GF_ASSERT(0);
  return INFTR_DIR_U;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ボスのポケモン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_boss_monster( TRHINT_WK *p_wk )
{
  ONOUE_Printf("ヒント：ボスのポケモン\n");
  return msg_hint_boss_poke_01 + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ドクター・ナースの方向
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_medic_dir( TRHINT_WK *p_wk )
{
  // 現在のフロア
  u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  // 主人公のいる部屋ID
  u8 now_room = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );

  // 回復トレーナーのいる部屋ID
  u8 medic_room = p_wk->p_dungeon_data->recover_room_id[now_floor];

  // メッセージを取得
  u32 msg_id = msg_hint_dir_unknown_01;
  u8 dir;
  u8 x1 = 0;
  u8 y1 = 0;
  u8 x2 = 0;
  u8 y2 = 0;

  ONOUE_Printf("現在のフロア：%u,回復トレーナーのフロア%u\n",now_floor ,now_floor );
  ONOUE_Printf("現在の部屋番号：%u,回復トレーナーの部屋番号%u\n",now_floor ,medic_room );

  // 同じ部屋にいる場合は、専用メッセージ
  if( now_room == medic_room )
  {
    // @todo ナースが同じ部屋にいる場合のメッセージIDを入れる
    return msg_hint_nurse_dir_u_01 + p_wk->talk_type;
  }
 
  if( now_room != 0 )
  {
    x1  = now_room % ROOM_MAX_X;
    y1  = now_room / ROOM_MAX_Y;
  }
  if( medic_room != 0 )
  {
    x2  = medic_room % ROOM_MAX_X;
    y2  = medic_room / ROOM_MAX_Y;
  }

  GF_ASSERT( medic_room != ROOM_MAX );

  dir = GetPosDirID( x1, y1, x2, y2 );
  switch( dir )
  {
  case INFTR_DIR_U:  // 上
    msg_id = msg_hint_nurse_dir_u_01;
    break;
  case INFTR_DIR_D:  // 下
    msg_id = msg_hint_nurse_dir_d_01;
    break;
  case INFTR_DIR_L:  // 左
    msg_id = msg_hint_nurse_dir_l_01;
    break;
  case INFTR_DIR_R:  // 右
    msg_id = msg_hint_nurse_dir_r_01;
    break;
  case INFTR_DIR_LU:  // 上左
    msg_id = msg_hint_nurse_dir_lu_01;
    break;
  case INFTR_DIR_RU:  // 上右
    msg_id = msg_hint_nurse_dir_ru_01;
    break;
  case INFTR_DIR_LD:  // 下左
    msg_id = msg_hint_nurse_dir_ld_01;
    break;
  case INFTR_DIR_RD:  // 下右
    msg_id = msg_hint_nurse_dir_rd_01;
    break;
  default:
    GF_ASSERT(0);
    msg_id = msg_hint_nurse_dir_u_01;
    break;
  }

  return msg_id + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ヒントなし
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_no_hint( TRHINT_WK *p_wk )
{
  return msg_hint_unknown_01 + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンのIDの場所からトレーナーIDを取得
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_id 
 *	@param	id_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 INFINITY_DUNGEON_GetTrIDByIDPos( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u8 id_pos )
{
  return p_dungeon_data->tr_data[floor_id].tr_id[id_pos];
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーごとのシードを取得する
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_id 
 *	@param	tr_id_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetTrSeed( INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u32 tr_id_pos )
{
  return p_dungeon_data->seed.tr_seed[floor_id][tr_id_pos];
}

//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナーの回復フラグを取得
 *
 *	@param	*p_dungeon_data 
 *	@param	tr_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetRecoverFlgPos( INF_DUNGEON_DATA *p_dungeon_data ,u16 tr_id )
{
  u32 i;
  u32 master_seed;
  u16 *p_trid;

  // トレーナーIDリストを取得
  p_trid = p_dungeon_data->recover_data;

  // 検索
  for( i= 0; i<INFINITY_DUNGEON_RECOVERTR_MAX; i++ )
  {
    ONOUE_Printf("trid =%d\n",p_trid[i]);
    if( p_trid[i] == tr_id )
    {
      return i;
    }
  }

  GF_ASSERT_MSG(0,"ID位置が発見できませんでした");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベータでの移動か調べる
 *
 *	@param	*p_wk 
 *
 *	@retval TRUEの時エレベータでの移動
 */
//-----------------------------------------------------------------------------
static BOOL IsMoveElevator( MAP_CHAGE_EVENT_WORK *p_wk )
{
  if( (p_wk->map_change_type == MAP_CHAGE_TYPE_FOOR) || (p_wk->warp_dir == WARP_INFORMATION) )
  {
    return TRUE;
  }
  return FALSE;
}

#ifdef PM_DEBUG

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナー配置タイプ上書き
 *
 *	@param	*p_room_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void INF_DEBUG_OverwritRoomData( ROOM_DATA *p_room_data )
{
  int i;
  // 全ての部屋タイプを固定する
  for( i = 0 ; i < ROOM_MAX ; i++ )
  {
    if( INF_TR_ROOM_POS_TYPE_NONE > g_debug_inf_tr_pos_type )
    {
      // 部屋タイプの取得
      ROOM_DATA  *p_rd = &p_room_data[i];
      p_rd->tr_pos_type = g_debug_inf_tr_pos_type;
      p_rd->room_type = GetRoomType(g_debug_inf_tr_pos_type);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フロアデータのデバッグ出力
 *
 *	@param	*p_room_data 
 *	@param	p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void INF_DEBUG_OutPutFloorData( ROOM_DATA *p_room_data ,INFINITYDUNGEON_SAVE *p_save )
{
  int i,j,k;
  int draw;

  for( i = 0; i<ROOM_MAX_Y; i++ )
  {
    for( draw = 0 ; draw <= 4; draw++ )
    {
      for( j = 0; j<ROOM_MAX_X; j++ )
      {
        int room_id = i*ROOM_MAX_Y + j;
        switch( draw )
        {
          // 上の列
        case 0:
          {
            OS_TFPrintf( 3,"■■");
            if( p_room_data[room_id].can_move[INF_MOVE_UP] == TRUE )
            {
              OS_TFPrintf( 3,"　");
            }
            else
            {
              OS_TFPrintf( 3,"■");
            }

            OS_TFPrintf( 3,"■■");
          }
          break;
        case 1:
          {
            if( p_room_data[room_id].room_type == ROOM_TYPE_BOSS)
            {
              OS_TFPrintf( 3,"■Ｂ　　■");
            }
            else if( p_room_data[room_id].room_type == ROOM_TYPE_ELEVATOR)
            {
              OS_TFPrintf( 3,"■Ｅ　　■");
            }
            else
            {
              OS_TFPrintf( 3,"■　　　■");
            }
          }
          break;
          // 真ん中の列
        case 2:
          {
            u8 now_room_id = INFINITYDUNGEON_SAVE_GetRoomID( p_save );
            if( p_room_data[room_id].can_move[INF_MOVE_LEFT] == TRUE )
            {
              OS_TFPrintf( 3,"　");
            }
            else
            {
              OS_TFPrintf( 3,"■");
            }

            // 現在地表示
            if( room_id == now_room_id )
            {
              OS_TFPrintf( 3,"　主　"); //部屋の中心
            }
            else
            {
              OS_TFPrintf( 3,"　　　"); //部屋の中心
            }

            if( p_room_data[room_id].can_move[INF_MOVE_RIGHT] == TRUE )
            {
              OS_TFPrintf( 3,"　");
            }
            else
            {
              OS_TFPrintf( 3,"■");
            }
          }
          break;
        case 3:
          {
            if( GetMemberNum(p_room_data[room_id].tr_pos_type) > 0 )
            {
              OS_TFPrintf( 3,"■%2d　%2d■",room_id,p_room_data[room_id].tr_pos_type);
            }
            else
            {
              OS_TFPrintf( 3,"■%2d　×■",room_id,p_room_data[room_id].tr_pos_type);
            }
          }
          break;
          // 下の列
        case 4:
          {
            OS_TFPrintf( 3,"■■");
            if( p_room_data[room_id].can_move[INF_MOVE_DOWN] == TRUE )
            {
              OS_TFPrintf( 3,"　");
            }
            else
            {
              OS_TFPrintf( 3,"■");
            }

            OS_TFPrintf( 3,"■■");
          }
          break;
        }
      }
      OS_TFPrintf( 3,"\n");
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン生成テスト
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void DEBUG_INFINITY_DUNGEON_MakeDungeonData()
{
  int i,n;
  s32 seed;
  s32 area;
  INF_DUNGEON_DATA *p_dungeon_data;
  
  // 生成テスト
  seed = GFUser_GetPublicRand( GFL_STD_RAND_MAX );
  OS_TFPrintf(3,"Seed = %u--------------\n",seed);
  for( i = 3 ; i <= 4; i++ )
  {
    for( n = 0 ; n < INFINITY_DUNGEON_AREA_ID_MAX; n++ )
    {
      p_dungeon_data = GFL_HEAP_AllocMemoryLo( HEAPID_FIELDMAP ,sizeof( INF_DUNGEON_DATA ) );
      SetDungeonData( p_dungeon_data ,n ,seed ,i ,i);
      SetTrainerData( p_dungeon_data ,n ,seed );
      GFL_HEAP_FreeMemory( p_dungeon_data );
    }
  }
}
#endif
