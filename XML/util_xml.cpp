#include "util_xml.h"

#include "chugin.h"
#include <string>
#include <vector>
#include <list>
#include <iostream>

// this increases the accuracy of ofToString() when saving floating point values
// but in the process of setting it also causes very small values to be ignored.
const float floatPrecision = 9;

//----------------------------------------
// a pretty useful tokenization system:
static std::vector<std::string> tokenize(const std::string & str, const std::string & delim);
static std::vector<std::string> tokenize(const std::string & str, const std::string & delim)
{
  std::vector<std::string> tokens;

  size_t p0 = 0, p1 = std::string::npos;
  while(p0 != std::string::npos)
  {
    p1 = str.find_first_of(delim, p0);
    if(p1 != p0)
    {
      std::string token = str.substr(p0, p1 - p0);
      tokens.push_back(token);
    }
    p0 = str.find_first_not_of(delim, p1);
  }
  return tokens;
}

//----------------------------------------
XmlSettings::XmlSettings(){
	storedHandle	= new TiXmlHandle(NULL);
	level			= 0;
	//we do this so that we have a valid handle
	//without the need for loadFile
	*storedHandle   = TiXmlHandle(&doc);
}
//---------------------------------------------------------
XmlSettings::~XmlSettings()
{

}

//---------------------------------------------------------
void XmlSettings::clear(){
	//we clear from our root level
	//this is usually the document
	//but if we are pushed  - it could
	//be all the tags inside of the pushed
	//node - including the node itself!

	storedHandle->ToNode()->Clear();
}

//---------------------------------------------------------
t_CKBOOL XmlSettings::open(const char * xmlFile){
	t_CKBOOL loadOkay = doc.LoadFile(xmlFile);

    //our push pop level should be set to 0!
	level = 0;

	*storedHandle = TiXmlHandle(&doc);
	return loadOkay;
}
//---------------------------------------------------------
t_CKBOOL XmlSettings::save(const char * path){
	return doc.SaveFile(path);
}

//---------------------------------------------------------
t_CKINT XmlSettings::getIntValue(std::string tag, t_CKINT defaultValue, t_CKINT which){
    char * tempStr = new char[MAX_TAG_VALUE_LENGTH_IN_CHARS];
	memset(tempStr, 0, MAX_TAG_VALUE_LENGTH_IN_CHARS);
	int returnValue = defaultValue;

	if (readTag(tag, tempStr, which)){
		returnValue = strtol(tempStr, NULL, 0);
	}
	delete [] tempStr;
	return returnValue;
}

//---------------------------------------------------------
t_CKFLOAT XmlSettings::getFloatValue(std::string tag, t_CKDOUBLE defaultValue, t_CKINT which){
	char * tempStr = new char[MAX_TAG_VALUE_LENGTH_IN_CHARS];
	memset(tempStr, 0, MAX_TAG_VALUE_LENGTH_IN_CHARS);
	double returnValue = defaultValue;

	if (readTag(tag, tempStr, which)){
		returnValue = strtof(tempStr,  NULL);
	}
	delete [] tempStr;
	return returnValue;
}

//---------------------------------------------------------
std::string XmlSettings::getStringValue(std::string tag, std::string defaultValue, t_CKINT which){
    char * tempStr = new char[MAX_TAG_VALUE_LENGTH_IN_CHARS];
	memset(tempStr, 0, MAX_TAG_VALUE_LENGTH_IN_CHARS);
	char * returnPtr = (char *) defaultValue.c_str();
	if (readTag(tag, tempStr, which)){
		returnPtr = tempStr;
	}
	std::string returnString(returnPtr);
	delete [] tempStr;
	return returnString;
}

