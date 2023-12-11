/*-----------------------------------------------------------------------------
 Entaro Adun!

 ChucK Manifold-Interface Amplitude Panning (MIAP).

 Based on the research of Zachary Seldess, as presented in his AES paper
 and conference talk, "MIAP: Manifold-Interface Amplitude Panning in Max/MSP
 and Pure Data", which in turn is based off the work of Steven Ellison and his
 first implementation of "Barycentric Panning."

 YouTube links:
 Part 1: https://www.youtube.com/watch?v=LUHVwQSkv9s
 Part 2: https://www.youtube.com/watch?v=RKvCAvHo7ZI

 Paper:
 Seldess, Zachary. 2014. "MIAP: Manifold-Interface Amplitude Panning
      in Max/MSP and Pure Data." Presentation at the annual conference

 This implementation is pared down from the Max/MSP object (as one would expect
 from a ChucK Chugin). The functionality of the various nodes Seldess describes
 in his paper can be replicated by linking various nodes to another nodes.

 This Chugin is very much open to contributions, and the author would like to see
 it evolve (of course, only if interest is generated, I find the subject
 somewhat fascinating).

 Attribution:
 This Chugin is based off the work of Zachary Seldess' MaxMSP and Pure Date externsl,
 and also based on Meyer Sound’s SpaceMap(R) multi-channel panning feature of CueStation.
 SpaceMap and CueStation are trademarks of Meyer Sound Laboratories, Incorporated.
-----------------------------------------------------------------------------*/

// this should align with the correct versions of these ChucK files
#include "chugin.h"

// general includes
#include <stdio.h>
#include <limits.h>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

