;--------------------------------------------------------------------------
; xMapファイルを解析してオーバーレイ配置情報を図示するツール
; 
; Akito Mori 2008/01/01
; 
; HSP 2.6で作成
; 実行にはhspda.dllとimgctl.hpiが必要（同じフォルダにこのファイルがあればよい）
; 
; グラフ作成した後にPNGとして出力。
; graphviz用のdotファイルも出力します。
;--------------------------------------------------------------------------
#include "llmod.as"
#include "misc.as"
#include "hspda.as"
#include "imgctl.as"
;--------------------------------------------
; 関数区間
;--------------------------------------------
#module
#deffunc PrintOverlayName str,int,int
	mref name, 32
	mref x,1
	mref y,2

;	dialog "name="+name+" x="+x+" y="+y,0

	color 255,255,255
	pos x-1,y
	print name
	pos x+1,y
	print name
	pos x,y+1
	print name
	pos x,y-1
	print name

	color 0,0,0
	pos x,y
	print name
	
	return

#global


;--------------------------------------------
;--------------------------------------------
; メイン
;--------------------------------------------
	gsel 0,-1
	cls 1
	width 150,60
	title "Overlay Map"

;	screen 0,150,60,4,,,150,60
	gsel 0,1

	pos 0,0
	objsize 150,30
	button "open",*openfile
	pos 0,30
	button "window off",*window_off

	

	sdim overlayname,64,200		;オーバーレイ領域の名前
	dim  overlayinfo,200,5		;テキスト情報から取り込んだ数値
	dim  overlaylink,200,100	;オーバーレイ開始アドレスと別オーバーレイの配置状態の関係を格納
	dim  overlaylinkcount,200	;そのオーバーレイが参照されている回数
	dim  stack,200				;
	sdim temp,  512
	sdim temp2, 256
	sdim temp3, 512
	sdim temp4, 256
	sdim mapfile, 1000000		;マップファイルの読み込みバッファ
	sdim xmapfile,15000000
	sdim dotbuf,  1000000		; Graphviz用生成バッファ
	sdim filename, 512

	OVERLAY_NAME_SIZE = 40
	IMG_WIDTH         = 6000
	IMG_HEIGHT        = 1000

	screen 2,IMG_WIDTH,IMG_HEIGHT,2,,,1024,900
	font "Arial",10
	
	gsel 0

	;----------------------------------------------
	; 引数が指定されていたら
	;----------------------------------------------
	filename = cmdline
	automode = 0
	strlen cmdline_length,filename
	if cmdline_length!=0 {
		automode=1
		exist filename
		if strsize=-1{
			dialog ""+filename+"は存在しません",0
			end
		}
		goto *readfile
	}
;--------------------------------------------
;　ループ
;--------------------------------------------
*main
	wait 30
	if automode=1{
;		dialog "終了",0
		end
	}
	goto main



;--------------------------------------------
;　リンク計算
;--------------------------------------------
*linkcalc
	linkcount = 0
	repeat lines
		linkp = cnt
		repeat lines

		; モジュールのお尻が別なモジュールのトップになっているか？
		
		if (overlayinfo.linkp.4 = overlayinfo.cnt.0) & (linkp!=cnt) {
			p = overlaylinkcount.linkp
			overlaylink.linkp.p = cnt
			overlaylinkcount.linkp+
			linkcount+
		}
		loop
	loop
	

	return


;--------------------------------------------
;　リンク状態テストプリント
;--------------------------------------------
*linkprint
	gsel 2,-1
	printpos = 0
	repeat lines
		if overlaylinkcount.cnt != 0{
			pos 100, printpos*11
			color 0,0,0
			print ""+cnt+": "+overlayname.cnt+" linkcount="+overlaylinkcount.cnt
			printpos+
		}
	loop

	font "ＭＳｺﾞｼｯｸ",20
	gsel 2
	pos 400,10
	print "リンクが発生した回数="+linkcount
	font "Arial",10

	return

;--------------------------------------------
;　再起リンク状態カウント取得
;--------------------------------------------
*linkcountget
	searchtop = 0
	stackmax  = 0
	stacknow = 0
	searchcount = 0
	searchnow   = searchtop
	repeat
;		dialog "searchnow="+searchnow+" count="+overlaylinkcount.searchnow+" name="+overlayname.searchnow,0
		if overlaylinkcount.searchnow != 0 {
			repeat overlaylinkcount.searchnow
				stack.stackmax = overlaylink.searchnow.cnt
				stackmax+
			loop
		}
 		else{
			if stacknow=stackmax{
				break
			}
		}
		searchnow = stack.stacknow
		stacknow+
	loop

;	dialog "stack="+stackmax,0

	return

