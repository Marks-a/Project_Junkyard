#include <stdio.h>   // <- Standart lib
#include <stdbool.h> // <- Bool lib
#include "aes.h"
#include "sbox.h"
#include "rcon.h"

void prepare_Block(const char *input, uint8_t **block, size_t *num_block)
{
    size_t len = strlen(input);                               // <- Converted into a full string from char, then into a int.
    *num_block = (len + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE; // <- Number of blocks needed, for the extra padding and such

    size_t total_size = (*num_block) * AES_BLOCK_SIZE; // <- Each block 16, if 2 or more, 16*n to get the full size (byte)

    *block = (uint8_t *)calloc(total_size, sizeof(uint8_t)); // <- Allocated mem for total size. Ammount(16*n)/size(8 int)

    if (*block == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    memcpy(*block, input, len); // Copy input to pad blocks. *block -> dest, input -> source, len -> amount

    // PKCS 7 padding
    uint8_t pad_value = total_size - len; // if 24 -> next multiple of 16 is 32 → pad = 8 (So there would not be 0x00 in empty spaces.)
    for (size_t i = len; i < total_size; i++)
    {
        (*block)[i] = pad_value;
    }
};

void revert_Hex(uint8_t **block)
{
    /*Note:
    This expects the **block to be in the PCSK 7 padding to be pressent.
    */
    if (*block == NULL)
    {
        fprintf(stderr, "Error: block is null.\n");
        return;
    }

    uint8_t *data = *block;
    size_t i = 0; // <-  Pad if exist.

    while (1)
    {
        // Keep looking until we find a multiple of 16 followed by valid padding
        if ((i % AES_BLOCK_SIZE == 0) && i > 0)
        {
            uint8_t pad_value = data[i - 1];

            // Validate PKCS#7 pad
            if (pad_value > 0 && pad_value <= AES_BLOCK_SIZE)
            {
                int valid = 1;
                for (int j = 0; j < pad_value; j++)
                {
                    if (data[i - 1 - j] != pad_value)
                    {
                        valid = 0;
                        break;
                    }
                }
                if (valid)
                {
                    size_t msg_len = i - pad_value;
                    char *text = malloc(msg_len + 1);
                    if (!text)
                    {
                        fprintf(stderr, "Memory allocation failed\n");
                        exit(1);
                    }
                    memcpy(text, data, msg_len);
                    text[msg_len] = '\0';

                    printf("\nRecovered message and copied to original *ptr: %s", text);
                    printf("\n");
                    free(text);
                    return;
                }
            }
        }
        i++;
        /*
        Note: Future dev.
        Safety check.
        Checks whenever the loop infinite it would break the loop.
        */
        if (i > MAX_BYTES)
        {
            fprintf(stderr, "Could not determine original size.\n");
            return;
        }
    }
};

void key_Expansion(const uint8_t *key, uint8_t *roundKeys)
{
    // Copy original key into first 16 bytes
    memcpy(roundKeys, key, AES_BLOCK_SIZE); // <- 0-15 is key, 16-176 is 00 (unasigned).

    int bytesGenerated = AES_BLOCK_SIZE; // <- 16 bytes
    int rconIndex = 1;
    uint8_t temp[4]; // <- Creating a temp

    while (bytesGenerated < AES_ROUND_KEYS_SIZE)
    {
        // Copy last 4 bytes from roundKeys to temp
        for (int i = 0; i < 4; i++)
        {
            temp[i] = roundKeys[bytesGenerated - 4 + i];
        }

        // Every 16 bytes (4 words), apply key schedule core
        if (bytesGenerated % AES_BLOCK_SIZE == 0)
        {
            rotWord(temp);
            subWord(temp);
            temp[0] ^= rcon[rconIndex++];
        }

        // XOR with 4 bytes from 16 bytes earlier
        for (int i = 0; i < 4; i++)
        {
            roundKeys[bytesGenerated] = roundKeys[bytesGenerated - AES_BLOCK_SIZE] ^ temp[i];
            bytesGenerated++;
        }
    }
}
void rotWord(uint8_t *word)
{
    uint8_t tmp = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = tmp;
}
void subWord(uint8_t *word)
{
    for (int i = 0; i < 4; i++)
    {
        word[i] = s_box[word[i]];
    }
}

void aes_Encrypt(uint8_t *block, uint8_t *roundKeys)
{
    /*
     Problem solved of not matching online cipher encryption !!!
    The proble was the round loops.
    The first round is the 0th.
    But the for loop started from i = 0, but needed to be 1.
*/
    // The 0th round
    add_Round(block, roundKeys);

    // The 1-9 rounds
    for (int i = 1; i < AES_ROUNDS; i++)
    {
        sub_Bytes(block);
        shift_Rows(block);
        mix_Column(block);
        add_Round(block, roundKeys + i * AES_BLOCK_SIZE);
    }

    // The final round without mix_Column func
    sub_Bytes(block);
    shift_Rows(block);
    add_Round(block, roundKeys + 10 * AES_BLOCK_SIZE);
}
void add_Round(uint8_t *block, uint8_t *roundKeys)
{
    for (int i = 0; i < 16; ++i)
        block[i] ^= roundKeys[i];
}
void sub_Bytes(uint8_t *block)
{
    for (int i = 0; i < 16; ++i)
    {
        block[i] = s_box[block[i]];
    }
}
void shift_Rows(uint8_t *block)
{
    uint8_t temp;

    // Row 1 shift by 1
    temp = block[1];
    block[1] = block[5];
    block[5] = block[9];
    block[9] = block[13];
    block[13] = temp;

    // Row 2 shift by 2
    temp = block[2];
    block[2] = block[10];
    block[10] = temp;
    temp = block[6];
    block[6] = block[14];
    block[14] = temp;

    // Row 3 shift by 3
    temp = block[3];
    block[3] = block[15];
    block[15] = block[11];
    block[11] = block[7];
    block[7] = temp;
}
void mix_Column(uint8_t *block)
{
    uint8_t temp[16];
    for (int i = 0; i < 4; ++i)
    {
        int col = i * 4;
        temp[col] = gmul(0x02, block[col]) ^ gmul(0x03, block[col + 1]) ^ block[col + 2] ^ block[col + 3];
        temp[col + 1] = block[col] ^ gmul(0x02, block[col + 1]) ^ gmul(0x03, block[col + 2]) ^ block[col + 3];
        temp[col + 2] = block[col] ^ block[col + 1] ^ gmul(0x02, block[col + 2]) ^ gmul(0x03, block[col + 3]);
        temp[col + 3] = gmul(0x03, block[col]) ^ block[col + 1] ^ block[col + 2] ^ gmul(0x02, block[col + 3]);
    }
    for (int i = 0; i < 16; ++i)
    {
        block[i] = temp[i];
    }
}

// Decrypting function.
void de_Crypt(uint8_t *block, uint8_t *roundKeys)
{
    // Round going backwards
    uint8_t round = AES_ROUNDS;

    add_Round(block, roundKeys + round * AES_BLOCK_SIZE);

    for (round = AES_ROUNDS - 1; round > 0; round--)
    {
        inv_Rows(block);
        inv_Bytes(block);
        add_Round(block, roundKeys + round * AES_BLOCK_SIZE);
        inv_Column(block);
    }

    // The 0th round
    inv_Rows(block);
    inv_Bytes(block);
    add_Round(block, roundKeys);
}
// Additional function for dycrypting
void inv_Rows(uint8_t *block)
{
    uint8_t temp;

    // Row 1
    temp = block[13];
    block[13] = block[9];
    block[9] = block[5];
    block[5] = block[1];
    block[1] = temp;

    // 2
    temp = block[2];
    block[2] = block[10];
    block[10] = temp;
    temp = block[6];
    block[6] = block[14];
    block[14] = temp;

    // 3
    temp = block[3];
    block[3] = block[7];
    block[7] = block[11];
    block[11] = block[15];
    block[15] = temp;
};
void inv_Bytes(uint8_t *block)
{
    for (int i = 0; i < AES_BLOCK_SIZE; i++)
    {
        block[i] = inv_s_box[block[i]];
    }
};
void inv_Column(uint8_t *block)
{
    uint8_t tmp[16];

    for (int i = 0; i < 4; i++)
    {
        int col = i * 4;

        tmp[col] = gmul(block[col], 0x0e) ^ gmul(block[col + 1], 0x0b) ^ gmul(block[col + 2], 0x0d) ^ gmul(block[col + 3], 0x09);
        tmp[col + 1] = gmul(block[col], 0x09) ^ gmul(block[col + 1], 0x0e) ^ gmul(block[col + 2], 0x0b) ^ gmul(block[col + 3], 0x0d);
        tmp[col + 2] = gmul(block[col], 0x0d) ^ gmul(block[col + 1], 0x09) ^ gmul(block[col + 2], 0x0e) ^ gmul(block[col + 3], 0x0b);
        tmp[col + 3] = gmul(block[col], 0x0b) ^ gmul(block[col + 1], 0x0d) ^ gmul(block[col + 2], 0x09) ^ gmul(block[col + 3], 0x0e);
    }

    memcpy(block, tmp, 16);
};

// Function used in mix_Column and inv_Column,
uint8_t gmul(uint8_t a, uint8_t b)
{
    uint8_t p = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (b & 1)
            p ^= a;
        bool hi_bit_set = a & 0x80;
        a <<= 1;
        if (hi_bit_set)
            a ^= 0x1b;
        b >>= 1;
    }
    return p;
}