#include "LibphystorqueryLinuxImpl.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <dirent.h>
/*!
 *  \author    Jichan (development@jc-lab.net)
 *  \version   1.0
 *  \date      2018.10
 *  \copyright Apache License 2.0
 */

#include <mntent.h>
#include <sys/statvfs.h>

#include <libudev.h>

#include <JsCPPUtils/SmartPointer.h>
#include <string>
#include <list>
#include <map>

#if JSCUTILS_PLATFORM_ISLINUX()

LibphystorqueryLinuxImpl::LibphystorqueryLinuxImpl()
{
	m_udev = udev_new();
}
LibphystorqueryLinuxImpl::~LibphystorqueryLinuxImpl()
{
	udev_unref(m_udev);
	m_udev = NULL;
}

JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> LibphystorqueryLinuxImpl::getVolumeList()
{
	JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> volumeList = new Libphystorquery::VolumeList();
	
	DIR *dir;
	struct dirent *file;
	std::list<std::string> devlist;
	
	ProcessContext processContext;
	
	find_mountpoints(&processContext);
	
	dir = opendir("/dev");
	if (!dir)
		return -1;
	while ((file = readdir(dir)))
	{
		if (strcmp(file->d_name, ".") && strcmp(file->d_name, ".."))
		{
			devlist.push_back(file->d_name);
		}
	}
	closedir(dir);
	
	dir = opendir("/sys/block");
	if (!dir)
		return -1;
	while ((file = readdir(dir)))
	{
		if (strcmp(file->d_name, ".") && strcmp(file->d_name, ".."))
		{
			int filenamelen = strlen(file->d_name);
			JsCPPUtils::SmartPointer<BlockDeviceInfo> spBlockDevice = new BlockDeviceInfo();
			spBlockDevice->name = file->d_name;
			spBlockDevice->path = "/sys/block/";
			spBlockDevice->path.append(file->d_name);
			spBlockDevice->readStat();
			find_process_get_properties_by_udev(&processContext, spBlockDevice);
			
			for (std::list<std::string>::const_iterator iter = devlist.begin(); iter != devlist.end(); iter++)
			{
				if ((iter->length() > filenamelen) && (strncmp(iter->c_str(), file->d_name, filenamelen) == 0))
				{
					JsCPPUtils::SmartPointer<BlockDeviceVolume> spPartitionInfo = new BlockDeviceVolume(spBlockDevice);
					spPartitionInfo->name = file->d_name;
					spPartitionInfo->path = "/dev/";
					spPartitionInfo->path.append(*iter);
					spBlockDevice->readStat();
					if (find_process_mountpoint(&processContext, spPartitionInfo) == 0)
					{
						JsCPPUtils::SmartPointer<Libphystorquery::VolumeInfo> volumeInfo = new Libphystorquery::VolumeInfo();
						volumeInfo->mountPath = spPartitionInfo->mnt_dir;
						volumeInfo->volumeName = spPartitionInfo->path;
						volumeInfo->deviceName = spBlockDevice->name;
						volumeList->list.push_back(volumeInfo);
					}
					spBlockDevice->partitions.push_back(spPartitionInfo);
				}
			}
		}
	}
	closedir(dir);
	
	return volumeList;
}

JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> LibphystorqueryLinuxImpl::openPhyDevice(const std::string& deviceName)
{
	JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> spDeviceInfo;
	JsCPPUtils::SmartPointer<BlockDeviceInfo> spBlockDevice = new BlockDeviceInfo();
	ProcessContext processContext;
	find_mountpoints(&processContext);
	spBlockDevice->name = deviceName;
	spBlockDevice->path = "/sys/block/";
	spBlockDevice->path.append(deviceName);
	if (spBlockDevice->readStat() != 0)
		return NULL;
	find_process_get_properties_by_udev(&processContext, spBlockDevice);
	
	spDeviceInfo = new Libphystorquery::DeviceInfo();
	spDeviceInfo->deviceName = spBlockDevice->name;
	spDeviceInfo->vendorName = spBlockDevice->vendor;
	spDeviceInfo->productName = spBlockDevice->model;
	spDeviceInfo->serialNumber = spBlockDevice->serial;
	return spDeviceInfo;
}

int LibphystorqueryLinuxImpl::find_process_get_properties_by_udev(ProcessContext *processContext, JsCPPUtils::SmartPointer<BlockDeviceInfo> blockDevice)
{
	struct udev_device *dev;
	dev = udev_device_new_from_subsystem_sysname(m_udev, "block", blockDevice->name.c_str());
	if (dev) {
		const char *data;
		if ((data = udev_device_get_property_value(dev, "ID_SERIAL_SHORT")))
		{
			blockDevice->serial = data;
		}
		if ((data = udev_device_get_property_value(dev, "ID_MODEL")))
		{
			blockDevice->model = data;
		}
		if ((data = udev_device_get_property_value(dev, "ID_VENDOR")))
		{
			blockDevice->vendor = data;
		}
		udev_device_unref(dev);
		return 0;
	}
	return -1;
}

int LibphystorqueryLinuxImpl::find_process_mountpoint(ProcessContext *processContext, JsCPPUtils::SmartPointer<BlockDeviceVolume> spPartitionInfo)
{
	for (std::list<MountInfo>::const_iterator iter = processContext->mountpoints.begin(); iter != processContext->mountpoints.end(); iter++)
	{
		if (spPartitionInfo->path == iter->mnt_fsname)
		{
			spPartitionInfo->mnt_dir = iter->mnt_dir;
			return 0;
		}
	}
	return 1;
}

int LibphystorqueryLinuxImpl::find_mountpoints(ProcessContext *processContext)
{
	FILE *fp;
	struct mntent ent;
	struct mntent *pent = &ent;
	char buffer[1024];
	
	fp = setmntent("/proc/mounts", "r");
	if (!fp) {
		return -1;
	}
	
#ifdef __USE_MISC
	while (NULL != (getmntent_r(fp, pent, buffer, sizeof(buffer))))
#else
	while (NULL != (pent = getmntent(fp)))
#endif
	{
		processContext->mountpoints.push_back(MountInfo(pent));
	}
	endmntent(fp);
}


#endif
