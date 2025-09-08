
public class AESFunctions {
    public AESFunctions() {
    }

    /*
     * ==============================================
     * Key Expansion
     */
    
public static byte[] keyExpansion(byte[] key) {
        // AES-128: expand 16-byte key to 176 bytes (11 round keys)
        byte[] w = new byte[176];
        System.arraycopy(key, 0, w, 0, 16);
        byte[] temp = new byte[4];
        int bytesGenerated = 16;
        int rconIter = 1;

        while (bytesGenerated < 176) {
            // last 4 bytes
            System.arraycopy(w, bytesGenerated - 4, temp, 0, 4);

            if (bytesGenerated % 16 == 0) {
                // RotWord
                byte t = temp[0];
                temp[0] = temp[1]; temp[1] = temp[2]; temp[2] = temp[3]; temp[3] = t;
                // SubWord
                temp[0] = (byte)AESBox.SBOX[temp[0] & 0xFF];
                temp[1] = (byte)AESBox.SBOX[temp[1] & 0xFF];
                temp[2] = (byte)AESBox.SBOX[temp[2] & 0xFF];
                temp[3] = (byte)AESBox.SBOX[temp[3] & 0xFF];
                // Rcon
                temp[0] ^= (byte)AESBox.RCON[rconIter++];
            }

            for (int i = 0; i < 4; i++) {
                w[bytesGenerated] = (byte)(w[bytesGenerated - 16] ^ temp[i]);
                bytesGenerated++;
            }
        }
        return w;
    }
    /*
     * ==============================================
     * Main function
     */
public static void subBytes(byte[] state) {
        for (int i = 0; i < 16; i++) state[i] = (byte)AESBox.SBOX[state[i] & 0xFF];
    }

    public static void shiftRows(byte[] s) {
        // state is column-major; rows are 0..3
        // row 1 shift left by 1
        byte tmp;

        // row 1 (indexes 1,5,9,13)
        tmp = s[1]; s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = tmp;
        // row 2 (indexes 2,6,10,14) shift left by 2
        byte t1 = s[2], t2 = s[6];
        s[2] = s[10]; s[6] = s[14]; s[10] = t1; s[14] = t2;
        // row 3 (indexes 3,7,11,15) shift left by 3 (i.e., right by 1)
        tmp = s[15]; s[15] = s[11]; s[11] = s[7]; s[7] = s[3]; s[3] = tmp;
    }

    public static void mixColumns(byte[] s) {
        for (int c = 0; c < 4; c++) {
            int i = c * 4;
            byte a0 = s[i], a1 = s[i+1], a2 = s[i+2], a3 = s[i+3];
            s[i]   = (byte)( (mul(a0,2) ^ mul(a1,3) ^ mul(a2,1) ^ mul(a3,1)) & 0xFF );
            s[i+1] = (byte)( (mul(a0,1) ^ mul(a1,2) ^ mul(a2,3) ^ mul(a3,1)) & 0xFF );
            s[i+2] = (byte)( (mul(a0,1) ^ mul(a1,1) ^ mul(a2,2) ^ mul(a3,3)) & 0xFF );
            s[i+3] = (byte)( (mul(a0,3) ^ mul(a1,1) ^ mul(a2,1) ^ mul(a3,2)) & 0xFF );
        }
    }

    public static void addRoundKey(byte[] state, byte[] roundKey, int round) {
        int offset = round * 16;
        for (int i = 0; i < 16; i++) state[i] ^= roundKey[offset + i];
    }
    

    /*
     * ==========================================================================
     * Helper function
     */
    public static byte xtime(byte x) {
        int xi = x & 0xFF;
        int res = xi << 1;
        if ((xi & 0x80) != 0) res ^= 0x1B;
        return (byte)(res & 0xFF);
    }

    public static byte mul(byte x, int by) {
        // multiply in GF(2^8) by {1,2,3}
        int xi = x & 0xFF;
        switch (by) {
            case 1: return (byte)xi;
            case 2: return xtime(x);
            case 3: return (byte)((xtime(x) ^ xi) & 0xFF);
            default: throw new IllegalArgumentException("mul by must be 1,2,3");
        }
    }

    public static byte[] hexToBytes(String hex) {
        if ((hex.length() & 1) != 0) throw new IllegalArgumentException("Hex length must be even");
        byte[] out = new byte[hex.length() / 2];
        for (int i = 0; i < out.length; i++) {
            int hi = Character.digit(hex.charAt(2*i), 16);
            int lo = Character.digit(hex.charAt(2*i+1), 16);
            if (hi < 0 || lo < 0) throw new IllegalArgumentException("Bad hex");
            out[i] = (byte)((hi << 4) | lo);
        }
        return out;
    }
    public static String bytesToHex(byte[] b) {
        StringBuilder sb = new StringBuilder(b.length * 2);
        for (byte x : b) sb.append(String.format("%02x", x & 0xFF));
        return sb.toString();
    }

    // public static String textTo


}