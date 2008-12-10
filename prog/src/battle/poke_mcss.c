
//============================================================================================
/**
 * @file	poke_mcss.c
 * @brief	ポケモンMCSS管理
 * @author	soga
 * @date	2008.11.14
 */
//============================================================================================

#include <gflib.h>

#include "system/gfl_use.h"

#include "btl_efftool.h"
#include "poke_mcss.h"

#include "arc_def.h"
#include "pokegra/pokegra_wb.naix"
#include "battgra/battgra_wb.naix"

extern	NNSG2dImagePaletteProxy		shadow_palette;

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================

#define	POKE_MCSS_MAX	( 6 )	//POKE_MCSSで管理するMCSSのMAX値

#define	POKE_MCSS_DEFAULT_SHIFT	( FX32_SHIFT - 4 )					//ポリゴン１辺の基準の長さにするシフト値
#define	POKE_MCSS_DEFAULT_LINE	( 1 << POKE_MCSS_DEFAULT_SHIFT )	//ポリゴン１辺の基準の長さ

//ポケモン一体を構成するMCSS用ファイルの構成
enum{
	POKEGRA_FRONT_M_NCGR = 0,
	POKEGRA_FRONT_F_NCGR,
	POKEGRA_FRONT_M_NCBR,
	POKEGRA_FRONT_F_NCBR,
	POKEGRA_FRONT_NCER,
	POKEGRA_FRONT_NANR,
	POKEGRA_FRONT_NMCR,
	POKEGRA_FRONT_NMAR,
	POKEGRA_FRONT_NCEC,
	POKEGRA_BACK_M_NCGR,
	POKEGRA_BACK_F_NCGR,
	POKEGRA_BACK_M_NCBR,
	POKEGRA_BACK_F_NCBR,
	POKEGRA_BACK_NCER,
	POKEGRA_BACK_NANR,
	POKEGRA_BACK_NMCR,
	POKEGRA_BACK_NMAR,
	POKEGRA_BACK_NCEC,
	POKEGRA_NORMAL_NCLR,
	POKEGRA_RARE_NCLR,

	POKEGRA_FILE_MAX,			//ポケモン一体を構成するMCSS用ファイルの総数

	POKEGRA_M_NCGR = 0,
	POKEGRA_F_NCGR,
	POKEGRA_M_NCBR,
	POKEGRA_F_NCBR,
	POKEGRA_NCER,
	POKEGRA_NANR,
	POKEGRA_NMCR,
	POKEGRA_NMAR,
	POKEGRA_NCEC
};

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

struct _POKE_MCSS_WORK
{
	GFL_TCBSYS				*tcb_sys;
	MCSS_SYS_WORK			*mcss_sys;
	MCSS_WORK				*mcss[ POKE_MCSS_POS_MAX ];
	NNSG2dImageProxy		image_proxy;		//影のテクスチャプロキシ
	NNSG2dImagePaletteProxy	palette_proxy;		//影のパレットプロキシ
	HEAPID					heapID;
};

typedef struct
{
	POKE_MCSS_WORK		*pmw;
	EFFTOOL_MOVE_WORK	emw;
}POKE_MCSS_TCB_WORK;

typedef struct	
{
	NNSG2dCharacterData*	pCharData;			//テクスチャキャラ
	NNSG2dPaletteData*		pPlttData;			//テクスチャパレット
	void					*pBufChar;			//テクスチャキャラバッファ
	void					*pBufPltt;			//テクスチャパレットバッファ
	int						chr_ofs;
	int						pal_ofs;
	POKE_MCSS_WORK			*pmw;
}TCB_LOADRESOURCE_WORK;

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

POKE_MCSS_WORK	*POKE_MCSS_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID );
void			POKE_MCSS_Exit( POKE_MCSS_WORK *pmw );
void			POKE_MCSS_Main( POKE_MCSS_WORK *pmw );
void			POKE_MCSS_Draw( POKE_MCSS_WORK *pmw );
void			POKE_MCSS_Add( POKE_MCSS_WORK *pmw, const POKEMON_PARAM *pp, int position );
void			POKE_MCSS_Del( POKE_MCSS_WORK *pmw, int position );
void			POKE_MCSS_SetMepachiFlag( POKE_MCSS_WORK *pmw, int position, int flag );
void			POKE_MCSS_SetAnmStopFlag( POKE_MCSS_WORK *pmw, int position, int flag );
void			POKE_MCSS_GetPokeDefaultPos( VecFx32 *pos, int position );
fx32			POKE_MCSS_GetPokeDefaultScale( int position );
void			POKE_MCSS_GetScale( POKE_MCSS_WORK *pmw, int position, VecFx32 *scale );
void			POKE_MCSS_SetScale( POKE_MCSS_WORK *pmw, int position, VecFx32 *scale );

