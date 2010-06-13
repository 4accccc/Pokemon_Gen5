//==============================================================================
/**
 * @file  codein_disp.c
 * @brief 文字入力インターフェース
 * @author  goto / Ariizumi(移植）/ mori
 * @date  2009.03.27
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

#include "print/wordset.h"
#include "print/printsys.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"

#include "sound/pm_sndsys.h"

#include "codein_pv.h"

#define USE_FontOam 0


//--------------------------------------------------------------
// 定義
//--------------------------------------------------------------
// 「けす」「おわり」を表示するための定義（８０ドットの文字列まではセンタリングで表示可能）
#define BUTTON_STR_W_NUM  ( 10 )      ///< ボタンに貼り付けるBMPのキャラ幅
#define BUTTON_STR_H_NUM  (  2 )      ///< ボタンに貼り付けるBMPWINのキャラ縦幅
#define BUTTON_STR_W_DOT  ( BUTTON_STR_W_NUM*8 )  
#define BUTTON_WIDTH      ( 128 )     ///< 画面に表示されるボタンの幅

//--------------------------------------------------------------
/**
 * @brief CLACT の 初期化
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CATS_Init( CODEIN_WORK* wk , const GFL_DISP_VRAM* vramBank )
{

  GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
  cellSysInitData.oamst_main = 0x10;  //デバッグメータの分
  cellSysInitData.oamnum_main = 128-0x10;
  GFL_CLACT_SYS_Create( &cellSysInitData , vramBank ,HEAPID_CODEIN );
  
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  wk->sys.cellUnit  = GFL_CLACT_UNIT_Create( 64 , 0, HEAPID_CODEIN );
  GFL_CLACT_UNIT_SetDefaultRend( wk->sys.cellUnit );
}

//--------------------------------------------------------------
/**
 * @brief コード用データ読み込み
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CodeRes_Load( CODEIN_WORK* wk )
{
  ARCHANDLE*    hdl;
  GFL_CLUNIT    *cellUnit;
  PALETTE_FADE_PTR pfd;

  cellUnit = wk->sys.cellUnit;
  pfd = wk->sys.pfd;  
  hdl = wk->sys.p_handle;

  ///< bg
  GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl , NARC_code_input_code_bg_NCGR , GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_CODEIN );
  GFL_ARCHDL_UTIL_TransVramScreen( hdl , NARC_code_input_code_bg_NSCR, GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_CODEIN );
  //090918 2本に変更しました。 Ariizumi
  PaletteWorkSet_Arc( pfd, ARCID_CODE_INPUT, NARC_code_input_code_bg_NCLR, HEAPID_CODEIN, FADE_MAIN_BG, 0x20 * 2, 0 );
  
  ///< bg-top
  GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl , NARC_code_input_code_bg_t_NCGR , GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_CODEIN );
  GFL_ARCHDL_UTIL_TransVramScreen( hdl , NARC_code_input_code_bg_t_NSCR, GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_CODEIN );
  GFL_ARCHDL_UTIL_TransVramPalette( hdl , NARC_code_input_code_bg_t_NCLR , PALTYPE_SUB_BG , 0, 0, HEAPID_CODEIN );
  
  ///< code
  wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CODE] = GFL_CLGRP_PLTT_Register( hdl , NARC_code_input_code_NCLR , CLSYS_DRAW_MAIN , 0,  HEAPID_CODEIN );
  wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CODE] = GFL_CLGRP_CGR_Register( hdl , NARC_code_input_code_NCGR , FALSE , CLSYS_DRAW_MAIN , HEAPID_CODEIN );
  wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CODE] = GFL_CLGRP_CELLANIM_Register( hdl , NARC_code_input_code_NCER , NARC_code_input_code_NANR , HEAPID_CODEIN );
  
  ///< cur
  wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] = GFL_CLGRP_PLTT_Register( hdl , NARC_code_input_code_cur_NCLR , CLSYS_DRAW_MAIN , 0x20,  HEAPID_CODEIN );
  wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] = GFL_CLGRP_CGR_Register( hdl , NARC_code_input_code_cur_NCGR , FALSE , CLSYS_DRAW_MAIN , HEAPID_CODEIN );
  wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] = GFL_CLGRP_CELLANIM_Register( hdl , NARC_code_input_code_cur_NCER , NARC_code_input_code_cur_NANR , HEAPID_CODEIN );

  ///< button
  wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON] = GFL_CLGRP_PLTT_Register( hdl , NARC_code_input_code_button_NCLR , CLSYS_DRAW_MAIN , 0x40,  HEAPID_CODEIN );
  wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_BUTTON] = GFL_CLGRP_CGR_Register( hdl , NARC_code_input_code_button_NCGR , FALSE , CLSYS_DRAW_MAIN , HEAPID_CODEIN );
  wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_BUTTON] = GFL_CLGRP_CELLANIM_Register( hdl , NARC_code_input_code_button_NCER , NARC_code_input_code_button_NANR , HEAPID_CODEIN );
  
  ///< font
  {
    BmpWinFrame_GraphicSet( GFL_BG_FRAME0_S , 1 , ePAL_FRAME , MENU_TYPE_SYSTEM , HEAPID_CODEIN );
    ///< フォント
    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , ePAL_FONT*0x20, 16*2, HEAPID_CODEIN );
  }
}


//--------------------------------------------------------------
/**
 * @brief もろもろ解放
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CodeRes_Delete( CODEIN_WORK* wk )
{
  int i;
  
  for ( i = 0; i < wk->code_max; i++ ){
    GFL_CLACT_WK_Remove( wk->code[ i ].clwk );
  }
  
  for ( i = 0; i < 2; i++ ){
    GFL_CLACT_WK_Remove( wk->bar[ i ].clwk );
    GFL_CLACT_WK_Remove( wk->btn[ i ].clwk );
  }
  
  for ( i = 0; i < 3; i++ ){
    GFL_CLACT_WK_Remove( wk->cur[ i ].clwk );
  }
  
  CI_pv_FontOam_SysDelete( wk );
  
  GFL_BMPWIN_Delete( wk->sys.win );
  
  //リソースの開放
  for( i=0;i<CIO_RES2_MAX;i++ )
  {
    GFL_CLGRP_PLTT_Release( wk->sys.resIdx[CIO_RES1_PLT][i] );
    GFL_CLGRP_CGR_Release( wk->sys.resIdx[CIO_RES1_NCG][i] );
    GFL_CLGRP_CELLANIM_Release( wk->sys.resIdx[CIO_RES1_ANM][i] );
  }
  
  GFL_CLACT_UNIT_Delete( wk->sys.cellUnit );
  GFL_CLACT_SYS_Delete();
  
}

//--------------------------------------------------------------
/**
 * @brief コードOAM作成
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CodeOAM_Create( CODEIN_WORK* wk )
{
  int i;
  int i_c = 0;
  int i_b = 0;
  
  GFL_CLWK_DATA clwkInit;
  GFL_CLUNIT    *cellUnit;
  PALETTE_FADE_PTR pfd;

  cellUnit = wk->sys.cellUnit;
  pfd = wk->sys.pfd;

  clwkInit.pos_x = 0;
  clwkInit.pos_y = 0;
  clwkInit.anmseq = 0;
  clwkInit.softpri = 10;
  clwkInit.bgpri = 0;
  
  for ( i = 0; i < wk->code_max + BAR_OAM_MAX; i++ ){
    
    if ( i == ( wk->bar[ i_b ].state + i_b + 1 ) ){
      
      clwkInit.pos_x = 72 + ( i * 8 ) + 4;
      clwkInit.pos_y = POS_CODE_Y;
      clwkInit.anmseq = eANM_CODE_BAR;

      wk->bar[ i_b ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CODE] ,
                          wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CODE] , 
                          wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CODE] ,
                          &clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
      i_b++;
    }
    else {      
      clwkInit.pos_x = 72 + ( i * 8 ) + 4;
      clwkInit.pos_y = POS_CODE_Y;
      clwkInit.anmseq = CI_pv_disp_CodeAnimeGet( wk->code[ i_c ].state, wk->code[ i_c ].size );
      wk->code[ i_c ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CODE] ,
                          wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CODE] , 
                          wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CODE] ,
                          &clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
      
      i_c++;
    }
  }

  // 入力ブロックが最初の１ブロックしか使用しない場合はバー「-」を消す
  if( wk->param.block[1]==0 && wk->param.block[2]==0){
    GFL_CLACT_WK_SetDrawEnable( wk->bar[ 0 ].clwk, FALSE );
    GFL_CLACT_WK_SetDrawEnable( wk->bar[ 1 ].clwk, FALSE );
  }
  
}

//--------------------------------------------------------------
/**
 * @brief カーソルOAMの作成
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurOAM_Create( CODEIN_WORK* wk )
{ 
  GFL_CLWK_DATA clwkInit;
  GFL_CLUNIT    *cellUnit;
  PALETTE_FADE_PTR pfd;

  cellUnit = wk->sys.cellUnit;
  pfd = wk->sys.pfd;
  
  clwkInit.pos_x = 0;
  clwkInit.pos_y = 0;
  clwkInit.anmseq = 0;
  clwkInit.softpri = 0;
  clwkInit.bgpri = 0;
  
  wk->cur[ 0 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] ,
                      wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] , 
                      wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] ,
                      &clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
  wk->cur[ 1 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] ,
                      wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] , 
                      wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] ,
                      &clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
  wk->cur[ 2 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] ,
                      wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] , 
                      wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] ,
                      &clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
  
  CI_pv_disp_CurBar_PosSet( wk, 0 );
  GFL_CLACT_WK_SetAnmSeq( wk->cur[ 0 ].clwk, 0 );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->cur[ 0 ].clwk, TRUE );
  
  wk->cur[ 1 ].move_wk.pos.x = 0;
  wk->cur[ 1 ].move_wk.pos.y = 0;
  wk->cur[ 1 ].state = eANM_CUR_SQ;
  CI_pv_disp_CurSQ_PosSet( wk, 0 );
  GFL_CLACT_WK_SetAnmSeq( wk->cur[ 1 ].clwk, wk->cur[ 1 ].state );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->cur[ 1 ].clwk, TRUE );
  GFL_CLACT_WK_SetObjMode( wk->cur[ 1 ].clwk, GX_OAM_MODE_XLU );
  
  wk->cur[ 2 ].move_wk.pos.x = 0;
  wk->cur[ 2 ].move_wk.pos.y = 0;
  wk->cur[ 2 ].state = eANM_CUR_SQ;
  CI_pv_disp_CurSQ_PosSet( wk, 0 );
  GFL_CLACT_WK_SetAnmSeq( wk->cur[ 2 ].clwk, wk->cur[ 2 ].state );
  GFL_CLACT_WK_SetObjMode( wk->cur[ 2 ].clwk, GX_OAM_MODE_XLU );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->cur[ 2 ].clwk, TRUE );
  
  CI_pv_disp_CurOAM_Visible( wk, 1, FALSE );
  CI_pv_disp_CurOAM_Visible( wk, 2, FALSE );
}


//--------------------------------------------------------------
/**
 * @brief Button OAM 作成
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_BtnOAM_Create( CODEIN_WORK* wk )
{ 
  GFL_CLWK_DATA clwkInit;
  GFL_CLUNIT    *cellUnit;
  PALETTE_FADE_PTR pfd;

  cellUnit = wk->sys.cellUnit;
  pfd = wk->sys.pfd;
  
  clwkInit.pos_x = 0;
  clwkInit.pos_y = 0;
  clwkInit.anmseq = 0;
  clwkInit.softpri = 10;
  clwkInit.bgpri = 0;
  
//  coap.pal    = 0;
  wk->btn[ 0 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_BUTTON] ,
                      wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON] , 
                      wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_BUTTON] ,
                      &clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
  
//  coap.pal    = 1;
  wk->btn[ 1 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_BUTTON] ,
                      wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON] , 
                      wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_BUTTON] ,
                      &clwkInit , CLSYS_DEFREND_MAIN , HEAPID_CODEIN );
  {
    GFL_CLACTPOS pos;
    
    pos.x = ( wk->sys.rht[ eHRT_BACK ].rect.left + wk->sys.rht[ eHRT_BACK ].rect.right ) / 2;
    pos.y = ( wk->sys.rht[ eHRT_BACK ].rect.top  + wk->sys.rht[ eHRT_BACK ].rect.bottom ) / 2;  

    GFL_CLACT_WK_SetPos( wk->btn[ 0 ].clwk, &pos , CLSYS_DEFREND_MAIN );
    GFL_CLACT_WK_SetAnmSeq( wk->btn[ 0 ].clwk, 0 );

    pos.x = ( wk->sys.rht[ eHRT_END ].rect.left + wk->sys.rht[ eHRT_END ].rect.right ) / 2;
    pos.y = ( wk->sys.rht[ eHRT_END ].rect.top  + wk->sys.rht[ eHRT_END ].rect.bottom ) / 2;  
    
    GFL_CLACT_WK_SetPos( wk->btn[ 1 ].clwk, &pos , CLSYS_DEFREND_MAIN );
    GFL_CLACT_WK_SetAnmSeq( wk->btn[ 1 ].clwk, 0 );
  }
}


//--------------------------------------------------------------
/**
 * @brief カーソルOAMのONOFF
 *
 * @param wk  
 * @param no  
 * @param flag  TRUE = ON
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurOAM_Visible( CODEIN_WORK* wk, int no, BOOL flag )
{
  GFL_CLACT_WK_SetDrawEnable( wk->cur[ no ].clwk , flag );
}

//--------------------------------------------------------------
/**
 * @brief カーソルのONOFF取得
 *
 * @param wk  
 * @param no  
 *
 * @retval  BOOL  
 *
 */
