//======================================================================
/**
 * @file	fldmmdl.h
 * @brief	動作モデル
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#ifndef MMDL_H_FILE
#define MMDL_H_FILE

#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "include/field/field_dir.h"

#include "field_g3dobj.h"

#include "field_g3d_mapper.h"
#include "field_nogrid_mapper.h"
#include "eventwork.h"
#include "map_attr.h"

#include "arc/fieldmap/fldmmdl_objcode.h"
#include "fldmmdl_code.h"

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
//	debug
//--------------------------------------------------------------
#define MMDL_PL_NULL	//定義でプラチナで行っていた処理無効化
//#define MMDL_BLACT_HEAD3_TEST //定義で三頭身テスト
#define DEBUG_MMDL	//定義でデバッグ機能有効
//#define DEBUG_MMDL_FRAME_60 //定義で1/60フレームで動く事を想定

#ifdef DEBUG_MMDL
#define DEBUG_MMDL_PRINT //定義でデバッグ出力有効

#ifdef DEBUG_ONLY_FOR_kagaya
#define DEBUG_MMDL_DEVELOP ///<作業者のみ有効なデバッグ処理
#endif


#define DEBUG_MMDL_RESOURCE_MEMORY_SIZE // ONでメモリサイズを計算

#endif

#define DEBUG_OBJCODE_STR_LENGTH (16) //動作モデルOBJコード文字列長さ

//#define MMDL_BBD_DRAW_OFFS_Z_USE //有効でビルボード表示位置Z補正有り

//--------------------------------------------------------------
//	fx
//--------------------------------------------------------------
#define NUM_FX32(a)	(FX32_ONE*(a))	///<整数->FX32型に
#define FX32_NUM(a)	((a)/FX32_ONE)	///<FX32->整数に
#define NUM_FX16(a)	(FX16_ONE*(a))	///<整数->FX16型に
#define FX16_NUM(a)	((a)/FX16_ONE)	///<FX16->整数に

//--------------------------------------------------------------
//	グリッド
//--------------------------------------------------------------
#define GRID_ONE	(1)	///<1グリッド
#define GRID		(16)///<1グリッド単位。実座標16
#define GRID_HALF	(GRID>>1)///<1/2グリッド
#define GRID_SHIFT	(4)///<グリッド実座標変換に必要なシフト数
#define GRID_SIZE(a) ((a)<<GRID_SHIFT)///<グリッド実サイズ変換
#define SIZE_GRID(a) ((a)>>GRID_SHIFT)///<実サイズグリッド変換
#define GRID_SIZE_FX32(a) (GRID_SIZE(a)*FX32_ONE)///<グリッド実サイズFX32変換
#define SIZE_GRID_FX32(a) (SIZE_GRID(a)/FX32_ONE)///<実サイズグリッドFX32変換
#define GRID_FX32 (GRID*FX32_ONE) ///<1グリッド実数
#define GRID_HALF_FX32 (GRID_FX32>>1) ///<1/2グリッド実数

//--------------------------------------------------------------
//	高さグリッド
//--------------------------------------------------------------
#if 0
#define H_GRID_ONE	(1)		///<1グリッド
#define H_GRID		(8)		///<1グリッド単位。実座標8
#define H_GRID_2	(H_GRID*2)

#define H_GRID_HALF	(H_GRID>>1)	///<1/2グリッド
#define H_GRID_SHIFT	(3)		///<グリッド実座標変換に必要なシフト数
#define H_GRID_SIZE(a) ((a)<<H_GRID_SHIFT) ///<グリッド実サイズ変換
#define SIZE_H_GRID(a) ((a)>>H_GRID_SHIFT) ///<実サイズグリッド変換
#define H_GRID_SIZE_FX32(a) (H_GRID_SIZE(a)*FX32_ONE) ///<グリッド実サイズFX32変換
#define SIZE_H_GRID_FX32(a) (SIZE_H_GRID(a)/FX32_ONE) ///<実サイズグリッドFX32変換
#define H_GRID_FX32 (H_GRID*FX32_ONE) ///<1グリッド実数

#define H_GRID_FELLOW_SIZE (H_GRID_ONE*2) ///<OBJ同士の当たり判定　高さヒットサイズ
#define H_GRID_G_GRID(a) ((a)/2) ///<高さグリッド->地面グリッドサイズに
#define G_GRID_H_GRID(a) ((a)*2) ///<地面グリッド->高さグリッドサイズに
#endif

///<OBJ同士の当たり判定　高さヒットサイズ
#define H_GRID_FELLOW_SIZE (GRID_ONE)

//--------------------------------------------------------------
//	移動制限
//--------------------------------------------------------------
//#define MOVE_LIMIT_NOT (0xffff) ///<移動制限無し>セーブデータs8の為、変更
#define	MOVE_LIMIT_NOT (-1) ///<移動制限無し

//--------------------------------------------------------------
//	速度
//--------------------------------------------------------------
#define FRAME_1	(1)			///<1フレーム
#define FRAME_1_SYNC (2)	///<1フレームのVシンク数
#define GRID_FRAME_1_FX32 (FX32_ONE/FRAME_1_SYNC) ///<グリッド1frame移動量

#define GRID_FRAME_32 (32)
#define GRID_FRAME_16 (16)
#define GRID_FRAME_8 (8)
#define GRID_FRAME_4 (4)
#define GRID_FRAME_2 (2)
#define GRID_FRAME_1 (1)

#define GRID_VALUE_SPEED_32 (GRID_FX32/GRID_FRAME_32)
#define GRID_VALUE_SPEED_16 (GRID_FX32/GRID_FRAME_16)
#define GRID_VALUE_SPEED_8 (GRID_FX32/GRID_FRAME_8)
#define GRID_VALUE_SPEED_4 (GRID_FX32/GRID_FRAME_4)
#define GRID_VALUE_SPEED_2 (GRID_FX32/GRID_FRAME_2)
#define GRID_VALUE_SPEED_1 (GRID_FX32/GRID_FRAME_1)

#define GRID_VALUE_SPEED_FRAME(g,f) ((GRID_FX32*g)/f)

//--------------------------------------------------------------
//	ビルボードアニメーションフレーム
//--------------------------------------------------------------
#define BLACTANM_FRAME_1 (FX32_ONE) ///<1フレーム速度
#define BLACTANM_FRAME_2 (BLACTANM_FRAME_1*2) ///<2フレーム速度
#define BLACTANM_FRAME_3 (BLACTANM_FRAME_1*3) ///<2フレーム速度
#define BLACTANM_FRAME_4 (BLACTANM_FRAME_1*4) ///<4フレーム速度
#define BLACTANM_FRAME_8 (BLACTANM_FRAME_1*8) ///<8フレーム速度
#define BLACTANM_FRAME_05 (BLACTANM_FRAME_1/2) ///<1/2フレーム速度
#define BLACTANM_STEP_FRAME (8) ///<一歩移動アニメのフレーム数
#define BLACTANM_STEP_DASH_FRAME (4) ///<ダッシュ一歩移動アニメフレーム数

//--------------------------------------------------------------
//	座標補整
//--------------------------------------------------------------
#define MMDL_VEC_X_GRID_OFFS_FX32 (GRID_HALF_FX32)///<X方向グリッド補整
#define MMDL_VEC_Y_GRID_OFFS_FX32 (0)///<Y方向グリッド補整
#define MMDL_VEC_Z_GRID_OFFS_FX32 (GRID_HALF_FX32)///<Z方向グリッド補整

//--------------------------------------------------------------
//	表示座標補整
//--------------------------------------------------------------
#define MMDL_BLACT_X_GROUND_OFFS_FX32 (0) ///<X方向地面補整
#define MMDL_BLACT_Y_GROUND_OFFS_FX32 (0) ///<Y方向地面補整
#define MMDL_BLACT_Z_GROUND_OFFS_FX32 (FX32_ONE*6) ///<Z方向地面補整

//--------------------------------------------------------------
///	MMDLSYS_STABIT MMDLSYSステータスビット
//--------------------------------------------------------------
typedef enum
{
	MMDLSYS_STABIT_NON = (0),///<ステータス無し
	MMDLSYS_STABIT_DRAW_INIT_COMP = (1<<0),///<描画初期化完了
	MMDLSYS_STABIT_MOVE_PROC_STOP = (1<<1),///<動作処理停止
	MMDLSYS_STABIT_DRAW_PROC_STOP = (1<<2),///<描画処理停止
	MMDLSYS_STABIT_SHADOW_JOIN_NOT = (1<<3),///<影を付けない
	MMDLSYS_STABIT_MOVE_INIT_COMP = (1<<4),///<動作処理初期化完了
}MMDLSYS_STABIT;

//--------------------------------------------------------------
///	MMDL_STABIT MMDLステータスビット
//--------------------------------------------------------------
typedef enum
{
	MMDL_STABIT_NON=(0),///<ステータス無し
	MMDL_STABIT_USE=(1<<0),///<使用中を表す
	MMDL_STABIT_MOVE=(1<<1),///<移動中を表す
	MMDL_STABIT_MOVE_START=(1<<2),///<移動開始を表す
	MMDL_STABIT_MOVE_END=(1<<3),///<移動終了を表す
	MMDL_STABIT_ACMD=(1<<4),///<アニメーションコマンド中
	MMDL_STABIT_ACMD_END=(1<<5),///<アニメーションコマンド終了
	MMDL_STABIT_PAUSE_MOVE=(1<<6),///<移動一時停止
	MMDL_STABIT_PAUSE_DIR=(1<<7),///<表示方向一時停止
	MMDL_STABIT_PAUSE_ANM=(1<<8),///<アニメ一時停止
	MMDL_STABIT_VANISH=(1<<9),///<非表示
	MMDL_STABIT_ZONE_DEL_NOT=(1<<10),///<ゾーン切り替えによる削除禁止
	MMDL_STABIT_ATTR_GET_ERROR=(1<<11),///<アトリビュート取得失敗
	MMDL_STABIT_HEIGHT_GET_ERROR=(1<<12),///<高さ取得失敗
	MMDL_STABIT_HEIGHT_VANISH_OFF=(1<<13),///<高さ取得不可時の非表示をOFF
	MMDL_STABIT_DRAW_PROC_INIT_COMP=(1<<14),///<描画初期化完了
	MMDL_STABIT_SHADOW_SET=(1<<15),///<影セット済み
	MMDL_STABIT_JUMP_START=(1<<16),///<ジャンプ開始
	MMDL_STABIT_JUMP_END=(1<<17),///<ジャンプ終了
	MMDL_STABIT_FELLOW_HIT_NON=(1<<18),///<OBJ同士の当たり判定無効
	MMDL_STABIT_TALK_OFF=(1<<19),///<話しかけ無効
	MMDL_STABIT_SHADOW_VANISH=(1<<20),///<影表示、非表示
	MMDL_STABIT_DRAW_PUSH=(1<<21),///<描画処理を退避
	MMDL_STABIT_BLACT_ADD_PRAC=(1<<22),///<ビルボードアクター追加中
	MMDL_STABIT_HEIGHT_GET_OFF=(1<<23),///<高さ取得をしない
	MMDL_STABIT_REFLECT_SET=(1<<24),///<映り込みをセットした
	MMDL_STABIT_ALIES=(1<<25),///<エイリアスである
	MMDL_STABIT_EFFSET_SHOAL=(1<<26),///<浅瀬エフェクトをセット
	MMDL_STABIT_ATTR_OFFS_OFF=(1<<27),///<アトリビュートオフセット設定OFF
	MMDL_STABIT_BRIDGE=(1<<28),///<橋移動中である
	MMDL_STABIT_HEIGHT_EXPAND=(1<<29),///<拡張高さに反応する
	MMDL_STABIT_RAIL_MOVE = (1<<30),///<レール動作
}MMDL_STABIT;

//--------------------------------------------------------------
///	MMDL_MOVEBIT MMDL動作ビット
//--------------------------------------------------------------
typedef enum
{
	MMDL_MOVEBIT_NON=(0),///<無し
	MMDL_MOVEBIT_SHADOW_SET=(1<<0),///<影をセットした
	MMDL_MOVEBIT_GRASS_SET=(1<<1),///<草をセットした
	MMDL_MOVEBIT_ATTR_GET_OFF=(1<<2),///<アトリビュート取得を一切行わない
	MMDL_MOVEBIT_MOVEPROC_INIT=(1<<3),///<動作初期化を行った
	MMDL_MOVEBIT_NEED_MOVEPROC_RECOVER=(1<<4),///<動作復帰関数を呼ぶ必要
}MMDL_MOVEBIT;

//--------------------------------------------------------------
///	MMDL_MOVEHITBIT MMDL移動ヒットビット
//--------------------------------------------------------------
typedef enum
{
	MMDL_MOVEHITBIT_NON=(0),///<ヒット無し
	MMDL_MOVEHITBIT_LIM=(1<<0),///<移動制限によるヒット
	MMDL_MOVEHITBIT_ATTR=(1<<1),///<アトリビュートヒット
	MMDL_MOVEHITBIT_OBJ=(1<<2),///<OBJ同士の衝突
	MMDL_MOVEHITBIT_HEIGHT=(1<<3),///<高低差によるヒット
	MMDL_MOVEHITBIT_OUTRANGE=(1<<4),///<範囲外ヒット
  
  MMDL_MOVEHITBIT_BITMAX = (5), ///<使用しているビット最大
}MMDL_MOVEHITBIT;

//--------------------------------------------------------------
///	フィールド動作モデルヘッダー引数識別
//--------------------------------------------------------------
typedef enum
{
	MMDL_PARAM_0 = 0,
	MMDL_PARAM_1,
	MMDL_PARAM_2,
}MMDL_H_PARAM;

//--------------------------------------------------------------
///	描画タイプ
//--------------------------------------------------------------
typedef enum
{
	MMDL_DRAWTYPE_NON = 0,		///<描画無し
	MMDL_DRAWTYPE_BLACT,			///<ビルボード描画
	MMDL_DRAWTYPE_MDL,			///<モデリング描画
}MMDL_DRAWTYPE;

//--------------------------------------------------------------
///	影種類
//--------------------------------------------------------------
typedef enum
{
	MMDL_SHADOW_NON,			///<影無し
	MMDL_SHADOW_ON,				///<影アリ
}MMDL_SHADOWTYPE;

//--------------------------------------------------------------
///	足跡種類
//--------------------------------------------------------------
typedef enum
{
	MMDL_FOOTMARK_NON = 0,		///<足跡無し
	MMDL_FOOTMARK_NORMAL,		///<２本足
	MMDL_FOOTMARK_CYCLE,			///<自転車
}MMDL_FOOTMARKTYPE;

//--------------------------------------------------------------
///	映りこみ種類
//--------------------------------------------------------------
typedef enum
{
	MMDL_REFLECT_NON = 0,		///<映りこみ無し
	MMDL_REFLECT_BLACT,			///<ビルボード映りこみ
}MMDL_REFLECTTYPE;

//--------------------------------------------------------------
///	ビルボードアクターアニメーション種類
//--------------------------------------------------------------
typedef enum
{
	MMDL_BLACT_ANMTBLNO_NON,		///<アニメ無し
	MMDL_BLACT_ANMTBLNO_HERO,	///<自機専用
	MMDL_BLACT_ANMTBLNO_BLACT,	///<汎用アニメ
  MMDL_BLACT_ANMTBLNO_CYCLEHERO_OLD, ///<自転車自機専用 旧
  MMDL_BLACT_ANMTBLNO_SWIMHERO, ///<波乗り自機専用
  MMDL_BLACT_ANMTBLNO_FLIP, ///<人物汎用 フリップ使用
  MMDL_BLACT_ANMTBLNO_NONFLIP, ///<人物汎用 フリップ未使用
  MMDL_BLACT_ANMTBLNO_CYCLEHERO, ///<自転車自機専用
  MMDL_BLACT_ANMTBLNO_ONECELL, ///<一パターンのみ
  MMDL_BLACT_ANMTBLNO_ITEMGET, ///<自機アイテムゲット
  MMDL_BLACT_ANMTBLNO_PCAZUKE, ///<自機ポケセン預け
  MMDL_BLACT_ANMTBLNO_SAVE, ///<自機レポート
  MMDL_BLACT_ANMTBLNO_PCWOMAN, ///<PC姉
  MMDL_BLACT_ANMTBLNO_POKE_FLIP, ///<ポケモン　反転使用
  MMDL_BLACT_ANMTBLNO_POKE_NONFLIP, ///<ポケモン反転未使用
  MMDL_BLACT_ANMTBLNO_FISHINGHERO, ///<釣り自機 new
  MMDL_BLACT_ANMTBLNO_CUTINHERO, ///<カットイン自機 new
  MMDL_BLACT_ANMTBLNO_YUREHERO, ///<尾瀬揺れ自機 new
	MMDL_BLACT_ANMTBLNO_MAX,		///<最大
}MMDL_BLACT_ANMTBLNO;

//--------------------------------------------------------------
/// ビルボードアクターモデルサイズ
//--------------------------------------------------------------
typedef enum
{
  MMDL_BLACT_MDLSIZE_32x32, //32x32
  MMDL_BLACT_MDLSIZE_16x16, //16x16
  MMDL_BLACT_MDLSIZE_64x64, //64x64
  MMDL_BLACT_MDLSIZE_MAX,
}MMDL_BLACT_MDLSIZE;

//--------------------------------------------------------------
///	描画関数番号
//--------------------------------------------------------------
typedef enum
{
	MMDL_DRAWPROCNO_NON,		///<描画無し
	MMDL_DRAWPROCNO_HERO,	///<自機専用
	MMDL_DRAWPROCNO_BLACT,	///<ビルボード汎用
  MMDL_DRAWPROCNO_CYCLEHERO, ///<自転車自機専用
  MMDL_DRAWPROCNO_SWIMHERO, ///<波乗り自機専用
  MMDL_DRAWPROCNO_BLACTALWAYSANIME, //ビルボード汎用＋常にアニメ
  MMDL_DRAWPROCNO_ITEMGET, ///<自機アイテムゲット
  MMDL_DRAWPROCNO_PCAZUKE, ///<自機PC預け
  MMDL_DRAWPROCNO_ONEANMLOOP, ///<１パターンアニメループ
//  MMDL_DRAWPROCNO_ONEANM, ///<１パターンアニメ
  MMDL_DRAWPROCNO_PCWOMAN, ///<PC姉
  MMDL_DRAWPROCNO_TPOKE, ///<連れ歩きポケモン
  MMDL_DRAWPROCNO_TPOKE_FLY, ///<連れ歩きポケモン 縦揺れ付き
  MMDL_DRAWPROCNO_MODEL, ///<ポリゴンモデル
  MMDL_DRAWPROCNO_FISHINGHERO, ///<釣り自機専用 new
  MMDL_DRAWPROCNO_YUREHERO, ///<尾瀬揺れ自機 new
	MMDL_DRAWPROCNO_MAX,		///<最大
}MMDL_DRAWPROCNO;

//--------------------------------------------------------------
///	フィールド動作モデル動作プライオリティオフセット
//--------------------------------------------------------------
enum
{
	MMDL_TCBPRI_OFFS_ST = 32,		///<基本
	MMDL_TCBPRI_OFFS_PAIR,			///<ペア動作
	MMDL_TCBPRI_OFFS_AFTER,			///<フィールド動作モデルよりも後
};

//--------------------------------------------------------------
///	セーブ用ワーク要素数
//--------------------------------------------------------------
#define MMDL_SAVEMMDL_MAX (64)

//--------------------------------------------------------------
//	モデル最大数
//--------------------------------------------------------------
#define MMDL_MDL_MAX (48)

//--------------------------------------------------------------
/// ビルボードアクターID 無効なID
//--------------------------------------------------------------
#define MMDL_BLACTID_NULL (0xffff)

//--------------------------------------------------------------
/// OBJコード性別 外部パラメタと一致
//--------------------------------------------------------------
typedef enum
{
  MMDL_OBJCODESEX_MALE = 0, ///<男性
  MMDL_OBJCODESEX_FEMALE, ///<女性
  MMDL_OBJCODESEX_NON, ///<性別無し
  MMDL_OBJCODESEX_MAX, ///<最大
}MMDL_OBJCODESEX;

//--------------------------------------------------------------
/// MMDL_HEADER ポジションタイプ 
//--------------------------------------------------------------
typedef enum
{
  MMDL_HEADER_POSTYPE_GRID = 0, ///<グリッドポジションタイプ
  MMDL_HEADER_POSTYPE_RAIL,     ///<レールポジションタイプ

  MMDL_HEADER_POSTYPE_MAX, ///<最大
}MMDL_HEADER_POSTYPE;

//--------------------------------------------------------------
/// MMDL_HEADER ポジションバッファサイズ 
//--------------------------------------------------------------
#define MMDL_HEADER_POSBUF_SIZE ( 8 )

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
//	型定義
//--------------------------------------------------------------
///MMDLSYS
typedef struct _TAG_MMDLSYS MMDLSYS;
//typedef struct _TAG_MMDLSYS FLDMMDLSYS; ///<旧名
///MMDL
typedef struct _TAG_MMDL MMDL;
//typedef struct _TAG_MMDL FLDMMDL; ///<旧名
///MMDL_BLACTCONT
typedef struct _TAG_MMDL_BLACTCONT MMDL_BLACTCONT;
///MMDL_SAVEDATA
typedef struct _TAG_MMDL_SAVEDATA MMDL_SAVEDATA;
///MMDL_ROCKPOS
typedef struct _TAG_MMDL_ROCKPOS MMDL_ROCKPOS;
///MMDL_G3DOBJCONT
typedef struct _TAG_MMDL_G3DOBJCONT MMDL_G3DOBJCONT;

//--------------------------------------------------------------
///	アトリビュート型 元:pl map_attr.h 後に消します。
//--------------------------------------------------------------
typedef u32 MATR;

//--------------------------------------------------------------
///	関数定義
//--------------------------------------------------------------
typedef void (*MMDL_MOVE_PROC_INIT)(MMDL*);///<動作初期化関数
typedef void (*MMDL_MOVE_PROC)(MMDL*);///<動作関数
typedef void (*MMDL_MOVE_PROC_DEL)(MMDL*);///<動作削除関数
typedef void (*MMDL_MOVE_PROC_RECOVER)(MMDL*);///<動作復帰関数
typedef void (*MMDL_DRAW_PROC_INIT)(MMDL*);///<描画初期化関数
typedef void (*MMDL_DRAW_PROC)(MMDL*);///<描画関数
typedef void (*MMDL_DRAW_PROC_DEL)(MMDL*);///<描画削除関数
typedef void (*MMDL_DRAW_PROC_PUSH)(MMDL*);///<描画退避関数
typedef void (*MMDL_DRAW_PROC_POP)(MMDL*);///<描画復帰関数
typedef u32 (*MMDL_DRAW_PROC_GET)(MMDL*,u32);///<描画取得関数

//--------------------------------------------------------------
/// ビルボードアクター追加時の初期化関数
/// idx 追加時のアクターインデックス
/// init_work アクター追加時に指定した任意のワーク
//--------------------------------------------------------------
typedef void (*MMDL_BLACTCONT_ADDACT_USERPROC)(u16 idx,void *init_work);


//--------------------------------------------------------------
///	MMDL_HEADER構造体
//--------------------------------------------------------------
typedef struct
{
	unsigned short id;			///<識別ID
	unsigned short obj_code;	///<表示するOBJコード
	unsigned short move_code;	///<動作コード
	unsigned short event_type;	///<イベントタイプ
	unsigned short event_flag;	///<イベントフラグ
	unsigned short event_id;	///<イベントID
	short dir;					///<指定方向
	unsigned short param0;		///<指定パラメタ 0
	unsigned short param1;		///<指定パラメタ 1
	unsigned short param2;		///<指定パラメタ 2
	short move_limit_x;			///<X方向移動制限
	short move_limit_z;			///<Z方向移動制限

  u32 pos_type; ///<ポジションタイプ
  u8  pos_buf[ MMDL_HEADER_POSBUF_SIZE ]; ///<ポジションバッファ
  
}MMDL_HEADER;

typedef MMDL_HEADER FLDMMDL_HEADER; //旧名


//--------------------------------------------------------------
///	MMDL_GRIDPOS構造体
//--------------------------------------------------------------
typedef struct {
  s16 gx;			///<グリッドX
  s16 gy;			///<グリッドY
  s16 gz;			///<グリッドZ
} MMDL_GRIDPOS;

//--------------------------------------------------------------
///	MMDL_HEADER_GRIDPOS構造体
// pos_typeがMMDL_HEADER_POSTYPE_GRIDの場合の
// pos_bufの内容
//--------------------------------------------------------------
typedef struct {
  unsigned short gx;			///<グリッドX
  unsigned short gz;			///<グリッドZ
  int y;						///<Y値 fx32型
} MMDL_HEADER_GRIDPOS;

//--------------------------------------------------------------
///	MMDL_HEADER_RAILPOS構造体
// pos_typeがMMDL_HEADER_POSTYPE_RAILの場合の
// pos_bufの内容
//--------------------------------------------------------------
typedef struct {
  u16 rail_index;       ///<レールインデックス
  u16 front_grid;       ///<前方レールグリッド座標
  s16 side_grid;        ///<サイドレールグリッド座標
} MMDL_HEADER_RAILPOS;


//--------------------------------------------------------------
///	MMDL_MOVE_PROC_LIST構造体
//--------------------------------------------------------------
typedef struct
{
	u32 move_code;///<動作コード
	MMDL_MOVE_PROC_INIT init_proc;///<初期化関数
	MMDL_MOVE_PROC move_proc;///<動作関数
	MMDL_MOVE_PROC_DEL delete_proc;///<削除関数
	MMDL_MOVE_PROC_RECOVER recover_proc;///<動作復元関数
}MMDL_MOVE_PROC_LIST;

///MMDL_MOVE_PROC_LISTサイズ
#define MMDL_MOVE_PROC_LIST_SIZE (sizeof(MMDL_MOVE_PROC_LIST))

//--------------------------------------------------------------
///	MMDL_DRAW_PROC_LIST構造体
//--------------------------------------------------------------
typedef struct
{
	MMDL_DRAW_PROC_INIT init_proc;///<初期化関数
	MMDL_DRAW_PROC draw_proc;///<描画関数
	MMDL_DRAW_PROC_DEL delete_proc;///<削除関数
	MMDL_DRAW_PROC_PUSH push_proc;///<退避関数
	MMDL_DRAW_PROC_POP pop_proc;///<復帰関数
	MMDL_DRAW_PROC_GET get_proc;///<取得関数
}MMDL_DRAW_PROC_LIST;

///MMDL_DRAW_PROC_LISTサイズ
#define MMDL_DRAW_PROC_LIST_SIZE (sizeof(MMDL_DRAW_PROC_LIST))

//--------------------------------------------------------------
///	MMDL_RESMNARC 構造体
//--------------------------------------------------------------
typedef struct
{
	int id;
	int narc_id;
}MMDL_RESMNARC;

#define MMDL_RESMNARC_SIZE (sizeof(MMDL_RESMNARC))

//--------------------------------------------------------------
///	MMDL_ACMD_LIST構造体
//--------------------------------------------------------------
typedef struct
{
	u16 code;///<実行するアニメーションコマンドコード
	u16 num;///<code実行回数
}MMDL_ACMD_LIST;

#define MMDL_ACMD_LIST_SIZE (sizeof(MMDL_ACMD_LIST))

//--------------------------------------------------------------
///	OBJCODE_STATE構造体
//--------------------------------------------------------------
#ifndef MMDL_PL_NULL
typedef struct
{
	int code;				///<OBJコード
	u32 type_draw:4;		///<MMDL_DRAWTYPE
	u32 type_shadow:2;		///<MMDL_SHADOWTYPE
	u32 type_footmark:4;	///<MMDL_FOOTMARKTYPE
	u32 type_reflect:2;		///<MMDL_REFLECTTYPE
	u32 dmy:20;				///<bit余り
}OBJCODE_STATE;
#endif

//--------------------------------------------------------------
///	OBJCODE_PARAM構造体　外部データと一致
//	0-1 OBJコード
//	2-3 リソースアーカイブインデックス 
//	4	表示タイプ
//	5	処理関数
//	6	影表示
//	7	足跡種類
//	8	映り込み
//	9	モデルサイズ
//	10	テクスチャサイズ
//	11	アニメID
//	12 性別
//	13-15 4ビット差分ダミー
//--------------------------------------------------------------
typedef struct
{
	u16 code;			///<OBJコード
	u8 draw_type;		///<MMDL_DRAWTYPE
	u8 draw_proc_no;	///<MMDL_DRAWPROCNO
  
	u8 shadow_type;		///<MMDL_SHADOWTYPE
	u8 footmark_type;	///<MMDL_FOOTMARKTYPE
	u8 reflect_type;	///<MMDL_REFLECTTYPE
	u8 mdl_size;		///<MMDL_BLACT_MDLSIZE
  
	u8 tex_size;		///<GFL_BBD_TEXSIZDEF... テクスチャサイズ
	u8 anm_id;			///<MMDL_BLACT_ANMTBLNO
  u8 sex;         ///<MMDL_OBJCODESEX
  u8 padding00;   ///<4byte境界ダミー
  
  u8 buf[16];     ///<データバッファ
}OBJCODE_PARAM;

typedef struct 
{
	u16 res_idx;		///<リソースインデックス
  u8 padding[14];
}OBJCODE_PARAM_BUF_BBD;

typedef struct
{
  u16 res_idx_mdl;
  u16 res_idx_tex;
  u16 res_idx_anm0;
  u16 res_idx_anm1;
  u16 res_idx_anm2;
  u8 padding[6];
}OBJCODE_PARAM_BUF_MDL;

///外部から生成されるOBJCODE_PARAM配列の要素数の格納サイズ
/// 0-3 OBJCODE_PARAM要素数
/// 4... 要素数分のOBJCODE_PARAM配列
#define OBJCODE_PARAM_TOTAL_NUMBER_SIZE (sizeof(u32))

//--------------------------------------------------------------
///	MMDL_BBDACT_ANMTBL構造体
//--------------------------------------------------------------
typedef struct
{
	const GFL_BBDACT_ANM * const *pAnmTbl;
	u32 anm_max;
}MMDL_BBDACT_ANMTBL;

//--------------------------------------------------------------
/// MMDL_CHECKSAME_DATA構造体
//--------------------------------------------------------------
typedef struct
{
  u16 id;
  u16 code;
  u16 zone_id;
  u16 dmy;
}MMDL_CHECKSAME_DATA;

//--------------------------------------------------------------
/// MMDL_BLACTWORK_USER
//--------------------------------------------------------------
typedef struct
{
  u16 flag; //アクターの状況を管理
  GFL_BBDACT_ACTUNIT_ID actID; //アクターID
}MMDL_BLACTWORK_USER;

//======================================================================
//	extern
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 動作モデルへのポインタ取得
 * @param gamedata GAMEDATAへのポインタ
 * @retval MMDLSYSへのポインタ
 *
 * 実態はgamesystem/game_data.cにあるが、アクセス制限のため
 * こちらに配置する
 */
