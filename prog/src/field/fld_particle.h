//======================================================================
/**
 * @file	fld_particle.h
 * @brief	フィールドパーティクル
 * @author	Saito
 *
 */
//======================================================================
#ifndef __FLD_PARTICLE_H__
#define __FLD_PARTICLE_H__

typedef struct FLD_PRTCL_SYS_tag * FLD_PRTCL_SYS_PTR;

extern FLD_PRTCL_SYS_PTR FLD_PRTCL_Init(const HEAPID inHeapID);
extern GFL_PTC_PTR FLD_PRTCL_Create(FLD_PRTCL_SYS_PTR sys);
extern void FLD_PRTCL_Delete(FLD_PRTCL_SYS_PTR sys);
extern void FLD_PRTCL_End(FLD_PRTCL_SYS_PTR sys);
extern void FLD_PRTCL_Main(void);
extern void *FLD_PRTCL_LoadResource(FLD_PRTCL_SYS_PTR sys, int inArcID, int inArcIdx);
extern void FLD_PRTCL_SetResource(FLD_PRTCL_SYS_PTR sys, void *resource, int inTexAtOnce, GFL_TCBSYS *tcbsys);
extern BOOL FLD_PRTCL_CheckEmitEnd(FLD_PRTCL_SYS_PTR sys);

#endif    //__FLD_PARTICLE_H__
