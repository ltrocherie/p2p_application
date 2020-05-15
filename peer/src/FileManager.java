import java.io.*;
import java.net.*;
import java.util.*;
import java.util.Arrays; 
import java.util.concurrent.BlockingQueue;
import java.security.MessageDigest;
import java.util.concurrent.locks.ReentrantLock;
/**
 * */

public class FileManager extends PeerConfig implements Runnable{

	// Data structure
	Map<String, boolean[]> fileManager;

	// lock for concurrent accesses
	ReentrantLock lock;

	// Instance for singleton behavior
	private static final FileManager fileManagerInstance = new FileManager();

	private FileManager(){
		fileManager = new HashMap<String, boolean[]>();
		lock = new ReentrantLock();
		try{
			buffermapInit();
		}catch(Exception e){
			e.printStackTrace();
		}
	}

	public static FileManager getInstance(){
		return fileManagerInstance;
	}

	// client methods

	public static String getFileChecksumMD5(File file) throws Exception
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

	void buffermapInit() throws Exception{
		// gets all the files in the seed/ folder and adds them in the file manager
		File[] filelist = fileList(folderName); 
	    for (File f : filelist) {
	    	String hash = getFileChecksumMD5(f);
	    	long bmlength = f.length()/pieceSize + 1;
	    	boolean[] buffermap = new boolean[(int) bmlength];
	    	Arrays.fill(buffermap, Boolean.TRUE);
	    	fileManager.put(hash, buffermap);
    	}
    	return;
	}

	void buffermapUpdate(String hash, boolean[] buffermap){
		lock.lock();
		for(Map.Entry<String, boolean[]> entry: fileManager.entrySet()){
			if(entry.getKey().equals(hash)){
				entry.setValue(buffermap);
				lock.unlock();
				return;
			}
		}
	    fileManager.put(hash, buffermap);
	    lock.unlock();
		return;
	}

	boolean[] getBuffermap(String hash){
		lock.lock();
		for(Map.Entry<String, boolean[]> entry: fileManager.entrySet()){
			if(entry.getKey().equals(hash)){
				lock.unlock();
				return entry.getValue();
			}
		}
		boolean[] ret = {};
		lock.unlock();
		return ret;
	}

	String getBuffermapToString(String hash){
		lock.lock();
		String res = "";
		for(Map.Entry<String, boolean[]> entry: fileManager.entrySet()){
			if(entry.getKey().equals(hash)){
				for(boolean bit: entry.getValue()){
					if(bit == true){res += "1";}
					if(bit == false){res += "0";}
				}
			}
		}
		lock.unlock();
		return res;
	}

	void printBuffermap(boolean[] buffermap){
		lock.lock();
		String res = new String();
		for(boolean bit: buffermap){
			if(bit == true){res += "1";}
			if(bit == false){res += "0";}
		}
		System.out.println(res);
		lock.unlock();
		return;
	}

	void printAll(){
		String res;
		/*
		Iterator<Map.Entry<String, boolean[]>> entries = fileManager.entrySet().iterator();
		while (entries.hasNext()) {
		    Map.Entry<String, boolean[]> entry = entries.next();
		    res = entry.getKey() + " : ";	
			for(boolean bit: entry.getValue()){
				if(bit == true){res += "1";}
				if(bit == false){res += "0";}
			}
			System.out.println(res);
			return;
		}*/
		lock.lock();
		for(Map.Entry<String, boolean[]> entry: fileManager.entrySet()){
			res = entry.getKey() + " has the buffermap: ";	
			System.out.println(res);
			printBuffermap(entry.getValue());
		}
		lock.unlock();
		return;
		/*
		fileManager.forEach((hash, buffermap) -> res = tuple.getKey() + " : ";	
			for(boolean bit: tuple.getValue()){
				if(bit == true){res += "1";}
				if(bit == false){res += "0";}
			}
			System.out.println(res););
		*/
		
	}


	public void run(){
		try{
			buffermapInit();
		}catch(Exception e){
			e.printStackTrace();
		}
	}
}