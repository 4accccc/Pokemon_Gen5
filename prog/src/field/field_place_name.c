//////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file   field_place_name.h
 * @brief  地名表示ウィンドウ
 * @author obata_toshihiro
 * @date   2009.07   
 *
 */
//////////////////////////////////////////////////////////////////////////////////////
#include "field_place_name.h"
#include "field/zonedata.h"
#include "arc/arc_def.h"
#include "arc/place_name.naix"
#include "arc/message.naix"
#include "msg/msg_place_name.h"
#include "system/bmp_oam.h"


//===================================================================================
/**
 * @brief 定数・マクロ
 */
//=================================================================================== 
// 最大文字数
#define MAX_NAME_LENGTH (16)

// 絶対値に変換する
#define ABS( n ) ( ((n)<0)? -(n) : (n) )

// 1キャラ = 8ドット
#define CHAR_SIZE (8) 

// 無効ゾーンID
#define INVALID_ZONE_ID (0xffffffff)

//-----------
// 表示設定
//-----------
#define BG_PALETTE_NO       (0)					// BGパレット番号
#define BG_FRAME            (GFL_BG_FRAME3_M)	// 使用するBGフレーム
#define BG_FRAME_PRIORITY   (1)					// BGフレームのプライオリティ

#define OBJ_PALETTE_NO (1)                      // OBJパレット番号

#define	COLOR_NO_LETTER     (1)		            // 文字本体のカラー番号
#define	COLOR_NO_SHADOW     (2)		            // 影部分のカラー番号
#define	COLOR_NO_BACKGROUND (0)					// 背景部のカラー番号

#define ALPHA_PLANE_1 (GX_BLEND_PLANEMASK_BG3)	// αブレンドの第1対象面
#define ALPHA_PLANE_2 (GX_BLEND_PLANEMASK_BG0)	// αブレンドの第2対象面
#define ALPHA_VALUE_1 (16)						// 第1対象面のαブレンディング係数
#define ALPHA_VALUE_2 ( 4)						// 第1対象面のαブレンディング係数

#define Y_CENTER_POS ( CHAR_SIZE * 2 - 1 )		// 背景帯の中心y座標

#define LAUNCH_INTERVAL (3)	// 文字発射間隔[単位：フレーム]

//--------------------
// アーカイブ・データ
//--------------------
#define	ARC_DATA_ID_MAX (29) // アーカイブ内データIDの最大値

//-----
// BG
//-----
#define NULL_CHAR_NO       ( 0)								// NULLキャラ番号
#define	BMPWIN_WIDTH_CHAR  (32)								// ウィンドウ幅(キャラクタ単位)
#define BMPWIN_HEIGHT_CHAR ( 2)								// ウィンドウ高さ(キャラクタ単位)
#define BMPWIN_WIDTH_DOT   (BMPWIN_WIDTH_CHAR * CHAR_SIZE)	// ウィンドウ幅(ドット単位)
#define BMPWIN_HEIGHT_DOT  (BMPWIN_HEIGHT_CHAR * CHAR_SIZE)	// ウィンドウ高さ(ドット単位)
#define BMPWIN_POS_X_CHAR  ( 0)								// ウィンドウのx座標(キャラクタ単位)
#define BMPWIN_POS_Y_CHAR  ( 1)								// ウィンドウのy座標(キャラクタ単位)

//------------
// OBJリソース
//------------
// キャラクタ・リソース・インデックス
enum
{
	CGR_RES_INDEX_MAX
};

// パレット・リソース・インデックス
enum
{
	PLTT_RES_INDEX_CHAR_UNIT,	// 文字ユニットで使用するパレット
	PLTT_RES_INDEX_MAX
};

// セルアニメ・リソース・インデックス
enum
{
	CLANM_RES_INDEX_MAX
};

// セルアクター・ユニット
enum
{
	CLUNIT_INDEX_CHAR_UNIT,		// 文字ユニット
	CLUNIT_INDEX_MAX,
};

//------------------
// 各状態の動作設定
//------------------
#define PROCESS_TIME_FADE_IN  (10)
#define PROCESS_TIME_WAIT_1   (10)
#define PROCESS_TIME_WAIT_2   (30)
#define PROCESS_TIME_FADE_OUT (20)

//-----------------
// システムの状態
//-----------------
typedef enum
{
	STATE_HIDE,			// 非表示
	STATE_FADE_IN,		// フェード・イン
	STATE_WAIT_1,		// 待機1
	STATE_LAUNCH,		// 文字発射
	STATE_WAIT_2,		// 待機2
	STATE_FADE_OUT,		// フェード・アウト
	STATE_MAX,
}
STATE;


//===================================================================================
/**
 * @brief 文字ユニット・パラメータ
 */
//===================================================================================
typedef struct
{
	float x;		// 座標
	float y;		// 
	float sx;		// 速度
	float sy;		// 
	float ax;		// 加速度
	float ay;		// 

	float dx;		// 目標位置
	float dy;		// 
	float dsx;		// 目標位置での速度
	float dsy;		// 

	int tx;			// 残り移動回数( x方向 )
	int ty;			// 残り移動回数( y方向 )
}
CHAR_UNIT_PARAM;

//===================================================================================
/**
 * @brief 文字ユニット構造体
 */
//===================================================================================
typedef struct
{
	GFL_BMP_DATA*   pBmp;		// ビットマップ
	BMPOAM_ACT_PTR  pBmpOamAct;	// BMPOAMアクター
	CHAR_UNIT_PARAM param;		// 移動パラメータ
	u8              width;		// 幅
	u8              height;		// 高さ
	BOOL            moving;		// 動作フラグ
}
CHAR_UNIT;

//===================================================================================
/**
 * @brief 文字ユニット構造体に関する関数群
 */
//===================================================================================
// 初期化関数
static void CHAR_UNIT_Initialize( CHAR_UNIT* p_unit, BMPOAM_SYS_PTR p_bmp_oam_sys, u32 pltt_idx, HEAPID heap_id );

// 後始末関数
static void CHAR_UNIT_Finalize( CHAR_UNIT* p_unit );

// ビットマップを作成する
static void CHAR_UNIT_SetCharCode( CHAR_UNIT* p_unit, GFL_FONT* p_font, STRCODE code, HEAPID heap_id );

// 移動開始時の初期パラメータを設定する
static void CHAR_UNIT_SetMoveParam( CHAR_UNIT* p_unit, const CHAR_UNIT_PARAM* p_param );

