
//============================================================================================
/**
 * @file	btlv_mcss.c
 * @brief	戦闘MCSS管理
 * @author	soga
 * @date	2008.11.14
 */
//============================================================================================

#include <gflib.h>

#include "system/gfl_use.h"
#include "system/mcss_tool.h"

#include "btlv_effect.h"

#include "battgra/battgra_wb.naix"

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================

#define	BTLV_MCSS_MAX	( 6 )	//BTLV_MCSSで管理するMCSSのMAX値

#define	BTLV_MCSS_DEFAULT_SHIFT	( FX32_SHIFT - 4 )					//ポリゴン１辺の基準の長さにするシフト値
#define	BTLV_MCSS_DEFAULT_LINE	( 1 << BTLV_MCSS_DEFAULT_SHIFT )	//ポリゴン１辺の基準の長さ

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

struct _BTLV_MCSS_WORK
{
	GFL_TCBSYS				*tcb_sys;
	MCSS_SYS_WORK			*mcss_sys;
	MCSS_WORK				*mcss[ BTLV_MCSS_POS_MAX ];

	u8						poke_mcss_ortho_mode			:1;
	u8														:7;
	u8						poke_mcss_tcb_move_execute;
	u8						poke_mcss_tcb_scale_execute;
	u8						poke_mcss_tcb_rotate_execute;

	u8						poke_mcss_tcb_blink_execute;
	u8						dummy;								//padding
	HEAPID					heapID;
};

typedef struct
{
	BTLV_MCSS_WORK		*bmw;
	int					position;
	EFFTOOL_MOVE_WORK	emw;
}BTLV_MCSS_TCB_WORK;

typedef struct	
{
	NNSG2dCharacterData*	pCharData;			//テクスチャキャラ
	NNSG2dPaletteData*		pPlttData;			//テクスチャパレット
	void					*pBufChar;			//テクスチャキャラバッファ
	void					*pBufPltt;			//テクスチャパレットバッファ
	int						chr_ofs;
	int						pal_ofs;
	BTLV_MCSS_WORK			*bmw;
}TCB_LOADRESOURCE_WORK;

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

BTLV_MCSS_WORK	*BTLV_MCSS_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID );
void			BTLV_MCSS_Exit( BTLV_MCSS_WORK *bmw );
void			BTLV_MCSS_Main( BTLV_MCSS_WORK *bmw );
void			BTLV_MCSS_Draw( BTLV_MCSS_WORK *bmw );
void			BTLV_MCSS_Add( BTLV_MCSS_WORK *bmw, const POKEMON_PARAM *pp, int position );
void			BTLV_MCSS_Del( BTLV_MCSS_WORK *bmw, int position );
void			BTLV_MCSS_SetOrthoMode( BTLV_MCSS_WORK *bmw );
void			BTLV_MCSS_ResetOrthoMode( BTLV_MCSS_WORK *bmw );
void			BTLV_MCSS_SetMepachiFlag( BTLV_MCSS_WORK *bmw, int position, int flag );
void			BTLV_MCSS_SetAnmStopFlag( BTLV_MCSS_WORK *bmw, int position, int flag );
int				BTLV_MCSS_GetVanishFlag( BTLV_MCSS_WORK *bmw, int position );
void			BTLV_MCSS_SetVanishFlag( BTLV_MCSS_WORK *bmw, int position, int flag );
void			BTLV_MCSS_GetPokeDefaultPos( VecFx32 *pos, int position );
fx32			BTLV_MCSS_GetPokeDefaultScale( BTLV_MCSS_WORK *bmw, int position );
void			BTLV_MCSS_GetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale );
void			BTLV_MCSS_SetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale );

void			BTLV_MCSS_MovePosition( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *pos, int frame, int wait, int count );
void			BTLV_MCSS_MoveScale( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *scale, int frame, int wait, int count );
void			BTLV_MCSS_MoveRotate( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *rotate, int frame, int wait, int count );
void			BTLV_MCSS_MoveBlink( BTLV_MCSS_WORK *bmw, int position, int type, int wait, int count );
BOOL			BTLV_MCSS_CheckTCBExecute( BTLV_MCSS_WORK *bmw, int position );
BOOL			BTLV_MCSS_CheckExistPokemon( BTLV_MCSS_WORK *bmw, int position );

