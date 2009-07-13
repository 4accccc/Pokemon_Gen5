////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   field_place_name.c
 * @brief  地名表示ウィンドウ
 * @author obata_toshihiro
 * @date   2009.07.08
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "field_place_name.h"
#include "field/zonedata.h"
#include "arc/arc_def.h"
#include "arc/area_win_gra.naix"
#include "arc/message.naix"
#include "msg/msg_place_name.h"


//===================================================================================
/**
 * 定数
 */
//===================================================================================

// 1キャラ = 8ドット
#define CHAR_SIZE (8) 

//-----------
// 表示設定
//-----------
#define PALETTE_NO          (0)					// パレット番号
#define BG_FRAME            (GFL_BG_FRAME2_M)	// 使用するBGフレーム
#define BG_FRAME_PRIORITY   (1)					// BGフレームのプライオリティ
#define	COLOR_NO_LETTER     (1)		            // 文字本体のカラー番号
#define	COLOR_NO_SHADOW     (2)		            // 影部分のカラー番号
#define	COLOR_NO_BACKGROUND (0)					// 背景部のカラー番号

//--------------------
// アーカイブ・データ
//--------------------
#define	ARC_DATA_ID_MAX (26) // アーカイブ内データIDの最大値

//----------------------
// グラフィック・サイズ
//----------------------
// ウィンドウのキャラクタ・データ
#define WIN_CGR_WIDTH_CHAR (32)							     // 幅  (キャラクタ数)
#define WIN_CGR_HEIGHT_CHAR (4)							     // 高さ(キャラクタ数)
#define WIN_CGR_WIDTH_DOT  (WIN_CGR_WIDTH_CHAR * CHAR_SIZE)	 // 幅  (ドット数)
#define WIN_CGR_HEIGHT_DOT (WIN_CGR_HEIGHT_CHAR * CHAR_SIZE) // 高さ(ドット数)

// ビットマップ・ウィンドウ
#define BMPWIN_WIDTH_CHAR  (17)							     // 幅  (キャラクタ数)
#define BMPWIN_HEIGHT_CHAR  (4)								 // 高さ(キャラクタ数)
#define BMPWIN_WIDTH_DOT   (BMPWIN_WIDTH_CHAR * CHAR_SIZE)   // 幅  (ドット数)
#define BMPWIN_HEIGHT_DOT  (BMPWIN_HEIGHT_CHAR * CHAR_SIZE)  // 高さ(ドット数)

//------------------
// 各状態の動作時間
//------------------
#define PROCESS_TIME_APPEAR    (10)	// 表示状態
#define PROCESS_TIME_WAIT      (60) // 待機状態
#define PROCESS_TIME_DISAPPEAR (10) // 退出状態

//-----------------
// システムの状態
//-----------------
typedef enum
{
	FIELD_PLACE_NAME_STATE_HIDE,		// 非表示
	FIELD_PLACE_NAME_STATE_APPEAR,		// 出現中
	FIELD_PLACE_NAME_STATE_WAIT,		// 待機(表示)中
	FIELD_PLACE_NAME_STATE_DISAPPEAR,	// 退出中
	FIELD_PLACE_NAME_STATE_MAX,
}
FIELD_PLACE_NAME_STATE;



//===================================================================================
/**
 * システム・ワーク
 */
//===================================================================================
struct _FIELD_PLACE_NAME
{ 
	HEAPID heapID;	// ヒープID

	// 地名の表示に使用するデータ
	FLDMSGBG*    pFldMsgBG;			// BMPWINへの文字列書き込みに使用するシステム( 外部インスタンスを参照 )
	GFL_MSGDATA* pMsgData;			// メッセージ・データ( インスタンスを保持 )
	GFL_BMPWIN*  pBmpWin;			// ビットマップ・ウィンドウ( インスタンスを保持 )
	FLDMSGPRINT* pFldMsgPrint;		// BMPWIN - MSGDATA の関連付け( インスタンスを保持 )
	
