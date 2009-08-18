//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wflby_3dobj.c
 *	@brief		３Dビルボードアクターオブジェ管理システム
 *	@author		tomoya takahashi
 *	@data		2007.11.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

//#include "gflib/res_manager.h"
//#include "gflib/texanm_sys.h"
//#include "gflib/bounding_box.h"

//#include "system/arc_util.h"
//#include "system/d3dobj.h"

#include "field/fldmmdl_pl_code.h"

#include "debug/mmodel.naix"

#include "wifi_lobby_other.naix"
#include "test_graphic/fld_act.naix"

#include "wflby_def.h"
#include "wflby_3dmatrix.h"
#include "wflby_def.h"

#include "wflby_3dobj.h"
#include "arc_def.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	主人公の表示位置の誤差
//=====================================
#define WFLBY_3DOBJ_OFS_Z	( FX32_CONST(16) )


//-------------------------------------
///	振り向き動作、１歩前動作描画フレーム
//=====================================
#if WB_FIX
#define WFLBY_3DOBJ_TRUN_FIRST_FRAME	(4)
#else
#define WFLBY_3DOBJ_TRUN_FIRST_FRAME	(1)
#endif


//-------------------------------------
///	特殊アニメ	回転
//=====================================
#define WFLBY_3DOBJ_ANM_ROTA_1SYNC	( 4 )	// 1ペンを見せているシンク数
static const u8	sc_WFLBY_3DOBJ_ANM_WAY[ WF2DMAP_WAY_NUM ] = {
	WF2DMAP_WAY_DOWN,
	WF2DMAP_WAY_LEFT,
	WF2DMAP_WAY_UP,
	WF2DMAP_WAY_RIGHT,
};

//-------------------------------------
///	特殊アニメ	足バタバタ
//=====================================
#define WFLBY_3DOBJ_ANM_BATA_FRAME	( 4 )


//-------------------------------------
///	影について
//	Y座標は常に床の位置
//	その他は動作する
//=====================================
#define WFLBY_3DOBJ_SHADOW_OFS_X		(0)
#define WFLBY_3DOBJ_SHADOW_MAT_Y	(WFLBY_3DMATRIX_FLOOR_MAT+FX32_CONST(2))
#define WFLBY_3DOBJ_SHADOW_OFS_Z	(FX32_CONST(-8))
#define WFLBY_3DOBJ_SHADOW_OBJID	( 20 )


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

typedef struct BOUNDING_BOX_tag{
	fx16 w;
	fx16 h;
	fx16 d;
	int ScaleW;
	int ScaleH;
	int ScaleD;
}BOUNDING_BOX;

//-------------------------------------
///	人物３Dオブジェクトデータ
//=====================================
typedef struct {
	u16	objid;			// 対応する人物ID
	u16 tex:15;			// テクスチャファイルナンバー
	u16 anm:1;			// Vram転送アニメフラグ
} WFLBY_3DMDL_DATA;


//-------------------------------------
///	アクターで表示できない３Dオブジェリソース
//=====================================
typedef struct {
	GFL_G3D_RES			*mdlres;
	u32					alpha;
} WFLBY_3DMDLRES;



//-------------------------------------
///	ワーク構造体
//=====================================
struct _WFLBY_3DOBJWK{
	u8						updata : 4;	// 更新フラグ
	u8						culling : 2;// カリング
	u8						objdraw : 2;// アクター表示
	u8						anmflag : 1;// 特殊アニメの有無
	u8						anm : 7;	// 特殊アニメ
	u8						anmframe;	// アニメフレーム
	u8						anmspeed;	// アニメスピード
	
	const WF2DMAP_OBJWK*	cp_objwk;	// 参照オブジェクトデータ
#if WB_FIX
	BLACT_WORK_PTR			p_act;		// 描画アクター
#endif
	GFL_G3D_OBJ				*shadow;		// 影モデル
	GFL_G3D_OBJSTATUS		shadow_st;		// 影ステータス
	BOOL					shadow_draw_flag;	// TRUE:影描画　FALSE:描画しない
	
	GFL_BBDACT_SYS			*p_blact;
	GFL_BBDACT_ACTUNIT_ID	act_idx;

	u16	lastst;			// 1つ前の状態
	u16	lastanm;		// 1つ前の保存アニメ(BLACT アニメオフセット)
	u16 lastfrm;		// 1つ前のフレーム
	fx32 lastframe;		// 1つ前の保存アニメ終了フレーム(BLACT frame)

	int	set_light_msk;	// 設定したライトフラグ
};

//-------------------------------------
///	システム構造体
//=====================================
struct _WFLBY_3DOBJSYS{

#if WB_FIX
	// リソースマネージャ
	RES_MANAGER_PTR		p_mdlresman;
	RES_MANAGER_PTR		p_anmresman;
	TEXRES_MANAGER_PTR	p_texresman;
#endif

	// ビルボードアクターシステム
#if WB_FIX
	BLACT_SET_PTR		p_blact;
#else
	GFL_BBDACT_SYS		*p_blact;
	GFL_BBDACT_RESUNIT_ID resunit_id;
#endif

	// 描画オブジェクトテーブル
	WFLBY_3DOBJWK*		p_obj;
	u32					objnum;

	// 影モデルリソースデータ
	WFLBY_3DMDLRES		shadowres;	
	//モデルデータ(カリングチェック用
	void *idx_file;
	
	// ライトマスク
	int					mdl_light_msk;

};



//-----------------------------------------------------------------------------
/**
 *				モデルデータ
 */
//-----------------------------------------------------------------------------
#define WFLBY_3DMDL_MDLDATA_NUM	(1)
#define WFLBY_3DMDL_MDLDATA_IDX	(NARC_wifi_lobby_other_wifi_hero_nsbmd)

//-----------------------------------------------------------------------------
/**
 *				アニメデータ
 */
//-----------------------------------------------------------------------------
enum{
	WFLBY_3DMDL_ANMDATA_NORMAL,
	WFLBY_3DMDL_ANMDATA_TRANS,
	WFLBY_3DMDL_ANMDATA_NUM
};
static const u16 sc_WFLBY_ANMDATA[ WFLBY_3DMDL_ANMDATA_NUM ] = {
	NARC_mmodel_charaobj_itpcv_dat,		// 通常アニメ用
	NARC_mmodel_hero_itpcv_dat			// 転送アニメ用
};

//-------------------------------------
///		アニメフレームデータ
//=====================================
enum{
	// 停止アニメ
	WFLBY_3DOBJ_ANM_TYPE_STOP_UP,
	WFLBY_3DOBJ_ANM_TYPE_STOP_DOWN,
	WFLBY_3DOBJ_ANM_TYPE_STOP_LEFT,
	WFLBY_3DOBJ_ANM_TYPE_STOP_RIGHT,

	// 歩きアニメ
	WFLBY_3DOBJ_ANM_TYPE_UP,
	WFLBY_3DOBJ_ANM_TYPE_DOWN,
	WFLBY_3DOBJ_ANM_TYPE_LEFT,
	WFLBY_3DOBJ_ANM_TYPE_RIGHT,

