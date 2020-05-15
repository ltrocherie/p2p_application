import java.io.*;
import java.net.*;
/**
 * */

public class Peer {
  
  static final AnnounceToTracker ann = new AnnounceToTracker();
  static private FileManager buffermap;

  public static void main(String[] args) {
    PeerConfig.getElementFromConfig();
    PeerConfig.getElementFromCommandLine(args);


    /*try{
        ann.sendMessage("../seed");
    }
    catch(Exception e){
        System.out.println("Erreur lors de l'envoi de announce");
    }*/


      /* idea :
  	use 1 thread that constently updates the files in the folder with the buffermaps
  	use 1 thread that constently listen on 1 port
  	create 1 thread every time the peer wants to send something
  	*/
    //(new Thread(buffermap)).start(); //doesn't work
    //boolean[] tab = {true, false, true};
    //buffermap.buffermapUpdate("coucou", tab);

    //(new Thread(new SendToPeer("localhost", "< interested -785361703"))).start();
    //(new Thread(new ReceiveFromPeer("localhost"))).start();

    /*
    Thread t = (new Thread(new PeerSecond()));
    t.start();
    System.out.println("main exec");
    FileManager fm = FileManager.getInstance();
    fm.printAll();
    */
    (new Thread(new SendToPeer("127.0.0.1", "interested 6503f4e83628d295d95500d6e68d8f24"))).start(); // aled


  }

}
