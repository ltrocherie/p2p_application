import java.io.*;
import java.net.*;

class TcpServer {

    private ServerSocket welcomeSocket;
    private Socket connectionSocket;
    private BufferedReader inFromClient;
    private DataOutputStream outToClient;


    String clientSentence;

    public TcpServer(int port) throws Exception{
        welcomeSocket = new ServerSocket(port);
    }


    public void waitForMessage() throws Exception {
        System.out.println("Server started");
        connectionSocket = welcomeSocket.accept();
        inFromClient = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
        outToClient = new DataOutputStream(connectionSocket.getOutputStream());
        System.out.println("New buffer");
        clientSentence = inFromClient.readLine();
        System.out.println("Received: " + clientSentence);

    }
}