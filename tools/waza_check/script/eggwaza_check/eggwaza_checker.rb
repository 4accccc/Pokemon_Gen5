require "personal_accessor.rb"
require "reverse_seed_poke_hash.rb"
require "egg_group_hash.rb"
require "egg_group_poke_list_hash.rb"
require "eggwaza_hash.rb"
require "mons_fullname_list.rb"
require "eggwaza_check_result.rb"
require "seed_poke_hash.rb"


# ƒ|ƒPƒ‚ƒ“1‘Ì•ª‚Ìƒ^ƒ}ƒS‹Zƒ`ƒFƒbƒN
# ( ƒ^ƒ}ƒS‹Z‚Ì”ìŒq‚¬‚É‚æ‚éŒp³ƒ‹[ƒg‚ğœ‚­ )
class EggWazaChecker

  def initialize
    @personal_accessor = PersonalAccessor.new
    @result = nil
    @mons_name = nil
    @eggwaza_list = nil
    @mother_list = nil
    @mothers_egg_group_list = nil
    @father_list = nil
    @eggwaza_exist_flag = false
  end

  def setup( mons_name )
    @result = EggWazaCheckResult.new
    @mons_name = mons_name
    @eggwaza_exist_flag = @personal_accessor.check_eggwaza_exist( @mons_name )
  end


  def analyze( mons_name )
    self.setup( mons_name )
    if @eggwaza_exist_flag then
      self.listup_eggwaza
      self.listup_mothers
      self.delete_mothers_belong_to_egg_group( "–³¶B" )
      self.delete_mothers_only_male
      self.listup_mothers_egg_group
      self.listup_fathers
      self.check_eggwaza_all
    end
    return @result
  end 

  def partial_analyze( mons_name, egg_waza_list )
    self.setup( mons_name )
    if @eggwaza_exist_flag then
      @eggwaza_list = egg_waza_list
      self.listup_mothers
      self.delete_mothers_belong_to_egg_group( "–³¶B" )
      self.delete_mothers_only_male
      self.listup_mothers_egg_group
      self.listup_fathers
      self.check_eggwaza_all
    end
    return @result
  end


  def listup_eggwaza
    @eggwaza_list = Array.new
    @eggwaza_list += $eggwaza_hash[ @mons_name ]
  end

  def listup_mothers
    @mother_list = $reverse_seed_poke_hash[ @mons_name ]
  end

  def delete_mothers_belong_to_egg_group( group_name )
    if @mother_list == nil then return end 
    new_mother_list = Array.new
    @mother_list.each do |mother| 
      if @personal_accessor.get_egg_group1( mother ) != group_name &&
         @personal_accessor.get_egg_group2( mother ) != group_name then
        new_mother_list << mother
      end
    end
    @mother_list = new_mother_list
  end

  def delete_mothers_only_male 
    if @mother_list == nil then return end 
    new_mother_list = Array.new
    @mother_list.each do |mother| 
      gender_type = @personal_accessor.get_gender_type( mother ) 
      if gender_type != "GENDER_TYPE_MALE_ONLY" then
        new_mother_list << mother
      end
    end
    @mother_list = new_mother_list
  end
  
  def listup_mothers_egg_group
    @mothers_egg_group_list = Array.new
    @mother_list.each do |mother|
      @mothers_egg_group_list << @personal_accessor.get_egg_group1( mother )
      @mothers_egg_group_list << @personal_accessor.get_egg_group2( mother )
    end
    @mothers_egg_group_list.uniq! # d•¡‚ğ”rœ
  end

  def listup_fathers
    @father_list = Array.new
    @mothers_egg_group_list.each do |egg_group|
      @father_list += $egg_group_poke_list_hash[ egg_group ]
    end
    @father_list.uniq! # d•¡‚ğ”rœ
  end

  def check_eggwaza_all
    @eggwaza_list.each do |waza_name|
      check_eggwaza( waza_name )
    end
  end

  def check_eggwaza( waza_name )
    father = search_learning_father( waza_name )
    if father != nil then
      @result.set( waza_name, father )
    end
  end

  def search_learning_father( waza_name )
    @father_list.each do |father|
      if @personal_accessor.check_waza_learning_except_eggwaza( father, 100, waza_name ) then
        return father
      end
    end
    return nil # ƒŒƒxƒ‹ or ƒ}ƒVƒ“‚Å‹Z‚ğŠo‚¦‚é•ƒŒó•â‚ª‚¢‚È‚¢
  end

  def get_father_list
    return @father_list
  end

  def get_invalid_eggwaza_list 
    invalid_waza_list = Array.new
    @eggwaza_list.each do |waza_name|
      if check_eggwaza_valid( waza_name ) == false then
        invalid_waza_list << waza_name
      end
    end
    return invalid_waza_list
  end

  def check_eggwaza_valid( waza_name )
    return @resul.get( waza_name ).include?( "›" )
  end

end