//--------------------------------------------------------------
BOOL CI_pv_disp_CurOAM_VisibleGet( CODEIN_WORK* wk, int no )
{
  return  GFL_CLACT_WK_GetDrawEnable( wk->cur[ no ].clwk );
}


//--------------------------------------------------------------
/**
 * @brief アニメ切り替え
 *
 * @param wk  
 * @param no  
 * @param anime 
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurOAM_AnimeChange( CODEIN_WORK* wk, int no, int anime )
{
  GFL_CLACT_WK_SetAnmSeq( wk->cur[ no ].clwk, anime );
}


//--------------------------------------------------------------
/**
 * @brief カーソルの位置を入力位置に
 *
 * @param wk  
 * @param id  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurBar_PosSet( CODEIN_WORK* wk, int id )
{
  GFL_CLACTPOS pos;
  GFL_CLWK *clwk;
  
  clwk = wk->code[ id ].clwk;
  
  wk->cur[ 0 ].state = id;
  
  GFL_CLACT_WK_GetPos( clwk, &pos , CLSYS_DEFREND_MAIN );
  pos.y += CUR_BAR_OY;
  GFL_CLACT_WK_SetPos( wk->cur[ 0 ].clwk, &pos , CLSYS_DEFREND_MAIN );
}

//--------------------------------------------------------------
/**
 * @brief カーソル位置をパネルに
 *
 * @param wk  
 * @param id  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurSQ_PosSet( CODEIN_WORK* wk, int id )
{
  GFL_CLACTPOS pos;
  
  pos.x = ( wk->sys.rht[ id + eHRT_NUM_0 ].rect.left + wk->sys.rht[ id + eHRT_NUM_0 ].rect.right ) / 2;
  pos.y = ( wk->sys.rht[ id + eHRT_NUM_0 ].rect.top  + wk->sys.rht[ id + eHRT_NUM_0 ].rect.bottom ) / 2;
  
  GFL_CLACT_WK_SetPos( wk->cur[ 1 ].clwk, &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmFrame( wk->cur[ 1 ].clwk, 0 );
}

//--------------------------------------------------------------
/**
 * @brief 
 *
 * @param wk  
 * @param id  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurSQ_PosSetEx( CODEIN_WORK* wk, int id, int cur_id )
{
  GFL_CLACTPOS pos;
  
  pos.x = ( wk->sys.rht[ id + eHRT_NUM_0 ].rect.left + wk->sys.rht[ id + eHRT_NUM_0 ].rect.right ) / 2;
  pos.y = ( wk->sys.rht[ id + eHRT_NUM_0 ].rect.top  + wk->sys.rht[ id + eHRT_NUM_0 ].rect.bottom ) / 2;
  
  GFL_CLACT_WK_SetPos( wk->cur[ cur_id ].clwk, &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmFrame( wk->cur[ cur_id ].clwk, 0 );

}

//--------------------------------------------------------------
/**
 * @brief カーソル更新
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurUpdate( CODEIN_WORK* wk )
{
  
  int id = 2;
  int anime;
  BOOL bAnime;
  
  for ( id = 1; id < 3; id++ ){
    
    anime = GFL_CLACT_WK_GetAnmSeq( wk->cur[ id ].clwk );
    
    if ( anime == eANM_CUR_TOUCH ){
      bAnime = GFL_CLACT_WK_CheckAnmActive( wk->cur[ id ].clwk );
      
      if ( bAnime == FALSE ){
        
        GFL_CLACT_WK_SetAnmSeq( wk->cur[ id ].clwk, wk->cur[ id ].state );
    
        if ( wk->sys.touch == TRUE ){
            CI_pv_disp_CurOAM_Visible( wk, 1, FALSE );
        }
        else {
            CI_pv_disp_CurOAM_Visible( wk, 1, TRUE );
        }
        CI_pv_disp_CurOAM_Visible( wk, 2, FALSE );
      }
    }
    else {
      
      if ( anime != wk->cur[ id ].state ){
        GFL_CLACT_WK_SetAnmSeq( wk->cur[ id ].clwk, wk->cur[ id ].state );
      }
      {
        anime = GFL_CLACT_WK_GetAnmSeq( wk->cur[ id ].clwk );
        if ( anime != eANM_CUR_TOUCH ){
          if ( wk->sys.touch == TRUE ){
            CI_pv_disp_CurOAM_Visible( wk, 1, FALSE );
          }
          else {
            CI_pv_disp_CurOAM_Visible( wk, 1, TRUE );
          } 
        }
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief FONTOAM pos set
 *
 * @param obj 
 * @param ox  
 * @param oy  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
static void FontOamPosSet(BMPOAM_ACT_PTR obj, int ox, int oy)
{
  if (obj != NULL)
  {
    BmpOam_ActorSetPos( obj , ox,oy );
  }
#if USE_FontOam
  int x;
  int y;
  if (obj != NULL){
    FONTOAM_SetMat(obj,  ox,  oy);
  } 
#endif
}

#define BTN_FONT_OAM_OX   ( 24 )
#define BTN_FONT_OAM_OY   ( 7 )
//--------------------------------------------------------------
/**
 * @brief ボタンのアニメ
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_BtnUpdate( CODEIN_WORK* wk )
{
  int i;
  s16 ox, oy;
  
  for ( i = 0; i < 2; i++ ){
  
    ox = wk->sys.rht[ i + eHRT_BACK ].rect.left + (BUTTON_WIDTH/2)-BUTTON_STR_W_DOT/2 -4 ;
    oy = ( wk->sys.rht[ i + eHRT_BACK ].rect.top  + wk->sys.rht[ i + eHRT_BACK ].rect.bottom ) / 2;
    oy -= BTN_FONT_OAM_OY;
    
    switch ( wk->btn[ i ].state ){
    case 0:
      wk->btn[ i ].move_wk.wait = 0;
      break;
      
    case 1:
      wk->btn[ i ].move_wk.wait++;
      if ( wk->btn[ i ].move_wk.wait == 1 ){
        GFL_CLACT_WK_SetAnmSeq( wk->btn[ i ].clwk, 1 );
        FontOamPosSet( wk->sys.bmp_obj[ i ], ox, oy-0);
      }
      else if ( wk->btn[ i ].move_wk.wait == 2 ){
        GFL_CLACT_WK_SetAnmSeq( wk->btn[ i ].clwk, 2 );
        FontOamPosSet( wk->sys.bmp_obj[ i ], ox, oy-0);
      }
      else if ( wk->btn[ i ].move_wk.wait == 10 ){
        GFL_CLACT_WK_SetAnmSeq( wk->btn[ i ].clwk, 0 );
        FontOamPosSet( wk->sys.bmp_obj[ i ], ox, oy+0 );        
        wk->btn[ i ].state++;
      }   
      break;
    
    default:
      GFL_CLACT_WK_SetAnmSeq( wk->btn[ i ].clwk, 0 );
      FontOamPosSet( wk->sys.bmp_obj[ i ], ox, oy );
      wk->btn[ i ].state = 0;
      break;
    }
  }
}


//--------------------------------------------------------------
/**
 * @brief アニメコードを取得
 *
 * @param state 
 * @param size  TRUE = Large
 *
 * @retval  int 
 *
 */
