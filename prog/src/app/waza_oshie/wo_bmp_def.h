/**
 *  @file wo_bmp_def.h
 *  @brief  技教え画面リテラル定義
 *  @author Miyuki Iwasawa , Horoyuki Nakamura
 *  @date 08.04.04
 */



//////////////////////////////////////////////////////////////////////////////////////
//BMPウィンドウ関連定義


enum {
  WO_PAL_YESNO_WIN = 11,  // YesNoウィンドウパレット
  WO_PAL_YESNO_WIN2,    // YesNoウィンドウパレット2
  WO_PAL_TALK_WIN,    // 進行ウィンドウパレット
  WO_PAL_TALK_FONT,   // 進行フォントパレット
  WO_PAL_SYS_FONT,    // システムフォントパレット
};

// メニューウィンドウ転送位置
#define WO_YESNO_WIN_CGX    ( 1 )
#define WO_YESNO_WIN_PX     (32-6)
#define WO_YESNO_WIN_PY     (10)

// 進行ウィンドウ転送位置
#define WO_TALK_WIN_CGX   ( WO_YESNO_WIN_CGX + TOUCH_SW_USE_CHAR_NUM )

#define W_PARAM_PX    (17)
#define W_PARAM_PY    (6)

// 「ぶんるい」
#define WIN_STR_CLASS_PX  ( 1 )
#define WIN_STR_CLASS_PY  ( W_PARAM_PY )
#define WIN_STR_CLASS_SX  ( 6 )
#define WIN_STR_CLASS_SY  ( 2 )
#define WIN_STR_CLASS_PAL ( WO_PAL_SYS_FONT )
#define WIN_STR_CLASS_CGX ( WO_TALK_WIN_CGX+TALK_WIN_CGX_SIZ )

// 「いりょく」
#define WIN_STR_ATTACK_PX ( 11 )
#define WIN_STR_ATTACK_PY ( W_PARAM_PY )
#define WIN_STR_ATTACK_SX ( 6 )
#define WIN_STR_ATTACK_SY ( 2 )
#define WIN_STR_ATTACK_PAL  ( WO_PAL_SYS_FONT )
#define WIN_STR_ATTACK_CGX  ( WIN_STR_CLASS_CGX + WIN_STR_CLASS_SX * WIN_STR_CLASS_SY )

// 「めいちゅう」
#define WIN_STR_HIT_PX    ( 20)
#define WIN_STR_HIT_PY    ( W_PARAM_PY )
#define WIN_STR_HIT_SX    ( 8 )
#define WIN_STR_HIT_SY    ( 2 )
#define WIN_STR_HIT_PAL   ( WO_PAL_SYS_FONT )
#define WIN_STR_HIT_CGX   ( WIN_STR_ATTACK_CGX + WIN_STR_ATTACK_SX * WIN_STR_ATTACK_SY )

// 威力値
#define WIN_PRM_ATTACK_PX ( WIN_STR_ATTACK_PX+5 )
#define WIN_PRM_ATTACK_PY ( WIN_STR_ATTACK_PY )
#define WIN_PRM_ATTACK_SX ( 3 )
#define WIN_PRM_ATTACK_SY ( 2 )
#define WIN_PRM_ATTACK_PAL  ( WO_PAL_SYS_FONT )
#define WIN_PRM_ATTACK_CGX  ( WIN_STR_HIT_CGX + WIN_STR_HIT_SX * WIN_STR_HIT_SY )

// 命中値
#define WIN_PRM_HIT_PX    ( WIN_STR_HIT_PX+8 )
#define WIN_PRM_HIT_PY    ( WIN_STR_HIT_PY )
#define WIN_PRM_HIT_SX    ( 3 )
#define WIN_PRM_HIT_SY    ( 2 )
#define WIN_PRM_HIT_PAL   ( WO_PAL_SYS_FONT )
#define WIN_PRM_HIT_CGX   ( WIN_PRM_ATTACK_CGX + WIN_PRM_ATTACK_SX * WIN_PRM_ATTACK_SY )

