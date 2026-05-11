package com.security.auth;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

public class DBConnection {
    public static Connection getConnection() throws SQLException {
        try {
            // تحميل درايفر أوراكل
            Class.forName("oracle.jdbc.driver.OracleDriver");
            
            // رابط الاتصال - تأكد من اسم الـ Database (غالباً xe لو نسخة Express)
            String url = "jdbc:oracle:thin:@localhost:1521:xe"; 
            String user = "cyber"; 
            String pass = "cyber"; // الباسورد بتاعك
            
            return DriverManager.getConnection(url, user, pass);
        } catch (ClassNotFoundException e) {
            throw new SQLException("Oracle Driver NOT found: " + e.getMessage());
        }
    }
}