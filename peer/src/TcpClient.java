import java.io.*;
import java.net.*;

class TcpClient {

    private BufferedReader inFromUser;
    private Socket clientSocket;
    private DataOutputStream outToServer;
    private BufferedReader inFromServer;

    String sentence;

    public TcpClient(int port, String message) throws Exception{
        /* Prepares the connection */
        inFromUser = new BufferedReader(new InputStreamReader(System.in));
        clientSocket = new Socket("localhost", port); //6789 for example
        outToServer = new DataOutputStream(clientSocket.getOutputStream());
        inFromServer = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));

        /* Sets the message */
        sentence = message;
    }

    public void sendMessage() throws Exception {

        outToServer.writeBytes(sentence);
        clientSocket.close();

        System.out.println("Message sent");


    }
}