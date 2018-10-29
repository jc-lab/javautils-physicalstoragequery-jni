/*!
 *  \author    Jichan (development@jc-lab.net)
 *  \version   1.0
 *  \date      2018.10
 *  \copyright Apache License 2.0
 */

#include <JsCPPUtils/Common.h>
#include "LibphystorqueryWindowsImpl.h"

#if JSCUTILS_PLATFORM_ISWINDOWS()

#include <JsCPPUtils/StringBuffer.h>

typedef struct _VOLUME_DISK_EXTENTS_LX {
	DWORD       NumberOfDiskExtents;
	DISK_EXTENT Extents[4];
} VOLUME_DISK_EXTENTS_LX, *PVOLUME_DISK_EXTENTS_LX;

static bool flipAndCodeBytes(const BYTE * str, int flip, char * buf);

LibphystorqueryWindowsImpl::LibphystorqueryWindowsImpl()
{
}


LibphystorqueryWindowsImpl::~LibphystorqueryWindowsImpl()
{
}

JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> LibphystorqueryWindowsImpl::getVolumeList()
{
	JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> object = new Libphystorquery::VolumeList;
	HANDLE hFindVolume = NULL;
	do {
		char szPathBuffer[MAX_PATH];
		char szNameBuffer[MAX_PATH] = { 0 };
		hFindVolume = FindFirstVolumeA(szNameBuffer, MAX_PATH);
		if (hFindVolume == INVALID_HANDLE_VALUE)
		{
			break;
		}
		do {
			size_t ts;
			DWORD dwRst = 0;
			JsCPPUtils::SmartPointer<Libphystorquery::VolumeInfo> volumeInfo;
			RtlSecureZeroMemory(szPathBuffer, sizeof(szPathBuffer));
			GetVolumePathNamesForVolumeNameA(szNameBuffer, szPathBuffer, MAX_PATH, &dwRst);
			volumeInfo = getVolumeInfo(szPathBuffer[0], szNameBuffer);
			if (volumeInfo != NULL) {
				object->list.push_back(volumeInfo);
			}
			RtlSecureZeroMemory(szNameBuffer, sizeof(szNameBuffer));
		} while (FindNextVolumeA(hFindVolume, szNameBuffer, MAX_PATH));

	} while (0);
	if ((hFindVolume != NULL) && (hFindVolume != INVALID_HANDLE_VALUE))
	{
		FindVolumeClose(hFindVolume);
		hFindVolume = INVALID_HANDLE_VALUE;
	}
	return object;
}

JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> LibphystorqueryWindowsImpl::openPhyDevice(const std::string& deviceName)
{
	JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> object;
	HANDLE hDrive;

	int i;
	HANDLE hTmpPartition;
	BOOL bRst;
	DWORD dwErr;
	DWORD dwBytesReturned;
	DWORD dwPropertyRegDataType;

	char *pBuffer;
	DISK_GEOMETRY geometry;
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader;
	STORAGE_PROPERTY_QUERY storagePropertyQuery;

	hDrive = ::CreateFileA(deviceName.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDrive == NULL || hDrive == INVALID_HANDLE_VALUE)
		return NULL;

	object = new Libphystorquery::DeviceInfo;
	object->deviceName = deviceName;

	bRst = DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &geometry, sizeof(geometry), &dwBytesReturned, NULL);
	if (bRst)
	{
	}

	pBuffer = NULL;
	do {
		DWORD dwSize;
		STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor;

		RtlSecureZeroMemory(&storageDescriptorHeader, sizeof(storageDescriptorHeader));
		RtlSecureZeroMemory(&storagePropertyQuery, sizeof(storagePropertyQuery));
		storagePropertyQuery.PropertyId = StorageDeviceProperty;
		storagePropertyQuery.QueryType = PropertyStandardQuery;

		dwBytesReturned = 0;
		if (!::DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY,
			&storagePropertyQuery, sizeof(storagePropertyQuery),
			&storageDescriptorHeader, sizeof(storageDescriptorHeader),
			&dwBytesReturned, NULL))
		{
			// error
			break;
		}

		// allocate the necessary memory for the output buffer
		dwSize = storageDescriptorHeader.Size;
		pBuffer = (char*)malloc(dwSize);
		memset(pBuffer, 0, dwSize);

		dwBytesReturned = 0;
		// get the storage device descriptor
		if (!::DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY,
			&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
			pBuffer, dwSize,
			&dwBytesReturned, NULL))
		{
			break;
		}

		// Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
		// followed by additional info like vendor ID, product ID, serial number, and so on.
		pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pBuffer;

		if (pDeviceDescriptor->VendorIdOffset > 0)
		{
			// get the vendor
			JsCPPUtils::StringBuffer<char> tmpsb;
			tmpsb = (pBuffer + pDeviceDescriptor->VendorIdOffset);
			object->vendorName = tmpsb.trim2_cstr();
		}

		if (pDeviceDescriptor->ProductIdOffset > 0)
		{
			// get the vendor
			JsCPPUtils::StringBuffer<char> tmpsb;
			tmpsb = (pBuffer + pDeviceDescriptor->ProductIdOffset);
			object->productName = tmpsb.trim2_cstr();
		}

		if (pDeviceDescriptor->SerialNumberOffset > 0)
		{
			JsCPPUtils::StringBuffer<char> tmpsb;
			char serbuf[64] = { 0 };
			flipAndCodeBytes((const BYTE*)(pBuffer + pDeviceDescriptor->SerialNumberOffset), 1, serbuf);
			tmpsb = serbuf;
			object->serialNumber = tmpsb.trim2_cstr();
		}

		/*
		switch (pDeviceDescriptor->BusType)
		{
		case BusTypeUsb:
			spitem->isExternalMedia = true;
			break;
		case BusType1394:
			spitem->isExternalMedia = true;
			break;
		case BusTypeSd:
			spitem->isExternalMedia = true;
			break;
		case BusTypeMmc:
			spitem->isExternalMedia = true;
			break;
		}
		*/
	} while (0);
	if (pBuffer != NULL)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	object->totalSize =
		(uint64_t)geometry.Cylinders.QuadPart *
		(uint64_t)geometry.TracksPerCylinder *
		(uint64_t)geometry.SectorsPerTrack *
		(uint64_t)geometry.BytesPerSector;

	return object;
}

