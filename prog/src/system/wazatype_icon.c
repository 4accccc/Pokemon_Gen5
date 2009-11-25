//==============================================================================
/**
 * @file	wazatype_icon.c
 * @brief	技タイプ＆分類アイコン
 * @author	matsuda
 * @date	2005.10.26(水)
 */
//==============================================================================
#include <gflib.h>
#include "system/wazatype_icon.h"
#include "arc_def.h"
#include "wazatype_icon.naix"


//==============================================================================
//	データ
//==============================================================================
//--------------------------------------------------------------
//	技タイプ
//--------------------------------------------------------------
///技タイプ毎のCGRID
ALIGN4 static const ARCDATID WazaTypeCgrNo[] = {
	NARC_wazatype_icon_p_st_type_normal_NCGR,			   //POKETYPE_NORMAL
	NARC_wazatype_icon_p_st_type_fight_NCGR,               //POKETYPE_BATTLE
	NARC_wazatype_icon_p_st_type_flight_NCGR,              //POKETYPE_HIKOU
	NARC_wazatype_icon_p_st_type_poison_NCGR,              //POKETYPE_POISON
	NARC_wazatype_icon_p_st_type_ground_NCGR,              //POKETYPE_JIMEN
	NARC_wazatype_icon_p_st_type_rock_NCGR,                //POKETYPE_IWA
	NARC_wazatype_icon_p_st_type_insect_NCGR,              //POKETYPE_MUSHI
	NARC_wazatype_icon_p_st_type_ghost_NCGR,               //POKETYPE_GHOST
	NARC_wazatype_icon_p_st_type_steel_NCGR,               //POKETYPE_METAL
	NARC_wazatype_icon_p_st_type_fire_NCGR,                //POKETYPE_FIRE
	NARC_wazatype_icon_p_st_type_water_NCGR,               //POKETYPE_WATER
	NARC_wazatype_icon_p_st_type_grass_NCGR,               //POKETYPE_KUSA
	NARC_wazatype_icon_p_st_type_ele_NCGR,                 //POKETYPE_ELECTRIC
	NARC_wazatype_icon_p_st_type_esp_NCGR,                 //POKETYPE_SP
	NARC_wazatype_icon_p_st_type_ice_NCGR,                 //POKETYPE_KOORI
	NARC_wazatype_icon_p_st_type_dragon_NCGR,              //POKETYPE_DRAGON
	NARC_wazatype_icon_p_st_type_evil_NCGR,                //POKETYPE_AKU
	NARC_wazatype_icon_p_st_type_style_NCGR,               //ICONTYPE_STYLE,		///<格好よさ
	NARC_wazatype_icon_p_st_type_beautiful_NCGR,           //ICONTYPE_BEAUTIFUL,	///<美しさ
	NARC_wazatype_icon_p_st_type_cute_NCGR,                //ICONTYPE_CUTE,			///<可愛さ
	NARC_wazatype_icon_p_st_type_intelli_NCGR,             //ICONTYPE_INTELLI,		///<賢さ
	NARC_wazatype_icon_p_st_type_strong_NCGR,              //ICONTYPE_STRONG,		///<逞しさ
};

///技タイプ毎のパレットオフセット番号
ALIGN4 static const u8 WazaTypePlttOffset[] = {
	0,	//POKETYPE_NORMAL
	0,	//POKETYPE_BATTLE
	1,	//POKETYPE_HIKOU
	1,	//POKETYPE_POISON
	0,	//POKETYPE_JIMEN
	0,	//POKETYPE_IWA
	2,	//POKETYPE_MUSHI
	1,	//POKETYPE_GHOST
	0,	//POKETYPE_METAL
	0,	//POKETYPE_FIRE
	1,	//POKETYPE_WATER
	2,	//POKETYPE_KUSA
	0,	//POKETYPE_ELECTRIC
	1,	//POKETYPE_SP
	1,	//POKETYPE_KOORI
	2,	//POKETYPE_DRAGON
	0,	//POKETYPE_AKU
	0,	//ICONTYPE_STYLE,			///<格好よさ
	1,	//ICONTYPE_BEAUTIFUL,		///<美しさ
	1,	//ICONTYPE_CUTE,			///<可愛さ
	2,	//ICONTYPE_INTELLI,		///<賢さ
	0,	//ICONTYPE_STRONG,		///<逞しさ
};

//--------------------------------------------------------------
//	分類
//--------------------------------------------------------------
///分類毎のCGRID
ALIGN4 static const u32 WazaKindCgrNo[] = {
	NARC_wazatype_icon_p_st_bunrui_buturi_NCGR,
	NARC_wazatype_icon_p_st_bunrui_tokusyu_NCGR,
	NARC_wazatype_icon_p_st_bunrui_henka_NCGR,
};

///分類毎のパレットオフセット番号
ALIGN4 static const u8 WazaKindPlttOffset[] = {
	0,		//KIND_BUTSURI
	1,		//KIND_TOKUSYU
	0,		//KIND_HENNKA
};



