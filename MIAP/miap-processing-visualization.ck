MIAP m;
MIAPOSCVis v;

4 => int ROWS;
4 => int COLS;

m.generateGrid(ROWS, COLS);

v.addAllNodes(m);
spork ~ v.oscSend(m, 0);

9.9 => float piInc;

while (true) {
    (piInc + 0.005) % (2 * pi) => piInc;
    (Math.sin(piInc) + 1.0) * 0.5 => float x;
    (Math.cos(piInc) + 1.0) * 0.5 => float y;

    x * 0.5 + .25 => x;
    y * 0.5 + .25 => y;

    m.setPosition(x, y);
    v.updatePos(x, y);
    /*for (0 => int i; i < ROWS * COLS; i++) {
        chout <= m.getNodeGain(i) <= "\t";
    }
    chout <= IO.newline();*/
    10::ms => now;
}
