#include "chugin.h"
#include "util_xml.h"

// general includes
#include <stdio.h>
#include <limits.h>

static t_CKUINT XmlSettings_offset_data = 0;

CK_DLL_CTOR( XmlSettings_ctor );
CK_DLL_DTOR( XmlSettings_dtor );
CK_DLL_MFUN( XmlSettings_open );
CK_DLL_MFUN( XmlSettings_save );
CK_DLL_MFUN( XmlSettings_getIntValue );
CK_DLL_MFUN( XmlSettings_getFloatValue );
CK_DLL_MFUN( XmlSettings_getStringValue );
CK_DLL_MFUN( XmlSettings_getNumTags );
CK_DLL_MFUN( XmlSettings_pushTag );
CK_DLL_MFUN( XmlSettings_popTag );
CK_DLL_MFUN( XmlSettings_getPushLevel );
CK_DLL_MFUN( XmlSettings_tagExists );
CK_DLL_MFUN( XmlSettings_getNumAttributes );
CK_DLL_MFUN( XmlSettings_attributeExists );
CK_DLL_MFUN( XmlSettings_getIntAttribute );
CK_DLL_MFUN( XmlSettings_getFloatAttribute );
CK_DLL_MFUN( XmlSettings_getStringAttribute );

CK_DLL_QUERY( XML )
{
    QUERY->setname(QUERY, "XML");
    
    QUERY->begin_class(QUERY, "XML", "Object");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, XmlSettings_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, XmlSettings_dtor);
    
    // add open
    QUERY->add_mfun(QUERY,  XmlSettings_open, "int", "open");
    QUERY->add_arg(QUERY,"string", "filename");
    QUERY->doc_func(QUERY, "Open a XML file");

    // add save
    QUERY->add_mfun(QUERY,  XmlSettings_open, "int", "save");
    QUERY->add_arg(QUERY,"string", "filename");
    QUERY->doc_func(QUERY, "Save XML file");

    // add getIntValue
    QUERY->add_mfun(QUERY,  XmlSettings_getIntValue, "int", "getIntValue");
    QUERY->add_arg(QUERY,"string", "tag");
    QUERY->add_arg(QUERY,"int", "defaultValue");
    QUERY->add_arg(QUERY,"int", "which");
    QUERY->doc_func(QUERY, "Get int value");

    // add getFloatValue
    QUERY->add_mfun(QUERY, XmlSettings_getFloatValue, "float", "getFloatValue");
    QUERY->add_arg( QUERY,"string", "tag" );
    QUERY->add_arg( QUERY,"float", "defaultValue" );
    QUERY->add_arg( QUERY,"int", "which" );
    QUERY->doc_func(QUERY, "Get float value");

    // add getStringValue
    QUERY->add_mfun(QUERY,XmlSettings_getStringValue, "string", "getStringValue");
    QUERY->add_arg(QUERY, "string", "tag" );
    QUERY->add_arg(QUERY, "string", "defaultValue" );
    QUERY->add_arg(QUERY, "int", "which" );
    QUERY->doc_func(QUERY, "Get string value");

    // add getNumTags
    QUERY->add_mfun(QUERY,XmlSettings_getNumTags, "int", "getNumTags" );
    QUERY->add_arg(QUERY, "string", "tag" );
    QUERY->doc_func(QUERY, "Get number of tags");

    // add pushTag
    QUERY->add_mfun(QUERY, XmlSettings_pushTag,"int", "pushTag" );
    QUERY->add_arg(QUERY, "string", "tag" );
    QUERY->add_arg(QUERY, "int", "which" );
    QUERY->doc_func(QUERY, "Push tag");

    // add popTag
    QUERY->add_mfun( QUERY, XmlSettings_popTag, "int", "popTag" );
    QUERY->doc_func(QUERY, "Pop tag");

    // add getPushLevel
    QUERY->add_mfun( QUERY, XmlSettings_getPushLevel, "int", "getPushLevel" );
    QUERY->doc_func(QUERY, "Push level");

    // add tagExists
    QUERY->add_mfun(QUERY, XmlSettings_tagExists,"int", "tagExists" );
    QUERY->add_arg(QUERY, "string", "tag" );
    QUERY->add_arg(QUERY, "int", "which" );
    QUERY->doc_func(QUERY, "Check if tag exists");

    // add getNumAttributes
    QUERY->add_mfun(QUERY, XmlSettings_getNumAttributes,"int", "getNumAttributes" );
    QUERY->add_arg(QUERY, "string", "tag" );
    QUERY->add_arg(QUERY, "int", "which" );
    QUERY->doc_func(QUERY, "Get number of attributes");

    // add attributeExists
    QUERY->add_mfun(QUERY,XmlSettings_attributeExists, "int", "attributeExists");
    QUERY->add_arg(QUERY, "string", "tag" );
    QUERY->add_arg(QUERY, "string", "attribute" );
    QUERY->add_arg(QUERY, "int", "which" );
    QUERY->doc_func(QUERY, "Check if Attribute exists");

    // add getIntAttribute
    QUERY->add_mfun(QUERY,  XmlSettings_getIntAttribute, "int", "getIntAttribute");
    QUERY->add_arg(QUERY,"string", "tag");
    QUERY->add_arg(QUERY,"string", "attribute");
    QUERY->add_arg(QUERY,"int", "defaultValue");
    QUERY->add_arg(QUERY,"int", "which");
    QUERY->doc_func(QUERY, "Get int attribute");

    // add getFloatAttribute
    QUERY->add_mfun(QUERY,  XmlSettings_getFloatAttribute, "float", "getFloatAttribute");
    QUERY->add_arg(QUERY,"string", "tag");
    QUERY->add_arg(QUERY,"string", "attribute");
    QUERY->add_arg(QUERY,"float", "defaultValue");
    QUERY->add_arg(QUERY,"int", "which");
    QUERY->doc_func(QUERY, "Get float attribute");

    // add getStringAttribute
    QUERY->add_mfun(QUERY,  XmlSettings_getStringAttribute, "string", "getStringAttribute");
    QUERY->add_arg(QUERY,"string", "tag");
    QUERY->add_arg(QUERY,"string", "attribute");
    QUERY->add_arg(QUERY,"string", "defaultValue");
    QUERY->add_arg(QUERY,"int", "which");
    QUERY->doc_func(QUERY, "Get string attribute");

    XmlSettings_offset_data = QUERY->add_mvar(QUERY, "int", "@%(CHUGIN_INITIALS)%_data", false);

    QUERY->end_class(QUERY);

    return TRUE;
}


