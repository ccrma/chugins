#include "chugin.h"
#include <string>
#include <vector>
#include <list>

#include "tinyxml.h"
//https://github.com/openframeworks/openFrameworks/blob/master/addons/ofxXmlSettings/src/ofxXmlSettings.cpp#L62


#define MAX_TAG_VALUE_LENGTH_IN_CHARS 1024

class XmlSettings{

	public:
        XmlSettings();
        //XmlSettings(std::string& xmlFile);

        ~XmlSettings();

		//void setVerbose(t_CKBOOL _verbose);

		t_CKBOOL open(const char * path);
		t_CKBOOL save(const char * path);

		//void clearTagContents(std::string& tag, t_CKINT which = 0);
		//void removeTag(std::string& tag, t_CKINT which = 0);

		t_CKBOOL tagExists(std::string tag, t_CKINT which);

		// removes all tags from within either the whole document
		// or the tag you are currently at using pushTag
		t_CKVOID clear();

		t_CKINT getIntValue(std::string tag, t_CKINT defaultValue, t_CKINT which);
		t_CKFLOAT getFloatValue(std::string tag, t_CKDOUBLE defaultValue, t_CKINT which);
		std::string getStringValue(std::string tag, std::string defaultValue, t_CKINT which);
		

		// t_CKINT setValue(std::string&  tag, t_CKINT value, t_CKINT which = 0);
		// t_CKINT setValue(std::string&  tag, t_CKDOUBLE value, t_CKINT which = 0);
		// t_CKINT setValue(std::string&  tag, std::string& value, t_CKINT which = 0);

		//advanced

		//-- pushTag/popTag
		//pushing a tag moves you inside it which has the effect of
		//temporarily treating the tag you are in as the document root
		//all setValue, readValue and getValue commands are then be relative to the tag you pushed.
		//this can be used with addValue to create multiple tags of the same name within
		//the pushed tag - normally addValue only lets you create multiple tags of the same
		//at the top most level.

		t_CKBOOL pushTag(std::string tag, t_CKINT which);
		t_CKINT	popTag();
		t_CKINT getPushLevel();

		//-- numTags
		//this only works for tags at the current root level
		//use pushTag and popTag to get number of tags whithin other tags
		// both getNumTags("PT"); and getNumTags("PT:X"); will just return the
		//number of <PT> tags at the current root level.
		t_CKINT getNumTags(std::string tag);

		//-- addValue/addTag
		//adds a tag to the document even if a tag with the same name
		//already exists - returns an index which can then be used to
		//modify the tag by passing it as the last argument to setValue

		//-- important - this only works for top level tags
		//   to put multiple tags inside other tags - use pushTag() and popTag()

		//t_CKINT addValue(const std::string&  tag, t_CKINT value);
		//t_CKINT addValue(const std::string&  tag, t_CKDOUBLE value);
		//t_CKINT	addValue(const std::string&  tag, const std::string& value);

		//t_CKINT addTag(const std::string& tag); //adds an empty tag at the current level

        // Attribute-related methods
		//t_CKINT addAttribute(const std::string& tag, const std::string& attribute, t_CKINT value, t_CKINT which = 0);
		//t_CKINT addAttribute(const std::string& tag, const std::string& attribute, t_CKDOUBLE value, t_CKINT which = 0);
		//t_CKINT addAttribute(const std::string& tag, const std::string& attribute, const std::string& value, t_CKINT which = 0);

		//t_CKINT addAttribute(const std::string& tag, const std::string& attribute, t_CKINT value);
		//t_CKINT addAttribute(const std::string& tag, const std::string& attribute, t_CKDOUBLE value);
		//t_CKINT	addAttribute(const std::string& tag, const std::string& attribute, const std::string& value);

		//t_CKVOID removeAttribute(const std::string& tag, const std::string& attribute, t_CKINT which = 0);
		//t_CKVOID clearTagAttributes(const std::string& tag, t_CKINT which = 0);

		t_CKINT	getNumAttributes(std::string tag, t_CKINT which = 0);
		t_CKBOOL attributeExists(std::string tag, std::string attribute, t_CKINT which = 0);
		//t_CKBOOL getAttributeNames(std::string tag, std::vector<std::string> outNames, t_CKINT which = 0);

		t_CKINT	getIntAttribute(std::string tag, std::string attribute, int defaultValue, t_CKINT which = 0);
		t_CKFLOAT getFloatAttribute(std::string tag, std::string attribute, double defaultValue, t_CKINT which = 0);
		std::string	getStringAttribute(std::string tag, std::string attribute, std::string defaultValue, t_CKINT which = 0);

		//t_CKINT setAttribute(const std::string& tag, const std::string& attribute, t_CKINT value, t_CKINT which = 0);
		//t_CKINT	setAttribute(const std::string& tag, const std::string& attribute, t_CKDOUBLE value, t_CKINT which = 0);
		//t_CKINT	setAttribute(const std::string& tag, const std::string& attribute, const std::string& value, t_CKINT which = 0);

		//t_CKINT	setAttribute(const std::string& tag, const std::string& attribute, t_CKINT value); // implement this
		//t_CKINT	setAttribute(const std::string& tag, const std::string& attribute, t_CKDOUBLE value);
		//t_CKINT	setAttribute(const std::string& tag, const std::string& attribute, const std::string& value);

		//t_CKBOOL loadFromBuffer( std::string buffer );
		//t_CKVOID copyXmlToString(std::string & str) const;

		TiXmlDocument doc;
		t_CKBOOL bDocLoaded;

	protected:

		TiXmlHandle * storedHandle;
		t_CKINT level;


		//t_CKINT	writeTag(const std::string&  tag, const std::string& valuesString, t_CKINT which = 0);
		t_CKBOOL readTag(std::string  tag, char * valueString, t_CKINT which = 0);	

		//t_CKINT	writeAttribute(const std::string& tag, const std::string& attribute, const std::string& valuesString, t_CKINT which = 0);

		TiXmlElement* getElementForAttribute(std::string tag, int which);
		t_CKBOOL readIntAttribute(std::string tag, std::string attribute, int& valueString, t_CKINT which);
		t_CKBOOL readDoubleAttribute(std::string tag, std::string attribute, double& outValue, t_CKINT which);
		t_CKBOOL readStringAttribute(std::string tag, std::string attribute, std::string& outValue, t_CKINT which);
};   



