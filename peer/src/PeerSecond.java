import java.io.*;
import java.net.*;
/**
 * */

public class PeerSecond extends PeerConfig implements Runnable {

  public static void main(String[] args) {
  	FileManager fm = FileManager.getInstance();
    boolean[] array = {true, false, false};
    fm.buffermapUpdate("46a89dffeb294ee81b452b4cf552cc38", array);

    //(new Thread(new SendToPeer("localhost", "< have 42"))).start();
    (new Thread(new ReceiveFromPeer("127.0.0.1", inPort))).start();


  }

  public void run(){
  	System.out.println("thread exec");
  	/*
  	FileManager fm = FileManager.getInstance();
    boolean[] array = {true, false, false};
    fm.buffermapUpdate("46a89dffeb294ee81b452b4cf552cc38", array);
    */
    //(new Thread(new ReceiveFromPeer("localhost"))).start();
  }
}
