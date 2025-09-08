#include <stdio.h>   // <- Standart lib
#include <string.h>  // <- Able to use string
#include <stdlib.h>  // <- Memory alloc functions.
#include <unistd.h>  // <- Used for ssize_t
#include <conio.h>
#include "info.h" // <- Func to print out what sys using, for debug.
#include "aes.h" // <- Header file with all the AES functions.

/*
Note: Could be improved with multipe threads.
Compile: gcc -o {name.exe} aesMain.c aes.c 
*/
#define SECRET_KEY "123456789012345" 

int key_input(const char *promt, char *userKey);
int main()
{
    print_system_info();
    char *message = "Encrypt me now!! or else!";
    uint8_t *pMessageHex = NULL;            // <- Text in hex
    size_t numBlocks = 0;                   
    uint8_t roundKeys[AES_ROUND_KEYS_SIZE]; 
    printf("The message is being encrypted. \n");

    // Prep the message.
    prepare_Block(message, &pMessageHex, &numBlocks);
    printf("Current action ended: prepare_Block. \n");
    printf("Padded message in hex: ");
    for (size_t i = 0; i < numBlocks * AES_BLOCK_SIZE; i++)
    {
        printf("%02x ", pMessageHex[i]);
        if ((i + 1) % AES_BLOCK_SIZE == 0)
            printf("\n");
    }

    // Prep the key.
    printf("The key is being expanded. \n");
    key_Expansion(SECRET_KEY, roundKeys);
    for (int i = 0; i < AES_ROUND_KEYS_SIZE; i++)
    {
        printf("%02x ", roundKeys[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }

    printf("The message in HEX is being encrypted. \n");
    // Encrypting the Hex message / Final encrypt
    printf("The message in HEX: ");
    for (size_t i = 0; i < numBlocks; i++)
    {
        aes_Encrypt(pMessageHex + (i * AES_BLOCK_SIZE), roundKeys);
    }

    for (int i = 0; i < AES_BLOCK_SIZE * numBlocks; i++)
    {
        printf("%02x", pMessageHex[i]);
    }
    printf("\n");

    printf("\n");
    printf("\n");

    // Decrypting.
    printf("The dycrpyting process begun!");
    for (size_t i = 0; i < numBlocks; i++)
    {
        de_Crypt(pMessageHex + (i * AES_BLOCK_SIZE), roundKeys);
    }
    printf("\n");

    printf("Message decrypted! \n");
    printf("The message in HEX (ASCII): ");
    for (int i = 0; i < AES_BLOCK_SIZE * numBlocks; i++)
    {
        printf("%02x", pMessageHex[i]);
    }
    printf("\n");
    revert_Hex(&pMessageHex);

    printf("Freeing the allocated message memory. \n");
    free(pMessageHex);
    return 0;
}




int key_input(const char *promt, char *userKey)
{
    int count = 0;
    char ch;

    printf("%s (type 'quit' to exit):\n", promt);
    printf("> ");

      while (1) {
        ch = _getch();

        if (ch == 13) {  
            userKey[count] = '\0';
            break;
        }

        if ((ch == 8 || ch == 127) && count > 0) {  
            count--;
            printf("\r> ");          
            for (int i = 0; i < count; i++)
                putchar(userKey[i]); 
            printf(" ");             
            printf(" [%d/%d]", count, AES_BLOCK_SIZE);
            fflush(stdout);
            continue;
        }

        if (count >= AES_BLOCK_SIZE) {
            continue;
        }

        userKey[count++] = ch;
        printf("\r> ");
        for (int i = 0; i < count; i++) {
            putchar(userKey[i]);
        }
        printf(" [%d/%d]  ", count, AES_BLOCK_SIZE); 
        fflush(stdout);
    }

    printf("\n");

    if (strcmp(userKey, "quit") == 0) {
        return 0;
    }

    if (count != AES_BLOCK_SIZE) {
        printf("Error: Key must be exactly %d characters.\n", AES_BLOCK_SIZE);
        return -1;
    }

    printf("Key accepted.\n");
    return 1;
}


/*
int main()
{
    print_system_info();
    char *message = "Encrypt me now!! or else!";
    uint8_t *pMessageHex = NULL;            // <- Text in hex
    size_t numBlocks = 0;                   // <- Number of block (2*16), for padding.
    uint8_t roundKeys[AES_ROUND_KEYS_SIZE]; // <- Expanded key in hex
    printf("The message is being encrypted. \n");

    // Prepering the message.
    prepare_Block(message, &pMessageHex, &numBlocks);
    printf("Current action ended: prepare_Block. \n");
    printf("Padded message in hex: ");
    for (size_t i = 0; i < numBlocks * AES_BLOCK_SIZE; i++)
    {
        printf("%02x ", pMessageHex[i]);
        if ((i + 1) % AES_BLOCK_SIZE == 0)
            printf("\n");
    }

    // Prepering the key.
    printf("The key is being expanded. \n");
    key_Expansion(SECRET_KEY, roundKeys);
    for (int i = 0; i < AES_ROUND_KEYS_SIZE; i++)
    {
        printf("%02x ", roundKeys[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }

    printf("The message in HEX is being encrypted. \n");
    // Encrypting the Hex message / Final encrypt
    printf("The message in HEX: ");
    for (size_t i = 0; i < numBlocks; i++)
    {
        aes_Encrypt(pMessageHex + (i * AES_BLOCK_SIZE), roundKeys);
    }

    for (int i = 0; i < AES_BLOCK_SIZE * numBlocks; i++)
    {
        printf("%02x", pMessageHex[i]);
    }
    printf("\n");

    printf("\n");
    printf("\n");

    // Decrypting.
    printf("The dycrpyting process begun!");
    for (size_t i = 0; i < numBlocks; i++)
    {
        de_Crypt(pMessageHex + (i * AES_BLOCK_SIZE), roundKeys);
    }
    printf("\n");

    printf("Message decrypted! \n");
    printf("The message in HEX (ASCII): ");
    for (int i = 0; i < AES_BLOCK_SIZE * numBlocks; i++)
    {
        printf("%02x", pMessageHex[i]);
    }
    printf("\n");
    revert_Hex(&pMessageHex);
    printf("The message in normal text in the for loop: ");

    printf("Freeing the allocated message memory. \n");
    free(pMessageHex);
    return 0;
}


*/


/*
int main()
{
    int userChoice; // User choice in select.
    char userKey;
    print_system_info();

    printf("Key input \n");
    while (1)
    {
        userChoice = key_input("Enter the 16 byte key or type \'quit\':", &userKey);
        if (userChoice == 1)
        {
            printf("Key accepted.\n");
            userChoice = (int)NULL;
            break;
        }
        else if (userChoice == 0)
        {
            printf("Exiting on user request.\n");
            userChoice = (int)NULL;
            break;
        }
    };

   
    while (1)
    {
        print_menu();

        if (scanf("%d", &userChoice) != 1)
        {
            printf("Invalid input. Exiting.\n");
            break;
        }
        getchar();

        switch (userChoice)
        {
        case 1:
            printf("Encrypt selected.\n");
            // Placeholder: call encryption function here
            break;

        case 2:
            printf("Decrypt selected.\n");
            // Placeholder: call decryption function here
            break;

        case 3:
            printf("Exiting...\n");
            return 0;

        default:
            printf("Invalid option. Please try again.\n");
        }
    }

    return 0;
}

*/