;--------------------------------------------
;　ＭＡＰ情報ソート
;--------------------------------------------
*sortinfo
	repeat lines-1
		src = cnt+1
		repeat lines-1
			des = cnt+1
			if overlayinfo.src.0 < overlayinfo.des.0{
				t0 = overlayinfo.des.0
				t1 = overlayinfo.des.1
				t2 = overlayinfo.des.2
				t3 = overlayinfo.des.3
				t4 = overlayinfo.des.4
				temp = overlayname.des

				overlayinfo.des.0 = overlayinfo.src.0
				overlayinfo.des.1 = overlayinfo.src.1
				overlayinfo.des.2 = overlayinfo.src.2
				overlayinfo.des.3 = overlayinfo.src.3
				overlayinfo.des.4 = overlayinfo.src.4
				overlayname.des   = overlayname.src

				overlayinfo.src.0 = t0
				overlayinfo.src.1 = t1
				overlayinfo.src.2 = t2
				overlayinfo.src.3 = t3
				overlayinfo.src.4 = t4
				overlayname.src   = temp
			}
		loop
	loop
	return

;--------------------------------------------
;　ワーク初期化
;--------------------------------------------
*initwork

	;参照情報初期化
	repeat 100
		linkp = cnt
		repeat 100
			overlaylink.linkp.cnt = -1
		loop
	loop

	

	return

;--------------------------------------------
; mapファイルからデータ取得
;--------------------------------------------
*mapfile2addr

	notemax lines

	infono=0

	;２行同時に取得し、同じオーバーレイ領域を指している場合は格納する
	;また、先頭アドレスが00000000の場合も除外する
	readline = startline+1
	repeat 
		readline = readline + 1

		;1行目の取得
		noteget temp,readline

		;2行目の取得
		noteget temp3,(readline+1)

		; 取得した行に文章がなかったらオーバーレイ区間終了とみなす
		strlen length,temp3
		if length < 5 {
			break
		}

		; 2行分のオーバーレイ名を取得
		strmid temp2, temp,29,OVERLAY_NAME_SIZE
		strmid temp4, temp3,29,OVERLAY_NAME_SIZE
		
		;一致するか比較する
		strlen length1, temp2
		strlen length2, temp4
		if length1=length2 {
			instr flag, temp2,temp4,0
			if flag=-1{
				continue
			}else{
;				dialog ""+readline+"行目と次の行は一致しました["+temp2+"]",0
			}
		}else{
			continue
		}
		
		;一致してもアドレスが00000000の場合は飛ばす
		strmid temp2,temp,2,8
		strtoint temp2,16
		if stat=0 {
			continue
		}

		

		;ここまで通ってきたら格納する
		strmid temp2, temp,2,8		;スタート位置取得
		strtoint temp2,16
		overlayinfo.infono.0 = stat

		strmid temp2, temp,20,8		;サイズ取得
		strtoint temp2,16
		overlayinfo.infono.1 = stat

		strmid temp2, temp3,2,8		;BSSスタート位置取得
		strtoint temp2,16
		overlayinfo.infono.2 = stat

		strmid temp2, temp3,20,8	;BSSサイズ取得
		strtoint temp2,16
		overlayinfo.infono.3 = stat

		;総サイズ取得
		overlayinfo.infono.4 = overlayinfo.infono.0+overlayinfo.infono.1+overlayinfo.infono.3

		;オーバーレイ名取得
		strmid temp2, temp, 29,OVERLAY_NAME_SIZE
		overlayname.infono = temp2
		
		infono+
	loop

	lines = infono

	return



;--------------------------------------------
; mapファイルからデータ取得してBMPに描画
;--------------------------------------------
*openfile
	if openfileflag=1{
		dialog "すんません。2回は読み込めまへん",0
		goto *main
	}

	openfileflag =1

	dialog "xMap",16,"mapファイル"
	if stat=0 : goto *main
	exist refstr
	if stat=0 : goto *main

	filename = refstr
*readfile
	bload filename, xmapfile
;	dialog "ファイルサイズ"+strsize,0

	count=0

	notesel xmapfile
	notemax lines

;	dialog "行数サイズ"+lines,0
	hitflag = 0

	repeat lines
		noteget temp,lines-cnt-1

		strlen length,temp
		if length>10{
			instr flag, temp, "# Memory map",0
			if flag!=-1{
				startline = lines-cnt-1
				hitflag = 1
;				dialog "「Memory map」が"+startline+"でみつかったよ",0
				break

			}
			count = count + 1
			if count=500{
				break
			}
		}
	loop
	if hitflag=0{
		dialog "下から500行探したけどみつからなかったよ",0
		goto *main
	}

	;--------ファイル読み込み-------------
;	bload "map.txt",mapfile
;	notesel mapfile
	


	; ---- リンク先情報初期化 ----
	gosub initwork


	;-----mapfileからオーバーレイのアドレス・サイズ取得	-------
	gosub mapfile2addr

	;-----アドレスの順でソート-----------
	gosub sortinfo

	;-----リンク参照を検索----------
	gosub linkcalc

	;-----結果表示----------
	gosub linkprint

	;-----被リンク回数をカウント----------
	gosub linkcountget


	;--------------------------------------------
	;仮表示
	;--------------------------------------------

