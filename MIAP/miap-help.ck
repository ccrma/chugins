// Eric Heep
// April 5th, 2017
// miap-help.ck

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
// setConstantPower()
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