// declaration of chugin constructor
CK_DLL_CTOR(miap_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(miap_dtor);

CK_DLL_MFUN(miap_addNode);
CK_DLL_MFUN(miap_updateNode);
CK_DLL_MFUN(miap_linkNodes);
CK_DLL_MFUN(miap_addTriset);
CK_DLL_MFUN(miap_clearTrisets);
CK_DLL_MFUN(miap_clearAll);
CK_DLL_MFUN(miap_generateGrid);

// setters
CK_DLL_MFUN(miap_setPosition);
CK_DLL_MFUN(miap_setConstantPower);
CK_DLL_MFUN(miap_setSquareRoot);
CK_DLL_MFUN(miap_setLinear);

// setters
CK_DLL_MFUN(miap_getPositionX);
CK_DLL_MFUN(miap_getPositionY);
CK_DLL_MFUN(miap_getNodeValue);
CK_DLL_MFUN(miap_getNodeX);
CK_DLL_MFUN(miap_getNodeY);
CK_DLL_MFUN(miap_getActiveTriset);
CK_DLL_MFUN(miap_getActiveNode);
CK_DLL_MFUN(miap_getNumNodes);
CK_DLL_MFUN(miap_getNumTrisets);

// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICK(miap_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT miap_data_offset = 0;

// utility functions
void computeVector(float x1, float y1, float x2, float y2, float *v) {
    v[0] = x1 - x2;
    v[1] = y1 - y2;
}

// Euclidean distance between two coordinates
float distance(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// cosine constant power panning
float cosinePower(float p) {
    return cos((0.5 * M_PI * p) + 1.5 * M_PI);
}

float dotProduct(float v[], float u[], int n) {
    float result = 0.0;

    for (int i = 0; i < n; i++) {
        result += v[i]*u[i];
    }

    return result;
}

// area of a triangle given the lengths of its sides
float heronArea(float a, float b, float c) {
    float s = (a + b + c) * 0.5;
    return sqrt(s * (s - a) * (s - b) * (s - c));
}


struct Node
{
    float x;
    float y;
    float value;

    int index;
    bool active;

    vector<int> linkID;
    vector<float> linkPercentage;
};


class Triset
{

public:

    Triset( Node &n1, Node &n2, Node &n3, int idx)
    : m_n1(&n1), m_n2(&n2), m_n3(&n3), index(idx)
    {
        dot02 = 0.0;
        dot12 = 0.0;

        u = 0.0;
        v = 0.0;

        ap = 0.0;
        bp = 0.0;
        cp = 0.0;

        n1Index = m_n1->index;
        n2Index = m_n2->index;
        n3Index = m_n3->index;

        n1Area = 0.0;
        n2Area = 0.0;
        n3Area = 0.0;

        active = false;

        precalculate();
    }

    Node * m_n1;
    Node * m_n2;
    Node * m_n3;

    int index;
    int active;

    int n1Index;
    int n2Index;
    int n3Index;

    // calculated upon initialization or if the user
    // changes the coordinates of a node
    void precalculate() {
        ab = distance(m_n1->x, m_n1->y, m_n2->x, m_n2->y);
        bc = distance(m_n2->x, m_n2->y, m_n3->x, m_n3->y);
        ca = distance(m_n3->x, m_n3->y, m_n1->x, m_n1->y);

        area = heronArea(ab, bc, ca);
        areaScalar = 1.0/area;

        // a few pointInTriset operations that never change
        computeVector(m_n3->x, m_n3->y, m_n1->x, m_n1->y, v0);
        computeVector(m_n2->x, m_n2->y, m_n1->x, m_n1->y, v1);

        dot00 = dotProduct(v0, v0, 2);
        dot01 = dotProduct(v0, v1, 2);
        dot11 = dotProduct(v1, v1, 2);

        invDenom = 1.0/(dot00 * dot11 - dot01 * dot01);
    }

    // http://blackpawn.com/texts/pointinpoly/
    // many parameters were precalculated in the constructor to save on processing
    // might make the code a bit more obscure
    bool pointInTriset(float x, float y)
    {
        computeVector(x, y, m_n1->x, m_n1->y, v2);

        // vector calculation
        dot02 = dotProduct(v0, v2, 2);
        dot12 = dotProduct(v1, v2, 2);

        // compute barycentric coordinates
        u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        return (u >= 0) && (v >= 0) && ((u + v) < 1);
    }

    // finds the areas of the three triangles that make up the a triset
    // those areas sum to 1.0, and the square root of each is set to be
    // the value for that node
    void setNodes(float x, float y)
    {
        ap = distance(m_n1->x, m_n1->y, x, y);
        bp = distance(m_n2->x, m_n2->y, x, y);
        cp = distance(m_n3->x, m_n3->y, x, y);

        n3Area = heronArea(ab, bp, ap);
        n2Area = heronArea(ca, ap, cp);
        n1Area = area - n3Area - n2Area;

        m_n1->value = n1Area * areaScalar;
        m_n2->value = n2Area * areaScalar;
        m_n3->value = n3Area * areaScalar;
    }

private:

    float v0[2];
    float v1[2];
    float v2[2];

    float dot00;
    float dot01;
    float dot11;
    float invDenom;

    // sides of the triset
    float ab, bc, ca;

    // area of the triest
    float area;
    float areaScalar;

    // evaluated in pointInTriset
    float dot02, dot12;
    float u, v;

    // evaluated in setTrisetNodes
    float ap, bp, cp;
    float n1Area, n2Area, n3Area;
};


// class definition for internal Chugin data
class MIAP
{
public:
    // constructor
    MIAP( t_CKFLOAT fs)
    {
        m_numNodes = 0;
        m_numTrisets = 0;
        m_panningType = CONSTANT_POWER;
    }

    // for Chugins extending UGen
    SAMPLE tick( SAMPLE in )
    {
        return in;
    }

    // set parameter example
    void addNode( t_CKFLOAT x, t_CKFLOAT y )
    {
        Node node;

        node.x = x;
        node.y = y;
        node.value = 0.0;
        node.active = false;
        node.index = m_numNodes;

        m_nodes.push_back( node );

        m_numNodes++;
    }

    void addTriset( t_CKINT n1, t_CKINT n2, t_CKINT n3 )
    {
        // you can't add a triset without at least three nodes
        if (m_numNodes < 3)
        {
            printf("You cannot add a triset without at least three nodes to connect to.");
            return;
        }

        // you can't associate a node to a triset
        // unless it's one of the created nodes
        if (n1 >= m_numNodes || n2 >= m_numNodes || n3 >= m_numNodes)
        {
            printf("You cannot add a triset to a node that has not been created");
            return;
        }

        Triset triset( m_nodes[n1], m_nodes[n2], m_nodes[n3], m_numTrisets );
        triset.index = m_numTrisets;
        m_trisets.push_back( triset );

        m_numTrisets++;
    }

    void clearNodes() {
        m_nodes.erase(m_nodes.begin(), m_nodes.begin() + m_nodes.size());
        m_nodes.clear();
        m_numNodes = 0;
    }

    void clearTrisets() {
        m_trisets.erase(m_trisets.begin(), m_trisets.begin() + m_trisets.size());
        m_trisets.clear();
        m_numTrisets = 0;
    }

    void clearAll() {
        clearTrisets();
        clearNodes();
    }

    // links two nodes, wherein node `a` will send
    // its value to node `b' and the percentage is how
    // much of that value will be added to it
    void linkNodes( t_CKINT a, t_CKINT b, t_CKFLOAT percentage)
    {
        // copy by reference
        vector<int> &linkID = m_nodes[a].linkID;
        vector<float> &linkPercentage = m_nodes[a].linkPercentage;

        // find item position in vector
        int pos = find(linkID.begin(), linkID.end(), b) - linkID.begin();

        // if value is not already in linkID or linkPercentage vectors
        // then we append it to the vector, else we update the percentage
        if (pos == linkID.size()) {
            linkID.push_back(b);
            linkPercentage.push_back(percentage);
        } else {
            linkPercentage[pos] = percentage;
        }
    }

    void setConstantPower()
    {
        m_panningType = CONSTANT_POWER;
    }

    void setSquareRoot()
    {
        m_panningType = SQUARE_ROOT;
    }

    void setLinear()
    {
        m_panningType = LINEAR;
    }

    // main user function for panning, sets the position of the
    // object to be "panned" in xy space
    void setPosition( t_CKFLOAT x, t_CKFLOAT y )
    {
        // clears all node values and active statuses
        clearAllNodeValues();
        updateTrisetNodeValues(x, y);

        // storing our coordinates in case a triset
        // needs to be updated when changing one of
        // its node coordinates
        m_x = x;
        m_y = y;
    }

    // main function that does all the stuff
    void updateTrisetNodeValues(float x, float y)
    {
        // first check if the coordinate is a already a node
        for (int i = 0; i < m_numNodes; i++) {
            if (m_nodes[i].x == x && m_nodes[i].y == y) {
                m_nodes[i].value = 1.0;
                updateNodeLink(&m_nodes[i]);
                return;
            }
        }

        // search for the triset the point falls in
        for (int i = 0; i < m_numTrisets; i++) {
            if (m_trisets[i].pointInTriset(x, y)) {
                m_trisets[i].active = true;
                m_trisets[i].setNodes(x, y);
                updateTrisetLinks(m_trisets[i]);
                return;
            }
        }
    }

    void updateNode( t_CKINT id, t_CKFLOAT x, t_CKFLOAT y )
    {
        // skips if no changes
        if (m_nodes[id].x == x && m_nodes[id].y == y) {
            return;
        }

        m_nodes[id].x = x;
        m_nodes[id].y = y;

        // updates any affected trisets
        for (int i = 0; i < m_numTrisets; i++) {
            if (m_trisets[i].n1Index == id || m_trisets[i].n2Index == id || m_trisets[i].n3Index == id) {
                m_trisets[i].precalculate();
            }
        }

        // update our trisets using our stored coordinates
        clearAllNodeValues();
        updateTrisetNodeValues(m_x, m_y);
    }

    // generates a MIAP grid that is rows by cols grid of Nodes,
    // the nodes are created sequentially from left to right per row,
    // this is a quick way to make a grid of equilateral triangles
    void generateGrid(int rows, int cols)
    {
        float horzLen = 1.0;
        float vertLen = pow(pow(horzLen, 2) - pow(horzLen/2.0, 2), 0.5);

        float inverseMax = 0.0;
        float xCenterNudge = 0.0;
        float yCenterNudge = 0.0;

        if (cols >= rows) {
            inverseMax = 1.0/((cols- 1) * horzLen + horzLen * 0.5);
            yCenterNudge = (1.0 - (((rows - 1) * vertLen) * inverseMax)) * 0.5;
        } else {
            inverseMax = 1.0/((rows - 1) * vertLen);
            xCenterNudge = (1.0 - (((cols - 1) * horzLen + horzLen * 0.5) * inverseMax)) * 0.5;
        }

        // add our nodes
        for (int i = 0; i < rows; i++) {
            float offset = 0.0;
            if (i % 2 != 0) {
                offset = horzLen * 0.5;
            }
            for (int j = 0; j < cols; j++) {
                float x = (j * horzLen + offset) * inverseMax + xCenterNudge;
                float y = i * vertLen * inverseMax + yCenterNudge;
                addNode(x, y);
            }
        }

        // add our trisets
        for (int i = 0; i < rows - 1; i++) {
            for (int j = 0; j < cols - 1; j++) {
                // trisets pointing down
                {
                    int n1 = j + (cols * i);
                    int n2 = n1 + 1;
                    int n3 = j + cols * (i + 1);

                    if (i % 2 == 1) {
                        n3++;
                    }

                    addTriset(n1, n2, n3);
                }
                // trisets pointing up
                {
                    int n1 = j + (cols * i) + 1;
                    int n2 = j + cols * (i + 1);
                    int n3 = n2 + 1;


                    if (i % 2 == 1) {
                        n1--;
                    }

                    addTriset(n1, n2, n3);
                }
            }
        }
    }

    int getActiveTriset()
    {
        for (int i = 0; i < m_numTrisets; i++) {
            if (m_trisets[i].active == true) {
                return i;
            }
        }
        return -1;
    }

    int getActiveNode(int which)
    {
        int idx = getActiveTriset();

        if (which == 0)
        {
             return m_trisets[idx].n1Index;
        }
        else if (which == 1)
        {
             return m_trisets[idx].n2Index;
        }
        else if (which == 2)
        {
             return m_trisets[idx].n3Index;
        }

        return -1;
    }

    float getNodeValue(int idx)
    {
        switch(m_panningType) {

        case CONSTANT_POWER:
            return cosinePower(m_nodes[idx].value);

        case SQUARE_ROOT:
            return sqrt(m_nodes[idx].value);

        case LINEAR:
            return m_nodes[idx].value;
        }
    }

    float getPositionX()
    {
        return m_x;
    }

    float getPositionY()
    {
        return m_y;
    }

    float getNodeX(int idx)
    {
        return m_nodes[idx].x;
    }

    float getNodeY(int idx)
    {
        return m_nodes[idx].y;
    }

    int getNumNodes()
    {
        return m_numNodes;
    }

    int getNumTrisets()
    {
        return m_numTrisets;
    }

private:
    // instance data
    vector<Node> m_nodes;
    vector<Triset> m_trisets;

    int m_numNodes;
    int m_numTrisets;
    float m_x;
    float m_y;

    enum PanningTypes {
        LINEAR = 0,
        CONSTANT_POWER,
        SQUARE_ROOT
    };

    PanningTypes m_panningType;

    void clearAllNodeValues() {
        for (int i = 0; i < m_numNodes; i++) {
            m_nodes[i].value = 0.0;
        }

        for (int i = 0; i < m_numTrisets; i++) {
            m_trisets[i].active = false;
        }
    }

    void updateTrisetLinks(Triset t)
    {
        updateNodeLink(t.m_n1);
        updateNodeLink(t.m_n2);
        updateNodeLink(t.m_n3);
    }

    void updateNodeLink(Node * n) {
        int id = 0;
        float percentage = 0.0;
        float value = 0.0;

        for (int i = 0; i < n->linkID.size(); i++) {
            id = n->linkID.at(i);
            percentage = n->linkPercentage.at(i);

            value = n->value * percentage;
            m_nodes[id].value += value;
        }
    }
};


// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( MIAP )
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "MIAP");

    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "MIAP", "UGen");
    QUERY->doc_class(QUERY, "ChucK Manifold-Interface Amplitude Panning (MIAP). Based on the research \
            of Zachary Seldess, as presented in his AES paper and conference talk, \
            'MIAP: Manifold-Interface Amplitude Panning in Max/MSP and Pure Data', which in turn \
            is based off the work of Steven Ellison and his first implementation of 'Barycentric Panning.'");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, miap_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, miap_dtor);

    // for UGen's only: add tick function
    QUERY->add_ugen_func(QUERY, miap_tick, NULL, 1, 1);

    // NOTE: if this is to be a UGen with more than 1 channel,
    // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
    // and declare a tickf function using CK_DLL_TICKF

    // example of adding setter method
    QUERY->add_mfun(QUERY, miap_addNode, "void", "addNode");
    QUERY->add_arg(QUERY, "float", "x");
    QUERY->add_arg(QUERY, "float", "y");
    QUERY->doc_func(QUERY, "Creates a Node at the (x, y) coordinate.");

    QUERY->add_mfun(QUERY, miap_updateNode, "void", "updateNode");
    QUERY->add_arg(QUERY, "int", "id");
    QUERY->add_arg(QUERY, "float", "x");
    QUERY->add_arg(QUERY, "float", "y");
    QUERY->doc_func(QUERY, "");

    QUERY->add_mfun(QUERY, miap_linkNodes, "void", "linkNodes");
    QUERY->add_arg(QUERY, "int", "a");
    QUERY->add_arg(QUERY, "int", "b");
    QUERY->add_arg(QUERY, "float", "percentage");
    QUERY->doc_func(QUERY, "Connects the output of Node `a` to Node `b`. This is a one-way connection, \
            and allows for the creation of Derived Nodes and Virtual Nodes, as described by Seldess");

    QUERY->add_mfun(QUERY, miap_addTriset, "void", "addTriset");
    QUERY->add_arg(QUERY, "int", "n1");
    QUERY->add_arg(QUERY, "int", "n2");
    QUERY->add_arg(QUERY, "int", "n3");
    QUERY->doc_func(QUERY, "Creates a Triset, which is to be associated to the ID of three Nodes given.");

    QUERY->add_mfun(QUERY, miap_clearTrisets, "void", "clearTrisets");
    QUERY->doc_func(QUERY, "Clears all Trisets and leaves the remaining Nodes.");

    QUERY->add_mfun(QUERY, miap_clearAll, "void", "clearAll");
    QUERY->doc_func(QUERY, "Clears all Trisets and Nodes.");

    QUERY->add_mfun(QUERY, miap_setPosition, "void", "position");
    QUERY->add_arg(QUERY, "float", "x");
    QUERY->add_arg(QUERY, "float", "y");
    QUERY->doc_func(QUERY, "Sets a position inside of the MIAP space, which is placed at the \
            given (x, y) coordinate.");

    QUERY->add_mfun(QUERY, miap_setConstantPower, "void", "setConstantPower");
    QUERY->doc_func(QUERY, "Applies a cosine curve to the output. Recommended for panning.");

    QUERY->add_mfun(QUERY, miap_setSquareRoot, "void", "setSquareRoot");
    QUERY->doc_func(QUERY, "Applies a square root curve to the output.");

    QUERY->add_mfun(QUERY, miap_setLinear, "void", "setLinear");
    QUERY->doc_func(QUERY, "No transformation is applied to the output.");

    QUERY->add_mfun(QUERY, miap_getNodeValue, "float", "nodeValue");
    QUERY->add_arg(QUERY, "int", "idx");
    QUERY->doc_func(QUERY, "Gets the value of the specified Node.");

    QUERY->add_mfun(QUERY, miap_getPositionX, "float", "positionX");
    QUERY->doc_func(QUERY, "Gets the current X position of the MIAP object.");

    QUERY->add_mfun(QUERY, miap_getPositionY, "float", "positionY");
    QUERY->doc_func(QUERY, "Gets the current Y position of the MIAP object.");

    QUERY->add_mfun(QUERY, miap_getNodeX, "float", "nodeX");
    QUERY->add_arg(QUERY, "int", "idx");
    QUERY->doc_func(QUERY, "");

    QUERY->add_mfun(QUERY, miap_getNodeY, "float", "nodeY");
    QUERY->add_arg(QUERY, "int", "idx");
    QUERY->doc_func(QUERY, "");

    QUERY->add_mfun(QUERY, miap_generateGrid, "void", "generateGrid");
    QUERY->add_arg(QUERY, "int", "rows");
    QUERY->add_arg(QUERY, "int", "cols");
    QUERY->doc_func(QUERY, "Creates a grid of equilateral triangles that has the provided number \
            of rows and columns. The coordinates of the Nodes are normalized between [0.0-1.0, 0.0-1.0]");

    QUERY->add_mfun(QUERY, miap_getActiveTriset, "int", "activeTriset");
    QUERY->doc_func(QUERY, "Gets the Triset that is currently active, returns -1 if one is not found.");

    QUERY->add_mfun(QUERY, miap_getActiveNode, "int", "activeNode");
    QUERY->add_arg(QUERY, "int", "index");
    QUERY->doc_func(QUERY, "Gets the ID of one of the Nodes associated with a currently active Triset,\
            the ID given is relative to the Triset, thus the values are restrict to 0, 1, and 2. \
            Returns -1 if one is not found.");

    QUERY->add_mfun(QUERY, miap_getNumNodes, "int", "numNodes");
    QUERY->doc_func(QUERY, "Gets the number of Nodes.");

    QUERY->add_mfun(QUERY, miap_getNumTrisets, "int", "numTrisets");
    QUERY->doc_func(QUERY, "Gets the number of Trisets.");

    // this reserves a variable in the ChucK internal class to store
    // reference to the c++ class we defined above
    miap_data_offset = QUERY->add_mvar(QUERY, "int", "@miap_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(miap_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, miap_data_offset) = 0;

    // instantiate our internal c++ class representation
    MIAP * miap_obj = new MIAP(API->vm->srate(VM));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, miap_data_offset) = (t_CKINT) miap_obj;
}


