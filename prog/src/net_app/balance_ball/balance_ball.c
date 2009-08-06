//==============================================================================
/**
 * @file	balance_ball.c
 * @brief	簡単な説明を書く
 * @author	goto
 * @date	2007.09.25(火)
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================

#include <gflib.h>
#include "system/main.h"
#include "system/wipe.h"

#include "net_app/balance_ball.h"

#include "bb_common.h"
#include "bb_comm_cmd.h"

#include "bb_server.h"
#include "bb_client.h"
#include "system/gfl_use.h"
#include "net_old/comm_def.h"
#include "net_old\comm_system.h"
#include "net_old\comm_state.h"
#include "net_old\comm_info.h"
#include "net_old\comm_tool.h"
#include "net_old\comm_command.h"

#define BB_COMM_END_CMD	( 999 )

enum{
	BB_DIS_ERROR_NONE,			// エラーなし
	BB_DIS_ERROR_CLOSEING,		// 切断エラー	終了中
	BB_DIS_ERROR_CLOSED,		// 切断エラー	終了
};

//==============================================================
// Prototype
//==============================================================
static void BB_SystemInit( BB_WORK* wk );
static void BB_VramBankSet( void );
static void BB_CATS_Init( BB_WORK* wk );
static BOOL BB_WipeStart( int type );
static void BB_MainSeq_Change( BB_WORK* wk, BOOL bEnd, int next_seq, int* seq );
static void BB_VBlank(GFL_TCB *tcb, void *work);
static void MainResource_Delete( BB_WORK* wk );
static void Reset_GameData( BB_WORK* wk );

static u32 BB_DIS_ERROR_Check( BB_WORK* wk );

static void BB_TcbMain(BB_WORK *wk);

#define BB_TRANSFER_NUM				( 32 )
#define BB_WIRE_RESS_ICON_PAL		( 14 )
#define BB_WIRE_RESS_ICON_PAL_SIZ	( 32 )

//==============================================================================
//  データ
//==============================================================================
static const GFL_DISP_VRAM vramSetTable = {
	GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_0_B,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_128K,		// サブOBJマッピングモード
};


//==============================================================================
//  
//==============================================================================
enum {
	
	eBB_SEQ_ENTRY_INIT		= 0,
	eBB_SEQ_ENTRY_MAIN,
	eBB_SEQ_ENTRY_EXIT,
	
	eBB_SEQ_FADE_IN,
	eBB_SEQ_FADE_IN_WAIT,
	
	eBB_SEQ_START,
	eBB_SEQ_PEN_DEMO,
	eBB_SEQ_START_WAIT,
	
	eBB_SEQ_COUNT_DOWN,
	eBB_SEQ_COUNT_DOWN_WAIT,
	
	eBB_SEQ_MAIN,
	
	eBB_SEQ_MAIN_END_INIT,
	eBB_SEQ_MAIN_END,
		
	eBB_SEQ_RESULT,
	eBB_SEQ_RESULT_WAIT,
	
	eBB_SEQ_FADE_OUT,
	eBB_SEQ_FADE_OUT_WAIT,

	eBB_SEQ_SCORE,
	eBB_SEQ_SCORE_WAIT,
	
	eBB_SEQ_END,
	eBB_SEQ_END_WAIT,
};

static void BalanceBall_MainInit( BB_WORK* wk )
{
	u16 anm_index;
//	BB_WORK* wk = GFL_PROC_AllocWork( proc, sizeof( BB_WORK ), HEAPID_BB );
//	GFL_STD_MemFill( wk, 0, sizeof( BB_WORK ) );

#if WB_FIX
	wk->seed_tmp = gf_get_seed();		///< 乱数のタネ退避
#else
	wk->seed_tmp = bb_gf_get_seed();		///< 乱数のタネ退避
#endif

	BB_SystemInit( wk );
	
	wk->vintr_tcb = GFUser_VIntr_CreateTCB(BB_VBlank, wk, 200);
	//sys_HBlankIntrStop();

#if WB_FIX
	initVramTransferManagerHeap( BB_TRANSFER_NUM, HEAPID_BB );	
#endif

	///< wi-fi アイコン( パレットをpalette_fade に渡す )
#if WB_FIX
  CLACT_U_WmIcon_SetReserveAreaCharManager( NNS_G2D_VRAM_TYPE_2DMAIN, GX_OBJVRAMMODE_CHAR_1D_128K);
  CLACT_U_WmIcon_SetReserveAreaPlttManager( NNS_G2D_VRAM_TYPE_2DMAIN );
#endif

#if WB_TEMP_FIX
	WirelessIconEasy();
	{
		NNSG2dPaletteData *palData;
		void* dat = WirelessIconPlttResGet( HEAPID_BB );

		NNS_G2dGetUnpackedPaletteData( dat, &palData );		
		PaletteWorkSet( wk->sys.pfd, palData->pRawData, FADE_MAIN_OBJ,
						BB_WIRE_RESS_ICON_PAL * 16, BB_WIRE_RESS_ICON_PAL_SIZ );

		GFL_HEAP_FreeMemory( dat );
	}
#endif

	///< 通信部分
	{
		int comm_num = CommInfoGetEntryNum();		
		wk->netid = CommGetCurrentID();		
		BB_CommCommandInit( wk );		
		if ( IsParentID( wk ) == TRUE ){			
			wk->p_server = BB_Server_AllocMemory( comm_num, &wk->sys );
		}
		wk->p_client = BB_Client_AllocMemory( comm_num, wk->netid, &wk->sys, wk->tcbsys );
		wk->p_client->rare_game = wk->rule.ball_type;
		wk->p_client->seed_use = wk->seed_tmp;
	}
	
	///< Touch Panel
#if WB_FIX
	{
		u32 active;
		InitTPSystem();
		active = InitTPNoBuff( 4 );
	//	wk->sys.btn = BMN_Create( ball_hit_tbl, BB_TOUCH_HIT_NUM, BB_Client_TouchPanel_CallBack, wk, HEAPID_BB );
	}
#endif
	
	///< 2D リソース読み込み
	BB_disp_BG_Load( wk );
	BB_disp_ResourceLoad( wk, wk->p_client );
	BB_disp_Manene_Add( wk, wk->p_client );
	BB_disp_Hand_Add( wk->p_client );
	BB_disp_Hanabi_OAM_Add( wk->p_client );
	BB_disp_Light_Add( wk->p_client );
	BB_disp_Pen_Add( wk->p_client );
	BR_ColorChange( wk, NetID_To_PlayerNo( wk, wk->netid ) );
	
	///< 3D リソース
	{		
		///< 新規登録したモデルは、クォータニオンの行列初期化を BB_Client_3D_PosInit ですること
		{	
			int i;
			int data[][2] = {
				{ NARC_balance_ball_kami_blue_nsbmd, NARC_balance_ball_kami_blue_nsbca	},
				{ NARC_balance_ball_kami_eme_nsbmd,  NARC_balance_ball_kami_eme_nsbca	},
				{ NARC_balance_ball_kami_mizu_nsbmd, NARC_balance_ball_kami_mizu_nsbca	},
				{ NARC_balance_ball_kami_pur_nsbmd,  NARC_balance_ball_kami_pur_nsbca	},
				{ NARC_balance_ball_kami_blue_nsbmd, NARC_balance_ball_kami_blue_nsbca	},
				{ NARC_balance_ball_kami_eme_nsbmd,  NARC_balance_ball_kami_eme_nsbca	},
				{ NARC_balance_ball_kami_mizu_nsbmd, NARC_balance_ball_kami_mizu_nsbca	},
				{ NARC_balance_ball_kami_pur_nsbmd,  NARC_balance_ball_kami_pur_nsbca	},
				{ NARC_balance_ball_kami_blue_nsbmd, NARC_balance_ball_kami_blue_nsbca	},
				{ NARC_balance_ball_kami_eme_nsbmd,  NARC_balance_ball_kami_eme_nsbca	},
				{ NARC_balance_ball_kami_mizu_nsbmd, NARC_balance_ball_kami_mizu_nsbca	},
				{ NARC_balance_ball_kami_pur_nsbmd,  NARC_balance_ball_kami_pur_nsbca	},
				{ NARC_balance_ball_kami_blue_nsbmd, NARC_balance_ball_kami_blue_nsbca	},
				{ NARC_balance_ball_kami_eme_nsbmd,  NARC_balance_ball_kami_eme_nsbca	},
				{ NARC_balance_ball_kami_mizu_nsbmd, NARC_balance_ball_kami_mizu_nsbca	},
				{ NARC_balance_ball_kami_pur_nsbmd,  NARC_balance_ball_kami_pur_nsbca	},
			};
			for ( i = 0; i < BB_KAMI_HUBUKI_MAX; i++ ){
				BB_disp_Model_Load( &wk->p_client->bb3d_kami[ i ], wk->sys.p_handle_bb, data[ i ][ 0 ] );
				BB_disp_Model_Init( &wk->p_client->bb3d_kami[ i ], wk->rule.ball_type ); 
			
			#if WB_FIX
				D3DOBJ_AnmLoadH( &wk->p_client->bb3d_kami[ i ].anm[ 0 ], &wk->p_client->bb3d_kami[ i ].mdl,
								 wk->sys.p_handle_bb, data[ i ][ 1 ], HEAPID_BB, &wk->sys.allocator );
			#else
				wk->p_client->bb3d_kami[i].p_anmres[0] = GFL_G3D_CreateResourceHandle( 
					wk->sys.p_handle_bb, data[i][1] );
				wk->p_client->bb3d_kami[ i ].anm[ 0 ] = GFL_G3D_ANIME_Create( 
					wk->p_client->bb3d_kami[i].g3drnd, wk->p_client->bb3d_kami[i].p_anmres[0], 0 ); 
			#endif
			
			#if WB_FIX
				D3DOBJ_AddAnm( &wk->p_client->bb3d_kami[ i ].obj, &wk->p_client->bb3d_kami[ i ].anm[ 0 ] );
				D3DOBJ_AnmSet( &wk->p_client->bb3d_kami[ i ].anm[ 0 ], 0 );
				D3DOBJ_SetDraw( &wk->p_client->bb3d_kami[ i ].obj, FALSE );
			#else
			  anm_index = GFL_G3D_OBJECT_AddAnime(wk->p_client->bb3d_kami[i].g3dobj, 
			    wk->p_client->bb3d_kami[ i ].anm[ 0 ]);
  			GFL_G3D_OBJECT_EnableAnime( wk->p_client->bb3d_kami[i].g3dobj, anm_index );
  			wk->p_client->bb3d_kami[i].draw_flag = FALSE;
			#endif
			
				{
					int ofs = 5;
					
					if ( wk->rule.ball_type ){
						ofs = 5;
					}
							
					if ( i % 2 ){
						wk->p_client->bb3d_kami[ i ].pos.x += FX32_CONST( i * 5 );
					}
					else {
						wk->p_client->bb3d_kami[ i ].pos.x -= FX32_CONST( i * 5 );
					}
					
					if ( i < BB_KAMI_HUBUKI_MAX ){
						wk->p_client->bb3d_kami[ i ].pos.z += FX32_CONST( ( i + 1 ) * ofs );
					}
					else {
						wk->p_client->bb3d_kami[ i ].pos.z -= FX32_CONST( i * 5 );
					}
				}					
			}
		}	
		BB_disp_Model_Load( &wk->p_client->bb3d_spot, wk->sys.p_handle_bb, NARC_balance_ball_spotlight_nsbmd );
		BB_disp_Model_Init( &wk->p_client->bb3d_spot, wk->rule.ball_type );
#if WB_FIX
		D3DOBJ_SetDraw( &wk->p_client->bb3d_spot.obj, FALSE ); 
#else
    wk->p_client->bb3d_spot.draw_flag = FALSE;
#endif

		{
			int ball_res[] = {
				NARC_balance_ball_ball_red_nsbmd,
				NARC_balance_ball_ball_blue_nsbmd,
				NARC_balance_ball_ball_yellow_nsbmd,
				NARC_balance_ball_ball_green_nsbmd,
			};
			int res_id = ball_res[ NetID_To_PlayerNo( wk, wk->netid ) ];
			BB_disp_Model_Load( &wk->p_client->bb3d_ball, wk->sys.p_handle_bb, res_id );
			BB_disp_Model_Init( &wk->p_client->bb3d_ball, wk->rule.ball_type );
			
			BB_disp_Model_Load( &wk->p_client->bb3d_mane[ 0 ], wk->sys.p_handle_bb, NARC_balance_ball_manene_nsbmd );
			BB_disp_Model_Load( &wk->p_client->bb3d_mane[ 1 ], wk->sys.p_handle_bb, NARC_balance_ball_manene_ochita_nsbmd );
			BB_disp_Model_Load( &wk->p_client->bb3d_mane[ 2 ], wk->sys.p_handle_bb, NARC_balance_ball_manene_fukki_nsbmd );		
			BB_disp_Model_Init( &wk->p_client->bb3d_mane[ 0 ], wk->rule.ball_type );
			BB_disp_Model_Init( &wk->p_client->bb3d_mane[ 1 ], wk->rule.ball_type );
			BB_disp_Model_Init( &wk->p_client->bb3d_mane[ 2 ], wk->rule.ball_type );

			///< アニメ設定
			{
				
				BB_3D_MODEL* b3d;
				int anime;
				
				///< 通常アニメ
				b3d = &wk->p_client->bb3d_mane[ 0 ];
				
				///< 歩く
				anime = NARC_balance_ball_manene_aruku_nsbca;
			#if WB_FIX
				D3DOBJ_AnmLoadH( &b3d->anm[ 0 ], &b3d->mdl, wk->sys.p_handle_bb, anime, HEAPID_BB, &wk->sys.allocator );
				D3DOBJ_AddAnm( &b3d->obj, &b3d->anm[ 0 ] );
				D3DOBJ_AnmSet( &b3d->anm[ 0 ], 0 );
			#else
				b3d->p_anmres[0] = GFL_G3D_CreateResourceHandle( wk->sys.p_handle_bb, anime );
				b3d->anm[ 0 ] = GFL_G3D_ANIME_Create( b3d->g3drnd, b3d->p_anmres[0], 0 ); 

			  anm_index = GFL_G3D_OBJECT_AddAnime(b3d->g3dobj, b3d->anm[ 0 ]);
  			//GFL_G3D_OBJECT_EnableAnime( b3d->g3dobj, anm_index );
			#endif
			
				///< あらら
				anime = NARC_balance_ball_manene_arara_nsbca;
			#if WB_FIX
				D3DOBJ_AnmLoadH( &b3d->anm[ 1 ], &b3d->mdl, wk->sys.p_handle_bb, anime, HEAPID_BB, &wk->sys.allocator );
				D3DOBJ_AnmSet( &b3d->anm[ 1 ], 0 );
			#else
				b3d->p_anmres[1] = GFL_G3D_CreateResourceHandle( wk->sys.p_handle_bb, anime );
				b3d->anm[ 1 ] = GFL_G3D_ANIME_Create( b3d->g3drnd, b3d->p_anmres[1], 0 ); 

			  anm_index = GFL_G3D_OBJECT_AddAnime(b3d->g3dobj, b3d->anm[ 1 ]);
  			//GFL_G3D_OBJECT_EnableAnime( b3d->g3dobj, anm_index );
			#endif
			
				///< おっとっと
			#if WB_FIX
				anime = NARC_balance_ball_manene_ottoto_nsbca;
				D3DOBJ_AnmLoadH( &b3d->anm[ 2 ], &b3d->mdl, wk->sys.p_handle_bb, anime, HEAPID_BB, &wk->sys.allocator );
				D3DOBJ_AnmSet( &b3d->anm[ 2 ], 0 );
			#else
				b3d->p_anmres[2] = GFL_G3D_CreateResourceHandle( wk->sys.p_handle_bb, anime );
				b3d->anm[ 2 ] = GFL_G3D_ANIME_Create( b3d->g3drnd, b3d->p_anmres[2], 0 ); 

			  anm_index = GFL_G3D_OBJECT_AddAnime(b3d->g3dobj, b3d->anm[ 2 ]);
  			//GFL_G3D_OBJECT_EnableAnime( b3d->g3dobj, anm_index );
			#endif
			
				///< 落ちるアニメ
				b3d = &wk->p_client->bb3d_mane[ 1 ];
				anime = NARC_balance_ball_manene_ochita_nsbca;			
			#if WB_FIX
				D3DOBJ_AnmLoadH( &b3d->anm[ 0 ], &b3d->mdl, wk->sys.p_handle_bb, anime, HEAPID_BB, &wk->sys.allocator );
				D3DOBJ_AddAnm( &b3d->obj, &b3d->anm[ 0 ] );
				D3DOBJ_AnmSet( &b3d->anm[ 0 ], 0 );
			#else
				b3d->p_anmres[0] = GFL_G3D_CreateResourceHandle( wk->sys.p_handle_bb, anime );
				b3d->anm[ 0 ] = GFL_G3D_ANIME_Create( b3d->g3drnd, b3d->p_anmres[0], 0 ); 

			  anm_index = GFL_G3D_OBJECT_AddAnime(b3d->g3dobj, b3d->anm[ 0 ]);
  			GFL_G3D_OBJECT_EnableAnime( b3d->g3dobj, anm_index );
			#endif
			
				///< 復帰アニメ
				b3d = &wk->p_client->bb3d_mane[ 2 ];
				anime = NARC_balance_ball_manene_fukki_nsbca;			
			#if WB_FIX
				D3DOBJ_AnmLoadH( &b3d->anm[ 0 ], &b3d->mdl, wk->sys.p_handle_bb, anime, HEAPID_BB, &wk->sys.allocator );
				D3DOBJ_AddAnm( &b3d->obj, &b3d->anm[ 0 ] );
				D3DOBJ_AnmSet( &b3d->anm[ 0 ], 0 );
			#else
				b3d->p_anmres[0] = GFL_G3D_CreateResourceHandle( wk->sys.p_handle_bb, anime );
				b3d->anm[ 0 ] = GFL_G3D_ANIME_Create( b3d->g3drnd, b3d->p_anmres[0], 0 ); 

			  anm_index = GFL_G3D_OBJECT_AddAnime(b3d->g3dobj, b3d->anm[ 0 ]);
  			GFL_G3D_OBJECT_EnableAnime( b3d->g3dobj, anm_index );
			#endif
			}
		}
		
	    if ( wk->rule.ball_type ){
		    wk->p_client->bb3d_ball.pos.y = BB_MODEL_OFS_Y2;
		    wk->p_client->bb3d_mane[ 0 ].pos.y = BB_MODEL_OFS_Y2;
		    wk->p_client->bb3d_mane[ 1 ].pos.y = BB_MODEL_OFS_Y2;
		    wk->p_client->bb3d_mane[ 2 ].pos.y = BB_MODEL_OFS_Y2;
		}
	}		
//	wk->p_client->bb3d_mane[ 0 ].pos.y -= FX32_CONST( Action_MoveValue_3Y() );
//	wk->p_client->bb3d_mane[ 0 ].pos.z += FX32_CONST( Action_MoveValue_3Z() );
#if WB_FIX
	wk->count_down = MNGM_COUNT_Init( CATS_GetClactSetPtr( wk->sys.crp ), HEAPID_BB );	
#else
	wk->count_down = MNGM_COUNT_Init( wk->sys.clunit, HEAPID_BB, wk->sys.plttslot );	
#endif
	PaletteWorkSet_VramCopy( wk->sys.pfd, FADE_MAIN_OBJ, 0 * 16, 16 * 0x20 );
	
	if ( wk->parent_wk->vchat ){
		mydwc_startvchat( HEAPID_BB );
	}
}


//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	proc	
 * @param	seq	
 *
 * @retval	GFL_PROC_RESULT	
 *
 */
