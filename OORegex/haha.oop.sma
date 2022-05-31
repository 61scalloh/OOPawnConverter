#include <amxmodx>

public oo_init()
{
	new Class:_c_PlayerClass = oo_class("PlayerClass")
	{
		oo_var(_c_PlayerClass, DT_CELL, "m_cell");
		oo_var(_c_PlayerClass, DT_CELL, "m_float");
		oo_var(_c_PlayerClass, DT_ARRAY[32], "m_array");

		oo_method(_c_PlayerClass, MT_CTOR, "Ctor", FP_CELL, FP_FLOAT, FP_STRING, FP_ARRAY, FP_VAL_BYREF, FP_VAL_BYREF)

		oo_method(_c_PlayerClass, MT_DTOR, "Dtor")
		oo_method(_c_PlayerClass, MT_METHOD, "Assign")
		oo_var(_c_PlayerClass, DT_CELL, "joejoe");
	}

	new Class:_c_Human = oo_class("Human", "PlayerClass")
	{
		oo_var(_c_Human, DT_CELL, "m_pPlayer");
		oo_var(_c_Human, DT_CELL, "m_dynarray");
		oo_var(_c_Human, DT_ARRAY[32], "m_name");

		oo_method(_c_Human, MT_CTOR, "Ctor", FP_STRING, FP_STRING, FP_STRING, FP_ARRAY, FP_ARRAY)

		oo_method(_c_Human, MT_DTOR, "Dtor")

		oo_var(_c_Human, DT_ARRAY[32], "diu");

		oo_method(_c_Human, MT_METHOD, "JoeBao")
	}
}

public PlayerClass@Ctor(Object:i, const Float:f, const s[], array[32], &Float:ha, &cell)
{
	new fuck = 999;
}

public Human@Dtor()
{
	new a = 0;
	if (1) {a=1} else {1=2}
}

public Human@JoeBao()
{
	new ha = 1 + 2 + 3;
}