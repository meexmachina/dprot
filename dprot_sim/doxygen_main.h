#ifndef dprot_sim_doxygen_main_h
#define dprot_sim_doxygen_main_h

/*! \mainpage dProt transport layer over the SLIP protocol
 \section Description
 The dProt protocol was written to meet the demands of very small micro-controller devices.
 I used the USART serial communication with AVR uC (XMEGA32D4) connected with a host computer as
 a test case.
 The protocol didn't have any solid demands but I stated it should be as lightweight as possible
 and as fast as possible sending large amount of information. The communication overhead of the
 tranport layer in average should be less than 15 bytes for 256 sent raw data.
 The data-link layer protocol was chosen to be SLIP. This protocol is very lightweight and robust.
 It doen't check for errors though. Thus the hight layer uses crc8 data checking and logical 
 matching to verify the received packets.
 */

#endif