//-----------------------------------------------------------------------------
// XML API
//-----------------------------------------------------------------------------
CK_DLL_CTOR( XmlSettings_ctor )
{
    OBJ_MEMBER_INT(SELF, XmlSettings_offset_data) = 0;

    XmlSettings * bcdata = new XmlSettings;
    OBJ_MEMBER_INT(SELF, XmlSettings_offset_data) = (t_CKINT) bcdata;
}

CK_DLL_DTOR( XmlSettings_dtor )
{
    delete (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    OBJ_MEMBER_INT(SELF, XmlSettings_offset_data) = 0;
}

CK_DLL_MFUN( XmlSettings_open )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    RETURN->v_int = xml->open( filename.c_str() );
}

CK_DLL_MFUN( XmlSettings_save )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string filename = GET_NEXT_STRING_SAFE( ARGS );
    RETURN->v_int = xml->save( filename.c_str() );
}

CK_DLL_MFUN( XmlSettings_getIntValue )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    t_CKINT defaultValue = GET_NEXT_INT(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    RETURN->v_int = xml->getIntValue( tag, defaultValue, which );
}

CK_DLL_MFUN( XmlSettings_getFloatValue )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    t_CKFLOAT defaultValue = GET_NEXT_FLOAT(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    RETURN->v_float = xml->getFloatValue( tag, defaultValue, which );
}

CK_DLL_MFUN( XmlSettings_getStringValue )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    std::string defaultValue = GET_NEXT_STRING_SAFE(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    std::string method = xml->getStringValue( tag, defaultValue, which );
    RETURN->v_string = (Chuck_String*)API->object->create_string(VM, method.c_str(), false);
}

CK_DLL_MFUN( XmlSettings_getNumTags )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    RETURN->v_int = xml->getNumTags( tag );
}

CK_DLL_MFUN( XmlSettings_pushTag )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    RETURN->v_int = xml->pushTag( tag, which );
}

CK_DLL_MFUN( XmlSettings_popTag )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    RETURN->v_int = xml->popTag();
}

CK_DLL_MFUN( XmlSettings_getPushLevel )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    RETURN->v_int = xml->getPushLevel();
}

CK_DLL_MFUN( XmlSettings_tagExists )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    RETURN->v_int = xml->tagExists( tag, which );
}

CK_DLL_MFUN( XmlSettings_attributeExists )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    std::string attribute = GET_NEXT_STRING_SAFE(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    RETURN->v_int = xml->attributeExists( tag, attribute, which );
}

CK_DLL_MFUN( XmlSettings_getNumAttributes )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    RETURN->v_int = xml->getNumAttributes( tag, which );
}

CK_DLL_MFUN( XmlSettings_getIntAttribute )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    std::string attribute = GET_NEXT_STRING_SAFE(ARGS);
    t_CKINT defaultValue = GET_NEXT_INT(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    RETURN->v_int = xml->getIntAttribute( tag, attribute, defaultValue, which );
}

CK_DLL_MFUN( XmlSettings_getFloatAttribute )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    std::string attribute = GET_NEXT_STRING_SAFE(ARGS);
    t_CKINT defaultValue = GET_NEXT_INT(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    RETURN->v_float = xml->getFloatAttribute( tag, attribute, defaultValue, which );
}

CK_DLL_MFUN( XmlSettings_getStringAttribute )
{
    XmlSettings * xml = (XmlSettings *)OBJ_MEMBER_INT(SELF, XmlSettings_offset_data);
    std::string tag = GET_NEXT_STRING_SAFE(ARGS);
    std::string attribute = GET_NEXT_STRING_SAFE(ARGS);
    std::string defaultValue = GET_NEXT_STRING_SAFE(ARGS);
    t_CKINT which = GET_NEXT_INT(ARGS);
    std::string method = xml->getStringAttribute( tag, attribute, defaultValue, which );
    RETURN->v_string = (Chuck_String*)API->object->create_string(VM, method.c_str(), false);
}
