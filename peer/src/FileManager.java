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
	Map<String, boolean[]> fileManager; // stores the hash and the buffermap for each file of this peer
	Map<String, String[]> peerManager;	// stores the hash of a file on the network and all the peers who have it
	Map<String, String> fileMatch; // stores the hash of a file and its path
	Map<String, String[]> filePieces; // stores the pieces for a given file

	// lock for concurrent accesses
	ReentrantLock lock;

	// Instance for singleton behavior
	private static final FileManager fileManagerInstance = new FileManager();

	private FileManager(){
		fileManager = new HashMap<String, boolean[]>();
		lock = new ReentrantLock();
		peerManager = new HashMap<String, String[]>();
		fileMatch = new HashMap<String, String>();
		filePieces = new HashMap<String, String[]>();
		try{
			buffermapInit();
		}catch(Exception e){
			e.printStackTrace();
		}
	}


	public static FileManager getInstance(){
		return fileManagerInstance;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// client methods

	void updateFileMatch(String hash, String filename){
		fileMatch.put(hash,filename);
	}

	void updateFilePieces(String key,String buffermap){
		if(filePieces.containsKey(key)){
			return;
		}
		byte[] str = buffermap.getBytes();
		int len = str.length*8;
		String[] arr = new String[len];
		Arrays.fill(arr,"");
		filePieces.put(key,arr);
	}

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

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to handle the file manager

	void buffermapInit() throws Exception{
		// gets all the files in the seed/ folder and adds them in the file manager
		File[] filelist = fileList(folderName); 
	    for (File f : filelist) {
	    	String hash = getFileChecksumMD5(f);
	    	long bmlength = f.length()/pieceSize + 1;
	    	boolean[] buffermap = new boolean[(int) bmlength];
	    	Arrays.fill(buffermap, Boolean.TRUE);
	    	fileManager.put(hash, buffermap);
	    	fileMatch.put(hash,f.getPath());
    	}
    	return;
	}


	void buffermapUpdate(String hash, boolean[] buffermap){
		lock.lock();
	    fileManager.put(hash, buffermap);
	    lock.unlock();
		return;
	}

	boolean[] getBuffermap(String hash){
		lock.lock();
		boolean[] ret = fileManager.get(hash);
		lock.unlock();
		return ret;
	}

	String getBuffermapToString(String hash){
		lock.lock();
		String res = "";
		String message = "";
		int index = 0;
		byte b=0;
		byte one=1;
		boolean first = true;
		if(!fileManager.containsKey(hash)){
			if(!fileMatch.containsKey(hash)){
				System.out.println("The file asked doesn't exist");
				PeerConfig.writeInLogs("The file asked doesn't exist");
				return "nok";
			}
			try{
				File f = new File(fileMatch.get(hash));
				long len = f.length() / PeerConfig.pieceSize;
				String full = ""+(byte)~0;
				message = message + full.repeat((int)len/8);
				if(len%8!=0){
					int rest = (int)(len % 8);
					int lastBytes = (byte) ((byte)~0 << (rest));
					message = message + lastBytes;
				}
				return message;
			}catch(Exception e){
				System.out.println("Error while reading file with key :"+hash);
				PeerConfig.writeInLogs("Error while reading file with key :"+hash);
			}
		}
		boolean[] value = fileManager.get(hash);
		for(boolean bit: value){
			if(index == 0 && !first) {
				res =  res + b;
				b = 0;
			}
			first = false;
			b = (byte) (b << 1);
			if(bit){
				b =(byte) (b + one);
			}
			index = (index + 1)%8;

			/*if(entry.getKey().equals(hash)){
				for(boolean bit: entry.getValue()){
					if(bit == true){res += "1";}
					if(bit == false){res += "0";}
				}
			}*/
			if(index != 0){
				res = res + (byte) (b << (8-index));
			}
		}
		lock.unlock();
		return res;
	}

	boolean[] getStringToBuffermap(String buffer){
		int index = 0;
		int piece = 0;
		int size = buffer.length() * 8;
		byte[] buff = buffer.getBytes();
		boolean[] buffermap = new boolean[size];
		while(index < buff.length) {
			byte bit = buff[index];
			byte mask = 0x01;
			for (int j = 0; j < 8; j++)
			{
				int value = bit & mask;
				if(value==1){ // il veut pas convertir en un booléen
					buffermap[piece] = true;
				}else{
					buffermap[piece] = false;
				}
				piece += 1;
				mask <<= 1;
			}
			index += 1;
		}
		return buffermap;
	}

	void printBuffermap(boolean[] buffermap){
		lock.lock();
		String res = new String();
		for(boolean bit: buffermap){
			if(bit == true){res += "1";}
			if(bit == false){res += "0";}
		}
		System.out.println(res);
		PeerConfig.writeInLogs(res);
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
			PeerConfig.writeInLogs(res);
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

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to handle the peer manager

	String[] getPeers(String hash){
		lock.lock();
		String[] ret = peerManager.get(hash);
		lock.unlock();
		return ret;
	}

	void peerUpdate(String fileHash, String[] peers){
		lock.lock();
	    peerManager.put(fileHash, peers);
	    lock.unlock();
		return;
	}

	void printAllPeers(){
		String res;
		lock.lock();
		for(Map.Entry<String, String[]> entry: peerManager.entrySet()){
			res = entry.getKey() + " is owned by : ";
			for(String peer: entry.getValue()){
				res += peer + " ";
			}
			System.out.println(res);
			PeerConfig.writeInLogs(res);
		}
		lock.unlock();
		return;		
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	String getPath(String hash){
		if(fileMatch.containsKey(hash)){
			return fileMatch.get(hash);
		}
		return "";
	}

	void printAllPaths(){
		String res;
		lock.lock();
		for(Map.Entry<String, String> entry: fileMatch.entrySet()){
			res = entry.getKey() + " is at: " + entry.getValue();
			System.out.println(res);
			PeerConfig.writeInLogs(res);
		}
		lock.unlock();
		return;		
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void storePieces(String message){
		String key = message.split(" ")[1];
		message = message.substring("have 8905e92afeb80fc7722ec89eb0bf0966 ".length(),message.length());
		if(message.length() <= 1){
			System.out.println("Error no given pieces");
			PeerConfig.writeInLogs("Error no given pieces");
			return;
		}
		message = message.substring(0,message.length()-1);
		String[] tableOfPieces = filePieces.get(key);
		if(tableOfPieces == null){
			System.out.println("You try to fill a file you were not interested in");
			PeerConfig.writeInLogs("You try to fill a file you were not interested in");
			return;
		}
		boolean[] buffermap = fileManager.get(key);
		while(message.length() >0){
			message = message.substring(1,message.length());
			String pieceNumber = message.split(":")[0];
			int ind = Integer.parseInt(pieceNumber);
			message = message.substring(pieceNumber.length(),message.length());
			byte[] bytesTable = message.getBytes();
			String text = new String(bytesTable, 0, PeerConfig.pieceSize);
			if(tableOfPieces[ind]==""){
				tableOfPieces[ind] = text;
				buffermap[ind] = true;
			}
			message= message.substring(text.length(),message.length());
		}
		if(checkIfFull(tableOfPieces)){
			writeFile(key);
			leechToSeed(key);
		}
	}


	boolean checkIfFull(String[] pieces){
		for(String piece: pieces){
			if(piece == ""){
				return false;
			}
		}
		return true;
	}

	void leechToSeed(String key){
		filePieces.remove(key);
		DatFileParser pars = new DatFileParser();
		String path = fileMatch.get(key);
		pars.addFileTo(PeerConfig.seedFile,path);
		pars.removeFileTo(PeerConfig.leechFile,path);
		fileMatch.remove(key);

	}

	void writeFile(String key){
		String[] tableOfPieces = filePieces.get(key);
		String path = fileMatch.get(key);
		File f = new File(path);
		if(f.exists() && !f.isDirectory()) {
			System.out.println("File "+path+" already exists");
			PeerConfig.writeInLogs("File "+path+" already exists");
		}else {
			String toWrite = "";
			for(String piece : tableOfPieces){
				toWrite = toWrite + piece;
			}
			try {
				BufferedWriter out = new BufferedWriter(
						new FileWriter(path));
				out.write(toWrite);
				out.close();
			} catch (IOException e) {
				System.out.println("Error while writing file "+path);
				PeerConfig.writeInLogs("Error while writing file "+path);
			}
		}
	}

	void updatePieces(String hash, String[] pieces){
		// need to update
		String res;
		//lock.lock();

		//lock.unlock();

		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	public void run(){
		try{
			buffermapInit();
		}catch(Exception e){
			e.printStackTrace();
		}
	}
}