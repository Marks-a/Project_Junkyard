import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.Arrays;
import java.io.File;

public class aes128 {
    static final int BLOCK_SIZE = 16;
    static final int KEY_SIZE = 16;
    static final int KEY_EXPANDED = 176;
    static final int ROUNDS = 10;
    static final int EXPANDED_KEY_SIZE = 176;

    public  void main(String[] args) {
        System.out.println("Running");
        File f = new File("toEncrypt.txt");

        String content = "";
        try {
            content = Files.readString(f.toPath());
        } catch (java.io.IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
            e.printStackTrace();
        }
        String key = "MySecretKey12345";
        byte[] byteKey = key.getBytes(StandardCharsets.UTF_8);
        byte[] byteMsg = content.getBytes(StandardCharsets.UTF_8);
        // byte[] byteMsg = msg.getBytes(StandardCharsets.UTF_8);
        String encryptedHex = encrypt(byteMsg, byteKey);
        pressEnterToContinue();
        System.out.println("Encrypted: " + encryptedHex);
        
    }

    /*
     * Main encrypt
     */
    public static String encrypt(byte[] plainT, byte[] key) {
        if (key.length != KEY_SIZE)
            throw new IllegalArgumentException("Invalid key size");
        byte[] padded = AESPad.pkcs7Pad(plainT);
        byte[] roundKeys = AESFunctions.keyExpansion(key);

        byte[] cipherT = new byte[padded.length];
        for (int i = 0; i < padded.length; i += BLOCK_SIZE) {
            byte[] block = Arrays.copyOfRange(padded, i, i + BLOCK_SIZE);
            byte[] encryptedBlock = encryptBlock(block, roundKeys);
            System.arraycopy(encryptedBlock, 0, cipherT, i, BLOCK_SIZE);
        }
        String cipherTHex = AESFunctions.bytesToHex(cipherT);
        return cipherTHex;
    }

    public static byte[] encryptBlock(byte[] plaintext16, byte[] key) {
        if (plaintext16.length != 16)
            throw new IllegalArgumentException("AES-128 operates on 16-byte block and 16-byte key.");

        byte[] state = Arrays.copyOf(plaintext16, 16);
        AESFunctions.addRoundKey(state, key, 0);
        for (int round = 1; round <= 9; round++) {
            AESFunctions.subBytes(state);
            AESFunctions.shiftRows(state);
            AESFunctions.mixColumns(state);
            AESFunctions.addRoundKey(state, key, round);
        }
        // Final round (no MixColumns)
        AESFunctions.subBytes(state);
        AESFunctions.shiftRows(state);
        AESFunctions.addRoundKey(state, key, 10);

        return state;
    }

    /*
     * ####################################################################
     */
    public static void encryptedFileCreate(String encryptedString) {
        File encryptedFile = new File("encrypted.txt");
        try {
            encryptedFile.createNewFile();
            Files.writeString(encryptedFile.toPath(), encryptedString);
        } catch (java.io.IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
            e.printStackTrace();
        }
    }

    private void pressEnterToContinue()
 { 
        System.out.println("Press Enter key to continue...");
        try
        {
            System.in.read();
        }  
        catch(Exception e)
        {}  
 }
}
