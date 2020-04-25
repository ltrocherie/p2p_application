import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
class loading {
    JPanel mainPanel;
    JMenuBar menuBar;
    JPanel lookPanel;
    JPanel IntPanel;
    JPanel GetPPanel;
    JPanel GetFPanel;

    void loadMainPage() {
        //Creating Buttons
        JLabel mainText = new JLabel("Welcome on the peer interface.\n"); // ALED j'arrive pas à rendre ça responsive
        JLabel blank = new JLabel("");
        mainText.setBounds(30, 120, 100, 100);
        JPanel Buttons = new JPanel(new GridLayout(3, 2, 10, 10));
        JButton buttonLo = new JButton("Look for a file");
        buttonLo.setBounds(130, 120, 100, 100);
        buttonLo.addActionListener(new ButtonListener());
        JButton buttonGet = new JButton("Get file loading informations");
        buttonGet.setBounds(130, 250, 100, 100);
        JButton buttonInt = new JButton("Announce interestment");
        buttonInt.setBounds(250, 120, 100, 100);
        JButton buttonGetP = new JButton("Get Pieces");
        buttonGetP.setBounds(250, 250, 100, 100);
        Buttons.add(mainText);
        Buttons.add(blank);
        Buttons.add(buttonLo);
        Buttons.add(buttonGet);
        Buttons.add(buttonGetP);
        Buttons.add(buttonInt);


        JMenuBar mb = new JMenuBar();
        JButton m1 = new JButton("Config");
        JButton m2 = new JButton("Close");
        mb.add(m1);
        mb.add(m2);

        //Creating the panel at bottom and adding components
        JTextField tf = new JTextField(10); // accepts upto 10 characters

        this.mainPanel = Buttons;
        this.menuBar = mb;
    }

    void loadLookPage() {

    }

    void loadGetFPage() {

    }

    void loadGetPPage() {

    }

    void loadIntPage() {

    }

    /*****************************   Getters **************************/
    public JPanel getMainPanel(){
        return this.mainPanel;
    }

    public JMenuBar getMenuBar(){
        return this.menuBar;
    }

    public JPanel getLookPanel() {
        return this.lookPanel;
    }

    public JPanel getIntPanel() {
        return IntPanel;
    }

    public JPanel getGetPPanel() {
        return GetPPanel;
    }

    public JPanel getGetFPanel() {
        return GetFPanel;
    }
}