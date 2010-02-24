
//============================================================================================
/**
 * @file  btlv_clact.h
 * @brief êÌì¨âÊñ CLACTêßå‰
 * @author  soga
 * @date  2009.05.14
 */
//============================================================================================

#pragma once

#define BTLV_CLACT_CLWK_MAX ( 7 )    //UNITÇÃMAX

typedef struct _BTLV_CLACT_WORK BTLV_CLACT_WORK;
typedef struct _BTLV_CLACT_CLWK BTLV_CLACT_CLWK;

extern  BTLV_CLACT_WORK*  BTLV_CLACT_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID );
extern  void              BTLV_CLACT_Exit( BTLV_CLACT_WORK *bclw );
extern  void              BTLV_CLACT_Main( BTLV_CLACT_WORK *bclw );

extern  int               BTLV_CLACT_Add( BTLV_CLACT_WORK *bclw, ARCID arcID, ARCDATID ncgrID, s16 posx, s16 posy );
extern  int               BTLV_CLACT_AddAffine( BTLV_CLACT_WORK *bclw, ARCID arcID, ARCDATID ncgrID, s16 posx, s16 posy, fx32 scalex, fx32 scaley );
extern  int               BTLV_CLACT_AddEx( BTLV_CLACT_WORK *bclw, ARCID arcID, ARCDATID ncgrID, ARCDATID nclrID,
                                            ARCDATID ncerID, ARCDATID nanrID, s16 posx, s16 posy, fx32 scalex, fx32 scaley );
extern  void              BTLV_CLACT_Delete( BTLV_CLACT_WORK *bclw, int index );

extern  void              BTLV_CLACT_MovePosition( BTLV_CLACT_WORK *bclw, int index, int type,
                                                   GFL_CLACTPOS *pos, int frame, int wait, int count );
extern  void              BTLV_CLACT_MoveScale( BTLV_CLACT_WORK *bclw, int index, int type,
                                                VecFx32 *scale, int frame, int wait, int count );
extern  void              BTLV_CLACT_SetAnime( BTLV_CLACT_WORK *bclw, int index, int anm_no );
extern  void              BTLV_CLACT_SetPaletteFade( BTLV_CLACT_WORK *bclw, int index,
                                                     u8 start_evy, u8 end_evy, s8 wait, u16 rgb );
extern  BOOL              BTLV_CLACT_CheckTCBExecute( BTLV_CLACT_WORK *bclw, int index );


