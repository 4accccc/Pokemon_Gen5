//==============================================================================
/**
 * @file	codein_disp.c
 * @brief	文字入力インターフェース
 * @author	goto
 * @date	2007.07.11(水)
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

#include "br_codein_pv.h"


//--------------------------------------------------------------
/**
 * @brief	コード用データ読み込み
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CodeRes_Load( BR_CODEIN_WORK* wk )
{
	ARCHANDLE*		hdl;
	GFL_CLUNIT		*cellUnit;
  BOOL ret;
  BR_RES_OBJ_DATA res;

	cellUnit = wk->sys.cellUnit;
	hdl = wk->sys.p_handle;

	///< code
  ret = BR_RES_GetOBJRes( wk->param.p_res, BR_RES_OBJ_NUM_S, &res );
  GF_ASSERT(ret);

	wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CODE] = res.ncl;
	wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CODE] = res.ncg;
	wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CODE] = res.nce;
	
	///< cur
  ret = BR_RES_GetOBJRes( wk->param.p_res, BR_RES_OBJ_NUM_CURSOR_S, &res );
  GF_ASSERT(ret);

	wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] = res.ncl;
	wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] = res.ncg;
	wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] = res.nce;


	///< button
  ret = BR_RES_GetOBJRes( wk->param.p_res, BR_RES_OBJ_SHORT_BTN_S, &res );
  GF_ASSERT(ret);

	wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON] = res.ncl;
	wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_BUTTON] = res.ncg;
	wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_BUTTON] = res.nce;
	
}


//--------------------------------------------------------------
/**
 * @brief	もろもろ解放
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CodeRes_Delete( BR_CODEIN_WORK* wk )
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
}

//--------------------------------------------------------------
/**
 * @brief	コードOAM作成
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CodeOAM_Create( BR_CODEIN_WORK* wk )
{
	int i;
	int i_c = 0;
	int i_b = 0;
	
	GFL_CLWK_DATA	clwkInit;
	GFL_CLUNIT 		*cellUnit;

	cellUnit = wk->sys.cellUnit;

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
													&clwkInit , CLSYS_DEFREND_SUB , wk->heapID );
      GFL_CLACT_WK_SetObjMode( wk->bar[ i_b ].clwk, GX_OAM_MODE_XLU );
			i_b++;
		}
		else {			
			clwkInit.pos_x = 72 + ( i * 8 ) + 4;
			clwkInit.pos_y = POS_CODE_Y;
			clwkInit.anmseq = CI_pv_disp_CodeAnimeGet( wk->code[ i_c ].state, wk->code[ i_c ].size );
			wk->code[ i_c ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CODE] ,
													wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CODE] , 
													wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CODE] ,
													&clwkInit , CLSYS_DEFREND_SUB , wk->heapID );
			
      GFL_CLACT_WK_SetObjMode( wk->code[ i_c ].clwk, GX_OAM_MODE_XLU );
			i_c++;
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief	カーソルOAMの作成
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurOAM_Create( BR_CODEIN_WORK* wk )
{	
	GFL_CLWK_DATA	clwkInit;
	GFL_CLUNIT 		*cellUnit;

	cellUnit = wk->sys.cellUnit;
	
	clwkInit.pos_x = 0;
	clwkInit.pos_y = 0;
	clwkInit.anmseq = 0;
	clwkInit.softpri = 0;
	clwkInit.bgpri = 0;
	
	wk->cur[ 0 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] ,
											&clwkInit , CLSYS_DEFREND_SUB, wk->heapID );
  GFL_CLACT_WK_SetObjMode( wk->cur[ 0 ].clwk, GX_OAM_MODE_XLU );
	wk->cur[ 1 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] ,
											&clwkInit , CLSYS_DEFREND_SUB, wk->heapID );
  GFL_CLACT_WK_SetObjMode( wk->cur[ 1 ].clwk, GX_OAM_MODE_XLU );
	wk->cur[ 2 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_CURSOL] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_CURSOL] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_CURSOL] ,
											&clwkInit , CLSYS_DEFREND_SUB, wk->heapID );
  GFL_CLACT_WK_SetObjMode( wk->cur[ 2 ].clwk, GX_OAM_MODE_XLU );
	
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
 * @brief	Button OAM 作成
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_BtnOAM_Create( BR_CODEIN_WORK* wk )
{	
	GFL_CLWK_DATA	clwkInit;
	GFL_CLUNIT 		*cellUnit;

	cellUnit = wk->sys.cellUnit;
	
	clwkInit.pos_x = 0;
	clwkInit.pos_y = 0;
	clwkInit.anmseq = 0;
	clwkInit.softpri = 10;
	clwkInit.bgpri = 0;
	
//	coap.pal		= 0;
	wk->btn[ 0 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_BUTTON] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_BUTTON] ,
											&clwkInit , CLSYS_DEFREND_SUB , wk->heapID );
  GFL_CLACT_WK_SetObjMode( wk->btn[ 0 ].clwk, GX_OAM_MODE_XLU );
	
  clwkInit.anmseq = 1;
	wk->btn[ 1 ].clwk = GFL_CLACT_WK_Create( cellUnit , wk->sys.resIdx[CIO_RES1_NCG][CIO_RES2_BUTTON] ,
											wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON] , 
											wk->sys.resIdx[CIO_RES1_ANM][CIO_RES2_BUTTON] ,
											&clwkInit , CLSYS_DEFREND_SUB , wk->heapID );
  GFL_CLACT_WK_SetObjMode( wk->btn[ 1 ].clwk, GX_OAM_MODE_XLU );
	{
		GFL_CLACTPOS pos;
		
		pos.x =  wk->sys.rht[ eHRT_BACK ].rect.left;
		pos.y =  wk->sys.rht[ eHRT_BACK ].rect.top + 16;

		GFL_CLACT_WK_SetPos( wk->btn[ 0 ].clwk, &pos , CLSYS_DEFREND_SUB );
		GFL_CLACT_WK_SetAnmSeq( wk->btn[ 0 ].clwk, 0 );

		pos.x =  wk->sys.rht[ eHRT_END ].rect.left;
		pos.y =  wk->sys.rht[ eHRT_END ].rect.top + 16;	
		
		GFL_CLACT_WK_SetPos( wk->btn[ 1 ].clwk, &pos , CLSYS_DEFREND_SUB );
		GFL_CLACT_WK_SetAnmSeq( wk->btn[ 1 ].clwk, 0 );
	}
}


//--------------------------------------------------------------
/**
 * @brief	カーソルOAMのONOFF
 *
 * @param	wk	
 * @param	no	
 * @param	flag	TRUE = ON
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurOAM_Visible( BR_CODEIN_WORK* wk, int no, BOOL flag )
{
	GFL_CLACT_WK_SetDrawEnable( wk->cur[ no ].clwk , flag );
}

//--------------------------------------------------------------
/**
 * @brief	カーソルのONOFF取得
 *
 * @param	wk	
 * @param	no	
 *
 * @retval	BOOL	
 *
 */