//--------------------------------------------------------------
extern MMDLSYS * GAMEDATA_GetMMdlSys(GAMEDATA *gamedata);

//--------------------------------------------------------------
///	fldmmdl.c
//--------------------------------------------------------------
extern u32 MMDL_BUFFER_GetWorkSize( void );
extern void MMDL_BUFFER_InitBuffer( void *p );

extern MMDLSYS * MMDLSYS_CreateSystem(
    HEAPID heapID, u32 max, MMDL_ROCKPOS *rockpos );
extern void MMDLSYS_FreeSystem( MMDLSYS *fos );

extern void MMDLSYS_SetupProc( MMDLSYS *fos, HEAPID heapID,
    const FLDMAPPER *pG3DMapper, FLDNOGRID_MAPPER* pNOGRIDMapper );
extern void MMDLSYS_DeleteProc( MMDLSYS *fos );
extern void MMDLSYS_UpdateProc( MMDLSYS *fos );
extern void MMDLSYS_VBlankProc( MMDLSYS *fos );

extern void MMDLSYS_SetupDrawProc( MMDLSYS *fos, const u16 *angleYaw );

extern MMDL * MMDLSYS_AddMMdl(
	MMDLSYS *fos, const MMDL_HEADER *header, int zone_id );
extern MMDL * MMDLSYS_AddMMdlParam( MMDLSYS *fos,
    s16 gx, s16 gz, u16 dir,
    u16 id, u16 code, u16 move, int zone_id );