// 動かす
static void CHAR_UNIT_Move( CHAR_UNIT* p_unit );

// 移動中かどうかを調べる
static BOOL CHAR_UNIT_IsMoving( CHAR_UNIT* p_unit ); 



//------------------------------------------------------------------------------------ 
/**
 * @brief 初期化関数
 *
 * @param p_unit        初期化する変数へのポインタ
 * @param p_bmp_oam_sys BMPOAMアクターを追加するシステム
 * @param pltt_idx      適用するパレットの登録インデックス
 * @param heap_id       使用するヒープID
 */
//------------------------------------------------------------------------------------
static void CHAR_UNIT_Initialize( CHAR_UNIT* p_unit, BMPOAM_SYS_PTR p_bmp_oam_sys, u32 pltt_idx, HEAPID heap_id )
{
	BMPOAM_ACT_DATA data;

	// ビットマップを作成
	p_unit->pBmp = GFL_BMP_Create( 2, 2, GFL_BMP_16_COLOR, heap_id );

	// BMPOAMアクターの初期設定
	data.bmp        = p_unit->pBmp;
	data.x          = 0;
	data.y          = 0;
	data.pltt_index = pltt_idx;
	data.pal_offset = 0;
	data.soft_pri   = 0;
	data.bg_pri     = BG_FRAME_PRIORITY;
	data.setSerface = CLSYS_DEFREND_MAIN;
	data.draw_type  = CLSYS_DRAW_MAIN; 

	// BMPOAMアクターを作成
	p_unit->pBmpOamAct = BmpOam_ActorAdd( p_bmp_oam_sys, &data );

	// 非表示に設定
	BmpOam_ActorSetDrawEnable( p_unit->pBmpOamAct, FALSE );

	// 初期化
	p_unit->param.tx = 0;
	p_unit->param.ty = 0;
	p_unit->width  = 0;
	p_unit->height = 0;
	p_unit->moving = FALSE;
}

//------------------------------------------------------------------------------------
/**
 * @brief 後始末関数
 *
 * @param p_unit 使用を終了する変数へのポインタ
 */
//------------------------------------------------------------------------------------
static void CHAR_UNIT_Finalize( CHAR_UNIT* p_unit )
{
	// BMPOAMアクターを破棄
	BmpOam_ActorDel( p_unit->pBmpOamAct );

	// ビットマップを破棄
	GFL_BMP_Delete( p_unit->pBmp );
}

//------------------------------------------------------------------------------------
/**
 * @brief ビットマップを作成する
 *
 * @param p_unit  書き込み先ビットマップを持つユニット
 * @param p_font  フォント
 * @param code    書き込む文字の文字コード
 * @param heap_id 使用するヒープID
 */
//------------------------------------------------------------------------------------
static void CHAR_UNIT_SetCharCode( CHAR_UNIT* p_unit, GFL_FONT* p_font, STRCODE code, HEAPID heap_id )
{
	int i;

	STRCODE code_arr[2];
	STRBUF* buf = NULL;
  HEAPID heap_low_id = GFL_HEAP_LOWID( heap_id );

	// 生の文字列を作成し, 文字バッファを作成
	code_arr[0] = code;
	code_arr[1] = GFL_STR_GetEOMCode();
	buf         = GFL_STR_CreateBuffer( 2, heap_low_id );
	GFL_STR_SetStringCodeOrderLength( buf, code_arr, 2 );

	// サイズを取得
	p_unit->width  = (u8)PRINTSYS_GetStrWidth( buf, p_font, 0 );
	p_unit->height = (u8)PRINTSYS_GetStrHeight( buf, p_font );

	// フォントの色を設定
	GFL_FONTSYS_SetColor( COLOR_NO_LETTER, COLOR_NO_SHADOW, COLOR_NO_BACKGROUND );

	// ビットマップに書き込む
	GFL_BMP_Clear( p_unit->pBmp, 0 );
	PRINTSYS_Print( p_unit->pBmp, 0, 0, buf, p_font );
	BmpOam_ActorBmpTrans( p_unit->pBmpOamAct );

	// 後始末
	GFL_STR_DeleteBuffer( buf );
}

//------------------------------------------------------------------------------------
/**
 * @brief 移動開始時の初期パラメータを設定する
 *
 * @param p_unit  設定する変数
 * @param p_param 初期パラメータ
 */
//------------------------------------------------------------------------------------ 
static void CHAR_UNIT_SetMoveParam( CHAR_UNIT* p_unit, const CHAR_UNIT_PARAM* p_param )
{
	const CHAR_UNIT_PARAM* p = p_param;
	float dist_x;
	float dist_y;

	// パラメータをコピー
	p_unit->param    = *p_param;
	p_unit->param.ax = 0;
	p_unit->param.ay = 0;
	p_unit->param.tx = 0;
	p_unit->param.ty = 0;

	// 加速度を計算
	dist_x = p->dx - p->x;
	dist_y = p->dy - p->y;
	if( dist_x != 0 ) p_unit->param.ax = ( ( p->dsx * p->dsx ) - ( p->sx * p->sx ) ) / ( 2 * dist_x );
	if( dist_y != 0 ) p_unit->param.ay = ( ( p->dsy * p->dsy ) - ( p->sy * p->sy ) ) / ( 2 * dist_y );

	// 移動回数を計算
	if( p_unit->param.ax != 0 ) p_unit->param.tx = (int)( ( p_unit->param.dsx - p_unit->param.sx ) / p_unit->param.ax );
	if( p_unit->param.ay != 0 ) p_unit->param.ty = (int)( ( p_unit->param.dsy - p_unit->param.sy ) / p_unit->param.ay );

	// BMPOAMアクターの表示位置を反映
	BmpOam_ActorSetPos( p_unit->pBmpOamAct, p_param->x, p_param->y );
}

//------------------------------------------------------------------------------------ 
/**
 * @brief 動かす
 *
 * @param p_unit 動かすユニット
 */
