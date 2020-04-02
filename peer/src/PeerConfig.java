public class PeerConfig{
  static int port = 8080; // pas une super idée le port http pour nos connexions

  static int inPort = 10000; // A voir, un pour les communications, l'autre pour les transferts de fichier
  static int outPort = 10000;

  static final String folderName = "seed/";
  static int pieceSize = 1024;
}
