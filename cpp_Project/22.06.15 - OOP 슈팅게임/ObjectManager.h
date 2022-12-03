#pragma once
#include "Actor.h"

class BaseObject;

#define OBJECT_LIST ObjectManager::Get_inst().object_list

class ObjectManager {
private:
	ObjectManager() {}
	static ObjectManager* inst;

public:
	static ObjectManager& Get_inst() {
		if (inst == nullptr) {
			inst = new ObjectManager;
			atexit(destroy);
		}
		
		return *inst;
	}

	static void destroy() {
		delete inst;
	}
	
public:
	list<BaseObject*> object_list;

public:
	void List_Update();
	void List_Render();
	void List_Collision();
	void List_Check_flag();
	bool List_Check_have(EObjectType type, int n);
	void List_Delete();
};