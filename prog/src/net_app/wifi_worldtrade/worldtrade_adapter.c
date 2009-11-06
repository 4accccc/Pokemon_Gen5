//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   worldtrade_adapter.c
 *  @brief  GTSを移植する際のパック関数群
 *  @author Toru=Nagihashi
 *  @data   2009.08.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>
//system
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "libdpw/dpw_tr.h"

//mine
#include "worldtrade_adapter.h"
//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *          構造体宣言
*/
//=============================================================================
#define POKEMON_DATA_SIZE   (236)
//POKETOOL_GetWorkSize()
//savedata\worldtrade\worldtrade_data.c  15

//=============================================================================
/**
 *          プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  PPPからニックネームを登録する関数
 *
 *  @param  WORDSET* wordset  ワードセット
 *  @param  bufID             バッファID
 *  @param  POKEMON_PASO_PARAM * ppp  PPP
 */
//-----------------------------------------------------------------------------
void WT_WORDSET_RegisterPokeNickNamePPP( WORDSET* wordset, u32 bufID, const POKEMON_PASO_PARAM * ppp )
{
  POKEMON_PARAM *pp = PP_CreateByPPP( ppp, HEAPID_WORLDTRADE );
  WORDSET_RegisterPokeNickName( wordset, bufID, pp );
  GFL_HEAP_FreeMemory( pp );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ポケモンパラメータ分のメモリ確保
 *
 *  @param  HEAPID heapID   ヒープID
 *
 *  @return PP構造体分のメモリ
 */
//-----------------------------------------------------------------------------
POKEMON_PARAM *PokemonParam_AllocWork( HEAPID heapID )
{
  return GFL_HEAP_AllocMemory( heapID, POKEMON_DATA_SIZE );
}
//----------------------------------------------------------------------------
/**
 *  @brief  ポケモンパラメータのコピー
 *
 *  @param  const POKEMON_PARAM *pp_src
 *  @param  *pp_dest
 */
//-----------------------------------------------------------------------------
void WT_PokeCopyPPtoPP( const POKEMON_PARAM *pp_src, POKEMON_PARAM *pp_dest )
{
  GFL_STD_MemCopy( pp_src, pp_dest, POKEMON_DATA_SIZE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  クリア
 *
 *  @param  GFL_BMPWIN * winGFLBMPWIN
 *  @param  trans_sw        転送フラグ
 */
//-----------------------------------------------------------------------------
void BmpMenuWinClear( GFL_BMPWIN * win, u8 trans_sw )
{
  GFL_BMPWIN_ClearScreen( win );
  switch( trans_sw )
  {
  case WINDOW_TRANS_ON:
    GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(win) );
    break;
  case WINDOW_TRANS_ON_V:
    GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(win) );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  文字列展開しつつMSGを取得
 *
 *  @param  WORDSET *wordset  ワードセット
 *  @param  *MsgManager       メッセージ
 *  @param  strID             文字ID
 *  @param  heapID            テンポラリヒープID
 *
 *  @return 文字列展開後のSTRBUF
 */
//-----------------------------------------------------------------------------
STRBUF * MSGDAT_UTIL_AllocExpandString( const WORDSET *wordset, GFL_MSGDATA *MsgManager, u32 strID, HEAPID heapID )
{
  STRBUF  *src;
  STRBUF  *dst;
  src = GFL_MSG_CreateString( MsgManager, strID );
  dst = GFL_STR_CreateBuffer( GFL_STR_GetBufferLength( src ), heapID );

  WORDSET_ExpandStr( wordset, src, dst );

  GFL_STR_DeleteBuffer( src );

  return dst;
}

//----------------------------------------------------------------------------
/**
 *  @brief  pppからppを作成し、コピーする
 *
 *  @param  const POKEMON_PASO_PARAM *ppp
 *  @param  *pp
 */
//-----------------------------------------------------------------------------
void PokeReplace( const POKEMON_PASO_PARAM *ppp, POKEMON_PARAM *pp )
{
  POKEMON_PARAM*  src = PP_CreateByPPP( ppp, HEAPID_WORLDTRADE );
  WT_PokeCopyPPtoPP( src, pp );
  GFL_HEAP_FreeMemory( src );
}


//----------------------------------------------------------------------------
/**
 *  @brief  PP2PPP
 *
 *  @param  POKEMON_PARAM *pp
 *
 *  @return PPP
 */
//-----------------------------------------------------------------------------
POKEMON_PASO_PARAM* PPPPointerGet( POKEMON_PARAM *pp )
{
  return (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst( pp );
}

//----------------------------------------------------------------------------
/**
 *  @brief  プロファイルを作成
 *
 *  @param  SAVE_CONTROL_WORK * savedata  セーブデータ
 *  @param  * profile                     プロファイル
 *
 */
//-----------------------------------------------------------------------------
static void EMAILSAVE_DCProfileCreateCommon( SAVE_CONTROL_WORK *sv, Dpw_Common_Profile *dc_profile )
{
  WIFI_HISTORY *wh = SaveData_GetWifiHistory(sv);
  MYSTATUS *my = SaveData_GetMyStatus(sv);
  //char *email_address = EMAILSAVE_AddressGet(sv);

  GFL_STD_MemClear(dc_profile, sizeof(Dpw_Common_Profile));

  dc_profile->version = PM_VERSION;
  dc_profile->language = PM_LANG;
  dc_profile->countryCode = WIFIHISTORY_GetMyNation(wh);
  dc_profile->localCode = WIFIHISTORY_GetMyArea(wh);
  dc_profile->playerId = MyStatus_GetID(my);

  STRTOOL_Copy( MyStatus_GetMyName(my), dc_profile->playerName, DPW_TR_NAME_SIZE );

  dc_profile->flag = 0; //ハングル文字を表示できるか
//  dc_profile->macAddr   ライブラリ内で格納するのでセットの必要なし

  //strcpy(dc_profile->mailAddr, email_address);

#if 0
  dc_profile->mailRecvFlag = FALSE;////EMAILSAVE_ParamGet(sv, EMAIL_PARAM_RECV_FLAG);
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_EMAIL)
  SVLD_SetCrc(GMDATA_ID_EMAIL);
#endif //CRC_LOADCHECK
#endif
}
void EMAILSAVE_DCProfileCreate_Update( SAVE_CONTROL_WORK * savedata, Dpw_Common_Profile * profile )
{
  EMAILSAVE_DCProfileCreateCommon( savedata, profile );
}

//=============================================================================
/**
 *          プリントフォント周り
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  文字列の幅を返す
 *
 *  @param  WT_PRINT *wk  プリント情報
 *  @param  *buf    文字列
 *  @param  magin   マージン
 *
 *  @return 幅
 */
//-----------------------------------------------------------------------------
int FontProc_GetPrintStrWidth( WT_PRINT *wk, u8 font_idx, STRBUF *buf, int magin )
{
  return PRINTSYS_GetStrWidth( buf, wk->font, magin );
}

//----------------------------------------------------------------------------
/**
 *  @brief  文字列表示に必要な情報を初期化
 *
 *  @param  WT_PRINT *wk  ワーク
 *  @param  CONFIG *cfg   コンフィグ
 */
//-----------------------------------------------------------------------------
void WT_PRINT_Init( WT_PRINT *wk, const CONFIG *cfg )
{
  GFL_STD_MemClear( wk, sizeof(WT_PRINT) );

  wk->tcbsys  = GFL_TCBL_Init( HEAPID_WORLDTRADE, HEAPID_WORLDTRADE, 32, 32 );
  wk->cfg     = cfg;
  wk->font    = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WORLDTRADE );


  {
    int i;
    WT_PRINT_QUE  *p_one;
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      p_one = &wk->one[i];
      p_one->que    = PRINTSYS_QUE_Create( HEAPID_WORLDTRADE );
      GFL_STD_MemClear( &p_one->util, sizeof(PRINT_UTIL) );
      p_one->use    = FALSE;
    }
  }

}

//----------------------------------------------------------------------------
/**
 *  @brief  文字列情報に必要な情報を破棄
 *
 *  @param  WT_PRINT *wk  ワーク
 */
//-----------------------------------------------------------------------------
void WT_PRINT_Exit( WT_PRINT *wk )
{
  {
    int i;
    WT_PRINT_QUE  *p_one;
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      p_one = &wk->one[i];
      PRINTSYS_QUE_Delete( p_one->que );
    }
  }

  GFL_FONT_Delete( wk->font );
  GFL_TCBL_Exit( wk->tcbsys );
}

