#!ruby -Ks
#!/usr/bin/ruby

#===================================================================
#
# @brief  フィールドOBJアニメデータアクセステーブル生成
#
# @data   10.03.11 
# @author miyuki iwasawa
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定


$KCODE = 'SJIS'

#====================================================
# 定数宣言
#====================================================
#ヘッダファイル出力
DST_FILENAME = "demo3d_demoid.h"  #出力ファイル名
DST_BRIEF_DAT = "各アニメテーブルを統合。resource/fldmmdl/anmlist.txtから自動生成"
DST_BRIEF_HEAD = "各アニメヘッダを統合。resource/fldmmdl/anmlist.txtから自動生成"
DST_AUTOR = "miyuki iwasawa"
DST_NOTE = "このファイルはresource/fldmmdl/tool/fmmdl_anm_list.rbによって自動生成されたものです。"

arg = $*

#構造体定義

$_size_prm = Struct.new("SizeParam",\
  :size, :char_size, :file )

$_list_prm = Struct.new("ListParam",\
	:idx, :f_name, :f_path, :name, :frm_num, :sizeTbl, :comment )

#====================================================
# 主処理 開始
#====================================================

class CMakeAnmList

  def output_header
    path = @header_path
    File::open( path ,"w"){ |file|
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t#{path}");
      file.puts(" * @brief\t" + DST_BRIEF_HEAD);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");
      file.puts("#ifndef __H_FLDMMDL_ANMTBL_H__\n");
      file.puts("#define __H_FLDMMDL_ANMTBL_H__\n\n");

      for anm in @list
        file.puts "#include \"" + anm.f_name + ".h\"" 
      end
      
      file.puts("\n\n");
      file.puts("//===================================================\n");
      file.puts("//ビルボードアクター　アニメーションテーブルID定義\n");
      file.puts("// *fldmmdl_list.xls->アニメID ANMID_XXXXに対応している\n");
      file.puts("//===================================================\n\n");
    
      file.puts("typedef enum\n{\n");
      file.puts("  MMDL_BLACT_ANMTBLNO_NON,")

      for anm in @list do
        file.puts("  MMDL_BLACT_ANMTBLNO_#{anm.name.upcase},  ///<#{anm.comment}")
      end
 
      file.puts("  MMDL_BLACT_ANMTBLNO_MAX,  ///<最大");
      file.puts("}MMDL_BLACT_ANMTBLNO;\n\n");

      file.puts("#endif //__H_FLDMMDL_ANMTBL_H__\n\n");
    }
  end
  
  def output_dat
    path = @dat_path
    File::open( path ,"w"){ |file|
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t#{path}");
      file.puts(" * @brief\t" + DST_BRIEF_DAT);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");
      file.puts("#ifndef __D_FLDMMDL_ANMTBL_D__\n");
      file.puts("#define __D_FLDMMDL_ANMTBL_D__\n\n");

      for anm in @list
        file.puts "#include \"" + anm.f_name + ".cdat\"" 
      end

      file.puts("\n\n");
      #テーブル出力
#      file.puts("#if 0\n\n");
      file.puts("//===========================================\n");
      file.puts("//ビルボードアクター　アニメーションテーブル\n");
      file.puts("//===========================================\n\n");
    
      file.puts("const MMDL_BBDACT_ANMTBL DATA_MMDL_BBDACT_ANM_ListTable[MMDL_BLACT_ANMTBLNO_MAX] =")
      file.puts("{")

      file.puts("  //MMDL_BLACT_ANMTBLNO_NON")
      file.puts("  { NULL, 0 },  ///<ダミー")

      for anm in @list do
        id = anm.name.upcase
        file.puts("  //MMDL_BLACT_ANMTBLNO_#{id}")
        file.puts("  { DATA_BlAct#{anm.name}Tbl, DRAW_ANMNO_#{id}_MAX },  ///<#{anm.comment}")
      end
      file.puts("};\n\n")
#      file.puts("#endif\n\n");

      file.puts("#endif //__D_FLDMMDL_ANMTBL_D__\n\n");
    }
  end
 
  def output_anm_list
    path = @anmlist_path
    File::open( path ,"w"){ |file|
      file.puts("//NCG変換対応IMD,,");
      file.puts("ANMID,パラメータ0,パラメータ1,パラメータ2");
      file.puts("ANMID_NON,,");
      
      for anm in @list do
        buf = "ANMID_#{anm.name.upcase},"

        for i in anm.sizeTbl do
          buf += "MDLSIZE_#{i.size}:#{i.char_size}:#{i.char_size}:#{anm.frm_num}:#{i.file}.imd:#{i.file}:#{anm.f_path},"
        end
        file.puts(buf);
      end

      file.puts("#END,,\n\n");
      file.puts("パラメータ内訳：対応モデル:横キャラ数:縦キャラ数:総パターン数:対応imd名:対応ダミーncg&nclファイル名:アニメ定義ファイルcsv\n\n");
      file.puts("//fldmmdl_list_symbol.hコピペ用フィールド\n");
      
      ct = 1
      file.puts("#define ANMID_NON 0 //ダミー");
      for anm in @list do
        file.puts("#define ANMID_#{anm.name.upcase} #{ct} //#{anm.comment}");
        ct += 1
      end
      
      file.puts("\n\n")
      file.puts("//fldmmdl_list.xls <説明タブ>コピペ用フィールド\n");
     
      file.puts("ANMID_NON");
      for anm in @list do
        file.puts("ANMID_#{anm.name.upcase}");
      end
      file.puts("//ダミー");
      for anm in @list do
        file.puts("//パターン数 #{anm.frm_num} #{anm.comment}");
      end
    }
  end
  
  def output_anm_id
    path = @anmid_path
    File::open( path ,"w"){ |file|
      file.puts("#! ruby -Ks");
      file.puts("#このファイルはresource/fldmmdl/tool/fmmdl_anm_list.rbで自動生成されています");
      file.puts("\n\n");
      file.puts("  $anmid_hash = {"); 
      ct = 1
      file.puts("    \"ANMID_NON\"=>0,");
      for anm in @list do
        file.puts("    \"ANMID_#{anm.name.upcase}\"=>#{ct}, ##{anm.comment}");
        ct += 1
      end
      file.puts("  }");
    }
  end

  def line_get fp
    line = fp.gets
    line.chomp!
    return line.split(",")
  end

  def initialize arg
    @header_path = arg[0]
    @dat_path = arg[1]
    @anmlist_path = arg[2]
    @anmid_path = arg[3] 
    @list_dir = arg[4]

    size = arg.size
    arg.slice!(0..4)

    @list = Array.new
    for n in arg do
      name = File::basename(n,'.xls')
      @list << $_list_prm.new
      cp = @list.last()

      print("anm_xls -> #{name}\n")
      cp.f_name = name
      cp.f_path = @list_dir+name+".csv"
      fp = File.open(cp.f_path,"r")
      
      fp.gets
      
      work = line_get(fp)
      cp.name = work[0]
      cp.frm_num = work[1].to_i
      cp.comment = work[3]

      fp.gets
      
      cp.sizeTbl = Array.new
      for i in 0..2 do
        work = line_get(fp)
        
        case work[1]
        when "32x32"
          size = 4
        when "16x16"
          size = 2
        when "64x64"
          size = 8
        else
          next
        end
      
        cp.sizeTbl << $_size_prm.new
        sp = cp.sizeTbl.last()
        sp.size = work[1] 
        sp.char_size = size 
        sp.file = work[2]
      end

      fp.close()

    end
  end
end #end of CMakeDemoTable

def convert arg
  c_list = CMakeAnmList.new(arg)
  c_list.output_header()
  c_list.output_dat()
  c_list.output_anm_list()
  c_list.output_anm_id()
end

convert(arg)
