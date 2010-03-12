#!ruby -Ks
#!/usr/bin/ruby

#===================================================================
#
# @brief  デモ再生アプリ用デモID/データアクセステーブル生成
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
DST_BRIEF_ID = "デモ再生アプリで使うデモID。resource/demo3d/demo_list.txtから自動生成"
DST_BRIEF_RES = "デモ再生アプリで使うリソースへのアクセステーブル。resource/demo3d/demo_list.txtから自動生成"
DST_AUTOR = "miyuki iwasawa"
DST_NOTE = "このファイルはdemo3d/conv/make_demoid.rbによって自動生成されたものです。"

arg = $*

#====================================================
# 主処理 開始
#====================================================

class CMakeDemoTable

  def output_demoid
    path = @id_path
    File::open( path ,"w"){ |file|
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t#{}");
      file.puts(" * @brief\t" + DST_BRIEF_ID);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");
      file.puts("#ifndef __DEMO3D_DEMOID_H__\n");
      file.puts("#define __DEMO3D_DEMOID_H__\n\n");

      file.puts "#define DEMO3D_ID_NULL (0)\n\n"

      count = 1
      for i in @list
        file.puts "#define DEMO3D_ID_" + i.upcase + " (" + count.to_s + ")" 
        count += 1
      end
    
      file.puts "\n#define DEMO3D_ID_MAX (" + count.to_s + ")\n\n"
      file.puts("#endif // __DEMO3D_DEMOID_H__");
    }
  end
  
  def output_demo_res
    path = @res_path
    File::open( path ,"w"){ |file|
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t" + path);
      file.puts(" * @brief\t" + DST_BRIEF_RES);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");

      for n in @list
        file.puts("#include \"#{n}.cdat\"")
      end

      count = 1
      file.puts("\n\nstatic const DEMO3D_SETUP_DATA c_demo3d_setup_data[ DEMO3D_ID_MAX ] = {")
      file.puts(" { NULL, NULL, NULL, NULL, 0 },")
      for n in @list
        scene = "#{n}_scene"
        res = "res_#{scene}_unit"

        file.puts(" { &data_#{scene}, demo3d_cmd_#{n}_data, demo3d_cmd_#{n}_end_data, #{res}, NELEMS(#{res}) },")
        count += 1
      end
      file.puts("};\n\n")
    }
  end

  def initialize arg
    @id_path = arg[0]
    @res_path = arg[1]

    size = arg.size
    arg.slice!(0..1)

    @list = Array.new
    for n in arg do
      name = File::basename(n,'.xls')
      @list << name
      print("demo_id -> #{name}\n")
    end
  end
end #end of CMakeDemoTable

def convert arg
  c_demo = CMakeDemoTable.new(arg)
  c_demo.output_demoid()
  c_demo.output_demo_res()
end

convert(arg)
