package it.unipi.iot.Server;

import org.eclipse.californium.core.CoapServer;

public class CoAPServer extends CoapServer {
    public CoAPServer() {
        super();
        add(new CoAPRegistration("register"));
        add(new CoAPDiscovery("discovery"));
    }
}