extern void MMDLSYS_SetMMdl( MMDLSYS *fos,
	const MMDL_HEADER *header, int zone_id, int count, EVENTWORK *eventWork );
extern MMDL * MMDLSYS_AddMMdlHeaderID( MMDLSYS *fos,
	const MMDL_HEADER *header, int zone_id, int count, EVENTWORK *eventWork,
  u16 objID );
extern void MMDL_Delete( MMDL * mmdl );
extern void MMDL_DeleteEvent( MMDL * mmdl, EVENTWORK *evwork );
extern void MMDLSYS_DeleteMMdl( const MMDLSYS *fos );


extern void MMDLSYS_Push( MMDLSYS *mmdlsys );
extern void MMDLSYS_Pop( MMDLSYS *mmdlsys );

extern u32 MMDL_SAVEDATA_GetWorkSize( void );
extern void MMDL_SAVEDATA_Init( void *p );
extern void MMDL_SAVEDATA_Save(
	MMDLSYS *mmdlsys, MMDL_SAVEDATA *savedata );
extern void MMDL_SAVEDATA_Load(
	MMDLSYS *mmdlsys, MMDL_SAVEDATA *savedata );

extern void MMDLSYS_Push( MMDLSYS *mmdlsys );
extern void MMDLSYS_Pop( MMDLSYS *mmdlsys );

