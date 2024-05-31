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

        // Verifica se il parametro "resource" è stato fornito
        if (resource == null) {
            exchange.respond(CoAP.ResponseCode.BAD_REQUEST, "Requested Resource parameter missing");
            return;
        }

        try(Connection connection = Database.getConnection()) {

            // Preparazione della query SQL per recuperare le informazioni
            String query = "SELECT * FROM iot_nodes WHERE resource_exposed = ?";
            PreparedStatement statement = connection.prepareStatement(query);
            statement.setString(1, resource);

            // Esecuzione della query
            ResultSet resultSet = statement.executeQuery();

            // Verifica se sono stati trovati risultati
            if (resultSet.next()) {
                // Costruisci la risposta con i dati ottenuti dal database
                String responseData = resultSet.getString("ip");
                exchange.respond(CoAP.ResponseCode.CONTENT, responseData, MediaTypeRegistry.TEXT_PLAIN);
            } else {
                // Risorsa non trovata
                exchange.respond(CoAP.ResponseCode.NOT_FOUND, "Resource not found");
            }

            // Chiudi la connessione al database
            resultSet.close();
            statement.close();
            connection.close();
        } catch (Exception e) {
            // Gestione degli errori
            exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR, "Internal Server Error");
            e.printStackTrace();
        }
    
    }

}
