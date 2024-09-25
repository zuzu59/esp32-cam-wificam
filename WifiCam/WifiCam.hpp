//
// Camera server
//
//zf240910.1150
//
// Sources:
// https://github.com/yoursunny/esp32cam


#ifndef WIFICAM_HPP
#define WIFICAM_HPP

#include <esp32cam.h>
#include <WebServer.h>

extern esp32cam::Resolution initialResolution;
extern WebServer server;

void addRequestHandlers();

#endif // WIFICAM_HPP