;	repeat lines
;		pos 5, cnt*11
;		print overlayname.cnt +": start="+overlayinfo.cnt.0+" size="+overlayinfo.cnt.1+" bss="+overlayinfo.cnt.2+" bsssize="+overlayinfo.cnt.3+" end="+overlayinfo.cnt.4
;		calcresult = overlayinfo.cnt.4-33554432
;		size = overlayinfo.cnt.4 - overlayinfo.cnt.0
;		print overlayname.cnt +": end="+calcresult+" size="+size
;	loop

;	dialog "stop",0

	gWidth  = IMG_WIDTH
	gHeight = IMG_HEIGHT
;	DIV_RATE = 540704	;fieldmapが一番大きくみえる
	DIV_RATE = 850000
;	DIV_RATE = 2040704	;mainが入る

;	DIV_RATE  = 3040704	;プログラムの最後まではいる
	DIV_NUM   = 150
	BAR_WIDTH = (gWidth/DIV_NUM)

	sizemax=0

	

	;--------------------------------------------
	;グラフ描画
	;--------------------------------------------

	repeat lines
		size = overlayinfo.cnt.4 - overlayinfo.cnt.0
		if size>sizemax {
			sizemax = size
		}
	loop

	pos 200,40
	print "sizemax="+sizemax

	;---------横枠線描画--------------
	repeat 20
		color 200,200,200
		y1 = gHeight - (((cnt*(65536))*gHeight)/DIV_RATE) - 11
		line 0,y1,gWidth,y1
		pos  5,y1,

		;横枠線の上に0x10000ずつ数値を書き込む
		color 0,0,0
		hexstr = cnt*65536
		str hexstr,22
		print "0x"+hexstr
	loop

	repeat lines
		size = overlayinfo.cnt.4 - overlayinfo.cnt.0
		top  = overlayinfo.cnt.0 - overlayinfo.0.4
		x1 = (gWidth*cnt)/DIV_NUM
		ys = (top*gHeight/DIV_RATE)
		y1 = gHeight - (((size*gHeight/DIV_RATE)+ys)  ) - 11
		y2 = gHeight - ys -11

		rnd r1,256
		rnd g1,256
		rnd b1,256
		
		color r1,g1,b1
		boxf x1,y1,(x1+BAR_WIDTH),y2
		line x1,y1,x1,0

;		if cnt<30{
;			dialog "size="+size+" top="+top+" x1="+x1+" y1="+y1+" y2="+y2+" main="+overlayinfo.0.0+" ntop="+overlayinfo.cnt.0,0
;		}

	loop	


	;--------------------------------------------
	;名前表示
	;--------------------------------------------
	diffcount = 0
	repeat lines
		size = overlayinfo.cnt.4 - overlayinfo.cnt.0
		top  = overlayinfo.cnt.0 - overlayinfo.0.4
		diff = size-nsize
		if diff < 0 : diff = diff*-1
		if diff > (DIV_RATE/20) : diffcount=0

		x1 = (gWidth*cnt)/DIV_NUM
		ys = (top*gHeight/DIV_RATE)
		y1 = gHeight - (((size*gHeight/DIV_RATE)+ys)  ) - 11 -(diffcount*15)

		PrintOverlayName overlayname.cnt, x1, y1

		diffcount+
		if diffcount=10 : diffcount=0

		nsize = size
	loop

;	dialog "lines="+lines

	;------------------------------------------------
	;描画したグラフをpngで出力
	;------------------------------------------------
;	bmpsave "overlay_graph.bmp"
	ic_DCtoDIB 2, 0, 0, IMG_WIDTH, IMG_HEIGHT, hdib
	if (stat != 0) { goto *main }	// エラー

	ic_DIBtoPNG "overlay_graph.png", hdib

	; DIBデータ解放
	ic_DeleteDIB hdib


	;------------------------------------------------
	;計算したリンク情報をGraphviz用に整形して出力
	;------------------------------------------------
	notesel dotbuf
	noteadd "digraph G {",-1
	noteadd "\tgraph [rankdir = RL]\;",-1
	noteadd "\tnode [shape = box]\;",-1

	;オーバーレイの個数分まわす
	repeat lines
		nowpoint = cnt

		;リンクされている回数分まわす
		repeat overlaylinkcount.nowpoint
			link = overlaylink.nowpoint.cnt
;			dialog ""+overlayname.link+"->"+overlayname.nowpoint,0
			noteadd "\t"+overlayname.link+"->"+overlayname.nowpoint+";", -1
		loop
	loop

	noteadd "}",-1
	notesave "overlaymap.dot"

	gsel 2,1

	;----------おわり-------------------
	goto *main

;--------------------------------------------
; overlay解析結果ウインドウOFF
;--------------------------------------------
*window_off
	gsel 2,-1
	goto *main


