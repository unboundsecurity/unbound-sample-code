

import com.dyadicsec.advapi.SDEKey;
import com.dyadicsec.advapi.SDESessionKey;

import java.util.UUID;

/**
 *
 * This sample contains 2 basic SDE operations :
 *  + string order preserving encryption.
 *  + credit card type preserving encryption.
 *
 */

public class Main {

    public static void main(String[] args) {

        if (args.length != 2) {
            System.out.println("Invalid args, should be : <partition>  <key_name>");
            System.exit(1);
        }

        String partition = args[0];
        String keyName = args[1];

        System.out.println(String.format("Partition : %s", partition));
        System.out.println(String.format("Key Name : %s", keyName));

        // find key by name and partition
        SDEKey sdeKey = SDEKey.findKey(partition, keyName);
        if (sdeKey == null) {
            System.out.println(String.format("Key not found, You can create it using 'ucl generate -t PRF --name %s -p %s'",
                    keyName
                    , partition));
            System.exit(1);
        } else {
            System.out.println("Key successfully loaded");
        }

        encryptOrderPreserving(sdeKey);
        encryptTypePreserving(sdeKey);
    }

    /**
     * generate SDE session key
     *
     * @param sdeKey
     * @param purpose
     * @return
     */
    private static SDESessionKey generateSessionKey(SDEKey sdeKey, int purpose) {
        String tweak = UUID.randomUUID().toString();
        return sdeKey.generateSessionKey(purpose, tweak);
    }

    /**
     * encrypt string order preserving
     *
     * @param sdeKey
     */
    private static void encryptOrderPreserving(SDEKey sdeKey) {

        System.out.println("Encrypt String Order Preserving");

        SDESessionKey sdeSessionKey = generateSessionKey(sdeKey, SDEKey.PURPOSE_OP_ENC);

        String str1 = UUID.randomUUID().toString();
        String str2 = UUID.randomUUID().toString();

        String encStr1 = sdeSessionKey.encryptOrderPreserving(str1, 36);
        String encStr2 = sdeSessionKey.encryptOrderPreserving(str2, 36);

        String decStr1 = sdeSessionKey.decryptOrderPreserving(encStr1);
        String decStr2 = sdeSessionKey.decryptOrderPreserving(encStr2);

        System.out.println(String.format("  Validating order : %s",
                str1.compareTo(str2) * encStr1.compareTo(encStr2) >= 0 ? "OK" : "FAILED"));

        System.out.println(String.format("  Validating correctness : %s",
                str1.equals(decStr1) && str2.compareTo(decStr2) >= 0 ? "OK" : "FAILED"));
    }

    /**
     * encrypt credit card type preserving
     *
     * @param sdeKey
     */
    private static void encryptTypePreserving(SDEKey sdeKey) {

        System.out.println("Encrypt Credit Card Type Preserving");

        SDESessionKey sdeSessionKey = generateSessionKey(sdeKey, SDEKey.PURPOSE_CREDIT_CARD_ENC);

        String creditCard = "378282246310005";

        String encCreditCard = sdeSessionKey.encryptCreditCard(creditCard);
        String decCreditCard = sdeSessionKey.decryptCreditCard(encCreditCard);

        System.out.println(String.format("  Original Credit Card : %s", creditCard));
        System.out.println(String.format("  Encrypted Credit Card : %s", encCreditCard));
        System.out.println(String.format("  Validating correctness : %s",
                creditCard.equals(decCreditCard) ? "OK" : "FAILED"));
    }

}
