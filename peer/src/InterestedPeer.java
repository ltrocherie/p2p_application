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
import java.util.Timer;
/**
 * */

public class InterestedPeer extends PeerConfig implements Sender{

    String message = "interested ";

    public void sendMessage(ArrayList<JTextField> texts){ 
      /*
          TODO : Rule Interested to send to each other peer on the network
      */
        String address = texts.get(0).getText();
        String port = texts.get(1).getText();
        String message = address.replaceAll("\\s+","") + ":" + port.replaceAll("\\s+","");
        String key = texts.get(2).getText();
        SendToPeer(message,key);
        flush(texts);
    }

    public void addValues(ArrayList<JTextField> texts){
        if(!texts.get(0).getText().equals("")){
            this.message = this.message +texts.get(0).getText() +" ";
            texts.get(0).setText("");
            System.out.println(this.message);
        }
    }

    public void flush(ArrayList<JTextField> texts){
        for(JTextField text : texts) {
            text.setText("");
        }
        this.message = "interested ";
    }

    public void sendFromLook(String str){
        String[] strParsed = str.split(" ");
        //C'est interessant à partir de 3
        int index = 3;
        String key = strParsed[1];
        while(strParsed[index] != "]"){
             SendToPeer(strParsed[index],key);
             index = index + 1;
        }
        if(index == 3){
            System.out.println("No peer have key "+ key + ". Interested not send");
            PeerConfig.writeInLogs("No peer have key "+ key + ". Interested not send");
        }
    }

    public void SendToPeer(String toSend,String key){
        String[] infos = toSend.split(":");
        try{
            Socket socket = new Socket(infos[0],Integer.parseInt(infos[1]));
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())),true);
            pw.println("interested "+key);
            System.out.println("< interested "+key);
            PeerConfig.writeInLogs("< interested "+key);
            String str = br.readLine();// Ca c'est pour suivre en temps réel sur le terminal.
            System.out.println(">"+str);
            PeerConfig.writeInLogs(">"+str);
            GetPiecesPeer GetPPeer = new GetPiecesPeer();
            GetPPeer.sendFromInt(str,infos[0],infos[1]);
            pw.println("END");
            pw.close();
            br.close();
            socket.close();
        }catch(Exception e){
            System.out.println("Error in interested");
            PeerConfig.writeInLogs("Error in interested");
        }

    }

}