//--------------------------------------------------------------
GFL_PROC_RESULT BalanceBallProc_Init( GFL_PROC* proc, int* seq, void * pwk, void * mywk )
{
	BB_WORK* wk;
	
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BB, HEAP_SIZE_BB );
	
	wk = GFL_PROC_AllocWork( proc, sizeof( BB_WORK ), HEAPID_BB );
	GFL_STD_MemFill( wk, 0, sizeof( BB_WORK ) );
	
	wk->parent_wk = pwk;
	
    //TCBシステム作成
    wk->tcb_work = GFL_HEAP_AllocClearMemory(HEAPID_BB, GFL_TCB_CalcSystemWorkSize( 64 ));
    wk->tcbsys = GFL_TCB_Init(64, wk->tcb_work);
	
	{
			
		int i;
		int no = 0;
		
		for( i = 0; i < 4; i++ ){
			
			wk->table[ i ] = 0xFF;			
			if( CommInfoGetMyStatus( i ) != NULL ){
				wk->table[ no ] = i;
				no++;
			}
		}
	}

	MNGM_ENRES_PARAM_Init( &wk->entry_param, wk->parent_wk->wifi_lobby, wk->parent_wk->p_save, wk->parent_wk->vchat, &wk->parent_wk->lobby_wk );

 	bb_gf_rand_init();

  //フォント読み込み
  wk->font_handle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_nftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_BB );
	wk->printQue = PRINTSYS_QUE_Create( HEAPID_BB );

	return GFL_PROC_RES_FINISH;
}

