//==============================================================================
/**
 * @file	gds_data_conv.c
 * @brief	ボックスやドレスアップ等をGDSサーバーで使用する形式に変換するツール
 * @author	matsuda
 * @date	2007.12.10(月)
 */
//==============================================================================
#include <gflib.h>
#include "savedata\save_control.h"
#include "pm_define.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "savedata/gds_profile.h"
#include "net_app/gds/gds_profile_local.h"
#include "battle/btl_common.h"
#include "savedata\battle_rec.h"
#include "savedata\battle_rec_local.h"
#include "gds_battle_rec.h"
#include "gds_ranking.h"
#include "gds_boxshot.h"
#include "gds_dressup.h"

#include "poke_tool/monsno_def.h"
//#include "gflib/strbuf_family.h"

#include "savedata/gds_profile.h"
#include "savedata/gds_profile_types.h"
#include "gds_data_conv.h"




#if GDS_FIX
//==============================================================================
//	ドレスアップ
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   ドレスアップデータをGTドレスアップ形式に変換する
 *
 * @param   dress			元データとなるドレスアップデータ
 * @param   gt_dress		変換後のデータセット先
 */
//--------------------------------------------------------------
void GDS_CONV_Dress_to_GTDress(SAVE_CONTROL_WORK *sv, const IMC_TELEVISION_SAVEDATA * dress, GT_GDS_DRESS * gt_dress)
{
	ImcTelevision_to_GTDress(dress, gt_dress);
	
	//チェックサム作成
	gt_dress->crc.crc16ccitt_hash = 
		GFL_STD_CrcCalc(gt_dress, sizeof(GT_GDS_DRESS) - GT_GDS_CRC_SIZE);
}

//--------------------------------------------------------------
/**
 * @brief   GTドレスアップをドレスアップ形式に変換する
 *
 * @param   gt_dress		元データとなるGTドレスアップデータ
 * @param   dress			変換後のデータセット先
 */
//--------------------------------------------------------------
void GDS_CONV_GTDress_to_Dress(const GT_GDS_DRESS * gt_dress, IMC_TELEVISION_SAVEDATA * dress)
{
	GTDress_to_ImcTelevision(gt_dress, dress);
}


//==============================================================================
//	ボックスショット
//		※ボックスは描画部分もGT構造体を使用するため、自分のボックスを送信する時の
//		  変換命令だけが必要
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   ボックスデータをGTボックスショットデータに変換
 *
 * @param   boxdata			ボックスデータへのポインタ
 * @param   tray_number		トレイ番号
 * @param   gt_box			データ代入先
 * @param   heap_id			テンポラリとして使用するヒープのID
 */
//--------------------------------------------------------------
void GDS_CONV_Box_to_GTBox(SAVE_CONTROL_WORK *sv, const BOX_DATA *boxdata, int tray_number, GT_BOX_SHOT *gt_box, int heap_id)
{
	STRBUF *box_name;
	int i;

	GFL_STD_MemClear( gt_box, sizeof( GT_BOX_SHOT ) );
	
	box_name = GFL_STR_CreateBuffer(GT_PLW_BOX_TRAYNAME_SIZE * 3, heap_id);	//念のため多めに確保
	BOXDAT_GetBoxName(boxdata, tray_number, box_name);
	STRBUF_GetStringCode(box_name, gt_box->box_name, GT_PLW_BOX_TRAYNAME_SIZE);
	GFL_STR_DeleteBuffer(box_name);
	
	for(i = 0; i < GT_PLW_BOX_MAX_POS; i++){
		gt_box->monsno[i] = BOXDAT_PokeParaGet(boxdata, tray_number, i, ID_PARA_monsno, NULL);
		gt_box->personal_rnd[i] = 
			BOXDAT_PokeParaGet(boxdata, tray_number, i, ID_PARA_personal_rnd, NULL);
		gt_box->id_no[i] = 
			BOXDAT_PokeParaGet(boxdata, tray_number, i, ID_PARA_id_no, NULL);
		if(BOXDAT_PokeParaGet(boxdata, tray_number, i, ID_PARA_monsno_egg, NULL) == MONSNO_TAMAGO){
			gt_box->egg_bit |= 1 << i;
		}
		gt_box->form_no[i] = BOXDAT_PokeParaGet(boxdata, tray_number, i, ID_PARA_form_no, NULL);
	}
	
	gt_box->wallPaper = BOXDAT_GetWallPaperNumber(boxdata, tray_number);
	
	//サーバー送信時にセットするのでここでは0初期化
	gt_box->category_no = 0;
	
	//チェックサム作成
	gt_box->crc.crc16ccitt_hash = 
		GFL_STD_CrcCalc(gt_box, sizeof(GT_BOX_SHOT) - GT_GDS_CRC_SIZE);
}

