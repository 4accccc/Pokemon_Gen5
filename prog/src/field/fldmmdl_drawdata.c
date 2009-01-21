//******************************************************************************
/**
 *
 * @file	fieldobj_drawdata.c
 * @brief	フィールド動作モデル描画データ
 * @author	kagaya
 * @data	05.07.20
 *
 */
//******************************************************************************

//==============================================================================
/*
	＜新規OBJの追加＞
	１：fieldobj_code.hに表示コードを追加
	２：ビルボードであればDATA_FLDMMDL_ResmNArcTbl_Tex[]へ表示コードに対応したテクスチャを指定
	３：FLDMMDL_DRAW_PROC_LIST型の初期化、描画、削除、退避復帰関数を追加
	４：DATA_FieldOBJDrawProcListRegTbl[]へ３で作成した関数の定義
	５：DATA_FLDMMDL_BlActHeaderPathIDTbl[]へ表示コードに対応した情報を追加
	６：DATA_FieldOBJCodeDrawStateTbl[]へ表示コードのステータスを追加
	
	＜新規モデリングの追加＞
	１：fieldobj_code.hにモデルIDを追加
	２：DATA_FLDMMDL_ResmNArcTbl_Mdl[]へモデルIDに対応したモデリングを指定
	３：常駐する必要がある場合、DATA_FLDMMDL_Regular_Mdl[]へ登録
	
	※フォグ無効化モデルを指定する場合
	DATA_FLDMMDL_BlActFogEnableOFFTbl[]へ登録
	
	＜新規アニメの追加＞
	１：fieldobj_code.hにアニメIDを追加
	２：DATA_FLDMMDL_ResmNArcTbl_Anm[]へアニメIDに対応したモデリングを指定
	３：常駐する必要がある場合、DATA_FLDMMDL_Regular_Anm[]へ登録
*/
//==============================================================================
#include "fldmmdl.h"
#include "../../arc/test_graphic/mmodel.naix"

//==============================================================================
//	extern
//==============================================================================

//==============================================================================
//	フィールド動作モデル 描画関数定義
//==============================================================================
//--------------------------------------------------------------
///	HERO
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Hero =
{
	FLDMMDL_DrawBlActHero_Init,
	FLDMMDL_DrawBlActHero_Draw,
	FLDMMDL_DrawBlActHero_Delete,
	FLDMMDL_DrawBlActHero_Push,
	FLDMMDL_DrawBlActHero_Pop,
};

//--------------------------------------------------------------
///	CYCLEHERO
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_HeroCycle = 
{
	FLDMMDL_DrawBlActHero_Init,
	FLDMMDL_DrawBlActHeroCycle_Draw,
	FLDMMDL_DrawBlActHero_Delete,
	FLDMMDL_DrawBlActHero_Push,
	FLDMMDL_DrawBlActHero_Pop,
};

//--------------------------------------------------------------
///	SWIMHERO
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_HeroSwim =
{
	FLDMMDL_DrawBlActHero_Init,
	FLDMMDL_DrawBlActHeroSwim_Draw,
	FLDMMDL_DrawBlActHero_Delete,
	FLDMMDL_DrawBlActHero_Push,
	FLDMMDL_DrawBlActHero_Pop,
};

//--------------------------------------------------------------
///	SPHERO
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_HeroSp =
{
	FLDMMDL_DrawBlActHero_Init,
	FLDMMDL_DrawBlActHeroSp_Draw,
	FLDMMDL_DrawBlActHero_Delete,
	FLDMMDL_DrawBlActHero_Push,
	FLDMMDL_DrawBlActHero_Pop,
};

//--------------------------------------------------------------
///	WATERHERO
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_HeroWater =
{
	FLDMMDL_DrawBlActHero_Init,
	FLDMMDL_DrawBlActHeroWater_Draw,
	FLDMMDL_DrawBlActHero_Delete,
	FLDMMDL_DrawBlActHero_Push,
	FLDMMDL_DrawBlActHero_Pop,
};

//--------------------------------------------------------------
///	FISHINGHERO
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_HeroFishing =
{
	FLDMMDL_DrawBlActHero_Init,
	FLDMMDL_DrawBlActHeroFishing_Draw,
	FLDMMDL_DrawBlActHero_Delete,
	FLDMMDL_DrawBlActHero_Push,
	FLDMMDL_DrawBlActHero_Pop,
};

//--------------------------------------------------------------
///	POKEHERO
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_HeroPoketch =
{
	FLDMMDL_DrawBlActHero_Init,
	FLDMMDL_DrawBlActHeroPoketch_Draw,
	FLDMMDL_DrawBlActHero_Delete,
	FLDMMDL_DrawBlActHero_Push,
	FLDMMDL_DrawBlActHero_Pop,
};

//--------------------------------------------------------------
///	BANZAIHERO
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_HeroBanzai =
{
	FLDMMDL_DrawBlActHero_Init,
	FLDMMDL_DrawBlActHeroBanzai_Draw,
	FLDMMDL_DrawBlActHero_Delete,
	FLDMMDL_DrawBlActHero_Push,
	FLDMMDL_DrawBlActHero_Pop,
};

//--------------------------------------------------------------
///	ビルボードアクター使用　一般人標準00
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_BlAct00 =
{
	FLDMMDL_DrawBlAct00_Init,
	FLDMMDL_DrawBlAct00_Draw,
	FLDMMDL_DrawBlAct00_Delete,
	FLDMMDL_DrawBlAct00_Push,
	FLDMMDL_DrawBlAct00_Pop,
};

//--------------------------------------------------------------
///	ビルボードアクター使用　一般人標準00　アニメなし
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_BlAct00AnmNon =
{
	FLDMMDL_DrawBlAct00_Init,
	FLDMMDL_DrawBlAct00AnmNon_Draw,
	FLDMMDL_DrawBlAct00_Delete,
	FLDMMDL_DrawBlAct00_Push,
	FLDMMDL_DrawBlAct00_Pop,
};

//--------------------------------------------------------------
///	ビルボードアクター使用　一般人標準00　１パターンアニメ
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_BlAct00AnmOneP =
{
	FLDMMDL_DrawBlAct00_Init,
	FLDMMDL_DrawBlAct00AnmOneP_Draw,
	FLDMMDL_DrawBlAct00_Delete,
	FLDMMDL_DrawBlAct00_Push,
	FLDMMDL_DrawBlAct00_Pop,
};

//--------------------------------------------------------------
///	ビルボードアクター使用　PCWOMAN
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_BlActPcwoman =
{
	FLDMMDL_DrawBlAct00_Init,
	FLDMMDL_DrawBlActPcWoman_Draw,
	FLDMMDL_DrawBlAct00_Delete,
	FLDMMDL_DrawBlAct00_Push,
	FLDMMDL_DrawBlAct00_Pop,
};

//--------------------------------------------------------------
///	ビルボードアクター使用　KOIKING
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_BlActKoiking =
{
	FLDMMDL_DrawBlActKoiking_Init,
	FLDMMDL_DrawBlActKoiking_Draw,
	FLDMMDL_DrawBlActKoiking_Delete,
	FLDMMDL_DrawBlActKoiking_Push,
	FLDMMDL_DrawBlActKoiking_Pop,
};

//--------------------------------------------------------------
///	3Dモデル使用　標準00
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Model00 =
{
	FLDMMDL_DrawModel00_Init,
	FLDMMDL_DrawModel00_Draw,
	FLDMMDL_DrawModel00_Delete,
	FLDMMDL_DrawModel00_Push,
	FLDMMDL_DrawModel00_Pop,
};

#if 0
//--------------------------------------------------------------
///	3Dモデル使用　標準00　看板
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Model00Board =
{
	FLDMMDL_DrawModel00_Board_Init,
	FLDMMDL_DrawModel00_Draw,
	FLDMMDL_DrawModel00_Delete,
	FLDMMDL_DrawModel00_Push,
	FLDMMDL_DrawModel00_Board_Pop,
};
#endif

//--------------------------------------------------------------
///	木の実
//--------------------------------------------------------------
#if 0
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Seed =
{
	FLDMMDL_DrawSeed_Init,
	FLDMMDL_DrawSeed_Draw,
	FLDMMDL_DrawSeed_Delete,
	FLDMMDL_DrawSeed_Push,
	FLDMMDL_DrawSeed_Pop,
};
#endif

//--------------------------------------------------------------
///	ビルボードアクター使用　鳥
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_BlActBird =
{
	FLDMMDL_DrawBlAct00_Init,
	FLDMMDL_DrawBlActBird_Draw,
	FLDMMDL_DrawBlAct00_Delete,
	FLDMMDL_DrawBlAct00_Push,
	FLDMMDL_DrawBlAct00_Pop,
};

//--------------------------------------------------------------
///	3Dモデル使用　標準00　雪玉
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Model00SnowBall =
{
	FLDMMDL_DrawModel00_SnowBall_Init,
	FLDMMDL_DrawModel00_Draw,
	FLDMMDL_DrawModel00_Delete,
	FLDMMDL_DrawModel00_Push,
	FLDMMDL_DrawModel00_SnowBall_Pop,
};

#if 0
//--------------------------------------------------------------
///	3Dモデル使用　標準00　本
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Model00Book =
{
	FLDMMDL_DrawModel00_Book_Init,
	FLDMMDL_DrawModel00_Draw,
	FLDMMDL_DrawModel00_Delete,
	FLDMMDL_DrawModel00_Push,
	FLDMMDL_DrawModel00_Book_Pop,
};

//--------------------------------------------------------------
///	3Dモデル使用　標準00　ドア２
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Model00Door2 =
{
	FLDMMDL_DrawModel00_Door2_Init,
	FLDMMDL_DrawModel00_Draw,
	FLDMMDL_DrawModel00_Delete,
	FLDMMDL_DrawModel00_Push,
	FLDMMDL_DrawModel00_Door2_Pop,
};
#endif

//--------------------------------------------------------------
///	NONDRAW
//--------------------------------------------------------------
const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Non =
{
	FLDMMDL_DrawNon_Init,
	FLDMMDL_DrawNon_Draw,
	FLDMMDL_DrawNon_Delete,
	FLDMMDL_DrawNon_Push,
	FLDMMDL_DrawNon_Pop,
};

//--------------------------------------------------------------
// @author	tomoya katahashi
///	LEGENDPOKE		伝説ポケモン　イア　エア
//--------------------------------------------------------------
const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_Legend =
{
	FLDMMDL_DrawLegend_Init,
	FLDMMDL_DrawLegend_Draw,
	FLDMMDL_DrawLegend_Delete,
	FLDMMDL_DrawLegend_Push,
	FLDMMDL_DrawLegend_Pop,
};

//--------------------------------------------------------------
// @author	tomoya katahashi
///	LEGENDPOKE		伝説ポケモン　レイ　アイ　ハイ
//--------------------------------------------------------------
const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_ReiAiHai =
{
	FLDMMDL_DrawReiAiHai_Init,
	FLDMMDL_DrawReiAiHai_Draw,
	FLDMMDL_DrawReiAiHai_Delete,
	FLDMMDL_DrawLegend_Push,
	FLDMMDL_DrawLegend_Pop,
};

//--------------------------------------------------------------
///	ビルボード機能オフ
//--------------------------------------------------------------
const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_BOff =
{
	FLDMMDL_DrawBlActOff_Init,
	FLDMMDL_DrawBlActOff_Draw,
	FLDMMDL_DrawBlActOff_Delete,
	FLDMMDL_DrawBlActOff_Push,
	FLDMMDL_DrawBlActOff_Pop,
};

//--------------------------------------------------------------
///	破れた世界　アグノム専用
//--------------------------------------------------------------
const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_BlActBldAgunomu =
{
	FLDMMDL_DrawBlActBldAgunomu_Init,
	FLDMMDL_DrawBlActAgunomu_Draw,
	FLDMMDL_DrawBlActBldAgunomu_Delete,
	FLDMMDL_DrawBlActBldAgunomu_Push,
	FLDMMDL_DrawBlActBldAgunomu_Pop,
};

//--------------------------------------------------------------
///	破れた世界　七層SPPOKE専用
//--------------------------------------------------------------
const FLDMMDL_DRAW_PROC_LIST DATA_FieldOBJDraw_BlActTw7SpPoke =
{
	FLDMMDL_DrawBlActTw7SpPoke_Init,
	FLDMMDL_DrawBlActTw7SpPoke_Draw,
	FLDMMDL_DrawBlActTw7SpPoke_Delete,
	FLDMMDL_DrawBlActTw7SpPoke_Push,
	FLDMMDL_DrawBlActTw7SpPoke_Pop,
};