//--------------------------------------------------------------
BOOL CI_pv_disp_CurOAM_VisibleGet( BR_CODEIN_WORK* wk, int no )
{
	return 	GFL_CLACT_WK_GetDrawEnable( wk->cur[ no ].clwk );
}


//--------------------------------------------------------------
/**
 * @brief	アニメ切り替え
 *
 * @param	wk	
 * @param	no	
 * @param	anime	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurOAM_AnimeChange( BR_CODEIN_WORK* wk, int no, int anime )
{
	GFL_CLACT_WK_SetAnmSeq( wk->cur[ no ].clwk, anime );
}


//--------------------------------------------------------------
/**
 * @brief	カーソルの位置を入力位置に
 *
 * @param	wk	
 * @param	id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurBar_PosSet( BR_CODEIN_WORK* wk, int id )
{
	GFL_CLACTPOS pos;
	GFL_CLWK *clwk;
	
	clwk = wk->code[ id ].clwk;
	
	wk->cur[ 0 ].state = id;
	
	GFL_CLACT_WK_GetPos( clwk, &pos , CLSYS_DEFREND_SUB );
	pos.y += CUR_BAR_OY;
	GFL_CLACT_WK_SetPos( wk->cur[ 0 ].clwk, &pos , CLSYS_DEFREND_SUB );
}

//--------------------------------------------------------------
/**
 * @brief	カーソル位置をパネルに
 *
 * @param	wk	
 * @param	id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurSQ_PosSet( BR_CODEIN_WORK* wk, int id )
{
	GFL_CLACTPOS pos;
  int w;
  int h;

  w = wk->sys.rht[ id + eHRT_NUM_0 ].rect.right - wk->sys.rht[ id + eHRT_NUM_0 ].rect.left;
  h = wk->sys.rht[ id + eHRT_NUM_0 ].rect.bottom - wk->sys.rht[ id + eHRT_NUM_0 ].rect.top;
	
	pos.x = wk->sys.rht[ id + eHRT_NUM_0 ].rect.left + w/2;
	pos.y = wk->sys.rht[ id + eHRT_NUM_0 ].rect.top + h/2;
	
	GFL_CLACT_WK_SetPos( wk->cur[ 1 ].clwk, &pos , CLSYS_DEFREND_SUB );
  GFL_CLACT_WK_SetAnmFrame( wk->cur[ 1 ].clwk, 0 );
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 * @param	id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurSQ_PosSetEx( BR_CODEIN_WORK* wk, int id, int cur_id )
{
	GFL_CLACTPOS pos;
  int w;
  int h;

  w = wk->sys.rht[ id + eHRT_NUM_0 ].rect.right - wk->sys.rht[ id + eHRT_NUM_0 ].rect.left;
  h = wk->sys.rht[ id + eHRT_NUM_0 ].rect.bottom - wk->sys.rht[ id + eHRT_NUM_0 ].rect.top;
	
	pos.x = wk->sys.rht[ id + eHRT_NUM_0 ].rect.left + w/2;
	pos.y = wk->sys.rht[ id + eHRT_NUM_0 ].rect.top + h/2;
	
	GFL_CLACT_WK_SetPos( wk->cur[ cur_id ].clwk, &pos , CLSYS_DEFREND_SUB );
  GFL_CLACT_WK_SetAnmFrame( wk->cur[ cur_id ].clwk, 0 );

}

//--------------------------------------------------------------
/**
 * @brief	カーソル更新
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_CurUpdate( BR_CODEIN_WORK* wk )
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
 * @brief	FONTOAM pos set
 *
 * @param	obj	
 * @param	ox	
 * @param	oy	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void FontOamPosSet(BMPOAM_ACT_PTR obj, int ox, int oy)
{
	if (obj != NULL)
	{
		BmpOam_ActorSetPos( obj , ox,oy );
	}
}

//--------------------------------------------------------------
/**
 * @brief	アニメコードを取得
 *
 * @param	state	
 * @param	size	TRUE = Large
 *
 * @retval	int	
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
 * @brief	サイズフラグの設定
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_SizeFlagSet( BR_CODEIN_WORK* wk )
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
 * @brief	移動先の位置を設定する
 *
 * @param	wk	
 * @param	mode	0 = set 1 = move_wk set
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_MovePosSet( BR_CODEIN_WORK* wk, int mode )
{
	int i;
	int bp;
	s16 base;

	base = wk->x_tbl[ wk->focus_now ];
	bp	 = 0;
	
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
		
		GFL_CLACT_WK_GetPos( wk->code[ i ].clwk, &pos , CLSYS_DEFREND_SUB );
		if ( mode == eMPS_SET ){
			pos.x = base;
			GFL_CLACT_WK_SetPos( wk->code[ i ].clwk, &pos , CLSYS_DEFREND_SUB );
		}
		else {
			wk->code[ i ].move_wk.pos.x	= ( base - pos.x ) / MOVE_WAIT;
			wk->code[ i ].move_wk.pos.y	= 0;
			wk->code[ i ].move_wk.wait	= MOVE_WAIT;
			wk->code[ i ].move_wk.scale	= 0;
		}
		
		if ( i == wk->bar[ bp ].state && bp != BAR_OAM_MAX ){
			GFL_CLACTPOS pos;
			
			GFL_CLACT_WK_GetPos( wk->bar[ bp ].clwk, &pos , CLSYS_DEFREND_SUB );
			
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
				GFL_CLACT_WK_SetPos( wk->bar[ bp ].clwk, &pos , CLSYS_DEFREND_SUB );
			}
			else {
				
				wk->bar[ bp ].move_wk.pos.x	= ( base - pos.x ) / MOVE_WAIT;
				wk->bar[ bp ].move_wk.pos.y	= 0;
				wk->bar[ bp ].move_wk.wait	= MOVE_WAIT;
			}
			bp++;
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief	当たり判定の作成
 *
 * @param	wk	
 * @param	no	
 * @param	sx	
 * @param	sy	
 *
 * @retval	static inline void	
 *
 */
