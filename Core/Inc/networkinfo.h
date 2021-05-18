/**
 * @brief networkinfo.h contains all the info of the tcp connection
 *        and the user configurations for the WiFi connection.
 * @details
 * @file networkinfo.h
 * @author  Joar Edling, joaredl@kth.se
 * @date 06-05-2021
**/

#ifndef INC_NETWORKINFO_H_
#define INC_NETWORKINFO_H_

/* Used for smartbin website connection, dont change this! */
static const char connection_type[] = "TCP"; //The connection type, example: "TCP", "UDP"...
static const char remote_ip[] = "smartbin.azurewebsites.net"; //Connects to a remote website or ip
static const char remote_port[] = "80"; //The port, example http has "80"

/* Used for WiFi connection, user change this to match your own network info! */
static const char SSID[] = ""; //Your SSID, example: "MyWiFi"
static const char PWD[] = ""; //Your Password, example: "Password123"
static const char deviceID[] = ""; //The device ID, exemple: "101"


#endif /* INC_NETWORKINFO_H_ */