// 戦闘説明
#define WIN_BTL_INFO_PX   ( 1 )
#define WIN_BTL_INFO_PY   ( 0 )
#define WIN_BTL_INFO_SX   ( 30 )
#define WIN_BTL_INFO_SY   ( 6 )
#define WIN_BTL_INFO_PAL  ( WO_PAL_SYS_FONT )
#define WIN_BTL_INFO_CGX  ( WIN_PRM_HIT_CGX + WIN_PRM_HIT_SX * WIN_PRM_HIT_SY )

// メッセージウィンドウ
#define WIN_MSG_PX    ( 2 )
#define WIN_MSG_PY    ( 19 )
#define WIN_MSG_SX    ( 27 )
#define WIN_MSG_SY    ( 4 )
#define WIN_MSG_PAL   ( WO_PAL_TALK_FONT )
#define WIN_MSG_CGX   ( WIN_BTL_INFO_CGX + WIN_BTL_INFO_SX * WIN_BTL_INFO_SY )

// 操作説明ウィンドウ
#define WIN_EXP_PX    (1)
#define WIN_EXP_PY    (0)
#define WIN_EXP_SX    (30)
#define WIN_EXP_SY    (3)
#define WIN_EXP_PAL   (WO_PAL_SYS_FONT)
#define WIN_EXP_CGX   ( WIN_MSG_CGX + WIN_MSG_SX * WIN_MSG_SY )

// 決定ボタンウィンドウ
#define WIN_ABTN_PX   (17)
#define WIN_ABTN_PY   (21)
#define WIN_ABTN_SX   (6)
#define WIN_ABTN_SY   (3)
#define WIN_ABTN_PAL  (WO_PAL_SYS_FONT)
#define WIN_ABTN_CGX  ( WIN_EXP_CGX + WIN_EXP_SX * WIN_EXP_SY )

// 戻るボタンウィンドウ
#define WIN_BACK_PX   (25)
#define WIN_BACK_PY   (21)
#define WIN_BACK_SX   (6)
#define WIN_BACK_SY   (3)
#define WIN_BACK_PAL  (WO_PAL_SYS_FONT)
#define WIN_BACK_CGX  ( WIN_ABTN_CGX + WIN_ABTN_SX * WIN_ABTN_SY )

// 技リスト
#define WIN_LIST_PX   ( 8 )
#define WIN_LIST_PY   ( 9 )
#define WIN_LIST_SX   ( 21 )
#define WIN_LIST_SY   ( 11 )
#define WIN_LIST_PAL  ( WO_PAL_SYS_FONT )
#define WIN_LIST_CGX  ( WIN_BACK_CGX + WIN_BACK_SX * WIN_BACK_SY )

// はい/いいえ
#define WIN_YESNO_PX  ( 23 )
#define WIN_YESNO_PY  ( 13 )
#define WIN_YESNO_SX  ( 7 )
#define WIN_YESNO_SY  ( 4 )
#define WIN_YESNO_PAL ( WO_PAL_TALK_FONT )
#define WIN_YESNO_CGX ( WIN_LIST_CGX + WIN_LIST_SX * WIN_LIST_SY )

// 上　タイトル
#define WIN_TITLE_PX  ( 1 )
#define WIN_TITLE_PY  ( 0 )
#define WIN_TITLE_SX  ( 20 )
#define WIN_TITLE_SY  ( 3 )
#define WIN_TITLE_PAL ( WO_PAL_SYS_FONT )
#define WIN_TITLE_CGX ( 1 )

// 上　技リスト
#define WIN_MWAZA_PX  (  7 )
#define WIN_MWAZA_PY  (  5 )
#define WIN_MWAZA_SX  ( 10 )
#define WIN_MWAZA_SY  ( 16 )
#define WIN_MWAZA_PAL ( WO_PAL_SYS_FONT )
#define WIN_MWAZA_CGX ( WIN_TITLE_CGX + WIN_TITLE_SX*WIN_TITLE_SY )

