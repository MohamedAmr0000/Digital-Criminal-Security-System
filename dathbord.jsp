package com.security.auth;

import java.io.IOException;
import java.io.PrintWriter;
import java.sql.*; // استيراد كل مكتبات SQL عشان الخطوط الحمراء تختفي
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

@WebServlet("/GetSessionServlet")
public class GetSessionServlet extends HttpServlet {

    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        
        String code = request.getParameter("code");
        response.setContentType("application/json");
        PrintWriter out = response.getWriter();

        try {
            Connection con = DBConnection.getConnection(); 
            // استخدمنا UPPER عشان نتفادى مشاكل الحروف الكبيرة والصغيرة في Oracle
            String sql = "SELECT * FROM sessions WHERE UPPER(session_code) = UPPER(?)";
            PreparedStatement ps = con.prepareStatement(sql);
            ps.setString(1, code);
            ResultSet rs = ps.executeQuery();

            if (rs.next()) {
                // الكود موجود في الجدول (زي اللي في صورة Oracle image_1d7e8a.jpg)
                out.print("{\"status\":\"success\"}");
            } else {
                out.print("{\"status\":\"error\"}");
            }
            con.close();
        } catch (Exception e) {
            out.print("{\"status\":\"error\", \"message\":\"" + e.getMessage() + "\"}");
        }
    }
}