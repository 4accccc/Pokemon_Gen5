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
#include <gflib.h>
#include "field_place_name.h"
#include "field/zonedata.h"
#include "arc/arc_def.h"
#include "arc/place_name.naix"
#include "arc/message.naix"
#include "msg/msg_place_name.h"
#include "system/bmp_oam.h"
#include "pm_define.h"
#include "field_oam_pal.h"  // for FIELDOAM_PALNO_PLACENAME


//===================================================================================
// ■ 定数・マクロ
//=================================================================================== 
// 最大文字数
#define MAX_NAME_LENGTH (BUFLEN_PLACE_NAME - BUFLEN_EOM_SIZE)

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
// BG //-----
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
#define PROCESS_TIME_WAIT_LAUNCH   (10)
#define PROCESS_TIME_WAIT_FADE_OUT   (30)
#define PROCESS_TIME_FADE_OUT (20)

//-----------------
// システムの状態
//-----------------
typedef enum {
	STATE_HIDE,			      // 非表示
  STATE_SETUP,          // 準備
	STATE_FADE_IN,	      // フェード・イン
	STATE_WAIT_LAUNCH,		// 発射待ち
	STATE_LAUNCH,		      // 文字発射
	STATE_WAIT_FADE_OUT,  // フェードアウト待ち
	STATE_FADE_OUT,	      // フェード・アウト
	STATE_NUM,
  STATE_MAX = STATE_NUM - 1
} STATE;


//===================================================================================
// ■ 文字ユニット・移動パラメータ
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
	float dsx;	// 目標位置での速度
	float dsy;	// 

	int tx;			// 残り移動回数( x方向 )
	int ty;			// 残り移動回数( y方向 )
}
CHAR_UNIT_PARAM;


//===================================================================================
// ■文字ユニット
//===================================================================================
typedef struct
{
	GFL_BMP_DATA*   bmp;		      // ビットマップ
	BMPOAM_ACT_PTR  bmpOamActor;	// BMPOAMアクター
	CHAR_UNIT_PARAM param;		    // 移動パラメータ
	u8              width;		    // 幅
	u8              height;		    // 高さ
	BOOL            moving;		    // 動作フラグ
}
CHAR_UNIT;


//===================================================================================
// ■文字ユニットに関する関数群
//===================================================================================
static void CHAR_UNIT_Initialize( 
    CHAR_UNIT* unit, BMPOAM_SYS_PTR bmpoam_sys, u32 pltt_idx, HEAPID heap_id );
static void CHAR_UNIT_Finalize( CHAR_UNIT* unit );
static void CHAR_UNIT_Write( 
    CHAR_UNIT* unit, GFL_FONT* font, STRCODE code, HEAPID heap_id );
static void CHAR_UNIT_SetMoveParam( CHAR_UNIT* unit, const CHAR_UNIT_PARAM* p_param );
static void CHAR_UNIT_MoveStart( CHAR_UNIT* unit );
static void CHAR_UNIT_Move( CHAR_UNIT* unit );
static BOOL CHAR_UNIT_IsMoving( CHAR_UNIT* unit ); 


//------------------------------------------------------------------------------------ 
/**
 * @brief 初期化関数
 *
 * @param unit       初期化する変数へのポインタ
 * @param bmpoam_sys BMPOAMアクターを追加するシステム
 * @param pltt_idx   適用するパレットの登録インデックス
 * @param heap_id    使用するヒープID
 */
//------------------------------------------------------------------------------------
static void CHAR_UNIT_Initialize( 
    CHAR_UNIT* unit, BMPOAM_SYS_PTR bmpoam_sys, u32 pltt_idx, HEAPID heap_id )
{
	BMPOAM_ACT_DATA actor_data;

	// ビットマップを作成
	unit->bmp = GFL_BMP_Create( 2, 2, GFL_BMP_16_COLOR, heap_id );

	// BMPOAMアクターを作成
	actor_data.bmp        = unit->bmp;
	actor_data.x          = 0;
	actor_data.y          = 0;
	actor_data.pltt_index = pltt_idx;
	actor_data.pal_offset = 0;
	actor_data.soft_pri   = 0;
	actor_data.bg_pri     = BG_FRAME_PRIORITY;
	actor_data.setSerface = CLSYS_DEFREND_MAIN;
	actor_data.draw_type  = CLSYS_DRAW_MAIN; 
	unit->bmpOamActor = BmpOam_ActorAdd( bmpoam_sys, &actor_data ); 
	BmpOam_ActorSetDrawEnable( unit->bmpOamActor, FALSE );  // 非表示

	// 初期化
	unit->param.tx = 0;
	unit->param.ty = 0;
	unit->width    = 0;
	unit->height   = 0;
	unit->moving   = FALSE;
}

//------------------------------------------------------------------------------------
/**
 * @brief 後始末関数
 *
 * @param unit 使用を終了する変数へのポインタ
 */
//------------------------------------------------------------------------------------
static void CHAR_UNIT_Finalize( CHAR_UNIT* unit )
{
	// BMPOAMアクターを破棄
	BmpOam_ActorDel( unit->bmpOamActor );

	// ビットマップを破棄
	GFL_BMP_Delete( unit->bmp );
}

//------------------------------------------------------------------------------------
/**
 * @brief 文字ユニットの持つビットマップに文字を書き込む
 *
 * @param unit    書き込み先ユニット
 * @param font    フォント
 * @param code    書き込む文字
 * @param heap_id 使用するヒープID
 */
