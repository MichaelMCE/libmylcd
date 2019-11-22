
// libusbd480 - http://mylcd.sourceforge.net/

// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.

/*
#include <windows.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
*/
//#include <setupapi.h>
//#include <ddk/hidsdi.h>




#include "mylcd.h"

#if (__BUILD_USBD480__ && __WIN32__)

//#define __DEBUG__ 1

#include "libusbd480.h"
#include "hid.h"


const int VendorID = USBD480_VID;
const int ProductID = USBD480_PID;

// defined in libusbd480.c
int process_inputReport (TUSBD480 *di, TUSBD480TOUCHCOORD16 *in_upos, TTOUCHCOORD *out_pos);


void getDeviceCapabilities (TUSBD480HID *hid)
{
	PHIDP_PREPARSED_DATA PreparsedData;

	HidD_GetPreparsedData(hid->deviceHandle, &PreparsedData);
	HidP_GetCaps(PreparsedData, &hid->capabilities);

#ifdef __DEBUG__
	printf("%s%X\n", "Usage Page: ", hid->capabilities.UsagePage);
	printf("%s%d\n", "Input Report Byte length: ", hid->capabilities.InputReportByteLength);
	printf("%s%d\n", "Output Report Byte length: ", hid->capabilities.OutputReportByteLength);
	printf("%s%d\n", "Feature Report Byte length: ", hid->capabilities.FeatureReportByteLength);
	printf("%s%d\n", "Number of Link Collection Nodes: ", hid->capabilities.NumberLinkCollectionNodes);
	printf("%s%d\n", "Number of Input Button Caps: ", hid->capabilities.NumberInputButtonCaps);
	printf("%s%d\n", "Number of InputValue Caps: ", hid->capabilities.NumberInputValueCaps);
	printf("%s%d\n", "Number of InputData Indices: ", hid->capabilities.NumberInputDataIndices);
	printf("%s%d\n", "Number of Output Button Caps: ", hid->capabilities.NumberOutputButtonCaps);
	printf("%s%d\n", "Number of Output Value Caps: ", hid->capabilities.NumberOutputValueCaps);
	printf("%s%d\n", "Number of Output Data Indices: ", hid->capabilities.NumberOutputDataIndices);
	printf("%s%d\n", "Number of Feature Button Caps: ", hid->capabilities.NumberFeatureButtonCaps);
	printf("%s%d\n", "Number of Feature Value Caps: ", hid->capabilities.NumberFeatureValueCaps);
	printf("%s%d\n", "Number of Feature Data Indices: ", hid->capabilities.NumberFeatureDataIndices);
#endif
	HidD_FreePreparsedData(PreparsedData);
}

int hid_FindUSBD480 (TUSBD480HID *hid, char *displaySerial, int useSerial)
{
	HIDD_ATTRIBUTES attributes;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = NULL;
	GUID hidGuid;
	HANDLE hDevInfo;
	ULONG required;
	ULONG length = 0;
	LONG result;
	int lastDevice = 0;
	int memberIndex = 0;
	char serialNumber[64];
	char serial[64];
	int deviceFound = 0; 

	devInfoData.cbSize = sizeof(devInfoData);
	hid->deviceHandle = NULL;

	HidD_GetHidGuid(&hidGuid);	
	hDevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
	
	do{
		result = SetupDiEnumDeviceInterfaces(hDevInfo, 0, &hidGuid, memberIndex, &devInfoData);
		if (result != 0){
			result = SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, NULL, 0, &length, NULL);
			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)calloc(1, length);
			if (detailData == NULL){
				printf("libusbd480: calloc() return NULL ptr\n");
				return 0;
			}
			detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			result = SetupDiGetDeviceInterfaceDetail (hDevInfo, &devInfoData, detailData, length, &required, NULL);
			hid->deviceHandle = CreateFile(detailData->DevicePath, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,OPEN_EXISTING, 0, NULL);
			if (hid->deviceHandle == NULL){
				printf("libusbd480: CreateFile(): returned a NULL handle\n");
				break;
			}
			attributes.Size = sizeof(attributes);
			result = HidD_GetAttributes(hid->deviceHandle, &attributes);
			deviceFound = 0;

			if (attributes.VendorID == VendorID && attributes.ProductID == ProductID){
				int success = 0;
				if (useSerial){
					HidD_GetSerialNumberString(hid->deviceHandle, serialNumber, 64);
					int j = 0;
					for (int i = 0; i < 20; i += 2)
						serial[j++] = serialNumber[i];
					serial[j] = '\0';
					
#ifdef __DEBUG__
					printf("libusbd480: found USBD480 with serial '%s', checking against expected serial '%s'\n", serial, displaySerial);
#endif				
					success = (strncmp(serial, displaySerial, 10) == 0);
				}else{
					success = 1;
				}

				if (success){
#ifdef __DEBUG__
					if (useSerial)
						printf("serials match\n");
#endif
					deviceFound = 1;
					getDeviceCapabilities(hid);
					strncpy(hid->devicePathname, detailData->DevicePath, sizeof(hid->devicePathname)-1);
				}else{
#ifdef __DEBUG__
					printf("serials don't match\n");
#endif
					deviceFound = 0;
					CloseHandle(hid->deviceHandle);
				}
			}else{
				CloseHandle(hid->deviceHandle);
			}
			free(detailData);
		}else{
			lastDevice = 1;
		}
		memberIndex = memberIndex + 1;
	}while(lastDevice == 0 && deviceFound == 0);

