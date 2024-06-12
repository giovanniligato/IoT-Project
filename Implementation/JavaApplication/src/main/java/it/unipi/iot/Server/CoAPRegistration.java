package it.unipi.iot.Server;

import com.google.gson.Gson;
import it.unipi.iot.Server.Driver.Database;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;

import java.sql.Connection;
import java.sql.PreparedStatement;

import it.unipi.iot.UserApplication.UserApplication;


public class CoAPRegistration extends CoapResource {

    public CoAPRegistration(String name){
        super(name);
        // Non-observable resource
        setObservable(false);
    }

    public void handlePOST(CoapExchange exchange) {

        byte[] request = exchange.getRequestPayload();

        String resourceExposed = new String(request);

        // IP address of the iot_node
        String ip = exchange.getSourceAddress().toString().substring(1);

        try(Connection connection = Database.getConnection()) {

            // Preparing the SQL query to register the iot_node
            PreparedStatement ps = connection.prepareStatement("REPLACE INTO iot_nodes (ip, resource_exposed) VALUES (?, ?)");
            ps.setString(1, ip);
            ps.setString(2, resourceExposed);

            // Execute the query
            ps.executeUpdate();

            // Verifies if the registration has been successful
            if (ps.getUpdateCount() < 1) {
                // Registration failed
                exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
            } else {
                // Registration successful
                exchange.respond(CoAP.ResponseCode.CREATED);

                // Initialize and start observing the resource
                if(resourceExposed.equals("temperatureandhumidity") || resourceExposed.equals("co") || resourceExposed.equals("hvac")) {
                    CoapObserver observer = new CoapObserver(ip, resourceExposed);
                    observer.startObserving();
                }

                if(resourceExposed.equals("movement")) {
                    // Initializing the UserApplication
                    UserApplication.initializeUri("coap://[" + ip + "]:5683/movement");
                }

            }
        } catch (Exception e) {
            // Error handling
            exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
        }

    }

}
