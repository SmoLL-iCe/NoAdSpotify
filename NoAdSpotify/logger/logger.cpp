#include "logger.h"
#include "../utils/mem.hpp"
#include "../utils/utils.h"

logger* l = nullptr;



logger::logger( )
{
	auto main = GetModuleHandle( nullptr );

	auto get_function_start = [=]( uint8_t* ptr ) -> uint8_t*
	{
		for ( size_t i = 0; i < 0x1000; ++i )
		{
			if ( *reinterpret_cast<uint32_t*>( ptr - i ) == 0x6AEC8B55 ) return ptr - i;
		}

		return nullptr;
	};

	m_jne_ad_missing_id			= mem::pattern_scan( main, 0, "0F 85 ?? ?? ?? ?? A1 ?? ?? ?? ?? F3 0F 7E 05 ?? ?? ?? ?? 89 45 E0 A0" );

	m_mov_skip_stuck_seconds	= mem::pattern_scan( main, 0, "B9 E8 03 00 00 F7 E9 83" );

	m_fn_is_skippable			= mem::pattern_scan( main, 0, "83 C4 10 BB ? ? ? ? 83 78 14 10" );

	m_fn_now_playing			= mem::pattern_scan( main, 0, "8B C1 89 85 ? ? ? ? 8B 55 08 C7 85" );

	m_fn_require_focus			= mem::pattern_scan( main, 0, "72 65 71 75 69 72 65 5F 66 6F 63 75 73 00 00 00 0D" );

	if ( m_fn_is_skippable )
		m_fn_is_skippable		= get_function_start( m_fn_is_skippable );

	if ( m_fn_now_playing )
		m_fn_now_playing		= get_function_start( m_fn_now_playing );

	if ( m_mov_skip_stuck_seconds )
		m_mov_skip_stuck_seconds += 1;

	if ( m_fn_require_focus )
	{
		auto pattern = new uint8_t[ 5 ];

		pattern[ 0 ] = 0x68;

		*reinterpret_cast<uint8_t**>( &pattern[ 1 ] ) = m_fn_require_focus;

		m_fn_require_focus = mem::find_pattern( main, mem::get_mod_size( main ), pattern, 5 );
	}

	if ( m_fn_require_focus )
		m_fn_require_focus = get_function_start( m_fn_require_focus );

	//printf( "m_jne_ad_missing_id 0x%p\n", m_jne_ad_missing_id );
	//printf( "m_mov_skipStuckSeconds 0x%p\n", m_mov_skipStuckSeconds );
	//printf( "m_fn_is_skippable 0x%p\n", m_fn_is_skippable );
	//printf( "m_fn_now_playing 0x%p\n", m_fn_now_playing );
	//printf( "m_fn_require_focus 0x%p\n", m_fn_require_focus );

}

bool logger::valid_ptrs( )
{
	return ( m_jne_ad_missing_id  && m_mov_skip_stuck_seconds && m_fn_is_skippable && m_fn_now_playing && m_fn_require_focus );
}

logger::~logger( ) = default;

logger* logger::instance( )
{
	if ( l == nullptr )
		l = new logger( );
	return l;
}


