package it.unipi.iot.UserApplication;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.MediaTypeRegistry;

import java.util.Scanner;

public class UserApplication {

    private static String coapUri = null;

    public static void startGUI() {
        Scanner scanner = new Scanner(System.in);
        boolean running = true;
        boolean loadingMessageDisplayed = false;
        boolean readyMessageDisplayed = false;

        while (running) {
            if (coapUri == null) {
                if (!loadingMessageDisplayed) {
                    System.out.println("===========================================");
                    System.out.println("Application is loading...");
                    System.out.println("Waiting for URI initialization.");
                    System.out.println("===========================================");
                    loadingMessageDisplayed = true;
                }
            } else {
                if (!readyMessageDisplayed) {
                    System.out.println("===========================================");
                    System.out.println("Application ready.");
                    readyMessageDisplayed = true;
                }
                System.out.println("===========================================");
                System.out.println("Type 'p' to press the button.");
                System.out.println("Type 'q' to quit.");
                System.out.println("===========================================");
                String input = scanner.nextLine();
                if (input.equalsIgnoreCase("p")) {
                    sendPostRequest(coapUri, "Button pressed from UserApplication.");
                    System.out.println("Command sent.");
                } else if (input.equalsIgnoreCase("q")) {
                    running = false;
                } else {
                    System.out.println("===========================================");
                    System.out.println("Unrecognized command.");
                    System.out.println("Use 'p' to press the button or 'q' to quit.");
                    System.out.println("===========================================");
                }
            }
            // Delay to avoid too fast looping
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        scanner.close();
        System.out.println("Application terminated.");
    }

    public static void initializeUri(String uri) {
        if (uri.startsWith("coap://")) {
            coapUri = uri;
            // System.out.println("URI inizializzato: " + coapUri);
        } else {
            // System.out.println("URI non valido. Assicurati che inizi con 'coap://'.");
        }
    }

    private static void sendPostRequest(String uri, String payload) {
        CoapClient client = new CoapClient(uri);
        CoapResponse response = client.post(payload, MediaTypeRegistry.TEXT_PLAIN);

        if (response != null) {
            // System.out.println("Codice di risposta: " + response.getCode());
            // System.out.println("Testo della risposta: " + response.getResponseText());
        } else {
            // System.out.println("Nessuna risposta ricevuta.");
        }
    }
}
