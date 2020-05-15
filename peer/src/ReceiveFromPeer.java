import java.io.*;
import java.net.*;
/**
 * */

public class ReceiveFromPeer extends PeerConfig implements Runnable{

  String connect;

  public ReceiveFromPeer(String connect){
    this.connect = connect;
  }

  public void receivePeer() throws Exception{


    Socket connectionSocket = new Socket(connect, peerBasePort); // Attention ici inPort peut être set à 0 ce qui signifie attribution automatique de port

    BufferedReader br = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
    PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(connectionSocket.getOutputStream())),true);

    while(true){ // exception when not connected to a peer
      String message = br.readLine();
      System.out.println(message);
      String answer = "";

      // Analyzing the message
      String delims = " ";
      String[] tokens = message.split(delims);
      /*
      for (int i = 0; i<tokens.length; i++) {
        System.out.println(tokens[i]);
      }
      */
      switch(tokens[0]){
        /* TODO : analyze the input */
        case "interested":
          //answer with have
          String buffermap = "";
          FileManager fm = FileManager.getInstance();
          answer = "have " + tokens[1] + " " + fm.getBuffermapToString(tokens[1]); 
          pw.println(answer);
          break;
        case "getpieces":
          break;
        case "have":
          answer = "> Ok";
          pw.println(answer);
          break;
        case "update":
          break;
        default:
          answer = "> Input not understood, please try again";
          pw.println(answer);
      }
        
      
    }
  }

  public void run(){
    try{
      receivePeer();
    } catch (Exception e){
      System.out.println("Listening Interrupted.");
      e.printStackTrace();
    }
    return;
  }
}
