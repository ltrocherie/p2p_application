import java.io.*;
import java.net.*;
import java.security.*;
/**
 * */

public class PeerConfig{
	  static int port = 8080; // pas une super idée le port http pour nos connexions

	  static int inPort = 10000; // A voir, un pour les communications, l'autre pour les transferts de fichier
	  static int outPort = 10000;
	  static String trackerIp = "127.0.0.1";
	  static int trackerPort = 10000;

	  static final String folderName = "../seed";
	  static int pieceSize = 1024;

	  /*** PRIVATE METHODS ***/

	File[] fileList(String folder){
	    File fold = new File(folder);
	    File[] list = fold.listFiles();
	    return list;
	}

	String parseFileList(File[] fileL) throws Exception{
	    String message = "announce listen " + inPort + " seed [";
	    for (final File fileEntry : fileL) {
	        message = message + fileEntry.getName();
	        message = message + " " + fileEntry.length();
	        message = message + " " + pieceSize;
	        message = message + " " + Buffermap.getFileChecksumMD5(fileEntry) + " ";
	    }
	    message = message.substring(0,message.length() - 1) + "]";
	    return message;
	}
}