//==============================================================================
//	フィールド動作モデル 描画関数まとめ
//==============================================================================
//--------------------------------------------------------------
///	描画関数まとめ　並びはOBJコードと一致->不順にした
//--------------------------------------------------------------
#if 0
const FLDMMDL_DRAW_PROC_LIST_REG DATA_FieldOBJDrawProcListRegTbl[] =
{
	{ HERO,			&DATA_FieldOBJDraw_Hero },
	{ BABYBOY1,		&DATA_FieldOBJDraw_BlAct00 },
	{ BABYGIRL1,	&DATA_FieldOBJDraw_BlAct00 },
	{ BOY1,			&DATA_FieldOBJDraw_BlAct00 },
	{ BOY2,			&DATA_FieldOBJDraw_BlAct00 },
	{ BOY3,			&DATA_FieldOBJDraw_BlAct00 },
	{ GIRL1,		&DATA_FieldOBJDraw_BlAct00 },
	{ GIRL2,		&DATA_FieldOBJDraw_BlAct00 },
	{ GIRL3,		&DATA_FieldOBJDraw_BlAct00 },
	{ MAN1,			&DATA_FieldOBJDraw_BlAct00 },
	{ MAN2,			&DATA_FieldOBJDraw_BlAct00 },
	{ MAN3,			&DATA_FieldOBJDraw_BlAct00 },
	{ WOMAN1,		&DATA_FieldOBJDraw_BlAct00 },
	{ WOMAN2,		&DATA_FieldOBJDraw_BlAct00 },
	{ WOMAN3,		&DATA_FieldOBJDraw_BlAct00 },
	{ MIDDLEMAN1,	&DATA_FieldOBJDraw_BlAct00 },
	{ MIDDLEWOMAN1,	&DATA_FieldOBJDraw_BlAct00 },
	{ OLDMAN1,		&DATA_FieldOBJDraw_BlAct00 },
	{ OLDWOMAN1,	&DATA_FieldOBJDraw_BlAct00 },
	{ BIGMAN,		&DATA_FieldOBJDraw_BlAct00 },
	{ MOUNT,		&DATA_FieldOBJDraw_BlAct00 },
	{ CYCLEHERO,	&DATA_FieldOBJDraw_HeroCycle },
	{REPORTER,	&DATA_FieldOBJDraw_BlAct00 },
	{CAMERAMAN,	&DATA_FieldOBJDraw_BlAct00 },
	{SHOPM1,	&DATA_FieldOBJDraw_BlAct00 },
	{SHOPW1,	&DATA_FieldOBJDraw_BlAct00 },
	{PCWOMAN1,	&DATA_FieldOBJDraw_BlActPcwoman },
	{PCWOMAN2,	&DATA_FieldOBJDraw_BlAct00 },
	{PCWOMAN3,	&DATA_FieldOBJDraw_BlAct00 },
	{ASSISTANTM,	&DATA_FieldOBJDraw_BlAct00 },
	{ASSISTANTW,	&DATA_FieldOBJDraw_BlAct00 },
	{BADMAN,	&DATA_FieldOBJDraw_BlAct00 },
	{SKIERM,	&DATA_FieldOBJDraw_BlAct00 },
	{SKIERW,	&DATA_FieldOBJDraw_BlAct00 },
	{POLICEMAN,	&DATA_FieldOBJDraw_BlAct00 },
	{IDOL,	&DATA_FieldOBJDraw_BlAct00 },
	{GENTLEMAN,	&DATA_FieldOBJDraw_BlAct00 },
	{LADY,	&DATA_FieldOBJDraw_BlAct00 },
	{CYCLEM,	&DATA_FieldOBJDraw_BlAct00 },
	{CYCLEW,	&DATA_FieldOBJDraw_BlAct00 },
	{WORKMAN,	&DATA_FieldOBJDraw_BlAct00 },
	{FARMER,	&DATA_FieldOBJDraw_BlAct00 },
	{COWGIRL,	&DATA_FieldOBJDraw_BlAct00 },
	{CLOWN,	&DATA_FieldOBJDraw_BlAct00 },
	{ARTIST,	&DATA_FieldOBJDraw_BlAct00 },
	{SPORTSMAN,	&DATA_FieldOBJDraw_BlAct00 },
	{SWIMMERM,	&DATA_FieldOBJDraw_BlAct00 },
	{SWIMMERW,	&DATA_FieldOBJDraw_BlAct00 },
	{BEACHGIRL,	&DATA_FieldOBJDraw_BlAct00 },
	{BEACHBOY,	&DATA_FieldOBJDraw_BlAct00 },
	{EXPLORE,	&DATA_FieldOBJDraw_BlAct00 },
	{FIGHTER,	&DATA_FieldOBJDraw_BlAct00 },
	{CAMPBOY,	&DATA_FieldOBJDraw_BlAct00 },
	{PICNICGIRL,	&DATA_FieldOBJDraw_BlAct00 },
	{FISHING,	&DATA_FieldOBJDraw_BlAct00 },
	{AMBRELLA,	&DATA_FieldOBJDraw_BlAct00 },
	{SEAMAN,	&DATA_FieldOBJDraw_BlAct00 },
	{BRINGM,	&DATA_FieldOBJDraw_BlAct00 },
	{BRINGW,	&DATA_FieldOBJDraw_BlAct00 },
	{WAITER,	&DATA_FieldOBJDraw_BlAct00 },
	{WAITRESS,	&DATA_FieldOBJDraw_BlAct00 },
	{VETERAN,	&DATA_FieldOBJDraw_BlAct00 },
	{GORGGEOUSM,	&DATA_FieldOBJDraw_BlAct00 },
	{GORGGEOUSW,	&DATA_FieldOBJDraw_BlAct00 },
	{BOY4,	&DATA_FieldOBJDraw_BlAct00 },
	{GIRL4,	&DATA_FieldOBJDraw_BlAct00 },
	{MAN4,	&DATA_FieldOBJDraw_BlAct00 },
	{WOMAN4,	&DATA_FieldOBJDraw_BlAct00 },
	{MAN5,	&DATA_FieldOBJDraw_BlAct00 },
	{WOMAN5,	&DATA_FieldOBJDraw_BlAct00 },
	{MYSTERY,	&DATA_FieldOBJDraw_BlAct00 },
	{PIKACHU,	&DATA_FieldOBJDraw_BlAct00 },
	{PIPPI,	&DATA_FieldOBJDraw_BlAct00 },
	{PURIN,	&DATA_FieldOBJDraw_BlAct00 },
	{KODUCK,	&DATA_FieldOBJDraw_BlAct00 },
	{PRASLE,	&DATA_FieldOBJDraw_BlAct00 },
	{MINUN,	&DATA_FieldOBJDraw_BlAct00 },
	{MARIL,	&DATA_FieldOBJDraw_BlAct00 },
	{ACHAMO,	&DATA_FieldOBJDraw_BlAct00 },
	{ENECO,	&DATA_FieldOBJDraw_BlAct00 },
	{GONBE,	&DATA_FieldOBJDraw_BlAct00 },
	{BABY,	&DATA_FieldOBJDraw_BlAct00 },
	{MIDDLEMAN2,	&DATA_FieldOBJDraw_BlAct00 },
	{MIDDLEWOMAN2,	&DATA_FieldOBJDraw_BlAct00 },
	{ROCK,	&DATA_FieldOBJDraw_BlAct00AnmNon },
	{BREAKROCK,	&DATA_FieldOBJDraw_BlAct00AnmNon },
	{TREE,	&DATA_FieldOBJDraw_BlAct00AnmNon },
	{MONATERBALL,	&DATA_FieldOBJDraw_BlAct00AnmNon },
	{SHADOW,	&DATA_FieldOBJDraw_BlAct00 },
	{BOUGH,	&DATA_FieldOBJDraw_BlAct00 },
	{FALLTREE,	&DATA_FieldOBJDraw_BlAct00 },
	{ BOARD_A, &DATA_FieldOBJDraw_Model00 },
	{ BOARD_B, &DATA_FieldOBJDraw_Model00 },
	{ BOARD_C, &DATA_FieldOBJDraw_Model00 },
	{ BOARD_D, &DATA_FieldOBJDraw_Model00 },
	{ BOARD_E, &DATA_FieldOBJDraw_Model00 },
	{ BOARD_F, &DATA_FieldOBJDraw_Model00 },
	{ HEROINE,	&DATA_FieldOBJDraw_Hero },
	{ CYCLEHEROINE,	&DATA_FieldOBJDraw_HeroCycle },
	{ DOCTOR,	&DATA_FieldOBJDraw_BlAct00 },
#if 0
	{ SEED, &DATA_FieldOBJDraw_Seed },
#else
	{ SEED, NULL },
#endif
	{ SNOWBALL, &DATA_FieldOBJDraw_Model00SnowBall },
	{ GINGABOSS, &DATA_FieldOBJDraw_BlAct00 },
	{ GKANBU1, &DATA_FieldOBJDraw_BlAct00 },
	{ GKANBU2, &DATA_FieldOBJDraw_BlAct00 },
	{ GKANBU3, &DATA_FieldOBJDraw_BlAct00 },
	{ GINGAM, &DATA_FieldOBJDraw_BlAct00 },
	{ GINGAW, &DATA_FieldOBJDraw_BlAct00 },
	{ LEADER1, &DATA_FieldOBJDraw_BlAct00 },
	{ LEADER2, &DATA_FieldOBJDraw_BlAct00 },
	{ LEADER3, &DATA_FieldOBJDraw_BlAct00 },
	{ LEADER4, &DATA_FieldOBJDraw_BlAct00 },
	{ LEADER5, &DATA_FieldOBJDraw_BlAct00 },
	{ LEADER6, &DATA_FieldOBJDraw_BlAct00 },
	{ LEADER7, &DATA_FieldOBJDraw_BlAct00 },
	{ LEADER8, &DATA_FieldOBJDraw_BlAct00 },
	{ BIGFOUR1, &DATA_FieldOBJDraw_BlAct00 },
	{ BIGFOUR2, &DATA_FieldOBJDraw_BlAct00 },
	{ BIGFOUR3, &DATA_FieldOBJDraw_BlAct00 },
	{ BIGFOUR4, &DATA_FieldOBJDraw_BlAct00 },
	{ CHAMPION, &DATA_FieldOBJDraw_BlAct00 },
	{ PAPA, &DATA_FieldOBJDraw_BlAct00 },
	{ MAMA, &DATA_FieldOBJDraw_BlAct00 },
	{ SEVEN1, &DATA_FieldOBJDraw_BlAct00 },
	{ SEVEN2, &DATA_FieldOBJDraw_BlAct00 },
	{ SEVEN3, &DATA_FieldOBJDraw_BlAct00 },
	{ SEVEN4, &DATA_FieldOBJDraw_BlAct00 },
	{ SEVEN5, &DATA_FieldOBJDraw_BlAct00 },
	{ SEVEN6, &DATA_FieldOBJDraw_BlAct00 },
	{ SEVEN7, &DATA_FieldOBJDraw_BlAct00 },
	{ RIVEL, &DATA_FieldOBJDraw_BlAct00 },
	{ BOAT, &DATA_FieldOBJDraw_BlAct00 },
	{ TRAIN, &DATA_FieldOBJDraw_BlAct00 },
	{ SPPOKE1, &DATA_FieldOBJDraw_ReiAiHai },
	{ SPPOKE2, &DATA_FieldOBJDraw_ReiAiHai },
	{ SPPOKE3, &DATA_FieldOBJDraw_ReiAiHai },
	{ SPPOKE4, &DATA_FieldOBJDraw_Legend },
	{ SPPOKE5, &DATA_FieldOBJDraw_Legend },
	{ SPPOKE6, &DATA_FieldOBJDraw_BlAct00AnmOneP },
	{ SPPOKE7, &DATA_FieldOBJDraw_BlAct00AnmOneP },
	{ SPPOKE8, &DATA_FieldOBJDraw_BlAct00 },
	{ SPPOKE9, &DATA_FieldOBJDraw_BlAct00AnmOneP },
	{ SPPOKE10, &DATA_FieldOBJDraw_BlAct00AnmOneP },
	{ SPPOKE11, &DATA_FieldOBJDraw_BlAct00AnmOneP },
	{ OBAKE, &DATA_FieldOBJDraw_BlAct00 },
	{ WOMAN6, &DATA_FieldOBJDraw_BlAct00 },
	{ OLDMAN2, &DATA_FieldOBJDraw_BlAct00 },
	{ OLDWOMAN2, &DATA_FieldOBJDraw_BlAct00 },
	{ OOKIDO, &DATA_FieldOBJDraw_BlAct00 },
	{ MIKAN, &DATA_FieldOBJDraw_BlAct00 },
	{ SUNGLASSES, &DATA_FieldOBJDraw_BlAct00 },
	{ TOWERBOSS, &DATA_FieldOBJDraw_BlAct00 },
	{ SNOWCOVER, &DATA_FieldOBJDraw_BlAct00 },
	{ TREECOVER, &DATA_FieldOBJDraw_BlAct00 },
	{ ROCKCOVER, &DATA_FieldOBJDraw_BlAct00 },
	{ BIRD, &DATA_FieldOBJDraw_BlActBird },
	{ BAG, &DATA_FieldOBJDraw_BlAct00AnmNon },
	{ MAID, &DATA_FieldOBJDraw_BlAct00 },
	{ SPHERO, &DATA_FieldOBJDraw_HeroSp },
	{ SPHEROINE, &DATA_FieldOBJDraw_HeroSp },
	{ WATERHERO, &DATA_FieldOBJDraw_HeroWater },
	{ WATERHEROINE, &DATA_FieldOBJDraw_HeroWater },
	{ SWIMHERO, &DATA_FieldOBJDraw_HeroSwim },
	{ SWIMHEROINE, &DATA_FieldOBJDraw_HeroSwim },
	{ VENTHOLE, &DATA_FieldOBJDraw_BlAct00AnmNon },
	{ BOOK, &DATA_FieldOBJDraw_Model00 },
	{ SPPOKE12, &DATA_FieldOBJDraw_BlAct00AnmNon },
	{ BALLOON, &DATA_FieldOBJDraw_BlAct00 },
	{ CONTESTHERO,	&DATA_FieldOBJDraw_Hero },
	{ CONTESTHEROINE,	&DATA_FieldOBJDraw_Hero },
	{ FISHINGHERO,	&DATA_FieldOBJDraw_HeroFishing },
	{ FISHINGHEROINE,	&DATA_FieldOBJDraw_HeroFishing },
	{ MOSS, &DATA_FieldOBJDraw_BlAct00 },
	{ FREEZES, &DATA_FieldOBJDraw_BlAct00 },
	{ POLE, &DATA_FieldOBJDraw_BlAct00 },
	{ DELIVERY, &DATA_FieldOBJDraw_BlAct00 },
	{ DSBOY, &DATA_FieldOBJDraw_BlAct00 },
	{ KOIKING, &DATA_FieldOBJDraw_BlActKoiking },
	{ POKEHERO, &DATA_FieldOBJDraw_HeroPoketch },
	{ POKEHEROINE, &DATA_FieldOBJDraw_HeroPoketch },
	{ SAVEHERO, &DATA_FieldOBJDraw_HeroSp },
	{ SAVEHEROINE, &DATA_FieldOBJDraw_HeroSp },
	{ BANZAIHERO, &DATA_FieldOBJDraw_HeroBanzai },
	{ BANZAIHEROINE, &DATA_FieldOBJDraw_HeroBanzai },
	{DOOR, &DATA_FieldOBJDraw_BlAct00AnmNon },
	{MONUMENT, &DATA_FieldOBJDraw_BlAct00AnmNon },
	{ PATIRITUSU, &DATA_FieldOBJDraw_BlAct00 },
	{ KINOCOCO, &DATA_FieldOBJDraw_BlAct00 },
	{ MIMITUTO, &DATA_FieldOBJDraw_BlAct00 },
	{ KOLUCKY, &DATA_FieldOBJDraw_BlAct00 },
	{ WANRIKY, &DATA_FieldOBJDraw_BlAct00 },
	{ DOOR2, &DATA_FieldOBJDraw_Model00 },
	{ GHOSTHERO, &DATA_FieldOBJDraw_Hero },
	{ GHOSTHEROINE,	&DATA_FieldOBJDraw_Hero },
	{ RTHERO, &DATA_FieldOBJDraw_Hero },
	{ ICPO, &DATA_FieldOBJDraw_BlAct00 },
	{ GKANBU4, &DATA_FieldOBJDraw_BlAct00 },
	{ BRAINS1, &DATA_FieldOBJDraw_BlAct00 },
	{ BRAINS2, &DATA_FieldOBJDraw_BlAct00 },
	{ BRAINS3, &DATA_FieldOBJDraw_BlAct00 },
	{ BRAINS4, &DATA_FieldOBJDraw_BlAct00 },
	{ PRINCESS, &DATA_FieldOBJDraw_BlAct00 },
	{ NAETLE, &DATA_FieldOBJDraw_BlAct00 },
	{ HAYASHIGAME, &DATA_FieldOBJDraw_BlAct00 },
	{ DODAITOSE, &DATA_FieldOBJDraw_BlAct00 },
	{ HIKOZARU, &DATA_FieldOBJDraw_BlAct00 },
	{ MOUKAZARU, &DATA_FieldOBJDraw_BlAct00 },
	{ GOUKAZARU, &DATA_FieldOBJDraw_BlAct00 },
	{ POCHAMA, &DATA_FieldOBJDraw_BlAct00 },
	{ POTTAISHI, &DATA_FieldOBJDraw_BlAct00 },
	{ EMPERTE, &DATA_FieldOBJDraw_BlAct00 },
	{ GUREGGRU, &DATA_FieldOBJDraw_BlAct00 },
	{ GIRATINAORIGIN, &DATA_FieldOBJDraw_BlAct00AnmOneP },
	{ BFSM, &DATA_FieldOBJDraw_BlAct00 },
	{ BFSW1, &DATA_FieldOBJDraw_BlAct00 },
	{ BFSW2, &DATA_FieldOBJDraw_BlAct00 },
	{ WIFISM, &DATA_FieldOBJDraw_BlAct00 },
	{ WIFISW, &DATA_FieldOBJDraw_BlAct00 },
	{ ROTOM, &DATA_FieldOBJDraw_BlAct00 },
	{ ROTOMF, &DATA_FieldOBJDraw_BlAct00 },
	{ ROTOMS, &DATA_FieldOBJDraw_BlAct00 },
	{ ROTOMI, &DATA_FieldOBJDraw_BlAct00 },
	{ ROTOMW, &DATA_FieldOBJDraw_BlAct00 },
	{ ROTOMG, &DATA_FieldOBJDraw_BlAct00 },
	{ DIRECTOR, &DATA_FieldOBJDraw_BlAct00 },
	{ FIREBOX, &DATA_FieldOBJDraw_BlAct00 },
	{ SKYBOX, &DATA_FieldOBJDraw_BlAct00 },
	{ ICEBOX, &DATA_FieldOBJDraw_BlAct00 },
	{ WATERBOX, &DATA_FieldOBJDraw_BlAct00 },
	{ GRASSBOX, &DATA_FieldOBJDraw_BlAct00 },
	{ GSET1, &DATA_FieldOBJDraw_BlAct00AnmNon },
	{ GSET2, &DATA_FieldOBJDraw_BlAct00AnmNon },
	{MONUMENT2A, &DATA_FieldOBJDraw_BlAct00AnmNon },
	{MONUMENT2B, &DATA_FieldOBJDraw_BlAct00AnmNon },
	{DPHERO,&DATA_FieldOBJDraw_Hero},
	{DPHEROINE,&DATA_FieldOBJDraw_Hero},
	{DPCONTESTHERO,&DATA_FieldOBJDraw_Hero},
	{DPCONTESTHEROINE,&DATA_FieldOBJDraw_Hero},
	{BSHERO,&DATA_FieldOBJDraw_HeroSp},
	{BSHEROINE,&DATA_FieldOBJDraw_HeroSp},
	{SWIMHERORT, &DATA_FieldOBJDraw_HeroSwim },
	{SWIMHEROINERT, &DATA_FieldOBJDraw_HeroSwim },
	{ SPHERORT, &DATA_FieldOBJDraw_HeroSp },
	{ SPHEROINERT, &DATA_FieldOBJDraw_HeroSp },
	{ ROTOMWALL, &DATA_FieldOBJDraw_Model00 },
	{ RTHEROINE, &DATA_FieldOBJDraw_Hero },
	{ BLDAGUNOMU, &DATA_FieldOBJDraw_BlActBldAgunomu },
	{ TW7YUKUSHI, &DATA_FieldOBJDraw_BlActTw7SpPoke },
	{ TW7EMULIT, &DATA_FieldOBJDraw_BlActTw7SpPoke },
	{ TW7AGUNOMU, &DATA_FieldOBJDraw_BlActTw7SpPoke },
	{ SAVEHERORT, &DATA_FieldOBJDraw_HeroSp },
	{ SAVEHEROINERT, &DATA_FieldOBJDraw_HeroSp },
	{ POKEHERORT, &DATA_FieldOBJDraw_HeroPoketch },
	{ POKEHEROINERT, &DATA_FieldOBJDraw_HeroPoketch },
	{ GSTSAVEHERO, &DATA_FieldOBJDraw_HeroSp },
	{ GSTSAVEHEROINE, &DATA_FieldOBJDraw_HeroSp },
	{ GSTPOKEHERO, &DATA_FieldOBJDraw_HeroPoketch },
	{ GSTPOKEHEROINE, &DATA_FieldOBJDraw_HeroPoketch },
	
	{ OBJCODEMAX, NULL },	//終端
};
#endif

