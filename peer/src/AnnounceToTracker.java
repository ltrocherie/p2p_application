import java.io.*;
import java.net.*;
/**
 * */

public class AnnounceToTracker extends PeerConfig{


    public void announceTracker(String connect) throws Exception{ // on devrait pas passer le dossier en paramètre ?
      /*
          TODO : Rules announce, look, getfile
      */

        File[] fileL = super.fileList(folderName); // This fonctionne pas normalement en ce moment.
        String message = super.parseFileList(fileL);
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


}
