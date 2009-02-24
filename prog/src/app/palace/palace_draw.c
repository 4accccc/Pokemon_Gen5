//==============================================================================
/**
 * @file	palace_draw.c
 * @brief	パレス：描画関連
 * @author	matsuda
 * @date	2009.02.20(金)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include "system\main.h"
#include "arc_def.h"
#include "palace.naix"
#include "palace_types.h"
#include "palace_draw.h"


//==============================================================================
//	定数定義
//==============================================================================
///ブロック1面のテクスチャサイズ
#define BLOCK_TEX_SIZE		(0x80)
///ブロック1面のパレットサイズ
#define BLOCK_PLTT_SIZE		(0x20)


void PalaceDraw_LoadTex(u32 destAddress, u32 destPlttAddress)
{
//	ARCHANDLE *hdl;
	int i;
	void *tex, *pltt;
	
//	hdl = GFL_ARC_OpenDataHandle(ARCID_PALACE, HEAPID_PALACE);
	tex = GFL_ARC_LoadDataAlloc(ARCID_PALACE, NARC_palace_block_tex16_ntft, HEAPID_PALACE);
	pltt = GFL_ARC_LoadDataAlloc(ARCID_PALACE, NARC_palace_block_tex16_ntfp, HEAPID_PALACE);
	
	//テクスチャ
	GX_BeginLoadTex();
	{
		for(i = 0; i < PALACE_POLYGON_MAX; i++){
	//		GX_LoadTex((void *)&tex_16plett64x64[0], 
	//			destAddress + BLOCK_TEX_SIZE*i, BLOCK_TEX_SIZE);
	//		GX_LoadTex((void *)&block_tex16_Texel[0], 
	//			destAddress + BLOCK_TEX_SIZE*i, BLOCK_TEX_SIZE);
			GX_LoadTex(tex, destAddress + BLOCK_TEX_SIZE*i, BLOCK_TEX_SIZE);
		}
	}
	GX_EndLoadTex();

	//パレット
	GX_BeginLoadTexPltt();
	{
	//	GX_LoadTexPltt((void *)&pal_16plett[0], destPlttAddress, BLOCK_PLTT_SIZE);
	//	GX_LoadTexPltt((void *)&block_tex16_Palette[0], destPlttAddress, BLOCK_PLTT_SIZE);
		GX_LoadTexPltt(pltt, destPlttAddress, BLOCK_PLTT_SIZE);
	}
	GX_EndLoadTexPltt();

	GFL_HEAP_FreeMemory(tex);
	GFL_HEAP_FreeMemory(pltt);
//	GFL_ARC_CloseDataHandle(hdl);
}


static const s16 gCubeGeometry[3 * 8] = {
	HALF_GRID, HALF_GRID, HALF_GRID,
	HALF_GRID, HALF_GRID, -HALF_GRID,
	HALF_GRID, -HALF_GRID, HALF_GRID,
	HALF_GRID, -HALF_GRID, -HALF_GRID,
	-HALF_GRID, HALF_GRID, HALF_GRID,
	-HALF_GRID, HALF_GRID, -HALF_GRID,
	-HALF_GRID, -HALF_GRID, HALF_GRID,
	-HALF_GRID, -HALF_GRID, -HALF_GRID
};

#define FX32_TO_FX10(x)		(((x) >> (FX32_DEC_SIZE - GX_FX10_DEC_SIZE)) & GX_FX10_MASK)
static const fx16 gCubeNormal[3*6] = {
	0, 0, FX16_ONE - 1,
	0, FX16_ONE - 1, 0,
	FX16_ONE - 1, 0, 0,
	0, 0, -FX16_ONE + 1,
	0, -FX16_ONE + 1, 0,
	-FX16_ONE + 1, 0, 0,
};

static const fx32 gCubeTexCoord[2 * 4] = {
	0, 0,
	0, 16 * FX32_ONE,
	16 * FX32_ONE, 0,
	16 * FX32_ONE, 16 * FX32_ONE,
};

static const GXRgb gCubeColor[8] = {
	GX_RGB(31, 31, 31),
	GX_RGB(31, 31, 0),
	GX_RGB(31, 0, 31),
	GX_RGB(31, 0, 0),
	GX_RGB(0, 31, 31),
	GX_RGB(0, 31, 0),
	GX_RGB(0, 0, 31),
	GX_RGB(0, 0, 0)
};

static inline void Color(GXRgb rgb)
{
#if 1
	G3_Color(rgb);
#else
	if(idx & 1){
		G3_Color(GX_RGB(16, 16, 16));
	}
	else{
		G3_Color(GX_RGB(24, 8, 16));
	}
#endif
}

static inline void Vtx(int idx, const VecFx16 *add_pos)
{
	G3_Vtx(gCubeGeometry[idx * 3] + add_pos->x, 
		gCubeGeometry[idx * 3 + 1] + add_pos->y, gCubeGeometry[idx * 3 + 2] + add_pos->z);
}

static inline void normal(int idx)
{
	G3_Normal(gCubeNormal[idx * 3], gCubeNormal[idx * 3 + 1], gCubeNormal[idx * 3 + 2]);
}

inline void tex_coord(int idx)
{
	G3_TexCoord(gCubeTexCoord[idx * 2], gCubeTexCoord[idx * 2 + 1]);
}

static inline void ColVtxQuad(int idx0, int idx1, int idx2, int idx3, int n_idx, const VecFx16 *add_pos, GXRgb rgb)
{
	tex_coord(3);
	normal(n_idx);
	Vtx(idx0, add_pos);

	tex_coord(2);
	normal(n_idx);
	Vtx(idx1, add_pos);

	tex_coord(0);
	normal(n_idx);
	Vtx(idx2, add_pos);

	tex_coord(1);
	normal(n_idx);
	Vtx(idx3, add_pos);
}

void PalaceDraw_Cube(const VecFx16 *add_pos, GXRgb rgb)
{
	int poly_no = 0, i;
	VecFx16 offset;
	
	G3_LightVector(GX_LIGHTID_0, FX16_SQRT1_3, -FX16_SQRT1_3, -FX16_SQRT1_3);
	G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));

	G3_PushMtx();

	// Rotate and translate
	G3_Translate(3 << (FX32_SHIFT - 1), 0, 0);

	{
	//	fx16	s = FX_SinIdx(Rotate);
	//	fx16	c = FX_CosIdx(Rotate);

	//	G3_RotX(s, c);
	//	G3_RotY(s, c);
	//	G3_RotZ(s, c);
		G3_Scale(FX32_ONE, FX32_ONE, FX32_ONE);
	}

	{
		G3_MtxMode(GX_MTXMODE_TEXTURE);
		G3_Identity();
		// Use an identity matrix for the texture matrix for simplicity
		G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
	}

	G3_MaterialColorDiffAmb(GX_RGB(23, 23, 23), // diffuse
							GX_RGB(8, 8, 8), // ambient
							TRUE	   // use diffuse as vtx color if TRUE
		);

	G3_MaterialColorSpecEmi(GX_RGB(31, 31, 31), // specular
							GX_RGB(0, 0, 0),	// emission
							FALSE	   // use shininess table if TRUE
		);
//DEMO_Set3DDefaultShininessTable();

	G3_PolygonAttr(GX_LIGHTMASK_0,  // disable lights
				   GX_POLYGONMODE_MODULATE, 	// modulation mode
				   GX_CULL_NONE,	   // cull back
				   0,				   // polygon ID(0 - 63)
				   31,				   // alpha(0 - 31)
				   0				   // OR of GXPolygonAttrMisc's value
		);

    G3_TexPlttBase(0, GX_TEXFMT_PLTT16);	//パレットは全てのポリゴン共通

	//---------------------------------------------------------------------------
	// Draw a cube:
	// Specify different colors for the vertices.
	//---------------------------------------------------------------------------
	G3_Begin(GX_BEGIN_QUADS);
	
	offset.x = 0;
	offset.y = 0;
	offset.z = 0;
	for(i = 0; i < PALACE_POLYGON_MAX / 3; i++){
		G3_TexImageParam(GX_TEXFMT_PLTT16,		// use 16 colors palette texture
						 GX_TEXGEN_TEXCOORD,	// use texcoord
						 GX_TEXSIZE_S16,		// 64 pixels
						 GX_TEXSIZE_T16,		// 64 pixels
						 GX_TEXREPEAT_NONE, 	// no repeat
						 GX_TEXFLIP_NONE,		// no flip
						 GX_TEXPLTTCOLOR0_TRNS,	// use color 0 of the palette
						 BLOCK_TEX_SIZE * poly_no);	   // the offset of the texture image
		
		if(i == 0 || i == PALACE_POLYGON_MAX / 3 - 1){
			ColVtxQuad(2, 0, 4, 6, 0, &offset, rgb);	//前
			offset.z -= ONE_GRID / PALACE_EDIT_Z;
		}
		else if(i & 1){
			ColVtxQuad(2, 0, 4, 6, 0, &offset, rgb);	//前
			offset.z--;
		}
		else{
			ColVtxQuad(2, 0, 4, 6, 0, &offset, rgb);	//前
			offset.z -= ONE_GRID / PALACE_EDIT_Z - 1;
		}
		poly_no++;
	}

#if 1
	offset.x = 0;
	offset.y = 0;
	offset.z = 0;
	for(i = 0; i < PALACE_POLYGON_MAX / 3; i++){
		G3_TexImageParam(GX_TEXFMT_PLTT16,		// use 16 colors palette texture
						 GX_TEXGEN_TEXCOORD,	// use texcoord
						 GX_TEXSIZE_S16,		// 64 pixels
						 GX_TEXSIZE_T16,		// 64 pixels
						 GX_TEXREPEAT_NONE, 	// no repeat
						 GX_TEXFLIP_NONE,		// no flip
						 GX_TEXPLTTCOLOR0_TRNS,	// use color 0 of the palette
						 BLOCK_TEX_SIZE * poly_no);	   // the offset of the texture image
		
		if(i == 0 || i == PALACE_POLYGON_MAX / 3 - 1){
			ColVtxQuad(3, 1, 0, 2, 2, &offset, rgb);	//右(カメラから見て)
			offset.x -= ONE_GRID / PALACE_EDIT_X;
		}
		else if(i & 1){
			ColVtxQuad(3, 1, 0, 2, 2, &offset, rgb);	//右(カメラから見て)
			offset.x--;
		}
		else{
			ColVtxQuad(3, 1, 0, 2, 2, &offset, rgb);	//右(カメラから見て)
			offset.x -= ONE_GRID / PALACE_EDIT_X - 1;
		}
		poly_no++;
	}

	offset.x = 0;
	offset.y = 0;
	offset.z = 0;
	for(i = 0; i < PALACE_POLYGON_MAX / 3; i++){
		G3_TexImageParam(GX_TEXFMT_PLTT16,		// use 16 colors palette texture
						 GX_TEXGEN_TEXCOORD,	// use texcoord
						 GX_TEXSIZE_S16,		// 64 pixels
						 GX_TEXSIZE_T16,		// 64 pixels
						 GX_TEXREPEAT_NONE, 	// no repeat
						 GX_TEXFLIP_NONE,		// no flip
						 GX_TEXPLTTCOLOR0_TRNS,	// use color 0 of the palette
						 BLOCK_TEX_SIZE * poly_no);	   // the offset of the texture image
		
		if(i == 0 || i == PALACE_POLYGON_MAX / 3 - 1){
			ColVtxQuad(5, 4, 0, 1, 1, &offset, rgb);	//上
			offset.y -= ONE_GRID / PALACE_EDIT_Y;
		}
		else if(i & 1){
			ColVtxQuad(5, 4, 0, 1, 1, &offset, rgb);	//上
			offset.y--;
		}
		else{
			ColVtxQuad(5, 4, 0, 1, 1, &offset, rgb);	//上
			offset.y -= ONE_GRID / PALACE_EDIT_Y - 1;
		}
		poly_no++;
	}
#endif

	#if 0
		ColVtxQuad(2, 0, 4, 6, 0, add_pos, rgb);	//前
		ColVtxQuad(7, 5, 1, 3, 3, add_pos, rgb);	//後
		ColVtxQuad(6, 4, 5, 7, 5, add_pos, rgb);	//左(カメラから見て)
		ColVtxQuad(3, 1, 0, 2, 2, add_pos, rgb);	//右(カメラから見て)
		ColVtxQuad(5, 4, 0, 1, 1, add_pos, rgb);	//上
		ColVtxQuad(6, 7, 3, 2, 4, add_pos, rgb);	//下
	#endif
	
	G3_End();

	G3_PopMtx(1);
}

#if 0
static const u32 LIGHTING_L_DOT_S_SHIFT = 8;
#include <nitro/code32.h>              // avoid byte access problems
void DEMO_Set3DDefaultShininessTable()
{
    u8      i;
    u32     shininess_table[32];
    u8     *pShininess = (u8 *)&shininess_table[0];
    const u8 tableLength = 32 * sizeof(u32);

    for (i = 0; i < tableLength - 1; i++)
    {
        // ShininessTable is ( 0.8 ) fixed point foramt, so we have to right-shift 8 bit for One multiply.
        // pShininess = (i*2+1)^4
        pShininess[i] = (u8)(((s64)(i * 2 + 1) * (i * 2 + 1) * (i * 2 + 1) * (i * 2 + 1)) >> (LIGHTING_L_DOT_S_SHIFT * (4 - 1)));       // byte access
    }

    // set the max value specialy   
    pShininess[tableLength - 1] = 0xFF;

    G3_Shininess(&shininess_table[0]);
}
#endif

