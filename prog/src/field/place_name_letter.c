////////////////////////////////////////////////////////////////////////////
/**
 * @file   place_name_letter.c
 * @brief  地名表示の文字オブジェクト
 * @author obata_toshihiro
 * @date   2010.03.26
 */
////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "place_name_letter.h"
#include "arc/place_name.naix"
#include "field/zonedata.h"

#include "print/printsys.h"  // for PRINTSYSS_xxxx
#include "field_oam_pal.h"  // for FLDOAM_PALNO_PLACENAME


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//    文字オブジェクト
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//===========================================================================
// ■ デバッグ
//===========================================================================
#ifdef PM_DEBUG
//#define DEBUG_MODE
#define PRINT_TARGET (3) // 出力先

#define STRING_LENGTH_CHECK

#endif //PM_DEBUG

#ifdef STRING_LENGTH_CHECK
extern BOOL PlaceNameLong;  //地名表示長い番の参照先
#endif //STRING_LENGTH_CHECK

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//    地名表示上部オブジェクト
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//===========================================================================
// ■ 定数
//===========================================================================
#define PN_PLATE_BMP_XSIZE_CHR (32) // ビットマップの x サイズ ( キャラ単位 )
#define PN_PLATE_BMP_YSIZE_CHR (2) // ビットマップの y サイズ ( キャラ単位 )
#define	COLOR_NO_LETTER (1)  // 文字本体のカラー番号
#define	COLOR_NO_SHADOW (2)  // 影部分のカラー番号
#define	COLOR_NO_BACK   (0)  // 背景部のカラー番号

//リクエストの種類
typedef enum
{
  PN_PLATE_REQ_NONE,    //リクエストなし
  PN_PLATE_REQ_ENTER,   //出現
  PN_PLATE_REQ_OUT,     //出て行く
  PN_PLATE_REQ_HIDE,    //隠れる

  PN_PLATE_REQ_SETUP,   //設定
  PN_PLATE_REQ_MAX,     //最大値
}PN_PLATE_REQ;

//===========================================================================
// ■ 地名表示オブジェクト抽象型
//===========================================================================
typedef struct
{
  u32 wk_size;
  void (*init)( struct _PN_PLATE *, void *, const void * );
  void (*exit)( struct _PN_PLATE *, void * );
  void (*setup)( struct _PN_PLATE *, void *, const void * );
  BOOL (*update)( struct _PN_PLATE *, void * );
}PN_PLATE_INTERFACE;

//===========================================================================
// ■ 動作オブジェクト
//===========================================================================
#define PN_MOVE_LINER
typedef struct
{
  GFL_POINT init;       //初期座標
  GFL_POINT end;        //目標位置
  GFL_POINT init_speed; //初速
  GFL_POINT end_speed;  //終了時の速度
  s16 startFrame;
  s16 endFrame;
}PN_MOVE_INIT_PARAM;
typedef struct
{
#ifdef PN_MOVE_LINER
  s16 x;
  s16 y;
  s32 now_frame;
  PN_MOVE_INIT_PARAM  param;
#else
  fx32 x;
  fx32 y;
  fx32 sx;
  fx32 sy;
  fx32 ax;
  fx32 ay;
  s32 tx;
  s32 ty;
#endif
}PN_MOVE;
static void PN_MOVE_Init( PN_MOVE *wk, const PN_MOVE_INIT_PARAM *param );
static void PN_MOVE_Main( PN_MOVE *wk );
static BOOL PN_MOVE_IsEnd( const PN_MOVE *wk );
static void PN_MOVE_GetPos( const PN_MOVE *wk, GFL_CLACTPOS *pos );


//===========================================================================
// ■ 地名表示オブジェクト
//===========================================================================
struct _PN_PLATE
{
  u32   seq;              //処理シーケンス
  u32   surface;          //CLUNITのサーフェース
  HEAPID heapID;          //ヒープID
  GFL_CLUNIT  *unit;      //ユニット
  ARCHANDLE* arcHandle;   //アーカイブ
  GFL_CLWK* backobj;      //黒い背景プレート
  PN_PLATE_REQ  req;      //処理リクエスト
  BOOL  is_update;        //処理中かどうか
  const PN_PLATE_INTERFACE *interface;  //処理別インターフェイス
  void *wk; //処理別のワーク
  u16 startFrame;
  u16 endFrame;
};

//===========================================================================
// ■ 地名表示オブジェクト  各クラスの実体
//===========================================================================
typedef struct
{
  PN_PLATE_TOP_SETUP_PARAM    setup_param;  //設定情報
  BMPOAM_ACT_PTR  bmpOamActor;	            //BMPOAMアクター
  GFL_BMP_DATA*   bmp;		                  //ビットマップ
  PN_MOVE         move;                     //移動計算
  GFL_CLWK*       placeIcon;                //場所アイコン
  u16             placeCgrIdx;              //場所アイコン用キャラ
  u16             placePltIdx;              //場所アイコン用パレット
  u16             placeCelIdx;              //場所アイコン用セル
  u16             placeIconIndex;           //ZoneTableから読み込んだ番号
} PN_PLATE_TOP;
typedef struct
{
  PN_PLATE_BOTTOM_SETUP_PARAM setup_param;  //設定情報
  PN_MOVE         move;                     //移動計算
  GFL_CLWK*       seasonIcon;               //季節アイコン
  u16             seasonCgrIdx;             //季節アイコン用キャラ
  u16             seasonPltIdx;             //季節アイコン用パレット
  u16             seasonCelIdx;             //季節アイコン用セル
} PN_PLATE_BOTTOM;

