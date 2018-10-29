/*!
 *  \author    Jichan (development@jc-lab.net)
 *  \version   1.0
 *  \date      2018.10
 *  \copyright Apache License 2.0
 */

#pragma once

#include <JsCPPUtils/Common.h>
#include "Libphystorquery.h"

#if JSCUTILS_PLATFORM_ISWINDOWS()

class LibphystorqueryWindowsImpl : public Libphystorquery
{
public:
	LibphystorqueryWindowsImpl();
	virtual ~LibphystorqueryWindowsImpl();

	JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> getVolumeList();
	JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> openPhyDevice(const std::string& deviceName);

private:
	JsCPPUtils::SmartPointer<Libphystorquery::VolumeInfo> getVolumeInfo(char letter, char *szVolumeName);

};

#endif