//---------------------------------------------------------
t_CKBOOL XmlSettings::readTag(std::string tag, char * valueString, t_CKINT which){

	std::vector<std::string> tokens = tokenize(tag,":");

	TiXmlHandle tagHandle = *storedHandle;
	for(int x=0;x<(int)tokens.size();x++){
		if(x == 0)tagHandle = tagHandle.ChildElement(tokens.at(x), which);
		else tagHandle = tagHandle.FirstChildElement( tokens.at(x) );
	}

	// once we've walked, let's get that value...
	TiXmlHandle valHandle = tagHandle.Child( 0 );

    //now, clear that vector!
	tokens.clear();

    // if that value is really text, let's get the value out of it !
    if (valHandle.Text()){
#ifdef __PLATFORM_WINDOWS__
    	int maxLen = ck_min(MAX_TAG_VALUE_LENGTH_IN_CHARS, (int)strlen(valHandle.Text()->Value())); // Windows-specific (removed std::)
#else
		int maxLen = std::min(MAX_TAG_VALUE_LENGTH_IN_CHARS, (int)strlen(valHandle.Text()->Value()));
#endif
    	memcpy(valueString, valHandle.Text()->Value(), maxLen);
    	return true;
    }  else {
		return false;
	}
}

t_CKINT XmlSettings::getNumTags(std::string tag){
	//this only works for tags at the current root level

	int pos = tag.find(":");

	if(pos > 0){
		tag = tag.substr(0,pos);
	}

	//grab the handle from the level we are at
	//normally this is the doc but could be a pushed node
	TiXmlHandle tagHandle = *storedHandle;

	int count = 0;

	//ripped from tinyXML as doing this ourselves once is a LOT! faster
	//than having this called n number of times in a while loop - we go from n*n iterations to n iterations

	TiXmlElement* child = ( storedHandle->FirstChildElement( tag ) ).Element();
	for (count = 0; child; child = child->NextSiblingElement( tag ), ++count){
		//nothing
	}

	return count;
}

//---------------------------------------------------------
t_CKBOOL XmlSettings::pushTag(std::string tag, t_CKINT which){

	int pos = tag.find(":");

	if(pos > 0){
		tag = tag.substr(0,pos);
	}

	//we only allow to push one tag at a time.
	TiXmlHandle isRealHandle = storedHandle->ChildElement(tag, which);

	if( isRealHandle.Node() ){
		*storedHandle = isRealHandle;
		level++;
		return true;
	}/*else{
		printf("pushTag - tag not found\n");
	}*/

	return false;
}

//---------------------------------------------------------
t_CKINT XmlSettings::popTag(){

	if(level >= 1){
		TiXmlHandle parent( (storedHandle->ToNode() )->Parent() );
		*storedHandle = parent;
		level--;
	}else{
		*storedHandle = TiXmlHandle(&doc);
		level = 0;
	}

	return level;
}

//---------------------------------------------------------
t_CKINT XmlSettings::getPushLevel(){
	return level;
}

//---------------------------------------------------------
t_CKBOOL XmlSettings::tagExists(std::string tag, t_CKINT which){

	std::vector<std::string> tokens = tokenize(tag,":");

	bool found = false;

	//grab the handle from the level we are at
	//normally this is the doc but could be a pushed node
	TiXmlHandle tagHandle = *storedHandle;

	if(which < 0) which = 0;

	for(int x=0;x<(int)tokens.size();x++){

		//we only support multi tags
		//with same name at root level
		if(x > 0) which = 0;

		TiXmlHandle isRealHandle = tagHandle.ChildElement( tokens.at(x), which);

		//as soon as we find a tag that doesn't exist
		//we return false;
		if ( !isRealHandle.ToNode() ){
			found = false;
			break;
		}
		else{
			found = true;
			tagHandle = isRealHandle;
		}
	}

	return found;
}

