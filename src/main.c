

//void setupESNIconnection(){
//
//}
//
//void sendReceiveESNI(){
//
//
//}
//
//void closeESNIconnection(){
//
//
//}


#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

char *buffer;



struct MemoryStruct {
    char *memory;
    size_t size;
};

size_t curlWriteFunction(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *) userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

void string2hexString(uint8_t *input, char *output, uint8_t length)
{
    int loop;
    int i;

    i=0;
    loop=0;

    while(loop < length)
    {
        sprintf((char*)(output+i),"%02X", input[loop]);
        loop+=1;
        i+=2;
    }
    //insert NULL at the end of the output string
    output[i++] = '\0';
}


int sessionIDgen(char *sessionID)
{
    //add memory management free()

    uint8_t *rnd = (uint8_t *)malloc(64 );

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    int resultCode = RAND_bytes(rnd,64); //fil rnd with 64 random bytes

    if (resultCode == 1){ //1=successful random number gen
        //successful rng
        SHA256_Update(&sha256, rnd, 64); //add rnd value to the message to be hashed
        SHA256_Final(hash, &sha256); //sha256hash the message and store in hash
        string2hexString(hash, sessionID, 16); //convert bytes to string and store in the var passed to func
        return 0; //success rng
    }else{

        return 1; //fail rng
    }

}


int main(void)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */


    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        //curl_easy_setopt();

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_URL, "https://only.esni.defo.ie");
        curl_easy_setopt(curl, CURLOPT_ESNI_STATUS, CURLESNI_ENABLE | CURLESNI_STRICT);
        curl_easy_setopt(curl, CURLOPT_ESNI_SERVER, "only.esni.defo.ie");
        curl_easy_setopt(curl, CURLOPT_ESNI_COVER, "cover.defo.ie");
        curl_easy_setopt(curl, CURLOPT_ESNI_ASCIIRR, "/wHKIsrOACQAHQAgQEj0hmUWbuMxfmHelxojc+Z/U52zI7GpK86S5dRmcVQAAhMBAQQAAAAAXfpreAAAAABd+oCQAAA=");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curlWriteFunction);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "data data data meh");

        char *sessionID = (char*)malloc( (16 * 2) +1 );

        int resultCode = sessionIDgen(sessionID);

        if(resultCode == 0) {
            //string2hexString(randomBuffer, hexRandomBuffer, 64);
            printf("SessionID: %s\n", sessionID);

            //return 0;
        }else {
            printf("Not enough random bytes for PRNG");
            return 1;
        }
        char sessionIDheader[(16*2)+1+14] = "X-Session-Id: ";
        strcat(sessionIDheader,sessionID);
        printf("sessionheader::  %s\n",sessionIDheader);
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "User-Agent: ");
        list = curl_slist_append(list, sessionIDheader);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);


#ifdef SKIP_PEER_VERIFICATION
        /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
        /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }else{
            printf("--------\n");
            printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
            printf("PageData: \n%s", (char*)chunk.memory);
        }
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}