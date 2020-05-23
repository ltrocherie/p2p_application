import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
public class gui extends PeerConfig{

    public static void main(String args[]) {
       PeerConfig.getElementFromConfig();
       guiRunner gR= new guiRunner();
       gR.run();
       (new Thread(new ReceiveFromPeer("127.0.0.1", inPort))).start();

    }
}


//
//    public class ButtonListener implements ActionListener{
//        private void function();
//        public ButtonListener(void function){
//            this.function = function;
//        }
//        public void actionPerformed(ActionEvent clic) {
//            this.function();
//        }
//    }
//
//    private void lookPage(){
//        construct_and_display_panel();
//    }
