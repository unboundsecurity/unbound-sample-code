import javax.security.auth.x500.X500Principal;

import sun.security.pkcs10.*;
import sun.security.x509.*;
import java.security.*;
import java.security.cert.X509Certificate;
import java.util.Date;

public class SelfSignCert
{
  public static void run() throws Exception
  {
    String subject = "CN=Test";
    KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA", "DYADIC");
    keyGen.initialize(2048, new SecureRandom());
    KeyPair keyPair = keyGen.generateKeyPair();
    byte[] csr = generateCSR(keyPair, subject);
    X509Certificate cert = signCSR(keyPair, subject, 365, csr);
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
}