static	void	BTLV_MCSS_MakeMAW( const POKEMON_PARAM *pp, MCSS_ADD_WORK *maw, int position );
static	void	BTLV_MCSS_SetDefaultScale( BTLV_MCSS_WORK *bmw, int position );

static	void	BTLV_MCSS_TCBInitialize( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *start, VecFx32 *end, int frame, int wait, int count, GFL_TCB_FUNC *func );
static	void	TCB_BTLV_MCSS_Move( GFL_TCB *tcb, void *work );
static	void	TCB_BTLV_MCSS_Scale( GFL_TCB *tcb, void *work );
static	void	TCB_BTLV_MCSS_Rotate( GFL_TCB *tcb, void *work );
static	void	TCB_BTLV_MCSS_Blink( GFL_TCB *tcb, void *work );

#ifdef PM_DEBUG
void	BTLV_MCSS_AddDebug( BTLV_MCSS_WORK *bmw, const MCSS_ADD_DEBUG_WORK *madw, int position );
#endif

//============================================================================================
/**
 *	ポケモンの立ち位置テーブル
 */
//============================================================================================
/*
static	const	VecFx32	poke_pos_table[]={
	{ FX_F32_TO_FX32( -2.5f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   8.0f ) },		//POS_AA
	{ FX_F32_TO_FX32(  4.5f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },		//POS_BB
	{ FX_F32_TO_FX32( -3.5f ),	FX_F32_TO_FX32( 1.2f ), FX_F32_TO_FX32(   8.5f ) },		//POS_A
	{ FX_F32_TO_FX32(  6.0f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(  -9.0f ) },		//POS_B
	{ FX_F32_TO_FX32( -0.5f ),	FX_F32_TO_FX32( 1.2f ), FX_F32_TO_FX32(   9.0f ) },		//POS_C
	{ FX_F32_TO_FX32(  2.0f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -11.0f ) },		//POS_D
	{ FX_F32_TO_FX32( -2.5f ),	FX_F32_TO_FX32( 1.2f ), FX_F32_TO_FX32(  10.0f ) },		//POS_E
	{ FX_F32_TO_FX32(  4.5f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },		//POS_F
};
*/


