//======================================================================
/**
 * @file	ctvt_game.c
 * @brief	通信TVTシステム：ミニゲーム画面
 * @author	ichiraku
 * @date	2010/11/25
 *
 * モジュール名：CTVT_GAME
 */
//======================================================================
#include <gflib.h>
#include <wchar.h>					//wcslen

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/net_err.h"
#include "system/wipe.h"
#include "system/camera_system.h"
#include "system/bmp_winframe.h"
#include "net/network_define.h"
#include "app/app_menu_common.h"

#include "msg/msg_comm_tvt.h"
#include "msg\msg_wifi_system.h"    // エラー用

#include "arc_def.h"
#include "comm_tvt.naix"
#include "tvt_balloon.naix"
#include "tvt_camera.naix"
#include "font/font.naix"

#include "ctvt_game.h"
#include "ctvt_game_balloon.h"
#include "ctvt_comm.h"
#include "ctvt_camera.h"
#include "ctvt_snd_def.h"
#include "ctvt_game_balloon_tbl.cdat"
#include "ctvt_game_cam.h"

//debug
#include "debug/debug_str_conv.h"

//======================================================================
//	global
//======================================================================
static GFL_STD_RandContext gRandContext = {0};

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//===============================
// カメラ撮影をスキップ
#ifdef PM_DEBUG
//#define CAMERA_SKIP       // コメントにするとSKIPしない
#endif
//===============================

#define CTVT_UNIT_IDX_MAX       (5)     // 3Dユニットの数   
#define CTVT_GAME_TIME_LIMIT    (60)    // 制限時間 (秒)
#define CTVT_GAME_TIME_RETRY    (5)     // リトライがでるまでの時間 (秒)
#define CTVT_GAME_TIME_RETRY_NO (30)    // 自動で「いいえ」を選択する時間 (秒)

// タイムゲージ
#define TIME_BAR_W        (23)          // 幅
#define TIME_BAR_H        (3)           // 高さ
#define TIME_BAR_X        (7)           // X
#define TIME_BAR_Y        (20)          // Y
#define TIME_BAR_COL_R    (0x1)         // パレットの色
#define TIME_BAR_COL_Y    (0x2)         // パレットの色
#define TIME_BAR_COL_G    (0x3)         // パレットの色
#define TIME_BAR_COL_BASE (0x4)         // パレットの色

#define BALLOON_RND_W_MAX       (4)     // 出現位置乱数のMAX値
#define BALLOON_RND_V_MAX       (3)     // 出現位置乱数のMAX値

// PLATE関係  
#define PLATE_NUM       (4)
#define PLATE_OFFSET_X  (-8)        // プレートの中央から名前までの距離
#define PLATE_OFFSET_Y  (8)         // 
#define PLATE_OFFSET_X_SCORE  (5)   // プレートの中央から点数までの距離
#define PLATE_OFFSET_X_P      (28)   // Pの文字までの距離
#define PLATE_OFFSET_X_CROWN  (-78)   // 王冠までの距離
#define PLATE_S_OFFSET_X_P    (-16)
#define PLATE_SWING_TIME    (180)       // 揺れるまでの待ち時間

// ２つ目ミニゲーム
#define TOUCH_CNT_1 (20)    // 初期段階のタッチポイント
#define TOUCH_CNT_2 (40)    // 第2段階のタッチポイント
#define TOUCH_CNT_3 (60)    // 第3段階のタッチポイント

#define SWING_INTERVAL  (80) // 風船の揺れのインターバル

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//ステート
typedef enum
{
  CGS_FADEOUT, //両方フェード
  CGS_FADEOUT_WAIT,

  CGS_FADEOUT_CAMERA,     // カメラ用
  CGS_FADEOUT_WAIT_CAMERA,// カメラ用

  CGS_CAMERA_INIT,  // カメラ撮影
  CGS_CAMERA_MAIN,  // カメラ撮影
  CGS_CAMERA_EXIT,  // カメラ撮影
  CGS_GAME_INIT,    // ゲーム初期化
  CGS_GAME_MAIN,    // ゲームメイン
  CGS_GAME_EXIT,    // ゲーム終了

}CTVT_GAME_STATE;


//-------------------------------------
///	セルリソースインデックス
//=====================================
typedef enum {
  CGOR_UPPER_PLT,     // ゲーム上画面
  CGOR_LOWER_PLT,     // ゲーム下画面  

  CGOR_UPPER_NCG,     // ゲーム上画面
  CGOR_LOWER_NCG,     // ゲーム下画面  

  CGOR_UPPER_ANM,     // ゲーム上画面
  CGOR_LOWER_ANM,     // ゲーム下画面  
  
  CGOR_MAX,   // 最大値
} CTVT_GAME_OBJ_RES;
#define CGOR_PLT_TOP  ( CGOR_UPPER_PLT )   // パレットTOP
#define CGOR_NCG_TOP  ( CGOR_UPPER_NCG )   // キャラクタTOP
#define CGOR_ANM_TOP  ( CGOR_UPPER_ANM )   // セルアニメTOP

//-------------------------------------
///	セルアニメインデックス
//=====================================
typedef enum {
  CGOAS_TIMEBAR,      // タイムバー
  CGOAS_PLATE,        // プレート
  CGOAS_LAST,         // 「ラストスパート」
  CGOAS_KEKKA,        // 「けっかはっぴょう」
  CGOAS_TITLE1,       // タイトル1
  CGOAS_TITLE2,       // タイトル2
  CGOAS_RAINBOW1,     // 虹 昼
  CGOAS_RAINBOW2,     // 虹 夜
  CGOAS_PART2,        // 2の文字
  CGOAS_P,            // P
} CTVT_GAME_OBJ_ANM_SUB;

typedef enum {
  CGOAM_NUM_0,        // 数字 0
  CGOAM_NUM_1,        // 数字 1
  CGOAM_NUM_2,        // 数字 2
  CGOAM_NUM_3,        // 数字 3
  CGOAM_NUM_4,        // 数字 4
  CGOAM_NUM_5,        // 数字 5
  CGOAM_NUM_6,        // 数字 6
  CGOAM_NUM_7,        // 数字 7
  CGOAM_NUM_8,        // 数字 8
  CGOAM_NUM_9,        // 数字 9
  CGOAM_P,            // P
  CGOAM_NUM_DRUM,     // 0-9ドラムロール
  CGOAM_PLATE_S,      // 結果発表のプレート
  CGOAM_CLOUD1,       // 雲1 倍角
  CGOAM_CLOUD2,       // 雲2 倍角
  CGOAM_OME1,         // おめでとうアニメ 倍角
  CGOAM_TEXT_O,       // 「お」 倍角
  CGOAM_TEXT_ME,      // 「め」 倍角
  CGOAM_TEXT_DE,      // 「で」 倍角
  CGOAM_TEXT_TO,      // 「と」 倍角
  CGOAM_TEXT_U,       // 「う」 倍角
  CGOAM_TEXT_EX,      // 「！」 倍角
  CGOAM_OME_CLOUD_L,  // おめでとう 雲  倍角
  CGOAM_OME_CLOUD_R,  // おめでとう 雲  倍角
  CGOAM_OME_PLATE_L,  // おめでとう 隠しプレート
  CGOAM_OME_PLATE_R,  // おめでとう 隠しプレート
  CGOAM_RAINBOW1,     // 虹 昼
  CGOAM_RAINBOW2,     // 虹 夜
  CGOAM_STAR1,        // 星 昼
  CGOAM_STAR2,        // 星 夜
  CGOAM_STAR1_2,      // おめでとう 星
  CGOAM_STAR2_2,      // おめでとう 星
  CGOAM_OME_PLATE_2,  // おめでとう 隠しプレート
  CGOAM_RANK1,        // 「1st」
  CGOAM_RANK2,        // 「おしい2nd」
  CGOAM_RANK3,        // 「ざんねん3rd」
  CGOAM_RANK4,        // 「ざんねん4th」
  CGOAM_DOT,          // 「・・・」

  // 2つ目のゲーム
  CGOAM_POMP_BASE,    // ポンプ土台
  CGOAM_POMP_HANDLE,  // ポンプ取っ手
  CGOAM_BUTTON_STAY,  // タッチするところ 通常
  CGOAM_BUTTON_TOUCH, // タッチするところ タッチ時
  CGOAM_BUTTON_LAST_STAY,  // タッチするところ 通常
  CGOAM_BUTTON_LAST_TOUCH, // タッチするところ タッチ時
  CGOAM_POMP_HANDLE_LAST,  // ポンプ取っ手 残り時間20秒板
  CGOAM_PEN,               // ペンタッチアニメ
} CTVT_GAME_OBJ_ANM_MAIN;

// PLATE task seq
enum
{
  PLATE_SEQ_MOVE_OUT,         // 画面外に移動
  PLATE_SEQ_MOVE_OUT_VANISH,  // 画面外に消える
  PLATE_SEQ_MOVE_IN_WAIT,     // 移動開始待ち
  PLATE_SEQ_MOVE_IN,          // 画面内に移動
};
// SCORE PLATE task seq
enum
{
  PLATE_SEQ_DRUMROLL,
  PLATE_SEQ_DRUMROLL_STOP,
  PLATE_SEQ_WAIT,
};

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//-------------------------------------
///	ゲーム中上画面プレート
//=====================================
typedef struct {

  GFL_TCB*      tcb;            // タスク
  HEAPID        heapID;

  u8            sub_seq;        // サブシーケンス

  u8            rank;           // 何位 
  u8            netID;          // NetID
  u16           point;          // 点数

  BOOL          is_blank;       // 空プレートフラグ
  BOOL          is_mine;        // 自分か

  // 自分プレート揺れ
  BOOL          is_swing;       // 揺れフラグ
  s8            swing_cnt;      // 揺れ状態
  u8            swing_wait;     // ゆれ待ち時間

  // 文字描画用
  GFL_FONT      *parent_fontHandle;
  GFL_MSGDATA   *parent_msgHandle;
  PRINT_QUE     *parent_printQue;

  GFL_BMPWIN    *bmpwin_name;   // プレイヤー名
  GFL_BMPWIN    *bmpwin_score;  // 得点
  BOOL          is_update_name; // プレイヤー名更新
  BOOL          is_update_score;// 得点更新
  STRBUF        *str_name;      // プレイヤー名
  STRBUF        *str_tmp;       // 得点用
  STRBUF        *str_expand;    // 得点用

  // プレート移動用
  u8            move_wait;      // 待ち時間 差をつけるため
  int           move_value;       // アニメカウント
  BOOL          is_move;        // 移動開始フラグ

  GFL_CLWK*       clwkPlate;      // 土台
  GFL_CLWK*       clwkPoint[3];   // 点数
  GFL_CLWK*       clwkP;          // 「P」

#ifdef PM_DEBUG
  BOOL  debug_blank;  // 表示したいだけフラグ
#endif
} GAME_PLATE_WORK;


//-------------------------------------
///	メインワーク
//=====================================
struct _CTVT_GAME_WORK
{
  HEAPID          heapId;
  CTVT_GAME_ID    game_id;        // ゲーム種類


  CTVT_GAME_STATE state;
  
  // 各ステート内でのシーケンス管理用
  // タイトル
  // ゲーム
  // 結果発表など
  u8    seq;      

  // 各シーケンス内でのシーケンス
  // 結果発表の中でまたシーケンス分けたいときなど
  // 結果発表ぐらいでしか使っていない
  u8    sub_seq;
  
  BOOL isRetry;     // TRUEだとリトライ

  //=====================================
  // カメラ関係
  //-------------------------------------
  // カメラ撮影用パラメータ
  CTVT_GAME_CAM_MEMBER_WORK cam_member[ CTVT_MEMBER_NUM-1 ];
  CTVT_GAME_CAM_PARAM       *p_cam_param;
  CTVT_GAME_CAM_WORK        *p_cam_wk;

  //=====================================
  // ゲームプレイ関係
  //-------------------------------------
  GFL_TCBSYS    *task;          // タスクシステム
  void          *taskMem;       // タスク用メモリ
  BALLOON_WORK  balloon_wk[ BALLOON_MAX ];    // 風船構造体
  u8            balloon_rnd_w;                // 風船出現調整
  u8            balloon_rnd_v;                // 風船出現調整
  u8            balloon_z_cnt;                // 風船出現調整
  u8            balloon_tbl_idx;              // 風船出現にどのテーブルを使用するか
  u8            balloon_tbl_cnt;              // テーブルの要素カウンタ
  u16           bonus_tbl[CTVT_MEMBER_NUM][5]; // ボーナス風船の出現フレームのテーブル
  u8            bonus_tbl_idx[CTVT_MEMBER_NUM];

  // 2つ目  本来は1つ目と2つ目でワークにわけてやるべきだが、
  // 今変えると変更点が多すぎるのでここに2つ目のものも書く
  BALLOON2_WORK *p_balloon2[CTVT_MEMBER_NUM];
  u8            touch_cnt[CTVT_MEMBER_NUM];   // 4人分のタッチ計測
  u8            level_cnt[CTVT_MEMBER_NUM];   // 4人分のタッチ段階
  u8            swing_interval;               // 風船揺れのインターバル

  // 制限時間管理
  u16           timeLimit;      // 制限時間 (秒)
  u32           befFrameCnt;    // フレームカウンタ保存用
  GFL_BMPWIN    *bmpWinTime;    // タイムゲージ

  // タイトル文字
  GFL_BMPWIN    *msgWinTitleFont;     // ゲームルール
  BOOL          isUpdateMsgTitleFont; // ゲームルール
  GFL_BMPWIN    *msgWinTitle;         // 説明
  BOOL          isUpdateMsgTitle;     // 説明
  u16           title_cnt;            // 自動開始

  // プレート系
  GFL_TCBSYS        *task_plate;          // タスクシステム
  void              *taskmem_plate;       // タスク用メモリ
  GAME_PLATE_WORK   *p_plate[CTVT_MEMBER_NUM];
  BOOL              plate_move;

  // 結果発表用
  BALLOON_S_WORK    *p_balloon_s_wk[CTVT_MEMBER_NUM];
  void              *p_plt_buff;    // BGアニメ用のパレット
  NNSG2dPaletteData *p_plt_data;    // パレットデータ
  u8                bg_pal_anm;     // パレットアニメ用フレーム
  u16               score_cnt;      // カウンタ用

  // リトライ用
  GFL_BMPWIN        *msgRetry;            // メッセージ描画用
  BOOL              isUpdateMsgRetry;     // メッセージ更新
  u16               msgWait;              // メッセージ表示待ち
  u16               retry_cnt;            // メッセージ表示待ち
  APP_TASKMENU_RES  *taskMenuRes;         // 共通リソース
  APP_TASKMENU_WORK *yesNoWork;           // はい・いいえ選択
  APP_TASKMENU_WORK *cancelWork;          // ちゅうし
  BOOL              isCancelGame;         // CANCEL_GAMEが送信されたか？
  BOOL              isExitGame;           // EXIT_GAMEが送信されたか？
  BOOL              isStartGame;          // START_GAMEが送信されたか？
  BOOL              isStartGame2;         // START_GAMEが送信されたか？
  u8                acceptBit;            // 参加したNetIDのビット管理
  u8                acceptBit2;           // 参加したNetIDのビット管理

  //===============================
  // 通信
  u32   rnd_seed;         // 乱数シード値
  u16   frame;            // 通信で受信したフレーム
  BOOL  isRecvFrame;      // フレーム受信したか
  // 以下親のみ
  BOOL  is_main;          // ゲームがプレイ中か
  u16   oya_frame;        // 親のフレーム経過数
  s16   balloon_wait;     // 風船出現待ち時間
  u8    balloon_flg;      // 風船出現フラグ
  s32   score[CTVT_MEMBER_NUM];    // スコア


  //=====================================
  // GRAPHIC関係
  //-------------------------------------
  //↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
  BOOL              is_init;
  u8                draw_flg;

  // 2D
  // セル関係
  u32         cellResIdx[ CGOR_MAX ];   // セルリソースインデックス
  GFL_CLUNIT  *cellUnit;                // セルアクターシステム
  // タイトル
  GFL_CLWK    *clwkTitle[4];            // タイトル
  // ゲームプレイ中
  GFL_CLWK    *clwkCloud_M[2];          // 雲
  GFL_CLWK    *clwkTime;                // タイムゲージ
  GFL_CLWK    *clwkLast;                // ラストスパートの文字
  GFL_CLWK    *clwkTouch;               // タッチするボタン
  GFL_CLWK    *clwkPompBase;            // 空気入れ土台
  GFL_CLWK    *clwkPompHandle;          // 空気入れ取っ手
  GFL_CLWK    *clwkPenAnm;              // ペンタッチアニメ

  // 結果発表
  GFL_CLWK    *clwkKekka;               // けっかはっぴょうの文字
  GFL_CLWK    *clwkOmeFont;             // 「おめでとう」倍角
  GFL_CLWK    *clwkOmeText[6];          // 「おめでとう」単文字 倍角
  GFL_CLWK    *clwkOmeCloud[2];         // エフェクト雲倍角
  GFL_CLWK    *clwkOmeHide[2];          // エフェクト隠すプレート
  GFL_CLWK    *clwkOmeNiji;             // エフェクト虹
  GFL_CLWK    *clwkOmeStar;             // エフェクト星
  GFL_CLWK    *clwkRank;                // 順位 
  GFL_CLWK    *clwkDot;                 // 三点リーダー 3,4位のみ

  // 3D
  GFL_G3D_UTIL    *g3dUtil;
  u16             unit_idx[CTVT_UNIT_IDX_MAX];           ///< GFL_G3D_UTILが割り振る番号
  GFL_G3D_CAMERA  *p_camera;

  GFL_TCB *vBlankTcb;

#ifdef PM_DEBUG
  //=====================================
  // DEBUG　主にゲーム調整用
  //-------------------------------------
  fx32  debug_r;    // 風船:半径特定用(当たり判定)
  fx32  debug_size; // 風船:サイズ変更
  fx32  debug_v;    // 風船:速さ変更
  u16   debug_wait; // 風船:個数変更(出現タイミング)
  u16   debug_b_cnt[4]; // どれだけ風船がでたか数える

