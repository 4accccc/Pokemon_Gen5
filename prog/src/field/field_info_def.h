/*
 *  @file   field_info_def.h
 *  @brief  地名表示およびフェスミッション情報表示システム共有デファイン
 *  @author Miyuki Iwasawa
 *  @date   11.06.10
 */

#pragma once

///地名表示のBGプライオリティ
#define FLD_PLACE_NAME_BG_PRIORITY  (1)

///フェスミッション情報表示のBGプライオリティ
#define FLD_FMISSION_INFO_BG_PRIORITY (FLD_PLACE_NAME_BG_PRIORITY)  //地名表示と同じ

///地名とフェスミッション情報表示のOBJソフトウェアプライオリティ
enum{
  FLD_FMISSION_INFO_OAM_SPRI0,  ///<フェス情報優先度1
  FLD_FMISSION_INFO_OAM_SPRI1,  ///<フェス情報優先度2
  FLD_FMISSION_INFO_OAM_SPRI2,  ///<フェス情報優先度3
  FLD_PLACE_NAME_OAM_SPRI,      ///<地名
};

