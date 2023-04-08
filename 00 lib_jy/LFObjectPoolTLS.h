#pragma once
#include "LFObjectPool.h"

#define CHUNCK_SIZE 500
#define USE_COUNT 1 // 1 : 낱개 단위 카운트, 2 : 청크 단위 카운트

template <typename T>
class LFObjectPoolTLS {
public:
	LFObjectPoolTLS(bool useCtor = false) : useCtor(useCtor), tlsIndex(TlsAlloc()) {};
	~LFObjectPoolTLS() {}

private:
	struct Chunk;
	static struct ChunkData {
	public:
		Chunk* p_myChunk;
		T object;

	public:
		void Free() {
			p_myChunk->Free(&object);
		}
	};

	static struct Chunk {
	public:
		Chunk() {}
		~Chunk() {}

	private:
		LFObjectPool<Chunk>* p_chunkPool;
		bool useCtor;
		int tlsIndex;

	private:
		ChunkData chunkDataArray[CHUNCK_SIZE];
		int allocIndex;
		alignas(64) int freeIndex;

	public:
		void Set(LFObjectPool<Chunk>* p_chunkPool, bool useCtor, int tlsIndex) {
			this->p_chunkPool = p_chunkPool;
			this->useCtor = useCtor;
			this->tlsIndex = tlsIndex;

			allocIndex = 0;
			freeIndex = 0;
			for (int i = 0; i < CHUNCK_SIZE; i++) {
				chunkDataArray[i].p_myChunk = this;
			}
		}

		T* Alloc() {
			T* p_object = &(chunkDataArray[allocIndex++].object);
			if (useCtor) new (p_object) T;

			if (CHUNCK_SIZE == allocIndex) {
				Chunk* p_chunk = p_chunkPool->Alloc();
				p_chunk->Set(p_chunkPool, useCtor, tlsIndex);
				TlsSetValue(tlsIndex, (LPVOID)p_chunk);
			}
			return p_object;
		}

		void Free(T* p_object) {
			if (useCtor) p_object->~T();

			if (CHUNCK_SIZE == InterlockedIncrement((DWORD*)&freeIndex)) {
				p_chunkPool->Free(this);
			}
		}
	};

private:
	LFObjectPool<Chunk> chunkPool;
	const int tlsIndex;
	const bool useCtor;
	int count = 0;

public:
	int GetChunkCapacity() {
		return chunkPool.GetCapacityCount();
	}

	int GetChunkUseCount() {
		return chunkPool.GetUseCount();
	}

	int GetUseCount() {
#if USE_COUNT
		return count;
#else
		return GetChunkUseCount() * CHUNCK_SIZE;
#endif
	}

	T* Alloc() {
#if USE_COUNT
		InterlockedIncrement((LONG*)&count);
#endif

		Chunk* p_chunk = (Chunk*)TlsGetValue(tlsIndex);
		if (nullptr == p_chunk) {
			p_chunk = chunkPool.Alloc();
			p_chunk->Set(&chunkPool, useCtor, tlsIndex);
			TlsSetValue(tlsIndex, p_chunk);
		}

		return p_chunk->Alloc();
	}

	void Free(T* p_object) {
#if USE_COUNT
		InterlockedDecrement((LONG*)&count);
#endif

		ChunkData* p_chunkData = (ChunkData*)((char*)p_object - sizeof(Chunk*));
		p_chunkData->Free();
	}
};
