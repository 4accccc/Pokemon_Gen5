/**********************************************************************

Copyright (c) 2006 Polygon Magic, Inc.

************************************************************************/
/*!
	file	npBpcImporter.h
	author	kubota
*/
#ifndef NPBPCIMPORTER_H
#define NPBPCIMPORTER_H
/*
#ifdef __cplusplus
extern "C" {
#endif
*/
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// include
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "npfx.h"
#include "core/npConst.h"
#include "npImporterDef.h"
#include "npBpc.h"

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// define
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// enum
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//座標系
typedef enum __NP_BPC_COORDINATESYSTEM
{
	NP_BPC_LEFT_HANDED,
	NP_BPC_RIGHT_HANDED
} NP_BPC_COORDINATESYSTEM;

typedef enum __NP_BPC_KINEMATICS
{
	NP_BPC_TRANSLATION,
	NP_BPC_ROTATION
} NP_BPC_KINEMATICS;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// typedef
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//! 変換情報レコード
typedef struct _CONVRECORD
{
	npS32	bpcfileNum;		// 左辺(file side number)
	npS32	engineNum;		// 右辺(engine side number)
}_CONVRECORD;

//!	ファイルインポート用コンテキスト
/*!	
	\brief		読み込みデータの中間保存場所。
				これは、読み込んだものの、まだ代入先となるオブジェクトが
				生成できない場合が有る為。
*/
typedef struct NPBPCTEXTURE_CONTEXT
{
	//texture と関係ないが version 一時保管場所
	npU32		version;
	// テクスチャカット情報中間保存場所
	// アトリビュート一個の分しか保存することを考えていないので注意。
	// 次のアトリビュート設定する前に使用しておかないと上書きされる。
	npS32		texCutNum;								//!< テクスチャカットの数
	npS32		reserved0;										
	npS32		reserved1;										
	npS32		texCutBankTbl[NP_BPC_KEYFRAME_MAX];
	npFVECTOR	texCutUVTbl[NP_BPC_KEYFRAME_MAX];	//!< テクスチャカットのＵＶテーブル
} NPBPCTEXTURE_CONTEXT;

// テクスチャカットデータ
typedef struct _BPC_TEXCUT
{
	npS32		bank;
	npFVECTOR	rect;
} _BPC_TEXCUT;

typedef struct _BPC_VELOCITY
{
	npFVECTOR	iniValue;
	npFVECTOR	iniVelocity;
	npFVECTOR	iniAcc;
	npFVECTOR	iniRc;
} _BPC_VELOCITY;

typedef struct _BPC_PARTICLEFCURVE
{
	npFVECTOR	ini;
	npFVECTOR	dir;
	npS32		type;
	npS32		elementNum;
	npS32		sequenceNum;
	npS32		mode;
	npFLOAT		hermiteRate;
	
} _BPC_PARTICLEFCURVE;
// ヒストグラム型生成
typedef struct _BPC_HISTGRAMGEN
{
	npS32		loop;
	npS32		randomFlag;
	npS32		sequenceNum;
	//npS32		num = sequenceNum;

} _BPC_HISTGRAMGEN;
// スクロール型生成
typedef struct _BPC_SCROLLGEN
{
	npFVECTOR		initialPos;
	npFLOAT			velocity[ 2 ];
	npS32			loop[ 2 ];
	npS32			textureBank;
} _BPC_SCROLLGEN;

// さまざまな設定を格納する
typedef struct __NP_BPC_DESC
{
	NP_BPC_COORDINATESYSTEM		coordinateSystem;// 座標系を反映する
	npU32						fps;
} NP_BPC_DESC;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// BlendMagic tool relation
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifdef NP_TOOL_USER
// ツリーノードの数
#define DEF_BPC_PARTICLE_COUNT	256

//! particle name hanger
typedef struct __NP_BPC_PARTICLE_CONTENTS 
{
	char	sParticleName[DEF_BPC_PARTICLE_COUNT][256];//particle name
	npU32	iParticleCount;// particle
	npU32	iParticleCounter;
} NP_BPC_PARTICLE_CONTENTS;

