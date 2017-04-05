// Eric Heep
// March 8th, 2017

// quick class for visualizing while composing
// pairs with the miap_visualization.pde Processsing file

public class MIAPOSCVis {
    OscOut out;
    out.dest("127.0.0.1", 12000);

    0.0 => float xPos;
    0.0 => float yPos;

    public void updatePos(float x, float y) {
        x => xPos;
        y => yPos;
    }

    public void addAllNodes(MIAP m) {
        for (0 => int i; i < m.numNodes(); i++) {
            out.start("/coord");
            out.add(i);
            out.add(m.getNodeX(i));
            out.add(m.getNodeY(i));
            out.send();

            out.start("/gain");
            out.add(i);
            out.add(m.getNodeValue(i));
            out.send();
        }
    }

    public void updateNonZeroNodes(MIAP m) {
        for (0 => int i; i < m.numNodes(); i++) {
            if (m.getNodeValue(i) > 0) {
                out.start("/gain");
                out.add(i);
                out.add(m.getNodeValue(i));
                out.send();
            }
        }
    }

    public void oscSend(MIAP m, int voice) {
        while (true) {
            out.start("/pos");
            out.add(0);
            out.add(xPos);
            out.add(yPos);
            out.send();

            if (m.getActiveTriset() >= 0) {
                out.start("/active");
                out.add(1);
                out.send();

                int activeNodes[3];
                m.getActiveNode(0) => activeNodes[0];
                m.getActiveNode(1) => activeNodes[1];
                m.getActiveNode(2) => activeNodes[2];

                for (0 => int i; i < activeNodes.size(); i++) {
                    out.start("/activeCoord");
                    out.add(i);
                    out.add(m.getNodeX(activeNodes[i]));
                    out.add(m.getNodeY(activeNodes[i]));
                    out.send();
                }
            }
            else {
                out.start("/active");
                out.add(0);
                out.send();
            }
            updateNonZeroNodes(m);
            second/30.0 => now;
        }
    }
}
