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
import java.sql.ResultSet;


public class CoAPDiscovery extends CoapResource {

    public CoAPDiscovery(String name){
        super(name);
        // Non-observable resource
        setObservable(false);
    }

    public void handleGET(CoapExchange exchange) {

        String resource = exchange.getRequestOptions().getUriQuery().stream()
                                  .filter(param -> param.startsWith("requested_resource="))
                                  .map(param -> param.substring("requested_resource=".length()))
                                  .findFirst()
                                  .orElse(null);

        // Verifies if the "resource" parameter has been provided
        if (resource == null) {
            exchange.respond(CoAP.ResponseCode.BAD_REQUEST, "Requested Resource parameter missing");
            return;
        }

        try(Connection connection = Database.getConnection()) {

            // Preparing the SQL query to retrieve the information
            String query = "SELECT * FROM iot_nodes WHERE resource_exposed = ?";
            PreparedStatement statement = connection.prepareStatement(query);
            statement.setString(1, resource);

            // Execute the query
            ResultSet resultSet = statement.executeQuery();

            // Verifies if results have been found
            if (resultSet.next()) {
                // Build the response with the data obtained from the database
                String responseData = resultSet.getString("ip");
                exchange.respond(CoAP.ResponseCode.CONTENT, responseData, MediaTypeRegistry.TEXT_PLAIN);
            } else {
                // Resource not found
                exchange.respond(CoAP.ResponseCode.NOT_FOUND, "Resource not found");
            }

            // Close the connection to the database
            resultSet.close();
            statement.close();
            connection.close();

        } catch (Exception e) {
            // Error handling
            exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR, "Internal Server Error");
            e.printStackTrace();
        }
    
    }

}
