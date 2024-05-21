package it.unipi.iot;

import it.unipi.iot.Server.CoAPServer;

public class Main {
    public static void main(String[] args) {

        System.out.println("Hello world!");

        CoAPServer server = new CoAPServer();

        // Starting the CoAP Server
        server.start();


    }
}