//===========================================================================
// ■private prototype
//===========================================================================
static void Pn_Plate_Request( PN_PLATE *plate, PN_PLATE_REQ req );
static void Pn_Plate_Top_Init( PN_PLATE *plate, void *wk_adrs, const void *param_adrs );
static void Pn_Plate_Top_Exit( PN_PLATE *plate, void *wk_adrs );
static void Pn_Plate_Top_Setup( PN_PLATE *plate, void *wk_adrs, const void *setup_param );
static BOOL Pn_Plate_Top_Update( PN_PLATE *plate, void *wk_adrs );
static void Pn_Plate_Bottom_Init( PN_PLATE *plate, void *wk_adrs, const void *param_adrs );
static void Pn_Plate_Bottom_Exit( PN_PLATE *plate, void *wk_adrs );
static void Pn_Plate_Bottom_Setup( PN_PLATE *plate, void *wk_adrs, const void *setup_param );
static BOOL Pn_Plate_Bottom_Update( PN_PLATE *plate, void *wk_adrs );

//===========================================================================
// ■data
//===========================================================================
//プレート上部と下部の違いを抽象化する機構
static const PN_PLATE_INTERFACE sc_pn_plate_interface[PN_PLATE_TYPE_MAX ] =
{
  //PN_PLATE_TYPE_TOP
  {
    sizeof(PN_PLATE_TOP),
    Pn_Plate_Top_Init,
    Pn_Plate_Top_Exit,
    Pn_Plate_Top_Setup,
    Pn_Plate_Top_Update
  },
  //PN_PLATE_TYPE_BOTTOM
  {
    sizeof(PN_PLATE_BOTTOM),
    Pn_Plate_Bottom_Init,
    Pn_Plate_Bottom_Exit,
    Pn_Plate_Bottom_Setup,
    Pn_Plate_Bottom_Update
  }
};
//上部が出現するときの動作
static const PN_MOVE_INIT_PARAM sc_pn_plate_top_enter_param =
{
  //初期座標
  {
    0,
    -32,
  },
  //目標位置
  {
    0,
    0,
  },
  //初速
  {
    0,
    2
  },
  //終了時の速度
  {
    0,
    0
  },
};
//上部が出て行くときの動作
static const PN_MOVE_INIT_PARAM sc_pn_plate_top_out_param =
{
  //初期座標
  {
    0,
    0,
  },
  //目標位置
  {
    0,
    -56-8,
  },
  //初速
  {
    0,
    -2
  },
  //終了時の速度
  {
    0,
    0
  },
};
//下部が出現するときの動作
static const PN_MOVE_INIT_PARAM sc_pn_plate_bottom_enter_param =
{
  //初期座標
  {
    0,
    32,
  },
  //目標位置
  {
    0,
    0,
  },
  //初速
  {
    0,
    -2
  },
  //終了時の速度
  {
    0,
    0
  },
};
//下部がでていくときの動作
static const PN_MOVE_INIT_PARAM sc_pn_plate_bottom_out_param =
{
  //初期座標
  {
    0,
    0,
  },
  //目標位置
  {
    0,
    32
  },
  //初速
  {
    0,
    -2
  },
  //終了時の速度
  {
    0,
    0
  },
};

//上画面バーのローカル座標
static const GFL_CLACTPOS sc_pn_plate_top_bar_pos =
{
  256/2,
  192/2,
};
//下画面バーのローカル座標
static const GFL_CLACTPOS sc_pn_plate_bottom_bar_pos =
{
  256/2,
  192/2,
};
//上画面地名文字OBJのローカル座標
static const GFL_CLACTPOS sc_pn_plate_name_pos =
{
  30,
  3,
};
//上画面場所アイコン
static const GFL_CLACTPOS sc_pn_plate_icon_pos =
{
  16,
  8,
};
//下画面季節アイコン
static const GFL_CLACTPOS sc_pn_plate_season_pos =
{
  256 - 30,
  192 - 4,
};

//===========================================================================
// ■public method
//===========================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  生成処理
 *
 *	@param	HEAPID heapID       ヒープID
 *	@param	type                プレートタイプ
 *	@param	void *init_param    プレートタイプ別の初期化構造体
 *	@param	*unit               セルユニット
 *	@param  arcHandle           アーカイブハンドル
 *	@param	surface             レンダーサーフェースインデックス
 *	@param	backobjPlttRegIdx   プレートで使用するパレットリソースインデックス
 *	@param	backobjCgrRegIdx    プレートで使用するキャラリソースインデックス  
 *	@param	backobjCellRegIdx   プレートで使用するセルリソースインデックス
 *
 *	@return プレートオブジェクト
 */
