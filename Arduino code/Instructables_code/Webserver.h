/* ************************************************************************************************
** Webserver for garage door opener.
** ************************************************************************************************ */

#pragma once
#include <Arduino.h>

void InitializeGarageDoorWebserver();
void printIPAddress(); // added by Brian
void ProcessGarageDoorWebserver();
