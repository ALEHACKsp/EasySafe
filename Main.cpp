#include <iostream>
#include <EasySafe.hpp>

int main() {

	/*
	* Setup EasySafe
	* --------------
	* First parameter is the payload
	* --------------
	* struct Payload {
	*	bool tests = false;
	*	bool syscall_hooking = false;
	* };
	*/
	auto instance = (new II::EasySafe({ true, true }));
	/*
	* Have a fantasy to do before the
	* protection instance is started?
	*/
	instance->beforeStart([&]() {
		std::cout << "Attempting to start EasySafe instance..." << std::endl;
	});
	/*
	* Implement a syshook
	* In case of a possible call, 
	* we will call the given callback.
	*/
	instance->AddSysHook((__int64)GetProcAddress(GetModuleHandleA("ntdll"), "NtQueryVirtualMemory"));
	/*
	* Setup our sys callback
	*/
	instance->onSysHook([&](PSYMBOL_INFO symbol_info, uintptr_t R10, uintptr_t RAX) {
	// Print what we know
		std::cout << "function: " << symbol_info->Name << " return value: " << std::hex << RAX << " return address:" << R10;
		II::EasySafe::RegisterPayload spoof({
			true,
			R10,			// Original R10
			0xDEADBEEF		// spoofed RAX
		});
		return spoof;
	});
	/*
	* Have a fantasy to do after the 
	* protection instance is started?
	*/
	instance->afterStart([&]() {
		std::cout << "EasySafe instance started successfully!" << std::endl;
	});

	instance->Init();

	// Run hooked function to test the hook
	MEMORY_BASIC_INFORMATION region = { nullptr };
	const auto status = NtQueryVirtualMemory(GetCurrentProcess(), GetModuleHandle(nullptr), MemoryBasicInformation, &region, sizeof(region), nullptr);
	// Print spoofed status
	std::cout << "[UNSAFE] NtQVM status: " << std::hex << status << std::endl;

	// Crash inline syscalls ( will be crash about 0xC0000005 )
	// MEMORY_BASIC_INFORMATION region2 = { nullptr };
	// const auto InlineStatus = INLINE_SYSCALL(NtQueryVirtualMemory)(GetCurrentProcess(), GetModuleHandle(nullptr), MemoryBasicInformation, &region2, sizeof(region2), nullptr);
	// std::cout << "[+] NtQVM status: " << std::hex << InlineStatus << std::endl;

	// Safe syscalls 
	instance->SafeSyscall([&]() {
		// Run hooked function to test the hook
		MEMORY_BASIC_INFORMATION region2 = { nullptr };
		const auto status2 = NtQueryVirtualMemory(GetCurrentProcess(), GetModuleHandle(nullptr), MemoryBasicInformation, &region2, sizeof(region2), nullptr);
		// Print spoofed status
		std::cout << "[ SAFE ] NtQVM status2: " << std::hex << status2 << std::endl;
	});

	return 0;
}