// Eric Heep
// April 5th, 2017
// miap-help.ck

// Attribution:
// This Chugin is based off the work of Zachary Seldess' MaxMSP and Pure Date externsl,
// and also based on Meyer Sound’s SpaceMap(R) multi-channel panning feature of CueStation.
// SpaceMap and CueStation are trademarks of Meyer Sound Laboratories, Incorporated.

// ~ primary functions

// The following functions are all you really need to know to use MIAP. Essentially, you first
// must place your Nodes in your MIAP space, and then associate those Nodes with a Triset. You
// can then set ta parameter of a ChucK UGen to accept values from those Nodes.

// addNode(float x, float y)
//   Creates a Node at the (x, y) coordinate.
//
// addTriset(int n1, int n2, int n3)
//   Creates a Triset, which is to be associated to the ID of three Nodes given.
//
// position(float x, float y)
//   Sets a position inside of the MIAP space, which is placed at the given (x, y) coordinate.
//
// nodeValue(int id)
//   Gets the value of the specified Node.
//
// setConstantPower() (default)
//   Applies a cosine curve to the output. Recommended for panning.
//
// setSquareRoot()
//   Applies a square root curve to the output.
//
// setLinear()
//   No transformation is applied to the output.

// ~ other functions

// linkNodes(int a, int b, float percentage)
//   Connects the output of Node `a` to Node `b`. This is a one-way connection, and allows
//   for the creation of Derived Nodes and Virtual Nodes, as described by Seldess.
//
// activeTriset()
//   Gets the Triset that is currently active, returns -1 if one is not found.
//
// activeNode(int ID)
//   Gets the ID of one of the Nodes associated with a currently active Triset, the ID
//   given is relative to the Triset, thus the values are restrict to 0, 1, and 2.
//   Returns -1 if one is not found.
//
// numNodes()
//   Gets the number of Nodes.
//
// numTrisets()
//   Gets the number of Trisets.
//
// nodeX(int ID);
//   Gets the X Coordinate of the Node given its ID.
//
// nodeY(int ID);
//   Gets the X Coordinate of the Node given its ID.
//
// generateGrid(int rows, int cols);
//   Creates a grid of equilateral triangles that has the provided number of rows and columns.
//   The edges of the grid are normalized from [0.0-1.0, 0.0-1.0].

MIAP m;

// lets add some nodes, their ID is based on their order of creation

m.addNode(0.0, 0.0); // Node 0
m.addNode(1.0, 0.0); // Node 1
m.addNode(1.0, 1.0); // Node 2
m.addNode(0.0, 1.0); // Node 3

// we've just created a square that with coordinates falling between [0.0-1.0, 0.0-1.0]

/*
    0 - [0.0, 0.0]                      1 - [0.0, 1.0]
                    *--------------*
                    |              |
                    |              |
                    |              |
                    |              |
                    |              |
                    |              |
                    |              |
                    *--------------*
    3 - [0.0, 1.0]                       2 - [1.0, 1.0]
*/

// and now we can associate Trisets with these Nodes
m.addTriset(0, 1, 2);
m.addTriset(2, 3, 0);

// this creates two triangles inside that square

/*
    0 - [0.0, 0.0]                      1 - [0.0, 1.0]
                    *--------------*
                    |..            |
                    |  ..          |
                    |    ..        |
                    |      ..      |
                    |        ..    |
                    |          ..  |
                    |            ..|
                    *--------------*
    3 - [0.0, 1.0]                       2 - [1.0, 1.0]
*/

// now if we set our position at 0.0, Node[0] will be at full gain
m.position(0.0, 0.0);
<<< "x = 0.0, y = 0.0 |", "Node Values:", m.nodeValue(0), m.nodeValue(1), m.nodeValue(2), m.nodeValue(3), "" >>>;

// and if we place it in between, both node 1 and 2 will be at half volume with a constant power curve
// (constant power panning is on by default)
m.position(0.5, 0.0);
<<< "x = 0.5, y = 0.0 |", "Node Values:", m.nodeValue(0), m.nodeValue(1), m.nodeValue(2), m.nodeValue(3), "" >>>;

// if we had a four channel setup, we could easily pan between the four by simply moving the position around,
// the only problem is merely two trisets are not enough to create a spatial presence in the middle of the square,
// let's clear the trisets, and add a "virtual" node in the center of the square
m.clearTrisets();
m.addNode(0.5, 0.5);

/*
    0 - [0.0, 0.0]                      1 - [0.0, 1.0]
                    *---------------*
                    |               |
                    |               |
                    | 4 - [0.5, 0.5]|
                    |       *       |
                    |               |
                    |               |
                    |               |
                    *---------------*
    3 - [0.0, 1.0]                       2 - [1.0, 1.0]
*/

// and lets make some new trisets that all include one side and the center node
m.addTriset(0, 1, 4);
m.addTriset(1, 2, 4);
m.addTriset(2, 3, 4);
m.addTriset(0, 3, 4);

/*
                    *---------------*
                    |..           ..|
                    |  ..       ..  |
                    |    ..   ..    |
                    |       *       |
                    |    ..   ..    |
                    |  ..       ..  |
                    |..           ..|
                    *---------------*
*/

// and if we link the center node to the outside nodes, and give them a quarter of the signal
// we'll influence the outside nodes with the value from the inside node, all while retaining
// constant power panning
m.linkNodes(4, 0, 0.25);
m.linkNodes(4, 1, 0.25);
m.linkNodes(4, 2, 0.25);
m.linkNodes(4, 3, 0.25);

m.position(0.5, 0.5);
<<< "x = 0.5, y = 0.5 |", "Node Values:", m.nodeValue(0), m.nodeValue(1), m.nodeValue(2), m.nodeValue(3), "" >>>;
<<< " -- Hooking up SinOscs to pan around the square --", "" >>>;
3::second => now;

// we'll pan around the space to show the values change in real time
SinOsc sinX => blackhole;
SinOsc sinY => blackhole;

while (true) {
    // panning at slightly different times
    sinX.freq(0.01);
    sinY.freq(0.0125);

    (sinX.last() + 1.0) * 0.5 => float x;
    (sinY.last() + 1.0) * 0.5 => float y;

    m.position(x, y);

    100::ms => now;
    // values to use for panning or any other type of parametization
    <<< "x =", x, "y =", y, "|", "Node Values:", m.nodeValue(0), m.nodeValue(1), m.nodeValue(2), m.nodeValue(3), "" >>>;
}
