#!ruby -Ks
# Nitro中間ファイル 16charオーバーの文字列を検出する

$KCODE = "SJIS" #文字コードをSJISに設定
require 'rexml/document'
require 'uconv'
require "find"

arg = $*
$input_path = arg[0]

$ext_list = [".imd",".ita",".itp",".ima"]

$h_anmtga = {
  "tex_image"=>1,
  "image_name"=>1,
  "tex_palette"=>2,
  "palette_name"=>2,
}

$h_search = {
  "tex_image"=>"name", \
  "tex_palette"=>"name", \
  "material"=>"name", \
  "polygon"=>"name", \
  "node"=>"name", \
  "tex_srt_anm"=>"material_name", \
  "image_name"=>"name", \
  "palette_name"=>"name", \
  "tex_pattern_anm"=>"material_name", \
  "mat_color_anm"=>"material_name", \
}


class C_NIX_CHECK

	def initialize()
    @err_msg = Array.new
	end
  
  def CheckMultiByte( elem_name, name, str, mode )
    sub_str = str.gsub(/[0-9A-Za-z_]+/, "")
    if sub_str.length() == 0 then
      return 0  #英数字+_のみで構成されていればOK
    end

    #英数字+_のみ許可
    if mode == 0 then
      @err_msg << (" +0 #{elem_name}->#{name} = \"#{str}\" char code error \"#{sub_str}\"")
      return 1
    end

    #テクスチャ名 英数字+_に加えて、アニメtgaのピリオド書式なら可
    if mode == 1 then
      sub_str = str.sub(/[0-9A-Za-z_]+\.[0-9]+/, "")
      if sub_str.length() == 0 then
        return 0
      end
      err_str = str.gsub(/[0-9A-Za-z_\.]+/, "")
      if err_str.length() > 0 then
        @err_msg << (" +1 #{elem_name}->#{name} = \"#{str}\" char code error \"#{err_str}\"")
      else
        @err_msg << (" +1 #{elem_name}->#{name} = \"#{str}\" str format error \"#{sub_str}\"")
      end
      return 1
    end
    #パレット名 英数字+_に加えて、アニメtgaのピリオド書式なら可
    if mode == 2 then
      sub_str = str.sub(/[0-9A-Za-z_]+\.[0-9]+_pl/, "")
      if sub_str.length() == 0 then
        return 0
      end
      err_str = str.gsub(/[0-9A-Za-z_\.]+/, "")
      if err_str.length() > 0 then
        @err_msg << (" +2 #{elem_name}->#{name} = \"#{str}\" char code error \"#{err_str}\"")
      else
        @err_msg << (" +2 #{elem_name}->#{name} = \"#{str}\" str format error \"#{sub_str}\"")
      end
      return 1
    end

    #パス名かどうか
    dir_path = File::dirname(str)
    f_name = File::basename(str)

    if dir_path != "." then
      sub_str = dir_path.gsub(/[0-9A-Za-z:_\/]+/, "")
      if sub_str.length() > 0 then
        @err_msg << (" + #{elem_name}->#{name} = \"#{str}\" char code error \"#{sub_str}\"")
        return 1
      end
    end

    sub_str = f_name.sub(/[0-9A-Za-z_]+\.[0-9]+\.[0-9A-Za-z]{3}/, "")
    if sub_str.length() > 0 then
      sub_str = f_name.sub(/[0-9A-Za-z_]+\.[0-9A-Za-z]{3}/, "")
      if sub_str.length() == 0 then
        return 0
      end
      err_str = f_name.gsub(/[0-9A-Za-z_\.]+/, "")
      if err_str.length() > 0 then
        @err_msg << (" + #{elem_name}->#{name} = \"#{str}\" char code error \"#{err_str}\"")
      else
        @err_msg << (" + #{elem_name}->#{name} = \"#{str}\" str format error \"#{sub_str}\"")
      end
      return 1
    end
  end

  def Check16CharOver( elem_name, name, str, mode )
    if CheckMultiByte( elem_name, name, str, mode ) == 1 then
      return 1
    end
    if str.length() <= 16 then return 0 end

    @err_msg << (" + #{elem_name}->#{name} = \"#{str}\" 16char over\n")
    return 1
  end


	def CheckWarningCore(elem)
    elem_name = Uconv.u8tosjis elem.name
    attrs = elem.attributes
  
    #mayaバイナリ名保存
    if elem_name == "create" then
      if attrs["user"] != nil 
        @user = Uconv.u8tosjis attrs["user"]
      end
      if attrs["date"] != nil 
        @date = Uconv.u8tosjis attrs["date"]
      end
      if attrs["source"] != nil 
        @source = Uconv.u8tosjis attrs["source"]
      end
    elsif elem_name == "original_create" && @source == "unknown" then
      @source = Uconv.u8tosjis attrs["source"]
    end
    
    val = $h_search.fetch(elem_name,nil)
    if val != nil then
      val_name = Uconv.u8tosjis attrs[val]

      mode = $h_anmtga.fetch(elem_name,0)
      Check16CharOver(elem_name, val, val_name, mode )

      if elem_name == "tex_image" then
        path_name = Uconv.u8tosjis attrs["path"]
        CheckMultiByte( "#{elem_name} #{val_name}", "path", path_name, 3 )
      end
    end

		if elem.has_elements? then
			elem.each_element{|e|
			  CheckWarningCore(e)
			}
		end
	end

	def NixCheckWarning( inFileName )
    @err_msg.clear
    @user = "unknown" 
    @date = "unknown" 
    @source = "unknown" 
    @inx_name = inFileName

    file = File.open(inFileName,"r")
		doc = REXML::Document.new(file)
		CheckWarningCore(doc.root)
    file.close()

    if @err_msg.length() == 0 then
      return
    end

    printf("#Search %s\n",inFileName)
    printf(" [ source %s,  user=%s,  date=%s ]\n",@source,@user,@date)
    for msg in @err_msg do
      puts(msg)
    end
    printf("__________________________________________________\n",inFileName)
	end

end #endf of class C_NIX_CHECK

def convert
  c_nix_check = C_NIX_CHECK.new()
  Find.find("."){ |path|
#    val = File.expand_path(path)

    if FileTest.directory?(path)
	    next
    end

	  ext = File.extname(path)
    idx = $ext_list.index(ext)
	  if idx != nil then
      c_nix_check.NixCheckWarning(path)
	  end
  }
end

convert