// texture cut list
typedef struct __NP_BPC_PARTICLE_TEXLIST
{
	npU16		iParticleCount;
	npU16		iParticleCounter;
	npU16		nNumOfUVList[DEF_BPC_PARTICLE_COUNT];
	npFVECTOR	uvList[DEF_BPC_PARTICLE_COUNT][NP_PARTICLE_KEYFRAME_MAX];
	npU16		nNumOfkey[DEF_BPC_PARTICLE_COUNT];
	npU16		keyIdList[DEF_BPC_PARTICLE_COUNT][NP_PARTICLE_KEYFRAME_MAX];
	npU16		keyframeList[DEF_BPC_PARTICLE_COUNT][NP_PARTICLE_KEYFRAME_MAX];

} NP_BPC_PARTICLE_TEXLIST;

//! Particle name is allocated.
//! Because a tree for the tool needs particle name.
extern	NP_BPC_PARTICLE_CONTENTS bpcContents;
// texture cut list を保持しておく
extern	NP_BPC_PARTICLE_TEXLIST bpcTexCutList;

#endif


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// function
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//ここが入り口
// file を読み込み、effect attribute をセットする
npBOOL npUtilParticleLoadBpc(void* pBuf ,npSIZE nSize ,NP_BPC_DESC*	pDesc ,npPARTICLEEMITCYCLE** ppRoot );

/*
ルートアトリビュートを初期化。
ルートアトリビュートに条件を限られているので、共用体で持たれている部分も初期化できる。
*/
npBOOL initRootParticleAttribute( npPARTICLEATTRIBUTE* pAttr );
/*
アトリビュートを初期化
読み込みデータにより、内部構造が選択される共用体の部分は初期化出来ない
*/
npBOOL initParticleAttribute( npPARTICLEATTRIBUTE *pAttr );
//オブジェクトを初期化
npBOOL initObject( npOBJECT *pObj, npS32 type, npU32 state );
//挙動パラメータを初期化
npBOOL initKinematics( npPARTICLEKINEMATICS *pKine, npS32 type );

//root のtranslation ,rotation の設定
void* setupRootKinematics( npPARTICLEKINEMATICS *pKine, void* pBuf );
void* setupRootVelocity( npPARTICLEVELOCITY *pVelo, void* pBuf );
//root 固定値の設定
void* setupRootFixed( npPARTICLEFIXED *pFix, void* pBuf );

//アトリビュートの内容物構築(再帰的に末端の子孫まで構築していく)
npBOOL buildParticleAttribute( npPARTICLEATTRIBUTE **ppChild,NPBPCTEXTURE_CONTEXT *pTexContext , NP_BPC_DESC* pDesc, void *pBuf );
//テクスチャカット情報を読み込んで保存して置く
void* readKeepTexCutInfo( NPBPCTEXTURE_CONTEXT *pTexContext, void *pBuf );

//child の発生の仕方
void* setupParticleFrameCycle( npPARTICLEFRAMECYCLE *pCycle, void *pBuf );
//発生タイプ:constant
void* setupParticleCnstCycle( npPARTICLECNSTCYCLE *pCycle, void *pBuf );
//発生タイプ:sequence
void* setupParticleSeqCycle( npPARTICLESEQCYCLE *pCycle, void *pBuf );
//発生タイプ共通
void* setupParticleFrameCycleCommon( npPARTICLEFRAMECYCLE *pCycle, void *pBuf );

//アトリビュートの内容物構築(再帰的に末端の子孫まで構築していく)
npBOOL buildParticleAttributeContent( npPARTICLEATTRIBUTE** ppAttribute, NPBPCTEXTURE_CONTEXT *pTexContext , NP_BPC_DESC* pDesc ,void *pBuf );

npBOOL buildParticleItem( npPARTICLEITEM** ppItem, NPBPCTEXTURE_CONTEXT *pTexContext, NP_BPC_DESC* pDesc, void *pBuf );
npBOOL buildParticleQuad( npPARTICLEQUAD** ppQuad, NPBPCTEXTURE_CONTEXT *pTexContext, NP_BPC_DESC* pDesc, void* pBuf );
npBOOL buildParticleEmitCycle( npPARTICLEEMITCYCLE **ppEmitCycle, NPBPCTEXTURE_CONTEXT *pTexContext, NP_BPC_DESC* pDesc , void* pBuf );
//放出タイミングオブジェクトのセットアップ
void* setupParticleEmitCycle( npPARTICLEEMITCYCLE *pEmitCycle, NPBPCTEXTURE_CONTEXT *pTexContext, NP_BPC_DESC* pDesc , void *pBuf );
void* setupParticleKinematics( npPARTICLEKINEMATICS *pKine, npPARTICLEATTRIBUTE *pAttr, NPBPCTEXTURE_CONTEXT *pTexContext, void* pBuf );