//------------------------------------------------------------------------------------
static void CHAR_UNIT_Write( CHAR_UNIT* unit, GFL_FONT* font, STRCODE code, HEAPID heap_id )
{
	int i;

	STRCODE string[2];
	STRBUF* strbuf = NULL;
  HEAPID heap_low_id = GFL_HEAP_LOWID( heap_id );
  PRINTSYS_LSB color = 
    PRINTSYS_LSB_Make( COLOR_NO_LETTER, COLOR_NO_SHADOW, COLOR_NO_BACKGROUND );

	// 生の文字列を作成し, 文字バッファを作成
	string[0] = code;
	string[1] = GFL_STR_GetEOMCode();
	strbuf    = GFL_STR_CreateBuffer( 2, heap_low_id );
	GFL_STR_SetStringCodeOrderLength( strbuf, string, 2 );

	// サイズ取得
	unit->width  = (u8)PRINTSYS_GetStrWidth( strbuf, font, 0 );
	unit->height = (u8)PRINTSYS_GetStrHeight( strbuf, font );

	// ビットマップに書き込む
	GFL_BMP_Clear( unit->bmp, 0 );
	PRINTSYS_PrintColor( unit->bmp, 0, 0, strbuf, font, color );
	BmpOam_ActorBmpTrans( unit->bmpOamActor );

	// 後始末
	GFL_STR_DeleteBuffer( strbuf );
}

//------------------------------------------------------------------------------------
/**
 * @brief 移動開始時の初期パラメータを設定する
 *
 * @param unit  設定する変数
 * @param param 初期パラメータ
 */
//------------------------------------------------------------------------------------ 
static void CHAR_UNIT_SetMoveParam( CHAR_UNIT* unit, const CHAR_UNIT_PARAM* param )
{
	float dist_x;
	float dist_y;

	// パラメータをコピー
	unit->param    = *param;
	unit->param.ax = 0;
	unit->param.ay = 0;
	unit->param.tx = 0;
	unit->param.ty = 0;

	// 加速度を計算
	dist_x = param->dx - param->x;
	if( dist_x != 0 )
  {
    unit->param.ax = 
      ( ( param->dsx * param->dsx ) - ( param->sx * param->sx ) ) / ( 2 * dist_x );
  }
	dist_y = param->dy - param->y;
	if( dist_y != 0 )
  {
    unit->param.ay = 
      ( ( param->dsy * param->dsy ) - ( param->sy * param->sy ) ) / ( 2 * dist_y );
  }

	// 移動回数を計算
	if( unit->param.ax != 0 )
  {
    unit->param.tx = (int)( ( unit->param.dsx - unit->param.sx ) / unit->param.ax );
  }
	if( unit->param.ay != 0 )
  {
    unit->param.ty = (int)( ( unit->param.dsy - unit->param.sy ) / unit->param.ay );
  }

	// BMPOAMアクターの表示位置を反映
	BmpOam_ActorSetPos( unit->bmpOamActor, param->x, param->y );
}

//------------------------------------------------------------------------------------ 
/**
 * @brief 文字ユニットの移動を開始する
 *
 * @param unit 移動を開始するユニット
 */
//------------------------------------------------------------------------------------ 
static void CHAR_UNIT_MoveStart( CHAR_UNIT* unit )
{
  unit->moving = TRUE;
}

//------------------------------------------------------------------------------------ 
/**
 * @brief 動かす
 *
 * @param unit 動かすユニット
 */
//------------------------------------------------------------------------------------ 
static void CHAR_UNIT_Move( CHAR_UNIT* unit )
{
	// 動作フラグがセットされていなければ, 動かない
	if( unit->moving != TRUE ) return;

	// x方向動作
	if( 0 < unit->param.tx )
	{
		// 移動
		unit->param.x += unit->param.sx;

		// 加速
		unit->param.sx += unit->param.ax;

		// 残り移動回数をデクリメント
		unit->param.tx--;
	}
	// y方向動作
	if( 0 < unit->param.ty )
	{
		// 移動
		unit->param.y += unit->param.sy;

		// 加速
		unit->param.sy += unit->param.ay;

		// 残り移動回数をデクリメント
		unit->param.ty--;
	}

	// 残り移動回数が無くなったら, 動作フラグを落とす
	if( ( unit->param.tx <= 0 ) && ( unit->param.ty <= 0 ) )
	{
		unit->moving = FALSE;
	}
	// BMPOAMアクターの表示位置に反映
	BmpOam_ActorSetPos( unit->bmpOamActor, unit->param.x, unit->param.y );
}

//------------------------------------------------------------------------------------ 
/**
 * @brief 移動中かどうかを調べる
 *
 * @param unit 調べるユニット
 *
 * @return 移動中なら TRUE
 */
//------------------------------------------------------------------------------------ 
static BOOL CHAR_UNIT_IsMoving( CHAR_UNIT* unit )
{
	return unit->moving;
}


//===================================================================================
// ■地名表示システム
//===================================================================================
struct _FIELD_PLACE_NAME
{ 
	HEAPID heapID;	// ヒープID

  //----------------------
	// 表示に使用するデータ
	GFL_MSGDATA* msg;	     // メッセージ・データ
  GFL_FONT*    font;     // フォント

  //----------------------
	// 地名
	STRBUF* nameBuf;		  // 表示中の地名文字列
	u8      nameLen;	    // 表示中の地名文字数

  //----------------------
	// BG
	GFL_BMPWIN*   bmpWin;	      // ビットマップ・ウィンドウ
	GFL_BMP_DATA* bmpOrg;	      // 文字が書き込まれていない状態
  u32           nullCharPos;  // NULLキャラクタのキャラNo.