static void BB_PenDemo( BB_CLIENT* wk, u32 x, u32 y )
{
	s16 mx, my;
	BOOL bMove;
	
	if ( x == 0 && y == 0 ){		
		wk->control.old_x = 0;
		wk->control.old_y = 0;		
		if ( wk->bb3d_ball.pow_count != 0 ){
			bMove = Quaternion_Rotation_Pow( &wk->bb3d_ball, 1.0f );
		//	bMove = Quaternion_Rotation_Pow( &wk->bb3d_mane[ 0 ], 0.5f );
		#if 0 //バグのようなので修正 2009.08.04(火) matsuda
			if ( bMove = TRUE )	{
    #else
      if(TRUE){ //今までの条件では常に真のはずなので、こうしてしまう matsuda
    #endif
				VecFx32 vec = { 0, BB_POINT_Y, 0 };
				VecFx32 tmp;
				MTX_MultVec43( &vec, &wk->bb3d_mane[ 0 ].tmp43, &wk->mane_pos );
			}				
		}
		else {
			wk->bb3d_ball.get_pow_x = 0.0f;
			wk->bb3d_ball.get_pow_y = 0.0f;
			Quaternion_Rotation( &wk->bb3d_ball, wk->control.old_x, wk->control.old_y, wk->control.old_x, wk->control.old_y, BB_COEFFICIENT_AB, FALSE );
			Quaternion_Rotation( &wk->bb3d_mane[ 0 ], wk->control.old_x, wk->control.old_y, wk->control.old_x, wk->control.old_y, BB_COEFFICIENT_AM, FALSE );
		}			
		//Snd_SeStopBySeqNo( BB_SND_KIRAKIRA, 0 );
	}
	else {
		f32 py = BB_COEFFICIENT_PEN( wk->mane_pos.y ); //100 - ( wk->mane_pos.y >> FX32_SHIFT ) ) / 40;
		
		if ( ( wk->mane_pos.y >> FX32_SHIFT ) >= 95 ){			///< 角度 5%までは ゆるい補正
			py = 0.1f;
		}		
		if ( wk->control.old_x == 0 ){
			wk->control.old_x = x;
			wk->control.old_y = y;
		}		
		bMove = Quaternion_Rotation( &wk->bb3d_ball, x, y, wk->control.old_x, wk->control.old_y, BB_COEFFICIENT_AB_PEN, TRUE );
	//	bMove = Quaternion_Rotation( &wk->bb3d_mane[ 0 ], x, y, wk->control.old_x, wk->control.old_y, BB_COEFFICIENT_AM - py, TRUE );		
  #if 0 //バグのようなので修正 2009.08.04(火) matsuda
		if ( bMove = TRUE ){
  #else
    if(TRUE){ //今までの条件では常に真のはずなので、こうしてしまう matsuda
  #endif
			VecFx32 vec = { 0, BB_POINT_Y, 0 };
			VecFx32 tmp;
			MTX_MultVec43( &vec, &wk->bb3d_mane[ 0 ].tmp43, &wk->mane_pos );
		}		
	#if WB_TEMP_FIX
		if ( Snd_SePlayCheck( BB_SND_KIRAKIRA ) == 0 ){
			Snd_SePlay( BB_SND_KIRAKIRA );			///< きらきら
		}	
	#endif
		wk->control.old_x = x;
		wk->control.old_y = y;
	}
}

static BOOL PenDemo( BB_CLIENT* wk )
{
	int now;
	int index;
	int x, y;
	s16 pos[][ 2 ] = {
		{ 0,0 },{ 0,-40 },{ 0,-30 },{ 0,-20 },{ 0,-10 },{ 0,0 },{ 0,10 },{ 0,20 },{ 0,30 },{ 0,40 },{ 0,00 },{ 0,0 },
		{ 0,0 },{ 0,-40 },{ 0,-30 },{ 0,-20 },{ 0,-10 },{ 0,0 },{ 0,10 },{ 0,20 },{ 0,30 },{ 0,40 },{ 0,00 },{ 0,0 },
		{ 0,0 },{ 10,0 },{ 20,0 },{ 30,0 },{ 40,0 },{ 50,0 },{ 60,0 },{ 70,0 },{ 80,0 },{ 90,0 },{ 0,0 },{ 0,0 },
		{ 0,0 },{ 10,0 },{ 20,0 },{ 30,0 },{ 40,0 },{ 50,0 },{ 60,0 },{ 70,0 },{ 80,0 },{ 90,0 },{ 0,0 },{ 0,0 },
	};
	
#if WB_FIX
	now = CATS_ObjectAnimeFrameGetCap( wk->cap_pen );
#else
  now = GFL_CLACT_WK_GetAnmFrame(wk->cap_pen);
#endif
//	OS_Printf( "now = %d\n", now );
	index = ( wk->seq2 * 12 ) + now;
	x = pos[ index ][ 0 ];
	y = pos[ index ][ 1 ];

	if ( x == 0 && y == 0 ){
		BB_PenDemo( wk, 0, 0 );
	}
	else {
		BB_PenDemo( wk, 128 + x, 128 + y );		
		if ( x != 0 ){
			BB_Stardust_Call( wk,  88 + x, 128 + y );			
		}
		else {
			BB_Stardust_Call( wk, 128 + x, 128 + y );
		}
	}
	
	if ( now == 0 ){
		if ( wk->seq != now ){
			wk->seq2++;
		}
	}
	wk->seq = now;
	if ( wk->seq2 == 2 ){
		if ( GFL_CLACT_WK_GetAnmSeq( wk->cap_pen ) != 2 ){
      GFL_CLACTPOS pos = {128, 100};
			GFL_CLACT_WK_SetPos( wk->cap_pen, &pos, CLSYS_DEFREND_MAIN );
		#if WB_FIX
			CATS_ObjectAnimeSeqCheckSetCap( wk->cap_pen, 2 );
		#else
		  if(GFL_CLACT_WK_GetAnmSeq(wk->cap_pen) != 2){
        GFL_CLACT_WK_SetAnmSeq(wk->cap_pen, 2);
      }
		#endif
		#if WB_FIX
			Quaternion_Identity( &wk->bb3d_ball.tq );
			Quaternion_Identity( &wk->bb3d_ball.cq );
		#else
			GFL_QUAT_Identity( wk->bb3d_ball.tq );
			GFL_QUAT_Identity( wk->bb3d_ball.cq );
		#endif
			Quaternion_Rotation( &wk->bb3d_ball, 0, 0, +5, +5, BB_COEFFICIENT_AMI, FALSE );
			Quaternion_Rotation( &wk->bb3d_ball, 0, 0, -5, -5, BB_COEFFICIENT_AMI, FALSE );	
		}
	}
	if ( wk->seq2 == 3 && now == 11 ){
		wk->seq  = 0;
		wk->seq2 = 0;
		GFL_CLACT_WK_SetDrawEnable( wk->cap_pen, FALSE );
#if WB_FIX
		Quaternion_Identity( &wk->bb3d_ball.tq );
		Quaternion_Identity( &wk->bb3d_ball.cq );
#else
		GFL_QUAT_Identity( wk->bb3d_ball.tq );
		GFL_QUAT_Identity( wk->bb3d_ball.cq );
#endif
		Quaternion_Rotation( &wk->bb3d_ball, 0, 0, +5, +5, BB_COEFFICIENT_AMI, FALSE );
		Quaternion_Rotation( &wk->bb3d_ball, 0, 0, -5, -5, BB_COEFFICIENT_AMI, FALSE );	
		return TRUE;
	}
	GFL_CLACT_WK_AddAnmFrame( wk->cap_pen, FX32_ONE*2 );
	
	return FALSE;
}


//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	proc	
 * @param	seq	
 *
 * @retval	GFL_PROC_RESULT	
 *
 */
//--------------------------------------------------------------
GFL_PROC_RESULT BalanceBallProc_Main( GFL_PROC* proc, int* seq, void * pwk, void * mywk )
{
	BB_WORK* wk = mywk;
	BOOL bEnd = FALSE;
	u32 dis_error;

	PRINTSYS_QUE_Main(wk->printQue);
  PRINT_UTIL_Trans(&wk->print_util_talk, wk->printQue);

	// 切断エラーチェック
	dis_error = BB_DIS_ERROR_Check( wk );
	switch( dis_error ){
	case BB_DIS_ERROR_CLOSEING:		// 切断エラー	終了中
		BB_TcbMain(wk);
		return GFL_PROC_RES_CONTINUE;

	case BB_DIS_ERROR_CLOSED:		// 切断エラー	終了
		return GFL_PROC_RES_FINISH;

	case BB_DIS_ERROR_NONE:			// エラーなし
	default:
		break;
	}

		
	switch ( *seq ){
	
	case eBB_SEQ_ENTRY_INIT:
		///< エントリー初期化
		bEnd = TRUE;
		wk->entry_work = MNGM_ENTRY_InitBalanceBall( &wk->entry_param, HEAPID_BB );
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_ENTRY_MAIN, seq );
		break;

	case eBB_SEQ_ENTRY_MAIN:
		///< エントリー中
		bEnd = MNGM_ENTRY_Wait( wk->entry_work );
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_ENTRY_EXIT, seq );
		break;

	case eBB_SEQ_ENTRY_EXIT:
		///< エントリー終了
		bEnd = TRUE;
		wk->rule.ball_type = MNGM_ENTRY_GetRareGame( wk->entry_work );
		MNGM_ENTRY_Exit( wk->entry_work );
		wk->entry_work = NULL;
#ifdef PM_DEBUG
		if ( GFL_UI_KEY_GetCont() & PAD_BUTTON_A ){
			wk->rule.ball_type = 1;
		}
#endif
		BalanceBall_MainInit( wk );
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_FADE_IN, seq );
		break;
	
	case eBB_SEQ_FADE_IN:
		///< 画面フェードイン開始
		bEnd = BB_WipeStart( WIPE_TYPE_DOORIN );
		BB_disp_NameWinAdd( wk, wk->p_client );
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_FADE_IN_WAIT, seq );
		break;
	
	
	case eBB_SEQ_FADE_IN_WAIT:
		///< 画面フェードイン中
		bEnd = WIPE_SYS_EndCheck();
		
	#ifdef PM_DEBUG
	#ifdef BB_GAME_SETUP_ON		
		if ( bEnd ){
			
			BOOL bSetup = Debug_GameSetup( wk );
			
			if ( bSetup == FALSE ){ 
				BB_TcbMain(wk);
				return GFL_PROC_RES_CONTINUE;
			}
		}
	#endif
	#endif
		
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_START, seq );
		break;
		
	case eBB_SEQ_START:
		///< スタートリクエスト
		bEnd = TRUE;//BB_Client_JumpOnToBall( wk->p_client );
		if ( bEnd == FALSE ){ break; }		
	//	BB_disp_NameWinDel( wk->p_client );
		BB_disp_InfoWinAdd( wk, wk->p_client );
		BB_Effect_Call( wk->p_client );
		BB_Fever_Call( wk->p_client );
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_PEN_DEMO, seq );
		break;

	case eBB_SEQ_PEN_DEMO:
		///< ペンデモ
		bEnd = PenDemo( wk->p_client );
		if ( bEnd ){
			if ( IsParentID( wk ) == TRUE ){
				wk->rule.rand	   = wk->seed_tmp;
				CommSendData( CCMD_BB_START, &wk->rule, sizeof( BB_RULE_COMM ) );
			}
		}
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_START_WAIT, seq );
		break;	
	
	case eBB_SEQ_START_WAIT:
		///< スタート待ち
		bEnd = wk->bStart;	
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_COUNT_DOWN, seq );
		break;
	
	
	case eBB_SEQ_COUNT_DOWN:
		///< カウントダウン
		bEnd = TRUE;		
		MNGM_COUNT_StartStart( wk->count_down, wk->tcbsys );		
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_COUNT_DOWN_WAIT, seq );
		break;
	
		
	case eBB_SEQ_COUNT_DOWN_WAIT:
		///< カウントダウン中
		bEnd = TRUE;
		bEnd = MNGM_COUNT_Wait( wk->count_down );		
		if ( bEnd ){
			BB_disp_NameWinDel( wk->p_client );
			BB_disp_InfoWinDel( wk->p_client );
		}	
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_MAIN, seq );
		break;
	
			
	case eBB_SEQ_MAIN:
		bEnd = wk->bEnd;
		if ( wk->bEnd ){
			if ( wk->p_client->game_sys.game_timer < BB_GAME_TIMER ){
				bEnd = FALSE;
				OS_Printf( " 親機が終了して、子機はまだ終わってない\n" );
			}
		}				
		if ( bEnd == FALSE ){			
			if ( IsParentID( wk ) == TRUE ){
				
				if ( BB_Server_Main( wk->p_server ) == FALSE ){
			#ifdef BB_GAME_ENDLESS	
					if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
						CommSendData( CCMD_BB_END, NULL, 0 );
						break;	
					}
			#else
				#if WB_TEMP_FIX
					Snd_SeStopBySeqNo( BB_SND_HAND, 0 );
				#endif
					CommSendData( CCMD_BB_END, NULL, 0 );
			#endif
				}
			}	
			BB_Client_GameCore( wk->p_client );
		}
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_MAIN_END_INIT, seq );
		break;
	
	case eBB_SEQ_MAIN_END_INIT:
		bEnd = TRUE;
	#if WB_TEMP_FIX
		Snd_SeStopBySeqNo( BB_SND_HANABI, 0 );			///< 花火どがーん停止
		Snd_SeStopBySeqNo( BB_SND_HAND, 0 );			///< 拍手も停止
	#endif
		MNGM_COUNT_StartTimeUp( wk->count_down, wk->tcbsys );
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_MAIN_END, seq );
		break;
		
	case eBB_SEQ_MAIN_END:
		bEnd = MNGM_COUNT_Wait( wk->count_down );
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_RESULT, seq );
		break;
		
	case eBB_SEQ_RESULT:
		bEnd = TRUE;
		wk->result.score = BB_P_TO_SCORE( wk->p_client->game_sys.point );//wk->p_client->game_sys.result_time;
		if ( wk->result.score >= BB_SCORE_MAX ){
			 wk->result.score = BB_SCORE_MAX;
		}
		if ( wk->p_client->game_sys.bFirstTouch == FALSE ){
			 wk->result.score = BB_SCORE_MIN_CHK;
		}