// 上　ポケモン名
#define WIN_MPRM_PX ( 21 )
#define WIN_MPRM_PY (  5 )
#define WIN_MPRM_SX (  9 )
#define WIN_MPRM_SY (  8 )
#define WIN_MPRM_PAL  ( WO_PAL_SYS_FONT )
#define WIN_MPRM_CGX  ( WIN_MWAZA_CGX + WIN_MWAZA_SX*WIN_MWAZA_SY )

#define BG_WLIST_PX (0)
#define BG_WLIST_PY (WIN_LIST_PY)
#define BG_WLIST_SX (16)
#define BG_WLIST_SY (4)

#define BG_WLIST_SPX  (0)
#define BG_WLIST_SPY  (0)

#define BG_ABTN_PX    (16)
#define BG_ABTN_PY    (20)
#define BG_ABTN_SX    (8)
#define BG_ABTN_SY    (4)
#define BG_ABTN_OX    (0)
#define BG_ABTN_OY    (12)

#define F_WLIST_PP1_OX  (10*8)
#define F_WLIST_PP2_OX  (F_WLIST_PP1_OX+5*8)

#define F_WLIST_NAME_OX (0)
#define F_WLIST_LEVEL_OX  (8)

//////////////////////////////////////////////////////////////////////////////////////
//
// メッセージテーブルID
enum {
  MSG_SEL_WAZA = 0,   // @0に　どのわざを　おぼえさせる？
  MSG_SET_CHECK,      // @1　を  おぼえさせますか？
  MSG_END_CHECK,      // @0に　わざを  おぼえさせるのを　あきらめますか？
  MSG_SET_WAZA,     // @0は　あたらしく　@1を　おぼえた！▼
  MSG_CHG_CHECK,      // @0は　あたらしく　@1を　おぼえたい……▼　しかし　～
  MSG_DEL_WAZA,     // １　２の……　ポカン！▼　@0は　@1の つかいかたを　～
  MSG_CHG_WAZA,     // @0は　あたらしく @1を　おぼえた！▼
  MSG_DEL_CHECK,      // それでは……　@1を　おぼえるのを　あきらめますか？
  MSG_CANCEL,       // @0は　@1を　おぼえずに　おわった！▼
  MSG_SUB_ITEM,     // @2は　おれいに　ハートのウロコを　１まい　あげた！▼
  MSG_DEL_ENTER,      // @0を　わすれさせて　よろしいですね？"
};

// はい・いいえ処理テーブルID
enum {
  YESNO_SET_CHECK = 0,  // @1　を  おぼえさせますか？
  YESNO_END_CHECK,    // @0に　わざを  おぼえさせるのを　あきらめますか？
  YESNO_CHG_CHECK,    // @0は　あたらしく　@1を　おぼえたい……▼　しかし　～
  YESNO_DEL_CHECK,    // それでは……　@1を　おぼえるのを　あきらめますか？
  YESNO_DEL_ENTER,    // @0を　わすれさせて　よろしいですね？"
};

// 文字展開モード
enum {
  STR_MODE_LEFT = 0,  // 左詰め
  STR_MODE_RIGHT,   // 右詰め
  STR_MODE_CENTER,  // 中央
};

#define TMP_MSGBUF_SIZ  ( 256 )   // 汎用文字列展開領域サイズ

#define WOFCOL_N_BLACK   PRINTSYS_LSB_Make( 1,  2, 0 )    // フォントカラー：黒
#define WOFCOL_N_WHITE   PRINTSYS_LSB_Make( 15, 2, 0 )  // フォントカラー：白
#define WOFCOL_W_BLACK   PRINTSYS_LSB_Make( 1,  2, 15 )     // フォントカラー：黒

