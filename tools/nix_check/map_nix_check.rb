#!ruby -Ks
# Nitro中間ファイル 16charオーバーの文字列を検出する

$KCODE = "SJIS" #文字コードをSJISに設定
require 'rexml/document'
require 'uconv'
require "find"

arg = $*
$input_path = arg[0]

$ext_list = [".imd"]

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

##############################################
#構造体定義
$_tex_image = Struct.new("TexImage", :name, :path, :pltt_name )

class C_NIX_CHECK

	def initialize()
    @err_msg = Array.new
    @tex_list = Array.new
	end
  
	def DataSearch(elem)
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
   
    if elem_name == "tex_image" then
      @tex_list << $_tex_image.new
      cp = @tex_list.last()
      cp.name = Uconv.u8tosjis attrs["name"]
      cp.path = Uconv.u8tosjis attrs["path"]
      cp.pltt_name = Uconv.u8tosjis attrs["palette_name"]
    end

    if elem_name == "material" then
      mat_name = Uconv.u8tosjis attrs["name"]
      tex_idx = attrs["tex_image_idx"].to_i
      pltt_idx = attrs["tex_palette_idx"].to_i

      if mat_name != @tex_list[tex_idx].name then
        @err_msg << (" + #{elem_name}-> \"#{mat_name}\" != \"#{@tex_list[tex_idx].name}\"")
      end
    end

		if elem.has_elements? then
			elem.each_element{|e|
			  DataSearch(e)
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
		DataSearch(doc.root)
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

