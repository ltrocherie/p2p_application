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

    while(true){
      //Socket connectionSocket = new Socket(connect, peerBasePort); // Attention ici inPort peut être set à 0 ce qui signifie attribution automatique de port
      ServerSocket welcomeSocket = new ServerSocket(peerBasePort);
      Socket connectionSocket = welcomeSocket.accept();

      BufferedReader br = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
      PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(connectionSocket.getOutputStream())),true);


      while(true){ // exception when not connected to a peer
        String message = br.readLine();
        if(message == null){
          break;
        }
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
            //System.out.println(indexes);
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
            /*
              Two cases :
              - answer to an interested
              - request
            */
            if(!interestedSent){ // checks
              answer = "have " + tokens[1] + " " + fm.getBuffermapToString(tokens[1]);
              pw.println(answer);
            }
            interestedSent = false;

            //need to deal with the answer and update the buffermap of this peer in the data structure of this file
            break;

          case "data":
            // TODO
            break;

          default:
            answer = "> Input not understood, please try again";
            pw.println(answer);
        }
          
        
      }
      br.close();
      pw.close();
      connectionSocket.close();
      welcomeSocket.close();
    }
  }

  public void run(){
    try{
      receivePeer();
    } catch (Exception e){
      System.out.println("Listening Interrupted.");
    }
    return;
  }
}
