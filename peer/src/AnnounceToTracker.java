import java.io.*;
import java.net.*;
/**
 * */

public class AnnounceToTracker extends PeerConfig{


    public void announceTracker(String[] args, String folderName) throws Exception{ // on devrait pas passer le dossier en paramètre ?
      /*
          TODO : Rules announce, look, getfile
      */
        if(args.length>0){
            super.port = Integer.parseInt(args[0]);
        }
        if(args.length>1){
            super.trackerIp = args[1];
        }
        if(args.length>2){
            super.trackerPort = Integer.parseInt(args[2]);
        }

        Socket socket = new Socket("localhost",super.trackerPort);
        File[] fileL = super.fileList(folderName); // This fonctionne pas normalement en ce moment.
        String message = super.parseFileList(fileL);
        System.out.println(message);
        BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        PrintWriter pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())),true);
        pw.println(message);
        String str = br.readLine();// Ca c'est pour suivre en temps réel dans le log.
        System.out.println(str);
        pw.println("END");
        pw.close();
        br.close();
        socket.close();

    }


}
