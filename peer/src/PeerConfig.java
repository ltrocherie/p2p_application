import java.io.*;
import java.net.*;
/**
 * */

public class PeerConfig{
	  static int port = 8080; // pas une super idée le port http pour nos connexions

	  static int inPort = 10000; // A voir, un pour les communications, l'autre pour les transferts de fichier
	  static int outPort = 10000;

	  static final String folderName = "../seed";
	  static int pieceSize = 1024;

	  /*** PRIVATE METHODS ***/

	File[] fileList(String folder){
	    File fold = new File(folder);
	    File[] list = fold.listFiles();
	    return list;
	}

	String parseFileList(File[] fileL) {
	    String message = "announce listen " + port + "seed [";
	    for (final File fileEntry : fileL) {
	        message = message + fileEntry.getName();
	        message = message + " " + fileEntry.getTotalSpace();
	        message = message + " " + pieceSize;
	        message = message + " " + fileEntry.hashCode() + " ";
	    }
	    message = message + "]";
	    return message;
	}
}
