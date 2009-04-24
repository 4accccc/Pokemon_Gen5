#!/usr/bin/perl
###############################################################################
#
#	@file	source_convert.pl
#	@brif	金銀環境のソースを関数名等を置き換えWB用にコンバートする
#	@date	090415
#	@auther toru_nagihashi
#	
#	機能	関数名、構造体名、定義名等を置き換え
#			引数の順番を変更可能
#
###############################################################################


#------------------------------------------------------------------------------
#		
#					定数
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#		
#					コンバートデータ
#
#------------------------------------------------------------------------------
$CONVERT_TAG	= "/*↑[GS_CONVERT_TAG]*/\n";

#下記配列で、[1]に入れる値
#この値をいれると、置き換えは手動で行うものという指定になる。
#また、この箇所はコメントアウトされ、
$REPLACE_SELF	= "CONVERT_REPLACE_SELF";

#GSからWBを置き換えするもの	[0]replace_src [1]replace_dst [2] 引数の並び順...（元のまま場合は[2]より何もいれない
#
#	例
#	@GS_TO_WB_REPLACE	= (
#	[ "SRC", "DST", "1", "2", "3" ],
#	#->文字列SRCをDSTに変更し、SRCの引数を1,2,3の順番に置き換えます
#	#（もし　SRC( a, b, c ) ならば　DST( a, b, c )になります。
#	#もし　SRC( a, b, c, d )ならば、DST( a, b, c )になります。
#	#いくつSRCの引数があろうと、1,2,3としか書かれていないので、3つしか置き換えられません）
#
#	[ "SRC2", "DST2", "3", "1" ],
#	#文字列SRC2をDST2に変更し、SRCの引数を3,1の順番に置き換えます
#	#（もし　SRC2( a, b, c ) ならば　DST2( c, a )になります。
#	#もし　SRC2( a, b, c, d )ならば、DST2( c, a )になります。
#
#	[ "SRC3", "SRC3" ],
#	#文字列SRC3は何も置き換えません。
#	#引数はそのままです。
#
#	[ "SRC3", $REPLACE_SELF ],
#	#文字列SRC3は何も置き換えません。変わりに、
#	#プロンプト上に手動で置き換えてくださいのメッセージが流れます。
#	);
@GS_TO_WB_REPLACE	= (
	#ファイル名
	[ "fieldobj_code.h", "fldmmdl_code.h" ],
	#インクルード
	[ "#include \"common.h\"", "#include <gflib.h>" ],
	[ "#include \"system/procsys.h\"", "//#include \"system/procsys.h\"" ],			#SYSTEM
	[ "#include \"system/arc_tool.h\"", "//#include \"system/arc_tool.h\"" ],
	[ "#include \"system/arc_util.h\"", "//#include \"system/arc_util.h\"" ],
	[ "#include \"system/clact_tool.h\"", "//#include \"system/clact_tool.h\"" ],
	[ "#include \"system/msgdata_util.h\"", "//#include \"system/msgdata_util.h\"" ],
	[ "#include \"system/lib_pack.h\"", "//#include \"system/lib_pack.h\"" ],
	["GF_BGL_BglIniAlloc","GFL_BG_Init"],
	["GF_BGL_HeapIDGet","GFL_BG_GetHeapID",0],
	["GF_BGL_InitBG","GFL_BG_SetBGMode"],
	["GF_BGL_InitBGDisp","GFL_BG_SetBGModeDisp"],
	["GF_BGL_BGControlSet","GFL_BG_SetBGControl",2,3,4],
	["GF_BGL_BGControlReset","GFL_BG_ResetBGControl",2,3,4],
	["GF_BGL_BGControlExit","GFL_BG_FreeBGControl",2],
	["GF_BGL_PrioritySet","GFL_BG_SetPriority"],
	["GF_BGL_VisibleSet","GFL_BG_SetVisible"],
	["GF_BGL_ScrollSet","GFL_BG_SetScroll",2,3,4],
	["GF_BGL_ScrollGetX","GFL_BG_GetScrollX",2],
	["GF_BGL_ScrollGetY","GFL_BG_GetScrollY",2],
	["GF_BGL_AffineScrollSet","GFL_BG_SetAffineScroll",2,3,4,5,6,7],
	["GF_BGL_AffineSet","GFL_BG_SetAffine",2,3,4,5],
	["GF_BGL_DataDecord","GFL_BG_DecodeData"],
	["GF_BGL_LoadScreenReq","GFL_BG_LoadScreenReq",2],
	["GF_BGL_LoadScreen","GFL_BG_LoadScreen",2,3,4,5],
	["GF_BGL_ScreenBufSet","GFL_BG_LoadScreenBuffer",2,3,4],
	["GF_BGL_LoadScreenFile",$REPLACE_SELF],
	["GF_BGL_LoadCharacterFile",$REPLACE_SELF],
	["GF_BGL_LoadCharacter","GFL_BG_LoadCharacter",2,3,4,5],
	["GF_BGL_ClearCharSet","GFL_BG_SetClearCharacter"],
	["GF_BGL_CharFill","GFL_BG_FillCharacter",2,3,4,5],
	["GF_BGL_PaletteSet","GFL_BG_LoadPalette"],
	["GF_BGL_BackGroundColorSet","GFL_BG_SetBackGroundColor"],
	["GF_BGL_ScrWrite","GFL_BG_WriteScreen",2,3,4,5,6,7],
	["GF_BGL_ScrWriteExpand","GFL_BG_WriteScreenExpand",2,3,4,5,6,7,8,9,10,11],
	["GF_BGL_ScrWriteFree","GFL_BG_WriteScreenFree",2,3,4,5,6,7,8,9,10,11],
	["GF_BGL_ScrFill","GFL_BG_FillScreen",2,3,4,5,6,7,8],
	["GF_BGL_ScrPalChange","GFL_BG_ChangeScreenPalette",2,3,4,5,6,7],
	["GF_BGL_ScrClear","GFL_BG_ClearScreen",2],
	["GF_BGL_ScrClearCode","GFL_BG_ClearScreenCode",2,3],
	["GF_BGL_ScrClearCodeVReq","GFL_BG_ClearScreenCodeVReq",2,3],
	["GF_BGL_CgxGet","GFL_BG_GetCharacterAdrs"],
	["GF_BGL_4BitCgxChange8BitMain","GFL_BG_ChangeCharacter4BitTo8Bit"],
	["GF_BGL_4BitCgxChange8Bit","GFL_BG_ChangeCharacter4BitTo8BitAlloc"],
	["GF_BGL_ScreenAdrsGet","GFL_BG_GetScreenBufferAdrs",2],
	["GF_BGL_ScreenSizGet","GFL_BG_GetScreenBufferSize",2],
	["GF_BGL_ScreenTypeGet","GFL_BG_GetScreenType",2],
	["GF_BGL_ScreenScrollXGet","GFL_BG_GetScreenScrollX",2],
	["GF_BGL_ScreenScrollYGet","GFL_BG_GetScreenScrollY",2],
	["GF_BGL_ScreenColorModeGet","GFL_BG_GetScreenColorMode",2],
	["GF_BGL_RadianGet","GFL_BG_GetRadian",2],
	["GF_BGL_ScaleXGet","GFL_BG_GetScaleX",2],
	["GF_BGL_ScaleYGet","GFL_BG_GetScaleY",2],
	["GF_BGL_CenterXGet","GFL_BG_GetCenterX",2],
	["GF_BGL_CenterYGet","GFL_BG_GetCenterY",2],
	["GF_BGL_PriorityGet","GFL_BG_GetPriority",2],
	["GF_BGL_NTRCHR_CharLoad",$REPLACE_SELF],
	["GF_BGL_NTRCHR_CharLoadEx",$REPLACE_SELF],
	["GF_BGL_NTRCHR_CharGet",$REPLACE_SELF],
	["GF_BGL_NTRCHR_PalLoad",$REPLACE_SELF],
	["GF_BGL_NTRCHR_ScrnLoad",$REPLACE_SELF],
	["GF_BGL_VBlankFunc","GFL_BG_VBlankFunc",0],
	["GF_BGL_LoadScreenV_Req","GFL_BG_LoadScreenV_Req",2],
	["GF_BGL_ScrollReq","GFL_BG_SetScrollReq",2,3,4],
	["GF_BGL_RadianSetReq","GFL_BG_SetRadianReq",2,3,4],
	["GF_BGL_ScaleSetReq","GFL_BG_SetScaleReq",2,3,4],
	["GF_BGL_RotateCenterSetReq","GFL_BG_SetRotateCenterReq",2,3,4],
	["GF_BGL_DotCheck","GFL_BG_CheckDot",2,3,4,5],
	["sys_InitHeapSystem","GFL_HEAP_Init"],
	["sys_CreateHeap","GFL_HEAP_CreateHeap"],
	["sys_CreateHeapLo","GFL_HEAP_CreateHeapLo"],
	["sys_DeleteHeap","GFL_HEAP_DeleteHeap"],
	["sys_AllocMemory","GFL_HEAP_AllocMemory"],
	["sys_AllocMemoryLo","GFL_HEAP_AllocMemoryLo"],
	["sys_FreeMemoryEz","GFL_HEAP_FreeMemory",2],
	["sys_FreeMemory","GFL_HEAP_FreeMemory"],
	["sys_InitAllocator","GFL_HEAP_InitAllocator"],
	["sys_CutMemoryBlockSize","GFL_HEAP_ResizeMemory"],
	["sys_GetHeapFreeSize","GFL_HEAP_GetHeapFreeSize"],
	["sys_GetHeapAllocatedSize",$REPLACE_SELF],
	["sys_CheckHeapSafe","GFL_HEAP_CheckHeapSafe"],
	["sys_PrintHeapFreeSize",$REPLACE_SELF],
	["sys_PrintHeapExistMemoryInfo",$REPLACE_SELF],
	["sys_GetHeapState",$REPLACE_SELF],
	["sys_PrintHeapConflict",$REPLACE_SELF],
	["sys_GetMemoryBlockSize",$REPLACE_SELF],
	["sys_CheckHeapFullReleased",$REPLACE_SELF],
	["PROC","GFL_PROC"],
	["PROC_Create",$REPLACE_SELF],
	["PROC_CreateChild",$REPLACE_SELF],
	["PROC_GetParentWork",$REPLACE_SELF],
	["PROC_SetPause",$REPLACE_SELF],
	["InitTPSystem","GFL_UI_TP_Init",0],
	["InitTP",$REPLACE_SELF],
	["InitTPNoBuff",$REPLACE_SELF],
	["MainTP","GFL_UI_TP_Main",0],
	["StopTP","GFL_UI_TP_Exit"],
	["RECT_HIT_TBL","GFL_UI_TP_HITTBL"],
	["TP_HIT_TBL","GFL_UI_TP_HITTBL"],
	["TP_HIT_END","GFL_UI_TP_HIT_END"],
	["RECT_HIT_END","GFL_UI_TP_HIT_END"],
	["TP_USE_CIRCLE","GFL_UI_TP_USE_CIRCLE"],
	["TP_HIT_NONE","GFL_UI_TP_HIT_NONE"],
	["RECT_HIT_NONE","GFL_UI_TP_HIT_NONE"],
	["GF_TP_RectHitCont","GFL_UI_TP_HitCont"],
	["GF_TP_HitCont","GFL_UI_TP_HitCont"],
	["GF_TP_RectHitTrg","GFL_UI_TP_HitTrg"],
	["GF_TP_HitTrg","GFL_UI_TP_HitTrg"],
	["GF_TP_SingleHitCont",$REPLACE_SELF],
	["GF_TP_SingleHitTrg",$REPLACE_SELF],
	["GF_TP_GetCont","GFL_UI_TP_GetCont"],
	["GF_TP_GetTrg","GFL_UI_TP_GetTrg"],
	["GF_TP_RectHitContSelf",$REPLACE_SELF],
	["GF_TP_HitSelf","GFL_UI_TP_HitSelf"],
	["GF_TP_GetPointCont","GFL_UI_TP_GetPointCont"],
	["GF_TP_GetPointTrg","GFL_UI_TP_GetPointTrg"],
	["sys.trg","GFL_UI_KEY_GetTrg()"],
	["sys.cont","GFL_UI_KEY_GetCont()"],
	["sys.repeat","GFL_UI_KEY_GetRepeat()"],
	["TCBSYS","GFL_TCBSYS"],
	["TCB_PTR","GFL_TCB*"],
	["TCBSYS_CalcSystemWorkSize","GFL_TCB_CalcSystemWorkSize"],
	["TCBSYS_Create","GFL_TCB_Init"],
	["TCBSYS_Main","GFL_TCB_Main"],
	["TCBSYS_AddTask","GFL_TCB_AddTask"],
	["TCBSYS_DeleteTask","GFL_TCB_DeleteTask"],
	["TCB_ChangeFunc","GFL_TCB_ChangeFunc"],
	["TCB_GetWork","GFL_TCB_GetWork"],
	["TCB_GetPriority","GFL_TCB_GetPriority"],
	["DebugTCBSys_FuncEntryCheck",$REPLACE_SELF],
	["memset","GFL_STD_MemFill"],
	["memcpy","GFL_STD_MemCopy"],
	["memcmp","GFL_STD_MemComp"],
	["gf_srand","GFUser_GetPublicRand"],
	["gf_rand","GFUser_GetPublicRand(GFUSER_RAND_PAST_MAX)"],
	["ArcUtil_BgCharSet","GFL_ARC_UTIL_TransVramBgCharacter"],
	["ArcUtil_ObjCharSet","GFL_ARC_UTIL_TransVramObjCharacter"],
	["ArcUtil_ScrnSet","GFL_ARC_UTIL_TransVramScreen",1,2,4,5,6,7,8],
	["ArcUtil_PalSet","GFL_ARC_UTIL_TransVramPalette"],
	["ArcUtil_PalSetEx","GFL_ARC_UTIL_TransVramPaletteEx"],
	["ArcUtil_PalSysLoad","GFL_ARC_UTIL_TransVramPaletteMakeProxy"],
	["ArcUtil_CharSysLoad","GFL_ARC_UTIL_TransVramCharacterMakeProxy"],
	["ArcUtil_CharSysLoadSyncroMappingMode","GFL_ARC_UTIL_TransVramCharacterMakeProxySyncroMappingMode"],
	["ArcUtil_TransTypeCharSysLoad","GFL_ARC_UTIL_LoadTransTypeCharacterMakeProxy"],
	["ArcUtil_CharDataGet","GFL_ARC_UTIL_LoadBGCharacter or GFL_ARC_UTIL_LoadOBJCharacter"],
	["ArcUtil_ScrnDataGet","GFL_ARC_UTIL_LoadScreen"],
	["ArcUtil_PalDataGet","GFL_ARC_UTIL_LoadPalette"],
	["ArcUtil_CellBankDataGet","GFL_ARC_UTIL_LoadCellBank"],
	["ArcUtil_AnimBankDataGet","GFL_ARC_UTIL_LoadAnimeBank"],
	["ArcUtil_UnCompress","GFL_ARC_UTIL_LoadUnCompress"],
	["ArcUtil_Load","GFL_ARC_UTIL_Load"],
	["ArcUtil_LoadEx","GFL_ARC_UTIL_LoadEx"],
	["ArcUtil_HDL_BgCharSet","GFL_ARCHDL_UTIL_TransVramBgCharacter"],
	["ArcUtil_HDL_ObjCharSet","GFL_ARCHDL_UTIL_TransVramObjCharacter"],
	["ArcUtil_HDL_ScrnSet","GFL_ARCHDL_UTIL_TransVramScreen",1,2,4,5,6,7,8],
	["ArcUtil_HDL_PalSet","GFL_ARCHDL_UTIL_TransVramPalette"],
	["ArcUtil_HDL_PalSetEx","GFL_ARCHDL_UTIL_TransVramPaletteEx"],
	["ArcUtil_HDL_PalSysLoad","GFL_ARCHDL_UTIL_TransVramPaletteMakeProxy"],
	["ArcUtil_HDL_CharSysLoad","GFL_ARCHDL_UTIL_TransVramCharacterMakeProxy"],
	["ArcUtil_HDL_CharSysLoadSyncroMappingMode","GFL_ARCHDL_UTIL_TransVramCharacterMakeProxySyncroMappingMode"],
	["ArcUtil_HDL_TransTypeCharSysLoad","GFL_ARCHDL_UTIL_LoadTransTypeCharacterMakeProxy"],
	["ArcUtil_HDL_CharDataGet","GFL_ARC_UTIL_LoadBGCharacter or GFL_ARC_UTIL_LoadOBJCharacter"],
	["ArcUtil_HDL_ScrnDataGet","GFL_ARCHDL_UTIL_LoadScreen"],
	["ArcUtil_HDL_PalDataGet","GFL_ARCHDL_UTIL_LoadPalette"],
	["ArcUtil_HDL_CellBankDataGet","GFL_ARCHDL_UTIL_LoadCellBank"],
	["ArcUtil_HDL_AnimBankDataGet","GFL_ARCHDL_UTIL_LoadAnimeBank"],
	["ArcUtil_HDL_UnCompress","GFL_ARCHDL_UTIL_LoadUnCompress"],
	["ArcUtil_HDL_Load","GFL_ARCHDL_UTIL_Load"],
	["ArcUtil_HDL_LoadEx","GFL_ARCHDL_UTIL_LoadEx"],
	["ArchiveDataLoad",$REPLACE_SELF],
	["ArchiveDataLoadMalloc",$REPLACE_SELF],
	["ArchiveDataLoadMallocLo",$REPLACE_SELF],
	["ArchiveDataLoadOfs",$REPLACE_SELF],
	["ArchiveDataLoadMallocOfs",$REPLACE_SELF],
	["ArchiveDataLoadMallocOfsLo",$REPLACE_SELF],
	["ArchiveDataFileCntGet",$REPLACE_SELF],
	["ArchiveDataSizeGet",$REPLACE_SELF],
	["ArchiveDataHandleOpen","GFL_ARC_OpenDataHandle"],
	["ArchiveDataHandleClose","GFL_ARC_CloseDataHandle"],
	["ArchiveDataLoadByHandle","GFL_ARC_LoadDataByHandle"],
	["ArchiveDataSizeGetByHandle","GFL_ARC_GetDataSizeByHandle"],
	["ArchiveDataLoadOfsByHandle","GFL_ARC_LoadDataOfsByHandle"],
	["ArchiveDataLoadOfsOnlyByHandle",$REPLACE_SELF],
	["ArchiveDataLoadAllocByHandle","GFL_ARC_LoadDataAllocByHandle"],
	["ArchiveDataLoadImgofsByHandle","GFL_ARC_LoadDataImgofsByHandle"],
	["ArchiveDataLoadByHandleContinue","GFL_ARC_LoadDataByHandleContinue"],
	["ArchiveDataSeekByHandle","GFL_ARC_SeekDataByHandle"],
	["ArchiveDataFileCntGetByHandle","GFL_ARC_GetDataFileCntByHandle"],
	["GF_BGL_DISPVRAM","GFL_DISP_VRAM"],
	["GF_Disp_SetBank","GFL_DISP_SetBank"],
	["GF_Disp_GX_VisibleControlInit","GFL_DISP_GX_InitVisibleControl"],
	["GF_Disp_GXS_VisibleControlInit","GFL_DISP_GXS_InitVisibleControl"],
	["GF_Disp_GX_VisibleControl","GFL_DISP_GX_SetVisibleControl"],
	["GF_Disp_GXS_VisibleControl","GFL_DISP_GXS_SetVisibleControl"],
	["GF_Disp_GX_VisibleControlDirect","GFL_DISP_GX_SetVisibleControlDirect"],
	["GF_Disp_GXS_VisibleControlDirect","GFL_DISP_GXS_SetVisibleControlDirect"],
	["GF_Disp_DispOn","GFL_DISP_SetDispOn"],
	["GF_Disp_DispOff","GFL_DISP_SetDispOff"],
	["GF_Disp_DispSelect","GFL_DISP_SetDispSelect"],
	["GF_Disp_MainVisibleGet","GFL_DISP_GetMainVisible"],
	["GF_Disp_SubVisibleGet","GFL_DISP_GetSubVisible"],
	["CATS_SYS_PTR",$REPLACE_SELF],
	["CATS_ObjectAffineSet","GFL_CLACT_WK_SetAffineParam"],
	["CATS_ObjectAffineSetCap","GFL_CLACT_WK_SetAffineParam"],
	["CATS_ObjectAffineGet","GFL_CLACTWkGetAffineParam"],
	["CATS_ObjectAffineGetCap","GFL_CLACTWkGetAffineParam"],
	["CATS_ObjectScaleSet","GFL_CLACT_WK_SetScale"],
	["CATS_ObjectScaleSetCap","GFL_CLACT_WK_SetScale"],
	["CATS_ObjectScaleAdd",$REPLACE_SELF],
	["CATS_ObjectScaleAddCap",$REPLACE_SELF],
	["CATS_ObjectScaleGet","GFL_CLACT_WK_GetScale"],
	["CATS_ObjectScaleGetCap","GFL_CLACT_WK_GetScale"],
	["CATS_ObjectRotationSet","GFL_CLACT_WK_SetRotation"],
	["CATS_ObjectRotationSetCap","GFL_CLACT_WK_SetRotation"],
	["CATS_ObjectRotationAdd",$REPLACE_SELF],
	["CATS_ObjectRotationAddCap",$REPLACE_SELF],
	["CATS_ObjectRotationGet","GFL_CLACT_WK_GetRotation"],
	["CATS_ObjectRotationGetCap","GFL_CLACT_WK_GetRotation"],
	["CATS_ObjectFlipSet","GFL_CLACT_WK_SetFlip"],
	["CATS_ObjectFlipSetCap","GFL_CLACT_WK_SetFlip"],
	["CATS_ObjectFlipGet","GFL_CLACT_WK_GetFlip"],
	["CATS_ObjectFlipGetCap","GFL_CLACT_WK_GetFlip"],
	["CATS_ObjectAffinePosSetCap","GFL_CLACT_WK_SetAffinePos"],
	["CATS_ObjectAffinePosGetCap","GFL_CLACT_WK_GetAffinePos"],
	["CATS_ObjectAffinePosMoveCap",$REPLACE_SELF],
	["CATS_ObjectMosaicSet","GFL_CLACT_WK_SetMosaic"],
	["CATS_ObjectObjModeSet","GFL_CLACT_WK_SetObjMode"],
	["CATS_ObjectObjModeSetCap","GFL_CLACT_WK_SetObjMode"],
	["CATS_ObjectObjModeGet","GFL_CLACT_WK_GetObjMode"],
	["CATS_ObjectObjModeGetCap","GFL_CLACT_WK_GetObjMode"],
	["CATS_ObjectUpdate","GFL_CLACT_WK_AddAnmFrame"],
	["CATS_ObjectUpdateCap","GFL_CLACT_WK_AddAnmFrame"],
	["CATS_ObjectUpdateCapEx","GFL_CLACT_WK_AddAnmFrame"],
	["CATS_ObjectUpdateNumCap","GFL_CLACT_WK_AddAnmFrame"],
	["CATS_ObjectAnimeReStartCap","GFL_CLACT_WK_ResetAnm"],
	["CATS_ObjectAnimeSeqNumGetCap","GFL_CLACT_WK_GetAnmSeq"],
	["CATS_ObjectAnimeSeqSetCap","GFL_CLACT_WK_SetAnmSeq"],
	["CATS_ObjectAutoAnimeSet","GFL_CLACT_WK_SetAutoAnmFlag"],
	["CATS_ObjectAutoAnimeSetCap","GFL_CLACT_WK_SetAutoAnmFlag"],
	["CATS_ObjectAutoAnimeGet","GFL_CLACT_WK_GetAutoAnmFlag"],
	["CATS_ObjectAutoAnimeGetCap","GFL_CLACT_WK_GetAutoAnmFlag"],
	["CATS_ObjectAutoAnimeSpeedSet","GFL_CLACT_WK_SetAutoAnmSpeed"],
	["CATS_ObjectAutoAnimeSpeedSetCap","GFL_CLACT_WK_SetAutoAnmSpeed"],
	["CATS_ObjectAutoAnimeSpeedGet","GFL_CLACT_WK_GetAutoAnmSpeed"],
	["CATS_ObjectAutoAnimeSpeedGetCap","GFL_CLACT_WK_GetAutoAnmSpeed"],
	["CATS_ObjectAnimeFrameSet","GFL_CLACT_WK_SetAnmFrame"],
	["CATS_ObjectAnimeFrameSetCap","GFL_CLACT_WK_SetAnmFrame"],
	["CATS_ObjectAnimeFrameGet","GFL_CLACT_WK_GetAnmFrame"],
	["CATS_ObjectAnimeFrameGetCap","GFL_CLACT_WK_GetAnmFrame"],
	["CATS_ObjectAnimeActiveCheck","GFL_CLACT_WK_CheckAnmActive"],
	["CATS_ObjectAnimeActiveCheckCap","GFL_CLACT_WK_CheckAnmActive"],
	["CATS_ObjectEnable","GFL_CLACT_UNIT_SetDrawEnable"],
	["CATS_ObjectEnableCap","GFL_CLACT_UNIT_SetDrawEnable"],
	["CATS_ObjectEnableGet","GFL_CLACT_UNIT_GetDrawEnable"],
	["CATS_ObjectEnableGetCap","GFL_CLACT_UNIT_GetDrawEnable"],
	["CATS_ObjectPaletteSet",$REPLACE_SELF],
	["CATS_ObjectPaletteSetCap",$REPLACE_SELF],
	["CATS_ObjectPaletteOffsetSet","GFL_CLACT_WK_SetPlttOffs"],
	["CATS_ObjectPaletteOffsetSetCap","GFL_CLACT_WK_SetPlttOffs"],
	["CATS_ObjectPaletteOffsetGetCap","GFL_CLACT_WK_GetPlttOffs"],
	["CATS_ObjectBGPriSet","GFL_CLACT_WK_SetBgPri"],
	["CATS_ObjectBGPriSetCap","GFL_CLACT_WK_SetBgPri"],
	["CATS_ObjectBGPriGetCap","CATS_ObjectBGPriGetCap"],
	["CATS_ObjectPriSet","GFL_CLACT_WK_SetSoftPri"],
	["CATS_ObjectPriSetCap","GFL_CLACT_WK_SetSoftPri"],
	["CATS_ObjectPriGet","GFL_CLACT_WK_GetSoftPri"],
	["CATS_ObjectPriGetCap","GFL_CLACT_WK_GetSoftPri"],
	["CATS_ObjectPosSet","GFL_CLACT_WK_SetWldPos"],
	["CATS_ObjectPosSetCap","GFL_CLACT_WK_SetWldPos"],
	["CATS_ObjectPosSet_SubSurface",$REPLACE_SELF],
	["CATS_ObjectPosSetCap_SubSurface",$REPLACE_SELF],
	["CATS_ObjectPosGet","GFL_CLACT_WK_GetPos"],
	["CATS_ObjectPosGetCap","GFL_CLACT_WK_GetPos"],
	["CATS_ObjectPosGet_SubSurface",$REPLACE_SELF],
	["CATS_ObjectPosGetCap_SubSurface",$REPLACE_SELF],
	["CATS_ObjectPosMove",$REPLACE_SELF],
	["CATS_ObjectPosMoveCap",$REPLACE_SELF],
	["CATS_ObjectPosMoveCapFx32",$REPLACE_SELF],
	["CATS_ObjectPosSetCapFx32",$REPLACE_SELF],
	["CATS_ObjectPosGetCapFx32",$REPLACE_SELF],
	["CATS_ObjectPosSetCapFx32_SubSurface",$REPLACE_SELF],
	["CATS_ObjectPosGetCapFx32_SubSurface",$REPLACE_SELF],
	["CATS_ObjectUserAttrAnimSeqGet","GFL_CLACT_WK_GetUserAttrAnmSeq"],
	["CATS_ObjectUserAttrNowAnimSeqGet","GFL_CLACT_WK_GetUserAttrAnmSeqNow"],
	["CATS_ObjectUserAttrAnimFrameGet","GFL_CLACT_WK_GetUserAttrAnmFrame"],
	["CATS_ObjectUserAttrNowAnimFrameGet","GFL_CLACT_WK_GetUserAttrAnmFrameNow"],
	["CATS_ObjectUserAttrCellGet","GFL_CLACT_WK_GetUserAttrCell"],
	["CATS_LoadResourceCharArcModeAdjust",$REPLACE_SELF],
	["CATS_LoadResourceCharArcModeAdjustAreaCont",$REPLACE_SELF],
	["CATS_LoadResourceCharArcModeAdjustAreaCont_Handle",$REPLACE_SELF],
	["CATS_ChangeResourceCharArc",$REPLACE_SELF],
	["CATS_ChangeResourcePlttArc",$REPLACE_SELF],
	["CATS_GetClactSetPtr",$REPLACE_SELF],
	["CATS_SetClactSetPtr",$REPLACE_SELF],
	["CATS_PlttProxy",$REPLACE_SELF],
	["CATS_PlttID_NoGet",$REPLACE_SELF],
	["CATS_ChangeResourceCharArcH",$REPLACE_SELF],
	["CATS_ChangeResourcePlttArcH",$REPLACE_SELF],
	["CATS_ChangeResourcePlttArcH_PlttWork",$REPLACE_SELF],
	["CLACT_MosaicSet","GFL_CLACT_WK_SetMosaic"],
	["CLACT_MosaicGet","GFL_CLACT_WK_GetMosaic"],
	["REND_OAMInit","GFL_CLACT_Init"],
	["CLACT_U_SetEasyInit","GFL_CLACT_UNIT_Create"],
	["CLACT_U_MakeHeader","GFL_CLACT_WK_SetCellResData or GFL_CLACT_WK_SetTrCellResData or GFL_CLACT_WK_SetMCellResData"],
	["CLACT_Add","GFL_CLACT_WK_Add"],
	["CLACT_DestSet","GFL_CLACT_UNIT_Delete"],
	["REND_OAM_Delete","GFL_CLACT_Exit"],
	["CLACT_AnmGet","GFL_CLACT_WK_GetAnmSeq"],
	["CLACT_AnmFrameChg","GFL_CLACT_WK_AddAnmFrame"],
	["CLACT_AnmFrameSet","GFL_CLACT_WK_SetAnmFrame"],
	["CLACT_AnmFrameGet","GFL_CLACT_WK_GetAnmFrame"],
	["CLACT_SetMatrix","GFL_CLACT_WK_SetWldPos"],
	["CLACT_SetAffineMatrix","GFL_CLACT_WK_SetAffinePos"],
	["CLACT_SetScale","GFL_CLACT_WK_SetScale"],
	["CLACT_SetScaleAffine","GFL_CLACT_WK_SetScale" ,1,2],
	["CLACT_SetRotation","GFL_CLACT_WK_SetRotation"],
	["CLACT_SetRotationAffine","GFL_CLACT_WK_SetRotation",1,2],
	["CLACT_SetDrawFlag","GFL_CLACT_WK_SetDrawEnable"],
	["CLACT_GetDrawFlag","GFL_CLACT_WK_GetDrawEnable"],
	["CLACT_SetAnmFlag","GFL_CLACT_WK_SetAutoAnmFlag"],
	["CLACT_GetAnmFlag","GFL_CLACT_WK_GetAutoAnmFlag"],
	["CLACT_SetAffineParam","GFL_CLACT_WK_SetAffineParam"],
	["CLACT_SetFlip","GFL_CLACT_WK_SetFlip"],
	["CLACT_GetMatrix","GFL_CLACT_WK_GetWldPos"],
	["CLACT_GetAffineMatrix","GFL_CLACT_WK_GetWldPos"],
	["CLACT_GetScale","GFL_CLACT_WK_GetScale"],
	["CLACT_GetRotation","GFL_CLACT_WK_GetRotation"],
	["CLACT_GetAnmFrame","GFL_CLACT_WK_GetAutoAnmSpeed"],
	["CLACT_GetAffineParam","GFL_CLACT_WK_GetAffineParam"],
	["CLACT_GetFlip","GFL_CLACT_WK_GetFlip"],
	["CLACT_GetAnmSeqNum","GFL_CLACT_WK_GetAnmSeqNum"],
	["CLACT_AnmChg","GFL_CLACT_WK_SetAnmSeq"],
	["CLACT_AnmChgCheck","GFL_CLACT_WK_SetAnmSeqDiff"],
	["CLACT_AnmReStart","GFL_CLACT_WK_ResetAnm"],
	["CLACT_BGPriorityChg","GFL_CLACT_WK_SetBgPri"],
	["CLACT_BGPriorityGet","GFL_CLACT_WK_GetBgPri"],
	["CLACT_PaletteNoChg",$REPLACE_SELF],
	["CLACT_PaletteNoChgAddTransPlttNo",$REPLACE_SELF],
	["CLACT_PaletteNoGet",$REPLACE_SELF],
	["CLACT_PaletteOffsetChg","GFL_CLACT_WK_SetPlttOffs"],
	["CLACT_PaletteOffsetChgAddTransPlttNo","GFL_CLACT_WK_GetPlttOffsMode"],
	["CLACT_PaletteOffsetGet","GFL_CLACT_WK_GetPlttOffs"],
	["CLACT_DrawPriorityGet","GFL_CLACT_WK_GetSoftPri"],
	["CLACT_ImageProxySet",$REPLACE_SELF],
	["CLACT_ImageProxyGet",$REPLACE_SELF],
	["CLACT_PaletteProxySet","GFL_CLACT_WK_SetPlttProxy"],
	["CLACT_PaletteProxyGet","GFL_CLACT_WK_GetPlttProxy"],
	["CLACT_VramTypeGet",$REPLACE_SELF],
	["CLACT_AnmActiveCheck","GFL_CLACT_WK_CheckAnmActive"],
	["CLACT_ObjModeSet","GFL_CLACT_WK_SetObjMode"],
	["CLACT_ObjModeGet","GFL_CLACT_WK_GetObjMode"],
	["CLACT_DrawPriorityChg","GFL_CLACT_WK_SetSoftPri"],
	["CATS_ObjectAutoAnimeSetCap","GFL_CLACT_WK_SetAutoAnmFlag"],
	["CATS_ObjectUpdate","GFL_CLACT_WK_AddAnmFrame"],
	["CATS_ObjectObjModeSetCap","GFL_CLACT_WK_SetObjMode"],
	["CATS_Draw","GFL_CLACT_SYS_Main"],
	["gflib/fontdata_man","print/gf_font"],
	["NUBER_CODETYPE","StrNumberCodeType"],
	["GF_FONTDATA_MAN","GFL_FONT"],
	["FontDataMan_Create","GFL_FONT_Create"],
	["FontDataMan_Delete","GFL_FONT_Delete"],
	["FontDataMan_GetBitmap","GFL_FONT_GetBitMap"],
	["FontDataMan_GetStrWidth","GFL_FONT_GetWidth"],
	["FontDataMan_GetStrLineWidth",$REPLACE_SELF],
	["FontDataMan_ErrorStrCheck",$REPLACE_SELF],
	["gflib/msg_print.h","print/printsys.h"],
	["GF_PRINTCOLOR","PRINTSYS_LSB"],
	["GF_PRINTCOLOR_GET_LETTER","PRINTSYS_LSB_GetL"],
	["GF_PRINTCOLOR_GET_SHADOW","PRINTSYS_LSB_GetS"],
	["GF_PRINTCOLOR_GET_GROUND","PRINTSYS_LSB_GetB"],
	["GF_PRINTCOLOR_MAKE","PRINTSYS_LSB_Make"],
	["MSG_PrintSysInit","PRINTSYS_Init"],
	["GF_BGL_BmpWinDataFill","GFL_BMP_Clear"],
	["TalkWinGraphicSet","BmpWinFrame_GraphicSet"],
	["BMP_MENULIST_Create","BmpMenuWork_ListCreate"],
	["MenuWinGraphicSet","BmpWinFrame_GraphicSet"],
	["BmpMenuWinWrite","BmpWinFrame_Write"],
	["Particle_DrawAll","GFL_PTC_DrawAll"],
	["Particle_CalcAll","GFL_PTC_CalcAll"],
	["GF_BMP_PrintMain","GFL_BMP_Print"],
	["GF_BMP_PrintMain256","GFL_BMP_Print16to256"],
	["GF_BGL_BmpFill","GFL_BMP_Fill"],
	["GF_BGL_BmpWinAllocGet","GFL_BMPWIN_Init"],
	["GFL_BMPWIN_Create",$REPLACE_SELF],
	["GF_BGL_BmpWinDel","GFL_BMPWIN_Delete"],
	["GF_BGL_BmpWinFree",$REPLACE_SELF],
	["GF_BGL_BmpWinOn","GFL_BMPWIN_MakeTransWindow"],
	["GF_BGL_BmpWinOnVReq","GFL_BMPWIN_MakeTransWindow_VBlank"],
	["GF_BGL_BmpWinMakeScrn","GFL_BMPWIN_MakeScreen"],
	["GF_BGL_BmpWinMakeScrnLimited",$REPLACE_SELF],
	["GF_BGL_BmpWinClearScrn","GFL_BMPWIN_ClearScreen"],
	["GF_BGL_BmpWinCgxOn","GFL_BMPWIN_TransVramCharacter"],
	["GF_BGL_BmpWinGet_Frame","GFL_BMPWIN_GetFrame"],
	["GF_BGL_BmpWinGet_SizeX","GFL_BMPWIN_GetSizeX"],
	["GF_BGL_BmpWinGet_SizeY","GFL_BMPWIN_GetSizeY"],
	["GF_BGL_BmpWinGet_PosX","GFL_BMPWIN_GetPosX"],
	["GF_BGL_BmpWinGet_PosY","GFL_BMPWIN_GetPosY"],
	["GF_BGL_BmpWinGet_Chrofs","GFL_BMPWIN_GetChrNum"],
	["GF_BGL_BmpWinSet_PosX","GFL_BMPWIN_SetPosX"],
	["GF_BGL_BmpWinSet_PosY","GFL_BMPWIN_SetPosY"],
	["GF_BGL_BmpWinSet_Pal","GFL_BMPWIN_SetPalette"]


);


