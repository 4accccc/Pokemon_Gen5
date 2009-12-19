//============================================================================================
/**
 * @file  soundTest.c
 * @brief
 * @author
 * @date
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"
#include "font/font.naix"

#include "msg/msg_snd_test_str.h"
#include "msg/msg_snd_test_name.h"
#include "msg/msg_monsname.h"

#include "system/main.h"

#include "poke_tool/monsno_def.h"
#include "print/printsys.h"
#include "print/str_tool.h"

#include "sound/pm_sndsys.h"
#include "sound/pm_wb_voice.h"

#include "sound/snd_viewer.h"

#include "arc/soundtest.naix"

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief
 *
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief データＩＮＤＥＸ
 */
//------------------------------------------------------------------
enum {
  NAMEIDX_BGM = 0,
  NAMEIDX_VOICE,
  NAMEIDX_SE,

  NAMEIDX_MAX,
};

enum {
  NOIDX_HEAPFREESIZ = 0,

  NOIDX_BGMNO,
  NOIDX_VOICENO,
  NOIDX_VOICEPAN,
  NOIDX_VOICESPEED,
  NOIDX_VOICECHORUSVOL,
  NOIDX_VOICECHORUSSPEED,
  NOIDX_SENO,
  NOIDX_SEPAN,
  NOIDX_SEPITCH,

  NOIDX_MAX,
};

//------------------------------------------------------------------
/**
 * @brief 文字列定数
 */
//------------------------------------------------------------------
#define NAME_BUF_SIZE (48*2)
#define NO_BUF_SIZE   (8*2)

//------------------------------------------------------------------
/**
 * @brief ディスプレイ環境データ
 */
//------------------------------------------------------------------
///ＶＲＡＭバンク設定構造体
static const GFL_DISP_VRAM dispVram = {
  GX_VRAM_BG_128_A,       //メイン2DエンジンのBGに割り当て
  GX_VRAM_BGEXTPLTT_NONE,     //メイン2DエンジンのBG拡張パレットに割り当て
  GX_VRAM_SUB_BG_32_H,      //サブ2DエンジンのBGに割り当て
  GX_VRAM_SUB_BGEXTPLTT_NONE,   //サブ2DエンジンのBG拡張パレットに割り当て
  GX_VRAM_OBJ_64_E,       //メイン2DエンジンのOBJに割り当て
  GX_VRAM_OBJEXTPLTT_NONE,    //メイン2DエンジンのOBJ拡張パレットにに割り当て
  GX_VRAM_SUB_OBJ_NONE,     //サブ2DエンジンのOBJに割り当て
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  //サブ2DエンジンのOBJ拡張パレットにに割り当て
  GX_VRAM_TEX_NONE,       //テクスチャイメージスロットに割り当て
  GX_VRAM_TEXPLTT_NONE,     //テクスチャパレットスロットに割り当て
  GX_OBJVRAMMODE_CHAR_1D_128K,  // メインOBJマッピングモード
  GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
};


//------------------------------------------------------------------
/**
 * @brief   ＢＧ設定
 */
//------------------------------------------------------------------
static const GFL_BG_SYS_HEADER bgsysHeader = {
  GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};

static const GFL_BG_BGCNT_HEADER BGcont = {
  0, 0, 0x800, 0,
  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_128x128,
  GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};
static const GFL_BG_BGCNT_HEADER Textcont = {
  0, 0, 0x800, 0,
  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x3800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_128x128,
  GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};
static const GFL_BG_BGCNT_HEADER Statuscont = {
  0, 0, 0x800, 0,
  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x3800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_128x128,
  GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};
#define BG_FRAME    (GFL_BG_FRAME3_M)
#define TEXT_FRAME    (GFL_BG_FRAME2_M)
#define STATUS_FRAME  (GFL_BG_FRAME0_S)

#define PLTT_1  (0x20)
#define CHAR_1  (0x20)
#define SCRN_1  (2)

#define BG_PLTTID (0) //0番～3本使用
#define TEXT_PLTTID (1) //0番～3本使用
#define BG_PLTTNUM  (3)
#define BG_CHARID (0) //0番～255キャラ使用
#define BG_CHARNUM  (32*8)
#define BG_SCRNNUM  (32*32)

//------------------------------------------------------------------
/**
 * @brief   ＢＧ描画データ
 */
//------------------------------------------------------------------
static const u8 bmpwinNameStDat[NAMEIDX_MAX][4] = { //px, py, sx, sy
  { 0x01, 0x03, 10, 2 },  //NAMEIDX_BGM
  { 0x0b, 0x03, 10, 2 },  //NAMEIDX_VOICE
  { 0x15, 0x03, 10, 2 },  //NAMEIDX_SE
};

static const u8 bmpwinNoStDat[NOIDX_MAX][4] = { //px, py, sx, sy
  { 0x03, 0x13, 6, 1 }, //NOIDX_HEAPFREESIZ

  { 0x04, 0x06, 4, 1 }, //NOIDX_BGMNO
  { 0x0e, 0x06, 4, 1 }, //NOIDX_VOICENO
  { 0x0e, 0x0c, 4, 1 }, //NOIDX_VOICEPAN
  { 0x0e, 0x0f, 4, 1 }, //NOIDX_VOICESPEED
  { 0x0e, 0x12, 4, 1 }, //NOIDX_VOICECHORUSVOL
  { 0x0e, 0x15, 4, 1 }, //NOIDX_VOICECHORUSSPEED
  { 0x18, 0x06, 4, 1 }, //NOIDX_SENO
  { 0x18, 0x0c, 4, 1 }, //NOIDX_SEPAN
  { 0x18, 0x0f, 4, 1 }, //NOIDX_SEPITCH
};

//------------------------------------------------------------------
/**
 * @brief ソフトウエアキーボード設定
 */
//------------------------------------------------------------------
static const GFL_SKB_SETUP skbData= {
  GFL_SKB_STRLEN_MAX, GFL_SKB_STRTYPE_SJIS,
  GFL_SKB_MODE_HIRAGANA, TRUE, PAD_BUTTON_START,
  GFL_DISPUT_BGID_M1, GFL_DISPUT_PALID_14, GFL_DISPUT_PALID_15,
};

//------------------------------------------------------------------
/**
 * @brief サウンドステータス設定
 */
//------------------------------------------------------------------
static const GFL_SNDVIEWER_SETUP sndViewerData= {
  0,
  GFL_DISPUT_BGID_S0, GFL_DISPUT_PALID_15,
  PMSND_GetNowSndHandlePointer,
  PMSND_GetBGMsoundNo,
  PMSND_GetBGMplayerNoIdx,
  PMSND_CheckOnReverb,
  GFL_SNDVIEWER_CONTROL_NONE,
};

//------------------------------------------------------------------
/**
 * @brief タッチパネル判定テーブル
 */