//==============================================================================
//	フィールド動作モデル レンダOBJまとめ
//==============================================================================
#if 0
//--------------------------------------------------------------
///	レンダリングOBJデータ
//--------------------------------------------------------------
const FLDMMDL_RESMNARC DATA_FLDMMDL_ResmNArcTbl_RenderOBJ[] =
{
	{ BOARD_A, NARC_mmodel_board_a_nsbmd },
	{ BOARD_B, NARC_mmodel_board_b_nsbmd },
	{ BOARD_C, NARC_mmodel_board_c_nsbmd },
	{ BOARD_D, NARC_mmodel_board_d_nsbmd },
	{ BOARD_E, NARC_mmodel_board_e_nsbmd },
	{ BOARD_F, NARC_mmodel_board_f_nsbmd },
	{ SNOWBALL, NARC_mmodel_gym06_o1_nsbmd },
	
	{ OBJCODEMAX, 0 },	//終端
};

#define FLDMMDL_RENDER_MAX (NELEMS(DATA_FLDMMDL_ResmNArcTbl_RenderOBJ))

//--------------------------------------------------------------
///	レンダリングOBJ総数
//--------------------------------------------------------------
const int DATA_FIELDOBJ_RenderOBJMax = FLDMMDL_RENDER_MAX - 1;
#endif