//--------------------------------------------------------------
int CI_pv_disp_CodeAnimeGet( int state, BOOL size )
{
  int anime = eANM_CODE_LN;
  
  if ( size == FALSE ){
    
     anime = eANM_CODE_SN;
  }
  
  anime += state;
  
  return anime;
}

//--------------------------------------------------------------
/**
 * @brief サイズフラグの設定
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_SizeFlagSet( CODEIN_WORK* wk )
{
  int i;
  
  for ( i = 0; i < wk->code_max; i++ ){
    
    if ( i >= wk->ls && i < wk->le ){
      
      wk->code[ i ].size = TRUE;
    }
    else {
      
      wk->code[ i ].size = FALSE;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief 移動先の位置を設定する
 *
 * @param wk  
 * @param mode  0 = set 1 = move_wk set
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_MovePosSet( CODEIN_WORK* wk, int mode )
{
  int i;
  int bp;
  s16 base;
  
  //OS_Printf( "focus %d\n", wk->focus_now );
  base = wk->x_tbl[ wk->focus_now ];
  bp   = 0;
  
  for ( i = 0; i < wk->code_max; i++ ){
    GFL_CLACTPOS pos;

    if ( i >= wk->ls && i < wk->le ){
      
      if ( i == wk->ls ){
        base += M_SIZE;
      }
      else {
        base += L_SIZE;
      }
    }
    else {
      if ( i == 0 ){
        base += M_SIZE;
      }
      else {
        base += S_SIZE;
      }     
    }
    
    GFL_CLACT_WK_GetPos( wk->code[ i ].clwk, &pos , CLSYS_DEFREND_MAIN );
    if ( mode == eMPS_SET ){
      pos.x = base;
      GFL_CLACT_WK_SetPos( wk->code[ i ].clwk, &pos , CLSYS_DEFREND_MAIN );
    }
    else {
      wk->code[ i ].move_wk.pos.x = ( base - pos.x ) / MOVE_WAIT;
      wk->code[ i ].move_wk.pos.y = 0;
      wk->code[ i ].move_wk.wait  = MOVE_WAIT;
      wk->code[ i ].move_wk.scale = 0;
    }
    
    if ( i == wk->bar[ bp ].state && bp != BAR_OAM_MAX ){
      GFL_CLACTPOS pos;
      
      GFL_CLACT_WK_GetPos( wk->bar[ bp ].clwk, &pos , CLSYS_DEFREND_MAIN );
      
      if ( wk->ls == wk->le ){
        base += S_SIZE;
      }
      else {
        if ( i > wk->ls && i < wk->le ){
          
          base += M_SIZE;
        }
        else {
          
          base += S_SIZE;
        }
      }
      
      if ( mode == eMPS_SET ){
        pos.x = base;
        GFL_CLACT_WK_SetPos( wk->bar[ bp ].clwk, &pos , CLSYS_DEFREND_MAIN );
      }
      else {
        
        wk->bar[ bp ].move_wk.pos.x = ( base - pos.x ) / MOVE_WAIT;
        wk->bar[ bp ].move_wk.pos.y = 0;
        wk->bar[ bp ].move_wk.wait  = MOVE_WAIT;
      }
      bp++;
    }
  }
  
}

//--------------------------------------------------------------
/**
 * @brief 当たり判定の作成
 *
 * @param wk  
 * @param no  
 * @param sx  
 * @param sy  
 *
 * @retval  static inline void  
 *
 */
