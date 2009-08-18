//==============================================================================
/**
 * @file	footprint_stamp.c
 * @brief	足跡ボードのスタンプ動作
 * @author	matsuda
 * @date	2008.01.20(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "net_app/wifi_lobby/wflby_system_def.h"
#include "footprint_common.h"
#include "footprint_board.naix"
#include "footprint_stamp.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/pokefoot.h"	//POKEFOOT_ARC_CHAR_DMMY定義の為
#include "net_app/footprint_main.h"
#include "system/wipe.h"
#include "system/wipe_def.h"

#include "print/wordset.h"
#include "footprint_tool.h"
#include "footprint_snd_def.h"
#include "arc_def.h"
#include "sound/pm_sndsys.h"
#include <calctool.h>


//==============================================================================
//	コントロールデータ
//==============================================================================
#include "footprint_stamp_control.dat"

//==============================================================================
//	定数定義
//==============================================================================
///連鎖管理のワーク番号を取得していない状態
#define CHAIN_WORK_NULL			(0xff)

///スタンプのZ座標
#define STAMP_POS_Z		(FX32_ONE * 16)	//(FX32_CONST(100))
///スタンプのスケール
#define STAMP_SCALE		(FX32_CONST(1.00f))

///スタンプのサイズ(fx32)
#define STAMP_SIZE			(FX32_ONE * 16 * 1)	//1grid(1gird=16unit)
///スタンプの半分のサイズ
#define STAMP_SIZE_HALF		(STAMP_SIZE / 2)

///スタンプ動作がシステムへ返す返事
typedef enum{
	RET_CONTINUE,		///<現状維持
	RET_DELETE,			///<削除要求
}STAMP_RET;

///足跡のテクスチャーサイズ(byte)
#define FOOTMARK_TEXTURE_SIZE	(16*16/4)

///スタンプで使用出来るポリゴンIDの開始値
#define STAMP_POLYGON_ID_START		(1)
///スタンプで使用出来るポリゴンIDの終了値
#define STAMP_POLYGON_ID_END		(63)
///スタンプでポリゴンIDの確保に失敗
#define STAMP_POLYGON_ID_ERROR		(0xff)


///スタンプがフレームアウトと見なす座標範囲
///ボードが横5グリッド、縦3グリッド(1grid=16unit)の為、それより少し大きめにしている
static const struct{
	fx32 left;
	fx32 right;
	fx32 top;
	fx32 bottom;
}StampFrameOutRect = {
	-FX32_ONE*16*(5+0),
	FX32_ONE*16*(5+0),
	FX32_ONE*16*(3+0),
	-FX32_ONE*16*(3+0),
};

//==============================================================================
//	型定義
//==============================================================================
///スタンプの動作関数の型
typedef STAMP_RET (* STAMP_MOVE_FUNC)(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
///描画実行前に行う関数の型
typedef void ( *STAMP_DRAW_FUNC )(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
///スタンプのあたり判定関数の型
typedef BOOL (* STAMP_HITCHECK)(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);
///スタンプの削除関数の型
typedef void (* STAMP_DELETE_FUNC)(STAMP_MOVE_PTR move);

///スペシャルエフェクトの動作関数の型
typedef BOOL (* SPECIAL_EFF_FUNC)(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);

//==============================================================================
//	構造体定義
//==============================================================================
//--------------------------------------------------------------
//	動作テーブルの型
//--------------------------------------------------------------
///動作テーブルの型
typedef struct{
	STAMP_MOVE_FUNC move_func;			///<動作関数
	STAMP_DRAW_FUNC draw_func;			///<独自に描画処理を行う場合指定
	STAMP_HITCHECK hitcheck_func;		///<ヒットチェック
	STAMP_DELETE_FUNC delete_func;  ///<独自に削除処理を行う場合指定
}STAMP_MOVE_DATA_TBL;

///スタンプの当たり判定チェック用のRECT型
typedef struct{
	fx32 left;
	fx32 right;
	fx32 top;
	fx32 bottom;
}STAMP_HIT_RECT;

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
///スタンプ3Dワーク
typedef struct{
	GFL_G3D_OBJ	*mdl;
	GFL_G3D_OBJSTATUS		obj;
	GFL_G3D_RND *rnder;
	GFL_G3D_RES *pResMdl;
	VecFx32 obj_rotate;
	BOOL draw_flag;
	
	NNSG3dResMdl *pModel;
	NNSG3dResTex *pMdlTex;
}STAMP_3D;

///スタンプ3DワークChild
typedef struct{
	GFL_G3D_OBJ	*mdl;
	GFL_G3D_OBJSTATUS		obj;
	VecFx32 obj_rotate;
	BOOL draw_flag;
}STAMP_3D_CHILD;

//--------------------------------------------------------------
//	消滅エフェクト
//--------------------------------------------------------------
///消滅エフェクト：共通初期動作
typedef struct{
	s32 wait;
	STAMP_HIT_RECT rect;
}ERASE_EFF_INIT_MOVE;

///消滅エフェクト：にじみ
typedef struct{
	int alpha;			///<アルファ値
	fx32 affine_xyz;	///<拡縮率
	u8 seq;			///<シーケンス番号
	u8 polygon_id;		///<ポリゴンID
	STAMP_HIT_RECT rect;
}ERASE_EFF_NIJIMI;

///消滅エフェクト：はじけ
typedef struct{
	STAMP_3D_CHILD		child_obj[HAJIKE_OBJ_CHILD_NUM];	///<描画OBJ
	u16 frame;
	u8 seq;
	STAMP_HIT_RECT rect[HAJIKE_OBJ_CHILD_NUM + 1];	// +1 = 親の分
}ERASE_EFF_HAJIKE;

///消滅エフェクト：ジグザグ
typedef struct{
	int seq;
	int wait;
	int turn_frame;
	STAMP_HIT_RECT rect;
}ERASE_EFF_ZIGZAG;

///消滅エフェクト：蛇行
typedef struct{
	int seq;
	fx32 theta;
	int wait;
	fx32 center_y;
	STAMP_HIT_RECT rect;
}ERASE_EFF_DAKOU;

///消滅エフェクト：軌跡
typedef struct{
	STAMP_3D_CHILD		child_obj[KISEKI_OBJ_CHILD_NUM];	///<描画OBJ
	u16 frame;
	u8 drop_no;
	u8 obj_hit;			///<TRUE:他OBJと衝突した
	u8 polygon_id;
	u8 seq;
	STAMP_HIT_RECT rect;
}ERASE_EFF_KISEKI;

///消滅エフェクト：揺れ
typedef struct{
	int seq;
	fx32 theta;
	fx32 center_x;
	STAMP_HIT_RECT rect;
}ERASE_EFF_YURE;

///消滅エフェクト：拡大
typedef struct{
	fx32 affine_xyz;	///<拡縮率
	u16 frame;
	u8 seq;			///<シーケンス番号
	STAMP_HIT_RECT rect;
}ERASE_EFF_KAKUDAI;

///消滅エフェクト：ブラーX
typedef struct{
	STAMP_3D_CHILD		child_obj[BRAR_X_OBJ_CHILD_NUM];	///<描画OBJ
	fx32 theta;
	u16 alpha;
	u8 polygon_id;
	u8 seq;
	STAMP_HIT_RECT rect[BRAR_X_OBJ_CHILD_NUM + 1];	// +1 = 親の分
}ERASE_EFF_BRAR_X;

///消滅エフェクト：ブラーY
typedef struct{
	STAMP_3D_CHILD		child_obj[BRAR_Y_OBJ_CHILD_NUM];	///<描画OBJ
	fx32 theta;
	u16 alpha;
	u8 polygon_id;
	u8 seq;
	STAMP_HIT_RECT rect[BRAR_Y_OBJ_CHILD_NUM + 1];	// +1 = 親の分
}ERASE_EFF_BRAR_Y;

///消滅エフェクト：たれ
typedef struct{
	fx32 affine_xyz;	///<拡縮率
	u16 frame;
	u8 seq;
	STAMP_HIT_RECT rect;
}ERASE_EFF_TARE;

//--------------------------------------------------------------
/**
 * @brief	消滅エフェクト動作ワーク
 */
//--------------------------------------------------------------
typedef union{
	ERASE_EFF_INIT_MOVE	init_move;
	ERASE_EFF_NIJIMI	nijimi;
	ERASE_EFF_HAJIKE	hajike;
	ERASE_EFF_ZIGZAG	zigzag;
	ERASE_EFF_DAKOU		dakou;
	ERASE_EFF_KISEKI	kiseki;
	ERASE_EFF_YURE		yure;
	ERASE_EFF_KAKUDAI	kakudai;
	ERASE_EFF_BRAR_X	brar_x;
	ERASE_EFF_BRAR_Y	brar_y;
	ERASE_EFF_TARE		tare;
}ERASE_EFF_WORK;


