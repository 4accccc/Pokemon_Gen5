//=============================================================================================
/**
 * @file	btl_pre_sc.h
 * @brief	ポケモンWB バトルシステム	プレサーバコマンド生成
 * @author	taya
 *
 * @date	2009.04.07	作成
 */
//=============================================================================================
#ifndef __BTL_PRE_SC_H__
#define __BTL_PRE_SC_H__

enum {
	PRE_SC_BUF_SIZE = 2048,
};

typedef enum {

	PRESC_ESCAPE_ROOT,
	PRESC_FIGHT_ROOT,
	PRESC_ITEM_ROOT,
	PRESC_CHANGE_ROOT,

	PRESC_WAZA_EXE,
	PRESC_MAX,

}PreScElem;

typedef enum {

	PRESC_FMT_SINGLE,	///< 単独要素
	PRESC_FMT_OPEN,		///< 要素を内包する（開始）
	PRESC_FMT_CLOSE,	///< 要素を内包する（終了）
	PRESC_FMT_MAX,
}PreScFormat;


typedef struct {
	u16  pointer;
	u16  code[ PRE_SC_BUF_SIZE ];
}PRE_SC_BUF;



static inline void PreSC_Init( PRE_SC_BUF* buf )
{
	buf->pointer = 0;
}

extern void PreSC_Put( PRE_SC_BUF* buf, PreScElem preSc, PreScFormat fmt, u32 numArgs, ... );


/*
<escape pokeID=0, success=0>
</escape>


<escape pokeID=0>
  <success>0</success>
</escape>

<escape pokeID=0>
  <success>1</success>
</escape>

<escape pokeID=0>
  <intr type="waza" pokeID=7>
    <waza_exe pokeID=7 wazaID=?? targetID=0>
      <waza_dmg pokeID=0 value=40 dead=0 />
    </waza_exe>
  </intr>
  <success>1</success>
</escape>

<escape pokeID=0>
  <intr type="waza" pokeID=7>
    <waza_exe pokeID=7 wazaID=?? >
      <waza_target pokeID=0/>
      <waza_dmg pokeID=0 value=40 dead=1 />
    </waza_exe>
  </intr>
</escape>


<fight_root pokeID=0>

  <confuse_chk>
    <dmg type="confuse" pokeID=0>
      <value>30</value>
      <dead>0</dead>
    </dmg>
  </confuse_chk>

	<exe_check>
		<fail cause=
	</exe_check>

</fight_root>



*/


#endif
