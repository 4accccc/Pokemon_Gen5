//=============================================================================
/**
 * @file	  pokemontrade_disptool.c
 * @bfief	  ポケモン交換グラフィック部分
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/12/18
 */
//=============================================================================


#include <gflib.h>
#include "arc_def.h"
#include "poke_tool/monsno_def.h"

#include "trade.naix"
#include "poke_icon.naix"

#include "net_app/pokemontrade.h"
#include "pokemontrade_local.h"
#include "msg/msg_poke_trade.h"
#include "gamesystem/msgspeed.h"  //MSGSPEED_GetWait
#include "font/font.naix"    // NARC_font_default_nclr

#include "system/bmp_winframe.h"
#include "savedata/box_savedata.h"
#include "pokeicon/pokeicon.h"
#include "app/app_menu_common.h"

#include "ircbattle.naix"
#include "trade.naix"
#include "tradedemo.naix"
#include "tradeirdemo.naix"
#include "app_menu_common.naix"






//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR初期化
 *  @param  POKEMON_TRADE_WORK
 *  @return none
 */
//-----------------------------------------------------------------------------

void POKETRADE_TOUCHBAR_Init(POKEMON_TRADE_WORK* pWork)
{
  //アイコンの設定
  //数分作る
  TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]  =
  {
    {
      TOUCHBAR_ICON_RETURN,
      { TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
    },
    {
      TOUCHBAR_ICON_CUR_L,
      { TOUCHBAR_ICON_X_00, TOUCHBAR_ICON_Y },
    },
    {
      TOUCHBAR_ICON_CUR_R,
      { TOUCHBAR_ICON_X_01, TOUCHBAR_ICON_Y },
    },
    {
      TOUCHBAR_ICON_CUTSOM1,
      { TOUCHBAR_ICON_X_00+8+4, TOUCHBAR_ICON_Y },
    },
    {
      TOUCHBAR_ICON_CUTSOM2,
      { TOUCHBAR_ICON_X_06+4, TOUCHBAR_ICON_Y },
    },
  };

  TOUCHBAR_SETUP  touchbar_setup;
  GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

  touchbar_setup.p_item   = touchbar_icon_tbl;        //上の窓情報
  touchbar_setup.item_num = NELEMS(touchbar_icon_tbl);//いくつ窓があるか
  touchbar_setup.p_unit   = pWork->cellUnit;                    //OBJ読み込みのためのCLUNIT
  touchbar_setup.is_notload_bg = TRUE;  //BGはなし
  touchbar_setup.bar_frm  = GFL_BG_FRAME0_S;            //BG読み込みのためのBG面上下画面判定にも必要
  touchbar_setup.bg_plt   = _TOUCHBAR_BG_PALPOS;      //BGﾊﾟﾚｯﾄ
  touchbar_setup.obj_plt  = 0;      //OBJﾊﾟﾚｯﾄ
  touchbar_setup.mapping  = APP_COMMON_MAPPING_128K;  //マッピングモード

  touchbar_icon_tbl[3].cg_idx = pWork->cellRes[CHAR_SCROLLBAR];
  touchbar_icon_tbl[3].plt_idx = pWork->cellRes[PAL_SCROLLBAR];
  touchbar_icon_tbl[3].cell_idx = pWork->cellRes[ANM_SCROLLBAR];
  touchbar_icon_tbl[3].active_anmseq  = 6;            //アクティブのときのアニメ
  touchbar_icon_tbl[3].noactive_anmseq  =   5;            //ノンアクティブのときのアニメ
  touchbar_icon_tbl[3].push_anmseq  =   4;            //押したときのアニメ（STOPになっていること）
  touchbar_icon_tbl[3].key  =   0;    //キーで押したときに動作させたいならば、ボタン番号
  touchbar_icon_tbl[3].se =   POKETRADESE_DECIDE;                 //押したときにSEならしたいならば、SEの番号

  touchbar_icon_tbl[4].cg_idx = pWork->cellRes[CHAR_SCROLLBAR];
  touchbar_icon_tbl[4].plt_idx = pWork->cellRes[PAL_SCROLLBAR];
  touchbar_icon_tbl[4].cell_idx = pWork->cellRes[ANM_SCROLLBAR];
  touchbar_icon_tbl[4].active_anmseq  = 9;            //アクティブのときのアニメ
  touchbar_icon_tbl[4].noactive_anmseq  =   8;            //ノンアクティブのときのアニメ
  touchbar_icon_tbl[4].push_anmseq  =   7;            //押したときのアニメ（STOPになっていること）
  touchbar_icon_tbl[4].key  =   0;    //キーで押したときに動作させたいならば、ボタン番号
  touchbar_icon_tbl[4].se =   POKETRADESE_DECIDE;                 //押したときにSEならしたいならば、SEの番号

  pWork->pTouchWork = TOUCHBAR_Init(&touchbar_setup, pWork->heapID);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  TOUCHBAR_SetSoftPriorityAll( pWork->pTouchWork, 2 );

  
}





