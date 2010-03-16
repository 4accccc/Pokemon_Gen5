//=============================================================================
/**
 * @file	  myitem_savedata_local.h
 * @brief	  手持ちアイテム構造体の定義ヘッダ
 * @author	tamada
 * @author	hiroyuki nakamura
 * @author	ohno
 * @date	  2005.10.13
 */
//=============================================================================

//------------------------------------------------------------------
/**
 * @brief	手持ちアイテム構造体の定義
 */
//------------------------------------------------------------------
struct _MYITEM {
	ITEM_ST MyNormalItem[ BAG_NORMAL_ITEM_MAX ];	// 手持ちの普通の道具
	ITEM_ST MyEventItem[ BAG_EVENT_ITEM_MAX ];		// 手持ちの大切な道具
	ITEM_ST MySkillItem[ BAG_WAZA_ITEM_MAX ];		// 手持ちの技マシン
	ITEM_ST MyDrugItem[ BAG_DRUG_ITEM_MAX ];		// 手持ちの薬
	ITEM_ST MyNutsItem[ BAG_NUTS_ITEM_MAX ];		// 手持ちの木の実
	u32	cnv_button[DUMMY_SHORTCUT_MAX];									// 便利ボタン @@OO削除予定  便利ウインドウは統合管理される
};

