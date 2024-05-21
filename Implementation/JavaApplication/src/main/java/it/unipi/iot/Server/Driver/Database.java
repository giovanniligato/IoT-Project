package it.unipi.iot.Server.Driver;

import com.zaxxer.hikari.HikariConfig;
import com.zaxxer.hikari.HikariDataSource;

import java.io.IOException;
import java.io.InputStream;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.Properties;

public class Database {
    private static HikariDataSource dataSource;

    static {
        try {
            Properties properties = loadPropertiesFile();
            HikariConfig config = new HikariConfig();

            config.setJdbcUrl(properties.getProperty("db.url"));
            config.setUsername(properties.getProperty("db.username"));
            config.setPassword(properties.getProperty("db.password"));
            config.setDriverClassName(properties.getProperty("db.driverClassName"));

            // HikariCP specific settings
            config.setMaximumPoolSize(Integer.parseInt(properties.getProperty("hikari.maximumPoolSize")));
            config.setMinimumIdle(Integer.parseInt(properties.getProperty("hikari.minimumIdle")));
            config.setIdleTimeout(Long.parseLong(properties.getProperty("hikari.idleTimeout")));
            config.setConnectionTimeout(Long.parseLong(properties.getProperty("hikari.connectionTimeout")));
            config.setMaxLifetime(Long.parseLong(properties.getProperty("hikari.maxLifetime")));

            dataSource = new HikariDataSource(config);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static Properties loadPropertiesFile() throws IOException {
        Properties properties = new Properties();
        try (InputStream input = Database.class.getClassLoader().getResourceAsStream("database.properties")) {
            if (input == null) {
                throw new IOException("Sorry, unable to find database.properties");
            }
            properties.load(input);
        }
        return properties;
    }

    public static Connection getConnection() throws SQLException {
        return dataSource.getConnection();
    }
}

