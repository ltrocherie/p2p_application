import java.io.*;
import java.net.*;
/**
 * */

public class ApplicationPeer {
    static int port = 8080;
    static final String folderName = "seed/";
    static int pieceSize = 1024;
    public static int main() {
        ApplicationPeer peer = new ApplicationPeer();
        try{
            peer.announce_tracker("ce site la marche pas");
        }
        return 0;
    }

    void announce_tracker(String connect) throws Exception{
        File[] fileL = this.fileList(folderName); // This fonctionne pas normalement en ce moment.
        String message = this.parseFileList(fileL);
        Socket socket = new Socket(connect,port);
        BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())),true);
        pw.println(message);
        String str = br.readLine();// Ca c'est pour suivre en temps réel dans le log.
        System.out.println("END");
        pw.println("END");
        pw.close();
        br.close();
        socket.close();
    }

    private File[] fileList(String folder){
        File fold = new File(folder);
        File[] list = fold.listFiles();
        return list;
    }

    private String parseFileList(File[] fileL) {
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