extern void MMDL_UpdateMoveProc( MMDL *mmdl );

extern BOOL MMDL_CheckPossibleAcmd( const MMDL * mmdl );
extern void MMDL_SetAcmd( MMDL * mmdl, u16 code );
extern void MMDL_SetLocalAcmd( MMDL * mmdl, u16 code );
extern BOOL MMDL_CheckEndAcmd( const MMDL * mmdl );
extern BOOL MMDL_EndAcmd( MMDL * mmdl );
extern void MMDL_FreeAcmd( MMDL * mmdl );

extern u32 MMDLSYS_CheckStatusBit(
	const MMDLSYS *mmdlsys, MMDLSYS_STABIT bit );
extern u16 MMDLSYS_GetMMdlMax( const MMDLSYS *mmdlsys );
extern u16 MMDLSYS_GetMMdlCount( const MMDLSYS *mmdlsys );
extern u16 MMDLSYS_GetTCBPriority( const MMDLSYS *mmdlsys );
extern HEAPID MMDLSYS_GetHeapID( const MMDLSYS *mmdlsys );
extern const MMDL * MMDLSYS_GetMMdlBuffer(
		const MMDLSYS *mmdlsys );
extern void MMDLSYS_SetArcHandle(
		MMDLSYS *mmdlsys, ARCHANDLE *handle );
