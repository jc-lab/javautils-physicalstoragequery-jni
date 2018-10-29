/*!
 *  \author    Jichan (development@jc-lab.net)
 *  \version   1.0
 *  \date      2018.10
 *  \copyright Apache License 2.0
 */

#pragma once

#include <JsCPPUtils/SmartPointer.h>
#include <JsCPPUtils/HashMap.h>
#include <stdint.h>

class AppHandleManager
{
public:
	enum ObjectType {
		TYPE_UNKNOWN = 0,
		TYPE_SP
	};

	class BaseObject {
	protected:
		ObjectType _type;
		void *_id;

	public:
		BaseObject() {
			_type = TYPE_UNKNOWN;
			_id = NULL;
		}
		virtual ~BaseObject() { }
		void *getId() {
			return this->_id;
		}
		ObjectType getType() {
			return this->_type;
		}
	};

	template <typename T>
	class SPObject : public BaseObject {
	public:
		JsCPPUtils::SmartPointer<T> ptr;

		SPObject(T *_ptr) {
			this->_type = TYPE_SP;
			this->ptr = _ptr;
			this->_id = (void*)_ptr;
		}
		SPObject(JsCPPUtils::SmartPointer<T> _ptr) {
			this->_type = TYPE_SP;
			this->ptr = _ptr;
			this->_id = (void*)_ptr.getPtr();
		}
		virtual ~SPObject() { }
	};

private:
	JsCPPUtils::HashMap<void*, JsCPPUtils::SmartPointer<BaseObject> > m_managedObjects;

public:
	AppHandleManager();
	~AppHandleManager();

	bool freeObject(void *id);

	/*
	template <typename T>
	BaseObject *storeObject(BaseObject *baseObject) {
		m_managedObjects[baseObject->getId()] = baseObject;
		return baseObject;
	}

	template <typename T>
	JsCPPUtils::SmartPointer<T> storeSPObject(T *ptr) {
		SPObject<T> *spObject = new SPObject<T>(ptr);
		m_managedObjects[(void*)ptr] = spObject;
		return spObject->ptr;
	}
	*/

	template <typename T>
	JsCPPUtils::SmartPointer<T> storeSPObject(JsCPPUtils::SmartPointer<T> ptr) {
		m_managedObjects[(void*)ptr.getPtr()] = new SPObject<T>(ptr);
		return ptr;
	}

	/*
	BaseObject *getObject(void *id) {
		JsCPPUtils::SmartPointer<BaseObject> baseObject = NULL;
		JsCPPUtils::HashMap<void*, JsCPPUtils::SmartPointer<BaseObject> >::Iterator iter;
		m_managedObjects.iteratoring_lock();
		iter = m_managedObjects.find(id);
		if (iter.hasNext()) {
			baseObject = iter.next();
		}
		m_managedObjects.iteratoring_unlock();
		return baseObject;
	}
	*/

	template<typename T>
	JsCPPUtils::SmartPointer<T> getSPObject(void *id) {
		JsCPPUtils::SmartPointer<BaseObject> baseObject = NULL;
		JsCPPUtils::HashMap<void*, JsCPPUtils::SmartPointer<BaseObject> >::Iterator iter;
		m_managedObjects.iteratoring_lock();
		iter = m_managedObjects.find(id);
		if (iter.hasNext()) {
			baseObject = iter.next();
		}
		m_managedObjects.iteratoring_unlock();

		if (baseObject == NULL)
			return NULL;

		if (baseObject->getType() == TYPE_SP)
		{
			SPObject<T> *spObject = (SPObject<T>*)baseObject.getPtr();
			return spObject->ptr;
		}

		return NULL;
	}

};