//------------------------------------------------------------------
enum {
  SNDTEST_TPEV_BGM_PLAY = 0,
  SNDTEST_TPEV_BGM_STOP,
  SNDTEST_TPEV_BGM_PAUSE,
  SNDTEST_TPEV_BGM_PLAYFADEIN,
  SNDTEST_TPEV_BGM_FADEOUTSTOP,
  SNDTEST_TPEV_BGM_FADEOUTPLAYFADEIN,
  SNDTEST_TPEV_BGM_PAUSEPUSH,
  SNDTEST_TPEV_BGM_POPPLAY,
  SNDTEST_TPEV_REVERB,

  SNDTEST_TPEV_VOICE_PLAY,
  SNDTEST_TPEV_VOICE_PLAYCHROUS,
  SNDTEST_TPEV_VOICE_PLAYREVERSE,
  SNDTEST_TPEV_VOICE_PLAYREVERSECHORUS,

  SNDTEST_TPEV_SE_PLAY,
  SNDTEST_TPEV_SE_STOP,

  SNDTEST_TPEV_EXIT,

  SNDTEST_TPEVTRG_MAX,
};

static const GFL_UI_TP_HITTBL eventTouchPanelTableTrg[SNDTEST_TPEVTRG_MAX + 1] = {
  { 0x08*8+0x04, 0x0a*8+0x02, 0x01*8+0x04, 0x03*8+0x02 }, //SNDTEST_TPEV_BGM_PLAY
  { 0x08*8+0x04, 0x0a*8+0x02, 0x03*8+0x07, 0x05*8+0x05 }, //SNDTEST_TPEV_BGM_STOP
  { 0x08*8+0x04, 0x0a*8+0x02, 0x06*8+0x02, 0x08*8+0x00 }, //SNDTEST_TPEV_BGM_PAUSE
  { 0x0b*8+0x04, 0x0d*8+0x02, 0x01*8+0x04, 0x03*8+0x02 }, //SNDTEST_TPEV_BGM_PLAYFADEIN
  { 0x0b*8+0x04, 0x0d*8+0x02, 0x03*8+0x07, 0x05*8+0x05 }, //SNDTEST_TPEV_BGM_FADEOUTSTOP
  { 0x0b*8+0x04, 0x0d*8+0x02, 0x06*8+0x02, 0x08*8+0x00 }, //SNDTEST_TPEV_BGM_FADEOUTPLAYFADEIN
  { 0x0f*8+0x04, 0x11*8+0x02, 0x02*8+0x02, 0x04*8+0x00 }, //SNDTEST_TPEV_BGM_PAUSEPUSH
  { 0x0f*8+0x04, 0x11*8+0x02, 0x04*8+0x05, 0x06*8+0x03 }, //SNDTEST_TPEV_BGM_POPPLAY
  { 0x15*8+0x01, 0x15*8+0x07, 0x07*8+0x00, 0x08*8+0x07 }, //SNDTEST_TPEV_REVERB

  { 0x08*8+0x04, 0x0a*8+0x02, 0x0b*8+0x04, 0x0d*8+0x02 }, //SNDTEST_TPEV_VOICE_PLAY
  { 0x08*8+0x04, 0x0a*8+0x02, 0x0d*8+0x07, 0x0f*8+0x05 }, //SNDTEST_TPEV_VOICE_PLAYCHROUS
  { 0x08*8+0x04, 0x0a*8+0x02, 0x10*8+0x02, 0x12*8+0x00 }, //SNDTEST_TPEV_VOICE_PLAYREVERSE
  { 0x08*8+0x04, 0x0a*8+0x02, 0x12*8+0x05, 0x14*8+0x03 }, //SNDTEST_TPEV_VOICE_PLAYREVERSECHORUS

  { 0x08*8+0x04, 0x0a*8+0x02, 0x15*8+0x04, 0x17*8+0x02 }, //SNDTEST_TPEV_SE_PLAY
  { 0x08*8+0x04, 0x0a*8+0x02, 0x17*8+0x07, 0x19*8+0x05 }, //SNDTEST_TPEV_SE_STOP

  { 0x16*8, 0x16*8+15, 0x1d*8, 0x1d*8+23 },       //SNDTEST_TPEV_EXIT

  {GFL_UI_TP_HIT_END,0,0,0},      //終了データ
};

enum {
  SNDTEST_TPEV_BGM_NUMDOWN1 = 0,
  SNDTEST_TPEV_BGM_NUMDOWN10,
  SNDTEST_TPEV_BGM_NUMUP1,
  SNDTEST_TPEV_BGM_NUMUP10,

  SNDTEST_TPEV_VOICE_NUMDOWN1,
  SNDTEST_TPEV_VOICE_NUMDOWN10,
  SNDTEST_TPEV_VOICE_NUMUP1,
  SNDTEST_TPEV_VOICE_NUMUP10,
  SNDTEST_TPEV_VOICE_PANDOWN1,
  SNDTEST_TPEV_VOICE_PANDOWN10,
  SNDTEST_TPEV_VOICE_PANUP1,
  SNDTEST_TPEV_VOICE_PANUP10,
  SNDTEST_TPEV_VOICE_SPEEDDOWN1,
  SNDTEST_TPEV_VOICE_SPEEDDOWN10,
  SNDTEST_TPEV_VOICE_SPEEDUP1,
  SNDTEST_TPEV_VOICE_SPEEDUP10,
  SNDTEST_TPEV_VOICE_CHORUSVOLDOWN1,
  SNDTEST_TPEV_VOICE_CHORUSVOLDOWN10,
  SNDTEST_TPEV_VOICE_CHORUSVOLUP1,
  SNDTEST_TPEV_VOICE_CHORUSVOLUP10,
  SNDTEST_TPEV_VOICE_CHORUSSPEEDDOWN1,
  SNDTEST_TPEV_VOICE_CHORUSSPEEDDOWN10,
  SNDTEST_TPEV_VOICE_CHORUSSPEEDUP1,
  SNDTEST_TPEV_VOICE_CHORUSSPEEDUP10,

  SNDTEST_TPEV_SE_NUMDOWN1,
  SNDTEST_TPEV_SE_NUMDOWN10,
  SNDTEST_TPEV_SE_NUMUP1,
  SNDTEST_TPEV_SE_NUMUP10,
  SNDTEST_TPEV_SE_PANDOWN1,
  SNDTEST_TPEV_SE_PANDOWN10,
  SNDTEST_TPEV_SE_PANUP1,
  SNDTEST_TPEV_SE_PANUP10,
  SNDTEST_TPEV_SE_PITCHDOWN1,
  SNDTEST_TPEV_SE_PITCHDOWN10,
  SNDTEST_TPEV_SE_PITCHUP1,
  SNDTEST_TPEV_SE_PITCHUP10,

  SNDTEST_TPEVCONT_MAX,
};

