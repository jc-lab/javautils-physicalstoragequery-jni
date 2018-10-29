/*!
 *  \author    Jichan (development@jc-lab.net)
 *  \version   1.0
 *  \date      2018.10
 *  \copyright Apache License 2.0
 */

#include "AppHandleManager.h"

AppHandleManager::AppHandleManager()
{
}


AppHandleManager::~AppHandleManager()
{
}

bool AppHandleManager::freeObject(void *id) {
	bool result = false;
	JsCPPUtils::HashMap<void*, JsCPPUtils::SmartPointer<BaseObject> >::Iterator iter;
	m_managedObjects.iteratoring_lock();
	iter = m_managedObjects.find(id);
	if (iter.hasNext()) {
		iter.next();
		iter.erase();
		result = true;
	}
	m_managedObjects.iteratoring_unlock();
	return result;
}