// implementation for the destructor
CK_DLL_DTOR(miap_dtor)
{
    // get our c++ class pointer
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    // check it
    if( miap_obj )
    {
        // clean up
        delete miap_obj;
        OBJ_MEMBER_INT(SELF, miap_data_offset) = 0;
        miap_obj = NULL;
    }
}


// implementation for tick function
CK_DLL_TICK(miap_tick)
{
    // get our c++ class pointer
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);

    // invoke our tick function; store in the magical out variable
    if(miap_obj) *out = miap_obj->tick(in);

    // yes
    return TRUE;
}

CK_DLL_MFUN(miap_addNode)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKFLOAT x = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT y = GET_NEXT_FLOAT(ARGS);
    miap_obj->addNode(x, y);
}

CK_DLL_MFUN(miap_updateNode)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT id = GET_NEXT_INT(ARGS);
    t_CKFLOAT x = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT y = GET_NEXT_FLOAT(ARGS);
    miap_obj->updateNode(id, x, y);
}

CK_DLL_MFUN(miap_linkNodes)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT a = GET_NEXT_INT(ARGS);
    t_CKINT b = GET_NEXT_INT(ARGS);
    t_CKFLOAT perc = GET_NEXT_FLOAT(ARGS);
    miap_obj->linkNodes(a, b, perc);
}