extern ARCHANDLE * MMDLSYS_GetArcHandle( MMDLSYS *mmdlsys );
extern GFL_TCBSYS * MMDLSYS_GetTCBSYS( MMDLSYS *fos );
extern ARCHANDLE * MMDLSYS_GetResArcHandle( MMDLSYS *fos );
extern void MMDLSYS_SetBlActCont(
		MMDLSYS *mmdlsys, MMDL_BLACTCONT *pBlActCont );
extern MMDL_BLACTCONT * MMDLSYS_GetBlActCont( MMDLSYS *mmdlsys );
extern void MMDLSYS_SetG3dObjCont(
	MMDLSYS *mmdlsys, MMDL_G3DOBJCONT *objcont );
extern MMDL_G3DOBJCONT * MMDLSYS_GetG3dObjCont( MMDLSYS *mmdlsys );

extern const FLDMAPPER * MMDLSYS_GetG3DMapper( const MMDLSYS *fos );
extern FLDNOGRID_MAPPER * MMDLSYS_GetNOGRIDMapper( const MMDLSYS *fos );
extern void MMDLSYS_SetFieldMapWork(
    MMDLSYS *fos, void *fieldMapWork );
extern void * MMDLSYS_GetFieldMapWork( MMDLSYS *fos );
extern u16 MMDLSYS_GetTargetCameraAngleYaw( const MMDLSYS *mmdlsys );

extern void MMDL_OnStatusBit( MMDL *mmdl, MMDL_STABIT bit );
extern void MMDL_OffStatusBit( MMDL *mmdl, MMDL_STABIT bit );
extern MMDL_STABIT MMDL_GetStatusBit( const MMDL * mmdl );
extern u32 MMDL_CheckStatusBit(const MMDL *mmdl, MMDL_STABIT bit);
extern MMDL_MOVEBIT MMDL_GetMoveBit( const MMDL * mmdl );
extern void MMDL_OnMoveBit( MMDL * mmdl, MMDL_MOVEBIT bit );
extern void MMDL_OffMoveBit( MMDL * mmdl, MMDL_MOVEBIT bit );
extern u32 MMDL_CheckMoveBit(const MMDL *mmdl,MMDL_MOVEBIT bit);
extern void MMDL_SetOBJID( MMDL * mmdl, u16 obj_id );
extern u16 MMDL_GetOBJID( const MMDL * mmdl );
extern void MMDL_SetZoneID( MMDL * mmdl, u16 zone_id );
extern u16 MMDL_GetZoneID( const MMDL * mmdl );
extern void MMDL_SetOBJCode( MMDL * mmdl, u16 code );
extern u16 MMDL_GetOBJCode( const MMDL * mmdl );
extern void MMDL_SetMoveCode( MMDL * mmdl, u16 code );
extern u16 MMDL_GetMoveCode( const MMDL * mmdl );
extern void MMDL_SetEventType( MMDL * mmdl, u16 type );
extern u16 MMDL_GetEventType( const MMDL * mmdl );
extern void MMDL_SetEventFlag( MMDL * mmdl, u16 flag );
extern u16 MMDL_GetEventFlag( const MMDL * mmdl );
extern void MMDL_SetEventID( MMDL * mmdl, u16 id );
extern u16 MMDL_GetEventID( const MMDL * mmdl );
extern BOOL MMDL_CheckAliesEventID( const MMDL * mmdl );
extern u32 MMDL_GetDirHeader( const MMDL * mmdl );
extern void MMDL_SetForceDirDisp( MMDL * mmdl, u16 dir );
extern void MMDL_SetDirDisp( MMDL * mmdl, u16 dir );
extern u16 MMDL_GetDirDisp( const MMDL * mmdl );
extern u16 MMDL_GetDirDispOld( const MMDL * mmdl );
extern void MMDL_SetDirMove( MMDL * mmdl, u16 dir );
extern u16 MMDL_GetDirMove( const MMDL * mmdl );
extern u16 MMDL_GetDirMoveOld( const MMDL * mmdl );
extern void MMDL_SetDirAll( MMDL * mmdl, u16 dir );
extern void MMDL_SetParam(MMDL *mmdl, u16 param, MMDL_H_PARAM no);
extern u16 MMDL_GetParam(const MMDL * mmdl, MMDL_H_PARAM param);
extern void MMDL_SetMoveLimitX( MMDL * mmdl, s16 x );
extern s16 MMDL_GetMoveLimitX( const MMDL * mmdl );
extern void MMDL_SetMoveLimitZ( MMDL * mmdl, s16 z );
extern s16 MMDL_GetMoveLimitZ( const MMDL * mmdl );
extern void MMDL_SetDrawStatus( MMDL * mmdl, u16 st );
extern u16 MMDL_GetDrawStatus( const MMDL * mmdl );
extern MMDLSYS * MMDL_GetMMdlSys( const MMDL * mmdl );
extern void * MMDL_InitMoveProcWork( MMDL * mmdl, int size );
extern void * MMDL_GetMoveProcWork( MMDL * mmdl );
extern void * MMDL_InitMoveSubProcWork( MMDL * mmdl, int size );
extern void * MMDL_GetMoveSubProcWork( MMDL * mmdl );
extern void * MMDL_InitMoveCmdWork( MMDL * mmdl, int size );
extern void * MMDL_GetMoveCmdWork( MMDL * mmdl );
extern void * MMDL_InitDrawProcWork( MMDL * mmdl, int size );
extern void * MMDL_GetDrawProcWork( MMDL * mmdl );
extern void MMDL_CallMoveInitProc( MMDL * mmdl );
extern void MMDL_CallMoveProc( MMDL * mmdl );
extern void MMDL_CallMoveDeleteProc( MMDL * mmdl );
extern void MMDL_CallMovePopProc( MMDL * mmdl );
extern void MMDL_CallDrawInitProc( MMDL * mmdl );
extern void MMDL_CallDrawProc( MMDL * mmdl );
extern void MMDL_CallDrawDeleteProc( MMDL * mmdl );
extern void MMDL_CallDrawPushProc( MMDL * mmdl );
extern void MMDL_CallDrawPopProc( MMDL * mmdl );
extern u32 MMDL_CallDrawGetProc( MMDL *mmdl, u32 state );
extern void MMDL_SetAcmdCode( MMDL * mmdl, u16 code );
extern u16 MMDL_GetAcmdCode( const MMDL * mmdl );
extern void MMDL_SetAcmdSeq( MMDL * mmdl, u16 no );
extern void MMDL_IncAcmdSeq( MMDL * mmdl );
extern u16 MMDL_GetAcmdSeq( const MMDL * mmdl );
extern void MMDL_SetMapAttr( MMDL * mmdl, u32 attr );
extern u32 MMDL_GetMapAttr( const MMDL * mmdl );
extern void MMDL_SetMapAttrOld( MMDL * mmdl, u32 attr );
extern u32 MMDL_GetMapAttrOld( const MMDL * mmdl );
extern u16 MMDL_GetZoneIDAlies( const MMDL * mmdl );
extern s16 MMDL_GetInitGridPosX( const MMDL * mmdl );
extern void MMDL_SetInitGridPosX( MMDL * mmdl, s16 x );
extern s16 MMDL_GetInitGridPosY( const MMDL * mmdl );
extern void MMDL_SetInitGridPosY( MMDL * mmdl, s16 y );
extern s16 MMDL_GetInitGridPosZ( const MMDL * mmdl );
extern void MMDL_SetInitGridPosZ( MMDL * mmdl, s16 z );
extern s16 MMDL_GetOldGridPosX( const MMDL * mmdl );
extern void MMDL_SetOldGridPosX( MMDL * mmdl, s16 x );
extern s16 MMDL_GetOldGridPosY( const MMDL * mmdl );
extern void MMDL_SetOldGridPosY( MMDL * mmdl, s16 y );
extern s16 MMDL_GetOldGridPosZ( const MMDL * mmdl );
extern void MMDL_SetOldGridPosZ( MMDL * mmdl, s16 z );
extern void MMDL_GetGridPos( const MMDL * mmdl, MMDL_GRIDPOS* pos );
extern s16 MMDL_GetGridPosX( const MMDL * mmdl );
extern void MMDL_SetGridPosX( MMDL * mmdl, s16 x );
extern void MMDL_AddGridPosX( MMDL * mmdl, s16 x );
extern s16 MMDL_GetGridPosY( const MMDL * mmdl );
extern void MMDL_SetGridPosY( MMDL * mmdl, s16 y );
extern void MMDL_AddGridPosY( MMDL * mmdl, s16 y );
extern s16 MMDL_GetGridPosZ( const MMDL * mmdl );
extern void MMDL_SetGridPosZ( MMDL * mmdl, s16 z );
extern void MMDL_AddGridPosZ( MMDL * mmdl, s16 z );
extern const VecFx32 * MMDL_GetVectorPosAddress( const MMDL * mmdl );
extern void MMDL_GetVectorPos( const MMDL * mmdl, VecFx32 *vec );
extern void MMDL_SetVectorPos( MMDL * mmdl, const VecFx32 *vec );
extern fx32 MMDL_GetVectorPosY( const MMDL * mmdl );
extern void MMDL_GetVectorDrawOffsetPos(
		const MMDL * mmdl, VecFx32 *vec );