#define HEART_PUT_PX  ( 34 )    // ハート書き換えX座標
#define HEART_PUT_PY  ( 5 )   // ハート書き換えY座標
#define HEART_MAX   ( 6 )   // ハート数

#define HEART_POINT_CHR ( 0x0e )  // ハートキャラ番号（ポイントあり）
#define HEART_NONE_CHR  ( 0x12 )  // ハートキャラ番号（ポイントなし）


// キャラリソースID
enum {
  WO_CHR_ID_APP_COMMON = 0,		// リストカーソル
  WO_CHR_ID_CURSOR,           // 選択カーソル
  WO_CHR_ID_KIND,             // 分類アイコン
  WO_CHR_ID_TYPE1,            // タイプアイコン1
  WO_CHR_ID_TYPE2,            // タイプアイコン2
  WO_CHR_ID_TYPE3,            // タイプアイコン3
  WO_CHR_ID_TYPE4,            // タイプアイコン4
  WO_CHR_ID_TYPE5,            // タイプアイコン5
  WO_CHR_ID_TYPE6,            // タイプアイコン6
  WO_CHR_ID_TYPE7,            // タイプアイコン7
  WO_CHR_ID_TYPE8,            // タイプアイコン8
  WO_CHR_ID_TYPE_MAX=WO_CHR_ID_TYPE8,
	WO_CHR_ID_POKEGRA,					// ポケモン正面絵
  WO_CHR_ID_MAX								// 最大
};

// パレットリソースID
enum {
  WO_PAL_ID_APP_COMMON = 0, // タッチバー共通素材パレット
  WO_PAL_ID_OBJ,            // いろいろ
  WO_PAL_ID_TYPE,           // タイプ・分類アイコン
  WO_PAL_ID_TYPE_M,         // タイプ・分類アイコン
	WO_PAL_ID_POKEGRA,				// ポケモン正面絵
  WO_PAL_ID_MAX							// 最大
};

// セルリソースID
enum {
  WO_CEL_ID_APP_COMMON = 0,   // タッチバー共通素材
  WO_CEL_ID_CURSOR,           // 選択カーソル
  WO_CEL_ID_TYPE,             // タイプアイコン
	WO_CEL_ID_POKEGRA,					// ポケモン正面絵
  WO_CEL_ID_MAX								// 最大
};

/*
// セルアニメリソースID
enum {
  WO_ANM_ID_APP_COMMON = 0,     // リストカーソル
  WO_ANM_ID_CURSOR,           // 選択カーソル
  WO_ANM_ID_TYPE,             // タイプアイコン
  WO_ANM_ID_MAX = WO_ANM_ID_TYPE - WO_ANM_ID_APP_COMMON + 1   // 最大
};
*/

// アニメナンバー
enum{
  ANMDW_CURSOR=0,
  ANMDW_ARROW_DT=0,
  ANMDW_ARROW_DF=0,
  ANMDW_ARROW_UT=0,
  ANMDW_ARROW_UF=0,
  ANMDW_ARROW_DTA=0,
  ANMDW_ARROW_DFA=0,
  ANMDW_ARROW_UTA=0,
  ANMDW_ARROW_UFA=0,
  ANMDW_EXIT=0,
};



// パレットNo
enum{
  PALDW_01,
  PALDW_02,
  PALDW_ARROW,
  PALDW_CURSOR,
};

///パレット数
#define DW_ACTPAL_NUM (5)
#define TICON_ACTPAL_IDX  (DW_ACTPAL_NUM)
#define TICON_ACTPAL_IDX_M  (0)

// ページカーソル左
#define PAGE_CUR_L_PX ( 146 )
#define PAGE_CUR_L_PY ( 8 )
// ページカーソル右
#define PAGE_CUR_R_PX ( 246 )
#define PAGE_CUR_R_PY ( 8 )
// リストカーソル上
#define LIST_CUR_U_PX ( 8*5 )
#define LIST_CUR_U_PY ( 192-24 )
// リストカーソル下
#define LIST_CUR_D_PX ( 0 )
#define LIST_CUR_D_PY ( 192-24 )
// 選択カーソル
#define SEL_CURSOR_PX ( 127 )
#define SEL_CURSOR_PY (  81 )
#define SEL_CURSOR_SY (  24 )