//==============================================================================
//	フィールド動作モデル テクスチャまとめ
//==============================================================================
//--------------------------------------------------------------
///	テクスチャID,アーカイブID
///	並びは不順
//--------------------------------------------------------------
const FLDMMDL_RESMNARC DATA_FLDMMDL_ResmNArcTbl_Tex[] =
{
	{ HERO, NARC_mmodel_hero_nsbtx },
	{ BABYBOY1, NARC_mmodel_babyboy1_nsbtx },
	{ BABYGIRL1, NARC_mmodel_babygirl1_nsbtx },
	{ BOY1, NARC_mmodel_boy1_nsbtx },
	{ BOY2, NARC_mmodel_boy2_nsbtx },
	{ BOY3, NARC_mmodel_boy3_nsbtx },
	{ GIRL1, NARC_mmodel_girl1_nsbtx },
	{ GIRL2, NARC_mmodel_girl2_nsbtx },
	{ GIRL3, NARC_mmodel_girl3_nsbtx },
	{ MAN1, NARC_mmodel_man1_nsbtx },
	{ MAN2, NARC_mmodel_man2_nsbtx },
	{ MAN3, NARC_mmodel_man3_nsbtx },
	{ WOMAN1, NARC_mmodel_woman1_nsbtx },
	{ WOMAN2, NARC_mmodel_woman2_nsbtx },
	{ WOMAN3, NARC_mmodel_woman3_nsbtx },
	{ MIDDLEMAN1, NARC_mmodel_middleman1_nsbtx },
	{ MIDDLEWOMAN1, NARC_mmodel_middlewoman1_nsbtx },
	{ OLDMAN1, NARC_mmodel_oldman1_nsbtx },
	{ OLDWOMAN1, NARC_mmodel_oldwoman1_nsbtx },
	{ BIGMAN, NARC_mmodel_bigman_nsbtx },
	{ MOUNT, NARC_mmodel_mount_nsbtx },
	{ CYCLEHERO, NARC_mmodel_cyclehero_nsbtx },
	{ REPORTER, NARC_mmodel_reporter_nsbtx },
	{ CAMERAMAN,NARC_mmodel_cameraman_nsbtx  },
	{ SHOPM1, NARC_mmodel_shopm1_nsbtx },
	{ SHOPW1, NARC_mmodel_shopw1_nsbtx },
	{ PCWOMAN1, NARC_mmodel_pcwoman1_nsbtx },
	{ PCWOMAN2, NARC_mmodel_pcwoman2_nsbtx },
	{ PCWOMAN3, NARC_mmodel_pcwoman3_nsbtx },
	{ ASSISTANTM, NARC_mmodel_assistantm_nsbtx },
	{ ASSISTANTW, NARC_mmodel_assistantw_nsbtx },
	{ BADMAN, NARC_mmodel_badman_nsbtx },
	{ SKIERM,NARC_mmodel_skierm_nsbtx  },
	{ SKIERW, NARC_mmodel_skierw_nsbtx },
	{ POLICEMAN, NARC_mmodel_policeman_nsbtx },
	{ IDOL, NARC_mmodel_idol_nsbtx },
	{ GENTLEMAN, NARC_mmodel_gentleman_nsbtx },
	{ LADY, NARC_mmodel_lady_nsbtx },
	{ CYCLEM, NARC_mmodel_cyclem_nsbtx },
	{ CYCLEW, NARC_mmodel_cyclew_nsbtx },
	{ WORKMAN, NARC_mmodel_workman_nsbtx },
	{ FARMER, NARC_mmodel_farmer_nsbtx },
	{ COWGIRL, NARC_mmodel_cowgirl_nsbtx },
	{ CLOWN, NARC_mmodel_clown_nsbtx },
	{ ARTIST, NARC_mmodel_artist_nsbtx },
	{ SPORTSMAN, NARC_mmodel_sportsman_nsbtx },
	{ SWIMMERM,NARC_mmodel_swimmerm_nsbtx  },
	{ SWIMMERW, NARC_mmodel_swimmerw_nsbtx },
	{ BEACHGIRL, NARC_mmodel_beachgirl_nsbtx },
	{ BEACHBOY, NARC_mmodel_beachboy_nsbtx },
	{ EXPLORE, NARC_mmodel_explore_nsbtx },
	{ FIGHTER, NARC_mmodel_fighter_nsbtx },
	{ CAMPBOY, NARC_mmodel_campboy_nsbtx },
	{ PICNICGIRL, NARC_mmodel_picnicgirl_nsbtx },
	{ FISHING, NARC_mmodel_fishing_nsbtx },
	{ AMBRELLA, NARC_mmodel_ambrella_nsbtx },
	{ SEAMAN,NARC_mmodel_seaman_nsbtx  },
	{ BRINGM, NARC_mmodel_bringm_nsbtx },
	{ BRINGW, NARC_mmodel_bringw_nsbtx },
	{ WAITER, NARC_mmodel_waiter_nsbtx },
	{ WAITRESS, NARC_mmodel_waitress_nsbtx },
	{ VETERAN, NARC_mmodel_veteran_nsbtx },
	{ GORGGEOUSM, NARC_mmodel_gorggeousm_nsbtx },
	{ GORGGEOUSW, NARC_mmodel_gorggeousw_nsbtx },
	{ BOY4,NARC_mmodel_boy4_nsbtx  },
	{ GIRL4, NARC_mmodel_girl4_nsbtx },
	{ MAN4, NARC_mmodel_man4_nsbtx },
	{ WOMAN4,NARC_mmodel_woman4_nsbtx  },
	{ MAN5, NARC_mmodel_man5_nsbtx },
	{ WOMAN5, NARC_mmodel_woman5_nsbtx },
	{ MYSTERY, NARC_mmodel_mystery_nsbtx },
	{ PIKACHU, NARC_mmodel_pikachu_nsbtx },
	{ PIPPI,NARC_mmodel_pippi_nsbtx  },
	{ PURIN, NARC_mmodel_purin_nsbtx },
	{ KODUCK, NARC_mmodel_koduck_nsbtx },
	{ PRASLE, NARC_mmodel_prasle_nsbtx },
	{ MINUN, NARC_mmodel_minun_nsbtx },
	{ MARIL,NARC_mmodel_maril_nsbtx  },
	{ ACHAMO, NARC_mmodel_achamo_nsbtx },
	{ ENECO,NARC_mmodel_eneco_nsbtx  },
	{ GONBE, NARC_mmodel_gonbe_nsbtx },
	{ BABY, NARC_mmodel_baby_nsbtx },
	{ MIDDLEMAN2, NARC_mmodel_middleman2_nsbtx },
	{ MIDDLEWOMAN2,NARC_mmodel_middlewoman2_nsbtx  },
	{ ROCK, NARC_mmodel_rock_nsbtx },
	{ BREAKROCK, NARC_mmodel_breakrock_nsbtx },
	{ TREE, NARC_mmodel_tree_nsbtx },
	{ MONATERBALL, NARC_mmodel_monaterball_nsbtx },
	{ SHADOW, NARC_mmodel_shadow_nsbtx },
	{ BOUGH, NARC_mmodel_bough_nsbtx },
	{ FALLTREE, NARC_mmodel_falltree_nsbtx },
	{ HEROINE, NARC_mmodel_heroine_nsbtx },
	{ CYCLEHEROINE, NARC_mmodel_cycleheroine_nsbtx },
	{ DOCTOR, NARC_mmodel_doctor_nsbtx },
	{ SEED00, NARC_mmodel_seed00_a_nsbtx },
	{ SEED01A, NARC_mmodel_seed01_a_nsbtx },
	{ SEED01B, NARC_mmodel_seed01_b_nsbtx },
	{ SEED01C, NARC_mmodel_seed01_c_nsbtx },
	{ GINGABOSS,NARC_mmodel_gingaboss_nsbtx }, 
	{ GKANBU1,NARC_mmodel_gkanbu1_nsbtx }, 
	{ GKANBU2,NARC_mmodel_gkanbu2_nsbtx }, 
	{ GKANBU3,NARC_mmodel_gkanbu3_nsbtx }, 
	{ GINGAM,NARC_mmodel_gingam_nsbtx }, 
	{ GINGAW,NARC_mmodel_gingaw_nsbtx }, 
	{ LEADER1,NARC_mmodel_leader1_nsbtx }, 
	{ LEADER2,NARC_mmodel_leader2_nsbtx }, 
	{ LEADER3,NARC_mmodel_leader3_nsbtx }, 
	{ LEADER4,NARC_mmodel_leader4_nsbtx }, 
	{ LEADER5,NARC_mmodel_leader5_nsbtx }, 
	{ LEADER6,NARC_mmodel_leader6_nsbtx }, 
	{ LEADER7,NARC_mmodel_leader7_nsbtx }, 
	{ LEADER8,NARC_mmodel_leader8_nsbtx }, 
	{ BIGFOUR1,NARC_mmodel_bigfour1_nsbtx }, 
	{ BIGFOUR2,NARC_mmodel_bigfour2_nsbtx }, 
	{ BIGFOUR3,NARC_mmodel_bigfour3_nsbtx }, 
	{ BIGFOUR4,NARC_mmodel_bigfour4_nsbtx }, 
	{ CHAMPION,NARC_mmodel_champion_nsbtx }, 
	{ PAPA,NARC_mmodel_papa_nsbtx }, 
	{ MAMA,NARC_mmodel_mama_nsbtx }, 
	{ SEVEN1,NARC_mmodel_seven1_nsbtx }, 
	{ SEVEN2,NARC_mmodel_seven2_nsbtx }, 
	{ SEVEN3,NARC_mmodel_seven3_nsbtx }, 
	{ SEVEN4,NARC_mmodel_seven4_nsbtx }, 
	{ SEVEN5,NARC_mmodel_seven5_nsbtx }, 
	{ SEVEN6,NARC_mmodel_seven6_nsbtx }, 
	{ SEVEN7,NARC_mmodel_seven7_nsbtx }, 
	{ RIVEL,NARC_mmodel_rivel_nsbtx }, 
	{ BOAT,NARC_mmodel_boat_nsbtx }, 
	{ TRAIN,NARC_mmodel_train_nsbtx }, 
	{ SPPOKE1,NARC_mmodel_sppoke1_nsbtx }, 
	{ SPPOKE2,NARC_mmodel_sppoke2_nsbtx }, 
	{ SPPOKE3,NARC_mmodel_sppoke3_nsbtx }, 
	{ SPPOKE4,NARC_mmodel_sppoke4_nsbtx }, 
	{ SPPOKE5,NARC_mmodel_sppoke5_nsbtx }, 
	{ SPPOKE6,NARC_mmodel_sppoke6_nsbtx }, 
	{ SPPOKE7,NARC_mmodel_sppoke7_nsbtx }, 
	{ SPPOKE8,NARC_mmodel_sppoke8_nsbtx }, 
	{ SPPOKE9,NARC_mmodel_sppoke9_nsbtx }, 
	{ SPPOKE10,NARC_mmodel_sppoke10_nsbtx }, 
	{ SPPOKE11,NARC_mmodel_sppoke11_nsbtx }, 
	{ OBAKE,NARC_mmodel_obake_nsbtx }, 
	{ WOMAN6,NARC_mmodel_woman6_nsbtx }, 
	{ OLDMAN2,NARC_mmodel_oldman2_nsbtx }, 
	{ OLDWOMAN2,NARC_mmodel_oldwoman2_nsbtx }, 
	{ OOKIDO,NARC_mmodel_ookido_nsbtx }, 
	{ MIKAN,NARC_mmodel_mikan_nsbtx }, 
	{ SUNGLASSES,NARC_mmodel_sunglasses_nsbtx }, 
	{ TOWERBOSS,NARC_mmodel_towerboss_nsbtx }, 
	{ SNOWCOVER,NARC_mmodel_snowcover_nsbtx }, 
	{ TREECOVER,NARC_mmodel_treecover_nsbtx }, 
	{ ROCKCOVER,NARC_mmodel_rockcover_nsbtx }, 
	{ BIRD,NARC_mmodel_bird_nsbtx }, 
	{ BAG,NARC_mmodel_bag_nsbtx }, 
	{ MAID,NARC_mmodel_maid_nsbtx }, 
	{ SPHERO, NARC_mmodel_sphero_nsbtx },
	{ SPHEROINE, NARC_mmodel_spheroine_nsbtx },
	{ WATERHERO, NARC_mmodel_waterhero_nsbtx },
	{ WATERHEROINE, NARC_mmodel_waterheroine_nsbtx },
	{ SWIMHERO, NARC_mmodel_swimhero_nsbtx },
	{ SWIMHEROINE, NARC_mmodel_swimheroine_nsbtx },
	{ VENTHOLE, NARC_mmodel_venthole_nsbtx },
	{ SPPOKE12, NARC_mmodel_sppoke12_nsbtx },
	{ BALLOON, NARC_mmodel_balloon_nsbtx },
	{ CONTESTHERO, NARC_mmodel_contesthero_nsbtx },
	{ CONTESTHEROINE, NARC_mmodel_cont_heroine_nsbtx },
	{ FISHINGHERO, NARC_mmodel_fishinghero_nsbtx },
	{ FISHINGHEROINE, NARC_mmodel_fish_heroine_nsbtx },
	{ MOSS, NARC_mmodel_moss_nsbtx },
	{ FREEZES, NARC_mmodel_freezes_nsbtx },
	{ POLE, NARC_mmodel_pole_nsbtx },
	{ SEED02A, NARC_mmodel_seed02_a_nsbtx },
	{ SEED02B, NARC_mmodel_seed02_b_nsbtx },
	{ SEED02C, NARC_mmodel_seed02_c_nsbtx },
	{ SEED03A, NARC_mmodel_seed03_a_nsbtx },
	{ SEED03B, NARC_mmodel_seed03_b_nsbtx },
	{ SEED03C, NARC_mmodel_seed03_c_nsbtx },
	{ SEED04A, NARC_mmodel_seed04_a_nsbtx },
	{ SEED04B, NARC_mmodel_seed04_b_nsbtx },
	{ SEED04C, NARC_mmodel_seed04_c_nsbtx },
	{ SEED05A, NARC_mmodel_seed05_a_nsbtx },
	{ SEED05B, NARC_mmodel_seed05_b_nsbtx },
	{ SEED05C, NARC_mmodel_seed05_c_nsbtx },
	{ SEED06A, NARC_mmodel_seed06_a_nsbtx },
	{ SEED06B, NARC_mmodel_seed06_b_nsbtx },
	{ SEED06C, NARC_mmodel_seed06_c_nsbtx },
	{ SEED07A, NARC_mmodel_seed07_a_nsbtx },
	{ SEED07B, NARC_mmodel_seed07_b_nsbtx },
	{ SEED07C, NARC_mmodel_seed07_c_nsbtx },
	{ SEED08A, NARC_mmodel_seed08_a_nsbtx },
	{ SEED08B, NARC_mmodel_seed08_b_nsbtx },
	{ SEED08C, NARC_mmodel_seed08_c_nsbtx },
	{ SEED09A, NARC_mmodel_seed09_a_nsbtx },
	{ SEED09B, NARC_mmodel_seed09_b_nsbtx },
	{ SEED09C, NARC_mmodel_seed09_c_nsbtx },
	{ SEED10A, NARC_mmodel_seed10_a_nsbtx },
	{ SEED10B, NARC_mmodel_seed10_b_nsbtx },
	{ SEED10C, NARC_mmodel_seed10_c_nsbtx },
	{ SEED11A, NARC_mmodel_seed11_a_nsbtx },
	{ SEED11B, NARC_mmodel_seed11_b_nsbtx },
	{ SEED11C, NARC_mmodel_seed11_c_nsbtx },
	{ SEED12A, NARC_mmodel_seed12_a_nsbtx },
	{ SEED12B, NARC_mmodel_seed12_b_nsbtx },
	{ SEED12C, NARC_mmodel_seed12_c_nsbtx },
	{ SEED13A, NARC_mmodel_seed13_a_nsbtx },
	{ SEED13B, NARC_mmodel_seed13_b_nsbtx },
	{ SEED13C, NARC_mmodel_seed13_c_nsbtx },
	{ SEED14A, NARC_mmodel_seed14_a_nsbtx },
	{ SEED14B, NARC_mmodel_seed14_b_nsbtx },
	{ SEED14C, NARC_mmodel_seed14_c_nsbtx },
	{ SEED15A, NARC_mmodel_seed15_a_nsbtx },
	{ SEED15B, NARC_mmodel_seed15_b_nsbtx },
	{ SEED15C, NARC_mmodel_seed15_c_nsbtx },
	{ SEED16A, NARC_mmodel_seed16_a_nsbtx },
	{ SEED16B, NARC_mmodel_seed16_b_nsbtx },
	{ SEED16C, NARC_mmodel_seed16_c_nsbtx },
	{ SEED17A, NARC_mmodel_seed17_a_nsbtx },
	{ SEED17B, NARC_mmodel_seed17_b_nsbtx },
	{ SEED17C, NARC_mmodel_seed17_c_nsbtx },
	{ SEED18A, NARC_mmodel_seed18_a_nsbtx },
	{ SEED18B, NARC_mmodel_seed18_b_nsbtx },
	{ SEED18C, NARC_mmodel_seed18_c_nsbtx },
	{ SEED19A, NARC_mmodel_seed19_a_nsbtx },
	{ SEED19B, NARC_mmodel_seed19_b_nsbtx },
	{ SEED19C, NARC_mmodel_seed19_c_nsbtx },
	{ SEED20A, NARC_mmodel_seed20_a_nsbtx },
	{ SEED20B, NARC_mmodel_seed20_b_nsbtx },
	{ SEED20C, NARC_mmodel_seed20_c_nsbtx },
	{ SEED21A, NARC_mmodel_seed21_a_nsbtx },
	{ SEED21B, NARC_mmodel_seed21_b_nsbtx },
	{ SEED21C, NARC_mmodel_seed21_c_nsbtx },
	{ SEED22A, NARC_mmodel_seed22_a_nsbtx },
	{ SEED22B, NARC_mmodel_seed22_b_nsbtx },
	{ SEED22C, NARC_mmodel_seed22_c_nsbtx },
	{ SEED23A, NARC_mmodel_seed23_a_nsbtx },
	{ SEED23B, NARC_mmodel_seed23_b_nsbtx },
	{ SEED23C, NARC_mmodel_seed23_c_nsbtx },
	{ SEED24A, NARC_mmodel_seed24_a_nsbtx },
	{ SEED24B, NARC_mmodel_seed24_b_nsbtx },
	{ SEED24C, NARC_mmodel_seed24_c_nsbtx },
	{ SEED25A, NARC_mmodel_seed25_a_nsbtx },
	{ SEED25B, NARC_mmodel_seed25_b_nsbtx },
	{ SEED25C, NARC_mmodel_seed25_c_nsbtx },
	{ SEED26A, NARC_mmodel_seed26_a_nsbtx },
	{ SEED26B, NARC_mmodel_seed26_b_nsbtx },
	{ SEED26C, NARC_mmodel_seed26_c_nsbtx },
	{ SEED27A, NARC_mmodel_seed27_a_nsbtx },
	{ SEED27B, NARC_mmodel_seed27_b_nsbtx },
	{ SEED27C, NARC_mmodel_seed27_c_nsbtx },
	{ SEED28A, NARC_mmodel_seed28_a_nsbtx },
	{ SEED28B, NARC_mmodel_seed28_b_nsbtx },
	{ SEED28C, NARC_mmodel_seed28_c_nsbtx },
	{ SEED29A, NARC_mmodel_seed29_a_nsbtx },
	{ SEED29B, NARC_mmodel_seed29_b_nsbtx },
	{ SEED29C, NARC_mmodel_seed29_c_nsbtx },
	{ SEED30A, NARC_mmodel_seed30_a_nsbtx },
	{ SEED30B, NARC_mmodel_seed30_b_nsbtx },
	{ SEED30C, NARC_mmodel_seed30_c_nsbtx },
	{ SEED31A, NARC_mmodel_seed31_a_nsbtx },
	{ SEED31B, NARC_mmodel_seed31_b_nsbtx },
	{ SEED31C, NARC_mmodel_seed31_c_nsbtx },
	{ SEED32A, NARC_mmodel_seed32_a_nsbtx },
	{ SEED32B, NARC_mmodel_seed32_b_nsbtx },
	{ SEED32C, NARC_mmodel_seed32_c_nsbtx },
	{ SEED33A, NARC_mmodel_seed33_a_nsbtx },
	{ SEED33B, NARC_mmodel_seed33_b_nsbtx },
	{ SEED33C, NARC_mmodel_seed33_c_nsbtx },
	{ SEED34A, NARC_mmodel_seed34_a_nsbtx },
	{ SEED34B, NARC_mmodel_seed34_b_nsbtx },
	{ SEED34C, NARC_mmodel_seed34_c_nsbtx },
	{ SEED35A, NARC_mmodel_seed35_a_nsbtx },
	{ SEED35B, NARC_mmodel_seed35_b_nsbtx },
	{ SEED35C, NARC_mmodel_seed35_c_nsbtx },
	{ SEED36A, NARC_mmodel_seed36_a_nsbtx },
	{ SEED36B, NARC_mmodel_seed36_b_nsbtx },
	{ SEED36C, NARC_mmodel_seed36_c_nsbtx },
	{ SEED37A, NARC_mmodel_seed37_a_nsbtx },
	{ SEED37B, NARC_mmodel_seed37_b_nsbtx },
	{ SEED37C, NARC_mmodel_seed37_c_nsbtx },
	{ SEED38A, NARC_mmodel_seed38_a_nsbtx },
	{ SEED38B, NARC_mmodel_seed38_b_nsbtx },
	{ SEED38C, NARC_mmodel_seed38_c_nsbtx },
	{ SEED39A, NARC_mmodel_seed39_a_nsbtx },
	{ SEED39B, NARC_mmodel_seed39_b_nsbtx },
	{ SEED39C, NARC_mmodel_seed39_c_nsbtx },
	{ SEED40A, NARC_mmodel_seed40_a_nsbtx },
	{ SEED40B, NARC_mmodel_seed40_b_nsbtx },
	{ SEED40C, NARC_mmodel_seed40_c_nsbtx },
	{ SEED41A, NARC_mmodel_seed41_a_nsbtx },
	{ SEED41B, NARC_mmodel_seed41_b_nsbtx },
	{ SEED41C, NARC_mmodel_seed41_c_nsbtx },
	{ SEED42A, NARC_mmodel_seed42_a_nsbtx },
	{ SEED42B, NARC_mmodel_seed42_b_nsbtx },
	{ SEED42C, NARC_mmodel_seed42_c_nsbtx },
	{ SEED43A, NARC_mmodel_seed43_a_nsbtx },
	{ SEED43B, NARC_mmodel_seed43_b_nsbtx },
	{ SEED43C, NARC_mmodel_seed43_c_nsbtx },
	{ SEED44A, NARC_mmodel_seed44_a_nsbtx },
	{ SEED44B, NARC_mmodel_seed44_b_nsbtx },
	{ SEED44C, NARC_mmodel_seed44_c_nsbtx },
	{ SEED45A, NARC_mmodel_seed45_a_nsbtx },
	{ SEED45B, NARC_mmodel_seed45_b_nsbtx },
	{ SEED45C, NARC_mmodel_seed45_c_nsbtx },
	{ SEED46A, NARC_mmodel_seed46_a_nsbtx },
	{ SEED46B, NARC_mmodel_seed46_b_nsbtx },
	{ SEED46C, NARC_mmodel_seed46_c_nsbtx },
	{ SEED47A, NARC_mmodel_seed47_a_nsbtx },
	{ SEED47B, NARC_mmodel_seed47_b_nsbtx },
	{ SEED47C, NARC_mmodel_seed47_c_nsbtx },
	{ SEED48A, NARC_mmodel_seed48_a_nsbtx },
	{ SEED48B, NARC_mmodel_seed48_b_nsbtx },
	{ SEED48C, NARC_mmodel_seed48_c_nsbtx },
	{ SEED49A, NARC_mmodel_seed49_a_nsbtx },
	{ SEED49B, NARC_mmodel_seed49_b_nsbtx },
	{ SEED49C, NARC_mmodel_seed49_c_nsbtx },
	{ SEED50A, NARC_mmodel_seed50_a_nsbtx },
	{ SEED50B, NARC_mmodel_seed50_b_nsbtx },
	{ SEED50C, NARC_mmodel_seed50_c_nsbtx },
	{ SEED51A, NARC_mmodel_seed51_a_nsbtx },
	{ SEED51B, NARC_mmodel_seed51_b_nsbtx },
	{ SEED51C, NARC_mmodel_seed51_c_nsbtx },
	{ SEED52A, NARC_mmodel_seed52_a_nsbtx },
	{ SEED52B, NARC_mmodel_seed52_b_nsbtx },
	{ SEED52C, NARC_mmodel_seed52_c_nsbtx },
	{ SEED53A, NARC_mmodel_seed53_a_nsbtx },
	{ SEED53B, NARC_mmodel_seed53_b_nsbtx },
	{ SEED53C, NARC_mmodel_seed53_c_nsbtx },
	{ SEED54A, NARC_mmodel_seed54_a_nsbtx },
	{ SEED54B, NARC_mmodel_seed54_b_nsbtx },
	{ SEED54C, NARC_mmodel_seed54_c_nsbtx },
	{ SEED55A, NARC_mmodel_seed55_a_nsbtx },
	{ SEED55B, NARC_mmodel_seed55_b_nsbtx },
	{ SEED55C, NARC_mmodel_seed55_c_nsbtx },
	{ SEED56A, NARC_mmodel_seed56_a_nsbtx },
	{ SEED56B, NARC_mmodel_seed56_b_nsbtx },
	{ SEED56C, NARC_mmodel_seed56_c_nsbtx },
	{ SEED57A, NARC_mmodel_seed57_a_nsbtx },
	{ SEED57B, NARC_mmodel_seed57_b_nsbtx },
	{ SEED57C, NARC_mmodel_seed57_c_nsbtx },
	{ SEED58A, NARC_mmodel_seed58_a_nsbtx },
	{ SEED58B, NARC_mmodel_seed58_b_nsbtx },
	{ SEED58C, NARC_mmodel_seed58_c_nsbtx },
	{ SEED59A, NARC_mmodel_seed59_a_nsbtx },
	{ SEED59B, NARC_mmodel_seed59_b_nsbtx },
	{ SEED59C, NARC_mmodel_seed59_c_nsbtx },
	{ SEED60A, NARC_mmodel_seed60_a_nsbtx },
	{ SEED60B, NARC_mmodel_seed60_b_nsbtx },
	{ SEED60C, NARC_mmodel_seed60_c_nsbtx },
	{ SEED61A, NARC_mmodel_seed61_a_nsbtx },
	{ SEED61B, NARC_mmodel_seed61_b_nsbtx },
	{ SEED61C, NARC_mmodel_seed61_c_nsbtx },
	{ SEED62A, NARC_mmodel_seed62_a_nsbtx },
	{ SEED62B, NARC_mmodel_seed62_b_nsbtx },
	{ SEED62C, NARC_mmodel_seed62_c_nsbtx },
	{ SEED63A, NARC_mmodel_seed63_a_nsbtx },
	{ SEED63B, NARC_mmodel_seed63_b_nsbtx },
	{ SEED63C, NARC_mmodel_seed63_c_nsbtx },
	{ SEED64A, NARC_mmodel_seed64_a_nsbtx },
	{ SEED64B, NARC_mmodel_seed64_b_nsbtx },
	{ SEED64C, NARC_mmodel_seed64_c_nsbtx },
	{ DELIVERY, NARC_mmodel_delivery_nsbtx },
	{ DSBOY, NARC_mmodel_dsboy_nsbtx },
	{ KOIKING, NARC_mmodel_koiking_nsbtx },
	{ POKEHERO, NARC_mmodel_pokehero_nsbtx },
	{ POKEHEROINE, NARC_mmodel_pokeheroine_nsbtx },
	{ SAVEHERO, NARC_mmodel_savehero_nsbtx },
	{ SAVEHEROINE, NARC_mmodel_saveheroine_nsbtx },
	{ BANZAIHERO, NARC_mmodel_banzaihero_nsbtx },
	{ BANZAIHEROINE, NARC_mmodel_banzaiheroine_nsbtx },
	{ DOOR, NARC_mmodel_door_nsbtx },
	{ MONUMENT, NARC_mmodel_monument_nsbtx },
	{ PATIRITUSU, NARC_mmodel_patiritusu_nsbtx },
	{ KINOCOCO, NARC_mmodel_kinococo_nsbtx },
	{ MIMITUTO, NARC_mmodel_mimituto_nsbtx },
	{ KOLUCKY, NARC_mmodel_kolucky_nsbtx },
	{ WANRIKY, NARC_mmodel_wanriky_nsbtx },
	{ GHOSTHERO, NARC_mmodel_hero_nsbtx },
	{ GHOSTHEROINE, NARC_mmodel_heroine_nsbtx },
	{ RTHERO, NARC_mmodel_hero_nsbtx },
	{ ICPO,NARC_mmodel_icpo_nsbtx },
	{ GKANBU4,NARC_mmodel_gkanbu4_nsbtx },
	{ BRAINS1,NARC_mmodel_brains1_nsbtx },
	{ BRAINS2,NARC_mmodel_brains2_nsbtx },
	{ BRAINS3,NARC_mmodel_brains3_nsbtx },
	{ BRAINS4,NARC_mmodel_brains4_nsbtx },
	{ PRINCESS,NARC_mmodel_princess_nsbtx },
	{ NAETLE,NARC_mmodel_naetle_nsbtx },
	{ HAYASHIGAME,NARC_mmodel_hayashigame_nsbtx	 },
	{ DODAITOSE,NARC_mmodel_dodaitose_nsbtx	 },
	{ HIKOZARU,NARC_mmodel_hikozaru_nsbtx },
	{ MOUKAZARU,NARC_mmodel_moukazaru_nsbtx	 },
	{ GOUKAZARU,NARC_mmodel_goukazaru_nsbtx	 },
	{ POCHAMA,NARC_mmodel_pochama_nsbtx },
	{ POTTAISHI,NARC_mmodel_pottaishi_nsbtx	 },
	{ EMPERTE,NARC_mmodel_emperte_nsbtx },
	{ GUREGGRU,NARC_mmodel_gureggru_nsbtx },
	{ GIRATINAORIGIN,NARC_mmodel_giratinaorigin_nsbtx },
	{ BFSM,NARC_mmodel_bfsm_nsbtx	 },
	{ BFSW1,NARC_mmodel_bfsw1_nsbtx },
	{ BFSW2,NARC_mmodel_bfsw2_nsbtx },
	{ WIFISM,NARC_mmodel_wifism_nsbtx },
	{ WIFISW,NARC_mmodel_wifisw_nsbtx },
	{ ROTOM,NARC_mmodel_rotom_nsbtx },
	{ ROTOMF,NARC_mmodel_rotomf_nsbtx },
	{ ROTOMS,NARC_mmodel_rotoms_nsbtx },
	{ ROTOMI,NARC_mmodel_rotomi_nsbtx },
	{ ROTOMW,NARC_mmodel_rotomw_nsbtx },
	{ ROTOMG,NARC_mmodel_rotomg_nsbtx },
	{ DIRECTOR,NARC_mmodel_director_nsbtx },
	{ FIREBOX,NARC_mmodel_firebox_nsbtx },
	{ SKYBOX,NARC_mmodel_skybox_nsbtx },
	{ ICEBOX,NARC_mmodel_icebox_nsbtx },
	{ WATERBOX,NARC_mmodel_waterbox_nsbtx },
	{ GRASSBOX,NARC_mmodel_grassbox_nsbtx },
	{ GSET1,NARC_mmodel_gset1_nsbtx },
	{ GSET2,NARC_mmodel_gset2_nsbtx },
	{ MONUMENT2A, NARC_mmodel_monument2a_nsbtx },
	{ MONUMENT2B, NARC_mmodel_monument2b_nsbtx },
	{ DPHERO, NARC_mmodel_dphero_nsbtx },
	{ DPHEROINE, NARC_mmodel_dpheroine_nsbtx },
	{ DPCONTESTHERO, NARC_mmodel_dpcontesthero_nsbtx },
	{ DPCONTESTHEROINE, NARC_mmodel_dpcont_heroine_nsbtx },
	{ BSHERO, NARC_mmodel_bshero_nsbtx },
	{ BSHEROINE, NARC_mmodel_bsheroine_nsbtx },
	{ SWIMHERORT, NARC_mmodel_swimhero_nsbtx },
	{ SWIMHEROINERT, NARC_mmodel_swimheroine_nsbtx },
	{ SPHERORT, NARC_mmodel_sphero_nsbtx },
	{ SPHEROINERT, NARC_mmodel_spheroine_nsbtx },
	{ RTHEROINE, NARC_mmodel_heroine_nsbtx },
	{ BLDAGUNOMU,NARC_mmodel_sppoke2_nsbtx }, 
	{ TW7YUKUSHI,NARC_mmodel_sppoke1_nsbtx }, 
	{ TW7EMULIT,NARC_mmodel_sppoke2_nsbtx }, 
	{ TW7AGUNOMU,NARC_mmodel_sppoke3_nsbtx }, 
	{ SAVEHERORT, NARC_mmodel_savehero_nsbtx },
	{ SAVEHEROINERT, NARC_mmodel_saveheroine_nsbtx },
	{ POKEHERORT, NARC_mmodel_pokehero_nsbtx },
	{ POKEHEROINERT, NARC_mmodel_pokeheroine_nsbtx },
	{ GSTSAVEHERO, NARC_mmodel_savehero_nsbtx },
	{ GSTSAVEHEROINE, NARC_mmodel_saveheroine_nsbtx },
	{ GSTPOKEHERO, NARC_mmodel_pokehero_nsbtx },
	{ GSTPOKEHEROINE, NARC_mmodel_pokeheroine_nsbtx },
	
	{ OBJCODEMAX, 0 },
};

