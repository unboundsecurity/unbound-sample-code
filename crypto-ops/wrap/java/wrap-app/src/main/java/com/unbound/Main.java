package com.unbound;

import com.dyadicsec.cryptoki.CKR_Exception;
import org.apache.commons.cli.*;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.security.GeneralSecurityException;

public class Main {

    private static final String JCE = "JCE";
    private static final String PKCS11 = "PKCS11";
    private static final String GEN_WRAPPING_KEY = "GEN_WRAPPING_KEY";
    private static final String GEN_WRAPPED_KEY = "GEN_WRAPPED_KEY";
    private static final String WRAP = "WRAP";
    private static final String IMPORT = "IMPORT";
    private static App app;
    private static Options options = new Options();
    private static CommandLine commandLine;

    static {
        options.addOption(Option.builder("u")
                .longOpt("user")
                .hasArg(true)
                .build());

        options.addOption(Option.builder("w")
                .longOpt("password")
                .hasArg(true)
                .build());

        options.addOption(Option.builder("t")
                .longOpt("token")
                .hasArg(true)
                .build());

        options.addOption(Option.builder("o")
                .longOpt("operation")
                .required()
                .desc("GEN_WRAPPING_KEY | GEN_WRAPPED_KEY | WRAP | IMPORT")
                .hasArg(true)
                .build());

        options.addOption(Option.builder("a")
                .longOpt("application")
                .required()
                .desc("JCE | PKCS11")
                .hasArg(true)
                .build());

        options.addOption(Option.builder("wrapping")
                .hasArg(true)
                .build());

        options.addOption(Option.builder("wrapped")
                .hasArg(true)
                .build());

        options.addOption(Option.builder("imported")
                .hasArg(true)
                .build());

        options.addOption(Option.builder("key")
                .hasArg(true)
                .build());

    }

    private static void halt(String error) {
        System.out.println(error);
        System.exit(1);
    }

    private static String readFile(File file) throws IOException {
        FileInputStream fileInputStream = new FileInputStream(file);
        byte[] buffer = new byte[fileInputStream.available()];
        int length = fileInputStream.read(buffer);
        fileInputStream.close();
        return new String(buffer, 0, length);
    }

    public static void main(String[] args) throws GeneralSecurityException, ParseException, IOException, CKR_Exception {

        initCommandLine(args);

        initAPP();

        doIt();
    }

    private static void initCommandLine(String[] args) throws ParseException {
        commandLine = new DefaultParser().parse(options, args);
    }

    private static void initAPP() throws ParseException, GeneralSecurityException, IOException, CKR_Exception {
        String appType = (String) commandLine.getParsedOptionValue("a");
        String user = (String) commandLine.getParsedOptionValue("u");
        String password = (String) commandLine.getParsedOptionValue("w");
        String token = (String) commandLine.getParsedOptionValue("t");

        switch (appType) {
            case JCE:
                System.out.println("Using JCE application");
                app = new JCEApp();
                break;
            case PKCS11:
                System.out.println("Using PKCS11 application");
                app = new PKCS11App();
                break;
            default:
                halt("Unsupported App");
        }
        app.load(user,password,token);
    }

    private static void doIt() throws ParseException, GeneralSecurityException, CKR_Exception, IOException {
        String operation = (String) commandLine.getParsedOptionValue("o");
        String wrappingKeyName = (String) commandLine.getParsedOptionValue("wrapping");
        String wrappedKeyName = (String) commandLine.getParsedOptionValue("wrapped");
        String importedKeyName = (String) commandLine.getParsedOptionValue("imported");
        String importedKeyData = (String) commandLine.getParsedOptionValue("key");

        switch (operation) {
            case GEN_WRAPPING_KEY:
                if (wrappingKeyName == null) halt("Missing wrapping key name");
                app.generateWrappingKey(wrappingKeyName);
                break;
            case GEN_WRAPPED_KEY:
                if (wrappedKeyName == null) halt("Missing wrapped key name");
                app.generateWrappedKey(wrappedKeyName);
                break;
            case WRAP:
                if (wrappingKeyName == null) halt("Missing wrapping key name");
                if (wrappedKeyName == null) halt("Missing wrapped key name");
                app.wrap(wrappingKeyName, wrappedKeyName);
                break;
            case IMPORT:
                if (importedKeyName == null) halt("Missing imported key name");
                if (importedKeyData == null) halt("Missing key data");
                app.importKey(importedKeyName, readFile(new File(importedKeyData)));
                break;
            default:
                halt("Unsupported Operation");
        }
    }
}