$CONVERT_CNT	= 0;

#------------------------------------------------------------------------------
#		
#					メイン
#
#------------------------------------------------------------------------------
#引数正当性チェック
if( @ARGV != 2 ){
	print "Usage: gs_convert.pl src_file_name dst_file_name \n";
	print "GSproject source and header file convert WBproject\n";
	exit(1);
}

print "\n▽コンバート開始\n";
print "変更元:$ARGV[0]\n";

#コンバート
&CONVERT_Execute( $ARGV[0], $ARGV[1] );

print "△コンバート完了\n";
print "変更先:$ARGV[1]\n";
print "変更回数:$CONVERT_CNT\n\n";

#終了
exit(0);

#------------------------------------------------------------------------------
#		
#					サブルーチン
#
#------------------------------------------------------------------------------
#
#	コンバート本体
#	@param	_[0]	コンバート元ファイル
#	@param	_[1]	コンバート先ファイル
#
#	@retval	TRUE	コンバート終了
#	@retval	FALSE	コンバートできなかった
#
sub CONVERT_Execute
{
	my( $src_file, $dst_file )	= @_;

	##コンバート元ファイルから文字列取得
	#ファイルオープン
	open( FILEIN, "<$src_file" );
	#全部の行を取得
	@buff	= <FILEIN>;

	#一端すべての改行を統一する
	for( my $tmp = 0; $tmp < @buff; $tmp++ ){
		my $tmp_word	= $buff[ $tmp ];
		$tmp_word		=~ s/\r\n/\n/g;
		$buff[ $tmp ]	= $tmp_word;
	}

	#ファイルクローズ
	close( FILEIN );

	#1行ごとのループ
	for( my $k = 0; $k < @buff; $k++ ){
		my $line	= $buff[ $k ];
		#置換処理
		for( $j = 0; $j < @GS_TO_WB_REPLACE; $j++ ){
			if( $line =~ m/$GS_TO_WB_REPLACE[$j][0]/ ){
				if( $GS_TO_WB_REPLACE[ $j ][1] eq $REPLACE_SELF ){
					print "手動で置き換えてください。 LINE= $k NAME= $GS_TO_WB_REPLACE[ $j ][0]\n";
				}else{
					$line	= &CONVERT_ReplaceName( $line, $GS_TO_WB_REPLACE[ $j ][0], $GS_TO_WB_REPLACE[ $j ][1] );
					$line .= $CONVERT_TAG;
					$CONVERT_CNT++;
				}
			}
		}
		$buff[ $k ]	= $line;
	}

	#引数置き換え
	my $ret = &CONVERT_RePlaceArgument( \@buff, \@GS_TO_WB_REPLACE );
	#print $ret;

	##コンバートした文字列を新規ファイルに作成
	#ファイルオープン
	open( FILEOUT, ">$dst_file" );
	#文字を書き込み
	print FILEOUT $ret;
	#ファイルクローズ
	close( FILEOUT );
}

