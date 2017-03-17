MIAP m;

// m.addTriset(0, 1, 2);
// m.addTriset(0, 1, 2);
// m.addTriset(0, 1, 2);
m.addNode(0.0, 0.0);
m.addNode(1.0, 0.0);
m.addNode(0.0, 1.0);

<<< m.getNodeX(0), m.getNodeY(0) >>>;
<<< m.getNodeX(1), m.getNodeY(1) >>>;
<<< m.getNodeX(2), m.getNodeY(2) >>>;

m.addTriset(0, 1, 2);

while (true) {
    m.setPosition(Math.random2f(0.0, 0.5), Math.random2f(0.0, 0.5));
    <<< m.getNodeGain(0) >>>;
    10::ms => now;
}
