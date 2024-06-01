package it.unipi.iot;

import it.unipi.iot.Server.CoAPServer;

import it.unipi.iot.UserApplication.UserApplication;

public class Main {
    public static void main(String[] args) {

        System.out.println("Starting the Java Server...");

        CoAPServer server = new CoAPServer();
        // Starting the CoAP Server
        server.start();

        UserApplication.startGUI();


    }
}