	// 動作に使用するデータ
	FIELD_PLACE_NAME_STATE state;			// 状態
	u16	                   stateCount;		// 状態カウンタ
	u32	                   currentZoneID;	// 現在表示中の地名に対応するゾーンID
	u32                    nextZoneID;		// 次に表示する地名に対応するゾーンID
};


//===================================================================================
/**
 * 非公開関数のプロトタイプ宣言
 */
//===================================================================================
// 初期化関数
static void SetupBG( FIELD_PLACE_NAME* p_sys );	// BGの設定を行う
// リソースの読み込み
static void LoadPaletteData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id );
static void LoadCharacterData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id );
static void LoadScreenData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id );
// 指定したゾーンIDに対応するビットマップウィンドウを作成する
static void CreateBitmapWindow( FIELD_PLACE_NAME* p_sys, u32 zone_id );
static void WriteStringToWindow( FIELD_PLACE_NAME* p_sys, u32 zone_id );
// 状態を変更する
static void SetState( FIELD_PLACE_NAME* p_sys, FIELD_PLACE_NAME_STATE next_state );
// 各状態時の動作
static void Process_HIDE( FIELD_PLACE_NAME* p_sys );
static void Process_APPEAR( FIELD_PLACE_NAME* p_sys );
static void Process_WAIT( FIELD_PLACE_NAME* p_sys );
static void Process_DISAPPEAR( FIELD_PLACE_NAME* p_sys );