void			POKE_MCSS_MovePosition( POKE_MCSS_WORK *pmw, int position, int move_type, VecFx32 *pos, int speed, int wait, int count );
void			POKE_MCSS_MoveScale( POKE_MCSS_WORK *pmw, int position, int move_type, VecFx32 *scale, int speed, int wait, int count );

static	void	POKE_MCSS_MakeMAW( const POKEMON_PARAM *pp, MCSS_ADD_WORK *maw, int position );
static	void	POKE_MCSS_LoadShadowResource( POKE_MCSS_WORK *pmw );
static	void	TCB_LoadResource( GFL_TCB *tcb, void *work );

static	void	TCB_POKE_MCSS_Move( GFL_TCB *tcb, void *work );
static	void	TCB_POKE_MCSS_Scale( GFL_TCB *tcb, void *work );

static	void	POKE_MCSS_DrawShadow( POKE_MCSS_WORK *pmw );

//============================================================================================
/**
 *	ポケモンの立ち位置テーブル
 */
//============================================================================================
static	const	VecFx32	poke_pos_table[]={
	{ FX_F32_TO_FX32( -2.5f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   8.0f ) },		//POS_AA
	{ FX_F32_TO_FX32(  4.5f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },		//POS_BB
	{ FX_F32_TO_FX32( -3.5f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   8.5f ) },		//POS_A
	{ FX_F32_TO_FX32(  6.0f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(  -9.0f ) },		//POS_B
	{ FX_F32_TO_FX32( -1.5f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   9.0f ) },		//POS_C
	{ FX_F32_TO_FX32(  3.0f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -11.0f ) },		//POS_D
	{ FX_F32_TO_FX32( -2.5f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(  10.0f ) },		//POS_E
	{ FX_F32_TO_FX32(  4.5f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },		//POS_F
};

//============================================================================================
/**
 *	ポケモンの立ち位置によるスケール補正テーブル
 */
//============================================================================================
static	const	fx32	poke_scale_table[]={
	FX_F32_TO_FX32( 0.75f ),	//POS_AA
	FX_F32_TO_FX32( 1.35f ),	//POS_BB
	FX_F32_TO_FX32( 0.8f ),		//POS_A
	FX_F32_TO_FX32( 1.30f ),	//POS_B
	FX_F32_TO_FX32( 0.8f ),		//POS_C
	FX_F32_TO_FX32( 1.40f ),	//POS_D
	FX_F32_TO_FX32( 0.8f ),		//POS_E
	FX_F32_TO_FX32( 1.4f ),		//POS_F
};

//============================================================================================
/**
 *	システム初期化
 *
 * @param[in]	tcb_sys	システム内で使用するTCBSYS構造体へのポインタ
 * @param[in]	heapID	ヒープID
 */
//============================================================================================
POKE_MCSS_WORK	*POKE_MCSS_Init( GFL_TCBSYS	*tcb_sys, HEAPID heapID )
{
	POKE_MCSS_WORK *pmw = GFL_HEAP_AllocClearMemory( heapID, sizeof( POKE_MCSS_WORK ) );

	pmw->mcss_sys = MCSS_Init( POKE_MCSS_MAX, heapID );
	pmw->tcb_sys  = tcb_sys;

	POKE_MCSS_LoadShadowResource( pmw );

	return pmw;
}

//============================================================================================
/**
 *	システム終了
 *
 * @param[in]	pmw	POKE_MCSS管理ワークへのポインタ
 */
//============================================================================================
void	POKE_MCSS_Exit( POKE_MCSS_WORK *pmw )
{
	MCSS_Exit( pmw->mcss_sys );
	GFL_HEAP_FreeMemory( pmw );
}

//============================================================================================
/**
 *	システムメイン
 *
 * @param[in]	pmw	POKE_MCSS管理ワークへのポインタ
 */
//============================================================================================
void	POKE_MCSS_Main( POKE_MCSS_WORK *pmw )
{
	MCSS_Main( pmw->mcss_sys );
}

//============================================================================================
/**
 *	描画
 *
 * @param[in]	pmw	POKE_MCSS管理ワークへのポインタ
 */
//============================================================================================
void	POKE_MCSS_Draw( POKE_MCSS_WORK *pmw )
{
	MCSS_Draw( pmw->mcss_sys );
//	POKE_MCSS_DrawShadow( pmw );
}

//============================================================================================
/**
 *	POKE_MCSS追加
 *
 * @param[in]	pmw			POKE_MCSS管理ワークへのポインタ
 * @param[in]	pp			POKEMON_PARAM構造体へのポインタ
 * @param[in]	position	ポケモンの立ち位置
 */
//============================================================================================
void	POKE_MCSS_Add( POKE_MCSS_WORK *pmw, const POKEMON_PARAM *pp, int position )
{
	MCSS_ADD_WORK	maw;
	VecFx32			scale;

	GF_ASSERT( position < POKE_MCSS_POS_MAX );
	GF_ASSERT( pmw->mcss[ position ] == NULL );

	POKE_MCSS_MakeMAW( pp, &maw, position );
	pmw->mcss[ position ] = MCSS_Add( pmw->mcss_sys,
									  poke_pos_table[ position ].x,
									  poke_pos_table[ position ].y,
									  poke_pos_table[ position ].z,
									  &maw );
	VEC_Set( &scale, 
			 poke_scale_table[ position ], 
			 poke_scale_table[ position ],
			 FX32_ONE );

	MCSS_SetScale( pmw->mcss[ position ], &scale );
}

//============================================================================================
/**
 *	POKE_MCSS削除
 *
 * @param[in]	pmw			POKE_MCSS管理ワークへのポインタ
 * @param[in]	position	ポケモンの立ち位置
 */
//============================================================================================
void	POKE_MCSS_Del( POKE_MCSS_WORK *pmw, int position )
{
	MCSS_Del( pmw->mcss_sys, pmw->mcss[ position ] );
	pmw->mcss[ position ] = NULL;
}

//============================================================================================
/**
 *	メパチ処理
 *
 * @param[in]	pmw			POKE_MCSS管理ワークへのポインタ
 * @param[in]	position	メパチさせたいポケモンの立ち位置
 * @param[in]	flag		メパチフラグ（POKE_MCSS_MEPACHI_ON、POKE_MCSS_MEPACHI_OFF）
 */
//============================================================================================
void	POKE_MCSS_SetMepachiFlag( POKE_MCSS_WORK *pmw, int position, int flag )
{
	GF_ASSERT( pmw->mcss[ position ] != NULL );
	if( flag == POKE_MCSS_MEPACHI_ON ){
		MCSS_SetMepachiFlag( pmw->mcss[ position ] );
	}
	else{
		MCSS_ResetMepachiFlag( pmw->mcss[ position ] );
	}
}

//============================================================================================
/**
 *	アニメストップ処理
 *
 * @param[in]	pmw			POKE_MCSS管理ワークへのポインタ
 * @param[in]	position	アニメストップさせたいポケモンの立ち位置
 * @param[in]	flag		アニメストップフラグ（POKE_MCSS_ANM_STOP_ON、POKE_MCSS_ANM_STOP_OFF）
 */
//============================================================================================
void	POKE_MCSS_SetAnmStopFlag( POKE_MCSS_WORK *pmw, int position, int flag )
{
	GF_ASSERT( pmw->mcss[ position ] != NULL );
	if( flag == POKE_MCSS_ANM_STOP_ON ){
		MCSS_SetAnmStopFlag( pmw->mcss[ position ] );
	}
	else{
		MCSS_ResetAnmStopFlag( pmw->mcss[ position ] );
	}
}

//============================================================================================
/**
 *	ポケモンの初期立ち位置を取得
 *
 * @param[out]	pos			初期立ち位置を格納するワークへのポインタ
 * @param[in]	position	取得するポケモンの立ち位置
 */
//============================================================================================
void	POKE_MCSS_GetPokeDefaultPos( VecFx32 *pos, int position )
{
	pos->x = poke_pos_table[ position ].x;
	pos->y = poke_pos_table[ position ].y;
	pos->z = poke_pos_table[ position ].z;
}

//============================================================================================
/**
 *	ポケモンの初期拡縮率を取得
 *
 * @param[in]	position	取得するポケモンの立ち位置
 */
//============================================================================================
fx32	POKE_MCSS_GetPokeDefaultScale( int position )
{
	return poke_scale_table[ position ];
}

//============================================================================================
/**
 *	ポケモンのスケール値を取得
 *
 * @param[in]	pmw			POKE_MCSS管理ワークへのポインタ
 * @param[in]	position	取得するポケモンの立ち位置
 * @param[out]	scale		取得したスケール値を格納するワークへのポインタ
 */
//============================================================================================
void	POKE_MCSS_GetScale( POKE_MCSS_WORK *pmw, int position, VecFx32 *scale )
{
	MCSS_GetScale( pmw->mcss[ position ], scale );
}

//============================================================================================
/**
 *	ポケモンのスケール値を格納
 *
 * @param[in]	pmw			POKE_MCSS管理ワークへのポインタ
 * @param[in]	position	格納するポケモンの立ち位置
 * @param[in]	scale		格納するスケール値
 */
//============================================================================================
void	POKE_MCSS_SetScale( POKE_MCSS_WORK *pmw, int position, VecFx32 *scale )
{
	MCSS_SetScale( pmw->mcss[ position ], scale );
}

//============================================================================================
/**
 *	ポケモン移動
 *
 * @param[in]	pmw			POKE_MCSS管理ワークへのポインタ
 * @param[in]	position	移動するポケモンの立ち位置
 * @param[in]	move_type	移動タイプ
 * @param[in]	pos			移動タイプにより意味が変化
 *							EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION	移動先
 *							EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in]	speed		移動スピード
 * @param[in]	count		往復カウント（POKEMCSS_CALCYUPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void	POKE_MCSS_MovePosition( POKE_MCSS_WORK *pmw, int position, int move_type, VecFx32 *pos, int speed, int wait, int count )
{
	POKE_MCSS_TCB_WORK	*pmtw = GFL_HEAP_AllocMemory( pmw->heapID, sizeof( POKE_MCSS_TCB_WORK ) );

	pmtw->pmw				= pmw;
	pmtw->emw.position		= position;
	pmtw->emw.move_type		= move_type;
	pmtw->emw.vec_time		= speed;
	pmtw->emw.vec_time_tmp	= speed;
	pmtw->emw.wait			= 0;
	pmtw->emw.wait_tmp		= wait;
	pmtw->emw.count			= count;
	pmtw->emw.end_value.x	= pos->x;
	pmtw->emw.end_value.y	= pos->y;
	pmtw->emw.end_value.z	= pos->z;

	MCSS_GetPosition( pmw->mcss[ position ], &pmtw->emw.start_value );

	switch( move_type ){
	case EFFTOOL_CALCTYPE_DIRECT:			//直接ポジションに移動
		break;
	case EFFTOOL_CALCTYPE_INTERPOLATION:	//移動先までを補間しながら移動
		BTL_EFFTOOL_CalcMoveVector( &pmtw->emw.start_value, pos, &pmtw->emw.vector, FX32_CONST( speed ) );
		break;
	case EFFTOOL_CALCTYPE_ROUNDTRIP_LONG:	//指定した区間を往復移動
		pmtw->emw.vec_time_tmp	*= 2;
	case EFFTOOL_CALCTYPE_ROUNDTRIP:		//指定した区間を往復移動
		pmtw->emw.vector.x = FX_Div( pos->x, FX32_CONST( speed ) );
		pmtw->emw.vector.y = FX_Div( pos->y, FX32_CONST( speed ) );
		pmtw->emw.vector.z = FX_Div( pos->z, FX32_CONST( speed ) );
		break;
	}

	GFL_TCB_AddTask( pmw->tcb_sys, TCB_POKE_MCSS_Move, pmtw, 0 );
}

//============================================================================================
/**
 *	ポケモン拡縮
 *
 * @param[in]	pmw			POKE_MCSS管理ワークへのポインタ
 * @param[in]	position	拡縮するポケモンの立ち位置
 * @param[in]	scale_type	拡縮タイプ
 * @param[in]	scale		拡縮タイプにより意味が変化
 *							EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION	最終的なスケール値
 *							EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in]	speed		拡縮スピード
 * @param[in]	wait		拡縮ウエイト
 * @param[in]	count		往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void	POKE_MCSS_MoveScale( POKE_MCSS_WORK *pmw, int position, int move_type, VecFx32 *scale, int speed, int wait, int count )
{
	POKE_MCSS_TCB_WORK	*pmtw = GFL_HEAP_AllocMemory( pmw->heapID, sizeof( POKE_MCSS_TCB_WORK ) );

	pmtw->pmw				= pmw;
	pmtw->emw.position		= position;
	pmtw->emw.move_type		= move_type;
	pmtw->emw.vec_time		= speed;
	pmtw->emw.vec_time_tmp	= speed;
	pmtw->emw.wait			= 0;
	pmtw->emw.wait_tmp		= wait;
	pmtw->emw.count			= count;
	pmtw->emw.end_value.x	= scale->x;
	pmtw->emw.end_value.y	= scale->y;
	pmtw->emw.end_value.z	= scale->z;

	MCSS_GetScale( pmw->mcss[ position ], &pmtw->emw.start_value );

	switch( move_type ){
	case EFFTOOL_CALCTYPE_DIRECT:			//直接拡縮
		break;
	case EFFTOOL_CALCTYPE_INTERPOLATION:	//最終的なスケール値までを補間しながら拡縮
		BTL_EFFTOOL_CalcMoveVector( &pmtw->emw.start_value, scale, &pmtw->emw.vector, FX32_CONST( speed ) );
		break;
	case EFFTOOL_CALCTYPE_ROUNDTRIP_LONG:	//指定した区間を往復拡縮
		pmtw->emw.vec_time_tmp	*= 2;
	case EFFTOOL_CALCTYPE_ROUNDTRIP:		//指定した区間を往復拡縮
		pmtw->emw.vector.x = FX_Div( scale->x, FX32_CONST( speed ) );
		pmtw->emw.vector.y = FX_Div( scale->y, FX32_CONST( speed ) );
		pmtw->emw.vector.z = FX_Div( scale->z, FX32_CONST( speed ) );
		break;
	}

	GFL_TCB_AddTask( pmw->tcb_sys, TCB_POKE_MCSS_Scale, pmtw, 0 );
}

//============================================================================================
/**
 *	POKEMON_PARAMからMCSS_ADD_WORKを生成する
 *
 * @param[in]	pp			POKEMON_PARAM構造体へのポインタ
 * @param[out]	maw			MCSS_ADD_WORKワークへのポインタ
 * @param[in]	position	ポケモンの立ち位置
 */
//============================================================================================
static	void	POKE_MCSS_MakeMAW( const POKEMON_PARAM *pp, MCSS_ADD_WORK *maw, int position )
{
	int	mons_no = PP_Get( pp, ID_PARA_monsno,	NULL ) - 1;
	int	form_no = PP_Get( pp, ID_PARA_form_no, NULL );
	int	sex		= PP_Get( pp, ID_PARA_sex,		NULL );
	int	rare	= PP_CheckRare( pp );

	int	file_start = POKEGRA_FILE_MAX * mons_no;						//ポケモンナンバーからファイルのオフセットを計算
	int	file_offset = ( ( position & 1 ) ) ? 0 : POKEGRA_BACK_M_NCGR;	//向きの計算

	//本来は別フォルム処理を入れる
#warning Another Form Nothing

	//性別のチェック
	switch( sex ){
	case PTL_SEX_MALE:
		break;
	case PTL_SEX_FEMALE:
		//オスメス書き分けしているかチェックする（サイズが０なら書き分けなし）
		sex = ( GFL_ARC_GetDataSize( ARCID_POKEGRA, file_start + file_offset + 1 ) == 0 ) ? PTL_SEX_MALE : PTL_SEX_FEMALE;
		break;
	case PTL_SEX_UNKNOWN:
		//性別なしは、オス扱いにする
		sex = PTL_SEX_MALE;
		break;
	default:
		//ありえない性別
		GF_ASSERT(0);
		break;
	}

	maw->arcID = ARCID_POKEGRA;
	maw->ncbr = file_start + file_offset + POKEGRA_M_NCBR + sex;
	maw->nclr = file_start + POKEGRA_NORMAL_NCLR + rare;
	maw->ncer = file_start + file_offset + POKEGRA_NCER;
	maw->nanr = file_start + file_offset + POKEGRA_NANR;
	maw->nmcr = file_start + file_offset + POKEGRA_NMCR;
	maw->nmar = file_start + file_offset + POKEGRA_NMAR;
	maw->ncec = file_start + file_offset + POKEGRA_NCEC;
}

//============================================================================================
/**
 *	影リソース読み込み
 */
//============================================================================================
static	void	POKE_MCSS_LoadShadowResource( POKE_MCSS_WORK *pmw )
{
	NNS_G2dInitImageProxy( &pmw->image_proxy );
//	NNS_G2dInitImagePaletteProxy( &pmw->palette_proxy );
	NNS_G2dInitImagePaletteProxy( &shadow_palette );
    //
    // VRAM 関連の初期化
    //
    {
		TCB_LOADRESOURCE_WORK *tlw = GFL_HEAP_AllocClearMemory( pmw->heapID, sizeof( TCB_LOADRESOURCE_WORK ) );
		tlw->chr_ofs = 0x18000;
		tlw->pal_ofs = 0x200;
		tlw->pmw	 = pmw;
		// load character data for 3D (software sprite)
		{
			tlw->pBufChar = GFL_ARC_UTIL_LoadBGCharacter( ARCID_BATTGRA, NARC_battgra_wb_shadow_NCBR, FALSE, &tlw->pCharData, pmw->heapID );
			GF_ASSERT( tlw->pBufChar != NULL);
        }

		// load palette data
		{
			tlw->pBufPltt = GFL_ARC_UTIL_LoadPalette( ARCID_BATTGRA, NARC_battgra_wb_shadow_NCLR, &tlw->pPlttData, pmw->heapID );
			GF_ASSERT( tlw->pBufPltt != NULL);

        }
		GFUser_VIntr_CreateTCB( TCB_LoadResource, tlw, 0 );
    }
}

//--------------------------------------------------------------------------
/**
 * リソースをVRAMに転送
 */
//--------------------------------------------------------------------------
static	void	TCB_LoadResource( GFL_TCB *tcb, void *work )
{
	TCB_LOADRESOURCE_WORK *tlw = ( TCB_LOADRESOURCE_WORK *)work;

	// Loading For 3D Graphics Engine.（本来は、VRAMマネージャを使用したい）
	NNS_G2dLoadImage2DMapping(
		tlw->pCharData,
		tlw->chr_ofs,
		NNS_G2D_VRAM_TYPE_3DMAIN,
		&tlw->pmw->image_proxy );

	GFL_HEAP_FreeMemory( tlw->pBufChar );

	// Loading For 3D Graphics Engine.
	NNS_G2dLoadPalette(
		tlw->pPlttData,
		tlw->pal_ofs,
		NNS_G2D_VRAM_TYPE_3DMAIN,
//		&tlw->pmw->palette_proxy );
		&shadow_palette );

	GFL_HEAP_FreeMemory( tlw->pBufPltt );

	GFL_HEAP_FreeMemory( work );
	GFL_TCB_DeleteTask( tcb );
}

//============================================================================================
/**
 *	ポケモン移動タスク
 */
//============================================================================================
static	void	TCB_POKE_MCSS_Move( GFL_TCB *tcb, void *work )
{
	POKE_MCSS_TCB_WORK	*pmtw = ( POKE_MCSS_TCB_WORK * )work;
	POKE_MCSS_WORK *pmw = pmtw->pmw;
	VecFx32	now_pos;
	BOOL	ret;

	MCSS_GetPosition( pmw->mcss[ pmtw->emw.position ], &now_pos );
	ret = BTL_EFFTOOL_CalcParam( &pmtw->emw, &now_pos );
	MCSS_SetPosition( pmw->mcss[ pmtw->emw.position ], &now_pos );
	if( ret == TRUE ){
		GFL_HEAP_FreeMemory( work );
		GFL_TCB_DeleteTask( tcb );
	}
}

//============================================================================================
/**
 *	ポケモン拡縮タスク
 */
//============================================================================================
static	void	TCB_POKE_MCSS_Scale( GFL_TCB *tcb, void *work )
{
	POKE_MCSS_TCB_WORK	*pmtw = ( POKE_MCSS_TCB_WORK * )work;
	POKE_MCSS_WORK *pmw = pmtw->pmw;
	VecFx32	now_scale;
	BOOL	ret;

	MCSS_GetScale( pmw->mcss[ pmtw->emw.position ], &now_scale );
	ret = BTL_EFFTOOL_CalcParam( &pmtw->emw, &now_scale );
	MCSS_SetScale( pmw->mcss[ pmtw->emw.position ], &now_scale );
	if( ret == TRUE ){
		GFL_HEAP_FreeMemory( work );
		GFL_TCB_DeleteTask( tcb );
	}
}

//============================================================================================
/**
 *	ポケモンの影描画
 */
//============================================================================================
static	void	POKE_MCSS_DrawShadow( POKE_MCSS_WORK *pmw )
{
    NNSG2dImageProxy		*image_p	= &pmw->image_proxy;
    NNSG2dImagePaletteProxy	*palette_p	= &pmw->palette_proxy;
	int	i;

	G3_PushMtx();

	G3_MtxMode( GX_MTXMODE_TEXTURE );
	G3_Identity();
	G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );

	G3_LookAt( NNS_G3dGlbGetCameraPos(), NNS_G3dGlbGetCameraUp(), NNS_G3dGlbGetCameraTarget(), NULL );

	// for software sprite-setting
	{
		G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),		// diffuse
								GX_RGB(16, 16, 16),		// ambient
								TRUE					// use diffuse as vtx color if TRUE
								);

		G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),		// specular
								GX_RGB( 0,  0,  0),		// emission
                                FALSE					// use shininess table if TRUE
                                );

		G3_PolygonAttr(GX_LIGHTMASK_NONE,				// no lights
					   GX_POLYGONMODE_MODULATE,			// modulation mode
					   GX_CULL_NONE,					// cull back
					   0,								// polygon ID(0 - 63)
					   31,								// alpha(0 - 31)
					   0								// OR of GXPolygonAttrMisc's value
					   );
	}
	G3_TexImageParam( image_p->attr.fmt,
					  GX_TEXGEN_TEXCOORD,
					  image_p->attr.sizeS,
					  image_p->attr.sizeT,
					  GX_TEXREPEAT_ST,
					  GX_TEXFLIP_NONE,
					  image_p->attr.plttUse,
					  image_p->vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_3DMAIN]);
	G3_TexPlttBase( palette_p->vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_3DMAIN],
				    palette_p->fmt);

	for( i = 0 ; i < POKE_MCSS_MAX ; i++ ){
		if( pmw->mcss[ i ] == NULL ) continue;

		G3_PushMtx();

		G3_Translate( poke_pos_table[ i ].x - POKE_MCSS_DEFAULT_LINE * 40, 
					  poke_pos_table[ i ].y,
					  poke_pos_table[ i ].z + POKE_MCSS_DEFAULT_LINE * 20 );
		G3_Scale( FX32_ONE * 80, FX32_ONE, FX32_ONE * 40 );

		G3_Begin(GX_BEGIN_QUADS);
		G3_TexCoord( 0, 0 );
		G3_Vtx( 0, 0, 0 );
		G3_TexCoord( FX32_ONE * 80,	0 );
		G3_Vtx( POKE_MCSS_DEFAULT_LINE, 0, 0 );
		G3_TexCoord( FX32_ONE * 80,	FX32_ONE * 40 );
		G3_Vtx( POKE_MCSS_DEFAULT_LINE, 0, -POKE_MCSS_DEFAULT_LINE );
		G3_TexCoord( 0,	FX32_ONE * 40 );
		G3_Vtx( 0, 0, -POKE_MCSS_DEFAULT_LINE );
		G3_End();

		G3_PopMtx(1);
	}

	G3_PopMtx(1);
}