//		OS_Printf( "触った？ = %d\n", wk->p_client->game_sys.bFirstTouch ); 
//		OS_Printf( "point    = %d\n", wk->p_client->game_sys.point ); 
//		OS_Printf( "score    = %d\n", wk->result.score ); 
		CommSendData( CCMD_BB_RESULT, &wk->result, sizeof( BB_RESULT_COMM ) );
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_RESULT_WAIT, seq );
		break;
	
		
	case eBB_SEQ_RESULT_WAIT:
		bEnd = FALSE;
		{
			int i;
			int num = 0;
			
			for ( i = 0; i < 4; i++ ){
				if ( wk->mem_result.result[ i ].score != 0 ){
					num++;
				}
			}
			if ( num == wk->p_client->comm_num ){
				bEnd = TRUE;
			}
		}
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_FADE_OUT, seq );
		break;
	
	case eBB_SEQ_FADE_OUT:
		bEnd = BB_WipeStart( WIPE_TYPE_DOOROUT );
		
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_FADE_OUT_WAIT, seq );
		break;
	
	
	case eBB_SEQ_FADE_OUT_WAIT:
		bEnd = WIPE_SYS_EndCheck();
		
	#if 0 //バグっぽいので修正 2009.08.06(木) matsuda
		if ( bEnd ){
			MainResource_Delete( wk );
		}
		
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_SCORE, seq );
	#else
		if ( bEnd ){
			MainResource_Delete( wk );
  		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_SCORE, seq );
		}
	#endif
		break;
	
	
	case eBB_SEQ_SCORE:
		bEnd = TRUE;
		{
			int i;
			int id;			
			for ( i = 0; i < 4; i++ ){
				id = NetID_To_PlayerNo( wk, i );
				if ( id == 0xFF ){ continue; }
				wk->result_param.score[ id ]  = wk->mem_result.result[ i ].score;
				if ( wk->result_param.score[ id ] <= BB_SCORE_MIN_CHK ){
					wk->result_param.score[ id ] = BB_SCORE_SET_MIN;
				}
			}
			// ランキング計算
//			wk->result_param.p_gadget = &wk->parent_wk->gadget;	// GADGETなくしました　tomoya
			MNGM_RESULT_CalcRank( &wk->result_param, wk->entry_param.num );
		}
		wk->result_work = MNGM_RESULT_InitBalanceBall( &wk->entry_param, &wk->result_param, HEAPID_BB );
		

		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_SCORE_WAIT, seq );
		break;
	
	
	case eBB_SEQ_SCORE_WAIT:
		bEnd = MNGM_RESULT_Wait( wk->result_work );
		
		if ( bEnd ){
			BOOL replay = MNGM_RESULT_GetReplay( wk->result_work );
			MNGM_RESULT_Exit( wk->result_work );
			wk->result_work = NULL;
			
			if ( replay ){
				wk->bRePlay = TRUE;
				Reset_GameData( wk );				
				BB_MainSeq_Change( wk, bEnd, eBB_SEQ_ENTRY_INIT, seq );
				if ( wk->parent_wk->vchat ){
					mydwc_stopvchat();
				}
				break;
			}
		}	
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_END, seq );
		break;
	
	case eBB_SEQ_END:
		bEnd = TRUE;
		
		CommTimingSyncStart( CCMD_BB_CONNECT_END );
	
		if ( wk->parent_wk->vchat ){
			mydwc_stopvchat();
		}
		BB_MainSeq_Change( wk, bEnd, eBB_SEQ_END_WAIT, seq );
		break;
	
		
	case eBB_SEQ_END_WAIT:
	default:
		bEnd = CommIsTimingSync( CCMD_BB_CONNECT_END );
		if(bEnd == TRUE){
			return GFL_PROC_RES_FINISH;
		}
		else{
			BB_TcbMain(wk);
			return GFL_PROC_RES_CONTINUE;
		}
		break;
	}
	
	if ( *seq >= eBB_SEQ_FADE_IN  && *seq < eBB_SEQ_FADE_OUT_WAIT ){		
		BB_disp_Manene_OAM_Update( wk->p_client );
//		BB_disp_Hanabi_OAM_Update( wk->p_client );
		BB_disp_Draw( wk );
	}
	
	BB_TcbMain(wk);
	return GFL_PROC_RES_CONTINUE;
}

