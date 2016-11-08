# luisricardo
IBM Watson IoT on BLUEMIX full prototype.
Included in this project:

- The sketchup model (SKP file) for building a 3d-printed robot.
- The Arduino (luisricardo.ino) and ESP8266 wifi module (luisri_wifi.ino) source code.
- The source code for a mobile app (.AIA) you can upload to MIT APP INVENTOR 2 for updating or customizing. 
  This app is for controlling the robot.
- A web page for controlling the robot. This web page should be deployed on a web server. 
  ** BLUEMIX allows deployment of this application using the Static Web Server buildpack.**
- A nodeRED flow for brokering messages between the control page or control app and the robot. 
  ** This nodeRED flow must be deployed on a BLUEMIX environment for exploiting the following services: Watson IoT, Cloudant, Twilio, and Twitter integration. **
  
The best is to start this project using BLUEMIX:
- Creating a new application using the nodeRED buildpack and import the supplied nodes.
- Creating a new application using the Statics Web Server builpack and deploying the supplied website application.
Then using MIT AppInventor2:
- Creating a new project for the mobile application and importing the AIA file.

And then modify all the code to fit your needs.
  
