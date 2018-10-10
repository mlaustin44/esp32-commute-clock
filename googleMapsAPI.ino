int getTrafficDuration (String origin, String destination, String departureTime, String trafficModel) {    //google API interface function, return travel time, with traffic, in seconds
  
  //build distance matrix command to send to google maps
  String command =
      "https://maps.googleapis.com/maps/api/distancematrix/json?origins=" +
      origin + "&destinations=" +
      destination; 
       
  if (departureTime != "") {
    command = command + "&departure_time=" + departureTime;
  }
  
  if (trafficModel != "") {
    command = command + "&traffic_model=" + trafficModel;
  }
  
  command = command + "&key=" + API_KEY;

  //send command to google maps

  client.connect(server, 443);

  client.println("GET " + command);
  client.println("Host: maps.googleapis.com");
  client.println("Connection: close");
  client.println();

  String body = "";

  while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        //Serial.println("headers received");
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and add to body string:
    while (client.available()) {
      char c = client.read();
      //Serial.write(c);
      body = body + c;
    }

  DynamicJsonBuffer jsonBuffer; //json parser 
  JsonObject& response = jsonBuffer.parseObject(body); //parse body retrieved from server

  //Serial.println("Requesting Traffic!"); //debug
  
  if (response.success()) {
    //Serial.println("Response Sucess!");
      if (response.containsKey("rows")) {
        JsonObject& element = response["rows"][0]["elements"][0];
        String status = element["status"];
        if(status == "OK") {
          //Serial.println("Status = OK!");
          durationInTraffic = element["duration_in_traffic"]["value"];  //the value of duration in traffic return the duration in seconds
          //Serial.println("Duration print inside if:");
          //Serial.println(durationInTraffic);
          } } }
      
  //Serial.println("Duration print before return:");
  //Serial.println(durationInTraffic);
  
  return durationInTraffic;  //return # of seconds in traffic
}
