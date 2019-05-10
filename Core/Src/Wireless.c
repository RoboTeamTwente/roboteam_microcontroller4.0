/*
 * Wireless.c
 *
 *  Created on: 6 feb. 2019
 *      Author: Cas Doornkamp
 */

#include "Wireless.h"
#include "SX1280/SX1280_Constants.h"
#include "gpio_util.h"
#include "SX1280/SX1280.h"
#include <stdbool.h>
#include "PuTTY.h"
#include "packing.h"

// make buffers
uint8_t TX_buffer[MAX_BUF_LENGTH] __attribute__((aligned(4)));
uint8_t RX_buffer[MAX_BUF_LENGTH] __attribute__((aligned(4)));

// init structs
SX1280_Settings set = {
        .frequency = 2400000000,
        .txPower = 31, // -18 + txPower = transmit power in dBm (13dBm max)
		.packettype = PACKET_TYPE_FLRC,
        .TX_ramp_time = RADIO_RAMP_20_US,
		.periodBase = BASE_62_us,
        .periodBaseCount = 24,
		.syncWords = {0x0, 0x0, 0x0},
		.syncWordTolerance = 2, // accepted wrong bits in a detected syncword
        .syncSensitivity = 1, // high sensitivity mode
        .crcSeed = {0xAC, 0xB6}, // seed value of 0xACB6 = 0b'1010110010110110
        .crcPoly = {0x10, 0x21}, // poly of P16(x) = x16 + x12 + x5 + 1
        .TXoffset = 0x80,
        .RXoffset = 0x00,
        .ModParam = {FLRC_BR_1_300_BW_1_2, FLRC_CR_3_4, BT_0_5},
        .PacketParam = {PREAMBLE_LENGTH_24_BITS, FLRC_SYNC_WORD_LEN_P32S, RX_MATCH_SYNC_WORD_1, PACKET_FIXED_LENGTH, RECEIVEPKTLEN, CRC_2_BYTE, NO_WHITENING},
        .DIOIRQ = {(TX_DONE|RX_DONE|CRC_ERROR|RXTX_TIMEOUT), (TX_DONE|RX_DONE|CRC_ERROR|RXTX_TIMEOUT), NONE, NONE}
};
SX1280_Packet_Status PacketStat;

SX1280 * Wireless_Init(float channel, SPI_HandleTypeDef * WirelessSpi){
	SX1280 * SX = &SX1280_struct;// pointer to the global struct

    SX->SPI_used = false;

    SX->payloadLength = 0;
	SX->RXbufferoffset = 0; // received location of the data buffer in SX1280
	SX->irqStatus = 0;      // last received IRQ status

    // set connections
    SX->SPI = WirelessSpi;
    SX->CS_pin = SX_NSS_pin;
    set_Pin(SX->CS_pin, HIGH);
    SX->BUSY_pin = SX_BUSY_pin;
    SX->IRQ_pin = SX_IRQ_pin;
    SX->RST_pin = SX_RST_pin;

    // set buffer locations
    SX->RXbuf = RX_buffer;
    SX->TXbuf = TX_buffer;

    // link settings
    SX->SX_settings = &set;
    SX->SX_settings->channel = channel;
    SX->Packet_status = &PacketStat;

    SX1280Setup(SX); // SX1280 init procedure

    setAutoFS(SX,false); // to go or not to go FS after TX or RX

    return SX;
};


void SendAutoPacket(SX1280* SX, uint8_t * data, uint8_t Nbytes){
    writeBuffer(SX, data, Nbytes);
    setAutoTX(SX, AUTO_TX_TIME);
};

void SendPacket(SX1280* SX, uint8_t * data, uint8_t Nbytes){
	clearIRQ(SX,ALL);
    writeBuffer(SX, data, Nbytes);
    setTX(SX, SX->SX_settings->periodBase, SX->SX_settings->periodBaseCount);
}

