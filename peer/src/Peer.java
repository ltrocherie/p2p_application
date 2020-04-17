import java.io.*;
import java.net.*;
/**
 * */

public class Peer {

  public static void main(String[] args) {

  	/* idea :
		use 1 thread that constently updates the files in the folder with the buffermaps
		use 1 thread that constently listen on 1 port
		create 1 thread every time the peer wants to send something
  	*/
	(new Thread(new Buffermap())).start();

    (new Thread(new SendToPeer("localhost", "< interested -785361703"))).start();
    (new Thread(new ReceiveFromPeer("localhost"))).start();

      
  }

}