//==============================================================================
//	フィールド動作モデル モデルまとめ
//==============================================================================
//--------------------------------------------------------------
///	モデルID,パステーブル。
///	並びは不順
//--------------------------------------------------------------
const FLDMMDL_RESMNARC DATA_FLDMMDL_ResmNArcTbl_Mdl[] =
{
	{ BLACT_MDLID_32x32,NARC_mmodel_billboad_nsbmd },
	{ BLACT_MDLID_16x16, NARC_mmodel_rock_nsbmd },
	{ BLACT_MDLID_16x16_1, NARC_mmodel_breakrock_nsbmd },
	{ BLACT_MDLID_SEED_ME, NARC_mmodel_seed00_a_nsbmd },
	{ BLACT_MDLID_SEED, NARC_mmodel_seed01_a_nsbmd },
	{ BLACT_MDLID_64x64, NARC_mmodel_sppoke4_nsbmd },
	{ BLACT_MDLID_FISHINGHERO, NARC_mmodel_fishinghero_nsbmd },
	{ BLACT_MDLID_16x32, NARC_mmodel_door_nsbmd },
	{ BLACT_MDLID_64x32, NARC_mmodel_monument_nsbmd },
	{ BLACT_MDLID_32x32FOGOFF,NARC_mmodel_billboad_nsbmd },
	{ BLACT_MDLID_32x32BOFF,	NARC_mmodel_billboard_off_nsbmd },
	{ BLACT_MDLID_GIRATINA, NARC_mmodel_giratinaorigin_nsbmd },
	{ BLACT_MDLID_GSET, NARC_mmodel_gset1_nsbmd },
	{ BLACT_MDLID_MONUMENT2A, NARC_mmodel_monument2a_nsbmd },
	{ BLACT_MDLID_MONUMENT2B, NARC_mmodel_monument2b_nsbmd },
	{ BLACT_MDLID_ROTATE32x32,NARC_mmodel_hero_rote_nsbmd },
	{ BLACT_MDLID_GSET2, NARC_mmodel_gset2_nsbmd },
	{ BLACT_MDLID_32x32BLD,NARC_mmodel_billboad_nsbmd },
	{ BLACT_MDLID_MAX, 0 },
};