//------------------------------------------------------------------------------------ 
static void CHAR_UNIT_Move( CHAR_UNIT* p_unit )
{
	// 動作フラグがセットされていなければ, 動かない
	if( p_unit->moving != TRUE ) return;

	// x方向動作
	if( 0 < p_unit->param.tx )
	{
		// 移動
		p_unit->param.x += p_unit->param.sx;

		// 加速
		p_unit->param.sx += p_unit->param.ax;

		// 残り移動回数をデクリメント
		p_unit->param.tx--;
	}
	// y方向動作
	if( 0 < p_unit->param.ty )
	{
		// 移動
		p_unit->param.y += p_unit->param.sy;

		// 加速
		p_unit->param.sy += p_unit->param.ay;

		// 残り移動回数をデクリメント
		p_unit->param.ty--;
	}

	// BMPOAMアクターの表示位置を反映
	BmpOam_ActorSetPos( p_unit->pBmpOamAct, p_unit->param.x, p_unit->param.y );

	// 残り移動回数が無くなったら, 動作フラグを落とす
	if( ( p_unit->param.tx <= 0 ) && ( p_unit->param.ty <= 0 ) )
	{
		p_unit->moving = FALSE;
	}
}

//------------------------------------------------------------------------------------ 
/**
 * @brief 移動中かどうかを調べる
 *
 * @param p_unit 調べるユニット
 *
 * @return 移動中なら TRUE
 */
//------------------------------------------------------------------------------------ 
static BOOL CHAR_UNIT_IsMoving( CHAR_UNIT* p_unit )
{
	return p_unit->moving;
}


//===================================================================================
/**
 * @brief システム・ワーク
 */
//===================================================================================
struct _FIELD_PLACE_NAME
{ 
	HEAPID heapID;	// ヒープID

	// 地名の表示に使用するデータ
	FLDMSGBG*    pFldMsgBG;			// BMPWINへの文字列書き込みに使用するシステム( 外部インスタンスを参照 )
	GFL_MSGDATA* pMsgData;			// メッセージ・データ( インスタンスを保持 )

	// BG
	GFL_BMPWIN*   pBmpWin;	        // ビットマップ・ウィンドウ
	GFL_BMP_DATA* pBGOriginalBmp;	// オリジナル・ビットマップ・データ

	// CLACTリソース( 登録インデックス )
	//u32 hResource_CGR[ CGR_RES_INDEX_MAX ];
	u32 hResource_PLTT[ PLTT_RES_INDEX_MAX ];
	//u32 hResource_CLANM[ CLANM_RES_INDEX_MAX ];

	// セルアクターユニット
	GFL_CLUNIT* pClactUnit[ CLUNIT_INDEX_MAX ];

	// BMPOAMシステム
	BMPOAM_SYS_PTR pBmpOamSys;

	// 文字ユニット
	CHAR_UNIT charUnit[ MAX_NAME_LENGTH ];

	// 動作に使用するデータ
	STATE state;			// 状態
	u16	  stateCount;		// 状態カウンタ
	u32	  currentZoneID;	// 現在表示中の地名に対応するゾーンID
	u32   nextZoneID;		// 次に表示する地名に対応するゾーンID
	u8    launchNum;		// 発射済み文字数

	// 地名
	STRBUF* pNameBuf;		// 表示中の地名文字列
	u8      nameLength;		// 表示中の地名文字数
};


//===================================================================================
/**
 * @brief システムに関する関数
 */
//===================================================================================
// BGの設定を行う
static void SetupBG( FIELD_PLACE_NAME* p_sys );	
static void LoadNullCharacterData();

// リソースの読み込み
static void LoadBGScreenData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id );
static void LoadBGCharacterData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id );
static void LoadBGPaletteData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id ); 
static void LoadClactResource( FIELD_PLACE_NAME* p_sys );

// セルアクターユニットを作成する
static void CreateClactUnit( FIELD_PLACE_NAME* p_sys );

// 文字ユニットを初期化する
static void CreateCharUnit( FIELD_PLACE_NAME* p_sys );

// 指定したゾーンIDに対応する地名を文字ユニットに書き込む
static void WritePlaceName( FIELD_PLACE_NAME* p_sys, u32 zone_id );

// 文字ユニットの動作を設定する
static void SetCharUnitAction( FIELD_PLACE_NAME* p_sys );

// 文字ユニットを発射する
static void LaunchCharUnit( FIELD_PLACE_NAME* p_sys, int unit_index );

// 状態を変更する
static void SetState( FIELD_PLACE_NAME* p_sys, STATE next_state );

// 強制的にウィンドウを退出させる
static void Cancel( FIELD_PLACE_NAME* p_sys );

// 全文字ユニットを非表示に設定する
static void SetAllCharUnitVisibleOff( FIELD_PLACE_NAME* p_sys );

// ビットマップ・ウィンドウをBGオリジナルデータに復元する
static void RecoveryBitmapWindow( FIELD_PLACE_NAME* p_sys );

// 表示中の文字ユニットをビットマップ・ウィンドウに書き込む
static void WriteCharUnitToBitmapWindow( FIELD_PLACE_NAME* p_sys );

// 各状態時の動作
static void Process_HIDE( FIELD_PLACE_NAME* p_sys );
static void Process_FADE_IN( FIELD_PLACE_NAME* p_sys );
static void Process_WAIT_1( FIELD_PLACE_NAME* p_sys );
static void Process_LAUNCH( FIELD_PLACE_NAME* p_sys );
static void Process_WAIT_2( FIELD_PLACE_NAME* p_sys );
static void Process_FADE_OUT( FIELD_PLACE_NAME* p_sys );

// 各状態時の描画処理
static void Draw_HIDE( FIELD_PLACE_NAME* p_sys );
static void Draw_FADE_IN( FIELD_PLACE_NAME* p_sys );
static void Draw_WAIT_1( FIELD_PLACE_NAME* p_sys );
static void Draw_LAUNCH( FIELD_PLACE_NAME* p_sys );
static void Draw_WAIT_2( FIELD_PLACE_NAME* p_sys );
static void Draw_FADE_OUT( FIELD_PLACE_NAME* p_sys );

// デバッグ出力
static void DebugPrint( FIELD_PLACE_NAME* p_sys );


//===================================================================================
/**
 * @brief 公開関数の実装( システムの稼動に関する関数 )
 */
//===================================================================================
//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを作成する
 *
 * @param heap_id      使用するヒープID
 * @param p_fld_msg_bg 使用するメッセージ表示システム
 *
 * @return 地名表示システム・ワークへのポインタ
 */
