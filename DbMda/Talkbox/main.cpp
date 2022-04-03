/* ----------------------------------------------------------------
 The DbMdaTalkbox chugin is a port of Paul Keller's open-source
 vst plugins here: http://mda.smartelectronix.com/ and
 here: https://sourceforge.net/projects/mda-vst

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

#include "mdaTalkboxProcessor.h"

CK_DLL_CTOR( dbld_ctor );
CK_DLL_DTOR( dbld_dtor );

CK_DLL_MFUN( dbld_printParams );
CK_DLL_MFUN( dbld_setParam );
CK_DLL_MFUN( dbld_getParam );

CK_DLL_TICKF( dbld_tickf );

t_CKINT dbld_data_offset = 0;

/* -------------------------------------------------------------------- */

CK_DLL_QUERY(DbMdaTalkbox)
{
    QUERY->setname(QUERY, "DbMdaTalkbox");
    QUERY->begin_class(QUERY, "DbMdaTalkbox", "UGen_Stereo");
    QUERY->add_ctor(QUERY, dbld_ctor);
    QUERY->add_dtor(QUERY, dbld_dtor);

    // params ---------------------------------------------
    QUERY->add_mfun(QUERY, dbld_printParams, "void", "printParams");

    QUERY->add_mfun(QUERY, dbld_setParam, "void", "setParam");
    QUERY->add_arg(QUERY, "int", "id");
    QUERY->add_arg(QUERY, "float", "value");

    QUERY->add_mfun(QUERY, dbld_getParam, "float", "getParam");
    QUERY->add_arg(QUERY, "int", "id");

    // stereo-in tick --------------------------------------------------
    // vs: add_ugen_funcf
    QUERY->add_ugen_funcf(QUERY, dbld_tickf, NULL, 2/*nin*/, 2/*nout*/);

    dbld_data_offset = QUERY->add_mvar(QUERY, "int", "@dbld_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbld_ctor)
{
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    float srate = API->vm->get_srate(API, SHRED);
    TalkboxProcessor *c = new TalkboxProcessor(srate);
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(dbld_dtor)
{
    TalkboxProcessor *c = (TalkboxProcessor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    }
}

CK_DLL_MFUN( dbld_printParams )
{
    TalkboxProcessor *c = (TalkboxProcessor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->printParams();
}

CK_DLL_MFUN( dbld_setParam )
{
    TalkboxProcessor *c = (TalkboxProcessor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int i = GET_NEXT_INT(ARGS); // 
    float f = GET_NEXT_FLOAT(ARGS);
    c->setParamValue(i, f);
}

CK_DLL_MFUN( dbld_getParam )
{
    t_CKINT index = GET_NEXT_INT(ARGS);
    TalkboxProcessor *c = (TalkboxProcessor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    RETURN->v_float = c->getParamValue(index);
}

CK_DLL_TICKF(dbld_tickf)
{
    // apparently 2 in, 1 out isn't really supported (cf: Mix2)
    TalkboxProcessor *c = (TalkboxProcessor *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->processMulti(in, 2, out, 2, nframes);
    return TRUE;
}