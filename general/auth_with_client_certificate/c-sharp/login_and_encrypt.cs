using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;

/****************
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
  static void set_server_cert_validation(bool no_cert)
  { 
    if (no_cert)
      ServicePointManager.ServerCertificateValidationCallback = (sender, cert, chain, errors) => true;
    else //Validate server certificate
    {
      X509Certificate2 server_cert = new X509Certificate2("path/to/server/CA/file");
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
    const string cert_file = "/path/to/client/certificate/file";
    const string cert_pwd = "Password1!";

    //Set encryption parameters
    const string key_name = "aes-for-enc"; //replace with key name in UKC to encrypt with
    const string partition = "test"; //replace with partition name where the key is located
    const string message = "sample message to encrypt";
    const string hostname = "localhost:8443"; //replace with server hostname and port

    //set to false to validate server certificate
    bool no_server_cert = true;
    
    set_server_cert_validation(no_server_cert);

    //create web client with certificate, which overloads standard WebClient class)
    WebClientWithCert client = new WebClientWithCert(cert_file, cert_pwd);

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