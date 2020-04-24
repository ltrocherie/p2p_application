import java.io.*;
import java.net.*;
import java.security.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.ListIterator;
/**
 * */

public class PeerConfig{
	  static int port = 8080; // pas une super idée le port http pour nos connexions

	  static int inPort = 15000; // A voir, un pour les communications, l'autre pour les transferts de fichier
	  static int outPort = 10000;
	  static String trackerIp = "127.0.0.1";
	  static int trackerPort = 10000;

	  static final String folderName = "../seed";
	  static int pieceSize = 1024;


	  public static void getElementFromConfig(){
		  try {
			  File file = new File("../config.ini");    //creates a new file instance
			  FileReader fr = new FileReader(file);   //reads the file
			  BufferedReader br = new BufferedReader(fr);  //creates a buffering character input stream
			  List<String> sb = new ArrayList<String>();    //constructs a string buffer with no characters
			  String line;
			  while ((line = br.readLine()) != null) {
				  sb.add(line);      //appends line to string buffer
			  }
			  fr.close();    //closes the stream and release the resources
			  ListIterator<String> it = sb.listIterator();
			  while (it.hasNext()) {
			  	  String stri = it.next();
				  if (stri != "") {
					  String[] arra = stri.split("=");
					  switch (arra[0]) {
						  case "tracker-address" :
						  	trackerIp=arra[1];
						  	break;
						  case "tracker-port" :
						  	trackerPort= Integer.parseInt(arra[1]);
						  	break;
						  case "open-port":
						  	inPort = Integer.parseInt(arra[1]);
						  	break;
						  default :
						  	System.out.println("Impossible de lire la configuration :"+it);
					  }

				  }
			  }
		  }
		  catch(IOException e)
		  {
			  e.printStackTrace();
		  }
	  }
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
