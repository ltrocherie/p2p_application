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


    Socket connectionSocket = new Socket(connect, inPort);

    BufferedReader br = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
    PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(connectionSocket.getOutputStream())),true);

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
    if(tokens[0].equals("<")){ // first part for the request of another peer
      switch(tokens[1]){
        /* TODO : analyze the input */
        case "interested":
          //answer with have
          String buffermap = "false";
          File[] filelist = fileList(folderName); 
          for (File f : filelist) {
            String hash = "" + f.hashCode();
            System.out.println(f.getName() + hash);
            if(hash.equals(tokens[2]))
              buffermap = "true";
          }
          answer = "> have " + tokens[2] + " " + buffermap; 
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
    }else{
      if(tokens[0].equals(">")){ // second part for analyzing an answer
      } 
      else{
        answer = "> Input not understood, please try again";
        pw.println(answer);
      }
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
