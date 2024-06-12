package it.unipi.iot.Server.JSON;

import java.util.ArrayList;
import java.util.List;


public class SenMLParser {

    public static List<String> parseSenmlPayload(String buffer) {
        List<String> values = new ArrayList<>();
        if (buffer == null) {
            // Return an empty list if the buffer is null
            return values; 
        }

        int pos = 0;
        int bufferLength = buffer.length();

        while (pos < bufferLength) {
            // Looking for the "v" field (Numeric value)
            if (buffer.startsWith("\"v\"", pos)) {
                pos += 4;
                int start = pos;
                while (pos < bufferLength && buffer.charAt(pos) != ',' && buffer.charAt(pos) != '}') {
                    pos++;
                }
                String value = buffer.substring(start, pos).trim();
                Integer number = Integer.parseInt(value);

                double result = number / 100000.0;
                String valueConverted = Double.toString(result);

                values.add(valueConverted);
            }
            // Looking for the "bv" field (Boolean value)
            else if (buffer.startsWith("\"bv\"", pos)) {
                pos += 5;
                int start = pos;
                while (pos < bufferLength && buffer.charAt(pos) != ',' && buffer.charAt(pos) != '}') {
                    pos++;
                }
                String value = buffer.substring(start, pos).trim();
                values.add(value);
            }

            pos++;
        }

        return values;
    }

}