#	置換処理
#	@param	_[0]	置き換えるもとの文字バッファ
#	@param	_[1]	置き換え元文字列
#	@param	_[2]	置き換え先文字列
#	@return			置き換済みの文字バッファ
sub CONVERT_ReplaceName
{
	my( $str_buff, $src_str, $dst_str )	= @_;
	$str_buff	=~	s/$src_str/$dst_str/g;
	return $str_buff;
}

#	引数を配列で取り出す処理
#	@param	_[0]	取り出す文字バッファ	リファレンス
#	@param	_[1]	コンバートテーブル		リファレンス
sub CONVERT_RePlaceArgument
{
	my( $str_buff, $convert_tbl )	= @_;
	my( @word );
	my( @temp_word );

	#	置き換え手順
	#	１．全文検索し、引数置き換えが必要な部分を検索し、保存
	#	２．置き換える
	
	$str_buff_s	= join("", @$str_buff );

	##	１．全文検索
	#単語ごとに区切り、配列に。単語ごとに処理する
	@word	= split( /( |\(|\)|:|;|,|\t|\n)/, $str_buff_s );
	for( my $i = 0; $i < @word; $i++ ){
		for( my $j = 0; $j < @$convert_tbl; $j++ ){
			#コンバートテーブルに引数置き換えが入っていて、
			#置き換え関数名が存在したらバッファに保存
			if( ($$convert_tbl[ $j ][2] != undef ) && ( $$convert_tbl[ $j ][0] eq $word[ $i ] ) ) {
				push( @temp_word, $i );
				#print "start  $word[$i]\n";
			}
		}
	}

	##	２．置換処理
	#
	foreach my $w (@temp_word ){
		$str_buff_s	= &Convert_RePlaceArgumentCore( $str_buff_s, $w, \@$convert_tbl );
	}

	return $str_buff_s;
}