//--------------------------------------------------------------
static inline void CODE_HitRectSet( BR_CODEIN_WORK* wk, int no, s16 sx, s16 sy )
{
	GFL_CLACTPOS pos;
	CODE_OAM* code;	
	
	code = &wk->code[ no ];
	
	//CATS_ObjectPosGetCap( code->cap, &x, &y );
	GFL_CLACT_WK_GetPos( code->clwk, &pos , CLSYS_DEFREND_SUB );
	
	code->hit->rect.top		= pos.y - sy;
	code->hit->rect.left	= pos.x - sx;
	code->hit->rect.bottom	= pos.y + sy;
	code->hit->rect.right	= pos.x + sx;
}

//--------------------------------------------------------------
/**
 * @brief	当たり判定の設定
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_disp_HitTableSet( BR_CODEIN_WORK* wk )
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

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_SysInit(BR_CODEIN_WORK* wk)
{
	wk->sys.bmpoam_sys = BmpOam_Init( wk->heapID , wk->sys.cellUnit );
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_SysDelete(BR_CODEIN_WORK* wk)
{

	BmpOam_ActorDel( wk->sys.bmp_obj[0] );
	BmpOam_ActorDel( wk->sys.bmp_obj[1] );
	
	BmpOam_Exit( wk->sys.bmpoam_sys );
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_ResourceLoad(BR_CODEIN_WORK* wk)
{
	ARCHANDLE *hdl = GFL_ARC_OpenDataHandle( ARCID_FONT , wk->heapID );
	GFL_ARC_CloseDataHandle( hdl );
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_Add( BR_CODEIN_WORK* wk )
{
	CI_pv_FontOam_ResourceLoad(wk);
	CI_pv_FontOam_Create(wk, 0,  32, 8, 0);
	CI_pv_FontOam_Create(wk, 1, 32, 8, 0);
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 * @param	flag	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_Enable(BR_CODEIN_WORK* wk, BOOL flag)
{
}

//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 * @param	no	
 * @param	x	
 * @param	y	
 * @param	pal_offset	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CI_pv_FontOam_Create(BR_CODEIN_WORK* wk, int no, int x, int y, int pal_offset)
{
  static const int sc_msgID[ ]=
  { 
    msg_05,
    msg_708,
  };

	s16 ox, oy;
	BMPOAM_ACT_DATA	finit;
	STRBUF*			str;
	int				pal_id;
	int				vram_size;
	GFL_MSGDATA*	man;
	GFL_BMP_DATA 	*bmp;
	
	
  man = BR_RES_GetMsgData( wk->param.p_res );
	str	= GFL_MSG_CreateString( man, sc_msgID[ no ] );
		

	///< BMP
	{
		///< FONT_BUTTON は 2dotでかい
		bmp = GFL_BMP_Create(10, 2, GFL_BMP_16_COLOR, wk->heapID);
		//PRINTSYS_PrintColor( bmp , 0,0,str,wk->sys.fontHandle, PRINTSYS_LSB_Make( 15, 13, 0) );
		PRINTSYS_Print( bmp , 0,0,str,wk->sys.fontHandle );
	}

	pal_id = eID_FNT_OAM;

	
	ox = wk->sys.rht[ no + eHRT_BACK ].rect.left;
	oy = wk->sys.rht[ no + eHRT_BACK ].rect.top;

	finit.bmp = bmp;
	finit.x = ox + x;
	finit.y = oy + y;
  finit.pltt_index = wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON];
	finit.pal_offset = 0;
	finit.soft_pri = 0;
	finit.bg_pri = 0;
	finit.setSerface = CLSYS_DEFREND_SUB;
	finit.draw_type = CLSYS_DRAW_SUB;
	
	OS_Printf("resIdx RES2_BUTTOn=%d\n", wk->sys.resIdx[CIO_RES1_PLT][CIO_RES2_BUTTON]);
	
	wk->sys.bmp_obj[ no ] = BmpOam_ActorAdd( wk->sys.bmpoam_sys , &finit );
	BmpOam_ActorBmpTrans( wk->sys.bmp_obj[ no ] );

	GFL_BMP_Delete( bmp );
	GFL_STR_DeleteBuffer(str);
}
