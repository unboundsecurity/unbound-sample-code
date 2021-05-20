package com.unbound.samples.encrypt;

import java.util.ArrayList;
import java.util.concurrent.TimeUnit;

import javax.management.RuntimeErrorException;

import com.dyadicsec.cryptoki.CKR_Exception;
import com.unbound.samples.encrypt.UnboundCrypto.*;

// To view UKC log run:
//docker exec -it ukc-ep tail -f /opt/ekm/logs/ekm.log | grep test

//Curl Examples:

//REKEY:
// curl 'https://localhost:8443/api/v1/keys/0x0061774aca17fec252/rekey?partitionId=casp' \
//   -X 'POST' \
//   -H 'Accept: application/json, text/plain, */*' \
//   -H 'Authorization: Bearer <Enter Bearer Here>' \
//   --insecure

//DISABLE:

// curl 'https://localhost:8443/api/v1/keys/0x0001e026db44370ec5/disable?partitionId=casp' \
//   -H 'Accept: application/json' \
//   -H 'Authorization: Bearer <Enter Bearer Here>' \
//   -H 'Content-Type: application/json' \
//   --insecure

//REVOKE:

// curl 'https://localhost:8443/api/v1/keys/0x00f1848f5f985d89fc/revoke?partitionId=casp' \
//   -H 'Accept: application/json' \
//   -H 'Authorization: Bearer <Enter Bearer Here>' \
//   -H 'Content-Type: application/json' \
//   --data-raw '{"reason":"<Enter Reason Here>","message":"<Enter Message here>"}' \
//   --insecure



public class Main {

  /**
   * A timer for benchmarking crypto ops
   */
  public static class Timer {
    long startNano;
    long endNano;
    public void start() {
      startNano = System.nanoTime();
    }

    public void end(long opNumber) {
      endNano = System.nanoTime();
      double durationMs =  (endNano - startNano) / (double)1000000;
      double avgPerOp = durationMs / opNumber;
      System.out.printf(" > Encrypting %s times took: %.2f ms | Avg per op: %.2f ms\n", opNumber, durationMs, avgPerOp);
    }
  }
  
  public static class EncryptDemo {
    private static final String UKC_USER_PASSWORD = "Password1!";
    private static final String UKC_USER_NAME = "demo";

    private static final String NON_CACHED_KEY_LABEL  = "AES Not Cached";
    private static final String CACHED_KEY_LABEL = "AES Cached";

    // The keys used for testing
    private  AesKey cachedKey;
    private  AesKey nonCachedKey;
    UnboundCrypto ukc;
    Timer timer = new Timer();
    final static String plainText = "This is top secret data";
    // nubmber of repeated encrypt operations to do for each test.
    int encryptIterationCount = 100;
    // this is our storage of encrypted records. Each record stores the encrypted data
    // along with a reference to the key that was used for encryption.
    // This supports key rotation. On decryption we know which key was used for which record.
    ArrayList<EncryptedData> encryptedRecords = new ArrayList<EncryptedData>();
    
    public void init() throws CKR_Exception {
      ukc = new UnboundCrypto();
      ukc.login("encrypter","Password1!");

      nonCachedKey = ukc.findKeyByLabel(NON_CACHED_KEY_LABEL);
      if(nonCachedKey == null) {
        // create a non-exportable key that can't be cached on the client
        nonCachedKey = ukc.createAesKey(NON_CACHED_KEY_LABEL, false);
      }

      cachedKey = ukc.findKeyByLabel(CACHED_KEY_LABEL);
      if(cachedKey == null) {
        // create an exportable key that can be cached on the client
        cachedKey = ukc.createAesKey(CACHED_KEY_LABEL, true);
      }
    }

    private void testEncrypt(AesKey encryptionKey) throws CKR_Exception {
      timer.start();
      for (int i = 0; i < encryptIterationCount; i++) {
        encryptedRecords.add(encryptionKey.encrypt(plainText));  
      }
      timer.end(encryptIterationCount);
    }

    public void print(String message) {
      System.out.println(message);
    }

    public void printHeader(String header) {
      print("\n--------------------------------------------------------------------------");
      print("  " + header);
      print("--------------------------------------------------------------------------");
    }

    private void pressAnyKeyToContinue(){ 
      System.out.println("\nClick 'Enter' to continue...");
      try
      {
          System.in.read();
      }  
      catch(Exception e)
      {}  
    }