static void MainResource_Delete( BB_WORK* wk )
{
	GFL_TCB_DeleteTask(wk->vintr_tcb);
	//sys_HBlankIntrStop();
#if WB_TEMP_FIX
	DellVramTransferManager();
#endif

#if WB_TEMP_FIX
	///< touch panel
	{
		{
			u32 active;
			active = StopTP();
		}
	//	BMN_Delete( wk->sys.btn );
	}
#endif
	
	///< 乱数のタネ戻す
#if WB_FIX
	gf_srand( wk->seed_tmp );
#else
	bb_gf_srand( wk->seed_tmp );
#endif
	///<

	///< OAM リソース解放
	BB_disp_Manene_OAM_Del( wk->p_client );
	BB_disp_Hand_Del( wk->p_client );
	BB_disp_Hanabi_OAM_Del( wk->p_client );
	BB_disp_Light_Del( wk->p_client );
	BB_disp_Pen_Del( wk->p_client );
	
	///< 3D 解放	
	BB_disp_Model_Delete( &wk->p_client->bb3d_ball );
	BB_disp_Model_Delete( &wk->p_client->bb3d_mane[ 0 ] );
	BB_disp_Model_Delete( &wk->p_client->bb3d_mane[ 1 ] );
	BB_disp_Model_Delete( &wk->p_client->bb3d_mane[ 2 ] );
	BB_disp_Model_Delete( &wk->p_client->bb3d_spot );
	
//	D3DOBJ_AnmDelete( &wk->p_client->bb3d_spot.anm[ 0 ], &wk->sys.allocator );
#if WB_FIX
	D3DOBJ_AnmDelete( &wk->p_client->bb3d_mane[ 0 ].anm[ 0 ], &wk->sys.allocator );
	D3DOBJ_AnmDelete( &wk->p_client->bb3d_mane[ 0 ].anm[ 1 ], &wk->sys.allocator );
	D3DOBJ_AnmDelete( &wk->p_client->bb3d_mane[ 0 ].anm[ 2 ], &wk->sys.allocator );
	D3DOBJ_AnmDelete( &wk->p_client->bb3d_mane[ 1 ].anm[ 0 ], &wk->sys.allocator );
	D3DOBJ_AnmDelete( &wk->p_client->bb3d_mane[ 2 ].anm[ 0 ], &wk->sys.allocator );
#else
  GFL_G3D_ANIME_Delete(wk->p_client->bb3d_mane[ 0 ].anm[ 0 ]);
  GFL_G3D_DeleteResource( wk->p_client->bb3d_mane[ 0 ].p_anmres[ 0 ] );
  GFL_G3D_ANIME_Delete(wk->p_client->bb3d_mane[ 0 ].anm[ 1 ]);
  GFL_G3D_DeleteResource( wk->p_client->bb3d_mane[ 0 ].p_anmres[ 1 ] );
  GFL_G3D_ANIME_Delete(wk->p_client->bb3d_mane[ 0 ].anm[ 2 ]);
  GFL_G3D_DeleteResource( wk->p_client->bb3d_mane[ 0 ].p_anmres[ 2 ] );
  GFL_G3D_ANIME_Delete(wk->p_client->bb3d_mane[ 1 ].anm[ 0 ]);
  GFL_G3D_DeleteResource( wk->p_client->bb3d_mane[ 1 ].p_anmres[ 0 ] );
  GFL_G3D_ANIME_Delete(wk->p_client->bb3d_mane[ 2 ].anm[ 0 ]);
  GFL_G3D_DeleteResource( wk->p_client->bb3d_mane[ 2 ].p_anmres[ 0 ] );
#endif

	{
		int i;
		for ( i = 0; i < BB_KAMI_HUBUKI_MAX; i++ ){
			BB_disp_Model_Delete( &wk->p_client->bb3d_kami[ i ] );
	#if WB_FIX
			D3DOBJ_AnmDelete( &wk->p_client->bb3d_kami[ i ].anm[ 0 ], &wk->sys.allocator );
			D3DOBJ_AnmDelete( &wk->p_client->bb3d_kami[ i ].anm[ 1 ], &wk->sys.allocator );
	#else
	    if(wk->p_client->bb3d_kami[i].anm[0] != NULL){
        GFL_G3D_ANIME_Delete(wk->p_client->bb3d_kami[i].anm[0]);
    		GFL_G3D_DeleteResource( wk->p_client->bb3d_kami[i].p_anmres[0] );
      }
      if(wk->p_client->bb3d_kami[i].anm[1] != NULL){
        GFL_G3D_ANIME_Delete(wk->p_client->bb3d_kami[i].anm[1]);
    		GFL_G3D_DeleteResource( wk->p_client->bb3d_kami[i].p_anmres[1] );
      }
	#endif
		}
	}

	MNGM_COUNT_Exit( wk->count_down, wk->sys.plttslot );

	BB_SystemExit( wk );

	///< ワーク解放
	if ( IsParentID( wk ) == TRUE ){
		
		BB_Server_FreeMemory( wk->p_server );
	}
	BB_Client_FreeMemory( wk->p_client );

	wk->p_server = NULL;
	wk->p_client = NULL;
}