static const GFL_UI_TP_HITTBL eventTouchPanelTableCont[SNDTEST_TPEVCONT_MAX + 1] = {
  { 0x05*8+0x01, 0x06*8+0x02, 0x01*8+0x02, 0x03*8+0x06 }, //SNDTEST_TPEV_BGM_NUMDOWN1
  { 0x06*8+0x04, 0x07*8+0x05, 0x01*8+0x02, 0x03*8+0x06 }, //SNDTEST_TPEV_BGM_NUMDOWN10
  { 0x05*8+0x01, 0x06*8+0x02, 0x08*8+0x01, 0x0a*8+0x05 }, //SNDTEST_TPEV_BGM_NUMUP1
  { 0x06*8+0x04, 0x07*8+0x05, 0x08*8+0x01, 0x0a*8+0x05 }, //SNDTEST_TPEV_BGM_NUMUP10

  { 0x05*8+0x01, 0x06*8+0x02, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_NUMDOWN1
  { 0x06*8+0x04, 0x07*8+0x05, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_NUMDOWN10
  { 0x05*8+0x01, 0x06*8+0x02, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_NUMUP1
  { 0x06*8+0x04, 0x07*8+0x05, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_NUMUP10
  { 0x0b*8+0x01, 0x0c*8+0x02, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_PANDOWN1
  { 0x0c*8+0x04, 0x0d*8+0x05, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_PANDOWN10
  { 0x0b*8+0x01, 0x0c*8+0x02, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_PANUP1
  { 0x0c*8+0x04, 0x0d*8+0x05, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_PANUP10
  { 0x0e*8+0x01, 0x0f*8+0x02, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_SPEEDDOWN1
  { 0x0f*8+0x04, 0x10*8+0x05, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_SPEEDDOWN10
  { 0x0e*8+0x01, 0x0f*8+0x02, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_SPEEDUP1
  { 0x0f*8+0x04, 0x10*8+0x05, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_SPEEDUP10
  { 0x11*8+0x01, 0x12*8+0x02, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_CHORUSVOLDOWN1
  { 0x12*8+0x04, 0x13*8+0x05, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_CHORUSVOLDOWN10
  { 0x11*8+0x01, 0x12*8+0x02, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_CHORUSVOLUP1
  { 0x12*8+0x04, 0x13*8+0x05, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_CHORUSVOLUP10
  { 0x14*8+0x01, 0x15*8+0x02, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_CHORUSSPEEDDOWN1
  { 0x15*8+0x04, 0x16*8+0x05, 0x0b*8+0x02, 0x0d*8+0x06 }, //SNDTEST_TPEV_VOICE_CHORUSSPEEDDOWN10
  { 0x14*8+0x01, 0x15*8+0x02, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_CHORUSSPEEDUP1
  { 0x15*8+0x04, 0x16*8+0x05, 0x12*8+0x01, 0x14*8+0x05 }, //SNDTEST_TPEV_VOICE_CHORUSSPEEDUP10

  { 0x05*8+0x01, 0x06*8+0x02, 0x15*8+0x02, 0x17*8+0x06 }, //SNDTEST_TPEV_SE_NUMDOWN1
  { 0x06*8+0x04, 0x07*8+0x05, 0x15*8+0x02, 0x17*8+0x06 }, //SNDTEST_TPEV_SE_NUMDOWN10
  { 0x05*8+0x01, 0x06*8+0x02, 0x1c*8+0x01, 0x1e*8+0x05 }, //SNDTEST_TPEV_SE_NUMUP1
  { 0x06*8+0x04, 0x07*8+0x05, 0x1c*8+0x01, 0x1e*8+0x05 }, //SNDTEST_TPEV_SE_NUMUP10
  { 0x0b*8+0x01, 0x0c*8+0x02, 0x15*8+0x02, 0x17*8+0x06 }, //SNDTEST_TPEV_SE_PANDOWN1
  { 0x0c*8+0x04, 0x0d*8+0x05, 0x15*8+0x02, 0x17*8+0x06 }, //SNDTEST_TPEV_SE_PANDOWN10
  { 0x0b*8+0x01, 0x0c*8+0x02, 0x1c*8+0x01, 0x1e*8+0x05 }, //SNDTEST_TPEV_SE_PANUP1
  { 0x0c*8+0x04, 0x0d*8+0x05, 0x1c*8+0x01, 0x1e*8+0x05 }, //SNDTEST_TPEV_SE_PANUP10
  { 0x0e*8+0x01, 0x0f*8+0x02, 0x15*8+0x02, 0x17*8+0x06 }, //SNDTEST_TPEV_SE_PITCHDOWN1
  { 0x0f*8+0x04, 0x10*8+0x05, 0x15*8+0x02, 0x17*8+0x06 }, //SNDTEST_TPEV_SE_PITCHDOWN10
  { 0x0e*8+0x01, 0x0f*8+0x02, 0x1c*8+0x01, 0x1e*8+0x05 }, //SNDTEST_TPEV_SE_PITCHUP1
  { 0x0f*8+0x04, 0x10*8+0x05, 0x1c*8+0x01, 0x1e*8+0x05 }, //SNDTEST_TPEV_SE_PITCHUP10

  {GFL_UI_TP_HIT_END,0,0,0},      //終了データ
};

static const u16 tpRepeatFrame[] = {
  16, 8, 4, 2, 1, 0,
};

//------------------------------------------------------------------
/**
 * @brief 初期値テーブル
 */
//------------------------------------------------------------------
static const int initNoData[NOIDX_MAX] = {
  0,  //NOIDX_HEAPFREESIZ

  PMSND_BGM_START,  //NOIDX_BGMNO
  PMVOICE_START,    //NOIDX_VOICENO
  64,         //NOIDX_VOICEPAN
  0,          //NOIDX_VOICESPEED
  0,          //NOIDX_VOICECHORUSVOL
  0,          //NOIDX_VOICECHORUSSPEED
  PMSND_SE_START,   //NOIDX_SENO
  0,          //NOIDX_SEPAN
  0,          //NOIDX_SEPITCH
};

#define NAMEMSG_STARTIDX_BGM  (msg_seq_bgm_dummy)
#define NAMEMSG_STARTIDX_SE   (msg_seq_se_dummy)
#define NAMEMSG_STARTIDX_VOICE  (msg_seq_pv)

//============================================================================================
/**
 *
 * @brief 構造体定義
 *
 */
//============================================================================================
typedef struct {
  HEAPID          heapID;
  int             seq;

  GFL_SNDVIEWER*  gflSndViewer;
  GFL_SKB*        gflSkb;
  BOOL            gflSkbSw;

  void*           skbStrBuf;

  GFL_TCB*        g2dVintr;

  PRINT_QUE*      printQue;
  GFL_FONT*       fontHandle;
  GFL_MSGDATA*    msgman;
  GFL_MSGDATA*    monsmsgman;

  GFL_BMPWIN*     bmpwinName[NAMEIDX_MAX];
  PRINT_UTIL      printUtilName[NAMEIDX_MAX];
  STRBUF*         setName[NAMEIDX_MAX];

  GFL_BMPWIN*     bmpwinNo[NOIDX_MAX];
  PRINT_UTIL      printUtilNo[NOIDX_MAX];
  int             setNo[NOIDX_MAX];

  STRBUF*         setNoBuffer;

  int             mode;
  BOOL            bgmPauseSw;
  BOOL            reverbFlag;

  u16             tpTrgRepeatFrame;
  u16             tpTrgRepeatCount;

  u16             trackBit;
  BOOL            voiceReserveFlag;
}SOUNDTEST_WORK;

enum {
  MODE_SOUND_SELECT = 0,
  MODE_SOUND_CONTROL,
};


//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief 呼び出し用PROC定義
 *
 *
 *
 *
 *
 */
//============================================================================================
static BOOL SoundTest(SOUNDTEST_WORK* sw);

//------------------------------------------------------------------
/**
 * @brief ワークの初期化＆破棄
 */
//------------------------------------------------------------------
static void SoundWorkInitialize(SOUNDTEST_WORK* sw)
{
  int i;

  sw->skbStrBuf = GFL_SKB_CreateSjisCodeBuffer(sw->heapID);
  sw->seq = 0;

  sw->bgmPauseSw = FALSE;
  sw->reverbFlag = FALSE;

  sw->mode = MODE_SOUND_SELECT;
  sw->tpTrgRepeatFrame = 0;
  sw->tpTrgRepeatCount = 0;

  for(i=0; i<NAMEIDX_MAX; i++)
    { sw->setName[i] = GFL_STR_CreateBuffer(NAME_BUF_SIZE, sw->heapID); }
  for( i=0; i<NOIDX_MAX; i++ ){ sw->setNo[i] = initNoData[i]; }
  sw->setNoBuffer = GFL_STR_CreateBuffer(NO_BUF_SIZE, sw->heapID);

  sw->msgman = GFL_MSG_Create
    (GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_snd_test_name_dat, sw->heapID);
  sw->monsmsgman = GFL_MSG_Create
    (GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monsname_dat, sw->heapID);

  sw->fontHandle = GFL_FONT_Create
    (ARCID_FONT, NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE ,sw->heapID);
  sw->printQue = PRINTSYS_QUE_Create(sw->heapID);

  sw->gflSndViewer = GFL_SNDVIEWER_Create( &sndViewerData, sw->heapID );

  sw->trackBit = 0;
  sw->voiceReserveFlag = FALSE;
}

static void SoundWorkFinalize(SOUNDTEST_WORK* sw)
{
  int i;

  GFL_SNDVIEWER_Delete( sw->gflSndViewer );

  PRINTSYS_QUE_Clear(sw->printQue);
  PRINTSYS_QUE_Delete(sw->printQue);
  GFL_FONT_Delete(sw->fontHandle);

  GFL_MSG_Delete(sw->monsmsgman);
  GFL_MSG_Delete(sw->msgman);

  GFL_STR_DeleteBuffer(sw->setNoBuffer);
  for(i=0; i<NAMEIDX_MAX; i++){ GFL_STR_DeleteBuffer(sw->setName[i]); }

  GFL_SKB_DeleteSjisCodeBuffer(sw->skbStrBuf);

  if(sw->voiceReserveFlag == TRUE){
    PMVOICE_PlayerHeapRelease();
  }
}

//------------------------------------------------------------------
/**
 * @brief proc関数
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT SoundTest2Proc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  SOUNDTEST_WORK* sw = NULL;

  sw = GFL_PROC_AllocWork(proc, sizeof(SOUNDTEST_WORK), GFL_HEAPID_APP);

  GFL_STD_MemClear(sw, sizeof(SOUNDTEST_WORK));
  sw->heapID = GFL_HEAPID_APP;

  SoundWorkInitialize(sw);

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT SoundTest2Proc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  SOUNDTEST_WORK* sw;
  sw = mywk;

  if(SoundTest(sw) == TRUE){
    return GFL_PROC_RES_CONTINUE;
  }
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT SoundTest2Proc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  SOUNDTEST_WORK* sw;
  sw = mywk;

  SoundWorkFinalize(sw);

  GFL_PROC_FreeWork(proc);

    return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief procテーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA SoundTest2ProcData = {
  SoundTest2Proc_Init,
  SoundTest2Proc_Main,
  SoundTest2Proc_End,
};





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief メイン
 *
 *
 *
 *
 *
 */
//============================================================================================
static void SetupSoundTestSys(SOUNDTEST_WORK* sw);
static void RemoveSoundTestSys(SOUNDTEST_WORK* sw);
static void MainSoundTestSys(SOUNDTEST_WORK* sw);

static BOOL checkTouchPanelEvent(SOUNDTEST_WORK* sw);
static BOOL checkTouchPanelEventTrg(SOUNDTEST_WORK* sw);
static BOOL checkTouchPanelEventCont(SOUNDTEST_WORK* sw);

static void bg_init(SOUNDTEST_WORK* sw);
static void bg_exit(SOUNDTEST_WORK* sw);

static void g2d_load(SOUNDTEST_WORK* sw);
static void g2d_draw(SOUNDTEST_WORK* sw);
static void g2d_unload(SOUNDTEST_WORK* sw);
static void g2d_vblank( GFL_TCB* tcb, void* work );

static void printName(SOUNDTEST_WORK* sw, int idx);
static void printNo(SOUNDTEST_WORK* sw, int idx, u32 numberSize );
static void numberInc(SOUNDTEST_WORK* sw, int idx, int max, int num );
static void numberDec(SOUNDTEST_WORK* sw, int idx, int min, int num );

static void setSelectName(SOUNDTEST_WORK* sw);
static void writeButton(SOUNDTEST_WORK* sw, u8 x, u8 y, BOOL flag );

#if 0//#ifdef PM_DEBUG
static u32  mcsControl(HEAPID heapID);
static void mcsControlEnd(void);
static void mcsControlReset(HEAPID heapID);
static void mcsControlSetTrackSt(HEAPID heapID, u32 trackSt);
#endif
//------------------------------------------------------------------
/**
 *
 * @brief コントロール切り替え
 *
 */
//------------------------------------------------------------------
static void checkControlChange(SOUNDTEST_WORK* sw)
{
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
    if( sw->mode == MODE_SOUND_SELECT ){
      GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
      sw->mode = MODE_SOUND_CONTROL;
    } else {
      GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_SUB);
      sw->mode = MODE_SOUND_SELECT;
    }
  }
}

//------------------------------------------------------------------
/**
 *
 * @brief フレームワーク
 *
 */
//------------------------------------------------------------------
static BOOL SoundTest(SOUNDTEST_WORK* sw)
{
  switch(sw->seq){
  case 0:
    SetupSoundTestSys(sw);
    sw->seq++;
    break;

  case 1:
#if 0//#ifdef PM_DEBUG
    {
      u32 result = mcsControl(sw->heapID);
      switch(result){
      case 1:
        // PC->MCS 接続完了
        mcsControlSetTrackSt(sw->heapID, sw->trackBit);
        mcsControlReset(sw->heapID);
        break;
      }
    }
#endif
#if 1
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      if(sw->voiceReserveFlag == FALSE){
        PMVOICE_PlayerHeapReserve(2, GFL_HEAPID_APP);
        sw->voiceReserveFlag = TRUE;
      } else {
        PMVOICE_PlayerHeapRelease();
        sw->voiceReserveFlag = FALSE;
      }
    }
#endif
    {
      //soundStatusコントロール設定
      u16 flag;
      if( sw->mode == MODE_SOUND_SELECT ){ flag = GFL_SNDVIEWER_CONTROL_NONE; }
      else { flag = GFL_SNDVIEWER_CONTROL_ENABLE; }

      GFL_SNDVIEWER_SetControl( sw->gflSndViewer, flag );
    }
    GFL_SNDVIEWER_Main(sw->gflSndViewer);

    {
      u16 trackBit = GFL_SNDVIEWER_GetTrackSt(sw->gflSndViewer);
      if(sw->trackBit != trackBit){
        sw->trackBit = trackBit;
#if 0//#ifdef PM_DEBUG
        mcsControlSetTrackSt(sw->heapID, sw->trackBit);
#endif
      }
    }

    MainSoundTestSys(sw);
    checkControlChange(sw);

    if( sw->mode == MODE_SOUND_SELECT ){
      checkTouchPanelEvent(sw);
    }
    break;

  case 2:
#if 0//#ifdef PM_DEBUG
    mcsControlEnd();
#endif
    RemoveSoundTestSys(sw);
    return FALSE;
  }
  return TRUE;
}





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief セットアップ
 *
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief セットアップ関数
 */
//------------------------------------------------------------------
static void SetupSoundTestSys(SOUNDTEST_WORK* sw)
{
  //VRAM設定
  GFL_DISP_SetBank(&dispVram);
  //BG初期化
  bg_init(sw);
  //２Ｄデータのロード
  g2d_load(sw);
}

//------------------------------------------------------------------
/**
 * @brief 開放関数
 */
//------------------------------------------------------------------
static void RemoveSoundTestSys(SOUNDTEST_WORK* sw)
{
  g2d_unload(sw);
  bg_exit(sw);
}

//------------------------------------------------------------------
/**
 * @brief システムメイン関数
 */
//------------------------------------------------------------------
static void MainSoundTestSys(SOUNDTEST_WORK* sw)
{
  g2d_draw(sw);
}

//------------------------------------------------------------------
/**
 * @brief   ＢＧ設定＆データ転送
 */
//------------------------------------------------------------------
static void bg_init(SOUNDTEST_WORK* sw)
{
  //ＢＧシステム起動
  GFL_BG_Init(sw->heapID);

  //ＢＧモード設定
  GFL_BG_SetBGMode(&bgsysHeader);

  //ＢＧコントロール設定
  GFL_BG_SetBGControl(BG_FRAME, &BGcont, GFL_BG_MODE_TEXT);
  GFL_BG_SetPriority(BG_FRAME, 1);
  GFL_BG_SetVisible(BG_FRAME, VISIBLE_ON);
  GFL_BG_SetBGControl(TEXT_FRAME, &Textcont, GFL_BG_MODE_TEXT);
  GFL_BG_SetPriority(TEXT_FRAME, 0);
  GFL_BG_SetVisible(TEXT_FRAME, VISIBLE_ON);
  GFL_BG_SetBGControl(STATUS_FRAME, &Statuscont, GFL_BG_MODE_TEXT);
  GFL_BG_SetPriority(STATUS_FRAME, 0);
  GFL_BG_SetVisible(STATUS_FRAME, VISIBLE_ON);

  //ビットマップウインドウ起動
  GFL_BMPWIN_Init(sw->heapID);

  //ディスプレイ面の選択
  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_SUB);
  GFL_DISP_SetDispOn();
}

static void bg_exit(SOUNDTEST_WORK* sw)
{
  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
  GFL_BMPWIN_Exit();
  GFL_BG_FreeBGControl(STATUS_FRAME);
  GFL_BG_FreeBGControl(TEXT_FRAME);
  GFL_BG_FreeBGControl(BG_FRAME);
  GFL_BG_Exit();
}

//------------------------------------------------------------------
/**
 * @brief   ２Ｄデータコントロール
 */
//------------------------------------------------------------------
static void g2d_load(SOUNDTEST_WORK* sw)
{
  int i;

  //背景読み込み
  void* bg_pltt = GFL_ARC_LoadDataAlloc
            (ARCID_SOUNDTEST, NARC_soundtest_soundtest_NCLR, sw->heapID);
  void* bg_char = GFL_ARC_LoadDataAlloc
            (ARCID_SOUNDTEST, NARC_soundtest_soundtest_NCGR, sw->heapID);
  void* bg_scrn = GFL_ARC_LoadDataAlloc
            (ARCID_SOUNDTEST, NARC_soundtest_soundtest_NSCR, sw->heapID);

  GFL_BG_LoadPalette
    (BG_FRAME, GFL_DISPUT_GetPltDataNNSbin(bg_pltt), PLTT_1*BG_PLTTNUM, PLTT_1*BG_PLTTID);
  GFL_BG_LoadCharacter
    (BG_FRAME, GFL_DISPUT_GetCgxDataNNSbin(bg_char), CHAR_1*BG_CHARNUM, CHAR_1*BG_CHARID);
  GFL_BG_LoadScreenBuffer
    (BG_FRAME, GFL_DISPUT_GetScrDataNNSbin(bg_scrn), SCRN_1*BG_SCRNNUM);

  GFL_HEAP_FreeMemory(bg_scrn);
  GFL_HEAP_FreeMemory(bg_char);
  GFL_HEAP_FreeMemory(bg_pltt);

  GFL_BG_LoadScreenReq(BG_FRAME);

  //文字表示ビットマップの作成
  GFL_BG_FillCharacter(TEXT_FRAME, 0, 1, 0);  // 先頭にクリアキャラ配置
  GFL_BG_ClearScreen(TEXT_FRAME);

  for(i=0; i<NAMEIDX_MAX; i++){
    sw->bmpwinName[i] = GFL_BMPWIN_Create(  TEXT_FRAME,
                      bmpwinNameStDat[i][0], bmpwinNameStDat[i][1],
                      bmpwinNameStDat[i][2], bmpwinNameStDat[i][3],
                      TEXT_PLTTID, GFL_BG_CHRAREA_GET_F );
    GFL_BMPWIN_MakeScreen(sw->bmpwinName[i]);
    PRINT_UTIL_Setup(&sw->printUtilName[i], sw->bmpwinName[i]);
  }
  for(i=0; i<NOIDX_MAX; i++){
    sw->bmpwinNo[i] = GFL_BMPWIN_Create(  TEXT_FRAME,
                      bmpwinNoStDat[i][0], bmpwinNoStDat[i][1],
                      bmpwinNoStDat[i][2], bmpwinNoStDat[i][3],
                      TEXT_PLTTID, GFL_BG_CHRAREA_GET_F );
    GFL_BMPWIN_MakeScreen(sw->bmpwinNo[i]);
    PRINT_UTIL_Setup(&sw->printUtilNo[i], sw->bmpwinNo[i]);
  }
  GFL_BG_LoadScreenReq(TEXT_FRAME);

  sw->g2dVintr = GFUser_VIntr_CreateTCB(g2d_vblank, (void*)sw, 0);
}

static void g2d_draw(SOUNDTEST_WORK* sw)
{
  int i;

  //選択サウンド名表示
  setSelectName(sw);
  for(i=0; i<NAMEIDX_MAX; i++){
    printName(sw, i);
    PRINT_UTIL_Trans(&sw->printUtilName[i], sw->printQue);
  }
  //数表示
  sw->setNo[NOIDX_HEAPFREESIZ] = PMSND_GetSndHeapFreeSize();
  for(i=0; i<NOIDX_MAX; i++){
    printNo(sw, i, 6);
    PRINT_UTIL_Trans(&sw->printUtilNo[i], sw->printQue);
  }
  PRINTSYS_QUE_Main(sw->printQue);
}

static void g2d_unload(SOUNDTEST_WORK* sw)
{
  int i;

  GFL_TCB_DeleteTask(sw->g2dVintr);

  for(i=0; i<NOIDX_MAX; i++){ GFL_BMPWIN_Delete(sw->bmpwinNo[i]); }
  for(i=0; i<NAMEIDX_MAX; i++){ GFL_BMPWIN_Delete(sw->bmpwinName[i]); }
}

static void g2d_vblank(GFL_TCB* tcb, void* work)
{
  SOUNDTEST_WORK* sw =  (SOUNDTEST_WORK*)work;
}





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief コントローラ
 *
 *
 *
 *
 *
 */
//============================================================================================
static void changeVStatus(int vpIdx, u8 pan, int vofs, int sofs);
//------------------------------------------------------------------
/**
 * @brief タッチパネルイベント判定
 */
//------------------------------------------------------------------
static BOOL checkTouchPanelEvent(SOUNDTEST_WORK* sw)
{
  if( checkTouchPanelEventTrg(sw) == TRUE ){ return TRUE; }
  if( checkTouchPanelEventCont(sw) == TRUE ){ return TRUE; }

  return FALSE;
}

//------------------------------------------------------------------
static BOOL checkTouchPanelEventTrg(SOUNDTEST_WORK* sw)
{
  int tblPos = GFL_UI_TP_HitTrg(eventTouchPanelTableTrg);
  u32 vpIdx;

  if(tblPos == GFL_UI_TP_HIT_NONE){
    return FALSE;
  }
  switch(tblPos){

  case SNDTEST_TPEV_BGM_PLAY:
    if( sw->bgmPauseSw == FALSE ){
      PMSND_PlayBGM(sw->setNo[NOIDX_BGMNO]);
    } else {
      PMSND_PauseBGM(FALSE);
      sw->bgmPauseSw = FALSE;
    }
#if 0//#ifdef PM_DEBUG
    mcsControlReset(sw->heapID);
#endif
    break;

  case SNDTEST_TPEV_BGM_STOP:
    PMSND_StopBGM();
    sw->bgmPauseSw = FALSE;
    break;

  case SNDTEST_TPEV_BGM_PAUSE:
    if( sw->bgmPauseSw == FALSE ){
      if( PMSND_CheckPlayBGM() == TRUE ){
        PMSND_PauseBGM(TRUE);
        sw->bgmPauseSw = TRUE;
      }
    }
    break;

  case SNDTEST_TPEV_BGM_PLAYFADEIN:
    PMSND_StopBGM();
    PMSND_PlayNextBGM(sw->setNo[NOIDX_BGMNO], 60, 60);
    sw->bgmPauseSw = FALSE;
    break;

  case SNDTEST_TPEV_BGM_FADEOUTSTOP:
    PMSND_PlayNextBGM(0, 60, 60);
    sw->bgmPauseSw = FALSE;
    break;

  case SNDTEST_TPEV_BGM_FADEOUTPLAYFADEIN:
    PMSND_PlayNextBGM(sw->setNo[NOIDX_BGMNO], 60, 60);
    sw->bgmPauseSw = FALSE;
    break;

  case SNDTEST_TPEV_BGM_PAUSEPUSH:
    PMSND_PauseBGM(TRUE);
    PMSND_PushBGM();
    sw->bgmPauseSw = FALSE;
    break;

  case SNDTEST_TPEV_BGM_POPPLAY:
    PMSND_PopBGM();
    PMSND_PauseBGM(FALSE);
    sw->bgmPauseSw = FALSE;
    break;

  case SNDTEST_TPEV_VOICE_PLAY:
    vpIdx = PMV_PlayVoice(  sw->setNo[NOIDX_VOICENO], 0 );
    //PMVOICE_SetStatus(vpIdx, sw->setNo[NOIDX_VOICEPAN], 0, sw->setNo[NOIDX_VOICESPEED]);
		changeVStatus(vpIdx, sw->setNo[NOIDX_VOICEPAN], 0, sw->setNo[NOIDX_VOICESPEED]);
    break;

  case SNDTEST_TPEV_VOICE_PLAYCHROUS:
    vpIdx = PMV_PlayVoice_Chorus( sw->setNo[NOIDX_VOICENO], 0,
                    sw->setNo[NOIDX_VOICECHORUSVOL],
                    sw->setNo[NOIDX_VOICECHORUSSPEED] );
    //PMVOICE_SetStatus(vpIdx, sw->setNo[NOIDX_VOICEPAN], 0, sw->setNo[NOIDX_VOICESPEED]);
		changeVStatus(vpIdx, sw->setNo[NOIDX_VOICEPAN], 0, sw->setNo[NOIDX_VOICESPEED]);
    break;

  case SNDTEST_TPEV_VOICE_PLAYREVERSE:
    vpIdx = PMV_PlayVoice_Reverse( sw->setNo[NOIDX_VOICENO], 0 );
    //PMVOICE_SetStatus(vpIdx, sw->setNo[NOIDX_VOICEPAN], 0, sw->setNo[NOIDX_VOICESPEED]);
		changeVStatus(vpIdx, sw->setNo[NOIDX_VOICEPAN], 0, sw->setNo[NOIDX_VOICESPEED]);
    break;

  case SNDTEST_TPEV_VOICE_PLAYREVERSECHORUS:
    vpIdx = PMV_PlayVoice_Custom( sw->setNo[NOIDX_VOICENO], 0,
                    sw->setNo[NOIDX_VOICEPAN],
                    TRUE,
                    sw->setNo[NOIDX_VOICECHORUSVOL],
                    sw->setNo[NOIDX_VOICECHORUSSPEED],
                    TRUE );
    //PMVOICE_SetStatus(vpIdx, sw->setNo[NOIDX_VOICEPAN], 0, sw->setNo[NOIDX_VOICESPEED]);
		changeVStatus(vpIdx, sw->setNo[NOIDX_VOICEPAN], 0, sw->setNo[NOIDX_VOICESPEED]);
    break;

  case SNDTEST_TPEV_SE_PLAY:
    PMSND_PlaySE(sw->setNo[NOIDX_SENO]);
    PMSND_SetStatusSE( PMSND_NOEFFECT, sw->setNo[NOIDX_SEPITCH], sw->setNo[NOIDX_SEPAN]);
    break;

  case SNDTEST_TPEV_SE_STOP:
    PMSND_StopSE();
    break;

  case SNDTEST_TPEV_REVERB:
    if(sw->reverbFlag == FALSE){
      PMSND_EnableCaptureReverb( 0x2000, 16000, 63, 0 );
      sw->reverbFlag = TRUE;
    } else {
      PMSND_DisableCaptureReverb();
      sw->reverbFlag = FALSE;
    }
    writeButton(sw, 0x07, 0x15, sw->reverbFlag );
    break;

  case SNDTEST_TPEV_EXIT:
    sw->seq++;
    break;
  }
  return TRUE;
}

static void changeVStatus(int vpIdx, u8 pan, int vofs, int sofs)
{
	s8	volume = PMV_GetVolume(vpIdx) + vofs;
	int	speed = PMV_GetSpeed(vpIdx) + sofs;

	PMV_SetPan(vpIdx, pan);
	PMV_SetVolume(vpIdx, volume);
	PMV_SetSpeed(vpIdx, speed);
}

//------------------------------------------------------------------
static BOOL checkTouchPanelEventCont(SOUNDTEST_WORK* sw)
{
  int tblPos = GFL_UI_TP_HitCont(eventTouchPanelTableCont);
  u32 vpIdx;

  if(tblPos == GFL_UI_TP_HIT_NONE){
    sw->tpTrgRepeatFrame = 0;
    sw->tpTrgRepeatCount = 0;
    return FALSE;
  }
  if(sw->tpTrgRepeatFrame){
    sw->tpTrgRepeatFrame--;
    return TRUE;
  }
  switch(tblPos){

  case SNDTEST_TPEV_BGM_NUMDOWN1:
    numberDec(sw, NOIDX_BGMNO, PMSND_BGM_START, 1);
    break;
  case SNDTEST_TPEV_BGM_NUMDOWN10:
    numberDec(sw, NOIDX_BGMNO, PMSND_BGM_START, 10);
    break;

  case SNDTEST_TPEV_BGM_NUMUP1:
    numberInc(sw, NOIDX_BGMNO, PMSND_BGM_END, 1);
    break;
  case SNDTEST_TPEV_BGM_NUMUP10:
    numberInc(sw, NOIDX_BGMNO, PMSND_BGM_END, 10);
    break;

  case SNDTEST_TPEV_VOICE_NUMDOWN1:
    numberDec(sw, NOIDX_VOICENO, PMVOICE_START, 1);
    break;
  case SNDTEST_TPEV_VOICE_NUMDOWN10:
    numberDec(sw, NOIDX_VOICENO, PMVOICE_START, 10);
    break;
  case SNDTEST_TPEV_VOICE_NUMUP1:
    numberInc(sw, NOIDX_VOICENO, PMVOICE_END, 1);
    break;
  case SNDTEST_TPEV_VOICE_NUMUP10:
    numberInc(sw, NOIDX_VOICENO, PMVOICE_END, 10);
    break;

  case SNDTEST_TPEV_SE_NUMDOWN1:
    numberDec(sw, NOIDX_SENO, PMSND_SE_START, 1);
    break;
  case SNDTEST_TPEV_SE_NUMDOWN10:
    numberDec(sw, NOIDX_SENO, PMSND_SE_START, 10);
    break;
  case SNDTEST_TPEV_SE_NUMUP1:
    numberInc(sw, NOIDX_SENO, PMSND_SE_END, 1);
    break;
  case SNDTEST_TPEV_SE_NUMUP10:
    numberInc(sw, NOIDX_SENO, PMSND_SE_END, 10);
    break;

  case SNDTEST_TPEV_VOICE_PANDOWN1:
    numberDec(sw, NOIDX_VOICEPAN, -127, 1);
    break;
  case SNDTEST_TPEV_VOICE_PANDOWN10:
    numberDec(sw, NOIDX_VOICEPAN, -127, 10);
    break;
  case SNDTEST_TPEV_VOICE_PANUP1:
    numberInc(sw, NOIDX_VOICEPAN, 127, 1);
    break;
  case SNDTEST_TPEV_VOICE_PANUP10:
    numberInc(sw, NOIDX_VOICEPAN, 127, 10);
    break;

  case SNDTEST_TPEV_SE_PANDOWN1:
    numberDec(sw, NOIDX_SEPAN, -127, 1);
    break;
  case SNDTEST_TPEV_SE_PANDOWN10:
    numberDec(sw, NOIDX_SEPAN, -127, 10);
    break;
  case SNDTEST_TPEV_SE_PANUP1:
    numberInc(sw, NOIDX_SEPAN, 127, 1);
    break;
  case SNDTEST_TPEV_SE_PANUP10:
    numberInc(sw, NOIDX_SEPAN, 127, 10);
    break;

  case SNDTEST_TPEV_VOICE_SPEEDDOWN1:
    numberDec(sw, NOIDX_VOICESPEED, 0x80000000, 1);
    break;
  case SNDTEST_TPEV_VOICE_SPEEDDOWN10:
    numberDec(sw, NOIDX_VOICESPEED, 0x80000000, 10);
    break;
  case SNDTEST_TPEV_VOICE_SPEEDUP1:
    numberInc(sw, NOIDX_VOICESPEED, 0x7fffffff, 1);
    break;
  case SNDTEST_TPEV_VOICE_SPEEDUP10:
    numberInc(sw, NOIDX_VOICESPEED, 0x7fffffff, 10);
    break;

  case SNDTEST_TPEV_VOICE_CHORUSSPEEDDOWN1:
    numberDec(sw, NOIDX_VOICECHORUSSPEED, 0x80000000, 1);
    break;
  case SNDTEST_TPEV_VOICE_CHORUSSPEEDDOWN10:
    numberDec(sw, NOIDX_VOICECHORUSSPEED, 0x80000000, 10);
    break;
  case SNDTEST_TPEV_VOICE_CHORUSSPEEDUP1:
    numberInc(sw, NOIDX_VOICECHORUSSPEED, 0x7fffffff, 1);
    break;
  case SNDTEST_TPEV_VOICE_CHORUSSPEEDUP10:
    numberInc(sw, NOIDX_VOICECHORUSSPEED, 0x7fffffff, 10);
    break;

  case SNDTEST_TPEV_VOICE_CHORUSVOLDOWN1:
    numberDec(sw, NOIDX_VOICECHORUSVOL, -127, 1);
    break;
  case SNDTEST_TPEV_VOICE_CHORUSVOLDOWN10:
    numberDec(sw, NOIDX_VOICECHORUSVOL, -127, 10);
    break;
  case SNDTEST_TPEV_VOICE_CHORUSVOLUP1:
    numberInc(sw, NOIDX_VOICECHORUSVOL, 127, 1);
    break;
  case SNDTEST_TPEV_VOICE_CHORUSVOLUP10:
    numberInc(sw, NOIDX_VOICECHORUSVOL, 127, 10);
    break;

  case SNDTEST_TPEV_SE_PITCHDOWN1:
    numberDec(sw, NOIDX_SEPITCH, 0x80000000, 1);
    break;
  case SNDTEST_TPEV_SE_PITCHDOWN10:
    numberDec(sw, NOIDX_SEPITCH, 0x80000000, 10);
    break;
  case SNDTEST_TPEV_SE_PITCHUP1:
    numberInc(sw, NOIDX_SEPITCH, 0x7fffffff, 1);
    break;
  case SNDTEST_TPEV_SE_PITCHUP10:
    numberInc(sw, NOIDX_SEPITCH, 0x7fffffff, 10);
    break;
  }
  sw->tpTrgRepeatFrame = tpRepeatFrame[sw->tpTrgRepeatCount];
  if(sw->tpTrgRepeatCount < NELEMS(tpRepeatFrame) - 1){ sw->tpTrgRepeatCount++; }

  return TRUE;
}





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief
 *
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 選択サウンド名取り出し
 */
//------------------------------------------------------------------
static void setSelectName(SOUNDTEST_WORK* sw)
{
  u32 msgIdx;
  int i;

  for(i=0; i<NAMEIDX_MAX; i++){ GFL_BMP_Clear(GFL_BMPWIN_GetBmp(sw->bmpwinName[i]), 0); }

  // BGM
  msgIdx = (sw->setNo[NOIDX_BGMNO]-initNoData[NOIDX_BGMNO]) + NAMEMSG_STARTIDX_BGM;
  GFL_MSG_GetString(sw->msgman, msgIdx, sw->setName[NAMEIDX_BGM]);
  // SE
  msgIdx = (sw->setNo[NOIDX_SENO]-initNoData[NOIDX_SENO]) + NAMEMSG_STARTIDX_SE;
  GFL_MSG_GetString(sw->msgman, msgIdx, sw->setName[NAMEIDX_SE]);
  // VOICE
  msgIdx = (sw->setNo[NOIDX_VOICENO]-initNoData[NOIDX_VOICENO]) + NAMEMSG_STARTIDX_VOICE;
  GFL_MSG_GetString(sw->monsmsgman, msgIdx, sw->setName[NAMEIDX_VOICE]);

}

//------------------------------------------------------------------
/**
 * @brief ナンバー処理
 */
//------------------------------------------------------------------
static void numberInc(SOUNDTEST_WORK* sw, int idx, int max, int num )
{
  sw->setNo[idx] += num;
  if(sw->setNo[idx] > max){ sw->setNo[idx] = max; }
}

static void numberDec(SOUNDTEST_WORK* sw, int idx, int min, int num )
{
  sw->setNo[idx] -= num;
  if(sw->setNo[idx] < min){ sw->setNo[idx] = min; }
}

//------------------------------------------------------------------
/**
 * @brief ボタン描画処理
 */
//------------------------------------------------------------------
static void writeButton(SOUNDTEST_WORK* sw, u8 x, u8 y, BOOL flag )
{
  u16 chrNo;
  u16 buf[2];
  u16 palMask = 1 << 12;

  if(flag == TRUE){ chrNo = 0xae; }
  else { chrNo = 0xbe; }

  buf[0] = chrNo | palMask;
  buf[1] = (chrNo+1) | palMask;
  GFL_BG_WriteScreen( BG_FRAME, buf, x, y, 2, 1 );
  GFL_BG_LoadScreenReq(BG_FRAME);
}

//------------------------------------------------------------------
/**
 * @brief 名前描画
 */
//------------------------------------------------------------------
static void printName(SOUNDTEST_WORK* sw, int idx)
{
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(sw->bmpwinName[idx]), 0);

    PRINT_UTIL_PrintColor(  &sw->printUtilName[idx], sw->printQue, 3, 7,
              sw->setName[idx], sw->fontHandle, PRINTSYS_LSB_Make(2,3,0));
}

//------------------------------------------------------------------
/**
 * @brief 数字描画
 */
//------------------------------------------------------------------
static void printNo(SOUNDTEST_WORK* sw, int idx, u32 numberSize )
{
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(sw->bmpwinNo[idx]), 0);

  STRTOOL_SetNumber(  sw->setNoBuffer, sw->setNo[idx], numberSize,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
    PRINT_UTIL_PrintColor(  &sw->printUtilNo[idx], sw->printQue, 1, 1,
              sw->setNoBuffer, sw->fontHandle, PRINTSYS_LSB_Make(2,3,0));
}



//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief MCS通信処理
 *
 *
 *
 *
 *
 */
//============================================================================================
#if 0//#ifdef PM_DEBUG
static u32 mcsControl(HEAPID heapID)
{
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
    // MCS接続処理
    GFL_MCS_SNDVIEWER_Exit();
    GFL_MCS_SNDVIEWER_Init(heapID);
  }
  return GFL_MCS_SNDVIEWER_Main(heapID);
}

static void mcsControlEnd(void)
{
  GFL_MCS_SNDVIEWER_Exit();
}

static void mcsControlReset(HEAPID heapID)
{
  u32 param[PNUM_COMM_PANEL_RESET] = {0};

  MCS_Sound_Send(COMM_PANEL_RESET, param, heapID);
}

static void mcsControlSetTrackSt(HEAPID heapID, u32 trackSt)
{
  u32 param[PNUM_COMM_SET_TRACKST];
  param[0] = trackSt;

  MCS_Sound_Send(COMM_SET_TRACKST, param, heapID);
}
#endif
