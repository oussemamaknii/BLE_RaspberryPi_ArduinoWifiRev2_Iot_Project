# BLE_RaspberryPi_ArduinoWifiRev2_Iot_Project
Secured ommunication RaspberryPi &lt;=> Arduino (Password, Encryption) over BLE with GPS, Fall Detection, Step Counter, Battery management to a MongoDB database and https flask server for data visualisation 

![arch](https://user-images.githubusercontent.com/58178524/171598740-99f4764e-19ca-4aa7-807e-6966ca2e7a80.png)

                    Simulation of the charging circuit
![image](https://user-images.githubusercontent.com/58178524/171598918-1652c005-6bd3-4b80-ab8b-52bf3bcf385b.png)

HTTPS Server for Integration
Both fixed and mobile platforms communicate with backend using JSON format, but due to the difference in database used, the data format is the main reason that made us implement this kind of solution, in other words the Objectld and ISODate data type in MongoDB have no corresponding data type parsing in Javascript, which makes it difficult to integrate data visualization. So basically, we processed the received data in the front end, rather than modifying it in the backend code on the server. So after sending the data with our FLASK Https based server that is connected to the mobile MongoDB database and secured using our certificate (public and private keys), We managed to we treat the entire data as a string,


Bluetooth low Energy Communication
BLE is a popular and one of the most appropriate solutions for IoT because of its energy efficiency. BLE is more energy efficient than ZigBee, Bluetooth classic, and Wi-Fi, for example. This means that BLE can allow IoT device communication for longer lengths of time than the alternatives (especially when the devices are battery-powered: our case). BLE's low data rate also makes it ideal for use in situations where just status data has to be sent, such as with sensors.
So, for our use case, to achieve such solution we created as a GATT profile BLE service that contains two characteristics and that will specifies in which profile data is exchanged, so the first characteristic was for reading, writing and notifying the central if some values changed, writing new values or reading informations to execute some tasks as alerting the current user if his patient felt by a led blinking and displaying a message on the LCD. However, the other characteristic was made specifically for securing the communication between our central and 
peripheral and that been implemented using uuid5 which will unlock encryption and authentication features.

Web Application
The data visualization part is merged for both platforms using VueJs as Frontend and SpringBoot as backend. Basically we used axios library to access the data of mobile sensors which will give us the access to our flask https server and eliminate the CORE policy problems, and for the visualization part we  added two components at the right part of the webpage, which shows the latest steps count, whether the patient felt or not and at what time exactly and where by the gps coordinates and finally the number of calories burnt for his daily needs. 

![image](https://user-images.githubusercontent.com/58178524/171599101-0044f0c9-3b8d-48c3-bbe6-a85d49745ad5.png)
