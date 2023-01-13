//#include <iostream>
//#include <Windows.h>
//#include <synchapi.h>
//#include <thread>
//#include <stack>
//#include <mutex>
//using namespace std;
//
//#define MAX_SESSION 5
//constexpr UINT64	INVALID_SESSION_ID = 0;
//
//union SESSION_ID {
//public:
//	SESSION_ID(UINT64 value) { session_id = value; }
//	SESSION_ID() { session_id = INVALID_SESSION_ID; }
//	SESSION_ID(DWORD index, DWORD unique_no) { this->s.index = index, this->s.unique = unique_no; }
//
//public:
//	struct { DWORD index, unique; } s;
//	UINT64	session_id = 0;
//#define		session_index  s.index   
//#define		session_unique s.unique
//
//public:
//	void operator=(UINT64 value) {
//		session_id = value;
//	}
//
//	operator UINT64() {
//		return session_id;
//	}
//
//	void operator=(const SESSION_ID& other) {
//		session_id = other.session_id;
//	}
//};
//
//struct Session {
//	alignas(128) SESSION_ID	session_id = INVALID_SESSION_ID;
//	alignas(4)	bool release_flag = false;
//	alignas(4) LONG io_count = 0;
//
//	bool send_flag = false;
//	bool use = false; // 테스트 용
//};
//
//Session* session_array[MAX_SESSION];
//
//mutex stack_lock;
//stack<DWORD> index_stack;
//void index_push(DWORD data) {
//	stack_lock.lock();
//	index_stack.push(data);
//	stack_lock.unlock();
//}
//bool index_pop(DWORD* data) {
//	stack_lock.lock();
//	if (index_stack.empty()) {
//		stack_lock.unlock();
//		return false;
//	}
//	*data = index_stack.top();
//	index_stack.pop();
//	stack_lock.unlock();
//	return true;
//}
//
//bool release(Session* p_session) {
//	if (0 < p_session->io_count)
//		return false;
//
//	// IO, 릴리즈 둘다 진행중이지 않음
//	// 릴리즈 플래그 Off, IO Count 0 -> 릴리즈 플레그 On, IO Count 0
//	if (0 == InterlockedCompareExchange64((long long*)&p_session->release_flag, 1, 0)) {
//		// closesocket
//		auto index = p_session->session_id.s.index;
//		p_session->session_id = INVALID_SESSION_ID;
//		p_session->use = false;
//		index_push(index);
//		return true;
//	}
//	else
//		return false;
//}
//
//SESSION_ID Get_SessionID() {
//	static DWORD unique = 1;
//
//	DWORD index;
//	if (false == index_pop(&index)) {
//		return INVALID_SESSION_ID;
//	}
//
//	SESSION_ID session_id(index, unique++);
//	return session_id;
//}
//
//bool accept() {
//	auto id = Get_SessionID();
//	if (INVALID_SESSION_ID == id) return false;
//	auto p_session = session_array[id.s.index];
//
//	// session set
//	p_session->session_id = id;
//	p_session->release_flag = false;
//	p_session->io_count = 1;
//	p_session->use = true;
//	return true;
//}
//
//bool SendPacket(SESSION_ID id) {
//	// 세션 검색
//	Session* p_session = session_array[id.session_index];
//
//	if (InterlockedExchange8((char*)&p_session->send_flag, true) == true)
//		return false;
//
//	// <- 릴리즈 인터락 성공
//	InterlockedIncrement((LONG*)&p_session->io_count);
//	// 재사용된 세션인지 판단
//	if (p_session->session_id != id) {
//		// <- 릴리즈 세션정리 성공, 세션 재사용
//		if (0 == InterlockedDecrement((LONG*)&p_session->io_count)) {
//			release(p_session);
//			return false;
//		}
//	}
//
//	// 여기서부터는 절대 재사용된 세션이 아님 (iocount 0이 나올 수 없음, 릴리즈 플레그 true일 수 없음)
//	// WSASend()
//
//	if (0 == InterlockedDecrement((LONG*)&p_session->io_count)) {
//		release(p_session);
//		return false;
//	}
//}
//
//void f() {
//	int a = 3;
//}
//
//void f2() {
//	return f();
//}
//
//int main() {
//	//for (int i = 0; i < MAX_SESSION; i++) {
//	//	index_stack.push(i);
//	//}
//
//	f2();
//	cout << 1 << endl;
//}