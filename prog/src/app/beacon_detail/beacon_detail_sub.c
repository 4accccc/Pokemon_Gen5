//=============================================================================
/**
 *
 *	@file		beacon_detail_sub.c
 *	@brief  すれ違い詳細画面 サブ
 *	@author	Miyuki Iwasawa
 *	@data		2010.02.01
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// 簡易会話表示システム
#include "system/pms_draw.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "beacon_detail_gra.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/beacon_detail.h"



//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
#include "msg/msg_beacon_detail.h"  // GMM
#include "beacon_status.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "wifi_unionobj.naix"
#include "wmi.naix"

#include "beacon_detail_local.h"
#include "beacon_detail_sub.h"

static void draw_BeaconWindowIni( BEACON_DETAIL_WORK* wk );


/*
 *  @brief  ビーコン詳細画面初期描画
 */
void BeaconDetail_InitialDraw( BEACON_DETAIL_WORK* wk )
{
  draw_BeaconWindowIni( wk );

}

/*
 *  @brief  ビーコンウィンドウ初期描画
 */
static void draw_BeaconWindowIni( BEACON_DETAIL_WORK* wk )
{
  int i,j;
  BEACON_WIN* bp;

  //プロフィール見出し
  for(i = 0;i < BEACON_PROF_MAX;i++){
    GFL_MSG_GetString( wk->msg, msg_prof01+i, wk->str_tmp);
  
    for(j = 0;j < BEACON_WIN_MAX;j++){
      bp = &wk->beacon_win[j];

      PRINT_UTIL_PrintColor( &bp->prof[i].putil, wk->print_que,
          0, 0, wk->str_tmp, wk->font, FCOL_WIN01 );
      GFL_BMPWIN_MakeTransWindow( bp->prof[i].win );
    }
  }
  //レコード見出し
  GFL_MSG_GetString( wk->msg, msg_record, wk->str_tmp);
  
  for(i = 0;i < BEACON_WIN_MAX;i++){
    bp = &wk->beacon_win[i];

    PRINT_UTIL_PrintColor( &bp->record.putil, wk->print_que,
          0, 0, wk->str_tmp, wk->font, FCOL_WIN01 );
    GFL_BMPWIN_MakeTransWindow( bp->record.win );
  }
}

/*
 *  @brief  ビーコンウィンドウ描画
 */
static void draw_BeaconWindow( BEACON_DETAIL_WORK* wk, u8 idx )
{
  BEACON_WIN* bp = &wk->beacon_win[idx];


}