//==============================================================================
//	フィールド動作モデル アニメまとめ
//==============================================================================
//--------------------------------------------------------------
///	アニメID,パステーブル。
///	並びは不順
//--------------------------------------------------------------
const FLDMMDL_RESMNARC DATA_FLDMMDL_ResmNArcTbl_Anm[] =
{
	{ BLACT_ANMID_NORMAL_0, NARC_mmodel_charaobj_itpcv_dat },
	{ BLACT_ANMID_HERO_0, NARC_mmodel_hero_itpcv_dat },
	{ BLACT_ANMID_SEED, NARC_mmodel_seed00_a_itpcv_dat },
	{ BLACT_ANMID_HEROCYCLE_0, NARC_mmodel_cyclehero_itpcv_dat },
	{ BLACT_ANMID_SPHERO, NARC_mmodel_sphero_itpcv_dat },
	{ BLACT_ANMID_WATERHERO, NARC_mmodel_waterhero_itpcv_dat },
	{ BLACT_ANMID_SWIMHERO, NARC_mmodel_swimhero_itpcv_dat },
	{ BLACT_ANMID_BIRD,	NARC_mmodel_bird_itpcv_dat },
	{ BLACT_ANMID_SPPOKE,	NARC_mmodel_sppoke_itpcv_dat },
	{ BLACT_ANMID_CONTESTHERO, NARC_mmodel_contesthero_itpcv_dat },
	{ BLACT_ANMID_FISHINGHERO, NARC_mmodel_fishinghero_itpcv_dat },
	{ BLACT_ANMID_PCWOMAN, NARC_mmodel_pcwoman1_itpcv_dat },
	{ BLACT_ANMID_KOIKING, NARC_mmodel_koiking_itpcv_dat },
	{ BLACT_ANMID_POKEHERO, NARC_mmodel_pokehero_itpcv_dat },
	{ BLACT_ANMID_SAVEHERO, NARC_mmodel_savehero_itpcv_dat },
	{ BLACT_ANMID_BANZAIHERO, NARC_mmodel_banzaihero_itpcv_dat },
	{ BLACT_ANMID_SPPOKE6, NARC_mmodel_sppoke6_itpcv_dat },
	{ BLACT_ANMID_SPPOKE7, NARC_mmodel_sppoke7_itpcv_dat },
	{ BLACT_ANMID_SPPOKE9, NARC_mmodel_sppoke9_itpcv_dat },
	{ BLACT_ANMID_SPPOKE10, NARC_mmodel_sppoke10_itpcv_dat },
	{ BLACT_ANMID_SPPOKE11, NARC_mmodel_sppoke11_itpcv_dat },
	{ BLACT_ANMID_GIRATINA, NARC_mmodel_giratinaorigin_itpcv_dat },
	{ BLACT_ANMID_BSHERO, NARC_mmodel_bshero_itpcv_dat },
	{ BLACT_ANMID_MAX, 0 },
};

//==============================================================================
//	フィールド動作モデル アニメーションテーブル
//==============================================================================
//--------------------------------------------------------------
///	自機
//--------------------------------------------------------------
#if 0
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_Hero[] =
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