  //----------------------
	// OBJ
	u32            resPltt[PLTT_RES_INDEX_MAX];  // パレットリソース
	BMPOAM_SYS_PTR bmpOamSys;                    // BMPOAMシステム
	GFL_CLUNIT*    clunit[CLUNIT_INDEX_MAX];     // セルアクターユニット

  //----------------------
	// 文字ユニット
	CHAR_UNIT charUnit[MAX_NAME_LENGTH];

  //----------------------
	// 動作に使用するデータ
	STATE state;			    // システム状態
	u16	  stateCount;		  // 状態カウンタ
	u32	  currentZoneID;  // 現在表示中の地名に対応するゾーンID
	u32   nextZoneID;		  // 次に表示する地名に対応するゾーンID
	u8    launchUnitNum;  // 発射済み文字数
  u8    writeCharNum;   // ビットマップへの書き込みが完了した文字数
};


//===================================================================================
// ■システムに関する関数
//===================================================================================
// BG
static void SetupBG( FIELD_PLACE_NAME* sys );
static void CleanBG( FIELD_PLACE_NAME* sys );
static void ResetBG( FIELD_PLACE_NAME* sys );	
static void LoadBGPalette( FIELD_PLACE_NAME* sys, u32 arc_id, u32 data_id ); 
static void AllocBGNullCharArea( FIELD_PLACE_NAME* sys ); 
static void AllocBGBitmap( FIELD_PLACE_NAME* sys );
static void RecoverBmpWin( FIELD_PLACE_NAME* sys );
// 文字ユニット
static void LoadClactResource( FIELD_PLACE_NAME* sys ); 
static void CreateClactUnit( FIELD_PLACE_NAME* sys );
static void CreateCharUnit( FIELD_PLACE_NAME* sys );
static void SetupCharUnitAction( FIELD_PLACE_NAME* sys );
static void SetAllCharUnitVisibleOff( FIELD_PLACE_NAME* sys );
static void LaunchCharUnit( FIELD_PLACE_NAME* sys );
static void MoveAllCharUnit( FIELD_PLACE_NAME* sys ); 
// BG, 文字ユニットへの書き込み
static void WritePlaceNameIntoCharUnit( FIELD_PLACE_NAME* sys );
static void WriteCharUnitIntoBitmapWindow( FIELD_PLACE_NAME* sys );
// 地名の更新
static void UpdatePlaceName( FIELD_PLACE_NAME* sys ); 
// 状態の変更
static void SetState( FIELD_PLACE_NAME* sys, STATE next_state );
static void Cancel( FIELD_PLACE_NAME* sys ); 
// 各状態時の動作
static void Process_HIDE( FIELD_PLACE_NAME* sys );
static void Process_SETUP( FIELD_PLACE_NAME* sys );
static void Process_FADE_IN( FIELD_PLACE_NAME* sys );
static void Process_WAIT_LAUNCH( FIELD_PLACE_NAME* sys );
static void Process_LAUNCH( FIELD_PLACE_NAME* sys );
static void Process_WAIT_FADE_OUT( FIELD_PLACE_NAME* sys );
static void Process_FADE_OUT( FIELD_PLACE_NAME* sys ); 
// 各状態時の描画処理
static void Draw_HIDE( FIELD_PLACE_NAME* sys );
static void Draw_SETUP( FIELD_PLACE_NAME* sys );
static void Draw_FADE_IN( FIELD_PLACE_NAME* sys );
static void Draw_WAIT_LAUNCH( FIELD_PLACE_NAME* sys );
static void Draw_LAUNCH( FIELD_PLACE_NAME* sys );
static void Draw_WAIT_FADE_OUT( FIELD_PLACE_NAME* sys );
static void Draw_FADE_OUT( FIELD_PLACE_NAME* sys );


//====================================================================================
// ■作成・破棄
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを作成する
 *
 * @param heap_id 使用するヒープID
 * @param msgbg   使用するメッセージ表示システム
 *
 * @return 地名表示システム・ワークへのポインタ
 */
