import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.ListIterator;


/**
 * */

public class AddFileSender extends PeerConfig implements Sender{

    DatFileParser parser;

    public AddFileSender(){
        this.parser = new DatFileParser();
    }

    public void sendMessage(ArrayList<JTextField> texts){ // on devrait pas passer le dossier en paramètre ?

    }

    public void addValues(ArrayList<JTextField> texts){
        if(texts.get(0).getText()!=""){
            this.parser.addFileTo(super.seedFile,texts.get(0).getText());
        }
        texts.get(0).setText("");
    }

    public void flush(ArrayList<JTextField> texts){
        for(JTextField text : texts) {
            text.setText("");
        }
    }

}
