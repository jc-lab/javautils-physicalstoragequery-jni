/*!
 *  \author    Jichan (development@jc-lab.net)
 *  \version   1.0
 *  \date      2018.10
 *  \copyright Apache License 2.0
 */

#pragma once

#include <JsCPPUtils/Common.h>
#include "Libphystorquery.h"

#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <mntent.h>

#if JSCUTILS_PLATFORM_ISLINUX()

class LibphystorqueryLinuxImpl : public Libphystorquery
{
private:
	struct MountInfo
	{
		std::string mnt_fsname;
		std::string mnt_dir;
	
		MountInfo() {}
		MountInfo(struct mntent *pent)
		{
			this->mnt_fsname = pent->mnt_fsname;
			this->mnt_dir = pent->mnt_dir;
		}
	};

	struct ProcessContext
	{
		std::list<MountInfo> mountpoints;
	};

	struct BlockDeviceVolume;

	struct BlockDeviceBase
	{
		JsCPPUtils::SmartPointer<BlockDeviceBase> parent;
		std::string path;
		std::string name;
		struct stat devstat;
		struct statvfs fsstat;
		std::list < JsCPPUtils::SmartPointer<BlockDeviceVolume> > partitions;
	
		BlockDeviceBase() {
			memset(&fsstat, 0, sizeof(fsstat));
		}
		BlockDeviceBase(JsCPPUtils::SmartPointer<BlockDeviceBase> _parent)
			: parent(_parent) {
			memset(&fsstat, 0, sizeof(fsstat)); 
		}
	
		int readStat()
		{
			int rc;
			if ((rc = stat(this->path.c_str(), &devstat)) != 0)
				return rc;
			rc = statvfs(this->path.c_str(), &fsstat);
			return 0;
		}
	};

	struct BlockDeviceInfo : BlockDeviceBase
	{
		std::string vendor;
		std::string model;
		std::string serial;
	};

	struct BlockDeviceVolume : BlockDeviceBase
	{
		std::string mnt_dir;
	
		BlockDeviceVolume()
			: BlockDeviceBase() {
		}
		BlockDeviceVolume(JsCPPUtils::SmartPointer<BlockDeviceBase> _parent)
			: BlockDeviceBase(_parent) {
		}
	};
	
public:
	LibphystorqueryLinuxImpl();
	virtual ~LibphystorqueryLinuxImpl();

	JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> getVolumeList() override;
	JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> openPhyDevice(const std::string& deviceName) override;

private:
	struct udev *m_udev;
	
	int find_process_get_properties_by_udev(ProcessContext *processContext, JsCPPUtils::SmartPointer<BlockDeviceInfo> blockDevice);
	int find_process_mountpoint(ProcessContext *processContext, JsCPPUtils::SmartPointer<BlockDeviceVolume> spPartitionInfo);
	int find_mountpoints(ProcessContext *processContext);
	int find_block_devices();
	
};

#endif