  s8    debug_touch_r;        // タッチ半径
  s8    debug_touch_ex_r;    // タッチ半径
#endif
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

// モード別メイン関数
static void CTVT_GAME_Init( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );
static BOOL CTVT_GAME_Update( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );
static void CTVT_GAME_Exit( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );

static void exit_error( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );

// ↑のシーケンス内での関数 本当はシーケンス関数をつくりたかったが、複雑になりそうなので個別で関数を定義した
static void game_update_seq_init_common( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_init_part1( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_init_part2( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static BOOL game_update_seq_title( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_title_end_part1( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_title_end_part2( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static BOOL game_update_seq_main_part1( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static BOOL game_update_seq_main_part2( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_score_init( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static BOOL game_update_seq_score( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_exit_common( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_exit_part1( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_exit_part2( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );

// 強制終了チェック
static const BOOL CTVT_GAME_CheckFinishReq( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );

// 乱数
static u32 get_rand( u32 range );

// GRAPHIC
static void GRAPHIC_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID );
static void GRAPHIC_Exit( CTVT_GAME_WORK* p_wk );
static void GRAPHIC_Draw( CTVT_GAME_WORK* p_wk );
static void GRAPHIC_Draw2( CTVT_GAME_WORK* p_wk );
static void VBlankFunc( GFL_TCB *p_tcb, void *p_work );
static void GRAPHIC_OBJ_Remove( GFL_CLWK* p_wk );

// BG
static void GRAPHIC_BG_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( CTVT_GAME_WORK* p_wk );
static void GRAPHIC_BG_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );

// 3d
static void GRAPHIC_3D_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID );
static void GRAPHIC_3D_Exit( CTVT_GAME_WORK* p_wk );

// resource
static void RESOURCE_Init_Part1( CTVT_GAME_WORK* p_wk, HEAPID heapID, ARCHANDLE* arcHandle );
static void RESOURCE_Exit_Part1( CTVT_GAME_WORK* p_wk );
static void RESOURCE_Init_Part2( CTVT_GAME_WORK* p_wk, HEAPID heapID, ARCHANDLE* arcHandle );
static void RESOURCE_Exit_Part2( CTVT_GAME_WORK* p_wk );
static void RESOURCE3D_Init_Score( CTVT_GAME_WORK* p_wk, HEAPID heapID );

// message
static void disp_message_retry( COMM_TVT_WORK *work, CTVT_GAME_WORK* p_wk, const u16 msgId );

// timebar
static void draw_timebar( CTVT_GAME_WORK* p_wk );

// retry
static const BOOL retry_check_finish_req( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
APP_TASKMENU_WORK* retry_open_yesno_menu( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
APP_TASKMENU_WORK* retry_open_cancel_menu( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );

// balloon
// 風船設定
static void balloon_set( CTVT_GAME_WORK *p_game_wk, BALLOON_WORK*  p_wk, u16  unit_idx );
// 全ての風船が終了しているか？
static BOOL balloon_end_check( CTVT_GAME_WORK *p_wk );
// 誰の風船を出すか計算して求める
static u8   balloon_find_idx( CTVT_GAME_WORK *p_wk );

// タスクに設定するタスク関数(風船用)
static void TaskFunc( GFL_TCB* p_tcb, void* p_work );
// タスクに設定するタスク関数(風船2用)
static void TaskFunc2( GFL_TCB* p_tcb, void* p_work );
// タスクに設定するタスク関数(結果発表の風専用)
static void TaskFuncScore( GFL_TCB* p_tcb, void* p_work );

// タスクに登録されている風船とタッチの当たり判定
static BOOL balloon_hit_check( CTVT_GAME_WORK* p_wk, u32 tpx, u32 tpy, u16 *ret_idx );
// タスクに登録されている風船同士の距離判定
static void balloon_collision_check( CTVT_GAME_WORK *p_wk );
// 2点の距離の壁判定処理(移動も自動)
static void collision_vec( fx32* x1, fx32* y1, fx32* x2, fx32* y2, fx32 dir, u8 avoid );
// レイと球の交差判定
static BOOL collision3d_sphere_checkRay( const VecFx32 *cp_rayPos, const VecFx32 *cp_rayWay, const GFL_COLLISION3D_CIRCLE *cp_circle );


//-------------------------------------
/// PLATE
//=====================================
// プレートの位置
static const u8 PLATE_POS[4][2] = 
{
  { 112, 25 },
  { 120, 59 },
  { 128, 93 },
  { 136, 127 },
};

// 結果発表プレートの位置
static const u8 PLATE_S_POS[4][2] = 
{
  { 32, 176 },
  { 96, 176 },
  { 160, 176 },
  { 224, 176 },
};


static GAME_PLATE_WORK* GAME_PLATE_CreatePlate( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork, const u8 netID );
static GAME_PLATE_WORK* GAME_PLATE_CreatePlate_Blank( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork, const u8 netID );
static void GAME_PLATE_DeletePlate( GAME_PLATE_WORK *p_wk );
static void GAME_PLATE_UpdatePlate( CTVT_GAME_WORK *p_wk );
static void GAME_PLATE_ActivePlate( COMM_TVT_WORK *work, GAME_PLATE_WORK *p_wk );
static void GAME_PLATE_SetPoint( CTVT_GAME_WORK *gameWork, const u8 netID, const u8 point );
static void GAME_PLATE_SetMovePlate( CTVT_GAME_WORK *gameWork, BOOL is_vanish );
static BOOL GAME_PLATE_MovePlate( GAME_PLATE_WORK *p_wk, const int value );
static void GAME_PLATE_ClearPlate( GAME_PLATE_WORK *p_wk );
static void TaskFuncPlate( GFL_TCB* p_tcb, void* p_work );
// 結果発表画面でのプレート
static void GAME_PLATE_ChangePlate( COMM_TVT_WORK *work, GAME_PLATE_WORK *p_wk );
static void TaskFuncScorePlate( GFL_TCB* p_tcb, void* p_work );

//------------------------------------------------------------------
//  処理時間AVERAGE
//------------------------------------------------------------------
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

#define DEBUG_MAIN_TIME_AVERAGE (1) // 時間AVERAGE　表示 １：ON　０：OFF
#define DEBUG_AVERAGE_MAX (10)

static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start[DEBUG_AVERAGE_MAX];    // 1フレーム開始
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_End[DEBUG_AVERAGE_MAX];    // 1フレーム開始
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[DEBUG_AVERAGE_MAX];      // AVERAGE
static int DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[DEBUG_AVERAGE_MAX]; // カウンタ
static int DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount[DEBUG_AVERAGE_MAX];    // 16000Over
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[DEBUG_AVERAGE_MAX];    // 16000Over
#define DEBUG_DEBUG_MAIN_TIME_AVERAGE_AVE_COUNT_TIME  ( 120 )

#define DEBUG_DEBUG_MAIN_TIME_AVERAGE_NOW_TIME_DRAW_KEY  ( PAD_BUTTON_R )

static void DEBUG_MAIN_TIME_AVERAGE_StartFunc( u8 id );
static void DEBUG_MAIN_TIME_AVERAGE_EndFunc( u8 id );
#endif

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
CTVT_GAME_WORK* CTVT_GAME_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId )
{
  CTVT_GAME_WORK* gameWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_GAME_WORK) );
  gameWork->heapId = heapId;

  //バージョンで白黒を変える
  if( GET_VERSION() == VERSION_B2 )
  {
  }
  else
  {
  }
  return gameWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void CTVT_GAME_TermSystem( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  ICHI_Printf("----CTVT_GAME_TermSystem----\n");

  GFL_HEAP_FreeMemory( gameWork );
}
//--------------------------------------------------------------
//	モード切替時初期化
//--------------------------------------------------------------
void CTVT_GAME_InitMode( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  const HEAPID heapID = gameWork->heapId;
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
  u8 i;

  ICHI_Printf("----CTVT_GAME_InitMode----\n");


  // カメラ撮影用のメンバー情報作成
  {
    const u8 connectNum = GFL_NET_GetConnectNum();
    u8  mem = 0;
    u8  netId;
    ARCDATID  dat_tbl[4] = {
      NARC_tvt_camera_char1_1_ntft,
      NARC_tvt_camera_char2_1_ntft,
      NARC_tvt_camera_char3_1_ntft,
      NARC_tvt_camera_char4_1_ntft,
    };

    for( i=0; i<CTVT_MEMBER_NUM-1; i++ )
    {
      GFL_STD_MemClear( &gameWork->cam_member[i], sizeof(CTVT_GAME_CAM_MEMBER_WORK) );
    }
    ICHI_Printf("接続人数 %d\n", connectNum);
    ICHI_Printf("メンバーのバッファアドレス\n");
    for( netId=0;netId<CTVT_MEMBER_NUM;netId++ )
    {
      GF_ASSERT( selfId < 4 );
      if( selfId == netId )
      {
        // 自分だとスキップ
        continue;
      }

      gameWork->cam_member[mem].isTwl = CTVT_COMM_CanUseCameraMember( work, commWork, netId );
      gameWork->cam_member[mem].netID = netId;

      // 接続している人がTWL起動のみ受信用メモリを確保する
      // NITROの場合かNPCだとキャラBMPを読み込む
      if( gameWork->cam_member[mem].isTwl == TRUE )
      {
        u8 j;
        for( j=0; j<CAPTURE_NUM; j++ )
        {
          gameWork->cam_member[mem].p_cap_buf[j] = GFL_NET_Align32Alloc( heapID, CAPTURE_SIZE);
          ICHI_Printf("mem[%d] %d adrs %x \n", mem,j,gameWork->cam_member[mem].p_cap_buf[j]);
        }
        ICHI_Printf("netId %d mem %d \n", netId, mem);
      }
      else
      {
        u8 j;
        u8 char_id = CTVT_COMM_GetCharId( work, commWork, netId );
        for( j=0; j<CAPTURE_NUM; j++ )
        {
          gameWork->cam_member[mem].p_cap_buf[j] = GFL_ARC_UTIL_Load( ARCID_TVT_CAMERA, dat_tbl[char_id]+j, FALSE, heapID );
        }
      }
      mem++;
    }

#if 1
    for( i=0; i<CTVT_MEMBER_NUM; i++ )
    {
      ICHI_Printf("%d: %d\n", i, CTVT_COMM_GetCharId( work, commWork, i ) );
    }
#endif
  }

  // カメラ撮影のパラメータ作成
  {
    BOOL isTwl = COMM_TVT_CanUseCamera();
    CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork(work);
    CAMERA_SYSTEM_WORK *camSys = CTVT_CAMERA_GetCamSys( work, camWork );
    u8  select_chara = CTVT_COMM_GetCharId( work, commWork, selfId );

    gameWork->p_cam_param = CTVT_GAME_CAM_AllocParam( heapID, isTwl, select_chara, gameWork->cam_member, camSys );
  }

  // 構造体初期化
  gameWork->state = CGS_CAMERA_INIT;
#ifdef CAMERA_SKIP
  gameWork->state = CGS_GAME_INIT;
#endif
  gameWork->isRetry  = FALSE;

  //上画面の会話アイコンを消す
  COMM_TVT_EraseTalkIcon( work );

  GFL_NET_ReloadIconTopOrBottom(TRUE , heapID );

  ICHI_Printf("----CTVT_GAME_InitMode END----\n");
}

//--------------------------------------------------------------
//	モード切替時開放
//--------------------------------------------------------------
void CTVT_GAME_TermMode( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  u8 i;
  const HEAPID heapId = gameWork->heapId;

  ICHI_Printf("----CTVT_GAME_TermMode----\n");

  // カメラ撮影用パラメータ破棄
  CTVT_GAME_CAM_FreeParam( gameWork->p_cam_param );

  // カメラ撮影用メンバー情報破棄
  for( i=0; i<CTVT_MEMBER_NUM-1; i++ )
  {
    u8 j;
    if( gameWork->cam_member[i].isTwl == TRUE )
    {
      for( j=0; j<CAPTURE_NUM; j++ )
      {
        GFL_NET_Align32Free( gameWork->cam_member[i].p_cap_buf[j] );
      }
    }
    else
    {
      for( j=0; j<CAPTURE_NUM; j++ )
      {
        GFL_HEAP_FreeMemory( gameWork->cam_member[i].p_cap_buf[j] );
      }
    }
  }

  // エラー落ちの解放
  if( COMM_TVT_GetNextMode( work ) == CTM_ERROR )
  {
    ICHI_Printf("エラー処理\n");
    exit_error( work, gameWork );
  }
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
const COMM_TVT_MODE CTVT_GAME_Main( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  const HEAPID heapId = gameWork->heapId;

  int i;

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(0);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON


  switch( gameWork->state ){
  case CGS_FADEOUT:
    {
      CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
      //StopCaptureは停止音待ち
      //if( CTVT_CAMERA_IsStopCapture( work , camWork ) == TRUE )
      {
        WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
            WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
        gameWork->state = CGS_FADEOUT_WAIT;
      }
    }
    break;

  case CGS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      // エラー落ち
      return CTM_END;
    }
    break;

  case CGS_CAMERA_INIT:  // カメラ撮影
    {
      gameWork->p_cam_wk = CTVT_GAME_CAM_Init( gameWork->p_cam_param, heapId );
      gameWork->state = CGS_CAMERA_MAIN;

      WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
          WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, heapId );
    }
    break;

  case CGS_CAMERA_MAIN:  // カメラ撮影
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      if( CTVT_GAME_CheckFinishReq( work, gameWork ) == FALSE )
      {
        BOOL ret;
        ret = CTVT_GAME_CAM_Main( gameWork->p_cam_wk );
        if( ret == TRUE )
        {
          gameWork->state = CGS_CAMERA_EXIT;
        }
      }
      else
      {
        return CTM_ERROR;
      }
    }
    break;

  case CGS_CAMERA_EXIT:  // カメラ撮影
    gameWork->state = CGS_FADEOUT_WAIT_CAMERA;
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
        WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, heapId );
    break;

  case CGS_FADEOUT_WAIT_CAMERA:// カメラ用
    CTVT_GAME_CAM_Exit( gameWork->p_cam_wk );
    gameWork->state = CGS_GAME_INIT;
    break;

  case CGS_GAME_INIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      CTVT_GAME_Init( work, gameWork );
      gameWork->state = CGS_GAME_MAIN;
    }

    break;

  case CGS_GAME_MAIN:
    if( CTVT_GAME_CheckFinishReq( work, gameWork ) == FALSE )
    {
      BOOL ret;
      ret = CTVT_GAME_Update( work , gameWork );
      if( ret == TRUE )
      {
        gameWork->state = CGS_GAME_EXIT;
      }
    }
    else
    {
      return CTM_ERROR;
    }
    break;

  case CGS_GAME_EXIT:
    CTVT_GAME_Exit( work, gameWork );

    if( gameWork->isRetry == TRUE )
    {
      gameWork->state = CGS_GAME_INIT;
    }
    else
    {
      // 会話モードに戻る
      //gameWork->state = CGS_FADEOUT;
      return CTM_TALK;
    }
    break;
  }

  // =========================
  // メッセージ
  if( gameWork->isUpdateMsgRetry == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( gameWork->msgRetry )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( gameWork->msgRetry );
      GFL_BMPWIN_MakeScreen( gameWork->msgRetry );
      GFL_BG_LoadScreenReq(CTVT_GAME_FRAME_M_MSG);
      gameWork->isUpdateMsgRetry = FALSE;
    }
  }
  // タイトル文字
  if( gameWork->isUpdateMsgTitleFont == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( gameWork->msgWinTitleFont )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( gameWork->msgWinTitleFont );
      GFL_BMPWIN_MakeScreen( gameWork->msgWinTitleFont );
      GFL_BG_LoadScreenReq(CTVT_GAME_FRAME_M_PLATE);
      gameWork->isUpdateMsgTitleFont = FALSE;
    }
  }
  // 説明文字
  if( gameWork->isUpdateMsgTitle == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( gameWork->msgWinTitle )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( gameWork->msgWinTitle );
      GFL_BMPWIN_MakeScreen( gameWork->msgWinTitle );
      GFL_BG_LoadScreenReq(CTVT_GAME_FRAME_M_PLATE);
      gameWork->isUpdateMsgTitle = FALSE;
    }
  }

  // COMM_TVT_Mainからコピーしてきた
  GFL_CLACT_SYS_Main();
  PRINTSYS_QUE_Main( COMM_TVT_GetPrintQue(work) );

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(0);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

  return CTM_GAME;
}


//----------------------------------------------------------------------------
/**
 *	@brief  選択したミニゲームID設定
 *
 *	@param	CTVT_GAME_WORK *gameWork
 *	@param	CTVT_GAME_ID game_id      ミニゲームID  CGI_XXX
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_SetGameID( CTVT_GAME_WORK *gameWork, const CTVT_GAME_ID game_id )
{
  gameWork->game_id = game_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  選択したミニゲームID取得
 *
 *	@param	CTVT_GAME_WORK *gameWork  ワーク
 *
 *	@return CTVT_GAME_ID  ミニゲームID  CGI_XXX
 */
//-----------------------------------------------------------------------------
const CTVT_GAME_ID CTVT_GAME_GetGameID( CTVT_GAME_WORK *gameWork )
{
  return gameWork->game_id;
}

// G3DUtil取得
GFL_G3D_UTIL* CTVT_GAME_GetgG3DUtil( CTVT_GAME_WORK *gameWork )
{
  return gameWork->g3dUtil;
}

// ゲームがプレイ中(風船が出ているところ)だとTRUE
const BOOL CTVT_GAME_IsMain( CTVT_GAME_WORK *gameWork )
{
  return gameWork->is_main;
}

// リトライ拒否フラグ設定
void CTVT_GAME_SetIsCancelGame( CTVT_GAME_WORK *gameWork, const BOOL flg )
{
  gameWork->isCancelGame = flg;
}

// 終了通知フラグ設定
void CTVT_GAME_SetIsExitGame( CTVT_GAME_WORK *gameWork, const BOOL flg )
{
  gameWork->isExitGame = flg;
}

// 開始通知フラグ設定
void CTVT_GAME_SetIsStartGame( CTVT_GAME_WORK *gameWork, const BOOL flg )
{
  gameWork->isStartGame = flg;
}

// 開始通知フラグ設定 確認用
void CTVT_GAME_SetIsStartGame2( CTVT_GAME_WORK *gameWork, const BOOL flg )
{
  gameWork->isStartGame2 = flg;
}

// リトライする人のNetIDを設定
void CTVT_GAME_SetAcceptBit( CTVT_GAME_WORK *gameWork, const u8 NetId )
{
  gameWork->acceptBit |= (1<<NetId);
}

// リトライする人のNetIDを設定 確認用
void CTVT_GAME_SetAcceptBit2( CTVT_GAME_WORK *gameWork, const u8 NetId )
{
  gameWork->acceptBit2 |= (1<<NetId);
}

// 接続している人が全員リトライするか
const BOOL CTVT_GAME_IsAcceptNum( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  const u8 connectNum = COMM_TVT_GetConnectNum( work );
  u8  i;

  // 0番目は自分(親)なので無視する
  for( i=1; i<connectNum; i++ )
  {
    // NetIdのビットがたっていなければFALSE
    if( !(gameWork->acceptBit & (1<<i)) )
    {
      return FALSE;
    }
  }

  return TRUE;
}

// 接続している人が全員リトライするか 確認用
const BOOL CTVT_GAME_IsAcceptNum2( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  const u8 connectNum = COMM_TVT_GetConnectNum( work );
  u8  i;

  // 0番目は自分(親)なので無視する
  for( i=1; i<connectNum; i++ )
  {
    // NetIdのビットがたっていなければFALSE
    if( !(gameWork->acceptBit2 & (1<<i)) )
    {
      return FALSE;
    }
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定した色(NetID)の風船に指定したテクスチャをはる
 *
 *	@param	CTVT_GAME_WORK *p_wk  ワーク
 *	@param	u8 netID              NetID =色
 *	@param	u8 tex_idx            テクスチャインデックス
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_BalloonTexTrans( CTVT_GAME_WORK *p_wk, const u8 netID, const u8 tex_idx )
{
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

#ifndef CAMERA_SKIP
  // 3Dにカメラキャプチャを貼り付ける
  {
    BOOL  rc;
    u32   dst;    // 転送先の実アドレス
    u16*  src;    // 転送元データ
    u32   size;   // 転送サイズ
    u8    type;   // 貼り付ける風船 3つある
    u8    mem_cnt=0;  // メンバーIDX
    u8    i;

    ICHI_Printf("テクスチャ貼り付け\n");

    // メンバーIDXを計算
    for( i=0; i<tex_idx; i++ )
    {
      // メンバーカウンタ
      if( selfId != i )
      {
        mem_cnt++;
      }
    }

    // 指定した色の風船の3種類のテクスチャをはる
    for( type=0; type<CAPTURE_NUM; type++ )
    {
      u16 unit_res_cnt = GFL_G3D_UTIL_GetUnitResCount( p_wk->g3dUtil, p_wk->unit_idx[netID] );
      GFL_G3D_RES* res = GFL_G3D_UTIL_GetResHandle( p_wk->g3dUtil, unit_res_cnt*netID + type );
      NNSG3dTexKey texkey = GFL_G3D_GetTextureDataVramKey( res );
      u32 tex_adr = NNS_GfdGetTexKeyAddr(texkey);
      u32 tex_size = NNS_GfdGetTexKeySize(texkey);

      // 転送
      {
        u16 tex_offset = (128*2);
        dst = tex_adr + tex_offset; // tex_offset分足しているのは、写真のテクスチャにする為
        // 貼り付ける画像選択
        if( selfId == tex_idx )
        {
          // TWLはキャプチャ画像、NITOROはキャラ画像
          src = (u16*)p_wk->p_cam_param->p_cap[type];
        }
        else
        {
          // 他のメンバーの場合
          // 
          // TWLなら受信した画像、NITOROならキャラ画像
          src = (u16*)p_wk->cam_member[mem_cnt].p_cap_buf[type];
        }
        size = CAPTURE_SIZE;
        rc = NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, size);
        GF_ASSERT(rc);
        ICHI_Printf("dst %x src %x size %x texsize %x \n", dst, src, size, tex_size);
        ICHI_Printf("netID %d texIDX %d mem_cnt %d", netID, tex_idx, mem_cnt);
      }
    }

  }
#endif
}


//----------------------------------------------------------------------------
/**
 *	@brief  風船出現
 *
 *	@param	CTVT_GAME_WORK  *gameWork
 *
 *	@retval TRUE  出現した
 *	@retval FALSE 出現できなかった
 */
//-----------------------------------------------------------------------------
BOOL CTVT_GAME_BalloonAppear( CTVT_GAME_WORK *p_wk )
{
  u8  i;

  // 空いてるタスクを探す
  for( i=0; i<BALLOON_MAX; i++ )
  {
    if( p_wk->balloon_wk[i].tcb == NULL )
    {
      p_wk->balloon_wk[i].tcb = GFL_TCB_AddTask( p_wk->task, TaskFunc, &p_wk->balloon_wk[i], 0 );
      GF_ASSERT( p_wk->balloon_wk[i].tcb != NULL );

      if( p_wk->balloon_wk[i].tcb != NULL )
      {
        // 誰の風船か
        u8  idx = balloon_find_idx( p_wk );

        // 風船出現位置調整
        {
          u8  rnd;
          u8  i;
          // 前の乱数と同じにならないようにする
          // 無限ループになるのが怖いから最大10回にする
          for( i=0; i<10; i++ )
          {
            rnd = get_rand(BALLOON_RND_W_MAX);
            if( rnd != p_wk->balloon_rnd_w )
            {
              break;
            }
          }
          p_wk->balloon_rnd_w = rnd;

          for( i=0; i<10; i++ )
          {
            rnd = get_rand(BALLOON_RND_V_MAX);
            if( rnd != p_wk->balloon_rnd_v )
            {
              break;
            }
          }
          p_wk->balloon_rnd_v = rnd;

          // Zをずらす為の調整用
          p_wk->balloon_z_cnt++;
          p_wk->balloon_z_cnt %= 3;
        }

        // 風船出現
        balloon_set( p_wk, &p_wk->balloon_wk[i], p_wk->unit_idx[idx] );
        p_wk->balloon_wk[i].parent_g3dUtil = p_wk->g3dUtil;
        // ボーナス風船判定
        {
          if( p_wk->bonus_tbl[idx][ p_wk->bonus_tbl_idx[idx] ] > p_wk->timeLimit )
          {
            p_wk->balloon_wk[i].type = BALLOON_BONUS;
            p_wk->bonus_tbl[idx][ p_wk->bonus_tbl_idx[idx] ] = 0;
            p_wk->bonus_tbl_idx[idx]++;
            p_wk->bonus_tbl_idx[idx] %= 5;
            //ICHI_Printf("ボーナス風船出現 %d \n", idx);
          }
          else
          {
            p_wk->balloon_wk[i].type = BALLOON_NORMAL;
          }
        }

        // 出現した風船が他の風船と重なっているかチェック
        {
          u8  j;
          for( j=0; j<BALLOON_MAX; j++ )
          {
            // 違う風船 かつ サイズが同じ かつ 表示している
            // ならチェックする
            if( ( j != i ) && ( p_wk->balloon_wk[i].size == p_wk->balloon_wk[j].size ) && ( p_wk->balloon_wk[j].visible == TRUE ) )
            {
              // 壁判定
              collision_vec( 
                  &p_wk->balloon_wk[i].status.trans.x, &p_wk->balloon_wk[i].status.trans.y,
                  &p_wk->balloon_wk[j].status.trans.x, &p_wk->balloon_wk[j].status.trans.y,
                  p_wk->balloon_wk[i].status.scale.x * 140, 1 );

            }
          }
        }

        return TRUE;
      }
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信から送られてくる乱数シード値を格納
 *
 *	@param	p_wk    ゲーム管理構造体
 *	        data    設定するゲーム情報
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommRndSeedSet( CTVT_GAME_WORK *p_wk, const u32 rnd_seed )
{
  // 受信データを更新
  p_wk->rnd_seed = rnd_seed;
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信から送られてくるフレーム経過数を格納
 *
 *	@param	p_wk    ゲーム管理構造体
 *	        frame   受信したフレーム
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommFrameSet( CTVT_GAME_WORK *p_wk, const u16 frame )
{
  p_wk->frame = frame;
  p_wk->isRecvFrame = TRUE;   // 受信したフラグを立てる
}

//----------------------------------------------------------------------------
/**
 *	@brief  経過フレーム取得
 *
 *	@param	CTVT_GAME_WORK  *gameWork 
 *
 *	@return u16 経過フレーム
 */
//-----------------------------------------------------------------------------
u16 CTVT_GAME_GetFrame( CTVT_GAME_WORK *gameWork )
{
  return gameWork->frame;
}

//----------------------------------------------------------------------------
/**
 *	@brief  親のフレーム取得
 *
 *	@param	CTVT_GAME_WORK  *gameWork 
 *
 *	@return u16 経過フレーム
 */
//-----------------------------------------------------------------------------
u16 CTVT_GAME_GetOyaFrame( CTVT_GAME_WORK *gameWork )
{
  return gameWork->oya_frame;
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信からTP座標が送られてきた場合の設定関数
 *	        内部で当たり判定する
 *
 *	@param	commWork    通信構造体
 *	        p_wk        ゲーム管理構造体
 *	        data        設定するゲームデータ
 *	        netID　     送信者のNetID
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommDataSet_TP( CTVT_COMM_WORK *commWork, CTVT_GAME_WORK *p_wk, const CTVT_GAME_SEND_DATA *data, int netID )
{
  // 親しかここは通らないはず
  BOOL  ret;
  u16   ret_idx;    // どの風船が当たったかIDXを格納

  ret = balloon_hit_check( p_wk, data->tpx, data->tpy, &ret_idx );
  // Hit Check
  // 当たりがある → 送信
  if( ret == TRUE )
  {
    CTVT_GAME_SEND_DATA send_data;
    send_data.burst_idx = ret_idx;
    send_data.burst_netID = netID;
    CTVT_COMM_SendGameReq( commWork, send_data, CCGDT_BURST );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  指定したIDXの風船を割る（タスクから消す）
 *	        スコアの計算も行う
 *
 *	@param	CTVT_GAME_WORK *p_wk
 *	@param	idx   割りたい風船のIDX(受信したものをそのまま指定してOK)
 *	@param  netID 割った人のnetID(受信したものをそのまま指定してOK)
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_BalloonDestroy( CTVT_GAME_WORK *p_wk, u16 idx, u8 netId )
{
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  // 得点計算

  // ポイント加算
  if( p_wk->balloon_wk[ idx ].unit_idx == netId )
  {
    if( p_wk->balloon_wk[ idx ].type == BALLOON_BONUS )
    {
      // ボーナス風船
      p_wk->score[ netId ] += 4;
    }
    else
    {
      // 通常風船
      p_wk->score[ netId ] += 2;
    }
    // 自分だけ鳴らす
    if( selfId == netId )
    {
      // SE再生
      PMSND_PlaySystemSE( SEQ_SE_LCG_05 );
    }
  }
  // ポイント減算
  else
  {
    if( p_wk->balloon_wk[ idx ].type == BALLOON_BONUS )
    {
      // ボーナス風船
      p_wk->score[ netId ] -= 2;
    }
    else
    {
      // 通常風船
      p_wk->score[ netId ] -= 1;
    }
    // マイナスにはならないようになった
    if( p_wk->score[ netId ] < 0 ){ p_wk->score[ netId] = 0; }

    // 自分だけ鳴らす
    if( selfId == netId )
    {
      // SE再生
      PMSND_PlaySystemSE( SEQ_SE_LCG_02 );
    }
  }

  // プレートの得点更新
  GAME_PLATE_SetPoint( p_wk, netId, p_wk->score[ netId ] );

  // タスク消去
  //CTVT_BALLOON_Destroy( &p_wk->balloon_wk[ idx ] );
  // 風船をHit時に変える -> 消去まで自動で行う
  p_wk->balloon_wk[ idx ].type = BALLOON_HIT;
  p_wk->balloon_wk[ idx ].anm_flg = TRUE;

}


//----------------------------------------------------------------------------
/**
 *	@brief  通信から送られてくるゲーム情報を格納
 *
 *	@param	p_wk    ゲーム管理構造体
 *	        score   設定するゲーム情報
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommScoreSet( CTVT_GAME_WORK *p_wk, const CTVT_GAME_SEND_SCORE *score )
{
  u8 i;

  for( i=0; i<CTVT_MEMBER_NUM; i++ )
  {
    // 受信データを更新
    p_wk->score[i] = score->score[i];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信からタッチしたと送られてきた時の処理関数
 *	        親しか呼ばない
 *
 *	@param	CTVT_COMM_WORK *commWork  通信ワーク
 *	@param	*p_wk                     ゲームワーク
 *	@param  pinpoint                  TRUEでピンポイントタッチ
 *	@param	netID                     ネットID
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommDataSet_Touch( CTVT_COMM_WORK *commWork, CTVT_GAME_WORK *p_wk, BOOL pinpoint, int netID )
{
  static const u8 touch_tbl[3] = 
  {
    TOUCH_CNT_1,
    TOUCH_CNT_2,
    TOUCH_CNT_3,
  };
  BOOL send = FALSE;

  GF_ASSERT( netID < CTVT_MEMBER_NUM );

  // タッチ数を計測しておく
  {
    u8  add = 1;

    // ピンポイントタッチの場合加算ポイント計算
    if( pinpoint == TRUE )
    {
      add = 2;
    }
    // 残り時間20秒以下でポイント倍
    if( p_wk->timeLimit <= 20 )
    {
      add *= 2;
    }

    // タッチ数加算
    p_wk->touch_cnt[ netID ] += add;

    // タッチ数判定
    {
      u8 tbl_idx = p_wk->level_cnt[ netID ];

      if( tbl_idx < 3 )
      {
        if( p_wk->touch_cnt[ netID ] >= touch_tbl[ tbl_idx ] &&
            CTVT_BALLOON2_IsEffect( p_wk->p_balloon2[netID] ) == FALSE )
        {
          p_wk->level_cnt[netID]++;   // タッチ段階上げる
          send = TRUE;
        }
      }
      // 破裂の判定
      else
      {
        // 順位によって追加タッチポイント
        const u8 add_cnt_tbl[] = {20,10,5,2};
        const u8 rank = p_wk->p_plate[netID]->rank;
        const u8 touch_cnt_max = TOUCH_CNT_3 + add_cnt_tbl[ rank ];

        if( p_wk->touch_cnt[ netID ] >= touch_cnt_max &&
            CTVT_BALLOON2_IsEffect( p_wk->p_balloon2[netID] ) == FALSE )
        {
          p_wk->touch_cnt[ netID ] = 0;
          p_wk->level_cnt[ netID ] = 0;
          send = TRUE;
        }
      }
    }
  }

  // 通信データ送信
  if( send == TRUE )
  {
    CTVT_GAME_SEND_DATA data ={0};
    //data.burst_netID = 1<<netID;
    data.burst_netID = netID;
    CTVT_COMM_SendGameReq( commWork, data, CCGDT_BURST2 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定したNetIDの風船を処理する
 *
 *	@param	CTVT_GAME_WORK *p_wk  ゲームワーク
 *	@param	netIdBit              処理する風船のNetID(ビット管理)
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommBalloonBurst( CTVT_GAME_WORK *p_wk, u8 netIdBit )
{
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  u8  i;

  for( i=0; i<CTVT_MEMBER_NUM; i++ )
  {
    // 処理する風船(=netID)をチェック
    if( netIdBit == i )
    {
      BOOL ret;
      // 風船大きくor破裂
      ret = CTVT_BALLOON2_Burst( p_wk->p_balloon2[i] );
      ICHI_Printf("風船BURST netID %d stage %d \n", i, p_wk->p_balloon2[i]->stage);

      // 破裂したなら得点加算
      if( ret == TRUE )
      {
        p_wk->score[ i ] += 2;
        // プレートの得点更新
        GAME_PLATE_SetPoint( p_wk, i, p_wk->score[ i ] );
      }

      // 自分だけ音を鳴らす
      if( selfId == i )
      {
        // SE再生
        if( ret == TRUE )
        {
          PMSND_PlaySystemSE( SEQ_SE_LCG_01 );    // 割れたSE
        }
        else
        {
          PMSND_PlaySystemSE( SEQ_SE_LCG_09 );    // 膨らみ小
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム 初期化
 *	        ミニゲーム1用
 *	        主にグラフィックの初期化をする
 *
 *	@param	COMM_TVT_WORK *work ワーク
 *	@param	*gameWork           ゲームワーク
 */
//-----------------------------------------------------------------------------
static void CTVT_GAME_Init( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  HEAPID  heapID = gameWork->heapId;
  u8  i;

  // GRAPHIC初期化
  GRAPHIC_Init( gameWork, heapID );

  // リソース
  {
    ARCHANDLE* arcHandle = GFL_ARC_OpenDataHandle( ARCID_TVT_BALLOON , heapID );
    CTVT_GAME_ID const game_id = CTVT_GAME_GetGameID( gameWork );

    switch( game_id ){
    case CGI_BALLOON1:
      RESOURCE_Init_Part1( gameWork, heapID, arcHandle );
      break;
    case CGI_BALLOON2:
      RESOURCE_Init_Part2( gameWork, heapID, arcHandle );
      break;
    }

    GFL_ARC_CloseDataHandle( arcHandle );
  }

  // メッセージ系初期化
  {
    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_MAIN_BG, CTVT_GAME_PAL_BG_M_FONT*0x20, 16*2, heapID );
    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_SUB_BG, CTVT_GAME_PAL_BG_S_FONT*0x20, 16*2, heapID );

    BmpWinFrame_GraphicSet( CTVT_GAME_FRAME_M_MSG, CTVT_GAME_BMPWIN_CGX_MAIN,
        CTVT_GAME_PAL_BG_M_WINFRAME, MENU_TYPE_SYSTEM, heapID );
    ICHI_Printf("メッセージ初期化\n");
  }

  // 変数初期化
  for( i=0; i<CTVT_MEMBER_NUM; i++ )
  {
    gameWork->p_balloon_s_wk[i] = NULL;
  }
  // CLWK
  for( i=0; i<4; i++ )
  {
    gameWork->clwkTitle[i] = NULL;            // タイトル
  }
  // ゲームプレイ中
  for( i=0; i<2; i++ )
  {
    gameWork->clwkCloud_M[i] = NULL;          // 雲
  }
  gameWork->clwkTime = NULL;                // タイムゲージ
  gameWork->clwkLast = NULL;                // ラストスパートの文字
  gameWork->clwkTouch = NULL;               // タッチするボタン
  gameWork->clwkPompBase = NULL;            // 空気入れ土台
  gameWork->clwkPompHandle = NULL;          // 空気入れ取っ手
  gameWork->clwkPenAnm = NULL;              // ペンタッチアニメ

  // 結果発表
  gameWork->clwkKekka = NULL;               // けっかはっぴょうの文字
  gameWork->clwkOmeFont = NULL;             // 「おめでとう」倍角
  for( i=0; i<6; i++ )
  {
    gameWork->clwkOmeText[i] = NULL;          // 「おめでとう」単文字 倍角
  }
  for( i=0; i<2; i++ )
  {
    gameWork->clwkOmeCloud[i] = NULL;         // エフェクト雲倍角
    gameWork->clwkOmeHide[i] = NULL;          // エフェクト隠すプレート
  }
  gameWork->clwkOmeNiji = NULL;             // エフェクト虹
  gameWork->clwkOmeStar = NULL;             // エフェクト星
  gameWork->clwkRank = NULL;                // 順位 
  gameWork->clwkDot = NULL;                 // 三点リーダー 3,4位のみ



  // 3D面のプライオリティ
  GFL_BG_SetPriority( CTVT_GAME_FRAME_M_3D, 2 ); 

  // 通信アイコン
  GFL_NET_ReloadIconTopOrBottom( TRUE , heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム 終了
 *
 *	@param	COMM_TVT_WORK *work ワーク
 *	@param	*gameWork           ゲームワーク
 */
//-----------------------------------------------------------------------------
static void CTVT_GAME_Exit( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork ) 
{
  // リソース破棄
  {
    CTVT_GAME_ID const game_id = CTVT_GAME_GetGameID( gameWork );

    switch( game_id ){
    case CGI_BALLOON1:
      RESOURCE_Exit_Part1( gameWork );
      break;
    case CGI_BALLOON2:
      RESOURCE_Exit_Part2( gameWork );
      break;
    }
  }

  // GRAPHIC破棄
  GRAPHIC_Exit( gameWork );
}

//----------------------------------------------------------------------------
/**
 *	@brief  エラー落ちの解放
 *
 *	@param	COMM_TVT_WORK *work 
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void exit_error( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  // 状態別解放
  switch( gameWork->state ){
  case CGS_CAMERA_MAIN:   // カメラメイン
  case CGS_CAMERA_EXIT:   // カメラフェードアウト中
    CTVT_GAME_CAM_Exit( gameWork->p_cam_wk );
    break;
  case CGS_GAME_MAIN:     // ゲーム中
    {
      const CTVT_GAME_ID game_id =  CTVT_GAME_GetGameID( gameWork );

      // 共通解放
      game_update_seq_exit_common( work, gameWork );

      // ゲーム別解放
      switch( game_id ){
      case CGI_BALLOON1:
        game_update_seq_exit_part1( work, gameWork );
        break;
      case CGI_BALLOON2:
        game_update_seq_exit_part2( work, gameWork );
        break;
      }

      // ゲーム大本の解放
      CTVT_GAME_Exit( work, gameWork );
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム  更新
 *
 *	@param	COMM_TVT_WORK *work ワーク
 *	@param	*gameWork           ゲームワーク
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL CTVT_GAME_Update( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  CTVT_CAMERA_WORK  *camWork  = COMM_TVT_GetCameraWork(work);
  CTVT_COMM_WORK    *commWork = COMM_TVT_GetCommWork( work );
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  // シーケンス管理
  enum{
    SEQ_INIT_RND,         // 乱数Seed値送る(親のみ)
    SEQ_INIT_SYNC_INIT,   // 初期化開始同期
    SEQ_INIT_SYNC_WAIT,   // 初期化開始同期待ち
    SEQ_INIT,             // 初期化
    SEQ_INIT_FADE,        // 初期化フェードイン
    SEQ_TITLE,            // タイトル
    SEQ_TITLE_FADE_WAIT,  // タイトルのフェード待ち
    // プレイ中
    SEQ_MAIN_SYNC_INIT,   // ゲーム開始同期
    SEQ_MAIN_SYNC_WAIT,   // ゲーム開始同期待ち
    SEQ_MAIN_PART1,       // ゲームメイン ゲーム1
    SEQ_MAIN_PART1_WAIT,  // 終了待ち ゲーム1
    SEQ_MAIN_PART2,       // ゲームメイン ゲーム2
    SEQ_SCORE_SYNC_INIT,  // スコア同期
    SEQ_SCORE_SYNC_WAIT,  // スコア同期待ち
    SEQ_SCORE_FADEOUT,    // スコアフェードアウト
    SEQ_SCORE_FADEOUT_WAIT,// スコアフェードアウト待ち
    // 結果発表
    SEQ_SCORE_INIT,       // スコア初期化
    SEQ_SCORE_FADEIN,     // スコアフェードイン
    SEQ_SCORE_FADEIN_WAIT,// スコアフェードイン待ち
    SEQ_SCORE_MAIN,       // スコアメイン

    // リトライ処理
    // 共通
    SEQ_RETRY_INIT,       // リトライ選択初期化
    SEQ_RETRY_EXIT,       // リトライ選択終了
    SEQ_RETRY_EXIT_WAIT,  // リトライ選択終了
    SEQ_YES_NO_WAIT,      // はい・いいえ選択待ち
    // 親
    SEQ_PARENT_RETRY,            // 子参加チェックして、開始通知
    SEQ_PARENT_RETRY2,           // 子参加チェックして、開始通知 確認用
    SEQ_PARENT_EXIT,             // 中止
    // 子
    SEQ_CHILD_RETRY,       // 開始待ち
    SEQ_CHILD_RETRY2,      // 開始待ち 確認用
    SEQ_CHILD_EXIT,        // 中止

    SEQ_EXIT_SYNC_INIT,   // ゲーム終了同期
    SEQ_EXIT_SYNC_WAIT,   // ゲーム終了同期待ち
    SEQ_EXIT,             // 終了
  };

  // シーケンス別処理
  switch( gameWork->seq ){
  case SEQ_INIT_RND:
    // 親なら乱数Seed値送信
    if( selfId == 0 )
    {
      CTVT_GAME_SEND_DATA data;

      RTCDate	date;
      RTCTime	time;
      u32		seed;

      GFL_RTC_GetDateTime(&date,&time);

      seed = date.year+date.month*0x100+date.day*0x10000+time.hour*0x100000+(time.minute+time.second)*0x1000000;

      // 通信データ送信
      data.rnd_seed = seed;
      CTVT_COMM_SendGameReq( commWork, data, CCGDT_RND );
    }
    ICHI_Printf("乱数シード値送信\n");
    gameWork->seq = SEQ_INIT_SYNC_INIT;
    break;
  case SEQ_INIT_SYNC_INIT:   // ゲーム開始同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_TITLE_SYNC, WB_NET_COMM_TVT );
      gameWork->seq = SEQ_INIT_SYNC_WAIT;

      ICHI_Printf("初期化開始同期待ち\n");
    }
    break;
  case SEQ_INIT_SYNC_WAIT:   // ゲーム開始同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_TITLE_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_INIT;
        ICHI_Printf("初期化開始同期完了\n");
      }
    }
    break;
  case SEQ_INIT:             // 初期化
    {
      const CTVT_GAME_ID game_id =  CTVT_GAME_GetGameID( gameWork );
      game_update_seq_init_common( work, gameWork );

      // ゲーム別処理
      switch( game_id ){
      case CGI_BALLOON1:
        game_update_seq_init_part1( work, gameWork );
        break;
      case CGI_BALLOON2:
        game_update_seq_init_part2( work, gameWork );
        break;
      };

      // ME再生 タイトル
      PMSND_PlayBGM( SEQ_ME_LCG_01 );

      gameWork->seq = SEQ_INIT_FADE;
      ICHI_Printf("Heap Free size[%x]\n",GFL_HEAP_GetHeapFreeSize(HEAPID_COMM_TVT));
      WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
          WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, gameWork->heapId );
    }
    break;
  case SEQ_INIT_FADE:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      gameWork->seq = SEQ_TITLE;
      G2S_BlendNone();
    }
    break;

  case SEQ_TITLE:
    if( game_update_seq_title( work, gameWork ) )
    {
      WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT , 
          WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, gameWork->heapId );

      // ME停止
      PMSND_StopBGM();
      gameWork->seq = SEQ_MAIN_SYNC_INIT;
    }
    break;

  case SEQ_MAIN_SYNC_INIT:   // ゲーム開始同期
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      // フェードアウトが終わったのでタイトル破棄＆ゲーム初期化
      {
        u8 i;

        // タイムゲージ表示
        GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTime, TRUE );

        // プレート表示
        for( i=0;i<CTVT_MEMBER_NUM; i++ )
        {
          GAME_PLATE_ActivePlate( work, gameWork->p_plate[i] );
        }

        // タイトルの説明文字破棄
        GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gameWork->msgWinTitleFont) , 0x0 );
        GFL_BMPWIN_ClearTransWindow( gameWork->msgWinTitleFont );

        BmpWinFrame_Clear( gameWork->msgWinTitle, WINDOW_TRANS_ON_V );
        GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gameWork->msgWinTitle) , 0x0 );
        GFL_BMPWIN_ClearTransWindow( gameWork->msgWinTitle );
        GFL_BG_LoadScreenReq(CTVT_GAME_FRAME_M_MSG);

        // タイムゲージ描画
        draw_timebar( gameWork );
      }

      {
        const CTVT_GAME_ID game_id =  CTVT_GAME_GetGameID( gameWork );
        // ゲーム別処理
        switch( game_id ){
        case CGI_BALLOON1:
          game_update_seq_title_end_part1( work, gameWork );
          break;
        case CGI_BALLOON2:
          game_update_seq_title_end_part2( work, gameWork );
          break;
        };
      }

      {
        GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
        GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_BEFORE_SYNC, WB_NET_COMM_TVT );
      }
      gameWork->seq = SEQ_MAIN_SYNC_WAIT;

      ICHI_Printf("ゲーム開始同期待ち\n");
    }
    break;

  case SEQ_MAIN_SYNC_WAIT:   // ゲーム開始同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_BEFORE_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_TITLE_FADE_WAIT;
        // BGM
        PMSND_PlayBGM( SEQ_BGM_LCG_01 );

        // BG位置戻す
        GFL_BG_SetScroll( CTVT_GAME_FRAME_S_BG, GFL_BG_SCROLL_Y_SET, 0 );

        WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN , 
            WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, gameWork->heapId );
        ICHI_Printf("ゲーム開始同期完了\n");
      }
    }
    break;

  case SEQ_TITLE_FADE_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      // プレート以外を暗くする
      // 背景を暗く
      G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0, -8 );
      gameWork->is_main = TRUE;

      {
        const CTVT_GAME_ID game_id =  CTVT_GAME_GetGameID( gameWork );

        // ゲーム別処理
        switch( game_id ){
        case CGI_BALLOON1:
          gameWork->seq = SEQ_MAIN_PART1;
          PMSND_PlaySystemSE( SEQ_SE_LCG_03 );    // スタートのホイッスル音
          break;
        case CGI_BALLOON2:
          gameWork->seq = SEQ_MAIN_PART2;

          // タッチする場所を分りやすくするため
          // ペンアニメを再生する
          GFL_CLACT_WK_SetDrawEnable( gameWork->clwkPenAnm, TRUE );
          GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkPenAnm, TRUE );
          GFL_CLACT_WK_ResetAnm( gameWork->clwkPenAnm );
          break;
        };
      }
    }
    break;
 
  case SEQ_MAIN_PART1:             // ゲームメイン
    if( game_update_seq_main_part1( work, gameWork ) )
    {
      gameWork->is_main = FALSE;
      gameWork->seq = SEQ_MAIN_PART1_WAIT;

      // BGM停止
      PMSND_StopBGM();
    }
    break;


  case SEQ_MAIN_PART1_WAIT:  // 終了待ち ゲーム1
    {
      // 風船だけ更新

      // 経過フレーム数をインクリメント
      // 親、子関係なくしないと、子が止まる可能性がある
      gameWork->frame++;

      GRAPHIC_Draw( gameWork );

      ICHI_Printf("風船が全部飛び待ち frame %d\n", gameWork->frame);
      // 全ての風船の終了まで待つ
      if( balloon_end_check( gameWork ) == TRUE )
      {
        gameWork->seq = SEQ_SCORE_SYNC_INIT;
      }
    }
    break;

  case SEQ_MAIN_PART2:             // ゲームメイン
    if( game_update_seq_main_part2( work, gameWork ) )
    {
      gameWork->is_main = FALSE;
      gameWork->seq = SEQ_SCORE_SYNC_INIT;

      // BGM停止
      PMSND_StopBGM();
    }
    break;

  case SEQ_SCORE_SYNC_INIT:  // スコア同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_SCORE_SYNC, WB_NET_COMM_TVT );
      gameWork->oya_frame = 0;    // 0以上だと通信メインで自動送信してしまう
      gameWork->seq = SEQ_SCORE_SYNC_WAIT;

      ICHI_Printf("スコア同期待ち\n");
    }
    break;
  case SEQ_SCORE_SYNC_WAIT:  // スコア同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_SCORE_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_SCORE_FADEOUT;
        ICHI_Printf("スコア同期完了\n");
      }
    }
    break;
  case SEQ_SCORE_FADEOUT:    // スコアフェードアウト
    WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT , 
        WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, gameWork->heapId );
    gameWork->seq = SEQ_SCORE_FADEOUT_WAIT;
    break;

  case SEQ_SCORE_FADEOUT_WAIT:// スコアフェードアウト待ち
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      gameWork->seq = SEQ_SCORE_INIT;
    }
    break;

  case SEQ_SCORE_INIT:       // スコア初期化
    game_update_seq_score_init( work, gameWork );
    gameWork->seq = SEQ_SCORE_FADEIN;
    break;

  case SEQ_SCORE_FADEIN:     // スコアフェードイン
    WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
        WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, gameWork->heapId );
    gameWork->seq = SEQ_SCORE_FADEIN_WAIT;
    break;

  case SEQ_SCORE_FADEIN_WAIT:// スコアフェードイン待ち
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      // ME再生 ドラムロール
      PMSND_PlayBGM( SEQ_ME_LCG_03 );

      gameWork->seq = SEQ_SCORE_MAIN;
    }
    break;

  case SEQ_SCORE_MAIN:       // スコアメイン
    if( game_update_seq_score( work, gameWork ) == TRUE )
    {
      gameWork->seq = SEQ_RETRY_INIT;
    }
    break;

  case SEQ_RETRY_INIT:       // リトライ選択初期化
    {
      // 画面を暗くする
      G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ , -8 );

      // リトライメッセージ表示
      BmpWinFrame_GraphicSet( CTVT_GAME_FRAME_M_MSG, CTVT_GAME_BMPWIN_CGX_MAIN,
          CTVT_GAME_PAL_BG_M_WINFRAME, MENU_TYPE_SYSTEM, gameWork->heapId );
      disp_message_retry( work, gameWork, COMM_TVT_GAME_17 );

      // APPTASK用共通リソース読み込み
      gameWork->taskMenuRes = APP_TASKMENU_RES_Create( CTVT_GAME_FRAME_M_MSG, 
                                               CTVT_GAME_PAL_BG_M_TASKMENU,
                                               COMM_TVT_GetFontHandle( work ),
                                               COMM_TVT_GetPrintQue( work ), 
                                               gameWork->heapId );
      // はい・いいえ選択表示
      gameWork->yesNoWork = retry_open_yesno_menu( work, gameWork );
      APP_TASKMENU_SetDisableKey( gameWork->yesNoWork, TRUE );

      // その他初期化
      gameWork->isRetry  = FALSE;
      gameWork->msgWait = 0;
      gameWork->retry_cnt   = 0;
      gameWork->isCancelGame= FALSE;
      gameWork->isExitGame  = FALSE;
      gameWork->isStartGame = FALSE;
      gameWork->isStartGame2= FALSE;
      gameWork->acceptBit   = 0;
      gameWork->acceptBit2  = 0;

      gameWork->seq = SEQ_YES_NO_WAIT;
      ICHI_Printf("はいいいえ選択表示\n");
    }
    break;

  case SEQ_RETRY_EXIT:       // リトライ選択終了
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
        WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, gameWork->heapId );
    gameWork->seq = SEQ_RETRY_EXIT_WAIT;
    break;

  case SEQ_RETRY_EXIT_WAIT:       // リトライ選択終了
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      // はい・いいえウィンドウ削除
      if( gameWork->yesNoWork != NULL )
      {
        APP_TASKMENU_CloseMenu( gameWork->yesNoWork );
      }
      // キャンセルボタン削除
      if( gameWork->cancelWork != NULL )
      {
        APP_TASKMENU_CloseMenu( gameWork->cancelWork );
        gameWork->cancelWork = NULL;
      }

      // APPTASK共通リソース破棄
      if( gameWork->taskMenuRes != NULL )
      {
        APP_TASKMENU_RES_Delete( gameWork->taskMenuRes );
        gameWork->taskMenuRes = NULL;
      }

      // メッセージ削除
      gameWork->isUpdateMsgRetry = FALSE;
      BmpWinFrame_Clear( gameWork->msgRetry, WINDOW_TRANS_ON_V );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gameWork->msgRetry) , 0x0 );
      GFL_BMPWIN_ClearTransWindow( gameWork->msgRetry );
      GFL_BG_LoadScreenReq(CTVT_GAME_FRAME_M_MSG);

      gameWork->seq = SEQ_EXIT_SYNC_INIT;
      ICHI_Printf("isRetry = %d \n", gameWork->isRetry);
    }
    break;

  case SEQ_YES_NO_WAIT:      // はい・いいえ選択待ち
    if( retry_check_finish_req( work, gameWork ) == FALSE )
    {
      gameWork->retry_cnt++;
      APP_TASKMENU_UpdateMenu( gameWork->yesNoWork );
      if( APP_TASKMENU_IsFinish( gameWork->yesNoWork ) == TRUE )
      {
        const u8 retVal = APP_TASKMENU_GetCursorPos( gameWork->yesNoWork );
        if( retVal == 0 )
        {
          // はい
          // 開始までしばらくお待ち～メッセージ表示
          disp_message_retry( work, gameWork, COMM_TVT_GAME_02 );
          // キャンセルボタン表示
          gameWork->cancelWork = retry_open_cancel_menu( work, gameWork );

          if( selfId == 0 )
          {
            gameWork->seq = SEQ_PARENT_RETRY;        // 親 開始通知
          }
          else
          {
            gameWork->seq = SEQ_CHILD_RETRY;   // 子 開始待ち
          }
          ICHI_Printf("はいを選んだ\n");
        }
        else
        {
          // いいえ
          // ちゅうしされました メッセージ表示
          disp_message_retry( work, gameWork, COMM_TVT_GAME_03 );
          gameWork->msgWait = 150;    // メッセージ表示待ち時間
          if( selfId == 0 )
          {
            gameWork->seq = SEQ_PARENT_EXIT;        // 親 中止
          }
          else
          {
            gameWork->seq = SEQ_CHILD_EXIT;   // 子 中止
          }
          ICHI_Printf("いいえを選んだ\n");
        }
        APP_TASKMENU_CloseMenu( gameWork->yesNoWork );
        gameWork->yesNoWork = NULL;
      }
      // 一定時間経過すると「いいえ」を選択したことにする
      else if( gameWork->retry_cnt > (CTVT_GAME_TIME_RETRY_NO*60) )
      {
        // いいえ
        // ちゅうしされました メッセージ表示
        disp_message_retry( work, gameWork, COMM_TVT_GAME_03 );
        gameWork->msgWait = 150;    // メッセージ表示待ち時間
        if( selfId == 0 )
        {
          gameWork->seq = SEQ_PARENT_EXIT;        // 親 中止
        }
        else
        {
          gameWork->seq = SEQ_CHILD_EXIT;   // 子 中止
        }
        APP_TASKMENU_CloseMenu( gameWork->yesNoWork );
        gameWork->yesNoWork = NULL;
        ICHI_Printf("いいえを選んだ\n");
      }
    }
    else
    {
      if( selfId == 0 )
      {
        gameWork->seq = SEQ_PARENT_EXIT;        // 親
      }
      else
      {
        gameWork->seq = SEQ_CHILD_EXIT;   // 子
      }
    }
    break;

    // 親
  case SEQ_PARENT_RETRY:            // 開始通知
    if( retry_check_finish_req( work, gameWork ) == FALSE )
    {
      // キャンセルボタン処理
      APP_TASKMENU_UpdateMenu( gameWork->cancelWork );
      if( APP_TASKMENU_IsFinish( gameWork->cancelWork ) == TRUE )
      {
        const u8 retVal = APP_TASKMENU_GetCursorPos( gameWork->cancelWork );
        if( retVal == 0 )
        {
          // キャンセルした
          disp_message_retry( work, gameWork, COMM_TVT_GAME_03 );
          gameWork->msgWait = 150;    // メッセージ表示待ち時間
          gameWork->seq = SEQ_PARENT_EXIT;        // 親 中止
          ICHI_Printf("キャンセルを選んだ\n");
        }
        
        APP_TASKMENU_CloseMenu( gameWork->cancelWork );
        gameWork->cancelWork = NULL;
      }

      // 子が全員参加したかチェック
      if( CTVT_GAME_IsAcceptNum( work, gameWork ) )
      {
        // 開始要求を送信し、確認待ちへ移行
        {
          CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
          CTVT_COMM_SendGameReq_Flg( work, commWork, CCGF_START_RETRY );
          gameWork->seq = SEQ_PARENT_RETRY2;
          ICHI_Printf("子が全員リトライを選んだ\n");
        }
      }
    }
    else
    {
      // 中止
      gameWork->seq = SEQ_PARENT_EXIT;        // 親
    }
    break;
  case SEQ_PARENT_RETRY2:           // 開始通知 確認用
    if( retry_check_finish_req( work, gameWork ) == FALSE )
    {
      // 子が全員参加したかチェック
      if( CTVT_GAME_IsAcceptNum2( work, gameWork ) )
      {
        // 開始要求 確認用を送信し、確認待ちへ移行
        {
          CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
          CTVT_COMM_SendGameReq_Flg( work, commWork, CCGF_START_RETRY2 );
          gameWork->isRetry = TRUE;
          gameWork->seq = SEQ_RETRY_EXIT;
          ICHI_Printf("子が全員リトライを選んだ 確認用\n");
        }
      }
    }
    else
    {
      // 中止
      gameWork->seq = SEQ_PARENT_EXIT;        // 親
    }
    break;
  case SEQ_PARENT_EXIT:             // 中止
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );

      if( gameWork->msgWait > 0 )
      {
        gameWork->msgWait--;
      }
      if( gameWork->msgWait == 0 )
      {
        CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
        CTVT_COMM_SendGameReq_Flg( work, commWork, CCGF_EXIT_RETRY );
        gameWork->seq = SEQ_RETRY_EXIT;
        if( gameWork->cancelWork != NULL )
        {
          APP_TASKMENU_CloseMenu( gameWork->cancelWork );
          gameWork->cancelWork = NULL;
        }
      }
    }
    break;
    // 子
  case SEQ_CHILD_RETRY:       // 開始待ち
    if( retry_check_finish_req( work, gameWork ) == FALSE )
    {
      // キャンセルボタン処理
      APP_TASKMENU_UpdateMenu( gameWork->cancelWork );
      if( APP_TASKMENU_IsFinish( gameWork->cancelWork ) == TRUE )
      {
        const u8 retVal = APP_TASKMENU_GetCursorPos( gameWork->cancelWork );
        if( retVal == 0 )
        {
          // キャンセルした
          disp_message_retry( work, gameWork, COMM_TVT_GAME_03 );
          gameWork->seq = SEQ_CHILD_EXIT;        // 親 中止
          ICHI_Printf("キャンセルした\n");
          gameWork->msgWait = 150;    // メッセージ表示待ち時間
        }
        
        APP_TASKMENU_CloseMenu( gameWork->cancelWork );
        gameWork->cancelWork = NULL;
      }

      // リトライ決定 常時送信
      CTVT_COMM_SendGameReq_Flg( work, commWork, CCGF_DECIDE_RETRY );

      // 親から開始通知が来た
      if( gameWork->isStartGame == TRUE )
      {
        gameWork->isStartGame = FALSE;
        gameWork->seq = SEQ_CHILD_RETRY2;
        ICHI_Printf("親から開始通知がきた\n");
      }
    }
    else
    {
      // 中止
      gameWork->seq = SEQ_CHILD_EXIT; // 
    }
    break;
  case SEQ_CHILD_RETRY2:      // 開始待ち 確認用
    if( retry_check_finish_req( work, gameWork ) == FALSE )
    {
      // リトライ決定 確認用 常時送信
      CTVT_COMM_SendGameReq_Flg( work, commWork, CCGF_DECIDE_RETRY2 );

      // 親からの開始通知 確認版がきた
      if( gameWork->isStartGame2 == TRUE )
      {
        gameWork->isRetry = TRUE;
        gameWork->isStartGame2 = FALSE;
        gameWork->seq = SEQ_RETRY_EXIT;
        ICHI_Printf("親から開始通知がきた 確認用\n");
      }
    }
    else
    {
      // 中止
      gameWork->seq = SEQ_CHILD_EXIT; // 
    }
    break;
  case SEQ_CHILD_EXIT:        // 中止
    {
      if( gameWork->msgWait > 0 )
      {
        gameWork->msgWait--;
      }
      if( gameWork->msgWait == 0 )
      {
        if( retry_check_finish_req( work, gameWork ) == FALSE )
        {
          CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );

          // キャンセル通知
          // 常時終了チェックで中止の処理をしている
          CTVT_COMM_SendGameReq_Flg( work, commWork, CCGF_CANCEL_RETRY );
        }
        else
        {
          // 中止
          gameWork->seq = SEQ_RETRY_EXIT; // 
          ICHI_Printf("子の中止 親から中止通知が来た\n");
        }
      }
    }
    break;

  case SEQ_EXIT_SYNC_INIT:   // ゲーム終了同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_END_SYNC, WB_NET_COMM_TVT );
      gameWork->seq = SEQ_EXIT_SYNC_WAIT;

      ICHI_Printf("ゲーム終了同期待ち\n");
    }
    break;
  case SEQ_EXIT_SYNC_WAIT:   // ゲーム終了同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_END_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_EXIT;
        ICHI_Printf("ゲーム終了同期完了\n");
      }
    }
    break;
  case SEQ_EXIT:             // 終了
    {
      const CTVT_GAME_ID game_id =  CTVT_GAME_GetGameID( gameWork );
      game_update_seq_exit_common( work, gameWork );

      // ゲーム別処理
      switch( game_id ){
      case CGI_BALLOON1:
        game_update_seq_exit_part1( work, gameWork );
        break;
      case CGI_BALLOON2:
        game_update_seq_exit_part2( work, gameWork );
        break;
      };

      gameWork->seq = 0;    // 次モードの為、初期化
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:初期化
 *	        ゲーム1･2共通
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_init_common( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;
  const HEAPID heapID = gameWork->heapId;
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  const u8 connectNum = COMM_TVT_GetConnectNum( work );

  // 乱数初期化
  GFL_STD_RandInit( &gRandContext, gameWork->rnd_seed );
  ICHI_Printf("乱数Seed値 %d \n", gameWork->rnd_seed);
  ICHI_Printf("送信データサイズ:%d \n", sizeof(CTVT_GAME_SEND_DATA));


  // OBJ作成
  {
    GFL_CLWK_DATA cellInitData;

    // ゲームプレイ中

    // タイムゲージ
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 168;
    cellInitData.anmseq = CGOAS_TIMEBAR;
    cellInitData.bgpri = 1;

    gameWork->clwkTime = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_UPPER_PLT],
        gameWork->cellResIdx[CGOR_UPPER_NCG],
        gameWork->cellResIdx[CGOR_UPPER_ANM],
        &cellInitData ,CLSYS_DEFREND_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTime, FALSE );

    // ラストスパートの文字
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.anmseq = CGOAS_LAST;
    cellInitData.bgpri = 1;

    gameWork->clwkLast = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_UPPER_PLT],
        gameWork->cellResIdx[CGOR_UPPER_NCG],
        gameWork->cellResIdx[CGOR_UPPER_ANM],
        &cellInitData ,CLSYS_DEFREND_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkLast, FALSE );
  }
  
  // プレート作成
  {
    // プレート用タスク作成
    // タスクシステムを作るのに必要なメモリサイズを計算
    gameWork->taskmem_plate = GFL_HEAP_AllocMemory( gameWork->heapId, GFL_TCB_CalcSystemWorkSize(PLATE_NUM) );
    // タスクシステムを作成
    gameWork->task_plate = GFL_TCB_Init( PLATE_NUM, gameWork->taskmem_plate );

    for( i=0; i<connectNum; i++ )
    {
      gameWork->p_plate[i] = GAME_PLATE_CreatePlate( work, gameWork, i );
      // タスクに登録
      gameWork->p_plate[i]->tcb = GFL_TCB_AddTask( gameWork->task_plate, TaskFuncPlate, gameWork->p_plate[i], 1 );
      ICHI_Printf("プレート %d 作成\n", i);
    }
    for( ;i<CTVT_MEMBER_NUM; i++ )
    {
      gameWork->p_plate[i] = GAME_PLATE_CreatePlate_Blank( work, gameWork, i );
      gameWork->p_plate[i]->tcb = GFL_TCB_AddTask( gameWork->task_plate, TaskFuncPlate, gameWork->p_plate[i], 1 );
      ICHI_Printf("プレート空 %d 作成\n", i);
    }

    gameWork->plate_move = FALSE;
  }

  // BMPWIN作成
  {
    // タイムゲージ用BMPWIN
    gameWork->bmpWinTime = GFL_BMPWIN_Create( CTVT_GAME_FRAME_S_TIMEBAR, 
        TIME_BAR_X, TIME_BAR_Y, TIME_BAR_W, TIME_BAR_H, CTVT_GAME_PAL_BG_S_TIMEBAR, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gameWork->bmpWinTime), 0x0 );

    // ゲームルール用
    gameWork->msgWinTitleFont = GFL_BMPWIN_Create( CTVT_GAME_FRAME_M_PLATE, 
        8, 0, 16, 4, CTVT_GAME_PAL_BG_M_FONT, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gameWork->msgWinTitleFont), 0x0 );

    // 説明文字
    gameWork->msgWinTitle = GFL_BMPWIN_Create( CTVT_GAME_FRAME_M_MSG, 
        1, 18, 30, 6, CTVT_GAME_PAL_BG_M_FONT, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gameWork->msgWinTitle ), 0x0 );

    // リトライ用
    gameWork->msgRetry = GFL_BMPWIN_Create( CTVT_GAME_FRAME_M_MSG , 
        1, 8, 30, 6, CTVT_GAME_PAL_BG_M_FONT, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gameWork->msgRetry ), 0x0 );
  }

  // 値初期化
  for( i=0; i<CTVT_MEMBER_NUM; i++ )
  {
    gameWork->score[i] = 0;
  }
  gameWork->balloon_rnd_w = BALLOON_RND_W_MAX + 1;  // +1は最初の乱数がどれでも通るように
  gameWork->balloon_rnd_v = BALLOON_RND_V_MAX + 1;  // +1は最初の乱数がどれでも通るように
  gameWork->balloon_z_cnt = 0;
  gameWork->balloon_flg   = 0;
  gameWork->balloon_wait  = 0;
  gameWork->balloon_tbl_idx = 0;
  gameWork->balloon_tbl_cnt = 0;
  gameWork->timeLimit     = CTVT_GAME_TIME_LIMIT;
  gameWork->befFrameCnt   = 0;
  gameWork->rnd_seed      = 0;
  gameWork->frame         = 0;
  gameWork->oya_frame     = 0;
  gameWork->is_main       = FALSE;
  gameWork->isRecvFrame   = FALSE;
  gameWork->title_cnt = 0;

  ICHI_Printf("ゲーム共通初期化完了\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:初期化
 *	        ゲーム1専用
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_init_part1( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;
  const HEAPID heapID = gameWork->heapId;
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  const u8 connectNum = COMM_TVT_GetConnectNum( work );

#ifndef CAMERA_SKIP
  // 3Dにカメラキャプチャを貼り付ける
  for(i=0; i<CTVT_MEMBER_NUM; i++ )
  {
    CTVT_GAME_BalloonTexTrans( gameWork, i, i );
  }
#endif

  // ゲーム用のリソース
  // BGロード
  {
    // 下画面
    GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_explain_bg1_NCLR, 
        PALTYPE_MAIN_BG, CTVT_GAME_PAL_BG_M_BG*32, 32*3, heapID );
    GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_explain_bg1_NCGR,
        CTVT_GAME_FRAME_M_BG, 0, 0, FALSE, heapID );
    GFL_ARC_UTIL_TransVramScreen( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_explain_bg1_NSCR, 
        CTVT_GAME_FRAME_M_BG, 0, 0, FALSE, heapID );

    // 上画面
    GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NCLR , 
        PALTYPE_SUB_BG, CTVT_GAME_PAL_BG_S_BG*32, 32*3, heapID );
    GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NCGR,
        CTVT_GAME_FRAME_S_BG, 0, 0, FALSE, heapID );
    GFL_ARC_UTIL_TransVramScreen( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NSCR, 
        CTVT_GAME_FRAME_S_BG,  0, 0, FALSE, heapID );

    // タイムゲージ用にパレットを読み込む
    GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_time_bar_NCLR, 
        PALTYPE_SUB_BG, CTVT_GAME_PAL_BG_S_TIMEBAR*32, 32, heapID );
    ICHI_Printf("タイトルBG読み込み\n");
  }

  // OBJ作成
  {
    GFL_CLWK_DATA cellInitData;

    // タイトル
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 1;

    for( i=0; i<3; i++ )
    {
      cellInitData.anmseq = CGOAS_TITLE1 + i;
      cellInitData.softpri = i;

      gameWork->clwkTitle[i] = GFL_CLACT_WK_Create( gameWork->cellUnit,
          gameWork->cellResIdx[CGOR_UPPER_PLT],
          gameWork->cellResIdx[CGOR_UPPER_NCG],
          gameWork->cellResIdx[CGOR_UPPER_ANM],
          &cellInitData ,CLSYS_DEFREND_SUB, heapID );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitle[i] , TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkTitle[i] , TRUE );
    }
  }
  // OBJ作成  倍角を使っているのでaffineで。
  {
    GFL_CLWK_AFFINEDATA cellAffineData;
    GFL_STD_MemClear( &cellAffineData, sizeof(GFL_CLWK_AFFINEDATA) );

    // 雲
    cellAffineData.clwkdata.pos_x  = 60;
    cellAffineData.clwkdata.pos_y  = 50;
    cellAffineData.clwkdata.anmseq = CGOAM_CLOUD1;
    cellAffineData.clwkdata.softpri= 0;
    cellAffineData.clwkdata.bgpri  = 3;

    cellAffineData.affinepos_x  = 0;
    cellAffineData.affinepos_y  = 0;
    cellAffineData.scale_x      = FX32_ONE;
    cellAffineData.scale_y      = FX32_ONE;
    cellAffineData.rotation     = 0;
    cellAffineData.affine_type  = CLSYS_AFFINETYPE_DOUBLE;

    gameWork->clwkCloud_M[0] = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkCloud_M[0], FALSE );
    GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkCloud_M[0], TRUE );

    cellAffineData.clwkdata.pos_x  = 200;
    cellAffineData.clwkdata.pos_y  = 90;
    cellAffineData.clwkdata.anmseq = CGOAM_CLOUD2;
    cellAffineData.clwkdata.bgpri  = 3;

    gameWork->clwkCloud_M[1] = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkCloud_M[1], FALSE );
    GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkCloud_M[1], TRUE );
  }
  ICHI_Printf("セルaffie作成\n");


  // 風船用タスク作成
  // タスクシステムを作るのに必要なメモリサイズを計算
  gameWork->taskMem = GFL_HEAP_AllocMemory( gameWork->heapId, GFL_TCB_CalcSystemWorkSize(BALLOON_MAX) );
  // タスクシステムを作成
  gameWork->task = GFL_TCB_Init( BALLOON_MAX, gameWork->taskMem );

  // 風船初期化
  for( i=0; i<BALLOON_MAX; i++ )
  {
    CTVT_BALLOON_Init( &gameWork->balloon_wk[i] );
  }


  // タイトルの文字描画
  {
    GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
    GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    STRBUF *str = GFL_MSG_CreateString( msgHandle, COMM_TVT_GAME_20 );
    const u8 nameLen = PRINTSYS_GetStrWidth( str, fontHandle, 0 );

    // ゲームルール
    PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp( gameWork->msgWinTitleFont ), 
        64 - (nameLen/2), 12, str, fontHandle,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    GFL_BMPWIN_TransVramCharacter( gameWork->msgWinTitleFont );
    GFL_BMPWIN_MakeScreen( gameWork->msgWinTitleFont );
    gameWork->isUpdateMsgTitleFont = TRUE;

    // 説明画面
    {
      WORDSET *wordSet= WORDSET_Create( gameWork->heapId );
      STRBUF *tempStr = GFL_STR_CreateBuffer( 64 , gameWork->heapId );

      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gameWork->msgWinTitle ), 0xF );
      str = GFL_MSG_CreateString( msgHandle, COMM_TVT_GAME_15 );

      // 風船色
      {
        u32 strID = COMM_TVT_GAME_24 + selfId;
        STRBUF *gameName = GFL_MSG_CreateString( msgHandle, strID );
        WORDSET_RegisterWord( wordSet, 0, gameName, 0, TRUE, PM_LANG );
        GFL_STR_DeleteBuffer( gameName );
      }
      WORDSET_ExpandStr( wordSet, tempStr, str );
      GFL_STR_DeleteBuffer( str );
      str = tempStr;

      PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp( gameWork->msgWinTitle ), 
          0, 0, str, fontHandle,CTVT_FONT_COLOR_BLACK );
      GFL_STR_DeleteBuffer( str );
      WORDSET_Delete( wordSet );
      GFL_BMPWIN_TransVramCharacter( gameWork->msgWinTitle );
      GFL_BMPWIN_MakeScreen( gameWork->msgWinTitle );
      BmpWinFrame_Write( gameWork->msgWinTitle, WINDOW_TRANS_ON_V, 
          CTVT_GAME_BMPWIN_CGX_MAIN, CTVT_GAME_PAL_BG_M_WINFRAME );
      gameWork->isUpdateMsgTitle = TRUE;
    }
  }

  // ボーナス風船をいつ出すかきめる
  for( i=0; i<CTVT_MEMBER_NUM; i++ )
  {
    ICHI_Printf("player %d ");

    gameWork->bonus_tbl_idx[i] = 0;
    gameWork->bonus_tbl[i][0] = ( get_rand(10) + 40 ); // 50～40
    gameWork->bonus_tbl[i][1] = ( get_rand(10) + 30 ); // 40～30
    gameWork->bonus_tbl[i][2] = ( get_rand(5) + 15 );  // 20～10
    gameWork->bonus_tbl[i][3] = ( get_rand(5) + 10 );  // 10～5
    gameWork->bonus_tbl[i][4] = ( get_rand(5) + 5 );   // 5～0

    ICHI_Printf("%d ", gameWork->bonus_tbl[i][0]);
    ICHI_Printf("%d ", gameWork->bonus_tbl[i][1]);
    ICHI_Printf("%d ", gameWork->bonus_tbl[i][2]);
    ICHI_Printf("%d ", gameWork->bonus_tbl[i][3]);
    ICHI_Printf("%d \n", gameWork->bonus_tbl[i][4]);
  }

#ifdef PM_DEBUG
  gameWork->debug_r    = 0x45;    // 半径計算用 サイズに掛ける割合 0x45は目安
  gameWork->debug_size = 0x200;   // サイズに掛ける値
  gameWork->debug_v    = 0x2000;  // ベクトルに掛ける値 
  gameWork->debug_wait = 15;      // 風船出現間隔(フレーム)
  GFL_STD_MemFill( gameWork->debug_b_cnt, 0, sizeof(u16)*4 );
#endif

  // サウンド
  {
    NNSSndHeapHandle  snd_heap = PMSND_GetSndHeap();
    // 前の状態保存
    {
      int ret;
      ret = NNS_SndHeapSaveState( snd_heap );
      GF_ASSERT_MSG( (ret!=-1), "サウンドヒープ状態保存失敗\n");
    }
    // 新しいグループ読み込み
    {
      BOOL ret;
      ret = NNS_SndArcLoadGroup(GROUP_LCG01, snd_heap);
      GF_ASSERT(ret);
    }
    OS_Printf("サウンドグループ読み込み GROUPID %d\n", GROUP_LCG01);
  }

  ICHI_Printf("ゲーム1初期化完了\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:初期化
 *	        ゲーム2専用
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_init_part2( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;
  const HEAPID heapID = gameWork->heapId;
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  const u8 connectNum = COMM_TVT_GetConnectNum( work );

#ifndef CAMERA_SKIP
  // 3Dにカメラキャプチャを貼り付ける
  for(i=0; i<CTVT_MEMBER_NUM; i++ )
  {
    CTVT_GAME_BalloonTexTrans( gameWork, i, 0 );
  }
#endif

  // ゲーム用のリソース
  // BGロード
  {
    // 下画面
    GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_explain_bg2_NCLR, 
        PALTYPE_MAIN_BG, CTVT_GAME_PAL_BG_M_BG*32, 32*6, heapID );
    GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_explain_bg2_NCGR,
        CTVT_GAME_FRAME_M_BG, 0, 0, FALSE, heapID );
    GFL_ARC_UTIL_TransVramScreen( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_explain_bg2_NSCR, 
        CTVT_GAME_FRAME_M_BG, 0, 0, FALSE, heapID );

    // 上画面
    GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NCLR , 
        PALTYPE_SUB_BG, CTVT_GAME_PAL_BG_S_BG*32, 32*3, heapID );
    GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NCGR,
        CTVT_GAME_FRAME_S_BG, 0, 0, FALSE, heapID );
    GFL_ARC_UTIL_TransVramScreen( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NSCR, 
        CTVT_GAME_FRAME_S_BG,  0, 0, FALSE, heapID );

    // BGパレットアニメ用
    {
      u16 *dat;
      gameWork->bg_pal_anm = 0;
      gameWork->p_plt_buff = GFL_ARC_UTIL_LoadPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NCLR, &gameWork->p_plt_data, gameWork->heapId );
      dat= (u16*)gameWork->p_plt_data->pRawData;
      GFL_BG_LoadPalette( CTVT_GAME_FRAME_S_BG, &dat[16], 0x20, 0 );
    }

    // タイムゲージ用にパレットを読み込む
    GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_time_bar_NCLR, 
        PALTYPE_SUB_BG, CTVT_GAME_PAL_BG_S_TIMEBAR*32, 32, heapID );
    ICHI_Printf("タイトルBG読み込み\n");
  }

  // OBJ作成
  {
    GFL_CLWK_DATA cellInitData;

    // タイトル
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.softpri = 1;
    cellInitData.bgpri = 1;

    for( i=0; i<2; i++ )
    {
      cellInitData.anmseq = CGOAS_TITLE1 + i;
      cellInitData.softpri = i;

      gameWork->clwkTitle[i] = GFL_CLACT_WK_Create( gameWork->cellUnit,
          gameWork->cellResIdx[CGOR_UPPER_PLT],
          gameWork->cellResIdx[CGOR_UPPER_NCG],
          gameWork->cellResIdx[CGOR_UPPER_ANM],
          &cellInitData ,CLSYS_DEFREND_SUB, heapID );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitle[i] , TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkTitle[i] , TRUE );
    }
    // part2の虹
    cellInitData.anmseq = CGOAS_RAINBOW2;
    cellInitData.softpri = 3;
    gameWork->clwkTitle[2] = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_UPPER_PLT],
        gameWork->cellResIdx[CGOR_UPPER_NCG],
        gameWork->cellResIdx[CGOR_UPPER_ANM],
        &cellInitData ,CLSYS_DEFREND_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitle[2] , TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkTitle[2] , TRUE );
    // part2の「2」
    cellInitData.anmseq = CGOAS_PART2;
    cellInitData.softpri = 3;
    gameWork->clwkTitle[3] = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_UPPER_PLT],
        gameWork->cellResIdx[CGOR_UPPER_NCG],
        gameWork->cellResIdx[CGOR_UPPER_ANM],
        &cellInitData ,CLSYS_DEFREND_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitle[3] , TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkTitle[3] , TRUE );

    // タッチするボタン
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.softpri = 1;
    cellInitData.bgpri = 1;
    cellInitData.anmseq = CGOAM_BUTTON_STAY;
    gameWork->clwkTouch = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellInitData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTouch, FALSE );

    // 空気入れ土台
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.softpri = 2;
    cellInitData.bgpri = 1;
    cellInitData.anmseq = CGOAM_POMP_BASE;
    gameWork->clwkPompBase = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellInitData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkPompBase, FALSE );

    // 空気入れ取っ手
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.softpri = 3;
    cellInitData.bgpri = 1;
    cellInitData.anmseq = CGOAM_POMP_HANDLE;
    gameWork->clwkPompHandle = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellInitData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkPompHandle, FALSE );
    GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkPompHandle, TRUE );

    // ペンタッチアニメ
    cellInitData.pos_x = 65;
    cellInitData.pos_y = 122;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 1;
    cellInitData.anmseq = CGOAM_PEN;
    gameWork->clwkPenAnm = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellInitData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkPenAnm, FALSE );
  }
  

  // 風船用タスク作成
  // タスクシステムを作るのに必要なメモリサイズを計算
  gameWork->taskMem = GFL_HEAP_AllocMemory( gameWork->heapId, GFL_TCB_CalcSystemWorkSize(CTVT_MEMBER_NUM+4) );
  // タスクシステムを作成
  // 4つ(=メンバー分)しか必要ない
  // +4はスコア風船分
  gameWork->task = GFL_TCB_Init( CTVT_MEMBER_NUM+4, gameWork->taskMem );

  // 風船初期化
  for( i=0; i<connectNum; i++ )
  {
    gameWork->p_balloon2[i] = CTVT_BALLOON2_Create( gameWork, i, TRUE, gameWork->heapId );
    gameWork->p_balloon2[i]->tcb = GFL_TCB_AddTask( gameWork->task, TaskFunc2, gameWork->p_balloon2[i], 1 );
  }
  for( ; i<CTVT_MEMBER_NUM; i++ )
  {
    gameWork->p_balloon2[i] = CTVT_BALLOON2_Create( gameWork, i, FALSE, gameWork->heapId );
    gameWork->p_balloon2[i]->tcb = GFL_TCB_AddTask( gameWork->task, TaskFunc2, gameWork->p_balloon2[i], 1 );
  }

  // タイトルの文字描画
  {
    GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
    GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    STRBUF *str = GFL_MSG_CreateString( msgHandle, COMM_TVT_GAME_20 );
    const u8 nameLen = PRINTSYS_GetStrWidth( str, fontHandle, 0 );

    // ゲームルール
    PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp( gameWork->msgWinTitleFont ), 
        64 - (nameLen/2), 12, str, fontHandle,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    GFL_BMPWIN_TransVramCharacter( gameWork->msgWinTitleFont );
    GFL_BMPWIN_MakeScreen( gameWork->msgWinTitleFont );
    gameWork->isUpdateMsgTitleFont = TRUE;

    // 説明画面
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( gameWork->msgWinTitle ), 0xF );
    str = GFL_MSG_CreateString( msgHandle, COMM_TVT_GAME_16 );
    PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp( gameWork->msgWinTitle ), 
        0, 0, str, fontHandle,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    GFL_BMPWIN_TransVramCharacter( gameWork->msgWinTitle );
    GFL_BMPWIN_MakeScreen( gameWork->msgWinTitle );
    BmpWinFrame_Write( gameWork->msgWinTitle, WINDOW_TRANS_ON_V, 
        CTVT_GAME_BMPWIN_CGX_MAIN, CTVT_GAME_PAL_BG_M_WINFRAME );
    gameWork->isUpdateMsgTitle = TRUE;
  }
  

  // 初期化
  GFL_STD_MemFill( gameWork->touch_cnt, 0, sizeof(u8)*CTVT_MEMBER_NUM );
  GFL_STD_MemFill( gameWork->level_cnt, 0, sizeof(u8)*CTVT_MEMBER_NUM );
  gameWork->swing_interval = 0;

  // サウンド
  {
    NNSSndHeapHandle  snd_heap = PMSND_GetSndHeap();
    // 前の状態保存
    {
      int ret;
      ret = NNS_SndHeapSaveState( snd_heap );
      GF_ASSERT_MSG( (ret!=-1), "サウンドヒープ状態保存失敗\n");
    }
    // 新しいグループ読み込み
    {
      BOOL ret;
      ret = NNS_SndArcLoadGroup(GROUP_LCG02, snd_heap);
      GF_ASSERT(ret);
    }
    OS_Printf("サウンドグループ読み込み GROUPID %d\n", GROUP_LCG02);
  }

#ifdef PM_DEBUG
  gameWork->debug_touch_r = 0;
  gameWork->debug_touch_ex_r = 0;
#endif

  ICHI_Printf("ゲーム2初期化完了\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:タイトル
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 *	@return TRUE で終了
 */
//-----------------------------------------------------------------------------
static BOOL game_update_seq_title( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{

  // 説明表示待ち
  gameWork->title_cnt++;
  if( gameWork->title_cnt > (360) )
  {
    return TRUE;
  }

  // BGの表現がゲーム1とゲーム2で違う
  {
    CTVT_GAME_ID const game_id = CTVT_GAME_GetGameID( gameWork );

    switch( game_id ){
    case CGI_BALLOON1:
      {
        gameWork->bg_pal_anm++;
        // BGスクロール
        if( !(gameWork->bg_pal_anm % 5) )
        {
          gameWork->bg_pal_anm = 0;
          GFL_BG_SetScroll( CTVT_GAME_FRAME_S_BG, GFL_BG_SCROLL_Y_INC, 1 );
        }
      }
      break;
    case CGI_BALLOON2:
      {
        // BGパレットアニメ
        gameWork->bg_pal_anm++;
        gameWork->bg_pal_anm %= 20;

        // 10フレームに一回
        if( !(gameWork->bg_pal_anm % 10) )
        {
          u8  pal = gameWork->bg_pal_anm/10 + 1;   // パレット番号1、2のアニメ
          u16 *dat= (u16*)gameWork->p_plt_data->pRawData;
          GFL_BG_LoadPalette( CTVT_GAME_FRAME_S_BG, &dat[16*pal], 0x20, 0 );
        }
      }
      break;
    }
  }

  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス：タイトル終了
 *	        ゲーム1専用
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_title_end_part1( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8 i;
  for( i=0; i<3; i++ )
  {
    // タイトル 非表示
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitle[i], FALSE );
  }

  for( i=0; i<2; i++ )
  {
    // 雲表示
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkCloud_M[i], TRUE );
    GFL_CLACT_WK_ResetAnm( gameWork->clwkCloud_M[i] );
  }

  // BGロード
  // した画面
  GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_l1_NCLR , 
      PALTYPE_MAIN_BG, CTVT_GAME_PAL_BG_M_BG*32, 32*1, gameWork->heapId );
  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_l1_NCGR,
      CTVT_GAME_FRAME_M_BG, 0, 0, FALSE, gameWork->heapId );
  GFL_ARC_UTIL_TransVramScreen( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_l1_NSCR, 
      CTVT_GAME_FRAME_M_BG,  0, 0, FALSE, gameWork->heapId );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス：タイトル終了
 *	        ゲーム2専用
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_title_end_part2( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8 i;
  for( i=0; i<4; i++ )
  {
    // タイトル 非表示
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitle[i], FALSE );
  }
  // タッチボタン表示
  GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTouch, TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkTouch, TRUE );
  // 空気入れ表示
  GFL_CLACT_WK_SetDrawEnable( gameWork->clwkPompBase, TRUE );
  GFL_CLACT_WK_SetDrawEnable( gameWork->clwkPompHandle, TRUE );

  // BGロード
  // した画面
  GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_l2_NCLR , 
      PALTYPE_MAIN_BG, CTVT_GAME_PAL_BG_M_BG*32, 32*7, gameWork->heapId );
  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_l2_NCGR,
      CTVT_GAME_FRAME_M_BG, 0, 0, FALSE, gameWork->heapId );
  GFL_ARC_UTIL_TransVramScreen( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_l2_NSCR, 
      CTVT_GAME_FRAME_M_BG,  0, 0, FALSE, gameWork->heapId );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:メイン
 *	        ゲーム1専用
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 *	@return TRUE で終了
 */
//-----------------------------------------------------------------------------
static BOOL game_update_seq_main_part1( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

#ifdef PM_DEBUG
  // Rボタン押しながらでデバッグ
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R)
  {
    // 当たり判定
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_UP)
    {
      gameWork->debug_r += 0x5;    // 半径特定用
      OS_Printf("debug_r %x \n", gameWork->debug_r );
    }
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_DOWN)
    {
      gameWork->debug_r -= 0x5;    // 半径特定用
      OS_Printf("debug_r %x \n", gameWork->debug_r );
    }

    // サイズ
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_RIGHT)
    {
      gameWork->debug_size += 0x50;
      OS_Printf("debug_size %x \n", gameWork->debug_size);
    }
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_LEFT)
    {
      gameWork->debug_size -= 0x50;
      OS_Printf("debug_size %x \n", gameWork->debug_size);
    }

    // 速さ
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_A)
    {
      gameWork->debug_v += 0x400;
      OS_Printf("debug_v %x \n", gameWork->debug_v);
    }
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_B)
    {
      gameWork->debug_v -= 0x400;
      OS_Printf("debug_v %x \n", gameWork->debug_v);
    }

    // 風船と風船の間隔
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_X)
    {
      gameWork->debug_wait += 5;
      OS_Printf("debug_wait %d \n", gameWork->debug_wait);
    }
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_Y)
    {
      gameWork->debug_wait -= 5;
      OS_Printf("debug_wait %d \n", gameWork->debug_wait);
    }
  }

  // 制限時間早める処理
  // デバッグ 制限時間早める
  if( selfId == 0 && GFL_UI_KEY_GetCont()&PAD_BUTTON_L)
  {
    gameWork->oya_frame += 5;
  }
  if( GFL_UI_KEY_GetTrg()&PAD_BUTTON_A )
  {
    gameWork->plate_move = TRUE;
    GAME_PLATE_SetMovePlate( gameWork, FALSE );
  }

  // ↑までデバッグ
