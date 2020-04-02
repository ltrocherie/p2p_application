import java.io.*;
import java.net.*;
/**
 * */

public class ReceiveFromPeer extends PeerConfig implements Runnable{

  String connect;

  public ReceiveFromPeer(String connect){
    this.connect = connect;
  }

  public String receivePeer() throws Exception{


    Socket connectionSocket = new Socket("localhost", inPort);

    BufferedReader br = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
    PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(connectionSocket.getOutputStream())),true);

    String message = br.readLine();
    System.out.println(message);
    String answer = "";
    switch(message){
      /* TODO : analyze the input */
      case "< have*":
        answer = "> Ok";
        pw.println(answer);
      default:
        answer = "> Input not understood, please try again";
        pw.println(answer);

    }
    return answer;
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