static	const	VecFx32	poke_pos_table[]={
	{ FX_F32_TO_FX32( -2.5f + 3.000f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   7.5f - 0.5f ) },		//POS_AA
	{ FX_F32_TO_FX32(  4.5f - 4.200f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },		//POS_BB
	{ FX_F32_TO_FX32( -3.5f + 3.500f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   8.5f ) },		//POS_A
	{ FX_F32_TO_FX32(  6.0f - 4.200f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(  -9.0f ) },		//POS_B
	{ FX_F32_TO_FX32( -0.5f + 3.845f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   9.0f ) },		//POS_C
	{ FX_F32_TO_FX32(  2.0f - 4.964f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -11.0f ) },		//POS_D
	{ FX_F32_TO_FX32( -2.5f + 3.845f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(  10.0f ) },		//POS_E
	{ FX_F32_TO_FX32(  4.5f - 4.964f ),	FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },		//POS_F
};

//============================================================================================
/**
 *	ポケモンの立ち位置によるスケール補正テーブル
 */
//============================================================================================
static	const	fx32	poke_scale_table[ 2 ][ BTLV_MCSS_POS_MAX ]={
	{
		0x0780 * 2,	//POS_AA
		0x1280,		//POS_BB
		0x0873 * 2,	//POS_A
		0x1322,		//POS_B
		0x0831 * 2,	//POS_C
		0x141e,		//POS_D
		0x1000 * 2,	//POS_E
		0x1000,		//POS_F
	},
	{
		FX32_ONE * 16 * 2,
		FX32_ONE * 16,
		FX32_ONE * 16 * 2,
		FX32_ONE * 16,
		FX32_ONE * 16 * 2,
		FX32_ONE * 16,
		FX32_ONE * 16 * 2,
		FX32_ONE * 16,
	}
};

//============================================================================================
/**
 *	システム初期化
 *
 * @param[in]	tcb_sys	システム内で使用するTCBSYS構造体へのポインタ
 * @param[in]	heapID	ヒープID
 */
//============================================================================================
BTLV_MCSS_WORK	*BTLV_MCSS_Init( GFL_TCBSYS	*tcb_sys, HEAPID heapID )
{
	BTLV_MCSS_WORK *bmw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_MCSS_WORK ) );

	bmw->mcss_sys = MCSS_Init( BTLV_MCSS_MAX, heapID );
	bmw->tcb_sys  = tcb_sys;

	return bmw;
}

//============================================================================================
/**
 *	システム終了
 *
 * @param[in]	bmw	BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_MCSS_Exit( BTLV_MCSS_WORK *bmw )
{
	MCSS_Exit( bmw->mcss_sys );
	GFL_HEAP_FreeMemory( bmw );
}

//============================================================================================
/**
 *	システムメイン
 *
 * @param[in]	bmw	BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_MCSS_Main( BTLV_MCSS_WORK *bmw )
{
	MCSS_Main( bmw->mcss_sys );
}

//============================================================================================
/**
 *	描画
 *
 * @param[in]	bmw	BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_MCSS_Draw( BTLV_MCSS_WORK *bmw )
{
	MCSS_Draw( bmw->mcss_sys );
}

//============================================================================================
/**
 *	BTLV_MCSS追加
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	pp			POKEMON_PARAM構造体へのポインタ
 * @param[in]	position	ポケモンの立ち位置
 */
//============================================================================================
void	BTLV_MCSS_Add( BTLV_MCSS_WORK *bmw, const POKEMON_PARAM *pp, int position )
{
	MCSS_ADD_WORK	maw;

	GF_ASSERT( position < BTLV_MCSS_POS_MAX );
	GF_ASSERT( bmw->mcss[ position ] == NULL );

	BTLV_MCSS_MakeMAW( pp, &maw, position );
	bmw->mcss[ position ] = MCSS_Add( bmw->mcss_sys,
									  poke_pos_table[ position ].x,
									  poke_pos_table[ position ].y,
									  poke_pos_table[ position ].z,
									  &maw );

	BTLV_MCSS_SetDefaultScale( bmw, position );
}

//============================================================================================
/**
 *	BTLV_MCSS削除
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	ポケモンの立ち位置
 */
//============================================================================================
void	BTLV_MCSS_Del( BTLV_MCSS_WORK *bmw, int position )
{
	MCSS_Del( bmw->mcss_sys, bmw->mcss[ position ] );
	bmw->mcss[ position ] = NULL;
}

//============================================================================================
/**
 *	正射影描画モードON
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_MCSS_SetOrthoMode( BTLV_MCSS_WORK *bmw )
{
	int	position;

	MCSS_SetOrthoMode( bmw->mcss_sys );

	bmw->poke_mcss_ortho_mode = 1;

	for( position = 0 ; position < BTLV_MCSS_POS_MAX ; position++ ){
		if( bmw->mcss[ position ] ){
			BTLV_MCSS_SetDefaultScale( bmw, position );
		}
	}
}

//============================================================================================
/**
 *	正射影描画モードOFF
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_MCSS_ResetOrthoMode( BTLV_MCSS_WORK *bmw )
{
	int	position;

	MCSS_ResetOrthoMode( bmw->mcss_sys );

	bmw->poke_mcss_ortho_mode = 0;

	for( position = 0 ; position < BTLV_MCSS_POS_MAX ; position++ ){
		if( bmw->mcss[ position ] ){
			BTLV_MCSS_SetDefaultScale( bmw, position );
		}
	}
}

//============================================================================================
/**
 *	メパチ処理
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	メパチさせたいポケモンの立ち位置
 * @param[in]	flag		メパチフラグ（BTLV_MCSS_MEPACHI_ON、BTLV_MCSS_MEPACHI_OFF）
 */
//============================================================================================
void	BTLV_MCSS_SetMepachiFlag( BTLV_MCSS_WORK *bmw, int position, int flag )
{
	GF_ASSERT( bmw->mcss[ position ] != NULL );
	if( flag == BTLV_MCSS_MEPACHI_FLIP ){
		MCSS_FlipMepachiFlag( bmw->mcss[ position ] );
	}
	else if( flag == BTLV_MCSS_MEPACHI_ON ){
		MCSS_SetMepachiFlag( bmw->mcss[ position ] );
	}
	else{
		MCSS_ResetMepachiFlag( bmw->mcss[ position ] );
	}
}

//============================================================================================
/**
 *	アニメストップ処理
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	アニメストップさせたいポケモンの立ち位置
 * @param[in]	flag		アニメストップフラグ（BTLV_MCSS_ANM_STOP_ON、BTLV_MCSS_ANM_STOP_OFF）
 */
//============================================================================================
void	BTLV_MCSS_SetAnmStopFlag( BTLV_MCSS_WORK *bmw, int position, int flag )
{
	GF_ASSERT( bmw->mcss[ position ] != NULL );
	if( flag == BTLV_MCSS_ANM_STOP_ON ){
		MCSS_SetAnmStopFlag( bmw->mcss[ position ] );
	}
	else{
		MCSS_ResetAnmStopFlag( bmw->mcss[ position ] );
	}
}

//============================================================================================
/**
 *	バニッシュフラグ取得
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	フラグ操作させたいポケモンの立ち位置
 */
//============================================================================================
int		BTLV_MCSS_GetVanishFlag( BTLV_MCSS_WORK *bmw, int position )
{
	GF_ASSERT( bmw->mcss[ position ] != NULL );
	return MCSS_GetVanishFlag( bmw->mcss[ position ] );
}

//============================================================================================
/**
 *	バニッシュ処理
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	フラグ操作させたいポケモンの立ち位置
 * @param[in]	flag		バニッシュフラグ（BTLV_MCSS_VANISH_ON、BTLV_MCSS_VANISH_OFF）
 */
//============================================================================================
void	BTLV_MCSS_SetVanishFlag( BTLV_MCSS_WORK *bmw, int position, int flag )
{
	GF_ASSERT( bmw->mcss[ position ] != NULL );
	if( flag == BTLV_MCSS_VANISH_ON ){
		MCSS_SetVanishFlag( bmw->mcss[ position ] );
	}
	else{
		MCSS_ResetVanishFlag( bmw->mcss[ position ] );
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
void	BTLV_MCSS_GetPokeDefaultPos( VecFx32 *pos, int position )
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
fx32	BTLV_MCSS_GetPokeDefaultScale( BTLV_MCSS_WORK *bmw, int position )
{
	return poke_scale_table[ bmw->poke_mcss_ortho_mode ][ position ];
}

//============================================================================================
/**
 *	ポケモンのスケール値を取得
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	取得するポケモンの立ち位置
 * @param[out]	scale		取得したスケール値を格納するワークへのポインタ
 */
//============================================================================================
void	BTLV_MCSS_GetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale )
{
	MCSS_GetScale( bmw->mcss[ position ], scale );
}

//============================================================================================
/**
 *	ポケモンのスケール値を格納
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	格納するポケモンの立ち位置
 * @param[in]	scale		格納するスケール値
 */
//============================================================================================
void	BTLV_MCSS_SetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale )
{
	MCSS_SetScale( bmw->mcss[ position ], scale );
}

//============================================================================================
/**
 *	ポケモン移動
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	移動するポケモンの立ち位置
 * @param[in]	type		移動タイプ
 * @param[in]	pos			移動タイプにより意味が変化
 *							EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION	移動先
 *							EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in]	frame		移動フレーム数（目的地まで何フレームで到達するか）
 * @param[in]	wait		移動ウエイト
 * @param[in]	count		往復カウント（POKEMCSS_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void	BTLV_MCSS_MovePosition( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *pos, int frame, int wait, int count )
{
	VecFx32	start;

	MCSS_GetPosition( bmw->mcss[ position ], &start );
	BTLV_MCSS_TCBInitialize( bmw, position, type, &start, pos, frame, wait, count, TCB_BTLV_MCSS_Move );
	bmw->poke_mcss_tcb_move_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 *	ポケモン拡縮
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	拡縮するポケモンの立ち位置
 * @param[in]	type		拡縮タイプ
 * @param[in]	scale		拡縮タイプにより意味が変化
 *							EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION	最終的なスケール値
 *							EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in]	frame		拡縮フレーム数（設定した拡縮値まで何フレームで到達するか）
 * @param[in]	wait		拡縮ウエイト
 * @param[in]	count		往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void	BTLV_MCSS_MoveScale( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *scale, int frame, int wait, int count )
{
	VecFx32	start;

	MCSS_GetScale( bmw->mcss[ position ], &start );
	BTLV_MCSS_TCBInitialize( bmw, position, type, &start, scale, frame, wait, count, TCB_BTLV_MCSS_Scale );
	bmw->poke_mcss_tcb_scale_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 *	ポケモン回転
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	回転するポケモンの立ち位置
 * @param[in]	type		回転タイプ
 * @param[in]	rotate		回転タイプにより意味が変化
 *							EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION	最終的な回転値
 *							EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in]	frame		回転フレーム数（設定した回転値まで何フレームで到達するか）
 * @param[in]	wait		回転ウエイト
 * @param[in]	count		往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void	BTLV_MCSS_MoveRotate( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *rotate, int frame, int wait, int count )
{
	VecFx32	start;

	MCSS_GetRotate( bmw->mcss[ position ], &start );
	BTLV_MCSS_TCBInitialize( bmw, position, type, &start, rotate, frame, wait, count, TCB_BTLV_MCSS_Rotate );
	bmw->poke_mcss_tcb_rotate_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 *	ポケモンまばたき
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	まばたきるポケモンの立ち位置
 * @param[in]	type		まばたきイプ
 * @param[in]	wait		まばたきウエイト
 * @param[in]	count		まばたきカウント
 */
//============================================================================================
void	BTLV_MCSS_MoveBlink( BTLV_MCSS_WORK *bmw, int position, int type, int wait, int count )
{
	BTLV_MCSS_TCB_WORK	*pmtw = GFL_HEAP_AllocMemory( bmw->heapID, sizeof( BTLV_MCSS_TCB_WORK ) );

	pmtw->bmw				= bmw;
	pmtw->position			= position;
	pmtw->emw.move_type		= type;
	pmtw->emw.wait			= 0;
	pmtw->emw.wait_tmp		= wait;
	pmtw->emw.count			= count * 2;	//閉じて開くを1回とカウントするために倍しておく

	switch( type ){
	case BTLEFF_MEPACHI_ON:
		BTLV_MCSS_SetMepachiFlag( bmw, position, BTLV_MCSS_MEPACHI_ON );
		break;
	case BTLEFF_MEPACHI_OFF:
		BTLV_MCSS_SetMepachiFlag( bmw, position, BTLV_MCSS_MEPACHI_OFF );
		break;
	case BTLEFF_MEPACHI_MABATAKI:
		GFL_TCB_AddTask( bmw->tcb_sys, TCB_BTLV_MCSS_Blink, pmtw, 0 );
		bmw->poke_mcss_tcb_blink_execute |= BTLV_EFFTOOL_Pos2Bit( position );
		break;
	}
}

//============================================================================================
/**
 *	タスクが起動中かチェック
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	ポケモンの立ち位置
 *
 * @retval: TRUE:起動中　FALSE:終了
 */
//============================================================================================
BOOL	BTLV_MCSS_CheckTCBExecute( BTLV_MCSS_WORK *bmw, int position )
{
	return ( ( bmw->poke_mcss_tcb_move_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
			 ( bmw->poke_mcss_tcb_scale_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
			 ( bmw->poke_mcss_tcb_rotate_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
			 ( bmw->poke_mcss_tcb_blink_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) );
}

//============================================================================================
/**
 *	指定された立ち位置のポケモンが存在するかチェック
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	position	ポケモンの立ち位置
 *
 * @retval	TRUE:存在する　FALSE:存在しない
 */
//============================================================================================
BOOL	BTLV_MCSS_CheckExistPokemon( BTLV_MCSS_WORK *bmw, int position )
{
	return (bmw->mcss[ position ] != NULL );
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
static	void	BTLV_MCSS_MakeMAW( const POKEMON_PARAM *pp, MCSS_ADD_WORK *maw, int position )
{
	int	dir = ( ( position & 1 ) ) ? MCSS_DIR_FRONT : MCSS_DIR_BACK;

	MCSS_TOOL_MakeMAWPP( pp, maw, dir );
}

//============================================================================================
/**
 *	ポケモンデフォルトスケールセット
 */
//============================================================================================
static	void	BTLV_MCSS_SetDefaultScale( BTLV_MCSS_WORK *bmw, int position )
{
	VecFx32			scale;

	GF_ASSERT( position < BTLV_MCSS_POS_MAX );
	GF_ASSERT( bmw->mcss[ position ] );

	VEC_Set( &scale, 
			 poke_scale_table[ bmw->poke_mcss_ortho_mode ][ position ], 
			 poke_scale_table[ bmw->poke_mcss_ortho_mode ][ position ],
			 FX32_ONE );

	MCSS_SetScale( bmw->mcss[ position ], &scale );

	VEC_Set( &scale, 
			 poke_scale_table[ 0 ][ position ], 
			 poke_scale_table[ 0 ][ position ] / 2,
			 FX32_ONE );

	MCSS_SetShadowScale( bmw->mcss[ position ], &scale );
}

//============================================================================================
/**
 *	ポケモン操作系タスク初期化処理
 */
//============================================================================================
static	void	BTLV_MCSS_TCBInitialize( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *start, VecFx32 *end, int frame, int wait, int count, GFL_TCB_FUNC *func )
{
	BTLV_MCSS_TCB_WORK	*pmtw = GFL_HEAP_AllocMemory( bmw->heapID, sizeof( BTLV_MCSS_TCB_WORK ) );

	pmtw->bmw				= bmw;
	pmtw->position			= position;
	pmtw->emw.move_type		= type;
	pmtw->emw.vec_time		= frame;
	pmtw->emw.vec_time_tmp	= frame;
	pmtw->emw.wait			= 0;
	pmtw->emw.wait_tmp		= wait;
	pmtw->emw.count			= count;
	pmtw->emw.start_value.x	= start->x;
	pmtw->emw.start_value.y	= start->y;
	pmtw->emw.start_value.z	= start->z;
	pmtw->emw.end_value.x	= end->x;
	pmtw->emw.end_value.y	= end->y;
	pmtw->emw.end_value.z	= end->z;

	switch( type ){
	case EFFTOOL_CALCTYPE_DIRECT:			//直接ポジションに移動
		break;
	case EFFTOOL_CALCTYPE_INTERPOLATION:	//移動先までを補間しながら移動
		BTLV_EFFTOOL_CalcMoveVector( &pmtw->emw.start_value, end, &pmtw->emw.vector, FX32_CONST( frame ) );
		break;
	case EFFTOOL_CALCTYPE_ROUNDTRIP_LONG:	//指定した区間を往復移動
		pmtw->emw.vec_time_tmp	*= 2;
	case EFFTOOL_CALCTYPE_ROUNDTRIP:		//指定した区間を往復移動
		pmtw->emw.vector.x = FX_Div( end->x, FX32_CONST( frame ) );
		pmtw->emw.vector.y = FX_Div( end->y, FX32_CONST( frame ) );
		pmtw->emw.vector.z = FX_Div( end->z, FX32_CONST( frame ) );
		break;
	}
	GFL_TCB_AddTask( bmw->tcb_sys, func, pmtw, 0 );
}

//============================================================================================
/**
 *	ポケモン移動タスク
 */
//============================================================================================
static	void	TCB_BTLV_MCSS_Move( GFL_TCB *tcb, void *work )
{
	BTLV_MCSS_TCB_WORK	*pmtw = ( BTLV_MCSS_TCB_WORK * )work;
	BTLV_MCSS_WORK *bmw = pmtw->bmw;
	VecFx32	now_pos;
	BOOL	ret;

	MCSS_GetPosition( bmw->mcss[ pmtw->position ], &now_pos );
	ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_pos );
	MCSS_SetPosition( bmw->mcss[ pmtw->position ], &now_pos );
	if( ret == TRUE ){
		bmw->poke_mcss_tcb_move_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ 0xff );
		GFL_HEAP_FreeMemory( work );
		GFL_TCB_DeleteTask( tcb );
	}
}

//============================================================================================
/**
 *	ポケモン拡縮タスク
 */
//============================================================================================
static	void	TCB_BTLV_MCSS_Scale( GFL_TCB *tcb, void *work )
{
	BTLV_MCSS_TCB_WORK	*pmtw = ( BTLV_MCSS_TCB_WORK * )work;
	BTLV_MCSS_WORK *bmw = pmtw->bmw;
	VecFx32	now_scale;
	BOOL	ret;

	MCSS_GetScale( bmw->mcss[ pmtw->position ], &now_scale );
	ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_scale );
	MCSS_SetScale( bmw->mcss[ pmtw->position ], &now_scale );
	if( ret == TRUE ){
		bmw->poke_mcss_tcb_scale_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ 0xff );
		GFL_HEAP_FreeMemory( work );
		GFL_TCB_DeleteTask( tcb );
	}
}

//============================================================================================
/**
 *	ポケモン回転タスク
 */
//============================================================================================
static	void	TCB_BTLV_MCSS_Rotate( GFL_TCB *tcb, void *work )
{
	BTLV_MCSS_TCB_WORK	*pmtw = ( BTLV_MCSS_TCB_WORK * )work;
	BTLV_MCSS_WORK *bmw = pmtw->bmw;
	VecFx32	now_rotate;
	BOOL	ret;

	MCSS_GetRotate( bmw->mcss[ pmtw->position ], &now_rotate );
	ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_rotate );
	MCSS_SetRotate( bmw->mcss[ pmtw->position ], &now_rotate );
	if( ret == TRUE ){
		bmw->poke_mcss_tcb_rotate_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ 0xff );
		GFL_HEAP_FreeMemory( work );
		GFL_TCB_DeleteTask( tcb );
	}
}

