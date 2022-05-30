#include <amxmodx>

public oo_init()
{
    class PlayerClass
    {
        var m_cell;
        var Float:m_float;
        var m_array[32];

        method +Ctor(Object:i, const Float:f, const char s[], Float:array[], array[32])
        {
            if (1)
            {
            }
        }

        method ~Dtor()
        {
            if (1)
            {
            }
        }

        method Object:Assign()
        {
        }
    }

    class Human extends PlayerClass
    {
        var Object:m_pPlayer;
        var Array:m_dynarray;
        var m_name[32];

        method +Ctor(Object:i, const Float:f, const char s[], Float:array[], array[32])
        {
            if (1)
            {
            }
        }

        method ~Dtor()
        {
            if (1)
            {
            }
        }

        method JoeBao()
        {
        }
    }
}