    public void runDemo() throws CKR_Exception, InterruptedException {
      print("--------------------------------------------------------------------------");
      print("        Welcome to Unbound ** Encrypt with key rotation demo **!");
      print("--------------------------------------------------------------------------");                                       
      print("This demo shows how to do encryption with key rotation using Unbound Key Control.");
      print("");
      print("The demo will show two modes of operation:");
      print("  1. Encryption is done on the client side: ");
      print("     This happens when the key is created as 'exportable' which allows client caching of the key material.");
      print("  2. Encryption is done on the server side with MPC: ");
      print("     This happens when the key is created as 'non-exportable' which prevent client access to the key material.");

      pressAnyKeyToContinue();

      printHeader("Before we start");
      print("During the demo its recommended to display the log of the UKC EP server.");
      print("This will help you understand exactly what's happenning.");
      print("To show the log run this command on the EP server:");
      print("    sudo tail -f /opt/ekm/logs/ekm.log");
      pressAnyKeyToContinue();

      // Initialization - creating keys
      printHeader("Initializing keys");
      print(" > Creating necessary AES keys if they don't already exist...");
      init();
      
      // Encryption using MPC with non cached key
      printHeader("Encryption with MPC (non-cached)");
      print("First we will encrypt with with the NON-CACHED key." + nonCachedKey.getKeyDataStr());
      print("Looking at the log you should notice " + encryptIterationCount + " ENCRYPT entries.");
      print("This indicates that the encryption was done using MPC on the server side.");
      pressAnyKeyToContinue();
      testEncrypt(nonCachedKey);
      pressAnyKeyToContinue();

      // Encryption on client with cached key
      printHeader("Encryption on client (cached)");
      print("Now lets do the same encyrption with the CACHED key." + cachedKey.getKeyDataStr());
      print("Looking at the log you should notice no new ENCRYPT entries.");
      print("Instead, you should see only one GET entry.");
      print("This indicates that the client fetched the key material and did all the encryption on the client machine.");
      print("You should also notice that the operation was much faster");
      pressAnyKeyToContinue();
      testEncrypt(cachedKey);
      pressAnyKeyToContinue();
      
      // key rotation
      printHeader("Key rotation");
      print("Now lets see what happens on key rotation.");
      print("Notice the key UID before rotation:" + cachedKey.getKeyDataStr());
      print("Please manually rotate the key named '" + cachedKey.label + "' by using UKC UI 'Rekey' operation.");
      String uidBefore = cachedKey.uid;
      pressAnyKeyToContinue();
      print("We now fetch the key again");
      cachedKey = ukc.findKeyByLabel(CACHED_KEY_LABEL);
      while(cachedKey.uid.equals(uidBefore)) {
        print("Looks like the key was not rotated. Please try again...");
        pressAnyKeyToContinue();
        cachedKey = ukc.findKeyByLabel(CACHED_KEY_LABEL);
      }
      print("Notice the key UID AFTER rotation has changed:" + cachedKey.getKeyDataStr());
      print("Lets now do encryption again using the new CACHED key.");
      pressAnyKeyToContinue();
      testEncrypt(cachedKey);
      pressAnyKeyToContinue();

      // Data recovery by decryption
      printHeader("Data recovery (decrypt)");
      print("To conclude we will demonstrate recovery by decrypting all records.");
      print("Each record is decrypted with the key that was used for its encryption.");
      for (EncryptedData encryptedData : encryptedRecords) {
        byte[] encrypted = encryptedData.key.decrypt(encryptedData);
        print("Decrypted with " + encryptedData.key.uid + ": " + new String(encrypted));
      }

      print("\nScroll through the lines above to verify the decryption was done properly.");
      pressAnyKeyToContinue();

      printHeader("Thats it... Thank you and Goodbye.");
      
      destroy();

    }

    public void destroy() throws CKR_Exception {
      ukc.logout();
    }

    public void deleteDemoKeys() throws CKR_Exception {
      deleteKey(NON_CACHED_KEY_LABEL);
      deleteKey(CACHED_KEY_LABEL);
    }

    public void deleteKey(String label) throws CKR_Exception {
      //delete old keys and create new ones
      AesKey key = ukc.findKeyByLabel(label);
      while(key != null) {
        key.delete();
        key = ukc.findKeyByLabel("non-sensitive");
      }
    }
  }
  

  public static void main(String[] args) throws Exception {
    EncryptDemo demo = new EncryptDemo();
    demo.runDemo();
    
  }
}