//============================================================================================
/**
 *	ポケモンまばたきタスク
 */
//============================================================================================
static	void	TCB_BTLV_MCSS_Blink( GFL_TCB *tcb, void *work )
{
	BTLV_MCSS_TCB_WORK	*pmtw = ( BTLV_MCSS_TCB_WORK * )work;
	BTLV_MCSS_WORK *bmw = pmtw->bmw;

	if( pmtw->emw.wait == 0 ){
		pmtw->emw.wait = pmtw->emw.wait_tmp;
		BTLV_MCSS_SetMepachiFlag( pmtw->bmw, pmtw->position, BTLV_MCSS_MEPACHI_FLIP );
		if( --pmtw->emw.count == 0 ){
			bmw->poke_mcss_tcb_blink_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ 0xff );
			GFL_HEAP_FreeMemory( work );
			GFL_TCB_DeleteTask( tcb );
		}
	}
	else{
		pmtw->emw.wait--;
	}
}

#ifdef PM_DEBUG
//============================================================================================
/**
 *	BTLV_MCSS追加（デバッグ用）
 *
 * @param[in]	bmw			BTLV_MCSS管理ワークへのポインタ
 * @param[in]	madw		MCSS_ADD_DEBUG_WORK構造体へのポインタ
 * @param[in]	position	ポケモンの立ち位置
 */
//============================================================================================
void	BTLV_MCSS_AddDebug( BTLV_MCSS_WORK *bmw, const MCSS_ADD_DEBUG_WORK *madw, int position )
{
	GF_ASSERT( position < BTLV_MCSS_POS_MAX );
	if( bmw->mcss[ position ] ){
		BTLV_MCSS_Del( bmw, position );
	}

	bmw->mcss[ position ] = MCSS_AddDebug( bmw->mcss_sys,
									  poke_pos_table[ position ].x,
									  poke_pos_table[ position ].y,
									  poke_pos_table[ position ].z,
									  madw );

	BTLV_MCSS_SetDefaultScale( bmw, position );
}
#endif
