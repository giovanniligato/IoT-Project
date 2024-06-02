package it.unipi.iot.Server;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.network.config.NetworkConfig;
import it.unipi.iot.Server.JSON.SenMLParser;
import it.unipi.iot.Server.Driver.Database;
import java.sql.Connection;
import java.sql.PreparedStatement;

import java.util.ArrayList;
import java.util.List;

public class CoapObserver {
    private CoapClient client;
    private CoapObserveRelation relation;
    
    private final String query;

    public CoapObserver(String ip, String resourceExposed) {
        String uri = "coap://[" + ip + "]/" + resourceExposed;
        client = new CoapClient(uri);
        
        switch(resourceExposed) {
            case "temperatureandhumidity":
                query = "INSERT INTO temphum_sensor (temperature, humidity) VALUES (?, ?)";
                break;
            case "co":
                query = "INSERT INTO co_sensor (co) VALUES (?)";
                break;
            case "hvac":
                query = "INSERT INTO hvac_actuator (status) VALUES (?)";
                break;
            default:
                query = null;
        }

         
        NetworkConfig.createStandardWithoutFile();
    }

    public void startObserving() {
        
        relation = client.observe(new CoapHandler() {
            @Override
            public void onLoad(CoapResponse response) {
                String content = response.getResponseText();
                System.out.println("Notifica ricevuta: " + content);
                List<String> values = SenMLParser.parseSenmlPayload(content);

                if (values.get(0).equals("-1.0")) {                    
                    return;
                }
                 
                // Aggiungi info al database
                try(Connection connection = Database.getConnection()) {
                    // Preparazione della query SQL per recuperare le informazioni
                    
                    PreparedStatement ps = connection.prepareStatement(query);
                    
                    // ciclo for con set_string 
                    for(int i = 0; i < values.size(); i++) {
                        if (values.get(i).equals("true"))
                            ps.setBoolean(i+1, true);
                        else if (values.get(i).equals("false"))
                            ps.setBoolean(i+1, false);
                        else
                            ps.setDouble(i+1, Double.parseDouble(values.get(i)));
                    }

                    // Esecuzione della query                           
                    ps.executeUpdate();

                    if (ps.getUpdateCount() < 1) {
                        // ERROR
                        // System.err.println("Errore nell'inserimento dei dati nel database");
                    } else {
                        // SUCCESS
                        // System.out.println("Dati inseriti correttamente");
                    }
                    
                } catch (Exception e) {
                    // Gestione degli errori
                    e.printStackTrace();
                }
            }

            @Override
            public void onError() {
                System.err.println("Errore nell'osservazione della risorsa");
            }
        });
        }

    public void stopObserving() {
        if (relation != null) {
            relation.proactiveCancel();
        }
        if (client != null) {
            client.shutdown();
        }
    }
}
