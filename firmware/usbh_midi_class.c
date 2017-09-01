/**
 ******************************************************************************
 * @file    usbh_midi_class.c
 * @author  Johannes Taelman
 * @version
 * @date
 * @brief   Very basic driver for USB Host MIDI class.
 *
 * @verbatim
 *
 * @endverbatim
 *
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */


#include "hal.h"

#if !HAL_USE_USBH
#error "USBHMIDI needs USBH"
#endif

#warning "Needs ChibiOS_16.1.8/community from https://github.com/ChibiOS/ChibiOS-Contrib,"
#warning "different than the one included in Chibios_16.1.8, "
#warning "otherwise compilation will fail on this file!"

#include <string.h>
#include "usbh_midi_class.h"
#include "usbh/internal.h"

#if USBH_MIDI_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBH_MIDI_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBH_MIDI_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBH_MIDI_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif


/*===========================================================================*/
/* USB Class driver loader for MIDI Class  						 		 	 */
/*===========================================================================*/

#define USB_AUDIO_CLASS 0x01
#define USB_MIDISTREAMING_SubCLASS 0x03

/*
* Definitions from the USB_MIDI_ or usb_midi_ namespace come from:
* "Universal Serial Bus Class Definitions for MIDI Devices, Revision 1.0"
*/

/* Appendix A.1: MS Class-Specific Interface Descriptor Subtypes */
#define USB_MIDI_SUBTYPE_MS_DESCRIPTOR_UNDEFINED 0x00
#define USB_MIDI_SUBTYPE_MS_HEADER              0x01
#define USB_MIDI_SUBTYPE_MIDI_IN_JACK           0x02
#define USB_MIDI_SUBTYPE_MIDI_OUT_JACK          0x03
#define USB_MIDI_SUBTYPE_MIDI_ELEMENT           0x04

/* Appendix A.2: MS Class-Specific Endpoint Descriptor Subtypes */
#define USB_MIDI_SUBTYPE_DESCRIPTOR_UNDEFINED   0x00
#define USB_MIDI_SUBTYPE_MS_GENERAL             0x01

/* Appendix A.3: MS MIDI IN and OUT Jack types */
#define USB_MIDI_JACK_TYPE_UNDEFINED            0x00
#define USB_MIDI_JACK_TYPE_EMBEDDED             0x01
#define USB_MIDI_JACK_TYPE_EXTERNAL             0x02

/* Appendix A.5.1 Endpoint Control Selectors */
#define USB_MIDI_EP_CONTROL_UNDEFINED           0x00
#define USB_MIDI_ASSOCIATION_CONTROL            0x01

USBHMIDIDriver USBHMIDID[USBH_MIDI_CLASS_MAX_INSTANCES];


typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint16_t bcdMSC;
	uint16_t wTotalLength;
} __attribute__((packed)) ms_interface_header_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bNumEmbMIDIJack;
	uint8_t baAssocJackID[0];
} __attribute__((packed)) ms_bulk_data_endpoint_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bJackType;
	uint8_t bJackID;
	uint8_t iJack;
} midi_in_jack_descriptor_t;

typedef struct {
	uint8_t baSourceID;
	uint8_t BaSourcePin;
} midi_jack_descriptor_pin_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bJackType;
	uint8_t bJackID;
	uint8_t bNrInputPins;
	midi_jack_descriptor_pin_t output_pins[0];
} midi_out_jack_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bElementID;
	uint8_t bNrInputPins;
	midi_jack_descriptor_pin_t input_pins[0];
} midi_element_descriptor_t;

typedef struct {
	uint8_t bNrOutputPins;
	uint8_t bInTerminalLink;
	uint8_t bOutTerminalLink;
	uint8_t bElCapsSize;
	uint8_t bmElementCaps[0];
} midi_element_descriptor_part2_t;

static void _init(void);
static usbh_baseclassdriver_t *_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _unload(usbh_baseclassdriver_t *drv);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	_init,
	_load,
	_unload
};

const usbh_classdriverinfo_t usbhMidiClassDriverInfo = {
	"MIDI", &class_driver_vmt
};

