#ifdef __LPC17XX__

#include "usbutil.h"
#include "log.h"
#include "buffers.h"
#include <algorithm>
#include <stdio.h>

extern "C" {
#include "bsp.h"
#include "LPC17xx.h"
#include "lpc17xx_gpio.h"
}

static void sendToHost(UsbDevice* usbDevice) {
    uint8_t previousEndpoint = Endpoint_GetCurrentEndpoint();
    Endpoint_SelectEndpoint(DATA_ENDPOINT_NUMBER);
    if(!Endpoint_IsINReady() || queue_empty(&usbDevice->sendQueue)) {
        return;
    }

    // get bytes from transmit FIFO into intermediate buffer
    int byteCount = 0;
    // TODO try removing the 64 byte limit when using stream sending
    while(!queue_empty(&usbDevice->sendQueue)
            && byteCount < USB_SEND_BUFFER_SIZE) {
        usbDevice->sendBuffer[byteCount++] =
            QUEUE_POP(uint8_t, &usbDevice->sendQueue);
    }

    if(byteCount > 0) {
        // TODO may need to pad the end with zeros?
        Endpoint_Write_Stream_LE(usbDevice->sendBuffer, byteCount, NULL);
    }
    Endpoint_ClearIN();
    Endpoint_SelectEndpoint(previousEndpoint);
}

void processInputQueue(UsbDevice* usbDevice) {
    USB_USBTask();
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;

    sendToHost(usbDevice);
}

void configureEndpoints() {
    Endpoint_ConfigureEndpoint(DATA_ENDPOINT_NUMBER, EP_TYPE_BULK,
            ENDPOINT_DIR_OUT, DATA_ENDPOINT_SIZE, ENDPOINT_BANK_DOUBLE);
    Endpoint_ConfigureEndpoint(DATA_ENDPOINT_NUMBER, EP_TYPE_BULK,
            ENDPOINT_DIR_IN, DATA_ENDPOINT_SIZE, ENDPOINT_BANK_DOUBLE);
}

void EVENT_USB_Device_ConfigurationChanged(void) {
    configureEndpoints();
}

void initializeUsb(UsbDevice* usbDevice) {
    debug("Initializing USB.....");

    USB_Init();
    USB_Connect();

    debug("Done.\r\n");
}

void readFromHost(UsbDevice* usbDevice, bool (*callback)(uint8_t*)) {
    uint8_t previousEndpoint = Endpoint_GetCurrentEndpoint();
    Endpoint_SelectEndpoint(DATA_ENDPOINT_NUMBER);

    while(Endpoint_IsOUTReceived()) {
        for(int i = 0; i < MAX_USB_PACKET_SIZE_BYTES; i++) {
            if(!QUEUE_PUSH(uint8_t, &usbDevice->receiveQueue,
                        Endpoint_Read_8())) {
                debug("Dropped write from host -- queue is full\r\n");
            }
        }
        processQueue(&usbDevice->receiveQueue, callback);
    }
    Endpoint_SelectEndpoint(previousEndpoint);
}

#endif // __LPC17XX__
