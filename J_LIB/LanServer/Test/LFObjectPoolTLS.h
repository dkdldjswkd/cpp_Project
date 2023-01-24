#pragma once
#include "LFObjectPool.h"

// 오전 3:46 2023-01-25

#define CHUNCK_SIZE 500
#define TLSPOOL_MONITORING

//struct T { int a; };
template <typename T>
class LFObjectPoolTLS {
public:
	LFObjectPoolTLS(bool flag_placementNew = false) : flag_placementNew(flag_placementNew), tlsIndex(TlsAlloc()) {};
	~LFObjectPoolTLS() {}

private:
	struct Chunk;

private:
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
	// 스레드마다 다른 Chunk를 들고있음
	static struct Chunk {
	public:
		Chunk() {}
		~Chunk() {}

	private:
		J_LIB::LFObjectPool<Chunk>* p_myPool;
		bool flag_placementNew;
		int tlsIndex;

	private:
		ChunkData chunkData_array[CHUNCK_SIZE];
		int alloc_index;
		int free_index;

	public:
		// 할당 시 필수
		void Set(J_LIB::LFObjectPool<Chunk>* p_chunkPool, bool flag_new, int _tlsIndex) {
			p_myPool = p_chunkPool;
			flag_placementNew = flag_new;
			tlsIndex = _tlsIndex;

			alloc_index = 0;
			free_index = 0;
			for (int i = 0; i < CHUNCK_SIZE; i++) {
				chunkData_array[i].p_chunk = this;
			}
		}

		inline bool CanAlloc() {
			if (alloc_index < CHUNCK_SIZE)
				return true;
			return false;
		}

		T* Alloc() {
			T* p_object = &(chunkData_array[alloc_index++].object);
			if (flag_placementNew) new (p_object) T;

			// 모두 할당했다면, 청크풀에서 재할당
			if (CHUNCK_SIZE == alloc_index) {
				Chunk* p_chunk = p_myPool->Alloc();
				p_chunk->Set(p_myPool, flag_placementNew, tlsIndex);
				TlsSetValue(tlsIndex, (LPVOID)p_chunk);
			}
			return p_object;
		}

		void Free(T* p_object) {
			if (flag_placementNew) p_object->~T();

			// 전부 반환했다면, 청크풀에 반환
			if (CHUNCK_SIZE == InterlockedIncrement((DWORD*)&free_index)) {
				if (free_index > alloc_index) CRASH(); //중복반환
				p_myPool->Free(this);
			}
		}
	};

private:
	J_LIB::LFObjectPool<Chunk> chunkPool;
	const int tlsIndex;
	const bool flag_placementNew;
	alignas(32) int use_count = 0;

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
		
		Chunk* p_chunk = (Chunk*)TlsGetValue(tlsIndex);
		if (nullptr == p_chunk) {
			// 해당 스레드에서 첫 호출
			p_chunk = chunkPool.Alloc();
			p_chunk->Set(&chunkPool, flag_placementNew, tlsIndex);
			TlsSetValue(tlsIndex, p_chunk);
		}

		return p_chunk->Alloc();
	}

	void Free(T* p_object) {
		#ifdef TLSPOOL_MONITORING
		InterlockedDecrement((LONG*)&use_count);
		#endif

		ChunkData* p_chunkData = (ChunkData*)((char*)p_object - sizeof(Chunk*));
		p_chunkData->Free();
	}
};