//----------------------------------------------------------------------------
/**
 *  @brief  文字列表情に必要な情報  メイン処理
 *
 *  @param  WT_PRINT *wk ワーク
 */
//-----------------------------------------------------------------------------
void WT_PRINT_Main( WT_PRINT *wk )
{
  {
    int i;
    WT_PRINT_QUE  *p_one;
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      p_one = &wk->one[i];
      if( p_one->use )
      {
        if( PRINT_UTIL_Trans( &p_one->util, p_one->que ) )
        {
          p_one->use  = FALSE;
        }
      }

      if( wk->stream[i] != NULL )
      {
        if( PRINTSYS_PrintStreamGetState(wk->stream[i]) == PRINTSTREAM_STATE_DONE )
        {
          PRINTSYS_PrintStreamDelete( wk->stream[i] );
          wk->stream[i] = NULL;
        }
      }
    }
  }


  GFL_TCBL_Main( wk->tcbsys );
}

//----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  PRINT_STREAM *stream  ストリーム
 *
 *  @return FALSEならば文字列描画終了 TRUEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL GF_MSG_PrintEndCheck( WT_PRINT *setup )
{
  int i;
  for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
  {

    if( setup->stream[i] != NULL )
    {
      if( PRINTSYS_PrintStreamGetState(setup->stream[i]) == PRINTSTREAM_STATE_DONE )
      {
        return FALSE;
      }
      else
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  GFL_BMPWIN *bmpwin  BMPWIN
 *  @param  *str  文字列
 *  @param  x     表示位置X
 *  @param  y     表示位置Y
 *  @param  WT_PRINT *setup   文字列に必要な情報
 *
 *  @return プリントストリーム
 */
//-----------------------------------------------------------------------------
void GF_STR_PrintSimple( GFL_BMPWIN *bmpwin, u8 font_idx, STRBUF *str, int x, int y, WT_PRINT *setup )
{
  int i;
  {
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      if( setup->stream[i] == NULL )
      {
        break;
      }
    }
    GF_ASSERT( i < WT_PRINT_BUFF_MAX );
  }

  setup->stream[i]  = PRINTSYS_PrintStream(
    bmpwin, x, y, str, setup->font,
    MSGSPEED_GetWait(), setup->tcbsys, 0, HEAPID_WORLDTRADE, 0x0f );

}
//----------------------------------------------------------------------------
/**
 *  @brief  色つき一括表示
 *
 *  @param  GFL_BMPWIN *bmpwin  書き込むBMPWIN
 *  @param  *str  文字列
 *  @param  x     X
 *  @param  y     Y
 *  @param  color 色
 *  @param  *setup  文字列用情報
 */
//-----------------------------------------------------------------------------
void GF_STR_PrintColor( GFL_BMPWIN *bmpwin, u8 font_idx, STRBUF *str, int x, int y, int put_type, PRINTSYS_LSB color, WT_PRINT *setup )
{
  WT_PRINT_QUE  *p_one  = NULL;

  {
    int i;
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      if( !setup->one[i].use )
      {
        p_one = &setup->one[i];
      }
    }
  }

  GF_ASSERT( p_one != NULL );

  PRINT_UTIL_Setup( &p_one->util, bmpwin );
  PRINT_UTIL_PrintColor( &p_one->util, p_one->que, x, y, str, setup->font, color );

  p_one->use  = TRUE;
}

//=============================================================================
/**
 *    BGWINFRM
 */
//=============================================================================
#if 0
//-------------------------------------
///   WIN_FRM
//=====================================
typedef struct {
  u16 * scr;      // スクリーンデータ
  u16 sx:6;     // スクリーンのＸサイズ
  u16 sy:6;     // スクリーンのＹサイズ
  u16 vanish:1;   // 表示フラグ　1 = ON, 0 = OFF
  u16 auto_flg:3;   // 自動移動フラグ　0 = ON, 1 = OFF
  // 2bit
  s8  px;       // 表示Ｘ座標
  s8  py;       // 表示Ｙ座標

  s8  mvx;      // １フレームのＸ移動量
  s8  mvy;      // １フレームのＹ移動量
  u8  frm;      // ＢＧフレーム
  u8  cnt;      // 動作カウンタ

  s8  area_lx;    // 描画範囲
  s8  area_rx;
  s8  area_uy;
  s8  area_dy;

}BGWINFRM;

//-------------------------------------
///   メインワーク
//=====================================
struct _BGWINFRM_WORK
{
  BGWINFRM * win;   // フレームデータ
  u16 max;      // フレーム数
  u16 trans_mode:15;  // 転送モード
  u16 auto_master:1;  // 自動移動マスターフラグ　0 = ON, 1 = OFF
  u32 heapID;     // ヒープＩＤ
};

//----------------------------------------------------------------------------
/**
 *  @brief  BGWINFRMの作成
 *
 *  @param  int mode  使用しない  （VBlankOnly）
 *  @param  max       フレームの数
 *  @param  heapID    ヒープID
 *
 *  @return BGWINFRM_WORK
 */
//-----------------------------------------------------------------------------
BGWINFRM_WORK * BGWINFRM_Create( int mode, int max, HEAPID heapID )
{
  BGWINFRM_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BGWINFRM_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BGWINFRM_WORK) );
  p_wk->max = max;
  p_wk->trans_mode  = mode;
  p_wk->auto_master = 0;

  p_wk->win = GFL_HEAP_AllocMemory( heapID, sizeof(BGWINFRM) * max );
  GFL_STD_MemClear( p_wk->win, sizeof(BGWINFRM) * max );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *  @brief  BGWINFRM_WORKの破棄
 *
 *  @param  BGWINFRM_WORK *wk   ワーク
 */