extern void MMDL_SetVectorDrawOffsetPos(
		MMDL * mmdl, const VecFx32 *vec );
extern void MMDL_GetVectorOuterDrawOffsetPos(
		const MMDL * mmdl, VecFx32 *vec );
extern void MMDL_SetVectorOuterDrawOffsetPos(
		MMDL * mmdl, const VecFx32 *vec );
extern void MMDL_GetVectorAttrDrawOffsetPos(
		const MMDL * mmdl, VecFx32 *vec );
extern void MMDL_SetVectorAttrDrawOffsetPos(
		MMDL * mmdl, const VecFx32 *vec );
extern s16 MMDL_GetHeightGrid( const MMDL * mmdl );
extern MMDL_BLACTCONT * MMDL_GetBlActCont( MMDL *mmdl );

extern BOOL MMDLSYS_CheckCompleteDrawInit( const MMDLSYS *mmdlsys );
extern void MMDLSYS_SetCompleteDrawInit( MMDLSYS *mmdlsys, BOOL flag );
extern void MMDLSYS_SetJoinShadow( MMDLSYS *fos, BOOL flag );
extern BOOL MMDLSYS_CheckJoinShadow( const MMDLSYS *fos );

extern void MMDLSYS_StopProc( MMDLSYS *mmdlsys );
extern void MMDLSYS_PlayProc( MMDLSYS *mmdlsys );
extern void MMDLSYS_PauseMoveProc( MMDLSYS *mmdlsys );
extern void MMDLSYS_ClearPauseMoveProc( MMDLSYS *mmdlsys );
extern BOOL MMDLSYS_GetPauseMoveFlag( MMDLSYS* mmdlsys );

extern u32 MMDL_CheckMMdlSysStatusBit(
	const MMDL *mmdl, MMDLSYS_STABIT bit );
extern BOOL MMDL_CheckStatusBitUse( const MMDL *mmdl );
extern void MMDL_OnStatusBitMove( MMDL *mmdl );
extern void MMDL_OffStatusBitMove( MMDL * mmdl );
extern BOOL MMDL_CheckStatusBitMove( const MMDL *mmdl );
extern void MMDL_OnStatusBitMoveStart( MMDL * mmdl );
extern void MMDL_OffStatusBitMoveStart( MMDL * mmdl );
extern BOOL MMDL_CheckStatusBitMoveStart( const MMDL * mmdl );
extern void MMDL_OnStatusBitMoveEnd( MMDL * mmdl );
extern void MMDL_OffStatusBitMoveEnd( MMDL * mmdl );
extern BOOL MMDL_CheckStatusBitMoveEnd( const MMDL * mmdl );
extern void MMDL_OnStatusBitCompletedDrawInit( MMDL * mmdl );
extern void MMDL_OffStatusBitCompletedDrawInit( MMDL * mmdl );
extern BOOL MMDL_CheckStatusBitCompletedDrawInit(const MMDL * mmdl);
extern BOOL MMDL_CheckStatusBitVanish( const MMDL * mmdl );
extern void MMDL_SetStatusBitVanish( MMDL * mmdl, BOOL flag );
extern void MMDL_SetStatusBitFellowHit( MMDL * mmdl, BOOL flag );
extern void MMDL_SetStatusBitMove( MMDL * mmdl, int flag );
extern BOOL MMDL_CheckStatusBitTalk( MMDL * mmdl );
extern void MMDL_SetStatusBitTalkOFF( MMDL * mmdl, BOOL flag );
extern void MMDL_OnStatusBitMoveProcPause( MMDL * mmdl );
extern void MMDL_OffStatusBitMoveProcPause( MMDL * mmdl );
extern BOOL MMDL_CheckStatusBitMoveProcPause( const MMDL * mmdl );
extern BOOL MMDL_CheckCompletedDrawInit( const MMDL * mmdl );
extern void MMDL_SetStatusBitHeightGetOFF( MMDL * mmdl, BOOL flag );
extern BOOL MMDL_CheckStatusBitHeightGetOFF( const MMDL * mmdl );
extern void MMDL_SetStatusBitNotZoneDelete( MMDL * mmdl, BOOL flag );
extern void MMDL_SetStatusBitAlies( MMDL * mmdl, BOOL flag );
extern BOOL MMDL_CheckStatusBitAlies( const MMDL * mmdl );
extern void MMDL_SetStatusBitShoalEffect( MMDL * mmdl, BOOL flag );
extern BOOL MMDL_CheckStatusBitShoalEffect( const MMDL * mmdl );
extern void MMDL_SetStatusBitAttrOffsetOFF( MMDL * mmdl, BOOL flag );
extern BOOL MMDL_CheckStatusBitAttrOffsetOFF( const MMDL * mmdl );
extern void MMDL_SetStatusBitBridge( MMDL * mmdl, BOOL flag );
extern BOOL MMDL_CheckStatusBitBridge( const MMDL * mmdl );
extern void MMDL_SetStatusBitReflect( MMDL * mmdl, BOOL flag );
extern BOOL MMDL_CheckStatusBitReflect( const MMDL * mmdl );
extern BOOL MMDL_CheckStatusBitAcmd( const MMDL * mmdl );
extern void MMDL_SetStatusBitHeightExpand( MMDL * mmdl, BOOL flag );
extern BOOL MMDL_CheckStatusBitHeightExpand( const MMDL * mmdl );

extern void MMDL_SetMoveBitAttrGetOFF( MMDL * mmdl, BOOL flag );
extern int MMDL_CheckMoveBitAttrGetOFF( const MMDL * mmdl );

extern BOOL MMDLSYS_SearchUseMMdl(
	const MMDLSYS *fos, MMDL **mmdl, u32 *no );
extern MMDL * MMDLSYS_SearchGridPos(
	const MMDLSYS *sys, s16 x, s16 z, BOOL old_hit );
extern MMDL * MMDLSYS_SearchGridPosEx(
	const MMDLSYS *sys, s16 x, s16 z, fx32 height, fx32 y_diff, BOOL old_hit );
extern MMDL * MMDLSYS_SearchMoveCode(
	const MMDLSYS *fos, u16 mv_code );
extern MMDL * MMDLSYS_SearchOBJID( const MMDLSYS *fos, u16 id );
extern void MMDLSYS_DeleteZoneUpdateMMdl( MMDLSYS *fos );

