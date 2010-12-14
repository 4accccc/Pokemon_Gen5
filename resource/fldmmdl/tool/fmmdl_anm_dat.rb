#!ruby -Ks
#!/usr/bin/ruby

#===================================================================
#
# @brief  フィールドOBJアニメテーブルデータ生成
#
# @data   10.12.09 
# @author iwasawa
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定
$KCODE = 'SJIS'
# String クラスの tr メソッドで日本語を使用可能にする。
require "jcode"

arg = $*
$input_path = arg[0]
$output_path = arg[1]

#====================================================
# 定数宣言
#====================================================
#ヘッダファイル出力
DST_BRIEF = "コンバート環境にあるデータからアニメパターン定義テーブルを生成します。"
DST_AUTOR = "miyuki iwasawa"
DST_NOTE = "このファイルはfldmmdl/tool/make_anmtbl_cdat.rbによって自動生成されたものです。"
STR_NARC_PRE = "NARC_data_demo3d_"
STR_COMMENT_LINE = "//=========================================================================\n"

CMD_PARAM_MAX  = 8
CMD_PARAM_END = CMD_PARAM_MAX-1

#データテーブル
$onoff_list = ["OFF","ON"]
$frame_rate_list = ["60fps","30fps"]
$frame_rate_time = ["60","30"]
$timezone_list = ["TIMEZONE_NONE","TIMEZONE_MORNING","TIMEZONE_NOON","TIMEZONE_EVENING","TIMEZONE_NIGHT","TIMEZONE_MIDNIGHT"]
$timezone_suffix = ["","_morning","_noon","_evening","_night","_midnight"]

$nsx_list = ["nsbtp","nsbta","nsbca","nsbma","nsbva"]

#構造体定義
$_frm_prm = Struct.new("FrameParam",\
	:idx, :h_flip, :v_flip, :frame, :com )

$_pattern_prm = Struct.new("PatternParam",\
  :name, :frm_tbl, :com, :frm_num, :parent, :jump )

#-----------------------------------------------
# anmtbl リソースデータcsv
#-----------------------------------------------
class CFMMdlAnmRes 
 
  #出力メイン
  def output dir
    output_cdat(dir+@org_name+".cdat")
    output_header(dir+@org_name+".h")
  end

  #データ出力
  def output_cdat fpath
	  fp = File.open(fpath,"w")
    
    fp.puts("//=========================================================================");
    fp.puts("/**");
    fp.puts(" * @file\t#{@org_name}.cdat");
    fp.puts(" * @brief\tANMID_#{@anm_id} -> #{@comment}");
    fp.puts(" * @autor\t" + DST_AUTOR);
    fp.puts(" * @note\t" + DST_NOTE);
    fp.puts(" * ");
    fp.puts(" * original = #{@org_name}.xls  pattern_num = #{@frame_max}");
    fp.puts(" */");
    fp.puts("//=========================================================================\n\n");

    #パターンテーブル出力
    for tbl in @anm_tbl do
      output_anmtbl( fp, tbl )
    end

    output_table( fp )
	  fp.close()
  end

  #パターン出力
  def output_anmtbl fp,tbl
    if tbl.parent == true then
      return
    end

    fp.puts("//============================================\n");
    fp.puts("// #{tbl.com}")
    fp.puts("//============================================\n");
    
    fp.puts("static const GFL_BBDACT_ANM DATA_BlAct#{@anm_name}_#{tbl.name}[] = {")
    
    for frm in tbl.frm_tbl do
      buf = "  { %2d, ANM#{frm.h_flip}, ANM#{frm.v_flip}, %2d },  ///<#{frm.com}" % [frm.idx,frm.frame]
      fp.puts(buf)
    end

    if tbl.jump < 0 then
      fp.puts("  { GFL_BBDACT_ANMCOM_END,0,0,0 },")
    else
      fp.puts("  { GFL_BBDACT_ANMCOM_JMP,0,0, #{tbl.jump} },")
    end

    fp.puts("};\n\n")
  end

  #テーブル出力
  def output_table fp
    fp.puts("static const GFL_BBDACT_ANM * const DATA_BlAct#{@anm_name}Tbl[DRAW_ANMNO_#{@anm_id}_MAX] = ")
    fp.puts("{")

    for anm in @anm_tbl do
      if anm.parent == true then
        fp.puts("  DATA_BlAct#{@parent_name}_#{anm.name},")
      else
        fp.puts("  DATA_BlAct#{@anm_name}_#{anm.name},")
      end
    end

    fp.puts("};\n\n")
  end
 
  #ヘッダ出力
  def output_header fpath
	  fp = File.open(fpath,"w")
    
    fp.puts("//=========================================================================");
    fp.puts("/**");
    fp.puts(" * @file\t#{@org_name}.h");
    fp.puts(" * @brief\tANMID_#{@anm_id} -> #{@comment} 用アニメパターン定義リテラル");
    fp.puts(" * @autor\t" + DST_AUTOR);
    fp.puts(" * @note\t" + DST_NOTE);
    fp.puts(" * ");
    fp.puts(" * original = #{@org_name}.xls  pattern_num = #{@frame_max}");
    fp.puts(" */");
    fp.puts("//=========================================================================\n\n");

    fp.puts("#ifndef __H_#{@org_name.upcase}_H__")
    fp.puts("#define __H_#{@org_name.upcase}_H__\n\n")
    
    fp.puts("///ANM_NO定義")
    ct = 0
    for anm in @anm_tbl do
      if anm.parent == true then
        #fp.puts("#define DRAW_ANMNO_#{@anm_id}_#{anm.name.upcase}#{ct}  ( #{ct} )\n")
      else
        fp.puts("#define DRAW_ANMNO_#{@anm_id}_#{anm.name.upcase}  ( #{ct} )\n")
      end
      ct += 1
    end
    fp.puts("#define DRAW_ANMNO_#{@anm_id}_MAX  ( #{@anm_tbl.length} )\n")
    
    fp.puts("\n\n")
    fp.puts("///フレーム数定義")
    ct = 0
    for anm in @anm_tbl do
      if anm.parent == true then
        next
