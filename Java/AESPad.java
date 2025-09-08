import java.util.Arrays;

public class AESPad {

    public static byte[] pkcs7Pad(byte[] data) {
        int padLen = aes128.BLOCK_SIZE - (data.length % aes128.BLOCK_SIZE);
        if (padLen == 0) padLen = aes128.BLOCK_SIZE;
        byte[] out = Arrays.copyOf(data, data.length + padLen);
        Arrays.fill(out, data.length, out.length, (byte) padLen);
        return out;
    }

}
