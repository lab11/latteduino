#include <SPI.h>
#include <Ethernet.h>

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(141, 212, 11, 150); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80

String HTTP_req;          // stores the HTTP request
boolean onStatus = 0;   // state of Machine, off by default

const int onPin = 2; //Output, Digital
const int eightOzPin = 3; //Output
const int tenOzPin = 5; //Output, Digital, Skipped Pin 4 due to SD Card Occupation
const int lidControlPin = 6; //Output, Digital
const int allowEightOzBrewPin = 7; //Output, Digital
const int allowTenOzBrewPin = 8; //Output, Digital
const int addWaterPin = A0; //Input, Analog
const int heatingPin = A1; //Input, Analog
const int onDetectionPin = A2; //Input, Analog

boolean isOn = false;
boolean eightOzAllowed = false;
boolean tenOzAllowed = false;
boolean enoughWater = false;
boolean isHeating = false;

void setup()
{
  Ethernet.begin(mac, ip); // initialize Ethernet device
  server.begin(); // start to listen for clients
  Serial.begin(9600); // for diagnostics
  pinMode(onPin, OUTPUT); // On button on pin 2
  pinMode(eightOzPin, OUTPUT); //Brews 8 Oz Cup on pin 3
  pinMode(tenOzPin, OUTPUT); //Brews 10 Oz Cup on pin 5
  pinMode(lidControlPin, OUTPUT); //Simulates opening and closing of lid on pin 6
  pinMode(allowEightOzBrewPin, OUTPUT); //Allows a physical user of the machine to brew an 8 Oz Cup on pin 7
  pinMode(allowTenOzBrewPin, OUTPUT); //Allows a physical user of the machine to brew a 10 Oz Cup on pin 8
  pinMode(addWaterPin, INPUT); //Tells Arduino if the machine requires water to be added on pin A1
  pinMode(heatingPin, INPUT); //Tells Arduino if the machine is currently heating water on pin A2
  //digitalWrite(eightOzPin, HIGH);
}

void loop()
{
  if(analogRead(onDetectionPin) > 420) {isOn = true;}
  else {isOn = false;}
  
  EthernetClient client = server.available();  // try to get client

  if (client) {  // got client?
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        HTTP_req += c;  // save the HTTP request 1 char at a time
        // last line of client request is blank and ends with \n
        // respond to client only after last line received
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          // send web page
          client.println("<!DOCTYPE html>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Arduino Coffee Machine Control</title>");
          client.println("</head>");
          client.println("<body>");
          int onStatus = analogRead(A5);
          client.print("<p style = color:red>");
          client.print(analogRead(A5));
          client.println("</p>");
          Serial.println(onStatus);
          client.println("<h1>latteduino</h1>");
          client.println("<p>Click to switch the machine on and off.</p>");
          client.println("<form method=\"get\">");
          ProcessPowerCheckbox(client);
          client.println("</form>");

          client.println("<p>Click to brew an 8 Oz cup of joe.</p>");
          client.println("<form method=\"get\">");
          Process8OzBrewCheckbox(client);
          client.println("</form>");
          
          client.println("</body>");
          client.println("</html>");
          Serial.print(HTTP_req);
          HTTP_req = "";    // finished with request, empty string
          break;
        }
        // every line of text received from the client ends with \r\n
        if (c == '\n') {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // a text character was received from client
          currentLineIsBlank = false;
        }
      } // end if (client.available())
    } // end while (client.connected())
    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
  } // end if (client)
}

// switch LED and send back HTML for LED checkbox
void ProcessPowerCheckbox(EthernetClient cl)
{
  int changed = 0;
  if (HTTP_req.indexOf("Power=2") > -1) {  // see if checkbox was clicked
    changed = 1;
  }
  if (isOn) {    // switch machine on
    if (changed)
    {
      digitalWrite(onPin, HIGH);
      delay(100);
      digitalWrite(onPin, LOW);
    }
    // checkbox is checked
    cl.println("<input type=\"checkbox\" name=\"Power\" value=\"2\" \
        onclick=\"submit();\" checked>Power");
  }
  if (!isOn) {             // switch machine off
    if (changed)
    {
      digitalWrite(onPin, HIGH);
      delay(100);
      digitalWrite(onPin, LOW);
    }
    // checkbox is unchecked
    cl.println("<input type=\"checkbox\" name=\"Power\" value=\"2\" \
        onclick=\"submit();\">Power");
  }
  changed = 0;
  delay(100);
}

void Process8OzBrewCheckbox(EthernetClient cl)
{
  int changed = 0;
  if (HTTP_req.indexOf("Brew=3") > -1) {  // see if checkbox was clicked
    changed = 1;
  }

  if (changed)
  {
    cl.println("<input type=\"checkbox\" name=\"Brew\" value=\"3\" \
        onclick=\"submit();\" checked>Brew");
    digitalWrite(3, HIGH);
    delay(100);
    digitalWrite(3, LOW);
    /*cl.println("<input type=\"checkbox\" name=\"Brew\" value=\"3\" \
        onclick=\"submit();\">Brew");*/
  }
  else
  {
    cl.println("<input type=\"checkbox\" name=\"Brew\" value=\"3\" \
        onclick=\"submit();\">Brew");
  }
  // checkbox is checked
  
  changed = 0;
  delay(100);
}