#endif


    // 風船出現管理(親だけ)
  if( selfId == 0 )
  {
    // 待ち時間が終わると出現フラグON
    if( gameWork->balloon_wait <= 0 )
    {
      gameWork->balloon_flg = 1;
    }

    // 出現フラグがON
    if( gameWork->balloon_flg == 1 )
    {
      // 風船出現させる
      {
        // 通信データ送信
        CTVT_GAME_SEND_DATA data = {0,};
        CTVT_COMM_SendGameReq( commWork, data, CCGDT_BALOON_APPEAR );
      }
      
      gameWork->balloon_flg = 0;
#ifdef PM_DEBUG
      gameWork->balloon_wait = gameWork->debug_wait;
#else
      gameWork->balloon_wait = 15;
#endif
    }
    else
    {
      gameWork->balloon_wait--;
    }
  }

  // タッチペンの座標を送信する
  {
    u32 tpx,tpy;
    const BOOL isTrg = GFL_UI_TP_GetPointTrg( &tpx,&tpy );

    if( isTrg == TRUE )
    {
      // 通信データ送信
      CTVT_GAME_SEND_DATA data;
      data.tpx = tpx;
      data.tpy = tpy;
      CTVT_COMM_SendGameReq( commWork, data, CCGDT_TP );
      //ICHI_Printf("touch!\n");
    }
  }

  // 受信していない場合
  // TRUEで受信していると、frameの値は更新されている
  if( gameWork->isRecvFrame == FALSE )
  {
    gameWork->frame++;
  }
  gameWork->isRecvFrame = FALSE;    // 受信していないに戻す

  if( selfId == 0 )
  {
    gameWork->oya_frame++;
  }
  //ICHI_Printf("フレーム %d\n", gameWork->frame);

  // 制限時間処理
  if( gameWork->timeLimit > 0 )
  {
    if( ( gameWork->frame - gameWork->befFrameCnt ) > 60 )
    {
      gameWork->befFrameCnt = gameWork->frame;
      gameWork->timeLimit--;

      // タイムゲージ描画
      draw_timebar( gameWork );
    }

    if( gameWork->timeLimit == 0 )
    {
      // 終了のホイッスル音
      PMSND_PlaySystemSE( SEQ_SE_LCG_04 );    // ホイッスル音
    }
  }
  else if( gameWork->timeLimit == 0 )
  {
    ICHI_Printf("TimeLimit が0 \n");

    // 終了時に親ならスコア送信
    if( selfId == 0 )
    {
      CTVT_GAME_SEND_SCORE score;
      u8  i;

      ICHI_Printf("-----送信スコア-------------\n");
      for(i=0; i<CTVT_MEMBER_NUM; i++ )
      {
        score.score[i] = gameWork->score[i];
        ICHI_Printf("%d -> %d\n",i, score.score[i]);
      }

      CTVT_COMM_SendGameReq_Score( commWork, score );
    }

    ICHI_Printf("ゲームメイン終了!\n");
    return TRUE; // 終了
  }

  // プレート移動
  if( ( gameWork->timeLimit == 40 )
      && gameWork->plate_move == FALSE )
  {
    gameWork->plate_move = TRUE;
    GAME_PLATE_SetMovePlate( gameWork, FALSE );
  }
  // ラストスパート
  if( gameWork->timeLimit == 20 && gameWork->plate_move == FALSE )
  {
    gameWork->plate_move = TRUE;
    GAME_PLATE_SetMovePlate( gameWork, TRUE );
  }
  if( gameWork->timeLimit <20 && gameWork->plate_move == FALSE )
  {
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkLast, TRUE );
  }

  if( gameWork->timeLimit == 20 )
  {
    // 元々のテンポをratio/256倍する
    PMSND_SetStatusBGM( 285, 85, PMSND_NOEFFECT );
    //PMSND_SetStatusBGM( 285, PMSND_NOEFFECT, PMSND_NOEFFECT );
  }
  
  // プレート更新
  GAME_PLATE_UpdatePlate( gameWork );

  // 風船の更新 & 描画
  GRAPHIC_Draw( gameWork );

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:メイン
 *	        ゲーム2専用
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 *	@return TRUE で終了
 */