/*

	ad missing id
	Spotify.exe+2A26D8 - 0F85 BB000000         - jne Spotify.exe+2A2799
	Spotify.exe+2A26DE - A1 30142D01           - mov eax,[Spotify.exe+ED1430] { ("ng id") }
	Spotify.exe+2A26E3 - F3 0F7E 05 28142D01   - movq xmm0,[Spotify.exe+ED1428] { ("ad missing id") }
	Spotify.exe+2A26EB - 89 45 E0              - mov [ebp-20],eax
	Spotify.exe+2A26EE - A0 34142D01           - mov al,[Spotify.exe+ED1434] { (100) }

	skipStuckSeconds
	Spotify.exe+2B0BF0 - B9 E8030000           - mov ecx,000003E8 { 1000 }
	Spotify.exe+2B0BF5 - F7 E9                 - imul ecx
	Spotify.exe+2B0BF7 - 83 C4 1C              - add esp,1C { 28 }
	Spotify.exe+2B0BFA - 89 56 04              - mov [esi+04],edx
	Spotify.exe+2B0BFD - 8B 55 D4              - mov edx,[ebp-2C]
	Spotify.exe+2B0C00 - 89 06                 - mov [esi],eax
	Spotify.exe+2B0C02 - 83 FA 10              - cmp edx,10 { 16 }

	skippable
	Spotify.exe+29FFF0 - 55                    - push ebp
	Spotify.exe+29FFF1 - 8B EC                 - mov ebp,esp
	Spotify.exe+29FFF3 - 6A FF                 - push -01 { 255 }
	Spotify.exe+29FFF5 - 68 FE011C01           - push Spotify.exe+DC01FE { (144) }
	Spotify.exe+29FFFA - 64 A1 00000000        - mov eax,fs:[00000000] { 0 }
	Spotify.exe+2A0000 - 50                    - push eax
	Spotify.exe+2A0001 - 83 EC 3C              - sub esp,3C { 60 }
	Spotify.exe+2A0004 - A1 18D05401           - mov eax,[Spotify.exe+114D018] { (738095117) }
	Spotify.exe+2A0009 - 33 C5                 - xor eax,ebp
	Spotify.exe+2A000B - 89 45 F0              - mov [ebp-10],eax
	Spotify.exe+2A000E - 53                    - push ebx
	Spotify.exe+2A000F - 56                    - push esi
	Spotify.exe+2A0010 - 50                    - push eax
	Spotify.exe+2A0011 - 8D 45 F4              - lea eax,[ebp-0C]
	Spotify.exe+2A0014 - 64 A3 00000000        - mov fs:[00000000],eax { 0 }
	Spotify.exe+2A001A - 8B F1                 - mov esi,ecx
	Spotify.exe+2A001C - 33 DB                 - xor ebx,ebx
	Spotify.exe+2A001E - 89 5D B8              - mov [ebp-48],ebx
	Spotify.exe+2A0021 - 8B 06                 - mov eax,[esi]
	Spotify.exe+2A0023 - 8B 40 64              - mov eax,[eax+64]
	Spotify.exe+2A0026 - FF D0                 - call eax
	Spotify.exe+2A0028 - 84 C0                 - test al,al
	Spotify.exe+2A002A - 75 6D                 - jne Spotify.exe+2A0099
	Spotify.exe+2A002C - A1 F4452B01           - mov eax,[Spotify.exe+EB45F4] { ("false") }
	Spotify.exe+2A0031 - 89 45 D8              - mov [ebp-28],eax
	Spotify.exe+2A0034 - A0 F8452B01           - mov al,[Spotify.exe+EB45F8] { (101) }
	Spotify.exe+2A0039 - C7 45 EC 0F000000     - mov [ebp-14],0000000F { 15 }
	Spotify.exe+2A0040 - C7 45 E8 05000000     - mov [ebp-18],00000005 { 5 }
	Spotify.exe+2A0047 - 88 45 DC              - mov [ebp-24],al
	Spotify.exe+2A004A - 88 5D DD              - mov [ebp-23],bl
	Spotify.exe+2A004D - 8D 45 D8              - lea eax,[ebp-28]
	Spotify.exe+2A0050 - 89 5D FC              - mov [ebp-04],ebx
	Spotify.exe+2A0053 - 50                    - push eax
	Spotify.exe+2A0054 - 68 60005501           - push Spotify.exe+1150060 { ("skippable") }

	now_playing
	Spotify.exe+C9D84 - A1 B07C2B01           - mov eax,[Spotify.exe+EB7CB0] { ("ing_uri") }
	Spotify.exe+C9D89 - F3 0F7E 05 A87C2B01   - movq xmm0,[Spotify.exe+EB7CA8] { ("now_playing_uri") }
	Spotify.exe+C9D91 - 89 45 E0              - mov [ebp-20],eax
	Spotify.exe+C9D94 - 66 A1 B47C2B01        - mov ax,[Spotify.exe+EB7CB4] { (00697275) }
	Spotify.exe+C9D9A - 66 89 45 E4           - mov [ebp-1C],ax
	Spotify.exe+C9D9E - A0 B67C2B01           - mov al,[Spotify.exe+EB7CB6] { (105) }

	require focus
	Spotify.exe+29FE09 - 68 A8005501           - push Spotify.exe+11500A8 { ("require_focus") }
	Spotify.exe+29FE0E - 50                    - push eax
	Spotify.exe+29FE0F - 8D 45 C0              - lea eax,[ebp-40]
	Spotify.exe+29FE12 - 50                    - push eax
	Spotify.exe+29FE13 - E8 0837E6FF           - call Spotify.exe+103520
	Spotify.exe+29FE18 - 83 C4 10              - add esp,10 { 16 }
	Spotify.exe+29FE1B - 83 78 14 10           - cmp dword ptr [eax+14],10 { 16 }
	Spotify.exe+29FE1F - 8B 48 10              - mov ecx,[eax+10]
	Spotify.exe+29FE22 - 72 02                 - jb Spotify.exe+29FE26
	Spotify.exe+29FE24 - 8B 00                 - mov eax,[eax]
	Spotify.exe+29FE26 - 6A 04                 - push 04 { 4 }
	Spotify.exe+29FE28 - 68 FC452B01           - push Spotify.exe+EB45FC { ("true") }
	Spotify.exe+29FE2D - 51                    - push ecx


*/