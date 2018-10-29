/*!
 *  \author    Jichan (development@jc-lab.net)
 *  \version   1.0
 *  \date      2018.10
 *  \copyright Apache License 2.0
 */

#pragma once

#include <JsCPPUtils/SmartPointer.h>

#include <list>
#include <string>

class Libphystorquery
{
public:
	struct VolumeInfo
	{
		std::string volumeName;
		std::string mountPath;
		std::string deviceName;
	};

	struct DeviceInfo
	{
		std::string deviceName;
		std::string vendorName;
		std::string productName;
		std::string serialNumber;
		int64_t totalSize;

		DeviceInfo() {
			this->totalSize = 0;
		}
	};

	struct VolumeList
	{
		std::list< JsCPPUtils::SmartPointer<VolumeInfo> > list;
	};

public:
	Libphystorquery() {}
	virtual ~Libphystorquery() {}
	virtual JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> getVolumeList() = 0;
	virtual JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> openPhyDevice(const std::string& deviceName) = 0;
};