//-----------------------------------------------------------------------------
PN_PLATE* PN_PLATE_Create( HEAPID heapID, PN_PLATE_TYPE type, const void *init_param, GFL_CLUNIT *unit, ARCHANDLE* arcHandle, u16 surface, u16 backobjPlttRegIdx, u16 backobjCgrRegIdx, u16 backobjCellRegIdx )
{
  const PN_PLATE_INTERFACE*  interface = &sc_pn_plate_interface[ type ];

  //作成
  u32 size  = sizeof(PN_PLATE) + interface->wk_size;
  PN_PLATE  *plate = GFL_HEAP_AllocMemory( heapID, size );
  GFL_STD_MemClear( plate, size );
  plate->heapID = heapID;
  plate->unit = unit;
  plate->surface  = surface;
  plate->arcHandle  = arcHandle;

  //タイプ別の処理をアタッチ
  plate->interface = interface;
  plate->wk = (s8*)plate + sizeof(PN_PLATE);

  //共通処理
  {
    GFL_CLWK_DATA data  = {0};
    plate->backobj  = GFL_CLACT_WK_Create( unit, 
          backobjCgrRegIdx, backobjPlttRegIdx, backobjCellRegIdx,
          &data, 0, heapID );
    GFL_CLACT_WK_SetObjMode( plate->backobj, GX_OAM_MODE_OBJWND );
    GFL_CLACT_WK_SetDrawEnable( plate->backobj, FALSE );
  }

  //初期化
  plate->interface->init( plate, plate->wk, init_param );

  return plate;
}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄処理
 *
 *	@param	PN_PLATE *plate プレートオブジェクト
 */
//-----------------------------------------------------------------------------
void PN_PLATE_Delete( PN_PLATE *plate )
{
  //終了
  plate->interface->exit( plate, plate->wk );

  //共通処理
  GFL_CLACT_WK_Remove( plate->backobj );

  //破棄
  GFL_HEAP_FreeMemory( plate );
}
//----------------------------------------------------------------------------
/**
 *	@brief  セットアップ処理
 *
 *	@param	PN_PLATE *plate     プレートオブジェクト
 *	@param	void *setup_param   タイプ別の設定構造体
 */