//------------------------------------------------------------------------------------
FIELD_PLACE_NAME* FIELD_PLACE_NAME_Create( HEAPID heap_id, FLDMSGBG* msgbg )
{
	FIELD_PLACE_NAME* sys;

	// システム・ワーク作成
	sys = (FIELD_PLACE_NAME*)GFL_HEAP_AllocClearMemory( heap_id, sizeof( FIELD_PLACE_NAME ) );

	// システムの設定
	sys->heapID        = heap_id;
	sys->currentZoneID = INVALID_ZONE_ID;
	sys->nextZoneID    = INVALID_ZONE_ID;
	sys->nameLen       = 0;
  sys->writeCharNum  = 0;
  sys->nullCharPos   = AREAMAN_POS_NOTFOUND;
  sys->bmpWin        = NULL;
  sys->bmpOrg        = NULL;
	sys->msg           = FLDMSGBG_CreateMSGDATA( msgbg, NARC_message_place_name_dat );
	sys->font          = FLDMSGBG_GetFontHandle( msgbg );
	sys->nameBuf       = GFL_STR_CreateBuffer( MAX_NAME_LENGTH + 1, sys->heapID );

	// BGの使用準備
  {
    GFL_BG_BGCNT_HEADER bgcnt = 
    {
      0, 0,					          // 初期表示位置
      0x800,						      // スクリーンバッファサイズ
      0,							        // スクリーンバッファオフセット
      GFL_BG_SCRSIZ_256x256,	// スクリーンサイズ
      GX_BG_COLORMODE_16,			// カラーモード
      GX_BG_SCRBASE_0x1000,		// スクリーンベースブロック
      GX_BG_CHARBASE_0x04000,	// キャラクタベースブロック
      GFL_BG_CHRSIZ_256x256,	// キャラクタエリアサイズ
      GX_BG_EXTPLTT_01,			  // BG拡張パレットスロット選択
      1,							        // 表示プライオリティー
      0,							        // エリアオーバーフラグ
      0,							        // DUMMY
      FALSE,						      // モザイク設定
    }; 
    GFL_BG_SetBGControl( BG_FRAME, &bgcnt, GFL_BG_MODE_TEXT );
  }
	//ResetBG( sys );
  //SetupBG( sys );

	// セルアクター用リソースの読み込み
	LoadClactResource( sys );

	// セルアクターユニットを作成
	CreateClactUnit( sys );
	sys->bmpOamSys = BmpOam_Init( heap_id, sys->clunit[ CLUNIT_INDEX_CHAR_UNIT ] );

	// 文字ユニットを作成
	CreateCharUnit( sys );

	// 初期状態を設定
	SetState( sys, STATE_HIDE );

	// 作成したシステムを返す
	return sys;
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを破棄する
 *
 * @param sys 破棄するシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Delete( FIELD_PLACE_NAME* sys )
{
	int i;

	// 文字バッファを解放
	GFL_STR_DeleteBuffer( sys->nameBuf );

	// BMPOAMシステムの後始末
	for( i=0; i<MAX_NAME_LENGTH; i++ )
	{ 
		CHAR_UNIT_Finalize( &sys->charUnit[i] );
	}
	BmpOam_Exit( sys->bmpOamSys );
	
	// セルアクターユニットの破棄
	for( i=0; i<CLUNIT_INDEX_MAX; i++ )
	{
		GFL_CLACT_UNIT_Delete( sys->clunit[i] );
	}
	
	// セルアクター用リソースの破棄
	for( i=0; i<PLTT_RES_INDEX_MAX; i++ )
	{
		GFL_CLGRP_PLTT_Release( sys->resPltt[i] );
	}
	
	// 描画用インスタンスの破棄
	GFL_MSG_Delete( sys->msg );

	// ビットマップ・ウィンドウの破棄
	if( sys->bmpWin ){ GFL_BMPWIN_Delete( sys->bmpWin ); }
	if( sys->bmpOrg ){ GFL_BMP_Delete( sys->bmpOrg ); }

	// BGSYSの後始末
	GFL_BG_FreeBGControl( BG_FRAME );

	// システム・ワーク解放
	GFL_HEAP_FreeMemory( sys );
} 


//====================================================================================
// ■動作
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムの動作処理
 *
 * @param sys 動かすシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Process( FIELD_PLACE_NAME* sys )
{
	// 状態カウンタを更新
	sys->stateCount++;

	// 状態に応じた動作
	switch( sys->state )
	{
  case STATE_HIDE:          Process_HIDE( sys );		       break;
  case STATE_SETUP:         Process_SETUP( sys );		       break;
  case STATE_FADE_IN:       Process_FADE_IN( sys );	       break;
  case STATE_WAIT_LAUNCH:   Process_WAIT_LAUNCH( sys );	   break;
  case STATE_LAUNCH:        Process_LAUNCH( sys );	       break;
  case STATE_WAIT_FADE_OUT: Process_WAIT_FADE_OUT( sys );	 break;
  case STATE_FADE_OUT:      Process_FADE_OUT( sys );	     break;
	}

	// 文字ユニットを動かす
  MoveAllCharUnit( sys );
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示ウィンドウの描画処理
 *
 * @param sys 描画対象システム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Draw( FIELD_PLACE_NAME* sys )
{ 
	switch( sys->state )
	{
  case STATE_HIDE:		        Draw_HIDE( sys );		        break;
  case STATE_SETUP:		        Draw_SETUP( sys );		      break;
  case STATE_FADE_IN:		      Draw_FADE_IN( sys );	      break;
  case STATE_WAIT_LAUNCH:		  Draw_WAIT_LAUNCH( sys );	  break;
  case STATE_LAUNCH:		      Draw_LAUNCH( sys );	        break;
  case STATE_WAIT_FADE_OUT:		Draw_WAIT_FADE_OUT( sys );	break;
  case STATE_FADE_OUT:	      Draw_FADE_OUT( sys );	      break;
	} 
}


//====================================================================================
// ■制御
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief ゾーンの切り替えを通知し, 新しい地名を表示する
 *
 * @param sys        ゾーン切り替えを通知するシステム
 * @param next_zone_id 新しいゾーンID
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Display( FIELD_PLACE_NAME* sys, u32 next_zone_id )
{ 
  // ゾーンの地名表示フラグが立っていない場所では表示しない
  if( ZONEDATA_GetPlaceNameFlag( next_zone_id ) != TRUE ) return;

	// 指定されたゾーンIDを次に表示すべきものとして記憶
	sys->nextZoneID = next_zone_id;

	// 表示中のウィンドウを退出させる
	Cancel( sys ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名ウィンドウを強制的に表示する
 *
 * @param sys   表示するシステム
 * @param zone_id 表示する場所のゾーンID
 */
//------------------------------------------------------------------------------------
extern void FIELD_PLACE_NAME_DisplayForce( FIELD_PLACE_NAME* sys, u32 zone_id )
{
  // 強制的に表示
  sys->currentZoneID = INVALID_ZONE_ID;
  sys->nextZoneID    = zone_id;

	// 表示中のウィンドウを退出させる
	Cancel( sys ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名ウィンドウの表示を強制的に終了する
 *
 * @param sys 表示を終了するシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Hide( FIELD_PLACE_NAME* sys )
{
	SetState( sys, STATE_HIDE );
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示ＢＧ復帰
 *
 * @param sys 表示を終了するシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_RecoverBG(FIELD_PLACE_NAME* sys)
{
  ResetBG( sys );
  GFL_BG_FillCharacterRelease( BG_FRAME, 1, 0 );
  GFL_BG_FillCharacter( BG_FRAME, 0, 1, 0 );
	
  RecoverBmpWin(sys);
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(sys->bmpWin) );
	GFL_BMPWIN_MakeTransWindow( sys->bmpWin );
}


//===================================================================================
// ■非公開関数
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief BGのセットアップ
 *
 * @param sys 地名表示システム
 */
//-----------------------------------------------------------------------------------
static void SetupBG( FIELD_PLACE_NAME* sys )
{ 
  // パレットデータ
	LoadBGPalette( sys, ARCID_PLACE_NAME, NARC_place_name_place_name_back_NCLR );

	// NULLキャラクタデータ
  AllocBGNullCharArea( sys );

  // ビットマップデータ
  AllocBGBitmap( sys );

	// ビットマップ・ウィンドウのデータをVRAMに転送
	GFL_BMPWIN_MakeTransWindow( sys->bmpWin ); 

  // BG表示設定
	GFL_BG_SetPriority( BG_FRAME, BG_FRAME_PRIORITY );
	GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF ); 
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, ALPHA_VALUE_1, ALPHA_VALUE_2 );
}

//-----------------------------------------------------------------------------------
/**
 * @brief BGのクリーンアップ処理
 *
 * @param sys 地名表示システム
 */
//-----------------------------------------------------------------------------------
static void CleanBG( FIELD_PLACE_NAME* sys )
{
	// キャラVRAM・スクリーンVRAMをクリア
	GFL_BG_ClearFrame( BG_FRAME );

	// NULLキャラクタデータ
  if( sys->nullCharPos != AREAMAN_POS_NOTFOUND )
  {
    GFL_BG_FreeCharacterArea( BG_FRAME, sys->nullCharPos, CHAR_SIZE * CHAR_SIZE / 2 );
    sys->nullCharPos = AREAMAN_POS_NOTFOUND;
  }

	// ビットマップ・ウィンドウ
  if( sys->bmpWin )
  {
    GFL_BMPWIN_Delete( sys->bmpWin );
    sys->bmpWin = NULL;
  }

  // キャラクタデータ
  if( sys->bmpOrg )
  {
    GFL_BMP_Delete( sys->bmpOrg );
    sys->bmpOrg = NULL;
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief BGの再設定
 *
 * @param sys 地名表示システム
 */ 
//-----------------------------------------------------------------------------------
static void ResetBG( FIELD_PLACE_NAME* sys )
{ 
  CleanBG( sys );
  SetupBG( sys );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレット・データを読み込む
 *
 * @param sys   読み込みを行うシステム
 * @param arc_id  アーカイブデータのインデックス
 * @param data_id アーカイブ内データインデックス
 */
//------------------------------------------------------------------------------------
static void LoadBGPalette( FIELD_PLACE_NAME* sys, u32 arc_id, u32 data_id )
{
	ARCHANDLE* handle;
	u32 size;
	void* src;
	NNSG2dPaletteData* pltt_data;
	handle = GFL_ARC_OpenDataHandle( arc_id, sys->heapID );		// アーカイブデータハンドルオープン
	size    = GFL_ARC_GetDataSizeByHandle( handle, data_id );	// データサイズ取得
	src   = GFL_HEAP_AllocMemory( sys->heapID, size );				// データバッファ確保
	GFL_ARC_LoadDataByHandle( handle, data_id, src );					// データ取得
	NNS_G2dGetUnpackedPaletteData( src, &pltt_data );					// バイナリからデータを展開
	GFL_BG_LoadPalette( BG_FRAME, pltt_data->pRawData, 0x20, BG_PALETTE_NO );	// パレットデータ転送
	GFL_HEAP_FreeMemory( src );											          // データバッファ解放
	GFL_ARC_CloseDataHandle( handle );										    // アーカイブデータハンドルクローズ
}

//------------------------------------------------------------------------------------
/**
 * @brief 空のBGキャラクタデータを1キャラ分VRAMに確保する
 *
 * @param sys 地名表示システム
 *
 * ※キャラクタ番号0を空白キャラクタとして使用するため。
 */
//------------------------------------------------------------------------------------
static void AllocBGNullCharArea( FIELD_PLACE_NAME* sys )
{
  if( sys->nullCharPos == AREAMAN_POS_NOTFOUND )  // if(未確保)
  {
    sys->nullCharPos = GFL_BG_AllocCharacterArea( 
        BG_FRAME, CHAR_SIZE * CHAR_SIZE / 2, GFL_BG_CHRAREA_GET_F );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief BGビットマップデータを確保する
 *
 * @brief sys 地名表示システム
 */
//------------------------------------------------------------------------------------
static void AllocBGBitmap( FIELD_PLACE_NAME* sys )
{
	// ビットマップ・ウィンドウ
  if( sys->bmpWin == NULL )
  {
    sys->bmpWin = GFL_BMPWIN_Create( BG_FRAME,
                                     BMPWIN_POS_X_CHAR, BMPWIN_POS_Y_CHAR, 
                                     BMPWIN_WIDTH_CHAR, BMPWIN_HEIGHT_CHAR,
                                     BG_PALETTE_NO, GFL_BMP_CHRAREA_GET_F ); 
  } 
  // キャラクタデータ
  if( sys->bmpOrg == NULL )
  {
    sys->bmpOrg = GFL_BMP_LoadCharacter( ARCID_PLACE_NAME, 
                                         NARC_place_name_place_name_back_NCGR, 
                                         FALSE, sys->heapID ); 
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief ビットマップ・ウィンドウをBGオリジナルデータに復元する
 *
 * @param sys 操作対象システム
 */
//-----------------------------------------------------------------------------------
static void RecoverBmpWin( FIELD_PLACE_NAME* sys )
{
	GFL_BMP_DATA* p_src = sys->bmpOrg;
	GFL_BMP_DATA* p_dst = GFL_BMPWIN_GetBmp( sys->bmpWin );

	// オリジナル(文字が書き込まれていない状態)をコピーして, VRAMに転送
	GFL_BMP_Copy( p_src, p_dst );	
	GFL_BMPWIN_TransVramCharacter( sys->bmpWin );	
}

//-----------------------------------------------------------------------------------
/**
 * @brief セルアクターで使用するリソースを読み込む
 *
 * @param sys 読み込み対象システム
 */
//-----------------------------------------------------------------------------------
static void LoadClactResource( FIELD_PLACE_NAME* sys )
{
	ARCHANDLE* p_arc_handle;

	p_arc_handle = GFL_ARC_OpenDataHandle( ARCID_PLACE_NAME, sys->heapID );

	sys->resPltt[ PLTT_RES_INDEX_CHAR_UNIT ] = 
		GFL_CLGRP_PLTT_RegisterEx( 
				p_arc_handle, NARC_place_name_place_name_string_NCLR,
				CLSYS_DRAW_MAIN, FIELDOAM_PALNO_PLACENAME * 32, 0, 1, sys->heapID );

	GFL_ARC_CloseDataHandle( p_arc_handle );
}

//-----------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを作成する
 *
 * @param sys 作成対象システム
 */
//-----------------------------------------------------------------------------------
static void CreateClactUnit( FIELD_PLACE_NAME* sys )
{
	// セルアクターユニットを作成
	sys->clunit[ CLUNIT_INDEX_CHAR_UNIT ] = 
		GFL_CLACT_UNIT_Create( MAX_NAME_LENGTH, BG_FRAME_PRIORITY, sys->heapID );

	// 初期設定
	GFL_CLACT_UNIT_SetDrawEnable( sys->clunit[ CLUNIT_INDEX_CHAR_UNIT ], TRUE );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字ユニットを初期化する
 *
 * @param sys 作成対象システム
 */
//-----------------------------------------------------------------------------------
static void CreateCharUnit( FIELD_PLACE_NAME* sys )
{
	int i;

	for( i=0; i<MAX_NAME_LENGTH; i++ )
	{
		CHAR_UNIT_Initialize( 
				&sys->charUnit[i], sys->bmpOamSys, 
				sys->resPltt[ PLTT_RES_INDEX_CHAR_UNIT ], sys->heapID );
	} 
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字ユニットの動作を設定する
 *
 * @param 設定対象システム
 */
//-----------------------------------------------------------------------------------
static void SetupCharUnitAction( FIELD_PLACE_NAME* sys )
{
	int i;
	int str_width;	// 文字列の幅
	int str_height;	// 文字列の高さ

	// 文字列のサイズを取得
	str_width  = PRINTSYS_GetStrWidth( sys->nameBuf, sys->font, 0 );
  str_height = PRINTSYS_GetStrHeight( sys->nameBuf, sys->font );

	// 各文字ユニットの設定
	{
		CHAR_UNIT_PARAM param;

		// 1文字目の目標パラメータを設定
		param.x   = 256;
		param.y   = Y_CENTER_POS - ( str_height / 2 );
		param.sx  = -20;
		param.sy  =  0;
		//param.dx  = 128 - ( str_width / 2 ) - ( sys->nameLen / 2 );	// 中央揃え
		param.dx  = 24;			// 左詰め
		param.dy  = param.y;
		param.dsx = 0;
		param.dsy = 0;

		// 1文字ずつ設定する
		for( i=0; i<sys->nameLen; i++ )
		{
			// 目標パラメータを設定
			CHAR_UNIT_SetMoveParam( &sys->charUnit[i], &param );

			// 目標パラメータを更新
			param.dx += sys->charUnit[i].width + 1;
		}
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 全文字ユニットを非表示に設定する
 *
 * @param sys 設定対象システム
 */
//-----------------------------------------------------------------------------------
static void SetAllCharUnitVisibleOff( FIELD_PLACE_NAME* sys )
{
	int i;
	
	for( i=0; i<MAX_NAME_LENGTH; i++ )
	{
		BmpOam_ActorSetDrawEnable( sys->charUnit[i].bmpOamActor, FALSE );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字ユニットを発射する
 *
 * @param sys 発射するシステム
 */
//-----------------------------------------------------------------------------------
static void LaunchCharUnit( FIELD_PLACE_NAME* sys )
{
  int i;

  // すでに全てのユニット発射済み
  if( sys->nameLen <= sys->launchUnitNum ) return;

  // 発射
  i = sys->launchUnitNum++;
	CHAR_UNIT_MoveStart( &sys->charUnit[i] );
  BmpOam_ActorSetDrawEnable( sys->charUnit[i].bmpOamActor, TRUE ); // 表示
}

//-----------------------------------------------------------------------------------
/**
 * @brief 全文字ユニットを動かす
 *
 * @param sys 動作対象システム
 */
//-----------------------------------------------------------------------------------
static void MoveAllCharUnit( FIELD_PLACE_NAME* sys )
{
  int i;

	for( i=0; i<sys->nameLen; i++ )
	{
		CHAR_UNIT_Move( &sys->charUnit[i] );
	}
} 

//-----------------------------------------------------------------------------------
/**
 * @brief 現在の地名を文字ユニットに書き込む
 *
 * @param sys   操作対象システム
 */
//----------------------------------------------------------------------------------- 
static void WritePlaceNameIntoCharUnit( FIELD_PLACE_NAME* sys )
{
  int idx;
	const STRCODE* strcode = NULL;

  // すでに書き込みが完了済み
  if( sys->nameLen <= sys->writeCharNum ) return;

  // 生の文字列を取得
  strcode = GFL_STR_GetStringCodePointer( sys->nameBuf );

	// 1文字ずつ設定
  idx = sys->writeCharNum++;
  CHAR_UNIT_Write( &sys->charUnit[idx], sys->font, strcode[idx], sys->heapID ); 
	sys->charUnit[idx].param.x = 256;  // 画面外へ
}

//-----------------------------------------------------------------------------------
/**
 * @brief 表示中の文字ユニットをビットマップ・ウィンドウに書き込む
 *
 * @param sys 操作対象システム
 */
//-----------------------------------------------------------------------------------
static void WriteCharUnitIntoBitmapWindow( FIELD_PLACE_NAME* sys )
{
	int i;
	u16 dx, dy;
	CHAR_UNIT* p_char;
	GFL_BMP_DATA* p_src_bmp = NULL;
	GFL_BMP_DATA* p_dst_bmp = GFL_BMPWIN_GetBmp( sys->bmpWin );

	// 表示中の全ての文字ユニットをコピーする
	for( i=0; i<sys->nameLen; i++ )
	{
		p_char = &sys->charUnit[i];
		p_src_bmp = p_char->bmp;
		dx = p_char->param.x - ( BMPWIN_POS_X_CHAR * CHAR_SIZE );
		dy = p_char->param.y - ( BMPWIN_POS_Y_CHAR * CHAR_SIZE );
		GFL_BMP_Print( p_src_bmp, p_dst_bmp, 0, 0, dx, dy, p_char->width, p_char->height, 0 );
	}

	// 更新されたキャラデータをVRAMに転送
	GFL_BMPWIN_TransVramCharacter( sys->bmpWin );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 表示する地名を更新する
 *
 * @param sys 更新するシステム
 */
//-----------------------------------------------------------------------------------
static void UpdatePlaceName( FIELD_PLACE_NAME* sys )
{
	u16 str_id;
  
  // 地名が変わらない
  if( sys->currentZoneID == sys->nextZoneID ){ return; }

	// ゾーンIDから地名文字列を取得する
	str_id = ZONEDATA_GetPlaceNameID( sys->nextZoneID );

  // エラー回避
	if( (str_id < 0) || (msg_place_name_max <= str_id) ){ str_id = 0; } 
	if( str_id == 0 ) 
  { //「なぞのばしょ」なら表示しない
    OBATA_Printf( "「なぞのばしょ」を検出( zone id = %d )\n", sys->nextZoneID );
    FIELD_PLACE_NAME_Hide( sys );
  }

  // 表示中のゾーンID, 地名を更新
  sys->currentZoneID = sys->nextZoneID;	   
	GFL_MSG_GetString( sys->msg,	str_id, sys->nameBuf );
	sys->nameLen = GFL_STR_GetBufferLength( sys->nameBuf );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param sys      状態を変更するシステム
 * @param next_state 設定する状態
 */
//-----------------------------------------------------------------------------------
static void SetState( FIELD_PLACE_NAME* sys, STATE next_state )
{
	// 状態を変更し, 状態カウンタを初期化する
	sys->state      = next_state;
	sys->stateCount = 0;

	// 遷移先の状態に応じた初期化
	switch( next_state )
	{
		case STATE_HIDE:	
			GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF );	// BGを非表示
			SetAllCharUnitVisibleOff( sys );			      // 文字ユニットを非表示に
			break;
    case STATE_SETUP:
      UpdatePlaceName( sys );          // 表示する地名を更新
      sys->writeCharNum = 0;          // 書き込み準備
      FIELD_PLACE_NAME_RecoverBG( sys );  // キャラ・スクリーンデータ復帰
      break;
		case STATE_FADE_IN:
      GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON );	// BGを表示
      Draw_FADE_IN( sys );
			break;
		case STATE_WAIT_LAUNCH:
			break;
		case STATE_LAUNCH:
			sys->launchUnitNum = 0;	     // 発射文字数をリセット
			SetupCharUnitAction( sys );	 // 文字ユニットの動きをセット
			break;
		case STATE_WAIT_FADE_OUT:
			break;
		case STATE_FADE_OUT:
			WriteCharUnitIntoBitmapWindow( sys );	// 現時点での文字をBGに書き込む
			SetAllCharUnitVisibleOff( sys );		// 文字ユニットを非表示に
			break;
	}

  // DEBUG:
  OBATA_Printf( "PLACE_NAME: set state ==> " );
  switch( next_state )
  {
  case STATE_HIDE:           OBATA_Printf( "HIDE\n" );           break;
  case STATE_SETUP:          OBATA_Printf( "SETUP\n" );          break;
  case STATE_FADE_IN:        OBATA_Printf( "FADE_IN\n" );        break;
  case STATE_WAIT_LAUNCH:    OBATA_Printf( "WAIT_LAUNCH\n" );    break;
  case STATE_LAUNCH:         OBATA_Printf( "LAUNCH\n" );         break;
  case STATE_WAIT_FADE_OUT:  OBATA_Printf( "WAIT_FADE_OUT\n" );  break;
  case STATE_FADE_OUT:       OBATA_Printf( "FADE_OUT\n" );       break;
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief 強制的にウィンドウを退出させる
 *
 * @param sys 状態を更新するシステム
 */
//-----------------------------------------------------------------------------------
static void Cancel( FIELD_PLACE_NAME* sys )
{
	STATE next_state;
	int   start_count;
	float passed_rate;
	
	// 現在の状態に応じた処理
	switch( sys->state )
	{
  case STATE_HIDE:	
  case STATE_FADE_OUT:
    return;
  case STATE_WAIT_LAUNCH:
  case STATE_WAIT_FADE_OUT:
  case STATE_LAUNCH:
    next_state  = STATE_WAIT_FADE_OUT;
    start_count = PROCESS_TIME_WAIT_FADE_OUT;
    break;
  case STATE_FADE_IN:
    next_state = STATE_FADE_OUT;
    // 経過済みフレーム数を算出
    // (stateCountから, 表示位置を算出しているため, 
    //  強制退出させた場合は stateCount を適切に計算する必要がある)
    passed_rate = sys->stateCount / (float)PROCESS_TIME_FADE_IN;
    start_count = (int)( (1.0f - passed_rate) * PROCESS_TIME_FADE_OUT );
    break;
  case STATE_SETUP:
    next_state = STATE_HIDE;
    start_count = 0;
    break;
	}

	// 状態を更新
	SetState( sys, next_state );
	sys->stateCount = start_count;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 非表示状態時の動作
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_HIDE( FIELD_PLACE_NAME* sys )
{
	// ゾーンチェンジが通知されたら, 次の状態へ
	if( sys->currentZoneID != sys->nextZoneID )
	{
		SetState( sys, STATE_SETUP );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 準備状態時の動作
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_SETUP( FIELD_PLACE_NAME* sys )
{
  // 新しい地名を書き込む
  WritePlaceNameIntoCharUnit( sys );

  // 書き込みが完了したら, 次の状態へ
  if( sys->nameLen <= sys->writeCharNum )
  {
    SetState( sys, STATE_FADE_IN );
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・イン状態時の動作
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_FADE_IN( FIELD_PLACE_NAME* sys )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_FADE_IN < sys->stateCount )
	{
		SetState( sys, STATE_WAIT_LAUNCH );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 発射待ち状態時の動作
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT_LAUNCH( FIELD_PLACE_NAME* sys )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_WAIT_LAUNCH < sys->stateCount )
	{
		SetState( sys, STATE_LAUNCH );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 発射状態時の動作
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_LAUNCH( FIELD_PLACE_NAME* sys )
{
	// 一定間隔で文字を発射
	if( sys->stateCount % LAUNCH_INTERVAL == 0 )
	{
		// 発射
		LaunchCharUnit( sys );

		// すべての文字を発射したら, 次の状態へ
		if( sys->nameLen <= sys->launchUnitNum )
		{
			SetState( sys, STATE_WAIT_FADE_OUT );
		}
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェードアウト待ち状態時の動作
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT_FADE_OUT( FIELD_PLACE_NAME* sys )
{
	int i;

	// 一定時間が経過
	if( PROCESS_TIME_WAIT_FADE_OUT < sys->stateCount )
	{
		// 動いている文字があるなら, 待機状態を維持
		for( i=0; i<sys->nameLen; i++ )
		{
			if( CHAR_UNIT_IsMoving( &sys->charUnit[i] ) == TRUE ) break;
		}

		// 次の状態へ
		SetState( sys, STATE_FADE_OUT );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・アウト状態時の動作
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_FADE_OUT( FIELD_PLACE_NAME* sys )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_FADE_OUT < sys->stateCount )
	{
		SetState( sys, STATE_HIDE );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 非表示状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_HIDE( FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 準備状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_SETUP( FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・イン状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_FADE_IN( FIELD_PLACE_NAME* sys )
{
	int val1, val2;
	float rate;

	// αブレンディング係数を更新
	rate  = (float)sys->stateCount / (float)PROCESS_TIME_FADE_IN;
	val1 = (int)( ALPHA_VALUE_1 * rate );
	val2 = (int)( ALPHA_VALUE_2 + (16 - ALPHA_VALUE_2) * (1.0f - rate) );
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, val1, val2 );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_WAIT_LAUNCH( FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字発射状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_LAUNCH( FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_WAIT_FADE_OUT( FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・アウト状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_FADE_OUT( FIELD_PLACE_NAME* sys )
{
	int val1, val2;
	float rate;
	
	// αブレンディング係数を更新
	rate  = (float)sys->stateCount / (float)PROCESS_TIME_FADE_OUT;
	val1 = (int)( ALPHA_VALUE_1 * (1.0f - rate) );
	val2 = (int)( ALPHA_VALUE_2 + (16 - ALPHA_VALUE_2) * rate );
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, val1, val2 );
} 
