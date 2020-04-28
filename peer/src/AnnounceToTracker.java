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

public class AnnounceToTracker extends PeerConfig implements Sender{


    public void sendMessage(ArrayList<JTextField> texts){ // on devrait pas passer le dossier en paramètre ?
      /*
          TODO : Rules announce, look, getfile
      */
        try{
            Socket socket = new Socket(super.trackerIp,super.trackerPort);
            /*File[] fileL = super.fileList("../seed"); // This fonctionne pas normalement en ce moment.
            String message = super.parseFileList(fileL);*/
            DatFileParser getMessage= new DatFileParser();
            String message = getMessage.getFilesFrom(super.seedFile);
            message = "announce listen " + super.inPort + " seed [" + message;
            message = message.substring(0,message.length() - 1) + "]";
            System.out.println(message);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())),true);
            pw.println(message);
            System.out.println(">"+message);
            String str = br.readLine();// Ca c'est pour suivre en temps réel sur le terminal.
            System.out.println("<"+str);
            pw.println("END");
            pw.close();
            br.close();
            socket.close();
        }catch(Exception e){
            System.out.println("Socket connecting error");
        }

    }
    public void addValues(ArrayList<JTextField> texts){

    }

    public void flush(ArrayList<JTextField> texts){

    }


}
