//赤外線BGパレットアニメーション ircbattematch ircbattemenu共用


#define _PALETTENUM_ANIM (9)
#define _PALETTENUM_ANIM_COUNT (_PALETTENUM_ANIM*2)

typedef struct{
  u16 palTrans[_PALETTENUM_ANIM*2][16];
  u16 count;
  HEAPID heapID;
} IRC_BG_WORK;


inline static void ircBGAnimInit(IRC_BG_WORK* pWork)
{
  int i;
  NNSG2dPaletteData* palData;
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_IRCBATTLE, pWork->heapID );
  void* arcData = GFL_ARCHDL_UTIL_LoadPalette( handle, NARC_ircbattle_ir_demo_ani_NCLR, &palData, pWork->heapID);

  GFL_STD_MemCopy(palData->pRawData, pWork->palTrans, _PALETTENUM_ANIM*32);
  for(i=0;i<_PALETTENUM_ANIM;i++){
    GFL_STD_MemCopy(pWork->palTrans[_PALETTENUM_ANIM-i-1], pWork->palTrans[_PALETTENUM_ANIM+i], 32);
  }
  GFL_HEAP_FreeMemory(arcData);
  GFL_ARC_CloseDataHandle( handle );
}


inline static BOOL ircBGAnimMain(IRC_BG_WORK* pWork)
{
  if((OS_GetVBlankCount() % 8)==1){
    GX_LoadBGPltt(&pWork->palTrans[pWork->count], 0, 32);
    pWork->count++;
    if(pWork->count == _PALETTENUM_ANIM){
      return TRUE;  //黒になった瞬間
    }
    if(pWork->count >= _PALETTENUM_ANIM_COUNT){
      pWork->count=0;
    }
  }
  return FALSE;
}