void ReceivePacket(SX1280* SX){
	clearIRQ(SX,ALL);
    getRXBufferStatus(SX);
    SX->expect_packet = true;
    readBuffer(SX, SX->payloadLength);

};

bool isWirelessConnected = false; // boolean to check whether we have a wireless connection or not
bool checkWirelessConnection() {
	return isWirelessConnected;
}

// -------------------------------------------- Handlers
void Wireless_IRQ_Handler(SX1280* SX, uint8_t * data, uint8_t Nbytes){
    uint16_t irq = getIRQ(SX);
    SX->irqStatus = irq;
    getPacketStatus(SX);
//	char msg[20];
//	sprintf(msg, "rssi: %d\n\r", SX->Packet_status->RSSISync);
//	Putty_printf(msg);
//	sprintf(msg, "errors: %d\n\r", SX->Packet_status->errors);
//	Putty_printf(msg);
    clearIRQ(SX,ALL);

    if(irq & CRC_ERROR) {
    	//isWirelessConnected = false;
    	toggle_Pin(LED6_pin);
    	setRX(SX, SX->SX_settings->periodBase, 8000);
    	return;
    }

    // process interrupts
    if(irq & TX_DONE){
//    	Putty_printf("TX_DONE...\n\r");
//    	toggle_pin(LED3_pin);
//    	if transmitted, switch to receive mode
//    	SX->SX_settings->syncWords[0] = robot_syncWord[get_Id()];
//    	setSyncWords(SX, SX->SX_settings->syncWords[0], 0x00, 0x00);
//    	setRX(SX, SX->SX_settings->periodBase, 0xFFFF);
    }

    if(irq & RX_DONE){
//    	Putty_printf("RX_DONE...\n\r");
    	isWirelessConnected = true;
    	toggle_Pin(LED5_pin);
    	// if signal is strong, then receive packet; otherwise wait for packets
    	if (SX->Packet_status->RSSISync < 160) {
//    		Putty_printf("noise: %d\n\r", SX->Packet_status->RSSISync);
    		ReceivePacket(SX);
    	}else{
//    		 not necessary to force setRX() here since configured in Rx Continuous mode
    		setRX(SX, SX->SX_settings->periodBase, 8000);
    	}
    }

    if(irq & SYNCWORD_VALID) {
    }

    if(irq & SYNCWORD_ERROR) {
    }

//    if(irq & CRC_ERROR) {
//    	isWirelessConnected = false;
//    	toggle_Pin(LED6_pin);
//    	setRX(SX, SX->SX_settings->periodBase, 8000);
//    }

    if(irq & RXTX_TIMEOUT) {
//    	SX->SX_settings->syncWords[0] = robot_syncWord[31];
//		setSyncWords(SX, SX->SX_settings->syncWords[0], SX->SX_settings->syncWords[1], SX->SX_settings->syncWords[2]);
//		SendPacket(SX, buf, 13);
    	isWirelessConnected = false;
    	setRX(SX, SX->SX_settings->periodBase, 8000);
    }

    if(irq & PREAMBLE_DETECTED) {
    }
};

void Wireless_DMA_Handler(SX1280* SX, uint8_t* output, ReceivedData* receivedData){
	DMA_Callback(SX);
	if (SX->expect_packet) {
		SX->expect_packet = false;
		for (int i=0; i<13; i++) {
			PC_to_Bot[i] = SX->RXbuf[3+i];
		}
		packetToRoboData(PC_to_Bot, receivedData);
		setRX(SX, SX->SX_settings->periodBase, 8000);
//    	char packet[30];
//    	Putty_printf("received packet: ");
//    	for (int i=0; i<13; i++) {
//    		sprintf(packet, "%X", SX->RXbuf[3+i]);
//    		Putty_printf(packet);
//    	}
//    	Putty_printf("\n\r");
	} else {
		setRX(SX, SX->SX_settings->periodBase, 8000);
	}
}