//--------------------------------------------------------------
/**
 * @brief	ゲームリセットしなくちゃいけない部分
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void Reset_GameData( BB_WORK* wk )
{
	wk->bEnd	= FALSE;
	wk->bStart	= FALSE;
	wk->bResult = FALSE;
	
	GFL_STD_MemFill( &wk->result, 0, sizeof( BB_RESULT_COMM ) );
	GFL_STD_MemFill( &wk->mem_result, 0, sizeof( BB_MENBER_RESULT_COMM ) );
}


//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	proc	
 * @param	seq	
 *
 * @retval	GFL_PROC_RESULT	
 *
 */
//--------------------------------------------------------------
GFL_PROC_RESULT BalanceBallProc_Exit( GFL_PROC* proc, int* seq, void * pwk, void * mywk )
{	
	switch ( *seq ){
	case 0:
		{
			u32 dis_error;
			BB_WORK* wk = mywk;
			dis_error = BB_DIS_ERROR_Check( wk );	// 切断エラーチェック

      //フォント破棄
    	GFL_FONT_Delete(wk->font_handle);
    	//PrintQue破棄
    	PRINTSYS_QUE_Delete(wk->printQue);

			//TCBシステム削除
			GFL_TCB_Exit( wk->tcbsys );
			GFL_HEAP_FreeMemory(wk->tcb_work);
			wk->tcbsys = NULL;
	
			GFL_PROC_FreeWork( proc );
			GFL_HEAP_DeleteHeap( HEAPID_BB );
			CommStateSetErrorCheck( FALSE, TRUE );

			if( dis_error != BB_DIS_ERROR_NONE ){
				return GFL_PROC_RES_FINISH;
			}

			CommTimingSyncStart( BB_COMM_END_CMD );
			(*seq)++;
		}
		break;

	default:
		if( (CommIsTimingSync( BB_COMM_END_CMD ) == TRUE) || 
			(CommGetConnectNum() < CommInfoGetEntryNum()) ){	// 人数が少なくなったらそのまま抜ける
			return GFL_PROC_RES_FINISH;
		}
		break;
	}

	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   TCBメイン
 *
 * @param   tcbsys		
 */
//--------------------------------------------------------------
static void BB_TcbMain(BB_WORK *wk)
{
	if(wk != NULL && wk->tcbsys != NULL){
		GFL_TCB_Main(wk->tcbsys);
	}
}







//--------------------------------------------------------------
/**
 * @brief	SYSTEM 初期化
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void BB_SystemInit( BB_WORK* wk )
{
	wk->sys.p_handle_bb = GFL_ARC_OpenDataHandle( ARCID_BB_RES, HEAPID_BB );	;
	wk->sys.p_handle_cr = GFL_ARC_OpenDataHandle( ARCID_COMMON_RES, HEAPID_BB );	
	GFL_BG_Init( HEAPID_BB );
	GFL_BMPWIN_Init( HEAPID_BB );
	wk->sys.pfd			= PaletteFadeInit( HEAPID_BB );
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						0x1000, HEAPID_BB, NULL);
	GFL_G3D_SetSystemSwapBufferMode(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#if WB_FIX
	wk->sys.camera_p	= GFC_AllocCamera( HEAPID_BB );
	BB_disp_CameraSet( wk );
#else
	wk->sys.camera_p	= BB_disp_CameraSet( wk );
#endif

	PaletteTrans_AutoSet( wk->sys.pfd, TRUE );
	PaletteFadeWorkAllocSet( wk->sys.pfd, FADE_MAIN_BG,	 0x200, HEAPID_BB );
	PaletteFadeWorkAllocSet( wk->sys.pfd, FADE_SUB_BG,	 0x200, HEAPID_BB );
	PaletteFadeWorkAllocSet( wk->sys.pfd, FADE_MAIN_OBJ, 0x200, HEAPID_BB );
	PaletteFadeWorkAllocSet( wk->sys.pfd, FADE_SUB_OBJ,	 0x200, HEAPID_BB );

	BB_VramBankSet( );
	BB_CATS_Init( wk );
	
	G3X_AlphaBlend( TRUE );
	
	{
		static const GXRgb edge_color_table[ 8 ] = {
			GX_RGB( 0, 0, 0),GX_RGB( 4, 4, 4),GX_RGB( 4, 4, 4),GX_RGB( 4, 4, 4),
			GX_RGB( 4, 4, 4),GX_RGB( 4, 4, 4),GX_RGB( 4, 4, 4),GX_RGB( 4, 4, 4),
		};	
		G3X_EdgeMarking( TRUE );
		G3X_SetEdgeColorTable( edge_color_table );
	}
	
#if WB_FIX
	sys_InitAllocator( &wk->sys.allocator, HEAPID_BB, 32 );
#endif
}


//--------------------------------------------------------------
/**
 * @brief	SYSTEM 解放
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_SystemExit( BB_WORK* wk )
{	
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0,  VISIBLE_OFF );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1,  VISIBLE_OFF );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2,  VISIBLE_OFF );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3,  VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	PaletteFadeWorkAllocFree( wk->sys.pfd, FADE_MAIN_BG );
	PaletteFadeWorkAllocFree( wk->sys.pfd, FADE_SUB_BG );
	PaletteFadeWorkAllocFree( wk->sys.pfd, FADE_MAIN_OBJ );
	PaletteFadeWorkAllocFree( wk->sys.pfd, FADE_SUB_OBJ );
	PaletteFadeFree( wk->sys.pfd );

	GFL_ARC_CloseDataHandle( wk->sys.p_handle_bb );
	GFL_ARC_CloseDataHandle( wk->sys.p_handle_cr );
	
	GFL_G3D_Exit();
	GFL_G3D_CAMERA_Delete( wk->sys.camera_p );
	
	
#if WB_FIX
	CATS_ResourceDestructor_S( wk->sys.csp, wk->sys.crp );
	CATS_FreeMemory( wk->sys.csp );
#else
  GFL_CLACT_UNIT_Delete(wk->sys.clunit);
  GFL_CLACT_SYS_Delete();
  PLTTSLOT_Exit(wk->sys.plttslot);
#endif

	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
	GFL_DISP_SetDispOn();
	
	G3X_AlphaBlend( FALSE );
}

//--------------------------------------------------------------
/**
 * @brief	VRAM 設定
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void BB_VramBankSet( void )
{
	GFL_DISP_GX_SetVisibleControlDirect(0);
	
	{	// BG SYSTEM
		GFL_BG_SYS_HEADER BGsys_data = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
		};
		GFL_BG_SetBGMode( &BGsys_data );
	}
	
	//VRAM設定
	{
		GFL_DISP_SetBank( &vramSetTable );

		//VRAMクリア
		GFL_STD_MemClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
		GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
		GFL_STD_MemClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
		GFL_STD_MemClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
	}
	
	//メイン画面フレーム設定
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME0_M　
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, 0x4000,
				GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
			},
			///<FRAME1_M　左幕
			{
				0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x14000, 0x8000,
				GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
			},
			///<FRAME2_M　右幕
			{
				0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x3800, GX_BG_CHARBASE_0x14000, 0x8000,
				GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
			},
			///<FRAME3_M　背景
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x4800, GX_BG_CHARBASE_0x10000, 0x4000,
				GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
			},
		};
//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 32, 0, HEAPID_BB );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME1_M, 32, 0, HEAPID_BB );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME2_M, 32, 0, HEAPID_BB );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME3_M, 32, 0, HEAPID_BB );
//		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &TextBgCntDat[ 0 ], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &TextBgCntDat[ 1 ], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &TextBgCntDat[ 2 ], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &TextBgCntDat[ 3 ], GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible(GFL_BG_FRAME1_M, VISIBLE_ON);
		GFL_BG_SetVisible(GFL_BG_FRAME2_M, VISIBLE_ON);
		GFL_BG_SetVisible(GFL_BG_FRAME3_M, VISIBLE_ON);
//		GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_M );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_M );
	}
	///< サブ画面フレーム設定
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME0_S　背景
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xb000, GX_BG_CHARBASE_0x00000, 0x4000,
				GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
			},
			///<FRAME1_S　←幕
			{
				0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xb800, GX_BG_CHARBASE_0x04000, 0x7000,
				GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
			},
			///<FRAME2_S　→幕
			{
				0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xc800, GX_BG_CHARBASE_0x04000, 0x7000,
				GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
			},
			///<FRAME3_S　↑幕
			{
				0, 0, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x04000, 0x7000,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
		};
		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 32, 0, HEAPID_BB );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME1_S, 32, 0, HEAPID_BB );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME2_S, 32, 0, HEAPID_BB );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME3_S, 32, 0, HEAPID_BB );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &TextBgCntDat[ 0 ], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat[ 1 ], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat[ 2 ], GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat[ 3 ], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_S );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_S );
	}
	GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 32, 0, HEAPID_BB );
	GFL_BG_SetClearCharacter( GFL_BG_FRAME1_S, 32, 0, HEAPID_BB );
	GFL_BG_SetClearCharacter( GFL_BG_FRAME2_S, 32, 0, HEAPID_BB );
	GFL_BG_SetClearCharacter( GFL_BG_FRAME3_S, 32, 0, HEAPID_BB );
	
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_ON );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	
	///< SUB画面をメイン画面にするため
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_SUB);
	GFL_DISP_SetDispOn();	
	
	G2_SetBlendAlpha(  0, GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, 7, 10 );					  
	G2S_SetBlendAlpha( 0, GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ, 7, 10 );
}

//--------------------------------------------------------------
/**
 * @brief	CATS の 初期化
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void BB_CATS_Init( BB_WORK* wk )
{
#if WB_FIX
	wk->sys.csp = CATS_AllocMemory( HEAPID_BB );
	{
		const TCATS_OAM_INIT OamInit = {
			0, 128, 0, 32,
			0, 128, 0, 32,
		};
		const TCATS_CHAR_MANAGER_MAKE Ccmm = {
			64 + 64,			///< ID管理 main + sub
			1024 * 0x40,		///< 64k
			 512 * 0x20,		///< 32k
			GX_OBJVRAMMODE_CHAR_1D_128K,
			GX_OBJVRAMMODE_CHAR_1D_32K
		};
		CATS_SystemInit( wk->sys.csp, &OamInit, &Ccmm, 16 + 16 );
	}
	
	{
		BOOL active;
		const TCATS_RESOURCE_NUM_LIST ResourceList = {
			64 + 64,	///< キャラ登録数 	main + sub
			16 + 16,	///< パレット登録数	main + sub
			128,		///< セル
			128,		///< セルアニメ
			16,			///< マルチセル
			16,			///< マルチセルアニメ
		};
		
		wk->sys.crp = CATS_ResourceCreate( wk->sys.csp );

		active   = CATS_ClactSetInit( wk->sys.csp, wk->sys.crp, 255 );
		GF_ASSERT( active );
		
		active	  = CATS_ResourceManagerInit( wk->sys.csp, wk->sys.crp, &ResourceList );
		GF_ASSERT( active );
	}
	CLACT_U_SetSubSurfaceMatrix( CATS_EasyRenderGet( wk->sys.csp ), 0, BB_SURFACE_Y << FX32_SHIFT );
#else
	GFL_CLSYS_INIT clsys_init = GFL_CLSYSINIT_DEF_DIVSCREEN;
	
	clsys_init.oamst_main = GFL_CLSYS_OAMMAN_INTERVAL;	//通信アイコンの分
	clsys_init.oamnum_main = 128-GFL_CLSYS_OAMMAN_INTERVAL;
	clsys_init.tr_cell = 32;	//セルVram転送管理数
	clsys_init.CGR_RegisterMax = 64 + 64;
	clsys_init.CELL_RegisterMax = 128;
	GFL_CLACT_SYS_Create(&clsys_init, &vramSetTable, HEAPID_BB);
	
	wk->sys.clunit = GFL_CLACT_UNIT_Create(255, 0, HEAPID_BB);
	GFL_CLACT_UNIT_SetDefaultRend(wk->sys.clunit);

	wk->sys.plttslot = PLTTSLOT_Init(HEAPID_BB, 15, 16);
#endif
}

//--------------------------------------------------------------
/**
 * @brief	ワイプ処理
 *
 * @param	type	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static BOOL BB_WipeStart( int type )
{
	WIPE_SYS_Start( WIPE_PATTERN_WMS, type, type,
					WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_BB );
	
	return TRUE;
}


//--------------------------------------------------------------
/**
 * @brief	シーケンス切り替え
 *
 * @param	wk			処理するワーク
 * @param	bEnd		終了フラグ
 * @param	next_seq	次のシーケンス
 * @param	seq			シーケンス
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void BB_MainSeq_Change( BB_WORK* wk, BOOL bEnd, int next_seq, int* seq )
{
	if ( bEnd ){

		*seq = next_seq;
	}
}


//--------------------------------------------------------------
/**
 * @brief	VBlank 関数
 *
 * @param	work	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void BB_VBlank(GFL_TCB *tcb, void *work)
{
	BB_WORK* wk = work;
	
#if WB_TEMP_FIX
	DoVramTransferManager();
#endif
	
	GFL_CLACT_SYS_VBlankFunc();

	PaletteFadeTrans( wk->sys.pfd );
	
	GFL_BG_VBlankFunc(  );
	
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}



















/// コマンド系

/// public

//--------------------------------------------------------------
/**
 * @brief	親機か？
 *
 * @param	wk	
 *
 * @retval	BOOL	TRUE=親
 *
 */
//--------------------------------------------------------------
BOOL IsParentID( BB_WORK* wk )
{	
	return ( wk->netid == COMM_PARENT_ID ) ? TRUE : FALSE;
}


//--------------------------------------------------------------
/**
 * @brief	開始コマンド
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_cmd_StartSet( BB_WORK* wk )
{
	wk->bStart = TRUE;
}


//--------------------------------------------------------------
/**
 * @brief	終了コマンド
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_cmd_EndSet( BB_WORK* wk )
{
	wk->bEnd = TRUE;
}



//----------------------------------------------------------------------------
/**
 *	@brief	切断エラーチェック　終了管理関数
 *
 *	@param	wk		ワーク
 *
 *	@retval	BB_DIS_ERROR_NONE,			// エラーなし
 *	@retval	BB_DIS_ERROR_CLOSEING,		// 切断エラー	終了中
 *	@retval	BB_DIS_ERROR_CLOSED,		// 切断エラー	終了
 */
//-----------------------------------------------------------------------------
static u32 BB_DIS_ERROR_Check( BB_WORK* wk )
{
	// 通信切断チェック
	if( MNGM_ERROR_CheckDisconnect( &wk->entry_param ) == TRUE ){

		if( wk->sys.comm_err_data.dis_err == FALSE ){
			// 真っ黒にする
			if( WIPE_SYS_EndCheck() == TRUE ){
				WIPE_SYS_ExeEnd();
			}
			WIPE_SetBrightnessFadeOut(WIPE_FADE_BLACK);

			// 終了宣言
			wk->sys.comm_err_data.dis_err = TRUE;

			return BB_DIS_ERROR_CLOSEING;
		}else{

			// まづは通信切断
			if( MNGM_ERROR_DisconnectWait( &wk->entry_param ) == FALSE ){
				return BB_DIS_ERROR_CLOSEING;
			}

			// 全リソース破棄
			if( wk->p_client != NULL ){
				BB_disp_NameWinDel( wk->p_client );
				BB_disp_InfoWinDel( wk->p_client );
				MainResource_Delete( wk );
			}

			if( wk->result_work != NULL ){
				MNGM_RESULT_Exit( wk->result_work );
				wk->result_work = NULL;
			}

			if( wk->entry_work != NULL ){
				MNGM_ENTRY_Exit( wk->entry_work );
				wk->entry_work = NULL;
			}

			return BB_DIS_ERROR_CLOSED;
		}
	}

	return BB_DIS_ERROR_NONE;
}

#if (WB_FIX == 0)
//ランダムの種を共有しているようなので、同じ計算式で出力する必要がある
//その為、ランダム関数をDPから移植 2009.08.04(火) matsuda
static u32 bb_gf_rand_next;

void bb_gf_rand_init(void)
{
  bb_gf_rand_next = GFUser_GetPublicRand(GFL_STD_RAND_MAX);
}

u32 bb_gf_get_seed(void)
{
	return bb_gf_rand_next;
}

void bb_gf_srand(u32 seed)
{
	bb_gf_rand_next = seed;
}

//another_rand
u16 bb_gf_rand(void)
{
    bb_gf_rand_next = bb_gf_rand_next *1103515245L + 24691;	// 
    return (u16)(bb_gf_rand_next / 65536L) ;
}
#endif
