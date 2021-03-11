using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;

/**************************************************************************************
 * This class overloads standard WebClient to add a client certificate to the request 
 */
class WebClientWithCert : WebClient
{
  private readonly X509Certificate2 certificate;
  public WebClientWithCert(string cert_file, string pwd)
  {
    certificate = new X509Certificate2(cert_file, pwd);
  }
  protected override WebRequest GetWebRequest(Uri address)
  {
    HttpWebRequest request = (HttpWebRequest)base.GetWebRequest(address);
    request.ClientCertificates.Add(certificate);
    return request;
  }

}

public class EncryptData
{
  static void set_server_cert_validation(bool check_server_ca, string server_ca_pem)
  {
    if (check_server_ca)
    {
      X509Certificate2 server_cert = new X509Certificate2(server_ca_pem);
      var validCerts = new Dictionary<X509Certificate2, string>()
      {
        { server_cert, server_cert.GetCertHashString() },
      };
      ServicePointManager.ServerCertificateValidationCallback += (sender, cert, chain, errors) =>
      {
        X509Certificate2 server_root_ca = chain.ChainElements[chain.ChainElements.Count - 1].Certificate;
        return errors == SslPolicyErrors.None
          && validCerts.ContainsValue(server_root_ca.GetCertHashString());
      };
    }
    else //Accept any certificate
      ServicePointManager.ServerCertificateValidationCallback = (sender, cert, chain, errors) => true;
  }
  public static string base64_encode(string plainText)
  {
    var plainTextBytes = System.Text.Encoding.UTF8.GetBytes(plainText);
    return System.Convert.ToBase64String(plainTextBytes);
  }

  static void set_request_headers(ref WebClientWithCert client)
  {
    client.Headers.Add("Content-Type", "application/json");

    //Default UKC user
    const string default_ukc_user = "user";
    //Encode UKC user with colon (:) at end
    string default_ukc_user_base64 = base64_encode(default_ukc_user + ":");

    //Add result to Authorization header
    client.Headers.Add("Authorization", $"Basic {default_ukc_user_base64}");
  }

  public static void Main(string[] args)
  {
    //Client certificate location and password
    const string client_cert_file = "/path/to/client/certificate/file";
    const string client_cert_pwd = "cert-file-password";

    //Set encryption parameters
    const string key_name = "aes-key"; //replace with key name in UKC to encrypt with
    const string partition = "test"; //replace with partition name where the key is located
    const string message = "sample message to encrypt";
    const string hostname = "localhost:8443"; //replace with server hostname and port

    /******************************************************
     * Validate UKC server certificate (default: false)
     * Set to true to validate server certificate.
     * Important: If set to true and use Windows, you first need to install UKC server CA in Trusted Root CA directory of the Current User Certificates
     */
    bool check_server_cert = false;
	//If check_server_cert is set to true, you need to obtain UKC server CA in pem format and set its path here
    const string server_ca_pem = "path/to/server/CA/PEM/file";

    set_server_cert_validation(check_server_cert, server_ca_pem);

    //create web client with client certificate, which overloads standard WebClient class
    WebClientWithCert client = new WebClientWithCert(client_cert_file, client_cert_pwd);

    //set headers and credentials
    set_request_headers(ref client);

    string uri_encrypt = $"https://{hostname}/api/v1/keys/{key_name}/encrypt?partitionId={partition}";
    string body_encrypt = $"{{\"clearText\": \"{message}\",\"dataEncoding\": \"PLAIN\"}}";

    try
    {
      string response = client.UploadString(uri_encrypt, null, body_encrypt);
      Console.WriteLine(response);
    }
    catch (Exception e)
    {
      Console.WriteLine(e.Message);
    }
  }
}