//-----------------------------------------------------------------------------
void PN_PLATE_SetupReq( PN_PLATE *plate, const void *setup_param )
{
  plate->interface->setup( plate, plate->wk, setup_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 *
 *	@param	PN_PLATE *plate プレートオブジェクト
 */
//-----------------------------------------------------------------------------
void PN_PLATE_Main( PN_PLATE *plate )
{
  if( plate->is_update )
  {
    plate->is_update  = plate->interface->update( plate, plate->wk );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  出現リクエスト
 *
 *	@param	PN_PLATE *plate プレートオブジェクト
 *	@param	start 開始フレーム数
 *	@param	end 出現するフレーム数
 */
//-----------------------------------------------------------------------------
void PN_PLATE_EnterReq( PN_PLATE *plate, s16 start, s16 end )
{
  plate->startFrame = start;
  plate->endFrame = end;
  Pn_Plate_Request( plate, PN_PLATE_REQ_ENTER );
}
//----------------------------------------------------------------------------
/**
 *	@brief  出現リクエスト
 *
 *	@param	PN_PLATE *plate プレートオブジェクト
 *	@param	start 開始フレーム数
 *	@param	end 出現するフレーム数
 */
//-----------------------------------------------------------------------------
void PN_PLATE_OutReq( PN_PLATE *plate, s16 start, s16 end )
{
  plate->startFrame = start;
  plate->endFrame = end;
  Pn_Plate_Request( plate, PN_PLATE_REQ_OUT );
}
//----------------------------------------------------------------------------
/**
 *	@brief  消去リクエスト
 *
 *	@param	PN_PLATE *plate プレートオブジェクト
 */
//-----------------------------------------------------------------------------
void PN_PLATE_HideReq( PN_PLATE *plate )
{
  Pn_Plate_Request( plate, PN_PLATE_REQ_HIDE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リクエスト処理
 *
 *	@param	PN_PLATE *plate プレートオブジェクト
 *	@param	req   リクエストの種類
 */
//-----------------------------------------------------------------------------
static void Pn_Plate_Request( PN_PLATE *plate, PN_PLATE_REQ req )
{
  GF_ASSERT( req < PN_PLATE_REQ_MAX );

  if( plate->req != req )
  {
    plate->req  = req;
    plate->seq  = 0;
    plate->is_update  = TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リクエストの処理が完了したかどうかチェック
 *
 *	@param	const PN_PLATE *plate プレートオブジェクト
 *	@retval TRUE  処理終了
 *	@retval FALSE 処理中
 */
//-----------------------------------------------------------------------------
BOOL PN_PLATE_IsEnd( const PN_PLATE *plate )
{
  return !plate->is_update;
}
//===========================================================================
// ■private method
//===========================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  上部プレートの初期化
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk_adrs          上部プレート用ワーク
 *	@param	void *param_adrs  初期化構造体
 */
//-----------------------------------------------------------------------------
static void Pn_Plate_Top_Init( PN_PLATE *plate, void *wk_adrs, const void *param_adrs )
{
  PN_PLATE_TOP  *wk = wk_adrs;
  const PN_PLATE_TOP_INIT_PARAM  *param  = param_adrs;

  //背面の設定
  GFL_CLACT_WK_SetAnmSeq( plate->backobj, 0 );

  //BMPの作成
  {
    wk->bmp = GFL_BMP_Create( 
      PN_PLATE_BMP_XSIZE_CHR, PN_PLATE_BMP_YSIZE_CHR, 
      GFL_BMP_16_COLOR, plate->heapID );
  }

  //BMPOAMの設定
  {
    BMPOAM_ACT_DATA actorData;

    GF_ASSERT( wk->bmp );
    GF_ASSERT( wk->bmpOamActor == NULL );

    actorData.bmp        = wk->bmp;
    actorData.x          = 0;
    actorData.y          = 0;
    actorData.pltt_index = param->fontPlttRegIdx;
    actorData.pal_offset = 0;
    actorData.soft_pri   = 0;
    actorData.bg_pri     = 0;
    actorData.setSerface = plate->surface;
    actorData.draw_type  = CLSYS_DRAW_MAIN; 

    // アクターを追加
    wk->bmpOamActor = BmpOam_ActorAdd( param->bmpOamSys, &actorData ); 
    BmpOam_ActorSetDrawEnable( wk->bmpOamActor, FALSE );  
  }

  //地名アイコンリソース読込み
  {
	  wk->placePltIdx = 
		GFL_CLGRP_PLTT_RegisterEx( 
				plate->arcHandle, NARC_place_name_icon_r01_NCLR,
				CLSYS_DRAW_MAIN, FLDOAM_PALNO_PLACENAME_PLACE_ICON * 32, 0, 1, plate->heapID );

	  wk->placeCgrIdx = GFL_CLGRP_CGR_Register( 
      plate->arcHandle, NARC_place_name_icon_r01_NCGR, 
      FALSE, CLSYS_DRAW_MAIN, plate->heapID );

	  wk->placeCelIdx = GFL_CLGRP_CELLANIM_Register( 
      plate->arcHandle, NARC_place_name_icon_r01_NCER, 
      NARC_place_name_icon_r01_NANR, plate->heapID );
  }

  //地名アイコン作成
  {
    GFL_CLWK_DATA data  = {0};
    wk->placeIcon = GFL_CLACT_WK_Create( plate->unit, 
          wk->placeCgrIdx, wk->placePltIdx, wk->placeCelIdx,
          &data, 0, plate->heapID );
    GFL_CLACT_WK_SetDrawEnable( wk->placeIcon, FALSE );
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  上部プレートの破棄
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk_adrs          上部プレート用ワーク
 */
//-----------------------------------------------------------------------------
static void Pn_Plate_Top_Exit( PN_PLATE *plate, void *wk_adrs )
{
  int i;
  PN_PLATE_TOP  *wk = wk_adrs;

  //地名アイコン破棄
  if( wk->placeIcon )
  {
    GFL_CLACT_WK_Remove( wk->placeIcon );
    wk->placeIcon = NULL;
  }

  //地名アイコンリソース破棄
  {
		GFL_CLGRP_PLTT_Release( wk->placePltIdx );
    GFL_CLGRP_CGR_Release( wk->placeCgrIdx );
    GFL_CLGRP_CELLANIM_Release( wk->placeCelIdx );
  }

  //BMPOAMの破棄
  if( wk->bmpOamActor )
  {
    BmpOam_ActorDel( wk->bmpOamActor );
    wk->bmpOamActor = NULL;
  }

  //BMPの破棄
  if( wk->bmp ) {
    GFL_BMP_Delete( wk->bmp );
    wk->bmp = NULL;
  } 

}
//----------------------------------------------------------------------------
/**
 *	@brief  上部プレートの設定
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk_adrs          上部プレート用ワーク
 *	@param	void *setup_param 設定構造体
 */
//-----------------------------------------------------------------------------
static void Pn_Plate_Top_Setup( PN_PLATE *plate, void *wk_adrs, const void *setup_param )
{
  PN_PLATE_TOP  *wk = wk_adrs;
  const PN_PLATE_TOP_SETUP_PARAM  *param  = setup_param;

  wk->setup_param = *param;
  Pn_Plate_Request( plate, PN_PLATE_REQ_SETUP );
}

//----------------------------------------------------------------------------
/**
 *	@brief  上部プレートのメイン処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk_adrs          上部プレート用ワーク
 *
 *	@retval TRUE  処理続行
 *  @retval FALSE 処理中断
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Top_Setup( PN_PLATE *plate, PN_PLATE_TOP *wk, u32 *seq );
static BOOL UpdateFunction_Top_Enter( PN_PLATE *plate, PN_PLATE_TOP *wk, u32 *seq );
static BOOL UpdateFunction_Top_Out( PN_PLATE *plate, PN_PLATE_TOP *wk, u32 *seq );
static BOOL UpdateFunction_Top_Hide( PN_PLATE *plate, PN_PLATE_TOP *wk, u32 *seq );
static BOOL Pn_Plate_Top_Update( PN_PLATE *plate, void *wk_adrs )
{
  PN_PLATE_TOP  *wk = wk_adrs;
  BOOL (*UpdateFunction[])( PN_PLATE *plate, PN_PLATE_TOP *wk, u32 *seq ) =
  {
    NULL,   //PN_PLATE_REQ_NONE,  //リクエストなし
    UpdateFunction_Top_Enter,     //PN_PLATE_REQ_ENTER,   //出現
    UpdateFunction_Top_Out,       //PN_PLATE_REQ_OUT,     //出て行く
    UpdateFunction_Top_Hide,      //PN_PLATE_REQ_HIDE      //隠れる
    UpdateFunction_Top_Setup,     //PN_PLATE_REQ_SETUP,   //（内部で使用）
  };

  GF_ASSERT( plate->req < PN_PLATE_REQ_MAX );
  GF_ASSERT( UpdateFunction[ plate->req ] != NULL );
  return UpdateFunction[ plate->req ]( plate, wk, &plate->seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief  上部プレートの設定メイン処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               上部プレート用ワーク
 *	@param	*seq              シーケンス
 *
 *	@retval TRUE  処理続行
 *	@retval FALSE 処理中断
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Top_Setup( PN_PLATE *plate, PN_PLATE_TOP *wk, u32 *seq )
{
  enum
  {
    SEQ_LOAD_PLACEICON,
    SEQ_PRINT_BMP,
    SEQ_LOAD_PLT,
    SEQ_LOAD_CHR,
    SEQ_EXIT,
  };

  //処理落ちしないように分割セットアップ
  switch( *seq )
  {
  case SEQ_LOAD_PLACEICON:
    wk->placeIconIndex  = ZONEDATA_GetPlaceIconIdx(wk->setup_param.zoneID);
    (*seq)++;
    break;
  case SEQ_PRINT_BMP:
    //文字をBMPに書き込む
    {
      PRINTSYS_LSB  color;

      //一旦消去
      GFL_BMP_Clear( wk->bmp, 0 );

      color = PRINTSYS_LSB_Make( COLOR_NO_LETTER, COLOR_NO_SHADOW, COLOR_NO_BACK );
#ifdef STRING_LENGTH_CHECK
      if( PlaceNameLong )
      {
        STRBUF  *strbuf = GFL_STR_CreateBuffer( 128, plate->heapID );
        STRCODE code[]  = L"あああああのホワイトフォレスト";
        code[15]  = GFL_STR_GetEOMCode();
        GFL_STR_SetStringCodeOrderLength( strbuf, code, 128 );
        PRINTSYS_PrintColor( wk->bmp, 0, 0, strbuf, wk->setup_param.font, color );
        GFL_STR_DeleteBuffer( strbuf );
      }
      else
#endif
      {
        PRINTSYS_PrintColor( wk->bmp, 0, 0, wk->setup_param.placeName, wk->setup_param.font, color );
      }
      BmpOam_ActorBmpTrans( wk->bmpOamActor );
    }
    (*seq)++;
    break;
  case SEQ_LOAD_PLT:
    //地名アイコンパレット上書き
    {
      NNSG2dPaletteData* plttData;
      void * plttBuff;
      ARCDATID  datID = wk->placeIconIndex + ZONEDATA_PLACEICON_OFS_NCLR;

      plttBuff  =  GFL_ARCHDL_UTIL_LoadPalette( plate->arcHandle, datID, &plttData, plate->heapID );

      GFL_CLGRP_PLTT_Replace( wk->placePltIdx, plttData, 1 );

      GFL_HEAP_FreeMemory( plttBuff );
    }
    (*seq)++;
    break;
  case SEQ_LOAD_CHR:
    //地名アイコンキャラ上書き
    {
      NNSG2dCharacterData*  charData;
      void * charBuff;
      ARCDATID  datID = wk->placeIconIndex + ZONEDATA_PLACEICON_OFS_NCGR;

      charBuff  = GFL_ARCHDL_UTIL_LoadOBJCharacter( plate->arcHandle, datID, FALSE, &charData, plate->heapID );


      GFL_CLGRP_CGR_Replace( wk->placeCgrIdx, charData );

      GFL_HEAP_FreeMemory( charBuff );
    }
    (*seq)++;
    break;
  case SEQ_EXIT:
    return FALSE;
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  上部プレートの出現メイン処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               上部プレート用ワーク
 *	@param	*seq              シーケンス
 *
 *	@retval TRUE  処理続行
 *	@retval FALSE 処理中断
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Top_Enter( PN_PLATE *plate, PN_PLATE_TOP *wk, u32 *seq )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  switch( *seq )
  {
  case SEQ_INIT:
    //移動計算初期化
    {
      PN_MOVE_INIT_PARAM  param = sc_pn_plate_top_enter_param;
      param.startFrame  = plate->startFrame;
      param.endFrame    = plate->endFrame;
      PN_MOVE_Init( &wk->move, &param );
    }

    BmpOam_ActorSetDrawEnable( wk->bmpOamActor, TRUE );
    GFL_CLACT_WK_SetDrawEnable( plate->backobj, TRUE );
    GFL_CLACT_WK_SetDrawEnable( wk->placeIcon, TRUE );
    (*seq)++;
    break;

  case SEQ_MAIN:
    //座標を計算
    PN_MOVE_Main( &wk->move );
    if( PN_MOVE_IsEnd( &wk->move ) )
    {
      (*seq)++;
    }
    break;

  case SEQ_EXIT:
      return FALSE;
  }

  //座標更新
  {
    GFL_CLACTPOS  plate_pos = sc_pn_plate_top_bar_pos;
    GFL_CLACTPOS  name_pos = sc_pn_plate_name_pos;
    GFL_CLACTPOS  icon_pos  = sc_pn_plate_icon_pos;
    GFL_CLACTPOS  now;
    PN_MOVE_GetPos( &wk->move, &now );

    plate_pos.x += now.x;
    plate_pos.y += now.y;
    GFL_CLACT_WK_SetPos( plate->backobj, &plate_pos, plate->surface );

    icon_pos.x += now.x;
    icon_pos.y += now.y;
    GFL_CLACT_WK_SetPos( wk->placeIcon, &icon_pos, plate->surface );

    name_pos.x  += now.x;
    name_pos.y  += now.y;
    BmpOam_ActorSetPos( wk->bmpOamActor, name_pos.x, name_pos.y );
  }
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  上部プレートの出て行くメイン処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               上部プレート用ワーク
 *	@param	*seq              シーケンス
 *
 *	@retval TRUE  処理続行
 *	@retval FALSE 処理中断
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Top_Out( PN_PLATE *plate, PN_PLATE_TOP *wk, u32 *seq )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  switch( *seq )
  {
  case SEQ_INIT:
    //移動計算初期化
    {
      PN_MOVE_INIT_PARAM  param = sc_pn_plate_top_out_param;
      param.startFrame  = plate->startFrame;
      param.endFrame    = plate->endFrame;
      PN_MOVE_Init( &wk->move, &param );
    }
    (*seq)++;
    break;
  case SEQ_MAIN:
    //座標を計算
    PN_MOVE_Main( &wk->move );
    if( PN_MOVE_IsEnd( &wk->move ) )
    {
      GFL_CLACT_WK_SetDrawEnable( plate->backobj, FALSE );
      GFL_CLACT_WK_SetDrawEnable( wk->placeIcon, FALSE );
      BmpOam_ActorSetDrawEnable( wk->bmpOamActor, FALSE );
      (*seq)++;
    }
    break;
  case SEQ_EXIT:
    return FALSE;
  }

  //座標更新
  {
    GFL_CLACTPOS  plate_pos = sc_pn_plate_top_bar_pos;
    GFL_CLACTPOS  name_pos = sc_pn_plate_name_pos;
    GFL_CLACTPOS  icon_pos  = sc_pn_plate_icon_pos;
    GFL_CLACTPOS  now;
    PN_MOVE_GetPos( &wk->move, &now );

    plate_pos.x += now.x;
    plate_pos.y += now.y;
    GFL_CLACT_WK_SetPos( plate->backobj, &plate_pos, plate->surface );

    icon_pos.x += now.x;
    icon_pos.y += now.y;
    GFL_CLACT_WK_SetPos( wk->placeIcon, &icon_pos, plate->surface );

    name_pos.x  += now.x;
    name_pos.y  += now.y;
    BmpOam_ActorSetPos( wk->bmpOamActor, name_pos.x, name_pos.y );
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  上部プレートの隠れる処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               上部プレート用ワーク
 *	@param	*seq              シーケンス
 *
 *	@retval TRUE  処理続行
 *	@retval FALSE 処理中断
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Top_Hide( PN_PLATE *plate, PN_PLATE_TOP *wk, u32 *seq )
{
  GFL_CLACT_WK_SetDrawEnable( plate->backobj, FALSE );
  GFL_CLACT_WK_SetDrawEnable( wk->placeIcon, FALSE );
  BmpOam_ActorSetDrawEnable( wk->bmpOamActor, FALSE );
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  下部プレートの初期化処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               下部プレート用ワーク
 *	@param	void *param_adrs  初期化構造体
 */
//-----------------------------------------------------------------------------
static void Pn_Plate_Bottom_Init( PN_PLATE *plate, void *wk_adrs, const void *param_adrs )
{
  PN_PLATE_BOTTOM  *wk = wk_adrs;
  const PN_PLATE_BOTTOM_INIT_PARAM  *param  = param_adrs;

  //背面の設定
  GFL_CLACT_WK_SetAnmSeq( plate->backobj, 1 );

  //季節アイコンリソース読込み
  {
	  wk->seasonPltIdx = 
		GFL_CLGRP_PLTT_RegisterEx( 
				plate->arcHandle, NARC_place_name_place_name_season_NCLR,
				CLSYS_DRAW_MAIN, FLDOAM_PALNO_PLACENAME_SEAZON_ICON * 32, 0, 1, plate->heapID );

	  wk->seasonCgrIdx = GFL_CLGRP_CGR_Register( 
      plate->arcHandle, NARC_place_name_place_name_season_NCGR, 
      FALSE, CLSYS_DRAW_MAIN, plate->heapID );

	  wk->seasonCelIdx = GFL_CLGRP_CELLANIM_Register( 
      plate->arcHandle, NARC_place_name_place_name_season_NCER, 
      NARC_place_name_place_name_season_NANR, plate->heapID );
  }

  //季節アイコン作成
  {
    GFL_CLWK_DATA data  = {0};
    wk->seasonIcon = GFL_CLACT_WK_Create( plate->unit, 
          wk->seasonCgrIdx, wk->seasonPltIdx, wk->seasonCelIdx,
          &data, 0, plate->heapID );
    GFL_CLACT_WK_SetDrawEnable( wk->seasonIcon, FALSE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  下部プレートの破棄処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               下部プレート用ワーク
 */
//-----------------------------------------------------------------------------
static void Pn_Plate_Bottom_Exit( PN_PLATE *plate, void *wk_adrs )
{
  PN_PLATE_BOTTOM  *wk = wk_adrs;

  //季節アイコン破棄
  if( wk->seasonIcon )
  {
    GFL_CLACT_WK_Remove( wk->seasonIcon );
    wk->seasonIcon = NULL;
  }

  //季節アイコンリソース破棄
  {
		GFL_CLGRP_PLTT_Release( wk->seasonPltIdx );
    GFL_CLGRP_CGR_Release( wk->seasonCgrIdx );
    GFL_CLGRP_CELLANIM_Release( wk->seasonCelIdx );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  下部プレートの初期化処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               下部プレート用ワーク
 *	@param	void *setup_param   設定構造体
 */
//-----------------------------------------------------------------------------
static void Pn_Plate_Bottom_Setup( PN_PLATE *plate, void *wk_adrs, const void *setup_param )
{
  PN_PLATE_BOTTOM  *wk = wk_adrs;
  const PN_PLATE_BOTTOM_SETUP_PARAM *param  = setup_param;

  wk->setup_param = *param;
  Pn_Plate_Request( plate, PN_PLATE_REQ_SETUP );
}
//----------------------------------------------------------------------------
/**
 *	@brief  下部プレートのメイン処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               下部プレート用ワーク
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Bottom_Setup( PN_PLATE *plate, PN_PLATE_BOTTOM *wk, u32 *seq );
static BOOL UpdateFunction_Bottom_Enter( PN_PLATE *plate, PN_PLATE_BOTTOM *wk, u32 *seq );
static BOOL UpdateFunction_Bottom_Out( PN_PLATE *plate, PN_PLATE_BOTTOM *wk, u32 *seq );
static BOOL UpdateFunction_Bottom_Hide( PN_PLATE *plate, PN_PLATE_BOTTOM *wk, u32 *seq );
static BOOL Pn_Plate_Bottom_Update( PN_PLATE *plate, void *wk_adrs )
{
  PN_PLATE_BOTTOM  *wk = wk_adrs;
  BOOL (*UpdateFunction[])( PN_PLATE *plate, PN_PLATE_BOTTOM *wk, u32 *seq ) =
  {
    NULL,   //PN_PLATE_REQ_NONE,  //リクエストなし
    UpdateFunction_Bottom_Enter,    //PN_PLATE_REQ_ENTER,   //出現
    UpdateFunction_Bottom_Out,      //PN_PLATE_REQ_OUT,     //出て行く
    UpdateFunction_Bottom_Hide,     //PN_PLATE_REQ_HIDE     //隠れる
    UpdateFunction_Bottom_Setup,    //PN_PLATE_REQ_SETUP,   //設定
  };

  GF_ASSERT( plate->req < PN_PLATE_REQ_MAX );
  GF_ASSERT( UpdateFunction[ plate->req ] != NULL );
  return UpdateFunction[ plate->req ]( plate, wk, &plate->seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief  下部プレートの設定メイン処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               下部プレート用ワーク
 *	@param	*seq              シーケンス
 *
 *	@retval TRUE  処理続行
 *	@retval FALSE 処理中断
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Bottom_Setup( PN_PLATE *plate, PN_PLATE_BOTTOM *wk, u32 *seq )
{
  //季節によって変更
  GFL_CLACT_WK_SetAnmSeq( wk->seasonIcon, wk->setup_param.seasonID );

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  下部プレートの出現メイン処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               下部プレート用ワーク
 *	@param	*seq              シーケンス
 *
 *	@retval TRUE  処理続行
 *	@retval FALSE 処理中断
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Bottom_Enter( PN_PLATE *plate, PN_PLATE_BOTTOM *wk, u32 *seq )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  switch( *seq )
  {
  case SEQ_INIT:
    //移動計算初期化
    {
      PN_MOVE_INIT_PARAM  param = sc_pn_plate_bottom_enter_param;
      param.startFrame  = plate->startFrame;
      param.endFrame    = plate->endFrame;
      PN_MOVE_Init( &wk->move, &param );
    }
    GFL_CLACT_WK_SetDrawEnable( plate->backobj, TRUE );
    GFL_CLACT_WK_SetDrawEnable( wk->seasonIcon, TRUE );
    (*seq)++;
    break;
  case SEQ_MAIN:
    //座標を計算
    PN_MOVE_Main( &wk->move );
    if( PN_MOVE_IsEnd( &wk->move ) )
    {
      (*seq)++;
    }
    break;
  case SEQ_EXIT:
    return FALSE;
  }

  {
    //座標を反映
    GFL_CLACTPOS  plate_pos = sc_pn_plate_bottom_bar_pos;
    GFL_CLACTPOS  season_pos  = sc_pn_plate_season_pos;
    GFL_CLACTPOS  now;
    PN_MOVE_GetPos( &wk->move, &now );

    plate_pos.x += now.x;
    plate_pos.y += now.y;
    GFL_CLACT_WK_SetPos( plate->backobj, &plate_pos, plate->surface );

    season_pos.x += now.x;
    season_pos.y += now.y;
    GFL_CLACT_WK_SetPos( wk->seasonIcon, &season_pos, plate->surface );
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  下部プレートの出て行くメイン処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               下部プレート用ワーク
 *	@param	*seq              シーケンス
 *
 *	@retval TRUE  処理続行
 *	@retval FALSE 処理中断
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Bottom_Out( PN_PLATE *plate, PN_PLATE_BOTTOM *wk, u32 *seq )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  switch( *seq )
  {
  case SEQ_INIT:
    //移動計算初期化
    {
      PN_MOVE_INIT_PARAM  param = sc_pn_plate_bottom_out_param;
      param.startFrame  = plate->startFrame;
      param.endFrame    = plate->endFrame;
      PN_MOVE_Init( &wk->move, &param );
    }
    (*seq)++;
    break;
  case SEQ_MAIN:
    //座標を計算
    PN_MOVE_Main( &wk->move );
    if( PN_MOVE_IsEnd( &wk->move ) )
    {
      GFL_CLACT_WK_SetDrawEnable( plate->backobj, FALSE );
      GFL_CLACT_WK_SetDrawEnable( wk->seasonIcon, FALSE );
      (*seq)++;
    }
    break;
  case SEQ_EXIT:
    return FALSE;
  }

  {
    //座標を反映
    GFL_CLACTPOS  plate_pos = sc_pn_plate_bottom_bar_pos;
    GFL_CLACTPOS  season_pos  = sc_pn_plate_season_pos;
    GFL_CLACTPOS  now;
    PN_MOVE_GetPos( &wk->move, &now );

    plate_pos.x += now.x;
    plate_pos.y += now.y;
    GFL_CLACT_WK_SetPos( plate->backobj, &plate_pos, plate->surface );

    season_pos.x += now.x;
    season_pos.y += now.y;
    GFL_CLACT_WK_SetPos( wk->seasonIcon, &season_pos, plate->surface );
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  下部プレートの消去処理
 *
 *	@param	PN_PLATE *plate   プレート共通ワーク
 *	@param	*wk               下部プレート用ワーク
 *	@param	*seq              シーケンス
 *
 *	@retval TRUE  処理続行
 *	@retval FALSE 処理中断
 */
//-----------------------------------------------------------------------------
static BOOL UpdateFunction_Bottom_Hide( PN_PLATE *plate, PN_PLATE_BOTTOM *wk, u32 *seq )
{
  GFL_CLACT_WK_SetDrawEnable( plate->backobj, FALSE );
  GFL_CLACT_WK_SetDrawEnable( wk->seasonIcon, FALSE );
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  移動計算初期化
 *
 *	@param	PN_MOVE *wk ワーク
 *	@param	PN_MOVE_INIT_PARAM *param 初期化パラメータ
 */
//-----------------------------------------------------------------------------
static void PN_MOVE_Init( PN_MOVE *wk, const PN_MOVE_INIT_PARAM *param )
{
  GFL_STD_MemClear( wk, sizeof(PN_MOVE) );
#ifdef PN_MOVE_LINER
  wk->now_frame = param->startFrame;
  wk->param = *param;

  //wk->x,wk->yの初期化のため
  PN_MOVE_Main( wk );
#else
  wk->x = FX32_CONST(param->init.x);
  wk->y = FX32_CONST(param->init.y);
  wk->sx  = FX32_CONST(param->init_speed.x);
  wk->sy  = FX32_CONST(param->init_speed.y);

  {
    //距離
    GFL_POINT d;
    d.x  = param->end.x  - param->init.x;
    d.y  = param->end.y  - param->init.y;

    //加速度計算
    if( d.x != 0 )
    {
      // v^2 -v0^2 = 2as より 
      wk->ax  = FX_Div( 
          ( FX32_CONST( param->end_speed.x * param->end_speed.x ) -
            FX32_CONST( param->init_speed.x * param->init_speed.x ) ),
          (2 * FX32_CONST(d.x) )
          );
    }
    if( d.y != 0 )
    {
      wk->ay  = FX_Div(
          ( FX32_CONST( param->end_speed.y * param->end_speed.y ) -
            FX32_CONST( param->init_speed.y * param->init_speed.y ) ),
          (2 * FX32_CONST(d.y) )
          );
    }

    //移動回数計算
    if( ( wk->ax >> FX32_SHIFT) != 0 )
    {
      wk->tx  = (param->end_speed.x - param->init_speed.x) / (wk->ax >> FX32_SHIFT);
    }
    if( (wk->ay >> FX32_SHIFT) != 0 )
    {
      wk->ty  = (param->end_speed.y - param->init_speed.y) / (wk->ay >> FX32_SHIFT);
    }
  }
#endif
}
//----------------------------------------------------------------------------
/**
 *	@brief  移動計算メイン
 *
 *	@param	PN_MOVE *wk ワーク
 */
//-----------------------------------------------------------------------------
static void PN_MOVE_Main( PN_MOVE *wk )
{
#ifdef PN_MOVE_LINER
  if( wk->now_frame++ < wk->param.endFrame )
  {
    wk->x = wk->param.init.x + (wk->param.end.x - wk->param.init.x) * wk->now_frame / wk->param.endFrame;
    wk->y = wk->param.init.y + (wk->param.end.y - wk->param.init.y) * wk->now_frame / wk->param.endFrame;
  }
  else
  {
    wk->x = wk->param.end.x;
    wk->y = wk->param.end.y;
  }
  NAGI_Printf( "x%d y%d\n", wk->x, wk->y );
#else
  if( wk->tx > 0 )
  {
    wk->x   += wk->sx;
    wk->sx  += wk->ax;
    wk->tx--;
  }
  if( wk->ty > 0 )
  {
    wk->y   += wk->sy;
    wk->sy  += wk->ay;
    wk->ty--;
  }
#endif
}
//----------------------------------------------------------------------------
/**
 *	@brief  移動計算終了チェック
 *
 *	@param	const PN_MOVE *wk ワーク
 *	@retval TRUE  終了
 *	@retval TRUE  処理中
 */
//-----------------------------------------------------------------------------
static BOOL PN_MOVE_IsEnd( const PN_MOVE *wk )
{
#ifdef PN_MOVE_LINER
  return wk->now_frame > wk->param.endFrame;
#else
  return wk->tx <= 0 && wk->ty <= 0;
#endif
}
//----------------------------------------------------------------------------
/**
 *	@brief  座標受け取り
 *
 *	@param	const PN_MOVE *wk ワーク
 *	@param	*pos              受け取り
 */
//-----------------------------------------------------------------------------
static void PN_MOVE_GetPos( const PN_MOVE *wk, GFL_CLACTPOS *pos )
{
#ifdef PN_MOVE_LINER
  pos->x  = wk->x;
  pos->y  = wk->y;
#else
  pos->x  = (wk->x >> FX32_SHIFT);
  pos->y  = (wk->y >> FX32_SHIFT);
#endif
}