//--------------------------------------------------------------
///	自機自転車
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_HeroCycle[] =
{
	{ 0, 15, BLACT_ANIM_LOOP },
	{ 16, 31, BLACT_ANIM_LOOP },
	{ 32, 47, BLACT_ANIM_LOOP },
	{ 48, 63, BLACT_ANIM_LOOP },
	
	{ 64, 71, BLACT_ANIM_LOOP },
	{ 72, 79, BLACT_ANIM_LOOP },
	
	{ 80, 83, BLACT_ANIM_END },
	{ 84, 87, BLACT_ANIM_END },
	{ 88, 91, BLACT_ANIM_END },
	{ 92, 95, BLACT_ANIM_END },
	
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	自機波乗り
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_SwimHero[] =
{
	{ 0, 2, BLACT_ANIM_LOOP },
	{ 3, 5, BLACT_ANIM_LOOP },
	{ 6, 9, BLACT_ANIM_LOOP },
	{ 10, 11, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	自機秘伝技
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_SpHero[] =
{
	{ 0, 10, BLACT_ANIM_END },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	自機水遣り
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_WaterHero[] =
{
	{ 0, 15, BLACT_ANIM_LOOP },
	{ 16, 28, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	村人普通 0
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_Normal0[] =
{
	{ 0, 15, BLACT_ANIM_LOOP },
	{ 16, 31, BLACT_ANIM_LOOP },
	{ 32, 47, BLACT_ANIM_LOOP },
	{ 48, 63, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	PCWOMAN 1
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_PcWoman[] =
{
	{ 0, 15, BLACT_ANIM_LOOP },
	{ 16, 31, BLACT_ANIM_LOOP },
	{ 32, 47, BLACT_ANIM_LOOP },
	{ 48, 63, BLACT_ANIM_LOOP },
	
	{ 64, 68, BLACT_ANIM_END},
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	木の実
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_Seed0[] =
{
	{ 0, 60, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	鳥
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_Bird[] =
{
	{ 0, 7, BLACT_ANIM_LOOP },
	{ 8, 16, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
//	@autor	tomoya takahashi
///	伝説ポケモン	イア　エア
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_Legend[] =
{
	{ 0, 16, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
//	@autor	tomoya takahashi
///	伝説ポケモン	レイアイハイ
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_ReiAiHai[] =
{
	{ 0, 16, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	釣り自機
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_FishingHero[] =
{
	{ 0, 15, BLACT_ANIM_END },
	{ 16, 31, BLACT_ANIM_END },
	{ 32, 47, BLACT_ANIM_END },
	{ 48, 64, BLACT_ANIM_END },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	コイキング
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_Koiking[] =
{
	{ 0, 32, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	POKEHERO
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_PokeHero[] =
{
	{ 0, 11, BLACT_ANIM_END },
	{ 12, 23, BLACT_ANIM_END },
	{ 24, 35, BLACT_ANIM_END },
	{ 36, 48, BLACT_ANIM_END },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	SAVEHERO
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_SaveHero[] =
{
	{ 0, 8, BLACT_ANIM_END },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	BANZAIHERO
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_BanzaiHero[] =
{
	{ 0, 20, BLACT_ANIM_END },
	{ 21, 41, BLACT_ANIM_END },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
/// SPPOKE
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_Sppoke[] =
{
	{ 0, 20, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	GIRATINAORIGIN
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_Giratina[] =
{
	{ 0, 19, BLACT_ANIM_LOOP },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};

//--------------------------------------------------------------
///	自機バトルサーチャ
//--------------------------------------------------------------
static const BLACT_ANIME_TBL DATA_FLDMMDL_BlActAnmTbl_BSHero[] =
{
	{ 0, 59, BLACT_ANIM_END },
	{ 0, 0, BLACT_ANIM_CMD_MAX },
};
#endif

//==============================================================================
//	フィールド動作モデル リソースIDまとめ
//==============================================================================

//==============================================================================
//	フィールド動作モデル レギュラー勢
//==============================================================================
//--------------------------------------------------------------
///	テクスチャ　レギュラー
//--------------------------------------------------------------
const int DATA_FLDMMDL_Regular_Tex[] =
{
	BABYBOY1,
	BABYGIRL1,
	BOY1,
	BOY2,
	BOY3,
	GIRL1,
	GIRL2,
	GIRL3,
	MAN1,
	MAN2,
	MAN3,
	WOMAN1,
	WOMAN2,
	WOMAN3,
	MIDDLEMAN1,
	MIDDLEWOMAN1,
	OLDMAN1,
	OLDWOMAN1,
	BIGMAN,
	MOUNT,
	
	OBJCODEMAX,
};

//--------------------------------------------------------------
///	モデル　レギュラー
//--------------------------------------------------------------
const int DATA_FLDMMDL_Regular_Mdl[] =
{
	BLACT_MDLID_32x32,
	BLACT_MDLID_16x16,
	BLACT_MDLID_16x16_1,
	BLACT_MDLID_MAX,
};

//--------------------------------------------------------------
///	アニメ　レギュラー
//--------------------------------------------------------------
const int DATA_FLDMMDL_Regular_Anm[] =
{
	BLACT_ANMID_NORMAL_0,
	BLACT_ANMID_MAX,
};

//==============================================================================
//	フィールド動作モデル ビルボードアクターヘッダー
//==============================================================================
//--------------------------------------------------------------
///	フィールド動作モデル　ビルボードヘッダーパス、IDテーブル
///	並びはfieldobj_code.h 表示コードに準ず←やめた。不順
//--------------------------------------------------------------
#if 0
const FLDMMDL_BLACT_HEADER_ID DATA_FLDMMDL_BlActHeaderPathIDTbl[] =
{
	{HERO,BLACT_MDLID_32x32,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
    {BABYBOY1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {BABYGIRL1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {BOY1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {BOY2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {BOY3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {GIRL1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {GIRL2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {GIRL3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {MAN1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {MAN2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {MAN3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {WOMAN1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {WOMAN2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {WOMAN3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {MIDDLEMAN1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {MIDDLEWOMAN1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {OLDMAN1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {OLDWOMAN1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {BIGMAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {MOUNT,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
    {CYCLEHERO,BLACT_MDLID_32x32,BLACT_ANMID_HEROCYCLE_0,DATA_FLDMMDL_BlActAnmTbl_HeroCycle},
	{REPORTER,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{CAMERAMAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SHOPM1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SHOPW1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{PCWOMAN1,BLACT_MDLID_32x32,BLACT_ANMID_PCWOMAN,DATA_FLDMMDL_BlActAnmTbl_PcWoman},
	{PCWOMAN2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{PCWOMAN3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ASSISTANTM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ASSISTANTW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BADMAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SKIERM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SKIERW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{POLICEMAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{IDOL,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GENTLEMAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{LADY,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{CYCLEM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{CYCLEW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{WORKMAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{FARMER,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{COWGIRL,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{CLOWN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ARTIST,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SPORTSMAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SWIMMERM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SWIMMERW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BEACHGIRL,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BEACHBOY,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{EXPLORE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{FIGHTER,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{CAMPBOY,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{PICNICGIRL,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{FISHING,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{AMBRELLA,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEAMAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BRINGM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BRINGW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{WAITER,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{WAITRESS,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{VETERAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GORGGEOUSM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GORGGEOUSW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BOY4,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GIRL4,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MAN4,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{WOMAN4,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MAN5,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{WOMAN5,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MYSTERY,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{PIKACHU,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{PIPPI,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{PURIN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{KODUCK,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{PRASLE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MINUN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MARIL,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ACHAMO,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ENECO,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GONBE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BABY,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MIDDLEMAN2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MIDDLEWOMAN2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ROCK,BLACT_MDLID_16x16,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BREAKROCK,BLACT_MDLID_16x16_1,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{TREE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MONATERBALL,BLACT_MDLID_16x16,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SHADOW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BOUGH,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{FALLTREE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{HEROINE,BLACT_MDLID_32x32,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
    {CYCLEHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_HEROCYCLE_0,DATA_FLDMMDL_BlActAnmTbl_HeroCycle},
	{DOCTOR,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEED00,BLACT_MDLID_SEED_ME,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED01A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED01B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED01C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{GINGABOSS,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GKANBU1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GKANBU2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GKANBU3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GINGAM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GINGAW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{LEADER1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{LEADER2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{LEADER3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{LEADER4,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{LEADER5,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{LEADER6,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{LEADER7,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{LEADER8,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BIGFOUR1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BIGFOUR2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BIGFOUR3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BIGFOUR4,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{CHAMPION,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{PAPA,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MAMA,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEVEN1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEVEN2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEVEN3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEVEN4,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEVEN5,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEVEN6,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEVEN7,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{RIVEL,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BOAT,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{TRAIN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SPPOKE1,BLACT_MDLID_32x32,BLACT_ANMID_SPPOKE,DATA_FLDMMDL_BlActAnmTbl_ReiAiHai},
	{SPPOKE2,BLACT_MDLID_32x32,BLACT_ANMID_SPPOKE,DATA_FLDMMDL_BlActAnmTbl_ReiAiHai},
	{SPPOKE3,BLACT_MDLID_32x32,BLACT_ANMID_SPPOKE,DATA_FLDMMDL_BlActAnmTbl_ReiAiHai},
	{SPPOKE4,BLACT_MDLID_64x64,BLACT_ANMID_SPPOKE,DATA_FLDMMDL_BlActAnmTbl_Legend},
	{SPPOKE5,BLACT_MDLID_64x64,BLACT_ANMID_SPPOKE,DATA_FLDMMDL_BlActAnmTbl_Legend},
	{SPPOKE6,BLACT_MDLID_64x64,BLACT_ANMID_SPPOKE6,DATA_FLDMMDL_BlActAnmTbl_Sppoke},
	{SPPOKE7,BLACT_MDLID_32x32,BLACT_ANMID_SPPOKE7,DATA_FLDMMDL_BlActAnmTbl_Sppoke},
	{SPPOKE8,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SPPOKE9,BLACT_MDLID_32x32,BLACT_ANMID_SPPOKE9,DATA_FLDMMDL_BlActAnmTbl_Sppoke},
	{SPPOKE10,BLACT_MDLID_64x64,BLACT_ANMID_SPPOKE10,DATA_FLDMMDL_BlActAnmTbl_Sppoke},
	{SPPOKE11,BLACT_MDLID_32x32,BLACT_ANMID_SPPOKE11,DATA_FLDMMDL_BlActAnmTbl_Sppoke},
	{OBAKE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{WOMAN6,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{OLDMAN2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{OLDWOMAN2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{OOKIDO,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MIKAN,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SUNGLASSES,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{TOWERBOSS,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SNOWCOVER,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{TREECOVER,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ROCKCOVER,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BIRD,BLACT_MDLID_32x32,BLACT_ANMID_BIRD,DATA_FLDMMDL_BlActAnmTbl_Bird},
	{BAG,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MAID,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SPHERO,BLACT_MDLID_32x32,BLACT_ANMID_SPHERO,DATA_FLDMMDL_BlActAnmTbl_SpHero },
	{SPHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_SPHERO,DATA_FLDMMDL_BlActAnmTbl_SpHero},
	{SWIMHERO,BLACT_MDLID_32x32,BLACT_ANMID_SWIMHERO,DATA_FLDMMDL_BlActAnmTbl_SwimHero},
	{SWIMHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_SWIMHERO,DATA_FLDMMDL_BlActAnmTbl_SwimHero},
	{WATERHERO,BLACT_MDLID_32x32,BLACT_ANMID_WATERHERO,DATA_FLDMMDL_BlActAnmTbl_WaterHero},
	{WATERHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_WATERHERO,DATA_FLDMMDL_BlActAnmTbl_WaterHero},
	{VENTHOLE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SPPOKE12,BLACT_MDLID_64x64,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{BALLOON,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{CONTESTHERO,BLACT_MDLID_32x32,BLACT_ANMID_CONTESTHERO,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{CONTESTHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_CONTESTHERO,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{FISHINGHERO,BLACT_MDLID_FISHINGHERO,BLACT_ANMID_FISHINGHERO,DATA_FLDMMDL_BlActAnmTbl_FishingHero},
	{FISHINGHEROINE,BLACT_MDLID_FISHINGHERO,BLACT_ANMID_FISHINGHERO,DATA_FLDMMDL_BlActAnmTbl_FishingHero},
	{MOSS,BLACT_MDLID_16x16,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{FREEZES,BLACT_MDLID_16x16,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{POLE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{SEED02A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED02B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED02C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED03A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED03B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED03C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED04A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED04B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED04C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED05A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED05B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED05C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED06A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED06B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED06C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED07A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED07B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED07C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED08A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED08B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED08C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED09A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED09B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED09C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED10A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED10B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED10C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED11A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED11B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED11C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED12A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED12B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED12C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED13A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED13B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED13C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED14A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED14B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED14C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED15A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED15B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED15C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED16A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED16B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED16C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED17A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED17B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED17C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED18A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED18B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED18C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED19A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED19B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED19C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED20A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED20B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED20C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED21A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED21B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED21C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED22A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED22B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED22C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED23A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED23B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED23C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED24A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED24B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED24C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED25A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED25B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED25C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED26A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED26B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED26C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED27A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED27B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED27C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED28A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED28B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED28C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED29A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED29B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED29C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED30A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED30B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED30C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED31A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED31B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED31C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED32A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED32B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED32C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED33A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED33B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED33C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED34A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED34B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED34C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED35A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED35B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED35C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED36A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED36B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED36C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED37A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED37B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED37C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED38A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED38B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED38C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED39A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED39B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED39C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED40A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED40B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED40C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED41A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED41B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED41C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED42A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED42B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED42C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED43A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED43B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED43C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED44A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED44B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED44C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED45A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED45B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED45C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED46A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED46B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED46C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED47A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED47B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED47C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED48A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED48B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED48C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED49A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED49B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED49C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED50A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED50B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED50C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED51A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED51B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED51C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED52A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED52B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED52C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED53A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED53B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED53C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED54A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED54B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED54C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED55A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED55B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED55C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED56A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED56B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED56C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED57A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED57B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED57C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED58A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED58B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED58C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED59A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED59B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED59C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED60A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED60B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED60C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED61A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED61B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED61C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED62A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED62B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED62C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED63A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED63B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED63C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED64A,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED64B,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{SEED64C,BLACT_MDLID_SEED,BLACT_ANMID_SEED,DATA_FLDMMDL_BlActAnmTbl_Seed0},
	{DELIVERY,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{DSBOY,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{KOIKING,BLACT_MDLID_32x32,BLACT_ANMID_KOIKING,DATA_FLDMMDL_BlActAnmTbl_Koiking},
	{POKEHERO,BLACT_MDLID_32x32,BLACT_ANMID_POKEHERO,DATA_FLDMMDL_BlActAnmTbl_PokeHero},
	{POKEHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_POKEHERO,DATA_FLDMMDL_BlActAnmTbl_PokeHero},
	{SAVEHERO,BLACT_MDLID_32x32,BLACT_ANMID_SAVEHERO,DATA_FLDMMDL_BlActAnmTbl_SaveHero },
	{SAVEHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_SAVEHERO,DATA_FLDMMDL_BlActAnmTbl_SaveHero},
	{BANZAIHERO,BLACT_MDLID_32x32,BLACT_ANMID_BANZAIHERO,DATA_FLDMMDL_BlActAnmTbl_BanzaiHero },
	{BANZAIHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_BANZAIHERO,DATA_FLDMMDL_BlActAnmTbl_BanzaiHero},
	{DOOR,BLACT_MDLID_16x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MONUMENT,BLACT_MDLID_64x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{PATIRITUSU,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{KINOCOCO,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MIMITUTO,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{KOLUCKY,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{WANRIKY,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{GHOSTHERO,BLACT_MDLID_32x32FOGOFF,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{GHOSTHEROINE,BLACT_MDLID_32x32FOGOFF,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{RTHERO,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{ ICPO,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ GKANBU4,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ BRAINS1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ BRAINS2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ BRAINS3,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ BRAINS4,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ PRINCESS,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ NAETLE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ HAYASHIGAME,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ DODAITOSE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ HIKOZARU,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ MOUKAZARU,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ GOUKAZARU,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ POCHAMA,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ POTTAISHI,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ EMPERTE,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ GUREGGRU,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ GIRATINAORIGIN,BLACT_MDLID_GIRATINA,BLACT_ANMID_GIRATINA,DATA_FLDMMDL_BlActAnmTbl_Giratina},
	{ BFSM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ BFSW1,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ BFSW2,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ WIFISM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ WIFISW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ ROTOM,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ ROTOMF,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ ROTOMS,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ ROTOMI,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ ROTOMW,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ ROTOMG,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ DIRECTOR,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ FIREBOX,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ SKYBOX,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ ICEBOX,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ WATERBOX,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ GRASSBOX,BLACT_MDLID_32x32,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ GSET1,BLACT_MDLID_GSET,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{ GSET2,BLACT_MDLID_GSET2,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MONUMENT2A,BLACT_MDLID_MONUMENT2A,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{MONUMENT2B,BLACT_MDLID_MONUMENT2B,BLACT_ANMID_NORMAL_0,DATA_FLDMMDL_BlActAnmTbl_Normal0},
	{DPHERO,BLACT_MDLID_32x32,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{DPHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{DPCONTESTHERO,BLACT_MDLID_32x32,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{DPCONTESTHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{BSHERO,BLACT_MDLID_32x32,BLACT_ANMID_BSHERO,DATA_FLDMMDL_BlActAnmTbl_BSHero },
	{BSHEROINE,BLACT_MDLID_32x32,BLACT_ANMID_BSHERO,DATA_FLDMMDL_BlActAnmTbl_BSHero},
	{SWIMHERORT,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_SWIMHERO,DATA_FLDMMDL_BlActAnmTbl_SwimHero},
	{SWIMHEROINERT,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_SWIMHERO,DATA_FLDMMDL_BlActAnmTbl_SwimHero},
	{SPHERORT,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_SPHERO,DATA_FLDMMDL_BlActAnmTbl_SpHero },
	{SPHEROINERT,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_SPHERO,DATA_FLDMMDL_BlActAnmTbl_SpHero},
	{RTHEROINE,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_HERO_0,DATA_FLDMMDL_BlActAnmTbl_Hero},
	{BLDAGUNOMU,BLACT_MDLID_32x32BLD,BLACT_ANMID_SPPOKE,DATA_FLDMMDL_BlActAnmTbl_ReiAiHai},
	{TW7YUKUSHI,BLACT_MDLID_32x32,BLACT_ANMID_SPPOKE,DATA_FLDMMDL_BlActAnmTbl_ReiAiHai},
	{TW7EMULIT,BLACT_MDLID_32x32,BLACT_ANMID_SPPOKE,DATA_FLDMMDL_BlActAnmTbl_ReiAiHai},
	{TW7AGUNOMU,BLACT_MDLID_32x32,BLACT_ANMID_SPPOKE,DATA_FLDMMDL_BlActAnmTbl_ReiAiHai},
	{SAVEHERORT,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_SAVEHERO,DATA_FLDMMDL_BlActAnmTbl_SaveHero },
	{SAVEHEROINERT,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_SAVEHERO,DATA_FLDMMDL_BlActAnmTbl_SaveHero},
	{POKEHERORT,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_POKEHERO,DATA_FLDMMDL_BlActAnmTbl_PokeHero},
	{POKEHEROINERT,BLACT_MDLID_ROTATE32x32,BLACT_ANMID_POKEHERO,DATA_FLDMMDL_BlActAnmTbl_PokeHero},
	{GSTSAVEHERO,BLACT_MDLID_32x32FOGOFF,BLACT_ANMID_SAVEHERO,DATA_FLDMMDL_BlActAnmTbl_SaveHero },
	{GSTSAVEHEROINE,BLACT_MDLID_32x32FOGOFF,BLACT_ANMID_SAVEHERO,DATA_FLDMMDL_BlActAnmTbl_SaveHero},
	{GSTPOKEHERO,BLACT_MDLID_32x32FOGOFF,BLACT_ANMID_POKEHERO,DATA_FLDMMDL_BlActAnmTbl_PokeHero},
	{GSTPOKEHEROINE,BLACT_MDLID_32x32FOGOFF,BLACT_ANMID_POKEHERO,DATA_FLDMMDL_BlActAnmTbl_PokeHero},
	{OBJCODEMAX,BLACT_MDLID_MAX,BLACT_ANMID_MAX,NULL},
};
#endif	


//--------------------------------------------------------------
///	フォグ無効コード
//	※本来ならヘッダーに纏めるべきだがヘッダーデータ全体に与える影響を考え
//	無効化するコードだけをここに指定。並びは不順。
//--------------------------------------------------------------
const int DATA_FLDMMDL_BlActFogEnableOFFTbl[] =
{
	BLACT_MDLID_32x32FOGOFF,
	BLACT_MDLID_MAX,  //終端識別
};

//==============================================================================
//	OBJコードステータスデータ
//==============================================================================
//--------------------------------------------------------------
//	OBJコード別描画ステータス　並びは不順
//--------------------------------------------------------------
const OBJCODE_STATE DATA_FieldOBJCodeDrawStateTbl[] =
{
 {HERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BABYBOY1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BABYGIRL1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BOY1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BOY2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BOY3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GIRL1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GIRL2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GIRL3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MAN1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MAN2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MAN3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WOMAN1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WOMAN2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WOMAN3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MIDDLEMAN1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MIDDLEWOMAN1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {OLDMAN1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {OLDWOMAN1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BIGMAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MOUNT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {CYCLEHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_CYCLE,FLDMMDL_REFLECT_BLACT,0},
 {REPORTER,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {CAMERAMAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SHOPM1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SHOPW1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {PCWOMAN1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {PCWOMAN2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {PCWOMAN3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {ASSISTANTM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {ASSISTANTW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BADMAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SKIERM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SKIERW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {POLICEMAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {IDOL,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GENTLEMAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {LADY,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {CYCLEM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_CYCLE,FLDMMDL_REFLECT_BLACT,0},
 {CYCLEW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_CYCLE,FLDMMDL_REFLECT_BLACT,0},
 {WORKMAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {FARMER,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {COWGIRL,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {CLOWN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {ARTIST,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPORTSMAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SWIMMERM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SWIMMERW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BEACHGIRL,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BEACHBOY,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {EXPLORE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {FIGHTER,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {CAMPBOY,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {PICNICGIRL,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {FISHING,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {AMBRELLA,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SEAMAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BRINGM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BRINGW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WAITER,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WAITRESS,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {VETERAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GORGGEOUSM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GORGGEOUSW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BOY4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GIRL4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MAN4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WOMAN4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MAN5,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WOMAN5,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MYSTERY,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {PIKACHU,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {PIPPI,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {PURIN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {KODUCK,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {PRASLE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MINUN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MARIL,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {ACHAMO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {ENECO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GONBE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BABY,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MIDDLEMAN2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MIDDLEWOMAN2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {ROCK,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BREAKROCK,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {TREE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MONATERBALL,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SHADOW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BOUGH,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {FALLTREE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BOARD_A,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {BOARD_B,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {BOARD_C,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {BOARD_D,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {BOARD_E,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {BOARD_F,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {HEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {CYCLEHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_CYCLE,FLDMMDL_REFLECT_BLACT,0},
 {DOCTOR,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SEED,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_BLACT,0},
 {SNOWBALL,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {GINGABOSS,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GKANBU1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GKANBU2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GKANBU3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GINGAM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GINGAW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {LEADER1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {LEADER2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {LEADER3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {LEADER4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {LEADER5,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {LEADER6,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {LEADER7,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {LEADER8,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BIGFOUR1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BIGFOUR2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BIGFOUR3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BIGFOUR4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {CHAMPION,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {PAPA,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MAMA,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SEVEN1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SEVEN2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SEVEN3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SEVEN4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SEVEN5,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SEVEN6,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SEVEN7,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {RIVEL,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BOAT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {TRAIN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE5,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE6,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE7,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE8,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE9,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE10,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPPOKE11,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {OBAKE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WOMAN6,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {OLDMAN2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {OLDWOMAN2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {OOKIDO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MIKAN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SUNGLASSES,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {TOWERBOSS,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SNOWCOVER,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {TREECOVER,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {ROCKCOVER,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BIRD,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BAG,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MAID,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SPHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SWIMHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SWIMHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WATERHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {WATERHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {VENTHOLE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BOOK,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {SPPOKE12,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BALLOON,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {CONTESTHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {CONTESTHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {FISHINGHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {FISHINGHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {MOSS,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {FREEZES,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {POLE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {DELIVERY,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {DSBOY,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {KOIKING,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {NONDRAW,FLDMMDL_DRAWTYPE_NON,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {POKEHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {POKEHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SAVEHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SAVEHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BANZAIHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BANZAIHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {DOOR,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {MONUMENT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_BLACT,0},
 {PATIRITUSU,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_CYCLE,FLDMMDL_REFLECT_BLACT,0},
 {KINOCOCO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_CYCLE,FLDMMDL_REFLECT_BLACT,0},
 {MIMITUTO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_CYCLE,FLDMMDL_REFLECT_BLACT,0},
 {KOLUCKY,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_CYCLE,FLDMMDL_REFLECT_BLACT,0},
 {WANRIKY,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_CYCLE,FLDMMDL_REFLECT_BLACT,0},
 {DOOR2,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {GHOSTHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GHOSTHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {RTHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ ICPO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ GKANBU4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ BRAINS1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ BRAINS2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ BRAINS3,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ BRAINS4,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ PRINCESS,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ NAETLE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ HAYASHIGAME,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ DODAITOSE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ HIKOZARU,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ MOUKAZARU,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ GOUKAZARU,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ POCHAMA,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ POTTAISHI,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ EMPERTE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ GUREGGRU,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ GIRATINAORIGIN,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ BFSM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ BFSW1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ BFSW2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ WIFISM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ WIFISW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ ROTOM,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ ROTOMF,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ ROTOMS,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ ROTOMI,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ ROTOMW,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ ROTOMG,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ DIRECTOR,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ FIREBOX,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ SKYBOX,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ ICEBOX,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ WATERBOX,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ GRASSBOX,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ GSET1,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{ GSET2,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
	{MONUMENT2A,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_BLACT,0},
	{MONUMENT2B,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_BLACT,0},
 {DPHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {DPHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {DPCONTESTHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {DPCONTESTHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BSHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BSHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SWIMHERORT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {SWIMHEROINERT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {SPHERORT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {SPHEROINERT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {ROTOMWALL,FLDMMDL_DRAWTYPE_MDL,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NON,FLDMMDL_REFLECT_NON,0},
 {RTHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {BLDAGUNOMU,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {TW7YUKUSHI,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {TW7EMULIT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {TW7AGUNOMU,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_ON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SAVEHERORT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {SAVEHEROINERT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {POKEHERORT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {POKEHEROINERT,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GSTSAVEHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GSTSAVEHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GSTPOKEHERO,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},
 {GSTPOKEHEROINE,FLDMMDL_DRAWTYPE_BLACT,FLDMMDL_SHADOW_NON,FLDMMDL_FOOTMARK_NORMAL,FLDMMDL_REFLECT_BLACT,0},

 {OBJCODEMAX,0,0,0,0,0},
};

//==============================================================================
//	木の実データ
//==============================================================================
#if 0
//--------------------------------------------------------------
///	種類、成長別木の実OBJコード
//--------------------------------------------------------------
const OBJCODE_SEEDDATA DATA_OBJCodeSeedDataTbl[] =
{
	{SEED01A,SEED01B,SEED01C},
	{SEED02A,SEED02B,SEED02C},
	{SEED03A,SEED03B,SEED03C},
	{SEED04A,SEED04B,SEED04C},
	{SEED05A,SEED05B,SEED05C},
	{SEED06A,SEED06B,SEED06C},
	{SEED07A,SEED07B,SEED07C},
	{SEED08A,SEED08B,SEED08C},
	{SEED09A,SEED09B,SEED09C},
	{SEED10A,SEED10B,SEED10C},
	{SEED11A,SEED11B,SEED11C},
	{SEED12A,SEED12B,SEED12C},
	{SEED13A,SEED13B,SEED13C},
	{SEED14A,SEED14B,SEED14C},
	{SEED15A,SEED15B,SEED15C},
	{SEED16A,SEED16B,SEED16C},
	{SEED17A,SEED17B,SEED17C},
	{SEED18A,SEED18B,SEED18C},
	{SEED19A,SEED19B,SEED19C},
	{SEED20A,SEED20B,SEED20C},
	{SEED21A,SEED21B,SEED21C},
	{SEED22A,SEED22B,SEED22C},
	{SEED23A,SEED23B,SEED23C},
	{SEED24A,SEED24B,SEED24C},
	{SEED25A,SEED25B,SEED25C},
	{SEED26A,SEED26B,SEED26C},
	{SEED27A,SEED27B,SEED27C},
	{SEED28A,SEED28B,SEED28C},
	{SEED29A,SEED29B,SEED29C},
	{SEED30A,SEED30B,SEED30C},
	{SEED31A,SEED31B,SEED31C},
	{SEED32A,SEED32B,SEED32C},
	{SEED33A,SEED33B,SEED33C},
	{SEED34A,SEED34B,SEED34C},
	{SEED35A,SEED35B,SEED35C},
	{SEED36A,SEED36B,SEED36C},
	{SEED37A,SEED37B,SEED37C},
	{SEED38A,SEED38B,SEED38C},
	{SEED39A,SEED39B,SEED39C},
	{SEED40A,SEED40B,SEED40C},
	{SEED41A,SEED41B,SEED41C},
	{SEED42A,SEED42B,SEED42C},
	{SEED43A,SEED43B,SEED43C},
	{SEED44A,SEED44B,SEED44C},
	{SEED45A,SEED45B,SEED45C},
	{SEED46A,SEED46B,SEED46C},
	{SEED47A,SEED47B,SEED47C},
	{SEED48A,SEED48B,SEED48C},
	{SEED49A,SEED49B,SEED49C},
	{SEED50A,SEED50B,SEED50C},
	{SEED51A,SEED51B,SEED51C},
	{SEED52A,SEED52B,SEED52C},
	{SEED53A,SEED53B,SEED53C},
	{SEED54A,SEED54B,SEED54C},
	{SEED55A,SEED55B,SEED55C},
	{SEED56A,SEED56B,SEED56C},
	{SEED57A,SEED57B,SEED57C},
	{SEED58A,SEED58B,SEED58C},
	{SEED59A,SEED59B,SEED59C},
	{SEED60A,SEED60B,SEED60C},
	{SEED61A,SEED61B,SEED61C},
	{SEED62A,SEED62B,SEED62C},
	{SEED63A,SEED63B,SEED63C},
	{SEED64A,SEED64B,SEED64C},
};
#endif

//==============================================================================
//	フィールドエフェクトレンダ表示OBJ
//==============================================================================
#define ROBJ_OFFS_X (FLDMMDL_BLACT_X_GROUND_OFFS_FX32)
#define ROBJ_OFFS_Y (FLDMMDL_BLACT_Y_GROUND_OFFS_FX32)
#define ROBJ_OFFS_Z (FLDMMDL_BLACT_Z_GROUND_OFFS_FX32-NUM_FX32(6))
#define BOARD_OFFS_X (FLDMMDL_BLACT_X_GROUND_OFFS_FX32)
#define BOARD_OFFS_Y (FLDMMDL_BLACT_Y_GROUND_OFFS_FX32)
#define BOARD_OFFS_Z (FLDMMDL_BLACT_Z_GROUND_OFFS_FX32-NUM_FX32(6)-NUM_FX32(2))

//--------------------------------------------------------------
///	表示コード別レンダ表示位置オフセット
//--------------------------------------------------------------
const CODEOFFS DATA_FieldOBJRenderOBJDrawOffset[] =
{
	{ BOARD_A,{BOARD_OFFS_X,BOARD_OFFS_Y,BOARD_OFFS_Z} },
	{ BOARD_B,{BOARD_OFFS_X,BOARD_OFFS_Y,BOARD_OFFS_Z} },
	{ BOARD_C,{BOARD_OFFS_X,BOARD_OFFS_Y,BOARD_OFFS_Z} },
	{ BOARD_D,{BOARD_OFFS_X,BOARD_OFFS_Y,BOARD_OFFS_Z} },
	{ BOARD_E,{BOARD_OFFS_X,BOARD_OFFS_Y,BOARD_OFFS_Z} },
	{ BOARD_F,{BOARD_OFFS_X,BOARD_OFFS_Y,BOARD_OFFS_Z} },
	{ BOOK, {ROBJ_OFFS_X,ROBJ_OFFS_Y,ROBJ_OFFS_Z}, },
	{ DOOR2, {ROBJ_OFFS_X,ROBJ_OFFS_Y,ROBJ_OFFS_Z}, },
	{ ROTOMWALL, {ROBJ_OFFS_X,ROBJ_OFFS_Y,ROBJ_OFFS_Z}, },
	{OBJCODEMAX,{0,0,0},},
};

