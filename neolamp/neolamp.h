/*
   Copyright (C) 2020 Michael Matthews

   This file is part of NeoLamp.

   NeoLamp is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   NeoLamp is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with NeoLamp.  If not, see <http://www.gnu.org/licenses/>.
*/

// Use web browser to view and copy SHA1 fingerprint of the certificate.
#define FINGERPRINT "77 00 54 2D DA E7 D8 03 27 31 23 99 EB 27 DB CB A5 4C 57 18"

// List of wifi access points and passwords.  Finish with NULL, NULL at the end of the variable length list.
#define WIFI_USERPASS "ssid-1", "password-1", "ssid-2", "password-2", "ssid-3", "password-3", NULL, NULL

// Account details for the feed at https://io.adafruit.com/
#define ADAFRUIT_KEY "x-aio-key"
#define ADAFRUIT_ACCOUNT "account-name"
#define ADAFRUIT_FEED "feed-name"
