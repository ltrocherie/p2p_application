import java.io.*;
import java.net.*;
import java.util.ArrayList;

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
      boolean interestedSent = false;

      // gets the data structure
      FileManager fm = FileManager.getInstance();

      // Analyzing the message
      String delims = "[ \\[\\]]";
      String[] tokens = message.split(delims);
      /*
      for (int i = 0; i<tokens.length; i++) {
        System.out.println("is" + tokens[i]);
      }
      */
      switch(tokens[0]){
        /* TODO : analyze the input */
        /*
            This part is for analyzing a request from another peer
        */
        case "interested":
          //answer with have
          answer = "have " + tokens[1] + " " + fm.getBuffermapToString(tokens[1]); 
          pw.println(answer);
          interestedSent = true;
          break;
        case "getpieces":
          ArrayList<Integer> indexes = new ArrayList<>();
          for (int i = 3; i < tokens.length; i++) {
            indexes.add(Integer.parseInt(String.valueOf(tokens[i])));
          }
          System.out.println(indexes);
          String buffermap = fm.getBuffermapToString(tokens[1]); 
          answer = "data " + tokens[1] + " [";
          // TODO : need to add the indexes + file part
          for (int i = 0; i < indexes.size(); i++) {
            answer += Character.forDigit(indexes.get(i),10) + ":" + "%Piece" + Character.forDigit(i,10) + "%";
            if(i != indexes.size() - 1)
              answer += " ";
          }
          answer += "]";
          pw.println(answer);
          break;
        case "have":
          // pb with all the cases : infinite loop need to do a if
          if(!interestedSent){ // checks
            answer = "have " + tokens[1] + " " + fm.getBuffermapToString(tokens[1]);
            pw.println(answer);
            interestedSent = false;
          }
          //need to deal with the answer
          break;
        /*
            This part is for analyzing an answer from another peer
        */
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
