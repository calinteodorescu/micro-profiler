if (MSVC)
	enable_language(ASM_MASM)
else()
	enable_language(ASM-ATT)
endif()

if (MSVC)
	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(ASMEXT _x86.asm)
	else()
		set(ASMEXT _x64.asm)
	endif()
else()
	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(ASMEXT _x86.s)
	else()
		set(ASMEXT _x64.s)
	endif()
	if (NOT APPLE)
		if (CMAKE_SIZEOF_VOID_P EQUAL 4)
			set(CMAKE_ASM-ATT_FLAGS ${CMAKE_ASM-ATT_FLAGS} --32)
		else()
			set(CMAKE_ASM-ATT_FLAGS ${CMAKE_ASM-ATT_FLAGS} --64)
		endif()
	endif()
endif()