#ifdef __DEBUG__
	if (deviceFound == 1)
		printf("libusbd480: using HID device: '%s'\n", hid->devicePathname);
#endif

	SetupDiDestroyDeviceInfoList(hDevInfo);
	return deviceFound;
}

int hid_OpenUSBD480 (TUSBD480HID *hid)
{
	hid->key = 33;	// a random id

#ifdef __DEBUG__
	ULONG InputReportBufferSize = 0;
	HidD_GetNumInputBuffers(hid->deviceHandle, &InputReportBufferSize);
	printf ("InputReportBufferSize: %d\n", (int)InputReportBufferSize);
#endif

	hid->hRead = CreateFile(hid->devicePathname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hid->hRead == INVALID_HANDLE_VALUE){
		printf("Invalid read handle\n");
		return 0;
	}
	hid->hIOPort = CreateIoCompletionPort(hid->hRead, hid->hIOPort, (ULONG_PTR)&hid->key, 1);
	if (hid->hIOPort == INVALID_HANDLE_VALUE){
		printf("Invalid hIOPort handle\n");
		return 0;
	}
	
	hid->state = 1;
	return 1;
}

void hid_SignalCloseReady (TUSBD480HID *hid)
{
	hid->state = 0;
	
	if (hid->hIOPort){
		PostQueuedCompletionStatus(hid->hIOPort, 0, (ULONG_PTR)0, 0);
		CloseHandle(hid->hIOPort);
		hid->hIOPort = NULL;
	}
}

void hid_CloseUSBD480 (TUSBD480HID *hid)
{

	if (hid->hRead){
		CloseHandle(hid->hRead);
		hid->hRead = NULL;
	}
	if (hid->deviceHandle){
		CloseHandle(hid->deviceHandle);
		hid->deviceHandle = NULL;
	}
}

void hid_ClearQueue (TUSBD480HID *hid)
{
	hid->tReports = 0;
}

int hid_get_report (TUSBD480HID *hid, void *report, size_t reportSize)
{
	OVERLAPPED *poverlapped = NULL;
	DWORD len = 0;
	DWORD *pkey = NULL;
	DWORD bytesRead = 0;
	
	
	if (hid->state && hid->tReports > 0){
		memcpy(report, &hid->report[hid->idx++].pos, sizeof(TUSBD480TOUCHCOORD16));
		return hid->tReports--;
	}
	
	hid->tReports = 0;
	DWORD result = 0;
	
	if (hid->state)
		result = ReadFile(hid->hRead, &hid->report, hid->capabilities.InputReportByteLength*32, &bytesRead, &hid->overlapped);
		
	//if (!result) return 0;
	if (hid->state)
	if (GetQueuedCompletionStatus(hid->hIOPort, &len, (void*)&pkey, &poverlapped, INFINITE)){
		if (result == 1 && len != bytesRead)
			return 0;

		if (&hid->overlapped == poverlapped && (int*)pkey == &hid->key){
			hid->tReports = len / sizeof(TUSBD480HIDREPORT);
			if (len > reportSize && hid->tReports && pkey != NULL){
				memcpy(report, &hid->report[0].pos, sizeof(TUSBD480TOUCHCOORD16));
				hid->idx = 1;
				return hid->tReports--;
			}
		}else{
			return -1;
		}
	}
	return 0;
}

int hid_GetTouchPosition (TUSBD480 *di, TUSBD480HID *hid, TTOUCHCOORD *pos)
{	
	if (di && hid && pos){
		int result = hid_get_report(hid, &di->upos, 16);
		if (!result){
			printf("libusbd480: hid_GetTouchPosition() error\n");
			return 0; 
		}else{
			return process_inputReport(di, &di->upos, pos);
		}
	}
	return 0;
}


#endif