JsCPPUtils::SmartPointer<Libphystorquery::VolumeInfo> LibphystorqueryWindowsImpl::getVolumeInfo(char letter, char *szVolumeName)
{
	JsCPPUtils::SmartPointer<Libphystorquery::VolumeInfo> object = new Libphystorquery::VolumeInfo;

	int i;
	HANDLE hVolume;
	BOOL bRst;
	DWORD dwErr = ERROR_SUCCESS;
	DWORD dwBytesReturned;
	TCHAR szTempBuf[MAX_PATH];
	DWORD dwMaximumComponentLength = 0;
	DWORD dwFileSystemFlags = 0;
	VOLUME_DISK_EXTENTS_LX volumeDiskExtents = {0};
	size_t tmpsz;

	if (letter) {
		char szMountPath[4] = {letter, ':', '\\', 0};
		object->mountPath = szMountPath;
	}

	tmpsz = strlen(szVolumeName);
	szVolumeName[tmpsz - 1] = 0;
	object->volumeName = szVolumeName;

	hVolume = CreateFileA(szVolumeName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hVolume != INVALID_HANDLE_VALUE)
	{
		do {
			/*
			dwBytesReturned = 0;
			bRst = DeviceIoControl(hVolume,
				IOCTL_STORAGE_CHECK_VERIFY,
				NULL, 0, NULL, 0, &dwBytesReturned, (LPOVERLAPPED)NULL);

			if (!bRst)
			{
				dwErr = GetLastError();
				break;
			}
			*/

			dwBytesReturned = 0;
			bRst = DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &volumeDiskExtents, sizeof(volumeDiskExtents), &dwBytesReturned, NULL);
			if (!bRst)
			{
				dwErr = GetLastError();
				break;
			}
			for (i = 0; i < volumeDiskExtents.NumberOfDiskExtents && volumeDiskExtents.NumberOfDiskExtents <= 4; i++)
			{
				PDISK_EXTENT pDiskExtent = &volumeDiskExtents.Extents[i];
				if (pDiskExtent->ExtentLength.QuadPart == 0)
				{
					continue;
				}

				if (0 <= pDiskExtent->DiskNumber && pDiskExtent->DiskNumber < 256)
				{
					char szDeviceName[40];
					sprintf_s(szDeviceName, "\\\\.\\PhysicalDrive%d", pDiskExtent->DiskNumber);
					object->deviceName = szDeviceName;
					break;
				}
			}
		} while (0);
		CloseHandle(hVolume);
	}

	return object;
}

static bool flipAndCodeBytes(const BYTE * str, int flip, char * buf)
{
	int i;
	int j = 0;
	int k = 0;

	buf[0] = '\0';

	//if (!j)
	{
		char p = 0;

		// First try to gather all characters representing hex digits only.
		j = 1;
		k = 0;
		buf[k] = 0;
		for (i = 0; j && str[i] != '\0'; ++i)
		{
			char c = tolower((unsigned char)str[i]);

			if (isspace((unsigned char)c))
				c = '0';

			++p;
			buf[k] <<= 4;

			if (c >= '0' && c <= '9')
				buf[k] |= (unsigned char)(c - '0');
			else if (c >= 'a' && c <= 'f')
				buf[k] |= (unsigned char)(c - 'a' + 10);
			else if (c >= 'A' && c <= 'F')
				buf[k] |= (unsigned char)(c - 'A' + 10);
			else
			{
				j = 0;
				break;
			}

			if (p == 2)
			{
				if (buf[k] != '\0' && !isprint((unsigned char)buf[k]))
				{
					j = 0;
					break;
				}
				++k;
				p = 0;
				buf[k] = 0;
			}

		}

		if (i != 40)
		{
			j = 0;
		}

		if (j && flip)
			// Flip adjacent characters
			for (j = 0; j < k; j += 2)
			{
				char t = buf[j];
				buf[j] = buf[j + 1];
				buf[j + 1] = t;
			}
	}

	if (!j)
	{
		// There are non-digit characters, gather them as is.
		j = 1;
		k = 0;
		for (i = 0; j && str[i] != '\0'; ++i)
		{
			char c = str[i];

			if (!isprint((unsigned char)c))
			{
				j = 0;
				break;
			}

			buf[k++] = c;
		}
	}

	if (!j)
	{
		// The characters are not there or are not printable.
		k = 0;
	}

	buf[k] = '\0';

	/*
	if (flip)
	// Flip adjacent characters
	for (j = 0; j < k; j += 2)
	{
	char t = buf[j];
	buf[j] = buf[j + 1];
	buf[j + 1] = t;
	}
	*/

	// Trim any beginning and end space
	i = j = -1;
	for (k = 0; buf[k] != '\0'; ++k)
	{
		if (!isspace((unsigned char)buf[k]))
		{
			if (i < 0)
				i = k;
			j = k;
		}
	}

	if ((i >= 0) && (j >= 0))
	{
		for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
			buf[k - i] = buf[k];
		buf[k - i] = '\0';
	}

	return buf;
}

#endif /* JSCUTILS_PLATFORM_ISWINDOWS */