extern BOOL MMDL_SearchUseOBJCode( const MMDL *mmdl, u16 code );
extern void MMDL_InitPosition(
	MMDL * mmdl, const VecFx32 *vec, u16 dir );
extern void MMDL_InitGridPosition(
    MMDL * mmdl, s16 gx, s16 gy, s16 gz, u16 dir );
extern void MMDL_ChangeMoveParam( MMDL *mmdl, const MMDL_HEADER *head );
extern void MMDL_ChangeOBJID( MMDL * mmdl, u16 id );
extern void MMDL_InitCheckSameData(
    const MMDL *mmdl, MMDL_CHECKSAME_DATA *outData );
extern BOOL MMDL_CheckSameData(
    const MMDL * mmdl, const MMDL_CHECKSAME_DATA *data );
extern BOOL MMDL_CheckSameDataIDOnly(
    const MMDL * mmdl, const MMDL_CHECKSAME_DATA *data );

extern const OBJCODE_PARAM * MMDLSYS_GetOBJCodeParam(
		const MMDLSYS *mmdlsys, u16 code );
extern const OBJCODE_PARAM * MMDL_GetOBJCodeParam(
		const MMDL *mmdl, u16 code );
extern const OBJCODE_PARAM_BUF_BBD * MMDL_GetOBJCodeParamBufBBD(
    const OBJCODE_PARAM *param );
extern const OBJCODE_PARAM_BUF_MDL * MMDL_GetOBJCodeParamBufMDL(
    const OBJCODE_PARAM *param );

extern void MMDL_MoveInitProcDummy( MMDL * mmdl );
extern void MMDL_MoveProcDummy( MMDL * mmdl );
extern void MMDL_MoveDeleteProcDummy( MMDL * mmdl );
extern void MMDL_MoveReturnProcDummy( MMDL * mmdl );

extern void MMDL_DrawInitProcDummy( MMDL * mmdl );
extern void MMDL_DrawProcDummy( MMDL * mmdl );
extern void MMDL_DrawDeleteProcDummy( MMDL * mmdl );
extern void MMDL_DrawPushProcDummy( MMDL * mmdl );
extern void MMDL_DrawPopProcDummy( MMDL * mmdl );

extern void MMDL_ChangeOBJCode( MMDL *mmdl, u16 code );

extern u32 MMDL_ROCKPOS_GetWorkSize( void );
extern void MMDL_ROCKPOS_Init( void *p );
extern void MMDL_ROCKPOS_SavePos( const MMDL *mmdl );
extern BOOL MMDLSYS_ROCKPOS_CheckRockFalled(
    const MMDLSYS *mmdlsys, const VecFx32 *pos );


extern void MMDLHEADER_SetGridPos( MMDL_HEADER* head, u16 gx, u16 gz, int y );
extern void MMDLHEADER_SetRailPos( MMDL_HEADER* head, u16 index, u16 front, u16 side );

#ifdef DEBUG_MMDL
extern u8 * DEBUG_MMDL_GetOBJCodeString( u16 code, HEAPID heapID );
#endif


//--------------------------------------------------------------
//	fldmmdl_movedata.c
//--------------------------------------------------------------
extern const MMDL_MOVE_PROC_LIST * const DATA_FieldOBJMoveProcListTbl[MV_CODE_MAX];
extern int (* const * const DATA_AcmdActionTbl[ACMD_MAX])( MMDL * );
extern int (* const * const DATA_RailAcmdActionTbl[ACMD_MAX])( MMDL * );
extern const int * const DATA_AcmdCodeDirChangeTbl[];

//--------------------------------------------------------------
//	fldmmdl_drawdata.c
//--------------------------------------------------------------
extern const MMDL_DRAW_PROC_LIST * const
	DATA_MMDL_DRAW_PROC_LIST_Tbl[MMDL_DRAWPROCNO_MAX];

extern const MMDL_BBDACT_ANMTBL
	DATA_MMDL_BBDACT_ANM_ListTable[MMDL_BLACT_ANMTBLNO_MAX];

extern const u16 DATA_MMDL_BLACT_MdlSize[MMDL_BLACT_MDLSIZE_MAX][2];

#ifndef MMDL_PL_NULL
extern const OBJCODE_STATE DATA_FieldOBJCodeDrawStateTbl[];
#endif

#ifndef MMDL_PL_NULL
extern const MMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Non;

extern const MMDL_RESMNARC DATA_MMDL_ResmNArcTbl_RenderOBJ[];
extern const int DATA_FIELDOBJ_RenderOBJMax;

extern const MMDL_RESMNARC DATA_MMDL_ResmNArcTbl_Tex[];
extern const MMDL_RESMNARC DATA_MMDL_ResmNArcTbl_Mdl[];
extern const MMDL_RESMNARC DATA_MMDL_ResmNArcTbl_Anm[];
extern const int DATA_MMDL_Regular_Tex[];
extern const int DATA_MMDL_Regular_Mdl[];
extern const int DATA_MMDL_Regular_Anm[];
extern const int DATA_MMDL_BlActFogEnableOFFTbl[];
#endif

//--------------------------------------------------------------
//	fldmmdl_move.c
//--------------------------------------------------------------
extern void MMDL_InitMoveProc( MMDL * mmdl );
extern void MMDL_UpdateMove( MMDL * mmdl );

extern u32 MMDL_HitCheckMove( const MMDL *mmdl,
	const VecFx32 *vec, s16 x, s16 y, s16 z, u16 dir );
extern u32 MMDL_HitCheckMoveCurrent(
	const MMDL * fmmdl, s16 x, s16 y, s16 z, u16 dir );
extern u32 MMDL_HitCheckMoveDir( const MMDL * mmdl, u16 dir );
extern int MMDL_HitCheckMoveFellow(
	const MMDL * mmdl, s16 x, s16 y, s16 z );
extern int MMDL_HitCheckMoveLimit(
	const MMDL * mmdl, s16 x, s16 z );
extern BOOL MMDL_GetMapPosAttr(
	const MMDL *mmdl, const VecFx32 *pos, u32 *attr );
extern BOOL MMDL_GetMapPosHeight(
	const MMDL *mmdl, const VecFx32 *pos, fx32 *height );

extern void MMDL_UpdateGridPosDir( MMDL * mmdl, u16 dir );
extern void MMDL_UpdateGridPosCurrent( MMDL * mmdl );
extern MAPATTR MMDL_GetMapDirAttr( MMDL * mmdl, u16 dir );

extern void MMDL_AddVectorPos( MMDL * mmdl, const VecFx32 *val );
extern void MMDL_AddVectorPosDir( MMDL * mmdl, u16 dir, fx32 val );
extern BOOL MMDL_UpdateCurrentHeight( MMDL * mmdl );
extern BOOL MMDL_UpdateCurrentMapAttr( MMDL * mmdl );

extern void MMDL_CallMoveProcAfterDrawInit( MMDL * mmdl );

extern s16 MMDL_TOOL_GetDirAddValueGridX( u16 dir );
extern s16 MMDL_TOOL_GetDirAddValueGridZ( u16 dir );
extern void MMDL_TOOL_AddDirVector( u16 dir, VecFx32 *vec, fx32 val );
extern void MMDL_TOOL_AddDirGrid( u16 dir, s16 *gx, s16 *gz, s16 val );
extern void MMDL_TOOL_GetCenterGridPos( s16 gx, s16 gz, VecFx32 *vec );
extern void MMDL_TOOL_VectorPosToGridPos(
	s16 *gx, s16 *gy, s16 *gz, const VecFx32 *vec );
extern void MMDL_TOOL_GridPosToVectorPos(
    const s16 gx, const s16 gy, const s16 gz, VecFx32 *pos );
extern u16 MMDL_TOOL_FlipDir( u16 dir );
extern u16 MMDL_TOOL_GetRangeDir( int ax, int az, int bx, int bz );
extern u16 MMDL_TOOL_GetAngleYawToDirFour( u16 dir, u16 angleYaw );

//--------------------------------------------------------------
//	fldmmdl_move_1.c
//--------------------------------------------------------------
extern void MMDL_MoveSubProcInit( MMDL * mmdl );
extern int MMDL_MoveSub( MMDL * mmdl );

//--------------------------------------------------------------
//	fldmmdl_move_2.c
//--------------------------------------------------------------
extern void MMDL_MoveHidePullOffFlagSet( MMDL * mmdl );

#ifndef MMDL_PL_NULL
extern void MMDL_MoveHideEoaPtrSet( MMDL * mmdl, EOA_PTR eoa );
extern EOA_PTR MMDL_MoveHideEoaPtrGet( MMDL * mmdl );
#endif

