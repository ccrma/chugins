10 => int NUM_MIAPS;

MIAP m[NUM_MIAPS];

for (0 => int i; i < NUM_MIAPS; i++) {
    m[i].addNode(0.0, 0.0);
    m[i].addNode(5.0, 0.0);
    m[i].addNode(0.0, 5.0);
    m[i].addNode(5.0, 5.0);
    m[i].addTriset(0, 1, 2);
    m[i].addTriset(1, 2, 3);
}

float piInc;

while (true) {
    (piInc + 0.005) % (2 * pi) => piInc;
    (Math.sin(piInc) + 1.0) * 0.5 => float x;
    (Math.cos(piInc) + 1.0) * 0.5 => float y;
    for (0 => int i; i < NUM_MIAPS; i++) {
        m[i].setPosition(x, y);
    }
    // <<< m.getNodeGain(0), m.getNodeGain(1), m.getNodeGain(2), m.getNodeGain(3), x, y >>>;
    1::samp => now;
}
