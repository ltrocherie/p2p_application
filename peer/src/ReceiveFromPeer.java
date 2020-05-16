import java.io.*;
import java.net.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;

/**
 * */

public class ReceiveFromPeer extends PeerConfig implements Runnable{

  String connect;
  int port;

  public ReceiveFromPeer(String connect, int port){
    this.connect = connect;
    this.port = port;
  }

  public void receivePeer() throws Exception{

    while(true){
      //Socket connectionSocket = new Socket(connect, peerBasePort); // Attention ici inPort peut être set à 0 ce qui signifie attribution automatique de port
      ServerSocket welcomeSocket = new ServerSocket(port);
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
            String fileToSend = new String(Files.readAllBytes(Paths.get(fm.getPath(tokens[1])))); // problem of path
            // splits the file
            String[] filePieces = new String[fileToSend.length()/pieceSize + 1];
            int globalIndex = 0;
            char[] tmp = new char[pieceSize];
            for (int i = 0; i < fileToSend.length(); i++) {
              tmp[i % pieceSize] = fileToSend.charAt(i);
              if(i % pieceSize == 0 && i > 0){
                String piece = new String(tmp);
                filePieces[globalIndex] = piece;
                globalIndex++;
                tmp = new char[pieceSize];
                //System.out.println("File cutting");              
              }
            }
            String piece = new String(tmp);
            filePieces[globalIndex] = piece;
            globalIndex++;
            tmp = new char[pieceSize];
            // creates the answer
            for (int i = 0; i < indexes.size(); i++) {
              answer += Character.forDigit(indexes.get(i),10) + ":"+ filePieces[indexes.get(i) - 1];
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

          case "nok":
            // TODO
            break;

          default:
            answer = "nok";
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
      e.printStackTrace();
    }
    return;
  }
}
