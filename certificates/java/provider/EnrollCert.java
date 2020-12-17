import sun.security.pkcs10.PKCS10;
import sun.security.x509.*;

import javax.security.auth.x500.X500Principal;
import java.security.*;
import java.security.cert.X509Certificate;
import java.util.Date;

public class EnrollCert
{
  private static KeyPair generateKeyPair(String provider) throws Exception
  {
    KeyPairGenerator keyGen = (provider==null) ?
      KeyPairGenerator.getInstance("RSA") :
      KeyPairGenerator.getInstance("RSA", provider);

    keyGen.initialize(2048, new SecureRandom());
    return keyGen.generateKeyPair();
  }

  private static byte[] generateCSR(KeyPair caKeyPair, String subject) throws Exception
  {
    // generate PKCS10 certificate request
    PKCS10 pkcs10 = new PKCS10(caKeyPair.getPublic());
    Signature signature = Signature.getInstance("SHA256WithRSA");
    signature.initSign(caKeyPair.getPrivate());

    X500Principal principal = new X500Principal(subject);

    X500Name x500name = new X500Name(principal.getEncoded());
    pkcs10.encodeAndSign(x500name, signature);
    return pkcs10.getEncoded();
  }

  private static X509Certificate signCSR(KeyPair caKeyPair, String issuer, int days, byte[] csr) throws Exception
  {
    Date from = new Date();
    Date to = new Date(from.getTime() + days * 24*60*60*1000);
    X509CertInfo info = new X509CertInfo();
    info.set(X509CertInfo.VALIDITY, new CertificateValidity(from, to));
    info.set(X509CertInfo.SERIAL_NUMBER, new CertificateSerialNumber(new java.util.Random().nextInt() & 0x7fffffff));
    info.set(X509CertInfo.VERSION, new CertificateVersion(CertificateVersion.V3));
    info.set(X509CertInfo.ALGORITHM_ID, new CertificateAlgorithmId(AlgorithmId.get("SHA256WithRSA")));
    info.set(X509CertInfo.ISSUER, new X500Name(issuer));

    PKCS10 req = new PKCS10(csr);

    info.set(X509CertInfo.KEY, new CertificateX509Key(req.getSubjectPublicKeyInfo()));
    info.set(X509CertInfo.SUBJECT, req.getSubjectName());
    X509CertImpl cert = new X509CertImpl(info);
    cert.sign(caKeyPair.getPrivate(), "SHA256WithRSA");
    return new X509CertImpl(cert.getEncodedInternal());
  }

  private static X509Certificate generateSelfSignCert(KeyPair keyPair, String subject, int days) throws Exception
  {
    byte[] csr = generateCSR(keyPair, subject);
    return signCSR(keyPair, subject, days, csr);
  }

  public static void run() throws Exception
  {
    KeyPair caKeyPair = generateKeyPair(null);
    String issuer = "CN=Issuer";
    X509Certificate caCert = generateSelfSignCert(caKeyPair, issuer, 365);

    KeyPair userKeyPair = generateKeyPair("DYADIC");
    String subject = "CN=Subject";
    byte[] csr = generateCSR(userKeyPair, subject);

    X509Certificate cert = signCSR(caKeyPair, issuer, 365, csr);
  }
}