//---------------------------------------------------------
t_CKINT XmlSettings::getNumAttributes(std::string tag, t_CKINT which){
	std::vector<std::string> tokens = tokenize(tag,":");
	TiXmlHandle tagHandle = *storedHandle;
	for (int x = 0; x < (int)tokens.size(); x++) {
		if (x == 0)
			tagHandle = tagHandle.ChildElement(tokens.at(x), which);
		else
			tagHandle = tagHandle.FirstChildElement(tokens.at(x));
	}

	if (tagHandle.ToElement()) {
		TiXmlElement* elem = tagHandle.ToElement();

		// Do stuff with the element here
		TiXmlAttribute* first = elem->FirstAttribute();
		if (first) {
			int count = 1;
			for (TiXmlAttribute* curr = first; curr != elem->LastAttribute(); curr = curr->Next())
				count++;
			return count;
		}
	}
	return 0;
}

//---------------------------------------------------------
t_CKBOOL XmlSettings::attributeExists(std::string tag, std::string attribute, t_CKINT which){
	std::vector<std::string> tokens = tokenize(tag,":");
	TiXmlHandle tagHandle = *storedHandle;
	for (int x = 0; x < (int)tokens.size(); x++) {
		if (x == 0)
			tagHandle = tagHandle.ChildElement(tokens.at(x), which);
		else
			tagHandle = tagHandle.FirstChildElement(tokens.at(x));
	}

	if (tagHandle.ToElement()) {
		TiXmlElement* elem = tagHandle.ToElement();

		// Do stuff with the element here
		for (TiXmlAttribute* a = elem->FirstAttribute(); a; a = a->Next()) {
			if (a->Name() == attribute)
				return true;
		}
	}
	return false;
}

//---------------------------------------------------------
TiXmlElement* XmlSettings::getElementForAttribute(std::string tag, int which){
	std::vector<std::string> tokens = tokenize(tag,":");
	TiXmlHandle tagHandle = *storedHandle;
	for (int x = 0; x < (int)tokens.size(); x++) {
		if (x == 0)
			tagHandle = tagHandle.ChildElement(tokens.at(x), which);
		else
			tagHandle = tagHandle.FirstChildElement(tokens.at(x));
	}
    return tagHandle.ToElement();
}

//---------------------------------------------------------
t_CKINT XmlSettings::getIntAttribute(std::string tag, std::string attribute, int defaultValue, t_CKINT which){
    int value = defaultValue;
	readIntAttribute(tag, attribute, value, which);
	return value;
}

//---------------------------------------------------------
t_CKFLOAT XmlSettings::getFloatAttribute(std::string tag, std::string attribute, t_CKDOUBLE defaultValue, t_CKINT which){
    double value = defaultValue;
	readDoubleAttribute(tag, attribute, value, which);
	return value;
}

//---------------------------------------------------------
std::string XmlSettings::getStringAttribute(std::string tag, std::string attribute, std::string defaultValue, t_CKINT which){
    std::string value = defaultValue;
	readStringAttribute(tag, attribute, value, which);
	return value;
}

//---------------------------------------------------------
t_CKBOOL XmlSettings::readIntAttribute(std::string tag, std::string attribute, int& outValue, t_CKINT which){

    TiXmlElement* elem = getElementForAttribute(tag, which);
    if (elem)
        return (elem->QueryIntAttribute(attribute, &outValue) == TIXML_SUCCESS);
    return false;
}


//---------------------------------------------------------
t_CKBOOL XmlSettings::readDoubleAttribute(std::string tag, std::string attribute, double& outValue, t_CKINT which){

    TiXmlElement* elem = getElementForAttribute(tag, which);
    if (elem)
        return (elem->QueryDoubleAttribute(attribute, &outValue) == TIXML_SUCCESS);
    return false;
}

//---------------------------------------------------------
t_CKBOOL XmlSettings::readStringAttribute(std::string tag, std::string attribute, std::string& outValue, t_CKINT which){

    TiXmlElement* elem = getElementForAttribute(tag, which);
    if (elem)
    {
        const std::string* value = elem->Attribute(attribute);
        if (value)
        {
            outValue = *value;
            return true;
        }
    }
    return false;
}
