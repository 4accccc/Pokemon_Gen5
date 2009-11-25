//==============================================================================
/**
 * @file	wazatype_icon.h
 * @brief	技タイプアイコンのヘッダ
 * @author	matsuda
 * @date	2005.10.26(水)
 */
//==============================================================================
#ifndef __WAZATYPE_ICON_H__
#define __WAZATYPE_ICON_H__


//==============================================================================
//	定数定義
//==============================================================================
///アイコンタイプ(技タイプ以外のタイプ)
enum{
	ICONTYPE_STYLE = 17,	///<格好よさ
	ICONTYPE_BEAUTIFUL,		///<美しさ
	ICONTYPE_CUTE,			///<可愛さ
	ICONTYPE_INTELLI,		///<賢さ
	ICONTYPE_STRONG,		///<逞しさ
};

///技タイプアイコンで使用するパレット本数
#define WTI_PLTT_NUM		(3)
///技分類アイコンで使用するパレット本数
#define WKI_PLTT_NUM		(3)

///技タイプアイコングラフィックの圧縮フラグ(キャラクタ)
#define WAZATYPEICON_COMP_CHAR		(0)
///技タイプアイコングラフィックの圧縮フラグ(パレット)
#define WAZATYPEICON_COMP_PLTT		(0)
///技タイプアイコングラフィックの圧縮フラグ(セル)
#define WAZATYPEICON_COMP_CELL		(0)
///技タイプアイコングラフィックの圧縮フラグ(セルアニメ)
#define WAZATYPEICON_COMP_CELLANM	(0)

///技分類アイコングラフィックの圧縮フラグ(キャラクタ)
#define WAZAKINDICON_COMP_CHAR		(0)
///技分類アイコングラフィックの圧縮フラグ(パレット)
#define WAZAKINDICON_COMP_PLTT		(0)
///技分類アイコングラフィックの圧縮フラグ(セル)
#define WAZAKINDICON_COMP_CELL		(0)
///技分類アイコングラフィックの圧縮フラグ(セルアニメ)
#define WAZAKINDICON_COMP_CELLANM	(0)

///技タイプアイコンのOAMタイプ定義
#define WAZATYPEICON_OAMTYPE		(OAMTYPE_32x16)


//==============================================================================
//	外部関数宣言
//==============================================================================
extern ARCDATID WazaTypeIcon_CgrIDGet(int waza_type);
extern ARCDATID WazaTypeIcon_PlttIDGet(void);
extern ARCDATID WazaTypeIcon_CellIDGet(void);
extern ARCDATID WazaTypeIcon_CellAnmIDGet(void);
extern u8 WazaTypeIcon_PlttOffsetGet(int waza_type);
extern ARCID WazaTypeIcon_ArcIDGet(void);

extern ARCDATID WazaKindIcon_CgrIDGet(int waza_kind);
extern ARCDATID WazaKindIcon_PlttIDGet(void);
extern ARCDATID WazaKindIcon_CellIDGet(void);
extern ARCDATID WazaKindIcon_CellAnmIDGet(void);
extern u8 WazaKindIcon_PlttOffsetGet(int waza_kind);
extern ARCID WazaKindIcon_ArcIDGet(void);


#endif	//__WAZATYPE_ICON_H__

