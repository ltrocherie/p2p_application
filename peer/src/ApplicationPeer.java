import java.io.*;
import java.net.*;

class ApplicationPeer {


    public static void main(String[] args) throws Exception{
        TcpServer tcpServer = new TcpServer(6790);
        TcpClient tcpClient = new TcpClient(6789, "salut");
        System.out.println("Init done");
        tcpClient.sendMessage();
        tcpServer.waitForMessage();

    }


}