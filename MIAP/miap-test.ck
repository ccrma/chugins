// Eric Heep
// April 4th, 2017
// miap-test.ck

// unit tests to help smooth out some kinks

class MIAPTest {

    int passedTests, totalTests;
    string errorMessages[0];

    fun void testHeron() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addTriset(0, 1, 2);

        // get sides of the triangle
        (0.0 + 1.0 + 0.0)/3.0 => float posX;
        (0.0 + 0.0 + 1.0)/3.0 => float posY;

        m.position(posX, posY);
        m.nodeValue(0) => float n1;
        m.nodeValue(1) => float n2;
        m.nodeValue(2) => float n3;
        assertAlmostEqual(n1, n2, n3, "testHeron");
    }

    fun void testNumNodes() {
        MIAP m;
        m.addNode(0.0, 0.0);
        assertEqual(m.numNodes(), 1, "testNumNodes");
        m.addNode(1.0, 0.0);
        assertEqual(m.numNodes(), 2, "testNumNodes");
    }

    fun void testUpdateNode() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.updateNode(0, 0.0, 1.0);
        assertEqual(m.numNodes(), 1, "testUpdateNodes");
        assertEqual(m.nodeY(0), 1.0, "testUpdateNodes");
    }

    fun void testUseUpdateNode() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addTriset(0, 1, 2);

        m.position(0.25, 0.25);
        m.nodeValue(0) => float val1;
        m.updateNode(0, 0.1, 0.1);
        m.nodeValue(0) => float val2;

        assertNotEqual(val1, val2, "testUseUpdateNode");
    }

    fun void testNumTrisets() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);
        m.addTriset(0, 1, 2);
        m.addTriset(1, 2, 3);
        assertEqual(m.numTrisets(), 2, "testNumTrisets");
    }

    fun void testGetActiveTriset() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);
        m.addTriset(0, 1, 2);
        m.addTriset(1, 2, 3);
        m.position(0.50, 0.25);
        assertEqual(m.activeTriset(), 0, "testActiveTriset");
        m.position(0.55, 0.75);
        assertEqual(m.activeTriset(), 1, "testActiveTriset");
    }

    fun void testGetActiveNodes() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);
        m.addTriset(0, 1, 2);
        m.addTriset(1, 2, 3);
        m.position(0.50, 0.75);
        assertEqual(m.activeNode(0), 1, "testGetActiveNode");
        assertEqual(m.activeNode(1), 2, "testGetActiveNode");
        assertEqual(m.activeNode(2), 3, "testGetActiveNode");
    }

    fun void testGridSize() {
        MIAP m;
        m.generateGrid(2, 2);
        assertEqual(m.numNodes(), 4, "testGridSize");
        assertEqual(m.numTrisets(), 2, "testGridSize");
    }

    fun void testGetNodeX() {
        MIAP m;
        m.addNode(0.25, 0.75);
        assertEqual(m.nodeX(0), 0.25, "testGetNodeX");
    }

    fun void testGetNodeY() {
        MIAP m;
        m.addNode(0.25, 0.75);
        assertEqual(m.nodeY(0), 0.75, "testGetNodeY");
    }

    fun void testLinkNodes() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addTriset(0, 1, 2);

        m.linkNodes(0, 1, 1.0);
        m.position(0.0, 0.0);
        assertEqual(m.nodeValue(0), m.nodeValue(1), "testLinkNodes");
    }

    fun void testLinkNodesPercentage() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addTriset(0, 1, 2);
        m.setLinear();

        Math.random2f(0.2, 0.8) => float perc;
        m.linkNodes(0, 1, perc);

        m.position(0.0, 0.0);
        assertAlmostEqual(m.nodeValue(0) * perc, m.nodeValue(1), "testLinkNodesPercentage");
    }

    fun void testLinkNodesUpdatePercentage() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addTriset(0, 1, 2);
        m.linkNodes(0, 1, 0.5);
        m.setLinear();

        Math.random2f(0.2, 0.8) => float perc;
        m.linkNodes(0, 1, perc);

        m.position(0.0, 0.0);
        assertAlmostEqual(m.nodeValue(0) * perc, m.nodeValue(1), "testLinkNodesUpdatePercentage");
    }

    fun void testSquareRoot() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addTriset(0, 1, 2);
        m.setLinear();
        m.position(0.25, 0.25);
        m.nodeValue(0) => float linearValue;
        m.setSquareRoot();
        m.nodeValue(0) => float squareRootValue;

        assertAlmostEqual(Math.sqrt(linearValue), squareRootValue, "testSquareRoot");
    }

    fun void testConstantPower() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addTriset(0, 1, 2);
        m.setLinear();
        m.position(0.25, 0.25);
        m.nodeValue(0) => float linearValue;
        m.setConstantPower();
        m.nodeValue(0) => float constantPowerValue;

        // cosine power transform
        Math.cos((0.5 * linearValue * pi) + 1.5 * pi) => linearValue;

        assertAlmostEqual(linearValue, constantPowerValue, "testConstantPower");
    }

    fun void testSquare() {
        MIAP m;
        m.addNode(0.0, 0.0); // 0
        m.addNode(0.0, 1.0); // 1
        m.addNode(1.0, 1.0); // 2
        m.addNode(1.0, 0.0); // 3
        m.addNode(0.5, 0.5); // 4

        m.addTriset(0, 1, 4);
        m.addTriset(1, 2, 4);
        m.addTriset(2, 3, 4);
        m.addTriset(3, 0, 4);

        m.setLinear();
        m.linkNodes(4, 0, 0.25);
        m.linkNodes(4, 1, 0.25);
        m.linkNodes(4, 2, 0.25);
        m.linkNodes(4, 3, 0.25);

        m.position(0.5, 0.5);

        m.nodeValue(0) => float v1;
        m.nodeValue(1) => float v2;
        m.nodeValue(2) => float v3;
        m.nodeValue(3) => float v4;

        assertEqual(v1 + v2 + v3 + v4, 1.0, "testSquare");
    }

    fun void testClearNodes() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.clearAll();

        assertEqual(m.numNodes(), 0, "testClearNodes");
    }

    fun void testAddNodeAfterClear() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.clearAll();

        m.addNode(1.0, 0.0);
        assertEqual(m.numNodes(), 1, "testAddNodeAfterClear");
    }

    fun void testClearTrisets() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);
        m.addTriset(0, 1, 2);
        m.addTriset(1, 2, 3);

        m.clearTrisets();
        assertEqual(m.numTrisets(), 0, "testClearTrisets");
    }

    fun void testAddTrisetAfterClear() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);
        m.addTriset(0, 1, 2);
        m.clearTrisets();

        m.addTriset(1, 2, 3);
        assertEqual(m.numTrisets(), 1, "testAddTrisetAfterClear");
    }

    fun void testUseTrisetAfterClear() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);

        m.addTriset(3, 1, 2);
        m.clearTrisets();

        m.addTriset(0, 1, 2);
        m.addTriset(1, 2, 3);
        m.position(0.0, 0.0);
        assertEqual(m.nodeValue(0), 1.0, "testUseTrisetAfterClear");
    }

    fun void testClearNodesAndTrisets() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);
        m.addTriset(0, 1, 2);
        m.addTriset(1, 2, 3);

        m.clearAll();
        assertEqual(m.numNodes(), 0, "testClearNodesAndTrisets");
        assertEqual(m.numTrisets(), 0, "testClearNodesAndTrisets");
    }

    fun string results() {
        chout <= IO.newline();
        <<< passedTests + "/" + totalTests + " PASSED", "" >>>;
        if (errorMessages.size() > 0) {
            for (0 => int i; i < errorMessages.size(); i++) {
                <<< errorMessages[i], "" >>>;
            }
        }
    }

    fun void update(int bool) {
        totalTests++;
        if (bool) {
            passedTests++;
        }
    }

    fun void logError(string error, string fnName) {
        errorMessages << fnName + " | " + error;
    }

    fun void assertAlmostEqual(float a, float b, string fn) {
        if (Math.fabs(a - b) <= 0.0001) {
            chout <= ".";
            update(true);
        } else {
            chout <= "E";
            logError(a + " " + b + " not almost equal", fn);
            update(false);
        }
        chout.flush();
    }

    fun void assertAlmostEqual(float a, float b, float c, string fn) {
        if (Math.fabs(Math.fabs(a - b) - Math.fabs(a - c)) <= 0.0001) {
            chout <= ".";
            update(true);
        } else {
            chout <= "E";
            logError(a + " " + b + " " + c + " not almost equal", fn);
            update(false);
        }
        chout.flush();
    }

    fun void assertLess(float a, float b, string fn) {
        if (a < b) {
            chout <= ".";
            update(true);
        } else {
            chout <= "E";
            logError(a + " !< " + b, fn);
            update(false);
        }
        chout.flush();
    }

    fun void assertEqual(float a, float b, string fn) {
        if (a == b) {
            chout <= ".";
            update(true);
        } else {
            cherr <= "E";
            logError(a + " does not equal " + b, fn);
            update(false);
        }
        chout.flush();
    }

    fun void assertEqual(int a, int b, string fn) {
        if (a == b) {
            chout <= ".";
            update(true);
        } else {
            cherr <= "E";
            logError(a + " does not equal " + b, fn);
            update(false);
        }
        chout.flush();
    }

    fun void assertNotEqual(float a, float b, string fn) {
        if (a != b) {
            chout <= ".";
            update(true);
        } else {
            cherr <= "E";
            logError(a + " should not equal " + b, fn);
            update(false);
        }
        chout.flush();
    }

    fun void assert(int bool, string fn) {
        if (bool) {
            chout <= ".";
            update(true);
        } else {
            cherr <= "E";
            logError("Does not assert true.", fn);
            update(false);
        }
        chout.flush();
    }
}

MIAPTest m;

m.testHeron();
m.testNumNodes();
m.testUpdateNode();
m.testUseUpdateNode();
m.testNumTrisets();
m.testGridSize();
m.testGetActiveTriset();
m.testGetActiveNodes();
m.testGetNodeX();
m.testGetNodeY();
m.testLinkNodes();
m.testLinkNodesPercentage();
m.testLinkNodesUpdatePercentage();
m.testSquareRoot();
m.testConstantPower();
m.testSquare();

m.testClearNodes();
m.testAddNodeAfterClear();
m.testClearTrisets();
m.testAddTrisetAfterClear();
m.testUseTrisetAfterClear();
m.testClearNodesAndTrisets();

m.results();