#	引数を配列で取り出す処理
#	@param	_[0]	取り出す文字バッファ
#	@param	_[1]	列
#	@param	_[2]	単語
#	@param	_[3]	コンバートテーブル

sub Convert_RePlaceArgumentCore
{
	my( $str_buff, $word_num, $convert_tbl )	= @_;
	my( $word );
	my( $is_start );
	my( $brace_cnt );
	my( $argc );

	my( $is_arg_start )	= 0;
	my( $arg_start_idx );
	my( $arg_name );
	my( @arg_buff );
	my( @argument );


	#	置き換える際の手順
	#	１．置き換え関数名チェック（手動かどうか）
	#	２．「(」をみて引数開始チェック
	#	３．「,」を数えて引数の数をカウント（「(」が多重ではないとき）
	#	４．「）」をみて、引数終了チェック（「（」が多重ではないとき）
	#	５．単語の位置を置き換え。


	#単語ごとにループする
	@word	= split( /( |\(|\)|:|;|,|\t|\n)/, $str_buff );
	for( my $j = $word_num; $j < @word; $j++ ){
		$w	= $word[ $j ];
		for( $i = 0; $i < @$convert_tbl; $i++ ){
			##１．開始チェック
			#コンバートテーブルの全ての命名をチェックし
			#コンバートテーブルに引数パラメータがあるかチェック
			if( ($$convert_tbl[ $i ][0] eq $w ) &&
				($$convert_tbl[ $i ][2] != undef ) ){

				@arg_buff		= ();
				$is_start		= 1;
				#print "start\n"
			}

			if( $is_start ){
				##２．引数開始チェック
				if( $w eq "(" ){
					$brace_cnt++;	#「(」の数をふやす（2個上なら多重）

					if( $brace_cnt == 1 ){
						$is_arg_start	= 1;
						$arg_start_idx	= $j+1;
						last;
					}
				}

				##３、引数かぞえ
				#「(」をみて、多重かどうかチェックする
				if( $brace_cnt == 1 ){
					if( $w eq "," ){

						$argc++;	#「,」をみて引数の数を数える
						push( @arg_buff, $arg_name );
						$arg_name	= "";
					}
				}


				##４、引数終了チェック
				if( $w eq ")" ){
					$brace_cnt--;	#「)」の数を減らす（2個上なら多重）

					#ここで0個になれば、終了
					if( $brace_cnt == 0 ){
						$is_arg_start	= 0;
						push( @arg_buff, $arg_name );

						#指定どおり引数を置き換える
						my $cnt	= 2;
						while( $$convert_tbl[ $i ][$cnt] != undef ){
							#置き換えたものを$argumentに入れる
							$argument[ $cnt-2 ]	= $arg_buff[ $$convert_tbl[ $i ][$cnt]-1 ];
							#最後以外カンマをつける
							if( $$convert_tbl[ $i ][$cnt+1] != undef ){
								$argument[ $cnt-2 ]	.= ",";
							}

							$cnt++;
						}

						#print "@argument \n";


						#作成した引数を挿入
						splice( @word, $arg_start_idx, 0, @argument );

						goto LOOPEND;
					}
				}


				#引数保存処理
				if( $is_arg_start == 1 ){
					if( $brace_cnt == 1 ){
						if( $w ne "," ){
							$arg_name	.=	$w;
						}
					}else{
						$arg_name	.=	$w;
					}
					#print $w;
					#print $word[ $j ];
					@word	= &DeleteArray( \@word, $j );
					$j--;#配列から今参照している単語を消したので、一手戻す
				}
			}
		}
	}
LOOPEND:

	return join( "", @word );
}


#	配列から要素を消す
#	@param	配列
#	@param	インデックス
sub DeleteArray {
	my($array,$idx) = @_;
	my @new = ();
	for( my $i = 0; $i < @$array; $i++ ) {
		if($idx != $i){
			push(@new, $$array[ $i ]);
		}
	}

	return @new;
}

