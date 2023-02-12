#pragma once
#include "LFObjectPool.h"

// ���� 11:07 2023-01-03

#define CHUNCK_SIZE 500
#define TLSPOOL_MONITORING

template <typename T>
class LFObjectPoolTLS {
public:
	LFObjectPoolTLS(bool flag_placementNew = false);
	~LFObjectPoolTLS();

public:
	struct Chunk;
	static struct ChunkData {
		Chunk* p_chunk;
		T object;
	};

public:
	static struct Chunk {
	public:
		Chunk() {}
		~Chunk() {}

	private:
		ChunkData chunkData_array[CHUNCK_SIZE];
		int alloc_count;
		alignas(32) int free_count;
		bool flag_placementNew;

	public:
		// ûũ���� �Ҵ����� �����Ͱ� ���ٸ� ret nullptr
		T* Alloc() {
			// �Ҵ� ����
			if (alloc_count < CHUNCK_SIZE) {
				ChunkData* p_chunkData = &chunkData_array[alloc_count++];
				T* p_object = &p_chunkData->object;
				if (flag_placementNew) new (p_object) T;
				return p_object;
			}

			// �Ҵ� ����
			return nullptr;
		}

		// true ��ȯ �� ������ ����
		bool Free(T* p_object) {
			if (flag_placementNew) p_object->~T();

			if (CHUNCK_SIZE == InterlockedIncrement((DWORD*)&free_count)) {
				if (free_count > alloc_count) CRASH();
				return true;
			}
			else
				return false;
		}

		void Clear(bool flag_placementNew) {
			alloc_count = 0;
			free_count = 0;
			this->flag_placementNew = flag_placementNew;
			for (auto& chunkData : chunkData_array) {
				chunkData.p_chunk = this;
			}
		}
	};

private:
	J_LIB::LFObjectPool<Chunk> chunkPool;
	const int tlsIndex;
	const bool flag_placementNew;
	alignas(32) int use_count = 0;

private:
	Chunk* ChunkAlloc() {
		Chunk* p_chunk = chunkPool.Alloc();
		p_chunk->Clear(flag_placementNew);
		return p_chunk;
	}

	void ChunkFree(Chunk* p_chunk) {
		chunkPool.Free(p_chunk);
	}

public:
	int Get_ChunkCapacity() {
		return chunkPool.GetCapacityCount();
	}

	int Get_ChunkUseCount() {
		return chunkPool.GetUseCount();
	}

	int Get_UseCount() {
		return use_count;
	}

	T* Alloc() {
#ifdef TLSPOOL_MONITORING
		InterlockedIncrement((LONG*)&use_count);
#endif

		// TLS���� ûũ�� ������
		Chunk* p_chunk = (Chunk*)TlsGetValue(tlsIndex);
		if (p_chunk == nullptr) {
			p_chunk = ChunkAlloc();
			TlsSetValue(tlsIndex, p_chunk);
		}

		// ûũ���� ������Ʈ �Ҵ�
		T* p_object = p_chunk->Alloc();
		if (p_object == nullptr) {
			p_chunk = ChunkAlloc();
			TlsSetValue(tlsIndex, p_chunk);
			p_object = p_chunk->Alloc();
		}

		return p_object;
	}

	void Free(T* p_object) {
#ifdef TLSPOOL_MONITORING
		InterlockedDecrement((LONG*)&use_count);
#endif

		ChunkData* p_chunkData = (ChunkData*)((char*)p_object - sizeof(Chunk*));
		if (p_chunkData->p_chunk->Free(p_object)) {
			ChunkFree(p_chunkData->p_chunk);
		}
	}
};

//------------------------------
// LFObjectPoolTLS
//------------------------------

template<typename T>
inline LFObjectPoolTLS<T>::LFObjectPoolTLS(bool flag_placementNew) : flag_placementNew(flag_placementNew), tlsIndex(TlsAlloc()) {
}

template<typename T>
inline LFObjectPoolTLS<T>::~LFObjectPoolTLS(){
}
