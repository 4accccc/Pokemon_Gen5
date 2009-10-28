#pragma once
typedef struct {
  int TotalPolygonNum;
  int TotalVertexNum;
}GFL_D3D_INFO;

extern void GFL_G3D_ClearG3dInfo(void);
extern const GFL_D3D_INFO *GFL_G3D_GetG3dInfoPtr(void);

