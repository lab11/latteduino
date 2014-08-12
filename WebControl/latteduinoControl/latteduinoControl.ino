#include <SPI.h>
#include <Ethernet.h>

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(141, 212, 11, 150); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80

String HTTP_req;          // stores the HTTP request
boolean onStatus = 0;   // state of Machine, off by default

const int onPin = 2;
const int eightOzPin = 3;
const int tenOzPin = 5; //Skipped Pin 4 due to SD Card Issues

void setup()
{
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for diagnostics
    pinMode(onPin, OUTPUT);       // On button on pin 2
}

void loop()
{
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
                    client.println("<h1>COFFEE</h1>");
                    client.println("<p>Click to switch the machine on and off.</p>");
                    client.println("<form method=\"get\">");
                    ProcessPowerCheckbox(client);
                    client.println("</form>");
                    
                    /*client.println("<p>Click to allow an 8 OZ cup to be brewed.</p>");
                    client.println("<form method=\"get\">");
                    Process8OzBrewCheckbox(client);
                    client.println("</form>");*/
                    
                    
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
        // the checkbox was clicked, toggle the LED
        if (onStatus) {
            onStatus = 0;
        }
        else {
            onStatus = 1;
        }
        //onStatus = !onStatus;
    }
    
    if (onStatus) {    // switch LED on
      if(changed)
        {
          digitalWrite(onPin, HIGH);
          delay(100);
          digitalWrite(onPin, LOW);
        }
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"Power\" value=\"2\" \
        onclick=\"submit();\" checked>Power");
    }
    if(!onStatus) {              // switch LED off
        if(changed)
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

/*void Process8OzBrewCheckbox(EthernetClient cl)
{
}*/