//-----------------------------------------------------------------------------
void BGWINFRM_Exit( BGWINFRM_WORK *wk )
{
  int i;
  for( i = 0; i < wk->max; i++ )
  {
    if( wk->win[i].scr != NULL )
    {
      GFL_HEAP_FreeMemory( wk->win[i].scr );
    }
  }

  GFL_HEAP_FreeMemory( wk->win );
  GFL_HEAP_FreeMemory( wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  フレーム追加
 *
 *  @param  BGWINFRM_WORK *wk ワーク
 *  @param  index             インデックス
 *  @param  frm               フレーム
 *  @param  sx                サイズX
 *  @param  sy                サイズY
 */
//-----------------------------------------------------------------------------
void BGWINFRM_Add( BGWINFRM_WORK *wk, int index, int frm, int sx, int sy )
{
  BGWINFRM * win;


  win = &wk->win[index];

  GF_ASSERT( win->scr == NULL );

  win->scr = GFL_HEAP_AllocMemory( wk->heapID, sx * sy * 2 );
  win->sx  = sx;
  win->sy  = sy;

  win->frm = frm;

  win->px  = 0;
  win->py  = 0;

  win->auto_flg = 0;

  win->vanish = FALSE;

  win->area_lx = 0;
  win->area_rx = 32;
  win->area_uy = 0;
  win->area_dy = 24;
}
//--------------------------------------------------------------------------------------------
/**
 * BMPWINを書き込む
 *
 * @param wk      フレームワーク
 * @param index   フレームインデックス
 * @param win     BMPWIN
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_BmpWinOn( BGWINFRM_WORK *wk, int index, GFL_BMPWIN *win )
{
  u16 * scr;
  u32 sx;
  u16 i, j;
  u16 chr, pal;

  scr = wk->win[index].scr;
  sx  = wk->win[index].sx;

  chr = GFL_BMPWIN_GetChrNum( win );

  //pal = win->palnum << 12;
  pal = GFL_BMPWIN_GetPalette(win) << 12;

  for( i=0; i<GFL_BMPWIN_GetSizeY(win); i++ ){
    if( ( GFL_BMPWIN_GetPosY(win) + i ) >= wk->win[index].sy ){
      break;
    }
    for( j=0; j<GFL_BMPWIN_GetSizeX(win); j++ ){
      if( ( GFL_BMPWIN_GetPosX(win) + j ) >= wk->win[index].sx ){
        break;
      }
      scr[ ( GFL_BMPWIN_GetPosY(win) + i ) * sx + GFL_BMPWIN_GetPosX(win) + j ] = chr + j + pal;
    }
    chr += GFL_BMPWIN_GetSizeX( win );
  }
}
//--------------------------------------------------------------------------------------------
/**
 * フレームの表示位置を取得
 *
 * @param wk      フレームワーク
 * @param index   フレームインデックス
 * @param px      Ｘ座標格納場所
 * @param py      Ｙ座標格納場所
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_PosGet( BGWINFRM_WORK *wk, int index, s8 *px, s8 *py  )
{
  if( px != NULL ){ *px = wk->win[index].px; }
  if( py != NULL ){ *py = wk->win[index].py; }
}
//--------------------------------------------------------------------------------------------
/**
 * フレーム自動移動メイン（全体）
 *
 * @param wk      フレームワーク
 *
 * @return  none
 *
 * @li  BGWINFRM_MoveOne() と同時に使用すると 2move/1frame になるので注意
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_MoveMain( BGWINFRM_WORK *wk )
{
  u32 i;

  if( wk->auto_master == 0 ){ return; }

  wk->auto_master = 0;

  for( i=0; i<wk->max; i++ ){
    if( BGWINFRM_MoveOne( wk, i ) == 1 ){
      wk->auto_master = 1;
    }
  }
}
//--------------------------------------------------------------------------------------------
/**
 * フレーム自動移動メイン（個別）
 *
 * @param wk      フレームワーク
 * @param index   フレームインデックス
 *
 * @retval  "0 = 停止中"
 * @retval  "1 = 移動中"
 *
 * @li  BGWINFRM_MoveMain() と同時に使用すると 2move/1frame になるので注意
 */
//--------------------------------------------------------------------------------------------
u32 BGWINFRM_MoveOne( BGWINFRM_WORK * wk, u32 index )
{
  if( wk->win[index].auto_flg == 0 ){
    return 0;
  }

//  wk->auto_master = 0;

  BGWINFRM_FrameOff( wk, index );
  wk->win[index].px += wk->win[index].mvx;
  wk->win[index].py += wk->win[index].mvy;
  BGWINFRM_FrameOn( wk, index );
  wk->win[index].cnt--;
//  if( wk->win[index].cnt == 0 || wk->win[index].vanish == BGWINFRM_OFF ){
  if( wk->win[index].cnt == 0 ){
    wk->win[index].auto_flg = 0;
    return 0;
  }

//  wk->auto_master = 1;

  return 1;
}
//--------------------------------------------------------------------------------------------
/**
 * フレーム自動移動設定
 *
 * @param wk      フレームワーク
 * @param index   フレームインデックス
 * @param mvx     Ｘ移動量（１フレーム）
 * @param mvy     Ｙ移動量（１フレーム）
 * @param cnt     移動フレーム数
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_MoveInit( BGWINFRM_WORK *wk, int index, int add_x, int add_y, int cnt )
{
  BGWINFRM * win = &wk->win[index];

  win->mvx = add_x;
  win->mvy = add_y;
  win->cnt = cnt;

  win->auto_flg = 1;

  wk->auto_master = 1;
}

//--------------------------------------------------------------------------------------------
/**
 * フレーム自動移動チェック
 *
 * @param wk      フレームワーク
 * @param index   フレームインデックス
 *
 * @retval  "0 = 停止中"
 * @retval  "1 = 移動中"
 */
//--------------------------------------------------------------------------------------------
u32 BGWINFRM_MoveCheck( BGWINFRM_WORK *wk, int index )
{
  return wk->win[index].auto_flg;
}

//--------------------------------------------------------------------------------------------
/**
 * フレームにスクリーンをセット
 *
 * @param wk    フレームワーク
 * @param index フレームインデックス
 * @param scr   スクリーンデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_FrameSet( BGWINFRM_WORK * wk, u32 index, u16 * scr )
{
  BGWINFRM * win = &wk->win[index];

  GFL_STD_MemCopy16( scr, win->scr, win->sx*win->sy*2 );
}
//--------------------------------------------------------------------------------------------
/**
 * フレームにアーカイブスクリーンデータをセット
 *
 * @param wk      フレームワーク
 * @param index   フレームインデックス
 * @param fileIdx   アーカイブファイルインデックス
 * @param dataIdx   データインデックス
 * @param comp    圧縮フラグ
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_FrameSetArc( BGWINFRM_WORK *wk, int index, ARCID arcID, ARCDATID datID, int comp )
{
  NNSG2dScreenData * scrn;
  void * buf;

  buf = GFL_ARC_UTIL_LoadScreen( arcID, datID, comp, &scrn, wk->heapID );

  BGWINFRM_FrameSet( wk, index, (void *)scrn->rawData );

  GFL_HEAP_FreeMemory( buf );
}
//----------------------------------------------------------------------------
/**
 *  @brief  フレームプット
 *
 *  @param  BGWINFRM_WORK *wk ワーク
 *  @param  index             インデックス
 *  @param  px                位置X
 *  @param  py                位置Y
 */
//-----------------------------------------------------------------------------
void BGWINFRM_FramePut( BGWINFRM_WORK *wk, int index, int px, int py )
{
  BGWINFRM * win;
  u8  wpx, wpy, wsx, wsy, bpx, bpy;

  win = &wk->win[index];

  win->px = px;
  win->py = py;

  if( px >= win->area_rx ||
    py >= win->area_dy ||
    px + win->sx < win->area_lx ||
    py + win->sy < win->area_uy ){
    return;
  }

  wpx = px;
  wsx = win->sx;
  bpx = 0;

  if( px < win->area_lx ){
    wpx = win->area_lx;
    wsx -= ( win->area_lx - px );
    bpx = win->area_lx - px;
  }
  if( px + win->sx >= win->area_rx ){
    wsx -= ( px + win->sx - win->area_rx );
  }

  wpy = py;
  wsy = win->sy;
  bpy = 0;

  if( py < win->area_uy ){
    wpy = win->area_uy;
    wsy -= ( win->area_uy - py );
    bpy = win->area_uy - py;
  }
  if( py + win->sy >= win->area_dy ){
    wsy -= ( py + win->sy - win->area_dy );
  }

  GFL_BG_WriteScreenExpand( win->frm, wpx, wpy, wsx, wsy, win->scr, bpx, bpy, win->sx, win->sy );
  GFL_BG_LoadScreenV_Req( win->frm );

  win->vanish = TRUE;
}
//--------------------------------------------------------------------------------------------
/**
 * フレームのスクリーンを取得
 *
 * @param wk      フレームワーク
 * @param index   フレームインデックス
 *
 * @return  スクリーンのポインタ
 */
//--------------------------------------------------------------------------------------------
u16 * BGWINFRM_FrameBufGet( BGWINFRM_WORK *wk, int index )
{
  return wk->win[index].scr;
}
//----------------------------------------------------------------------------
/**
 *  @brief  フレーム表示
 *
 *  @param  BGWINFRM_WORK *wk ワーク
 *  @param  index             インデックス
 */
//-----------------------------------------------------------------------------
void BGWINFRM_FrameOn( BGWINFRM_WORK *wk, int index )
{
  BGWINFRM_FramePut( wk, index, wk->win[index].px, wk->win[index].py );
}
//--------------------------------------------------------------------------------------------
/**
 * フレームを非表示
 *
 * @param wk      フレームワーク
 * @param index   フレームインデックス
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_FrameOff( BGWINFRM_WORK *wk, int index )
{
  BGWINFRM * win;
  s8  px, py, sx, sy;

  win = &wk->win[index];

  win->vanish = FALSE;

  if( win->px >= win->area_rx ||
    win->py >= win->area_dy ||
    win->px + win->sx < win->area_lx ||
    win->py + win->sy < win->area_uy ){
    return;
  }

  px = win->px;
  sx = win->sx;

  if( win->px < win->area_lx ){
    px = win->area_lx;
    sx -= ( win->area_lx - win->px );
  }
  if( win->px + win->sx >= win->area_rx ){
    sx -= ( win->px + win->sx - win->area_rx );
  }

  py = win->py;
  sy = win->sy;

  if( win->py < win->area_uy ){
    py = win->area_uy;
    sy -= ( win->area_uy - win->py );
  }
  if( win->py + win->sy >= win->area_dy ){
    sy -= ( win->py + win->sy - win->area_dy );
  }

  GFL_BG_FillScreen( win->frm, 0, px, py, sx, sy, GFL_BG_SCRWRT_PALNL );
  GFL_BG_LoadScreenV_Req( win->frm );
}
#endif