void* setupParticleVelocity( npPARTICLEVELOCITY *pVelo, void* pBuf );
void* setupVelocity( npPARTICLEVELOCITY *pVelo, void* pBuf );
void* setupParticleFixed( npPARTICLEFIXED *pFix, void* pBuf );
//固定値の設定
void* setupFixed( npPARTICLEFIXED *pFixed, void* pBuf );
//BPCヒストグラム（パラパラアニメ）からNPFカーブ挙動データをセットアップ
void* setupParticleTwoFCurveFromHistgram( npPARTICLEKINEMATICS *pUVCoord, npPARTICLEITEM *pItem, NPBPCTEXTURE_CONTEXT *pTexContext, void *pBuf );
void* setupParticleFCurveFromSeqLine( npPARTICLEKINEMATICS *pUVCoord, npPARTICLEITEM *pItem, NPBPCTEXTURE_CONTEXT *pTexContext, void *pBuf );
//スクロール情報から二つのベロシティ情報を設定する
void* setupParticleTwoVelocityFromScroll( npPARTICLEKINEMATICS *pUVCoord, npPARTICLEITEM *pItem, NPBPCTEXTURE_CONTEXT *pTexContext,void *pBuf );
//パーティクルFカーブパラメータセットアップ
void* setupParticleFCurve( npPARTICLEFCURVE *pFCrv, void *pBuf );
//Fカーブパラメータセットアップ
void* setupFCurve( npPARTICLEFCURVE *pFCrv, void *pBuf );
//パーティクルNULLパラメータからFIXEDパラメータをセットアップ
void* setupParticleFixedFromNull( npPARTICLEFIXED *pFixed, void *pBuf );
// 座標系対応 //////////////////////////////////////////////
// left handed type
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void* setupRootLeftKinematics( npPARTICLEKINEMATICS *pKine, NP_BPC_KINEMATICS kineType , void* pBuf );
void* setupParticleLeftKinematics( npPARTICLEKINEMATICS *pKine, npPARTICLEATTRIBUTE *pAttr, NPBPCTEXTURE_CONTEXT *pTexContext, NP_BPC_KINEMATICS kineType , void* pBuf );
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void* setupRootLeftFixed( npPARTICLEFIXED *pFix, NP_BPC_KINEMATICS kineType , void* pBuf );
void* setupParticleLeftFixed( npPARTICLEFIXED *pFix, NP_BPC_KINEMATICS kineType , void* pBuf );
void* setupLeftFixed( npPARTICLEFIXED *pFixed, NP_BPC_KINEMATICS kineType , void* pBuf );
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void* setupRootLeftVelocity( npPARTICLEVELOCITY *pVelo, NP_BPC_KINEMATICS kineType , void* pBuf );
void* setupParticleLeftVelocity( npPARTICLEVELOCITY *pVelo, NP_BPC_KINEMATICS kineType , void* pBuf );
void* setupLeftVelocity( npPARTICLEVELOCITY *pVelo, NP_BPC_KINEMATICS kineType , void* pBuf );
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void* setupParticleLeftFCurve( npPARTICLEFCURVE *pFCrv, NP_BPC_KINEMATICS kineType , void *pBuf );
void* setupLeftFCurve( npPARTICLEFCURVE *pFCrv, NP_BPC_KINEMATICS kineType , void *pBuf );

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*!

	親ノードの最後に自身のノードを追加する

	@param	pParent	親ノード
	@param	pNode	追加するノード

	@note	pNode はルート( 親がない )である必要がある

*/
void npParticleAppendNode( npPARTICLENODE* pParent, npPARTICLENODE* pNode );
/*
#ifdef __cplusplus
}
#endif
*/
#endif //NPBPCIMPORTER_H

