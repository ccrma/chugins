/*----------------------------------------------------------------------------
  miniAudicle
  Graphical ChucK audio programming environment

  Copyright (c) 2005 Spencer Salazar.  All rights reserved.
    http://chuck.cs.princeton.edu/
    http://soundlab.cs.princeton.edu/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// name: miniAudicle_import.cpp
// desc: miniAudicle class library base
//
// authors: Spencer Salazar (ssalazar@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//    date: spring 2005
//-----------------------------------------------------------------------------

#include "miniAudicle_import.h"
#include "miniAudicle_ui_elements.h"
#include "chuck_type.h"
#include "chuck_errmsg.h"
#include "chuck_instr.h"


//-----------------------------------------------------------------------------
// MAUIElement
//-----------------------------------------------------------------------------
CK_DLL_CTOR( mauielement_ctor );
CK_DLL_DTOR( mauielement_dtor );
CK_DLL_MFUN( mauielement_display );
CK_DLL_MFUN( mauielement_hide );
CK_DLL_MFUN( mauielement_destroy );
CK_DLL_MFUN( mauielement_name_set );
CK_DLL_MFUN( mauielement_name_get );
CK_DLL_MFUN( mauielement_size );
CK_DLL_MFUN( mauielement_height );
CK_DLL_MFUN( mauielement_width );
CK_DLL_MFUN( mauielement_position );
CK_DLL_MFUN( mauielement_x );
CK_DLL_MFUN( mauielement_y );
CK_DLL_MFUN( mauielement_onchange );

//-----------------------------------------------------------------------------
// MAUISlider API
//-----------------------------------------------------------------------------
CK_DLL_CTOR( maslider_ctor );
CK_DLL_DTOR( maslider_dtor );
CK_DLL_MFUN( maslider_value_set );
CK_DLL_MFUN( maslider_value_get );
CK_DLL_MFUN( maslider_range );
CK_DLL_MFUN( maslider_frange );
CK_DLL_MFUN( maslider_irange );
CK_DLL_MFUN( maslider_max );
CK_DLL_MFUN( maslider_min );
CK_DLL_MFUN( maslider_set_precision );
CK_DLL_MFUN( maslider_get_precision );
CK_DLL_MFUN( maslider_set_display_format );
CK_DLL_MFUN( maslider_get_display_format );
CK_DLL_MFUN( maslider_set_orientation );
CK_DLL_MFUN( maslider_get_orientation );

//-----------------------------------------------------------------------------
// MAUIView API
//-----------------------------------------------------------------------------
CK_DLL_CTOR( mauiview_ctor );
CK_DLL_DTOR( mauiview_dtor );
CK_DLL_MFUN( mauiview_add_element );

//-----------------------------------------------------------------------------
// MAUIButton API
//-----------------------------------------------------------------------------
CK_DLL_CTOR( mauibutton_ctor );
CK_DLL_DTOR( mauibutton_dtor );
CK_DLL_MFUN( mauibutton_set_state );
CK_DLL_MFUN( mauibutton_get_state );
CK_DLL_MFUN( mauibutton_push_type );
CK_DLL_MFUN( mauibutton_toggle_type );
CK_DLL_MFUN( mauibutton_set_image );
CK_DLL_MFUN( mauibutton_unset_image );

//-----------------------------------------------------------------------------
// MAUILED API
//-----------------------------------------------------------------------------
CK_DLL_CTOR( mauiled_ctor );
CK_DLL_DTOR( mauiled_dtor );
CK_DLL_MFUN( mauiled_light );
CK_DLL_MFUN( mauiled_unlight );
CK_DLL_MFUN( mauiled_get_color );
CK_DLL_MFUN( mauiled_set_color );

//-----------------------------------------------------------------------------
// MAUI_Text API
//-----------------------------------------------------------------------------
CK_DLL_CTOR( mauitext_ctor );
CK_DLL_DTOR( mauitext_dtor );

//-----------------------------------------------------------------------------
// MAUI_Gauge API
//-----------------------------------------------------------------------------
CK_DLL_CTOR( mauigauge_ctor );
CK_DLL_DTOR( mauigauge_dtor );
CK_DLL_MFUN( mauigauge_set_value );
CK_DLL_MFUN( mauigauge_get_value );


//namespace miniAudicle
//{

t_CKBOOL init_class_mauielement( Chuck_DL_Query * QUERY );
t_CKBOOL init_class_maslider( Chuck_DL_Query * QUERY );
t_CKBOOL init_class_mauiview( Chuck_DL_Query * QUERY );
t_CKBOOL init_class_mauibutton( Chuck_DL_Query * QUERY );
t_CKBOOL init_class_mauiled( Chuck_DL_Query * QUERY );
t_CKBOOL init_class_mauitext( Chuck_DL_Query * QUERY );
t_CKBOOL init_class_mauigauge( Chuck_DL_Query * QUERY );

static t_CKINT mauielement_offset_data = 0;

//-----------------------------------------------------------------------------
// name: init_maui()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_maui( Chuck_DL_Query * QUERY )
{
    // log
    EM_log( CK_LOG_SEVERE, "loading miniAudicle UI (MAUI) API..." );
    // push indent level
    EM_pushlog();

    // go
    //EM_log( CK_LOG_SEVERE, "module MAUI_Element..." );    
    if( !init_class_mauielement( QUERY ) ) goto error;
    //EM_log( CK_LOG_SEVERE, "module MAUI_Slider..." );
    if( !init_class_maslider( QUERY ) ) goto error;
    //EM_log( CK_LOG_SEVERE, "module MAUI_View..." );
    if( !init_class_mauiview( QUERY ) ) goto error;
    //EM_log( CK_LOG_SEVERE, "module MAUI_Button..." );
    if( !init_class_mauibutton( QUERY ) ) goto error;
    //EM_log( CK_LOG_SEVERE, "module MAUI_LED..." );
    if( !init_class_mauiled( QUERY ) ) goto error;
    //EM_log( CK_LOG_SEVERE, "module MAUI_Text..." );
    if( !init_class_mauitext( QUERY ) ) goto error;
    //EM_log( CK_LOG_SEVERE, "module MAUI_Gauge..." );
    if( !init_class_mauigauge( QUERY ) ) goto error;

    // pop indent level
    EM_poplog();

    return TRUE;

error:

    // pop indent level
    EM_poplog();

    return FALSE;
}

// MAUI_Element implementation
#pragma mark --MAUI_Element--
//-----------------------------------------------------------------------------
// name: init_class_mauielement()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_class_mauielement( Chuck_DL_Query * QUERY )
{
    Chuck_DL_Func * func = NULL;
    
    // log
    EM_log( CK_LOG_INFO, "class 'MAUI_Element'" );
    
    // import
    QUERY->begin_class(QUERY, "MAUI_Element", "Event");
    QUERY->add_ctor(QUERY, mauielement_ctor);
    QUERY->add_dtor(QUERY, mauielement_dtor);
    
    // add member
    mauielement_offset_data = QUERY->add_mvar(QUERY, "int", "@data", FALSE);
    
    QUERY->add_mfun(QUERY, mauielement_display, "void", "display");
    
    // add hide()
    QUERY->add_mfun(QUERY, mauielement_hide, "void", "hide");
    
    // add destroy()
    QUERY->add_mfun(QUERY, mauielement_destroy, "void", "destroy");
    
    // add name()
    QUERY->add_mfun(QUERY, mauielement_name_set, "string", "name");
    QUERY->add_arg( QUERY,"string", "n" );
    
    // add name()
    QUERY->add_mfun(QUERY, mauielement_name_get, "string", "name");
    
    // add size()
    QUERY->add_mfun(QUERY, mauielement_size, "void", "size");
    QUERY->add_arg( QUERY,"float", "w" );
    QUERY->add_arg( QUERY,"float", "h" );
    
    // add width()
    QUERY->add_mfun(QUERY, mauielement_width, "float", "width");
    
    // add height()
    QUERY->add_mfun(QUERY, mauielement_height, "float", "height");
    
    // add position()
    QUERY->add_mfun(QUERY, mauielement_position, "void", "position");
    QUERY->add_arg( QUERY,"float", "x" );
    QUERY->add_arg( QUERY,"float", "y" );
    
    // add x()
    QUERY->add_mfun(QUERY, mauielement_x, "float", "x");
    
    // add y()
    QUERY->add_mfun(QUERY, mauielement_y, "float", "y");
    
    // add onChange()
    QUERY->add_mfun(QUERY, mauielement_onchange, "Event", "onChange");
    
    // end the class import
    QUERY->end_class(QUERY);
    
    return TRUE;
    
error:
    
    // end the class import
    QUERY->end_class(QUERY);
    
    return FALSE;
}

// constructor
CK_DLL_CTOR( mauielement_ctor )
{ }

// destructor
CK_DLL_DTOR( mauielement_dtor )
{ }

// display
CK_DLL_MFUN( mauielement_display )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    if( maui )
    {
        maui->display();
    }
}

// hide
CK_DLL_MFUN( mauielement_hide )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    if( maui )
        maui->hide();
}

// destroy
CK_DLL_MFUN( mauielement_destroy )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    if( maui )
        maui->destroy();
}

// name
CK_DLL_MFUN( mauielement_name_set )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    Chuck_String * s = GET_NEXT_STRING(ARGS);
    if( maui && s )
    {
        maui->set_name( s->str );
        RETURN->v_string = s;
    }
    else
    {
        RETURN->v_string = NULL;
    }
}

// name
CK_DLL_MFUN( mauielement_name_get )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_string = new Chuck_String( maui->get_name() );
}

// size
CK_DLL_MFUN( mauielement_size )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    t_CKFLOAT w = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT h = GET_NEXT_FLOAT(ARGS);
    maui->set_size( (t_CKDOUBLE)w, (t_CKDOUBLE)h );
}

// height
CK_DLL_MFUN( mauielement_height )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_float = maui->get_height();
}

// width
CK_DLL_MFUN( mauielement_width )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_float = maui->get_width();
}

// position
CK_DLL_MFUN( mauielement_position )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    t_CKFLOAT x = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT y = GET_NEXT_FLOAT(ARGS);
    maui->set_position( (t_CKDOUBLE)x, (t_CKDOUBLE)y );
}

// x
CK_DLL_MFUN( mauielement_x )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_float = maui->get_x();
}

// y
CK_DLL_MFUN( mauielement_y )
{
    UI::Element * maui = (UI::Element *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_float = maui->get_y();
}

// onChange
CK_DLL_MFUN( mauielement_onchange )
{
//    UI::Element * maui = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_object = SELF;
}

// MAUI_Slider implementation
#pragma mark --MAUI_Slider--
//-----------------------------------------------------------------------------
// name: init_class_maslider()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_class_maslider( Chuck_DL_Query * QUERY )
{
    Chuck_DL_Func * func = NULL;
    //    Chuck_Value * value = NULL;
    
    // log
    EM_log( CK_LOG_INFO, "class 'MAUI_Slider'" );
    
    QUERY->begin_class(QUERY, "MAUI_Slider", "MAUI_Element");
    QUERY->add_ctor(QUERY, maslider_ctor);
    QUERY->add_dtor(QUERY, maslider_dtor);
    
    // add value()
    QUERY->add_mfun(QUERY, maslider_value_set, "float", "value");
    QUERY->add_arg( QUERY,"float", "v" );
    
    // add value()
    QUERY->add_mfun(QUERY, maslider_value_get, "float", "value");
    
    // add range()
    QUERY->add_mfun(QUERY, maslider_range, "void", "range");
    QUERY->add_arg( QUERY,"float", "min" );
    QUERY->add_arg( QUERY,"float", "max" );
    
    // add frange()
    QUERY->add_mfun(QUERY, maslider_frange, "void", "frange");
    QUERY->add_arg( QUERY,"float", "min" );
    QUERY->add_arg( QUERY,"float", "max" );
    
    // add irange()
    QUERY->add_mfun(QUERY, maslider_irange, "void", "irange");
    QUERY->add_arg( QUERY,"int", "min" );
    QUERY->add_arg( QUERY,"int", "max" );
    
    // add max()
    QUERY->add_mfun(QUERY, maslider_max, "float", "max");
    
    // add min()
    QUERY->add_mfun(QUERY, maslider_min, "float", "min");
    
    // add get_precision()
    QUERY->add_mfun(QUERY, maslider_get_precision, "int", "precision");
    
    // add set_precision()
    QUERY->add_mfun(QUERY, maslider_set_precision, "int", "precision");
    QUERY->add_arg( QUERY,"int", "p" );
    
    // add get_display_format()
    QUERY->add_mfun(QUERY, maslider_get_display_format, "int", "displayFormat");
    
    // add set_display_format()
    QUERY->add_mfun(QUERY, maslider_set_display_format, "int", "displayFormat");
    QUERY->add_arg( QUERY,"int", "df" );
    
    // add get_orientation()
    QUERY->add_mfun(QUERY, maslider_get_orientation, "int", "orientation");
    
    // add set_orientation()
    QUERY->add_mfun(QUERY, maslider_set_orientation, "int", "orientation");
    QUERY->add_arg( QUERY,"int", "o" );
    
    // add bestFormat
    QUERY->add_svar( QUERY, "int", "bestFormat", TRUE, (void *)&UI::Slider::best_format );
    
    // add decimalFormat
    QUERY->add_svar( QUERY, "int", "decimalFormat", TRUE, (void *)&UI::Slider::decimal_format );
    
    // add scientificFormat
    QUERY->add_svar( QUERY, "int", "scientificFormat", TRUE, (void *)&UI::Slider::scientific_format );
    
    // add integerFormat
    QUERY->add_svar( QUERY, "int", "integerFormat", TRUE, (void *)&UI::Slider::integer_format );
    
    // add vertical
    QUERY->add_svar( QUERY, "int", "vertical", TRUE, (void *)&UI::Slider::vertical );
    
    // add horizontal
    QUERY->add_svar( QUERY, "int", "horizontal", TRUE, (void *)&UI::Slider::horizontal );
    
    // wrap up
    QUERY->end_class(QUERY);
    
    return TRUE;
    
error:
    
    // wrap up
    QUERY->end_class(QUERY);
    
    return FALSE;
}

// ctor
CK_DLL_CTOR( maslider_ctor )
{
    UI::Slider * slider = new UI::Slider;
    
    slider->init();
    slider->set_event( (Chuck_Event *)SELF );

    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = (t_CKINT)slider;
}

// dtor
CK_DLL_DTOR( maslider_dtor )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    slider->destroy();
    delete slider;
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = 0;
}

// value
CK_DLL_MFUN( maslider_value_set )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    t_CKFLOAT v = GET_NEXT_FLOAT(ARGS);
    slider->set_value( v );
    RETURN->v_float = slider->get_value();
}

// value
CK_DLL_MFUN( maslider_value_get )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_float = slider->get_value();
}

// range
CK_DLL_MFUN( maslider_range )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    t_CKFLOAT lo = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT hi = GET_NEXT_FLOAT(ARGS);
    slider->set_range( lo, hi );
}

// frange
CK_DLL_MFUN( maslider_frange )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    t_CKFLOAT lo = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT hi = GET_NEXT_FLOAT(ARGS);
    slider->set_range( lo, hi );
}

// irange
CK_DLL_MFUN( maslider_irange )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    t_CKINT lo = GET_NEXT_INT(ARGS);
    t_CKINT hi = GET_NEXT_INT(ARGS);
    slider->set_range( (t_CKFLOAT)lo, (t_CKFLOAT)hi );
}

// max
CK_DLL_MFUN( maslider_max )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_float = slider->get_max();
}

// min
CK_DLL_MFUN( maslider_min )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_float = slider->get_min();
}

// get_precision
CK_DLL_MFUN( maslider_get_precision )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_int = slider->get_precision();
}

// set_precision
CK_DLL_MFUN( maslider_set_precision )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    slider->set_precision( GET_NEXT_INT( ARGS ) );
    RETURN->v_int = slider->get_precision();
}

// get_display_format
CK_DLL_MFUN( maslider_get_display_format )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_int = slider->get_display_format();
}

// set_display_format
CK_DLL_MFUN( maslider_set_display_format )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    slider->set_display_format( GET_NEXT_INT( ARGS ) );
    RETURN->v_int = slider->get_display_format();
}

// get_orientation
CK_DLL_MFUN( maslider_get_orientation )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_int = slider->get_orientation();
}

// set_orientation
CK_DLL_MFUN( maslider_set_orientation )
{
    UI::Slider * slider = (UI::Slider *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    slider->set_orientation( GET_NEXT_INT( ARGS ) );
    RETURN->v_int = slider->get_orientation();
}

// MAUI_View implementation
#pragma mark --MAUI_View--
//-----------------------------------------------------------------------------
// name: init_class_mauiview()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_class_mauiview( Chuck_DL_Query * QUERY )
{
    Chuck_DL_Func * func = NULL;
    //    Chuck_Value * value = NULL;
    
    // log
    EM_log( CK_LOG_INFO, "class 'MAUI_View'" );
    
    // import
    QUERY->begin_class(QUERY, "MAUI_View", "MAUI_Element");
    QUERY->add_ctor( QUERY, mauiview_ctor );
    QUERY->add_dtor( QUERY, mauiview_dtor );
    
    // add add_element()
    QUERY->add_mfun(QUERY, mauiview_add_element, "void", "addElement");
    QUERY->add_arg( QUERY,"MAUI_Element", "e" );
    
    // wrap up
    QUERY->end_class(QUERY);
    
    return TRUE;
    
error:
    
    // wrap up
    QUERY->end_class(QUERY);
    
    return FALSE;
}

// ctor
CK_DLL_CTOR( mauiview_ctor )
{
    UI::View * view = new UI::View;
    view->init();
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = (t_CKINT)view;
    view->set_event( (Chuck_Event *)SELF );
}

// dtor
CK_DLL_DTOR( mauiview_dtor )
{
    UI::View * view = (UI::View *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    view->destroy();
    delete view;
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = 0;
}

// add_element
CK_DLL_MFUN( mauiview_add_element )
{
    UI::View * view = (UI::View *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    UI::Element * e = ( UI::Element * )OBJ_MEMBER_INT(GET_NEXT_OBJECT(ARGS),
                                                      mauielement_offset_data );
    view->add_element( e );
}

// MAUI_Button implementation
#pragma mark --MAUI_Button--
//-----------------------------------------------------------------------------
// name: init_class_mauibutton()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_class_mauibutton( Chuck_DL_Query * QUERY )
{
    Chuck_DL_Func * func = NULL;
    //    Chuck_Value * value = NULL;
    
    // log
    EM_log( CK_LOG_INFO, "class 'MAUI_Button'" );
    
    // import
    QUERY->begin_class(QUERY, "MAUI_Button", "MAUI_Element");
    QUERY->add_ctor( QUERY, mauibutton_ctor );
    QUERY->add_dtor( QUERY, mauibutton_dtor );
    
    // add get_state()
    QUERY->add_mfun(QUERY, mauibutton_get_state, "int", "state");
    
    // add set_state()
    QUERY->add_mfun(QUERY, mauibutton_set_state, "int", "state");
    QUERY->add_arg( QUERY,"int", "s" );
    
    // add pushType()
    QUERY->add_mfun(QUERY, mauibutton_push_type, "void", "pushType");
    
    // add toggleType()
    QUERY->add_mfun(QUERY, mauibutton_toggle_type, "void", "toggleType");
    
    // add unsetImage()
    QUERY->add_mfun(QUERY, mauibutton_unset_image, "void", "unsetImage");
    
    // add setImage()
    QUERY->add_mfun(QUERY, mauibutton_set_image, "int", "setImage");
    QUERY->add_arg( QUERY,"string", "filepath" );
    
    // wrap up
    QUERY->end_class(QUERY);
    
    return TRUE;
    
error:
    // wrap up
    QUERY->end_class(QUERY);
    
    return FALSE;
}

// ctor
CK_DLL_CTOR( mauibutton_ctor )
{
    UI::Button * button = new UI::Button;
    button->init();
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = (t_CKINT)button;
    button->set_event( (Chuck_Event *)SELF );
}

// dtor
CK_DLL_DTOR( mauibutton_dtor )
{
    UI::Button * button = (UI::Button *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    button->destroy();
    delete button;
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = 0;
}

// state
CK_DLL_MFUN( mauibutton_get_state )
{
    UI::Button * button = (UI::Button *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_int = button->get_state();
}

// state
CK_DLL_MFUN( mauibutton_set_state )
{
    UI::Button * button = (UI::Button *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    button->set_state( GET_NEXT_INT(ARGS) );
    RETURN->v_int = button->get_state();
}

// push_type
CK_DLL_MFUN( mauibutton_push_type )
{
    UI::Button * button = (UI::Button *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    button->set_action_type( UI::Button::push_type );
}

// toggle_type
CK_DLL_MFUN( mauibutton_toggle_type )
{
    UI::Button * button = (UI::Button *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    button->set_action_type( UI::Button::toggle_type );
}

CK_DLL_MFUN( mauibutton_set_image )
{
    UI::Button * button = (UI::Button *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    Chuck_String * s = GET_NEXT_STRING(ARGS);
    RETURN->v_int = button->set_image(s->str);
}

CK_DLL_MFUN( mauibutton_unset_image )
{
    UI::Button * button = (UI::Button *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    button->unset_image();
}

// MAUI_LED implementation
#pragma mark --MAUI_LED--
//-----------------------------------------------------------------------------
// name: init_class_mauiled()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_class_mauiled( Chuck_DL_Query * QUERY )
{
    Chuck_DL_Func * func = NULL;
    //    Chuck_Value * value = NULL;
    
    // log
    EM_log( CK_LOG_INFO, "class 'MAUI_LED'" );
    
    // import
    QUERY->begin_class(QUERY, "MAUI_LED", "MAUI_Element");
    QUERY->add_ctor( QUERY, mauiled_ctor );
    QUERY->add_dtor( QUERY, mauiled_dtor );
    
    // add light()
    QUERY->add_mfun(QUERY, mauiled_light, "void", "light");
    
    // add unlight()
    QUERY->add_mfun(QUERY, mauiled_unlight, "void", "unlight");
    
    // add color()
    QUERY->add_mfun(QUERY, mauiled_get_color, "int", "color");
    
    // add color()
    QUERY->add_mfun(QUERY, mauiled_set_color, "int", "color");
    QUERY->add_arg( QUERY, "int", "c" );
    
    // add red
    QUERY->add_svar( QUERY, "int", "red", TRUE, (void *)&UI::LED::red );
    
    // add green
    QUERY->add_svar( QUERY, "int", "green", TRUE, (void *)&UI::LED::green );
    
    // add blue
    QUERY->add_svar( QUERY, "int", "blue", TRUE, (void *)&UI::LED::blue );
    
    // wrap up
    QUERY->end_class(QUERY);
    
    return TRUE;
    
error:
    // wrap up
    QUERY->end_class(QUERY);
    
    return FALSE;
}

// ctor
CK_DLL_CTOR( mauiled_ctor )
{
    UI::LED * led = new UI::LED;
    led->init();
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = (t_CKINT)led;
    led->set_event( (Chuck_Event *)SELF );
}

// dtor
CK_DLL_DTOR( mauiled_dtor )
{
    UI::LED * led = (UI::LED *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    led->destroy();
    delete led;
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = 0;
}

// light
CK_DLL_MFUN( mauiled_light )
{
    UI::LED * led = (UI::LED *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    led->light();
}

// unlight
CK_DLL_MFUN( mauiled_unlight )
{
    UI::LED * led = (UI::LED *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    led->unlight();
}

// get_color
CK_DLL_MFUN( mauiled_get_color )
{
    UI::LED * led = (UI::LED *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_int = led->get_color();
}

// set_color
CK_DLL_MFUN( mauiled_set_color )
{
    UI::LED * led = (UI::LED *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    led->set_color( ( UI::LED::color ) GET_NEXT_INT( ARGS ) );
    RETURN->v_int = led->get_color();
}

// MAUI_Text implementation
#pragma mark --MAUI_Text--
//-----------------------------------------------------------------------------
// name: init_class_mauitext()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_class_mauitext( Chuck_DL_Query * QUERY )
{
    //    Chuck_DL_Func * func = NULL;
    //    Chuck_Value * value = NULL;
    
    // log
    EM_log( CK_LOG_INFO, "class 'MAUI_Text'" );
    
    // import
    QUERY->begin_class(QUERY, "MAUI_Text", "MAUI_Element");
    QUERY->add_ctor( QUERY, mauitext_ctor );
    QUERY->add_dtor( QUERY, mauitext_dtor );

    // wrap up
    QUERY->end_class(QUERY);
    
    return TRUE;
    
error:
    // wrap up
    QUERY->end_class(QUERY);
    
    return FALSE;
}

// ctor
CK_DLL_CTOR( mauitext_ctor )
{
    UI::Text * text = new UI::Text;
    text->init();
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = (t_CKINT)text;
    text->set_event( (Chuck_Event *)SELF );
}

// dtor
CK_DLL_DTOR( mauitext_dtor )
{
    UI::Text * text = (UI::Text *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    text->destroy();
    delete text;
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = 0;
}

// MAUI_Gauge implementation
#pragma mark --MAUI_Gauge--
//-----------------------------------------------------------------------------
// name: init_class_mauigauge()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_class_mauigauge( Chuck_DL_Query * QUERY )
{
    Chuck_DL_Func * func = NULL;
    
    // log
    EM_log( CK_LOG_INFO, "class 'MAUI_Gauge'" );
    
    // import
    QUERY->begin_class(QUERY, "MAUI_Gauge", "MAUI_Element");
    QUERY->add_ctor( QUERY, mauigauge_ctor );
    QUERY->add_dtor( QUERY, mauigauge_dtor );

    // add (set) value()
    QUERY->add_mfun(QUERY, mauigauge_set_value, "void", "value");
    QUERY->add_arg( QUERY,"float", "v" );
    
    // add (get) value()
    QUERY->add_mfun(QUERY, mauigauge_get_value, "void", "value");
    
    // wrap up
    QUERY->end_class(QUERY);
    
    return TRUE;
    
error:
    // wrap up
    QUERY->end_class(QUERY);
    
    return FALSE;
}

// ctor
CK_DLL_CTOR( mauigauge_ctor )
{
    UI::Gauge * gauge = new UI::Gauge;
    gauge->init();
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = (t_CKINT)gauge;
    gauge->set_event( (Chuck_Event *)SELF );
}

// dtor
CK_DLL_DTOR( mauigauge_dtor )
{
    UI::Gauge * gauge = (UI::Gauge *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    gauge->destroy();
    delete gauge;
    OBJ_MEMBER_INT(SELF, mauielement_offset_data) = 0;
}

// set_value
CK_DLL_MFUN( mauigauge_set_value )
{
    UI::Gauge * gauge = (UI::Gauge *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    gauge->set_value( GET_NEXT_FLOAT( ARGS ) );
    RETURN->v_float = gauge->get_value();
}

// get_value
CK_DLL_MFUN( mauigauge_get_value )
{
    UI::Gauge * gauge = (UI::Gauge *)OBJ_MEMBER_INT(SELF, mauielement_offset_data);
    RETURN->v_float = gauge->get_value();
}


//}