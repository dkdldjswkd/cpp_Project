#pragma once
#include "LFObjectPool.h"

// ���� 10:04 2023-01-24

#define CHUNCK_SIZE 500
#define TLSPOOL_MONITORING

template <typename T>
class LFObjectPoolTLS {
public:
	LFObjectPoolTLS(bool flag_placementNew = false) : flag_placementNew(flag_placementNew), tlsIndex(TlsAlloc()) {};
	~LFObjectPoolTLS() {}

public:
	struct Chunk;
	static struct ChunkData {
	public:
		Chunk* p_chunk;
		T object;

	public:
		void Free() {
			p_chunk->Free(&object);
		}
	};

public:
	// �����帶�� �ٸ� Chunk�� �������
	static struct Chunk {
	public:
		Chunk() {}
		~Chunk() {}

	private:
		J_LIB::LFObjectPool<Chunk>* p_myPool;
		ChunkData chunkData_array[CHUNCK_SIZE];
		int alloc_index;
		alignas(32) int free_index;
		bool flag_placementNew;

	public:
		inline bool CanAlloc() {
			if (alloc_index < CHUNCK_SIZE)
				return true;
			return false;
		}

		T* Alloc() {
			T* p_object = &(chunkData_array[alloc_index++].object);
			if (flag_placementNew) new (p_object) T;
			return p_object;
		}

		// ����ȣ���� �� ����.
		void Free(T* p_object) {
			if (flag_placementNew) p_object->~T();
			if (CHUNCK_SIZE == InterlockedIncrement((DWORD*)&free_index)) {
				if (free_index > alloc_index) CRASH(); //�ߺ���ȯ
				p_myPool->Free(this);
			}
		}

		void Set(bool flag_placementNew, J_LIB::LFObjectPool<Chunk>* p_chunkPool) {
			this->flag_placementNew = flag_placementNew;
			p_myPool = p_chunkPool;
			alloc_index = 0;
			free_index = 0;
			for (int i = 0; i < CHUNCK_SIZE; i++) {
				chunkData_array[i].p_chunk = this;
			}
		}
	};

private:
	J_LIB::LFObjectPool<Chunk> chunkPool;
	const int tlsIndex;
	const bool flag_placementNew;
	alignas(32) int use_count = 0;

private:
	// ��ȿ�� Chunk ��ȯ
	Chunk* ChunkAlloc() {
		Chunk* p_chunk = (Chunk*)TlsGetValue(tlsIndex);
		if (p_chunk == nullptr) {
			// �����忡�� ���� 1ȸ ȣ�� ��
			p_chunk = chunkPool.Alloc();
			p_chunk->Set(flag_placementNew, &chunkPool);
			TlsSetValue(tlsIndex, (LPVOID)p_chunk);
		}
		if (false == p_chunk->CanAlloc()) {
			// ûũ ������ ��� �Ҵ����ػ���
			p_chunk = chunkPool.Alloc();
			p_chunk->Set(flag_placementNew, &chunkPool);
			TlsSetValue(tlsIndex, (LPVOID)p_chunk);
		}
		return p_chunk;
	}

public:
	int Get_ChunkCapacity() {
		return chunkPool.GetCapacityCount();
	}

	int Get_ChunkUseCount() {
		return chunkPool.Get_UseCount();
	}

	int Get_UseCount() {
		return use_count;
	}

	T* Alloc() {
		#ifdef TLSPOOL_MONITORING
		InterlockedIncrement((LONG*)&use_count);
		#endif
		
		// ChunkAlloc()�� ���� ûũ�� ��ȿ�� ûũ (�ƴ϶�� ����)
		return ChunkAlloc()->Alloc();
	}

	void Free(T* p_object) {
		#ifdef TLSPOOL_MONITORING
		InterlockedDecrement((LONG*)&use_count);
		#endif

		ChunkData* p_chunkData = (ChunkData*)((char*)p_object - sizeof(Chunk*));
		p_chunkData->Free();
	}
};