//--------------------------------------------------------------
//	fldmmdl_draw.c
//--------------------------------------------------------------
extern void MMDLSYS_InitDraw( MMDLSYS *fos );
extern void MMDLSYS_DeleteDraw( MMDLSYS *fos );

extern void MMDL_UpdateDraw( MMDL * mmdl );


#ifndef MMDL_PL_NULL
extern const OBJCODE_STATE * MMDL_TOOL_GetOBJCodeState( u16 code );
extern const OBJCODE_STATE * MMDL_GetOBJCodeState( const MMDL *mmdl );
#endif

extern BOOL MMDL_CheckDrawPause( const MMDL * mmdl );
extern void * MMDL_DrawArcDataAlloc(
		const MMDLSYS *fos, u32 datID, int fb );
extern void MMDL_GetDrawVectorPos(
		const MMDL * mmdl, VecFx32 *vec );

//--------------------------------------------------------------
//	fldmmdl_draw_0.c
//--------------------------------------------------------------
extern BOOL MMDL_DrawYureHero_SetAnimeFrame( MMDL *mmdl, u32 frame );

//--------------------------------------------------------------
//	fldmmdl_acmd.c
//--------------------------------------------------------------
extern GFL_TCB * MMDL_SetAcmdList(
	MMDL * mmdl, const MMDL_ACMD_LIST *list );
extern BOOL MMDL_CheckEndAcmdList( GFL_TCB * tcb );
extern void MMDL_EndAcmdList( GFL_TCB * tcb );

extern u16 MMDL_ChangeDirAcmdCode( u16 dir, u16 code );
extern u16 MMDL_GetAcmdDir( u16 code );

extern void MMDL_ActionAcmd( MMDL * mmdl );
extern BOOL MMDL_ActionLocalAcmd( MMDL * mmdl );

//--------------------------------------------------------------
///	fldmmdl_blact.c
//--------------------------------------------------------------
extern void MMDL_BLACTCONT_Setup( MMDLSYS *mmdlsys,
	GFL_BBDACT_SYS *pBbdActSys, int res_max );
extern void MMDL_BLACTCONT_Release( MMDLSYS *mmdlsys );
extern void MMDL_BLACTCONT_Update( MMDLSYS *mmdlsys );
extern void MMDL_BLACTCONT_ProcVBlank( MMDLSYS *mmdlsys );
extern void MMDL_BLACTCONT_SetGlobalScaleOne( MMDLSYS *mmdlsys );

extern void MMDL_BLACTCONT_AddResourceTex(
	MMDLSYS *mmdlsys, const u16 *code, int max );

extern BOOL MMDL_BLACTCONT_AddActor(
    MMDL *mmdl, u16 code, GFL_BBDACT_ACTUNIT_ID *outID );
extern void MMDL_BLACTCONT_DeleteActor( MMDL *mmdl, u32 actID );

extern BOOL MMDL_BLACTCONT_USER_AddActor( MMDLSYS *mmdlsys,
    u16 code, MMDL_BLACTWORK_USER *userAct, const VecFx32 *pos,
    MMDL_BLACTCONT_ADDACT_USERPROC init_proc, void *init_work );
extern void MMDL_BLACTCONT_USER_DeleteActor(
    MMDLSYS *mmdlsys, MMDL_BLACTWORK_USER *userAct );

extern GFL_BBDACT_SYS * MMDL_BLACTCONT_GetBbdActSys(
		MMDL_BLACTCONT *pBlActCont );
extern GFL_BBDACT_RESUNIT_ID MMDL_BLACTCONT_GetResUnitID(
		MMDL_BLACTCONT *pBlActCont );

extern BOOL MMDL_BLACTCONT_CheckOBJCodeRes(
		MMDLSYS *mmdlsys, u16 code );
extern BOOL MMDL_BLACTCONT_AddOBJCodeRes(
    MMDLSYS *mmdlsys, u16 code, BOOL trans, BOOL guest );
extern void MMDL_BLACTCONT_DeleteOBJCodeRes( MMDLSYS *mmdlsys, u16 code );

extern const MMDL_BBDACT_ANMTBL * MMDL_BLACTCONT_GetObjAnimeTable(
  const MMDLSYS *mmdlsys, u16 code );

#ifdef DEBUG_MMDL_RESOURCE_MEMORY_SIZE
extern u32 DEBUG_MMDL_GetUseResourceMemorySize( void );
#endif

//--------------------------------------------------------------
//	fldmmdl_railmove.c
//--------------------------------------------------------------
extern void MMDL_InitRailMoveProc( MMDL * mmdl );
extern void MMDL_UpdateRailMove( MMDL * mmdl );
extern MAPATTR MMDL_GetRailLocationAttr( const MMDL * mmdl, const RAIL_LOCATION* location );

extern u16 MMDL_RAIL_GetAngleYawToDirFour( MMDL * mmdl, u16 angleYaw );


//--------------------------------------------------------------
//	fldmmdl_railmove_0.c
//--------------------------------------------------------------
extern void MMDL_SetRailHeaderBefore( MMDL* mmdl, const MMDL_HEADER* head );
extern void MMDL_SetRailHeaderAfter( MMDL* mmdl, const MMDL_HEADER* head );

// ロケーション保存領域処理
// この処理は、MOVE_PROCが初期化されていないタイミングで、SetRailHeaderAfterが動作したときに、RailWorkの初期設定を渡すための処理です。
//動作コード用ワークを使用しています。
extern BOOL MMdl_CheckSetUpLocation( const MMDL* mmdl );
extern BOOL MMdl_GetSetUpLocation( const MMDL* mmdl, RAIL_LOCATION* location );
//-----------------------------------------------

// レールロケーションの取得
extern void MMDL_SetRailLocation( MMDL * fmmdl, const RAIL_LOCATION* location );
extern void MMDL_GetRailLocation( const MMDL * fmmdl, RAIL_LOCATION* location );
extern void MMDL_GetOldRailLocation( const MMDL * fmmdl, RAIL_LOCATION* location );
// DIR方向のロケーションの取得　BOOLで移動できるかチェックを行う。
// FALSEが帰ってきても、locationには、移動先のロケーションが入っている。
// その値は、イベントチェックなどで使用している。
extern BOOL MMDL_GetRailFrontLocation( const MMDL *mmdl, RAIL_LOCATION* location );
extern BOOL MMDL_GetRailDirLocation( const MMDL *mmdl, u16 dir, RAIL_LOCATION* location );
extern void MMDL_Rail_UpdateGridPosDir( MMDL *mmdl, u16 dir );

extern FIELD_RAIL_WORK* MMDL_GetRailWork( const MMDL * fmmdl );
extern BOOL MMDL_ReqRailMove( MMDL * fmmdl, u16 dir, s16 wait );
extern u32 MMDL_HitCheckRailMove( const MMDL *mmdl,
	const RAIL_LOCATION* now_location, const RAIL_LOCATION* next_location );
extern u32 MMDL_HitCheckRailMoveCurrent( const MMDL *mmdl, const RAIL_LOCATION* next_location );
extern u32 MMDL_HitCheckRailMoveDir( const MMDL *mmdl, u16 dir );
extern BOOL MMDL_HitCheckRailMoveFellow(
	const MMDL * mmdl, const RAIL_LOCATION* location );
extern MMDL * MMDLSYS_SearchRailLocation( const MMDLSYS *sys, const RAIL_LOCATION* location, BOOL old_hit );

// レール移動　今の前方方向
extern void MMDL_Rail_GetFrontWay( const MMDL *mmdl, VecFx16* way );
extern void MMDL_Rail_GetDirLineWay( const MMDL *mmdl, u16 dir, VecFx16* way );

//--------------------------------------------------------------
//	fldmmdl_g3dobj.c
//--------------------------------------------------------------
extern void MMDL_G3DOBJCONT_Setup(
    MMDLSYS *mmdlsys, FLD_G3DOBJ_CTRL *g3dobj_ctrl );
extern void MMDL_G3DOBJCONT_Delete( MMDLSYS *mmdlsys );
extern FLD_G3DOBJ_CTRL * MMDL_G3DOBJCONT_GetFldG3dObjCtrl( MMDL *mmdl );
extern FLD_G3DOBJ_OBJIDX MMDL_G3DOBJCONT_AddObject( MMDL *mmdl, u16 code );
extern void MMDL_G3DOBJCONT_DeleteObject(
    MMDL *mmdl, FLD_G3DOBJ_OBJIDX idx );

#endif //MMDL_H_FILE
