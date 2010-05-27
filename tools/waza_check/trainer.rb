# @brief  トレーナークラス
# @file   trainer.rb
# @author obata
# @date   2010.05.28

require "waza_check.rb" 


class Pokemon
  def initialize
    @mons_name = nil
    @level     = 0
    @waza_list = Array.new
  end 

  attr_reader :mons_name, :level, :waza_list

  def set_mons_name( mons_name )
    @mons_name = mons_name
  end

  def set_level( level )
    @level = level
  end

  def setup_default_waza
    @waza_list = GetDefaultWazaList( @mons_name, @level )
  end

  def set_waza_list( waza_list )
    @waza_list = waza_list
  end

  def add_waza( waza_name )
    if waza_name != nil then
      @waza_list << waza_name
    end
  end
end


class Trainer
  def initialize 
    @name         = nil
    @tyep         = nil
    @pokemon_list = Array.new
  end

  attr_reader :name, :type, :pokemon_list

  def set_name( name )
    @name = name
  end

  def set_type( type )
    @type = type
  end

  def add_pokemon( pokemon )
    @pokemon_list << pokemon
  end
end
