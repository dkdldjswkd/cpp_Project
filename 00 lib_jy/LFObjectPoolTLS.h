#pragma once
#include "LFObjectPool.h"

#define CHUNCK_SIZE 500

template <typename T>
class LFObjectPoolTLS {
public:
	LFObjectPoolTLS(bool use_ctor = false) : use_ctor(use_ctor), tlsIndex(TlsAlloc()) {};
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
		bool use_ctor;
		int tlsIndex;

	private:
		ChunkData chunkData_array[CHUNCK_SIZE];
		int alloc_index;
		alignas(64) int free_index;

	public:
		void Set(LFObjectPool<Chunk>* _p_chunkPool, bool _use_ctor, int _tlsIndex) {
			p_chunkPool = _p_chunkPool;
			use_ctor = _use_ctor;
			tlsIndex = _tlsIndex;

			alloc_index = 0;
			free_index = 0;
			for (int i = 0; i < CHUNCK_SIZE; i++) {
				chunkData_array[i].p_myChunk = this;
			}
		}

		T* Alloc() {
			T* p_object = &(chunkData_array[alloc_index++].object);
			if (use_ctor) new (p_object) T;

			if (CHUNCK_SIZE == alloc_index) {
				Chunk* p_chunk = p_chunkPool->Alloc();
				p_chunk->Set(p_chunkPool, use_ctor, tlsIndex);
				TlsSetValue(tlsIndex, (LPVOID)p_chunk);
			}
			return p_object;
		}

		void Free(T* p_object) {
			if (use_ctor) p_object->~T();

			if (CHUNCK_SIZE == InterlockedIncrement((DWORD*)&free_index)) {
				p_chunkPool->Free(this);
			}
		}
	};

private:
	LFObjectPool<Chunk> chunkPool;
	const int tlsIndex;
	const bool use_ctor;

public:
	int GetChunkCapacity() {
		return chunkPool.GetCapacityCount();
	}

	int GetChunkUseCount() {
		return chunkPool.GetUseCount();
	}

	int GetUseCount() {
		return GetChunkUseCount() * CHUNCK_SIZE;
	}

	T* Alloc() {
		Chunk* p_chunk = (Chunk*)TlsGetValue(tlsIndex);
		if (nullptr == p_chunk) {
			p_chunk = chunkPool.Alloc();
			p_chunk->Set(&chunkPool, use_ctor, tlsIndex);
			TlsSetValue(tlsIndex, p_chunk);
		}

		return p_chunk->Alloc();
	}

	void Free(T* p_object) {
		ChunkData* p_chunkData = (ChunkData*)((char*)p_object - sizeof(Chunk*));
		p_chunkData->Free();
	}
};