	// 走りアニメ
	WFLBY_3DOBJ_ANM_TYPE_RUN_UP,
	WFLBY_3DOBJ_ANM_TYPE_RUN_DOWN,
	WFLBY_3DOBJ_ANM_TYPE_RUN_LEFT,
	WFLBY_3DOBJ_ANM_TYPE_RUN_RIGHT,
};
#define WFLBY_3DOBJ_ONEANM_FRAME		( 16 )
#define WFLBY_3DOBJ_ONEANM_HFRAME		( 8 )	// 半分
#define WFLBY_3DOBJ_ONEANM_RUNFRAME		( 4 )	// 半分
#if WB_FIX
static const BLACT_ANIME_TBL WFLBY_3DOBJ_ANM_TR_FRAME_DATA[] =
{
	{ 0, 15, BLACT_ANIM_LOOP },
	{ 16, 31, BLACT_ANIM_LOOP },
	{ 32, 47, BLACT_ANIM_LOOP },
	{ 48, 63, BLACT_ANIM_LOOP },
	
	{ 64, 79, BLACT_ANIM_LOOP },	//ダッシュ
	{ 80, 95, BLACT_ANIM_LOOP },
	{ 96, 111, BLACT_ANIM_LOOP },
	{ 112, 127, BLACT_ANIM_LOOP },
	
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};
#else
static const GFL_BBDACT_ANM PCstopLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopRAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopDAnm[] = {
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCwalkLAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkRAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkUAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 20, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkDAnm[] = {
	{ 22, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 23, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCrunLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 13, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM* playerBBDactAnmTable[] = { 
	PCstopUAnm, PCstopDAnm, PCstopLAnm, PCstopRAnm,
	PCwalkUAnm, PCwalkDAnm, PCwalkLAnm, PCwalkRAnm,
	PCrunUAnm, PCrunDAnm, PCrunLAnm, PCrunRAnm,
};

//--------------------------------------------------------------
//	NPCのアニメテーブル
//--------------------------------------------------------------
static const GFL_BBDACT_ANM NPC_walkLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM NPC_walkRAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM NPC_walkUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM NPC_walkDAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM * DATA_BlActNpcTbl[] = {
	//STOP 主人公と同様のアニメルーチンを通す為、テーブルの順番だけあわす。ここはダミー
	NPC_walkUAnm,
	NPC_walkDAnm,
	NPC_walkLAnm,
	NPC_walkRAnm,
	//WALK
	NPC_walkUAnm,
	NPC_walkDAnm,
	NPC_walkLAnm,
	NPC_walkRAnm,
	//RUN 主人公と同様のアニメルーチンを通す為、テーブルの順番だけあわす。ここはダミー
	NPC_walkUAnm,
	NPC_walkDAnm,
	NPC_walkLAnm,
	NPC_walkRAnm,
};

#endif	//WB_FIX



//-----------------------------------------------------------------------------
/**
 *				人物ファイルデータ
 */
//-----------------------------------------------------------------------------
#define WFLBY_3DMDL_DATA_NUM		( 20 )	// ユニオン16+主人公2+おねえさん+おにいさん
#define WFLBY_3DMDL_USE_DATA_NUM	( 19 )	// 使用しない主人公1を抜いた人数
#define WFLBY_3DMDL_DATA_HERO_NUM	( 2 )
static const WFLBY_3DMDL_DATA	sc_WFLBY_3DMDL_DATA[ WFLBY_3DMDL_DATA_NUM ] = {
	// 主人公2
	{
		PLHERO,
		NARC_fld_act_hero_nsbtx,	//NARC_mmodel_hero_nsbtx,
		TRUE
	},
	{
		PLHEROINE,
		NARC_fld_act_hero_nsbtx,	//NARC_mmodel_heroine_nsbtx,
		TRUE
	},

	// ユニオンキャラクタ16
	{
		PLBOY1,
		NARC_fld_act_achamo_nsbtx,	//NARC_mmodel_boy1_nsbtx,
		FALSE
	},
	{
		PLBOY3,
		NARC_fld_act_artist_nsbtx,	//NARC_mmodel_boy3_nsbtx,
		FALSE
	},
	{
		PLMAN3,
		NARC_fld_act_badman_nsbtx,	//NARC_mmodel_man3_nsbtx,
		FALSE
	},
	{
		PLBADMAN,
		NARC_fld_act_beachgirl_nsbtx,	//NARC_mmodel_badman_nsbtx,
		FALSE
	},
	{
		PLEXPLORE,
		NARC_fld_act_idol_nsbtx,	//NARC_mmodel_explore_nsbtx,
		FALSE
	},
	{
		PLFIGHTER,
		NARC_fld_act_lady_nsbtx,	//NARC_mmodel_fighter_nsbtx,
		FALSE
	},
	{
		PLGORGGEOUSM,
		NARC_fld_act_oldman1_nsbtx,	//NARC_mmodel_gorggeousm_nsbtx,
		FALSE
	},
	{
		PLMYSTERY,
		NARC_fld_act_policeman_nsbtx,	//NARC_mmodel_mystery_nsbtx,
		FALSE
	},
	{
		PLGIRL1,
		NARC_fld_act_rivel_nsbtx,	//NARC_mmodel_girl1_nsbtx,
		FALSE
	},
	{
		PLGIRL2,
		NARC_fld_act_waiter_nsbtx,	//NARC_mmodel_girl2_nsbtx,
		FALSE
	},
	{
		PLWOMAN2,
		NARC_fld_act_achamo_nsbtx,	//NARC_mmodel_woman2_nsbtx,
		FALSE
	},
	{
		PLWOMAN3,
		NARC_fld_act_artist_nsbtx,	//NARC_mmodel_woman3_nsbtx,
		FALSE
	},
	{
		PLIDOL,
		NARC_fld_act_badman_nsbtx,	//NARC_mmodel_idol_nsbtx,
		FALSE
	},
	{
		PLLADY,	
		NARC_fld_act_beachgirl_nsbtx,	//NARC_mmodel_lady_nsbtx,
		FALSE
	},
	{
		PLCOWGIRL,
		NARC_fld_act_idol_nsbtx,	//NARC_mmodel_cowgirl_nsbtx,
		FALSE
	},
	{
		PLGORGGEOUSW,
		NARC_fld_act_lady_nsbtx,	//NARC_mmodel_gorggeousw_nsbtx,
		FALSE
	},
	{
		PLWIFISW,
		NARC_fld_act_oldman1_nsbtx,	//NARC_mmodel_wifisw_nsbtx,
		FALSE
	},
	{
		PLWIFISM,
		NARC_fld_act_policeman_nsbtx,	//NARC_mmodel_wifism_nsbtx,
		FALSE
	},
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//-------------------------------------
///	　人物ファイルデータ
//=====================================
static const WFLBY_3DMDL_DATA* WFLBY_3DOBJ_GetMdlData( u32 objid );
static u16 WFLBY_3DOBJ_GetResID( WFLBY_3DOBJSYS* p_sys, u32 objid );


//-------------------------------------
///		アニメデータの取得
//=====================================
static u32 WFLBY_3DMDL_ANM_GetAnmOffs( BOOL walk, u32 way );
static fx32 WFLBY_3DMDL_ANM_GetAnmFrame( u16 frame_max, u16 now_frame, u32 anm_frame_max );


//-------------------------------------
///	アクターで表示できない３Dオブジェリソース
//=====================================
static void WFLBY_3DMDL_RES_Load( WFLBY_3DMDLRES* p_wk, ARCHANDLE* p_handle, u32 dataidx, u32 gheapID );
static void WFLBY_3DMDL_RES_Delete( WFLBY_3DMDLRES* p_wk );
static void WFLBY_3DMDL_RES_SetAlpha( WFLBY_3DMDLRES* p_wk, u32 alpha );
static u32 WFLBY_3DMDL_RES_GetAlpha( const WFLBY_3DMDLRES* cp_wk );
static void WFLBY_3DMDL_RES_InitD3DOBJ( WFLBY_3DMDLRES* p_wk, GFL_G3D_OBJ** p_obj );
static void WFLBY_3DMDL_RES_DeleteD3DOBJ( GFL_G3D_OBJ** p_obj );


//-------------------------------------
///	３Dオブジェクトワーク
//=====================================
static WFLBY_3DOBJWK* WFLBY_3DOBJSYS_GetCleanWk( WFLBY_3DOBJSYS* p_sys );
static BOOL WFLBY_3DOBJWK_CheckMove( const WFLBY_3DOBJWK* cp_wk );
static void WFLBY_3DOBJWK_CleanWk( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_Draw( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_Updata( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_Updata_Normal( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_Updata_Walk( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_Updata_Turn( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_Updata_Run( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_AnmUpdata( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_Anm_Rota( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_Anm_Jump( WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_Anm_BataBata( WFLBY_3DOBJWK* p_wk );
static BOOL WFLBY_3DOBJWK_GetAnmSave( u32 st );
static void WFLBY_3DOBJWK_CheckCulling( WFLBY_3DOBJSYS* p_sys, WFLBY_3DOBJWK* p_wk );
static void WFLBY_3DOBJWK_ContBlactDrawFlag( WFLBY_3DOBJWK* p_wk );
static BOOL WFLBY_3DOBJWK_CheckCullingBlact(WFLBY_3DOBJSYS* p_sys, GFL_BBDACT_SYS *p_blact,GFL_BBDACT_ACTUNIT_ID act_idx);
static u32	BB_CullingCheck3DModelNonResQuick(	const VecFx32* trans_p, const BOUNDING_BOX *inBox);
static void	g3d_trans_BBD( GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz );


//----------------------------------------------------------------------------
/**
 *	@brief	３Dオブジェクト描画システム		作成
 *
 *	@param	objnum		オブジェクト数
 *	@param	hero_sex	主人公の性別
 *	@param	heapID		ヒープID
 *	@param	gheapID		グラフィックヒープID
 *
 *	@return	システムワーク
 */
//-----------------------------------------------------------------------------
WFLBY_3DOBJSYS* WFLBY_3DOBJSYS_Init( u32 objnum, u32 hero_sex, u32 heapID, u32 gheapID )
{
	WFLBY_3DOBJSYS* p_sys;


	p_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(WFLBY_3DOBJSYS) );

	// オブジェクトテーブル作成
	{
		int i;
		
		p_sys->p_obj	= GFL_HEAP_AllocClearMemory( heapID, sizeof(WFLBY_3DOBJWK)*objnum );
		p_sys->objnum	= objnum;
	
		for( i=0; i<p_sys->objnum; i++ ){
			WFLBY_3DOBJWK_CleanWk( &p_sys->p_obj[i] );
		}
	}

#if WB_FIX
	// リソースオブジェを作成
	p_sys->p_mdlresman = RESM_Init( WFLBY_3DMDL_MDLDATA_NUM, heapID );
	p_sys->p_anmresman = RESM_Init( WFLBY_3DMDL_ANMDATA_NUM, heapID );
	p_sys->p_texresman = TEXRESM_Init( WFLBY_3DMDL_DATA_NUM, heapID );
#endif

	// ビルボードアクター作成
	{
	#if WB_FIX
		BLACT_SETDATA	setdata;

		BLACT_InitSys( 1, heapID );
		
		setdata.WorkNum = objnum;
		setdata.heap	= heapID;
		p_sys->p_blact = BLACT_InitSet( &setdata );
	#else
		VecFx32 scale={FX32_ONE*16, FX32_ONE*16, FX32_ONE*16};
		
		p_sys->p_blact = GFL_BBDACT_CreateSys(
			WFLBY_3DMDL_MDLDATA_NUM + WFLBY_3DMDL_ANMDATA_NUM + WFLBY_3DMDL_DATA_NUM, 
			objnum, g3d_trans_BBD, heapID);
		GFL_BBD_SetScale( GFL_BBDACT_GetBBDSystem(p_sys->p_blact), &scale );
	#endif
	}

	// 読み込み処理
	{
		ARCHANDLE* p_handle;
		ARCHANDLE* p_handle_other;
		void* p_file;
		int i;
		
		p_handle		= GFL_ARC_OpenDataHandle( ARCID_FLDMMLD, heapID );
		p_handle_other	= GFL_ARC_OpenDataHandle( ARCID_WIFILOBBY_OTHER_GRA, heapID );

		// モデル
		{
			p_file = GFL_ARCHDL_UTIL_Load( p_handle_other, WFLBY_3DMDL_MDLDATA_IDX, FALSE, gheapID );
		#if WB_FIX
			RESM_AddResNormal( p_sys->p_mdlresman, p_file, WFLBY_3DMDL_MDLDATA_IDX );
		#else
			p_sys->idx_file = p_file;
		#endif
			WFLBY_LIGHT_SetUpMdl( p_file );

			// モデルデータからライトデータを取得する
			{
				NNSG3dResMdlSet* p_mdlset;
				NNSG3dResMdl* p_mdlres;
	
				p_mdlset = NNS_G3dGetMdlSet( p_file );
				p_mdlres = NNS_G3dGetMdlByIdx( p_mdlset, 0 );
				p_sys->mdl_light_msk = NNS_G3dMdlGetMdlLightEnableFlag( p_mdlres, 0 );
			}

		}

	#if WB_FIX
		// アニメデータ
		{
			for( i=0; i<WFLBY_3DMDL_ANMDATA_NUM; i++ ){
				p_file = GFL_ARCHDL_UTIL_Load( p_handle, sc_WFLBY_ANMDATA[i], FALSE, gheapID );
				RESM_AddResNormal( p_sys->p_anmresman, p_file, sc_WFLBY_ANMDATA[i] );
			}
		}
	#endif

		// テクスチャデータ
		{
			int				skip;
			GFL_BBDACT_RESDATA *bbdres;
			int tblno = 0;
			
			bbdres = GFL_HEAP_AllocClearMemory(
				gheapID, sizeof(GFL_BBDACT_RESDATA) * WFLBY_3DMDL_USE_DATA_NUM);
			
			// 主人公の登録をスキップ
			if( hero_sex == PM_MALE ){
				skip = 1;	// 女のリソースをスキップ
			}else{
				skip = 0;	// 男のリソースをスキップ
			}

			for( i=0; i<WFLBY_3DMDL_DATA_NUM; i++ ){
				if( skip == i ){
					continue;
				}
				
			#if WB_FIX
				if( sc_WFLBY_3DMDL_DATA[i].anm == FALSE ){
					tex_cut = TRUE;
				}else{
					tex_cut = FALSE;
				}

				p_file = GFL_ARCHDL_UTIL_Load( p_handle, sc_WFLBY_3DMDL_DATA[i].tex, FALSE, gheapID );
				p_tex = TEXRESM_AddResNormal( p_sys->p_texresman, p_file, sc_WFLBY_3DMDL_DATA[i].tex, tex_cut, gheapID );
				// 転送とテクスチャ領域をカット
				if( tex_cut == TRUE ){	
					TEXRESM_AllocVramKeyPTR( p_tex );
					TEXRESM_TexLoadPTR( p_tex );
					TEXRESM_CutTexPTR( p_tex );
				}
			#else
				bbdres[tblno].texFmt = GFL_BBD_TEXFMT_PAL16;
				bbdres[tblno].texSiz = GFL_BBD_TEXSIZ_32x512;
				bbdres[tblno].celSizX = 32;
				bbdres[tblno].celSizY = 32;
				if(i < WFLBY_3DMDL_DATA_HERO_NUM){
					bbdres[tblno].arcID = ARCID_FLDACT;
					bbdres[tblno].datID = NARC_fld_act_tex32x32_nsbtx;	//転送領域確保用のダミー
					bbdres[tblno].dataCut = GFL_BBDACT_RESTYPE_DATACUT;//GFL_BBDACT_RESTYPE_TRANSSRC;
				}
				else{
					bbdres[tblno].arcID = ARCID_FLDACT;
					bbdres[tblno].datID = sc_WFLBY_3DMDL_DATA[i].tex;
					bbdres[tblno].dataCut = GFL_BBDACT_RESTYPE_DATACUT;
				}
				tblno++;
			#endif
			}
			p_sys->resunit_id = GFL_BBDACT_AddResourceUnit(p_sys->p_blact, bbdres, tblno);
			GFL_HEAP_FreeMemory(bbdres);
		}


		// 影
		WFLBY_3DMDL_RES_Load( &p_sys->shadowres, p_handle_other, NARC_wifi_lobby_other_kage_nsbmd, gheapID );

		GFL_ARC_CloseDataHandle( p_handle );
		GFL_ARC_CloseDataHandle( p_handle_other );
	}
	return p_sys;
}

//BBD用VRAM転送関数
static void	g3d_trans_BBD( GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz )
{
	NNS_GFD_DST_TYPE transType;

	if( type == GFL_BBDACT_TRANSTYPE_DATA ){
		transType = NNS_GFD_DST_3D_TEX_VRAM;
	} else {
		transType = NNS_GFD_DST_3D_TEX_PLTT;
	}
	NNS_GfdRegisterNewVramTransferTask( transType, dst, (void*)src, siz );
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dオブジェクト描画システム	破棄
 *
 *	@param	p_sys		システム
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJSYS_Exit( WFLBY_3DOBJSYS* p_sys )
{
  // 残っているモデルを破棄
  {
    int i;
    for(i = 0; i < p_sys->objnum; i++){
      if(WFLBY_3DOBJWK_CheckMove( &p_sys->p_obj[i] ) == TRUE){
        WFLBY_3DOBJWK_Del(&p_sys->p_obj[i]);
      }
    }
  }
  
	// 読み込んだリソースを破棄する
	{
		WFLBY_3DMDL_RES_Delete( &p_sys->shadowres );
	#if WB_FIX
		TEXRESM_DeleteAllRes( p_sys->p_texresman );
		RESM_DeleteAllRes( p_sys->p_mdlresman );
		RESM_DeleteAllRes( p_sys->p_anmresman );
	#else
		GFL_BBDACT_RemoveResourceUnit(p_sys->p_blact, p_sys->resunit_id, WFLBY_3DMDL_USE_DATA_NUM);
		GFL_HEAP_FreeMemory(p_sys->idx_file);
	#endif
	}

	// ビルボードアクター破棄
	{
	#if WB_FIX
		BLACT_DestSet( p_sys->p_blact );
		BLACT_DestSys();
	#else
		GFL_BBDACT_DeleteSys(p_sys->p_blact);
	#endif
	}

#if WB_FIX
	// リソースマネージャの破棄
	TEXRESM_Delete( p_sys->p_texresman );
	RESM_Delete( p_sys->p_mdlresman );
	RESM_Delete( p_sys->p_anmresman );
#endif

	// モデルデータ破棄
	GFL_HEAP_FreeMemory( p_sys->p_obj );

	// システム自体を破棄
	GFL_HEAP_FreeMemory( p_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dオブジェクト描画システム	更新
 *
 *	@param	p_sys		システム
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJSYS_Updata( WFLBY_3DOBJSYS* p_sys )
{
	int i;
	
	// 表示データの更新
	for( i=0; i<p_sys->objnum; i++ ){
		// p_obj->updataがTRUEならWF2DMAP_OBJのデータが反映されます。
		// p_obj->updataがFALSEでp_obj->anmflagがTRUEなら特殊アニメのデータが反映されます。
		WFLBY_3DOBJWK_Updata( &p_sys->p_obj[i] );		// 通常アップデート
		WFLBY_3DOBJWK_AnmUpdata( &p_sys->p_obj[i] );	// 特殊アニメアップデート
		WFLBY_3DOBJWK_CheckCulling( p_sys, &p_sys->p_obj[i] );	// カリングチェック
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dオブジェクト描画システム		描画
 *
 *	@param	p_sys		システム
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJSYS_Draw( WFLBY_3DOBJSYS* p_sys, GFL_G3D_CAMERA *p_camera )
{
	int i;

	
	// ビルボードアクター表示
#if WB_FIX
	BLACT_DrawSys();
#else
	GFL_BBDACT_Main( p_sys->p_blact );
	GFL_BBDACT_Draw(p_sys->p_blact, p_camera, NULL);
#endif


	// 影の描画
	// 影のカラーが０なら表示しない
	if( WFLBY_3DMDL_RES_GetAlpha( &p_sys->shadowres ) > 0 ){
		for( i=0; i<p_sys->objnum; i++ ){
			if( WFLBY_3DOBJWK_CheckMove( &p_sys->p_obj[i] ) == TRUE ){
				WFLBY_3DOBJWK_Draw( &p_sys->p_obj[i] );
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dオブジェクト描画システム	VBlank処理
 *
 *	@param	p_sys		システム
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJSYS_VBlank( WFLBY_3DOBJSYS* p_sys )
{
#if WB_FIX	//g3d_trans_BBDで転送されるのでこの関数の代わりさえいらないはず
	// ビルボードアクター
	BLACT_VBlankFunc( p_sys->p_blact );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	出せるトレーナタイプかチェック
 *
 *	@param	trtype	トレーナタイプ
 *
 *	@retval	TRUE	出せる
 *	@retval	FALSE	出せない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DOBJSYS_CheckTrType( u32 trtype )
{
	const WFLBY_3DMDL_DATA* cp_data;

	cp_data = WFLBY_3DOBJ_GetMdlData( trtype );
	if( cp_data == NULL ){
		return FALSE;
	}
	return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief	影のアルファ値を設定する
 *
 *	@param	p_sys		システムワーク
 *	@param	alpha		アルファ値
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJSYS_SetShadowAlpha( WFLBY_3DOBJSYS* p_sys, u32 alpha )
{
	WFLBY_3DMDL_RES_SetAlpha( &p_sys->shadowres, alpha );
}

//----------------------------------------------------------------------------
/**
 *	@brief	影のアルファ値を取得する
 *
 *	@param	cp_sys		システムワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
u32 WFLBY_3DOBJSYS_GetShadowAlpha( const WFLBY_3DOBJSYS* cp_sys )
{
	return WFLBY_3DMDL_RES_GetAlpha( &cp_sys->shadowres );
}


//----------------------------------------------------------------------------
/**
 *	@brief	３Dオブジェクト描画システム	描画オブジェ作成
 *
 *	@param	p_sys		システム
 *	@param	cp_objwk	オブジェクトデータワーク
 *
 *	@return	描画オブジェクトワーク
 */
//-----------------------------------------------------------------------------
WFLBY_3DOBJWK* WFLBY_3DOBJWK_New( WFLBY_3DOBJSYS* p_sys, const WF2DMAP_OBJWK* cp_objwk )
{
	WFLBY_3DOBJWK* p_wk;
  
	// 空いているワークを取得
	p_wk = WFLBY_3DOBJSYS_GetCleanWk( p_sys );
	p_wk->p_blact = p_sys->p_blact;
	
	// 参照先オブジェクトポインタ保存
	p_wk->cp_objwk = cp_objwk;

	// アクター生成
	{
		const WFLBY_3DMDL_DATA* cp_data;
	#if WB_FIX
		BLACT_HEADER	header;
		BLACT_ADD		add;
		RES_OBJ_PTR		p_resobj;
		TEXRES_OBJ_PTR	p_texresobj;
		void*			p_imd;
		const NNSG3dResTex* cp_itx;
		TEXANM_DATATBL texanm;
		NNSGfdTexKey	texkey;
		NNSGfdTexKey	tex4x4key;
		NNSGfdPlttKey	plttkey;
		VecFx32			scale = {FX32_ONE*2,FX32_ONE*2,FX32_ONE*2};
	#endif
		u32				objid;
		VecFx32			matrix = {0,0,0};
		u16 res_id;
		GFL_BBDACT_ACTDATA actdata;
		
		// オブジェクト構成データ取得
		objid	= WF2DMAP_OBJWkDataGet( cp_objwk, WF2DMAP_OBJPM_CHARA );
		cp_data = WFLBY_3DOBJ_GetMdlData( objid );
		res_id = WFLBY_3DOBJ_GetResID( p_sys, objid );

	#if WB_FIX
		// モデルデータ取得
		{
			p_resobj = RESM_GetResObj( p_sys->p_mdlresman, WFLBY_3DMDL_MDLDATA_IDX );
			p_imd	 = RESM_GetRes( p_resobj );
		}

		// テクスチャデータ取得
		{
			p_texresobj = TEXRESM_GetResObj( p_sys->p_texresman, cp_data->tex );
			cp_itx		= TEXRESM_GetResPTR( p_texresobj );
		}

		// テクスチャアニメデータ
		{
			u32 anm_id;
			const void* cp_buff;
			if( cp_data->anm == TRUE ){
				anm_id = sc_WFLBY_ANMDATA[WFLBY_3DMDL_ANMDATA_TRANS];
			}else{
				anm_id = sc_WFLBY_ANMDATA[WFLBY_3DMDL_ANMDATA_NORMAL];
			}
			p_resobj	= RESM_GetResObj( p_sys->p_anmresman, anm_id );
			cp_buff		= RESM_GetRes( p_resobj );

			TEXANM_UnPackLoadFile( cp_buff, &texanm );
		}
		
		if( cp_data->anm == TRUE ){	
			// 転送アニメで設定
			BLACT_MakeHeaderVramAnm( &header, p_imd, cp_itx, WFLBY_3DOBJ_ANM_TR_FRAME_DATA, &texanm );
		}else{
			// 通常アニメで設定
			texkey		= TEXRESM_GetTexKeyPTR( p_texresobj );
			tex4x4key	= TEXRESM_GetTex4x4KeyPTR( p_texresobj );
			plttkey		= TEXRESM_GetPlttKeyPTR( p_texresobj );
			BLACT_MakeHeaderNormalAnm( &header, p_imd, cp_itx, WFLBY_3DOBJ_ANM_TR_FRAME_DATA, &texanm, texkey, tex4x4key, plttkey  );
		}
	#endif
	
		// 座標を設定
		{
			WF2DMAP_POS pos;
			pos = WF2DMAP_OBJWkMatrixGet( cp_objwk );
			WFLBY_3DMATRIX_GetPosVec( &pos, &matrix );
			matrix.x += WFLBY_BLDACT_PL_OFFSET_X;
			matrix.y += WFLBY_BLDACT_PL_OFFSET_Y;
			matrix.z += WFLBY_3DOBJ_OFS_Z;
		}

		// ワークを作成
	#if WB_FIX
		add.blact_s = p_sys->p_blact;
		add.pHeader	= &header;
		add.matrix	= matrix;
		add.scale	= scale;
		p_wk->p_act = BLACT_Add( &add );
	#else
		actdata.resID = res_id;
		actdata.sizX = FX16_ONE;
		actdata.sizY = FX16_ONE;
		actdata.alpha = 31;
		actdata.drawEnable = TRUE;
		actdata.lightMask = p_sys->mdl_light_msk;
		actdata.trans = matrix;
		actdata.func = NULL;
		actdata.work = NULL;
		p_wk->act_idx = GFL_BBDACT_AddAct(p_sys->p_blact, p_sys->resunit_id, &actdata, 1);
		if(res_id == p_sys->resunit_id){	//res_idが先頭なら主人公
			//ダミーと転送用のリソースを入れ替える
			GFL_BBDACT_BindActTexResLoad(
				p_sys->p_blact, p_wk->act_idx, ARCID_FLDACT, sc_WFLBY_3DMDL_DATA[res_id].tex);
			GFL_BBDACT_SetAnimeTable(
				p_sys->p_blact, p_wk->act_idx, playerBBDactAnmTable, NELEMS(playerBBDactAnmTable));
		}
		else{
			GFL_BBDACT_SetAnimeTable(
				p_sys->p_blact, p_wk->act_idx, DATA_BlActNpcTbl, NELEMS(DATA_BlActNpcTbl));
		}
		GFL_BBDACT_SetAnimeIdxOn(p_sys->p_blact, p_wk->act_idx, 0);
	#endif
	
	#if WB_FIX
		// コールバックの設定
		BLACT_DrawBeforeProcSet( p_wk->p_act, WFLBY_3DOBJWK_CallBack_BlactDraw, p_wk );
	#endif
	

		// 影を作成
		WFLBY_3DMDL_RES_InitD3DOBJ( &p_sys->shadowres, &p_wk->shadow );
		p_wk->shadow_draw_flag = TRUE;
    
		// 座標をあわせる
	#if WB_FIX
		D3DOBJ_SetMatrix( &p_wk->shadow, 
				matrix.x + WFLBY_3DOBJ_SHADOW_OFS_X,
				WFLBY_3DOBJ_SHADOW_MAT_Y,
				matrix.z + WFLBY_3DOBJ_SHADOW_OFS_Z );
	#else
		VEC_Set(&p_wk->shadow_st.trans, matrix.x + WFLBY_3DOBJ_SHADOW_OFS_X,
			WFLBY_3DOBJ_SHADOW_MAT_Y, matrix.z + WFLBY_3DOBJ_SHADOW_OFS_Z);
		VEC_Set(&p_wk->shadow_st.scale, FX32_ONE, FX32_ONE, FX32_ONE);
		MTX_Identity33(&p_wk->shadow_st.rotate);
	#endif
	}

	


	// 更新開始
	p_wk->updata = TRUE;

	// 表示とカリングフラグ
	p_wk->objdraw = TRUE;
	p_wk->culling = FALSE;

	// ライトマスク設定
	p_wk->set_light_msk = p_sys->mdl_light_msk;

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dオブジェクト描画システム	描画オブジェ破棄
 *
 *	@param	p_wk		描画オブジェクトワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_Del( WFLBY_3DOBJWK* p_wk )
{
#if WB_FIX
	BLACT_Delete( p_wk->p_act );
#else
	GFL_BBDACT_RemoveAct(p_wk->p_blact, p_wk->act_idx, 1);
	WFLBY_3DMDL_RES_DeleteD3DOBJ(&p_wk->shadow);
#endif
	GFL_STD_MemFill( p_wk, 0, sizeof(WFLBY_3DOBJWK) );
}


//----------------------------------------------------------------------------
/**
 *	@brief	描画オブジェクトの更新フラグを設定
 *
 *	@param	p_wk		ワーク
 *	@param	updata		アップデートフラグ
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_SetUpdata( WFLBY_3DOBJWK* p_wk, BOOL updata )
{
	p_wk->updata = updata;
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画オブジェクトの更新フラグを取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE		更新中
 *	@retval	FALSE		更新していない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DOBJWK_GetUpdata( const WFLBY_3DOBJWK* cp_wk )
{
	return cp_wk->updata;
}


//----------------------------------------------------------------------------
/**
 *	@brief	light_mskを設定
 *
 *	@param	p_wk		ワーク
 *	@param	light_msk	ライトマスク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_SetLight( WFLBY_3DOBJWK* p_wk, u32 light_msk )
{
	p_wk->set_light_msk = light_msk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カリングフラグを取得中
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	カリングチェックに引っかかって非表示中
 *	@retval	FALSE	カリングに引っかからずに表示中
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DOBJWK_GetCullingFlag( const WFLBY_3DOBJWK* cp_wk )
{
	return cp_wk->culling;
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画オブジェクトの座標を設定
 *
 *	@param	p_wk		ワーク
 *	@param	cp_vec		座標
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_SetMatrix( WFLBY_3DOBJWK* p_wk, const WF2DMAP_POS* cp_pos )
{
	VecFx32 vec;
	WFLBY_3DMATRIX_GetPosVec( cp_pos, &vec );
	vec.z += WFLBY_3DOBJ_OFS_Z;


	WFLBY_3DOBJWK_Set3DMatrix( p_wk, &vec );
	
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画オブジェクトの座標を取得
 *
 *	@param	cp_wk		ワーク
 *	@param	p_vec		座標
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_GetMatrix( const WFLBY_3DOBJWK* cp_wk, WF2DMAP_POS* p_pos )
{
#if WB_FIX
	const VecFx32* cp_vec;
	cp_vec = BLACT_MatrixGet( cp_wk->p_act );
	WFLBY_3DMATRIX_GetVecPos( cp_vec, p_pos );
#else
	VecFx32 trans;
	GFL_BBD_GetObjectTrans(GFL_BBDACT_GetBBDSystem(cp_wk->p_blact), cp_wk->act_idx, &trans);
	WFLBY_3DMATRIX_GetVecPos( &trans, p_pos );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	3D座標を設定する
 *
 *	@param	p_wk	人物ワーク
 *	@param	cp_vec	ベクトル
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_Set3DMatrix( WFLBY_3DOBJWK* p_wk, const VecFx32* cp_vec )
{
	VecFx32 trans;
	
#if WB_FIX
	BLACT_MatrixSet( p_wk->p_act, cp_vec );

	// 影に反映ただし、影はのY位置は常に定位置
	D3DOBJ_SetMatrix( &p_wk->shadow, cp_vec->x + WFLBY_3DOBJ_SHADOW_OFS_X,
			WFLBY_3DOBJ_SHADOW_MAT_Y, cp_vec->z + WFLBY_3DOBJ_SHADOW_OFS_Z );
#else
	trans = *cp_vec;
	trans.x += WFLBY_BLDACT_PL_OFFSET_X;
	trans.y += WFLBY_BLDACT_PL_OFFSET_Y;
	GFL_BBD_SetObjectTrans(GFL_BBDACT_GetBBDSystem(p_wk->p_blact), p_wk->act_idx, &trans);

	// 影に反映ただし、影はのY位置は常に定位置
	p_wk->shadow_st.trans.x = trans.x + WFLBY_3DOBJ_SHADOW_OFS_X;
	p_wk->shadow_st.trans.y = WFLBY_3DOBJ_SHADOW_MAT_Y;
	p_wk->shadow_st.trans.z = trans.z + WFLBY_3DOBJ_SHADOW_OFS_Z;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	３D座標を取得する
 *
 *	@param	cp_wk	人物ワーク
 *	@param	p_vec	ベクトル
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_Get3DMatrix( const WFLBY_3DOBJWK* cp_wk, VecFx32* p_vec )
{
#if WB_FIX
	const VecFx32* cp_vec;
	
	cp_vec = BLACT_MatrixGet( cp_wk->p_act );
	*p_vec = *cp_vec;
#else
	GFL_BBD_GetObjectTrans(GFL_BBDACT_GetBBDSystem(cp_wk->p_blact), cp_wk->act_idx, p_vec);
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	表示している方向を変更
 *
 *	@param	p_wk	人物ワーク
 *	@param	way		方向
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_SetWay( WFLBY_3DOBJWK* p_wk, WF2DMAP_WAY way )
{
	u32 anm;
	anm = WFLBY_3DMDL_ANM_GetAnmOffs( TRUE, way );
#if WB_FIX
	BLACT_AnmOffsChg( p_wk->p_act, anm );
	BLACT_AnmFrameSetOffs( p_wk->p_act, 0 );
#else
	GFL_BBDACT_SetAnimeIdx(p_wk->p_blact, p_wk->act_idx, anm);
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメ登録	（更新フラグをOFFにした後の動作）
 *
 *	@param	p_wk	ワーク
 *	@param	anm		アニメナンバー
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_StartAnm( WFLBY_3DOBJWK* p_wk, WFLBY_3DOBJ_ANMTYPE anm )
{
	p_wk->anmflag	= TRUE;
	p_wk->anm		= anm;
	p_wk->anmframe	= 0;
	p_wk->anmspeed	= 1;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメ破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_EndAnm( WFLBY_3DOBJWK* p_wk )
{
	p_wk->anmflag = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメスピードを設定する
 *
 *	@param	p_wk		ワーク	
 *	@param	speed		スピード
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_SetAnmSpeed( WFLBY_3DOBJWK* p_wk, u8 speed )
{
	p_wk->anmspeed	= speed;
}


//----------------------------------------------------------------------------
/**
 *	@brief	描画オブジェクトの描画フラグを設定
 *
 *	@param	p_wk		ワーク
 *	@param	flag		描画フラグ
 */
//-----------------------------------------------------------------------------
void WFLBY_3DOBJWK_SetDrawFlag( WFLBY_3DOBJWK* p_wk, BOOL flag )
{
	p_wk->objdraw = flag;
	WFLBY_3DOBJWK_ContBlactDrawFlag( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画オブジェクトの描画フラグを取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE	描画中
 *	@retval	FALSE	非表示中
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DOBJWK_GetDrawFlag( const WFLBY_3DOBJWK* cp_wk )
{
	return cp_wk->objdraw;
}




//-----------------------------------------------------------------------------
/**
 *				プライベート関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	objidに対応した人物データを取得する
 *
 *	@param	objid		オブジェクトID
 *
 *	@return	人物データ
 */
//-----------------------------------------------------------------------------
static const WFLBY_3DMDL_DATA* WFLBY_3DOBJ_GetMdlData( u32 objid )
{
	int i;

	for( i=0; i<WFLBY_3DMDL_DATA_NUM; i++ ){
		if( sc_WFLBY_3DMDL_DATA[i].objid == objid ){
			return &sc_WFLBY_3DMDL_DATA[i];
		}
	}
	GF_ASSERT(0);
	return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	objidに対応したGFL_BBDACT_RESUNIT_IDを取得する
 *
 *  @param  p_sys		
 *	@param	objid		オブジェクトID
 *
 *	@return	GFL_BBDACT_RESUNIT_ID
 */
//-----------------------------------------------------------------------------
static u16 WFLBY_3DOBJ_GetResID( WFLBY_3DOBJSYS* p_sys, u32 objid )
{
	int i;

	for( i=0; i<WFLBY_3DMDL_DATA_NUM; i++ ){
		if( sc_WFLBY_3DMDL_DATA[i].objid == objid ){
			if(i < WFLBY_3DMDL_DATA_HERO_NUM){
				return p_sys->resunit_id;	//主人公リソースは必ず先頭
			}
			else{
				return p_sys->resunit_id + i - 1;	//居ない主人公1分、-1
			}
		}
	}
	GF_ASSERT(0);
	return p_sys->resunit_id;
}


//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションオフセットを取得する
 *
 *	@param	walk		歩きアニメか	TRUE：歩き	FALSE：走り
 *	@param	way			方向
 *
 *	@return	アニメーションオフセット
 */
//-----------------------------------------------------------------------------
static u32 WFLBY_3DMDL_ANM_GetAnmOffs( BOOL walk, u32 way )
{
	if( walk ){
		return WFLBY_3DOBJ_ANM_TYPE_UP + way;
	}
	return WFLBY_3DOBJ_ANM_TYPE_RUN_UP + way;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレームを取得
 *
 *	@param	frame_max		最大フレーム
 *	@param	now_frame		今のフレーム
 */
//-----------------------------------------------------------------------------
static fx32 WFLBY_3DMDL_ANM_GetAnmFrame( u16 frame_max, u16 now_frame, u32 anm_frame_max )
{
	fx32 frame;

	// OBJシステムでアクションコマンドを設定してからアップデートするので、
	// frame_maxになることはない
	now_frame ++;
	
	frame = ( now_frame * anm_frame_max ) / frame_max;
	frame *= FX32_ONE;
	return frame;
}



//----------------------------------------------------------------------------
/**
 *	@brief	３Dモデルリソースを読み込む
 *
 *	@param	p_wk		ワーク
 *	@param	p_handle	ハンドル
 *	@param	dataidx		データインデックス
 *	@param	gheapID		グラフィックヒープID
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMDL_RES_Load( WFLBY_3DMDLRES* p_wk, ARCHANDLE* p_handle, u32 dataidx, u32 gheapID )
{
#if WB_FIX
	void* p_mdl;
	
	//  モデルデータ読み込み＆テクスチャ転送＆テクスチャ実データ破棄
	WFLBY_3DMAPOBJ_TEX_LoatCutTex( &p_mdl, p_handle, dataidx, gheapID );

	// モデルのリソースをGFL_G3D_OBJの形に形成
	p_wk->mdlres.pResMdl	= p_mdl;
	p_wk->mdlres.pModelSet	= NNS_G3dGetMdlSet( p_wk->mdlres.pResMdl );
	p_wk->mdlres.pModel		= NNS_G3dGetMdlByIdx( p_wk->mdlres.pModelSet, 0 );
	p_wk->mdlres.pMdlTex	= NNS_G3dGetTex( p_wk->mdlres.pResMdl );

	// モデルにバインドする
	BindTexture( p_wk->mdlres.pResMdl, p_wk->mdlres.pMdlTex );

	// ぽりごんID を設定
	NNS_G3dMdlSetMdlPolygonIDAll( p_wk->mdlres.pModel, WFLBY_3DOBJ_SHADOW_OBJID );
#else
	//  モデルデータ読み込み＆テクスチャ転送＆テクスチャ実データ破棄
	WFLBY_3DMAPOBJ_TEX_LoatCutTex( &p_wk->mdlres, p_handle, dataidx, gheapID );

	//ポリゴンID を設定
	{
		NNSG3dResFileHeader *file_head;
		NNSG3dResMdl *pMdl;
		
		file_head = GFL_G3D_GetResourceFileHeader(p_wk->mdlres);
		pMdl = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(file_head), 0);
		NNS_G3dMdlSetMdlPolygonIDAll( pMdl, WFLBY_3DOBJ_SHADOW_OBJID );
	}
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dモデルリソースを破棄する
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMDL_RES_Delete( WFLBY_3DMDLRES* p_wk )
{
#if WB_FIX
	D3DOBJ_MdlDelete( &p_wk->mdlres );
#else
	GFL_G3D_FreeVramTexture(p_wk->mdlres);
	GFL_G3D_DeleteResource(p_wk->mdlres);
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	3Dモデルリソースにアルファ値を設定
 *
 *	@param	p_wk		ワーク
 *	@param	alpha		アルファ値
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMDL_RES_SetAlpha( WFLBY_3DMDLRES* p_wk, u32 alpha )
{
#if WB_FIX
	p_wk->alpha = alpha;
	NNS_G3dMdlSetMdlAlphaAll( p_wk->mdlres.pModel, p_wk->alpha );
#else
	NNSG3dResFileHeader *file_head;
	NNSG3dResMdl *pMdl;
	
	p_wk->alpha = alpha;

	file_head = GFL_G3D_GetResourceFileHeader(p_wk->mdlres);
	pMdl = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(file_head), 0);
	NNS_G3dMdlSetMdlAlphaAll( pMdl, p_wk->alpha );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dモデルリソースからアルファ値を取得
 *
 *	@param	cp_wk		ワーク
 */
//-----------------------------------------------------------------------------
static u32 WFLBY_3DMDL_RES_GetAlpha( const WFLBY_3DMDLRES* cp_wk )
{
	return cp_wk->alpha;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dモデルリソースを使った表示オブジェワークを初期化
 *
 *	@param	p_wk		ワーク
 *	@param	p_obj		表示オブジェクト
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMDL_RES_InitD3DOBJ( WFLBY_3DMDLRES* p_wk, GFL_G3D_OBJ** p_obj )
{
#if WB_FIX
	D3DOBJ_Init( p_obj, &p_wk->mdlres );
#else
	GFL_G3D_RND* g3drnd;
	
	g3drnd = GFL_G3D_RENDER_Create( p_wk->mdlres, 0, p_wk->mdlres );
	*p_obj = GFL_G3D_OBJECT_Create( g3drnd, NULL, 0 );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dモデルリソースを使った表示オブジェワークを削除
 *
 *	@param	p_obj		表示オブジェクト
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMDL_RES_DeleteD3DOBJ( GFL_G3D_OBJ** p_obj )
{
	GFL_G3D_RENDER_Delete( GFL_G3D_OBJECT_GetG3Drnd(*p_obj) );
	GFL_G3D_OBJECT_Delete( *p_obj );
	*p_obj = NULL;
}


//----------------------------------------------------------------------------
/**
 *	@brief	空いているワークを取得する
 *
 *	@param	p_sys	システムワーク
 *
 *	@return	空いているワーク
 */
//-----------------------------------------------------------------------------
static WFLBY_3DOBJWK* WFLBY_3DOBJSYS_GetCleanWk( WFLBY_3DOBJSYS* p_sys )
{
	int i;


	for( i=0; i<p_sys->objnum; i++ ){
		if( WFLBY_3DOBJWK_CheckMove( &p_sys->p_obj[i] ) == FALSE ) {
			return &p_sys->p_obj[i];
		}
	}

	GF_ASSERT( 0 );
	return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ワークが動作中かチェックする
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	動作中
 *	@retval	FALSE	動作してない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_3DOBJWK_CheckMove( const WFLBY_3DOBJWK* cp_wk )
{
	if( cp_wk->cp_objwk == NULL ){
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェクトワークを空にする
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_CleanWk( WFLBY_3DOBJWK* p_wk )
{
	GFL_STD_MemFill( p_wk, 0, sizeof(WFLBY_3DOBJWK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	３Dオブジェワーク	人物以外の描画処理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_Draw( WFLBY_3DOBJWK* p_wk )
{
	// 影の描画
#if WB_FIX
	D3DOBJ_Draw( &p_wk->shadow );
#else
	if(p_wk->shadow_draw_flag == TRUE){
		GFL_G3D_DRAW_DrawObject(p_wk->shadow, &p_wk->shadow_st);
	}
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェクトワーク	更新
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_Updata( WFLBY_3DOBJWK* p_wk )
{
	u32 st;
	u32 frame;
	WF2DMAP_POS pos;
	static void (*pUpdata[WF2DMAP_OBJST_NUM])( WFLBY_3DOBJWK* p_wk ) = {
		WFLBY_3DOBJWK_Updata_Normal,
		WFLBY_3DOBJWK_Updata_Turn,
		WFLBY_3DOBJWK_Updata_Walk,
		WFLBY_3DOBJWK_Updata_Run,
		WFLBY_3DOBJWK_Updata_Normal,
		WFLBY_3DOBJWK_Updata_Walk,
		WFLBY_3DOBJWK_Updata_Walk,
		WFLBY_3DOBJWK_Updata_Walk,
		WFLBY_3DOBJWK_Updata_Walk,
		WFLBY_3DOBJWK_Updata_Walk,
		WFLBY_3DOBJWK_Updata_Walk,
	};
	
	// 更新フラグがたっていなければ更新しない
	if( p_wk->updata == FALSE ){
		return ;
	}

	// オブジェクトのフレーム数に対応したアニメを流す
	st		= WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_ST );
	frame	= WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_FRAME );

	// 以前のフレーム数より今のフレーム数が小さいとき
	// 状態が変更したときに以前のアニメとフレーム数を保存する
#if WB_FIX
	if( (p_wk->lastfrm > frame) || (p_wk->lastst != st) ){
		if( WFLBY_3DOBJWK_GetAnmSave( p_wk->lastst ) == TRUE ){
			// アニメフレームを保存する
			p_wk->lastanm	= BLACT_AnmOffsGet( p_wk->p_act );
			p_wk->lastframe = BLACT_AnmFrameGetOffs( p_wk->p_act );
		}
		p_wk->lastst = st;
	}
#else
	p_wk->lastanm	= GFL_BBDACT_GetAnimeIdx(p_wk->p_blact, p_wk->act_idx);
	p_wk->lastframe = GFL_BBDACT_GetAnimeFrmIdx(p_wk->p_blact, p_wk->act_idx);
#endif
	p_wk->lastfrm	= frame;
	
	// 更新
	pUpdata[ st ]( p_wk );

	// 座標を反映
	pos = WF2DMAP_OBJWkFrameMatrixGet( p_wk->cp_objwk );
	WFLBY_3DOBJWK_SetMatrix( p_wk, &pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief	その場待機アニメ
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_Updata_Normal( WFLBY_3DOBJWK* p_wk )
{
	u32 way;
	u32 anm;

	// WAYの方向を向いているだけ
	way	= WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_WAY );

	// 方向のアニメオフセット取得
	anm = WFLBY_3DMDL_ANM_GetAnmOffs( TRUE, way );

	// アクターに設定
#if WB_FIX
	BLACT_AnmOffsChg( p_wk->p_act, anm );
	BLACT_AnmFrameSetOffs( p_wk->p_act, 0 );
#else
	GFL_BBDACT_SetAnimeIdx(p_wk->p_blact, p_wk->act_idx, anm);
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	歩きアニメ
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_Updata_Walk( WFLBY_3DOBJWK* p_wk )
{
	u32 way;
	u32 anm;
	fx32 frame;
	u16 frame_max;
	u16 now_frame;

	// WAYの方向にあるく
	way	= WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_WAY );

	// 方向のアニメオフセット取得
	anm = WFLBY_3DMDL_ANM_GetAnmOffs( TRUE, way );

	// フレーム数取得
	frame_max = WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_ENDFRAME );
	now_frame = WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_FRAME );
	frame = WFLBY_3DMDL_ANM_GetAnmFrame( frame_max, now_frame, WFLBY_3DOBJ_ONEANM_HFRAME );

	// アクターに設定
#if WB_FIX
	BLACT_AnmOffsChg( p_wk->p_act, anm );
#else
	;
#endif

	// 前のアニメと一緒なら前のフレームから続ける
#if WB_FIX
	if( p_wk->lastanm == anm ){
		BLACT_AnmFrameSetOffs( p_wk->p_act, 0 );
		BLACT_AnmFrameChg( p_wk->p_act, frame+p_wk->lastframe );
	}else{
		BLACT_AnmFrameSetOffs( p_wk->p_act, 0 );
		BLACT_AnmFrameChg( p_wk->p_act, frame );
	}
#else
	if( p_wk->lastanm == anm ){
		GFL_BBDACT_SetAnimeIdxContinue(p_wk->p_blact, p_wk->act_idx, anm);
//※check		GFL_BBDACT_SetAnimeFrmIdx(p_wk->p_blact, p_wk->act_idx, frame);
	}else{
		GFL_BBDACT_SetAnimeIdx(p_wk->p_blact, p_wk->act_idx, anm);
	}
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	振り向きアニメ
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_Updata_Turn( WFLBY_3DOBJWK* p_wk )
{
	u32 way;
	u32 anm;
	u16 now_frame;

	// フレーム数取得
	now_frame = WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_FRAME );

	way	= WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_WAY );
	//anm = WFLBY_3DMDL_ANM_GetAnmOffs( TRUE, way );
	anm = WFLBY_3DOBJ_ANM_TYPE_STOP_UP + way;
	
	if( now_frame < WFLBY_3DOBJ_TRUN_FIRST_FRAME ){
	#if WB_FIX
		BLACT_AnmFrameSetOffs( p_wk->p_act, WFLBY_3DOBJ_TRUN_FIRST_FRAME*FX32_ONE );	// 1歩前に
	#else
		GFL_BBDACT_SetAnimeIdxContinue(p_wk->p_blact, p_wk->act_idx, anm );
//		GFL_BBDACT_SetAnimeFrmIdx(p_wk->p_blact, p_wk->act_idx, 1);	// 1歩前に
	#endif
	}else{
		// アクターに設定
	#if WB_FIX
		BLACT_AnmOffsChg( p_wk->p_act, anm );
		BLACT_AnmFrameSetOffs( p_wk->p_act, 0 );
	#else
		GFL_BBDACT_SetAnimeIdx(p_wk->p_blact, p_wk->act_idx, anm);
	#endif
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	走りアニメ
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_Updata_Run( WFLBY_3DOBJWK* p_wk )
{
	u32 way;
	u32 anm;
	fx32 frame;
	u16 frame_max;
	u16 now_frame;

	// WAYの方向にあるく
	way	= WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_WAY );

	// 方向のアニメオフセット取得
	anm = WFLBY_3DMDL_ANM_GetAnmOffs( FALSE, way );

	// フレーム数取得
	frame_max = WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_ENDFRAME );
	now_frame = WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_FRAME );
	frame = WFLBY_3DMDL_ANM_GetAnmFrame( frame_max, now_frame, WFLBY_3DOBJ_ONEANM_RUNFRAME );

	// アクターに設定
#if WB_FIX
	BLACT_AnmOffsChg( p_wk->p_act, anm );
#else
	;
#endif

	// 前のアニメと一緒なら前のフレームから続ける
#if WB_FIX
	if( p_wk->lastanm == anm ){
		BLACT_AnmFrameSetOffs( p_wk->p_act, 0 );
		BLACT_AnmFrameChg( p_wk->p_act, frame+p_wk->lastframe );
	}else{
		BLACT_AnmFrameSetOffs( p_wk->p_act, 0 );
		BLACT_AnmFrameChg( p_wk->p_act, frame );
	}
#else
	if( p_wk->lastanm == anm ){
		GFL_BBDACT_SetAnimeIdxContinue(p_wk->p_blact, p_wk->act_idx, anm);
//※check		GFL_BBDACT_SetAnimeFrmIdx(p_wk->p_blact, p_wk->act_idx, frame+p_wk->lastframe);
	}else{
		GFL_BBDACT_SetAnimeIdx(p_wk->p_blact, p_wk->act_idx, anm);
	}
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	特殊アニメアップデート処理
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_AnmUpdata( WFLBY_3DOBJWK* p_wk )
{
	static void (*pUpdata[WFLBY_3DOBJ_ANM_NUM])( WFLBY_3DOBJWK* p_wk ) = {
		WFLBY_3DOBJWK_Anm_Rota,
		WFLBY_3DOBJWK_Anm_Jump,
		WFLBY_3DOBJWK_Anm_BataBata,
	};

	// 更新停止＋アニメ再生中なら設定
	if( (p_wk->updata == FALSE) && (p_wk->anmflag == TRUE) ){
		pUpdata[ p_wk->anm ]( p_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	特殊アニメ		回転
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_Anm_Rota( WFLBY_3DOBJWK* p_wk )
{
	u32 anmidx;
	u32 blact_anm;
	
	if( (p_wk->anmframe % WFLBY_3DOBJ_ANM_ROTA_1SYNC) == 0 ){
		// アニメ変更
		anmidx = p_wk->anmframe / WFLBY_3DOBJ_ANM_ROTA_1SYNC;
		blact_anm = WFLBY_3DMDL_ANM_GetAnmOffs( TRUE, sc_WFLBY_3DOBJ_ANM_WAY[ anmidx ] );
	#if WB_FIX
		BLACT_AnmOffsChg( p_wk->p_act, blact_anm );
		BLACT_AnmFrameSetOffs( p_wk->p_act, 0 );
	#else
		GFL_BBDACT_SetAnimeIdx(p_wk->p_blact, p_wk->act_idx, blact_anm);
	#endif
	}
	if( (p_wk->anmframe + p_wk->anmspeed) < (WFLBY_3DOBJ_ANM_ROTA_1SYNC*WF2DMAP_WAY_NUM) ){
		p_wk->anmframe += p_wk->anmspeed;
	}else{
		p_wk->anmframe = 0;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ジャンプアニメ
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_Anm_Jump( WFLBY_3DOBJWK* p_wk )
{
	u32 way;
	u32 anm;
	
	if( p_wk->anmframe == 0 ){
		// 向いている方向に歩くアニメの２こま目を設定
		way = WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_WAY );


		// WAYの方向を向いているだけ
		way	= WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_WAY );

		// 方向のアニメオフセット取得
		anm = WFLBY_3DMDL_ANM_GetAnmOffs( TRUE, way );

		// アクターに設定
	#if WB_FIX
		BLACT_AnmOffsChg( p_wk->p_act, anm );
		BLACT_AnmFrameSetOffs( p_wk->p_act, WFLBY_3DOBJ_TRUN_FIRST_FRAME*FX32_ONE );
	#else
		GFL_BBDACT_SetAnimeIdx(p_wk->p_blact, p_wk->act_idx, anm);
		GFL_BBDACT_SetAnimeFrmIdx(p_wk->p_blact, p_wk->act_idx, WFLBY_3DOBJ_TRUN_FIRST_FRAME);
	#endif
		p_wk->anmframe++;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	あしバタバタアニメ
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_Anm_BataBata( WFLBY_3DOBJWK* p_wk )
{
	u32 way;
	u32 anm;
	
	if( p_wk->anmframe == 0 ){
		// 向いている方向に歩くアニメの1こま目を設定
		way = WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_WAY );

		// 方向のアニメオフセット取得
		anm = WFLBY_3DMDL_ANM_GetAnmOffs( TRUE, way );

		// アクターに設定
	#if WB_FIX
		BLACT_AnmOffsChg( p_wk->p_act, anm );
		BLACT_AnmFrameSetOffs( p_wk->p_act, WFLBY_3DOBJ_TRUN_FIRST_FRAME*FX32_ONE );
	#else
		GFL_BBDACT_SetAnimeIdx(p_wk->p_blact, p_wk->act_idx, anm);
		GFL_BBDACT_SetAnimeFrmIdx(p_wk->p_blact, p_wk->act_idx, WFLBY_3DOBJ_TRUN_FIRST_FRAME);
	#endif
	}else if( p_wk->anmframe == WFLBY_3DOBJ_ANM_BATA_FRAME ){

		// 向いている方向に歩くアニメの3こま目を設定
		way = WF2DMAP_OBJWkDataGet( p_wk->cp_objwk, WF2DMAP_OBJPM_WAY );

		// 方向のアニメオフセット取得
		anm = WFLBY_3DMDL_ANM_GetAnmOffs( TRUE, way );

		// アクターに設定
	#if WB_FIX
		BLACT_AnmOffsChg( p_wk->p_act, anm );
		BLACT_AnmFrameSetOffs( p_wk->p_act, (WFLBY_3DOBJ_TRUN_FIRST_FRAME*3)*FX32_ONE );
	#else
		GFL_BBDACT_SetAnimeIdx(p_wk->p_blact, p_wk->act_idx, anm);
		GFL_BBDACT_SetAnimeFrmIdx(p_wk->p_blact, p_wk->act_idx, (WFLBY_3DOBJ_TRUN_FIRST_FRAME*3) );
	#endif
	}

	p_wk->anmframe = (p_wk->anmframe + 1) % (WFLBY_3DOBJ_ANM_BATA_FRAME*2);
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメを保存する状態なのかチェックする
 *
 *	@param	st		状態
 *
 *	@retval	TRUE	セーブする
 *	@retval	FALSE	セーブしない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_3DOBJWK_GetAnmSave( u32 st )
{
	switch( st ){
	case WF2DMAP_OBJST_WALK:
	case WF2DMAP_OBJST_RUN:
	case WF2DMAP_OBJST_WALLWALK:
	case WF2DMAP_OBJST_SLOWWALK:
	case WF2DMAP_OBJST_STAYWALK8:
	case WF2DMAP_OBJST_STAYWALK16:
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カリングチェック
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_CheckCulling( WFLBY_3DOBJSYS* p_sys, WFLBY_3DOBJWK* p_wk )
{
	BOOL result;


	if( WFLBY_3DOBJWK_CheckMove( p_wk ) ){
		
		result = WFLBY_3DOBJWK_CheckCullingBlact( p_sys, p_wk->p_blact, p_wk->act_idx );
		
		if( result == FALSE ){
			p_wk->culling = TRUE;
		}else{
			p_wk->culling = FALSE;
		}

		// 表示設定
		WFLBY_3DOBJWK_ContBlactDrawFlag( p_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画管理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DOBJWK_ContBlactDrawFlag( WFLBY_3DOBJWK* p_wk )
{
	if( (p_wk->culling == FALSE) && (p_wk->objdraw == TRUE) ){
		// 表示ON
	#if WB_FIX
		BLACT_ObjDrawFlagSet( p_wk->p_act, TRUE );
		D3DOBJ_SetDraw( &p_wk->shadow, TRUE );
	#else
		GFL_BBDACT_SetDrawEnable(p_wk->p_blact, p_wk->act_idx, TRUE);
		p_wk->shadow_draw_flag = TRUE;
	#endif
	}else{
		// 表示OFF
	#if WB_FIX
		BLACT_ObjDrawFlagSet( p_wk->p_act, FALSE );
		D3DOBJ_SetDraw( &p_wk->shadow, FALSE );
	#else
		GFL_BBDACT_SetDrawEnable(p_wk->p_blact, p_wk->act_idx, FALSE);
		p_wk->shadow_draw_flag = FALSE;
	#endif
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビルボードアクター	描画前コールバック
 *
 *	@param	p_act		アクター
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
#if WB_FIX
static void WFLBY_3DOBJWK_CallBack_BlactDraw( GFL_BBDACT_ACTUNIT_ID p_act ,void* p_work )
{
	WFLBY_3DOBJWK* p_wk = p_work;
	NNSG3dResMdl*  p_mdl;

	p_mdl = BLACT_ResMdlGet( p_act );
	NNS_G3dMdlSetMdlLightEnableFlagAll( p_mdl, p_wk->set_light_msk );
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief	ビルボードアクター専用	カリング処理
 *			常駐を増やしたくないので	無理やりなやり方で行います
 *
 *	@param	p_act	アクター
 *		
 *	@retval	TRUE	描画する
 *	@retval	FALSE	描画しない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_3DOBJWK_CheckCullingBlact(WFLBY_3DOBJSYS* p_sys, GFL_BBDACT_SYS *p_blact, GFL_BBDACT_ACTUNIT_ID act_idx)
{
	BOOL ret;
	VecFx32 matrix;
	NNSG3dResMdlInfo*	p_mdlinfo;	
	BOUNDING_BOX		box;
	MtxFx33				mtx;
	NNSG3dResMdl*		p_mdlres;
	
	// リソース取得
#if WB_FIX
	p_mdlres = BLACT_MdlResGet( p_act );
	p_mdlinfo = NNS_G3dGetMdlInfo( p_mdlres );		// モデルリソースインフォデータ取得
#else
	p_mdlres = NNS_G3dGetMdlByIdx( NNS_G3dGetMdlSet( p_sys->idx_file ), 0 );
	p_mdlinfo = NNS_G3dGetMdlInfo( p_mdlres );		// モデルリソースインフォデータ取得
#endif

	// 座標
#if WB_FIX
	matrix = *BLACT_MatrixGet(p_act);
#else
	GFL_BBD_GetObjectTrans(GFL_BBDACT_GetBBDSystem(p_blact), act_idx, &matrix);
#endif

	// ビルボードは板なのでモデルとしては奥行きがないが、
	// カメラの方向を向くように回転するので、
	// その角度によっては、奥行きが出てくる。
	// ビルボードがどんな方向に回転しても大丈夫なように
	// 奥行きに高さの大きさを入れる
	box.w		= p_mdlinfo->boxW;
	box.h		= p_mdlinfo->boxH;
	box.d		= p_mdlinfo->boxH;
	box.ScaleW	= p_mdlinfo->boxPosScale >> FX32_SHIFT;
	box.ScaleH	= p_mdlinfo->boxPosScale >> FX32_SHIFT;
	box.ScaleD	= p_mdlinfo->boxPosScale >> FX32_SHIFT;

	matrix.x	+= FX_Mul( p_mdlinfo->boxX, p_mdlinfo->boxPosScale );
	matrix.y	+= FX_Mul( p_mdlinfo->boxY, p_mdlinfo->boxPosScale );
	matrix.z	+= FX_Mul( p_mdlinfo->boxZ, p_mdlinfo->boxPosScale );
	matrix.z	-= FX_Mul( p_mdlinfo->boxH, p_mdlinfo->boxPosScale );	// 奥行きを高さ分増やすので、Zの値もH分移動させる

	MTX_Identity33( &mtx );
	NNS_G3dGlbSetBaseRot( &mtx );
	
	// スケール設定
#if WB_FIX
	NNS_G3dGlbSetBaseScale( BLACT_ScaleGet(p_act) );
#else
	{
		VecFx32 scale = {FX32_ONE, FX32_ONE, FX32_ONE};
		NNS_G3dGlbSetBaseScale( &scale );
	}
#endif

	ret = BB_CullingCheck3DModelNonResQuick( &matrix, &box );


	if( ret == 0 ){
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *
 *@brief	バウンディングボックスセット
 *
 *@param	x		基準X座標
 *@param	y		基準Y座標
 *@param	z		基準Z座標
 *@param	width	幅
 *@param	height	高さ
 *@param	depth	奥行き
 *@param	outBox	ボックステスト用パラメータ格納アドレス
 *
 *@return	なし 
 *
 */
//-----------------------------------------------------------------------------
static void SetBoxSize( const fx16 x,
						const fx16 y,
						const fx16 z,
						const fx16 width,
						const fx16 height,
						const fx16 depth,
						GXBoxTestParam *outBox)
{
	//
	// パラメータをセット
	//
	outBox->x		= x;				// 左上のｘ座標	
	outBox->y		= y;				// 左上のｙ座標
	outBox->z		= z;				// 左上のｚ座標
	outBox->width	= width;			// 四角の幅
	outBox->height	= height;			// 四角の高さ
	outBox->depth	= depth;			// 立方体の奥行き
}

//----------------------------------------------------------------------------
/**
 *
 *@brief	ボックスチェック
 *
 *@param	inBox	ボックステスト用パラメータへのポインタ
 *
 *@return	u32		0以外の値であれば、ボックスの一部または全部が視体積内
 *					0であれば、ボックスの全てが視体積外 
 *
 */
//-----------------------------------------------------------------------------
static u32 CheckBoundingBox( const GXBoxTestParam	*inBox )
{
	s32 result = 1;			// 結果

	// ポリゴン設定を行う
	NNS_G3dGePolygonAttr(
			GX_LIGHTMASK_0,         
			GX_POLYGONMODE_MODULATE,
			GX_CULL_NONE,           
			0,                      
			0,                      
			GX_POLYGON_ATTR_MISC_FAR_CLIPPING
			| GX_POLYGON_ATTR_MISC_DISP_1DOT
			);

	//ポリゴンアトリビュート反映 
	NNS_G3dGeBegin( GX_BEGIN_TRIANGLES );							
	NNS_G3dGeEnd();	

	//
	// ボックステスト
	//
	NNS_G3dGeBoxTest( inBox );

	//
	// 同期を取る
	//
	NNS_G3dGeFlushBuffer();

	//
	// 今設定されているボックスが視体積内かチェック
	//
	while ( G3X_GetBoxTestResult(&result) != 0 ) ;

	return result;
}

//----------------------------------------------------------------------------
/**
 *
 *@brief	カリングチェック(リソースを使用しない)
 *
 *@param	trans_p			平行移動値
 *@param	rot_p			回転行列
 *@param	scale_p			拡縮値
 *
 *@return	u32			0以外の値であれば、ボックスの一部または全部が視体積内
 *						0であれば、ボックスの全てが視体積外
 *
 */
//-----------------------------------------------------------------------------
static u32	BB_CullingCheck3DModelNonResQuick(	const VecFx32* trans_p, const BOUNDING_BOX *inBox)
{
	u32 check_ret;		// チェックの戻り値用
	GXBoxTestParam bounding_box;

	SetBoxSize( 
			0,
			0,
			0,
			inBox->w,	// 幅
			inBox->h,	// 高さ
			inBox->d,	// 奥行き
			&bounding_box);

	//
	// 座標をセット
	//
	// 位置設定
	NNS_G3dGlbSetBaseTrans(trans_p);
	// 角度設定
	///NNS_G3dGlbSetBaseRot(rot_p);
	// スケール設定
	///NNS_G3dGlbSetBaseScale(scale_p);
	
	NNS_G3dGlbFlush();		//　ジオメトリコマンドを転送

	// 上のボックステストの箱の値をposScaleでかけることにより本当の値になる
	NNS_G3dGePushMtx();
	NNS_G3dGeScale( inBox->ScaleW*FX32_ONE,
					inBox->ScaleH*FX32_ONE,
					inBox->ScaleD*FX32_ONE );

	check_ret = CheckBoundingBox(&bounding_box);	// チェック

	NNS_G3dGePopMtx(1);
	
	return check_ret;

}

