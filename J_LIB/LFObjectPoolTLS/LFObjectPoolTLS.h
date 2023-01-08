#pragma once
#include "LFObjectPool.h"

// 오후 11:07 2023-01-03

#define CHUNCK_SIZE 500

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
		// 청크에서 할당해줄 데이터가 없다면 ret nullptr
		T* Alloc() {
			// 할당 성공
			if (alloc_count < CHUNCK_SIZE) {
				ChunkData* p_chunkData = &chunkData_array[alloc_count++];
				T* p_object = &p_chunkData->object;
				if (flag_placementNew) new (p_object) T;
				return p_object;
			}

			// 할당 실패
			return nullptr;
		}

		// true 반환 시 데이터 꽉참
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
	static J_LIB::LFObjectPool<Chunk> chunkPool;

private:
	const int tlsIndex;
	bool flag_placementNew;

private:
	static Chunk* ChunkAlloc() {
		Chunk* p_chunk = chunkPool.Alloc();
		p_chunk->Clear(flag_placementNew);
		return p_chunk;
	}

	static void ChunkFree(Chunk* p_chunk) {
		chunkPool.Free(p_chunk);
	}

public:
	int Get_ChunkCapacity() {
		return chunkPool.GetCapacityCount();
	}

	int Get_ChunkUseCount() {
		return chunkPool.GetUseCount();
	}

	T* Alloc() {
		// TLS에서 청크를 가져옴
		Chunk* p_chunk = (Chunk*)TlsGetValue(tlsIndex);
		if (p_chunk == nullptr) {
			p_chunk = ChunkAlloc();
			TlsSetValue(tlsIndex, p_chunk);
		}

		// 청크에서 오브젝트 할당
		T* p_object = p_chunk->Alloc();
		if (p_object == nullptr) {
			p_chunk = ChunkAlloc();
			TlsSetValue(tlsIndex, p_chunk);
			p_object = p_chunk->Alloc();
		}

		return p_object;
	}

	void Free(T* p_object) {
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
