# IEC61850 MMS server on Embedded system
Author: **[Dengxue Yan](https://sites.google.com/site/ydengxue/)**
****

This is a framework of IEC61850 MMS server on a device. It has been tested on Linux on OMAP-L138. The code has not been finished. It supports read and write. But the interface to the device signal is still open and is emulated by an array in the memory. The code is single-thread process at present. But it is easy to convert it to multi-thread process to support multiple clients synchronously.
 
The code is compiled by arm-none-linux-gnueabi-gcc on Ubuntu. If you are interested in IEC61850 GOOSE or SMV code on bare-metal system, send me email at Dengxue.Yan@wustl.edu
