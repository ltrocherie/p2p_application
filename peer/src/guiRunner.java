import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
public class guiRunner {
    JFrame frame;

    public guiRunner(){
        //Creating the Frame
        frame = new JFrame("Peer interface");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(500, 500);
    }
    public void run() {
        //Adding Components to the frame.
        //frame.getContentPane().add(BorderLayout.NORTH, mb);
        //frame.getContentPane().add(Buttons);
        //mainPanel = frame.getContentPane();
        loading loader = new loading();
        loader.loadMainPage();
        JPanel currentPanel = loader.getMainPanel();
        JMenuBar menuBar = loader.getMenuBar();
        this.frame.getContentPane().add(BorderLayout.NORTH,menuBar);
        this.frame.getContentPane().add(currentPanel);
        this.frame.setVisible(true);
    }

    void changeToMain(){
        this.frame.removeAll();
        JPanel currentPanel = loader.getMainPanel();
        JMenuBar menuBar = loader.getMenuBar();
        this.frame.getContentPane().add(BorderLayout.NORTH,menuBar);
        this.frame.getContentPane().add(currentPanel);
        this.frame.revalidate();
        this.frame.repaint();
    }

    void changeToLook(){
        this.frame.removeAll();
        JPanel currentPanel = loader.getLookPanel();
        JMenuBar menuBar = loader.getMenuBar();
        this.frame.getContentPane().add(BorderLayout.NORTH,menuBar);
        this.frame.getContentPane().add(currentPanel);
        this.frame.revalidate();
        this.frame.repaint();
    }

    void changeToInter(){
        this.frame.removeAll();
        JPanel currentPanel = loader.getIntPanel();
        JMenuBar menuBar = loader.getMenuBar();
        this.frame.getContentPane().add(BorderLayout.NORTH,menuBar);
        this.frame.getContentPane().add(currentPanel);
        this.frame.revalidate();
        this.frame.repaint();
    }

    void changeToGetP(){
        this.frame.removeAll();
        JPanel currentPanel = loader.getGetFPanel();
        JMenuBar menuBar = loader.getMenuBar();
        this.frame.getContentPane().add(BorderLayout.NORTH,menuBar);
        this.frame.getContentPane().add(currentPanel);
        this.frame.revalidate();
        this.frame.repaint();
    }

    void changeToGetF(){
        this.frame.removeAll();
        JPanel currentPanel = loader.getGetPPanel();
        JMenuBar menuBar = loader.getMenuBar();
        this.frame.getContentPane().add(BorderLayout.NORTH,menuBar);
        this.frame.getContentPane().add(currentPanel);
        this.frame.revalidate();
        this.frame.repaint();
    }

}