//-----------------------------------------------------------------------------
static BOOL game_update_seq_main_part2( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  u16   anmseq_stay;
  u16   anmseq_touch;
  u8 i;

  // 追加仕様で、ゲーム始まる前にペンでタッチのアニメを再生する
  // アニメが終了するまでゲームを開始しない
  if( GFL_CLACT_WK_CheckAnmActive(gameWork->clwkPenAnm) == TRUE )
  {
    // プレートだけは更新
    GAME_PLATE_UpdatePlate( gameWork );
    return FALSE;
  }
  else if( GFL_CLACT_WK_GetDrawEnable( gameWork->clwkPenAnm ) == TRUE )
  {
    PMSND_PlaySystemSE( SEQ_SE_LCG_03 );    // スタートのホイッスル音
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkPenAnm, FALSE );  // 非表示
  }

  // 残り時間でタッチボタンを変更
  if( gameWork->timeLimit <= 20 )
  {
    anmseq_stay   = CGOAM_BUTTON_LAST_STAY;
    anmseq_touch  = CGOAM_BUTTON_LAST_TOUCH;

    // 変わってない時は変更する
    if( GFL_CLACT_WK_GetAnmSeq( gameWork->clwkTouch ) == CGOAM_BUTTON_STAY )
    {
      GFL_CLACT_WK_SetAnmSeq( gameWork->clwkTouch, CGOAM_BUTTON_LAST_STAY );
    }
    if( GFL_CLACT_WK_GetAnmSeq( gameWork->clwkTouch ) == CGOAM_BUTTON_TOUCH )
    {
      GFL_CLACT_WK_SetAnmSeq( gameWork->clwkTouch, CGOAM_BUTTON_LAST_TOUCH );
    }
  }
  else
  {
    anmseq_stay   = CGOAM_BUTTON_STAY;
    anmseq_touch  = CGOAM_BUTTON_TOUCH;
  }

  // 風船がアクティブの時のみタッチ判定
  if( CTVT_BALLOON2_GetActive( gameWork->p_balloon2[selfId] ) == TRUE )
  {
    static GFL_UI_TP_HITTBL button_tbl[5] =
    {
      // 左
      {168, 172, 16, 20 }, // ピンポイント範囲
      {142, 192, 0, 47 }, // タッチ範囲

      // 右
      {168, 172, 232, 236 }, // ピンポイント範囲
      {142, 192, 208, 255 }, // タッチ範囲
      {GFL_UI_TP_HIT_END,0,0,0}
    };
    int retTrg;

#ifdef PM_DEBUG
    button_tbl[0].rect.top -= gameWork->debug_touch_ex_r;
    button_tbl[0].rect.bottom += gameWork->debug_touch_ex_r;
    button_tbl[0].rect.left -= gameWork->debug_touch_ex_r;
    button_tbl[0].rect.right += gameWork->debug_touch_ex_r;
    button_tbl[1].rect.top -= gameWork->debug_touch_r;
    button_tbl[1].rect.right += gameWork->debug_touch_r;
    button_tbl[1].rect.left -= gameWork->debug_touch_r;
    button_tbl[1].rect.right += gameWork->debug_touch_r;
#endif

    retTrg = GFL_UI_TP_HitTrg( button_tbl );
    if( retTrg != GFL_UI_TP_HIT_NONE )
    {
      CTVT_GAME_SEND_DATA data = {0};
      // 
      if( retTrg == 0 || retTrg == 2 )
      {
        // ピンポイントタッチ
        // 通信データ送信
        CTVT_COMM_SendGameReq( commWork, data, CCGDT_TOUCH_EX );
        // SE再生
        PMSND_PlaySystemSE( SEQ_SE_LCG_07 );
      }
      else
      if( retTrg == 1 || retTrg == 3 )
      {
        // 通信データ送信
        CTVT_COMM_SendGameReq( commWork, data, CCGDT_TOUCH );
        // SE再生
        PMSND_PlaySystemSE( SEQ_SE_LCG_06 );
      }

      // 風船を揺らす
      // 送信しない時も追加条件に加えたほうがいい？
      if( gameWork->swing_interval == 0 )
      {
        gameWork->swing_interval = SWING_INTERVAL;

        CTVT_BALLOON2_SetSwing( gameWork->p_balloon2[selfId] );
      }

      // 空気入れのアニメーション再生
      if( GFL_CLACT_WK_CheckAnmActive(gameWork->clwkPompHandle) == FALSE )
      {
        GFL_CLACT_WK_ResetAnm( gameWork->clwkPompHandle );

        // SE再生 再生待ちする
        {
          SEPLAYER_ID seplayer = PMSND_GetSE_DefaultPlayerID( SEQ_SE_LCG_06 );
          if( PMSND_CheckPlaySE_byPlayerID( seplayer) == FALSE )
          {
            PMSND_PlaySystemSE( SEQ_SE_LCG_06 );
          }
        }
      }

      // タッチボタンのアニメーションを
      // タッチ中のアニメシーケンスに変更
      if( GFL_CLACT_WK_GetAnmSeq( gameWork->clwkTouch ) != anmseq_touch )
      {
        GFL_CLACT_WK_SetAnmSeq( gameWork->clwkTouch, anmseq_touch );
      }
      if( GFL_CLACT_WK_CheckAnmActive(gameWork->clwkTouch) == FALSE )
      {
        GFL_CLACT_WK_ResetAnm( gameWork->clwkTouch );
      }
    }
  }

  // タッチしていない時にタッチボタンを戻す
  if( GFL_CLACT_WK_GetAnmSeq( gameWork->clwkTouch ) != anmseq_stay )
  {
    if( GFL_CLACT_WK_CheckAnmActive(gameWork->clwkTouch) == FALSE )
    {
      GFL_CLACT_WK_SetAnmSeq( gameWork->clwkTouch, anmseq_stay );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkTouch, TRUE );
    }
  }
 
  // インターバル
  if( gameWork->swing_interval > 0 )
  {
    gameWork->swing_interval--;
  }


  // 受信していない場合
  // TRUEで受信していると、frameの値は更新されている
  if( gameWork->isRecvFrame == FALSE )
  {
    gameWork->frame++;
  }
  gameWork->isRecvFrame = FALSE;    // 受信していないに戻す

  if( selfId == 0 )
  {
    gameWork->oya_frame++;
  }

  // 制限時間処理
  if( gameWork->timeLimit > 0 )
  {
    if( ( gameWork->frame - gameWork->befFrameCnt ) > 60 )
    {
      gameWork->befFrameCnt = gameWork->frame;
      gameWork->timeLimit--;

      // タイムゲージ描画
      draw_timebar( gameWork );
    }

    if( gameWork->timeLimit == 0 )
    {
      // 終了のホイッスル音
      PMSND_PlaySystemSE( SEQ_SE_LCG_04 );    // ホイッスル音

      // 風船非表示
      for( i=0; i<CTVT_MEMBER_NUM; i++ )
      {
        gameWork->p_balloon2[i]->visible = FALSE;
      }
    }
  }
  else if( gameWork->timeLimit == 0 )
  {
    ICHI_Printf("TimeLimit が0 \n");
    gameWork->frame++;
    // 終了時に親ならスコア送信
    if( selfId == 0 )
    {
      CTVT_GAME_SEND_SCORE score;
      u8  i;

      ICHI_Printf("-----送信スコア-------------\n");
      for(i=0; i<CTVT_MEMBER_NUM; i++ )
      {
        score.score[i] = gameWork->score[i];
        ICHI_Printf("%d -> %d\n",i, score.score[i]);
      }

      CTVT_COMM_SendGameReq_Score( commWork, score );
    }

    ICHI_Printf("ゲームメイン終了!\n");
    return TRUE; // 終了
  }

  // プレート移動
  if( ( gameWork->timeLimit == 40 )
      && gameWork->plate_move == FALSE )
  {
    gameWork->plate_move = TRUE;
    GAME_PLATE_SetMovePlate( gameWork, FALSE );
  }
  // ラストスパート
  if( gameWork->timeLimit == 20 && gameWork->plate_move == FALSE )
  {
    gameWork->plate_move = TRUE;
    GAME_PLATE_SetMovePlate( gameWork, TRUE );
  }
  if( gameWork->timeLimit <20 && gameWork->plate_move == FALSE )
  {
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkLast, TRUE );
  }

  if( gameWork->timeLimit == 20 )
  {
    // 元々のテンポをratio/256倍する
    PMSND_SetStatusBGM( 285, 85, PMSND_NOEFFECT );
    //PMSND_SetStatusBGM( 285, PMSND_NOEFFECT, PMSND_NOEFFECT );
    // 取っ手変更
    GFL_CLACT_WK_SetAnmSeq( gameWork->clwkPompHandle, CGOAM_POMP_HANDLE_LAST );
    GFL_CLACT_WK_StopAnm( gameWork->clwkPompHandle );
  }
  
  // プレート更新
  GAME_PLATE_UpdatePlate( gameWork );

  // 風船の更新 & 描画
  GRAPHIC_Draw2( gameWork );


