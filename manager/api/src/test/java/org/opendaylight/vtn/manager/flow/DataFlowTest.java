package org.opendaylight.vtn.manager.flow;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import org.junit.Test;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;


public class DataFlowTest extends TestBase {
    protected static int ids = 123;
    protected static long created = 14374808;
    protected static short idle = 300;
    protected static short hard = 0;

/*
* Test case for getter methods.
*/
    @Test
    public void testGetter() {
        List<DataFlow> list = new ArrayList<DataFlow>();
        String string1 = "tuuid";
        String string2 = "uuid";
        VNodePath inpath = new VBridgePath(string1, string2);
        Node node = NodeCreator.createOFNode(0L);
        NodeConnector nc = NodeConnectorCreator.
            createOFNodeConnector((short)0, node);
        String name = "data";
        VNodePath outpath = new VBridgePath(string1, string2);
        PortLocation inport = new PortLocation(nc, name);
        PortLocation outport = new PortLocation(nc, name);
        DataFlow dataFlow = new DataFlow(ids, created, idle, hard, inpath, inport,
                                     outpath, outport);
        assertEquals(created, dataFlow.getCreationTime());
        assertEquals(outpath, dataFlow.getEgressNodePath());
        assertEquals(hard, dataFlow.getHardTimeout());
        assertEquals(ids, dataFlow.getFlowId());
        assertEquals(inport, dataFlow.getIngressPort());
        assertEquals(idle, dataFlow.getIdleTimeout());
        assertEquals(outport, dataFlow.getEnressPort());
        ids = -1;
        created = -1;
        idle = 300;
        hard = 45;
        DataFlow dataFlowobj = new DataFlow(ids, created, idle, hard, null, null,
                null, null);
        assertEquals(hard, dataFlowobj.getHardTimeout());
        assertEquals(ids, dataFlowobj.getFlowId());
        assertEquals(null, dataFlowobj.getIngressPort());
        assertEquals(idle, dataFlowobj.getIdleTimeout());
        assertEquals(null, dataFlowobj.getEnressPort());
    }
    /**
     * Test case for {@link FlowStatslist#equals(Object)}
     *
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        ids = 10;
        created = 143456808;
        idle = 234;
        hard = 45;
        String string1 = "uid";
        String string2 = "uuid";
        VNodePath inpath = new VBridgePath(string1, string2);
        Node node = NodeCreator.createOFNode(4L);
        NodeConnector nc = NodeConnectorCreator.createOFNodeConnector(
               (short)4, node);
        String name = "data_1";
        VNodePath outpath = new VBridgePath(string1, string2);
        PortLocation inport = new PortLocation(nc, name);
        PortLocation outport = new PortLocation(nc, name);
        DataFlow dataFlow = new DataFlow(ids, created, idle, hard, inpath,
                inport, outpath, outport);
        toTestEquals(set, dataFlow, new DataFlow(ids, created, idle, hard,
                inpath, inport, outpath, outport));
    }
}