#define SEL_CURSOR_P_PX (0)
#define SEL_CURSOR_P_PY (192-24)
#define SEL_CURSOR_P_SX (40)

#define SEL_CURSOR_B_PX (256-64)
#define SEL_CURSOR_B_PY (192-32)


// 分類アイコン
#define KIND_ICON_PX  ( 73 )
#define KIND_ICON_PY  ( 55 )
// タイプアイコン１(下画面)
#define TYPE_ICON1_PX ( 49 )
#define TYPE_ICON1_PY ( 80 )
// タイプアイコン２
#define TYPE_ICON2_PX ( TYPE_ICON1_PX )
#define TYPE_ICON2_PY ( TYPE_ICON1_PY+24 )
// タイプアイコン３
#define TYPE_ICON3_PX ( TYPE_ICON1_PX )
#define TYPE_ICON3_PY ( TYPE_ICON2_PY+24 )
// タイプアイコン４
#define TYPE_ICON4_PX ( TYPE_ICON1_PX )
#define TYPE_ICON4_PY ( TYPE_ICON3_PY+24 )

// タイプアイコン５
#define TYPE_ICON5_PX ( 38 )
#define TYPE_ICON5_PY ( 48 )
// タイプアイコン６
#define TYPE_ICON6_PX ( TYPE_ICON5_PX )
#define TYPE_ICON6_PY ( TYPE_ICON5_PY+32 )
// タイプアイコン７
#define TYPE_ICON7_PX ( TYPE_ICON5_PX )
#define TYPE_ICON7_PY ( TYPE_ICON6_PY+32 )
// タイプアイコン８
#define TYPE_ICON8_PX ( TYPE_ICON5_PX )
#define TYPE_ICON8_PY ( TYPE_ICON7_PY+32 )

#define POKE_PX (204)
#define POKE_PY (124)

////////////////////////////////////////////////////////////////////
//リスト
#define LIST_NUM    (4)
#define LIST_NUM_OFS  (LIST_NUM-1)
#define LIST_SY     (24)

#define CPOS_YMAX     (LIST_NUM+1)
#define CPOS_XMAX     (3)

enum{
 CPOS_LIST01,
 CPOS_LIST02,
 CPOS_LIST03,
 CPOS_LIST04,
 CPOS_PAGE_DW,
 CPOS_PAGE_UP,
 CPOS_BACK,
 CPOS_ABTN,
};

/////////////////////////////////////////////////////////////////////
//タッチパネル
#define TP_WAZA_PX  (0)
#define TP_WAZA_SX  (16*8)
#define TP_WAZA_SY  (LIST_SY)
#define TP_WAZA1_PY (4*8)
#define TP_WAZA2_PY (TP_WAZA1_PY+TP_WAZA_SY)
#define TP_WAZA3_PY (TP_WAZA2_PY+TP_WAZA_SY)
#define TP_WAZA4_PY (TP_WAZA3_PY+TP_WAZA_SY)

#define TP_SB_SX  (5*8)
#define TP_SB_SY  (3*8)
#define TP_SB_PY  (21*8)
#define TP_SBD_PX (0)
#define TP_SBU_PX (TP_SBD_PX+TP_SB_SX)

#define TP_ABTN_PX  (16*8)
#define TP_ABTN_PY  ((21*8)-2)
#define TP_ABTN_SX  (8*8)
#define TP_ABTN_SY  (24+2)

#define TP_BACK_PX  (24*8)
#define TP_BACK_PY  (TP_ABTN_PY)
#define TP_BACK_SX  (TP_ABTN_SX)
#define TP_BACK_SY  (TP_ABTN_SY)
