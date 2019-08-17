/*
  Web Server

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13, so can not use them as IO
 * Analog inputs attached to pins A0 through A5 (optional)
 */


#include <SPI.h>
#include <Ethernet.h>

#define RELAY_PIN 2

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 2, 17);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

String readString;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void processRequest(EthernetClient client) {
  boolean refresh = false;
  if (readString.indexOf("submit=On") > 0) {
    digitalWrite(RELAY_PIN, HIGH);
    refresh = true;
  }
  if (readString.indexOf("submit=Off") > 0) {
    digitalWrite(RELAY_PIN, LOW);
    refresh = true;
  }
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  // client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  if (refresh) {
    client.println("<meta http-equiv='refresh' content=\"0; URL='/'\" />");
  }
  client.println("<style>");
  client.println(".red { color: red } .green { color: green };");
  client.println("</style>");
  client.println("</head>");
  client.println("<h1><a href='/'>Relay Home</a></h1>");
  if (refresh) {
    client.println(readString);
  }
  if (digitalRead(RELAY_PIN) == HIGH) {
    client.println("Pin is ON");
  } else {
    client.println("Pin is OFF");
  }
  client.println("<form ACTION=\"/\" method=get >");
  client.println("<input type=submit name=\"submit\" value=\"On\" class='green'>");
  client.println("<input type=submit name=\"submit\" value=\"Off \" class='red'>");
  client.println("</form>");
  client.println("<br />");
  client.println("</html>");
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //read char by char HTTP request
        if (readString.length() < 100) {

          //store characters to string
          readString += c;
          Serial.print(c); //print what server receives to serial monitor
        } else {
          Serial.write(c);
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          processRequest(client);
          readString = "";
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