//==============================================================================
//
//	技タイプアイコン
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   指定技タイプのCGRIDを取得します
 * @param   waza_type		技タイプ(POKETYPE_NORMAL, POKETYPE_BATTLE..等、又は ICONTYPE_???)
 * @retval  CGRID(ARCのデータINDEX)
 */
//--------------------------------------------------------------
ARCDATID WazaTypeIcon_CgrIDGet(int waza_type)
{
	GF_ASSERT(waza_type < NELEMS(WazaTypeCgrNo));
	return WazaTypeCgrNo[waza_type];
}

//--------------------------------------------------------------
/**
 * @brief   技タイプアイコンのパレットIDを取得します
 * @retval  パレットID(ARCのデータINDEX)
 */
//--------------------------------------------------------------
ARCDATID WazaTypeIcon_PlttIDGet(void)
{
	return NARC_wazatype_icon_st_type_NCLR;
}

//--------------------------------------------------------------
/**
 * @brief   指定技タイプのセルIDを取得します
 * @retval  セルID(ARCのデータINDEX)
 */
//--------------------------------------------------------------
ARCDATID WazaTypeIcon_CellIDGet(void)
{
	return NARC_wazatype_icon_p_st_type_normal_NCER;
}

//--------------------------------------------------------------
/**
 * @brief   指定技タイプのセルアニメIDを取得します
 * @retval  セルアニメID(ARCのデータINDEX)
 */
//--------------------------------------------------------------
ARCDATID WazaTypeIcon_CellAnmIDGet(void)
{
	return NARC_wazatype_icon_p_st_type_normal_NANR;
}

//--------------------------------------------------------------
/**
 * @brief   指定技タイプのパレット番号オフセットを取得します
 * @param   waza_type		技タイプ(NORMAL_TYPE, FIRE_TYPE..等、又は ICONTYPE_???)
 * @retval  パレット番号のオフセット
 */
//--------------------------------------------------------------
u8 WazaTypeIcon_PlttOffsetGet(int waza_type)
{
	GF_ASSERT(waza_type < NELEMS(WazaTypePlttOffset));
	return WazaTypePlttOffset[waza_type];
}

//--------------------------------------------------------------
/**
 * @brief   技タイプアイコンのアーカイブINDEXを取得します
 * @retval  アーカイブINDEX
 */
//--------------------------------------------------------------
ARCID WazaTypeIcon_ArcIDGet(void)
{
	return ARCID_WAZATYPE_ICON;
}





//==============================================================================
//
//	分類アイコン
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   指定技分類のCGRIDを取得します
 * @param   waza_kind		技分類(KIND_BUTURI or KIND_TOKUSYU or KIND_HENNKA)
 * @retval  CGRID(ARCのデータINDEX)
 */
//--------------------------------------------------------------
ARCDATID WazaKindIcon_CgrIDGet(int waza_kind)
{
	GF_ASSERT(waza_kind < NELEMS(WazaKindCgrNo));
	return WazaKindCgrNo[waza_kind];
}

//--------------------------------------------------------------
/**
 * @brief   技分類アイコンのパレットIDを取得します
 * @retval  パレットID(ARCのデータINDEX)
 */
//--------------------------------------------------------------
ARCDATID WazaKindIcon_PlttIDGet(void)
{
	return NARC_wazatype_icon_st_type_NCLR;
}

//--------------------------------------------------------------
/**
 * @brief   指定技分類のセルIDを取得します
 * @retval  セルID(ARCのデータINDEX)
 */
//--------------------------------------------------------------
ARCDATID WazaKindIcon_CellIDGet(void)
{
	return NARC_wazatype_icon_p_st_type_normal_NCER;
}

//--------------------------------------------------------------
/**
 * @brief   指定技分類のセルアニメIDを取得します
 * @retval  セルアニメID(ARCのデータINDEX)
 */
//--------------------------------------------------------------
ARCDATID WazaKindIcon_CellAnmIDGet(void)
{
	return NARC_wazatype_icon_p_st_type_normal_NANR;
}

//--------------------------------------------------------------
/**
 * @brief   指定技分類のパレット番号オフセットを取得します
 * @param   waza_kind		技分類(KIND_BUTURI or KIND_TOKUSYU or KIND_HENNKA)
 * @retval  パレット番号のオフセット
 */
//--------------------------------------------------------------
u8 WazaKindIcon_PlttOffsetGet(int waza_kind)
{
	GF_ASSERT(waza_kind < NELEMS(WazaKindPlttOffset));
	return WazaKindPlttOffset[waza_kind];
}

//--------------------------------------------------------------
/**
 * @brief   技分類アイコンのアーカイブINDEXを取得します
 * @retval  アーカイブINDEX
 */
//--------------------------------------------------------------
ARCID WazaKindIcon_ArcIDGet(void)
{
	return ARCID_WAZATYPE_ICON;
}


