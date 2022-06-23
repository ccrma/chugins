## DbBox2D API for ChucK

### DbBox2D API for ChucK

We expose a _subset_ of the entire [Box2D API](https://box2d.org/documentation/md__d_1__git_hub_box2d_docs_dynamics.html)  to simplify the 
programming model and to target expected use-cases like 
[sonification](https://en.wikipedia.org/wiki/Sonification).

Many Box2D parameters are 2 or 3 dimensional. Rather than
represent vectors as lists of floats, we leverage
ChucK's builtin types, `complex` and `vec3` to
improve interface expressivity.  

Currently we don't expose body or joint deletion to minimize
the instability. The memory associated with a previous world 
is released on `worldBegin` and when the DbBox2D goes out of scope.

#### Constants

| Member                  | Description                                                            |
| :---------------------- | :--------------------------------------------------------------------- |
| `int staticType` (0)    | One of 3 body types. Used for rooms, rocks, etc.                       |
| `int kinematicType` (1) | One of 3 body types. Used for platforms. (ie: moving but "immovable"). |
| `int dynamicType` (2)   | One of 3 body types. Used for moving body. (ie: most of them).         |
| `float degToRad`        | Constant to convert from degrees to radians.                           |
| `float radToDeg`        | Constant to convert from radians to degrees.                           |

#### Scene Construction

New shapes are usually created between calls to `worldBegin` 
and `worldEnd`.  Each shape is assigned a shape id which can be used to query the current
state of various parameters.  The `bodyType` parameter is can be one of
of three values `staticType`, `kinematicType`, or `dynamicType`, available
as a member variable off the DbBox2D instance.  

| Method                                                                                             | Description                                                                       |
| :------------------------------------------------------------------------------------------------- | :-------------------------------------------------------------------------------- |
| `worldBegin(complex gravity)`                                                                      | cancels current world and starts a new one.                                       |
| `worldEnd()`                                                                                       | finishes the current world definition. Simulation can now happen.                 |
| `int newPoint(complex pos)`                                                                        | creates a static point. Often used as joint anchor point.                         |
| `int newEdge(complex p1, complex p2)`                                                              | creates a solid, two-sided static line. Often used for floors and walls.          |
| `int newCircle(complex pos, float radius, float density, int bodyType)`                            | creates a solid circle.                                                           |
| `int newTriangle(complex p1, complex p2, complex p3, float density, int bodyType)`                 | creates a solid triangle.                                                         |
| `int newTrianglePos(complex p1, complex p2, complex p3, complex pos, float density, int bodyType)` | alternate triangle creator whose coords are relative to pos                       |
| `int newPolygon(complex pts[], complex pos, float density, int bodyType)`                          | creates a solid polygon given an array of points.  Polygons must be convex.      |
| `int newRoom(complex pos, complex size, float density, int bodyType)`                              | creates a "room" comprised of 4 rectangles.  Usually staticType or kinematicType. |

Joints can be added to dynamically constrain bodies in the simulation.

| Method                                                                                                                                           | Description                                     |
| :----------------------------------------------------------------------------------------------------------------------------------------------- | :---------------------------------------------- |
| `int newRevoluteJoint(int bodyA, int bodyB, complex localAnchorA, complex localAnchorB, float refAngle, float motorSpeed, float maxMotorTorque)` | creates a revolute joint. Returns a _joint_ id. |
| `int newDistanceJoint(int bodyA, int bodyB, complex localAnchorA, complex localAnchorB, float freqHz, float damping)`                            | creates a distance joint. When freqHz != 0 we creat a spring joint. |

#### Scene Queries

Scene queries are useful for delivering internal scene state to interested
parties. For example, it is used to implement Fiddle's Box2D plotting surface.

| Member                  | Description                                                            |
| :---------------------- | :--------------------------------------------------------------------- |
| `int getNumBodies()`                                          | returns the total number of bodies in the world.                                                                         |
| `int getNumBodyShapes(int bodyId)`                            | returns the number of shapes associated with the identified body. Usually 1.                                             |
| `int getBodyShapeType(int bodyId, int shapeIndex)`            | returns the body shape type: 0: circle, 1: edge, 2:polygon, 3:chain.                                                     |
| `float getCircleRadius(int bodyId, int shapeIndex)`           | returns the radius of the identified circle.                                                                             |
| `getEdgePoints(int bodyId, int shapeIndex, complex pts[])`    | returns the 2 points of the identified edge.                                                                             |
| `getPolygonPoints(int bodyId, int shapeIndex, complex pts[])` | returns the n points of the identified polygon.                                                                          |
| `getChainPoints(int bodyId, int shapeIndex, complex pts[])`   | returns the n points of the identified chain.                                                                            |
| `int getNumJoints()`                                          | returns the total number of joints in the world.                                                                         |
| `complex getJointBodies(int jointId)`                         | returns two bodyIds associated with the enumerated joint (stuff into complex).                                           |

#### Simulation Query + Control

Once you have created a world you can control and query the simulation.
Object properties can be changed during the simulation. To modify object
properities its class (Body, Contact, Joint) couples with its id to uniquely
identify the object.

| Method                                                        | Description                                                                                                              |
| :------------------------------------------------------------ | :----------------------------------------------------------------------------------------------------------------------- |
| `step(dur amt)`                                               | causes the simulation to proceed to its next step. amt is typically 1/60th of a second.                                  |
| `dur getAvgSimTime()`                                         | returns the average measured compute time for a simulation step.                                                         |
| `complex getPosition(int id)`                                 | returns the position of the identified body.                                                                             |
| `complex getVelocity(int id)`                                 | returns the linear velocity of the identified body.                                                                      |
| `float getAngle(int id)`                                      | returns the angle (in radians) of the identified body.                                                                   |
| `complex getAngularVelocity(int id)`                          | returns the angular velocity of the identified body.                                                                     |
| `int getNumContacts()`                                        | returns the number of contact events that occured in the last frame.                                                     |
| `vec3 getContact(int id)`                                     | returns the contact info for the identified _contact_. 3 values are stuffed into the vec3: shapeAId, shapeBId, touching. |
| `setGravity(complex g)`                                       | allows you to change gravity during the simulation.                                                                      |
| `setFriction(int id, float x)`                                | change the friction value for identified body.                                                                           |
| `setDensity(int id, float x)`                                 | change the density value for identified body.                                                                            |
| `setRestitution(int id, float x)`                             | change the restitution (bounciness) value for identified body.                                                           |

#### Custom Dynamics and the Event Callback

To programatically exert forces acting on one or more bodies in the world, 
use these force-modification entrypoints to target the scene changes.

| Member                  | Description                                                            |
| :---------------------- | :--------------------------------------------------------------------- |
| `applyForce(int bodyId, complex force, complex pt)`           | apply a force to the identified body at the worldspace pt.                                                               |
| `applyImpulse(int bodyId, complex force)`                     | apply an impulse force to the identified body. Force is applyied at body center.                                         |
| `applyTorque(int bodyId, float torque)`                       | apply a torque to the identified body.                                                                                   |
| `applyAngularImpulse(int bodyId, float torque)`               | apply an angular impulse force to the identified body.                                                                   |
