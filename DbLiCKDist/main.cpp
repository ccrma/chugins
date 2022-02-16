/* ----------------------------------------------------------------
 DbLiCKDist is a ugen that implements LiCK's (github/heuermh/lick)
 distortion chugen's natively.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 U.S.A.
 ---------------------------------------------------------------------
*/
#define _USE_MATH_DEFINES

#include "chuck_dl.h"
#include "chuck_def.h"

#include "DbLiCKDistort.h"


CK_DLL_CTOR( dbld_ctor );
CK_DLL_DTOR( dbld_dtor );
CK_DLL_MFUN( dbld_distortion );
CK_DLL_MFUN( dbld_setparam );
CK_DLL_TICK( dbld_tick );

// set basis

t_CKINT dbld_data_offset = 0;

/* -------------------------------------------------------------------- */

CK_DLL_QUERY(DbLiCKDistort)
{
    QUERY->setname(QUERY, "DbLiCKDistort");
    QUERY->begin_class(QUERY, "DbLiCKDistort", "UGen");
    QUERY->add_ctor(QUERY, dbld_ctor);
    QUERY->add_dtor(QUERY, dbld_dtor);

    // set
    QUERY->add_mfun(QUERY, dbld_distortion, "string", "distortion");
    QUERY->add_arg(QUERY, "string", "name");

    // setparam
    QUERY->add_mfun(QUERY, dbld_setparam, "int", "setparam");
    QUERY->add_arg(QUERY, "int", "id");
    QUERY->add_arg(QUERY, "float", "value");

    // tick
    QUERY->add_ugen_func(QUERY, dbld_tick, NULL, 1, 1);


    dbld_data_offset = QUERY->add_mvar(QUERY, "int", "@dbLiCKdistort_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbld_ctor)
{
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    DbLiCKDistortMgr * c = new DbLiCKDistortMgr();
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(dbld_dtor)
{
    DbLiCKDistortMgr *c = (DbLiCKDistortMgr *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_MFUN(dbld_distortion) // set
{
    DbLiCKDistortMgr *c = (DbLiCKDistortMgr *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    std::string nm = GET_NEXT_STRING_SAFE(ARGS);
    RETURN->v_int = c->Set(nm.c_str()); // 0 is success
}

CK_DLL_MFUN(dbld_setparam) // set
{
    DbLiCKDistortMgr *c = (DbLiCKDistortMgr *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int i = GET_NEXT_INT(ARGS); // 
    float f = GET_NEXT_FLOAT(ARGS);
    c->SetParam(i, f);
}

CK_DLL_TICK(dbld_tick)
{
    DbLiCKDistortMgr *c = (DbLiCKDistortMgr *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    *out = c->Tick(in);
    return TRUE;
}