//--------------------------------------------------------------
//	システム
//--------------------------------------------------------------
///スタンプ動作制御ワーク
typedef struct _STAMP_MOVE_WORK{
	STAMP_PARAM param;		///<スタンプパラメータ

	STAMP_3D s3d;
	
	ERASE_EFF_WORK erase_eff;	///<消滅エフェクト動作ワーク
	
	u32 initialize:4;			///<TRUE:初期化完了
	u32 tex_load_req:4;			///<TRUE:テクスチャVRAM転送リクエストを発行している
	u32 move_type:8;			///<動作タイプ(MOVE_???)
	u32 next_move_type:8;		///<動作タイプ予約(連鎖発生時に引き継ぐ動作タイプがセットされる)
	u32 chain_work_no:8;		///<使用している連鎖ワークの番号
}STAMP_MOVE_WORK;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static STAMP_MOVE_PTR Stamp_Create(FOOTPRINT_SYS_PTR fps, STAMP_SYSTEM_WORK *ssw, const STAMP_PARAM *param, ARCHANDLE *hdl_main, ARCHANDLE *hdl_mark, BOOL arceus_flg);
static void Stamp_Free(STAMP_MOVE_PTR move);
static void Stamp_DeletePack(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move, int stamp_no);
static void Stamp_PosConvert(FOOTPRINT_SYS_PTR fps, int x, int y, fx32 *ret_x, fx32 *ret_y);
static BOOL Stamp_MdlLoadH(STAMP_SYSTEM_WORK *ssw, STAMP_3D *s3d, ARCHANDLE *hdl_main, ARCHANDLE *hdl_mark, const STAMP_PARAM *param, BOOL arceus_flg);
static void Stamp_TexRewrite(NNSG3dResTex *pTex, ARCHANDLE *hdl_main, ARCHANDLE *hdl_mark, const STAMP_PARAM *param, BOOL arceus_flg);
static void Stamp_TexDotErase(NNSG3dResTex *pTex, int move_type);
static void Stamp_TexDotFlip(NNSG3dResTex *pTex, int move_type);
static void VWaitTCB_MdlLoad(GFL_TCB* tcb, void *work);
static BOOL Stamp_LoadTexDataSet(STAMP_SYSTEM_WORK *ssw, GFL_G3D_RES *p_resmdl);
static void Stamp_LoadTexDataFree(STAMP_SYSTEM_WORK *ssw, GFL_G3D_RES *p_resmdl);
static BOOL StampMoveTool_MoveTypeSet(STAMP_MOVE_PTR move, int move_type);
static u8 StampMoveTool_MoveTypeGet(const STAMP_PARAM *param);
static int Stamp_PolygonIDGet(STAMP_SYSTEM_WORK *ssw);
static void Stamp_PolygonIDFree(STAMP_SYSTEM_WORK *ssw, int polygon_id);
static void Stamp_HitRectCreate(GFL_G3D_OBJSTATUS *obj, const STAMP_PARAM *param, STAMP_HIT_RECT *rect, int affine_flag);
static BOOL Stamp_RectHitCheck(const STAMP_HIT_RECT *rect0, const STAMP_HIT_RECT *rect1);
static void Stamp_ChainWorkGet(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static int Stamp_ChainAdd(STAMP_SYSTEM_WORK *ssw, int chain_work_no, int move_type);
static void Stamp_ChainSub(STAMP_SYSTEM_WORK *ssw, int chain_work_no);
static void Special_EffectSet(STAMP_SYSTEM_WORK *ssw);
static BOOL SpecialFlashEff(STAMP_SYSTEM_WORK *ssw, SPECIAL_FLASH_WORK *flash_work, int game_status, int board_type);
static void _StampMoveDraw(STAMP_3D *s3d);
static void _StampMoveDrawChild(STAMP_3D_CHILD *child);
static void _SetCameraDistance(STAMP_SYSTEM_WORK *ssw, fx32 distance, GFL_G3D_CAMERA *camera);
static void _AddCameraDistance(STAMP_SYSTEM_WORK *ssw, const fx32 inDist,GFL_G3D_CAMERA* camera_ptr);
static void _AddCameraAngleRev(STAMP_SYSTEM_WORK *ssw, const FOOT_CAMERA_ANGLE *inAngle,GFL_G3D_CAMERA *camera_ptr);
static void _SetCameraAngleRev(STAMP_SYSTEM_WORK *ssw, const FOOT_CAMERA_ANGLE *inAngle,GFL_G3D_CAMERA *camera_ptr);
static void _SetCameraAngleRot(STAMP_SYSTEM_WORK *ssw, const FOOT_CAMERA_ANGLE *inAngle,GFL_G3D_CAMERA *camera_ptr);

static STAMP_RET StampMove_FirstWait(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_Nijimi(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_Hajike(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_Zigzag(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_Dakou(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_Kiseki(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_Yure(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_Kakudai(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_BrarX(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_BrarY(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static STAMP_RET StampMove_Tare(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);

static void StampDraw_Nijimi(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static void StampDraw_Hajike(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static void StampDraw_Kiseki(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static void StampDraw_BrarX(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);
static void StampDraw_BrarY(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move);

static BOOL StampHitcheck_FirstWait(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target);
static BOOL StampHitcheck_Nijimi(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);
static BOOL StampHitcheck_Hajike(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);
static BOOL StampHitcheck_Zigzag(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);
static BOOL StampHitcheck_Dakou(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);
static BOOL StampHitcheck_Kiseki(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);
static BOOL StampHitcheck_Yure(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);
static BOOL StampHitcheck_Kakudai(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move, STAMP_MOVE_PTR target);
static BOOL StampHitcheck_BrarX(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);
static BOOL StampHitcheck_BrarY(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);
static BOOL StampHitcheck_Tare(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target);

static void StampDel_Hajike(STAMP_MOVE_PTR move);
static void StampDel_Kiseki(STAMP_MOVE_PTR move);
static void StampDel_BrarX(STAMP_MOVE_PTR move);
static void StampDel_BrarY(STAMP_MOVE_PTR move);

static BOOL SpecialMove_Nijimi(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static BOOL SpecialMove_Hajike(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static BOOL SpecialMove_Zigzag(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static BOOL SpecialMove_Dakou(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static BOOL SpecialMove_Kiseki(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static BOOL SpecialMove_Yure(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static BOOL SpecialMove_Kakudai(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static BOOL SpecialMove_BrarX(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static BOOL SpecialMove_BrarY(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static BOOL SpecialMove_Tare(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera);
static void Foot_RotateMtx(VecFx32 *rotate, MtxFx33 *dst_mtx);

//==============================================================================
//	シーケンステーブル
//==============================================================================
///スタンプ動作関数テーブル		※MOVE_TYPE_???と並びを同じにしておくこと！
static const STAMP_MOVE_DATA_TBL StampMoveDataTbl[] = {
	{StampMove_FirstWait,	NULL,					StampHitcheck_FirstWait,      NULL},
	{StampMove_Nijimi,		StampDraw_Nijimi,		StampHitcheck_Nijimi,   NULL},
	{StampMove_Hajike,		StampDraw_Hajike,		StampHitcheck_Hajike,   StampDel_Hajike},
	{StampMove_Zigzag,		NULL,					StampHitcheck_Zigzag,         NULL},
	{StampMove_Dakou,		NULL,					StampHitcheck_Dakou,            NULL},
	{StampMove_Kiseki,		StampDraw_Kiseki,		StampHitcheck_Kiseki,   StampDel_Kiseki},
	{StampMove_Yure,		NULL,					StampHitcheck_Yure,             NULL},
	{StampMove_Kakudai,		NULL,					StampHitcheck_Kakudai,        NULL},
	{StampMove_BrarX,		StampDraw_BrarX,		StampHitcheck_BrarX,      StampDel_BrarX},
	{StampMove_BrarY,		StampDraw_BrarY,		StampHitcheck_BrarY,      StampDel_BrarY},
	{StampMove_Tare,		NULL,					StampHitcheck_Tare,             NULL},
};

///スタンプの移動タイプ  ※StampMoveDataTbl、SpecialEffectDataTblと並びを同じにしておくこと！
enum{
	MOVE_TYPE_FIRST_WAIT,	///<初期ウェイト
	MOVE_TYPE_NIJIMI,
	MOVE_TYPE_HAJIKE,
	MOVE_TYPE_ZIGZAG,
	MOVE_TYPE_DAKOU,
	MOVE_TYPE_KISEKI,
	MOVE_TYPE_YURE,
	MOVE_TYPE_KAKUDAI,
	MOVE_TYPE_BRAR_X,
	MOVE_TYPE_BRAR_Y,
	MOVE_TYPE_TARE,
};
///性格から動作タイプを取り出すテーブル
ALIGN4 static const u8 Seikaku_to_MoveType_Tbl[] = {
	MOVE_TYPE_KISEKI,	//がんばりや
	MOVE_TYPE_NIJIMI,	//さみしがり
	MOVE_TYPE_KISEKI,	//ゆうかん
	MOVE_TYPE_YURE,		//いじっぱり
	MOVE_TYPE_YURE,		//やんちゃ
	MOVE_TYPE_DAKOU,	//ずぶとい
	MOVE_TYPE_ZIGZAG,	//すなお
	MOVE_TYPE_BRAR_Y,	//のんき
	MOVE_TYPE_KAKUDAI,	//わんぱく
	MOVE_TYPE_BRAR_X,	//のうてんき
	MOVE_TYPE_NIJIMI,	//おくびょう
	MOVE_TYPE_KISEKI,	//せっかち
	MOVE_TYPE_ZIGZAG,	//まじめ
	MOVE_TYPE_BRAR_X,	//ようき
	MOVE_TYPE_HAJIKE,	//むじゃき
	MOVE_TYPE_TARE,		//ひかえめ
	MOVE_TYPE_DAKOU,	//おっとり
	MOVE_TYPE_ZIGZAG,	//れいせい
	MOVE_TYPE_BRAR_Y,	//てれや
	MOVE_TYPE_KAKUDAI,	//うっかりや
	MOVE_TYPE_TARE,		//おだやか
	MOVE_TYPE_DAKOU,	//おとなしい
	MOVE_TYPE_YURE,		//なまいき
	MOVE_TYPE_NIJIMI,	//しんちょう
	MOVE_TYPE_HAJIKE,	//きまぐれ
};

///スタンプのヒットサイズ
static const STAMP_HIT_RECT StampDefaultHitRect[] = {
	{-(STAMP_SIZE_HALF / 3), STAMP_SIZE_HALF / 3, STAMP_SIZE_HALF / 3, -(STAMP_SIZE_HALF / 3)},
	{-(STAMP_SIZE_HALF / 2), STAMP_SIZE_HALF / 2, STAMP_SIZE_HALF / 2, -(STAMP_SIZE_HALF / 2)},
	{-(STAMP_SIZE_HALF/3*2), STAMP_SIZE_HALF/3*2, STAMP_SIZE_HALF / 3*2, -(STAMP_SIZE_HALF / 3*2)},
};


//--------------------------------------------------------------
//	スペシャルエフェクトの動作シーケンステーブル
//--------------------------------------------------------------
///スペシャルエフェクトの動作シーケンステーブル	※MOVE_TYPE_???と並びを同じにしておくこと！！
static const SPECIAL_EFF_FUNC SpecialEffectDataTbl[] = {
	NULL,						//MOVE_TYPE_FIRST_WAIT
	SpecialMove_Nijimi,         //MOVE_TYPE_NIJIMI
	SpecialMove_Hajike,         //MOVE_TYPE_HAJIKE
	SpecialMove_Zigzag,         //MOVE_TYPE_ZIGZAG
	SpecialMove_Dakou,          //MOVE_TYPE_DAKOU
	SpecialMove_Kiseki,         //MOVE_TYPE_KISEKI
	SpecialMove_Yure,           //MOVE_TYPE_YURE
	SpecialMove_Kakudai,        //MOVE_TYPE_KAKUDAI
	SpecialMove_BrarX,          //MOVE_TYPE_BRAR_X
	SpecialMove_BrarY,          //MOVE_TYPE_BRAR_Y
	SpecialMove_Tare,           //MOVE_TYPE_TARE
};


//--------------------------------------------------------------
//	スタンプ配置座標データ
//--------------------------------------------------------------
#define STAMP_BASE_POS_OFFSET	(16)	//16ドット間隔
//16ドット間隔
static const s32 StampPosBaseX[] = {
	-238076,
	-209404,
	-180732,
	-150012,
	-121340,
	-90620,
	-61948,
	-31228,
	-1020,
	28676,
	57348,
	88068,
	116740,
	147460,
	176132,
	206852,
	235524,
};

//16ドット間隔
static const s32 StampPosBaseY[] = {
	146191,
	117519,
	87311,
	57615,
	27919,
	-2289,
	-31985,
	-61681,
	-91377,
	-121073,
	-149745,
};

//--------------------------------------------------------------
//	デバッグ用変数
//--------------------------------------------------------------
#ifdef PM_DEBUG
static struct{
	STAMP_3D s3d;
	GFL_G3D_OBJSTATUS		obj[4];
	GFL_G3D_OBJ	*mdl[4];
	GFL_G3D_RND *rnder[4];
	VecFx32 obj_rotate[4];
	BOOL draw_flag[4];
}DebugStamp;

static u8 debug_sp_eff = 0;
#endif


//==============================================================================
//
//	スタンプシステム
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   スタンプシステム初期化
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   arceus_flg	アルセウス公開フラグ(TRUE:公開OK)
 */
//--------------------------------------------------------------
void StampSys_Init(STAMP_SYSTEM_WORK *ssw, BOOL arceus_flg, fx32 *camera_distance, FOOT_CAMERA_ANGLE *camera_angle, VecFx32 *camera_target, VecFx32 *camera_up)
{
	GFL_STD_MemClear(ssw, sizeof(STAMP_SYSTEM_WORK));
  ssw->camera_distance = camera_distance;
  ssw->camera_angle = camera_angle;
  ssw->camera_target = camera_target;
  ssw->camera_up = camera_up;
  
#ifdef PM_DEBUG
	{
		ARCHANDLE *hdl_main, *hdl_mark;
		STAMP_PARAM debug_stamp = {
			0x001f, 493, 0x0000, 0, 0, 0,
		};
		int i;
		
		hdl_main = GFL_ARC_OpenDataHandle( ARCID_FOOTPRINT_GRA, HEAPID_FOOTPRINT );
		hdl_mark = GFL_ARC_OpenDataHandle(ARCID_POKEFOOT_GRA, HEAPID_FOOTPRINT);

		Stamp_MdlLoadH(ssw, &DebugStamp.s3d, hdl_main, hdl_mark, &debug_stamp, arceus_flg);
		for(i = 0; i < 4; i++){
	#if WB_FIX
			D3DOBJ_Init( &DebugStamp.s3d.obj[i], &DebugStamp.s3d.mdl );
			D3DOBJ_SetDraw( &DebugStamp.s3d.obj[i], FALSE );
	#else
      DebugStamp.rnder[i] = 
        GFL_G3D_RENDER_Create( DebugStamp.s3d.pResMdl, 0, DebugStamp.s3d.pResMdl );
      DebugStamp.mdl[i] = GFL_G3D_OBJECT_Create(DebugStamp.rnder[i], NULL, 0);
      VEC_Set(&DebugStamp.obj[i].trans, 0, 0, 0);
      VEC_Set(&DebugStamp.obj[i].scale, FX32_ONE, FX32_ONE, FX32_ONE);
      MTX_Identity33(&DebugStamp.obj[i].rotate);
      VEC_Set(&DebugStamp.obj_rotate[i], 0, 0, 0);
      DebugStamp.draw_flag[i] = FALSE;
	#endif
		}

		GFL_ARC_CloseDataHandle( hdl_main );
		GFL_ARC_CloseDataHandle( hdl_mark );
	}
#endif
}

//--------------------------------------------------------------
/**
 * @brief   スタンプシステムを破棄
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 */
//--------------------------------------------------------------
void StampSys_Exit(STAMP_SYSTEM_WORK *ssw)
{
	int i;
	
	for(i = 0; i < STAMP_MAX; i++){
		if(ssw->move[i] != NULL){
			Stamp_Free(ssw->move[i]);
		}
	}
#ifdef PM_DEBUG
#if WB_FIX
	for(i = 0; i < 4; i++){
  	D3DOBJ_MdlDelete(&DebugStamp.mdl);
  }
#else
	for(i = 0; i < 4; i++){
    GFL_G3D_OBJECT_Delete(DebugStamp.mdl[i]);
    GFL_G3D_RENDER_Delete(DebugStamp.rnder[i]);
  }
  GFL_G3D_DeleteResource(DebugStamp.s3d.pResMdl);
#endif
#endif
}

//--------------------------------------------------------------
/**
 * @brief   スタンプを新規作成する
 *
 * @param   ssw				スタンプシステムへのポインタ
 * @param   param			スタンプパラメータ
 * @param   hdl_main		メイングラフィックへのハンドル
 * @param   hdl_mark		足跡グラフィックへのハンドル
 * @param   arceus_flg		アルセウス公開フラグ(TRUE:公開OK)
 *
 * @retval	TRUE:作成成功。　FALSE:作成失敗
 */
//--------------------------------------------------------------
BOOL StampSys_Add(FOOTPRINT_SYS_PTR fps, STAMP_SYSTEM_WORK *ssw, const STAMP_PARAM *param, ARCHANDLE *hdl_main, ARCHANDLE *hdl_mark, BOOL arceus_flg)
{
	int i;
	
	if(ssw->special_effect_type != 0){
		return FALSE;
	}
	
	for(i = 0; i < STAMP_MAX; i++){
		if(ssw->move[i] == NULL){
			ssw->move[i] = Stamp_Create(fps, ssw, param, hdl_main, hdl_mark, arceus_flg);
			if(ssw->move[i] == NULL){
				OS_TPrintf("スタンプの新規登録失敗：テクスチャかメモリがいっぱい\n");
				return FALSE;
			}
			return TRUE;
		}
	}
	
	OS_TPrintf("スタンプの新規登録失敗：領域いっぱいだった\n");
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ更新処理
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 */
//--------------------------------------------------------------
void StampSys_Update(STAMP_SYSTEM_WORK *ssw, GFL_G3D_CAMERA * camera_ptr, int game_status, int board_type)
{
	int i;
	STAMP_MOVE_PTR move;
	STAMP_RET ret;
	BOOL sp_ret;
	
#ifdef PM_DEBUG		//サウンドチェック用
	if((GFL_UI_KEY_GetTrg() & PAD_BUTTON_A) && ssw->special_effect_type == 0){
		GFL_STD_MemClear(&ssw->special_work, sizeof(STAMP_SPECIAL_WORK));
		debug_sp_eff++;
		debug_sp_eff %= NELEMS(SpecialEffectDataTbl);
		ssw->special_effect_type = debug_sp_eff;
	}
#endif

	//スペシャルエフェクト発動前の動作
	if(ssw->special_effect_req != 0){
		SpecialFlashEff(ssw, &ssw->flash_work, game_status, board_type);
	}
	
	//スペシャルエフェクト動作
	if(ssw->special_effect_type != 0){
		if(SpecialEffectDataTbl[ssw->special_effect_type](ssw, &ssw->special_work, camera_ptr) == TRUE){
			ssw->special_effect_type = 0;
		}
	}
	
	//スタンプ動作
	for(i = 0; i < STAMP_MAX; i++){
		move = ssw->move[i];
		if(move != NULL){
			if(move->move_type == 0 && move->next_move_type != 0){
				move->move_type = move->next_move_type;
			}
			
			ret = StampMoveDataTbl[move->move_type].move_func(ssw, move);
			switch(ret){
			case RET_DELETE:
				Stamp_DeletePack(ssw, move, i);
				break;
			}
		}
		move++;
	}
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ描画
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 */
//--------------------------------------------------------------
void StampSys_ObjDraw(STAMP_SYSTEM_WORK *ssw)
{
	int i;
	STAMP_MOVE_PTR move;
	
	for(i = 0; i < STAMP_MAX; i++){
		move = ssw->move[i];
		if(move != NULL){
			if(StampMoveDataTbl[move->move_type].draw_func != NULL){
				StampMoveDataTbl[move->move_type].draw_func(ssw, move);
			}
			else{
		#if WB_FIX
				D3DOBJ_Draw(&move->s3d.obj);
		#else
		    _StampMoveDraw(&move->s3d);
		#endif
			}
		}
	}

#ifdef PM_DEBUG
	for(i = 0; i < 4; i++){
	#if WB_FIX
		D3DOBJ_Draw(&DebugStamp.obj[i]);
	#else
	  if(DebugStamp.draw_flag[i] == TRUE){
      GFL_G3D_DRAW_DrawObject(DebugStamp.mdl[i], &DebugStamp.obj[i]);
    }
	#endif
	}
#endif
}

//--------------------------------------------------------------
/**
 * スタンプ描画(親)
 *
 * @param   s3d		
 */
//--------------------------------------------------------------
static void _StampMoveDraw(STAMP_3D *s3d)
{
  if(s3d->draw_flag == TRUE){
    GFL_G3D_DRAW_DrawObject(s3d->mdl, &s3d->obj);
  }
}

//--------------------------------------------------------------
/**
 * スタンプ描画(子)
 *
 * @param   child		
 */
//--------------------------------------------------------------
static void _StampMoveDrawChild(STAMP_3D_CHILD *child)
{
  if(child->draw_flag == TRUE){
    GFL_G3D_DRAW_DrawObject(child->mdl, &child->obj);
  }
}

//--------------------------------------------------------------
/**
 * @brief   スタンプのヒットチェック
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 */
//--------------------------------------------------------------
void StampSys_HitCheck(STAMP_SYSTEM_WORK *ssw)
{
	int i, move_max, normal_max, move_count, normal_count;
	STAMP_MOVE_PTR move, normal;
	BOOL ret;
	int sp_eff = 0;
	
	//ヒットチェック対象のmoveポインタのみを抜き出す
	move_max = 0;
	normal_max = 0;
	for(i = 0; i < STAMP_MAX; i++){
		move = ssw->move[i];
		if(move != NULL){
			if(move->move_type != 0){	//既に動作しているスタンプ
				ssw->hitcheck_move[move_max] = move;
				move_max++;
			}
			else{	//まだ動作前のスタンプ
				ssw->hitcheck_move[STAMP_MAX - 1 - normal_max] = move;
				normal_max++;
			}
		}
	}
	
	//ヒットチェック
	for(normal_count = 0; normal_count < normal_max; normal_count++){
		normal = ssw->hitcheck_move[STAMP_MAX - 1 - normal_count];
		for(move_count = 0; move_count < move_max; move_count++){
			move = ssw->hitcheck_move[move_count];
			if(StampMoveDataTbl[move->move_type].hitcheck_func(ssw, move, normal) == TRUE){
				ret = StampMoveTool_MoveTypeSet(normal, move->move_type);
				if(ret == TRUE && normal->chain_work_no == CHAIN_WORK_NULL){
					normal->chain_work_no = move->chain_work_no;
					if(Stamp_ChainAdd(ssw, move->chain_work_no, move->move_type) == TRUE){
						sp_eff++;
					}
					break;	//あたり判定が発生したので次のノーマルスタンプの判定へ。
				}
			}
		}
	}
	
	//スペシャルエフェクト発動条件を満たしていればスペシャル発動
	if(ssw->special_effect_req != 0 && ssw->special_effect_start_effect_end == TRUE){
		for(i = 0; i < STAMP_MAX; i++){
			move = ssw->move[i];
			if(move != NULL && move->move_type != ssw->special_effect_req 
					&& move->next_move_type != ssw->special_effect_req){
				if(StampMoveTool_MoveTypeSet(move, ssw->special_effect_req) == FALSE){
					//既に動作しているスタンプは削除する
					Stamp_DeletePack(ssw, move, i);
				}
			}
		}
		Special_EffectSet(ssw);
	}
}

//--------------------------------------------------------------
/**
 * @brief   スタンプVwait更新処理
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   game_status	ゲームステータス
 */
//--------------------------------------------------------------
void StampSys_VWaitUpdate(STAMP_SYSTEM_WORK *ssw, int game_status)
{
	int i;
	GFL_G3D_RES **tex;
	
	tex = ssw->load_tex;
	for(i = 0; i < LOAD_TEX_BUFFER_MAX; i++){
		if(*tex != NULL){
      GFL_G3D_TransOnlyTexture(*tex);
			*tex = NULL;
			OS_TPrintf("転送した %d\n", i);
		}
		tex++;
	}
	
	if(ssw->v_wipe_req && game_status != FOOTPRINT_GAME_STATUS_FINISH){
		WIPE_SetMstBrightness(WIPE_DISP_MAIN, ssw->v_wipe_no);
		ssw->v_wipe_req = 0;
	}
}


//==============================================================================
//
//	スタンプ
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief	スタンプを作成する
 *
 * @param	param			スタンプパラメータへのポインタ
 * @param	ssw				スタンプシステムワークへのポインタ
 * @param	hdl_main		足跡ボードのメインnarcファイルのハンドル
 * @param	hdl_mark		足跡グラフィックのnarcファイルのハンドル
 * @param   arceus_flg		アルセウス公開フラグ(TRUE:公開OK)
 *
 * @retval	生成したスタンプ動作ワークへのポインタ
 * @retval	生成出来なかった場合はNULL
 */
//--------------------------------------------------------------
static STAMP_MOVE_PTR Stamp_Create(FOOTPRINT_SYS_PTR fps, STAMP_SYSTEM_WORK *ssw, const STAMP_PARAM *param, ARCHANDLE *hdl_main, ARCHANDLE *hdl_mark, BOOL arceus_flg)
{
	STAMP_MOVE_PTR move;
	fx32 x, y;
	BOOL mdl_ret;
	
	move = GFL_HEAP_AllocMemory(HEAPID_FOOTPRINT, sizeof(STAMP_MOVE_WORK));
	GFL_STD_MemClear(move, sizeof(STAMP_MOVE_WORK));
	move->chain_work_no = CHAIN_WORK_NULL;
	
	move->param = *param;
	if(move->param.monsno > MONSNO_END){
		GF_ASSERT(0);
		move->param.monsno = MONSNO_METAMON;
	}
	
	//モデルデータ読み込み
	//D3DOBJ_MdlLoadH(&move->s3d.mdl, hdl_main, NARC_footprint_board_ashiato_nsbmd, HEAPID_FOOTPRINT);
	mdl_ret = Stamp_MdlLoadH(ssw, &move->s3d, hdl_main, hdl_mark, &move->param, arceus_flg);
	if(mdl_ret == FALSE){
		return NULL;
	}
	
#if WB_FIX
	//レンダーオブジェクトに登録
	D3DOBJ_Init( &move->s3d.obj, &move->s3d.mdl );
	
	//座標設定
	Stamp_PosConvert(fps, param->x, param->y, &x, &y);
	D3DOBJ_SetMatrix( &move->s3d.obj, x, y, STAMP_POS_Z);
	D3DOBJ_SetScale(&move->s3d.obj, STAMP_SCALE, STAMP_SCALE, STAMP_SCALE);
	D3DOBJ_SetDraw( &move->s3d.obj, FALSE );
#else
	//レンダーオブジェクトに登録
  move->s3d.rnder = GFL_G3D_RENDER_Create( move->s3d.pResMdl, 0, move->s3d.pResMdl );
  move->s3d.mdl = GFL_G3D_OBJECT_Create(move->s3d.rnder, NULL, 0);  //アニメ無し
	
	//座標設定
	Stamp_PosConvert(fps, param->x, param->y, &x, &y);
  VEC_Set(&move->s3d.obj.trans, x, y, STAMP_POS_Z);
  VEC_Set(&move->s3d.obj.scale, STAMP_SCALE, STAMP_SCALE, STAMP_SCALE);
  MTX_Identity33(&move->s3d.obj.rotate);
  VEC_Set(&move->s3d.obj_rotate, 0, 0, 0);
  move->s3d.draw_flag = FALSE;
#endif

	return move;
}

//--------------------------------------------------------------
/**
 * スタンプ(子)を生成する
 *
 * @param   parent_s3d		親スタンプへのポインタ
 *
 * @retval  STAMP_3D_CHILD *		
 */
//--------------------------------------------------------------
static void _StampChildAdd(STAMP_3D *parent_s3d, STAMP_3D_CHILD *child)
{
  child->mdl = GFL_G3D_OBJECT_Create(parent_s3d->rnder, NULL, 0);
  VEC_Set(&child->obj.trans, 0, 0, STAMP_POS_Z);
  VEC_Set(&child->obj.scale, STAMP_SCALE, STAMP_SCALE, STAMP_SCALE);
  MTX_Identity33(&child->obj.rotate);
  VEC_Set(&child->obj_rotate, 0, 0, 0);
  child->draw_flag = TRUE;
}

//--------------------------------------------------------------
/**
 * スタンプ(子)を削除する
 *
 * @param   parent_s3d		親スタンプへのポインタ
 *
 * @retval  STAMP_3D_CHILD *		
 */
//--------------------------------------------------------------
static void _StampChildDel(STAMP_3D *parent_s3d, STAMP_3D_CHILD *child)
{
  GFL_G3D_OBJECT_Delete(child->mdl);
  child->draw_flag = FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   テクスチャデータを足跡に書き換えてVRAM転送(モデルデータの読み込みなども行う)
 *
 * @param   p_mdl			モデルデータ代入先
 * @param   hdl_main		メイングラフィックへのハンドル
 * @param   hdl_mark		足跡グラフィックへのハンドル
 * @param   param			スタンプパラメータへのポインタ
 * @param   arceus_flg		アルセウス公開フラグ(TRUE:公開OK)
 * 
 * @retval	TRUE:成功。　FALSE:失敗
 *
 *	D3DOBJ_MdlLoadH関数の中身を抜き出して必要な部分をカスタマイズしたもの
 */
//--------------------------------------------------------------
static BOOL Stamp_MdlLoadH(STAMP_SYSTEM_WORK *ssw, STAMP_3D *s3d, ARCHANDLE *hdl_main, ARCHANDLE *hdl_mark, const STAMP_PARAM *param, BOOL arceus_flg)
{
	// モデルﾃﾞｰﾀ読み込み
#if WB_FIX
	p_mdl->pResMdl = GFL_ARCHDL_UTIL_Load(hdl_main, NARC_footprint_board_ashiato_nsbmd, 
		FALSE, HEAPID_FOOTPRINT );
#else
  s3d->pResMdl = GFL_G3D_CreateResourceHandle( hdl_main, NARC_footprint_board_ashiato_nsbmd );
#endif

	// モデルデータ設定＆テクスチャ転送
	{
		BOOL vram_ret;
    NNSG3dResFileHeader *nns3dhead;
    NNSG3dResMdlSet *mdlset;
    
		// リソース読み込み済みである必要がある
		nns3dhead = GFL_G3D_GetResourceFileHeader(s3d->pResMdl);
		GF_ASSERT( nns3dhead );
		
		// モデルﾃﾞｰﾀ取得
		mdlset	= NNS_G3dGetMdlSet( nns3dhead );
		s3d->pModel		= NNS_G3dGetMdlByIdx( mdlset, 0 );
		s3d->pMdlTex		= NNS_G3dGetTex( nns3dhead );
    
		if( s3d->pMdlTex ){
			//テクスチャ書き換え
			Stamp_TexRewrite(s3d->pMdlTex, hdl_main, hdl_mark, param, arceus_flg);
			
			// テクスチャデータ転送
			//LoadVRAMTexture( s3d->pMdlTex );
      vram_ret = GFL_G3D_AllocVramTexture(s3d->pResMdl);
			if(vram_ret == FALSE || Stamp_LoadTexDataSet(ssw, s3d->pResMdl) == FALSE){
				//VRAM or TCBが確保出来ない場合は、全て解放する

				//VramKey破棄
				if(vram_ret == TRUE){
          GFL_G3D_FreeVramTexture(s3d->pResMdl);
				}
				
				// 全リソース破棄
				if(nns3dhead){
          GFL_G3D_DeleteResource(s3d->pResMdl);
          s3d->pResMdl = NULL;
				}
				GFL_STD_MemFill( s3d, 0, sizeof(STAMP_3D) );

				OS_TPrintf("!!!!!VRAM or TCBの確保失敗!!!!!! vram_ret = %d\n", vram_ret);
				return FALSE;
			}
			
#if WB_FIX
			BindTexture( nns3dhead, s3d->pMdlTex );
#endif
		}
	}
	
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   テクスチャVwait転送リクエストを設定する
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   tex		転送するテクスチャリソースへのポインタ
 *
 * @retval  TRUE:リクエスト受付成功
 * @retval  TRUE:リクエスト受付失敗
 */
//--------------------------------------------------------------
static BOOL Stamp_LoadTexDataSet(STAMP_SYSTEM_WORK *ssw, GFL_G3D_RES *p_resmdl)
{
	int i;
	
	for(i = 0; i < LOAD_TEX_BUFFER_MAX; i++){
		if(ssw->load_tex[i] == NULL){
			ssw->load_tex[i] = p_resmdl;
			return TRUE;
		}
	}
//	GF_ASSERT(0);	//テクスチャVWait転送バッファがいっぱいだった
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   テクスチャVwait転送リクエストを解除する
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   tex		転送するテクスチャリソースへのポインタ
 *
 * @retval  TRUE:リクエスト受付成功
 * @retval  TRUE:リクエスト受付失敗
 */
//--------------------------------------------------------------
static void Stamp_LoadTexDataFree(STAMP_SYSTEM_WORK *ssw, GFL_G3D_RES *p_resmdl)
{
	int i;
	
	for(i = 0; i < LOAD_TEX_BUFFER_MAX; i++){
		if(ssw->load_tex[i] == p_resmdl){
			ssw->load_tex[i] = NULL;
			return;
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief   テクスチャイメージを指定ポケモンの足跡に書き換える
 *
 * @param   pTex			テクスチャリソースへのポインタ
 * @param   hdl_mark		足跡グラフィックのアーカイブハンドルへのポインタ
 * @param   param			スタンプパラメータへのポインタ
 * @param   arceus_flg		アルセウス公開許可フラグ(TRUE:公開OK)
 */
//--------------------------------------------------------------
static void Stamp_TexRewrite(NNSG3dResTex *pTex, ARCHANDLE *hdl_main, ARCHANDLE *hdl_mark, const STAMP_PARAM *param, BOOL arceus_flg)
{
	u16 *pPalDest;
	u8 *pDest, *pSrc;
	enum{
		AFTER_COLOR_NO = 1,	//色つきの場所は全てこのカラー番号に変換する
	};
	NNSG2dCharacterData *pChar;
	int foot_disp;
	
	foot_disp = FootprintTool_FootDispCheck(param->monsno, param->form_no, arceus_flg);

	pDest = (u8*)((u8*)pTex + pTex->texInfo.ofsTex);	//テクスチャ領域
	if(foot_disp == TRUE){
		pSrc = GFL_ARCHDL_UTIL_Load(hdl_mark, POKEFOOT_ARC_CHAR_DMMY + param->monsno, //足跡
			TRUE, GFL_HEAP_LOWID(HEAPID_FOOTPRINT));
		NNS_G2dGetUnpackedCharacterData(pSrc, &pChar); 
	}
	else{
		pSrc = GFL_ARCHDL_UTIL_Load(hdl_main, NARC_footprint_board_wifi_mark_NCGR, //WIFIマーク
			FALSE, GFL_HEAP_LOWID(HEAPID_FOOTPRINT));
		NNS_G2dGetUnpackedCharacterData(pSrc, &pChar);
	}
	
	//テクスチャ領域を全てクリア(あらかじめ入っているダミーの足跡データをクリアしている)
	GFL_STD_MemClear16(pDest, FOOTMARK_TEXTURE_SIZE);

	OS_TPrintf("スタンプのmonsno = %d\n", param->monsno);
	//2Dグラフィックを変換しながらテクスチャ領域にデータを入れる
	{
		int x, y, i, s, w, dest_shift;
		u8 *read_src, *read_data;
		u16 *write_data;
		
		read_src = pChar->pRawData;
		read_data = read_src;
		write_data = (u16*)pDest;
		for(w = 0; w < 4; w++){
			switch(w){
			case 0:
				if(foot_disp == TRUE){
					read_data = &read_src[0x20 * 4];	//128kマッピングなので空白が入っている
				}
				else{
					read_data = &read_src[0x20 * 0];
				}
				write_data = (u16*)pDest;
				break;
			case 1:
				if(foot_disp == TRUE){
					read_data = &read_src[0x20 * 5];	//128kマッピングなので空白が入っている
				}
				else{
					read_data = &read_src[0x20 * 1];
				}
				write_data = (u16*)pDest;
				write_data++;
				break;
			case 2:
				if(foot_disp == TRUE){
					read_data = read_src;
				}
				else{
					read_data = &read_src[0x20 * 2];
				}
				write_data = (u16*)pDest;
				write_data = &write_data[2*8];
				break;
			case 3:
				if(foot_disp == TRUE){
					read_data = &read_src[0x20 * 1];	//128kマッピングなので空白が入っている
				}
				else{
					read_data = &read_src[0x20 * 3];
				}
				write_data = (u16*)pDest;
				write_data = &write_data[2*8 + 1];
				break;
			}
			
			i = 0;
			for(y = 0; y < 8; y++){
				dest_shift = 0;
				for(s = 0; s < 4; s++){
					//OS_TPrintf("read_data %d = %x\n", i, read_data[i]);
					if(read_data[i] & 0x0f){
						*write_data |= AFTER_COLOR_NO << dest_shift;
					}
					dest_shift += 2;
					if(read_data[i] & 0xf0){
						*write_data |= AFTER_COLOR_NO << dest_shift;
					}
					dest_shift += 2;
					i++;
				}
				//OS_TPrintf("texbit = %x\n", *write_data);
				write_data++;
				write_data++;
			}
		}
	}

	//足跡のパレット変更
	pPalDest = (u16*)((u8*)pTex + pTex->plttInfo.ofsPlttData);
	pPalDest[1] = param->color;

	GFL_HEAP_FreeMemory(pSrc);
}

//--------------------------------------------------------------
/**
 * @brief   テクスチャイメージを一部削る
 *
 * @param   pTex			テクスチャリソースへのポインタ
 * @param   hdl_mark		足跡グラフィックのアーカイブハンドルへのポインタ
 * @param   param			スタンプパラメータへのポインタ
 */
//--------------------------------------------------------------
static void Stamp_TexDotErase(NNSG3dResTex *pTex, int move_type)
{
	u16 *pDest;
	int i;
	
	pDest = (u16*)((u8*)pTex + pTex->texInfo.ofsTex);	//テクスチャ領域
	
	switch(move_type){
	case MOVE_TYPE_ZIGZAG:
		pDest[0] = 0;
		pDest[1] = 0;
		break;
	case MOVE_TYPE_DAKOU:
		for(i = 0; i < 32; i += 2){
			pDest[i] &= 0xfffc;
		}
		break;
	}
}

//--------------------------------------------------------------
/**
 * @brief   テクスチャイメージをドット単位で操作し、フリップする
 *
 * @param   pTex			テクスチャリソースへのポインタ
 * @param   hdl_mark		足跡グラフィックのアーカイブハンドルへのポインタ
 * @param   param			スタンプパラメータへのポインタ
 */
//--------------------------------------------------------------
static void Stamp_TexDotFlip(NNSG3dResTex *pTex, int move_type)
{
	u32 *pDest;
	u32 *pSrc;
	int x, y, dest_x, dest_y, data;
	
	pDest = (u32*)((u8*)pTex + pTex->texInfo.ofsTex);	//テクスチャ領域
	pSrc = GFL_HEAP_AllocMemory(HEAPID_FOOTPRINT, FOOTMARK_TEXTURE_SIZE);
	GFL_STD_MemCopy16(pDest, pSrc, FOOTMARK_TEXTURE_SIZE);
	GFL_STD_MemClear16(pDest, FOOTMARK_TEXTURE_SIZE);
	
	switch(move_type){
	case MOVE_TYPE_ZIGZAG:
		dest_x = 30;
		for(y = 0; y < 16; y++){
			dest_y = 0;
			for(x = 0; x < 32; x += 2){
				data = (pSrc[y] >> x) & 3;
				pDest[dest_y] |= data << dest_x;
				dest_y++;
			}
			dest_x -= 2;
		}
		break;
	case MOVE_TYPE_DAKOU:
		dest_x = 0;
		for(y = 0; y < 16; y++){
			dest_y = 15;
			for(x = 0; x < 32; x += 2){
				data = (pSrc[y] >> x) & 3;
				pDest[dest_y] |= data << dest_x;
				dest_y--;
			}
			dest_x += 2;
		}
		break;
	}
	
	GFL_HEAP_FreeMemory(pSrc);
}

//--------------------------------------------------------------
/**
 * @brief   スタンプを解放する
 *
 * @param   move		スタンプ動作ワークへのポインタ
 */
//--------------------------------------------------------------
static void Stamp_Free(STAMP_MOVE_PTR move)
{
#if WB_FIX
	D3DOBJ_MdlDelete( &move->s3d.mdl );
#else
  if(StampMoveDataTbl[move->move_type].delete_func != NULL){
    StampMoveDataTbl[move->move_type].delete_func(move);
  }
  GFL_G3D_OBJECT_Delete(move->s3d.mdl);
  GFL_G3D_RENDER_Delete(move->s3d.rnder);
  GFL_G3D_DeleteResource(move->s3d.pResMdl);
#endif
	GFL_HEAP_FreeMemory(move);
}

//--------------------------------------------------------------
/**
 * @brief   スタンプの削除に必要な処理をまとめたもの
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   stamp_no	スタンプ番号
 */
//--------------------------------------------------------------
static void Stamp_DeletePack(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move, int stamp_no)
{
	if(move->initialize == 0 || move->tex_load_req == TRUE){
		Stamp_LoadTexDataFree(ssw, move->s3d.pResMdl);
	}
	Stamp_ChainSub(ssw, move->chain_work_no);
	Stamp_Free(move);
	ssw->move[stamp_no] = NULL;
}

//--------------------------------------------------------------
/**
 * @brief	スタンプの2D座標を3D座標に変換する
 *
 * @param	x			2D座標X
 * @param	y			2D座標Y
 * @param	ret_x		Xの変換座標代入先
 * @param	ret_y		Yの変換座標代入先
 */
//--------------------------------------------------------------
static void Stamp_PosConvert(FOOTPRINT_SYS_PTR fps, int x, int y, fx32 *ret_x, fx32 *ret_y)
{
#if 0
	*ret_x = FX32_CONST(x - 256/2);
	*ret_y = FX32_CONST(-(y - 196/2));
	
	{
		VecFx32 near, far;
		NNS_G3dScrPosToWorldLine(x, y, &near, &far);
		OS_TPrintf("tp_x = %d, tp_y = %d, \nnear.x = %d, near.y = %d, near.z = %d, \nfar.x = %d, far.y = %d, far.z = %d\n\n", x, y, near.x, near.y,near.z, far.x,far.y,far.z);
	}
#elif 0
	fx32 width, height;
	fx32 width_half, height_half;
	
	Footprint_WorldWidthHeightGet(fps, &width, &height);
	*ret_x = width * x / 256 - width / 2;
	*ret_y = -(height * y / 196 - height / 2);
#else
	int tbl_no;
	
	tbl_no = x / STAMP_BASE_POS_OFFSET;
	if(tbl_no >= NELEMS(StampPosBaseX)){	//一応
		tbl_no = NELEMS(StampPosBaseX) - 1;
	}
	*ret_x = StampPosBaseX[tbl_no];
	if(tbl_no < NELEMS(StampPosBaseX) - 1){
		*ret_x += (StampPosBaseX[tbl_no + 1] - StampPosBaseX[tbl_no]) 
			/ STAMP_BASE_POS_OFFSET * (x % STAMP_BASE_POS_OFFSET);
	}

	tbl_no = y / STAMP_BASE_POS_OFFSET;
	if(tbl_no >= NELEMS(StampPosBaseY)){	//一応
		tbl_no = NELEMS(StampPosBaseY) - 1;
	}
	*ret_y = StampPosBaseY[tbl_no];
	if(tbl_no < NELEMS(StampPosBaseY) - 1){
		*ret_y += (StampPosBaseY[tbl_no + 1] - StampPosBaseY[tbl_no]) 
			/ STAMP_BASE_POS_OFFSET * (y % STAMP_BASE_POS_OFFSET);
	}
#endif
}

//--------------------------------------------------------------
/**
 * @brief   ポリゴンIDを取得する
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 *
 * @retval  ポリゴンID
 */
//--------------------------------------------------------------
static int Stamp_PolygonIDGet(STAMP_SYSTEM_WORK *ssw)
{
	int i, k;
	
	for(i = STAMP_POLYGON_ID_START; i < 32; i++){
		if((ssw->polygon_id_manage[0] & (1 << i)) == 0){
			ssw->polygon_id_manage[0] |= 1 << i;
			return i;
		}
	}
	k = 0;
	for( ; i < STAMP_POLYGON_ID_END; i++){
		if((ssw->polygon_id_manage[1] & (1 << k)) == 0){
			ssw->polygon_id_manage[1] |= 1 << k;
			return i;
		}
		k++;
	}
	OS_TPrintf("ポリゴンIDの確保に失敗\n");
	return STAMP_POLYGON_ID_ERROR;
}

//--------------------------------------------------------------
/**
 * @brief   ポリゴンIDの解放を行う
 *
 * @param   ssw				スタンプシステムワークへのポインタ
 * @param   polygon_id		ポリゴンID
 */
//--------------------------------------------------------------
static void Stamp_PolygonIDFree(STAMP_SYSTEM_WORK *ssw, int polygon_id)
{
	if(polygon_id == STAMP_POLYGON_ID_ERROR){
		return;
	}
	
	if(polygon_id < 32){
		ssw->polygon_id_manage[0] &= 0xffffffff ^ (1 << polygon_id);
	}
	else{
		ssw->polygon_id_manage[1] &= 0xffffffff ^ (1 << (polygon_id - 32));
	}
}

//--------------------------------------------------------------
/**
 * @brief   ヒット範囲を作成します。
 *
 * @param   move			スタンプへのポインタ
 * @param   rect			ヒット範囲代入先
 * @param   affine_flag		TRUE:拡縮をしている。FALSEにすると拡縮率の計算を省く為、軽くなります
 */
//--------------------------------------------------------------
static void Stamp_HitRectCreate(GFL_G3D_OBJSTATUS *obj, const STAMP_PARAM *param, STAMP_HIT_RECT *rect, int affine_flag)
{
	VecFx32 pos;
	int hit_size;
	fx64 bairitu_x, bairitu_y, add_x, add_y;
	
	OS_TPrintf("param->monsno = %d, form_no = %d\n", param->monsno, param->form_no);
	hit_size = FootprintTool_FootHitSizeGet(param->monsno, param->form_no);
	GF_ASSERT(hit_size < NELEMS(StampDefaultHitRect));
	
	pos = obj->trans;

	rect->left = pos.x + StampDefaultHitRect[hit_size].left;
	rect->right = pos.x + StampDefaultHitRect[hit_size].right;
	rect->top = pos.y + StampDefaultHitRect[hit_size].top;
	rect->bottom = pos.y + StampDefaultHitRect[hit_size].bottom;
//	OS_TPrintf("hit_rect fx_x = %d, fx_y = %d\n", fx_x, fx_y);
//	OS_TPrintf("hit_rect normal left=%d, right=%d, top=%d, bottom=%d\n", rect->left, rect->right, rect->top, rect->bottom);
	if(affine_flag == TRUE){
		bairitu_x = ((fx64)obj->scale.x) * 100 / FX32_ONE;
		add_x = (rect->right - rect->left) * bairitu_x / 100 - (rect->right - rect->left);
		rect->right += add_x / 2;
		rect->left += -(add_x / 2);
		
		bairitu_y = ((fx64)obj->scale.y) * 100 / FX32_ONE;
		add_y = (rect->top - rect->bottom) * bairitu_y / 100 - (rect->top - rect->bottom);
		rect->top += add_y / 2;
		rect->bottom += -(add_y / 2);
//		OS_TPrintf("hit_rect affine left=%d, right=%d, top=%d, bottom=%d\n", rect->left, rect->right, rect->top, rect->bottom);
	}

#ifdef PM_DEBUG
	{
		int i;
		
		if(GFL_UI_KEY_GetCont() & PAD_BUTTON_X){
      DebugStamp.obj[0].trans.x = rect->left;
      DebugStamp.obj[0].trans.y = rect->top;
      DebugStamp.obj[1].trans.x = rect->left;
      DebugStamp.obj[1].trans.y = rect->bottom;
      DebugStamp.obj[2].trans.x = rect->right;
      DebugStamp.obj[2].trans.y = rect->top;
      DebugStamp.obj[3].trans.x = rect->right;
      DebugStamp.obj[3].trans.y = rect->bottom;
			for(i = 0; i < 4; i++){
        DebugStamp.draw_flag[i] = TRUE;
			}
		}
		else{
			for(i = 0; i < 4; i++){
        DebugStamp.draw_flag[i] = FALSE;
			}
		}
	}
#endif
}

//--------------------------------------------------------------
/**
 * @brief   矩形同士の当たり判定チェック
 *
 * @param   rect0		矩形１
 * @param   rect1		矩形２
 *
 * @retval  TRUE:ヒット有り。　FALSE:ヒット無し
 */
//--------------------------------------------------------------
static BOOL Stamp_RectHitCheck(const STAMP_HIT_RECT *rect0, const STAMP_HIT_RECT *rect1)
{
	if( (rect0->left <= rect1->right) && (rect1->left <= rect0->right) &&
			(rect0->top >= rect1->bottom) && (rect1->top >= rect0->bottom)) {
		return TRUE;
	}else{
		return FALSE;
	}
}

//--------------------------------------------------------------
/**
 * @brief   連鎖管理ワーク番号を取得する
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 */
//--------------------------------------------------------------
static void Stamp_ChainWorkGet(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	int i;
	STAMP_CHAIN_WORK *chain_work;
	
	chain_work = ssw->chain_work;
	for(i = 0; i < STAMP_MAX; i++){
		if(chain_work->stamp_num == 0){
			move->chain_work_no = i;
			chain_work->chain = 0;
			chain_work->stamp_num++;
			return;
		}
		chain_work++;
	}
	GF_ASSERT(0);	//チェインワークの確保が出来なかった
}

//--------------------------------------------------------------
/**
 * @brief   連鎖管理ワークを参照しているスタンプを増やす
 *
 * @param   ssw					スタンプシステムへのポインタ
 * @param   chain_work_no		連鎖ワーク管理番号
 * @param   move_type			連鎖中の動作タイプ
 *
 * @retval  TRUE:スペシャルエフェクト発動
 * @retval  FALSE:スペシャルエフェクトは発動していない
 */
//--------------------------------------------------------------
static int Stamp_ChainAdd(STAMP_SYSTEM_WORK *ssw, int chain_work_no, int move_type)
{
	if(chain_work_no == CHAIN_WORK_NULL){
		return FALSE;
	}
	
	ssw->chain_work[chain_work_no].chain++;
	ssw->chain_work[chain_work_no].stamp_num++;
	OS_TPrintf("%d連鎖! work_no = %d\n", ssw->chain_work[chain_work_no].chain, chain_work_no);
	
	if(ssw->special_effect_type == 0 && ssw->special_effect_req == 0 
			&& ssw->chain_work[chain_work_no].chain >= SpecialChainNum[ssw->player_max]){
		ssw->special_effect_req = move_type;
		GFL_STD_MemClear(&ssw->flash_work, sizeof(SPECIAL_FLASH_WORK));
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   連鎖管理ワークを参照しているスタンプを減らす
 *
 * @param   ssw					スタンプシステムワークへのポインタ
 * @param   chain_work_no		連鎖ワーク管理番号
 */
//--------------------------------------------------------------
static void Stamp_ChainSub(STAMP_SYSTEM_WORK *ssw, int chain_work_no)
{
	int temp;
	
	if(chain_work_no == CHAIN_WORK_NULL){
		return;
	}
	
	//スタンプは消えても連鎖回数は持続するので、参照しているスタンプ数だけを引く
	temp = ssw->chain_work[chain_work_no].stamp_num;
	temp--;
	if(temp < 0){		//念のため
		GF_ASSERT(0);
		temp = 0;
	}
	ssw->chain_work[chain_work_no].stamp_num = temp;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクトのパラメータセット
 *
 * @param   ssw		スタンプシステムへのポインタ
 */
//--------------------------------------------------------------
static void Special_EffectSet(STAMP_SYSTEM_WORK *ssw)
{
	GFL_STD_MemClear(&ssw->special_work, sizeof(STAMP_SPECIAL_WORK));
	ssw->special_effect_type = ssw->special_effect_req;
	ssw->special_effect_req = 0;
	ssw->special_effect_start_effect_end = 0;
}

//==============================================================================
//
//	動作
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   スタンプ動作タイプを設定する
 *
 * @param   move			スタンプへのポインタ
 * @param   move_type		動作タイプ
 *
 * @retval  TRUE:成功。　FALSE:失敗
 */
//--------------------------------------------------------------
static BOOL StampMoveTool_MoveTypeSet(STAMP_MOVE_PTR move, int move_type)
{
	if(move->move_type != 0 || move->next_move_type != 0 || move->initialize == 0){
		return FALSE;	//既に他のエフェクト動作をしている
	}
	move->next_move_type = move_type;
	GFL_STD_MemClear(&move->erase_eff, sizeof(ERASE_EFF_WORK));
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプパラメータから動作タイプを取り出す
 * @param   param		スタンプパラメータへのポインタ
 * @retval  動作タイプ
 */
//--------------------------------------------------------------
static u8 StampMoveTool_MoveTypeGet(const STAMP_PARAM *param)
{
	u8 seikaku;
	
	seikaku = POKETOOL_GetSeikaku(param->personal_rnd);
	return Seikaku_to_MoveType_Tbl[seikaku];
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ初期動作：一定時間経ったら各エフェクト動作に移行
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_FirstWait(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_INIT_MOVE *init_move = &move->erase_eff.init_move;
	
	init_move->wait++;
	if(init_move->wait == 2){
		Stamp_HitRectCreate(&move->s3d.obj, &move->param, &init_move->rect, FALSE);
		move->s3d.draw_flag = TRUE;	//テクスチャがVBlankで転送されてから描画ONする
		move->initialize = TRUE;
	}

	if(init_move->wait > STAMP_FIRST_WAIT){
		Stamp_ChainWorkGet(ssw, move);
		StampMoveTool_MoveTypeSet(move, StampMoveTool_MoveTypeGet(&move->param));
	}
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：一定時間経ったら各エフェクト動作に移行
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_FirstWait(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target)
{
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：3倍程度に拡大しながら背景に徐々に透過され消える
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_Nijimi(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_NIJIMI *nijimi = &move->erase_eff.nijimi;
	
	switch(nijimi->seq){
	case 0:
		nijimi->alpha = 31 << 8;
		nijimi->polygon_id = Stamp_PolygonIDGet(ssw);
		nijimi->affine_xyz = FX32_ONE;
		nijimi->seq++;
		//break;
	case 1:
		nijimi->affine_xyz += NIJIMI_ADD_AFFINE;
		if(nijimi->alpha - NIJIMI_ADD_ALPHA < 0x100){
			move->s3d.draw_flag = FALSE;
			nijimi->seq++;
			break;
		}
		nijimi->alpha -= NIJIMI_ADD_ALPHA;
		move->s3d.obj.scale.x = nijimi->affine_xyz;
		move->s3d.obj.scale.x = nijimi->affine_xyz;
		move->s3d.obj.scale.x = FX32_ONE;
		break;
	default:
		Stamp_PolygonIDFree(ssw, nijimi->polygon_id);
		return RET_DELETE;
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &nijimi->rect, TRUE);
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ描画前処理：3倍程度に拡大しながら背景に徐々に透過され消える
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 */
//--------------------------------------------------------------
static void StampDraw_Nijimi(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_NIJIMI *nijimi = &move->erase_eff.nijimi;

	if(nijimi->polygon_id != STAMP_POLYGON_ID_ERROR){
		NNS_G3dGlbPolygonAttr(0, 0, 0, nijimi->polygon_id, nijimi->alpha >> 8, 0);
		NNS_G3dMdlUseGlbPolygonID(move->s3d.pModel);
		NNS_G3dMdlUseGlbAlpha(move->s3d.pModel);
	}
#if WB_FIX
	D3DOBJ_Draw(&move->s3d.obj);
#else
  _StampMoveDraw(&move->s3d);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：3倍程度に拡大しながら背景に徐々に透過され消える
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_Nijimi(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target)
{
	ERASE_EFF_NIJIMI *nijimi = &move->erase_eff.nijimi;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;

	if(Stamp_RectHitCheck(&nijimi->rect, &init_move->rect) == TRUE){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：足跡が4分割し、4つのオブジェクトになて四方に飛び散って消える
 * 						オブジェクトの移動範囲は、押した点から32グリッド程度
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_Hajike(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_HAJIKE *hajike = &move->erase_eff.hajike;
	int i;
	
	switch(hajike->seq){
	case 0:
		for(i = 0; i < HAJIKE_OBJ_CHILD_NUM; i++){
	#if WB_FIX
			D3DOBJ_Init( &hajike->child_obj[i], &move->s3d.mdl );
	#else
	    _StampChildAdd(&move->s3d, &hajike->child_obj[i]);
	#endif
			hajike->child_obj[i].obj.trans = move->s3d.obj.trans;
		}
		hajike->seq++;
		//break;
	case 1:
		//本体は左上
		move->s3d.obj.trans.x -= HAJIKE_ADD_X;
		move->s3d.obj.trans.y -= HAJIKE_ADD_Y;
		//右上
		hajike->child_obj[0].obj.trans.x += HAJIKE_ADD_X;
		hajike->child_obj[0].obj.trans.y -= HAJIKE_ADD_Y;
		//左下
		hajike->child_obj[1].obj.trans.x -= HAJIKE_ADD_X;
		hajike->child_obj[1].obj.trans.y += HAJIKE_ADD_Y;
		//右下
		hajike->child_obj[2].obj.trans.x += HAJIKE_ADD_X;
		hajike->child_obj[2].obj.trans.y += HAJIKE_ADD_Y;
		
		hajike->frame++;
		if(hajike->frame > HAJIKE_DELETE_FRAME){
			return RET_DELETE;
		}
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &hajike->rect[0], FALSE);
	for(i = 0; i < HAJIKE_OBJ_CHILD_NUM; i++){
		Stamp_HitRectCreate(&hajike->child_obj[i].obj, &move->param, &hajike->rect[1 + i], FALSE);
	}
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ描画前処理：足跡が4分割し、4つのオブジェクトになて四方に飛び散って消える
 * 						オブジェクトの移動範囲は、押した点から32グリッド程度
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 */
//--------------------------------------------------------------
static void StampDraw_Hajike(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_HAJIKE *hajike = &move->erase_eff.hajike;
	int i;
	
	for(i = 0; i < HAJIKE_OBJ_CHILD_NUM; i++){
    _StampMoveDrawChild(&hajike->child_obj[i]);
	}
  _StampMoveDraw(&move->s3d);
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：足跡が4分割し、4つのオブジェクトになて四方に飛び散って消える
 * 						オブジェクトの移動範囲は、押した点から32グリッド程度
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_Hajike(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target)
{
	ERASE_EFF_HAJIKE *hajike = &move->erase_eff.hajike;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;
	int i;
	
	for(i = 0; i < HAJIKE_OBJ_CHILD_NUM + 1; i++){
		if(Stamp_RectHitCheck(&hajike->rect[i], &init_move->rect) == TRUE){
			return TRUE;
		}
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ削除：足跡が4分割し、4つのオブジェクトになて四方に飛び散って消える
 * 						オブジェクトの移動範囲は、押した点から32グリッド程度
 *
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static void StampDel_Hajike(STAMP_MOVE_PTR move)
{
	ERASE_EFF_HAJIKE *hajike = &move->erase_eff.hajike;
	int i;
	
	for(i = 0; i < HAJIKE_OBJ_CHILD_NUM; i++){
    if(hajike->child_obj[i].mdl != NULL){
      _StampChildDel(&move->s3d, &hajike->child_obj[i]);
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：タッチした点から水平-90度回転し、
 *						垂直右方向に蛇行移動し、フレームアウトする
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_Zigzag(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_ZIGZAG *zig = &move->erase_eff.zigzag;
	u16 rot, before_rot;

	switch(zig->seq){
	case 0:
#if WB_FIX
		rot = D3DOBJ_GetRota(&move->s3d.obj, D3DOBJ_ROTA_WAY_Z);
#else
    rot = move->s3d.obj_rotate.z;
#endif
		rot -= 65536/4/ZIGZAG_THETA_FRAME;
		if(rot <= 65536/8*7){
			//Stamp_TexDotErase(move->s3d.pMdlTex, move->move_type);
			Stamp_TexDotFlip(move->s3d.pMdlTex, move->move_type);
			Stamp_LoadTexDataSet(ssw, move->s3d.pResMdl);
			move->tex_load_req = TRUE;
			rot += 65536/4;
			zig->seq++;
		}
#if WB_FIX
		D3DOBJ_SetRota(&move->s3d.obj, rot, D3DOBJ_ROTA_WAY_Z);
#else
		move->s3d.obj_rotate.z = rot;
		Foot_RotateMtx(&move->s3d.obj_rotate, &move->s3d.obj.rotate);
#endif
		break;
	case 1:
		move->tex_load_req = 0;
#if WB_FIX
		rot = D3DOBJ_GetRota(&move->s3d.obj, D3DOBJ_ROTA_WAY_Z);
#else
    rot = move->s3d.obj_rotate.z;
#endif
		rot -= 65536/4/ZIGZAG_THETA_FRAME;
		if(rot >= 65536/4*3){
			rot = 0;
			zig->seq++;
		}
		
#if WB_FIX
		D3DOBJ_SetRota(&move->s3d.obj, rot, D3DOBJ_ROTA_WAY_Z);
#else
		move->s3d.obj_rotate.z = rot;
		Foot_RotateMtx(&move->s3d.obj_rotate, &move->s3d.obj.rotate);
#endif
		break;
	case 2:
		zig->wait++;
		if(zig->wait < ZIGZAG_THETA_AFTER_WAIT){
			break;
		}
		zig->wait = 0;
		zig->seq++;
		//break;
	case 3:
		if(move->s3d.obj.trans.x < StampFrameOutRect.left || move->s3d.obj.trans.x > StampFrameOutRect.right
				|| move->s3d.obj.trans.y > StampFrameOutRect.top || move->s3d.obj.trans.y < StampFrameOutRect.bottom){
			//OS_TPrintf("ジグザグ：フレームアウト x=%d, y=%d\n", fx_x, fx_y);
			return RET_DELETE;
		}
		
		if(zig->turn_frame < ZIGZAG_TURN_FRAME){
		  move->s3d.obj.trans.x += ZIGZAG_ADD_X;
		  move->s3d.obj.trans.y -= ZIGZAG_ADD_Y;
		}
		else{
		  move->s3d.obj.trans.x += ZIGZAG_ADD_X;
		  move->s3d.obj.trans.y += ZIGZAG_ADD_Y;
		}
		zig->turn_frame++;
		if(zig->turn_frame >= ZIGZAG_TURN_FRAME*2){
			zig->turn_frame = 0;
		}
		break;
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &zig->rect, FALSE);
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：タッチした点から水平-90度回転し、
 *						垂直右方向に蛇行移動し、フレームアウトする
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_Zigzag(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target)
{
	ERASE_EFF_ZIGZAG *zig = &move->erase_eff.zigzag;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;

	if(Stamp_RectHitCheck(&zig->rect, &init_move->rect) == TRUE){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：タッチした点から水平90度回転し、
 * 						垂直左方向に蛇行移動し、フレームアウトする
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_Dakou(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_DAKOU *dakou = &move->erase_eff.dakou;
	u16 rot;
	fx32 offset_y;
	
	switch(dakou->seq){
	case 0:
#if WB_FIX
		rot = D3DOBJ_GetRota(&move->s3d.obj, D3DOBJ_ROTA_WAY_Z);
#else
    rot = move->s3d.obj_rotate.z;
#endif
		rot += 65536/4/DAKOU_THETA_FRAME;
		if(rot >= 65536/4/2){
			Stamp_TexDotFlip(move->s3d.pMdlTex, move->move_type);
			rot -= 65536/4;
			Stamp_LoadTexDataSet(ssw, move->s3d.pResMdl);
			move->tex_load_req = TRUE;
			dakou->seq++;
		}
#if WB_FIX
		D3DOBJ_SetRota(&move->s3d.obj, rot, D3DOBJ_ROTA_WAY_Z);
#else
		move->s3d.obj_rotate.z = rot;
		Foot_RotateMtx(&move->s3d.obj_rotate, &move->s3d.obj.rotate);
#endif
		break;
	case 1:
		move->tex_load_req = 0;
#if WB_FIX
		rot = D3DOBJ_GetRota(&move->s3d.obj, D3DOBJ_ROTA_WAY_Z);
#else
    rot = move->s3d.obj_rotate.z;
#endif
		rot += 65536/4/DAKOU_THETA_FRAME;
		if(rot >= 0 && rot < 65536/4*1){
			rot = 0;
			dakou->seq++;
		}
#if WB_FIX
		D3DOBJ_SetRota(&move->s3d.obj, rot, D3DOBJ_ROTA_WAY_Z);
#else
		move->s3d.obj_rotate.z = rot;
		Foot_RotateMtx(&move->s3d.obj_rotate, &move->s3d.obj.rotate);
#endif
		break;
	case 2:
		dakou->wait++;
		if(dakou->wait < DAKOU_THETA_AFTER_WAIT){
			break;
		}
		dakou->center_y = move->s3d.obj.trans.y;
		dakou->wait = 0;
		dakou->seq++;
		//break;
	case 3:
		if(move->s3d.obj.trans.x < StampFrameOutRect.left || move->s3d.obj.trans.x > StampFrameOutRect.right
				|| move->s3d.obj.trans.y > StampFrameOutRect.top || move->s3d.obj.trans.y < StampFrameOutRect.bottom){
			//OS_TPrintf("蛇行：フレームアウト x=%d, y=%d\n", fx_x, fx_y);
			return RET_DELETE;
		}
		
		dakou->theta += DAKOU_ADD_THETA;
		if(dakou->theta >= (360 << FX32_SHIFT)){
			dakou->theta -= 360 << FX32_SHIFT;
		}
		offset_y = FX_Mul(GFL_CALC_Sin360FX(dakou->theta), DAKOU_FURIHABA_Y);// - DAKOU_FURIHABA_Y/2;
		move->s3d.obj.trans.x -= DAKOU_ADD_X;
		move->s3d.obj.trans.y = dakou->center_y + offset_y;
		break;
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &dakou->rect, FALSE);
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：タッチした点から水平90度回転し、
 * 						垂直左方向に蛇行移動し、フレームアウトする
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_Dakou(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target)
{
	ERASE_EFF_DAKOU *dakou = &move->erase_eff.dakou;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;

	if(Stamp_RectHitCheck(&dakou->rect, &init_move->rect) == TRUE){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：タッチした点から、垂直上方向に軌跡を残しながら移動、フレームアウトする
 * 						移動中、他のオブジェクトと接触するとそこで足跡(+軌跡)は消える
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_Kiseki(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_KISEKI *kiseki = &move->erase_eff.kiseki;
	int i;
	VecFx32 pos;
	
	switch(kiseki->seq){
	case 0:
		kiseki->polygon_id = Stamp_PolygonIDGet(ssw);

		for(i = 0; i < KISEKI_OBJ_CHILD_NUM; i++){
	#if WB_FIX
			D3DOBJ_Init( &kiseki->child_obj[i], &move->s3d.mdl );
	#else
	    _StampChildAdd(&move->s3d, &kiseki->child_obj[i]);
	#endif
	    kiseki->child_obj[i].obj.trans = move->s3d.obj.trans;
			kiseki->child_obj[i].draw_flag = FALSE;
		}
		kiseki->seq++;
		//break;
	case 1:
	  pos = move->s3d.obj.trans;
		if(move->s3d.obj.trans.x < StampFrameOutRect.left 
		    || move->s3d.obj.trans.x > StampFrameOutRect.right
				|| move->s3d.obj.trans.y > StampFrameOutRect.top 
				|| move->s3d.obj.trans.y < StampFrameOutRect.bottom
				|| kiseki->obj_hit == TRUE){
			//OS_TPrintf("軌跡：フレームアウト x=%d, y=%d\n", fx_x, fx_y);
			for(i = 0; i < KISEKI_OBJ_CHILD_NUM; i++){
        kiseki->child_obj[i].draw_flag = FALSE;
			}
			move->s3d.draw_flag = FALSE;
			kiseki->seq++;
			break;
		}

    move->s3d.obj.trans.y += KISEKI_ADD_Y;
		
		if(kiseki->frame % KISEKI_DROP_FRAME == 0){
      kiseki->child_obj[kiseki->drop_no].obj.trans.x = pos.x;
      kiseki->child_obj[kiseki->drop_no].obj.trans.y = pos.y + KISEKI_ADD_Y;
      kiseki->child_obj[kiseki->drop_no].obj.trans.z = pos.z;
			kiseki->child_obj[kiseki->drop_no].draw_flag = TRUE;
			kiseki->drop_no++;
			if(kiseki->drop_no >= KISEKI_OBJ_CHILD_NUM){
				kiseki->drop_no = 0;
			}
		}
		kiseki->frame++;
		break;
	default:
		Stamp_PolygonIDFree(ssw, kiseki->polygon_id);
		return RET_DELETE;
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &kiseki->rect, FALSE);
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ描画前処理：タッチした点から、垂直上方向に軌跡を残しながら移動、
 * 						フレームアウトする移動中、他のオブジェクトと接触すると
 *						そこで足跡(+軌跡)は消える
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 */
//--------------------------------------------------------------
static void StampDraw_Kiseki(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_KISEKI *kiseki = &move->erase_eff.kiseki;
	int i;
	
	//子
	if(kiseki->polygon_id != STAMP_POLYGON_ID_ERROR){
		NNS_G3dGlbPolygonAttr(0, 0, 0, kiseki->polygon_id, KISEKI_ALPHA, 0);
		NNS_G3dMdlUseGlbPolygonID(move->s3d.pModel);
		NNS_G3dMdlUseGlbAlpha(move->s3d.pModel);
	}
	for(i = 0; i < KISEKI_OBJ_CHILD_NUM; i++){
    _StampMoveDrawChild(&kiseki->child_obj[i]);
	}
	
	//親	半透明の濃度を元に戻してから描画
	if(kiseki->polygon_id != STAMP_POLYGON_ID_ERROR){
		NNS_G3dGlbPolygonAttr(0, 0, 0, kiseki->polygon_id, 31, 0);
		NNS_G3dMdlUseGlbPolygonID(move->s3d.pModel);
		NNS_G3dMdlUseGlbAlpha(move->s3d.pModel);
	}
  _StampMoveDraw(&move->s3d);
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：タッチした点から、垂直上方向に軌跡を残しながら移動、
 * 						フレームアウトする移動中、他のオブジェクトと接触すると
 *						そこで足跡(+軌跡)は消える
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_Kiseki(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target)
{
	ERASE_EFF_KISEKI *kiseki = &move->erase_eff.kiseki;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;

	if(Stamp_RectHitCheck(&kiseki->rect, &init_move->rect) == TRUE){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ削除：タッチした点から、垂直上方向に軌跡を残しながら移動、フレームアウトする
 * 						移動中、他のオブジェクトと接触するとそこで足跡(+軌跡)は消える
 *
 * @param   move		スタンプへのポインタ
 */
//--------------------------------------------------------------
static void StampDel_Kiseki(STAMP_MOVE_PTR move)
{
	ERASE_EFF_KISEKI *kiseki = &move->erase_eff.kiseki;
	int i;
	
	for(i = 0; i < KISEKI_OBJ_CHILD_NUM; i++){
    if(kiseki->child_obj[i].mdl != NULL){
      _StampChildDel(&move->s3d, &kiseki->child_obj[i]);
    }
	}
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：タッチした点から垂直下方向に足跡が、
 * 						左右に振れながら移動しフレームアウトする
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_Yure(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_YURE *yure = &move->erase_eff.yure;
	u16 rot;
	fx32 offset_x;
	
	switch(yure->seq){
	case 0:
		yure->center_x = move->s3d.obj.trans.x;
		yure->seq++;
		//break;
	case 1:
		if(move->s3d.obj.trans.x < StampFrameOutRect.left || move->s3d.obj.trans.x > StampFrameOutRect.right
				|| move->s3d.obj.trans.y > StampFrameOutRect.top || move->s3d.obj.trans.y < StampFrameOutRect.bottom){
			//OS_TPrintf("揺れ：フレームアウト x=%d, y=%d\n", fx_x, fx_y);
			return RET_DELETE;
		}
		
		yure->theta += YURE_ADD_THETA;
		if(yure->theta >= (360 << FX32_SHIFT)){
			yure->theta -= 360 << FX32_SHIFT;
		}
		offset_x = FX_Mul(GFL_CALC_Sin360FX(yure->theta), YURE_FURIHABA_X);
		move->s3d.obj.trans.x = yure->center_x + offset_x;
		move->s3d.obj.trans.y -= YURE_ADD_Y;
		break;
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &yure->rect, FALSE);
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：タッチした点から垂直下方向に足跡が、
 * 						左右に振れながら移動しフレームアウトする
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_Yure(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target)
{
	ERASE_EFF_YURE *yure = &move->erase_eff.yure;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;

	if(Stamp_RectHitCheck(&yure->rect, &init_move->rect) == TRUE){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：一気に4倍程度(64x64)まで拡大してから一瞬で消える
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_Kakudai(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_KAKUDAI *kakudai = &move->erase_eff.kakudai;
	
	switch(kakudai->seq){
	case 0:
		kakudai->affine_xyz = FX32_ONE;
		kakudai->seq++;
		//break;
	case 1:
		kakudai->frame++;
		if(kakudai->frame > KAKUDAI_DELETE_FRAME){
			return RET_DELETE;
		}
		kakudai->affine_xyz += KAKUDAI_ADD_AFFINE;
	#if WB_FIX
		D3DOBJ_SetScale(&move->s3d.obj, kakudai->affine_xyz, kakudai->affine_xyz, FX32_ONE);
	#else
    VEC_Set(&move->s3d.obj.scale, kakudai->affine_xyz, kakudai->affine_xyz, FX32_ONE);
	#endif
		break;
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &kakudai->rect, TRUE);
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：一気に4倍程度(64x64)まで拡大してから一瞬で消える
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_Kakudai(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target)
{
	ERASE_EFF_KAKUDAI *kakudai = &move->erase_eff.kakudai;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;

	if(Stamp_RectHitCheck(&kakudai->rect, &init_move->rect) == TRUE){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：タッチした点を中心に横軸にブラーしながら徐々に消える。範囲は3倍程度
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_BrarX(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_BRAR_X *brar_x = &move->erase_eff.brar_x;
	fx32 offset;
	int i;
	
	switch(brar_x->seq){
	case 0:
		brar_x->polygon_id = Stamp_PolygonIDGet(ssw);

		for(i = 0; i < BRAR_X_OBJ_CHILD_NUM; i++){
	#if WB_FIX
			D3DOBJ_Init( &brar_x->child_obj[i], &move->s3d.mdl );
	#else
	    _StampChildAdd(&move->s3d, &brar_x->child_obj[i]);
	#endif
	    brar_x->child_obj[i].obj.trans = move->s3d.obj.trans;
			brar_x->child_obj[i].draw_flag = TRUE;
		}
		brar_x->alpha = 31 << 8;
		brar_x->seq++;
		//break;
	case 1:
		//ALPHA
		if(brar_x->alpha - BRAR_X_ADD_ALPHA < 0x100){
			for(i = 0; i < BRAR_X_OBJ_CHILD_NUM; i++){
				brar_x->child_obj[i].draw_flag = FALSE;
			}
			move->s3d.draw_flag = FALSE;
			brar_x->seq++;
			break;
		}
		brar_x->alpha -= BRAR_X_ADD_ALPHA;
		
		//SIN, THETA
		brar_x->theta += BRAR_X_ADD_THETA;
		if(brar_x->theta >= (360 << FX32_SHIFT)){
			brar_x->theta -= 360 << FX32_SHIFT;
		}
		offset = FX_Mul(GFL_CALC_Sin360FX(brar_x->theta), BRAR_X_FURIHABA);

		//POS SET
		for(i = 0; i < BRAR_X_OBJ_CHILD_NUM; i++){
			if(i & 1){
				brar_x->child_obj[i].obj.trans.x = move->s3d.obj.trans.x + offset;
				brar_x->child_obj[i].obj.trans.y = move->s3d.obj.trans.y;
				brar_x->child_obj[i].obj.trans.z = move->s3d.obj.trans.z;
			}
			else{
				brar_x->child_obj[i].obj.trans.x = move->s3d.obj.trans.x - offset;
				brar_x->child_obj[i].obj.trans.y = move->s3d.obj.trans.y;
				brar_x->child_obj[i].obj.trans.z = move->s3d.obj.trans.z;
			}
		}
		break;
	default:
		Stamp_PolygonIDFree(ssw, brar_x->polygon_id);
		return RET_DELETE;
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &brar_x->rect[0], FALSE);
	for(i = 0; i < BRAR_X_OBJ_CHILD_NUM; i++){
		Stamp_HitRectCreate(&brar_x->child_obj[i].obj, &move->param, &brar_x->rect[1 + i], FALSE);
	}
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ描画前処理：タッチした点を中心に横軸にブラーしながら徐々に消える。範囲は3倍程度
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 */
//--------------------------------------------------------------
static void StampDraw_BrarX(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_BRAR_X *brar_x = &move->erase_eff.brar_x;
	int i;
	
	if(brar_x->polygon_id != STAMP_POLYGON_ID_ERROR){
		NNS_G3dGlbPolygonAttr(0, 0, 0, brar_x->polygon_id, brar_x->alpha >> 8, 0);
		NNS_G3dMdlUseGlbPolygonID(move->s3d.pModel);
		NNS_G3dMdlUseGlbAlpha(move->s3d.pModel);
	}
	
	for(i = 0; i < BRAR_X_OBJ_CHILD_NUM; i++){
    _StampMoveDrawChild(&brar_x->child_obj[i]);
	}
  _StampMoveDraw(&move->s3d);
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：タッチした点を中心に横軸にブラーしながら徐々に消える。
 *									範囲は3倍程度
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_BrarX(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target)
{
	ERASE_EFF_BRAR_X *brar_x = &move->erase_eff.brar_x;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;
	int i;
	
	for(i = 0; i < BRAR_X_OBJ_CHILD_NUM + 1; i++){
		if(Stamp_RectHitCheck(&brar_x->rect[i], &init_move->rect) == TRUE){
			return TRUE;
		}
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ削除：タッチした点を中心に横軸にブラーしながら徐々に消える。範囲は3倍程度
 *
 * @param   move		スタンプへのポインタ
 */
//--------------------------------------------------------------
static void StampDel_BrarX(STAMP_MOVE_PTR move)
{
	ERASE_EFF_BRAR_X *brar_x = &move->erase_eff.brar_x;
	int i;
	
	for(i = 0; i < BRAR_X_OBJ_CHILD_NUM; i++){
    if(brar_x->child_obj[i].mdl != NULL){
      _StampChildDel(&move->s3d, &brar_x->child_obj[i]);
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：タッチした点を中心に縦軸にブラーしながら徐々に消える。範囲は3倍程度
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_BrarY(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_BRAR_Y *brar_y = &move->erase_eff.brar_y;
	fx32 offset;
	int i;
	
	switch(brar_y->seq){
	case 0:
		brar_y->polygon_id = Stamp_PolygonIDGet(ssw);

		for(i = 0; i < BRAR_Y_OBJ_CHILD_NUM; i++){
		#if WB_FIX
			D3DOBJ_Init( &brar_y->child_obj[i], &move->s3d.mdl );
		#else
	    _StampChildAdd(&move->s3d, &brar_y->child_obj[i]);
		#endif
		  brar_y->child_obj[i].obj.trans = move->s3d.obj.trans;
			brar_y->child_obj[i].draw_flag = TRUE;
		}
		brar_y->alpha = 31 << 8;
		brar_y->seq++;
		//break;
	case 1:
		//ALPHA
		if(brar_y->alpha - BRAR_Y_ADD_ALPHA < 0x100){
			for(i = 0; i < BRAR_Y_OBJ_CHILD_NUM; i++){
				brar_y->child_obj[i].draw_flag = FALSE;
			}
			move->s3d.draw_flag = FALSE;
			brar_y->seq++;
			break;
		}
		brar_y->alpha -= BRAR_Y_ADD_ALPHA;
		
		//SIN, THETA
		brar_y->theta += BRAR_Y_ADD_THETA;
		if(brar_y->theta >= (360 << FX32_SHIFT)){
			brar_y->theta -= 360 << FX32_SHIFT;
		}
		offset = FX_Mul(GFL_CALC_Sin360FX(brar_y->theta), BRAR_Y_FURIHABA);

		//POS SET
		for(i = 0; i < BRAR_Y_OBJ_CHILD_NUM; i++){
			if(i & 1){
        brar_y->child_obj[i].obj.trans.x = move->s3d.obj.trans.x;
        brar_y->child_obj[i].obj.trans.y = move->s3d.obj.trans.y + offset;
        brar_y->child_obj[i].obj.trans.z = move->s3d.obj.trans.z;
			}
			else{
        brar_y->child_obj[i].obj.trans.x = move->s3d.obj.trans.x;
        brar_y->child_obj[i].obj.trans.y = move->s3d.obj.trans.y - offset;
        brar_y->child_obj[i].obj.trans.z = move->s3d.obj.trans.z;
			}
		}
		break;
	default:
		Stamp_PolygonIDFree(ssw, brar_y->polygon_id);
		return RET_DELETE;
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &brar_y->rect[0], FALSE);
	for(i = 0; i < BRAR_Y_OBJ_CHILD_NUM; i++){
		Stamp_HitRectCreate(&brar_y->child_obj[i].obj, &move->param, &brar_y->rect[1 + i], FALSE);
	}
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ描画前処理：タッチした点を中心に縦軸にブラーしながら徐々に消える。範囲は3倍程度
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 */
//--------------------------------------------------------------
static void StampDraw_BrarY(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_BRAR_Y *brar_y = &move->erase_eff.brar_y;
	int i;
	
	if(brar_y->polygon_id != STAMP_POLYGON_ID_ERROR){
		NNS_G3dGlbPolygonAttr(0, 0, 0, brar_y->polygon_id, brar_y->alpha >> 8, 0);
		NNS_G3dMdlUseGlbPolygonID(move->s3d.pModel);
		NNS_G3dMdlUseGlbAlpha(move->s3d.pModel);
	}
	
	for(i = 0; i < BRAR_Y_OBJ_CHILD_NUM; i++){
    _StampMoveDrawChild(&brar_y->child_obj[i]);
	}
  _StampMoveDraw(&move->s3d);
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：タッチした点を中心に縦軸にブラーしながら徐々に消える。
 *									範囲は3倍程度
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_BrarY(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move, STAMP_MOVE_PTR target)
{
	ERASE_EFF_BRAR_Y *brar_y = &move->erase_eff.brar_y;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;
	int i;
	
	for(i = 0; i < BRAR_Y_OBJ_CHILD_NUM + 1; i++){
		if(Stamp_RectHitCheck(&brar_y->rect[i], &init_move->rect) == TRUE){
			return TRUE;
		}
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ削除：タッチした点を中心に縦軸にブラーしながら徐々に消える。範囲は3倍程度
 *
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static void StampDel_BrarY(STAMP_MOVE_PTR move)
{
	ERASE_EFF_BRAR_Y *brar_y = &move->erase_eff.brar_y;
	int i;
	
	for(i = 0; i < BRAR_Y_OBJ_CHILD_NUM; i++){
    if(brar_y->child_obj[i].mdl != NULL){
      _StampChildDel(&move->s3d, &brar_y->child_obj[i]);
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   スタンプ動作：タッチした点から下方向に64dot程度拡大して消える。
 * 							ペンキがたれるようなイメージ
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 *
 * @retval	システムへ返す返事(RET_???)
 */
//--------------------------------------------------------------
static STAMP_RET StampMove_Tare(STAMP_SYSTEM_WORK *ssw, STAMP_MOVE_PTR move)
{
	ERASE_EFF_TARE *tare = &move->erase_eff.tare;
	fx32 offset;
	
	switch(tare->seq){
	case 0:
		tare->affine_xyz = FX32_ONE;
		tare->seq++;
		//break;
	case 1:
		tare->frame++;
		if(tare->frame > TARE_DELETE_FRAME){
			return RET_DELETE;
		}
		tare->affine_xyz += TARE_ADD_AFFINE;
	#if WB_FIX
		D3DOBJ_SetScale(&move->s3d.obj, FX32_ONE, tare->affine_xyz, FX32_ONE);
	#else
    VEC_Set(&move->s3d.obj.scale, FX32_ONE, tare->affine_xyz, FX32_ONE);
	#endif
	
	//	offset = 128 - (128 * scale_x / FX32_ONE);
		offset = -(FX32_ONE*2/2 * (tare->affine_xyz - FX32_ONE) / FX32_ONE);
		move->s3d.obj.trans.y += offset;
		break;
	}

	Stamp_HitRectCreate(&move->s3d.obj, &move->param, &tare->rect, TRUE);
	return RET_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   スタンプヒットチェック：タッチした点から下方向に64dot程度拡大して消える。
 * 							ペンキがたれるようなイメージ
 *
 * @param   ssw			スタンプシステムワークへのポインタ
 * @param   move		スタンプへのポインタ
 * @param   target		まだノーマル状態のスタンプへのポインタ
 *
 * @retval  TRUE:あたり判定発生
 * @retval  FALSE:あたり判定無し
 */
//--------------------------------------------------------------
static BOOL StampHitcheck_Tare(STAMP_SYSTEM_WORK *ssw,STAMP_MOVE_PTR move,STAMP_MOVE_PTR target)
{
	ERASE_EFF_TARE *tare = &move->erase_eff.tare;
	ERASE_EFF_INIT_MOVE *init_move = &target->erase_eff.init_move;

	if(Stamp_RectHitCheck(&tare->rect, &init_move->rect) == TRUE){
		return TRUE;
	}
	return FALSE;
}


//==============================================================================
//	スペシャルエフェクト発動前のフラッシュ
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト発動前のフラッシュエフェクト
 *
 * @param   ssw				スタンプシステムワークへのポインタ
 * @param   flash_work		フラッシュワーク
 * @param   game_status		ゲームステータス
 * @param   board_type		ボードタイプ
 *
 * @retval  TRUE:エフェクト終了。
 * @retval  FALSE:エフェクト継続中
 */
//--------------------------------------------------------------
static BOOL SpecialFlashEff(STAMP_SYSTEM_WORK *ssw, SPECIAL_FLASH_WORK *flash_work, int game_status, int board_type)
{
	int white_black;
	
	if(game_status == FOOTPRINT_GAME_STATUS_FINISH){
		return FALSE;
	}
	
	if(board_type == FOOTPRINT_BOARD_TYPE_WHITE){
		white_black = -1;
	}
	else{
		white_black = 1;
	}
	
	switch(flash_work->seq){
	case 0:
		flash_work->evy += SPECIAL_FLASH_ADD_EVY;
		if(flash_work->evy >= SPECIAL_FLASH_EVY_MAX){
			flash_work->evy = SPECIAL_FLASH_EVY_MAX;
			flash_work->seq++;
		}
		ssw->v_wipe_req = TRUE;
		ssw->v_wipe_no = (flash_work->evy >> 8) * white_black;
		break;
	case 1:
		flash_work->evy -= SPECIAL_FLASH_ADD_EVY;
		if(flash_work->evy <= 0){
			flash_work->evy = 0;
			flash_work->loop++;
			if(flash_work->loop < SPECIAL_FLASH_LOOP){
				flash_work->seq = 0;
			}
			else{
				flash_work->seq++;
			}
		}
		ssw->v_wipe_req = TRUE;
		ssw->v_wipe_no = (flash_work->evy >> 8) * white_black;
		break;
	default:
		ssw->special_effect_start_effect_end = TRUE;
		return TRUE;
	}
	
	return FALSE;
}

//==============================================================================
//	スペシャルエフェクト
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：カメラをグーッと寄って戻す
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_Nijimi(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_NIJIMI *sp_nijimi = &sp->sp_nijimi;
	
	switch(sp_nijimi->seq){
	case 0:
#if WB_FIX
		sp_nijimi->default_len = GFC_GetCameraDistance(camera_ptr);
#else
    sp_nijimi->default_len = *(ssw->camera_distance);
#endif
		sp_nijimi->seq++;
		PMSND_PlaySE(FOOTPRINT_SE_SP_NIJIMI);
		//break;
	case 1:
		_AddCameraDistance(ssw, SP_NIJIMI_FORWARD_ADD_LEN, camera_ptr);
		sp_nijimi->wait++;
		if(sp_nijimi->wait >= SP_NIJIMI_FORWARD_SYNC){
			sp_nijimi->wait = 0;
			sp_nijimi->seq++;
		}
		break;
	case 2:
		sp_nijimi->wait++;
		if(sp_nijimi->wait > SP_NIJIMI_WAIT_SYNC){
			sp_nijimi->wait = 0;
			sp_nijimi->seq++;
		}
		break;
	case 3:
		_AddCameraDistance(ssw, SP_NIJIMI_BACK_ADD_LEN, camera_ptr);
		sp_nijimi->wait++;
		if(sp_nijimi->wait >= SP_NIJIMI_BACK_SYNC 
				|| (*(ssw->camera_distance)) >= sp_nijimi->default_len){
			_SetCameraDistance(ssw, sp_nijimi->default_len, camera_ptr);
			GFL_G3D_CAMERA_Switching(camera_ptr);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：カメラ回転
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_Hajike(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_HAJIKE *sp_hajike = &sp->sp_hajike;
	fx32 value;
	
	switch(sp_hajike->seq){
	case 0:
		PMSND_PlaySE(FOOTPRINT_SE_SP_HAJIKE);
		sp_hajike->seq++;
		//break;
	case 1:
		sp_hajike->count++;
		value = FX32_ONE*4 / SP_HAJIKE_ROTATION_SYNC * sp_hajike->count;
		
		// Y
		if(value < FX32_ONE*2){
			sp_hajike->up_vec.y = FX32_ONE - value;
		}
		else{
			sp_hajike->up_vec.y = -FX32_ONE + (value - FX32_ONE*2);
		}
		
		// X
		if(value < FX32_ONE*1){
			sp_hajike->up_vec.x = value;
		}
		else if(value < FX32_ONE*2){
			sp_hajike->up_vec.x = FX32_ONE - (value - FX32_ONE);
		}
		else if(value < FX32_ONE*3){
			sp_hajike->up_vec.x = -(value - FX32_ONE*2);
		}
		else{
			sp_hajike->up_vec.x = -FX32_ONE + (value - FX32_ONE*3);
		}
		
		if(sp_hajike->count >= SP_HAJIKE_ROTATION_SYNC){
			sp_hajike->up_vec.y = FX32_ONE;
			sp_hajike->up_vec.x = 0;
			sp_hajike->count = 0;
			sp_hajike->loop++;
			if(sp_hajike->loop >= SP_HAJIKE_ROTATION_NUM){
				sp_hajike->seq++;
			}
		}
		GFL_G3D_CAMERA_SetCamUp(camera_ptr, &sp_hajike->up_vec);
  	GFL_G3D_CAMERA_Switching(camera_ptr);
		break;
	default:
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：画面全体が右に傾く
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_Zigzag(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_ZIGZAG *sp_zigzag = &sp->sp_zigzag;
	FOOT_CAMERA_ANGLE angle = {0,0,0,0};
	
	switch(sp_zigzag->seq){
	case 0:
		sp_zigzag->default_angle = *(ssw->camera_angle);
		sp_zigzag->calc_angle_y = sp_zigzag->default_angle.y;
		sp_zigzag->seq++;
		PMSND_PlaySE(FOOTPRINT_SE_SP_ZIGZAG);
		//break;
	case 1:
		angle.y += SP_ZIGZAG_ADD_ANGLE;
		sp_zigzag->calc_angle_y += SP_ZIGZAG_ADD_ANGLE;
		_AddCameraAngleRev(ssw, &angle, camera_ptr);
		
		if(sp_zigzag->calc_angle_y <= sp_zigzag->default_angle.y - SP_ZIGZAG_ANGLE_MAX){
			sp_zigzag->seq++;
		}
		break;
	case 2:	
		sp_zigzag->wait++;
		if(sp_zigzag->wait >= SP_ZIGZAG_WAIT){
			sp_zigzag->seq++;
		}
		break;
	case 3:
		angle.y += SP_ZIGZAG_SUB_ANGLE;
		sp_zigzag->calc_angle_y += SP_ZIGZAG_SUB_ANGLE;
		_AddCameraAngleRev(ssw, &angle, camera_ptr);
		
		if(sp_zigzag->calc_angle_y >= sp_zigzag->default_angle.y){
			_SetCameraAngleRev(ssw, &sp_zigzag->default_angle, camera_ptr);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：画面全体が左に傾く
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_Dakou(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_DAKOU *sp_dakou = &sp->sp_dakou;
	FOOT_CAMERA_ANGLE angle = {0,0,0,0};
	
	switch(sp_dakou->seq){
	case 0:
		sp_dakou->default_angle = *(ssw->camera_angle);
		sp_dakou->calc_angle_y = sp_dakou->default_angle.y;
		sp_dakou->seq++;
		PMSND_PlaySE(FOOTPRINT_SE_SP_DAKOU);
		//break;
	case 1:
		angle.y += SP_DAKOU_ADD_ANGLE;
		sp_dakou->calc_angle_y += SP_DAKOU_ADD_ANGLE;
		_AddCameraAngleRev(ssw, &angle, camera_ptr);
		
		if(sp_dakou->calc_angle_y >= sp_dakou->default_angle.y + SP_DAKOU_ANGLE_MAX){
			sp_dakou->seq++;
		}
		break;
	case 2:	
		sp_dakou->wait++;
		if(sp_dakou->wait >= SP_DAKOU_WAIT){
			sp_dakou->seq++;
		}
		break;
	case 3:
		angle.y += SP_DAKOU_SUB_ANGLE;
		sp_dakou->calc_angle_y += SP_DAKOU_SUB_ANGLE;
		_AddCameraAngleRev(ssw, &angle, camera_ptr);
		
		if(sp_dakou->calc_angle_y <= sp_dakou->default_angle.y){
			_SetCameraAngleRev(ssw, &sp_dakou->default_angle, camera_ptr);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：画面全体が上に傾く
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_Kiseki(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_KISEKI *sp_kiseki = &sp->sp_kiseki;
	FOOT_CAMERA_ANGLE angle = {0,0,0,0};
	
	switch(sp_kiseki->seq){
	case 0:
		sp_kiseki->default_angle = *(ssw->camera_angle);
		sp_kiseki->calc_angle_x = sp_kiseki->default_angle.x;
		PMSND_PlaySE(FOOTPRINT_SE_SP_KISEKI);
		sp_kiseki->seq++;
		//break;
	case 1:
		angle.x += SP_KISEKI_ADD_ANGLE;
		sp_kiseki->calc_angle_x += SP_KISEKI_ADD_ANGLE;
		_AddCameraAngleRev(ssw, &angle, camera_ptr);
		
		if(sp_kiseki->calc_angle_x >= sp_kiseki->default_angle.x + SP_KISEKI_ANGLE_MAX){
			sp_kiseki->seq++;
		}
		break;
	case 2:	
		sp_kiseki->wait++;
		if(sp_kiseki->wait >= SP_KISEKI_WAIT){
			sp_kiseki->seq++;
		}
		break;
	case 3:
		angle.x += SP_KISEKI_SUB_ANGLE;
		sp_kiseki->calc_angle_x += SP_KISEKI_SUB_ANGLE;
		_AddCameraAngleRev(ssw, &angle, camera_ptr);
		
		if(sp_kiseki->calc_angle_x <= sp_kiseki->default_angle.x){
			_SetCameraAngleRev(ssw, &sp_kiseki->default_angle, camera_ptr);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：左右の動きだけ揺れスタンプの動きをする
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_Yure(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_YURE *sp_yure = &sp->sp_yure;
	fx32 offset;
	FOOT_CAMERA_ANGLE angle = {0,0,0,0};
	
	switch(sp_yure->seq){
	case 0:
		sp_yure->default_angle = *(ssw->camera_angle);
		sp_yure->seq++;
		PMSND_PlaySE(FOOTPRINT_SE_SP_YURE);
		//break;
	case 1:
		sp_yure->theta += SP_YURE_ADD_THETA;
		if(sp_yure->theta >= (360 << FX32_SHIFT)){
			sp_yure->theta -= 360 << FX32_SHIFT;
			sp_yure->loop++;
			if(sp_yure->loop >= SP_YURE_LOOP_NUM){
				sp_yure->seq++;
				sp_yure->theta = 0;
			}
		}
		offset = FX_Mul(GFL_CALC_Sin360FX(sp_yure->theta), SP_YURE_FURIHABA_X);

		angle = sp_yure->default_angle;
		angle.y = offset;
		_SetCameraAngleRot(ssw, &angle, camera_ptr);
		break;
	default:
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：カメラをぐーっと引いて戻す
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_Kakudai(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_KAKUDAI *sp_kakudai = &sp->sp_kakudai;
	
	switch(sp_kakudai->seq){
	case 0:
		sp_kakudai->default_len = *(ssw->camera_distance);
		sp_kakudai->seq++;
		PMSND_PlaySE(FOOTPRINT_SE_SP_KAKUDAI);
		//break;
	case 1:
		_AddCameraDistance(ssw, SP_KAKUDAI_FORWARD_ADD_LEN, camera_ptr);
		sp_kakudai->wait++;
		if(sp_kakudai->wait >= SP_KAKUDAI_FORWARD_SYNC){
			sp_kakudai->wait = 0;
			sp_kakudai->seq++;
		}
		break;
	case 2:
		sp_kakudai->wait++;
		if(sp_kakudai->wait > SP_KAKUDAI_WAIT_SYNC){
			sp_kakudai->wait = 0;
			sp_kakudai->seq++;
		}
		break;
	case 3:
		_AddCameraDistance(ssw, SP_KAKUDAI_BACK_ADD_LEN, camera_ptr);
		sp_kakudai->wait++;
		if(sp_kakudai->wait >= SP_KAKUDAI_BACK_SYNC 
				|| (*(ssw->camera_distance)) <= sp_kakudai->default_len){
			_SetCameraDistance(ssw, sp_kakudai->default_len, camera_ptr);
			GFL_G3D_CAMERA_Switching(camera_ptr);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：カメラをスタンプと同じように動かす
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_BrarX(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_BRAR_X *sp_brar_x = &sp->sp_brar_x;
	VecFx32 move = {0,0,0};
	fx32 offset_x;
	
	switch(sp_brar_x->seq){
	case 0:
	#if WB_FIX
		sp_brar_x->default_pos = GFC_GetCameraPos(camera_ptr);
		sp_brar_x->default_target = GFC_GetLookTarget(camera_ptr);
	#else
	  GFL_G3D_CAMERA_GetPos(camera_ptr, &sp_brar_x->default_pos);
	  GFL_G3D_CAMERA_GetTarget(camera_ptr, &sp_brar_x->default_target);
	#endif
		sp_brar_x->seq++;
		PMSND_PlaySE(FOOTPRINT_SE_SP_BRAR_X);
		//break;
	case 1:
		sp_brar_x->theta += SP_BRAR_X_ADD_THETA;
		if(sp_brar_x->theta >= (360 << FX32_SHIFT)){
			sp_brar_x->theta -= 360 << FX32_SHIFT;
			sp_brar_x->loop++;
			if(sp_brar_x->loop >= SP_BRAR_X_LOOP_NUM){
				sp_brar_x->seq++;
				sp_brar_x->theta = 0;
			}
		}
		offset_x = FX_Mul(GFL_CALC_Sin360FX(sp_brar_x->theta), SP_BRAR_X_FURIHABA_X);

		move.x = offset_x;
#if WB_FIX
		GFC_SetLookTarget(&sp_brar_x->default_target, camera_ptr);
		GFC_SetCameraPos(&sp_brar_x->default_pos, camera_ptr);
		GFC_ShiftCamera(&move, camera_ptr);
#else
    {
      VecFx32 pos, target;
      
      VEC_Add(&sp_brar_x->default_pos, &move, &pos);
      VEC_Add(&sp_brar_x->default_target, &move, &target);
      GFL_G3D_CAMERA_SetTarget(camera_ptr, &target);
      GFL_G3D_CAMERA_SetPos(camera_ptr, &pos);
    	GFL_G3D_CAMERA_Switching(camera_ptr);
    }
#endif
		break;
	default:
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：カメラをスタンプと同じように動かす
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_BrarY(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_BRAR_Y *sp_brar_y = &sp->sp_brar_y;
	VecFx32 move = {0,0,0};
	fx32 offset_y;
	
	switch(sp_brar_y->seq){
	case 0:
#if WB_FIX
		sp_brar_y->default_pos = GFC_GetCameraPos(camera_ptr);
		sp_brar_y->default_target = GFC_GetLookTarget(camera_ptr);
#else
	  GFL_G3D_CAMERA_GetPos(camera_ptr, &sp_brar_y->default_pos);
	  GFL_G3D_CAMERA_GetTarget(camera_ptr, &sp_brar_y->default_target);
#endif
		sp_brar_y->seq++;
		PMSND_PlaySE(FOOTPRINT_SE_SP_BRAR_Y);
		//break;
	case 1:
		sp_brar_y->theta += SP_BRAR_Y_ADD_THETA;
		if(sp_brar_y->theta >= (360 << FX32_SHIFT)){
			sp_brar_y->theta -= 360 << FX32_SHIFT;
			sp_brar_y->loop++;
			if(sp_brar_y->loop >= SP_BRAR_Y_LOOP_NUM){
				sp_brar_y->seq++;
				sp_brar_y->theta = 0;
			}
		}
		offset_y = FX_Mul(GFL_CALC_Sin360FX(sp_brar_y->theta), SP_BRAR_Y_FURIHABA_Y);

		move.y = offset_y;
#if WB_FIX
		GFC_SetLookTarget(&sp_brar_y->default_target, camera_ptr);
		GFC_SetCameraPos(&sp_brar_y->default_pos, camera_ptr);
		GFC_ShiftCamera(&move, camera_ptr);
#else
    {
      VecFx32 pos, target;
      
      VEC_Add(&sp_brar_y->default_pos, &move, &pos);
      VEC_Add(&sp_brar_y->default_target, &move, &target);
      GFL_G3D_CAMERA_SetTarget(camera_ptr, &target);
      GFL_G3D_CAMERA_SetPos(camera_ptr, &pos);
      GFL_G3D_CAMERA_Switching(camera_ptr);
    }
#endif
		break;
	default:
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   スペシャルエフェクト：画面全体が下に傾く
 *
 * @param   ssw		スタンプシステムワークへのポインタ
 * @param   sp		スペシャルワークへのポインタ
 *
 * @retval  TRUE:終了。　FALSE:エフェクト中
 */
//--------------------------------------------------------------
static BOOL SpecialMove_Tare(STAMP_SYSTEM_WORK *ssw, STAMP_SPECIAL_WORK *sp, GFL_G3D_CAMERA * camera_ptr)
{
	SPECIAL_EFF_TARE *sp_tare = &sp->sp_tare;
	FOOT_CAMERA_ANGLE angle = {0,0,0,0};
	
	switch(sp_tare->seq){
	case 0:
		sp_tare->default_angle = *(ssw->camera_angle);
		sp_tare->calc_angle_x = sp_tare->default_angle.x;
		sp_tare->seq++;
		PMSND_PlaySE(FOOTPRINT_SE_SP_TARE);
		//break;
	case 1:
		angle.x += SP_TARE_ADD_ANGLE;
		sp_tare->calc_angle_x += SP_TARE_ADD_ANGLE;
		_AddCameraAngleRev(ssw, &angle, camera_ptr);
		
		if(sp_tare->calc_angle_x <= sp_tare->default_angle.x - SP_TARE_ANGLE_MAX){
			sp_tare->seq++;
		}
		break;
	case 2:	
		sp_tare->wait++;
		if(sp_tare->wait >= SP_TARE_WAIT){
			sp_tare->seq++;
		}
		break;
	case 3:
		angle.x += SP_TARE_SUB_ANGLE;
		sp_tare->calc_angle_x += SP_TARE_SUB_ANGLE;
		_AddCameraAngleRev(ssw, &angle, camera_ptr);
		
		if(sp_tare->calc_angle_x >= sp_tare->default_angle.x){
			_SetCameraAngleRev(ssw, &sp_tare->default_angle, camera_ptr);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   回転行列の計算
 *
 * @param   status		
 * @param   rotate		
 */
//--------------------------------------------------------------
static void Foot_RotateMtx(VecFx32 *rotate, MtxFx33 *dst_mtx)
{
	MtxFx33 mtx, calc_mtx;
	
	MTX_Identity33( &mtx );
	MTX_RotX33( &calc_mtx, FX_SinIdx( rotate->x ), FX_CosIdx( rotate->x ) );
	MTX_Concat33( &calc_mtx, &mtx, &mtx );
	MTX_RotZ33( &calc_mtx, FX_SinIdx( rotate->z ), FX_CosIdx( rotate->z ) );
	MTX_Concat33( &calc_mtx, &mtx, &mtx );
	MTX_RotY33( &calc_mtx, FX_SinIdx( rotate->y ), FX_CosIdx( rotate->y ) );
	MTX_Concat33( &calc_mtx, &mtx, &mtx );
	
	*dst_mtx = mtx;
}

//---------------------------------------------------------------------------
/**
 * @brief	カメラ位置を注視点、距離、アングルから算出する
 * 
 * プラチナのcamera.cから移植
 */
//---------------------------------------------------------------------------
void Foot_SetCamPosByTarget_Dist_Ang(VecFx32 *pos, FOOT_CAMERA_ANGLE *angle, fx32 distance, VecFx32 *target)
{
	u16 f_angle_x;
	
	//仰角⇒地面からの傾きに変換
	f_angle_x = -angle->x;
	/*== カメラ座標を求める ==*/
	pos->x = FX_Mul( FX_Mul( FX_SinIdx(angle->y), distance ), FX_CosIdx( angle->x ) );
	
	pos->z = FX_Mul(FX_Mul(FX_CosIdx(angle->y), distance), FX_CosIdx(angle->x));
	
	pos->y = FX_Mul( FX_SinIdx( f_angle_x ), distance );

	/*== 視点からの距離にする ==*/
	VEC_Add(pos, target, pos);
}

//---------------------------------------------------------------------------
/**
 * @brief	注視点をカメラ位置、距離、アングルから算出する
 * 
 * @param	camera_ptr	カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
static void Foot_SetTargetByCamPos_Dist_Ang(VecFx32 *target, FOOT_CAMERA_ANGLE *angle, fx32 distance, VecFx32 *camPos, GFL_G3D_CAMERA* camera_ptr)
{
	u16 angle_x;
	//仰角⇒地面からの傾きに変換
	angle_x = -angle->x;
	target->x = -FX_Mul( FX_Mul( FX_SinIdx( angle->y ), distance ), FX_CosIdx( angle->x ) );
	target->z = -FX_Mul( FX_Mul( FX_CosIdx( angle->y ), distance ), FX_CosIdx( angle->x ) );
	target->y = -FX_Mul( FX_SinIdx( angle_x ), distance );
	VEC_Add(target, camPos, target);
}

//---------------------------------------------------------------------------
/**
 * @brief	カメラ位置を注視点、距離、アングルから算出する
 * 
 * @param	camera_ptr	カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
static void _SetCameraDistance(STAMP_SYSTEM_WORK *ssw, fx32 distance, GFL_G3D_CAMERA *camera)
{
  VecFx32 pos;
  
  *(ssw->camera_distance) = distance;
 	Foot_SetCamPosByTarget_Dist_Ang(&pos, ssw->camera_angle, *(ssw->camera_distance), ssw->camera_target);

	GFL_G3D_CAMERA_SetPos( camera, &pos );
	GFL_G3D_CAMERA_Switching(camera);
}

//--------------------------------------------------------------
/**
 * カメラ距離加算
 *
 * @param   ssw		
 * @param   inDist		
 * @param   camera_ptr		
 */
//--------------------------------------------------------------
static void _AddCameraDistance(STAMP_SYSTEM_WORK *ssw, const fx32 inDist,GFL_G3D_CAMERA* camera_ptr)
{
  VecFx32 pos;
  
  *(ssw->camera_distance) += inDist;
	Foot_SetCamPosByTarget_Dist_Ang(&pos, ssw->camera_angle, *(ssw->camera_distance), ssw->camera_target);

	GFL_G3D_CAMERA_SetPos( camera_ptr, &pos );
	GFL_G3D_CAMERA_Switching(camera_ptr);
}

//---------------------------------------------------------------------------
/**
 * @brief	カメラアングル加算	公転
 * 
 * @param	inAngle			加算アングル
 * @param	camera_ptr		カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
static void _AddCameraAngleRev(STAMP_SYSTEM_WORK *ssw, const FOOT_CAMERA_ANGLE *inAngle,GFL_G3D_CAMERA *camera_ptr)
{
  VecFx32 pos;

	ssw->camera_angle->x += inAngle->x;
	ssw->camera_angle->y += inAngle->y;
	ssw->camera_angle->z += inAngle->z;
	Foot_SetCamPosByTarget_Dist_Ang(&pos, ssw->camera_angle, *(ssw->camera_distance), ssw->camera_target);

	GFL_G3D_CAMERA_SetPos( camera_ptr, &pos );
	GFL_G3D_CAMERA_Switching(camera_ptr);
}

//---------------------------------------------------------------------------
/**
 * @brief	カメラアングルセット	公転
 * 
 * @param	inAngle			アングル
 * @param	camera_ptr		カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
static void _SetCameraAngleRev(STAMP_SYSTEM_WORK *ssw, const FOOT_CAMERA_ANGLE *inAngle,GFL_G3D_CAMERA *camera_ptr)
{
  VecFx32 pos;

	*(ssw->camera_angle) = *inAngle;
	Foot_SetCamPosByTarget_Dist_Ang(&pos, ssw->camera_angle, *(ssw->camera_distance), ssw->camera_target);

	GFL_G3D_CAMERA_SetPos( camera_ptr, &pos );
	GFL_G3D_CAMERA_Switching(camera_ptr);
}

//---------------------------------------------------------------------------
/**
 * @brief	カメラアングルセット	自転
 * 
 * @param	inAngle			アングル
 * @param	camera_ptr		カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
static void _SetCameraAngleRot(STAMP_SYSTEM_WORK *ssw, const FOOT_CAMERA_ANGLE *inAngle,GFL_G3D_CAMERA *camera_ptr)
{
  VecFx32 target, pos;
  
	GFL_G3D_CAMERA_GetPos(camera_ptr, &pos);

	*(ssw->camera_angle) = *inAngle;
  Foot_SetTargetByCamPos_Dist_Ang(
    &target, ssw->camera_angle, *(ssw->camera_distance), &pos, camera_ptr);

	GFL_G3D_CAMERA_SetTarget(camera_ptr, &target);
	GFL_G3D_CAMERA_Switching(camera_ptr);
}
