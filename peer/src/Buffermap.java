import java.io.*;
import java.net.*;
import java.util.*;
import java.security.MessageDigest;
/**
 * */

public class Buffermap extends PeerConfig implements Runnable{

	byte[] buffermapArray;
	List<String> hashArray;

	public Buffermap(){
		buffermapArray = new byte[pieceSize/8];
		hashArray = new ArrayList<String>();
	}

	private static String getFileChecksumMD5(File file) throws Exception
	{
		//Use MD5 algorithm
		MessageDigest md5Digest = MessageDigest.getInstance("MD5");

	    //Get file input stream for reading the file content
	    FileInputStream fis = new FileInputStream(file);
	     
	    //Create byte array to read data in chunks
	    byte[] byteArray = new byte[1024];
	    int bytesCount = 0; 
	      
	    //Read file data and update in message digest
	    while ((bytesCount = fis.read(byteArray)) != -1) {
	        md5Digest.update(byteArray, 0, bytesCount);
	    };
	     
	    //close the stream; We don't need it now.
	    fis.close();
	     
	    //Get the hash's bytes
	    byte[] bytes = md5Digest.digest();
	     
	    //This bytes[] has bytes in decimal format;
	    //Convert it to hexadecimal format
	    StringBuilder sb = new StringBuilder();
	    for(int i=0; i< bytes.length ;i++)
	    {
	        sb.append(Integer.toString((bytes[i] & 0xff) + 0x100, 16).substring(1));
	    }
	     
	    //return complete hash
	   return sb.toString();
	}

	void buffermapUpdate() throws Exception{
		File[] filelist = fileList(folderName); 
	    for (File f : filelist) {
	    	hashArray.add(getFileChecksumMD5(f));
	    	//buffermapArray.append();
    	}
    	for (int i = 0; i < hashArray.size(); i++) {
    		System.out.println(hashArray.get(i));
    	}
	}

	public void run(){
		try{
			buffermapUpdate();
		}catch(Exception e){
			e.printStackTrace();
		}
	}
}