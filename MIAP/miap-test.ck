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
        m.setPosition(posX, posY);
        m.getNodeValue(0) => float n1;
        m.getNodeValue(1) => float n2;
        m.getNodeValue(2) => float n3;
        assertAlmostEqual(n1, n2, n3, "testHeron");
    }

    fun void testNumNodes() {
        MIAP m;
        m.addNode(0.0, 0.0);
        assert(m.numNodes() == 1, "testNumNodes");
        m.addNode(1.0, 0.0);
        assert(m.numNodes() == 2, "testNumNodes");
    }

    fun void testNumTrisets() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);
        m.addTriset(0, 1, 2);
        m.addTriset(1, 2, 3);
        assert(m.numTrisets() == 2, "testNumTrisets");
    }

    fun void testGetActiveTriset() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);
        m.addTriset(0, 1, 2);
        m.addTriset(1, 2, 3);
        m.setPosition(0.50, 0.25);
        assert(m.getActiveTriset() == 0, "testActiveTriset");
        m.setPosition(0.55, 0.75);
        assert(m.getActiveTriset() == 1, "testActiveTriset");
    }

    fun void testGetActiveNodes() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addNode(1.0, 1.0);
        m.addTriset(0, 1, 2);
        m.addTriset(1, 2, 3);
        m.setPosition(0.50, 0.75);
        assert(m.getActiveNode(0) == 1, "testGetActiveNode");
        assert(m.getActiveNode(1) == 2, "testGetActiveNode");
        assert(m.getActiveNode(2) == 3, "testGetActiveNode");
    }

    fun void testGridSize() {
        MIAP m;
        m.generateGrid(2, 2);
        assert(m.numNodes() == 4, "testGridSize");
        assert(m.numTrisets() == 2, "testGridSize");
    }

    fun void testGetNodeX() {
        MIAP m;
        m.addNode(0.25, 0.75);
        assert(m.getNodeX(0) == 0.25, "testGetNodeX");
    }

    fun void testGetNodeY() {
        MIAP m;
        m.addNode(0.25, 0.75);
        assert(m.getNodeY(0) == 0.75, "testGetNodeY");
    }

    fun void testLinkNodes() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addTriset(0, 1, 2);

        m.linkNodes(0, 1, 1.0);
        m.setPosition(0.0, 0.0);
        assert(m.getNodeValue(0) == m.getNodeValue(1), "testLinkNodes");
    }

    fun void testLinkNodesPercentage() {
        MIAP m;
        m.addNode(0.0, 0.0);
        m.addNode(1.0, 0.0);
        m.addNode(0.0, 1.0);
        m.addTriset(0, 1, 2);
        Math.random2f(0.2, 0.8) => float perc;
        m.linkNodes(0, 1, perc);
        m.setPosition(0.0, 0.0);
        assertAlmostEqual(m.getNodeValue(0) * perc, m.getNodeValue(1), "testLinkNodes");
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
            logError(a + " " + b + "not almost equal", fn);
        }
        chout.flush();
    }

    fun void assertAlmostEqual(float a, float b, float c, string fn) {
        if (Math.fabs(Math.fabs(a - b) - Math.fabs(a - c)) <= 0.0001) {
            chout <= ".";
            update(true);
        } else {
            chout <= "E";
            logError(a + " " + b + " " + c + "not almost equal", fn);
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
        }
        chout.flush();
    }
}

MIAPTest m;

m.testHeron();
m.testNumNodes();
m.testNumTrisets();
m.testGridSize();
m.testGetActiveTriset();
m.testGetActiveNodes();
m.testGetNodeX();
m.testGetNodeY();
m.testLinkNodes();
m.testLinkNodesPercentage();

m.results();