#ifdef PM_DEBUG
  // Rボタン押しながらでデバッグ
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R)
  {
    // タッチ範囲
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_UP)
    {
      gameWork->debug_touch_r += 1;    // 半径特定用
      OS_Printf("debug_touch_r %d \n", gameWork->debug_touch_r );
    }
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_DOWN)
    {
      gameWork->debug_touch_r -= 1;    // 半径特定用
      OS_Printf("debug_touch_r %d \n", gameWork->debug_touch_r );
    }

    // ピンポイント範囲
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_RIGHT)
    {
      gameWork->debug_touch_ex_r += 1;    // 半径特定用
      OS_Printf("debug_touch_ex_r %d \n", gameWork->debug_touch_ex_r );
    }
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_LEFT)
    {
      gameWork->debug_touch_ex_r -= 1;    // 半径特定用
      OS_Printf("debug_touch_ex_r %d \n", gameWork->debug_touch_ex_r );
    }
  }
  // 制限時間早める処理
  // デバッグ 制限時間早める
  if( selfId == 0 && GFL_UI_KEY_GetCont()&PAD_BUTTON_L)
  {
    gameWork->oya_frame += 10;
  }
  // ↑までデバッグ
#endif

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:スコア初期化
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_score_init( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8 i;
  HEAPID heapID = gameWork->heapId;
  CTVT_GAME_ID game_id = CTVT_GAME_GetGameID( gameWork );

  // サウンド
  {
    NNSSndHeapHandle  snd_heap = PMSND_GetSndHeap();
    // グループアンロード
    // 状態を戻すと自動で解放するので呼ばなくて良い
    //NNS_SndHeapDestroy(snd_heap);
    
    // 保存しておいた状態を戻す
    {
      int level = NNS_SndHeapGetCurrentLevel( snd_heap );
      NNS_SndHeapLoadState( snd_heap, level );
    }
    OS_Printf("サウンドグループ 保存状態に戻す\n");
  }

  // ゲーム中のOBJは結果発表では使わないので破棄する
  {
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTime, FALSE );  // タイムゲージ
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkLast, FALSE );  // ラストスパート
    // タイムバーのBMPWIN破棄
    GFL_BMPWIN_ClearTransWindow( gameWork->bmpWinTime );

    // ゲーム1だけ破棄するもの
    if( game_id == CGI_BALLOON1 )
    {
      for( i=0; i<2; i++ )
      {
        GFL_CLACT_WK_SetDrawEnable( gameWork->clwkCloud_M[i], FALSE );
      }
    }
    // ゲーム2だけ破棄するもの
    else if( game_id == CGI_BALLOON2 )
    {
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTouch, FALSE );   // タッチするボタン
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkPompBase, FALSE );   // 空気入れ
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkPompHandle, FALSE );   // 空気入れ
    }
  }

  // プレートをスコアのものに変更
  {
    // プレート変更
    for( i=0;i<CTVT_MEMBER_NUM; i++ )
    {
      GAME_PLATE_ChangePlate( work, gameWork->p_plate[i] );
      // 動作関数変更
      GFL_TCB_ChangeFunc( gameWork->p_plate[i]->tcb, TaskFuncScorePlate );
      if( gameWork->p_plate[i]->is_blank == FALSE )
      {
        // ドラムロールON
        gameWork->p_plate[i]->sub_seq = PLATE_SEQ_DRUMROLL;
      }
    }
  }

  // BGロード
  // 下画面
  // ゲームIDによって読み込むパレットを変えている
  {
    u8 game_id = CTVT_GAME_GetGameID( gameWork );
    ARCDATID arc_id = NARC_tvt_balloon_tv_score_bg1_NCLR + game_id;
    GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, arc_id, 
        PALTYPE_MAIN_BG, CTVT_GAME_PAL_BG_M_BG*32, 32*4, gameWork->heapId );
    GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_score_bg_NCGR,
        CTVT_GAME_FRAME_M_BG, 0, 0, FALSE, gameWork->heapId );
    GFL_ARC_UTIL_TransVramScreen( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_score_bg_NSCR, 
        CTVT_GAME_FRAME_M_BG,  0, 0, FALSE, gameWork->heapId );
    GFL_BG_SetVisible( CTVT_GAME_FRAME_S_TIMEBAR, VISIBLE_OFF );
  }

  // 結果発表で使用するOBJ
  // ※ミニゲーム1とミニゲーム2でアニメシーケンスを変えている
  {
    GFL_CLWK_DATA cellInitData;
    // けっかはっぴょう
    cellInitData.pos_x  = 128;
    cellInitData.pos_y  = 96;
    cellInitData.softpri= 0;
    cellInitData.bgpri  = 1;
    cellInitData.anmseq = CGOAS_KEKKA;
    gameWork->clwkKekka = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_UPPER_PLT],
        gameWork->cellResIdx[CGOR_UPPER_NCG],
        gameWork->cellResIdx[CGOR_UPPER_ANM],
        &cellInitData ,CLSYS_DEFREND_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkKekka, TRUE );

    // 隠しプレート
    cellInitData.softpri= 0;
    cellInitData.bgpri  = 3;
    for( i=0; i<2; i++ )
    {
      if( game_id == CGI_BALLOON1 )
      {
        cellInitData.anmseq = CGOAM_OME_PLATE_L+i;
      }
      else if( game_id == CGI_BALLOON2 )
      {
        cellInitData.anmseq = CGOAM_OME_PLATE_2;
      }
      gameWork->clwkOmeHide[i] = GFL_CLACT_WK_Create( gameWork->cellUnit,
          gameWork->cellResIdx[CGOR_LOWER_PLT],
          gameWork->cellResIdx[CGOR_LOWER_NCG],
          gameWork->cellResIdx[CGOR_LOWER_ANM],
          &cellInitData ,CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeHide[i], FALSE );
    }
    // 虹
    cellInitData.softpri= 1;
    cellInitData.bgpri  = 3;
    cellInitData.anmseq = CGOAM_RAINBOW1 + game_id;
    gameWork->clwkOmeNiji = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellInitData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeNiji, FALSE );
    // 星
    cellInitData.softpri= 8;
    cellInitData.bgpri  = 1;
    cellInitData.anmseq = CGOAM_STAR1 + game_id;
    gameWork->clwkOmeStar = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellInitData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeStar, FALSE );

    // 順位
    cellInitData.softpri= 1;
    cellInitData.bgpri  = 1;
    cellInitData.anmseq = CGOAM_RANK1;
    gameWork->clwkRank = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellInitData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkRank, FALSE );

    // 三点リーダ
    cellInitData.softpri= 1;
    cellInitData.bgpri  = 1;
    cellInitData.anmseq = CGOAM_DOT;
    gameWork->clwkDot = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellInitData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkDot, FALSE );
  }
  // 倍角を使っているのでaffineで
  {
    GFL_CLWK_AFFINEDATA cellAffineData;
    GFL_STD_MemClear( &cellAffineData, sizeof(GFL_CLWK_AFFINEDATA) );

    // おめでとう
    cellAffineData.clwkdata.pos_x  = 128;
    cellAffineData.clwkdata.pos_y  = 96;
    cellAffineData.clwkdata.anmseq = CGOAM_OME1;
    cellAffineData.clwkdata.softpri= 1;
    cellAffineData.clwkdata.bgpri  = 1;

    cellAffineData.affinepos_x  = 0;
    cellAffineData.affinepos_y  = 0;
    cellAffineData.scale_x      = FX32_ONE;
    cellAffineData.scale_y      = FX32_ONE;
    cellAffineData.rotation     = 0;
    cellAffineData.affine_type  = CLSYS_AFFINETYPE_DOUBLE;

    gameWork->clwkOmeFont = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeFont, FALSE );

    // お
    cellAffineData.clwkdata.pos_x  = 59;
    cellAffineData.clwkdata.pos_y  = 80;
    cellAffineData.clwkdata.anmseq = CGOAM_TEXT_O;
    cellAffineData.clwkdata.softpri= 2;
    cellAffineData.clwkdata.bgpri  = 1;
    gameWork->clwkOmeText[0] = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeText[0], FALSE );

    // め
    cellAffineData.clwkdata.pos_x  = 90;
    cellAffineData.clwkdata.pos_y  = 80;
    cellAffineData.clwkdata.anmseq = CGOAM_TEXT_ME;
    cellAffineData.clwkdata.softpri= 3;
    cellAffineData.clwkdata.bgpri  = 1;
    gameWork->clwkOmeText[1] = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeText[1], FALSE );

    // で
    cellAffineData.clwkdata.pos_x  = 120;
    cellAffineData.clwkdata.pos_y  = 80;
    cellAffineData.clwkdata.anmseq = CGOAM_TEXT_DE;
    cellAffineData.clwkdata.softpri= 4;
    cellAffineData.clwkdata.bgpri  = 1;
    gameWork->clwkOmeText[2] = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeText[2], FALSE );

    // と
    cellAffineData.clwkdata.pos_x  = 150;
    cellAffineData.clwkdata.pos_y  = 80;
    cellAffineData.clwkdata.anmseq = CGOAM_TEXT_TO;
    cellAffineData.clwkdata.softpri= 5;
    cellAffineData.clwkdata.bgpri  = 1;
    gameWork->clwkOmeText[3] = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeText[3], FALSE );

    // う
    cellAffineData.clwkdata.pos_x  = 177;
    cellAffineData.clwkdata.pos_y  = 80;
    cellAffineData.clwkdata.anmseq = CGOAM_TEXT_U;
    cellAffineData.clwkdata.softpri= 6;
    cellAffineData.clwkdata.bgpri  = 1;
    gameWork->clwkOmeText[4] = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeText[4], FALSE );

    // ！
    cellAffineData.clwkdata.pos_x  = 200;
    cellAffineData.clwkdata.pos_y  = 80;
    cellAffineData.clwkdata.anmseq = CGOAM_TEXT_EX;
    cellAffineData.clwkdata.softpri= 6;
    cellAffineData.clwkdata.bgpri  = 1;
    gameWork->clwkOmeText[5] = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeText[5], FALSE );

    // 雲
    cellAffineData.clwkdata.pos_x  = 128;
    cellAffineData.clwkdata.pos_y  = 96;
    cellAffineData.clwkdata.softpri= 3;
    for( i=0; i<2; i++ )
    {
      if( game_id == CGI_BALLOON1 )
      {
        cellAffineData.clwkdata.anmseq = CGOAM_OME_CLOUD_L+i;
      }
      else if( game_id == CGI_BALLOON2 )
      {
        cellAffineData.clwkdata.anmseq = CGOAM_STAR1_2+i;
      }
      gameWork->clwkOmeCloud[i] = GFL_CLACT_WK_CreateAffine( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_LOWER_PLT],
        gameWork->cellResIdx[CGOR_LOWER_NCG],
        gameWork->cellResIdx[CGOR_LOWER_ANM],
        &cellAffineData ,CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeCloud[i], FALSE );
    }
  }

  // 風船作成
  {
    static fx32 tbl[] = { // 30ずつずらしてみた
      FX32_CONST(60),   // 1位
      FX32_CONST(30),
      FX32_CONST(0),
      FX32_CONST(-30),  // 4位
    };

    // 順位づけ
    // 得点が同じだと順位を同じにする
    {
      u8 j;
      u8 rank[PLATE_NUM] = {1,1,1,1};
      for( i=1; i<PLATE_NUM; i++ )
      {
        for( j=0; j<i; j++ )
        {
          if( gameWork->p_plate[j]->point < gameWork->p_plate[i]->point)
          {
            rank[j]++;
          }
          if( gameWork->p_plate[j]->point > gameWork->p_plate[i]->point)
          {
            rank[i]++;
          }
        }
      }
      for( i=0; i<PLATE_NUM; i++ )
      {
        gameWork->p_plate[i]->rank = rank[i];
        ICHI_Printf("%d point %d rank %d \n",i,gameWork->p_plate[i]->point,rank[i]);
      }
    }

    // ワークを作成して、タスクに登録
    for( i=0;i<CTVT_MEMBER_NUM; i++ )
    {
      ICHI_Printf("p_wk create %x\n", gameWork->p_balloon_s_wk[i]);
      if( gameWork->p_plate[i]->is_blank == FALSE )
      {
        u8 rank = gameWork->p_plate[i]->rank;
        u8 netID= gameWork->p_plate[i]->netID;
        ICHI_Printf("balloon_s作成 %d\n", i);
        gameWork->p_balloon_s_wk[i] = CTVT_BALLOON_S_Create( gameWork, netID, rank, tbl[rank-1], gameWork->heapId );
        gameWork->p_balloon_s_wk[i]->tcb = GFL_TCB_AddTask( gameWork->task, TaskFuncScore, gameWork->p_balloon_s_wk[i], 2 );
      }
    }
  }

  // 結果発表用に3Dリソース読み込み、今までのは破棄
  {
    const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

    // ユニット破棄
    for( i=0; i<CTVT_UNIT_IDX_MAX; i++ )
    {
      GFL_G3D_UTIL_DelUnit( gameWork->g3dUtil, gameWork->unit_idx[i] );
    }

    // ユニット作成
    RESOURCE3D_Init_Score( gameWork, heapID );
    

    // テクスチャ貼り付け
    {
      BOOL  rc;
      u32   dst;    // 転送先の実アドレス
      u16*  src;    // 転送元データ
      u32   size;   // 転送サイズ
      u8    type;   // 貼り付ける写真 
      u8    mem_cnt=0;  // メンバーIDX

      ICHI_Printf("テクスチャ貼り付け\n");

      for(i=0; i<CTVT_MEMBER_NUM; i++ )
      {
        // 2種類のテクスチャをはる
        for( type=0; type<2; type++ )
        {

          u16 unit_res_cnt = GFL_G3D_UTIL_GetUnitResCount( gameWork->g3dUtil, gameWork->unit_idx[i] );
          GFL_G3D_RES* res = GFL_G3D_UTIL_GetResHandle( gameWork->g3dUtil, unit_res_cnt*i + type );
          NNSG3dTexKey texkey = GFL_G3D_GetTextureDataVramKey( res );
          u32 tex_adr = NNS_GfdGetTexKeyAddr(texkey);
          u32 tex_size = NNS_GfdGetTexKeySize(texkey);

          // 転送
          {
            u16 tex_offset = (128*2);
            dst = tex_adr + tex_offset; // tex_offset分足しているのは、写真のテクスチャにする為
            // 貼り付ける画像選択
            if( selfId == i )
            {
              // TWLはキャプチャ画像、NITOROはキャラ画像
              src = (u16*)gameWork->p_cam_param->p_cap[type];
            }
            else
            {
              // 他のメンバーの場合
              // 
              // TWLなら受信した画像、NITOROならキャラ画像
              src = (u16*)gameWork->cam_member[mem_cnt].p_cap_buf[type];
            }
            size = CAPTURE_SIZE;
            rc = NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, size);
            GF_ASSERT(rc);
            ICHI_Printf("dst %x src %x size %x texsize %x \n", dst, src, size, tex_size);
          }
        }

        // メンバーカウンタ
        if( selfId != i )
        {
          mem_cnt++;
        }
      }
    }
  }

  gameWork->sub_seq = 0;
  gameWork->score_cnt = 0;

#ifdef PM_DEBUG
  ICHI_Printf("風船個数 プレイヤー0 %d \n", gameWork->debug_b_cnt[ 0 ]);
  ICHI_Printf("風船個数 プレイヤー1 %d \n", gameWork->debug_b_cnt[ 1 ]);
  ICHI_Printf("風船個数 プレイヤー2 %d \n", gameWork->debug_b_cnt[ 2 ]);
  ICHI_Printf("風船個数 プレイヤー3 %d \n", gameWork->debug_b_cnt[ 3 ]);
