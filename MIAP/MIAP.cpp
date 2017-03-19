// Eric Heep
// March 15th, 2017

// Manifold-Something Amplitude Panning
// Meyer Sounds's SpaceMap abstract spatialization.

// Seldess, Zachary. 2014. "MIAP: Manifold-Interface Amplitude Panning
//      in Max/MSP and Pure Data." Presentation at the annual conference

// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"

// general includes
#include <stdio.h>
#include <limits.h>
#include <vector>
#include <cmath>

using namespace std;

// declaration of chugin constructor
CK_DLL_CTOR(miap_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(miap_dtor);

// example of getter/setter
CK_DLL_MFUN(miap_addNode);
CK_DLL_MFUN(miap_addTriset);
CK_DLL_MFUN(miap_numNodes);
CK_DLL_MFUN(miap_numTrisets);
CK_DLL_MFUN(miap_setPosition);
CK_DLL_MFUN(miap_getNodeGain);
CK_DLL_MFUN(miap_getNodeX);
CK_DLL_MFUN(miap_getNodeY);
CK_DLL_MFUN(miap_generateGrid);
CK_DLL_MFUN(miap_getActiveTriset);
CK_DLL_MFUN(miap_getActiveNode);

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
    float gain;

    int index;
    bool active;
};


class Triset
{

Node * m_n1;
Node * m_n2;
Node * m_n3;

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

    int index;
    int active;

    int n1Index;
    int n2Index;
    int n3Index;

    // http://blackpawn.com/texts/pointinpoly/
    // many values were precalculated in the constructor to save on processing
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
    // the gain for that node
    void setNodes(float x, float y)
    {
        ap = distance(m_n1->x, m_n1->y, x, y);
        bp = distance(m_n2->x, m_n2->y, x, y);
        cp = distance(m_n3->x, m_n3->y, x, y);

        n3Area = heronArea(ab, bp, ap);
        n2Area = heronArea(ca, ap, cp);
        n1Area = area - n3Area - n2Area;

        m_n1->gain = cosinePower(n1Area * areaScalar);
        m_n2->gain = cosinePower(n2Area * areaScalar);
        m_n3->gain = cosinePower(n3Area * areaScalar);
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
        node.gain = 0.0;
        node.active = false;
        node.index = m_numNodes;

        m_nodes.push_back( node );

        m_numNodes++;
    }

    void addTriset( t_CKINT n1, t_CKINT n2, t_CKINT n3 )
    {
        if (m_numNodes < 3)
        {
            // add error condition here eventually, you can't
            // add a triset without at least three nodes
            return;
        }

        if (n1 >= m_numNodes || n2 >= m_numNodes || n3 >= m_numNodes)
        {
            // add error condition here eventually, you can't
            // associate a node to a triset unless it's one of the
            // created nodes
            return;
        }

        Triset triset( m_nodes[n1], m_nodes[n2], m_nodes[n3], m_numTrisets );

        triset.index = m_numTrisets;

        m_trisets.push_back( triset );

        m_numTrisets++;
    }

    // main user function for panning, sets the position of the
    // object to be "panned" in xy space
    void setPosition(float x, float y) {
        // getActiveTriset() => int possiblePrevTriset;;

        // checks to see if the active triset is the current
        // triset, cuts down on processing in the common case
        // that the position is still inside a triset
        // if (possiblePrevTriset >= 0) {
        //     if (pointInTriset(pos, trisets[possiblePrevTriset])) {
        //         setTrisetNodes(pos, trisets[possiblePrevTriset]);
        //         return;
        //     }
        // }

        // if it is a new triset, we clear the active trisets,
        // and then scan all the trisets to find where the position
        clearAll();

        // if the position (derived node) does not fall in the
        // previous triset, then we scan to see if it falls
        // inside of a new triset
        for (int i = 0; i < m_numTrisets; i++) {
            if (m_trisets[i].pointInTriset(x, y)) {
                m_trisets[i].active = true;
                m_trisets[i].setNodes(x, y);
                break;
            }
        }
    }

    // generates a MIAP grid that is rows by cols grid of Nodes
    // the nodes are creating sequentially from left to right per row
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
                    int n3 = n2+ 1;


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
        for (int i = 0; i < m_numNodes; i++) {
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

    float getNodeGain(int idx)
    {
        return m_nodes[idx].gain;
    }

    float getNodeX(int idx)
    {
        return m_nodes[idx].x;
    }

    float getNodeY(int idx)
    {
        return m_nodes[idx].y;
    }

    int numNodes()
    {
        return m_numNodes;
    }

    int numTrisets()
    {
        return m_numTrisets;
    }

private:
    // instance data
    vector<Node> m_nodes;
    vector<Triset> m_trisets;

    int m_numNodes;
    int m_numTrisets;

    void clearAll() {
        for (int i = 0; i < m_numNodes; i++) {
            m_nodes[i].gain = 0.0;
        }

        for (int i = 0; i < m_numTrisets; i++) {
            m_trisets[i].active = false;
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

    QUERY->add_mfun(QUERY, miap_addTriset, "void", "addTriset");
    QUERY->add_arg(QUERY, "int", "n1");
    QUERY->add_arg(QUERY, "int", "n2");
    QUERY->add_arg(QUERY, "int", "n3");

    QUERY->add_mfun(QUERY, miap_setPosition, "void", "setPosition");
    QUERY->add_arg(QUERY, "float", "x");
    QUERY->add_arg(QUERY, "float", "y");

    QUERY->add_mfun(QUERY, miap_getNodeGain, "float", "getNodeGain");
    QUERY->add_arg(QUERY, "int", "idx");

    QUERY->add_mfun(QUERY, miap_getNodeX, "float", "getNodeX");
    QUERY->add_arg(QUERY, "int", "idx");

    QUERY->add_mfun(QUERY, miap_getNodeY, "float", "getNodeY");
    QUERY->add_arg(QUERY, "int", "idx");

    QUERY->add_mfun(QUERY, miap_generateGrid, "void", "generateGrid");
    QUERY->add_arg(QUERY, "int", "rows");
    QUERY->add_arg(QUERY, "int", "cols");

    QUERY->add_mfun(QUERY, miap_getActiveTriset, "int", "getActiveTriset");

    QUERY->add_mfun(QUERY, miap_getActiveTriset, "int", "getActiveNode");
    QUERY->add_arg(QUERY, "int", "index");

    QUERY->add_mfun(QUERY, miap_numNodes, "int", "numNodes");

    QUERY->add_mfun(QUERY, miap_numTrisets, "int", "numTrisets");

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
    MIAP * miap_obj = new MIAP(API->vm->get_srate());

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

CK_DLL_MFUN(miap_addTriset)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT n1 = GET_NEXT_INT(ARGS);
    t_CKINT n2 = GET_NEXT_INT(ARGS);
    t_CKINT n3 = GET_NEXT_INT(ARGS);
    miap_obj->addTriset(n1, n2, n3);
}

CK_DLL_MFUN(miap_setPosition)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKFLOAT x = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT y = GET_NEXT_FLOAT(ARGS);
    miap_obj->setPosition(x, y);
}

CK_DLL_MFUN(miap_getNodeGain)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    t_CKINT idx = GET_NEXT_INT(ARGS);
    RETURN->v_float = miap_obj->getNodeGain(idx);
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

CK_DLL_MFUN(miap_numNodes)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    RETURN->v_int = miap_obj->numNodes();
}

CK_DLL_MFUN(miap_numTrisets)
{
    MIAP * miap_obj = (MIAP *) OBJ_MEMBER_INT(SELF, miap_data_offset);
    RETURN->v_int = miap_obj->numTrisets();
}