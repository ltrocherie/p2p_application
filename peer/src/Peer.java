import java.io.*;
import java.net.*;
/**
 * */

public class Peer {

  public static void main(String[] args) {

    (new Thread(new SendToPeer("localhost", "> cmd"))).start();
    (new Thread(new ReceiveFromPeer("localhost"))).start();

  }

}
