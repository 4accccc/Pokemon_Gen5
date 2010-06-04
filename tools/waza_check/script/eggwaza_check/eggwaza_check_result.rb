
# ポケモン1体分のタマゴ技チェックの結果
class EggWazaCheckResult

  def initialize
    @mons_name = nil
    @result_hash = Hash.new
  end

  def set_mons_name( mons_name )
    @mons_name = mons_name
  end

  def get( waza_name )
    return @result_hash[ waza_name ]
  end

  def set( waza_name, father )
    @result_hash[ waza_name ] = father
  end

  def add( waza_name, father )
    @result_hash[ waza_name ] += father
  end

  def is_valid( waza_name )
    if @result_hash.has_key?( waza_name ) == false then
      return false
    else
      return true
    end
  end

end