//--------------------------------------------------------------
/**
 * @brief   GTボックスショットに登録カテゴリー番号を代入
 *
 * @param   gt_box			GTボックスショットへのポインタ
 * @param   category_no		登録カテゴリー番号
 *
 * チェックサムを作り直す為、登録カテゴリー番号をセットする場合は必ずこの関数を使用してください
 */
//--------------------------------------------------------------
void GDS_GTBoxShot_SetCategoryNo(SAVE_CONTROL_WORK *sv, GT_BOX_SHOT *gt_box, int category_no)
{
	gt_box->category_no = category_no;
	//チェックサムを作り直す
	gt_box->crc.crc16ccitt_hash = 
		GFL_STD_CrcCalc(gt_box, sizeof(GT_BOX_SHOT) - GT_GDS_CRC_SIZE);
}

#endif  //GDS_FIX

//==============================================================================
//
//	GDSプロフィール
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   GDSプロフィールをGTGDSプロフィール形式に変換する
 *
 * @param   gpp			元データとなるGDSプロフィール
 * @param   gt_gpp		変換後のデータセット先
 */
//--------------------------------------------------------------
void GDS_CONV_GDSProfile_to_GTGDSProfile(SAVE_CONTROL_WORK *sv, const GDS_PROFILE_PTR gpp, GT_GDS_PROFILE * gt_gpp)
{
	GF_ASSERT(sizeof(GT_GDS_PROFILE) == sizeof(GDS_PROFILE));
	
	GFL_STD_MemClear(gt_gpp, sizeof(GT_GDS_PROFILE));
	GFL_STD_MemCopy(gpp, gt_gpp, sizeof(GT_GDS_PROFILE));
}

//--------------------------------------------------------------
/**
 * @brief   GTGDSプロフィールをGDSプロフィール形式に変換する
 *
 * @param   gt_dress		元データとなるGTドレスアップデータ
 * @param   dress			変換後のデータセット先
 */
//--------------------------------------------------------------
void GDS_CONV_GTGDSProfile_to_GDSProfile(const GT_GDS_PROFILE * gt_gpp, GDS_PROFILE_PTR gpp)
{
	GF_ASSERT(sizeof(GT_GDS_PROFILE) == sizeof(GDS_PROFILE));

	GFL_STD_MemCopy(gt_gpp, gpp, sizeof(GT_GDS_PROFILE));
}

#if GDS_FIX
//--------------------------------------------------------------
/**
 * @brief   GDSプロフィールをランキングプロフィール形式に変換する
 *
 * @param   gpp			元データとなるGDSプロフィール
 * @param   gt_rp		変換後のデータセット先
 */
//--------------------------------------------------------------
void GDS_CONV_GDSProfile_to_GTRankingProfile(SAVE_CONTROL_WORK *sv, const GDS_PROFILE_PTR gpp, GT_RANKING_PROFILE * gt_rp)
{
	GFL_STD_MemClear(gt_rp, sizeof(GT_RANKING_PROFILE));
	
	gt_rp->version_code = gpp->version_code;
	gt_rp->language = gpp->language;
	gt_rp->birthday_month = gpp->birthday_month;
	gt_rp->trainer_view = gpp->trainer_view;
	gt_rp->monsno = gpp->monsno;
	gt_rp->egg_flag = gpp->egg_flag;
}
#endif  //GDS_FIX
