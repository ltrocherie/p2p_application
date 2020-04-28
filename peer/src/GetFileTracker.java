import java.io.*;
import java.net.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.ListIterator;

/**
 * */

public class GetFileTracker extends PeerConfig implements Sender{

    String message = "getfile ";
    public void sendMessage(ArrayList<JTextField> texts){
        try{
            Socket socket = new Socket(super.trackerIp,super.trackerPort);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())),true);
            pw.println(message + texts.get(0).getText());
            System.out.println(">" + message + texts.get(0).getText());
            String str = br.readLine();// Ca c'est pour suivre en temps réel sur le terminal.
            System.out.println("<"+str);
            pw.println("END");
            texts.get(0).setText("");
            pw.close();
            br.close();
            socket.close();
        }catch(Exception e){
            System.out.println("Error in look"); // Faudra peut être donner des erreurs plus explicites.
        }
    }

    public void addValues(ArrayList<JTextField> texts){

    }

    public void flush(ArrayList<JTextField> texts){
        for(JTextField text : texts) {
            text.setText("");
        }
    }
}