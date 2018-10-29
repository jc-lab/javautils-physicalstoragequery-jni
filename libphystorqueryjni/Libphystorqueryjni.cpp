/*!
 *  \author    Jichan (development@jc-lab.net)
 *  \version   1.0
 *  \date      2018.10
 *  \copyright Apache License 2.0
 */

#include "Libphystorqueryjni.h"

#include "AppHandleManager.h"
#include "Libphystorquery.h"

#if JSCUTILS_PLATFORM_ISWINDOWS()
#include "LibphystorqueryWindowsImpl.h"
JsCPPUtils::SmartPointer<Libphystorquery> g_libPhyStoreQueryImpl = new LibphystorqueryWindowsImpl();
#elif JSCUTILS_PLATFORM_ISLINUX()
#include "LibphystorqueryLinuxImpl.h"
JsCPPUtils::SmartPointer<Libphystorquery> g_libPhyStoreQueryImpl = new LibphystorqueryLinuxImpl();
#endif

AppHandleManager g_appHandleManager;

/*
* Class:     kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni
* Method:    freeObject
* Signature: (J)V
*/
JNIEXPORT void JNICALL Java_kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni_freeObject
(JNIEnv *env, jobject obj, jlong handle)
{
	g_appHandleManager.freeObject((void*)handle);
}

/*
* Class:     kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni
* Method:    getVolumeList
* Signature: ()J
*/
JNIEXPORT jlong JNICALL Java_kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni_getVolumeList
(JNIEnv *env, jobject obj)
{
	JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> ptr = g_libPhyStoreQueryImpl->getVolumeList();
	if (ptr != NULL)
	{
		g_appHandleManager.storeSPObject(ptr);
		return (jlong)ptr.getPtr();
	}
	return 0;
}

/*
* Class:     kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni
* Method:    getVolumeListCount
* Signature: (J)I
*/
JNIEXPORT jint JNICALL Java_kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni_getVolumeListCount
(JNIEnv *env, jobject obj, jlong handle)
{
	JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> ptr = g_appHandleManager.getSPObject<Libphystorquery::VolumeList>((void*)handle);
	if (ptr == NULL)
		return -1;
	return ptr->list.size();
}

/*
* Class:     kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni
* Method:    getVolumePhyDeviceName
* Signature: (JI)Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni_getVolumePhyDeviceName
(JNIEnv *env, jobject obj, jlong handle, jint index)
{
	JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> ptr = g_appHandleManager.getSPObject<Libphystorquery::VolumeList>((void*)handle);
	std::list< JsCPPUtils::SmartPointer<Libphystorquery::VolumeInfo> >::iterator iter;
	int i;
	if (ptr == NULL)
		return NULL;
	if (index >= ptr->list.size())
		return NULL;
	iter = ptr->list.begin();
	for (i = 0; i < index; i++)
		iter++;
	return env->NewStringUTF((*iter)->deviceName.c_str());
}

/*
* Class:     kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni
* Method:    getVolumeMountPath
* Signature: (JI)Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni_getVolumeMountPath
(JNIEnv *env, jobject obj, jlong handle, jint index)
{
	JsCPPUtils::SmartPointer<Libphystorquery::VolumeList> ptr = g_appHandleManager.getSPObject<Libphystorquery::VolumeList>((void*)handle);
	std::list< JsCPPUtils::SmartPointer<Libphystorquery::VolumeInfo> >::iterator iter;
	int i;
	if (ptr == NULL)
		return NULL;
	if (index >= ptr->list.size())
		return NULL;
	iter = ptr->list.begin();
	for (i = 0; i < index; i++)
		iter++;
	return env->NewStringUTF((*iter)->mountPath.c_str());
}

/*
* Class:     kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni
* Method:    openPhyDevice
* Signature: (Ljava/lang/String;)J
*/
JNIEXPORT jlong JNICALL Java_kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni_openPhyDevice
(JNIEnv *env, jobject obj, jstring deviceName)
{
	const char *nativeDeviceName = env->GetStringUTFChars(deviceName, NULL);
	JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> ptr = g_libPhyStoreQueryImpl->openPhyDevice(nativeDeviceName);
	if (ptr != NULL)
	{
		g_appHandleManager.storeSPObject(ptr);
		return (jlong)ptr.getPtr();
	}
	return 0;
}

/*
* Class:     kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni
* Method:    getPhyDeviceVendorName
* Signature: (J)Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni_getPhyDeviceVendorName
(JNIEnv * env, jobject obj, jlong handle)
{
	JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> ptr = g_appHandleManager.getSPObject<Libphystorquery::DeviceInfo>((void*)handle);
	if (ptr == NULL)
		return NULL;
	return env->NewStringUTF(ptr->vendorName.c_str());
}

/*
* Class:     kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni
* Method:    getPhyDeviceProductName
* Signature: (J)Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni_getPhyDeviceProductName
(JNIEnv * env, jobject obj, jlong handle)
{
	JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> ptr = g_appHandleManager.getSPObject<Libphystorquery::DeviceInfo>((void*)handle);
	if (ptr == NULL)
		return NULL;
	return env->NewStringUTF(ptr->productName.c_str());
}

/*
* Class:     kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni
* Method:    getPhyDeviceSerialNumber
* Signature: (J)Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_kr_jclab_javautils_physicalstoragequery_internal_Libphystorqueryjni_getPhyDeviceSerialNumber
(JNIEnv * env, jobject obj, jlong handle)
{
	JsCPPUtils::SmartPointer<Libphystorquery::DeviceInfo> ptr = g_appHandleManager.getSPObject<Libphystorquery::DeviceInfo>((void*)handle);
	if (ptr == NULL)
		return NULL;
	return env->NewStringUTF(ptr->serialNumber.c_str());
}