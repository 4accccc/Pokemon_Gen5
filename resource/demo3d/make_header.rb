#!/usr/bin/ruby
#===================================================================
#
# @brief  デモ再生アプリ
#
# @data   09.10.10
# @author genya_hosaka
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定
$KCODE = 'SJIS'
# String クラスの tr メソッドで日本語を使用可能にする。
require "jcode"

#====================================================
# 定数宣言
#====================================================
#ヘッダファイル出力
DST_FILENAME = "demo3d_resdata.cdat"  #出力ファイル名
DST_BRIEF = "コンバート環境にあるデータからデモ再生に使うテーブルを生成します。"
DST_AUTOR = "genya_hosaka"
DST_NOTE = "このファイルはdemo3d/make_header.rbによって自動生成されたものです。"
ARC_ID = "ARCID_DEMO3D_GRA"


def check_ext(file)
  ext = file.slice(/...\z/m)
  
  return ( ext == "imd" || ext == "ica" || ext == "ita" ||
           ext == "ima" || ext == "itp" || ext == "iva" );
end


def print_scene(file, dir)
  #フォルダ内のファイルを列挙
  scene_dir = Dir::entries( dir )

  #デモIDの文字列を生成
  demoid = "DEMO3D_ID_" + dir.upcase

  # sceneヘッダ
  file.puts("//=========================================================================\n");
  file.puts("// " + demoid );
  file.puts("//=========================================================================\n");

  
  #1オリジン
  cnt = 1; 
  loop do

    puts cnt

    is_find = false

    #フォルダ内のファイルを順繰り
    scene_dir.each{|i|
      #拡張子チェック
      if check_ext(i)
        # 2文字の数値チェック
        if i.slice(/\d\d/).to_i == cnt
        
          # 初回のみヘッダ書き込み
          if is_find == false
            line = "static const GFL_G3D_UTIL_RES res_unit_" + sprintf("%02d",cnt) + "[] =\n";
            file.puts( line );
            file.puts("{\n");
          end
          
          puts i
          puts 
          line = "\t{ " + ARC_ID + ", NARC_demo3d_" + i.sub(/.i/,"_nsb") + ", GFL_G3D_RESARC },";
          file.puts line

          is_find = true          
        end
      end
    }
    if is_find == false
      break
    else
      file.puts("};\n");
      cnt += 1
    end
  end
end

def print_access_table(file,dir)
  #フォルダ内のファイルを列挙
  scene_dir = Dir::entries( dir )
  #フォルダ内のファイルを順繰り
  scene_dir.each{|i|
    #拡張子チェック
    if check_ext(i)
      # 2文字の数値チェック
      if i.slice(/camera/) == "camera"
          line = "\t{ " + dir + "_setup, NELEMS(" + dir + "_setup), NARC_demo3d_" + i.sub(/.ica/,"_bin") + " },";
          file.puts line        
        end
    end
  }
  
end


#====================================================
# 主処理 開始
#====================================================

File::open( DST_FILENAME ,"w"){ |file|
    file.puts("//=========================================================================");
    file.puts("/**");
    file.puts(" * @file\t" + DST_FILENAME);
    file.puts(" * @brief\t" + DST_BRIEF);
    file.puts(" * @autor\t" + DST_AUTOR);
    file.puts(" * @note\t" + DST_NOTE);
    file.puts(" */");
    file.puts("//=========================================================================\n");
    file.puts("#pragma once\n\n");
    file.puts("#include <gflib.h>\n");
    file.puts("#include \"arc/demo3d.naix\"\n\n");
    
    #カレントにあるフォルダをしらみつぶし
    pwd =  Dir::entries( Dir::pwd )
    for i in pwd
      if File::ftype( i ) == "directory" 
        # 特殊パスは排除
        if i != "." && i != ".." && i != ".svn"
          print_scene(file,i)
        end
      end
    end

    file.puts("\n\n");

    file.puts("//=========================================================================\n");
    file.puts("// アクセステーブル" );
    file.puts("//=========================================================================\n");
    file.puts "static const DEMO3D_SETUP_DATA c_demo3d_setup_Data[ DEMO3D_ID_MAX ] = {"
    file.puts "\t{ 0, 0, 0 },"

    #カレントにあるフォルダをしらみつぶし
    pwd =  Dir::entries( Dir::pwd )
    for i in pwd
      if File::ftype( i ) == "directory" 
        # 特殊パスは排除
        if i != "." && i != ".." && i != ".svn"
          print_access_table(file,i)
        end
      end
    end
  file.puts "};"

};
