//============================================================================================
/**
 * @file	font2tex.h
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#pragma once

#include "gflib.h"

typedef struct F2T_WORK_tag
{
  u16           texSizIdxS;
  u16           texSizIdxT;

  NNSGfdTexKey  texVramKey;   //ÉeÉNÉXÉ`ÉÉÇuÇqÇ`ÇlÉLÅ[
  NNSGfdPlttKey plttVramKey;  //ÉpÉåÉbÉgÇuÇqÇ`ÇlÉLÅ[
  u32           texOffset;
  u32           plttOffset;
}F2T_WORK;

extern BOOL F2T_CopyString(
    const GFL_G3D_RES* g3Dtex, const char* tex_name,
    void *plt_data, const char* plt_name,
    const STRBUF* str, u16 xpos, u16 ypos, PRINTSYS_LSB lsb, HEAPID heapID, F2T_WORK *outWork );
extern BOOL F2T_CopyStringAlloc(
    void *plt_data, const STRBUF* str, 
    u16 xpos, u16 ypos, PRINTSYS_LSB lsb, HEAPID heapID, F2T_WORK *outWork );
extern u16 F2T_GetTexSizTblSize(void);
extern u16 F2T_GetTexSize(const int inIdx);
extern GXTexSizeS F2T_GetTexSizeS(const int inIdx);
extern GXTexSizeT F2T_GetTexSizeT(const int inIdx);