//------------------------------------------------------------------------------------
FIELD_PLACE_NAME* FIELD_PLACE_NAME_Create( HEAPID heap_id, FLDMSGBG* p_fld_msg_bg )
{
	FIELD_PLACE_NAME* p_sys;

	// システム・ワーク作成
	p_sys = (FIELD_PLACE_NAME*)GFL_HEAP_AllocClearMemory( heap_id, sizeof( FIELD_PLACE_NAME ) );

	// システムの設定
	p_sys->heapID    = heap_id;
	p_sys->pFldMsgBG = p_fld_msg_bg;

	// BGの使用準備
	SetupBG( p_sys );

	// セルアクター用リソースの読み込み
	LoadClactResource( p_sys );

	// セルアクターユニットを作成
	CreateClactUnit( p_sys );

	// BMPOAMシステム作成
	p_sys->pBmpOamSys = BmpOam_Init( heap_id, p_sys->pClactUnit[ CLUNIT_INDEX_CHAR_UNIT ] );

	// 文字ユニットを作成
	CreateCharUnit( p_sys );

	// メッセージ・データを作成
	p_sys->pMsgData = FLDMSGBG_CreateMSGDATA( p_sys->pFldMsgBG, NARC_message_place_name_dat );

	// 文字バッファを作成
	p_sys->pNameBuf = GFL_STR_CreateBuffer( MAX_NAME_LENGTH + 1, p_sys->heapID );

	// その他の初期化
	p_sys->currentZoneID = INVALID_ZONE_ID;
	p_sys->nextZoneID    = INVALID_ZONE_ID;
	p_sys->nameLength    = 0;

	// 初期状態を設定
	SetState( p_sys, STATE_HIDE );

	// 作成したシステムを返す
	return p_sys;
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムの動作処理
 *
 * @param p_sys 動かすシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Process( FIELD_PLACE_NAME* p_sys )
{
	int i;

	// 状態カウンタを更新
	p_sys->stateCount++;

	// 状態に応じた動作
	switch( p_sys->state )
	{
		case STATE_HIDE:		Process_HIDE( p_sys );		break;
		case STATE_FADE_IN:		Process_FADE_IN( p_sys );	break;
		case STATE_WAIT_1:		Process_WAIT_1( p_sys );	break;
		case STATE_LAUNCH:		Process_LAUNCH( p_sys );	break;
		case STATE_WAIT_2:		Process_WAIT_2( p_sys );	break;
		case STATE_FADE_OUT:	Process_FADE_OUT( p_sys );	break;
	}

	// 文字ユニットを動かす
	for( i=0; i<p_sys->nameLength; i++ )
	{
		CHAR_UNIT_Move( &p_sys->charUnit[i] );
	}
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示ウィンドウの描画処理
 *
 * @param p_sys 描画対象システム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Draw( FIELD_PLACE_NAME* p_sys )
{ 
	// 状態に応じた描画処理
	switch( p_sys->state )
	{
		case STATE_HIDE:		Draw_HIDE( p_sys );		break;
		case STATE_FADE_IN:		Draw_FADE_IN( p_sys );	break;
		case STATE_WAIT_1:		Draw_WAIT_1( p_sys );	break;
		case STATE_LAUNCH:		Draw_LAUNCH( p_sys );	break;
		case STATE_WAIT_2:		Draw_WAIT_2( p_sys );	break;
		case STATE_FADE_OUT:	Draw_FADE_OUT( p_sys );	break;
	}

	// DEBUG: デバッグ出力
	//DebugPrint( p_sys );
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを破棄する
 *
 * @param p_sys 破棄するシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Delete( FIELD_PLACE_NAME* p_sys )
{
	int i;

	// 文字バッファを解放
	GFL_STR_DeleteBuffer( p_sys->pNameBuf );

	// BMPOAMシステムの後始末
	for( i=0; i<MAX_NAME_LENGTH; i++ )
	{ 
		CHAR_UNIT_Finalize( &p_sys->charUnit[i] );
	}
	BmpOam_Exit( p_sys->pBmpOamSys );
	
	// セルアクターユニットの破棄
	for( i=0; i<CLUNIT_INDEX_MAX; i++ )
	{
		GFL_CLACT_UNIT_Delete( p_sys->pClactUnit[i] );
	}
	
	// セルアクター用リソースの破棄
	for( i=0; i<CLANM_RES_INDEX_MAX; i++ )
	{
		//GFL_CLGRP_CELLANIM_Release( p_sys->hResource_CLANM[i] );
	}
	for( i=0; i<PLTT_RES_INDEX_MAX; i++ )
	{
		GFL_CLGRP_PLTT_Release( p_sys->hResource_PLTT[i] );
	}
	for( i=0; i<CGR_RES_INDEX_MAX; i++ )
	{
		//GFL_CLGRP_CGR_Release( p_sys->hResource_CGR[i] );
	}
	
	// 描画用インスタンスの破棄
	GFL_MSG_Delete( p_sys->pMsgData );

	// ビットマップ・ウィンドウの破棄
	GFL_BMPWIN_Delete( p_sys->pBmpWin );
	GFL_BMP_Delete( p_sys->pBGOriginalBmp );

	// BGSYSの後始末
	GFL_BG_FreeBGControl( BG_FRAME );

	// システム・ワーク解放
	GFL_HEAP_FreeMemory( p_sys );
} 


//===================================================================================
/**
 * 公開関数の実装( システムの制御に関する関数 )
 */
//===================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief ゾーンの切り替えを通知し, 新しい地名を表示する
 *
 * @param p_sys        ゾーン切り替えを通知するシステム
 * @param next_zone_id 新しいゾーンID
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Display( FIELD_PLACE_NAME* p_sys, u32 next_zone_id )
{ 
  // ゾーンの地名表示フラグが立っていない場合は表示しない
  if( ZONEDATA_GetPlaceNameFlag( next_zone_id ) != TRUE ) return;

	// 指定されたゾーンIDを次に表示すべきものとして記憶
	p_sys->nextZoneID = next_zone_id;

	// 表示中のウィンドウを退出させる
	Cancel( p_sys ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名ウィンドウを強制的に表示する
 *
 * @param p_sys   表示するシステム
 * @param zone_id 表示する場所のゾーンID
 */
//------------------------------------------------------------------------------------
extern void FIELD_PLACE_NAME_DisplayForce( FIELD_PLACE_NAME* p_sys, u32 zone_id )
{
  // 強制的に表示
  p_sys->currentZoneID = INVALID_ZONE_ID;
  p_sys->nextZoneID    = zone_id;

	// 表示中のウィンドウを退出させる
	Cancel( p_sys ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名ウィンドウの表示を強制的に終了する
 *
 * @param p_sys 表示を終了するシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Hide( FIELD_PLACE_NAME* p_sys )
{
	SetState( p_sys, STATE_HIDE );
}


//===================================================================================
/**
 * 非公開関数の実装
 */
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief BGの設定を行う
 *
 * @param p_sys 設定を行うシステム
 */ 
//-----------------------------------------------------------------------------------
static void SetupBG( FIELD_PLACE_NAME* p_sys )
{ 
	GFL_BG_BGCNT_HEADER bgcnt = 
	{
		0, 0,					    // 初期表示位置
		0x800,						// スクリーンバッファサイズ
		0,							// スクリーンバッファオフセット
		GFL_BG_SCRSIZ_256x256,		// スクリーンサイズ
		GX_BG_COLORMODE_16,			// カラーモード
		GX_BG_SCRBASE_0x1000,		// スクリーンベースブロック
		GX_BG_CHARBASE_0x04000,		// キャラクタベースブロック
		GFL_BG_CHRSIZ_256x256,		// キャラクタエリアサイズ
		GX_BG_EXTPLTT_01,			// BG拡張パレットスロット選択
		1,							// 表示プライオリティー
		0,							// エリアオーバーフラグ
		0,							// DUMMY
		FALSE,						// モザイク設定
	};

	GFL_BG_SetBGControl( BG_FRAME, &bgcnt, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( BG_FRAME, BG_FRAME_PRIORITY );
	GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON ); 
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, ALPHA_VALUE_1, ALPHA_VALUE_2 );

	// キャラVRAM・スクリーンVRAMをクリア
	GFL_BG_ClearFrame( BG_FRAME );

	// NULLキャラクタデータをセット
	LoadNullCharacterData();

	// ビットマップ・ウィンドウを作成する
	p_sys->pBmpWin = GFL_BMPWIN_Create( 
			BG_FRAME,
			BMPWIN_POS_X_CHAR, BMPWIN_POS_Y_CHAR, BMPWIN_WIDTH_CHAR, BMPWIN_HEIGHT_CHAR,
			BG_PALETTE_NO, GFL_BMP_CHRAREA_GET_F );

	// パレット・キャラクタ・スクリーンを転送
	LoadBGCharacterData( p_sys, ARCID_PLACE_NAME, NARC_place_name_place_name_back_NCGR );
	LoadBGPaletteData( p_sys, ARCID_PLACE_NAME, NARC_place_name_place_name_back_NCLR );
	//LoadBGScreenData( p_sys, ARCID_PLACE_NAME, NARC_area_win_gra_place_name_back_NSCR );
	
	// ビットマップ・ウィンドウのデータをVRAMに転送
	GFL_BMPWIN_MakeTransWindow( p_sys->pBmpWin );

	// DEBUG: スクリーン・バッファを書き出す
	/*
	{
		int i,j;
		u8* p_screen = (u8*)GFL_BG_GetScreenBufferAdrs( BG_FRAME );

		OBATA_Printf( "=============================== screen\n" );

		for( i=0; i<32; i++ )
		{
			for( j=0; j<32; j++ )
			{
				OBATA_Printf( "%d ", p_screen[ i*32 + j ] );
			}
			OBATA_Printf( "\n" );
		}
	}
	*/
}

//------------------------------------------------------------------------------------
/**
 * @brief 空のキャラクタデータを1キャラ分VRAMに確保する
 */
//------------------------------------------------------------------------------------
static void LoadNullCharacterData()
{
	GFL_BG_AllocCharacterArea( BG_FRAME, CHAR_SIZE * CHAR_SIZE / 2, GFL_BG_CHRAREA_GET_F );
	//GFL_BG_FillCharacter( BG_FRAME, 0, 1, 0 );
}

//------------------------------------------------------------------------------------
/**
 * @brief スクリーン・データを読み込む
 *
 * @param p_sys   読み込みを行うシステム
 * @param arc_id  アーカイブデータのインデックス
 * @param data_id アーカイブ内データインデックス
 */
//------------------------------------------------------------------------------------
static void LoadBGScreenData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id )
{
	ARCHANDLE* p_h_arc;
	u32 size;
	void* p_src;
	void* p_tmp;
	NNSG2dScreenData* p_screen;
	p_h_arc = GFL_ARC_OpenDataHandle( arc_id, p_sys->heapID );					// アーカイブデータハンドルオープン
	size    = GFL_ARC_GetDataSizeByHandle( p_h_arc, data_id );					// データサイズ取得
	p_src   = GFL_HEAP_AllocMemoryLo( p_sys->heapID, size );					// データバッファ確保
	GFL_ARC_LoadDataByHandle( p_h_arc, data_id, p_src );						// データ取得
	NNS_G2dGetUnpackedScreenData( p_src, &p_screen );							// バイナリからデータを展開
	GFL_BG_LoadScreenBuffer( BG_FRAME, p_screen->rawData, p_screen->szByte );	// BGSYS内部バッファに転送
	GFL_BG_LoadScreenReq( BG_FRAME );											// VRAMに転送
	GFL_HEAP_FreeMemory( p_src );												// データバッファ解放
	GFL_ARC_CloseDataHandle( p_h_arc );											// アーカイブデータハンドルクローズ
}

//------------------------------------------------------------------------------------
/**
 * @brief キャラクタデータをロードする
 *
 * @param p_sys   読み込みを行うシステム
 * @param arc_id  アーカイブデータのインデックス
 * @param data_id アーカイブ内データインデックス
 */
//------------------------------------------------------------------------------------
static void LoadBGCharacterData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id )
{
	/*
		u32 size;
		void* pSrc;
		ARCHANDLE* handle;
		NNSG2dCharacterData* pChar;

		handle = GFL_ARC_OpenDataHandle( arc_id, p_sys->heapID );

		size = GFL_ARC_GetDataSizeByHandle( handle, data_id );
		pSrc = GFL_HEAP_AllocMemoryLo( p_sys->heapID, size );
		GFL_ARC_LoadDataByHandle(handle, data_id, (void*)pSrc);

		NNS_G2dGetUnpackedCharacterData(pSrc, &pChar);
		GFL_BG_LoadCharacter(BG_FRAME, pChar->pRawData, pChar->szByte, 0);

		GFL_HEAP_FreeMemory( pSrc );

		GFL_ARC_CloseDataHandle( handle ); 
	*/

	int i;
	u16 sx, sy, dx, dy;
	GFL_BMP_DATA* p_dst_bmp = NULL;

	// コピー元・コピー先ビットマップを取得
	p_sys->pBGOriginalBmp = GFL_BMP_LoadCharacter( arc_id, data_id, FALSE, p_sys->heapID );
	p_dst_bmp             = GFL_BMPWIN_GetBmp( p_sys->pBmpWin );

	// 1キャラずつコピーする
	sx = 0;
	sy = 0;
	dx = 0;
	dy = 0;
	for( i=0; i<BMPWIN_WIDTH_CHAR * BMPWIN_HEIGHT_CHAR; i++ )
	{
		// コピー
		GFL_BMP_Print( p_sys->pBGOriginalBmp, p_dst_bmp, dx, sy, dx, dy, CHAR_SIZE, CHAR_SIZE, 0 );

		// コピー元座標の更新
		sx += CHAR_SIZE;
		if( BMPWIN_WIDTH_DOT <= sx )
		{
			sx  = 0;
			sy += CHAR_SIZE;
		}
		// コピー先座標の更新
		dx += CHAR_SIZE;
		if( BMPWIN_WIDTH_DOT <= dx )
		{
			dx  = 0;
			dy += CHAR_SIZE;
		}
	}
}

//------------------------------------------------------------------------------------
/**
 * @brief パレット・データを読み込む
 *
 * @param p_sys   読み込みを行うシステム
 * @param arc_id  アーカイブデータのインデックス
 * @param data_id アーカイブ内データインデックス
 */
//------------------------------------------------------------------------------------
static void LoadBGPaletteData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id )
{
	ARCHANDLE* p_h_arc;
	u32 size;
	void* p_src;
	NNSG2dPaletteData* p_palette;
	p_h_arc = GFL_ARC_OpenDataHandle( arc_id, p_sys->heapID );				// アーカイブデータハンドルオープン
	size    = GFL_ARC_GetDataSizeByHandle( p_h_arc, data_id );				// データサイズ取得
	p_src   = GFL_HEAP_AllocMemoryLo( p_sys->heapID, size );				// データバッファ確保
	GFL_ARC_LoadDataByHandle( p_h_arc, data_id, p_src );					// データ取得
	NNS_G2dGetUnpackedPaletteData( p_src, &p_palette );						// バイナリからデータを展開
	GFL_BG_LoadPalette( BG_FRAME, p_palette->pRawData, 0x20, BG_PALETTE_NO );	// パレットデータ転送
	GFL_HEAP_FreeMemory( p_src );											// データバッファ解放
	GFL_ARC_CloseDataHandle( p_h_arc );										// アーカイブデータハンドルクローズ
}

//-----------------------------------------------------------------------------------
/**
 * @brief セルアクターで使用するリソースを読み込む
 *
 * @param p_sys 読み込み対象システム
 */
//-----------------------------------------------------------------------------------
static void LoadClactResource( FIELD_PLACE_NAME* p_sys )
{
	ARCHANDLE* p_arc_handle;

	p_arc_handle = GFL_ARC_OpenDataHandle( ARCID_PLACE_NAME, p_sys->heapID );

	p_sys->hResource_PLTT[ PLTT_RES_INDEX_CHAR_UNIT ] = 
		GFL_CLGRP_PLTT_RegisterEx( 
				p_arc_handle, NARC_place_name_place_name_string_NCLR,
				CLSYS_DRAW_MAIN, OBJ_PALETTE_NO * 32, 0, 1, p_sys->heapID );

	GFL_ARC_CloseDataHandle( p_arc_handle );
}

//-----------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを作成する
 *
 * @param p_sys 作成対象システム
 */
//-----------------------------------------------------------------------------------
static void CreateClactUnit( FIELD_PLACE_NAME* p_sys )
{
	// セルアクターユニットを作成
	p_sys->pClactUnit[ CLUNIT_INDEX_CHAR_UNIT ] = 
		GFL_CLACT_UNIT_Create( MAX_NAME_LENGTH, BG_FRAME_PRIORITY, p_sys->heapID );

	// 初期設定
	GFL_CLACT_UNIT_SetDrawEnable( p_sys->pClactUnit[ CLUNIT_INDEX_CHAR_UNIT ], TRUE );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字ユニットを初期化する
 *
 * @param p_sys 作成対象システム
 */
//-----------------------------------------------------------------------------------
static void CreateCharUnit( FIELD_PLACE_NAME* p_sys )
{
	int i;

	for( i=0; i<MAX_NAME_LENGTH; i++ )
	{
		CHAR_UNIT_Initialize( 
				&p_sys->charUnit[i], p_sys->pBmpOamSys, 
				p_sys->hResource_PLTT[ PLTT_RES_INDEX_CHAR_UNIT ], p_sys->heapID );
	} 
}

//-----------------------------------------------------------------------------------
/**
 * @brief 指定したゾーンIDに対応する地名を文字ユニットに書き込む
 *
 * @param p_sys   操作対象システム
 * @param zone_id ゾーンID
 */
//----------------------------------------------------------------------------------- 
static void WritePlaceName( FIELD_PLACE_NAME* p_sys, u32 zone_id )
{
	int            i;
	u16            str_id;				// メッセージデータ内の文字列ID
	const STRCODE* p_str_code = NULL;	// 生の文字列
	GFL_FONT*      p_font     = NULL;	// フォント

	// フォントを取得
	p_font = FLDMSGBG_GetFontHandle( p_sys->pFldMsgBG );		

	// ゾーンIDから文字列を取得する
	str_id = ZONEDATA_GetPlaceNameID( zone_id );					// メッセージ番号を決定
	if( str_id < 0 | msg_place_name_max <= str_id ) str_id = 0;		// メッセージ番号の範囲チェック
	GFL_MSG_GetString( p_sys->pMsgData,	str_id, p_sys->pNameBuf );	// 地名文字列を取得
	p_str_code = GFL_STR_GetStringCodePointer( p_sys->pNameBuf );	// 生の文字列を取得
	p_sys->nameLength = GFL_STR_GetBufferLength( p_sys->pNameBuf );	// 文字数を取得

	// 文字数チェック
	//GF_ASSERT_MSG( ( p_sys->nameLength < MAX_NAME_LENGTH ), "最大文字数をオーバーしています。" );	// 止めない
	if( MAX_NAME_LENGTH < p_sys->nameLength )
	{
		OBATA_Printf( "地名の最大文字数をオーバーしています。\n" );
		p_sys->nameLength =	MAX_NAME_LENGTH;
	}

	// 1文字ずつ設定
	for( i=0; i<p_sys->nameLength; i++ )
	{
		// ビットマップを作成する
		CHAR_UNIT_SetCharCode( &p_sys->charUnit[i], p_font, p_str_code[i], p_sys->heapID );

		// 表示状態を設定
		BmpOam_ActorSetDrawEnable( p_sys->charUnit[i].pBmpOamAct, TRUE );
	}
	for( i=p_sys->nameLength; i<MAX_NAME_LENGTH; i++ )
	{
		// 表示状態を設定
		BmpOam_ActorSetDrawEnable( p_sys->charUnit[i].pBmpOamAct, FALSE );
	}

	// TEMP: "なぞのばしょ" なら表示しない
	if( str_id == 0 ) 
  {
    OBATA_Printf( "「なぞのばしょ」を検出( zone id = %d )\n", zone_id );
    FIELD_PLACE_NAME_Hide( p_sys );
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字ユニットの動作を設定する
 *
 * @param 設定対象システム
 */
//-----------------------------------------------------------------------------------
static void SetCharUnitAction( FIELD_PLACE_NAME* p_sys )
{
	int i;
	int str_width;	// 文字列の幅
	int str_height;	// 文字列の高さ
	const STRCODE* p_str_code = NULL;
	GFL_FONT*      p_font     = NULL;

	// フォントを取得
	p_font = FLDMSGBG_GetFontHandle( p_sys->pFldMsgBG );		

	// 文字列のサイズを取得
	str_width  = PRINTSYS_GetStrWidth( p_sys->pNameBuf, p_font, 0 );
    str_height = PRINTSYS_GetStrHeight( p_sys->pNameBuf, p_font );

	// 各文字ユニットの設定
	{
		CHAR_UNIT_PARAM param;

		// 1文字目の目標パラメータを設定
		param.x   = 256;
		param.y   = Y_CENTER_POS - ( str_height / 2 );
		param.sx  = -20;
		param.sy  =  0;
		//param.dx  = 128 - ( str_width / 2 ) - ( p_sys->nameLength / 2 );	// 中央揃え
		param.dx  = 24;			// 左詰め
		param.dy  = param.y;
		param.dsx = 0;
		param.dsy = 0;

		// 1文字ずつ設定する
		for( i=0; i<p_sys->nameLength; i++ )
		{
			// 目標パラメータを設定
			CHAR_UNIT_SetMoveParam( &p_sys->charUnit[i], &param );

			// 目標パラメータを更新
			param.dx += p_sys->charUnit[i].width + 1;
		}
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字ユニットを発射する
 *
 * @param p_sys      システム
 * @param unit_index 発射する文字ユニット番号(何文字目か)
 */
//-----------------------------------------------------------------------------------
static void LaunchCharUnit( FIELD_PLACE_NAME* p_sys, int unit_index )
{
	// 動作フラグを立てる
	p_sys->charUnit[ unit_index ].moving = TRUE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param p_sys      状態を変更するシステム
 * @param next_state 設定する状態
 */
//-----------------------------------------------------------------------------------
static void SetState( FIELD_PLACE_NAME* p_sys, STATE next_state )
{
	// 状態を変更し, 状態カウンタを初期化する
	p_sys->state      = next_state;
	p_sys->stateCount = 0;

	// 遷移先の状態に応じた初期化
	switch( next_state )
	{
		case STATE_HIDE:	
			GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF );	// BGを非表示
			SetAllCharUnitVisibleOff( p_sys );			// 文字ユニットを非表示に
			break;
		case STATE_FADE_IN:
      GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON );	// BGを表示
			p_sys->currentZoneID = p_sys->nextZoneID;	// 表示中ゾーンIDを更新
			RecoveryBitmapWindow( p_sys );				// ビットマップウィンドウを復帰
			WritePlaceName( p_sys, p_sys->nextZoneID );	// 新しい地名を書き込む
			SetCharUnitAction( p_sys );					// 文字ユニットの動きをセット
			break;
		case STATE_WAIT_1:
			break;
		case STATE_LAUNCH:
			p_sys->launchNum = 0;	// 発射文字数をリセット
			break;
		case STATE_WAIT_2:
			break;
		case STATE_FADE_OUT:
			WriteCharUnitToBitmapWindow( p_sys );	// 現時点での文字をBGに書き込む
			SetAllCharUnitVisibleOff( p_sys );		// 文字ユニットを非表示に
			break;
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 強制的にウィンドウを退出させる
 *
 * @param p_sys 状態を更新するシステム
 */
//-----------------------------------------------------------------------------------
static void Cancel( FIELD_PLACE_NAME* p_sys )
{
	STATE next_state;
	int   start_count;
	float passed_rate;
	
	// 現在の状態に応じた処理
	switch( p_sys->state )
	{
		case STATE_HIDE:	
		case STATE_FADE_OUT:
			return;
		case STATE_WAIT_1:
		case STATE_WAIT_2:
		case STATE_LAUNCH:
			next_state  = STATE_WAIT_2;
			start_count = PROCESS_TIME_WAIT_2;
			break;
		case STATE_FADE_IN:
			next_state = STATE_FADE_OUT;
			// 経過済みフレーム数を算出
			// (stateCountから, 表示位置を算出しているため, 強制退出させた場合, stateCount を適切に計算する必要がある)
			passed_rate = p_sys->stateCount / (float)PROCESS_TIME_FADE_IN;			// 経過した時間の割合を算出
			start_count = (int)( (1.0f - passed_rate) * PROCESS_TIME_FADE_OUT );	// 遷移先状態の経過済みカウントを算出
			break;
	}

	// 状態を更新
	SetState( p_sys, next_state );
	p_sys->stateCount = start_count;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 全文字ユニットを非表示に設定する
 *
 * @param p_sys 設定対象システム
 */
//-----------------------------------------------------------------------------------
static void SetAllCharUnitVisibleOff( FIELD_PLACE_NAME* p_sys )
{
	int i;
	
	for( i=0; i<MAX_NAME_LENGTH; i++ )
	{
		BmpOam_ActorSetDrawEnable( p_sys->charUnit[i].pBmpOamAct, FALSE );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief ビットマップ・ウィンドウをBGオリジナルデータに復元する
 *
 * @param p_sys 操作対象システム
 */
//-----------------------------------------------------------------------------------
static void RecoveryBitmapWindow( FIELD_PLACE_NAME* p_sys )
{
	GFL_BMP_DATA* p_src = p_sys->pBGOriginalBmp;
	GFL_BMP_DATA* p_dst = GFL_BMPWIN_GetBmp( p_sys->pBmpWin );

	// オリジナル(文字が書き込まれていない状態)をコピーして, VRAMに転送
	GFL_BMP_Copy( p_src, p_dst );	
	GFL_BMPWIN_TransVramCharacter( p_sys->pBmpWin );	
}

//-----------------------------------------------------------------------------------
/**
 * @brief 表示中の文字ユニットをビットマップ・ウィンドウに書き込む
 *
 * @param p_sys 操作対象システム
 */
//-----------------------------------------------------------------------------------
static void WriteCharUnitToBitmapWindow( FIELD_PLACE_NAME* p_sys )
{
	int i;
	u16 dx, dy;
	CHAR_UNIT* p_char;
	GFL_BMP_DATA* p_src_bmp = NULL;
	GFL_BMP_DATA* p_dst_bmp = GFL_BMPWIN_GetBmp( p_sys->pBmpWin );

	// 表示中の全ての文字ユニットをコピーする
	for( i=0; i<p_sys->nameLength; i++ )
	{
		p_char = &p_sys->charUnit[i];
		p_src_bmp = p_char->pBmp;
		dx = p_char->param.x - ( BMPWIN_POS_X_CHAR * CHAR_SIZE );
		dy = p_char->param.y - ( BMPWIN_POS_Y_CHAR * CHAR_SIZE );
		GFL_BMP_Print( p_src_bmp, p_dst_bmp, 0, 0, dx, dy, p_char->width, p_char->height, 0 );
	}

	// 更新されたキャラデータをVRAMに転送
	GFL_BMPWIN_TransVramCharacter( p_sys->pBmpWin );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 非表示状態時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_HIDE( FIELD_PLACE_NAME* p_sys )
{
	// ゾーン切り替えを検出したら, 次の状態へ
	if( p_sys->currentZoneID != p_sys->nextZoneID )
	{
		SetState( p_sys, STATE_FADE_IN );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・イン状態時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_FADE_IN( FIELD_PLACE_NAME* p_sys )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_FADE_IN < p_sys->stateCount )
	{
		SetState( p_sys, STATE_WAIT_1 );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT_1( FIELD_PLACE_NAME* p_sys )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_WAIT_1 < p_sys->stateCount )
	{
		SetState( p_sys, STATE_LAUNCH );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字発射状態時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_LAUNCH( FIELD_PLACE_NAME* p_sys )
{
	// 一定間隔が経過したら, 文字を発射
	if( p_sys->stateCount % LAUNCH_INTERVAL == 0 )
	{
		// 発射
		LaunchCharUnit( p_sys, p_sys->launchNum );

		// 発射した文字の数をカウント
		p_sys->launchNum++;

		// すべての文字を発射したら, 次の状態へ
		if( p_sys->nameLength <= p_sys->launchNum )
		{
			SetState( p_sys, STATE_WAIT_2 );
		}
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT_2( FIELD_PLACE_NAME* p_sys )
{
	int i;

	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_WAIT_2 < p_sys->stateCount )
	{
		// 動いている文字があるなら, 待機状態を維持
		for( i=0; i<p_sys->nameLength; i++ )
		{
			if( CHAR_UNIT_IsMoving( &p_sys->charUnit[i] ) == TRUE ) break;
		}

		// 次の状態へ
		SetState( p_sys, STATE_FADE_OUT );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・アウト状態時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_FADE_OUT( FIELD_PLACE_NAME* p_sys )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_FADE_OUT < p_sys->stateCount )
	{
		SetState( p_sys, STATE_HIDE );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 非表示状態時の描画処理
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_HIDE( FIELD_PLACE_NAME* p_sys )
{
	// BGを非表示
	//GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF );	
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・イン状態時の描画処理
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_FADE_IN( FIELD_PLACE_NAME* p_sys )
{
	int val1, val2;
	float rate;

	// BGを表示
	//GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON );	
	
	// αブレンディング係数を更新
	rate  = (float)p_sys->stateCount / (float)PROCESS_TIME_FADE_IN;
	val1 = (int)( ALPHA_VALUE_1 * rate );
	val2 = (int)( ALPHA_VALUE_2 + (16 - ALPHA_VALUE_2) * (1.0f - rate) );
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, val1, val2 );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の描画処理
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_WAIT_1( FIELD_PLACE_NAME* p_sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字発射状態時の描画処理
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_LAUNCH( FIELD_PLACE_NAME* p_sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の描画処理
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_WAIT_2( FIELD_PLACE_NAME* p_sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・アウト状態時の描画処理
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_FADE_OUT( FIELD_PLACE_NAME* p_sys )
{
	int val1, val2;
	float rate;
	
	// αブレンディング係数を更新
	rate  = (float)p_sys->stateCount / (float)PROCESS_TIME_FADE_OUT;
	val1 = (int)( ALPHA_VALUE_1 * (1.0f - rate) );
	val2 = (int)( ALPHA_VALUE_2 + (16 - ALPHA_VALUE_2) * rate );
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, val1, val2 );
}

//-----------------------------------------------------------------------------------
/**
 * @brief デバッグ出力
 *
 * @param p_sys データを出力するシステム
 */
//-----------------------------------------------------------------------------------
static void DebugPrint( FIELD_PLACE_NAME* p_sys )
{
	char* str;

	switch( p_sys->state )
	{
		case STATE_HIDE:		str = "HIDE";		break;
		case STATE_FADE_IN:		str = "FADE_IN";	break;
		case STATE_WAIT_1:		str = "WAIT_1";		break;
		case STATE_LAUNCH:		str = "LAUNCE";		break;
		case STATE_WAIT_2:		str = "WAIT_2";		break;
		case STATE_FADE_OUT:	str = "FADE_OUT";	break;
	}

	// DEBUG:
	OBATA_Printf( "-------------------------------FIELD_PLACE_NAME\n" );
	OBATA_Printf( "state         = %s\n", str );
	OBATA_Printf( "stateCount    = %d\n", p_sys->stateCount );
	OBATA_Printf( "currentZoneID = %d\n", p_sys->currentZoneID );
	OBATA_Printf( "nextZoneID    = %d\n", p_sys->nextZoneID );
}
