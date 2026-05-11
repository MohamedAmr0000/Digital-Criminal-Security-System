package com.security.auth;

import com.fazecast.jSerialComm.SerialPort;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Scanner;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

@WebServlet("/GetSensorData1")
public class GetSensorData extends HttpServlet {
    private static SerialPort comPort;

    @Override
    public void init() throws ServletException {
        if (comPort == null || !comPort.isOpen()) {
            comPort = SerialPort.getCommPort("COM3");
            comPort.setBaudRate(115200);
            // تقليل وقت الانتظار لضمان سرعة الاستجابة
            comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 500, 0);
            
            if (comPort.openPort()) {
                System.out.println("Port opened successfully on COM3");
            } else {
                System.err.println("CRITICAL: Failed to open port. Check connection!");
            }
        }
    }

    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        response.setContentType("application/json");
        response.setCharacterEncoding("UTF-8");
        PrintWriter out = response.getWriter();

        String bpm = "0";

        // استخدام مزامنة لمنع تداخل القراءات من مستخدمين مختلفين
        synchronized (GetSensorData.class) {
            if (comPort != null && comPort.isOpen()) {
                try {
                    if (comPort.bytesAvailable() > 0) {
                        Scanner scanner = new Scanner(comPort.getInputStream());
                        // قراءة أحدث سطر متاح
                        if (scanner.hasNextLine()) {
                            String rawData = scanner.nextLine().trim();
                            // استخراج الأرقام فقط
                            bpm = rawData.replaceAll("[^0-9]", "");
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        // تنظيف نهائي للبيانات
        if (bpm.isEmpty() || bpm.length() > 3) {
            bpm = "0";
        }

        out.print("{\"bpm\": " + bpm + ", \"spo2\": 0}");
    }

    @Override
    public void destroy() {
        if (comPort != null && comPort.isOpen()) {
            comPort.closePort();
            System.out.println("Port closed.");
        }
    }
}