//--------------------------------------------------------------
static inline void CODE_HitRectSet( CODEIN_WORK* wk, int no, s16 sx, s16 sy )
{
  GFL_CLACTPOS pos;
  CODE_OAM* code; 
  
  code = &wk->code[ no ];
  
  GFL_CLACT_WK_GetPos( code->clwk, &pos , CLSYS_DEFREND_MAIN );
  
  code->hit->rect.top   = pos.y - sy;
  code->hit->rect.left  = pos.x - sx;
  code->hit->rect.bottom  = pos.y + sy;
  code->hit->rect.right = pos.x + sx;
}

//--------------------------------------------------------------
/**
 * @brief 当たり判定の設定
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_HitTableSet( CODEIN_WORK* wk )
{
  int i;
  s16 sx;
  s16 sy;
  
  for ( i = 0; i < wk->code_max; i++ ){
        
    if ( i >= wk->ls && i < wk->le ){
      
      sx = L_SIZE / 2;
      sy = L_SIZE / 2;
    }
    else {
      
      sx = S_SIZE / 2;
      sy = S_SIZE;
    }
    
    CODE_HitRectSet( wk, i, sx, sy );
  }
}


#include "msg/msg_code_input.h"

#define PRINT_COL_BLACK   (GF_PRINTCOLOR_MAKE( 15, 13, 2))    // フォントカラー：白
#define PRINT_COL_BUTTON  (PRINTSYS_LSB_Make( 10,  9, 0))   // フォントカラー：黒
#define PRINT_NUM_CLEAR   (0x00)
#define PRINT_NUM_CLEAR_2 (0xEE)

//--------------------------------------------------------------
/**
 * @brief 
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_SysInit(CODEIN_WORK* wk)
{
  wk->sys.bmpoam_sys = BmpOam_Init( HEAPID_CODEIN , wk->sys.cellUnit );
}

//--------------------------------------------------------------
/**
 * @brief 
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_SysDelete(CODEIN_WORK* wk)
{
  GFL_CLGRP_PLTT_Release( wk->sys.bmpOamPltIdx );

  BmpOam_ActorDel( wk->sys.bmp_obj[0] );
  BmpOam_ActorDel( wk->sys.bmp_obj[1] );
  
  BmpOam_Exit( wk->sys.bmpoam_sys );
#if USE_FontOam
  FontProc_UnloadFont( FONT_BUTTON );
  
  FONTOAM_Delete(wk->sys.font_obj[ 0 ]);
  CharVramAreaFree(&wk->sys.font_vram[ 0 ]);
  
  FONTOAM_Delete(wk->sys.font_obj[ 1 ]);
  CharVramAreaFree(&wk->sys.font_vram[ 1 ]);
  
  FONTOAM_SysDelete(wk->sys.fontoam_sys);     
#endif
}

//--------------------------------------------------------------
/**
 * @brief 
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_ResourceLoad(CODEIN_WORK* wk)
{
  ARCHANDLE *hdl = GFL_ARC_OpenDataHandle( ARCID_FONT , HEAPID_CODEIN );
  wk->sys.bmpOamPltIdx = GFL_CLGRP_PLTT_RegisterComp( hdl , NARC_font_default_nclr , CLSYS_DRAW_MAIN , 0x20*4,  HEAPID_CODEIN );
  GFL_ARC_CloseDataHandle( hdl );
#if USE_FontOam
  CATS_LoadResourcePlttWorkArc(wk->sys.pfd, FADE_MAIN_OBJ,
                 wk->sys.csp, wk->sys.crp,
                 ARC_FONT, NARC_font_talk_ncrl, FALSE,
                 1, NNS_G2D_VRAM_TYPE_2DMAIN, eID_FNT_OAM );  
#endif
}

//--------------------------------------------------------------
/**
 * @brief 
 *
 * @param wk  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_Add( CODEIN_WORK* wk )
{
  CI_pv_FontOam_ResourceLoad(wk);
  CI_pv_FontOam_Create(wk, 0,  78, 165, 0);
  CI_pv_FontOam_Create(wk, 1, 172, 165, 0);
}

//--------------------------------------------------------------
/**
 * @brief 
 *
 * @param wk  
 * @param flag  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_Enable(CODEIN_WORK* wk, BOOL flag)
{
#if USE_FontOam
  FONTOAM_SetDrawFlag(wk->sys.font_obj[ 0 ], flag);
  FONTOAM_SetDrawFlag(wk->sys.font_obj[ 1 ], flag);
#endif
}



//--------------------------------------------------------------
/**
 * @brief 
 *
 * @param wk  
 * @param no  
 * @param x 
 * @param y 
 * @param pal_offset  
 *
 * @retval  none  
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_Create(CODEIN_WORK* wk, int no, int x, int y, int pal_offset)
{
  s16 ox, oy;
  BMPOAM_ACT_DATA finit;
  STRBUF*     str;
  int       pal_id;
  int       vram_size;
  GFL_MSGDATA*  man;
  GFL_BMP_DATA  *bmp;
  
  
  
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_code_input_dat, HEAPID_CODEIN );
  str = GFL_MSG_CreateString( man, msg_04 + no );
    

  ///< BMP
  {
    int length;
    ///< FONT_BUTTON は 2dotでかい
    bmp    = GFL_BMP_Create( BUTTON_STR_W_NUM, BUTTON_STR_H_NUM, GFL_BMP_16_COLOR, HEAPID_CODEIN);
    length = PRINTSYS_GetStrWidth( str, wk->sys.fontHandle, 0 );
    OS_Printf("str width=%d\n",length);
    PRINTSYS_PrintColor( bmp , (BUTTON_STR_W_DOT-length)/2,
                         0,str,wk->sys.fontHandle, PRINTSYS_LSB_Make( 15, 14, 0) );
  }

  pal_id = eID_FNT_OAM;

  // BMPOAMの表示位置決定
  ox  = wk->sys.rht[ no + eHRT_BACK ].rect.left + (BUTTON_WIDTH/2)-BUTTON_STR_W_DOT/2 -4 ;
  oy  = ( wk->sys.rht[ no + eHRT_BACK ].rect.top  + wk->sys.rht[ no + eHRT_BACK ].rect.bottom ) / 2;
  oy  -= BTN_FONT_OAM_OY;

  // BMPOAM初期設定
  finit.bmp = bmp;
  finit.x   = ox;
  finit.y   = oy;
  finit.pltt_index = wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON];
  finit.pal_offset = 0;
  finit.soft_pri   = 0;
  finit.bg_pri     = 0;
  finit.setSerface = CLSYS_DEFREND_MAIN;
  finit.draw_type  = CLSYS_DRAW_MAIN;
  
  OS_Printf("bmpOamPltIdx=%d\n",wk->sys.bmpOamPltIdx );
  OS_Printf("resIdx RES2_BUTTOn=%d\n", wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON]);
  
  wk->sys.bmp_obj[ no ] = BmpOam_ActorAdd( wk->sys.bmpoam_sys , &finit );
  BmpOam_ActorBmpTrans( wk->sys.bmp_obj[ no ] );
  GFL_BMP_Delete( bmp );
  GFL_STR_DeleteBuffer(str);
  GFL_MSG_Delete(man);
}


//=============================================================================================
/**
 * @brief   BMPWINに文字列を描画する
 *
 * @param   win   BMPWIN
 * @param   no    文章NO
 * @param   frm   フレーム
 * @param   x     位置X
 * @param   y     位置Y
 * @param   sx    幅W
 * @param   sy    高さ
 * @param   fontHandle    フォント
 * @param   max    入力できる数値の上限
 */