//===================================================================================
/**
 * 公開関数の実装( システムの稼動に関する関数 )
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

	// ビットマップ・ウィンドウ作成
	p_sys->pBmpWin = GFL_BMPWIN_Create( BG_FRAME, 0, 0, BMPWIN_WIDTH_CHAR, BMPWIN_HEIGHT_CHAR, 0, GFL_BMP_CHRAREA_GET_B );

	// メッセージ・データを作成
	p_sys->pMsgData = FLDMSGBG_CreateMSGDATA( p_sys->pFldMsgBG, NARC_message_place_name_dat );

	// メッセージ描画情報を作成
	p_sys->pFldMsgPrint = FLDMSGPRINT_SetupPrint( p_sys->pFldMsgBG, p_sys->pMsgData, p_sys->pBmpWin );


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
	// 状態カウンタを更新
	p_sys->stateCount++;

	// 状態に応じた動作を行う
	switch( p_sys->state )
	{
		case FIELD_PLACE_NAME_STATE_HIDE:		Process_HIDE( p_sys );		break;
		case FIELD_PLACE_NAME_STATE_APPEAR:		Process_APPEAR( p_sys );	break;
		case FIELD_PLACE_NAME_STATE_WAIT:		Process_WAIT( p_sys );		break;
		case FIELD_PLACE_NAME_STATE_DISAPPEAR:	Process_DISAPPEAR( p_sys );	break;
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
	int scroll_value = 0;

	// スクロール値を計算
	switch( p_sys->state )
	{
		case FIELD_PLACE_NAME_STATE_HIDE:	
			scroll_value = BMPWIN_HEIGHT_DOT;
			break;
		case  FIELD_PLACE_NAME_STATE_APPEAR:	
			scroll_value = BMPWIN_HEIGHT_DOT - (int)( BMPWIN_HEIGHT_DOT * ( ( (float)( p_sys->stateCount ) ) / PROCESS_TIME_APPEAR ) );
			break;
		case FIELD_PLACE_NAME_STATE_WAIT:
			scroll_value = 0;	
			break;
		case FIELD_PLACE_NAME_STATE_DISAPPEAR:	
			scroll_value = (int)( BMPWIN_HEIGHT_DOT * ( ( (float)( p_sys->stateCount ) ) / PROCESS_TIME_DISAPPEAR ) );	
			break;
	}
	
	// スクロール値を設定
	GFL_BG_SetScroll( BG_FRAME, GFL_BG_SCROLL_Y_SET, scroll_value );

	// デバッグ出力
	/*
	{
		char* state;
		switch( p_sys->state )
		{
			case FIELD_PLACE_NAME_STATE_HIDE:		state = "HIDE";	break;
			case FIELD_PLACE_NAME_STATE_APPEAR:		state = "APPEAR";	break;
			case FIELD_PLACE_NAME_STATE_WAIT:		state = "WAIT";	break;
			case FIELD_PLACE_NAME_STATE_DISAPPEAR:	state = "DISAPPER";	break;
			default:								state = "error"; break;
		}
		OBATA_Printf( "state = %s,  stateCount = %d\n", state, p_sys->stateCount );
	}
	*/
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
	// 描画用インスタンスの破棄
	FLDMSGPRINT_Delete( p_sys->pFldMsgPrint );
	GFL_MSG_Delete( p_sys->pMsgData );
	GFL_BMPWIN_Delete( p_sys->pBmpWin );

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
void FIELD_PLACE_NAME_ZoneChange( FIELD_PLACE_NAME* p_sys, u32 next_zone_id )
{
	// 指定されたゾーンIDを次に表示すべきものとして記憶
	p_sys->nextZoneID = next_zone_id;

	// 出現中 or 待機中 なら, 強制的に退出させる
	if( ( p_sys->state == FIELD_PLACE_NAME_STATE_APPEAR ) ||
		( p_sys->state == FIELD_PLACE_NAME_STATE_WAIT ) )
	{
		SetState( p_sys, FIELD_PLACE_NAME_STATE_DISAPPEAR ); 
	}
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
	SetState( p_sys, FIELD_PLACE_NAME_STATE_HIDE );
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
	GFL_BMP_DATA* p_win_bmp;
	GFL_BMP_DATA* p_name_bmp;
	GFL_BMP_DATA* p_combine;
	void* p_char_adrs;

	GFL_BG_BGCNT_HEADER bgcnt = 
	{
		0, 0,				    // 初期表示位置
		0x800,					// スクリーンバッファサイズ
		0,						// スクリーンバッファオフセット
		GFL_BG_SCRSIZ_256x256,	// スクリーンサイズ
		GX_BG_COLORMODE_16,		// カラーモード
		GX_BG_SCRBASE_0x0800,	// スクリーンベースブロック
		GX_BG_CHARBASE_0x04000,	// キャラクタベースブロック
		GFL_BG_CHRSIZ_256x256,	// キャラクタエリアサイズ
		GX_BG_EXTPLTT_01,		// BG拡張パレットスロット選択
		1,						// 表示プライオリティー
		0,						// エリアオーバーフラグ
		0,						// DUMMY
		FALSE,					// モザイク設定
	};

	GFL_BG_SetBGControl( BG_FRAME, &bgcnt, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( BG_FRAME, BG_FRAME_PRIORITY );
	GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON ); 
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
static void LoadPaletteData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id )
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
	GFL_BG_LoadPalette( BG_FRAME, p_palette->pRawData, 0x20, PALETTE_NO );	// パレットデータ転送
	GFL_HEAP_FreeMemory( p_src );											// データバッファ解放
	GFL_ARC_CloseDataHandle( p_h_arc );										// アーカイブデータハンドルクローズ
}

//------------------------------------------------------------------------------------
/**
 * @brief キャラクタ・データをビットマップ・ウィンドウの内部バッファに読み込む
 *
 * @param p_sys   読み込みを行うシステム
 * @param arc_id  アーカイブデータのインデックス
 * @param data_id アーカイブ内データインデックス
 */