#endif

}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:スコア
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL game_update_seq_score( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  const u8 connectNum = COMM_TVT_GetConnectNum( work );

  enum{
    SUB_SEQ_DRUMROLL,     // ドラムロール中
    SUB_SEQ_DRUMROLL_STOP,// ドラムロールストップ
    SUB_SEQ_HIDE_ANIME,   // 隠すプレートアニメ
    SUB_SEQ_HIDE_ANIME_WAIT1, // 隠すプレートアニメ待ち 1位用
    SUB_SEQ_HIDE_ANIME_WAIT2, // 隠すプレートアニメ待ち 2位用
    SUB_SEQ_HIDE_ANIME_WAIT3, // 隠すプレートアニメ待ち 3，4位用
    SUB_SEQ_OME_ANIME_WAIT, // おめでとうアニメ待ち
    SUB_SEQ_END_WAIT,     // 終了待ち
    SUB_SEQ_END,          // 終了
  };

  switch( gameWork->sub_seq ){
  case SUB_SEQ_DRUMROLL:     // ドラムロール中
    for( i=0; i<CTVT_MEMBER_NUM; i++ )
    {
      if( gameWork->p_plate[i]->is_blank == FALSE )
      {
        if( gameWork->p_balloon_s_wk[i]->visible == FALSE )
        {
          // 得点ロールをOFF
          if( gameWork->p_plate[i]->sub_seq == PLATE_SEQ_DRUMROLL )
          {
            gameWork->p_plate[i]->sub_seq = PLATE_SEQ_DRUMROLL_STOP;
            ICHI_Printf("ドラムロールOFF\n");
          }
        }
      }
    }
    //ICHI_Printf("score_cnt %d \n", gameWork->score_cnt);
    gameWork->score_cnt++;
    if( gameWork->score_cnt > 462 ) // 462 = 2位が破裂するまでのフレーム数
    {
      gameWork->sub_seq = SUB_SEQ_DRUMROLL_STOP;
    }
    break;
  case SUB_SEQ_DRUMROLL_STOP:// ドラムロールストップ
    for( i=0; i<PLATE_NUM; i++ )
    {
      gameWork->p_plate[i]->sub_seq = PLATE_SEQ_DRUMROLL_STOP;
      ICHI_Printf("全得点表示\n");
    }

    // ME再生 ファンファーレ
    PMSND_PlayBGM( SEQ_ME_LCG_02 );
    gameWork->sub_seq = SUB_SEQ_HIDE_ANIME;
    break;
  case SUB_SEQ_HIDE_ANIME:   // 隠すプレートアニメ

    
    // 順位によって演出が違う
    if( gameWork->p_plate[selfId]->rank == 1 )
    {
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeNiji, TRUE );

      // その他１位の時に表示しておくもの
      // 雲・星
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeCloud[0], TRUE );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeCloud[1], TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkOmeCloud[0], TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkOmeCloud[1], TRUE );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkOmeCloud[0] );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkOmeCloud[1] );

      // 隠しプレート
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeHide[0], TRUE );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeHide[1], TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkOmeHide[0], TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkOmeHide[1], TRUE );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkOmeHide[0] );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkOmeHide[1] );

      // 周りの星
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeStar, TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkOmeStar, TRUE );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkOmeStar );


      gameWork->sub_seq = SUB_SEQ_HIDE_ANIME_WAIT1;
    }
    else if( gameWork->p_plate[selfId]->rank == 2 ) 
    {
      gameWork->sub_seq = SUB_SEQ_HIDE_ANIME_WAIT2;
    }
    else
    {
      gameWork->sub_seq = SUB_SEQ_HIDE_ANIME_WAIT3;
    }

    // 順位に合ったアニメシーケンスに変更
    GFL_CLACT_WK_SetAnmSeq( gameWork->clwkRank, ( CGOAM_RANK1 + (gameWork->p_plate[selfId]->rank-1) ) );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkRank, FALSE );

    ICHI_Printf("隠しプレートアニメ開始\n");
    break;
  case SUB_SEQ_HIDE_ANIME_WAIT1: // 隠すプレートアニメ待ち
    if( GFL_CLACT_WK_CheckAnmActive( gameWork->clwkOmeHide[0] ) == FALSE )
    {
      // プレートは消す
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeHide[0], FALSE );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeHide[1], FALSE );

      // おめでとうのアニメ
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeFont, TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkOmeFont, TRUE );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkOmeFont );

      ICHI_Printf("おめでとうアニメ開始\n");
      gameWork->sub_seq = SUB_SEQ_OME_ANIME_WAIT;
    }
    break;

  case SUB_SEQ_HIDE_ANIME_WAIT2: // 隠すプレートアニメ待ち 2位用
    {
      // 「おしい2nd」
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkRank, TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkRank, TRUE );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkRank );

      gameWork->score_cnt = 0;    // 次フレーム時用に初期化
      gameWork->sub_seq = SUB_SEQ_END_WAIT;
    }
    break;

  case SUB_SEQ_HIDE_ANIME_WAIT3: // 隠すプレートアニメ待ち 3，4位用
    {
      // 「ざんねん xxx ...」
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkRank, TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkRank, TRUE );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkRank );
      // 三点リーダー
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkDot, TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkDot, TRUE );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkDot );

      gameWork->score_cnt = 0;    // 次フレーム時用に初期化
      gameWork->sub_seq = SUB_SEQ_END_WAIT;
    }
    break;

  case SUB_SEQ_OME_ANIME_WAIT: // おめでとうアニメ待ち
    if( GFL_CLACT_WK_CheckAnmActive( gameWork->clwkOmeFont ) == FALSE )
    {
      // その他表示
      
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeFont, FALSE );

      // おめでとう 単文字表示
      for( i=0; i<6; i++ )
      {
        GFL_CLACT_WK_SetDrawEnable( gameWork->clwkOmeText[i], TRUE );
        GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkOmeText[i], TRUE );
        GFL_CLACT_WK_ResetAnm( gameWork->clwkOmeText[i] );
      }
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkRank, TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkRank, TRUE );
      GFL_CLACT_WK_ResetAnm( gameWork->clwkRank );

      gameWork->score_cnt = 0;    // 次フレーム時用に初期化
      gameWork->sub_seq = SUB_SEQ_END_WAIT;
    }
    break;
  case SUB_SEQ_END_WAIT:     // 終了待ち
    gameWork->score_cnt++;
    if( gameWork->score_cnt > (CTVT_GAME_TIME_RETRY*60) )
    {
      gameWork->sub_seq = SUB_SEQ_END;
    }
    break;
  case SUB_SEQ_END:          // 終了
    return TRUE;
  }

  // GRAPHIC
  {
    // 3D描画
    GFL_G3D_DRAW_Start();

    // カメラ更新
    GFL_G3D_CAMERA_Switching( gameWork->p_camera );
    GFL_G3D_DRAW_SetLookAt();

    // アニメーション更新
    for( i=0; i<CTVT_MEMBER_NUM; i++ )
    {
      u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( gameWork->g3dUtil, gameWork->unit_idx[i] );
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( gameWork->g3dUtil, head_obj_idx );
      GFL_G3D_OBJECT_LoopAnimeFrame( obj, 0, FX32_ONE );  // ica
      GFL_G3D_OBJECT_LoopAnimeFrame( obj, 1, FX32_ONE );  // itp
    }

    // 風船タスクメイン
    GFL_TCB_Main( gameWork->task );

    GFL_G3D_DRAW_End();
  }

  // プレート更新
  GAME_PLATE_UpdatePlate( gameWork );

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:終了
 *	        ゲーム共通
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_exit_common( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;

  // サウンド
  // グループをアンロードしていなかった場合考えて
  // もう1度チェックする
  {
    NNSSndHeapHandle  snd_heap = PMSND_GetSndHeap();
    
    // 保存しておいた状態をチェックして
    // 戻していなかったら戻す
    {
      int level = NNS_SndHeapGetCurrentLevel( snd_heap );
      if( level == 1 )
      {
        NNS_SndHeapLoadState( snd_heap, level );
        OS_Printf("強制サウンドグループロード\n");
      }
    }
  }

  // 結果発表用風船破棄
  for( i=0;i<CTVT_MEMBER_NUM; i++ )
  {
    if( gameWork->p_balloon_s_wk[i] != NULL )
    {
      CTVT_BALLOON_S_Delete( gameWork->p_balloon_s_wk[i] );
    }
    gameWork->p_balloon_s_wk[i] = NULL;
  }
  
  // プレート削除
  {
    for( i=0;i<CTVT_MEMBER_NUM; i++ )
    {
      if( gameWork->p_plate[i] )
      {
        GAME_PLATE_DeletePlate( gameWork->p_plate[i] );
        gameWork->p_plate[i] = NULL;
      }
    }
  }

  // プレート用タスク
  GFL_TCB_Exit( gameWork->task_plate );
  if( gameWork->task_plate != NULL )
  {
    GFL_HEAP_FreeMemory( gameWork->task_plate );
    gameWork->task_plate = NULL;
  }
  if( gameWork->taskmem_plate != NULL )
  {
    GFL_HEAP_FreeMemory( gameWork->taskmem_plate );
    gameWork->taskmem_plate = NULL;
  }

  // 風船用タスク
  GFL_TCB_Exit( gameWork->task );
  if( gameWork->task != NULL )
  {
    GFL_HEAP_FreeMemory( gameWork->task );
    gameWork->task = NULL;
  }
  if( gameWork->taskMem != NULL )
  {
    GFL_HEAP_FreeMemory( gameWork->taskMem );
    gameWork->taskMem = NULL;
  }

  // OBJ破棄 
  for( i=0; i<3; i++ )
  {
    GRAPHIC_OBJ_Remove( gameWork->clwkTitle[i] );
    gameWork->clwkTitle[i] = NULL;
  }
  for( i=0; i<2; i++ )
  {
    GRAPHIC_OBJ_Remove( gameWork->clwkCloud_M[i] ); // 下画面の雲
    gameWork->clwkCloud_M[i] = NULL;
  }
  GRAPHIC_OBJ_Remove( gameWork->clwkTime );  // タイムゲージ
  gameWork->clwkTime = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkLast );  // ラストスパート
  gameWork->clwkLast = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkTouch ); // タッチするボタン
  gameWork->clwkTouch = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkPompBase ); // 空気入れ
  gameWork->clwkPompBase = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkPompHandle ); // 空気入れ
  gameWork->clwkPompHandle = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkPenAnm );  // ペンアニメ
  gameWork->clwkPenAnm = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkKekka );
  gameWork->clwkKekka = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkOmeFont );
  gameWork->clwkOmeFont = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkOmeNiji );
  gameWork->clwkOmeNiji = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkRank );
  gameWork->clwkRank = NULL;
  GRAPHIC_OBJ_Remove( gameWork->clwkDot );
  gameWork->clwkDot = NULL;
  for( i=0; i<6; i++ )
  {
    GRAPHIC_OBJ_Remove( gameWork->clwkOmeText[i] );
    gameWork->clwkOmeText[i] = NULL;
  }
  for( i=0; i<2; i++ )
  {
    GRAPHIC_OBJ_Remove( gameWork->clwkOmeHide[i] );
  }
  for( i=0; i<2; i++ )
  {
    GRAPHIC_OBJ_Remove( gameWork->clwkOmeCloud[i] );
  }

  // BMPWIN破棄
  if( gameWork->bmpWinTime )
  {
    // タイムバー
    GFL_BMPWIN_Delete( gameWork->bmpWinTime );
    gameWork->bmpWinTime = NULL;
  }
  if( gameWork->msgWinTitleFont )
  {
    // ゲームルール
    GFL_BMPWIN_Delete( gameWork->msgWinTitleFont );
    gameWork->msgWinTitleFont = NULL;
  }
  if( gameWork->msgWinTitle )
  {
    // ゲーム説明
    GFL_BMPWIN_Delete( gameWork->msgWinTitle );
    gameWork->msgWinTitle = NULL;
  }
  if( gameWork->msgRetry )
  {
    // リトライ文
    GFL_BMPWIN_Delete( gameWork->msgRetry );
    gameWork->msgRetry = NULL;
  }

  // リトライ用
  // はい・いいえウィンドウ削除
  if( gameWork->yesNoWork != NULL )
  {
    APP_TASKMENU_CloseMenu( gameWork->yesNoWork );
  }
  // キャンセルボタン削除
  if( gameWork->cancelWork != NULL )
  {
    APP_TASKMENU_CloseMenu( gameWork->cancelWork );
    gameWork->cancelWork = NULL;
  }
  // APPTASK共通リソース破棄
  if( gameWork->taskMenuRes != NULL )
  {
    APP_TASKMENU_RES_Delete( gameWork->taskMenuRes );
    gameWork->taskMenuRes = NULL;
  }

  // 画面を暗いのから戻す
  //G2_BlendNone();
  //G2S_BlendNone();
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:終了
 *	        ゲーム1専用
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_exit_part1( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;

  // 風船破棄
  for( i=0; i<BALLOON_MAX; i++ )
  {
    CTVT_BALLOON_Destroy( &gameWork->balloon_wk[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:終了
 *	        ゲーム2専用
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_exit_part2( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;
  // 2つ目の風船破棄
  for( i=0; i<CTVT_MEMBER_NUM; i++ )
  {
    if( gameWork->p_balloon2[i] )
    {
      CTVT_BALLOON2_Delete( gameWork->p_balloon2[i] );
    }
  }

  // BGパレットアニメのパレットデータ破棄
  if( gameWork->p_plt_buff )
  {
    GFL_HEAP_FreeMemory( gameWork->p_plt_buff );
    gameWork->p_plt_buff = NULL;
  }
}

//--------------------------------------------------------------
//	強制終了チェック
//--------------------------------------------------------------
static const BOOL CTVT_GAME_CheckFinishReq( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );

  if( COMM_TVT_GetConnectNum( work ) <= 1 )
  {
    //誰も居なくなった。
    OS_Printf("強制終了：誰もいなくなった\n");
    NetErr_App_ReqErrorDispForce( dwc_error_0015 );
    return TRUE;
  }
  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  乱数値を返す
 *	        ※返す値は、 0～(range-1)の範囲。range=0だと、32bit全範囲
 *
 *	@param	u32 range 乱数の取りうる値の範囲
 *
 *	@return u32 乱数値
 */
//-----------------------------------------------------------------------------
static u32 get_rand( u32 range )
{
  u32 result;
  result = GFL_STD_Rand( &gRandContext, range );
  return result;
}

//=============================================================================
/**
 *      GRAPHIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  描画関係初期化
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID )
{
  static const GFL_DISP_VRAM sc_vramSetTable =
  {
    GX_VRAM_BG_128_A,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,           // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_128_D,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_0_B,          // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_0_G,      // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_128K, // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_128K, // サブOBJマッピングモード
  };

  // レジスタ初期化
	GX_SetVisibleWnd( GX_WNDMASK_NONE );
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );
 
  //VRAMクリアー
  GFL_DISP_ClearVRAM( 0 );

  // VRAMバンク設定
  GFL_DISP_SetBank( &sc_vramSetTable );

  // ディスプレイON
  GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
  GFL_DISP_SetDispOn();

  //表示
  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();

  //描画モジュール
  GRAPHIC_BG_Init( p_wk, heapID );
  GRAPHIC_3D_Init( p_wk, heapID );

  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &sc_vramSetTable ,heapID );

    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );

    p_wk->cellUnit = GFL_CLACT_UNIT_Create( 64, 0, heapID );
    GFL_CLACT_UNIT_SetDefaultRend( p_wk->cellUnit );
  }
  // ライブｷｬｽﾀｰのセルアクターを作り直さないといけない

  // VBlank
  p_wk->vBlankTcb = GFUser_VIntr_CreateTCB( VBlankFunc , p_wk , 1 );

  p_wk->is_init = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画関係破棄
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Exit( CTVT_GAME_WORK* p_wk )
{
  GF_ASSERT( p_wk->is_init );

  GFL_TCB_DeleteTask( p_wk->vBlankTcb );

  GFL_CLACT_UNIT_Delete( p_wk->cellUnit );
  GFL_CLACT_SYS_Delete();

  GRAPHIC_3D_Exit( p_wk );
  GRAPHIC_BG_Exit( p_wk );

  p_wk->is_init = FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画関係描画処理
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Draw( CTVT_GAME_WORK* p_wk )
{
  u8 i;

  if( p_wk->is_init )
  {
    // 3D描画
    GFL_G3D_DRAW_Start();

    // カメラ更新
    GFL_G3D_CAMERA_Switching( p_wk->p_camera );
    GFL_G3D_DRAW_SetLookAt();

    //ライト設定
    NNS_G3dGlbLightVector( 0, 0, 0, FX32_ONE );
    NNS_G3dGlbLightColor( 0, GX_RGB(31,31,31) );
    NNS_G3dGlbMaterialColorDiffAmb(
        GX_RGB(31,31,31), GX_RGB(31,31,31), FALSE );
    NNS_G3dGlbMaterialColorSpecEmi(
        GX_RGB(31,31,31), GX_RGB(31,31,31), FALSE );

    // アニメーション更新
    for( i=0; i<CTVT_MEMBER_NUM; i++ )
    {
      int j;
      u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->g3dUtil, p_wk->unit_idx[i] );
      for( j= 0; j<3; j++ )
      {
        u16 obj_idx = head_obj_idx + j;
        GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( p_wk->g3dUtil, obj_idx );
        GFL_G3D_OBJECT_LoopAnimeFrame( obj, 0, FX32_ONE );
        GFL_G3D_OBJECT_LoopAnimeFrame( obj, 1, FX32_ONE );
      }
    }


#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(1);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    // 風船タスクメイン
    GFL_TCB_Main( p_wk->task );
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(1);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

    // 風船同士の当たり判定
    balloon_collision_check( p_wk );

    // クリアーカラー(水色)
    G3X_SetClearColor( GX_RGB(18, 29, 31 ), 0, 0x7fff, 63, FALSE);

    GFL_G3D_DRAW_End();
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  描画関係描画処理
 *	        ゲーム2で使う
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Draw2( CTVT_GAME_WORK* p_wk )
{
  u8 i;

  if( p_wk->is_init )
  {
    // 3D描画
    GFL_G3D_DRAW_Start();

    // カメラ更新
    GFL_G3D_CAMERA_Switching( p_wk->p_camera );
    GFL_G3D_DRAW_SetLookAt();

    //ライト設定
    NNS_G3dGlbLightVector( 0, 0, 0, FX32_ONE );
    NNS_G3dGlbLightColor( 0, GX_RGB(31,31,31) );
    NNS_G3dGlbMaterialColorDiffAmb(
        GX_RGB(31,31,31), GX_RGB(31,31,31), FALSE );
    NNS_G3dGlbMaterialColorSpecEmi(
        GX_RGB(31,31,31), GX_RGB(31,31,31), FALSE );

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(1);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    // 風船タスクメイン
    GFL_TCB_Main( p_wk->task );
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(1);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

    GFL_G3D_DRAW_End();
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  VBlank
 *
 *	@param	GFL_TCB *p_tcb
 *	@param	*p_work 
 */
//-----------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB *p_tcb, void *p_work )
{
  CTVT_GAME_WORK* p_wk = p_work;

  // BG転送  
  GFL_BG_VBlankFunc();
  // OBJの更新
  GFL_CLACT_SYS_VBlankFunc();

  //machine_useでやってた
  //  NNS_GfdDoVramTransfer();
}

//----------------------------------------------------------------------------
/**
 *	@brief  セルワークセーフリリース
 *
 *	@param	GFL_CLWK* p_wk 
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Remove( GFL_CLWK* p_wk )
{
  if( p_wk != NULL )
  {
    GFL_CLACT_WK_Remove( p_wk );
    p_wk = NULL;
  }
}

//=============================================================================
/**
 *      GRAPHIC_BG
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  BG描画  初期化
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID )
{
  GFL_BG_Init( heapID );
  GFL_BMPWIN_Init( heapID );

  // BGモード設定
  { 
    GFL_BG_SYS_HEADER sysh = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGMode( &sysh );
  }

  // 個別フレーム設定
  {
    //Main
    // BG1 BG (背景
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 MSG (文字
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 PLATE (プレート
    static const GFL_BG_BGCNT_HEADER header_main3 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x10000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // Sub
    // BG0 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub0 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 PLATE (プレート
    static const GFL_BG_BGCNT_HEADER header_sub1 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 TIMEBAR (タイムゲージ
    static const GFL_BG_BGCNT_HEADER header_sub2 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    GRAPHIC_BG_SetupBgFunc( &header_main1, CTVT_GAME_FRAME_M_BG, GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_main2, CTVT_GAME_FRAME_M_MSG, GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_main3, CTVT_GAME_FRAME_M_PLATE, GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_sub0, CTVT_GAME_FRAME_S_BG, GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_sub1, CTVT_GAME_FRAME_S_PLATE, GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_sub2, CTVT_GAME_FRAME_S_TIMEBAR, GFL_BG_MODE_TEXT );
  }

  // ブレンド
  G2_SetBlendAlpha(
      GX_BLEND_PLANEMASK_BG0,
      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
      0, 16 );
  // 3Dのアルファをきれいに出すため、第1対象面にBG0を指定している
  // ev1とev2は使われない  // TWLプログラミングマニュアル「2D面とのαブレンディング」参考


  // BGプライオリティを設定
  GFL_BG_SetPriority( CTVT_GAME_FRAME_M_3D, 2 ); 
  GFL_BG_SetPriority( CTVT_GAME_FRAME_M_MSG,0 ); 
  GFL_BG_SetPriority( CTVT_GAME_FRAME_M_BG, 3 ); 
  GFL_BG_SetPriority( CTVT_GAME_FRAME_M_PLATE, 0 ); 
  GFL_BG_SetPriority( CTVT_GAME_FRAME_S_BG, 3 ); 
  GFL_BG_SetPriority( CTVT_GAME_FRAME_S_PLATE, 0 ); 
  GFL_BG_SetPriority( CTVT_GAME_FRAME_S_TIMEBAR, 1 ); 

  GFL_BG_SetVisible( CTVT_GAME_FRAME_M_3D,    VISIBLE_ON );
  GFL_BG_SetVisible( CTVT_GAME_FRAME_M_MSG,   VISIBLE_ON );
  GFL_BG_SetVisible( CTVT_GAME_FRAME_M_BG,    VISIBLE_ON );
  GFL_BG_SetVisible( CTVT_GAME_FRAME_M_PLATE, VISIBLE_ON ); 
  GFL_BG_SetVisible( CTVT_GAME_FRAME_S_BG,    VISIBLE_ON );
  GFL_BG_SetVisible( CTVT_GAME_FRAME_S_PLATE, VISIBLE_ON );
  GFL_BG_SetVisible( CTVT_GAME_FRAME_S_TIMEBAR, VISIBLE_ON );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG描画  破棄
 *
 *	@param	CTVT_GAME_WORK* p_wk 
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( CTVT_GAME_WORK* p_wk )
{
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG設定
 *	        comm_tvt_sys.cのGRAPHIC_BG_SetupBgFuncをそのままコピーしてきた
 *
 *	@param	const GFL_BG_BGCNT_HEADER *bgCont 
 *	@param	bgPlane 
 *	@param	mode 
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//=============================================================================
/**
 *      GRAPHIC_3D
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D環境  初期化
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID )
{
  // 3Dシステム初期化
  GFL_G3D_Init(
      GFL_G3D_VMANLNK, GFL_G3D_TEX128K,
      GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, heapID, NULL );

  GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );

  G3X_SetClearColor(GX_RGB(0x1f,0x1f,0x1f),0,0x7fff,63,FALSE); //color,alpha,depth,polygonID,fog
  G3X_AlphaBlend( TRUE );
  G3X_EdgeMarking( FALSE );
  G3X_AntiAlias( TRUE );
  G3X_SetFog( FALSE, 0, 0, 0 );

  //カメラ設定
#if 0
  // 正射影
  {
    static const VecFx32 cam_pos = { FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(400.0f) };
    static const VecFx32 cam_target = { FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f) };
    static const VecFx32 defaultCameraUp = { 0, FX32_ONE, 0 };

    p_wk->p_camera = GFL_G3D_CAMERA_CreateOrtho(
        FX32_CONST(96),
        -FX32_CONST(96),
        -FX32_CONST(128),
        FX32_CONST(128),
        defaultCameraNear, defaultCameraFar, 0,
        &cam_pos, &defaultCameraUp, &cam_target,
        heapID );

    GFL_G3D_CAMERA_Switching( p_wk->p_camera );
  }
#endif
  // 透視射影
  { 
    static const VecFx32 cam_pos    = { 0, 0 , 250*FX32_ONE};
    static const VecFx32 cam_target = { 0,0,0};
    p_wk->p_camera = GFL_G3D_CAMERA_CreateDefault( &cam_pos, &cam_target, heapID );

    GFL_G3D_CAMERA_Switching( p_wk->p_camera );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  3D環境  破棄
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Exit( CTVT_GAME_WORK* p_wk )
{
  GFL_G3D_CAMERA_Delete( p_wk->p_camera );
  GFL_G3D_Exit();
}

//=============================================================================
/**
 *      RESOURCE
 */
//=============================================================================
// RES
// プレイヤー1
static const GFL_G3D_UTIL_RES res_3d_table1[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbtp, GFL_G3D_UTIL_RESARC },  // アニメ
};
// プレイヤー2
static const GFL_G3D_UTIL_RES res_3d_table2[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbtp, GFL_G3D_UTIL_RESARC },  // アニメ
};
// プレイヤー3
static const GFL_G3D_UTIL_RES res_3d_table3[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbtp, GFL_G3D_UTIL_RESARC },  // アニメ
};
// プレイヤー4
static const GFL_G3D_UTIL_RES res_3d_table4[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbtp, GFL_G3D_UTIL_RESARC },  // アニメ
};
// エフェクト
static const GFL_G3D_UTIL_RES res_3d_table5[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_effect1_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_effect1_nsbca, GFL_G3D_UTIL_RESARC },
};

// ANIME 風船
static const GFL_G3D_UTIL_ANM anm_3d_table[] =
{
  { 3, 0 }, // nsbmp
  { 4, 0 }, // nsbtp
};

// ANIME エフェクト
static const GFL_G3D_UTIL_ANM anm_3d_table2[] =
{
  { 1, 0 },
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] = 
{
  {
    0,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    0,      // テクスチャリソースID
    anm_3d_table,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table),      // アニメリソース数
  },
  {
    1,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    1,      // テクスチャリソースID
    anm_3d_table,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table),      // アニメリソース数
  },
  {
    2,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    2,      // テクスチャリソースID
    anm_3d_table,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table),      // アニメリソース数
  },
};
static const GFL_G3D_UTIL_OBJ obj_3d_table2[] = 
{
  {
    0,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    0,      // テクスチャリソースID
    anm_3d_table2,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table2),      // アニメリソース数
  },
};

//======================================================
// ゲーム2の風船 アニメはなし
// プレイヤー1
static const GFL_G3D_UTIL_RES res_3d_table1_2[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big1_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big1_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big1_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
};
// プレイヤー2
static const GFL_G3D_UTIL_RES res_3d_table2_2[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big2_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big2_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big2_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
};
// プレイヤー3
static const GFL_G3D_UTIL_RES res_3d_table3_2[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big3_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big3_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big3_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
};
// プレイヤー4
static const GFL_G3D_UTIL_RES res_3d_table4_2[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big4_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big4_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon_big4_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
};

// エフェクト
static const GFL_G3D_UTIL_RES res_3d_table5_2[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_effect2_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_effect2_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_effect2_nsbma, GFL_G3D_UTIL_RESARC },
};

// ANIME エフェクト
static const GFL_G3D_UTIL_ANM anm_3d_table2_2[] =
{
  { 1, 0 },
  { 2, 0 },
};

static const GFL_G3D_UTIL_OBJ obj_3d_table2_2[] = 
{
  {
    0,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    0,      // テクスチャリソースID
    anm_3d_table2_2,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table2_2),      // アニメリソース数
  },
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table_2[] = 
{
  {
    0,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    0,      // テクスチャリソースID
    NULL,   // アニメテーブル(複数指定のため)
    0,      // アニメリソース数
  },
  {
    1,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    1,      // テクスチャリソースID
    NULL,   // アニメテーブル(複数指定のため)
    0,      // アニメリソース数
  },
  {
    2,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    2,      // テクスチャリソースID
    NULL,   // アニメテーブル(複数指定のため)
    0,      // アニメリソース数
  },
};


// 結果発表用
// プレイヤー1
static const GFL_G3D_UTIL_RES res_3d_table1_score[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbtp, GFL_G3D_UTIL_RESARC },  // アニメ
};
// プレイヤー2
static const GFL_G3D_UTIL_RES res_3d_table2_score[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbtp, GFL_G3D_UTIL_RESARC },  // アニメ
};
// プレイヤー3
static const GFL_G3D_UTIL_RES res_3d_table3_score[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbtp, GFL_G3D_UTIL_RESARC },  // アニメ
};
// プレイヤー4
static const GFL_G3D_UTIL_RES res_3d_table4_score[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbtp, GFL_G3D_UTIL_RESARC },  // アニメ
};

// ANIME 風船
static const GFL_G3D_UTIL_ANM anm_3d_table_score[] =
{
  { 2, 0 }, // nsbmp
  { 3, 0 }, // nsbtp
};

static const GFL_G3D_UTIL_OBJ obj_3d_table_score[] = 
{
  {
    0,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    0,      // テクスチャリソースID
    anm_3d_table_score,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table_score),      // アニメリソース数
  },
  {
    1,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    1,      // テクスチャリソースID
    anm_3d_table_score,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table_score),      // アニメリソース数
  },
};

#define CTVT_UNIT_RES_MAX (22)    // 3種 * 4人 + アニメ2種*4 +エフェクト2
#define CTVT_UNIT_OBJ_MAX (13)    // 3種 * 4 + エフェクト1

// セットアップデータ
// 覚書
// ユニットのOBJとリソースの数は違うので、
// ただ単にunit_idx を使用しても、そこからOBJを取得するか、
// リソースを取得するかで、加算の値が違う
// ここで言うと、OBJは３つだが、リソースは4つなので、
// ユニットの0番目は0
// ユニットの1番目は4 となる ←モデルが3つなので、＋３と間違えやすい
static const GFL_G3D_UTIL_SETUP G3DUtilSetup[] =
{
  { res_3d_table1, NELEMS(res_3d_table1), obj_3d_table, NELEMS(obj_3d_table) },
  { res_3d_table2, NELEMS(res_3d_table2), obj_3d_table, NELEMS(obj_3d_table) },
  { res_3d_table3, NELEMS(res_3d_table3), obj_3d_table, NELEMS(obj_3d_table) },
  { res_3d_table4, NELEMS(res_3d_table4), obj_3d_table, NELEMS(obj_3d_table) },
  { res_3d_table5, NELEMS(res_3d_table5), obj_3d_table2, NELEMS(obj_3d_table2) },
};

static const GFL_G3D_UTIL_SETUP G3DUtilSetup_2[] =
{
  { res_3d_table1_2, NELEMS(res_3d_table1_2), obj_3d_table_2, NELEMS(obj_3d_table_2) },
  { res_3d_table2_2, NELEMS(res_3d_table2_2), obj_3d_table_2, NELEMS(obj_3d_table_2) },
  { res_3d_table3_2, NELEMS(res_3d_table3_2), obj_3d_table_2, NELEMS(obj_3d_table_2) },
  { res_3d_table4_2, NELEMS(res_3d_table4_2), obj_3d_table_2, NELEMS(obj_3d_table_2) },
  { res_3d_table5_2, NELEMS(res_3d_table5_2), obj_3d_table2_2, NELEMS(obj_3d_table2_2) },
};

static const GFL_G3D_UTIL_SETUP G3DUtilSetup_score[] =
{
  { res_3d_table1_score, NELEMS(res_3d_table1_score), obj_3d_table_score, NELEMS(obj_3d_table_score) },
  { res_3d_table2_score, NELEMS(res_3d_table2_score), obj_3d_table_score, NELEMS(obj_3d_table_score) },
  { res_3d_table3_score, NELEMS(res_3d_table3_score), obj_3d_table_score, NELEMS(obj_3d_table_score) },
  { res_3d_table4_score, NELEMS(res_3d_table4_score), obj_3d_table_score, NELEMS(obj_3d_table_score) },
  { res_3d_table5, NELEMS(res_3d_table5), obj_3d_table2, NELEMS(obj_3d_table2) },
};

//----------------------------------------------------------------------------
/**
 *	@brief  リソース読み込み
 *	        ミニゲーム1用
 *
 *	@param	CTVT_GAME_WORK* p_wk 
 */