static usbh_baseclassdriver_t *_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {
	int i;
	USBHMIDIDriver *midip;
	(void)dev;

	if (_usbh_match_descriptor(descriptor, rem, USBH_DT_INTERFACE,
			USB_AUDIO_CLASS, USB_MIDISTREAMING_SubCLASS, -1) != HAL_SUCCESS)
		return NULL;

	const usbh_interface_descriptor_t * const ifdesc = (const usbh_interface_descriptor_t *)descriptor;

	if ((ifdesc->bAlternateSetting != 0)
			|| (ifdesc->bNumEndpoints < 1)) {
		return NULL;
	}

	/* alloc driver */
	for (i = 0; i < USBH_MIDI_CLASS_MAX_INSTANCES; i++) {
		if (USBHMIDID[i].dev == NULL) {
			midip = &USBHMIDID[i];
			goto alloc_ok;
		}
	}

	uwarn("Can't alloc MIDI driver");

	/* can't alloc */
	return NULL;

alloc_ok:
	/* initialize the driver's variables */
	midip->ifnum = ifdesc->bInterfaceNumber;
	midip->nOutputPorts = 0;
	midip->nInputPorts = 0;

	/* parse the configuration descriptor */
	if_iterator_t iif;
	generic_iterator_t iep;
	iif.iad = 0;
	iif.curr = descriptor;
	iif.rem = rem;
	generic_iterator_t ics;

	for (cs_iter_init(&ics, (generic_iterator_t *)&iif); ics.valid; cs_iter_next(&ics)) {
		switch(ics.curr[2]) {
		case USB_MIDI_SUBTYPE_MS_HEADER: {
			ms_interface_header_descriptor_t *intf_hdr = (ms_interface_header_descriptor_t *)&ics.curr[0];
			uinfof("    Midi interface header, version = %4X",
					intf_hdr->bcdMSC);
		} break;
		case USB_MIDI_SUBTYPE_MIDI_IN_JACK: {
			midi_in_jack_descriptor_t *in_jack_desc = (midi_in_jack_descriptor_t *)&ics.curr[0];
			uinfof("    Midi In jack, bJackType = %d, bJackID = %d, iJack=%d",
					in_jack_desc->bJackType,in_jack_desc->bJackID,in_jack_desc->iJack);
//			char name[32];
//			bool res = usbhStdReqGetStringDescriptor(dev, in_jack_desc->iJack, dev->langID0, sizeof(name), (uint8_t *)name);
//			if (res) {
//				uinfof("    name %s", name);
//			} else {
//				uinfof("    noname");
//			}
		} break;
		case USB_MIDI_SUBTYPE_MIDI_OUT_JACK: {
			midi_out_jack_descriptor_t *out_jack_desc = (midi_out_jack_descriptor_t *)ics.curr;
			uinfof("    Midi Out jack, bJackType = %d, bJackID = %d, bNrInputPins=%d",
					out_jack_desc->bJackType,out_jack_desc->bJackID,out_jack_desc->bNrInputPins);
		} break;
		default:
			uinfof("    Midi Class-Specific descriptor, Length=%d, Type=%02x",
					ics.curr[0], ics.curr[1]);
			int j;
			for(j=2;j<ics.curr[0];j++)
				uinfof("  %02X", ics.curr[j]);
			}

	}

	iif.iad = 0;
	iif.curr = descriptor;
	iif.rem = rem;
	for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
		usbh_endpoint_descriptor_t * epdesc = (usbh_endpoint_descriptor_t *)ep_get(&iep);
		if ((epdesc->bEndpointAddress & 0x80) &&
				((epdesc->bmAttributes == USBH_EPTYPE_BULK) ||
						(epdesc->bmAttributes == USBH_EPTYPE_INT))
						) {
			// some devices use BULK (UC33), some devices use INT endpoints (Launchpad Mini)
			uinfof("IN endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);

			for (cs_iter_init(&ics, &iep); ics.valid; cs_iter_next(&ics)) {
				ms_bulk_data_endpoint_descriptor_t *ms_ep_desc = (ms_bulk_data_endpoint_descriptor_t *)ics.curr;
				if (ms_ep_desc->bDescriptorSubType == USB_MIDI_SUBTYPE_MS_GENERAL) {
					uinfof("    Midi IN endpoint descriptor, bNumEmbMIDIJack=%d",
							ms_ep_desc->bNumEmbMIDIJack);
					int j;
					for(j=0;j<ms_ep_desc->bNumEmbMIDIJack;j++)
						uinfof("    baAssocJackID =  %02X", ms_ep_desc->baAssocJackID[j]);
					midip->nInputPorts = ms_ep_desc->bNumEmbMIDIJack;
				} else {
					uinfof("    Midi IN endpoint descriptor???");
				}
			}

			// Pretend it is an INT IN endpoint to avoid a NAK flood
			epdesc->bmAttributes |= USBH_EPTYPE_INT;
			usbhEPObjectInit(&midip->epin, dev, epdesc);
			midip->epin.type = USBH_EPTYPE_INT;
			usbhEPSetName(&midip->epin, "MIDI[IIN ]");
		} else if (((epdesc->bEndpointAddress & 0x80) == 0) &&
			((epdesc->bmAttributes == USBH_EPTYPE_BULK) ||
					(epdesc->bmAttributes == USBH_EPTYPE_INT))
					) {
			// again, some devices use BULK, some devices use INT endpoints
			uinfof("OUT endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);

			for (cs_iter_init(&ics, &iep); ics.valid; cs_iter_next(&ics)) {
				ms_bulk_data_endpoint_descriptor_t *ms_ep_desc = (ms_bulk_data_endpoint_descriptor_t *)ics.curr;
				if (ms_ep_desc->bDescriptorSubType == USB_MIDI_SUBTYPE_MS_GENERAL) {
					uinfof("    Midi OUT endpoint descriptor, bNumEmbMIDIJack=%d",
							ms_ep_desc->bNumEmbMIDIJack);
					int j;
					for(j=0;j<ms_ep_desc->bNumEmbMIDIJack;j++)
						uinfof("    baAssocJackID =  %02X", ms_ep_desc->baAssocJackID[j]);
					midip->nOutputPorts = ms_ep_desc->bNumEmbMIDIJack;
				} else {
					uinfof("    Midi OUT endpoint descriptor???");
				}
			}

			usbhEPObjectInit(&midip->epout, dev, epdesc);
			usbhEPSetName(&midip->epout, "MIDI[IOUT]");
		} else {
			uinfof("unsupported endpoint found: bEndpointAddress=%02x, bmAttributes=%02x",
					epdesc->bEndpointAddress, epdesc->bmAttributes);
		}
	}

	midip->state = USBHMIDI_STATE_ACTIVE;

	return (usbh_baseclassdriver_t *)midip;

}


static void _stop_locked(USBHMIDIDriver *midip) {
	if (midip->state == USBHMIDI_STATE_ACTIVE)
		return;

	osalDbgCheck(midip->state == USBHMIDI_STATE_READY);

	usbhEPClose(&midip->epin);
	usbhEPClose(&midip->epout);

	midip->nOutputPorts = 0;
	midip->nInputPorts = 0;
	midip->name[0] = 0;

	midip->state = USBHMIDI_STATE_ACTIVE;

	if (midip->config->cb_disconnect)
		midip->config->cb_disconnect(midip->config);
}

static void _unload(usbh_baseclassdriver_t *drv) {
	USBHMIDIDriver *const midip = (USBHMIDIDriver *)drv;
	_stop_locked(midip);
	midip->state = USBHMIDI_STATE_STOP;
}

static void _object_init(USBHMIDIDriver *midip) {
	osalDbgCheck(midip != NULL);
//	memset(midip, 0, sizeof(*midip));
	midip->info = &usbhMidiClassDriverInfo;
	midip->state = USBHMIDI_STATE_STOP;
}

static void _init(void) {
	uint8_t i;
	for (i = 0; i < USBH_MIDI_CLASS_MAX_INSTANCES; i++) {
		_object_init(&USBHMIDID[i]);
	}
}

static void _in_cb(usbh_urb_t *urb) {
	USBHMIDIDriver *const midip = (USBHMIDIDriver *)urb->userData;
	switch (urb->status) {
	case USBH_URBSTATUS_OK:
		if (midip->config->cb_report) {
			midip->config->cb_report(midip->config, (uint32_t *)midip->report_buffer, urb->actualLength/4);
		}
		break;
	case USBH_URBSTATUS_TIMEOUT:
		//no data
		break;
	case USBH_URBSTATUS_DISCONNECTED:
		uwarn("MIDI: URB IN disconnected");

		return;
	case USBH_URBSTATUS_ERROR:
		uwarn("MIDI: URB IN error");
		return;
	default:
		uerrf("MIDI: URB IN status unexpected = %d", urb->status);
		break;
	}
	usbhURBObjectResetI(&midip->in_urb);
	usbhURBSubmitI(&midip->in_urb);
}

void usbhmidiStart(USBHMIDIDriver *midip) {
	osalDbgCheck(midip);
	osalDbgCheck((midip->state == USBHMIDI_STATE_ACTIVE)
			|| (midip->state == USBHMIDI_STATE_READY));

	if (midip->state == USBHMIDI_STATE_READY)
		return;

	usbhDeviceReadString(midip->dev, &midip->name[0], sizeof(midip->name), midip->dev->devDesc.iProduct, midip->dev->langID0);

	/* init the URBs */
	usbhURBObjectInit(&midip->in_urb, &midip->epin, _in_cb, midip,
			midip->report_buffer, USBH_MIDI_BUFSIZE);

	/* open the bulk IN/OUT endpoints */
	usbhEPOpen(&midip->epin);
	usbhEPOpen(&midip->epout);

	osalSysLock();
	usbhURBSubmitI(&midip->in_urb);
	osalSysUnlock();

	midip->state = USBHMIDI_STATE_READY;
}

void usbhmidiStop(USBHMIDIDriver *midip) {
	osalDbgCheck((midip->state == USBHMIDI_STATE_ACTIVE)
			|| (midip->state == USBHMIDI_STATE_READY));

	if (midip->state != USBHMIDI_STATE_READY)
		return;

	osalSysLock();
	usbhEPCloseS(&midip->epin);
	usbhEPCloseS(&midip->epout);
	midip->state = USBHMIDI_STATE_ACTIVE;
	osalSysUnlock();

	midip->nOutputPorts = 0;
	midip->nInputPorts = 0;

	midip->name[0] = 0;
}

msg_t usbhmidi_sendbuffer(USBHMIDIDriver *midip, uint8_t *buffer, int size) {
	if (midip->state != USBHMIDI_STATE_READY)
		return;
	int actual_len;
	msg_t status = usbhBulkTransfer(&midip->epout, buffer,
			size, &actual_len, MS2ST(1000));
	if (status == USBH_URBSTATUS_OK) return MSG_OK;
	return status;
}