//------------------------------------------------------------------------------------
static void LoadCharacterData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id )
{
	int i;
	GFL_BMP_DATA* p_src;	// 転送元ビットマップ
	GFL_BMP_DATA* p_bmp;	// 転送先ビットマップ
	int x_src, y_src;		// 転送元座標
	int x_dest, y_dest;		// 転送先座標
	int copy_char_num;		// 転送キャラクタ数

	// 転送先ビットマップ = ビットマップウィンドウ
	p_bmp = GFL_BMPWIN_GetBmp( p_sys->pBmpWin );

	// 転送元ビットマップを作成( アーカイブからキャラデータを取得 )
	p_src = GFL_BMP_LoadCharacter( arc_id, data_id, 0, p_sys->heapID );

	// 読み込んだキャラデータを, ビットマップウィンドウに転送
	x_src  = CHAR_SIZE;
	y_src  = 0;
	x_dest = 0;
	y_dest = 0;
	copy_char_num = (WIN_CGR_WIDTH_CHAR * WIN_CGR_HEIGHT_CHAR) - 1;		// 先頭キャラ以外をコピーする
	for( i=0; i<copy_char_num; i++ )
	{
		// 1キャラずつコピー
		GFL_BMP_Print( p_src, p_bmp, x_src, y_src, x_dest, y_dest, CHAR_SIZE, CHAR_SIZE, 0 );

		// コピー元座標を更新
		x_src += CHAR_SIZE;
		if( WIN_CGR_WIDTH_DOT <= x_src  )
		{
			x_src  = 0;
			y_src += CHAR_SIZE;
		}
		// コピー先座標を更新
		x_dest += CHAR_SIZE;
		if( BMPWIN_WIDTH_DOT <= x_dest )
		{
			x_dest  = 0;
			y_dest += CHAR_SIZE;
		}
	}
	
	// 読み込んだキャラデータを破棄
	GFL_BMP_Delete( p_src ); 
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
static void LoadScreenData( FIELD_PLACE_NAME* p_sys, u32 arc_id, u32 data_id )
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

//-----------------------------------------------------------------------------------
/**
 * @brief 指定したゾーンIDに対応するビットマップウィンドウを作成する
 *
 * @param p_sys   操作対象システム
 * @param zone_id ゾーンID
 */
//----------------------------------------------------------------------------------- 
static void CreateBitmapWindow( FIELD_PLACE_NAME* p_sys, u32 zone_id )
{ 
	u16 win_id;

	// キャラクタ・パレットの書き換え
	win_id = ZONEDATA_GetPlaceNameWinID( zone_id );			// ウィンドウIDを取得
	if( win_id < 0 | ARC_DATA_ID_MAX < win_id ) win_id = 0; // ウィンドウIDの範囲チェック
	LoadCharacterData( p_sys, ARCID_PLACE_NAME, win_id );	// ビットマップ・ウィンドウの内部バッファにキャラクタ・データを転送
	LoadPaletteData( p_sys, ARCID_PLACE_NAME, win_id + 1 ); // パレット転送

	// ビットマップ・ウィンドウの内部バッファに文字を書き込む
	WriteStringToWindow( p_sys, zone_id );

	// ビットマップ・ウィンドウのデータをVRAMに転送
	GFL_BMPWIN_TransVramCharacter( p_sys->pBmpWin );	// VRAMにキャラデータを転送	
	GFL_BMPWIN_MakeScreen( p_sys->pBmpWin );			// BGSYSの内部バッファを更新
	GFL_BG_LoadScreenReq( BG_FRAME );					// BGSYSの内部バッファをVRAMに転送


	// デバッグ出力
	OBATA_Printf( "win_id = %d\n", (int)win_id );
}

//-----------------------------------------------------------------------------------
/**
 * @brief ビットマップ・ウィンドウに対し, 地名の書き込みを行う
 *
 * @param p_sys   操作対象システム
 * @param zone_id ゾーンID
 */
//----------------------------------------------------------------------------------- 
static void WriteStringToWindow( FIELD_PLACE_NAME* p_sys, u32 zone_id )
{
	u16 str_id;
	int str_width;
	int str_x;
	STRBUF* p_str_buf = NULL;
	GFL_FONT* p_font = NULL;

	str_id    = ZONEDATA_GetPlaceNameID( zone_id );				// メッセージ番号を決定
	if( str_id < 0 | msg_place_name_max <= str_id ) str_id = 0;	// メッセージ番号の範囲チェック
	p_str_buf = GFL_STR_CreateBuffer( 256, p_sys->heapID );		// バッファを作成
	GFL_MSG_GetString( p_sys->pMsgData,	str_id, p_str_buf );	// 地名文字列を取得
	p_font    = FLDMSGBG_GetFontHandle( p_sys->pFldMsgBG );		// フォント情報を取得
	str_width = PRINTSYS_GetStrWidth( p_str_buf, p_font, 0 );	// 文字列の幅を取得( 文字間隔はデフォルトで0 )
	str_x     = ( BMPWIN_WIDTH_DOT / 2 ) - ( str_width / 2 );	// 文字列の左端x座標を算出

	// 書き込み
	FLDMSGPRINT_PrintColor( 
			p_sys->pFldMsgPrint, str_x, 1*CHAR_SIZE+2, str_id, 
			PRINTSYS_LSB_Make( COLOR_NO_LETTER, COLOR_NO_SHADOW, COLOR_NO_BACKGROUND ) );

	// 後始末
	GFL_STR_DeleteBuffer( p_str_buf );

	// デバッグ出力
	OBATA_Printf( "BMPWIN_WIDTH_DOT / 2 = %d\n", BMPWIN_WIDTH_DOT / 2 );
	OBATA_Printf( "str_width / 2 = %d\n", str_width / 2 );
	OBATA_Printf( "str_x = %d\n", str_x );
	OBATA_Printf( "str_id = %d\n", (int)str_id );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param p_sys      状態を変更するシステム
 * @param next_state 設定する状態
 */
//-----------------------------------------------------------------------------------
static void SetState( FIELD_PLACE_NAME* p_sys, FIELD_PLACE_NAME_STATE next_state )
{
	u16 start_count = 0;

	// 出現状態 -> 退出状態に移行する場合, 状態カウントを適切に計算する必要がある
	// ( stateCount から ウィンドウの表示位置を計算しているため )
	if( ( p_sys->state == FIELD_PLACE_NAME_STATE_APPEAR ) &&
		( next_state   == FIELD_PLACE_NAME_STATE_DISAPPEAR ) )
	{
		float rate  = p_sys->stateCount / (float)PROCESS_TIME_APPEAR;	// 経過した時間の割合を算出
		start_count = (int)( (1.0f - rate) * PROCESS_TIME_DISAPPEAR );	// 退出状態開始時のカウントを算出
	}
	
	// 状態を変更し, 状態カウンタを初期化する
	p_sys->state      = next_state;
	p_sys->stateCount = start_count;

	// 出現状態に移行する時は, 表示中のゾーンIDを更新する
	if( next_state == FIELD_PLACE_NAME_STATE_APPEAR )
	{
		p_sys->currentZoneID = p_sys->nextZoneID; 
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 非表示状態( FIELD_PLACE_NAME_STATE_HIDE )時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_HIDE( FIELD_PLACE_NAME* p_sys )
{
	// ゾーンの切り替え通知を受けていたら, 出現状態に移行する
	if( p_sys->currentZoneID != p_sys->nextZoneID )
	{
		// 出現状態に移行し,
		SetState( p_sys, FIELD_PLACE_NAME_STATE_APPEAR );

		// ビットマップウィンドウを構築
		CreateBitmapWindow( p_sys, p_sys->currentZoneID );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 出現状態( FIELD_PLACE_NAME_STATE_APPEAR )時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_APPEAR( FIELD_PLACE_NAME* p_sys )
{
	// 一定時間が経過したら, 待機状態へ移行する
	if( PROCESS_TIME_APPEAR < p_sys->stateCount )
	{
		SetState( p_sys, FIELD_PLACE_NAME_STATE_WAIT );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機( FIELD_PLACE_NAME_STATE_WAIT )時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT( FIELD_PLACE_NAME* p_sys )
{
	// 一定時間が経過したら, 退出状態へ移行する
	if( PROCESS_TIME_WAIT < p_sys->stateCount )
	{
		SetState( p_sys, FIELD_PLACE_NAME_STATE_DISAPPEAR );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 退出状態( FIELD_PLACE_NAME_STATE_DISAPPEAR )時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_DISAPPEAR( FIELD_PLACE_NAME* p_sys )
{
	// 一定時間が経過したら, 非表示状態へ移行する
	if( PROCESS_TIME_DISAPPEAR < p_sys->stateCount )
	{
		SetState( p_sys, FIELD_PLACE_NAME_STATE_HIDE );
	}
}