//-----------------------------------------------------------------------------
static void RESOURCE_Init_Part1( CTVT_GAME_WORK* p_wk, HEAPID heapID, ARCHANDLE* arcHandle )
{
  u8  i;

  // OBJ
  // 上画面(SUB)
  p_wk->cellResIdx[CGOR_UPPER_PLT] = GFL_CLGRP_PLTT_RegisterComp( arcHandle, 
      NARC_tvt_balloon_game_obj_u_NCLR, CLSYS_DRAW_SUB, 
      CTVT_GAME_PAL_OBJ_COMMON*32, heapID );
  p_wk->cellResIdx[CGOR_UPPER_NCG] = GFL_CLGRP_CGR_Register( arcHandle, 
      NARC_tvt_balloon_game_obj_u_NCGR, FALSE, CLSYS_DRAW_SUB, heapID );
  p_wk->cellResIdx[CGOR_UPPER_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle, 
      NARC_tvt_balloon_game_obj_u_NCER, NARC_tvt_balloon_game_obj_u_NANR, heapID );

  // 下画面(MAIN)
  p_wk->cellResIdx[CGOR_LOWER_PLT] = GFL_CLGRP_PLTT_RegisterComp( arcHandle, 
      NARC_tvt_balloon_game_obj_l_NCLR, CLSYS_DRAW_MAIN, 
      CTVT_GAME_PAL_OBJ_COMMON*32, heapID );
  p_wk->cellResIdx[CGOR_LOWER_NCG] = GFL_CLGRP_CGR_Register( arcHandle, 
      NARC_tvt_balloon_game_obj_l_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );
  p_wk->cellResIdx[CGOR_LOWER_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle, 
      NARC_tvt_balloon_game_obj_l_NCER, NARC_tvt_balloon_game_obj_l_NANR, heapID );
  
  // 3Dリソース
  {
    // 3D管理ユーティリティーのセットアップ
    p_wk->g3dUtil = GFL_G3D_UTIL_Create( CTVT_UNIT_RES_MAX, CTVT_UNIT_OBJ_MAX, heapID );

    // ユニット追加
    for( i=0; i<CTVT_UNIT_IDX_MAX; i++ )
    {
      p_wk->unit_idx[i] = GFL_G3D_UTIL_AddUnit( p_wk->g3dUtil, &G3DUtilSetup[i] );
      ICHI_Printf("ユニット追加 プレイヤー%d = %d\n", i, p_wk->unit_idx[i]);
    }

    // アニメーション有効化
    {
      for( i=0; i<CTVT_MEMBER_NUM; i++ )
      {
        int j;
        u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->g3dUtil, p_wk->unit_idx[i] );
        for( j= 0; j<3; j++ )
        {
          u16 obj_idx = head_obj_idx + j;
          GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( p_wk->g3dUtil, obj_idx );
          GFL_G3D_OBJECT_EnableAnime( obj, 0 ); // ica
          GFL_G3D_OBJECT_EnableAnime( obj, 1 ); // itp
        }
      }

      // エフェクトのアニメーション有効化
      {
        u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->g3dUtil, CTVT_UNIT_IDX_EFFECT );
        u16 obj_idx = head_obj_idx;
        GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( p_wk->g3dUtil, obj_idx );
        GFL_G3D_OBJECT_EnableAnime( obj, 0 );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リソース破棄
 *	        ミニゲーム1用
 *
 *	@param	CTVT_GAME_WORK* p_wk 
 */
//-----------------------------------------------------------------------------
static void RESOURCE_Exit_Part1( CTVT_GAME_WORK* p_wk )
{
  u8  i;

  // リソースの破棄
  for( i=CGOR_PLT_TOP; i<CGOR_NCG_TOP; i++ )
  {
    GFL_CLGRP_PLTT_Release( p_wk->cellResIdx[i] );
  }
  for( i=CGOR_NCG_TOP;i<CGOR_ANM_TOP;i++ )
  {
    GFL_CLGRP_CGR_Release( p_wk->cellResIdx[i] );
  }
  for( i=CGOR_ANM_TOP;i<CGOR_MAX;i++ )
  {
    GFL_CLGRP_CELLANIM_Release( p_wk->cellResIdx[i] );
  }

  // ユニット破棄
  for( i=0; i<CTVT_UNIT_IDX_MAX; i++ )
  {
    GFL_G3D_UTIL_DelUnit( p_wk->g3dUtil, p_wk->unit_idx[i] );
  }

  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( p_wk->g3dUtil );
}

//----------------------------------------------------------------------------
/**
 *	@brief  リソース読み込み
 *	        ミニゲーム2用
 *
 *	@param	CTVT_GAME_WORK* p_wk 
 */
//-----------------------------------------------------------------------------
static void RESOURCE_Init_Part2( CTVT_GAME_WORK* p_wk, HEAPID heapID, ARCHANDLE* arcHandle )
{
  u8  i;

  // OBJ
  // 上画面(SUB)
  p_wk->cellResIdx[CGOR_UPPER_PLT] = GFL_CLGRP_PLTT_RegisterComp( arcHandle, 
      NARC_tvt_balloon_game_obj_u_NCLR, CLSYS_DRAW_SUB, 
      CTVT_GAME_PAL_OBJ_COMMON*32, heapID );
  p_wk->cellResIdx[CGOR_UPPER_NCG] = GFL_CLGRP_CGR_Register( arcHandle, 
      NARC_tvt_balloon_game_obj_u_NCGR, FALSE, CLSYS_DRAW_SUB, heapID );
  p_wk->cellResIdx[CGOR_UPPER_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle, 
      NARC_tvt_balloon_game_obj_u_NCER, NARC_tvt_balloon_game_obj_u_NANR, heapID );

  // 下画面(MAIN)
  p_wk->cellResIdx[CGOR_LOWER_PLT] = GFL_CLGRP_PLTT_RegisterComp( arcHandle, 
      NARC_tvt_balloon_game_obj_l_NCLR, CLSYS_DRAW_MAIN, 
      CTVT_GAME_PAL_OBJ_COMMON*32, heapID );
  p_wk->cellResIdx[CGOR_LOWER_NCG] = GFL_CLGRP_CGR_Register( arcHandle, 
      NARC_tvt_balloon_game_obj_l_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );
  p_wk->cellResIdx[CGOR_LOWER_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle, 
      NARC_tvt_balloon_game_obj_l_NCER, NARC_tvt_balloon_game_obj_l_NANR, heapID );
  
  // 3Dリソース
  {
    // 3D管理ユーティリティーのセットアップ
    p_wk->g3dUtil = GFL_G3D_UTIL_Create( CTVT_UNIT_RES_MAX, CTVT_UNIT_OBJ_MAX, heapID );

    // ユニット追加
    for( i=0; i<CTVT_UNIT_IDX_MAX; i++ )
    {
      p_wk->unit_idx[i] = GFL_G3D_UTIL_AddUnit( p_wk->g3dUtil, &G3DUtilSetup_2[i] );
      ICHI_Printf("ユニット追加 プレイヤー%d = %d\n", i, p_wk->unit_idx[i]);
    }

    // アニメーション有効化
    // エフェクトのアニメーション有効化
    {
      u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->g3dUtil, CTVT_UNIT_IDX_EFFECT );
      u16 obj_idx = head_obj_idx;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( p_wk->g3dUtil, obj_idx );
      GFL_G3D_OBJECT_EnableAnime( obj, 0 );
      GFL_G3D_OBJECT_EnableAnime( obj, 1 );
    }

    ICHI_Printf("総オブジェクト数 %d\n", GFL_G3D_UTIL_GetObjCount(p_wk->g3dUtil));
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リソース破棄
 *	        ミニゲーム2用
 *
 *	@param	CTVT_GAME_WORK* p_wk 
 */
//-----------------------------------------------------------------------------
static void RESOURCE_Exit_Part2( CTVT_GAME_WORK* p_wk )
{
  u8  i;

  // リソースの破棄
  for( i=CGOR_PLT_TOP; i<CGOR_NCG_TOP; i++ )
  {
    GFL_CLGRP_PLTT_Release( p_wk->cellResIdx[i] );
  }
  for( i=CGOR_NCG_TOP;i<CGOR_ANM_TOP;i++ )
  {
    GFL_CLGRP_CGR_Release( p_wk->cellResIdx[i] );
  }
  for( i=CGOR_ANM_TOP;i<CGOR_MAX;i++ )
  {
    GFL_CLGRP_CELLANIM_Release( p_wk->cellResIdx[i] );
  }

  // ユニット破棄
  for( i=0; i<CTVT_UNIT_IDX_MAX; i++ )
  {
    GFL_G3D_UTIL_DelUnit( p_wk->g3dUtil, p_wk->unit_idx[i] );
  }

  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( p_wk->g3dUtil );
}


//----------------------------------------------------------------------------
/**
 *	@brief  結果発表で使用する3Dリソースを読み込み
 *	        ゲーム中の3Dリソースは破棄してから呼ぶ
 *
 *	@param	CTVT_GAME_WORK* p_wk
 *	@param	heapID 
 */
//-----------------------------------------------------------------------------
static void RESOURCE3D_Init_Score( CTVT_GAME_WORK* p_wk, HEAPID heapID )
{
  u8 i;
  
  // ユニット追加
  for( i=0; i<CTVT_UNIT_IDX_MAX; i++ )
  {
    p_wk->unit_idx[i] = GFL_G3D_UTIL_AddUnit( p_wk->g3dUtil, &G3DUtilSetup_score[i] );
    ICHI_Printf("ユニット追加 プレイヤー%d = %d\n", i, p_wk->unit_idx[i]);
  }

  // アニメーション有効化
  {
    for( i=0; i<CTVT_MEMBER_NUM; i++ )
    {
      u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->g3dUtil, p_wk->unit_idx[i] );

      u16 obj_idx = head_obj_idx;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( p_wk->g3dUtil, obj_idx );
      GFL_G3D_OBJECT_EnableAnime( obj, 0 ); // ica
      GFL_G3D_OBJECT_EnableAnime( obj, 1 ); // itp
    }

    // エフェクトのアニメーション有効化
    {
      u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->g3dUtil, CTVT_UNIT_IDX_EFFECT );
      u16 obj_idx = head_obj_idx;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( p_wk->g3dUtil, obj_idx );
      GFL_G3D_OBJECT_EnableAnime( obj, 0 );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ表示 リトライメッセージ
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	p_wk
 *	@param	u16 msgId 
 */
//-----------------------------------------------------------------------------
static void disp_message_retry( COMM_TVT_WORK *work, CTVT_GAME_WORK* p_wk, const u16 msgId ) 
{
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
  GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
  GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
  PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
  WORDSET *wordSet= WORDSET_Create( heapId );
  STRBUF *tempStr = GFL_STR_CreateBuffer( 64 , heapId );
  STRBUF *str;

  PRINTSYS_QUE_Clear( printQue );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->msgRetry), 0xF );
  str = GFL_MSG_CreateString( msgHandle, msgId );

  // ゲーム名
  {
    u32 strID = COMM_TVT_GAME_18 + CTVT_GAME_GetGameID( p_wk );
    STRBUF *gameName = GFL_MSG_CreateString( msgHandle, strID );
    WORDSET_RegisterWord( wordSet, 0, gameName, 0, TRUE, PM_LANG );
    GFL_STR_DeleteBuffer( gameName );
  }
  WORDSET_ExpandStr( wordSet, tempStr, str );
  GFL_STR_DeleteBuffer( str );
  str = tempStr;

  PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp( p_wk->msgRetry ), 
      0, 0, str, fontHandle,CTVT_FONT_COLOR_BLACK );
  GFL_STR_DeleteBuffer( str );
  WORDSET_Delete( wordSet );
  GFL_BMPWIN_TransVramCharacter( p_wk->msgRetry );
  GFL_BMPWIN_MakeScreen( p_wk->msgRetry );

  BmpWinFrame_Write( p_wk->msgRetry, WINDOW_TRANS_ON_V, 
      CTVT_GAME_BMPWIN_CGX_MAIN, CTVT_GAME_PAL_BG_M_WINFRAME );
  p_wk->isUpdateMsgRetry = TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  TIMEbar描画
 *
 *	@param	CTVT_GAME_WORK* p_wk 
 */
//-----------------------------------------------------------------------------
static void draw_timebar( CTVT_GAME_WORK* p_wk )
{
  const u8 pos_x  = 0;
  const u8 pos_y  = 1;
  const u8 size_x = 177;  // 塗りつぶす幅 22キャラ+1dot
  const u8 size_y = 12;   // 塗りつぶす高さ
  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( p_wk->bmpWinTime );
  u8 def = (TIME_BAR_W*8) / CTVT_GAME_TIME_LIMIT; // 1秒で何ドット減るか
  u8 len = def * p_wk->timeLimit; // 何ドットベース色になるか
  u8 col;

  if( p_wk->timeLimit < 20 )
  {
    col = TIME_BAR_COL_R;
  }
  else if( p_wk->timeLimit < 40 )
  {
    col = TIME_BAR_COL_Y;
  }
  else
  {
    col = TIME_BAR_COL_G;
  }
  GFL_BMP_Fill( bmp, pos_x, pos_y, size_x, size_y, TIME_BAR_COL_BASE );
  GFL_BMP_Fill( bmp, pos_x, pos_y, len, size_y, col );

  GFL_BMPWIN_MakeTransWindow( p_wk->bmpWinTime );
}


//----------------------------------------------------------------------------
/**
 *	@brief  リトライ時の常時終了チェック
 *
 *	@param	COMM_TVT_WORK *work 
 *	@param	*gameWork 
 *
 *	@return TRUE で終了
 */
//-----------------------------------------------------------------------------
static const BOOL retry_check_finish_req( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  BOOL  ret = FALSE;

  // 子のリトライ拒否が通知
  if( gameWork->isCancelGame == TRUE )
  {
    if( COMM_TVT_GetSelfIdx(work) == 0 )
    {
      ret = TRUE;
    }
  }

  // 親から中止が通知
  if( gameWork->isExitGame == TRUE )
  {
    if( COMM_TVT_GetSelfIdx(work) != 0 )
    {
      ret = TRUE;
    }
  }

  if( ret == TRUE )
  {
    // メニューが出てる場合はとじる
    if( gameWork->yesNoWork != NULL )
    {
      APP_TASKMENU_CloseMenu( gameWork->yesNoWork );
      gameWork->yesNoWork = NULL;
    }
    if( gameWork->cancelWork != NULL )
    {
      APP_TASKMENU_CloseMenu( gameWork->cancelWork );
      gameWork->cancelWork = NULL;
    }

    // ちゅうしされました メッセージ表示
    disp_message_retry( work, gameWork, COMM_TVT_GAME_03 );
    gameWork->msgWait = 90;    // メッセージ表示待ち時間
    gameWork->isRetry = FALSE;  // 念のため

    ICHI_Printf("常時チェック中止!\n");
  }

  return ret;
}


//----------------------------------------------------------------------------
/**
 *	@brief  YesNoを出す
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 *
 *	@return APP_TASKUMENU構造体
 */
//-----------------------------------------------------------------------------
APP_TASKMENU_WORK* retry_open_yesno_menu( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  APP_TASKMENU_WORK *menuWork;
  APP_TASKMENU_INITWORK initWork = {0};
  APP_TASKMENU_ITEMWORK itemWork[2] = {0};
  GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );

  itemWork[0].str = GFL_MSG_CreateString( msgHandle, COMM_TVT_SYS_03 );
  itemWork[1].str = GFL_MSG_CreateString( msgHandle, COMM_TVT_SYS_04 );
  itemWork[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  itemWork[1].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  itemWork[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  itemWork[1].type = APP_TASKMENU_WIN_TYPE_NORMAL;

  initWork.heapId = gameWork->heapId;
  initWork.itemNum = 2;
  initWork.itemWork = itemWork;
  initWork.posType  = ATPT_LEFT_UP;
  initWork.charPosX = 32-APP_TASKMENU_PLATE_WIDTH_YN_WIN;
  initWork.charPosY = 15;
  initWork.w = APP_TASKMENU_PLATE_WIDTH_YN_WIN;
  initWork.h = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;

  menuWork = APP_TASKMENU_OpenMenu( &initWork , gameWork->taskMenuRes );

  GFL_STR_DeleteBuffer( itemWork[0].str );
  GFL_STR_DeleteBuffer( itemWork[1].str );

  return menuWork;
}


//----------------------------------------------------------------------------
/**
 *	@brief  「ちゅうし」をだす
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 *
 *	@return APP_TASKMENU構造体
 */
//-----------------------------------------------------------------------------
APP_TASKMENU_WORK* retry_open_cancel_menu( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  APP_TASKMENU_WORK *menuWork;
  APP_TASKMENU_INITWORK initWork = {0};
  APP_TASKMENU_ITEMWORK itemWork = {0};
  GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );

  itemWork.str = GFL_MSG_CreateString( msgHandle, COMM_TVT_GAME_22 );
  itemWork.msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  itemWork.type = APP_TASKMENU_WIN_TYPE_NORMAL;

  initWork.heapId = gameWork->heapId;
  initWork.itemNum = 1;
  initWork.itemWork = &itemWork;
  initWork.posType = ATPT_LEFT_UP;
  initWork.charPosX = 32-APP_TASKMENU_PLATE_WIDTH_YN_WIN;
  initWork.charPosY = 15;
  initWork.w = APP_TASKMENU_PLATE_WIDTH_YN_WIN;
  initWork.h = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;

  menuWork = APP_TASKMENU_OpenMenu( &initWork , gameWork->taskMenuRes );

  GFL_STR_DeleteBuffer( itemWork.str );

  return menuWork;
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船出現の初期化
 *	        初期位置や方向など
 *
 *	@param	CTVT_GAME_WORK *p_game_wk
 *	@param	p_wk        設定する風船の構造体
 *	@param	unit_idx    ユニットID 　どのモデルか？
 */
//-----------------------------------------------------------------------------
static void balloon_set( CTVT_GAME_WORK *p_game_wk, BALLOON_WORK*  p_wk, u16  unit_idx )
{
  p_wk->p_game_wk = p_game_wk;
  // サイズ決定 
  //p_wk->size  = get_rand(3) + 1;  // 1～3;
  p_wk->size  = p_game_wk->balloon_z_cnt + 1;

  p_wk->unit_idx    = unit_idx;   // モデル指定
  p_wk->visible     = TRUE;       // フラグON
  p_wk->alpha = 31;

  p_wk->save_frame = p_game_wk->frame;


  //ICHI_Printf("風船出現 %d unit %d ", p_wk->tcb, unit_idx);

  {
    // 風船の初期位置
    fx32  x;
    fx32  y;
    fx32  z;

    // X軸乱数( X軸をBALLOON_RND_W_MAXに分けて、その中での乱数 )
    {
      u16 rate = 256 / BALLOON_RND_W_MAX;
      u16 tmp;

      // 最初に0からrateの乱数をだして、調整用の乱数分足す
      tmp = get_rand( rate ) + (rate * p_game_wk->balloon_rnd_w);
      x = FX32_CONST(-128) + FX32_ONE * tmp;
    }

    // Y軸乱数
    y = FX32_ONE * ( -240 );

    // Z軸乱数( -100,-200,-300 )
    //z = 0;
    //z = FX32_CONST(-60) * (4 - rnd);
    z = FX32_CONST(-80) * (4 - p_wk->size);

    VEC_Set( &p_wk->status.trans, x, y, z );
    //ICHI_Printf("pos%d:%d:%d ", FX_Whole(x), FX_Whole(y), FX_Whole(z));
  }

  {
    // 風船のサイズ(大・中・小)
#ifdef PM_DEBUG
    fx32 size = p_game_wk->debug_size * p_wk->size;
#else
    fx32 size = 0x200 * p_wk->size;
#endif

    size += 0x700;  // デフォルト値を足す
    VEC_Set( &p_wk->status.scale, size, size, size );
    //ICHI_Printf("size%x ", size);
  }
  // 風船の初期ベクトル
  {
    VecFx32 vec;
    MtxFx33 rot;
    u16     ang;
    u16     rate = 60 / BALLOON_RND_V_MAX;   // 60はてきとう
    fx32    vy =  ( get_rand(2) + 1) * 0xa00;

    VEC_Set( &vec, 0, FX32_ONE, 0 );    // 上へのベクトル = 速さの基準

    ang = get_rand( rate ) + ( rate * p_game_wk->balloon_rnd_v ) + 330; // +330=360-30 unsigned型だから-できなかった
    ang %= 360;
    ang *= 182;  // 65536÷360 = 182.044...
    GFL_CALC3D_MTX_CreateRot( 0, 0, ang, &rot );

    MTX_MultVec33( &vec, &rot, &p_wk->vec );    // 角度ベクトル
    VEC_Normalize( &p_wk->vec, &p_wk->vec );

    // 残り20秒ではやくなる
    if( p_game_wk->timeLimit <= 20 )
    {
#ifdef PM_DEBUG
      vy += p_game_wk->debug_v;
#else
      vy += 0x2000;
#endif
    }

    // 速さをきめる
    GFL_CALC3D_VEC_MulScalar( &p_wk->vec, vy, &p_wk->vec );
    // 速さを一定以上にする
    p_wk->vec.y += 0xc00;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  全ての風船が終了しているかチェック
 *
 *	@param	CTVT_GAME_WORK *p_wk 
 *
 *	@return TRUE  終了している
 */
//-----------------------------------------------------------------------------
static BOOL balloon_end_check( CTVT_GAME_WORK *p_wk )
{
  u8    i;
  for( i=0; i<BALLOON_MAX; i++ )
  {
    if( p_wk->balloon_wk[i].visible == TRUE )
    {
      // 1つでも表示しているものがあるとFALSE
      return FALSE;
    }
  }

  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  誰の風船を出すか計算して返す
 *
 *	@param	CTVT_GAME_WORK *p_wk 
 *
 *	@return u8  出すプレイヤー番号 0からの連番
 */
//-----------------------------------------------------------------------------
static u8 balloon_find_idx( CTVT_GAME_WORK *p_wk )
{
  u8  idx = p_wk->balloon_tbl_idx;
  u8  cnt = p_wk->balloon_tbl_cnt;
  u8  ret_idx;

  GF_ASSERT( idx<BALLOON_TBL_MAX );
  GF_ASSERT( cnt<CTVT_MEMBER_NUM );
  ret_idx = balloon_tbl[ idx ][ cnt ];

  // 次へ
  p_wk->balloon_tbl_cnt++;
  if( p_wk->balloon_tbl_cnt >= CTVT_MEMBER_NUM )
  {
    p_wk->balloon_tbl_cnt = 0;

    // 次のテーブルを求める
    p_wk->balloon_tbl_idx = get_rand( BALLOON_TBL_MAX );
  }

#ifdef PM_DEBUG
  p_wk->debug_b_cnt[ ret_idx ]++;
#endif

  return ret_idx;
}

// タスクに設定するタスク関数(風船用)
static void TaskFunc( GFL_TCB* p_tcb, void* p_work )
{
  BALLOON_WORK  *p_wk = (BALLOON_WORK*)p_work;

  if( p_wk->visible == TRUE )
  {

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(2);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    CTVT_BALLOON_Update( p_wk );
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(2);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(3);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    CTVT_BALLOON_Draw( p_wk );
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(3);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
  }
}

// タスクに設定するタスク関数(風船2用)
static void TaskFunc2( GFL_TCB* p_tcb, void* p_work )
{
  BALLOON2_WORK  *p_wk = (BALLOON2_WORK*)p_work;

  if( p_wk->visible == TRUE )
  {
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(2);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    CTVT_BALLOON2_Update( p_wk );
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(2);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(3);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    CTVT_BALLOON2_Draw( p_wk );
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(3);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
  }
}


// タスクに設定するタスク関数(結果発表の風専用)
static void TaskFuncScore( GFL_TCB* p_tcb, void* p_work )
{
  BALLOON_S_WORK  *p_wk = (BALLOON_S_WORK*)p_work;

  CTVT_BALLOON_S_Update( p_wk );
  CTVT_BALLOON_S_Draw( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  タスクに登録されている風船とタッチの当たり判定
 *
 *	@param	CTVT_GAME_WORK* p_wk    ゲーム管理構造体
 *	@param	tpx       タッチX座標
 *	@param	tpy       タッチY座標
 *	@param	*ret_idx  割れた風船のIDXを格納（当たったときのみ）　
 *
 *	@retval TRUE    当たり有り
 *	@retval FALSE   当たり無し
 */
//-----------------------------------------------------------------------------
static BOOL balloon_hit_check( CTVT_GAME_WORK* p_wk, u32 tpx, u32 tpy, u16 *ret_idx )
{
  u8  i;

  GFL_COLLISION3D_CIRCLE    circle;
  VecFx32   center;   // 中心座標
  fx32      r;        // 半径

  VecFx32   rayPos;             // レイ開始座標
  VecFx32   rayWay;             // レイ方向
  BOOL      ret = FALSE;        // 返り値用
  u8        save_size = 0;      // 当たったやつのサイズ保存しておく

  // スクリーン座標からレイ取得
  GFL_CALC3D_VEC_Get2DPosTo3DRay( &rayPos, &rayWay, tpx, tpy );
  //OS_Printf("レイ座標%f:%f:%f レイ方向%f:%f:%f \n",(float)((float)rayPos.x/FX32_ONE),(float)((float)rayPos.y/FX32_ONE),(float)((float)rayPos.z/FX32_ONE),(float)((float)rayWay.x/FX32_ONE), (float)((float)rayWay.y/FX32_ONE), (float)((float)rayWay.z/FX32_ONE));


  for( i=0; i<BALLOON_MAX; i++ )
  {
    BOOL  b_hit = FALSE;

    if( p_wk->balloon_wk[i].visible == FALSE || p_wk->balloon_wk[i].type == BALLOON_HIT )
    {
      // 表示されていないならスキップ
      continue;
    }

    //OS_Printf("モデル座標 %.2f:%.2f:%.2f\n", (float)((float)p_wk->balloon_wk[i].status.trans.x/FX32_ONE), (float)((float)p_wk->balloon_wk[i].status.trans.y/FX32_ONE),(float)((float)p_wk->balloon_wk[i].status.trans.z/FX32_ONE) );


    // 球設定
    // 半径はサイズによって変わるので、割合を掛けている FX_MULは使用しない
    r = p_wk->balloon_wk[i].status.scale.x * 0x45;
#ifdef PM_DEBUG
    r = p_wk->balloon_wk[i].status.scale.x * p_wk->debug_r;
#endif
    VEC_Set( &center, p_wk->balloon_wk[i].status.trans.x, p_wk->balloon_wk[i].status.trans.y, p_wk->balloon_wk[i].status.trans.z );

    //OS_Printf("w %.2f PosScale %.2f 半径 %.2f size %x \n", (float)((float)modelInfo->boxW/FX32_ONE),(float)((float)modelInfo->posScale/FX32_ONE),(float)((float)r/FX32_ONE), p_wk->balloon_wk[i].status.scale.x );

    GFL_COLLISION3D_CIRCLE_SetData( &circle, &center, r );

    // 当たり判定
    b_hit = collision3d_sphere_checkRay( &rayPos, &rayWay, &circle );
    if( b_hit == TRUE )
    {
      //ICHI_Printf("当たった！\n");

      // Hitデータ保存
      ret = TRUE;

      // サイズ大だと手前にあるはずなので、それ以降の処理はしない
      if( p_wk->balloon_wk[i].size == BALLOON_SIZE_L )
      {
        *ret_idx = i;   // 何番目か保存
        return TRUE;
      }

      // サイズ比較 (手前にあるか奥にあるか)
      if( p_wk->balloon_wk[i].size > save_size )
      {
        *ret_idx = i;   // 何番目か保存
        save_size = p_wk->balloon_wk[i].size;   // サイズを保存しておく
      }
    }
  }

  return ret;
}


// タスクに登録されている風船同士の距離判定
//----------------------------------------------------------------------------
/**
 *	@brief  タスクに登録されている風船同士の距離判定
 *
 *	@param	CTVT_GAME_WORK *p_wk 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void balloon_collision_check( CTVT_GAME_WORK *p_wk )
{
  u8 i, j;

  // 19*19回
  for( i=0; i<BALLOON_MAX - 1; i++ )
  {

    // 表示していない風船はスキップ
    // 割れている風船もスキップ
    if( p_wk->balloon_wk[i].visible == FALSE || p_wk->balloon_wk[i].type == BALLOON_HIT )
    {
      continue;
    }

    for( j=i+1; j< BALLOON_MAX; j++ )
    {
      // 風船同士のサイズが同じ かつ 対象の風船も表示している
      if( p_wk->balloon_wk[i].size == p_wk->balloon_wk[j].size
          && p_wk->balloon_wk[j].visible == TRUE && p_wk->balloon_wk[j].type != BALLOON_HIT )
      {
        // 壁判定
        collision_vec( 
            &p_wk->balloon_wk[i].status.trans.x, &p_wk->balloon_wk[i].status.trans.y,
            &p_wk->balloon_wk[j].status.trans.x, &p_wk->balloon_wk[j].status.trans.y,
            p_wk->balloon_wk[i].status.scale.x * 140, 0 );
      }
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  2点間の距離判定を行い 衝突していると移動する
 *	        壁判定の処理
 *	        ※避けるのはX座標だけ
 *
 *	@param	fx32* x1  点1のX座標
 *	@param	fx32* y1  点1のY座標
 *	@param	fx32* x2  点2のX座標
 *	@param	fx32* y2  点2のY座標
 *	@param	fx32  dir 衝突の距離 
 *	@param  u8    avoid 避け方 0:両方避ける 1:点1が避ける 2:点2が避ける
 */
//-----------------------------------------------------------------------------
static void collision_vec( fx32* x1, fx32* y1, fx32* x2, fx32* y2, fx32 dir, u8 avoid )
{
  VecFx32 v1;   // ベクトル1
  VecFx32 v2;   // ベクトル2
  fx32    dist; // ベクトル間の距離

  VEC_Set( &v1, *x1, *y1, 0 );
  VEC_Set( &v2, *x2, *y2, 0 );

  // 距離取得
  dist = VEC_Distance( &v1, &v2 );

  // 衝突している
  if( dist < dir )
  {
    fx32  sub;
    fx32  *right, *left;

    sub = dir - dist;   // 避ける長さ
    //ICHI_Printf("直径 %x ", dir);
    //ICHI_Printf("避ける長さ %x \n", sub);

    // 右にある点と左にある点判別
    if( *x1 < *x2 )
    {
      left  = x1;
      right = x2;
    }
    else
    {
      left  = x2;
      right = x1;
    }

    // 避け方判別
    switch( avoid ){
    case 0:   // 両方避ける
      *right  += FX_Div(sub, 0x4800);
      *left   -= FX_Div(sub, 0x4800);
      break;
    case 1:   // 点1が避ける
      if( right == x1 )
      {
        *x1 += sub;
      }
      else if( left == x1 )
      {
        *x1 -= sub;
      }
      *y1 -= sub;   // 関数の意味とはことなるが風船の見た目のために入れた
      break;
    case 2:   // 点2が避ける
      if( right == x2 )
      {
        *x2 += sub;
      }
      else if( left == x2 )
      {
        *x2 -= sub;
      }
      break;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レイと球の交差判定
 *
 *	@param	*cp_rayPos  レイ開始座標
 *	@param	*cp_rayWay  レイ方向(正規化されていること！)
 *	@param	*cp_circle  円構造体
 *
 *	@retval	TRUE		当たる	
 *	@retval	FALSE		当たらない
 */
//-----------------------------------------------------------------------------
static BOOL collision3d_sphere_checkRay( const VecFx32 *cp_rayPos, const VecFx32 *cp_rayWay, const GFL_COLLISION3D_CIRCLE *cp_circle )
{
  // メモ覚書
  // ゲームプログラミングの為のリアルタイム衝突 .P177
  // 光線の式  R(t) = P + td, t>=0
  //           P:光線の原点
  //           d:光線の方向 (正規化されている)
  //
  //  球の式   (X-C)･(X-C) = r*r
  //           C:球の中心
  //           r:半径
  //
  //  上記2つの式でR(t) = Xと置き換えると
  //   (P + td - C)･(P + td - C) = r*r
  //  となる
  //  

  VecFx32   m;      // レイ->円へのベクトル
  fx32      c;      // 
  fx32      b;
  fx32      disc;


  // レイ->円ベクトル算出
  VEC_Subtract( cp_rayPos, &cp_circle->center, &m );
  // 
  c = VEC_DotProduct( &m, &m ) - FX_MUL( cp_circle->r, cp_circle->r);
  //OS_Printf("c %.2f \n", (float)((float)c/FX32_ONE));

  // 少なくとも1つの実数解が存在している場合、交差している
  if( c <= 0 ) return TRUE;

  b = VEC_DotProduct( &m, cp_rayWay );
  //OS_Printf("b %.2f \n", (float)((float)b/FX32_ONE));
  // レイの原点が球の外側にあり、レイが球から離れた方向を指している場合には早期に終了
  if( b > 0 ) return FALSE;

  disc = FX_Mul( b, b ) - c;
  //OS_Printf("d %.2f \n", (float)((float)disc/FX32_ONE));

  // 負の判別式はレイが球を外れていることに一致
  if( disc < 0 ) return FALSE;

  return TRUE;
}

//=============================================================================
/**
 *    PLATE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  プレート作成
 *
 *	@param	CTVT_GAME_WORK *p_wk  ワーク
 *	@param	u8 netID              NetID
 *
 *	@return 作成したプレート
 */
//-----------------------------------------------------------------------------
static GAME_PLATE_WORK* GAME_PLATE_CreatePlate( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork, const u8 netID )
{
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  const u8 baseX = PLATE_POS[netID][0];
  const u8 baseY = PLATE_POS[netID][1];
  GAME_PLATE_WORK *p_plate;
  p_plate = GFL_HEAP_AllocClearMemory( gameWork->heapId, sizeof(GAME_PLATE_WORK) );
  p_plate->heapID = gameWork->heapId;
  p_plate->rank   = netID + 1;
  p_plate->netID  = netID;
  p_plate->is_blank = FALSE;
  p_plate->is_mine = (netID==selfId) ? TRUE : FALSE;
  p_plate->swing_cnt= 0;
  p_plate->swing_wait= 0;
  p_plate->is_swing= FALSE;

  // セル作成
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = baseX;
    cellInitData.pos_y = baseY;
    cellInitData.anmseq = CGOAS_PLATE;
    cellInitData.softpri= 3;
    cellInitData.bgpri  = 1;
    p_plate->clwkPlate = GFL_CLACT_WK_Create( gameWork->cellUnit,
          gameWork->cellResIdx[CGOR_UPPER_PLT],
          gameWork->cellResIdx[CGOR_UPPER_NCG],
          gameWork->cellResIdx[CGOR_UPPER_ANM],
          &cellInitData ,CLSYS_DEFREND_SUB, gameWork->heapId );
    GFL_CLACT_WK_SetDrawEnable( p_plate->clwkPlate, FALSE );

    // 番号順でパレットをずらす 色変え
    GFL_CLACT_WK_SetPlttOffs( p_plate->clwkPlate, netID, CLWK_PLTTOFFS_MODE_PLTT_TOP );

    // ポイントのP
    cellInitData.pos_x = baseX + PLATE_OFFSET_X_P;
    cellInitData.pos_y = baseY;
    cellInitData.anmseq = CGOAS_P;
    cellInitData.softpri= 1;
    cellInitData.bgpri  = 1;
    p_plate->clwkP = GFL_CLACT_WK_Create( gameWork->cellUnit,
          gameWork->cellResIdx[CGOR_UPPER_PLT],
          gameWork->cellResIdx[CGOR_UPPER_NCG],
          gameWork->cellResIdx[CGOR_UPPER_ANM],
          &cellInitData ,CLSYS_DEFREND_SUB, gameWork->heapId );
    GFL_CLACT_WK_SetDrawEnable( p_plate->clwkP, FALSE );
  }

  // BMPWIN作成
  {
    u8  posx;
    u8  posy;
    u8  sizex = 2*PERSON_NAME_SIZE; // 文字幅
    u8  sizey = 4;  // 文字高さ

    // 計算式はキャラ単位に上手く合わせるため
    // 名前用
    posx = (baseX/8) + PLATE_OFFSET_X;
    posy = (baseY/8) - 2;
    p_plate->bmpwin_name = GFL_BMPWIN_Create( CTVT_GAME_FRAME_S_PLATE,
        posx, posy, sizex, sizey, CTVT_GAME_PAL_BG_S_FONT, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_plate->bmpwin_name), 0x0 );

    // 得点用
    posx = (baseX/8) + PLATE_OFFSET_X_SCORE;
    posy = (baseY/8) - 2;
    sizex= 2*3;
    p_plate->bmpwin_score = GFL_BMPWIN_Create( CTVT_GAME_FRAME_S_PLATE,
        posx, posy, sizex, sizey, CTVT_GAME_PAL_BG_S_FONT, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_plate->bmpwin_score), 0x0 );
  }

  // 文字描画
  p_plate->parent_fontHandle= COMM_TVT_GetFontHandle( work );
  p_plate->parent_msgHandle = COMM_TVT_GetMegHandle( work );
  p_plate->parent_printQue  = COMM_TVT_GetPrintQue( work );

  p_plate->str_tmp   = GFL_STR_CreateBuffer( (5*2+EOM_SIZE), gameWork->heapId );
  p_plate->str_expand= GFL_STR_CreateBuffer( (5*2+EOM_SIZE), gameWork->heapId );
  p_plate->str_name  = GFL_STR_CreateBuffer( 32, gameWork->heapId );

#ifdef PM_DEBUG
  p_plate->debug_blank = FALSE;
#endif
  return p_plate;
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレート作成 空
 *
 *	@param	CTVT_GAME_WORK *p_wk  ワーク
 *	@param	u8 netID              NetID
 *
 *	@return 作成したプレート
 */
//-----------------------------------------------------------------------------
static GAME_PLATE_WORK* GAME_PLATE_CreatePlate_Blank( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork, const u8 netID )
{
  GAME_PLATE_WORK *p_plate;
  p_plate = GFL_HEAP_AllocClearMemory( gameWork->heapId, sizeof(GAME_PLATE_WORK) );
  p_plate->heapID = gameWork->heapId;
  p_plate->rank   = 0;
  p_plate->netID  = netID;
  p_plate->is_update_name = FALSE;
  p_plate->is_update_score = FALSE;
  p_plate->is_blank = TRUE;

#ifdef PM_DEBUG
  p_plate->debug_blank = TRUE;
#endif

  return p_plate;
}
//----------------------------------------------------------------------------
/**
 *	@brief  プレート削除
 *
 *	@param	GAME_PLATE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void GAME_PLATE_DeletePlate( GAME_PLATE_WORK *p_wk )
{
  if( p_wk->is_blank == FALSE )
  {
    u8 i;
    // BMPWIN破棄
    GFL_BMPWIN_Delete( p_wk->bmpwin_name );
    GFL_BMPWIN_Delete( p_wk->bmpwin_score );

    // 文字バッファ削除
    GFL_STR_DeleteBuffer( p_wk->str_expand );
    GFL_STR_DeleteBuffer( p_wk->str_tmp );
    GFL_STR_DeleteBuffer( p_wk->str_name );

    // セル破棄
    GRAPHIC_OBJ_Remove( p_wk->clwkPlate );
    GRAPHIC_OBJ_Remove( p_wk->clwkP );
    for( i=0; i<3; i++ )
    {
      GRAPHIC_OBJ_Remove( p_wk->clwkPoint[i] );
    }
  }

  if( p_wk != NULL )
  {
    GFL_HEAP_FreeMemory( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレート更新
 *
 *	@param	GAME_PLATE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void GAME_PLATE_UpdatePlate( CTVT_GAME_WORK *p_wk )
{
  u8 i;
  u8 move_in = 0;
  u8 plate_num = 0;
  BOOL  move_end = TRUE;    // プレート移動が終了したかをゲームワークに渡す
  // プレートの移動管理
  if( p_wk->plate_move == TRUE )
  {
    for( i=0; i<PLATE_NUM; i++ )
    {
      if( p_wk->p_plate[i]->sub_seq == PLATE_SEQ_MOVE_IN_WAIT )
      {
        move_in++;
      }
      if( p_wk->p_plate[i]->is_blank == FALSE )
      {
        plate_num++;
      }

      // まだ移動中か
      if( p_wk->p_plate[i]->is_move == TRUE )
      {
        move_end = FALSE;
      }
    }
    // ifで判定しないと、1秒間に何回もSetMovePlateが呼ばれる
    if( move_end == TRUE )
    {
      p_wk->plate_move = FALSE;
    }
    if( move_in == plate_num )
    {
      for( i=0; i<plate_num; i++ )
      {
        p_wk->p_plate[i]->sub_seq = PLATE_SEQ_MOVE_IN;
        p_wk->p_plate[i]->move_wait = (plate_num - p_wk->p_plate[i]->rank) * 40;   // フェードインの待ち時間
      }
    }
  }

  GFL_TCB_Main( p_wk->task_plate );
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレート表示
 *
 *	@param	GAME_PLATE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void GAME_PLATE_ActivePlate(COMM_TVT_WORK *work, GAME_PLATE_WORK *p_wk )
{
  if( p_wk->is_blank == FALSE )
  {
    // OBJ表示
    GFL_CLACT_WK_SetDrawEnable( p_wk->clwkPlate, TRUE );
    GFL_CLACT_WK_SetDrawEnable( p_wk->clwkP, TRUE );

    // 名前描画
#ifdef PM_DEBUG
    if( p_wk->debug_blank == FALSE )
#endif
    {
      GFL_FONT *fontHandle  = p_wk->parent_fontHandle;
      GFL_MSGDATA *msgHandle= p_wk->parent_msgHandle;
      PRINT_QUE *printQue   = p_wk->parent_printQue;
      u8        posx = 0;
      u8        posy = 8 + ((p_wk->netID+1)*2);

      {
        CTVT_COMM_WORK        *commWork = COMM_TVT_GetCommWork( work );
        CTVT_COMM_MEMBER_DATA *memData = CTVT_COMM_GetMemberData( work, commWork, p_wk->netID );
        MYSTATUS *myStatus = (MYSTATUS*)memData->myStatusBuff;
        MyStatus_CopyNameString( myStatus, p_wk->str_name );

        PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp(p_wk->bmpwin_name),
            posx, posy, p_wk->str_name, fontHandle, CTVT_FONT_COLOR_BLACK );
        GFL_BMPWIN_TransVramCharacter( p_wk->bmpwin_name );
        GFL_BMPWIN_MakeScreen( p_wk->bmpwin_name );
        p_wk->is_update_name = TRUE;
      }

      // 得点
      {
        WORDSET *wordSet = WORDSET_Create(p_wk->heapID);
        GFL_MSG_GetString( msgHandle, COMM_TVT_GAME_23, p_wk->str_tmp );

        WORDSET_RegisterNumber( wordSet, 0, p_wk->point, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
        WORDSET_ExpandStr( wordSet, p_wk->str_expand, p_wk->str_tmp );


        PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp(p_wk->bmpwin_score),
            posx, posy, p_wk->str_expand, fontHandle, CTVT_FONT_COLOR_BLACK );
        WORDSET_Delete( wordSet );
        GFL_BMPWIN_TransVramCharacter( p_wk->bmpwin_score );
        GFL_BMPWIN_MakeScreen( p_wk->bmpwin_score );

        p_wk->is_update_score = TRUE;
      }
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  プレート  得点設定
 *
 *	@param	CTVT_GAME_WORK *p_wk ワーク
 *	@param	u8 netID        どのプレートか
 *	@param	u8 point        得点
 */
//-----------------------------------------------------------------------------
static void GAME_PLATE_SetPoint( CTVT_GAME_WORK *gameWork, const u8 netID, const u8 point )
{
  GAME_PLATE_WORK *p_wk = NULL;
  u8  i;

  // NetID を探す
  for( i=0; i<PLATE_NUM; i++ )
  {
    if( gameWork->p_plate[i]->netID == netID )
    {
      p_wk = gameWork->p_plate[i];
      break;
    }
  }
  GF_ASSERT( p_wk );

  p_wk->point = point;

  if( p_wk->is_blank == FALSE )
  {
    GFL_FONT *fontHandle  = p_wk->parent_fontHandle;
    GFL_MSGDATA *msgHandle= p_wk->parent_msgHandle;
    PRINT_QUE *printQue   = p_wk->parent_printQue;
    u8        posx = 0;
    u8        posy = 8 + ((p_wk->rank)*2);

    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->bmpwin_score), 0x0 );
    // 得点
    {
      WORDSET *wordSet = WORDSET_Create(p_wk->heapID);
      GFL_MSG_GetString( msgHandle, COMM_TVT_GAME_23, p_wk->str_tmp );

      WORDSET_RegisterNumber( wordSet, 0, point, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( wordSet, p_wk->str_expand, p_wk->str_tmp );

      PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp(p_wk->bmpwin_score),
          posx, posy, p_wk->str_expand, fontHandle, CTVT_FONT_COLOR_BLACK );
      WORDSET_Delete( wordSet );

      p_wk->is_update_score = TRUE;
    }

    ICHI_Printf("NetId %d %d 得点更新 %d \n", p_wk->netID, netID, point );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレート  移動開始
 *
 *	@param	CTVT_GAME_WORK *gameWork
 *	@param	is_vanish     　画面外に移動したらそのまま消えるか
 */
//-----------------------------------------------------------------------------
static void GAME_PLATE_SetMovePlate( CTVT_GAME_WORK *gameWork, BOOL is_vanish )
{
  u8 i, j;
  
  u8  rank[4] = {1,2,3,4};
  u8  point[4] = {0,};
  u8  plate_num = 0;

  // 得点保存
  for( i=0; i<PLATE_NUM; i++ )
  {
    if( gameWork->p_plate[i]->is_blank == FALSE )
    {
      point[i] = gameWork->p_plate[i]->point;
      plate_num++;
    }
  }
  // 新しいランクを設定
  for( i=0; i<plate_num-1; i++ )
  {
    for( j=i; j<plate_num; j++ )
    {
      u8 tmp_rank;
      u8 tmp_point;

      if( point[j] > point[i] )
      {
        tmp_point = point[i];
        point[i] = point[j];
        point[j] = tmp_point;

        tmp_rank = rank[i];
        rank[i] = rank[j];
        rank[j] = tmp_rank;
      }
    }
  }

  // 移動するときの設定
  for( i=0; i<plate_num; i++ )
  {
    GAME_PLATE_WORK *p_wk = gameWork->p_plate[i];
    
    if( p_wk->is_blank == FALSE )
    {
      // 最初の移動時はランク順になっていないので、下から移動
      if( gameWork->timeLimit == 40 )
      {
        p_wk->move_wait = (plate_num-1 - i) * 40;  // Xフレームごとに移動
      }
      else
      {
        p_wk->move_wait = (plate_num - p_wk->rank) * 40;  // Xフレームごとに移動
      }
      p_wk->is_move   = TRUE;
      p_wk->move_value= 0;
      if( is_vanish == FALSE )
      {
        p_wk->sub_seq   = PLATE_SEQ_MOVE_OUT;
      }
      else
      {
        p_wk->sub_seq   = PLATE_SEQ_MOVE_OUT_VANISH;
      }
      // 順位は最後に入れる
      p_wk->rank      = rank[i];

      ICHI_Printf("%d rank %d wait %d \n", i, p_wk->rank, p_wk->move_wait);
    }
  }
}

static BOOL GAME_PLATE_MovePlate( GAME_PLATE_WORK *p_wk, const int value )
{
  // 

  // 動かす
  GFL_CLACTPOS pos;
  s16 tmpx;
  GFL_CLACT_WK_GetPos( p_wk->clwkPlate, &pos, CLSYS_DEFREND_SUB );
  pos.x += value*8;
  GFL_CLACT_WK_SetPos( p_wk->clwkPlate, &pos, CLSYS_DEFREND_SUB ); 
  tmpx = pos.x;

  pos.x = tmpx + PLATE_OFFSET_X_P;
  GFL_CLACT_WK_SetPos( p_wk->clwkP, &pos, CLSYS_DEFREND_SUB ); 


#ifdef PM_DEBUG
  if( p_wk->debug_blank == FALSE )
#endif
  {
    {
      GFL_BMPWIN_ClearScreen( p_wk->bmpwin_name );
      GFL_BMPWIN_SetPosX( p_wk->bmpwin_name, (tmpx/8) + PLATE_OFFSET_X );
      //GFL_BMPWIN_TransVramCharacter( p_wk->bmpwin_name );
      GFL_BMPWIN_MakeScreen( p_wk->bmpwin_name );
    }
    {
      GFL_BMPWIN_ClearScreen( p_wk->bmpwin_score );
      GFL_BMPWIN_SetPosX( p_wk->bmpwin_score, (tmpx/8) + PLATE_OFFSET_X_SCORE );
      //GFL_BMPWIN_TransVramCharacter( p_wk->bmpwin_score );
      GFL_BMPWIN_MakeScreen( p_wk->bmpwin_score );
    }
    GFL_BG_LoadScreenV_Req( CTVT_GAME_FRAME_S_PLATE );
  }


  // 移動終了判定
  if( p_wk->sub_seq == PLATE_SEQ_MOVE_OUT
      || p_wk->sub_seq == PLATE_SEQ_MOVE_OUT_VANISH )
  {
    if( tmpx < -384 )
    {
      return TRUE;
    }
  }
  else if( p_wk->sub_seq == PLATE_SEQ_MOVE_IN )
  {
    if( tmpx <= PLATE_POS[ p_wk->rank-1 ][0] )
    {
      // 元の位置に戻った
      return TRUE;
    }
  }

  return FALSE;
}
static void GAME_PLATE_ClearPlate( GAME_PLATE_WORK *p_wk )
{
  // 
  GFL_CLACTPOS pos;
  s16   tmpx;
  pos.x = 384;
  pos.y = PLATE_POS[p_wk->rank-1][1];
  GFL_CLACT_WK_SetPos( p_wk->clwkPlate, &pos, CLSYS_DEFREND_SUB ); 
  tmpx = pos.x;
  pos.x = tmpx + PLATE_OFFSET_X_P;
  GFL_CLACT_WK_SetPos( p_wk->clwkP, &pos, CLSYS_DEFREND_SUB ); 

#ifdef PM_DEBUG
  if( p_wk->debug_blank == FALSE )
#endif
  {
    {
      GFL_BMPWIN_SetPosX( p_wk->bmpwin_name, (tmpx/8) + PLATE_OFFSET_X );
      GFL_BMPWIN_SetPosY( p_wk->bmpwin_name, (pos.y/8) - 2 );
    }
    {
      GFL_BMPWIN_SetPosX( p_wk->bmpwin_score, (tmpx/8) + PLATE_OFFSET_X_SCORE );
      GFL_BMPWIN_SetPosY( p_wk->bmpwin_score, (pos.y/8) - 2 );
    }
  }

  // 名前とスコアを描画し直す->位置修正のため
#ifdef PM_DEBUG
  if( p_wk->debug_blank == FALSE )
#endif
  {
    GFL_FONT *fontHandle  = p_wk->parent_fontHandle;
    GFL_MSGDATA *msgHandle= p_wk->parent_msgHandle;
    PRINT_QUE *printQue   = p_wk->parent_printQue;
    u8        posx = 0;
    u8        posy = 8 + ((p_wk->rank)*2);

    {
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->bmpwin_name), 0x0 );
      PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp(p_wk->bmpwin_name),
          posx, posy, p_wk->str_name, fontHandle, CTVT_FONT_COLOR_BLACK );
      GFL_BMPWIN_TransVramCharacter( p_wk->bmpwin_name );
      GFL_BMPWIN_MakeScreen( p_wk->bmpwin_name );
      p_wk->is_update_name = TRUE;
    }

    // 得点
    {
      WORDSET *wordSet = WORDSET_Create(p_wk->heapID);
      GFL_MSG_GetString( msgHandle, COMM_TVT_GAME_23, p_wk->str_tmp );

      WORDSET_RegisterNumber( wordSet, 0, p_wk->point, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( wordSet, p_wk->str_expand, p_wk->str_tmp );


      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->bmpwin_score), 0x0 );
      PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp(p_wk->bmpwin_score),
          posx, posy, p_wk->str_expand, fontHandle, CTVT_FONT_COLOR_BLACK );
      WORDSET_Delete( wordSet );
      GFL_BMPWIN_TransVramCharacter( p_wk->bmpwin_score );
      GFL_BMPWIN_MakeScreen( p_wk->bmpwin_score );

      p_wk->is_update_score = TRUE;
    }
  }
  GFL_BG_LoadScreenV_Req( CTVT_GAME_FRAME_S_PLATE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレートタスク関数
 */
//-----------------------------------------------------------------------------
static void TaskFuncPlate( GFL_TCB* p_tcb, void* p_work )
{
  GAME_PLATE_WORK *p_wk = (GAME_PLATE_WORK*)p_work;

  // 文字更新
  if( p_wk->is_update_name == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( p_wk->parent_printQue, GFL_BMPWIN_GetBmp( p_wk->bmpwin_name )) == FALSE )
    {
      p_wk->is_update_name = FALSE;
      GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->bmpwin_name );
    }
  }
  if( p_wk->is_update_score == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( p_wk->parent_printQue, GFL_BMPWIN_GetBmp( p_wk->bmpwin_score )) == FALSE )
    {
      p_wk->is_update_score = FALSE;
      GFL_BMPWIN_MakeTransWindow( p_wk->bmpwin_score );
    }
  }

  // 移動
  if( p_wk->is_move == TRUE )
  {
    // 移動待ち
    if( p_wk->move_wait > 0 )
    {
      p_wk->move_wait--;
    }

    if( p_wk->move_wait == 0 )
    {
      switch(p_wk->sub_seq){
      case PLATE_SEQ_MOVE_OUT:
        if( GAME_PLATE_MovePlate( p_wk, -1 ) == TRUE )
        {
          p_wk->sub_seq = PLATE_SEQ_MOVE_IN_WAIT;

          GAME_PLATE_ClearPlate( p_wk );
        }
        break;
      case PLATE_SEQ_MOVE_OUT_VANISH:
        if( GAME_PLATE_MovePlate( p_wk, -1 ) == TRUE )
        {
          p_wk->is_move = FALSE;
        }
        break;
      case PLATE_SEQ_MOVE_IN_WAIT:
        break;
      case PLATE_SEQ_MOVE_IN:
        if( GAME_PLATE_MovePlate( p_wk, -1 ) == TRUE )
        {
          p_wk->sub_seq = 0;
          p_wk->is_move = FALSE;
          p_wk->is_swing = FALSE;
        }
        break;
      }
    }
  }
  // 自分だけ定期的に揺れる
  else if( p_wk->is_mine == TRUE )
  {
    // 一定期間
    if( p_wk->is_swing == FALSE )
    {
      p_wk->swing_wait++;
      if( p_wk->swing_wait >= PLATE_SWING_TIME )
      {
        p_wk->is_swing = TRUE;
        p_wk->swing_wait = 0;
      }
    }

    // 実際に揺らす
    if( p_wk->is_swing == TRUE )
    {
      if( p_wk->swing_cnt > 14 )
      {
        p_wk->swing_cnt = 0;
        p_wk->is_swing = FALSE;
      }
      else
      {
        // 右左左右になる
        switch( p_wk->swing_cnt ){
        case 0:
        case 6:
        case 8:
        case 14:
          GAME_PLATE_MovePlate( p_wk, -1 );
          break;
        case 2:
        case 4:
        case 10:
        case 12:
          GAME_PLATE_MovePlate( p_wk, 1 );
          break;
        }

        p_wk->swing_cnt += 1;
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレート変更
 *
 *	@param	GAME_PLATE_WORK *p_wk 
 */
//-----------------------------------------------------------------------------
static void GAME_PLATE_ChangePlate( COMM_TVT_WORK *work, GAME_PLATE_WORK *p_wk )
{
  u8 i;
  CTVT_GAME_WORK  *gameWork = COMM_TVT_GetGameWork( work );

  if( p_wk->is_blank == FALSE )
  {
    const u8 baseX = PLATE_S_POS[p_wk->netID][0];
    const u8 baseY = PLATE_S_POS[p_wk->netID][1];

    // 上画面から下画面表示に変更
    GRAPHIC_OBJ_Remove( p_wk->clwkPlate );
    GRAPHIC_OBJ_Remove( p_wk->clwkP );

    // セルを変更
    {
      GFL_CLWK_DATA cellInitData;
      cellInitData.pos_x = baseX;
      cellInitData.pos_y = baseY;
      cellInitData.anmseq = CGOAM_PLATE_S;
      cellInitData.softpri= 3;
      cellInitData.bgpri  = 1;
      p_wk->clwkPlate = GFL_CLACT_WK_Create( gameWork->cellUnit,
          gameWork->cellResIdx[CGOR_LOWER_PLT],
          gameWork->cellResIdx[CGOR_LOWER_NCG],
          gameWork->cellResIdx[CGOR_LOWER_ANM],
          &cellInitData ,CLSYS_DEFREND_MAIN, gameWork->heapId );
      GFL_CLACT_WK_SetDrawEnable( p_wk->clwkPlate, TRUE );

      // 番号順でパレットをずらす 色変え
      GFL_CLACT_WK_SetPlttOffs( p_wk->clwkPlate, p_wk->netID, CLWK_PLTTOFFS_MODE_PLTT_TOP );

      // ポイントのP
      cellInitData.pos_x = baseX + PLATE_S_OFFSET_X_P;
      cellInitData.pos_y = baseY + 8;
      cellInitData.anmseq = CGOAM_P;
      cellInitData.softpri= 1;
      cellInitData.bgpri  = 1;
      p_wk->clwkP = GFL_CLACT_WK_Create( gameWork->cellUnit,
          gameWork->cellResIdx[CGOR_LOWER_PLT],
          gameWork->cellResIdx[CGOR_LOWER_NCG],
          gameWork->cellResIdx[CGOR_LOWER_ANM],
          &cellInitData ,CLSYS_DEFREND_MAIN, gameWork->heapId );
      GFL_CLACT_WK_SetDrawEnable( p_wk->clwkP, TRUE );
    }


    // 点数はセル作成
    for( i=0; i<3; i++ )
    {
      GFL_CLWK_DATA cellInitData;
      cellInitData.pos_x = baseX + (i*9);
      cellInitData.pos_y = baseY + 8;
      cellInitData.anmseq = CGOAM_NUM_DRUM;
      cellInitData.softpri= 0;
      cellInitData.bgpri  = 1;
      p_wk->clwkPoint[i] = GFL_CLACT_WK_Create( gameWork->cellUnit,
          gameWork->cellResIdx[CGOR_LOWER_PLT],
          gameWork->cellResIdx[CGOR_LOWER_NCG],
          gameWork->cellResIdx[CGOR_LOWER_ANM],
          &cellInitData ,CLSYS_DEFREND_MAIN, gameWork->heapId );
      GFL_CLACT_WK_SetAutoAnmFlag( p_wk->clwkPoint[i], TRUE );
      GFL_CLACT_WK_SetAnmIndex( p_wk->clwkPoint[i], i*3 );
      ICHI_Printf("anmIndex %d \n",GFL_CLACT_WK_GetAnmIndex(p_wk->clwkPoint[i]) );
    }
  }


  // 名前
#ifdef PM_DEBUG
  if( p_wk->debug_blank == FALSE )
#endif
  {
    GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
    GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );

    CTVT_COMM_WORK        *commWork = COMM_TVT_GetCommWork( work );
    CTVT_COMM_MEMBER_DATA *memData = CTVT_COMM_GetMemberData( work, commWork, p_wk->netID );
    MYSTATUS *myStatus = (MYSTATUS*)memData->myStatusBuff;
    STRBUF *str = MyStatus_CreateNameString( myStatus, gameWork->heapId );
    const u8 nameLen = PRINTSYS_GetStrWidth( str, fontHandle, 0 );

    // BMPWINは作り直す
    GFL_BMPWIN_ClearTransWindow( p_wk->bmpwin_name );
    GFL_BMPWIN_Delete( p_wk->bmpwin_name );
    GFL_BG_SetVisible( CTVT_GAME_FRAME_S_PLATE, VISIBLE_OFF ); 
    p_wk->bmpwin_name = GFL_BMPWIN_Create( CTVT_GAME_FRAME_M_PLATE,
        8*p_wk->netID, 20, 2*PERSON_NAME_SIZE, 4, CTVT_GAME_PAL_BG_M_FONT, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->bmpwin_name), 0x0 );

    PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp(p_wk->bmpwin_name),
        32 - (nameLen/2), 0, str, fontHandle, CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    GFL_BMPWIN_TransVramCharacter( p_wk->bmpwin_name );
    GFL_BMPWIN_MakeScreen( p_wk->bmpwin_name );
    p_wk->is_update_name = TRUE;
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  結果発表のプレートタスク関数
 *
 *	@param	GFL_TCB* p_tcb
 *	@param	p_work 
 */
//-----------------------------------------------------------------------------
static void TaskFuncScorePlate( GFL_TCB* p_tcb, void* p_work )
{
  GAME_PLATE_WORK *p_wk = (GAME_PLATE_WORK*)p_work;

  // 空プレートなら何もしない
  if( p_wk->is_blank == TRUE )
  {
    return;
  }

  // 文字更新
  if( p_wk->is_update_name == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( p_wk->parent_printQue, GFL_BMPWIN_GetBmp( p_wk->bmpwin_name )) == FALSE )
    {
      p_wk->is_update_name = FALSE;
      GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->bmpwin_name );
    }
  }
  if( p_wk->is_update_score == TRUE )
  {
    if( PRINTSYS_QUE_IsExistTarget( p_wk->parent_printQue, GFL_BMPWIN_GetBmp( p_wk->bmpwin_score )) == FALSE )
    {
      p_wk->is_update_score = FALSE;
      GFL_BMPWIN_MakeTransWindow( p_wk->bmpwin_score );
    }
  }


  switch( p_wk->sub_seq ){
  case PLATE_SEQ_DRUMROLL:
    // アニメ再生中 何もしない
    break;
  case PLATE_SEQ_DRUMROLL_STOP:
    // 得点を計算
    // 百の位
    GFL_CLACT_WK_SetAnmSeq( p_wk->clwkPoint[0], CGOAM_NUM_0 + (p_wk->point/100));
    // 十の位
    GFL_CLACT_WK_SetAnmSeq( p_wk->clwkPoint[1], CGOAM_NUM_0 + (p_wk->point/10));
    // 一の位
    GFL_CLACT_WK_SetAnmSeq( p_wk->clwkPoint[2], CGOAM_NUM_0 + (p_wk->point%10));
    p_wk->sub_seq = PLATE_SEQ_WAIT;
    break;
  case PLATE_SEQ_WAIT:
    // まち
    break;
  }
}


#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

static void DEBUG_MAIN_TIME_AVERAGE_StartFunc( u8 id )
{
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start[id] = OS_GetTick(); 
}

static void DEBUG_MAIN_TIME_AVERAGE_EndFunc( u8 id )
{
  OSTick now_time;
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_End[id] = OS_GetTick(); 

  now_time = OS_TicksToMicroSeconds(DEBUG_DEBUG_MAIN_TIME_AVERAGE_End[id] - DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start[id]);

  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[id] += now_time;
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id] ++;
  if( now_time > 16000 ){
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount[id] ++;
#if DEBUG_MAIN_TIME_AVERAGE
    OS_TPrintf( " id %d : ", id );
    OS_TPrintf( " over tick %ld", now_time );
    OS_TPrintf( " ave tick %d\n", (DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[id] / DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id]) );
#endif
  }
  if( DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[id] < now_time ){
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[id] = now_time;
  }
  if( DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id] >= DEBUG_DEBUG_MAIN_TIME_AVERAGE_AVE_COUNT_TIME ){

#if DEBUG_MAIN_TIME_AVERAGE
    OS_TPrintf( " id %d : ", id );
    OS_TPrintf( " ave tick %d", 
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[id] / DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id] );
    OS_TPrintf( " Over 120::%d",
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount[id] );
    OS_TPrintf( " Max %d\n",
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[id] );
#endif

    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[id] = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id] = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount[id] = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[id]       = 0;
  }

}

#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
