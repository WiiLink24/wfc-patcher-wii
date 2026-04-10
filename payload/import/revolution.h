#pragma once

#ifdef __cplusplus
extern "C" {

namespace wwfc::RVL
{
#endif

typedef enum {
    SCLanguageJapanese = 0x00,
    SCLanguageEnglish = 0x01,
    SCLanguageGerman = 0x02,
    SCLanguageFrench = 0x03,
    SCLanguageSpanish = 0x04,
    SCLanguageItalian = 0x05,
    SCLanguageDutch = 0x06,
    SCLanguageSimplifiedChinese = 0x07,
    SCLanguageTraditionalChinese = 0x08,
    SCLanguageKorean = 0x09,
    SCLanguageCount,
} SCLanguage;

[[gnu::longcall]] bool OSDisableInterrupts( //
    void
) AT(ADDRESS_OSDisableInterrupts);

[[gnu::longcall]] void OSRestoreInterrupts( //
    bool enabled
) AT(ADDRESS_OSRestoreInterrupts);

[[gnu::longcall]] void OSReport( //
    const char* format, ...
) AT(ADDRESS_OSReport);

[[gnu::longcall]] void DCFlushRange( //
    void* ptr, u32 size
) AT(ADDRESS_DCFlushRange);

[[gnu::longcall]] void ICInvalidateRange( //
    void* ptr, u32 size
) AT(ADDRESS_ICInvalidateRange);

typedef struct {
    void* data;
    u32 size;
} IOVector;

[[gnu::longcall]] s32 IOS_Open( //
    const char* path, u32 flags
) AT(ADDRESS_IOS_Open);

[[gnu::longcall]] s32 IOS_Close( //
    s32 fd
) AT(ADDRESS_IOS_Close);

[[gnu::longcall]] s32 IOS_Ioctlv( //
    s32 fd, u32 cmd, u32 in_count, u32 out_count, IOVector* vec
) AT(ADDRESS_IOS_Ioctlv);

#if RMC

[[gnu::longcall]] u8 SCGetLanguage( //
    void
) AT(RMCXD_PORT(0x801B1D0C, 0x801B1C6C, 0x801B1C2C, 0x801B2068, 0x801B160C));

#endif // RMC

[[gnu::longcall]] bool SCGetProductSN( //
    u32* serial
) AT(ADDRESS_SCGetProductSN);

#ifdef __cplusplus
}
}
#endif
