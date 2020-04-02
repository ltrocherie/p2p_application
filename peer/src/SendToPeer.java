import java.io.*;
import java.net.*;
/**
 * */

public class SendToPeer extends PeerConfig implements Runnable{

  String connect;
  String cmd;

  public SendToPeer(String connect, String cmd){
    this.connect = connect;
    this.cmd = cmd;
  }

  public String announcePeer() throws Exception{

    ServerSocket welcomeSocket = new ServerSocket(inPort);
    Socket socket = welcomeSocket.accept();

    BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())),true);

    // Tells another peer what is given in cmd
    pw.println(cmd);
    String answer = br.readLine();
    System.out.println(answer + "\n");

    pw.close();
    br.close();
    socket.close();

    return answer;
  }

  public void run(){
    try{
      announcePeer();
    } catch (Exception e){
      System.out.println("Sending Interrupted.");
      e.printStackTrace();
    }
    return;
  }


}