#        fp.puts("#define FMMDL_ANMFRM_NUM_#{@anm_id}_#{anm.name.upcase}#{ct}  ( #{anm.frm_num} )\n")
      else
        fp.puts("#define DRAW_ANMFRM_#{@anm_id}_#{anm.name.upcase}  ( #{anm.frm_num} )\n")
      end
      ct += 1
    end

    fp.puts("\n\n#endif //__H_#{@org_name.upcase}_H__")

	  fp.close()
  end
  
  #ライン取得
  def get_line fp
    line = fp.gets

    if line == nil then
      return nil
    end
    line.chomp!

    work = line.split(",")
    return work
  end

  #パターン名パース
  def parse_pattern fp
    while line = fp.gets
      line.chomp!
      
      work = line.split(",")
      if /#END/ =~ work[0] then
        return nil
      end
      
      if /#[a-zA-Z0-9_]+/ =~ work[0] then
        @anm_tbl << $_pattern_prm.new 
        cp = @anm_tbl.last()
        cp.name = work[0].sub("#","")
        cp.com = work[5]
        cp.frm_num = 0
        cp.parent = true
        return cp
      end

      if /[a-zA-Z0-9_]+/ =~ work[0] then
        @anm_tbl << $_pattern_prm.new 
        cp = @anm_tbl.last()
        cp.frm_tbl = Array.new
        cp.name = work[0]
        cp.com = work[5]
        cp.frm_num = 0
        cp.parent = false
        cp.jump = -1
        return cp
      end
    end
    return nil
  end

  #パターン取得
  def get_anime fp, pat
    while line = fp.gets
      line.chomp!
      work = line.split(",")

      if /#END/ =~ work[0] then
        return false
      end

      if /[0-9]+/ =~ work[1] then

        if work[2] == "JUMP" then
          pat.jump = work[4].to_i
          return true
        end

        pat.frm_tbl << $_frm_prm.new
        cp = pat.frm_tbl.last()

        cp.idx = work[1].to_i
        if cp.idx >= @frame_max then
          print("ERROR! #{@pat.name} のパターンNo #{cp.idx} が不正です max = #{@frame_max-1}\n")
          exit 1
        end

        cp.h_flip = work[2]
        cp.v_flip = work[3]
        cp.frame = work[4].to_i
        cp.com = work[5]
        pat.frm_num += cp.frame
        next    
      end
      return true
    end
    return false
  end

  #初期化、オブジェクト作成
  def initialize fpath
    @base_name = ""
    @org_name = ""
    @anm_id = ""
    @anm_name = ""
    @parent_name = ""
    @comment = ""
    @frame_max = 0

    @anm_tbl = Array.new

    @org_name = File::basename(fpath,'.csv')
    @base_name = @org_name.sub("fmmdl_anm_", "")
    
    print("\n#コンバートスタート #{fpath}\n")
    fp = File.open(fpath,"r")
    if fp == nil then
      exit 1
    end

    #一行読み飛ばし
    fp.gets

    #ヘッダ取得
    work = get_line( fp )
    @anm_name = work[0]
    @anm_id = @anm_name.upcase
    @frame_max = work[1].to_i
    @parent_name = work[2]
    @comment = work[3]

    print("#シーン " + @base_name + " ->" + @anm_name  + "コンバート開始\n\n")

    #不要行読み飛ばし
    fp.gets
    fp.gets
    fp.gets
    fp.gets
    fp.gets
    fp.gets

    #アニメパラメータ取得開始
    while 1 do
      #パターン名サーチ
      pat = parse_pattern(fp)
      if pat == nil then
        break
      end
      #パターン取得
      ret = get_anime(fp,pat) 
      if ret == false then
        break
      end
    end

    if @anm_tbl.length == 0 then
      print("ERROR! アニメデータがありません\n")
      exit 1
    end
  end
end
#end class CDemo3DRes 

def convert
  c_anm = CFMMdlAnmRes.new($input_path)
  print("output = #{$output_path}\n")
  c_anm.output($output_path)
end

convert