CK_DLL_MFUN(miap_addTriset)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT n1 = GET_NEXT_INT(ARGS);
    t_CKINT n2 = GET_NEXT_INT(ARGS);
    t_CKINT n3 = GET_NEXT_INT(ARGS);
    miap_obj->addTriset(n1, n2, n3);
}

CK_DLL_MFUN(miap_clearTrisets)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    miap_obj->clearTrisets();
}

CK_DLL_MFUN(miap_clearAll)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    miap_obj->clearAll();
}

CK_DLL_MFUN(miap_setConstantPower)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    miap_obj->setConstantPower();
}

CK_DLL_MFUN(miap_setSquareRoot)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    miap_obj->setSquareRoot();
}

CK_DLL_MFUN(miap_setLinear)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    miap_obj->setLinear();
}

CK_DLL_MFUN(miap_setPosition)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKFLOAT x = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT y = GET_NEXT_FLOAT(ARGS);
    miap_obj->setPosition(x, y);
}

CK_DLL_MFUN(miap_getNodeValue)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT idx = GET_NEXT_INT(ARGS);
    RETURN->v_float = miap_obj->getNodeValue(idx);
}

CK_DLL_MFUN(miap_getPositionX)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    RETURN->v_float = miap_obj->getPositionX();
}

CK_DLL_MFUN(miap_getPositionY)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    RETURN->v_float = miap_obj->getPositionY();
}

CK_DLL_MFUN(miap_getNodeX)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT idx = GET_NEXT_INT(ARGS);
    RETURN->v_float = miap_obj->getNodeX(idx);
}

CK_DLL_MFUN(miap_getNodeY)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT idx = GET_NEXT_INT(ARGS);
    RETURN->v_float = miap_obj->getNodeY(idx);
}

CK_DLL_MFUN(miap_generateGrid)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT rows = GET_NEXT_INT(ARGS);
    t_CKINT cols = GET_NEXT_INT(ARGS);
    miap_obj->generateGrid(rows, cols);
}

CK_DLL_MFUN(miap_getActiveTriset)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    RETURN->v_int = miap_obj->getActiveTriset();
}

CK_DLL_MFUN(miap_getActiveNode)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT idx = GET_NEXT_INT(ARGS);
    RETURN->v_int = miap_obj->getActiveNode(idx);
}

CK_DLL_MFUN(miap_getNumNodes)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    RETURN->v_int = miap_obj->getNumNodes();
}

CK_DLL_MFUN(miap_getNumTrisets)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    RETURN->v_int = miap_obj->getNumTrisets();
}
