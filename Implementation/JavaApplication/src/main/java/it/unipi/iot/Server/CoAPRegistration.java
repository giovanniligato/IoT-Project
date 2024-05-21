package it.unipi.iot.Server;

import com.google.gson.Gson;
import it.unipi.iot.Server.Driver.Database;
import it.unipi.iot.Server.JSON.RegistrationRequest;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;

import java.sql.Connection;
import java.sql.PreparedStatement;

public class CoAPRegistration extends CoapResource {

    public CoAPRegistration(String name){
        super(name);
        // Non-observable resource
        setObservable(false);
    }

    public void handlePOST(CoapExchange exchange) {

        byte[] request = exchange.getRequestPayload();

        String resourceExposed = new String(request);

/*        String json = new String(request);

        // Parse JSON
        Gson gson = new Gson();
        try {
            RegistrationRequest registrationRequest = gson.fromJson(json, RegistrationRequest.class);
        }
        catch (Exception e) {
            exchange.respond(CoAP.ResponseCode.BAD_REQUEST);
            return;
        }
*/
        // IP address of the iot_node
        String ip = exchange.getSourceAddress().toString().substring(1);

        try(Connection connection = Database.getConnection()) {
            PreparedStatement ps = connection.prepareStatement("REPLACE INTO iot_nodes (ip, resource_exposed) VALUES (?, ?)");
            ps.setString(1, ip);
            ps.setString(2, resourceExposed);
            ps.executeUpdate();
            if (ps.getUpdateCount() < 1) {
                exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
            } else {
                exchange.respond(CoAP.ResponseCode.CREATED);
            }
        } catch (Exception e) {
            exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
        }
    }

}