//=============================================================================================
void CI_pv_disp_BMP_WindowAdd( GFL_BMPWIN** win, int no, int frm, 
                               int x, int y, int sx, int sy, GFL_FONT *fontHandle, int max )
{
  *win = GFL_BMPWIN_Create( frm , x, y, sx, sy, ePAL_FONT, GFL_BMP_CHRAREA_GET_B );
           
  BmpWinFrame_Write( *win , WINDOW_TRANS_OFF, 1, ePAL_FRAME );
  
  GFL_BMPWIN_MakeScreen( *win );
  
  CI_pv_BMP_MsgSet( *win, msg_01+no, fontHandle, max );
}


  


#define CODEIN_MSG_SIZE ( 22*2*2 )

//=============================================================================================
/**
 * @brief 文字列描画
 *
 * @param   win         BMPWIN
 * @param   mes_id      メッセージID
 * @param   fontHandle  フォントハンドル
 * @param   max         入力できる数値の上限
 */
//=============================================================================================
void CI_pv_BMP_MsgSet( GFL_BMPWIN * win, int mes_id , GFL_FONT *fontHandle, int max )
{
  GFL_MSGDATA *man;
  WORDSET     *wset;
  STRBUF* str,*expand;
  
  // 文字列取得＆加工
  wset   = WORDSET_Create( HEAPID_CODEIN );
  man    = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_code_input_dat, HEAPID_CODEIN );
  str    = GFL_MSG_CreateString( man, mes_id );
  expand = GFL_STR_CreateBuffer( CODEIN_MSG_SIZE, HEAPID_CODEIN );

  WORDSET_RegisterNumber( wset, 0, max, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_ZENKAKU );
  WORDSET_ExpandStr( wset, expand, str );

  // 描画
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( win ) , 15 );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( win ) , 0,0,expand, fontHandle );

  // 解放
  GFL_STR_DeleteBuffer( expand );
  GFL_STR_DeleteBuffer( str );
  GFL_MSG_Delete( man );  
  WORDSET_Delete( wset );

  // 転送
  GFL_BMPWIN_TransVramCharacter( win );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
}


//=============================================================================================
/**
 * @brief エラー表示に特化した文字列描画ルーチン
 *
 * @param   win   
 * @param   mode    
 * @param   fontHandle    
 * @param   max   
 */
//=============================================================================================
void CI_pv_BMP_ErrorMsgSet( GFL_BMPWIN * win, int mode , GFL_FONT *fontHandle, int max )
{
  if(mode==CODEIN_MODE_TRAIN_NO){
    // そのトレインナンバーはありません
    CI_pv_BMP_MsgSet( win, msg_06, fontHandle, max );
  }else if(mode==CODEIN_MODE_RANK){
    // そのランクはありません
    CI_pv_BMP_MsgSet( win, msg_07, fontHandle, max );
  }
}
