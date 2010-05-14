■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■                                                                          ■
■  Readme-TwlSDK-5_3_patch5-20091109.txt                                   ■
■                                                                          ■
■  Plus Patch for TWL-SDK 5.3                                              ■
■                                                                          ■
■  Nov. 09, 2009                                                           ■
■                                                                          ■
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■


【 はじめに 】

    このパッチは、TWL-SDK 5.3 正式版 のリリース以降に発見された不具合を修正す
    るためのものです。インストールするためには、同梱されているファイルを
    TWL-SDK 5.3 正式版 (090807)がインストールされているディレクトリに上書きコ
    ピーしてください。


【 注意 】

    TwlSDK のライブラリをビルドした事がある場合、本パッチを当てただけだとキャ
    ッシュの影響でプログラムをビルドするときにエラーが発生する可能性があります。
    もしビルド時にエラーが発生した場合は、TwlSDK のルートディレクトリで
    「make clobber」を実行することでキャッシュが削除されます。
    その後、同じ TwlSDK のルートディレクトリで「make」することで、この症状は改
    善されます。


【 ファイルリスト 】
■ソースファイル
	/TwlSDK/build/libraries/fx/common/src/fx_vec.c
	/TwlSDK/build/libraries/init/ARM9.TWL/src/crt0.FLX.c
	/TwlSDK/build/libraries/os/ARM9/src/os_china.c
	/TwlSDK/build/libraries/os/common/src/os_init.c

■ヘッダファイル
	/TwlSDK/include/nitro/version.h
	/TwlSDK/include/twl/version.h

■ライブラリファイル
	/TwlSDK/dspcomponents/audio/audio.a
	/TwlSDK/dspcomponents/graphics/graphics.a
	/TwlSDK/lib/ARM9-TS/Debug/crt0.HYB.TWL.o
	/TwlSDK/lib/ARM9-TS/Debug/crt0.LTD.TWL.o
	/TwlSDK/lib/ARM9-TS/Debug/crt0.o
	/TwlSDK/lib/ARM9-TS/Debug/libdsp_g711.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp_g711.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp_g711.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp_g711.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp_graphics.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp_graphics.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp_graphics.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Debug/libdsp_graphics.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libfx.a
	/TwlSDK/lib/ARM9-TS/Debug/libfx.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libfx.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Debug/libfx.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libfx.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Debug/libfx.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Debug/libos.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libtcl.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Debug/libtcl.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Debug/libtcl.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Debug/libtcl.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/crt0.HYB.TWL.o
	/TwlSDK/lib/ARM9-TS/Release/crt0.LTD.TWL.o
	/TwlSDK/lib/ARM9-TS/Release/crt0.o
	/TwlSDK/lib/ARM9-TS/Release/libdsp_g711.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp_g711.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp_g711.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp_g711.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp_graphics.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp_graphics.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp_graphics.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Release/libdsp_graphics.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libfx.a
	/TwlSDK/lib/ARM9-TS/Release/libfx.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libfx.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Release/libfx.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libfx.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Release/libfx.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libos.a
	/TwlSDK/lib/ARM9-TS/Release/libos.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libos.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Release/libos.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libos.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Release/libos.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libtcl.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Release/libtcl.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Release/libtcl.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Release/libtcl.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/crt0.HYB.TWL.o
	/TwlSDK/lib/ARM9-TS/Rom/crt0.LTD.TWL.o
	/TwlSDK/lib/ARM9-TS/Rom/crt0.o
	/TwlSDK/lib/ARM9-TS/Rom/libdsp_g711.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp_g711.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp_g711.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp_g711.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp_graphics.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp_graphics.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp_graphics.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Rom/libdsp_graphics.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libfx.a
	/TwlSDK/lib/ARM9-TS/Rom/libfx.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libfx.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Rom/libfx.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libfx.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Rom/libfx.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Rom/libos.TWL.LTD.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libtcl.TWL.HYB.a
	/TwlSDK/lib/ARM9-TS/Rom/libtcl.TWL.HYB.thumb.a
	/TwlSDK/lib/ARM9-TS/Rom/libtcl.TWL.LTD.a
	/TwlSDK/lib/ARM9-TS/Rom/libtcl.TWL.LTD.thumb.a

■コンポーネントファイル
	/TwlSDK/components/ichneumon/ARM7-TS/Debug/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS/Release/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS/Rom/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS.thumb/Debug/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS.thumb/Release/ichneumon_sub.nef
	/TwlSDK/components/ichneumon/ARM7-TS.thumb/Rom/ichneumon_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS/Debug/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS/Release/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS/Rom/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS.thumb/Debug/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS.thumb/Release/mongoose_sub.nef
	/TwlSDK/components/mongoose/ARM7-TS.thumb/Rom/mongoose_sub.nef
	/TwlSDK/components/ferret/ARM7-TS.LTD/Debug/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD/Release/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD/Rom/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Debug/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Release/ferret.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Rom/ferret.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Debug/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Release/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Rom/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Debug/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Release/ichneumon.tef
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Rom/ichneumon.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Debug/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Release/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Rom/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Debug/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Release/mongoose.tef
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Rom/mongoose.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD/Debug/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD/Release/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD/Rom/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Debug/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Release/racoon.tef
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Rom/racoon.tef
	/TwlSDK/components/ferret/ARM7-TS.LTD/Debug/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD/Release/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD/Rom/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Debug/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Release/ferret.TWL.FLX.sbin
	/TwlSDK/components/ferret/ARM7-TS.LTD.thumb/Rom/ferret.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Debug/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Release/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB/Rom/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Debug/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Release/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/ichneumon/ARM7-TS.HYB.thumb/Rom/ichneumon.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Debug/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Release/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB/Rom/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Debug/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Release/mongoose.TWL.FLX.sbin
	/TwlSDK/components/mongoose/ARM7-TS.HYB.thumb/Rom/mongoose.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Debug/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Release/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD/Rom/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Debug/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Release/racoon.TWL.FLX.sbin
	/TwlSDK/components/racoon/ARM7-TS.LTD.thumb/Rom/racoon.TWL.FLX.sbin

■ドキュメント・その他
	/TwlSDK/bin/ARM9-TS/Rom/NandFiler.srl
	/TwlSDK/bin/ARM9-TS/Rom/NandFiler.tad
	/TwlSDK/bin/ARM9-TS/Rom/TwlNmenu.srl
	/TwlSDK/bin/ARM9-TS/Rom/TwlNmenu.tad
	/TwlSDK/build/demos/os/Makefile
	/TwlSDK/man/ja_JP/a-z.html
	/TwlSDK/man/ja_JP/demos/os/osdemos.html
	/TwlSDK/man/ja_JP/dsp/common/DSP_PlayShutterSound.html
	/TwlSDK/man/ja_JP/dsp/common/DSP_PlaySound.html
	/TwlSDK/man/ja_JP/fx/vec/VEC_Fx16Normalize.html
	/TwlSDK/man/ja_JP/fx/vec/VEC_Normalize.html
	/TwlSDK/man/ja_JP/os/list_os.html
	/TwlSDK/man/ja_JP/os/os.html
	/TwlSDK/man/ja_JP/tools/makerom.html
	/TwlSDK/tools/bin/makerom.exe
	/TwlSDK/tools/bin/makerom.TWL.exe


以上
