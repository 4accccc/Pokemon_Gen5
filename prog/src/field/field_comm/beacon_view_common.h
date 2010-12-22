/*
 *  @file   beacon_view_common.c
 *  @brief  すれ違いログ＆フェスミッションログ画面　共有サブルーチン群 ヘッダ
 *  @author Miyuki Iwasawa
 *  @date   10.10.20
 */

#pragma once

typedef struct _POINT{
  int x,y;
}POINT;

typedef struct _TRIANGLE{
  POINT p[3];
}TRIANGLE;

/*
 *  @brief  三角形あたり判定
 *
 *  三角形は反時計回り
 * 1-0 p-a, b-a
 * 2-1 p-b, c-b
 * 0-2 p-c, a-c
 */
extern BOOL BVCCalc_PointInTriangle( POINT* p, TRIANGLE* tri );

/*
 *  @brief  SE再生
 */
extern void BVCSound_PlaySE( u16 se_no );

/*
 *  @brief  アクター　座標セット
 */
extern void BVCAct_SetPosition( GFL_CLWK* act, s16 x, s16 y );

/*
 *  @brief  アクター アニメスタート
 */
extern void BVCAct_AnmStart( GFL_CLWK* act, u8 anm_no );

/*
 *  @brief  特殊ポップアップ起動チェック 
 */
extern BOOL BVC_CheckSpecialPopup( BEACON_VIEW_PTR wk );

/*
 *  @brief  アクティブ・パッシブ切替
 */
extern void BVC_SetViewPassive( BEACON_VIEW_PTR wk, BOOL passive_f );

/*
 *  @brief  プリントキュー　処理スピードアップリクエスト
 *
 *  処理分割数を減らし、処理負荷を上げる代わりに、
 *  プリント処理終了までのスピードをアップする
 */
extern void BVC_PrintQueLimmitUpSet( BEACON_VIEW_PTR wk, BOOL flag );

/*
 *  @brief  メニューバーアニメセット
 */
extern void BVC_MenuBarViewSet( BEACON_VIEW_PTR wk, MENU_ID id, MENU_STATE state );

/*
 *  @brief  メニューバーアニメウェイト
 */
extern BOOL BVC_MenuBarCheckAnm( BEACON_VIEW_PTR wk, MENU_ID id );

/*
 *  @brief  メニューバーあたり判定
 */
extern int BVCTouchIn_CheckMenu( BEACON_VIEW_PTR wk, POINT* tp );

/*
 *  @brief  サブシーケンス　GPower使用メイン
 */
extern BOOL BVCSubSeq_GPower( BEACON_VIEW_PTR wk );


//========================================================================
//選択メニュー処理系
//========================================================================
/*
 *  @brief  TaskMenu YesNo生成
 */
extern void BVCTMenu_Create( BEACON_VIEW_PTR wk, u8 menu_idx );

/*
 *  @brief  TaskMenu アップデート
 */
extern int BVCTMenu_Update( BEACON_VIEW_PTR wk,u8 menu_idx );

/*
 *  @brief  TaskMenu 終了アニメ待ち
 */
extern BOOL BVCTMenu_YnEndWait( BEACON_VIEW_PTR wk, u8 select );

/*
 *  @brief  TaskMenu 終了
 */
extern void BVCTMenu_YnEnd( BEACON_VIEW_PTR wk );

/*
 *  @brief  TaskMenu Gパワー確認ダイアログ終了アニメ待ち
 */
extern BOOL BVCTMenu_CheckEndWait( BEACON_VIEW_PTR wk, u8 select );

/*
 *  @brief  TaskMenu 終了
 */
extern void BVCTMenu_CheckEnd( BEACON_VIEW_PTR wk );

  
//========================================================================
//文字列処理系
//========================================================================
/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
extern void BVCPrint_GetMsgToBufEx( BEACON_VIEW_PTR wk, u8 msg_id, const GFL_MSGDATA* handle );

/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
extern void BVCPrint_GetMsgToBuf( BEACON_VIEW_PTR wk, u8 msg_id );

/*
 *  @brief  ポップアップウィンドウ文字列描画
 */
extern void BVCPrint_PopupWindow( BEACON_VIEW_PTR wk, STRBUF* str, int* task_ct );

//------------------------------------------------------------
/*
 *  @brief  ポップアップ　タイムウェイトチェック
 */
//------------------------------------------------------------
extern BOOL BVCPrint_PopupWindowTimeWaitCheck( u8* p_wait );

//------------------------------------------------------------
/**
 * ビーコン情報から表示するメッセージIDを取得する
 *
 * @param   info		対象のビーコン情報へのポインタ
 */
//------------------------------------------------------------
extern u32 BVCPrint_GetBeaconMsgID(const GAMEBEACON_INFO *info);

/*
 *  @brief  メニューウィンドウ表示
 */
extern void BVCDraw_MenuWindow( BEACON_VIEW_PTR wk, u8 msg_id, int* task_ct );

/*
 *  @brief  メニューウィンドウ表示クリア
 */
extern void BVCDraw_ClearMenuWindow( BEACON_VIEW_PTR wk );

/*
 *  @brief  メニューアクター Viewセット
 */
extern void BVCDraw_MenuIconVisibleSet( BEACON_VIEW_PTR wk, BOOL visible_f );



/////////////////////////////////////////////////////////////////////
/*
 *  @brief  BMPウィンドウメッセージ表示キュー処理 
 */
/////////////////////////////////////////////////////////////////////
extern void BVCPrintReq_BmpwinPrintEx( BEACON_VIEW_PTR wk, BMP_WIN* win, STRBUF* str, u8 x, u8 y, u8 clear, PRINTSYS_LSB color, int* task_ct, BOOL clear_f );
extern void BVCPrintReq_BmpwinPrint( BEACON_VIEW_PTR wk, BMP_WIN* win, STRBUF* str, u8 x, u8 y, u8 clear, PRINTSYS_LSB color, int* task_ct );

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ アップダウン 
 */
/////////////////////////////////////////////////////////////////////
extern void BVCTaskAdd_MsgUpdown( BEACON_VIEW_PTR wk, u8 dir, int* task_ct );

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ ポップアップ 
 */
/////////////////////////////////////////////////////////////////////
extern void BVCEffReq_PopupMsgFromInfo( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info );
extern void BVCEffReq_PopupMsgSys( BEACON_VIEW_PTR wk, u8 msg_id );

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ Gパワー使用確認 
 */
/////////////////////////////////////////////////////////////////////
extern BOOL BVCEffReq_PopupMsgGPower( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, BOOL haifu_f );
extern void BVCEffReq_PopupMsgGPowerMine( BEACON_VIEW_PTR wk );
extern void BVCEffReq_PopupMsgGPowerSpecial( BEACON_VIEW_PTR wk, GPOWER_ID sp_gpower_id );


